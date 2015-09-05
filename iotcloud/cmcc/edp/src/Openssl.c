#ifdef _ENCRYPT
 
#include "Openssl.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define RSA_E_LEN_BYTE                          4
#define RSA_N_LEN_BYTE                          128
#define RSA_E                                   65537
#define RSA_PADDING_LEN                         11

#define AES_KEY_LEN                             128

static RSA* g_rsa = NULL;
static AES_KEY g_aes_encrypt_key;
static AES_KEY g_aes_decrypt_key;
static EncryptAlgType g_encrypt_alg_type = kTypeAes;

static void exit_fun(){
    if (g_rsa){
	RSA_free(g_rsa);
	g_rsa = NULL;
    }
}

EdpPacket* PacketEncryptReq(EncryptAlgType type){
    if (type != kTypeAes){
	return NULL;
    }

    /* init rsa */
    if (!g_rsa){
	atexit(exit_fun);

	BIGNUM* bne = NULL;
	int ret = 0;
	bne=BN_new();
	BN_set_word(bne, RSA_E);
	g_rsa = RSA_new();
	ret=RSA_generate_key_ex(g_rsa, RSA_N_LEN_BYTE * 8, bne, NULL);
	if (ret != 1){
	    return NULL;
	}
    }

    EdpPacket* pkg = NewBuffer();
    unsigned remainlen = RSA_N_LEN_BYTE + RSA_E_LEN_BYTE + 1; 
    WriteByte(pkg, ENCRYPTREQ);
    WriteRemainlen(pkg, remainlen);

    /* write e */
    memset(pkg->_data + pkg->_write_pos, 0, RSA_E_LEN_BYTE);
    unsigned char tmp[RSA_E_LEN_BYTE];
    int len = BN_bn2bin(g_rsa->e, tmp);
    if (len > RSA_E_LEN_BYTE){
	DeleteBuffer(&pkg);
	return NULL;
    }
    memcpy(pkg->_data + pkg->_write_pos + RSA_E_LEN_BYTE - len,
	   tmp, len);
    pkg->_write_pos += RSA_E_LEN_BYTE;

    /* write n */
    len = BN_bn2bin(g_rsa->n, pkg->_data + pkg->_write_pos);

    if (len != RSA_N_LEN_BYTE){
	DeleteBuffer(&pkg);
	return NULL;
    }

    pkg->_write_pos += RSA_N_LEN_BYTE;

    
    /* 
     * The type of symmetric encryption algorithm.
     * Right now, only support AES whose code is 1
     */
    g_encrypt_alg_type = type;
    WriteByte(pkg, g_encrypt_alg_type); 		

    return pkg;
}

int32 UnpackEncryptResp(EdpPacket* pkg){
    uint32 remainlen;
    if (ReadRemainlen(pkg, &remainlen))
	return ERR_UNPACK_ENCRYPT_RESP;

    uint16 key_len;
    if (ReadUint16(pkg, &key_len))
	return ERR_UNPACK_ENCRYPT_RESP;
    
    if (remainlen != key_len + 2)
	return ERR_UNPACK_ENCRYPT_RESP;

    unsigned rsa_size = RSA_size(g_rsa);
    unsigned slice_size = rsa_size - RSA_PADDING_LEN;
    
    unsigned char key[BUFFER_SIZE] = {0};
    unsigned decrypt_len = 0;
    int len = 0;
    unsigned char* from = pkg->_data + pkg->_read_pos;
    unsigned char* to = key;
    
    uint32 i = 0;
    for (i=0; i<key_len; i+=rsa_size){
	len = RSA_private_decrypt(rsa_size, from+i, to, 
				  g_rsa, RSA_PKCS1_PADDING);
	decrypt_len += len;
	if (decrypt_len > BUFFER_SIZE){
	    return ERR_UNPACK_ENCRYPT_RESP;
	}

	to += len;
    }

    printf("%s\n", key);
    switch (g_encrypt_alg_type){
    case kTypeAes:
	if(AES_set_encrypt_key(key, AES_KEY_LEN, &g_aes_encrypt_key) < 0) {
	    return ERR_UNPACK_ENCRYPT_RESP;
	}

	if(AES_set_decrypt_key(key, AES_KEY_LEN, &g_aes_decrypt_key) < 0) {
	    return ERR_UNPACK_ENCRYPT_RESP;
	}
	break;

    default:
	return ERR_UNPACK_ENCRYPT_RESP;
	break;
    }

    return 0;
}

/*************** AES ***************/
static int aes_encrypt(EdpPacket* pkg, int remain_pos){
    /* 加密 */
    uint32 in_len = pkg->_write_pos - remain_pos;
    unsigned char* in = pkg->_data + remain_pos;
    /* AES 支持加密后数据存放在加密前的buffer中 */
    unsigned char* out = in;
    
    size_t div = in_len / AES_BLOCK_SIZE;
    size_t mod = in_len % AES_BLOCK_SIZE;
    size_t adder = 0;
    size_t block = 0;

    size_t padding_len = AES_BLOCK_SIZE - mod;
    unsigned char* padding_addr = in + div * AES_BLOCK_SIZE;
    if (mod){
	padding_addr += mod;
    }
    ++div;
    /* 填充
     * (1) 如果被加密数据长度刚好是AES_BLOCK_SIZE的整数倍，则在后面
     *  填充AES_BLOCK_SIZE个'0' + AES_BLOCK_SIZE，如果AES_BLOCK_SIZE等于
     *  16，则填充16个 '0'+16
     * (2) 如果不是整数倍，假设最后一段长度为n，则在末尾填充AES_BLOCK_SIZE-n
     * 个'0' + AES_BLOCK_SIZE - n。比如AES_BLOCK_SIZE = 16, n=11, 则在最后
     * 填充5个 '0'+5.
     */
    memset(padding_addr, '0' + padding_len, padding_len);

    /* AES 支持加解密前后存放在同一buffer中 */
    for (block=0; block<=div; ++block){
	adder = block * AES_BLOCK_SIZE;
	AES_encrypt(in + adder, out + adder, &g_aes_encrypt_key);
    }

    /* 加密后的remainlen会变得大，其占用的空间可能会变大
     * 利用一个临时的EdpPacket来测试加密后remainlen的长度是否发生改变
     * 若改变，则加密后的数据应该依次往后移，以为remainlen留出足够空间
     */
    size_t len_aft_enc =  div * AES_BLOCK_SIZE;

    char tmp_buf[5];
    EdpPacket tmp_pkg;
    tmp_pkg._data = tmp_buf;
    tmp_pkg._write_pos = 1;
    tmp_pkg._read_pos = 0;
    WriteRemainlen(&tmp_pkg, len_aft_enc);
    int diff = tmp_pkg._write_pos - remain_pos;
    if (diff > 0){
	int i = len_aft_enc;
	for (; i>0; i--){
	    *(in + i + diff - 1)  = *(in + i - 1);
	}
    }
    
    pkg->_write_pos = 1;
    pkg->_read_pos = 0;
    WriteRemainlen(pkg, len_aft_enc);
    pkg->_write_pos += len_aft_enc;

    return len_aft_enc;
}

static int aes_decrypt(EdpPacket* pkg, int remain_pos){
    size_t in_len = pkg->_write_pos - pkg->_read_pos;
    unsigned char* in = pkg->_data + pkg->_read_pos;
    unsigned char* out = in;

    size_t offset = 0;
    for (offset=0; (offset+AES_BLOCK_SIZE)<=in_len; offset+=AES_BLOCK_SIZE){
	AES_decrypt(in + offset, out + offset, &g_aes_decrypt_key);
    }

    size_t padding_len = *(in + offset -1) - '0';
    if (padding_len > AES_BLOCK_SIZE){
	printf(" padding_len = %d %02x %d\n", 
	       padding_len, *(in + offset - 1), AES_BLOCK_SIZE);
	return -1;
    }

    /* 解密后的remainlen会变小，其占用空间可能变小
     * 利用一个临时的EdpPacket来测试加密后remainlen的长度是否发生改变
     * 若改变，则解密后的数据应该依次往前移，以消除多余空间
     */
    uint32 len_aft_dec = offset - padding_len;
    char tmp_buf[5];
    EdpPacket tmp_pkg;
    tmp_pkg._data = tmp_buf;
    tmp_pkg._write_pos = 1;
    tmp_pkg._read_pos = 0;
    WriteRemainlen(&tmp_pkg, len_aft_dec);

    int diff = remain_pos - tmp_pkg._write_pos;
    if (diff > 0){
	int i = 0;
	for (i=0; i<len_aft_dec; i++){
	    *(in + i - diff)  = *(in + i);
	}
    }
    
    pkg->_write_pos = 1;
    pkg->_read_pos = 0;
    WriteRemainlen(pkg, len_aft_dec);
    pkg->_read_pos = 1;
    pkg->_write_pos += len_aft_dec;

    return len_aft_dec;
}

/*************** end AES ***************/

int SymmEncrypt(EdpPacket* pkg){
    int ret = 0;
    uint32 remain_len = 0;
    uint32 remain_pos = 0;

    pkg->_read_pos = 1;
    ReadRemainlen(pkg, &remain_len);
    assert(remain_len == (pkg->_write_pos - pkg->_read_pos));
    if (remain_len == 0){	/* no data for encrypting */
	pkg->_read_pos = 1;
	return 0;
    }
    remain_pos = pkg->_read_pos;

    switch (g_encrypt_alg_type){
    case kTypeAes:
	ret = aes_encrypt(pkg, remain_pos);
	break;

    default:
	ret = -1;
	break;
    }

    return ret;
}

int SymmDecrypt(EdpPacket* pkg){
    int ret = 0;
    uint32 remain_len = 0;
    uint32 remain_pos = 0;

    pkg->_read_pos = 1;
    ReadRemainlen(pkg, &remain_len);
    assert(remain_len == (pkg->_write_pos - pkg->_read_pos));
    if (remain_len == 0){	/* no data for decrypting */
	pkg->_read_pos = 1;
	return 0;
    }
    remain_pos = pkg->_read_pos;

    switch (g_encrypt_alg_type){
    case kTypeAes:
	ret = aes_decrypt(pkg, remain_pos);
	break;

    default:
	ret = -1;
	break;
    }

    return ret;
}
#endif
