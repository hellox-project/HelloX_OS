
#include "kapi.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "limits.h"


#ifndef	__SSH_DEF_H__
#define __SSH_DEF_H__

/*
 * SSH-2 message type codes.
 */
#define SSH2_MSG_DISCONNECT                       1	/* 0x1 */
#define SSH2_MSG_IGNORE                           2	/* 0x2 */
#define SSH2_MSG_UNIMPLEMENTED                    3	/* 0x3 */
#define SSH2_MSG_DEBUG                            4	/* 0x4 */
#define SSH2_MSG_SERVICE_REQUEST                  5	/* 0x5 */
#define SSH2_MSG_SERVICE_ACCEPT                   6	/* 0x6 */
#define SSH2_MSG_KEXINIT                          20	/* 0x14 */
#define SSH2_MSG_NEWKEYS                          21	/* 0x15 */
#define SSH2_MSG_KEXDH_INIT                       30	/* 0x1e */
#define SSH2_MSG_KEXDH_REPLY                      31	/* 0x1f */
#define SSH2_MSG_KEX_DH_GEX_REQUEST_OLD           30	/* 0x1e */
#define SSH2_MSG_KEX_DH_GEX_REQUEST               34	/* 0x22 */
#define SSH2_MSG_KEX_DH_GEX_GROUP                 31	/* 0x1f */
#define SSH2_MSG_KEX_DH_GEX_INIT                  32	/* 0x20 */
#define SSH2_MSG_KEX_DH_GEX_REPLY                 33	/* 0x21 */
#define SSH2_MSG_KEXRSA_PUBKEY                    30    /* 0x1e */
#define SSH2_MSG_KEXRSA_SECRET                    31    /* 0x1f */
#define SSH2_MSG_KEXRSA_DONE                      32    /* 0x20 */
#define SSH2_MSG_USERAUTH_REQUEST                 50	/* 0x32 */
#define SSH2_MSG_USERAUTH_FAILURE                 51	/* 0x33 */
#define SSH2_MSG_USERAUTH_SUCCESS                 52	/* 0x34 */
#define SSH2_MSG_USERAUTH_BANNER                  53	/* 0x35 */
#define SSH2_MSG_USERAUTH_PK_OK                   60	/* 0x3c */
#define SSH2_MSG_USERAUTH_PASSWD_CHANGEREQ        60	/* 0x3c */
#define SSH2_MSG_USERAUTH_INFO_REQUEST            60	/* 0x3c */
#define SSH2_MSG_USERAUTH_INFO_RESPONSE           61	/* 0x3d */
#define SSH2_MSG_GLOBAL_REQUEST                   80	/* 0x50 */
#define SSH2_MSG_REQUEST_SUCCESS                  81	/* 0x51 */
#define SSH2_MSG_REQUEST_FAILURE                  82	/* 0x52 */
#define SSH2_MSG_CHANNEL_OPEN                     90	/* 0x5a */
#define SSH2_MSG_CHANNEL_OPEN_CONFIRMATION        91	/* 0x5b */
#define SSH2_MSG_CHANNEL_OPEN_FAILURE             92	/* 0x5c */
#define SSH2_MSG_CHANNEL_WINDOW_ADJUST            93	/* 0x5d */
#define SSH2_MSG_CHANNEL_DATA                     94	/* 0x5e */
#define SSH2_MSG_CHANNEL_EXTENDED_DATA            95	/* 0x5f */
#define SSH2_MSG_CHANNEL_EOF                      96	/* 0x60 */
#define SSH2_MSG_CHANNEL_CLOSE                    97	/* 0x61 */
#define SSH2_MSG_CHANNEL_REQUEST                  98	/* 0x62 */
#define SSH2_MSG_CHANNEL_SUCCESS                  99	/* 0x63 */
#define SSH2_MSG_CHANNEL_FAILURE                  100	/* 0x64 */
#define SSH2_MSG_USERAUTH_GSSAPI_RESPONSE               60
#define SSH2_MSG_USERAUTH_GSSAPI_TOKEN                  61
#define SSH2_MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE      63
#define SSH2_MSG_USERAUTH_GSSAPI_ERROR                  64
#define SSH2_MSG_USERAUTH_GSSAPI_ERRTOK                 65
#define SSH2_MSG_USERAUTH_GSSAPI_MIC                    66

#define APIEXTRA  0

#define SSH_MAX_BACKLOG 32768
//#define NULL ((void *)0)

#define GET_32BIT_MSB_FIRST(cp) \
	(((unsigned long)(unsigned char)(cp)[0] << 24) | \
	((unsigned long)(unsigned char)(cp)[1] << 16) | \
	((unsigned long)(unsigned char)(cp)[2] << 8) | \
	((unsigned long)(unsigned char)(cp)[3]))

#define GET_32BIT(cp) GET_32BIT_MSB_FIRST(cp)

#define PUT_32BIT_MSB_FIRST(cp, value) ( \
	(cp)[0] = (unsigned char)((value) >> 24), \
	(cp)[1] = (unsigned char)((value) >> 16), \
	(cp)[2] = (unsigned char)((value) >> 8), \
	(cp)[3] = (unsigned char)(value) )

#define PUT_32BIT(cp, value) PUT_32BIT_MSB_FIRST(cp, value)

/*
 * Various remote-bug flags.
 */
#define BUG_CHOKES_ON_SSH1_IGNORE                 1
#define BUG_SSH2_HMAC                             2
#define BUG_NEEDS_SSH1_PLAIN_PASSWORD        	  4
#define BUG_CHOKES_ON_RSA	        			  8
#define BUG_SSH2_RSA_PADDING	        		 16
#define BUG_SSH2_DERIVEKEY                       32
#define BUG_SSH2_REKEY                           64
#define BUG_SSH2_PK_SESSIONID                   128
#define BUG_SSH2_MAXPKT							256
#define BUG_CHOKES_ON_SSH2_IGNORE               512
#define BUG_CHOKES_ON_WINADJ                   1024
#define BUG_SENDS_LATE_REQUEST_REPLY           2048
#define BUG_SSH2_OLDGEX                        4096

#define DH_MIN_SIZE								1024
#define DH_MAX_SIZE								8192

#define OUR_V2_MAXPKT 0x4000UL
#define SSH2_MKKEY_ITERS (2)

#define SSH2_TTY_OP_ISPEED 128
#define SSH2_TTY_OP_OSPEED 129


#define OUR_V2_BIGWIN 0x7fffffff
#define OUR_V2_WINSIZE 16384

#define SSH2_KEX_MAX_HASH_LEN (32) 

#define lenof(x) ( (sizeof((x))) / (sizeof(*(x))))


#define  SSH_NETBUF_LEN     2048//10240

#define  S_NEED_MORE_DATA  S_FALSE+1
#define  S_LOGOUT          S_FALSE+2
#define  S_USERAUTH_ERR    S_FALSE+3
#define  S_OTHER_ERR       S_FALSE+4

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef uint32 word32;

typedef unsigned int     BignumInt;
typedef unsigned __int64 BignumDblInt;
typedef BignumInt* Bignum;

typedef struct
{
    long		 length;	    /* length of packet: see below */
    long		 forcepad;	    /* SSH-2: force padding to at least this length */
    int			  type;		    /* only used for incoming packets */
    unsigned long sequence; /* SSH-2 incoming sequence number */
    unsigned char *data;    /* allocated storage */
    unsigned char *body;    /* offset of payload within `data' */
    long		   savedpos;	    /* dual-purpose saved packet position: see below */
    long           maxlen;	    /* amount of storage allocated for `data' */
    long           encrypted_len;	    /* for SSH-2 total-size counting */

    unsigned downstream_id;
    const char *additional_log_text;
}Packet;

typedef struct
{
	long len, pad, payload, packetlen, maclen;
	int i;
	int cipherblk;
	unsigned long incoming_sequence;
	Packet *pktin;

}rdpkt2_state;


enum {
    /*
     * SSH-2 key exchange algorithms
     */
    KEX_WARN,
    KEX_DHGROUP1,
    KEX_DHGROUP14,
    KEX_DHGEX,
    KEX_RSA,
    KEX_MAX
};


enum {
    /*
     * SSH ciphers (both SSH-1 and SSH-2)
     */
    CIPHER_WARN,		       /* pseudo 'cipher' */
    CIPHER_3DES,
    CIPHER_BLOWFISH,
    CIPHER_AES,			       /* (SSH-2 only) */
    CIPHER_DES,
    CIPHER_ARCFOUR,
    CIPHER_MAX			       /* no. ciphers (inc warn) */
};

enum {
    BUSY_NOT,	    /* Not busy, all user interaction OK */
    BUSY_WAITING,   /* Waiting for something; local event loops still running
		       so some local interaction (e.g. menus) OK, but network
		       stuff is suspended */
    BUSY_CPU	    /* Locally busy (e.g. crypto); user interaction suspended */
};

typedef enum 
{
	SSH2_PKTCTX_NOKEX,
	SSH2_PKTCTX_DHGROUP,
	SSH2_PKTCTX_DHGEX,
	SSH2_PKTCTX_RSAKEX
} Pkt_KCtx;
typedef enum 
{
	SSH2_PKTCTX_NOAUTH,
	SSH2_PKTCTX_PUBLICKEY,
	SSH2_PKTCTX_PASSWORD,
	SSH2_PKTCTX_GSSAPI,
	SSH2_PKTCTX_KBDINTER
} Pkt_ACtx;



typedef struct  
{
	void *(*init)(void); /* also allocates context */
	void (*bytes)(void *, void *, int);
	void (*final)(void *, unsigned char *); /* also frees context */
	int hlen; /* output length in bytes */
	char *text_name;
}ssh_hash;   


typedef struct 
{
	char *name, *groupname;
	enum { KEXTYPE_DH, KEXTYPE_RSA } main_type;
	/* For DH */
	const unsigned char *pdata, *gdata; /* NULL means group exchange */
	int plen, glen;
	ssh_hash *hash;
} ssh_kex ;


typedef struct
{
	int bits;
	int bytes;
#ifdef MSCRYPTOAPI
	unsigned long exponent;
	unsigned char *modulus;
#else
	Bignum modulus;
	Bignum exponent;
	Bignum private_exponent;
	Bignum p;
	Bignum q;
	Bignum iqmp;
#endif
	char *comment;
}RSAKey;


typedef struct  
{
	void *(*make_context)(void);
	void (*free_context)(void *);
	void (*setkey) (void *, unsigned char *key);
	/* whole-packet operations */
	void (*generate) (void *, unsigned char *blk, int len, unsigned long seq);
	int (*verify) (void *, unsigned char *blk, int len, unsigned long seq);
	/* partial-packet operations */
	void (*start) (void *);
	void (*bytes) (void *, unsigned char const *, int);
	void (*genresult) (void *, unsigned char *);
	int (*verresult) (void *, unsigned char const *);
	char *name;
	int len;
	char *text_name;
}ssh_mac;

typedef  struct   
{
	void *(*make_context)(void);
	void (*free_context)(void *);
	void (*setiv) (void *, unsigned char *key);	/* for SSH-2 */
	void (*setkey) (void *, unsigned char *key);/* for SSH-2 */
	void (*encrypt) (void *, unsigned char *blk, int len);
	void (*decrypt) (void *, unsigned char *blk, int len);
	char *name;
	int blksize;
	int keylen;
	unsigned int flags;
#define SSH_CIPHER_IS_CBC	1
	char *text_name;
}ssh2_cipher;

typedef  struct  
{
	int nciphers;
	ssh2_cipher *const *list;

}ssh2_ciphers;


typedef struct  
{
    char *name;
    /* For zlib@openssh.com: if non-NULL, this name will be considered once
     * userauth has completed successfully. */
    char *delayed_name;
    void *(*compress_init) (void);
    void (*compress_cleanup) (void *);
    int (*compress) (void *, unsigned char *block, int len,
		     unsigned char **outblock, int *outlen);
    void *(*decompress_init) (void);
    void (*decompress_cleanup) (void *);
    int (*decompress) (void *, unsigned char *block, int len,
		       unsigned char **outblock, int *outlen);
    int (*disable_compression) (void *);
    char *text_name;
}ssh_compress;

typedef  struct 
{
	int nkexes;
	const  ssh_kex *const *list;
}ssh_kexes ;


typedef struct 
{
	int crLine;
	int nbits, pbits, warn_kex, warn_cscipher, warn_sccipher;
	Bignum p, g, e, f, K;
	void *our_kexinit;
	int our_kexinitlen;
	int kex_init_value, kex_reply_value;
	ssh_mac** maclist;
	int nmacs;
	ssh2_cipher *cscipher_tobe;
	ssh2_cipher *sccipher_tobe;
	ssh_mac *csmac_tobe;
	ssh_mac *scmac_tobe;
	ssh_compress *cscomp_tobe;
	ssh_compress *sccomp_tobe;
	char *hostkeydata, *sigdata, *rsakeydata, *keystr, *fingerprint;
	int hostkeylen, siglen, rsakeylen;
	void *hkey;		       /* actual host key */
	void *rsakey;		       /* for RSA kex */
	unsigned char exchange_hash[SSH2_KEX_MAX_HASH_LEN];
	int n_preferred_kex;
	ssh_kexes *preferred_kex[KEX_MAX];
	int n_preferred_ciphers;
	ssh2_ciphers *preferred_ciphers[CIPHER_MAX];
	ssh_compress *preferred_comp;
	int userauth_succeeded;	    /* for delayed compression */
	int pending_compression;
	int got_session_id, activated_authconn;
	Packet *pktout;
	int dlgret;
	int guessok;
	int ignorepkt;
}ssh2_transport_state ;


typedef struct 
{
	uint32 h[4];
} MD5_Core_State;

typedef struct
{
#ifdef MSCRYPTOAPI
	unsigned long hHash;
#else
	MD5_Core_State core;
	unsigned char block[64];
	int blkused;
	unsigned int lenhi, lenlo;
#endif
}MD5Context ;


typedef struct 
{
	void *(*newkey) (char *data, int len);
	void (*freekey) (void *key);
	char *(*fmtkey) (void *key);
	unsigned char *(*public_blob) (void *key, int *len);
	unsigned char *(*private_blob) (void *key, int *len);
	void *(*createkey) (unsigned char *pub_blob, int pub_len,
		unsigned char *priv_blob, int priv_len);
	void *(*openssh_createkey) (unsigned char **blob, int *len);
	int (*openssh_fmtkey) (void *key, unsigned char *blob, int len);
	int (*pubkey_bits) (void *blob, int len);
	char *(*fingerprint) (void *key);
	int (*verifysig) (void *key, char *sig, int siglen,
		char *data, int datalen);
	unsigned char *(*sign) (void *key, char *data, int datalen,
		int *siglen);
	char *name;
	char *keytype;		       /* for host key cache */
}ssh_signkey;


typedef struct 
{
	uint32 h[5];
	unsigned char block[64];
	int blkused;
	uint32 lenhi, lenlo;
} SHA_State;


typedef struct 
{
	unsigned long hi, lo;
} uint64;


typedef struct 
{
	uint64 h[8];
	unsigned char block[128];
	int blkused;
	uint32 len[4];
} SHA512_State;


typedef struct  {
	void *(*make_context)(void);
	void (*free_context)(void *);
	void (*sesskey) (void *, unsigned char *key);	/* for SSH-1 */
	void (*encrypt) (void *, unsigned char *blk, int len);
	void (*decrypt) (void *, unsigned char *blk, int len);
	int blksize;
	char *text_name;
}ssh_cipher;

typedef struct 
{
	uint32 h[8];
	unsigned char block[64];
	int blkused;
	uint32 lenhi, lenlo;
} SHA256_State;

typedef struct  
{
	Bignum p, q, g, y, x;
}dss_key;


enum {				       /* channel types */
    CHAN_MAINSESSION,
    CHAN_X11,
    CHAN_AGENT,
    CHAN_SOCKDATA,
    CHAN_SOCKDATA_DORMANT,	       /* one the remote hasn't confirmed */
    /*
     * CHAN_SHARING indicates a channel which is tracked here on
     * behalf of a connection-sharing downstream. We do almost nothing
     * with these channels ourselves: all messages relating to them
     * get thrown straight to sshshare.c and passed on almost
     * unmodified to downstream.
     */
    CHAN_SHARING,
    /*
     * CHAN_ZOMBIE is used to indicate a channel for which we've
     * already destroyed the local data source: for instance, if a
     * forwarded port experiences a socket error on the local side, we
     * immediately destroy its local socket and turn the SSH channel
     * into CHAN_ZOMBIE.
     */
    CHAN_ZOMBIE
};


struct bufchain_granule 
{
	struct bufchain_granule *next;
	char *bufpos, *bufend, *bufmax;
};


typedef struct bufchain_tag 
{
	struct bufchain_granule *head, *tail;
	int buffersize;		       /* current amount of buffered data */
} bufchain;

/*
 * 2-3-4 tree storing channels.
 */
struct ssh_channel 
{
    void* ssh;			       /* pointer back to main context */
    unsigned remoteid, localid;
    int type;

	/* True if we opened this channel but server hasn't confirmed. */
	int halfopen;

    /* True if we opened this channel but server hasn't confirmed. */
  
#define CLOSES_SENT_EOF    1
#define CLOSES_SENT_CLOSE  2
#define CLOSES_RCVD_EOF    4
#define CLOSES_RCVD_CLOSE  8
    int closes;

    int pending_eof;

    /*
     * True if this channel is causing the underlying connection to be
     * throttled.
     */
    int throttling_conn;
    union {
	struct ssh2_data_channel 
	{
	    bufchain outbuffer;
	    unsigned remwindow, remmaxpkt;
	    /* locwindow is signed so we can cope with excess data. */
	    int locwindow, locmaxwin;
	    /*
	     * remlocwin is the amount of local window that we think
	     * the remote end had available to it after it sent the
	     * last data packet or window adjust ack.
	     */
	    int remlocwin;
	    /*
	     * These store the list of channel requests that haven't
	     * been acked.
	     */
	    struct outstanding_channel_request *chanreq_head, *chanreq_tail;
	    enum { THROTTLED, UNTHROTTLING, UNTHROTTLED } throttle_state;
	} v2;
    } v;
    union 
	{
	struct ssh_agent_channel 
	{
	    unsigned char *message;
	    unsigned char msglen[4];
	    unsigned lensofar, totallen;
            int outstanding_requests;
	} a;

	struct ssh_pfd_channel 
	{
            struct PortForwarding *pf;
	} pfd;
	struct ssh_sharing_channel 
	{
	    void *ctx;
	} sharing;
    } u;
};
typedef struct 
{
	int       sd;

	int       port;
	char      address[64];

	char      username[32];
	char      password[32];


	char *v_c, *v_s;
	void *exhash;

	char      version[64];

	int       login;
	int       faild;
		
	rdpkt2_state rdpkt2_state;
	const  ssh_cipher *cipher;
	
	ssh2_cipher *cscipher, *sccipher;
	void *cs_cipher_ctx, *sc_cipher_ctx;
	const  ssh_mac *csmac, *scmac;
	void *cs_mac_ctx, *sc_mac_ctx;
	const  ssh_compress *cscomp, *sccomp;
	void *cs_comp_ctx, *sc_comp_ctx;
	ssh_kex *kex;
	ssh_signkey *hostkey;

	void*            pcb;
	unsigned int     wp;


	enum {
		SSH_STATE_PREPACKET,
		SSH_STATE_BEFORE_SIZE,
		SSH_STATE_INTERMED,
		SSH_STATE_SESSION,
		SSH_STATE_CLOSED
	} state;

	unsigned long incoming_data_size, outgoing_data_size, deferred_data_size;
	unsigned long max_data_size;
	int kex_in_progress;
	unsigned long next_rekey, last_rekey;
	//char *deferred_rekey_reason;    /* points to STATIC string; don't free */
	
	Packet **queue;
	int queuelen, queuesize;
	int queueing;

	unsigned char *deferred_send_data;
	int deferred_len, deferred_size;

	int bare_connection;
	int attempting_connshare;
	
	unsigned long v2_outgoing_sequence;

	void *connshare;

	int remote_bugs;

	Pkt_KCtx pkt_kctx;
	Pkt_ACtx pkt_actx;

	ssh2_transport_state* trans_state;

	struct ssh_channel *mainchan;      /* primary session channel */

//	int kex_in_progress;
	int ospeed, ispeed;		       /* temporaries */
	int term_width, term_height;
	char *hostkey_str;
	void* kex_ctx;
	void* frontend;
	int protocol_initial_phase_done;

	unsigned char v2_session_id[SSH2_KEX_MAX_HASH_LEN];
	int v2_session_id_len;
	int     setup;
	int     error_id;

	int     term_rows;
	int     term_cols;
	void*    hThread;

}ssh_session;



void*  ssh_new(int size);
void*  ssh_rnew(void* p,int size);
void   ssh_free(void* p);
int    toint(unsigned u);
void   smemclr(void *b, int n); 

Bignum bignum_from_bytes(const unsigned char *data, int nbytes);
int    bignum_bitcount(Bignum bn);
int    bignum_byte(Bignum bn, int i);
int    random_byte(void);

Bignum bn_power_2(int n);
void bignum_set_bit(Bignum bn, int i, int value);
Bignum modpow(Bignum base, Bignum exp, Bignum mod);
Bignum bigsub(Bignum a, Bignum b);


Bignum modmul(Bignum p, Bignum q, Bignum mod);
Bignum bigmod(Bignum a, Bignum b);
int    bignum_cmp(Bignum a, Bignum b);
int    ssh1_bignum_length(Bignum bn);
int    ssh1_write_bignum(void *data, Bignum bn);
int    ssh1_read_bignum(const unsigned char *data, int len, Bignum * result);

Bignum bigmul(Bignum a, Bignum b);
Bignum bigadd(Bignum a, Bignum b);
Bignum bigmuladd(Bignum a, Bignum b, Bignum addend);
Bignum modinv(Bignum number, Bignum modulus);

void   decbn(Bignum n);
Bignum copybn(Bignum b);
void   freebn(Bignum b);
void   bn_restore_invariant(Bignum b);
int    ssh2_bignum_length(Bignum bn);

const char* dh_validate_f(void *handle, Bignum f);
Bignum      dh_find_K(void *, Bignum f);
void        dh_cleanup(void *);
void*       dh_setup_gex(Bignum pval, Bignum gval);
Bignum      dh_create_e(void *, int nbits);

#define snewn(n, type) ((type *)ssh_new((n)*sizeof(type)))
#define sresize(ptr, n, type) (type *)ssh_rnew(ptr,n*sizeof(type))
#define sfree(p) ssh_free(p)


void   analyze_str(const char* src,int len,char* dst);
int    ssh_msg_analyze(ssh_session*  ssh,uint8* data,int len);
int    ssh_send_text(ssh_session*  ssh,const char* text,int len);


void MD5Init( MD5Context *context);
void MD5Update(MD5Context *context, unsigned char const *buf,unsigned len);
void MD5Final(unsigned char digest[16], MD5Context *context);
void MD5Simple(void const *p, unsigned len, unsigned char output[16]);

void *hmacmd5_make_context(void);
void hmacmd5_free_context(void *handle);
void hmacmd5_key(void *handle, void const *key, int len);
void hmacmd5_do_hmac(void *handle, unsigned char const *blk, int len,unsigned char *hmac);
int smemeq(const void *av, const void *bv, int len);

void SHA_Init(SHA_State * s);
void SHA_Bytes(SHA_State * s, const void *p, int len);
void SHA_Final(SHA_State * s, unsigned char *output);
void SHA_Simple(const void *p, int len, unsigned char *output);

void SHA256_Init(SHA256_State * s);
void SHA256_Bytes(SHA256_State * s, const void *p, int len);
void SHA256_Final(SHA256_State * s, unsigned char *output);
void SHA256_Simple(const void *p, int len, unsigned char *output);

void SHA512_Init(SHA512_State * s);
void SHA512_Bytes(SHA512_State * s, const void *p, int len);
void SHA512_Final(SHA512_State * s, unsigned char *output);
void SHA512_Simple(const void *p, int len, unsigned char *output);

#endif //__SSH_DEF_H__
