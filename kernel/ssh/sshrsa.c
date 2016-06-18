/*
* RSA implementation
*/


#include "ssh_def.h"
//#include "misc.h"


#define BIGNUM_INT_BITS 16
#define BIGNUM_INT_BYTES (BIGNUM_INT_BITS / 8)
#define rol(x,y) ( ((x) << (y)) | (((uint32)x) >> (32-y)) )

extern Bignum Zero;
extern ssh_hash ssh_sha1;
extern ssh_hash ssh_sha256;



Bignum modmul(Bignum p, Bignum q, Bignum mod);
Bignum modpow(Bignum base_in, Bignum exp, Bignum mod);
Bignum bigmod(Bignum a, Bignum b);


static void sha512_mpint(SHA512_State * s, Bignum b)
{
	unsigned char lenbuf[4];
	int len;
	len = (bignum_bitcount(b) + 8) / 8;
	PUT_32BIT(lenbuf, len);
	SHA512_Bytes(s, lenbuf, 4);
	while (len-- > 0) {
		lenbuf[0] = bignum_byte(b, len);
		SHA512_Bytes(s, lenbuf, 1);
	}
	smemclr(lenbuf, sizeof(lenbuf));
}

Bignum newbn(int length)
{
	Bignum b;

	//assert(length >= 0 && length < INT_MAX / BIGNUM_INT_BITS);

	b = (Bignum)ssh_new((length + 1)*sizeof(BignumInt));// snewn(length + 1, BignumInt);
	//if (!b)
	//	abort();		       /* FIXME */
	memset(b, 0, (length + 1) * sizeof(*b));
	b[0] = length;

	return b;
}





void freersakey( RSAKey *key)
{
	if (key->modulus)
		freebn(key->modulus);
	if (key->exponent)
		freebn(key->exponent);
	if (key->private_exponent)
		freebn(key->private_exponent);
	if (key->p)
		freebn(key->p);
	if (key->q)
		freebn(key->q);
	if (key->iqmp)
		freebn(key->iqmp);
	if (key->comment)
		ssh_free(key->comment);
}

int rsastr_len( RSAKey *key)
{
	Bignum md, ex;
	int mdlen, exlen;

	md = key->modulus;
	ex = key->exponent;
	mdlen = (bignum_bitcount(md) + 15) / 16;
	exlen = (bignum_bitcount(ex) + 15) / 16;
	return 4 * (mdlen + exlen) + 20;
}



void rsastr_fmt(char *str,  RSAKey *key)
{
	Bignum md, ex;
	int len = 0, i, nibbles;
	static const char hex[] = "0123456789abcdef";

	md = key->modulus;
	ex = key->exponent;

	len += sprintf(str + len, "0x");

	nibbles = (3 + bignum_bitcount(ex)) / 4;
	if (nibbles < 1)
		nibbles = 1;
	for (i = nibbles; i--;)
		str[len++] = hex[(bignum_byte(ex, i / 2) >> (4 * (i % 2))) & 0xF];

	len += sprintf(str + len, ",0x");

	nibbles = (3 + bignum_bitcount(md)) / 4;
	if (nibbles < 1)
		nibbles = 1;
	for (i = nibbles; i--;)
		str[len++] = hex[(bignum_byte(md, i / 2) >> (4 * (i % 2))) & 0xF];

	str[len] = '\0';
}

/* ----------------------------------------------------------------------
* Implementation of the ssh-rsa signing key type. 
*/

static void getstring(char **data, int *datalen, char **p, int *length)
{
	*p = NULL;
	if (*datalen < 4)
		return;
	*length = toint(GET_32BIT(*data));
	if (*length < 0)
		return;
	*datalen -= 4;
	*data += 4;
	if (*datalen < *length)
		return;
	*p = *data;
	*data += *length;
	*datalen -= *length;
}

static Bignum getmp(char **data, int *datalen)
{
	char *p;
	int length;
	Bignum b;

	getstring(data, datalen, &p, &length);
	if (!p) 		return NULL;

	b = bignum_from_bytes((unsigned char *)p, length);

	return b;
}


/*
 * Compute (base ^ exp) % mod, provided mod == p * q, with p,q
 * distinct primes, and iqmp is the multiplicative inverse of q mod p.
 * Uses Chinese Remainder Theorem to speed computation up over the
 * obvious implementation of a single big modpow.
 */
Bignum crt_modpow(Bignum base, Bignum exp, Bignum mod,
                  Bignum p, Bignum q, Bignum iqmp)
{
    Bignum pm1, qm1, pexp, qexp, presult, qresult, diff, multiplier, ret0, ret;

    /*
     * Reduce the exponent mod phi(p) and phi(q), to save time when
     * exponentiating mod p and mod q respectively. Of course, since p
     * and q are prime, phi(p) == p-1 and similarly for q.
     */
    pm1 = copybn(p);
    decbn(pm1);
    qm1 = copybn(q);
    decbn(qm1);
    pexp = bigmod(exp, pm1);
    qexp = bigmod(exp, qm1);

    /*
     * Do the two modpows.
     */
    presult = modpow(base, pexp, p);
    qresult = modpow(base, qexp, q);

    /*
     * Recombine the results. We want a value which is congruent to
     * qresult mod q, and to presult mod p.
     *
     * We know that iqmp * q is congruent to 1 * mod p (by definition
     * of iqmp) and to 0 mod q (obviously). So we start with qresult
     * (which is congruent to qresult mod both primes), and add on
     * (presult-qresult) * (iqmp * q) which adjusts it to be congruent
     * to presult mod p without affecting its value mod q.
     */
    if (bignum_cmp(presult, qresult) < 0) {
        /*
         * Can't subtract presult from qresult without first adding on
         * p.
         */
        Bignum tmp = presult;
        presult = bigadd(presult, p);
        freebn(tmp);
    }
    diff = bigsub(presult, qresult);
    multiplier = bigmul(iqmp, q);
    ret0 = bigmuladd(multiplier, diff, qresult);

    /*
     * Finally, reduce the result mod n.
     */
    ret = bigmod(ret0, mod);

    /*
     * Free all the intermediate results before returning.
     */
    freebn(pm1);
    freebn(qm1);
    freebn(pexp);
    freebn(qexp);
    freebn(presult);
    freebn(qresult);
    freebn(diff);
    freebn(multiplier);
    freebn(ret0);

    return ret;
}

/*
 * This function is a wrapper on modpow(). It has the same effect as
 * modpow(), but employs RSA blinding to protect against timing
 * attacks and also uses the Chinese Remainder Theorem (implemented
 * above, in crt_modpow()) to speed up the main operation.
 */
static Bignum rsa_privkey_op(Bignum input, RSAKey *key)
{
    Bignum random, random_encrypted, random_inverse;
    Bignum input_blinded, ret_blinded;
    Bignum ret;

    SHA512_State ss;
    unsigned char digest512[64];
    int digestused = lenof(digest512);
    int hashseq = 0;

    /*
     * Start by inventing a random number chosen uniformly from the
     * range 2..modulus-1. (We do this by preparing a random number
     * of the right length and retrying if it's greater than the
     * modulus, to prevent any potential Bleichenbacher-like
     * attacks making use of the uneven distribution within the
     * range that would arise from just reducing our number mod n.
     * There are timing implications to the potential retries, of
     * course, but all they tell you is the modulus, which you
     * already knew.)
     * 
     * To preserve determinism and avoid Pageant needing to share
     * the random number pool, we actually generate this `random'
     * number by hashing stuff with the private key.
     */
    while (1) {
	int bits, byte, bitsleft, v;
	random = copybn(key->modulus);
	/*
	 * Find the topmost set bit. (This function will return its
	 * index plus one.) Then we'll set all bits from that one
	 * downwards randomly.
	 */
	bits = bignum_bitcount(random);
	byte = 0;
	bitsleft = 0;
	while (bits--) {
	    if (bitsleft <= 0) {
		bitsleft = 8;
		/*
		 * Conceptually the following few lines are equivalent to
		 *    byte = random_byte();
		 */
		if (digestused >= lenof(digest512)) {
		    unsigned char seqbuf[4];
		    PUT_32BIT(seqbuf, hashseq);
		    SHA512_Init(&ss);
		    SHA512_Bytes(&ss, "RSA deterministic blinding", 26);
		    SHA512_Bytes(&ss, seqbuf, sizeof(seqbuf));
		    sha512_mpint(&ss, key->private_exponent);
		    SHA512_Final(&ss, digest512);
		    hashseq++;

		    /*
		     * Now hash that digest plus the signature
		     * input.
		     */
		    SHA512_Init(&ss);
		    SHA512_Bytes(&ss, digest512, sizeof(digest512));
		    sha512_mpint(&ss, input);
		    SHA512_Final(&ss, digest512);

		    digestused = 0;
		}
		byte = digest512[digestused++];
	    }
	    v = byte & 1;
	    byte >>= 1;
	    bitsleft--;
	    bignum_set_bit(random, bits, v);
	}
        bn_restore_invariant(random);

	/*
	 * Now check that this number is strictly greater than
	 * zero, and strictly less than modulus.
	 */
	if (bignum_cmp(random, Zero) <= 0 ||
	    bignum_cmp(random, key->modulus) >= 0) {
	    freebn(random);
	    continue;
	}

        /*
         * Also, make sure it has an inverse mod modulus.
         */
        random_inverse = modinv(random, key->modulus);
        if (!random_inverse) {
	    freebn(random);
	    continue;
        }

        break;
    }

    /*
     * RSA blinding relies on the fact that (xy)^d mod n is equal
     * to (x^d mod n) * (y^d mod n) mod n. We invent a random pair
     * y and y^d; then we multiply x by y, raise to the power d mod
     * n as usual, and divide by y^d to recover x^d. Thus an
     * attacker can't correlate the timing of the modpow with the
     * input, because they don't know anything about the number
     * that was input to the actual modpow.
     * 
     * The clever bit is that we don't have to do a huge modpow to
     * get y and y^d; we will use the number we just invented as
     * _y^d_, and use the _public_ exponent to compute (y^d)^e = y
     * from it, which is much faster to do.
     */
    random_encrypted = crt_modpow(random, key->exponent,
                                  key->modulus, key->p, key->q, key->iqmp);
    input_blinded = modmul(input, random_encrypted, key->modulus);
    ret_blinded = crt_modpow(input_blinded, key->private_exponent,
                             key->modulus, key->p, key->q, key->iqmp);
    ret = modmul(ret_blinded, random_inverse, key->modulus);

    freebn(ret_blinded);
    freebn(input_blinded);
    freebn(random_inverse);
    freebn(random_encrypted);
    freebn(random);

    return ret;
}

static void rsa2_freekey(void *key);   /* forward reference */

void* rsa2_newkey(char *data, int len)
{
	char *p;
	int slen;
	RSAKey *rsa;

	rsa = (RSAKey*)ssh_new( sizeof(RSAKey));
	getstring(&data, &len, &p, &slen);

	if (!p || slen != 7 || memcmp(p, "ssh-rsa", 7)) 
	{
		ssh_free(rsa);
		return NULL;
	}

	rsa->exponent = getmp(&data, &len);
	rsa->modulus = getmp(&data, &len);
	rsa->private_exponent = NULL;
	rsa->p = rsa->q = rsa->iqmp = NULL;
	rsa->comment = NULL;


	if (!rsa->exponent || !rsa->modulus) 
	{
		rsa2_freekey(rsa);
		return NULL;
	}

	return rsa;
}

static void rsa2_freekey(void *key)
{
	RSAKey *rsa = (RSAKey *) key;

	freersakey(rsa);
	ssh_free(rsa);
}

static char *rsa2_fmtkey(void *key)
{
	RSAKey *rsa = (RSAKey *) key;
	char *p;
	int len;

	len = rsastr_len(rsa);

	p = (char*)ssh_new(len);

	rsastr_fmt(p, rsa);

	return p;
}

static unsigned char *rsa2_public_blob(void *key, int *len)
{
	RSAKey *rsa = (RSAKey *) key;
	int elen, mlen, bloblen;
	int i;
	unsigned char *blob, *p;

	elen = (bignum_bitcount(rsa->exponent) + 8) / 8;
	mlen = (bignum_bitcount(rsa->modulus) + 8) / 8;

	/*
	* string "ssh-rsa", mpint exp, mpint mod. Total 19+elen+mlen.
	* (three length fields, 12+7=19).
	*/
	bloblen = 19 + elen + mlen;
	blob = snewn(bloblen, unsigned char);
	p = blob;
	PUT_32BIT(p, 7);
	p += 4;
	memcpy(p, "ssh-rsa", 7);
	p += 7;
	PUT_32BIT(p, elen);
	p += 4;
	for (i = elen; i--;)
		*p++ = bignum_byte(rsa->exponent, i);
	PUT_32BIT(p, mlen);
	p += 4;
	for (i = mlen; i--;)
		*p++ = bignum_byte(rsa->modulus, i);
	
	//assert(p == blob + bloblen);
	*len = bloblen;

	return blob;
}

static unsigned char *rsa2_private_blob(void *key, int *len)
{
	RSAKey *rsa = (RSAKey *) key;
	int dlen, plen, qlen, ulen, bloblen;
	int i;
	unsigned char *blob, *p;

	dlen = (bignum_bitcount(rsa->private_exponent) + 8) / 8;
	plen = (bignum_bitcount(rsa->p) + 8) / 8;
	qlen = (bignum_bitcount(rsa->q) + 8) / 8;
	ulen = (bignum_bitcount(rsa->iqmp) + 8) / 8;

	/*
	* mpint private_exp, mpint p, mpint q, mpint iqmp. Total 16 +
	* sum of lengths.
	*/
	bloblen = 16 + dlen + plen + qlen + ulen;
	blob = snewn(bloblen, unsigned char);
	p = blob;
	PUT_32BIT(p, dlen);
	p += 4;
	for (i = dlen; i--;)
		*p++ = bignum_byte(rsa->private_exponent, i);
	PUT_32BIT(p, plen);
	p += 4;
	for (i = plen; i--;)
		*p++ = bignum_byte(rsa->p, i);
	PUT_32BIT(p, qlen);
	p += 4;
	for (i = qlen; i--;)
		*p++ = bignum_byte(rsa->q, i);
	PUT_32BIT(p, ulen);
	p += 4;
	for (i = ulen; i--;)
		*p++ = bignum_byte(rsa->iqmp, i);
//	assert(p == blob + bloblen);
	*len = bloblen;
	return blob;
}

static void *rsa2_createkey(unsigned char *pub_blob, int pub_len,unsigned char *priv_blob, int priv_len)
{
	RSAKey* rsa;
	char *pb = (char *) priv_blob;

	rsa = (RSAKey*)rsa2_newkey((char *) pub_blob, pub_len);
	rsa->private_exponent = getmp(&pb, &priv_len);
	rsa->p = getmp(&pb, &priv_len);
	rsa->q = getmp(&pb, &priv_len);
	rsa->iqmp = getmp(&pb, &priv_len);

	/*if (!rsa_verify(rsa)) 
	{
		rsa2_freekey(rsa);
		return NULL;
	}*/

	return rsa;
}

static void *rsa2_openssh_createkey(unsigned char **blob, int *len)
{
	char **b = (char **) blob;
	RSAKey *rsa;

	rsa = snewn(1,RSAKey);
	rsa->comment = NULL;

	rsa->modulus = getmp(b, len);
	rsa->exponent = getmp(b, len);
	rsa->private_exponent = getmp(b, len);
	rsa->iqmp = getmp(b, len);
	rsa->p = getmp(b, len);
	rsa->q = getmp(b, len);

	if (!rsa->modulus || !rsa->exponent || !rsa->private_exponent || 	!rsa->iqmp || !rsa->p || !rsa->q) 
	{
			rsa2_freekey(rsa);
			return NULL;
	}

	/*if (!rsa_verify(rsa)) {
		rsa2_freekey(rsa);
		return NULL;
	}*/

	return rsa;
}

static int rsa2_encblob(uint8* blobbuf,int* bloblen,Bignum x)
{
	int  i;

	PUT_32BIT(blobbuf+*bloblen, ssh2_bignum_length(x)-4); 
	*bloblen += 4; 

	for (i = ssh2_bignum_length(x)-4; i-- ;) 
	{
		blobbuf[*bloblen]=bignum_byte(x,i);

		*bloblen ++;
	}

	return 0;
}
static int rsa2_openssh_fmtkey(void *key, unsigned char *blob, int len)
{
	RSAKey *rsa = ( RSAKey *) key;
	int bloblen;

	bloblen =
		ssh2_bignum_length(rsa->modulus) +
		ssh2_bignum_length(rsa->exponent) +
		ssh2_bignum_length(rsa->private_exponent) +
		ssh2_bignum_length(rsa->iqmp) +
		ssh2_bignum_length(rsa->p) + ssh2_bignum_length(rsa->q);

	if (bloblen > len) 	return bloblen;
		
	/*
#define ENC(x) \ 
	PUT_32BIT(blob+bloblen, ssh2_bignum_length((x))-4); bloblen += 4; \
		for (i = ssh2_bignum_length((x))-4; i-- ;) blob[bloblen++]=bignum_byte((x),i);

	ENC(rsa->modulus);
	ENC(rsa->exponent);
	ENC(rsa->private_exponent);
	ENC(rsa->iqmp);
	ENC(rsa->p);
	ENC(rsa->q);*/

	bloblen = 0;

	rsa2_encblob(blob,&bloblen,rsa->modulus);
	rsa2_encblob(blob,&bloblen,rsa->exponent);
	rsa2_encblob(blob,&bloblen,rsa->private_exponent);
	rsa2_encblob(blob,&bloblen,rsa->iqmp);
	rsa2_encblob(blob,&bloblen,rsa->p);
	rsa2_encblob(blob,&bloblen,rsa->q);

	return bloblen;
}

static int rsa2_pubkey_bits(void *blob, int len)
{
	 RSAKey *rsa;
	int ret;

	rsa = rsa2_newkey((char *) blob, len);
	if (!rsa)
		return -1;
	ret = bignum_bitcount(rsa->modulus);
	rsa2_freekey(rsa);

	return ret;
}

static char *rsa2_fingerprint(void *key)
{
	RSAKey *rsa = (RSAKey *) key;
	 MD5Context md5c;
	unsigned char digest[16], lenbuf[4];
	char buffer[16 * 3 + 40];
	char *ret;
	int numlen, i;

	MD5Init(&md5c);
	MD5Update(&md5c, (unsigned char *)"\0\0\0\7ssh-rsa", 11);

#define ADD_BIGNUM(bignum) \
	numlen = (bignum_bitcount(bignum)+8)/8; \
	PUT_32BIT(lenbuf, numlen); MD5Update(&md5c, lenbuf, 4); \
	for (i = numlen; i-- ;) { \
	unsigned char c = bignum_byte(bignum, i); \
	MD5Update(&md5c, &c, 1); \
	}
	ADD_BIGNUM(rsa->exponent);
	ADD_BIGNUM(rsa->modulus);
#undef ADD_BIGNUM

	MD5Final(digest, &md5c);

	sprintf(buffer, "ssh-rsa %d ", bignum_bitcount(rsa->modulus));
	for (i = 0; i < 16; i++)
		sprintf(buffer + strlen(buffer), "%s%02x", i ? ":" : "",
		digest[i]);
	ret = snewn(strlen(buffer) + 1, char);
	if (ret)
		strcpy(ret, buffer);
	return ret;
}

/*
* This is the magic ASN.1/DER prefix that goes in the decoded
* signature, between the string of FFs and the actual SHA hash
* value. The meaning of it is:
* 
* 00 -- this marks the end of the FFs; not part of the ASN.1 bit itself
* 
* 30 21 -- a constructed SEQUENCE of length 0x21
*    30 09 -- a constructed sub-SEQUENCE of length 9
*       06 05 -- an object identifier, length 5
*          2B 0E 03 02 1A -- object id { 1 3 14 3 2 26 }
*                            (the 1,3 comes from 0x2B = 43 = 40*1+3)
*       05 00 -- NULL
*    04 14 -- a primitive OCTET STRING of length 0x14
*       [0x14 bytes of hash data follows]
* 
* The object id in the middle there is listed as `id-sha1' in
* ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1d2.asn (the
* ASN module for PKCS #1) and its expanded form is as follows:
* 
* id-sha1                OBJECT IDENTIFIER ::= {
*    iso(1) identified-organization(3) oiw(14) secsig(3)
*    algorithms(2) 26 }
*/
static const unsigned char asn1_weird_stuff[] = 
{
	0x00, 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B,
	0x0E, 0x03, 0x02, 0x1A, 0x05, 0x00, 0x04, 0x14,
};

#define ASN1_LEN ( (int) sizeof(asn1_weird_stuff) )

static int rsa2_verifysig(void *key, char *sig, int siglen,	char *data, int datalen)
{
	RSAKey *rsa = (RSAKey *) key;
	Bignum in, out;
	char *p;
	int slen;
	int bytes, i, j, ret;
	unsigned char hash[20];

	getstring(&sig, &siglen, &p, &slen);
	if (!p || slen != 7 || memcmp(p, "ssh-rsa", 7)) {
		return 0;
	}
	in = getmp(&sig, &siglen);
	if (!in)
		return 0;
	out = modpow(in, rsa->exponent, rsa->modulus);
	freebn(in);

	ret = 1;

	bytes = (bignum_bitcount(rsa->modulus)+7) / 8;
	/* Top (partial) byte should be zero. */
	if (bignum_byte(out, bytes - 1) != 0)
		ret = 0;
	/* First whole byte should be 1. */
	if (bignum_byte(out, bytes - 2) != 1)
		ret = 0;
	/* Most of the rest should be FF. */
	for (i = bytes - 3; i >= 20 + ASN1_LEN; i--) {
		if (bignum_byte(out, i) != 0xFF)
			ret = 0;
	}
	/* Then we expect to see the asn1_weird_stuff. */
	for (i = 20 + ASN1_LEN - 1, j = 0; i >= 20; i--, j++) {
		if (bignum_byte(out, i) != asn1_weird_stuff[j])
			ret = 0;
	}
	/* Finally, we expect to see the SHA-1 hash of the signed data. */
	SHA_Simple(data, datalen, hash);
	for (i = 19, j = 0; i >= 0; i--, j++) {
		if (bignum_byte(out, i) != hash[j])
			ret = 0;
	}
	freebn(out);

	return ret;
}

static unsigned char *rsa2_sign(void *key, char *data, int datalen,int *siglen)
{
	RSAKey *rsa = (RSAKey *) key;
	unsigned char *bytes;
	int nbytes;
	unsigned char hash[20];
	Bignum in, out;
	int i, j;

	SHA_Simple(data, datalen, hash);

	nbytes = (bignum_bitcount(rsa->modulus) - 1) / 8;
//	assert(1 <= nbytes - 20 - ASN1_LEN);
	bytes = snewn(nbytes, unsigned char);

	bytes[0] = 1;
	for (i = 1; i < nbytes - 20 - ASN1_LEN; i++)
		bytes[i] = 0xFF;
	for (i = nbytes - 20 - ASN1_LEN, j = 0; i < nbytes - 20; i++, j++)
		bytes[i] = asn1_weird_stuff[j];
	for (i = nbytes - 20, j = 0; i < nbytes; i++, j++)
		bytes[i] = hash[j];

	in = bignum_from_bytes(bytes, nbytes);
	sfree(bytes);

	out = rsa_privkey_op(in, rsa);
	freebn(in);

	nbytes = (bignum_bitcount(out) + 7) / 8;
	bytes = snewn(4 + 7 + 4 + nbytes, unsigned char);
	PUT_32BIT(bytes, 7);
	memcpy(bytes + 4, "ssh-rsa", 7);
	PUT_32BIT(bytes + 4 + 7, nbytes);
	for (i = 0; i < nbytes; i++)
		bytes[4 + 7 + 4 + i] = bignum_byte(out, nbytes - 1 - i);
	freebn(out);

	*siglen = 4 + 7 + 4 + nbytes;
	return bytes;
}


/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end, and pass those blocks to the core SHA algorithm.
 */

static void SHA_Core_Init(uint32 h[5])
{
	h[0] = 0x67452301;
	h[1] = 0xefcdab89;
	h[2] = 0x98badcfe;
	h[3] = 0x10325476;
	h[4] = 0xc3d2e1f0;
}


void SHATransform(word32 * digest, word32 * block)
{
	word32 w[80];
	word32 a, b, c, d, e;
	int t;

#ifdef RANDOM_DIAGNOSTICS
	{
		extern int random_diagnostics;
		if (random_diagnostics) {
			int i;
			printf("SHATransform:");
			for (i = 0; i < 5; i++)
				printf(" %08x", digest[i]);
			printf(" +");
			for (i = 0; i < 16; i++)
				printf(" %08x", block[i]);
		}
	}
#endif

	for (t = 0; t < 16; t++)
		w[t] = block[t];

	for (t = 16; t < 80; t++) {
		word32 tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
		w[t] = rol(tmp, 1);
	}

	a = digest[0];
	b = digest[1];
	c = digest[2];
	d = digest[3];
	e = digest[4];

	for (t = 0; t < 20; t++) {
		word32 tmp =
			rol(a, 5) + ((b & c) | (d & ~b)) + e + w[t] + 0x5a827999;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}
	for (t = 20; t < 40; t++) {
		word32 tmp = rol(a, 5) + (b ^ c ^ d) + e + w[t] + 0x6ed9eba1;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}
	for (t = 40; t < 60; t++) {
		word32 tmp = rol(a,
			5) + ((b & c) | (b & d) | (c & d)) + e + w[t] +
			0x8f1bbcdc;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}
	for (t = 60; t < 80; t++) {
		word32 tmp = rol(a, 5) + (b ^ c ^ d) + e + w[t] + 0xca62c1d6;
		e = d;
		d = c;
		c = rol(b, 30);
		b = a;
		a = tmp;
	}

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;

#ifdef RANDOM_DIAGNOSTICS
	{
		extern int random_diagnostics;
		if (random_diagnostics) {
			int i;
			printf(" =");
			for (i = 0; i < 5; i++)
				printf(" %08x", digest[i]);
			printf("\n");
		}
	}
#endif
}
void SHA_Init(SHA_State * s)
{
    SHA_Core_Init(s->h);
    s->blkused = 0;
    s->lenhi = s->lenlo = 0;
}

void SHA_Bytes(SHA_State * s, const void *p, int len)
{
    const unsigned char *q = (const unsigned char *) p;
    uint32 wordblock[16];
    uint32 lenw = len;
    int i;

    /*
     * Update the length field.
     */
    s->lenlo += lenw;
    s->lenhi += (s->lenlo < lenw);

    if (s->blkused && s->blkused + len < 64) {
	/*
	 * Trivial case: just add to the block.
	 */
	memcpy(s->block + s->blkused, q, len);
	s->blkused += len;
    } else {
	/*
	 * We must complete and process at least one block.
	 */
	while (s->blkused + len >= 64) {
	    memcpy(s->block + s->blkused, q, 64 - s->blkused);
	    q += 64 - s->blkused;
	    len -= 64 - s->blkused;
	    /* Now process the block. Gather bytes big-endian into words */
	    for (i = 0; i < 16; i++) {
		wordblock[i] =
		    (((uint32) s->block[i * 4 + 0]) << 24) |
		    (((uint32) s->block[i * 4 + 1]) << 16) |
		    (((uint32) s->block[i * 4 + 2]) << 8) |
		    (((uint32) s->block[i * 4 + 3]) << 0);
	    }
	    SHATransform(s->h, wordblock);
	    s->blkused = 0;
	}
	memcpy(s->block, q, len);
	s->blkused = len;
    }
}

void SHA_Final(SHA_State * s, unsigned char *output)
{
    int i;
    int pad;
    unsigned char c[64];
    uint32 lenhi, lenlo;

    if (s->blkused >= 56)
	pad = 56 + 64 - s->blkused;
    else
	pad = 56 - s->blkused;

    lenhi = (s->lenhi << 3) | (s->lenlo >> (32 - 3));
    lenlo = (s->lenlo << 3);

    memset(c, 0, pad);
    c[0] = 0x80;
    SHA_Bytes(s, &c, pad);

    c[0] = (lenhi >> 24) & 0xFF;
    c[1] = (lenhi >> 16) & 0xFF;
    c[2] = (lenhi >> 8) & 0xFF;
    c[3] = (lenhi >> 0) & 0xFF;
    c[4] = (lenlo >> 24) & 0xFF;
    c[5] = (lenlo >> 16) & 0xFF;
    c[6] = (lenlo >> 8) & 0xFF;
    c[7] = (lenlo >> 0) & 0xFF;

    SHA_Bytes(s, &c, 8);

    for (i = 0; i < 5; i++) {
	output[i * 4] = (s->h[i] >> 24) & 0xFF;
	output[i * 4 + 1] = (s->h[i] >> 16) & 0xFF;
	output[i * 4 + 2] = (s->h[i] >> 8) & 0xFF;
	output[i * 4 + 3] = (s->h[i]) & 0xFF;
    }
}

void SHA_Simple(const void *p, int len, unsigned char *output)
{
    SHA_State s;

    SHA_Init(&s);
    SHA_Bytes(&s, p, len);
    SHA_Final(&s, output);
    smemclr(&s, sizeof(s));
}


const ssh_signkey ssh_rsa = 
{
	rsa2_newkey,
	rsa2_freekey,
	rsa2_fmtkey,
	rsa2_public_blob,
	rsa2_private_blob,
	rsa2_createkey,
	rsa2_openssh_createkey,
	rsa2_openssh_fmtkey,
	rsa2_pubkey_bits,
	rsa2_fingerprint,
	rsa2_verifysig,
	rsa2_sign,
	"ssh-rsa",
	"rsa2"
};



static const  ssh_kex ssh_rsa_kex_sha1 = 
{
	"rsa1024-sha1", NULL, KEXTYPE_RSA, NULL, NULL, 0, 0, &ssh_sha1
};

static const  ssh_kex ssh_rsa_kex_sha256 = 
{
	"rsa2048-sha256", NULL, KEXTYPE_RSA, NULL, NULL, 0, 0, &ssh_sha256
};

static const  ssh_kex *const rsa_kex_list[] = {
	&ssh_rsa_kex_sha256,
	&ssh_rsa_kex_sha1
};

const  ssh_kexes ssh_rsa_kex = {
	sizeof(rsa_kex_list) / sizeof(*rsa_kex_list),
	rsa_kex_list
};
