
#include "ssh_def.h"
#include "ssh_pkt.h"

#define OUR_V2_PACKETLIMIT 0x9000UL

#define PKT_SETP1      1
#define PKT_SETP2      2
#define PKT_SETP3      3
#define PKT_SETP4      4
#define PKT_SETP5      5

#define PKT_DATA_ERROR     0x100

int    ssh2_pkt_construct(ssh_session* ssh, Packet *pkt);


int toint(unsigned u)
{   
    if (u <= (unsigned)INT_MAX)
        return (int)u;
    else if (u >= (unsigned)INT_MIN)   /* wrap in cast _to_ unsigned is OK */
        return INT_MIN + (int)(u - (unsigned)INT_MIN);
    else
        return INT_MIN; /* fallback; should never occur on binary machines */
}


void ssh_free_packet(Packet *pkt)
{
	if(pkt)
	{
		ssh_free(pkt->data);
		ssh_free(pkt);
	}

}

Packet *ssh_new_packet(void)
{
	Packet* pkt = (Packet*)ssh_new(sizeof(Packet));

	pkt->body = pkt->data = NULL;
	pkt->maxlen = 0;

	return pkt;
}

unsigned char *ssh2_mpint_fmt(Bignum b, int *len)
{
	unsigned char *p;
	int i, n = (bignum_bitcount(b) + 7) / 8;

	p = snewn(n + 1, unsigned char);
	p[0] = 0;

	for (i = 1; i <= n; i++)
		p[i] = bignum_byte(b, n - i);

	i = 0;
	while (i <= n && p[i] == 0 && (p[i + 1] & 0x80) == 0)
		i++;

	memmove(p, p + i, n + 1 - i);
	*len = n + 1 - i;

	return p;
}


Packet *ssh_read_packet(ssh_session* ssh,uint8* data,int* datalen)
{
	rdpkt2_state *st    = &ssh->rdpkt2_state;
	Packet* pkt         = NULL;
	static int     read_state      = S_OK;
	
		//continue last pkt read

	
	switch(read_state)
	{
		case PKT_SETP1:
			goto _STEP1;
			break;
		case PKT_SETP2:
			goto _STEP2;
			break;
		case PKT_SETP3:
			goto _STEP3;
			break;
		case PKT_SETP4:
			goto _STEP4;
			break;
	}

	st->pktin   = ssh_new_packet();

    st->pktin->type   = 0;
    st->pktin->length = 0;
    if (ssh->sccipher)
	{
		st->cipherblk = ssh->sccipher->blksize;
	}
    else
	{
		st->cipherblk = 8;
	}

    if (st->cipherblk < 8)
	{
		st->cipherblk = 8;
	}

    st->maclen = ssh->scmac ? ssh->scmac->len : 0;

    if (ssh->sccipher && (ssh->sccipher->flags & SSH_CIPHER_IS_CBC) &&	ssh->scmac) 
	{
		//PrintLine("ssh->sccipher OK");
		/* May as well allocate the whole lot now. */
		st->pktin->data = snewn(OUR_V2_PACKETLIMIT + st->maclen + APIEXTRA,	unsigned char);

		/* Read an amount corresponding to the MAC. */
		for (st->i = 0; st->i < st->maclen; st->i++) 
		{
			_STEP1:
			if(*datalen == 0) 			
			{
				read_state  = PKT_SETP1;				
				goto _END;
			}

			st->pktin->data[st->i] = *data;
			(*datalen)--; data ++;
		}

		st->packetlen = 0;
		{
			unsigned char seq[4];
			ssh->scmac->start(ssh->sc_mac_ctx);
			PUT_32BIT(seq, st->incoming_sequence);
			ssh->scmac->bytes(ssh->sc_mac_ctx, seq, 4);
		}

		for (;;) 
		{	 
			for (st->i = 0; st->i < st->cipherblk; st->i++) 
			{				
				_STEP2:
				if(*datalen == 0)  
				{
					read_state  = PKT_SETP2;
					goto _END;			 
				}
			
				st->pktin->data[st->packetlen+st->maclen+st->i] = *data;
				(*datalen)--; data ++;
			}	    
			ssh->sccipher->decrypt(ssh->sc_cipher_ctx,  st->pktin->data + st->packetlen,   st->cipherblk);
	    
			ssh->scmac->bytes(ssh->sc_mac_ctx, st->pktin->data + st->packetlen, st->cipherblk);
			st->packetlen += st->cipherblk;

			/* See if that gives us a valid packet. */
			if (ssh->scmac->verresult(ssh->sc_mac_ctx,  st->pktin->data + st->packetlen) 
				&& 	((st->len = toint(GET_32BIT(st->pktin->data))) == st->packetlen-4))
					break;

			if (st->packetlen >= OUR_V2_PACKETLIMIT) 
			{
				//ssh_free_packet(st->pktin);
				read_state  = PKT_DATA_ERROR;
				goto _END;

			}	    
		}
		
		st->pktin->maxlen  = st->packetlen + st->maclen;
		st->pktin->data    = ssh_rnew(st->pktin->data,st->pktin->maxlen + APIEXTRA);
    } 
	else 
	{
		st->pktin->data = snewn(st->cipherblk + APIEXTRA, unsigned char);
			
		for (st->i = st->len = 0; st->i < st->cipherblk; st->i++) 
		{
			_STEP3:
			if(*datalen == 0) 
			{
				read_state  = PKT_SETP3;
				goto _END;
			}			 
				
			st->pktin->data[st->i] = *data; data++; (*datalen)--;
		}

		if (ssh->sccipher) 
		{
			ssh->sccipher->decrypt(ssh->sc_cipher_ctx, st->pktin->data, st->cipherblk);
			
		}

		st->len = toint(GET_32BIT(st->pktin->data));

		if (st->len < 0 || st->len > OUR_V2_PACKETLIMIT || (st->len + 4) % st->cipherblk != 0) 
		{		
			ssh_free_packet(st->pktin);		
		}

		st->packetlen = st->len + 4;

		st->pktin->maxlen = st->packetlen + st->maclen;
		st->pktin->data   = ssh_rnew(st->pktin->data, st->pktin->maxlen + APIEXTRA);

	
		for (st->i = st->cipherblk; st->i < st->packetlen + st->maclen;     st->i++) 
		{
			_STEP4:
			if(*datalen == 0) 
			{
				read_state  = PKT_SETP4;
				goto _END;
			}

			st->pktin->data[st->i] = *data; data++; (*datalen)--;
		}
	
		if (ssh->sccipher)
		{
			ssh->sccipher->decrypt(ssh->sc_cipher_ctx,st->pktin->data + st->cipherblk, st->packetlen - st->cipherblk);
		}

		if (ssh->scmac && !ssh->scmac->verify(ssh->sc_mac_ctx, st->pktin->data,st->len + 4, st->incoming_sequence)) 
		{
			read_state  = PKT_DATA_ERROR;
			goto _END;
		}
    }
    /* Get and sanity-check the amount of random padding. */
    st->pad = st->pktin->data[4];
    if (st->pad < 4 || st->len - st->pad < 1) 
	{
		read_state  = PKT_DATA_ERROR;
		goto _END;
    }
    /*
     * This enables us to deduce the payload length.
     */
    st->payload        = st->len - st->pad - 1;
    st->pktin->length   = st->payload + 5;
	st->pktin->encrypted_len = st->packetlen;
    st->pktin->sequence = st->incoming_sequence++;
    st->pktin->length = st->packetlen - st->pad;
    
    /*
     * Decompress packet payload.
     */
    {
		unsigned char *newpayload;
		int newlen;
		int oldlen;
		if (ssh->sccomp &&  ssh->sccomp->decompress(ssh->sc_comp_ctx,   st->pktin->data + 5, st->pktin->length - 5, &newpayload, &newlen)) 
		{
			if (st->pktin->maxlen < newlen + 5) 
			{
				oldlen = st->pktin->maxlen;
				st->pktin->maxlen = newlen + 5;
				st->pktin->data = ssh_rnew(st->pktin->data,st->pktin->maxlen + APIEXTRA);
			}
			st->pktin->length = 5 + newlen;
			memcpy(st->pktin->data + 5, newpayload, newlen);
			sfree(newpayload);
		}
    }

    /*
     * RFC 4253 doesn't explicitly say that completely empty packets
     * with no type byte are forbidden, so treat them as deserving
     * an SSH_MSG_UNIMPLEMENTED.
     */
    if (st->pktin->length <= 5) 
	{ 
		/* == 5 we hope, but robustness */
        //ssh2_msg_something_unimplemented(ssh, st->pktin);
		read_state  = PKT_DATA_ERROR;
		goto _END;        
    }

	read_state = S_OK;
_END:
	
	if(read_state != S_OK)
	{		
		if(read_state == PKT_DATA_ERROR)
		{
			ssh_free_packet(st->pktin);
			st->pktin    = NULL;		
			read_state   = S_OK;
		}		

		return NULL;
	}
	else 
	{
		st->pktin->type    = st->pktin->data[5];
		st->pktin->body    = st->pktin->data + 6;
		st->pktin->length -= 6;        
		st->pktin->savedpos = 0;
	}
		
	return st->pktin;
}

Packet *ssh_read_packet_old(ssh_session* ssh,uint8* data,int* len)
{
	rdpkt2_state *st    = &ssh->rdpkt2_state;
	Packet* pkt         = NULL;//
	int     cipherblk  = 8;
	int     datalen    = *len;
	int     i;

	st->pktin       = ssh_new_packet();
	pkt             = st->pktin;

	if (ssh->sccipher)
		st->cipherblk = ssh->sccipher->blksize;
	else
		st->cipherblk = 8;

	if (st->cipherblk < 8)
		st->cipherblk = 8;		

	if (ssh->sccipher && (ssh->sccipher->flags & SSH_CIPHER_IS_CBC) &&	ssh->scmac)
	{
		i = 0;
	}
		
	st->pktin->data = (uint8*)ssh_new(cipherblk);
	st->cipherblk   = 8;

	for (i  = 0; i < cipherblk; i++) 
	{
		while (datalen == 0)
		{
			return NULL;
		}

		pkt->data[i] = *data; data++;
		datalen --;
	}
	
	st->len = toint(GET_32BIT(st->pktin->data));
	st->packetlen = st->len + 4;

	/*
	 * Allocate memory for the rest of the packet.
	 */
	pkt->maxlen = st->packetlen + st->maclen;
	pkt->data = (uint8*)ssh_rnew(pkt->data, pkt->maxlen + APIEXTRA);

	for (st->i = st->cipherblk; st->i < st->packetlen + st->maclen;	st->i++) 
	{
		while (datalen == 0) 
		{
			return NULL;
		}

		pkt->data[st->i] = *data; data++;
		datalen--;
	}
	
	st->pad             = pkt->data[4];
	st->payload         = st->len - st->pad - 1;
	pkt->length         = st->payload + 5;
	pkt->encrypted_len  = st->packetlen;
	pkt->sequence       = st->incoming_sequence++;
	pkt->length         = st->packetlen - st->pad;


	pkt->type    = pkt->data[5];
	pkt->body    = pkt->data + 6;
	pkt->length -= 6;

	pkt->savedpos = 0;

	*len = datalen;

	return pkt;
}


void ssh_pkt_getstring(Packet *pkt, char **p, int *length)
{
	int len;

	*p      = NULL;
	*length = 0;

	if (pkt->length - pkt->savedpos < 4) 	
	{
		return;
	}

	len = toint(GET_32BIT(pkt->body + pkt->savedpos));

	if (len < 0) return;

	*length       = len;
	pkt->savedpos += 4;

	if (pkt->length - pkt->savedpos < *length)
	{
		return;
	}

	*p = (char *)(pkt->body + pkt->savedpos);

	pkt->savedpos += *length;

}

/*
 * Packet construction functions. Mostly shared between SSH-1 and SSH-2.
 */
void ssh_pkt_ensure( Packet *pkt, int length)
{
    if (pkt->maxlen < length) 
	{
		unsigned char *body = pkt->body;

		int offset = body ? body - pkt->data : 0;
		int oldlen = pkt->maxlen;
	
		pkt->maxlen = length + 256;
		pkt->data   = ssh_rnew(pkt->data,pkt->maxlen + APIEXTRA);
	
		if (body) pkt->body = pkt->data + offset;
    }
}

void ssh_pkt_adddata( Packet *pkt, const void *data, int len)
{
    pkt->length += len;
    ssh_pkt_ensure(pkt, pkt->length);
    memcpy(pkt->data + pkt->length - len, data, len);
}

void ssh_pkt_addbyte( Packet *pkt, unsigned char byte)
{
    ssh_pkt_adddata(pkt, &byte, 1);
}

void ssh2_pkt_addbool( Packet *pkt, unsigned char value)
{
    ssh_pkt_adddata(pkt, &value, 1);
}

void ssh_pkt_adduint32( Packet *pkt, unsigned long value)
{
    unsigned char x[4];
    PUT_32BIT(x, value);
    ssh_pkt_adddata(pkt, x, 4);
}

void ssh_pkt_addstring_start( Packet *pkt)
{
    ssh_pkt_adduint32(pkt, 0);
    pkt->savedpos = pkt->length;
}

void ssh_pkt_addstring_str( Packet *pkt, const char *data)
{
    ssh_pkt_adddata(pkt, data, strlen(data));
    PUT_32BIT(pkt->data + pkt->savedpos - 4, pkt->length - pkt->savedpos);
}

void ssh_pkt_addstring_data( Packet *pkt, const char *data, int len)
{
    ssh_pkt_adddata(pkt, data, len);
    PUT_32BIT(pkt->data + pkt->savedpos - 4, pkt->length - pkt->savedpos);
}

void ssh_pkt_addstring( Packet *pkt, const char *data)
{
    ssh_pkt_addstring_start(pkt);
    ssh_pkt_addstring_str(pkt, data);
}

Packet *ssh2_pkt_init(int pkt_type)
{
	Packet *pkt = ssh_new_packet();

	pkt->length = 5; /* space for packet length + padding length */
	pkt->forcepad = 0;
	pkt->type = pkt_type;

	ssh_pkt_addbyte(pkt, (unsigned char) pkt_type);

	pkt->body = pkt->data + pkt->length; /* after packet type */
	pkt->downstream_id = 0;
	pkt->additional_log_text = NULL;

	return pkt;
}


/*
 * Throttle or unthrottle _all_ local data streams (for when sends
 * on the SSH connection itself back up).
 */
void ssh_throttle_all(ssh_session* ssh, int enable, int bufsize)
{
	return;
}

void ssh_set_frozen(ssh_session* ssh, int frozen)
{
	//pause 
	//if (ssh->s)
	//	sk_set_frozen(ssh->s, frozen);
	//ssh->frozen = frozen;
}

/*
 * Defer an SSH-2 packet.
 */
void ssh2_pkt_defer_noqueue(ssh_session* ssh,  Packet *pkt, int noignore)
{
    int len;

    if (ssh->cscipher != NULL && (ssh->cscipher->flags & SSH_CIPHER_IS_CBC) && 	ssh->deferred_len == 0 && !noignore &&
	!(ssh->remote_bugs & BUG_CHOKES_ON_SSH2_IGNORE)) 
	{
		/*
		 * Interpose an SSH_MSG_IGNORE to ensure that user data don't
		 * get encrypted with a known IV.
		 */
		Packet *ipkt = ssh2_pkt_init(SSH2_MSG_IGNORE);
		ssh2_pkt_addstring_start(ipkt);
		ssh2_pkt_defer_noqueue(ssh, ipkt, TRUE);
    }

    len = ssh2_pkt_construct(ssh, pkt);
    if (ssh->deferred_len + len > ssh->deferred_size) 
	{	
		int oldlen = ssh->deferred_size;

		ssh->deferred_size = ssh->deferred_len + len + 128;
		ssh->deferred_send_data = ssh_rnew(ssh->deferred_send_data,ssh->deferred_size);
    }

    memcpy(ssh->deferred_send_data + ssh->deferred_len, pkt->body, len);
    ssh->deferred_len += len;
    ssh->deferred_data_size += pkt->encrypted_len;
    ssh_free_packet(pkt);
}

void ssh_pkt_defersend(ssh_session* ssh)
{
	int backlog = 0;

	// backlog = s_write(ssh, ssh->deferred_send_data, ssh->deferred_len);
	//_hx_printf("send1: %d bytes\r\n",ssh->deferred_len);
	if(ssh->deferred_len > 0 )
	{
		backlog = send(ssh->sd,ssh->deferred_send_data, ssh->deferred_len,0);
	}
		
	ssh->deferred_len = ssh->deferred_size = 0;
	sfree(ssh->deferred_send_data);
	ssh->deferred_send_data = NULL;
	if (backlog > SSH_MAX_BACKLOG)
	{
		ssh_throttle_all(ssh, 1, backlog);
	}

	ssh->outgoing_data_size += ssh->deferred_data_size;
	
	/*if (!ssh->kex_in_progress &&
		!ssh->bare_connection &&
		ssh->max_data_size != 0 &&
		ssh->outgoing_data_size > ssh->max_data_size)
		do_ssh2_transport(ssh, "too much data sent", -1, NULL);*/

	ssh->deferred_data_size = 0;
}
/*
 * Construct an SSH-2 final-form packet: compress it, encrypt it,
 * put the MAC on it. Final packet, ready to be sent, is stored in
 * pkt->data. Total length is returned.
 */
int ssh2_pkt_construct(ssh_session* ssh, Packet *pkt)
{
    int cipherblk, maclen, padding, i;

    //if (ssh->logctx)
     //   ssh2_log_outgoing_packet(ssh, pkt);

    if (ssh->bare_connection) 
	{
        /*
         * Trivial packet construction for the bare connection
         * protocol.
         */
        PUT_32BIT(pkt->data + 1, pkt->length - 5);
        pkt->body = pkt->data + 1;
        ssh->v2_outgoing_sequence++;   /* only for diagnostics, really */
        return pkt->length - 1;
    }

    /*
     * Compress packet payload.
     */
    {
	unsigned char *newpayload;
	int newlen;
	if (ssh->cscomp && ssh->cscomp->compress(ssh->cs_comp_ctx, pkt->data + 5,
				  pkt->length - 5, &newpayload, &newlen)) 
		{
			pkt->length = 5;
			ssh2_pkt_adddata(pkt, newpayload, newlen);
			sfree(newpayload);
		}
    }

    /*
     * Add padding. At least four bytes, and must also bring total
     * length (minus MAC) up to a multiple of the block size.
     * If pkt->forcepad is set, make sure the packet is at least that size
     * after padding.
     */
    cipherblk = ssh->cscipher ? ssh->cscipher->blksize : 8;  /* block size */
    cipherblk = cipherblk < 8 ? 8 : cipherblk;	/* or 8 if blksize < 8 */
    padding = 4;
    if (pkt->length + padding < pkt->forcepad)
	padding = pkt->forcepad - pkt->length;
    padding += 	(cipherblk - (pkt->length + padding) % cipherblk) % cipherblk;

    //assert(padding <= 255);

    maclen = ssh->csmac ? ssh->csmac->len : 0;
    ssh2_pkt_ensure(pkt, pkt->length + padding + maclen);
    pkt->data[4] = padding;

    for (i = 0; i < padding; i++)
		{
		pkt->data[pkt->length + i] = 0xcd;//random_byte();
		}

    PUT_32BIT(pkt->data, pkt->length + padding - 4);
    
	if (ssh->csmac)
		{
			ssh->csmac->generate(ssh->cs_mac_ctx, pkt->data,pkt->length + padding,ssh->v2_outgoing_sequence);
		}
    ssh->v2_outgoing_sequence++;       /* whether or not we MACed */

    if (ssh->cscipher)
	{
		ssh->cscipher->encrypt(ssh->cs_cipher_ctx, pkt->data, pkt->length + padding);
	}

    pkt->encrypted_len = pkt->length + padding;

    /* Ready-to-send packet starts at pkt->data. We return length. */
    pkt->body = pkt->data;
    return pkt->length + padding + maclen;
}

/*
 * Send an SSH-2 packet immediately, without queuing or deferring.
 */
void ssh2_pkt_send_noqueue(ssh_session* ssh,  Packet *pkt)
{
    int len;
    int backlog = 0;

    if (ssh->cscipher != NULL && (ssh->cscipher->flags & SSH_CIPHER_IS_CBC)) 
	{
		// We need to send two packets, so use the deferral mechanism. 
		ssh2_pkt_defer_noqueue(ssh, pkt, FALSE);
		ssh_pkt_defersend(ssh);

		return;
    }

    len     = ssh2_pkt_construct(ssh, pkt);
	//_hx_printf("send2: %d bytes\r\n",len);
   // backlog = s_write(ssh, pkt->body, len);
	if(len > 0 )
	{
		backlog = send(ssh->sd,pkt->body, len,0);
	}

	//logdata_out(pkt->body, len);
    //if (backlog > SSH_MAX_BACKLOG)
	//{
		//ssh_throttle_all(ssh, 1, backlog);
	//}

    ssh->outgoing_data_size += pkt->encrypted_len;    

	/*if (!ssh->kex_in_progress &&
        !ssh->bare_connection &&
	ssh->max_data_size != 0 &&
	ssh->outgoing_data_size > ssh->max_data_size)
	do_ssh2_transport(ssh, "too much data sent", -1, NULL);

	error
	*/

    ssh_free_packet(pkt);
}

void ssh2_pkt_queuesend(ssh_session* ssh)
{
	int i;

	//assert(!ssh->queueing);

	for (i = 0; i < ssh->queuelen; i++)
		ssh2_pkt_defer_noqueue(ssh, ssh->queue[i], FALSE);
	ssh->queuelen = 0;

	ssh_pkt_defersend(ssh);
}

void ssh2_pkt_queue(ssh_session* ssh, Packet *pkt)
{
	
	if (ssh->queuelen >= ssh->queuesize) 
	{
		ssh->queuesize = ssh->queuelen + 32;
		ssh->queue     = ssh_rnew(ssh->queue, ssh->queuesize*sizeof(Packet *));
	}

	ssh->queue[ssh->queuelen++] = pkt;
}

void ssh2_pkt_send(ssh_session* ssh, Packet *pkt)
{
	if (ssh->queueing)

		ssh2_pkt_queue(ssh, pkt);
	else
		ssh2_pkt_send_noqueue(ssh, pkt);
}


void ssh2_pkt_addmp(Packet *pkt, Bignum b)
{
	unsigned char *p;
	int len;

	p = ssh2_mpint_fmt(b, &len);
	ssh_pkt_addstring_start(pkt);
	ssh_pkt_addstring_data(pkt, (char *)p, len);

	sfree(p);
}


void ssh2_pkt_defer(ssh_session*  ssh, Packet *pkt)
{
	if (ssh->queueing)
	{
		ssh2_pkt_queue(ssh, pkt);
	}
	else
	{
		ssh2_pkt_defer_noqueue(ssh, pkt, FALSE);
	}
}


void ssh2_pkt_send_with_padding(ssh_session*  ssh,  Packet *pkt,int padsize)
{

	ssh2_pkt_defer(ssh, pkt);

	if (ssh->cscipher &&    !(ssh->remote_bugs & BUG_CHOKES_ON_SSH2_IGNORE)) 
	{
		int stringlen, i;

		stringlen = (256 - ssh->deferred_len);
		stringlen += ssh->cscipher->blksize - 1;
		stringlen -= (stringlen % ssh->cscipher->blksize);
		if (ssh->cscomp) 
		{		
			stringlen -= ssh->cscomp->disable_compression(ssh->cs_comp_ctx);
		}

		pkt = ssh2_pkt_init(SSH2_MSG_IGNORE);

		ssh2_pkt_addstring_start(pkt);
		for (i = 0; i < stringlen; i++) 
		{
			char c = (char) random_byte();
			ssh2_pkt_addstring_data(pkt, &c, 1);
		}

		ssh2_pkt_defer(ssh, pkt);
	}

	ssh_pkt_defersend(ssh);    
}


/*
 * Packet decode functions for both SSH-1 and SSH-2.
 */
unsigned long ssh_pkt_getuint32( Packet *pkt)
{
    unsigned long value;

	if (pkt->length - pkt->savedpos < 4)
	{
		return 0;		       /* arrgh, no way to decline (FIXME?) */
	}
    
	value = GET_32BIT(pkt->body + pkt->savedpos);
    pkt->savedpos += 4;

    return value;
}


Bignum ssh2_pkt_getmp(Packet *pkt)
{
	char *p;
	int length;
	Bignum b;

	ssh_pkt_getstring(pkt, &p, &length);

	if (!p) return NULL;

	if (p[0] & 0x80) return NULL;

	b = bignum_from_bytes((unsigned char *)p, length);

	return b;
}