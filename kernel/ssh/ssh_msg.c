
#include "ssh_def.h"
#include "ssh_pkt.h"
#include "ssh/ssh.h"

#define GETTICKCOUNT GetTickCount

extern  ssh_signkey ssh_rsa;
extern  ssh_signkey ssh_dss;
extern  ssh_mac ssh_hmac_sha256;
extern  ssh_mac ssh_hmac_sha1;
extern  ssh_mac ssh_hmac_sha1_96;
extern  ssh_mac ssh_hmac_md5;
extern  ssh_mac ssh_hmac_sha1_96_buggy;
extern  ssh_mac ssh_hmac_sha1_buggy;
extern  ssh_kexes ssh_rsa_kex;

extern   ssh2_ciphers ssh2_3des;

extern  ssh_kexes ssh_diffiehellman_gex;

static  ssh_signkey *hostkey_algs[] = { &ssh_rsa, &ssh_dss };

static  ssh_mac *macs[] = 
{
	&ssh_hmac_sha256, /*&ssh_hmac_sha1, &ssh_hmac_sha1_96,*/ &ssh_hmac_md5
};

/*const static struct ssh_mac *buggymacs[] = 
{
	&ssh_hmac_sha1_buggy, &ssh_hmac_sha1_96_buggy, &ssh_hmac_md5
};*/

static void *ssh_comp_none_init(void)
{
	return NULL;
}

static void ssh_comp_none_cleanup(void *handle)
{
}

int  hx_strcspn(const char* src, const char * sub)
{
	int  pos   = 0;
	int  len1 = strlen(src);	
	int  i;


	for(i=0;i<len1;i++)
	{
		if(src[i] == sub[0])
		{
			return i;
		}
	}
	
	return 0;
}
static int ssh_comp_none_block(void *handle, unsigned char *block, int len,	unsigned char **outblock, int *outlen)
{
	return 0;
}
static int ssh_comp_none_disable(void *handle)
{
	return 0;
}

 static  ssh_compress ssh_comp_none = 
{
	"none", NULL,
	ssh_comp_none_init, ssh_comp_none_cleanup, ssh_comp_none_block,
	ssh_comp_none_init, ssh_comp_none_cleanup, ssh_comp_none_block,
	ssh_comp_none_disable, NULL
};
//struct
static  ssh_compress *compressions[] = 
{
	NULL, &ssh_comp_none
};



/*
 * Utility routine for decoding comma-separated strings in KEXINIT.
 */
static int in_commasep_string(char *needle, char *haystack, int haylen)
{
    int needlen;

    if (!needle || !haystack)	       /* protect against null pointers */
		return 0;
    
	needlen = strlen(needle);
    while (1) 
	{
		if (haylen >= needlen &&       /* haystack is long enough */
			!memcmp(needle, haystack, needlen) &&	/* initial match */
			(haylen == needlen || haystack[needlen] == ',')
			/* either , or EOS follows */
			)
			return 1;
		/*
		 * If not, search for the next comma and resume after that.
		 * If no comma found, terminate.
		 */
		while (haylen > 0 && *haystack != ',')
		{
			haylen--, haystack++;
		}

		if (haylen == 0)	return 0;
	
		haylen--, haystack++;	       /* skip over comma itself */
    }

	return 0;
}

/*
 * Utility routines for putting an SSH-protocol `string' and
 * `uint32' into a hash state.
 */
static void hash_string(ssh_hash *h, void *s, void *str, int len)
{
    unsigned char lenblk[4];

    PUT_32BIT(lenblk, len);
    h->bytes(s, lenblk, 4);
    h->bytes(s, str, len);
}
/*
 * Similar routine for checking whether we have the first string in a list.
 */
static int first_in_commasep_string(char *needle, char *haystack, int haylen)
{
    int needlen;

    if (!needle || !haystack)	       /* protect against null pointers */
		return 0;

    needlen = strlen(needle);
    
    if (haylen >= needlen &&   /* haystack is long enough */
		!memcmp(needle, haystack, needlen) &&	/* initial match */
		(haylen == needlen || haystack[needlen] == ',')	/* either , or EOS follows */
	)
	{
		return 1;
	}

    return 0;
}

int ssh_step1(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;
	char *str, *preferred;
	int i, j, len;

	s = ssh->trans_state;

	ssh->setup       = 1;

	ssh->kex         = NULL;
	ssh->hostkey     = NULL;
	s->cscipher_tobe = NULL;
	s->sccipher_tobe = NULL;
	s->csmac_tobe    = NULL;
	s->scmac_tobe    = NULL;
	s->cscomp_tobe   = NULL;
	s->sccomp_tobe   = NULL;
	s->warn_kex      = s->warn_cscipher = s->warn_sccipher = FALSE;

	//PrintLine("setp1");

	pktin->savedpos += 16;						/* skip garbage cookie */
	ssh_pkt_getstring(pktin, &str, &len);    /* key exchange algorithms */
	if (!str) 
	{
		return S_FALSE;
	}

	preferred = NULL;
	for (i = 0; i < s->n_preferred_kex; i++) 
	{
		ssh_kexes *k = s->preferred_kex[i];

		if (!k) 
		{
			s->warn_kex = TRUE;
		} 
		else 
		{
			for (j = 0; j < k->nkexes; j++) 
			{
				if (!preferred) preferred = k->list[j]->name;
				if (in_commasep_string(k->list[j]->name, str, len)) 
				{
					ssh->kex = (ssh_kex*)k->list[j];
					break;
				}
			}
		}
		if (ssh->kex)
			break;
	}
	if (!ssh->kex) 
	{
		return S_FALSE;
	}

	s->guessok = first_in_commasep_string(preferred, str, len);
	ssh_pkt_getstring(pktin, &str, &len);    // host key algorithms 
	if (!str) 
	{
		return S_FALSE;
	}
	for (i = 0; i < lenof(hostkey_algs); i++) 
	{
		if (in_commasep_string(hostkey_algs[i]->name, str, len)) 
		{
			ssh->hostkey = hostkey_algs[i];
			break;
		}
	}

	if (!ssh->hostkey) 
	{
		return S_FALSE;
	}

	//s->guessok = 1;//
	s->guessok = s->guessok && first_in_commasep_string(hostkey_algs[0]->name, str, len);

	ssh_pkt_getstring(pktin, &str, &len);    // client->server cipher 
	if (!str) 
	{
		return S_FALSE;
	}
	for (i = 0; i < s->n_preferred_ciphers; i++) 
	{
		ssh2_ciphers *c = s->preferred_ciphers[i];
		if (!c) 
		{
			s->warn_cscipher = TRUE;
		} else 
		{
			for (j = 0; j < c->nciphers; j++) 
			{
				if (in_commasep_string(c->list[j]->name, str, len)) 
				{
					s->cscipher_tobe = c->list[j];
					break;
				}
			}
		}
		if (s->cscipher_tobe)
			break;
	}
	if (!s->cscipher_tobe) 
	{
		return S_FALSE;

	}

	ssh_pkt_getstring(pktin, &str, &len);    // server->client cipher 
	if (!str) 
	{
		return S_FALSE;
	}
	for (i = 0; i < s->n_preferred_ciphers; i++) 
	{
		ssh2_ciphers *c = s->preferred_ciphers[i];
		if (!c) 
		{
			s->warn_sccipher = TRUE;
		} 
		else 
		{
			for (j = 0; j < c->nciphers; j++) 
			{
				if (in_commasep_string(c->list[j]->name, str, len)) 
				{
					s->sccipher_tobe = c->list[j];
					break;
				}
			}
		}
		if (s->sccipher_tobe)
			break;
	}
	if (!s->sccipher_tobe) 
	{
		return S_FALSE;
	}

	ssh_pkt_getstring(pktin, &str, &len);    // client->server mac 
	if (!str) 
	{
		return S_FALSE;
	}

	for (i = 0; i < s->nmacs; i++) 
	{
		if (in_commasep_string(s->maclist[i]->name, str, len)) 
		{
			s->csmac_tobe = s->maclist[i];
			break;
		}
	}
	ssh_pkt_getstring(pktin, &str, &len);    // server->client mac 

	if (!str) 
	{
		return S_FALSE;
	}
	for (i = 0; i < s->nmacs; i++) 
	{
		if (in_commasep_string(s->maclist[i]->name, str, len)) 
		{
			s->scmac_tobe = s->maclist[i];
			break;
		}
	}
	ssh_pkt_getstring(pktin, &str, &len);  // client->server compression 
	if (!str) 
	{
		return S_FALSE;
	}

	for (i = 0; i < lenof(compressions) + 1; i++) 
	{
		ssh_compress *c = i == 0 ? s->preferred_comp : compressions[i - 1];

		if (in_commasep_string(c->name, str, len)) 
		{
			s->cscomp_tobe = c;
			break;
		} else if (in_commasep_string(c->delayed_name, str, len)) 
		{
			if (s->userauth_succeeded) 
			{
				s->cscomp_tobe = c;
				break;
			} 
			else 
			{
				s->pending_compression = TRUE;  // try this later 
			}
		}
	}
	ssh_pkt_getstring(pktin, &str, &len);  // server->client compression 
	if (!str) 
	{
		return S_FALSE;
	}
	for (i = 0; i < lenof(compressions) + 1; i++) 
	{
		ssh_compress *c = i == 0 ? s->preferred_comp : compressions[i - 1];
		if (in_commasep_string(c->name, str, len)) 
		{
			s->sccomp_tobe = c;
			break;
		} else if (in_commasep_string(c->delayed_name, str, len)) 
		{
			if (s->userauth_succeeded) 
			{
				s->sccomp_tobe = c;
				break;
			} else {
				s->pending_compression = TRUE;  // try this later 
			}
		}
	}

	ssh->exhash = ssh->kex->hash->init();
	hash_string(ssh->kex->hash, ssh->exhash, ssh->v_c, strlen(ssh->v_c));
	hash_string(ssh->kex->hash, ssh->exhash, ssh->v_s, strlen(ssh->v_s));
	hash_string(ssh->kex->hash, ssh->exhash, s->our_kexinit, s->our_kexinitlen);

	sfree(s->our_kexinit);
	// Include the type byte in the hash of server's KEXINIT 
	hash_string(ssh->kex->hash, ssh->exhash,pktin->body - 1, pktin->length + 1);

	if (ssh->kex->main_type == KEXTYPE_DH) 
	{
		int csbits, scbits;

        csbits = s->cscipher_tobe->keylen;
        scbits = s->sccipher_tobe->keylen;
        s->nbits = (csbits > scbits ? csbits : scbits);
        
        if (s->nbits > ssh->kex->hash->hlen * 8)
            s->nbits = ssh->kex->hash->hlen * 8;
		       
        if (!ssh->kex->pdata) 
		{            
            ssh->pkt_kctx = SSH2_PKTCTX_DHGEX;

            s->pbits = 512 << ((s->nbits - 1) / 64);
            if (s->pbits < DH_MIN_SIZE)  s->pbits = DH_MIN_SIZE;
            if (s->pbits > DH_MAX_SIZE)  s->pbits = DH_MAX_SIZE;

            if ((ssh->remote_bugs & BUG_SSH2_OLDGEX)) 
			{
                s->pktout = ssh2_pkt_init(SSH2_MSG_KEX_DH_GEX_REQUEST_OLD);
                ssh2_pkt_adduint32(s->pktout, s->pbits);
            }
			else 
			{
                s->pktout = ssh2_pkt_init(SSH2_MSG_KEX_DH_GEX_REQUEST);
                ssh2_pkt_adduint32(s->pktout, DH_MIN_SIZE);
                ssh2_pkt_adduint32(s->pktout, s->pbits);
                ssh2_pkt_adduint32(s->pktout, DH_MAX_SIZE);
            }
            ssh2_pkt_send_noqueue(ssh, s->pktout);
		}
	}	

	return S_OK;
}

static void hash_mpint(ssh_hash *h, void *s, Bignum b)
{
	unsigned char *p;
	int len;

	p = ssh2_mpint_fmt(b, &len);
	hash_string(h, s, p, len);

	sfree(p);
}

static void hash_uint32(const  ssh_hash *h, void *s, unsigned i)
{
	unsigned char intblk[4];
	PUT_32BIT(intblk, i);
	h->bytes(s, intblk, 4);
}


static void ssh2_mkkey(ssh_session* ssh, Bignum K, unsigned char *H, char chr,	unsigned char *keyspace)
{
	ssh_hash *h = ssh->kex->hash;
	void *s;

	/* First hlen bytes. */
	s = h->init();
	
	if (!(ssh->remote_bugs & BUG_SSH2_DERIVEKEY))
	{
		hash_mpint(h, s, K);
	}

	h->bytes(s, H, h->hlen);
	h->bytes(s, &chr, 1);
	h->bytes(s, ssh->v2_session_id, ssh->v2_session_id_len);
	h->final(s, keyspace);

	/* Next hlen bytes. */
	s = h->init();
	if (!(ssh->remote_bugs & BUG_SSH2_DERIVEKEY))
	{
		hash_mpint(h, s, K);
	}

	h->bytes(s, H, h->hlen);
	h->bytes(s, keyspace, h->hlen);
	h->final(s, keyspace + h->hlen);

}


/*
 * Construct the common parts of a CHANNEL_OPEN.
 */
static  Packet *ssh2_chanopen_init(  char *type)
{
    Packet *pktout;

    pktout = ssh2_pkt_init(SSH2_MSG_CHANNEL_OPEN);

    ssh2_pkt_addstring(pktout, type);
    ssh2_pkt_adduint32(pktout, 256);
    ssh2_pkt_adduint32(pktout, 16384);/* our window size */
    ssh2_pkt_adduint32(pktout, OUR_V2_MAXPKT);      /* our max pkt size */

    return pktout;
}


/*
 * Set up most of a new ssh_channel for SSH-2.
 */
void bufchain_init(bufchain *ch)
{
	ch->head = ch->tail = NULL;
	ch->buffersize = 0;
}

static void ssh2_channel_init(struct ssh_channel *c)
{
    ssh_session* ssh = c->ssh;

    c->localid = 0x100;// alloc_channel_id(ssh);
    c->closes = 0;
    c->pending_eof = FALSE;
    c->throttling_conn = FALSE;
    c->v.v2.locwindow = c->v.v2.locmaxwin = c->v.v2.remlocwin = OUR_V2_WINSIZE;
	
	//ssh_is_simple(ssh) ? OUR_V2_BIGWIN : OUR_V2_WINSIZE;
    c->v.v2.chanreq_head = NULL;
    c->v.v2.throttle_state = UNTHROTTLED;
    
	bufchain_init(&c->v.v2.outbuffer);
}

static void ssh2_setup_pty( struct ssh_channel *c,  Packet *pktin,void *ctx)
{
	 ssh_session* ssh = c->ssh;
	 Packet *pktout = NULL;

	/* Unpick the terminal-speed string. */
	/* XXX perhaps we should allow no speeds to be sent. */
	ssh->ospeed = 38400; ssh->ispeed = 38400; /* last-resort defaults */
	

	pktout = ssh2_pkt_init(SSH2_MSG_CHANNEL_REQUEST);
	ssh2_pkt_adduint32(pktout, c->remoteid);
	ssh2_pkt_addstring(pktout, "pty-req");
	ssh2_pkt_addbool(pktout, 1);

	ssh2_pkt_addstring(pktout, "xterm");// xterm conf_get_str(ssh->conf, CONF_termtype)
	ssh2_pkt_adduint32(pktout, ssh->term_cols);//ssh->term_width
	ssh2_pkt_adduint32(pktout, ssh->term_rows);//ssh->term_height
	ssh2_pkt_adduint32(pktout, 0);	       /* pixel width */
	ssh2_pkt_adduint32(pktout, 0);	       /* pixel height */
	ssh2_pkt_addstring_start(pktout);

	//parse_ttymodes(ssh, ssh2_send_ttymode, (void *)pktout);
	ssh2_pkt_addbyte(pktout, SSH2_TTY_OP_ISPEED);
	ssh2_pkt_adduint32(pktout, ssh->ispeed);
	ssh2_pkt_addbyte(pktout, SSH2_TTY_OP_OSPEED);
	ssh2_pkt_adduint32(pktout, ssh->ospeed);
	ssh2_pkt_addstring_data(pktout, "\0", 1); /* TTY_OP_END */
	ssh2_pkt_send(ssh, pktout);

	ssh->state = SSH_STATE_INTERMED;

}

int ssh_step8(ssh_session*  ssh,Packet* pktin)
{	
	ssh->setup = 8;
	ssh->login = TRUE;

	return S_OK;
}

int ssh_step7(ssh_session*  ssh,Packet* pktin)
{
	Packet *pktout = NULL;

	//s = ssh->trans_state;
	ssh->setup = 7;

	ssh->mainchan = snewn(1,struct ssh_channel);
	ssh->mainchan->ssh = ssh;

	ssh2_channel_init(ssh->mainchan);


	if (ssh_pkt_getuint32(pktin) != ssh->mainchan->localid) 
	{
		return S_FALSE;
	}

	ssh->mainchan->remoteid       = ssh_pkt_getuint32(pktin);
	ssh->mainchan->halfopen       = FALSE;
	ssh->mainchan->type           = CHAN_MAINSESSION;
	ssh->mainchan->v.v2.remwindow = ssh_pkt_getuint32(pktin);
	ssh->mainchan->v.v2.remmaxpkt = ssh_pkt_getuint32(pktin);
	
	ssh2_setup_pty(ssh->mainchan,NULL,NULL);

	pktout = ssh2_pkt_init(SSH2_MSG_CHANNEL_REQUEST);
	ssh2_pkt_adduint32(pktout, ssh->mainchan->remoteid);
	ssh2_pkt_addstring(pktout, "shell");
	ssh2_pkt_addbool(pktout, 1);
	ssh2_pkt_send(ssh, pktout);
	

	return S_OK;
}

int ssh_step6(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;

	ssh->setup = 6;

	if(pktin->type == SSH2_MSG_USERAUTH_FAILURE)
	{		
		return S_FALSE;
	}
	s = ssh->trans_state;

	s->pktout = ssh2_chanopen_init("session");	
	ssh2_pkt_send(ssh, s->pktout);
		

	return S_OK;
}
int ssh_step5(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;
	ssh_callbk    pcall      = (ssh_callbk)ssh->pcb;

	s = ssh->trans_state;
	ssh->setup = 5;
		
	//pcall(SSH_REQ_ACCOUNT,0,0);

	s->pktout = ssh2_pkt_init(SSH2_MSG_USERAUTH_REQUEST);
	ssh2_pkt_addstring(s->pktout, ssh->username);
	ssh2_pkt_addstring(s->pktout, "ssh-connection");
	/* service requested */
	ssh2_pkt_addstring(s->pktout, "password");
	ssh2_pkt_addbool(s->pktout, FALSE);
	ssh2_pkt_addstring(s->pktout, ssh->password);
	ssh2_pkt_send_with_padding(ssh, s->pktout, 256);
		
	return S_OK;
}

int ssh_step4(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;

	s = ssh->trans_state;
	ssh->setup = 4;

	ssh->incoming_data_size = 0;       /* start counting from here */

    /*
     * We've seen server NEWKEYS, so create and initialise
     * server-to-client session keys.
     */
    if (ssh->sc_cipher_ctx)
	{
		ssh->sccipher->free_context(ssh->sc_cipher_ctx);
	}

    ssh->sccipher = s->sccipher_tobe;
    ssh->sc_cipher_ctx = ssh->sccipher->make_context();

    if (ssh->sc_mac_ctx)
	{
		ssh->scmac->free_context(ssh->sc_mac_ctx);
	}

    ssh->scmac = s->scmac_tobe;
    ssh->sc_mac_ctx = ssh->scmac->make_context();

    if (ssh->sc_comp_ctx)
	{
		ssh->sccomp->decompress_cleanup(ssh->sc_comp_ctx);
	}
    ssh->sccomp = s->sccomp_tobe;
    ssh->sc_comp_ctx = ssh->sccomp->decompress_init();

    /*
     * Set IVs on server-to-client keys. Here we use the exchange
     * hash from the _first_ key exchange.
     */
    {
		unsigned char keyspace[SSH2_KEX_MAX_HASH_LEN * SSH2_MKKEY_ITERS];

		//assert(sizeof(keyspace) >= ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
		ssh2_mkkey(ssh,s->K,s->exchange_hash,'D',keyspace);
		//assert((ssh->sccipher->keylen+7) / 8 <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
		ssh->sccipher->setkey(ssh->sc_cipher_ctx, keyspace);
		ssh2_mkkey(ssh,s->K,s->exchange_hash,'B',keyspace);
		//assert(ssh->sccipher->blksize <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
		ssh->sccipher->setiv(ssh->sc_cipher_ctx, keyspace);
		ssh2_mkkey(ssh,s->K,s->exchange_hash,'F',keyspace);
		//assert(ssh->scmac->len <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
		ssh->scmac->setkey(ssh->sc_mac_ctx, keyspace);
		smemclr(keyspace, sizeof(keyspace));
    }

    /*
     * Free shared secret.
     */
    freebn(s->K);

      /*
     * Otherwise, schedule a timer for our next rekey.
     */
    ssh->kex_in_progress = FALSE;
    //ssh->last_rekey = GETTICKCOUNT();

	//s->done_service_req = FALSE;
	//s->we_are_in = s->userauth_success = FALSE;

     ssh->protocol_initial_phase_done = TRUE;
	s->pktout = ssh2_pkt_init(SSH2_MSG_SERVICE_REQUEST);
	ssh2_pkt_addstring(s->pktout, "ssh-userauth");
	ssh2_pkt_send(ssh, s->pktout);
		

	return S_OK;
}

int ssh_step3(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;

	s = ssh->trans_state;
	ssh->setup = 3;
	
	ssh_pkt_getstring(pktin, &s->hostkeydata, &s->hostkeylen);
	if (!s->hostkeydata) 
	{
		return S_FALSE;
	}
	s->hkey = ssh->hostkey->newkey(s->hostkeydata, s->hostkeylen);
	s->f = ssh2_pkt_getmp(pktin);
	if (!s->f) 
	{
		return S_FALSE;
	}
	ssh_pkt_getstring(pktin, &s->sigdata, &s->siglen);
	if (!s->sigdata) 
	{
		return S_FALSE;
	}
		
	if (dh_validate_f(ssh->kex_ctx, s->f)) 
	{
		return S_FALSE;	
	}
	
	 s->K = dh_find_K(ssh->kex_ctx, s->f);

     /* We assume everything from now on will be quick, and it might* involve user interaction. */
     //set_busy_status(ssh->frontend, BUSY_NOT);

     hash_string(ssh->kex->hash, ssh->exhash, s->hostkeydata, s->hostkeylen);
     if (!ssh->kex->pdata) 
	 {
         if (!(ssh->remote_bugs & BUG_SSH2_OLDGEX))
               hash_uint32(ssh->kex->hash, ssh->exhash, DH_MIN_SIZE);
          hash_uint32(ssh->kex->hash, ssh->exhash, s->pbits);
          if (!(ssh->remote_bugs & BUG_SSH2_OLDGEX))
                hash_uint32(ssh->kex->hash, ssh->exhash, DH_MAX_SIZE);
           hash_mpint(ssh->kex->hash, ssh->exhash, s->p);
           hash_mpint(ssh->kex->hash, ssh->exhash, s->g);
     }
     hash_mpint(ssh->kex->hash, ssh->exhash, s->e);
     hash_mpint(ssh->kex->hash, ssh->exhash, s->f);

     dh_cleanup(ssh->kex_ctx);
     freebn(s->f);
     if (!ssh->kex->pdata) 
	 {
        freebn(s->g);
        freebn(s->p);
     }
    
	 hash_mpint(ssh->kex->hash, ssh->exhash, s->K);	 
	 ssh->kex->hash->final(ssh->exhash, s->exchange_hash);

	 ssh->kex_ctx = NULL;

	 if (!s->hkey || !ssh->hostkey->verifysig(s->hkey, s->sigdata, s->siglen, (char *)s->exchange_hash,		 ssh->kex->hash->hlen)) 
	 {
		 return S_FALSE;		 
			 
	 }

	 s->keystr = ssh->hostkey->fmtkey(s->hkey);
    if (!s->got_session_id) 
	{
        /*
         * Authenticate remote host: verify host key. (We've already
         * checked the signature of the exchange hash.)
         */
        s->fingerprint = ssh->hostkey->fingerprint(s->hkey);
		s->dlgret = 1; 
        sfree(s->fingerprint);
        /*
         * Save this host key, to check against the one presented in
         * subsequent rekeys.
         */
        ssh->hostkey_str = s->keystr;
    }

	  ssh->hostkey->freekey(s->hkey);

    /*
     * The exchange hash from the very first key exchange is also
     * the session id, used in session key construction and
     * authentication.
     */
    if (!s->got_session_id) 
	{	
		memcpy(ssh->v2_session_id, s->exchange_hash,	       sizeof(s->exchange_hash));
	    ssh->v2_session_id_len = ssh->kex->hash->hlen;	
		s->got_session_id = TRUE;
    }

	   /*
     * Send SSH2_MSG_NEWKEYS.
     */
    s->pktout = ssh2_pkt_init(SSH2_MSG_NEWKEYS);
    ssh2_pkt_send_noqueue(ssh, s->pktout);
    ssh->outgoing_data_size = 0;       /* start counting from here */

    /*
     * We've sent client NEWKEYS, so create and initialise
     * client-to-server session keys.
     */
    if (ssh->cs_cipher_ctx)
		ssh->cscipher->free_context(ssh->cs_cipher_ctx);
    
	ssh->cscipher = s->cscipher_tobe;
    ssh->cs_cipher_ctx = ssh->cscipher->make_context();

    if (ssh->cs_mac_ctx)
		ssh->csmac->free_context(ssh->cs_mac_ctx);
    
	ssh->csmac = s->csmac_tobe;
    ssh->cs_mac_ctx = ssh->csmac->make_context();

    if (ssh->cs_comp_ctx)
		ssh->cscomp->compress_cleanup(ssh->cs_comp_ctx);
    ssh->cscomp = s->cscomp_tobe;
    ssh->cs_comp_ctx = ssh->cscomp->compress_init();

    /*
     * Set IVs on client-to-server keys. Here we use the exchange
     * hash from the _first_ key exchange.
     */
    {
	unsigned char keyspace[SSH2_KEX_MAX_HASH_LEN * SSH2_MKKEY_ITERS];
	//assert(sizeof(keyspace) >= ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
	ssh2_mkkey(ssh,s->K,s->exchange_hash,'C',keyspace);
	//assert((ssh->cscipher->keylen+7) / 8 <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
	ssh->cscipher->setkey(ssh->cs_cipher_ctx, keyspace);
	ssh2_mkkey(ssh,s->K,s->exchange_hash,'A',keyspace);
	//assert(ssh->cscipher->blksize <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
	ssh->cscipher->setiv(ssh->cs_cipher_ctx, keyspace);
	ssh2_mkkey(ssh,s->K,s->exchange_hash,'E',keyspace);
	//assert(ssh->csmac->len <=	       ssh->kex->hash->hlen * SSH2_MKKEY_ITERS);
	ssh->csmac->setkey(ssh->cs_mac_ctx, keyspace);
	smemclr(keyspace, sizeof(keyspace));
    }

    /*
     * Now our end of the key exchange is complete, we can send all
     * our queued higher-layer packets.
     */
    ssh->queueing = FALSE;
    ssh2_pkt_queuesend(ssh);
	
	 
	return S_OK;

}
int ssh_step2(ssh_session*  ssh,Packet* pktin)
{
	ssh2_transport_state* s;

	s = ssh->trans_state;
	ssh->setup = 2;

	if (pktin->type != SSH2_MSG_KEX_DH_GEX_GROUP) 
	{
		return S_FALSE;
    }

    s->p = ssh2_pkt_getmp(pktin);
    s->g = ssh2_pkt_getmp(pktin);

    if (!s->p || !s->g) 
	{
		return S_FALSE;
    }
    ssh->kex_ctx       = dh_setup_gex(s->p, s->g);
    s->kex_init_value  = SSH2_MSG_KEX_DH_GEX_INIT;
    s->kex_reply_value = SSH2_MSG_KEX_DH_GEX_REPLY;
      
    //set_busy_status(ssh->frontend, BUSY_CPU); /* this can take a while */
    s->e = dh_create_e(ssh->kex_ctx, s->nbits * 2);
    s->pktout = ssh2_pkt_init(s->kex_init_value);
     
	ssh2_pkt_addmp(s->pktout, s->e);
    ssh2_pkt_send_noqueue(ssh, s->pktout);

    //set_busy_status(ssh->frontend, BUSY_WAITING); /* wait for server */	

	return  S_OK;
}
int ssh_send_version_info(ssh_session*  ssh)
{
	char ver[] = "SSH-2.0-HX_Release_1.78\r\n";
	int  len   = 0;
		

	len = hx_strcspn(ver, "\r\n");
	ssh->v_c = snewn(len + 1, char);
	memcpy(ssh->v_c, ver, len);
	ssh->v_c[len] = 0;

	len = send(ssh->sd,ver, strlen(ver),0);

	return len;
}

int ssh_init_kexinfo(ssh_session*  ssh,Packet* pkt)
{
	ssh2_transport_state*  s;
	int commalist_started;
	int i,k,j;
	unsigned char v =FALSE;

	
	if(!ssh->trans_state)
	{
		ssh->trans_state = (ssh2_transport_state*)ssh_new(sizeof(ssh2_transport_state));		
	}

	s = ssh->trans_state;
    /*
     * Be prepared to work around the buggy MAC problem.
     */
	s->maclist = macs;
	s->nmacs   = lenof(macs);

	//begin_key_exchange

	// Set up the preferred key exchange
	s->preferred_kex[s->n_preferred_kex++] =	  &ssh_diffiehellman_gex;
	s->preferred_kex[s->n_preferred_kex++] =  &ssh_rsa_kex;
	
	//Set up the preferred ciphers		
	s->preferred_ciphers[s->n_preferred_ciphers++] = &ssh2_3des;
	
	s->preferred_comp = &ssh_comp_none;

	ssh->queueing = TRUE;

	/*
	 * Flag that KEX is in progress.
	 */
	ssh->kex_in_progress = TRUE;

	/*
	 * Construct and send our key exchange packet.
	 */
	s->pktout = ssh2_pkt_init(SSH2_MSG_KEXINIT);
	for (i = 0; i < 16; i++)
	{
		ssh2_pkt_addbyte(s->pktout, (unsigned char) random_byte());
	}

	/* List key exchange algorithms. */
	ssh2_pkt_addstring_start(s->pktout);
	
	commalist_started = 0;
	for (i = 0; i < s->n_preferred_kex; i++) 
	{		
		ssh_kexes *k = s->preferred_kex[i];
		
		if (!k) continue;	       /// warning flag 

		for (j = 0; j < k->nkexes; j++) 
		{
			if (commalist_started)
			{
				ssh2_pkt_addstring_str(s->pktout, ",");
			}

			ssh2_pkt_addstring_str(s->pktout, k->list[j]->name);
			commalist_started = 1;
		}
	}

	if (!s->got_session_id) 
	{           
        ssh2_pkt_addstring_start(s->pktout);
        for (i = 0; i < lenof(hostkey_algs); i++) 
		{
			ssh2_pkt_addstring_str(s->pktout, hostkey_algs[i]->name);
            if (i < lenof(hostkey_algs) - 1) 
			{
			ssh2_pkt_addstring_str(s->pktout, ",");
			}
         }
	} else 		
	{   
		ssh2_pkt_addstring(s->pktout, ssh->hostkey->name);
    }


	/* List encryption algorithms (client->server then server->client). */
	for (k = 0; k < 2; k++) 
	{
	    ssh2_pkt_addstring_start(s->pktout);
	    commalist_started = 0;
	    for (i = 0; i < s->n_preferred_ciphers; i++) 
		{
			ssh2_ciphers *c = s->preferred_ciphers[i];
			if (!c) continue;	       /* warning flag */
			for (j = 0; j < c->nciphers; j++) 
			{
				if (commalist_started)
					ssh2_pkt_addstring_str(s->pktout, ",");
				ssh2_pkt_addstring_str(s->pktout, c->list[j]->name);
				commalist_started = 1;
			}
	    }
	}

	/* List MAC algorithms (client->server then server->client). */
	for (j = 0; j < 2; j++) 
	{
	    ssh2_pkt_addstring_start(s->pktout);
	    for (i = 0; i < s->nmacs; i++) 
		{
			ssh2_pkt_addstring_str(s->pktout, s->maclist[i]->name);
			if (i < s->nmacs - 1)
				ssh2_pkt_addstring_str(s->pktout, ",");
	    }
	}
	/* List client->server compression algorithms,
	 * then server->client compression algorithms. (We use the
	 * same set twice.) */
	for (j = 0; j < 2; j++) 
	{
	    ssh2_pkt_addstring_start(s->pktout);
	    

	    /* Prefer non-delayed versions */
	    ssh2_pkt_addstring_str(s->pktout, s->preferred_comp->name);
	    /* We don't even list delayed versions of algorithms until
	     * they're allowed to be used, to avoid a race. See the end of
	     * this function. */
	    if (s->userauth_succeeded && s->preferred_comp->delayed_name) 
		{
			ssh2_pkt_addstring_str(s->pktout, ",");
			ssh2_pkt_addstring_str(s->pktout,s->preferred_comp->delayed_name);
	    }
	   
	}
	/* List client->server languages. Empty list. */
	ssh2_pkt_addstring_start(s->pktout);
	/* List server->client languages. Empty list. */
	ssh2_pkt_addstring_start(s->pktout);
	/* First KEX packet does _not_ follow, because we're not that brave. */

	ssh2_pkt_addbool(s->pktout, v);
	/* Reserved. */
	ssh2_pkt_adduint32(s->pktout, 0);
    

    s->our_kexinitlen = s->pktout->length - 5;
    s->our_kexinit = snewn(s->our_kexinitlen, unsigned char);
    memcpy(s->our_kexinit, s->pktout->data + 5, s->our_kexinitlen); 

    ssh2_pkt_send_noqueue(ssh, s->pktout);
	
	return 0;
}

int ssh_init_versioninfo(ssh_session*  ssh,uint8* data,int len)
{
	int verlen;

	strncpy(ssh->version,(char*)data,len);

	verlen   = hx_strcspn(ssh->version, "\r\n");
	ssh->v_s = snewn(verlen + 1, char);
	memcpy(ssh->v_s, ssh->version, verlen);
	ssh->v_s[verlen] = 0;

	ssh_send_version_info(ssh);

	ssh_init_kexinfo(ssh,NULL);

	ssh->setup = 0;

	return S_OK;
}

int ssh_send_text(ssh_session*  ssh,const char* text,int len)
{
	Packet *pktout;
	
	pktout = ssh2_pkt_init(SSH2_MSG_CHANNEL_DATA);

	ssh2_pkt_adduint32(pktout,ssh->mainchan->remoteid);
	ssh2_pkt_addstring_start(pktout);
	ssh2_pkt_addstring_data(pktout, text, len);
	ssh2_pkt_send(ssh, pktout);


	return S_OK;
}

int ssh_recv_svr_kexinfo(ssh_session* ssh,Packet* pkt)
{
	return 0;
}


typedef int (* ssh_step_func)(ssh_session* ssh,Packet* pktin);
static ssh_step_func stepfunc[8] = {
	                                ssh_step1,ssh_step2,ssh_step3,ssh_step4,
								    ssh_step5,ssh_step6,ssh_step7,ssh_step8
									};



int ssh_channel_msg(ssh_session* ssh,Packet* pktin)
{
	ssh_callbk    pcall       = (ssh_callbk)ssh->pcb;
	uint32       localid      = ssh_pkt_getuint32(pktin);
	char*        strbuf       = NULL;	
	int          len          = 0;

	
	if(pktin->type == SSH2_MSG_CHANNEL_EOF)		
	{
		pcall(SSH_USER_LOGOUT,NULL,0,ssh->wp);
		return S_LOGOUT;
	}

	if(pktin->type != SSH2_MSG_CHANNEL_DATA)		
	{
		return S_OK;
	}

	ssh_pkt_getstring(pktin, &strbuf, &len);
	pcall(SSH_SVR_TEXT,strbuf,len,ssh->wp);

	/*while(len > 0)
	{
		int  outlen = sizeof(strout);

		if(outlen > len)
		{
			outlen = len;
		}
		analyze_str(strbuf,outlen,strout);
		pcall(SSH_SVR_TEXT,strout,ssh->wp);
		
		strbuf += outlen;
		len -= outlen;
	}*/
	

	return S_OK;
}


int ssh_msg_analyze(ssh_session*  ssh,uint8* data,int len)
{
	Packet* pkt     = NULL;	
	uint8*  datapos = data;	
	int     datalen = len;
	int     leftlen = len;
	int     ret     = S_OK;
		
	if(ssh->version[0] == 0 )
	{
		return ssh_init_versioninfo(ssh,data,datalen);
	}
	
	//test
	/*extern BOOL  s_TestData;
	if(ssh->login != FALSE && s_TestData ) 
	{
		PrintLine("test");
		return S_OK;
	}*/

	while(datalen > 0)
	{
		int pktlen = 0;

		pkt = ssh_read_packet(ssh,datapos,&datalen);
		if ( pkt )
		{	
			if(ssh->login == FALSE)
			{
				ssh_step_func pfunc = stepfunc[ssh->setup];
				
				ret = pfunc(ssh,pkt);
				if(ret != S_OK)
				{
					ssh->error_id = ssh->setup;					
					break;					
				}
			}
			else
			{					
				ssh_channel_msg(ssh,pkt);				
			}				
											
			ssh_free_packet(pkt);
			datapos = data+(len-datalen);
		}
		else
		{			
			ret = S_PKT_ERR;			
			break;
		}
	}
	
	return ret;
}