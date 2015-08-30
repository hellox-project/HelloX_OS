/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwBT Bluetooth stack.
 * 
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */


/* rfcomm.c
 *
 * Implementation of the RFCOMM protocol. A subset of the ETSI TS 07.10 standard with 
 * some Bluetooth-specific adaptations.
 */


#include "lwbt/l2cap.h"
#include "lwbt/rfcomm.h"
#include "lwbt/lwbt_memp.h"
#include "lwbt/fcs.h"
#include "lwbt/lwbtopts.h"
#include "lwip/debug.h"

struct rfcomm_pcb_listen *rfcomm_listen_pcbs; /* List of all RFCOMM PCBs listening for 
												 an incomming connection on a specific
												 server channel */
struct rfcomm_pcb *rfcomm_active_pcbs;  /* List of all active RFCOMM PCBs */
struct rfcomm_pcb *rfcomm_tmp_pcb;

/* Forward declarations */
struct rfcomm_pcb *rfcomm_get_active_pcb(u8_t cn, struct bd_addr *bdaddr);


/* 
 * rfcomm_init():
 * 
 * Initializes the rfcomm layer.
 */
void rfcomm_init(void)
{
	/* Clear globals */
	rfcomm_listen_pcbs = NULL;
	rfcomm_active_pcbs = NULL;
	rfcomm_tmp_pcb = NULL;
}

/*
 * rfcomm_tmr():
 *
 * Called every 1s and implements the command timer that
 * removes a DLC if it has been waiting for a response enough
 * time.
 */
void rfcomm_tmr(void)
{
	struct rfcomm_pcb *pcb, *tpcb;
	err_t ret;

	/* Step through all of the active pcbs */
	for(pcb = rfcomm_active_pcbs; pcb != NULL; pcb = pcb->next) {
		if(pcb->to != 0) {
			--pcb->to;
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_tmr: %d\n", pcb->to));
			if(pcb->to == 0) {
				/* Timeout */
				if(pcb->cn == 0) {
					/* If DLC 0 timed out, disconnect all other DLCs on this multiplexer session first */
					for(tpcb = rfcomm_active_pcbs; tpcb != NULL; tpcb = tpcb->next) {
						if(tpcb->cn != 0 && bd_addr_cmp(&(tpcb->l2cappcb->remote_bdaddr), &(pcb->l2cappcb->remote_bdaddr))) {
							//RFCOMM_RMV(&rfcomm_active_pcbs, tpcb); /* Remove pcb from active list */
							tpcb->state = RFCOMM_CLOSED;
							RFCOMM_EVENT_DISCONNECTED(tpcb,ERR_OK,ret); /* Notify upper layer */
						}
					}
				}
				/* Disconnect this DLC */
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_tmr: Timeout! Disconnect this DLC. State = %d\n", pcb->state));
				//RFCOMM_RMV(&rfcomm_active_pcbs, pcb); /* Remove pcb from active list */
				pcb->state = RFCOMM_CLOSED;
				RFCOMM_EVENT_DISCONNECTED(pcb,ERR_OK,ret); /* Notify upper layer */
			}
		}
	}
}

/*
 * rfcomm_lp_disconnected():
 *
 * Called by the application to indicate that the lower protocol disconnected. Closes
 * any active PCBs in the lists
 */
err_t rfcomm_lp_disconnected(struct l2cap_pcb *l2cappcb)
{
	struct rfcomm_pcb *pcb, *tpcb;
	err_t ret = ERR_OK;

	pcb = rfcomm_active_pcbs;
	while(pcb != NULL) {
		tpcb = pcb->next;
		if(bd_addr_cmp(&(l2cappcb->remote_bdaddr), &(pcb->l2cappcb->remote_bdaddr))) {
			pcb->state = RFCOMM_CLOSED;
			RFCOMM_EVENT_DISCONNECTED(pcb,ERR_OK,ret); /* Notify upper layer */
		}
		pcb = tpcb;
	}

	return ret;
}

/* 
 * rfcomm_new():
 *
 * Creates a new RFCOMM protocol control block but doesn't place it on
 * any of the RFCOMM PCB lists.
 */
struct rfcomm_pcb * rfcomm_new(struct l2cap_pcb *l2cappcb) 
{
	struct rfcomm_pcb *pcb;

	pcb = lwbt_memp_malloc(MEMP_RFCOMM_PCB);
	if(pcb != NULL) {
		memset(pcb, 0, sizeof(struct rfcomm_pcb));
		pcb->l2cappcb = l2cappcb;

		pcb->cl = RFCOMM_CL; /* Default convergence layer */
		pcb->n = RFCOMM_N; /* Default maximum frame size */ 

		pcb->state = RFCOMM_CLOSED;
		return pcb;
	}
	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_new: Could not allocate a new pcb\n"));
	return NULL;
}

/* 
 * rfcomm_close():
 *
 * Closes the RFCOMM protocol control block.
 */
void rfcomm_close(struct rfcomm_pcb *pcb) 
{
#if RFCOMM_FLOW_QUEUEING
	if(pcb->buf != NULL) {
		pbuf_free(pcb->buf);
	}
#endif
	if(pcb->state == RFCOMM_LISTEN) {
		RFCOMM_RMV((struct rfcomm_pcb **)&rfcomm_listen_pcbs, pcb);
		lwbt_memp_free(MEMP_RFCOMM_PCB_LISTEN, pcb);
	} else {
		RFCOMM_RMV(&rfcomm_active_pcbs, pcb);
		lwbt_memp_free(MEMP_RFCOMM_PCB, pcb);
	}
	pcb = NULL;
}

/* 
 * rfcomm_reset_all():
 *
 * Closes all active and listening RFCOMM protocol control blocks.
 */
void rfcomm_reset_all(void) 
{
	struct rfcomm_pcb *pcb, *tpcb;
	struct rfcomm_pcb_listen *lpcb, *tlpcb;

	for(pcb = rfcomm_active_pcbs; pcb != NULL;) {
		tpcb = pcb->next;
		rfcomm_close(pcb);
		pcb = tpcb;
	}

	for(lpcb = rfcomm_listen_pcbs; lpcb != NULL;) {
		tlpcb = lpcb->next;
		rfcomm_close((struct rfcomm_pcb *)lpcb);
		lpcb = tlpcb;
	}

	rfcomm_init();
}

/* 
 * rfcomm_get_active_pcb():
 *
 * Return the active PCB with the matching Bluetooth address and channel number.
 */
struct rfcomm_pcb * rfcomm_get_active_pcb(u8_t cn, struct bd_addr *bdaddr)
{
	struct rfcomm_pcb *pcb;
	for(pcb = rfcomm_active_pcbs; pcb != NULL; pcb = pcb->next) {
		if(pcb->cn == cn && bd_addr_cmp(&(pcb->l2cappcb->remote_bdaddr), 
					bdaddr)) {
			break;
		}
	}
	return pcb;
}

/* 
 * rfcomm_dm():
 *
 * Sends a RFCOMM disconnected mode frame in response to a command when disconnected.
 */
static err_t rfcomm_dm(struct l2cap_pcb *pcb, struct rfcomm_hdr *hdr) 
{
	struct pbuf *p;
	struct rfcomm_hdr *rfcommhdr;
	err_t ret;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_dm\n"));

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_DM_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_dm: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	rfcommhdr = p->payload;
	rfcommhdr->addr = hdr->addr & 0xFB; /* Set direction bit to 0 for the response */
	rfcommhdr->ctrl = RFCOMM_DM;
	rfcommhdr->len = 1; /* EA bit set to 1 to indicate a 7 bit length field */
	((u8_t *)p->payload)[RFCOMM_HDR_LEN_1] = fcs8_crc_calc(p, RFCOMM_CRC_CHECK_LEN);

	ret = l2ca_datawrite(pcb, p);
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_connect():
 *
 * Sends a RFCOMM start asynchronous balanced mode frame to startup the channel. Also
 * specify the function to be called when the channel has been connected.
 */
err_t rfcomm_connect(struct rfcomm_pcb *pcb, u8_t cn,
		err_t (* connected)(void *arg, struct rfcomm_pcb *tpcb,	err_t err))
{
	struct rfcomm_hdr *hdr;
	struct pbuf *p;
	err_t ret;
	struct rfcomm_pcb *tpcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_connect\n"));

	pcb->connected = connected;
	pcb->cn = cn;
	pcb->rfcommcfg |= RFCOMM_CFG_IR; /* Set role to initiator */ 

	/* Create multiplexer session if one does not already exist */
	if(cn != 0) {
		tpcb = rfcomm_get_active_pcb(0, &pcb->l2cappcb->remote_bdaddr);

		if(tpcb == NULL) {
			pcb->state = W4_RFCOMM_MULTIPLEXER;
			RFCOMM_REG(&rfcomm_active_pcbs, pcb);
			pcb = rfcomm_new(pcb->l2cappcb);
			pcb->rfcommcfg |= RFCOMM_CFG_IR; /* Set role to initiator */
		} 
	} 

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_SABM_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_connect: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	hdr = p->payload;
	hdr->addr = (1 << 0) | ((pcb->rfcommcfg & RFCOMM_CFG_IR) << 1) | (((pcb->rfcommcfg & RFCOMM_CFG_IR) ^ 1) << 2) | (pcb->cn << 3);
	hdr->ctrl = RFCOMM_SABM;
	hdr->len = (1 << 0) | (0 << 1);
	((u8_t *)p->payload)[RFCOMM_HDR_LEN_1] = fcs8_crc_calc(p, RFCOMM_CRC_CHECK_LEN);

	if((ret = l2ca_datawrite(pcb->l2cappcb, p)) == ERR_OK) {
		pcb->state = W4_RFCOMM_SABM_RSP;
		pcb->to = 5*RFCOMM_TO; /* Set acknowledgement timer, 50-300s (5*10-60s) */
	}

	if((tpcb = rfcomm_get_active_pcb(pcb->cn, &pcb->l2cappcb->remote_bdaddr)) == NULL) {
		RFCOMM_REG(&rfcomm_active_pcbs, pcb);
	}

	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_disconnect():
 *
 * Sends a RFCOMM disconnect frame to close the channel.
 */
err_t rfcomm_disconnect(struct rfcomm_pcb *pcb)
{
	struct rfcomm_hdr *hdr;
	struct pbuf *p;
	err_t ret;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_disconnect\n"));

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_DISC_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_disconnect: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	p = pbuf_alloc(PBUF_RAW, RFCOMM_DISC_LEN, PBUF_RAM);
	hdr = p->payload;
	hdr->addr = (1 << 0) | ((pcb->rfcommcfg & RFCOMM_CFG_IR) << 1) | (((pcb->rfcommcfg & RFCOMM_CFG_IR) ^ 1) << 2) | (pcb->cn << 3);
	hdr->ctrl = RFCOMM_DISC;
	hdr->len = (1 << 0) | (0 << 1);
	((u8_t *)p->payload)[RFCOMM_HDR_LEN_1] = fcs8_crc_calc(p, RFCOMM_CRC_CHECK_LEN);
	pcb->state = W4_RFCOMM_DISC_RSP;

	if((ret = l2ca_datawrite(pcb->l2cappcb, p)) == ERR_OK) {
		pcb->to = RFCOMM_TO; /* Set acknowledgement timer, 10-60s */
	}
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_ua():
 *
 * Sends a RFCOMM unnumbered acknowledgement to respond to a connection request.
 */
static err_t rfcomm_ua(struct l2cap_pcb *pcb, struct rfcomm_hdr *hdr)
	//RESPONDER
{
	struct pbuf *p;
	struct rfcomm_hdr *rfcommhdr;
	err_t ret;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_ua\n"));

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_UA_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_ua: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	rfcommhdr = p->payload;
	rfcommhdr->addr = hdr->addr & 0xFB; /* Set direction bit to 0 for the response */
	rfcommhdr->ctrl = RFCOMM_UA;
	rfcommhdr->len = 1; /* EA bit set to 1 to indicate a 7 bit length field */
	((u8_t *)p->payload)[RFCOMM_HDR_LEN_1] = fcs8_crc_calc(p, RFCOMM_CRC_CHECK_LEN);

	ret = l2ca_datawrite(pcb, p);
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_pn():
 *
 * Sends a RFCOMM parameter negotiation multiplexer frame to negotiate the parameters
 * of a data link connection. Also specify the function to be called when a PN 
 * response is received
 */
err_t rfcomm_pn(struct rfcomm_pcb *pcb, 
		err_t (* pn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err))
{
	struct pbuf *p;
	struct rfcomm_msg_hdr *cmdhdr;
	struct rfcomm_pn_msg *pnmsg;
	err_t ret;
	struct rfcomm_pcb *opcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_pn\n"));

	opcb = rfcomm_get_active_pcb(0, &pcb->l2cappcb->remote_bdaddr);

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_MSGHDR_LEN + RFCOMM_PNMSG_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_pn: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Set multiplexer parameter negotiation command header */
	cmdhdr = p->payload;
	cmdhdr->type = RFCOMM_PN_CMD;
	cmdhdr->len = 1 | (RFCOMM_PNMSG_LEN << 1);

	/* Set multiplexer parameter negotiation command paramenters */
	pnmsg = (void *)(((u8_t *)p->payload) + RFCOMM_MSGHDR_LEN);
	pnmsg->dlci = (((opcb->rfcommcfg & RFCOMM_CFG_IR) ^ 1) << 0) | (pcb->cn << 1);
	pnmsg->i_cl = 0 | (RFCOMM_CL << 4);
	pnmsg->p = 0;
	pnmsg->t = 0;
	pnmsg->n = RFCOMM_N;
	pnmsg->na = 0;
	pnmsg->k = RFCOMM_K;

	if((ret = rfcomm_uih(opcb, 0, p)) == ERR_OK) {
		pcb->pn_rsp = pn_rsp;
		opcb->to = RFCOMM_TO; /* Set acknowledgement timer, 10-60s */
	}
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_test():
 *
 * .
 */
err_t rfcomm_test(struct rfcomm_pcb *pcb, err_t (* test_rsp)(void *arg, struct rfcomm_pcb *tpcb, err_t err)) 
{
	struct pbuf *p;
	struct rfcomm_msg_hdr *cmdhdr;
	err_t ret;
	struct rfcomm_pcb *opcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_test\n"));

	opcb = rfcomm_get_active_pcb(0, &pcb->l2cappcb->remote_bdaddr);

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_MSGHDR_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_test: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Set multiplexer modem status command header */
	cmdhdr = p->payload;
	cmdhdr->type = RFCOMM_TEST_CMD;
	cmdhdr->len = 1 | (0 << 1);

	if((ret = rfcomm_uih(opcb, 0, p)) == ERR_OK) {
		opcb->test_rsp = test_rsp;
		opcb->to = RFCOMM_TO; /* Set acknowledgement timer, 10-60s */
	}
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_msc():
 *
 * Sends a RFCOMM modem status multiplexer frame. Also specify the function to be 
 * called when a MSC response is received.
 */
err_t rfcomm_msc(struct rfcomm_pcb *pcb, u8_t fc, 
		err_t (* msc_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err))
{
	struct pbuf *p;
	struct rfcomm_msg_hdr *cmdhdr;
	struct rfcomm_msc_msg *mscmsg;
	err_t ret;
	struct rfcomm_pcb *opcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_msc\n"));

	opcb = rfcomm_get_active_pcb(0, &pcb->l2cappcb->remote_bdaddr);

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_MSGHDR_LEN + RFCOMM_MSCMSG_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_msc: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Set multiplexer modem status command header */
	cmdhdr = p->payload;
	cmdhdr->type = RFCOMM_MSC_CMD;
	cmdhdr->len = 1 | (RFCOMM_MSCMSG_LEN << 1);

	/* Set multiplexer parameter negotiation command paramenters */
	mscmsg = (void *)(((u8_t *)p->payload) + RFCOMM_MSGHDR_LEN);
	// mscmsg->dlci = (1 << 0) | (1 << 1) | (((pcb->rfcommcfg & RFCOMM_CFG_IR) ^ 1) << 2) | (pcb->cn << 3);
	mscmsg->dlci = (1 << 0) | (1 << 1) | (0 << 2) | (pcb->cn << 3);
	mscmsg->rs232 = (1 << 0) | (fc << 1) | (0x23 << 2);

	if((ret = rfcomm_uih(opcb, 0, p)) == ERR_OK) {
		pcb->msc_rsp = msc_rsp;
		opcb->to = RFCOMM_TO; /* Set acknowledgement timer, 10-60s */
	}
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_rpn():
 *
 * Sends a RFCOMM remote port negotiation multiplexer frame to set communication 
 * settings at the remote end of the data link connection.
 */
err_t rfcomm_rpn(struct rfcomm_pcb *pcb, u8_t br,
		err_t (* rpn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err))
	 //INITIATOR
{
	struct pbuf *p;
	struct rfcomm_msg_hdr *cmdhdr;
	struct rfcomm_rpn_msg *rpnmsg;
	err_t ret;
	struct rfcomm_pcb *opcb;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_rpn\n"));

	opcb = rfcomm_get_active_pcb(0, &pcb->l2cappcb->remote_bdaddr);

	if((p = pbuf_alloc(PBUF_RAW, RFCOMM_MSGHDR_LEN + RFCOMM_RPNMSG_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_rpn: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Set remote port negotiation command header */
	cmdhdr = p->payload;
	cmdhdr->type = RFCOMM_RPN_CMD;
	cmdhdr->len = 1 | (RFCOMM_RPNMSG_LEN << 1);

	/* Set remote port negotiation command paramenters */
	rpnmsg = (void *)(((u8_t *)p->payload) + RFCOMM_MSGHDR_LEN);
	rpnmsg->dlci = (1 << 0) | (1 << 1) | (((opcb->rfcommcfg & RFCOMM_CFG_IR) ^ 1) << 2) | (pcb->cn << 3);
	rpnmsg->br = br;
	rpnmsg->mask = 1;

	if((ret = rfcomm_uih(opcb, 0, p)) == ERR_OK) {
		pcb->rpn_rsp = rpn_rsp;
		opcb->to = RFCOMM_TO; /* Set acknowledgement timer, 10-60s */

	}
	pbuf_free(p);
	return ret;
}

/* 
 * rfcomm_uih():
 *
 * Sends a RFCOMM unnumbered information frame with header check.
 */
err_t rfcomm_uih(struct rfcomm_pcb *pcb, u8_t cn, struct pbuf *q)
	//RESPONDER & INITIATOR
{
	struct pbuf *p, *r;
	err_t ret;
	u16_t tot_len = 0;

	/* Decrease local credits */
	if(pcb->cl == 0xF && pcb->state == RFCOMM_OPEN && pcb->cn != 0) {
		if(pcb->k != 0) {
			--pcb->k;
		} else {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Out of local credits\n"));
#if RFCOMM_FLOW_QUEUEING
			if(q != NULL) {
				/* Packet can be queued? */
				if(pcb->buf != NULL) {
					return ERR_OK; /* Drop packet */
				} else {
					/* Copy PBUF_REF referenced payloads into PBUF_RAM */
					q = pbuf_take(q);
					/* Remember pbuf to queue, if any */
					pcb->buf = q;
					/* Pbufs are queued, increase the reference count */
					pbuf_ref(q);
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Queued packet %p on channel %d\n", (void *)q, pcb->cn));
				}
			}
#else
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Dropped packet.\n"));
			return ERR_OK; /* Drop packet */
#endif /* RFCOMM_FLOW_QUEUEING */      
		}
	}

	if(q != NULL) {
		tot_len = q->tot_len;
	}

	/* Size of information must be less than maximum frame size */
	if(tot_len > pcb->n) {
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Size of information must be less than maximum frame size\n"));
		return ERR_MEM;
	}

	if(tot_len < 127) {
		if((p = pbuf_alloc(PBUF_RAW, RFCOMM_UIH_LEN, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Could not allocate memory for pbuf\n"));
			return ERR_MEM; /* Could not allocate memory for pbuf */
		}
	} else {
		if((p = pbuf_alloc(PBUF_RAW, RFCOMM_UIH_LEN+1, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Could not allocate memory for pbuf\n")); 
			return ERR_MEM; /* Could not allocate memory for pbuf */
		}
	}

	/* Setup RFCOMM header */
	if(cn == 0) {
		((u8_t *)p->payload)[0] = (1 << 0) | ((pcb->rfcommcfg & RFCOMM_CFG_IR) << 1) | (0 << 2) | (0 << 3);
	} else {
		((u8_t *)p->payload)[0] = (1 << 0) | ((pcb->rfcommcfg & RFCOMM_CFG_IR) << 1) | (0 << 2) | (cn << 3);
	}
	((u8_t *)p->payload)[1] = RFCOMM_UIH;
	if(q != NULL) {
		if(q->tot_len < 127) {
			((u8_t *)p->payload)[2] = (1 << 0) | (q->tot_len << 1);
		} else {
			((u16_t *)p->payload)[1] = (0 << 0) | (q->tot_len << 1);
		}
		/* Add information data to pbuf */
		pbuf_chain(p, q);
	} else {
		((u8_t *)p->payload)[2] = (1 << 0) | (0 << 1); /* Empty UIH frame */
	}
	/* Add information FCS to pbuf */
	if((r = pbuf_alloc(PBUF_RAW, 1, PBUF_RAM)) == NULL) { 
		pbuf_free(p);
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih: Could not allocate memory for pbuf\n"));
		return ERR_MEM; /* Could not allocate memory for pbuf */
	}

	if(cn == 0) {
		((u8_t *)r->payload)[0] = pcb->uih0_out_fcs;
	} else {
		((u8_t *)r->payload)[0] = pcb->uih_out_fcs;
	}

	pbuf_chain(p, r);
	pbuf_free(r);

	ret = l2ca_datawrite(pcb->l2cappcb, p);

	/* Dealloc the RFCOMM header. Lower layers will handle rest of packet */
	pbuf_free(p);
	if(q) {
		//pbuf_dechain(p); /* Have q point to information + FCS */
		pbuf_realloc(q, q->tot_len-1); /* Remove FCS from packet */
	}

	return ret;
}

/* 
 * rfcomm_uih_credits():
 *
 * Sends a RFCOMM unnumbered information frame with header check and credit based 
 * flow control.
 */
err_t rfcomm_uih_credits(struct rfcomm_pcb *pcb, u8_t credits, struct pbuf *q)
	//RESPONDER & INITIATOR
{
	struct pbuf *p, *r;
	err_t ret;
	u16_t tot_len = 0;

	/* Decrease local credits */
	if(pcb->cl == 0xF && pcb->state == RFCOMM_OPEN && pcb->cn != 0) {
		if(pcb->k != 0) {
			--pcb->k;
		} else {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Out of local credits\n"));
#if RFCOMM_FLOW_QUEUEING
			if(q != NULL) {
				/* Packet can be queued? */
				if(pcb->buf != NULL) {
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Buffer full. Dropped packet\n"));
					return ERR_OK; /* Drop packet */
				} else {
					/* Copy PBUF_REF referenced payloads into PBUF_RAM */
					q = pbuf_take(q);
					/* Remember pbuf to queue, if any */
					pcb->buf = q;
					/* Pbufs are queued, increase the reference count */
					pbuf_ref(q);
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Queued packet %p on channel %d\n", (void *)q, pcb->cn));
				}
			}
#else
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Dropped packet\n"));
#endif /* RFCOMM_FLOW_QUEUEING */
			return ERR_OK;
		}
	}

	if(q != NULL) {
		tot_len = q->tot_len;
	}

	/* Size of information must be less than maximum frame size */
	if(tot_len > pcb->n) {
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Size of information must be less than maximum frame size = %d Packet lenght = %d\n", pcb->n, q->tot_len));
		return ERR_MEM;
	}

	if(tot_len < 127) {
		if((p = pbuf_alloc(PBUF_RAW, RFCOMM_UIHCRED_LEN, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Could not allocate memory for pbuf\n"));
			return ERR_MEM; /* Could not allocate memory for pbuf */
		}
	} else {
		if((p = pbuf_alloc(PBUF_RAW, RFCOMM_UIHCRED_LEN+1, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Could not allocate memory for pbuf\n"));
			return ERR_MEM; /* Could not allocate memory for pbuf */
		}
	}

	/* Setup RFCOMM header */
	((u8_t *)p->payload)[0] = (1 << 0) | ((pcb->rfcommcfg & RFCOMM_CFG_IR) << 1) | (0  << 2) | (pcb->cn << 3);
	((u8_t *)p->payload)[1] = RFCOMM_UIH_PF;
	if(q != NULL) {
		if(q->tot_len < 127) {
			((u8_t *)p->payload)[2] = (1 << 0) | (q->tot_len << 1);
			((u8_t *)p->payload)[3] = credits;
		} else {
			((u16_t *)p->payload)[1] = (0 << 0) | (q->tot_len << 1);
			((u8_t *)p->payload)[4] = credits;
		}
		/* Add information data to pbuf */
		pbuf_chain(p, q);
	} else {
		/* Credit only UIH frame */
		((u8_t *)p->payload)[2] = (1 << 0) | (0 << 1);
	}

	/* Add information FCS to pbuf */
	if((r = pbuf_alloc(PBUF_RAW, 1, PBUF_RAM)) == NULL) {
		pbuf_free(p);
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: Could not allocate memory for pbuf\n"));
		return ERR_MEM; /* Could not allocate memory for pbuf */
	}

	((u8_t *)r->payload)[0] = pcb->uihpf_out_fcs;
	pbuf_chain(p, r);
	pbuf_free(r);

	/* Increase remote credits */
	pcb->rk += credits;

	LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_uih_credits: p->tot_len = %d pcb->k = %d pcb->rk = %d\n", p->tot_len, pcb->k, pcb->rk));

	ret = l2ca_datawrite(pcb->l2cappcb, p);

	/* Free RFCOMM header. Higher layers will handle rest of packet */
	pbuf_free(p);
	if(q) {
		//pbuf_dechain(p);
		pbuf_realloc(q, q->tot_len-1); /* Remove FCS from packet */
	}

	return ret;
}

/* 
 * rfcomm_process_msg():
 *
 * Parses the received RFCOMM message and handles it.
 */
void rfcomm_process_msg(struct rfcomm_pcb *pcb, struct rfcomm_hdr *rfcommhdr, struct l2cap_pcb *l2cappcb, struct pbuf *p)
{
	struct rfcomm_msg_hdr *cmdhdr, *rsphdr;
	struct rfcomm_pn_msg *pnreq;
	struct rfcomm_msc_msg *mscreq;
	struct rfcomm_rpn_msg *rpnreq;
	struct rfcomm_pcb *tpcb; /* Temp pcb */
	struct rfcomm_pcb_listen *lpcb; /* Listen pcb */
	struct pbuf *q;
	err_t ret;

	cmdhdr = p->payload;
	pbuf_header(p, -RFCOMM_MSGHDR_LEN);

	switch(cmdhdr->type) {
		case RFCOMM_PN_CMD:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM PN command\n"));
			pnreq = p->payload;

			/* Check if the DLC is already established */
			tpcb = rfcomm_get_active_pcb((pnreq->dlci >> 1), &pcb->l2cappcb->remote_bdaddr);

			if(tpcb == NULL) {
				/* Check if the server channel exists */
				for(lpcb = rfcomm_listen_pcbs; lpcb != NULL; lpcb = lpcb->next) {
					if(lpcb->cn == (pnreq->dlci >> 1)) {
						break;
					}
				}
				if(lpcb != NULL) {
					/* Found a listening pcb with a matching server channel number, now initiate a new PCB 
					   with default configuration */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: Allocate RFCOMM PCB for CN %d******************************\n", lpcb->cn));
					if((tpcb = rfcomm_new(pcb->l2cappcb)) == NULL) {
						LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: could not allocate PCB\n"));
						return;
					}
					tpcb->cn = lpcb->cn;
					tpcb->callback_arg = lpcb->callback_arg;
					tpcb->accept = lpcb->accept;
					tpcb->state = RFCOMM_CFG;

					RFCOMM_REG(&rfcomm_active_pcbs, tpcb);
				} else {
					/* Channel does not exist, refuse connection with DM frame */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: Channel does not exist, refuse connection with DM frame CN %d\n", (pnreq->dlci >> 1)));
					rfcomm_dm(pcb->l2cappcb, rfcommhdr);
					break;
				}
			}
			/* Get suggested parameters */
			tpcb->cl = pnreq->i_cl >> 4;
			tpcb->p = pnreq->p;
			if(tpcb->n > pnreq->n) {
				tpcb->n = pnreq->n;
			}
			tpcb->k = pnreq->k;
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_PN_CMD. tpcb->k = %d\n", tpcb->k));

			/* Send PN response */
			cmdhdr->type = cmdhdr->type & 0xFD; /* Set C/R to response */
			if(tpcb->cl == 0xF) {
				pnreq->i_cl = 0 | (0xE << 4); /* Credit based flow control */
			} else {
				pnreq->i_cl = 0; /* Remote device conforms to bluetooth version 1.0B. No flow control */
			}
			pnreq->p = tpcb->p;
			pnreq->n = tpcb->n;
			pnreq->k = tpcb->k;
			pbuf_header(p, RFCOMM_MSGHDR_LEN);

			rfcomm_uih(pcb, 0, p);
			break;
		case RFCOMM_PN_RSP:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM PN response\n"));
			pcb->to = 0; /* Reset response timer */
			pnreq = p->payload;
			/* Find PCB with matching server channel number and bluetooth address */
			tpcb = rfcomm_get_active_pcb((pnreq->dlci >> 1), &pcb->l2cappcb->remote_bdaddr);

			if(tpcb != NULL) {
				/* Use negotiated settings that may have changed from the default ones */
				if((pnreq->i_cl >> 4) == 0xE) {
					tpcb->cl = 0xF; /* Credit based flow control */
					tpcb->k = pnreq->k==0?7:pnreq->k; /* Inital credit value */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_PN_RSP. tpcb->k = %d\n", tpcb->k));
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: Credit based flow control is used for outgoing packets 0x%x %d %d\n", (pnreq->i_cl >> 4), pnreq->k, pnreq->n));
				} else {
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: No flow control used for outgoing packets 0x%x\n", (pnreq->i_cl >> 4)));
					tpcb->cl = 0; /* Remote device conform to bluetooth version 1.0B. No flow control */
				}
				tpcb->n = pnreq->n; /* Maximum frame size */

				if(tpcb->state == W4_RFCOMM_MULTIPLEXER) {
					rfcomm_connect(tpcb, tpcb->cn, tpcb->connected); /* Create a connection for a channel that 
																		waits for the multiplexer connection to
																		be established */
				}

				pcb->state = RFCOMM_OPEN;
				RFCOMM_EVENT_PN_RSP(tpcb,ERR_OK,ret); 
			} /* else silently discard */
			break; 
		case RFCOMM_TEST_CMD:
			/* Send TEST response */
			cmdhdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
			pbuf_header(p, RFCOMM_MSGHDR_LEN);

			rfcomm_uih(pcb, 0, p);
			break;
		case RFCOMM_TEST_RSP:
			pcb->to = 0; /* Reset response timer */
			RFCOMM_EVENT_TEST(pcb,ERR_OK,ret);
			break;
		case RFCOMM_FCON_CMD:
			/* Enable transmission of data on all channels in session except cn 0 */
			for(tpcb = rfcomm_active_pcbs; tpcb != NULL; tpcb = tpcb->next) {
				if(bd_addr_cmp(&(tpcb->l2cappcb->remote_bdaddr), &(l2cappcb->remote_bdaddr)) &&
						tpcb->cn != 0) {
					tpcb->rfcommcfg |= RFCOMM_CFG_FC;
				}
			}
			/* Send FC_ON response */
			cmdhdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
			pbuf_header(p, RFCOMM_MSGHDR_LEN);

			rfcomm_uih(pcb, 0, p);
			break;
		case RFCOMM_FCON_RSP:
			break;
		case RFCOMM_FCOFF_CMD:
			/* Disable transmission of data on all channels in session except cn 0 */
			for(tpcb = rfcomm_active_pcbs; tpcb != NULL; tpcb = tpcb->next) {
				if(bd_addr_cmp(&(tpcb->l2cappcb->remote_bdaddr), &(l2cappcb->remote_bdaddr)) &&
						tpcb->cn != 0) {
					tpcb->rfcommcfg &= ~RFCOMM_CFG_FC;
				}
			}
			/* Send FC_OFF response */
			cmdhdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
			pbuf_header(p, RFCOMM_MSGHDR_LEN);

			rfcomm_uih(pcb, 0, p);
			break;
		case RFCOMM_FCOFF_RSP:
			break;
		case RFCOMM_MSC_CMD:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_MSC_CMD\n"));
			mscreq = p->payload;    
			/* Find DLC */
			tpcb = rfcomm_get_active_pcb((mscreq->dlci >> 3), &pcb->l2cappcb->remote_bdaddr);

			if(tpcb != NULL) {
				/* Set flow control bit. Ignore remaining fields in the MSC since this is a type 1 
				   device */
				if((mscreq->rs232 >> 1) & 0x01) {
					tpcb->rfcommcfg |= RFCOMM_CFG_FC;
				} else {
					tpcb->rfcommcfg &= ~RFCOMM_CFG_FC;
				}

				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcommm_process_msg: fc bit = %d\n", (mscreq->rs232 >> 1) & 0x01));

				/* Send MSC response */
				cmdhdr->type = cmdhdr->type & 0xFD; /* Set C/R to response */
				pbuf_header(p, RFCOMM_MSGHDR_LEN);

				if(!(tpcb->rfcommcfg & RFCOMM_CFG_IR) && !(tpcb->rfcommcfg & RFCOMM_CFG_MSC_IN)) { /* We are the responder and should send a MSC command before responding to one */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcommm_process_msg: We are the responder and should send a MSC command before responding to one\n"));
					rfcomm_msc(tpcb, 0, NULL);
				}

				rfcomm_uih(pcb, 0, p);

				tpcb->rfcommcfg |= RFCOMM_CFG_MSC_OUT;

				if(tpcb->rfcommcfg & RFCOMM_CFG_MSC_IN && tpcb->state != RFCOMM_OPEN) {
					tpcb->state = RFCOMM_OPEN;
					if(tpcb->rfcommcfg & RFCOMM_CFG_IR) {
						RFCOMM_EVENT_CONNECTED(tpcb,ERR_OK,ret);
					} else {
						RFCOMM_EVENT_ACCEPT(tpcb,ERR_OK,ret);
					}
				}
			} /* else silently discard */
			break;
		case RFCOMM_MSC_RSP:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_MSC_RSP\n"));
			/* Information received in response is only a copy of the signals that where sent in the command */
			pcb->to = 0; /* Reset response timer */

			mscreq = p->payload;

			/* Find PCB with matching server channel number and Bluetooth address */
			tpcb = rfcomm_get_active_pcb((mscreq->dlci >> 3), &pcb->l2cappcb->remote_bdaddr);

			if(tpcb != NULL) {

				if(tpcb->rfcommcfg & RFCOMM_CFG_MSC_IN) {
					RFCOMM_EVENT_MSC(tpcb,ERR_OK,ret); /* We have sent a MSC after initial configuration of 
														  the connection was done */
				} else {
					tpcb->rfcommcfg |= RFCOMM_CFG_MSC_IN;
					if(tpcb->rfcommcfg & RFCOMM_CFG_MSC_OUT) {
						tpcb->state = RFCOMM_OPEN;
						if(tpcb->rfcommcfg & RFCOMM_CFG_IR) {
							RFCOMM_EVENT_CONNECTED(tpcb,ERR_OK,ret);
						} else {
							RFCOMM_EVENT_ACCEPT(tpcb,ERR_OK,ret);
						}
					}
				}
			} /* else silently discard */
			break;
		case RFCOMM_RPN_CMD:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_RPN_CMD\n"));
			/* Send RPN response */
			if(cmdhdr->len == 8) {
				/* RPN command was a request to set up the link's parameters */
				/* All parameters accepted since this is a type 1 device */
				cmdhdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
				pbuf_header(p, RFCOMM_MSGHDR_LEN);
				//rfcomm_uih(pcb->l2cappcb, rfcommhdr, p);
				rfcomm_uih(pcb, 0, p);
			} else if(cmdhdr->len == 1) {
				/* RPN command was a request for the link's parameters */
				if((q = pbuf_alloc(PBUF_RAW, RFCOMM_RPNMSG_LEN+RFCOMM_MSGHDR_LEN, PBUF_RAM)) == NULL)
				{
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: Could not allocate memory at line: %d\n", __LINE__ - 1));
					break;
				}
				rsphdr = q->payload;
				rsphdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
				rsphdr->len = RFCOMM_RPNMSG_LEN;
				pbuf_header(q, -RFCOMM_MSGHDR_LEN);
				rpnreq = q->payload;
				rpnreq->dlci = ((u8_t *)p->payload)[0]; /* Copy DLCI from command to response */
				rpnreq->br = RFCOMM_COM_BR; /* Default baud rate */
				rpnreq->cfg = RFCOMM_COM_CFG; /* Default data bits, stop bits, parity and parity type */
				rpnreq->fc = RFCOMM_COM_FC; /* Default flow control */
				rpnreq->xon = RFCOMM_COM_XON; /* Default */
				rpnreq->xoff = RFCOMM_COM_XOFF; /* Default */
				rpnreq->mask = 0xFFFF; /* All parameters are valid */
				pbuf_header(q, RFCOMM_MSGHDR_LEN);

				rfcomm_uih(pcb, 0, q);
				pbuf_free(q);
			} else {
				//SHOULD NOT HAPPEN. LENGTH SHOULD ALWAYS BE 1 OR 8
			}
			break;
		case RFCOMM_RPN_RSP:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_RPN_CMD\n"));
			pcb->to = 0; /* Reset response timer */
			rpnreq = p->payload;
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_msc: rpn response received 0x%x\n", rpnreq->br));

			/* Find PCB with matching server channel number and bluetooth address */
			tpcb = rfcomm_get_active_pcb((rpnreq->dlci >> 3), &pcb->l2cappcb->remote_bdaddr);

			if(tpcb != NULL) {
				RFCOMM_EVENT_RPN(tpcb,ERR_OK,ret);
			}
			break;
		case RFCOMM_RLS_CMD:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: RFCOMM_RLS_CMD\n"));
			/* Send RLS response */
			cmdhdr->type = cmdhdr->type & 0xBF; /* Set C/R to response */
			pbuf_header(p, RFCOMM_MSGHDR_LEN);

			rfcomm_uih(pcb, 0, p);
			break;
		case RFCOMM_RLS_RSP:
			break;
		case RFCOMM_NSC_RSP:
			break;
		default:
			/* Send NSC response */
			if ((q = pbuf_alloc(PBUF_RAW, RFCOMM_MSGHDR_LEN, PBUF_RAM)) == NULL)
			{
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_process_msg: Could not allocate memory at line: %d\n", __LINE__ - 1));
				break;
			}
			rsphdr = q->payload; 
			rsphdr->type = ((cmdhdr->type & 0x03) << 0) | (RFCOMM_NSC_RSP << 2);
			rsphdr->len = 0;

			rfcomm_uih(pcb, 0, q);
			pbuf_free(q);
			break;
	}
}

/* 
 * rfcomm_input():
 *
 * Called by the lower layer. Does a frame check, parses the header and forward it to 
 * the upper layer or handle the command frame.
 */
err_t rfcomm_input(void *arg, struct l2cap_pcb *l2cappcb, struct pbuf *p, err_t err)
{
	struct rfcomm_hdr rfcommhdr;
	struct rfcomm_pcb *pcb, *tpcb;
	struct rfcomm_pcb_listen *lpcb;

	s16_t len = 0;
	u8_t hdrlen;
	u8_t fcs;

	struct pbuf *q;
	err_t ret;

	rfcommhdr.addr = *((u8_t *)p->payload);
	rfcommhdr.ctrl = ((u8_t *)p->payload)[1];

	/* Find PCB with matching server channel number and bluetooth address */
	pcb = rfcomm_get_active_pcb((rfcommhdr.addr >> 3), &l2cappcb->remote_bdaddr);

	if(pcb == NULL && rfcommhdr.ctrl != RFCOMM_SABM) {
		/* Channel does not exist */
		if(rfcommhdr.ctrl != RFCOMM_DM_PF && rfcommhdr.ctrl != RFCOMM_DM) {
			/* Send a DM response */
			LWIP_DEBUGF(RFCOMM_DEBUG,("Send a DM response to CN %d rfcomm.ctrl == 0x%x\n", (rfcommhdr.addr >> 3), rfcommhdr.ctrl));
			rfcomm_dm(l2cappcb, &rfcommhdr);
		} /* else silently discard packet */ 
		pbuf_free(p);
		return ERR_OK;
	}

	/* Check if length field is 1 or 2 bytes long and remove EA bit */
	if((((u8_t *)p->payload)[2] & 0x01) == 1) {
		hdrlen = RFCOMM_HDR_LEN_1;
		rfcommhdr.len = (((u8_t *)p->payload)[2] >> 1) & 0x007F;
	} else {
		hdrlen = RFCOMM_HDR_LEN_2;
		rfcommhdr.len = (((u16_t *)p->payload)[1] >> 1) & 0x7FFF;
	}

	if(rfcommhdr.ctrl == RFCOMM_UIH_PF) {
		if(pcb->cl == 0xF) {
			rfcommhdr.k = ((u8_t *)p->payload)[hdrlen++];
		}
	}

	/* Frame check */
	for(q = p; q != NULL; q = q->next) {
		len += q->len;
		if(len > (rfcommhdr.len + hdrlen)) {
			len -= q->len;
			len = rfcommhdr.len - len;
			len += hdrlen;
			break;
		}
	}

	fcs = ((u8_t *)q->payload)[len];
	if(rfcommhdr.ctrl == RFCOMM_UIH) {
		if(pcb->cn == 0) {
			if(fcs != pcb->uih0_in_fcs) { /* Check against the precalculated fcs */
				/*if(fcs8_crc_check(p, RFCOMM_UIHCRC_CHECK_LEN, fcs) != 0)  */
				/* Packet discarded due to failing frame check sequence */
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: UIH packet discarded due to failing frame check sequence\n"));
				pbuf_free(p);
				return ERR_OK;
			}
		}
	} else if(rfcommhdr.ctrl == RFCOMM_UIH) {
		if(fcs != pcb->uih_in_fcs) { /* Check against the precalculated fcs */
			/*if(fcs8_crc_check(p, RFCOMM_UIHCRC_CHECK_LEN, fcs) != 0)  */
			/* Packet discarded due to failing frame check sequence */
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: UIH packet discarded due to failing frame check sequence\n"));
			pbuf_free(p);
			return ERR_OK;
		}
	} else if(rfcommhdr.ctrl == RFCOMM_UIH_PF) {
		if(fcs != pcb->uihpf_in_fcs) { /* Check against the precalculated fcs */
			/*if(fcs8_crc_check(p, RFCOMM_UIHCRC_CHECK_LEN, fcs) != 0)  */
			/* Packet discarded due to failing frame check sequence */
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: UIH_PF packet discarded due to failing frame check sequence RFCS = 0x%x LFCS = 0x%x\n", 
						fcs, pcb->uihpf_in_fcs));
			pbuf_free(p);
			return ERR_OK;
		}
	} else {
		if(fcs8_crc_check(p, hdrlen, fcs) != 0) {
			/* Packet discarded due to failing frame check sequence */
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Packet discarded due to failing frame check sequence\n"));
			pbuf_free(p);
			return ERR_OK;
		}
	}

	pbuf_header(p, -hdrlen); /* Adjust information pointer */
	pbuf_realloc(p, rfcommhdr.len); /* Remove fcs from packet */

	switch(rfcommhdr.ctrl) {
		case RFCOMM_SABM:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: RFCOMM_SABM\n"));
			if(pcb == NULL) {
				/* Check if the server channel exists */
				lpcb = NULL;
				if(rfcommhdr.addr >> 3 == 0) { /* Only the multiplexer channel can be connected without first 
												  configuring it */
					for(lpcb = rfcomm_listen_pcbs; lpcb != NULL; lpcb = lpcb->next) {
						if(lpcb->cn == (rfcommhdr.addr >> 3)) {
							break;
						}
					}
				}
				if(lpcb != NULL) {
					/* Found a listening pcb with a matching server channel number, now initiate a
					 * new active PCB with default configuration */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Allocate RFCOMM PCB for CN %d******************************\n", lpcb->cn));
					if((pcb = rfcomm_new(l2cappcb)) == NULL) {
						/* No memory to allocate PCB. Refuse connection attempt */
						LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: No memory to allocate PCB. Refuse connection attempt CN %d******************************\n", lpcb->cn));
						rfcomm_dm(l2cappcb, &rfcommhdr);
						pbuf_free(p);
						return ERR_OK;
					}
					pcb->cn = lpcb->cn;
					pcb->callback_arg = lpcb->callback_arg;
					pcb->accept = lpcb->accept;

					RFCOMM_REG(&rfcomm_active_pcbs, pcb);
				} else {
					/* Channel does not exist or multiplexer is not connected, refuse connection with DM frame */
					LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Channel does not exist, refuse connection with DM frame CN %d\n", (rfcommhdr.addr >> 3)));
					rfcomm_dm(l2cappcb, &rfcommhdr);
					pbuf_free(p);
					break;
				}
			}
			/* Set role to responder */
			pcb->rfcommcfg &= ~RFCOMM_CFG_IR;

			/* Send UA frame as response to SABM frame */
			rfcomm_ua(l2cappcb, &rfcommhdr);

			/* FCS precalculation for UIH frames */
			pbuf_header(p, hdrlen); /* Reuse the buffer for the current header */

			/* Change header values to refelct an UIH frame sent to the initiator */
			*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0. We are the responder */
			*((u8_t *)p->payload) &= 0xFD; /* Set C/R bit to 0. We are the responder */
			((u8_t *)p->payload)[1] = RFCOMM_UIH;
			pcb->uih_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);
			((u8_t *)p->payload)[1] = RFCOMM_UIH_PF;
			pcb->uihpf_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

			/* Change header values to refelct an UIH frame from to the initiator */
			//*((u8_t *)p->payload) |= 0x04; /* Set direction bit to 1. We are the responder */
			*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0. We are the responder */
			*((u8_t *)p->payload) |= 0x02; /* Set C/R bit to 1. We are the responder */
			((u8_t *)p->payload)[1] = RFCOMM_UIH;
			pcb->uih_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);
			((u8_t *)p->payload)[1] = RFCOMM_UIH_PF;
			pcb->uihpf_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

			/* UIH frame received on the control channel */
			*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0 */
			((u8_t *)p->payload)[1] = RFCOMM_UIH;
			pcb->uih0_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

			/* Change header values to reflect an UIH frame sent on the control channel */
			*((u8_t *)p->payload) &= 0xF9; /* Set C/R bit and direction bit to 0 */
			((u8_t *)p->payload)[1] = RFCOMM_UIH;
			pcb->uih0_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

			pbuf_free(p);
			break;
		case RFCOMM_UA:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: RFCOMM_UA\n"));
			pcb->to = 0;
			if(pcb->state ==  W4_RFCOMM_SABM_RSP) {
				pcb->state = RFCOMM_CFG;
				/* FCS precalculation for UIH frames */
				pbuf_header(p, hdrlen); /* Reuse the buffer for the current header */
				/* Change header values to refelct an UIH frame sent to the responder */
				*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0. We are the initiator */
				*((u8_t *)p->payload) |= 0x02; /* Set C/R bit to 1. We are the intitiator */
				((u8_t *)p->payload)[1] = RFCOMM_UIH;
				pcb->uih_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);
				((u8_t *)p->payload)[1]= RFCOMM_UIH_PF;
				pcb->uihpf_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

				/* Change header values to reflect an UIH frame sent to the responder */
				*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0. We are the intitiator */
				*((u8_t *)p->payload) &= 0xFD; /* Set C/R bit to 0. We are the initiator */
				((u8_t *)p->payload)[1] = RFCOMM_UIH;
				pcb->uih_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);
				((u8_t *)p->payload)[1] = RFCOMM_UIH_PF;
				pcb->uihpf_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

				/* UIH frame sent on the control channel */
				*((u8_t *)p->payload) &= 0xFB; /* Set direction bit to 0 */
				*((u8_t *)p->payload) |= 0x02; /* Set C/R bit to 1. We are the intitiator */
				((u8_t *)p->payload)[1] = RFCOMM_UIH;
				pcb->uih0_out_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

				/* Change header values to reflect an UIH frame received on the control channel */
				*((u8_t *)p->payload) &= 0xF9; /* Set C/R bit and direction bit to 0 */
				((u8_t *)p->payload)[1] = RFCOMM_UIH;
				pcb->uih0_in_fcs = fcs8_crc_calc(p, RFCOMM_UIHCRC_CHECK_LEN);

				if(pcb->cn == 0) {	
					for(tpcb = rfcomm_active_pcbs; tpcb != NULL; tpcb = tpcb->next) {
						if(bd_addr_cmp(&(tpcb->l2cappcb->remote_bdaddr), &(pcb->l2cappcb->remote_bdaddr)) &&
								tpcb->state == W4_RFCOMM_MULTIPLEXER) {
							rfcomm_pn(tpcb, NULL); /* Send a parameter negotiation command to negotiate the 
													  connection settings for a channel that waits for the
													  multiplexer connection to be established */
							break;
						}
					}
				} else {
					rfcomm_msc(pcb, 0, NULL); /* Send a modem status command to set V.24 control signals for
												 the RFCOMM connection */
				}
			} else if (pcb->state == W4_RFCOMM_DISC_RSP) {
				//RFCOMM_RMV(&rfcomm_active_pcbs, pcb);
				pcb->state = RFCOMM_CLOSED;
				RFCOMM_EVENT_DISCONNECTED(pcb,ERR_OK,ret);
			} else {
				/* A response without an outstanding request is silently discarded */
			}
			pbuf_free(p);
			break;
		case RFCOMM_DM_PF:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: RFCOMM_DM_PF\n"));
		case RFCOMM_DM:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: RFCOMM_DM\n"));
			pcb->to = 0;
			//RFCOMM_RMV(&rfcomm_active_pcbs, pcb);
			if(pcb->state ==  W4_RFCOMM_SABM_RSP) {
				pcb->state = RFCOMM_CLOSED;
				RFCOMM_EVENT_CONNECTED(pcb,ERR_CONN,ret);
			} else {
				pcb->state = RFCOMM_CLOSED;
				RFCOMM_EVENT_DISCONNECTED(pcb,ERR_CONN,ret);
			}
			pbuf_free(p);
			break;
		case RFCOMM_DISC:
			LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: RFCOMM_DISC\n"));
			//RFCOMM_RMV(&rfcomm_active_pcbs, pcb);
			/* Send UA frame as response to DISC frame */
			ret = rfcomm_ua(l2cappcb, &rfcommhdr);
			pcb->state = RFCOMM_CLOSED;
			RFCOMM_EVENT_DISCONNECTED(pcb,ERR_OK,ret);
			pbuf_free(p);
			break;
		case RFCOMM_UIH_PF: 
			if((rfcommhdr.addr >> 3) == 0) {
				/* Process multiplexer command/response */
				rfcomm_process_msg(pcb, &rfcommhdr, l2cappcb, p);
				pbuf_free(p);
			} else if(pcb->cl == 0xF) {
				/* Process credit based frame */
				if(pcb->rk != 0) {
					--pcb->rk; /* Decrease remote credits */
				}
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Received local credits: %d Existing local credits: %d\n", rfcommhdr.k, pcb->k));
				if((pcb->k + rfcommhdr.k) < 255) {
					pcb->k += rfcommhdr.k; /* Increase local credits */
#if RFCOMM_FLOW_QUEUEING
					q = pcb->buf;
					/* Queued packet present? */
					if (q != NULL) {
						/* NULL attached buffer immediately */
						pcb->buf = NULL;
						LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: sending queued packet.\n"));
						/* Send the queued packet */
						rfcomm_uih(pcb, pcb->cn, q); 
						/* Free the queued packet */
						pbuf_free(q);
					}
#endif /* RFCOMM_FLOW_QUEUEING */	
				} else {
					pcb->k = 255;
				}
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Forward RFCOMM_UIH_PF credit packet to higher layer\n"));
				RFCOMM_EVENT_RECV(pcb,ERR_OK,p,ret); /* Process information. Application must free pbuf */
			} else {
				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Forward RFCOMM_UIH_PF non credit packet to higher layer\n"));
				RFCOMM_EVENT_RECV(pcb,ERR_OK,p,ret); /* Process information. Application must free pbuf */
			}
			break;
		case RFCOMM_UIH:
			if((rfcommhdr.addr >> 3) == 0) {
				/* Process multiplexer command/response */
				rfcomm_process_msg(pcb, &rfcommhdr, l2cappcb, p);
				pbuf_free(p);
			} else {
				if(pcb->rk != 0) {
					--pcb->rk; /* Decrease remote credits */
				}

				LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Forward RFCOMM_UIH packet to higher layer\n"));
				RFCOMM_EVENT_RECV(pcb,ERR_OK,p,ret); /* Process information. Application must free pbuf */
			}
			break;
		default:
			/* Unknown or illegal frame type. Throw it away! */
			pbuf_free(p);
			break;
	}
	return ERR_OK;
}

/* 
 * rfcomm_arg():
 *
 * Used to specify the argument that should be passed callback functions.
 */
void rfcomm_arg(struct rfcomm_pcb *pcb, void *arg)
{
	pcb->callback_arg = arg;
}

/* 
 * rfcomm_recv():
 * 
 * Used to specify the function that should be called when a RFCOMM connection 
 * receives data.
 */
void rfcomm_recv(struct rfcomm_pcb *pcb, 
		err_t (* recv)(void *arg, struct rfcomm_pcb *pcb, struct pbuf *p, err_t err))
{
	pcb->recv = recv;
}

/* 
 * rfcomm_disc():
 * 
 * Used to specify the function that should be called when a RFCOMM channel is 
 * disconnected
 */
void rfcomm_disc(struct rfcomm_pcb *pcb,
		err_t (* disc)(void *arg, struct rfcomm_pcb *pcb, err_t err))
{
	pcb->disconnected = disc;
}

/* 
 * rfcomm_listen():
 * 
 * Set the state of the connection to be LISTEN, which means that it is
 * able to accept incoming connections. The protocol control block is
 * reallocated in order to consume less memory. Setting the connection to
 * LISTEN is an irreversible process. Also specify the function that should
 * be called when the channel has been connected.
 */
err_t rfcomm_listen(struct rfcomm_pcb *npcb, u8_t cn, 
		err_t (* accept)(void *arg, struct rfcomm_pcb *pcb, err_t err))
{
	struct rfcomm_pcb_listen *lpcb;

	if((lpcb = lwbt_memp_malloc(MEMP_RFCOMM_PCB_LISTEN)) == NULL) {
		LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_listen: Could not allocate memory for pcb\n"));
		return ERR_MEM;
	}
	lpcb->cn = cn;
	lpcb->callback_arg = npcb->callback_arg;
	lpcb->accept = accept;
	lpcb->state = RFCOMM_LISTEN;

	lwbt_memp_free(MEMP_RFCOMM_PCB, npcb);
	RFCOMM_REG((struct rfcomm_pcb **)&rfcomm_listen_pcbs, (struct rfcomm_pcb *)lpcb);
	return ERR_OK;
}
