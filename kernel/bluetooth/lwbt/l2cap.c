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


/* l2cap.c
 *
 * Implementation of the logical link control and adaption protocol (L2CAP). Supports 
 * higher level protocol multiplexing, packet segmentation and reassembly, and the 
 * conveying of quality of service information.
 */


#include "lwbt/l2cap.h"
#include "lwbt/lwbt_memp.h"
#include "lwbt/lwbtopts.h"
#include "lwip/debug.h"

/* Next Identifier to be sent */
u8_t sigid_nxt;

/* The L2CAP PCB lists. */
struct l2cap_pcb_listen *l2cap_listen_pcbs;  /* List of all L2CAP PCBs in CLOSED state
												but awaiting an incoming conn req */
struct l2cap_pcb *l2cap_active_pcbs;  /* List of all L2CAP PCBs that are in a
										 state in which they accept or send
										 data */
struct l2cap_pcb *l2cap_tmp_pcb;

/* Temp signal */
struct l2cap_sig *l2cap_tmp_sig;

/* Global variable involved in input processing of l2cap data segements */
struct l2cap_seg *l2cap_insegs;
struct l2cap_seg *l2cap_tmp_inseg;

/* Forward declarations */
static u16_t l2cap_cid_alloc(void);


/* 
 * l2cap_init():
 * 
 * Initializes the L2CAP layer.
 */

	void
l2cap_init(void)
{
	/* Clear globals */
	l2cap_listen_pcbs = NULL;
	l2cap_active_pcbs = NULL;
	l2cap_tmp_pcb = NULL;
	l2cap_tmp_sig = NULL;
	l2cap_insegs = NULL;
	l2cap_tmp_inseg = NULL;

	/* Initialize the signal identifier (0x00 shall never be used) */
	sigid_nxt = 0x00;
}

/*
 * l2cap_tmr():
 *
 * Called every 1s and implements the retransmission timer that
 * removes a channel if it has been waiting for a request enough
 * time. It also includes a configuration timer.
 */

	void
l2cap_tmr(void)
{
	struct l2cap_sig *sig;
	struct l2cap_pcb *pcb;
	err_t ret; 

	/* Step through all of the active pcbs */
	for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
		/* Step through any unresponded signals */
		for(sig = pcb->unrsp_sigs; sig != NULL; sig = sig->next) {
			/* Check if channel is not reliable */
			if(pcb->cfg.outflushto < 0xFFFF) {
				/* Check if rtx is active. Otherwise ertx is active */
				if(sig->rtx > 0) {
					/* Adjust rtx timer */
					--sig->rtx;
					/* Check if rtx has expired */
					if(sig->rtx == 0) {
						if(sig->nrtx == 0) {
							/* Move pcb to closed state */
							pcb->state = L2CAP_CLOSED;
							/* Indicate disconnect to upper layer */
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_tmr: Max number of retransmissions (rtx) has expired\n"));
							L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
						} else {
							--sig->nrtx;
							/* Indicate timeout to upper layer */
							L2CA_ACTION_TO_IND(pcb,ERR_OK,ret);
							/* Retransmitt signal w timeout doubled */
							sig->rtx += sig->rtx;
							ret = l2cap_rexmit_signal(pcb, sig);
						}
					} /* if */
				} else {
					/* Adjust ertx timer */
					--sig->ertx;
					/* Check if ertx has expired */
					if(sig->ertx == 0) {
						if(sig->nrtx == 0) {
							/* Move pcb to closed state */
							pcb->state = L2CAP_CLOSED;
							/* Indicate disconnect to upper layer */
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_tmr: Max number of retransmissions (ertx) has expired\n"));
							L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
						} else {
							--sig->nrtx;
							/* Indicate timeout to upper layer */
							L2CA_ACTION_TO_IND(pcb,ERR_OK,ret);
							/* Disable ertx, activate rtx and retransmitt signal */
							sig->ertx = 0;
							sig->rtx = L2CAP_RTX;
							ret = l2cap_rexmit_signal(pcb, sig);
						}
					} /* if */
				} /* else */
			} /* if */
		} /* for */

		/* Check configuration timer */
		if(pcb->state == L2CAP_CONFIG) {
			/* Check if configuration timer is active */
			if(pcb->cfg.cfgto > 0) {
				--pcb->cfg.cfgto;
				LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_tmr: Configuration timer = %d\n", pcb->cfg.cfgto));
				/* Check if config timer has expired */
				if(pcb->cfg.cfgto == 0) {
					/* Connection attempt failed. Disconnect */
					l2ca_disconnect_req(pcb, NULL);
					/* Notify the application that the connection attempt failed */
					if(pcb->cfg.l2capcfg & L2CAP_CFG_IR) {
						L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_CFG_TO, 0x0000, ret);
					} else {
						L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
					}
					pcb->cfg.cfgto = L2CAP_CFG_TO; /* Reset timer */
				}
			}
		}
	} /* for */
}

/* 
 * l2cap_write():
 * 
 * Output L2CAP data to the lower layers. Segments the packet in to PDUs.
 */

	err_t
l2cap_write(struct bd_addr *bdaddr, struct pbuf *p, u16_t len) 
{
	u8_t pb = L2CAP_ACL_START;
	u16_t maxsize;
	u16_t outsize;
	err_t ret = ERR_OK;
	struct pbuf *q;
	u16_t i = 0;

	/*u16_t i;
	  struct pbuf *q;
	  for(q = p; q != NULL; q = q->next) {
	  for(i = 0; i < q->len; ++i) {
	  LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: 0x%x\n", ((u8_t *)q->payload)[i]));
	  }
	  LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: *\n"));
	  }
	  */

	maxsize = lp_pdu_maxsize();
	q = p;

	while(len && ret == ERR_OK) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: len %d maxsize %d p->len %d\n", len, maxsize, p->len));
		if(len > maxsize) {
			ret = lp_acl_write(bdaddr, q, maxsize, pb);
			len -= maxsize;
			outsize = maxsize;
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: Outsize before %d\n", outsize));
			while(q->len < outsize) {
				outsize -= q->len;
				q = q->next;
			}
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: Outsize after %d\n", outsize));
			if(outsize) {
				pbuf_header(q, -outsize);
				i += outsize;
			}
			pb = L2CAP_ACL_CONT;
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: FRAG\n"));
		} else {
			ret = lp_acl_write(bdaddr, q, len, pb);
			len = 0;
		}
	}
	pbuf_header(q, i);
	LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_write: DONE\n"));
	return ret;
}

/* 
 * l2cap_process_sig():
 * 
 * Parses the received message handles it.
 */

	void
l2cap_process_sig(struct pbuf *q, struct l2cap_hdr *l2caphdr, struct bd_addr *bdaddr)
{
	struct l2cap_sig_hdr *sighdr;
	struct l2cap_sig *sig = NULL;
	struct l2cap_pcb *pcb = NULL;
	struct l2cap_pcb_listen *lpcb;
	struct l2cap_cfgopt_hdr *opthdr;
	u16_t result, status, flags, psm, dcid, scid;
	u16_t len; 
	u16_t siglen;
	struct pbuf *p, *r = NULL, *s = NULL, *data;
	err_t ret;
	u8_t i;
	u16_t rspstate = L2CAP_CFG_SUCCESS;

	if(q->len != q->tot_len) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Fragmented packet received. Reassemble into one buffer\n"));
		if((p = pbuf_alloc(PBUF_RAW, q->tot_len, PBUF_RAM)) != NULL) {
			i = 0;
			for(r = q; r != NULL; r = r->next) {
				memcpy(((u8_t *)p->payload) + i, r->payload, r->len);
				i += r->len;
			}
		} else {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Could not allocate buffer for fragmented packet\n"));
			return; 
		}
	} else {
		p = q;
	}

	len = l2caphdr->len;

	while(len > 0) {
		/* Set up signal header */
		sighdr = p->payload;
		pbuf_header(p, -L2CAP_SIGHDR_LEN);

		/* Check if this is a response/reject signal, and if so, find the matching request */
		if(sighdr->code % 2) { /* if odd this is a resp/rej signal */
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Response/reject signal received id = %d code = %d\n", 
						sighdr->id, sighdr->code));
			for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
				for(sig = pcb->unrsp_sigs; sig != NULL; sig = sig->next) {
					if(sig->sigid == sighdr->id) {
						break; /* found */
					} 
				}
				if(sig != NULL) {
					break;
				}
			}
		} else {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Request signal received id = %d code = %d\n", 
						sighdr->id, sighdr->code));
		}

		/* Reject packet if length exceeds MTU */
		if(l2caphdr->len > L2CAP_MTU) {		      
			/* Alloc size of reason in cmd rej + MTU */
			if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+2, PBUF_RAM)) != NULL) {
				((u16_t *)data->payload)[0] = L2CAP_MTU_EXCEEDED;
				((u16_t *)data->payload)[1] = L2CAP_MTU;

				l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
			}
			break;
		}

		switch(sighdr->code) {
			case L2CAP_CMD_REJ:
				/* Remove signal from unresponded list and deallocate it */
				L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
				pbuf_free(sig->p);
				lwbt_memp_free(MEMP_L2CAP_SIG, sig);
				LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Our command was rejected so we disconnect\n")); 
				l2ca_disconnect_req(pcb, NULL);
				break;
			case L2CAP_CONN_REQ:
				psm = ((u16_t *)p->payload)[0];
				/* Search for a listening pcb */
				for(lpcb = l2cap_listen_pcbs; lpcb != NULL; lpcb = lpcb->next) {
					if(lpcb->psm == psm) {
						/* Found a listening pcb with the correct PSM */
						break;
					}
				}
				/* If no matching pcb was found, send a connection rsp neg (PSM) */
				if(lpcb == NULL) {
					/* Alloc size of data in conn rsp signal */
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM)) != NULL) {
						((u16_t *)data->payload)[0] = L2CAP_CONN_REF_PSM;
						((u16_t *)data->payload)[1] = 0; /* No further info available */
						ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);
					}
				} else {
					/* Initiate a new active pcb */
					pcb = l2cap_new();
					if(pcb == NULL) {
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: could not allocate PCB\n"));
						/* Send a connection rsp neg (no resources available) and alloc size of data in conn rsp 
						   signal */
						if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM)) != NULL) {
							((u16_t *)data->payload)[0] = L2CAP_CONN_REF_RES;
							((u16_t *)data->payload)[1] = 0; /* No further info available */
							ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);
						}
					}
					bd_addr_set(&(pcb->remote_bdaddr),bdaddr);

					pcb->scid = l2cap_cid_alloc();
					pcb->dcid = ((u16_t *)p->payload)[1];
					pcb->psm = psm;
					pcb->callback_arg = lpcb->callback_arg;
					pcb->l2ca_connect_ind = lpcb->l2ca_connect_ind;

					pcb->state = L2CAP_CONFIG;
					L2CAP_REG(&l2cap_active_pcbs, pcb);

					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: A connection request was received. Send a response\n"));
					data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_RSP_SIZE, PBUF_RAM);
					if(data == NULL) {
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_connect_rsp: Could not allocate memory for pbuf\n"));
						break;
					}
					((u16_t *)data->payload)[0] = pcb->scid;
					((u16_t *)data->payload)[1] = pcb->dcid;
					((u16_t *)data->payload)[2] = L2CAP_CONN_SUCCESS;
					((u16_t *)data->payload)[3] = 0x0000; /* No further information available */

					/* Send the response */
					ret = l2cap_signal(pcb, L2CAP_CONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);
				}
				break;
			case L2CAP_CONN_RSP:
				if(pcb == NULL) {
					/* A response without a matching request is silently discarded */
					break;
				}
				LWIP_ASSERT("l2cap_process_sig: conn rsp, active pcb->state == W4_L2CAP_CONNECT_RSP\n",
						pcb->state == W4_L2CAP_CONNECT_RSP);
				result = ((u16_t *)p->payload)[2];
				status = ((u16_t *)p->payload)[3];
				switch(result) {
					case L2CAP_CONN_SUCCESS:
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Conn_rsp_sucess, status %d\n", status));

						LWIP_ASSERT("l2cap_process_sig: conn rsp success, pcb->scid == ((u16_t *)p->payload)[1]\n",
								pcb->scid == ((u16_t *)p->payload)[1]);

						/* Set destination connection id */
						pcb->dcid = ((u16_t *)p->payload)[0];

						/* Remove signal from unresponded list and deallocate it */
						L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
						pbuf_free(sig->p);
						lwbt_memp_free(MEMP_L2CAP_SIG, sig);

						/* Configure connection */
						pcb->state = L2CAP_CONFIG;

						/* If initiator send a configuration request */
						if(pcb->cfg.l2capcfg & L2CAP_CFG_IR) {
							l2ca_config_req(pcb);
							pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_REQ;
						}
						break;
					case L2CAP_CONN_PND:
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Conn_rsp_pnd, status %d\n", status));

						/* Disable rtx and enable ertx */
						sig->rtx = 0;
						sig->ertx = L2CAP_ERTX;
						break;
					default:
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Conn_rsp_neg, result %d\n", result));
						/* Remove signal from unresponded list and deallocate it */
						L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
						pbuf_free(sig->p);
						lwbt_memp_free(MEMP_L2CAP_SIG, sig);

						L2CA_ACTION_CONN_CFM(pcb,result,status,ret);
						break;
				}
				break;
			case L2CAP_CFG_REQ:
				siglen = sighdr->len;
				dcid = ((u16_t *)p->payload)[0];
				flags = ((u16_t *)p->payload)[1];
				siglen -= 4;
				pbuf_header(p, -4);


				LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Congfiguration request, flags = %d\n", flags));

				/* Find PCB with matching cid */
				for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: dcid = 0x%x, pcb->scid = 0x%x, pcb->dcid = 0x%x\n\n", dcid, pcb->scid, pcb->dcid));
					if(pcb->scid == dcid) {
						/* Matching cid found */
						break;
					}
				}
				/* If no matching cid was found, send a cmd reject (Invalid cid) */
				if(pcb == NULL) {
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Cfg req: no matching cid was found\n"));
					/* Alloc size of reason in cmd rej + data (dcid + scid) */
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL) {
						((u16_t *)data->payload)[0] = L2CAP_INVALID_CID;
						((u16_t *)data->payload)[1] = dcid; /* Requested local cid */
						((u16_t *)data->payload)[2] = L2CAP_NULL_CID; /* Remote cid not known */

						ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
					}
				} else { /* Handle config request */
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Handle configuration request\n"));
					pcb->ursp_id = sighdr->id; /* Set id of request to respond to */

					/* Parse options and add to pcb */
					while(siglen > 0) {
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Siglen = %d\n", siglen));
						opthdr = p->payload;
						/* Check if type of action bit indicates a non-hint. Hints are ignored */
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Type of action bit = %d\n", L2CAP_OPTH_TOA(opthdr)));
						if(L2CAP_OPTH_TOA(opthdr) == 0) {
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Type = %d\n", L2CAP_OPTH_TYPE(opthdr)));
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Length = %d\n", opthdr->len));
							switch(L2CAP_OPTH_TYPE(opthdr)) {
								case L2CAP_CFG_MTU:
									LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Out MTU = %d\n", ((u16_t *)p->payload)[1]));
									pcb->cfg.outmtu = ((u16_t *)p->payload)[1];
									break;
								case L2CAP_FLUSHTO:
									LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: In flush timeout = %d\n", ((u16_t *)p->payload)[1]));
									pcb->cfg.influshto = ((u16_t *)p->payload)[1];
									break;
								case L2CAP_QOS:
									/* If service type is Best Effort or No Traffic the remainder fields will be ignored */
									if(((u8_t *)p->payload)[3] == L2CAP_QOS_GUARANTEED) {
										LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: This implementation does not support the guaranteed QOS service type"));
										if(rspstate == L2CAP_CFG_SUCCESS) {
											rspstate = L2CAP_CFG_UNACCEPT;
											if(pcb->cfg.opt != NULL) {
												pbuf_free(pcb->cfg.opt);
												pcb->cfg.opt = NULL;
											}
										}
										s = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + opthdr->len, PBUF_RAM);
										memcpy((u8_t *)s->payload, (u8_t *)p->payload, L2CAP_CFGOPTHDR_LEN + opthdr->len);
										if(pcb->cfg.opt == NULL) {
											pcb->cfg.opt = s;
										} else {
											pbuf_chain(pcb->cfg.opt, s);
											pbuf_free(s);
										}
									}
									break;
								default:
									if(rspstate != L2CAP_CFG_REJ) {
										/* Unknown option. Add to unknown option type buffer */
										if(rspstate != L2CAP_CFG_UNKNOWN) {
											rspstate = L2CAP_CFG_UNKNOWN;
											if(pcb->cfg.opt != NULL) {
												pbuf_free(pcb->cfg.opt);
												pcb->cfg.opt = NULL;
											}
										}
										s = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + opthdr->len, PBUF_RAM);
										memcpy((u8_t *)s->payload, (u8_t *)p->payload, L2CAP_CFGOPTHDR_LEN + opthdr->len);
										if(pcb->cfg.opt == NULL) {
											pcb->cfg.opt = s;
										} else {
											pbuf_chain(pcb->cfg.opt, s);
											pbuf_free(s);
										}
									}
									break; 
							} /* switch */
						} /* if(L2CAP_OPTH_TOA(opthdr) == 0) */
						pbuf_header(p, -(L2CAP_CFGOPTHDR_LEN + opthdr->len));
						siglen -= L2CAP_CFGOPTHDR_LEN + opthdr->len;
					} /* while */

					/* If continuation flag is set we don't send the final response just yet */
					if((flags & 0x0001) == 1) {
						/* Send success result with no options until the full request has been received */
						if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_RSP_SIZE, PBUF_RAM)) == NULL) {
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Could not allocate memory for pbuf\n"));
							break;
						}
						((u16_t *)data->payload)[0] = pcb->dcid;
						((u16_t *)data->payload)[1] = 0;
						((u16_t *)data->payload)[2] = L2CAP_CFG_SUCCESS;
						ret = l2cap_signal(pcb, L2CAP_CFG_RSP, pcb->ursp_id, &(pcb->remote_bdaddr), data);
						break;
					}

					/* Send a configure request for outgoing link if it hasnt been configured */
					if(!(pcb->cfg.l2capcfg & L2CAP_CFG_IR) && !(pcb->cfg.l2capcfg & L2CAP_CFG_OUT_REQ)) {
						l2ca_config_req(pcb);
						pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_REQ;
					}

					/* Send response to configuration request */
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Send response to configuration request\n"));
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_RSP_SIZE, PBUF_RAM)) != NULL) {
						((u16_t *)data->payload)[0] = pcb->dcid;
						((u16_t *)data->payload)[1] = 0; /* Flags (No continuation) */
						((u16_t *)data->payload)[2] = rspstate; /* Result */
						if(pcb->cfg.opt != NULL) {
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: pcb->cfg.opt->len = %d\n", pcb->cfg.opt->len));
							pbuf_chain(data, pcb->cfg.opt); /* Add option type buffer to data buffer */
							pbuf_free(pcb->cfg.opt);
							pcb->cfg.opt = NULL;
						}
						ret = l2cap_signal(pcb, L2CAP_CFG_RSP, pcb->ursp_id, &(pcb->remote_bdaddr), data);
					}

					if(rspstate == L2CAP_CFG_SUCCESS) {
						pcb->cfg.l2capcfg |= L2CAP_CFG_OUT_SUCCESS;
						/* L2CAP connection established if a successful configuration response has been sent */
						if(pcb->cfg.l2capcfg & L2CAP_CFG_IN_SUCCESS) {
							/* IPCP connection established, notify upper layer that connection is open */
							pcb->state = L2CAP_OPEN;
							if(pcb->cfg.l2capcfg & L2CAP_CFG_IR) {
								L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_SUCCESS, 0x0000, ret);
							} else {
								L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
							}
						}
					}
				} /* else */
				break;
			case L2CAP_CFG_RSP:
				if(pcb == NULL) {
					/* A response without a matching request is silently discarded */
					break;
				}

				/* Remove signal from unresponded list and deallocate it */
				L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
				pbuf_free(sig->p);
				lwbt_memp_free(MEMP_L2CAP_SIG, sig);

				LWIP_ASSERT(("l2cap_process_sig: cfg rsp, active pcb->state == L2CAP_CONFIG\n"),
						pcb->state == L2CAP_CONFIG);

				siglen = sighdr->len;
				scid = ((u16_t *)p->payload)[0];
				flags = ((u16_t *)p->payload)[1];
				result = ((u16_t *)p->payload)[2];
				siglen -= 6;
				pbuf_header(p, -6);

				LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Outgoing configuration result == %d continuation flag == %d\n", result, flags));

				/* Handle config request */
				switch(result) {
					case L2CAP_CFG_SUCCESS:
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Successfull outgoing configuration\n"));
						pcb->cfg.l2capcfg |= L2CAP_CFG_IN_SUCCESS; /* Local side of the connection
																	  has been configured for outgoing data */
						pcb->cfg.cfgto = L2CAP_CFG_TO; /* Reset configuration timeout */

						if(pcb->cfg.outflushto != L2CAP_CFG_DEFAULT_OUTFLUSHTO) {
							lp_write_flush_timeout(&pcb->remote_bdaddr, pcb->cfg.outflushto);
						}

						/* L2CAP connection established if a successful configuration response has been sent */
						if(pcb->cfg.l2capcfg & L2CAP_CFG_OUT_SUCCESS) {
							pcb->state = L2CAP_OPEN;
							if(pcb->cfg.l2capcfg & L2CAP_CFG_IR) {
								L2CA_ACTION_CONN_CFM(pcb, L2CAP_CONN_SUCCESS, 0x0000, ret);
							} else {
								L2CA_ACTION_CONN_IND(pcb, ERR_OK, ret);
							}
						}
						break;
					case L2CAP_CFG_UNACCEPT:
						/* Parse and add options to pcb */
						while(siglen > 0) {
							opthdr = p->payload;
							/* Check if type of action bit indicates a non-hint. Hints are ignored */
							if(L2CAP_OPTH_TOA(opthdr) == 0) {
								switch(L2CAP_OPTH_TYPE(opthdr)) {
									case L2CAP_CFG_MTU:
										if(L2CAP_MTU > ((u16_t *)p->payload)[1]) {
											pcb->cfg.outmtu = ((u16_t *)p->payload)[1];
										} else {
											LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Configuration of MTU failed\n"));
											l2ca_disconnect_req(pcb, NULL);
											return;
										}
										break;
									case L2CAP_FLUSHTO:
										pcb->cfg.influshto = ((u16_t *)p->payload)[1];
										break;
									case L2CAP_QOS:
										/* If service type Best Effort is not accepted we will close the connection */
										if(((u8_t *)p->payload)[3] != L2CAP_QOS_BEST_EFFORT) {
											LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Unsupported service type\n"));
											l2ca_disconnect_req(pcb, NULL);
											return;
										}
										break;
									default:
										/* Should not happen, skip option */
										break; 
								} /* switch */
							} /* if(L2CAP_OPTH_TOA(opthdr) == 0) */
							pbuf_header(p, -(L2CAP_CFGOPTHDR_LEN + opthdr->len));
							siglen -= L2CAP_CFGOPTHDR_LEN + opthdr->len;
						} /* while */

						/* Send out a new configuration request if the continuation flag isn't set */
						if((flags & 0x0001) == 0) {
							l2ca_config_req(pcb);
						}
						break;
					case L2CAP_CFG_REJ:
						/* Fallthrough */
					case L2CAP_CFG_UNKNOWN:
						/* Fallthrough */
					default:
						if((flags & 0x0001) == 0) {
							LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Configuration failed\n"));
							l2ca_disconnect_req(pcb, NULL);
							return;
						}
						break;
				} /* switch(result) */  

				/* If continuation flag is set we must send a NULL configuration request */
				if((flags & 0x0001) == 1) {
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Continuation flag is set. Send empty (default) config request signal\n"));
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_CFG_REQ_SIZE, PBUF_RAM)) == NULL) {
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Could not allocate memory for pbuf\n"));
						return;
					}
					/* Assemble config request packet */
					((u16_t *)data->payload)[0] = pcb->scid;
					((u16_t *)data->payload)[2] = 0; 
					l2cap_signal(pcb, L2CAP_CFG_REQ, 0, &(pcb->remote_bdaddr), data);
				}
				break;
			case L2CAP_DISCONN_REQ:
				siglen = sighdr->len;
				dcid = ((u16_t *)p->payload)[0];
				siglen = siglen - 2;
				flags = ((u16_t *)p->payload)[1];
				siglen = siglen - 2;
				pbuf_header(p, -4);

				/* Find PCB with matching cid */
				for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
					if(pcb->scid == dcid) {
						/* Matching cid found */
						break;
					}
				}
				/* If no matching cid was found, send a cmd reject (Invalid cid) */
				if(pcb == NULL) {
					/* Alloc size of reason in cmd rej + data (dcid + scid) */
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL) {
						((u16_t *)data->payload)[0] = L2CAP_INVALID_CID;
						((u16_t *)data->payload)[1] = dcid; /* Requested local cid */
						((u16_t *)data->payload)[2] = L2CAP_NULL_CID; /* Remote cid not known */

						ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
					}
				} else { /* Handle disconnection request */
					if((data = pbuf_alloc(PBUF_RAW, L2CAP_DISCONN_RSP_SIZE, PBUF_RAM)) != NULL) {
						((u16_t *)data->payload)[0] = pcb->scid;
						((u16_t *)data->payload)[1] = pcb->dcid;
						ret = l2cap_signal(pcb, L2CAP_DISCONN_RSP, sighdr->id, &(pcb->remote_bdaddr), data);

						/* Give upper layer indication */
						pcb->state = L2CAP_CLOSED;
						LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_process_sig: Disconnection request\n"));
						L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);  
					}	  
				}
				break;
			case L2CAP_DISCONN_RSP:
				if(pcb == NULL) {
					/* A response without a matching request is silently discarded */
					break;
				}
				/* Remove signal from unresponded list and deallocate it */
				L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
				pbuf_free(sig->p);
				lwbt_memp_free(MEMP_L2CAP_SIG, sig);

				L2CA_ACTION_DISCONN_CFM(pcb,ret); /* NOTE: Application should
													 now close the connection */
				break;
			case L2CAP_ECHO_REQ:
				if( pcb != NULL)
				{
					pcb->ursp_id = sighdr->id; 
					ret = l2cap_signal(pcb, L2CAP_ECHO_RSP, sighdr->id, &(pcb->remote_bdaddr), p);
				} else {
					ret = l2cap_signal(NULL, L2CAP_ECHO_RSP, sighdr->id, bdaddr, p);
				}
				pbuf_free(p);
				break;
			case L2CAP_ECHO_RSP:
				if(pcb == NULL) {
					/* A response without a matching request is silently discarded */
					break;
				}
				/* Remove signal from unresponded list and deallocate it */
				L2CAP_SIG_RMV(&(pcb->unrsp_sigs), sig);
				pbuf_free(sig->p);
				lwbt_memp_free(MEMP_L2CAP_SIG, sig);

				/* Remove temporary pcb from active list */
				L2CAP_RMV(&l2cap_active_pcbs, pcb);
				L2CA_ACTION_PING_CFM(pcb,L2CAP_ECHO_RCVD,ret);
				break;
			default:
				/* Alloc size of reason in cmd rej */
				if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE, PBUF_RAM)) != NULL) {
					((u16_t *)data->payload)[0] = L2CAP_CMD_NOT_UNDERSTOOD;

					ret = l2cap_signal(NULL, L2CAP_CMD_REJ, sighdr->id, bdaddr, data);
				}
				break;
		} /* switch */
		len = len - (sighdr->len + L2CAP_SIGHDR_LEN);
		pbuf_header(p, -(sighdr->len));
	} /* while */
}

/* 
 * l2cap_input():
 * 
 * Called by the lower layer. Reassembles the packet, parses the header and forward
 * it to the upper layer or the signal handler.
 */

	void
l2cap_input(struct pbuf *p, struct bd_addr *bdaddr)
{
	struct l2cap_seg *inseg;
	struct hci_acl_hdr *aclhdr;
	struct pbuf *data;
	err_t ret;

	pbuf_header(p, HCI_ACL_HDR_LEN);
	aclhdr = p->payload;
	pbuf_header(p, -HCI_ACL_HDR_LEN);

	pbuf_realloc(p, aclhdr->len);

	for(inseg = l2cap_insegs; inseg != NULL; inseg = inseg->next) {
		if(bd_addr_cmp(bdaddr, &(inseg->bdaddr))) {
			break;
		}
	}

	/* Reassembly procedures */
	/* Check if continuing fragment or start of L2CAP packet */
	if(((aclhdr->conhdl_pb_bc >> 12) & 0x03)== L2CAP_ACL_CONT) { /* Continuing fragment */
		if(inseg == NULL)  {
			/* Discard packet */
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Continuing fragment. Discard packet\n"));
			pbuf_free(p);
			return;
		} else if(inseg->p->tot_len + p->tot_len > inseg->len) { /* Check if length of
																	segment exceeds
																	l2cap header length */
			/* Discard packet */
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Continuing fragment. Length exceeds L2CAP hdr length. Discard packet\n"));
			pbuf_free(inseg->p);
			L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
			lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

			pbuf_free(p);
			return;
		}
		/* Add pbuf to segement */
		pbuf_chain(inseg->p, p);
		pbuf_free(p);

	} else if(((aclhdr->conhdl_pb_bc >> 12) & 0x03) == L2CAP_ACL_START) { /* Start of L2CAP packet */
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Start of L2CAP packet p->len = %d, p->tot_len = %d\n", 
					p->len, p->tot_len));
		if(inseg != NULL) { /* Check if there are segments missing in a previous packet */
			/* Discard previous packet */
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Start of L2CAP packet. Discard previous packet\n"));
			pbuf_free(inseg->p);
		} else {
			inseg = lwbt_memp_malloc(MEMP_L2CAP_SEG);
			bd_addr_set(&(inseg->bdaddr), bdaddr);
			L2CAP_SEG_REG(&(l2cap_insegs), inseg);
		}
		inseg->p = p;
		inseg->l2caphdr = p->payload;
		inseg->len = inseg->l2caphdr->len + L2CAP_HDR_LEN;
		for(inseg->pcb = l2cap_active_pcbs; inseg->pcb != NULL; inseg->pcb = inseg->pcb->next) {
			if(inseg->pcb->scid == inseg->l2caphdr->cid) {
				break; /* found */
			}
		}
	} else {
		/* Discard packet */
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Discard packet\n"));
		pbuf_free(inseg->p);
		L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
		lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

		pbuf_free(p);
		return;
	}
	if(inseg->p->tot_len < inseg->len) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Get continuing segments\n"));
		return; /* Get continuing segments */
	}

	/* Handle packet */
	switch(inseg->l2caphdr->cid) {
		case L2CAP_NULL_CID:
			/* Illegal */
			LWIP_DEBUGF(L2CAP_DEBUG,("l2cap_input: Illegal null cid\n"));
			pbuf_free(inseg->p);
			break;
		case L2CAP_SIG_CID:
			pbuf_header(inseg->p, -L2CAP_HDR_LEN);
			l2cap_process_sig(inseg->p, inseg->l2caphdr, bdaddr);
			pbuf_free(inseg->p);
			break;
		case L2CAP_CONNLESS_CID:
			/* Not needed by PAN, LAN access or DUN profiles */
			pbuf_free(inseg->p);
			break;
		default:
			if(inseg->l2caphdr->cid < 0x0040 || inseg->pcb == NULL) {
				/* Reserved for specific L2CAP functions or channel does not exist */
				/* Alloc size of reason in cmd rej */
				if((data = pbuf_alloc(PBUF_RAW, L2CAP_CMD_REJ_SIZE+4, PBUF_RAM)) != NULL) {
					((u16_t *)data->payload)[0] = L2CAP_INVALID_CID;
					((u16_t *)data->payload)[1] = inseg->l2caphdr->cid;
					((u16_t *)data->payload)[2] = L2CAP_NULL_CID;

					ret = l2cap_signal(NULL, L2CAP_CMD_REJ, l2cap_next_sigid(), bdaddr, data);
				}
				pbuf_free(inseg->p);
				break;
			}

			pbuf_header(inseg->p, -L2CAP_HDR_LEN);

			/* Forward packet to higher layer */
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Forward packet to higher layer\n"));
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_input: Remote BD address: %02x:%02x:%02x:%02x:%02x:%02x\n",
						inseg->pcb->remote_bdaddr.addr[5],
						inseg->pcb->remote_bdaddr.addr[4],
						inseg->pcb->remote_bdaddr.addr[3],
						inseg->pcb->remote_bdaddr.addr[2],
						inseg->pcb->remote_bdaddr.addr[1],
						inseg->pcb->remote_bdaddr.addr[0]));

			L2CA_ACTION_RECV(inseg->pcb,inseg->p,ERR_OK,ret);
			break;
	}

	/* Remove input segment */
	L2CAP_SEG_RMV(&(l2cap_insegs), inseg);
	lwbt_memp_free(MEMP_L2CAP_SEG, inseg);

}

/* 
 * l2cap_cid_alloc():
 * 
 * Allocates a channel identifier (CID). They are local names representing a logical 
 * channel endpoint on the device.
 */

	static u16_t
l2cap_cid_alloc(void)
{
	u16_t cid;
	struct l2cap_pcb *pcb;

	for (cid = L2CAP_MIN_CID; cid < L2CAP_MAX_CID; ++cid) {
		for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
			if(pcb->scid == cid) {
				break;
			}
		}
		if(pcb == NULL) {
			return cid;
		}
	}
	return 0;
}

/* 
 * l2cap_new():
 * 
 * Creates a new L2CAP protocol control block but doesn't place it on
 * any of the L2CAP PCB lists.
 */

	struct l2cap_pcb *
l2cap_new(void)
{
	struct l2cap_pcb *pcb;

	pcb = lwbt_memp_malloc(MEMP_L2CAP_PCB);
	if(pcb != NULL) {
		memset(pcb, 0, sizeof(struct l2cap_pcb));
		pcb->state = L2CAP_CLOSED;

		/* Initialize configuration parameter options with default values */

		/* Maximum Transmission Unit */
		pcb->cfg.inmtu = L2CAP_MTU; /* The MTU that this implementation support */
		pcb->cfg.outmtu = 672; /* Default MTU. Two Baseband DH5 packets minus the Baseband ACL headers and 
								  L2CAP header. This can be set here since we will never send any signals
								  larger than the L2CAP sig MTU (48 bytes) before L2CAP has been configured 
								  */

		/* Flush Timeout */
		pcb->cfg.influshto = 0xFFFF;
		pcb->cfg.outflushto = 0xFFFF;

		pcb->cfg.cfgto = L2CAP_CFG_TO; /* Maximum time before terminating a negotiation.
										  Cfg shall not last more than 120s */
		pcb->cfg.opt = NULL;
		return pcb;
	}
	LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_new: Could not allocate memory for pcb\n"));
	return NULL;
}

/* 
 * l2cap_close():
 * 
 * Closes the L2CAP protocol control block.
 */

	err_t
l2cap_close(struct l2cap_pcb *pcb)
{
	struct l2cap_sig *tmpsig;

	if(pcb->state == L2CAP_LISTEN) {
		L2CAP_RMV((struct l2cap_pcb **)&l2cap_listen_pcbs, pcb);
		lwbt_memp_free(MEMP_L2CAP_PCB_LISTEN, pcb);
	} else {
		L2CAP_RMV(&l2cap_active_pcbs, pcb);
		/* Free any unresponded signals */
		while(pcb->unrsp_sigs != NULL) {
			tmpsig = pcb->unrsp_sigs;
			pcb->unrsp_sigs = pcb->unrsp_sigs->next;
			lwbt_memp_free(MEMP_L2CAP_SIG, tmpsig);
		}

		lwbt_memp_free(MEMP_L2CAP_PCB, pcb);
	}
	pcb = NULL;
	return ERR_OK;
}

/* 
 * l2cap_reset_all():
 *
 * Closes all active and listening L2CAP protocol control blocks.
 */

	void
l2cap_reset_all(void)
{ 
	struct l2cap_pcb *pcb, *tpcb;
	struct l2cap_pcb_listen *lpcb, *tlpcb;
	struct l2cap_seg *seg, *tseg;

	for(pcb = l2cap_active_pcbs; pcb != NULL;) {
		tpcb = pcb->next;
		l2cap_close(pcb);
		pcb = tpcb;
	}

	for(lpcb = l2cap_listen_pcbs; lpcb != NULL;) {
		tlpcb = lpcb->next;
		l2cap_close((struct l2cap_pcb *)lpcb);
		lpcb = tlpcb;
	}

	for(seg = l2cap_insegs; seg != NULL;) {
		tseg = seg->next;
		L2CAP_SEG_RMV(&(l2cap_insegs), seg);
		lwbt_memp_free(MEMP_L2CAP_SEG, seg);
		seg = tseg;
	}

	l2cap_init();
}

/* L2CAP to L2CAP signalling events
*/


/* 
 * l2cap_signal():
 * 
 * Assembles the signalling packet and passes it to the lower layer.
 */

	err_t
l2cap_signal(struct l2cap_pcb *pcb, u8_t code, u16_t ursp_id, struct bd_addr *remote_bdaddr, 
		struct pbuf *data)
{
	struct l2cap_sig *sig;
	struct l2cap_sig_hdr *sighdr;
	struct l2cap_hdr *hdr;
	err_t ret;

	/* Alloc a new signal */
	LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: Allocate memory for l2cap_sig. Code = 0x%x\n", code));
	if((sig = lwbt_memp_malloc(MEMP_L2CAP_SIG)) == NULL) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: could not allocate memory for l2cap_sig\n"));
		return ERR_MEM;
	}

	/* Alloc a pbuf for signal */
	if((sig->p = pbuf_alloc(PBUF_RAW, L2CAP_HDR_LEN+L2CAP_SIGHDR_LEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Setup signal header and leave room for l2cap hdr */
	sighdr = (struct l2cap_sig_hdr *)(((u8_t *)sig->p->payload)+L2CAP_HDR_LEN);

	/* Chain data to signal and set length of signal data */
	if(data == NULL) {
		sighdr->len = 0;
	} else {
		pbuf_chain(sig->p, data);
		pbuf_free(data);
		sighdr->len = data->tot_len;
	}

	sighdr->code = code;

	if(sighdr->code % 2) { /* If odd this is a resp/rej signal */
		sig->sigid = ursp_id; /* Get id */
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: Sending response/reject signal with id = %d code = %d\n", 
					sig->sigid, sighdr->code));
	} else {
		sig->sigid = l2cap_next_sigid(); /* Alloc id */
		sig->rtx = L2CAP_RTX; /* Set Response Timeout Expired timer (in seconds)
								 should be at least as large as the BB flush timeout */
		sig->nrtx = L2CAP_MAXRTX; /* Set max number of retransmissions */
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: Sending request signal with id = %d code = %d\n", 
					sig->sigid, sighdr->code));
	}
	sighdr->id = sig->sigid; /* Set id */

	/* Set up L2CAP hdr */
	hdr = sig->p->payload;
	hdr->len = sig->p->tot_len - L2CAP_HDR_LEN;
	hdr->cid = L2CAP_SIG_CID; /* 0x0001 */

	ret = l2cap_write(remote_bdaddr, sig->p, sig->p->tot_len); /* Send peer L2CAP signal */

	/* Put signal on unresponded list if it's a request signal, else deallocate it */
	if(ret == ERR_OK && (sighdr->code % 2) == 0) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: Registering sent request signal with id = %d code = %d\n", 
					sig->sigid, sighdr->code));
		L2CAP_SIG_REG(&(pcb->unrsp_sigs), sig);
	} else {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_signal: Deallocating sent response/reject signal with id = %d code = %d\n", 
					sig->sigid, sighdr->code));
		pbuf_free(sig->p);
		sig->p = NULL;
		lwbt_memp_free(MEMP_L2CAP_SIG, sig);
	}

	return ret;
}

/* 
 * l2cap_rexmit_signal():
 * 
 * Called by the l2cap timer. Retransmitts a signal.
 */

	err_t
l2cap_rexmit_signal(struct l2cap_pcb *pcb, struct l2cap_sig *sig)
{
	err_t ret;

	/* Set up L2CAP hdr */
	ret = l2cap_write(&(pcb->remote_bdaddr), sig->p, sig->p->tot_len); /* Send peer L2CAP signal */

	return ret;
}

/* Upper-Layer to L2CAP signaling events
*/


/* 
 * l2ca_connect_req():
 * 
 * Initiates the sending of a connect request message. Requests the creation of a 
 * channel representing a logicalconnection to a physical address. Input parameters 
 * are the target protocol(PSM) and remote devices 48-bit address (BD_ADDR). Also 
 * specify the function to be called when a confirm has been received.
 */

	err_t
l2ca_connect_req(struct l2cap_pcb *pcb, struct bd_addr *bdaddr, u16_t psm, 
		u8_t role_switch, err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *lpcb,
			u16_t result, u16_t status))
{
	err_t ret;
	struct pbuf *data;

	if(bdaddr != NULL) {
		bd_addr_set(&(pcb->remote_bdaddr),bdaddr);
	} else {
		return ERR_VAL;
	}

	pcb->psm = psm;
	pcb->l2ca_connect_cfm = l2ca_connect_cfm;
	pcb->scid = l2cap_cid_alloc();
	LWIP_ASSERT("l2ca_connect_req: out of CIDs\n", pcb->scid != 0);

	pcb->cfg.l2capcfg |= L2CAP_CFG_IR; /* We are the initiator of this connection */

	if(!lp_is_connected(bdaddr)) {
		ret = lp_connect_req(bdaddr, role_switch); /* Create ACL link w pcb state == CLOSED */
	} else {
		if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_REQ_SIZE, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_connect_req: Could not allocate memory for pbuf\n"));
			return ERR_MEM;
		}
		((u16_t *)data->payload)[0] = psm;
		((u16_t *)data->payload)[1] = pcb->scid;
		ret = l2cap_signal(pcb, L2CAP_CONN_REQ, 0, &(pcb->remote_bdaddr), data); /* Send l2cap_conn_req signal */

		pcb->state = W4_L2CAP_CONNECT_RSP;
	}

	L2CAP_REG(&l2cap_active_pcbs, pcb);

	return ret;
}

/* 
 * l2ca_config_req():
 * 
 * Requests the initial configuration (or reconfiguration) of a channel to a new set 
 * of channel parameters. Input parameters are the local CID endpoint, new incoming 
 * receivable MTU (InMTU), new outgoing flow specification, and flush and link 
 * timeouts. Also specify the function to be called when a confirm has been received.
 */

	err_t
l2ca_config_req(struct l2cap_pcb *pcb)
{
	struct pbuf *p, *q;
	struct l2cap_cfgopt_hdr *opthdr;
	err_t ret;

	switch(pcb->state) {
		case L2CAP_OPEN:
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: state = L2CAP_OPEN. Suspend transmission\n"));
			/* Note: Application should have suspended data transmission, otherwise outgoing data will be 
			   dropped */
			pcb->state = L2CAP_CONFIG;
			/* Fallthrough */
		case L2CAP_CONFIG:
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: state = L2CAP_CONFIG\n"));

			if((p = pbuf_alloc(PBUF_RAW, L2CAP_CFG_REQ_SIZE, PBUF_RAM)) == NULL) {
				LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
				return ERR_MEM;
			}

			/* Assemble config request packet. Only options that has to be changed will be
			   sent */
			((u16_t *)p->payload)[0] = pcb->dcid;
			/* In this implementation we do not send multiple cmds in one
			   signal packet. Therefore we will never send a config_req packet
			   that will cause the signal to be larger than the minimum L2CAP MTU
			   48 bytes. Hence, this flag will always be cleared */
			((u16_t *)p->payload)[1] = 0; 

			/* Add MTU and out flush timeout to cfg packet if not default value. QoS (Best effort) is always 
			   set to default and can be skipped */
			if(pcb->cfg.inmtu != L2CAP_CFG_DEFAULT_INMTU) {
				if((q = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + L2CAP_MTU_LEN, PBUF_RAM)) == NULL) {
					pbuf_free(p);
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
					return ERR_MEM;
				}
				opthdr = q->payload;
				opthdr->type = L2CAP_CFG_MTU;
				opthdr->len = L2CAP_MTU_LEN;
				((u16_t *)q->payload)[1] = pcb->cfg.inmtu;
				pbuf_chain(p, q);
				pbuf_free(q);
			}

			if(L2CAP_OUT_FLUSHTO != L2CAP_CFG_DEFAULT_OUTFLUSHTO) {
				if((q = pbuf_alloc(PBUF_RAW, L2CAP_CFGOPTHDR_LEN + L2CAP_FLUSHTO_LEN, PBUF_RAM)) == NULL) {
					pbuf_free(p);
					LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: Could not allocate memory for pbuf\n"));
					return ERR_MEM;
				}
				opthdr = q->payload;
				opthdr->type = L2CAP_FLUSHTO;
				opthdr->len = L2CAP_FLUSHTO_LEN;
				((u16_t *)q->payload)[1] = pcb->cfg.outflushto = L2CAP_OUT_FLUSHTO;
				pbuf_chain(p, q);
				pbuf_free(q);
			}

			/* Send config request signal */
			ret = l2cap_signal(pcb, L2CAP_CFG_REQ, 0, &(pcb->remote_bdaddr), p);
			break;
		default:
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_config_req: state = L2CAP_?. Invalid state\n"));
			return ERR_CONN; /* Invalid state. Connection is not in OPEN or CONFIG state */
	}
	return ret;
}

/* 
 * l2ca_disconnect_req():
 * 
 * Requests the disconnection of the channel. Also specify the function to be called
 * when a confirm is received
 */

	err_t
l2ca_disconnect_req(struct l2cap_pcb *pcb, err_t (* l2ca_disconnect_cfm)(void *arg, struct l2cap_pcb *pcb))
{
	struct pbuf *data;
	err_t ret;

	if(pcb->state == L2CAP_OPEN || pcb->state == L2CAP_CONFIG) {
		if((data = pbuf_alloc(PBUF_RAW, L2CAP_DISCONN_REQ_SIZE, PBUF_RAM)) == NULL) {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_disconnect_req: Could not allocate memory for pbuf\n")); 
			return ERR_MEM;
		}
		pcb->l2ca_disconnect_cfm = l2ca_disconnect_cfm;

		((u16_t *)data->payload)[0] = pcb->dcid;
		((u16_t *)data->payload)[1] = pcb->scid;

		ret = l2cap_signal(pcb, L2CAP_DISCONN_REQ, 0, &(pcb->remote_bdaddr), data);

		if(ret == ERR_OK) {
			pcb->state = W4_L2CAP_DISCONNECT_RSP;  
		}
	} else {
		return ERR_CONN; /* Signal not supported in this state */
	}

	return ret;
}

/* 
 * l2ca_datawrite():
 * 
 * Transfers data across the channel.
 */

	err_t
l2ca_datawrite(struct l2cap_pcb *pcb, struct pbuf *p)
{
	err_t ret;
	struct l2cap_hdr *l2caphdr;
	struct pbuf *q;

	if(pcb->state != L2CAP_OPEN) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_datawrite: State != L2CAP_OPEN. Dropping data\n"));
		return ERR_CONN;
	}

	/* Build L2CAP header */
	if((q = pbuf_alloc(PBUF_RAW, L2CAP_HDR_LEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_datawrite: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	pbuf_chain(q, p);

	l2caphdr = q->payload;
	l2caphdr->cid = pcb->dcid;

	/* If length of the data exceeds the OutMTU then only the first OutMTU bytes are sent */
	if(p->tot_len > pcb->cfg.outmtu) {
		/* Send peer L2CAP data */
		l2caphdr->len = pcb->cfg.outmtu;
		if((ret = l2cap_write(&(pcb->remote_bdaddr), q, pcb->cfg.outmtu + L2CAP_HDR_LEN)) == ERR_OK) {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_datawrite: Length of data exceeds the OutMTU p->tot_len = %d\n", p->tot_len));
			ret = ERR_BUF; /* Length of data exceeds the OutMTU */ 
		}
	} else {
		/* Send peer L2CAP data */
		l2caphdr->len = p->tot_len;
		LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_datawrite: q->tot_len = %d\n", q->tot_len));
		ret = l2cap_write(&(pcb->remote_bdaddr), q, q->tot_len);
	}

	/* Free L2CAP header. Higher layers will handle rest of packet */
	p = pbuf_dechain(q);
	pbuf_free(q);

	return ret;
}

/* 
 * l2ca_ping():
 * 
 * Sends an empty L2CAP echo request message. Also specify the function that should
 * be called when a L2CAP echo reply has been received.
 */

	err_t
l2ca_ping(struct bd_addr *bdaddr, struct l2cap_pcb *tpcb,
		err_t (* l2ca_pong)(void *arg, struct l2cap_pcb *pcb, u8_t result))
{
	err_t ret;

	if(!lp_is_connected(bdaddr)) {
		return ERR_CONN;
	}

	bd_addr_set(&(tpcb->remote_bdaddr), bdaddr);
	tpcb->l2ca_pong = l2ca_pong;

	L2CAP_REG(&l2cap_active_pcbs, tpcb);

	ret = l2cap_signal(tpcb, L2CAP_ECHO_REQ, 0, &(tpcb->remote_bdaddr), NULL); /* Send l2cap_echo_req signal */

	return ret;
}

/* Lower-Layer to L2CAP signaling events
*/


/* 
 * lp_connect_cfm():
 * 
 * Confirms the request to establish a lower layer (Baseband) connection.
 */

	void
lp_connect_cfm(struct bd_addr *bdaddr, u8_t encrypt_mode, err_t err)
{
	struct l2cap_pcb *pcb;
	struct pbuf *data;
	err_t ret;

	for(pcb = l2cap_active_pcbs; pcb != NULL; pcb = pcb->next) {
		if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr)) {
			break;
		}
	}
	if(pcb == NULL) {
		/* Silently discard */
		LWIP_DEBUGF(L2CAP_DEBUG, ("lp_connect_cfm: Silently discard\n")); 
	} else {
		if(err == ERR_OK) {
			pcb->encrypt = encrypt_mode;
			/* Send l2cap_conn_req signal if no error */
			if((data = pbuf_alloc(PBUF_RAW, L2CAP_CONN_REQ_SIZE, PBUF_RAM)) != NULL) {
				((u16_t *)data->payload)[0] = pcb->psm;
				((u16_t *)data->payload)[1] = pcb->scid;
				if((ret = l2cap_signal(pcb, L2CAP_CONN_REQ, 0, &(pcb->remote_bdaddr), data)) == ERR_OK) {
					pcb->state = W4_L2CAP_CONNECT_RSP;
				} else {
					L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available? */
				}
				LWIP_DEBUGF(L2CAP_DEBUG, ("lp_connect_cfm: l2cap_conn_req signal sent. err = %d\nPSM = 0x%x\nscid = 0x%x\nencrypt mode = 0x%x\n", err, pcb->psm, pcb->scid, pcb->encrypt)); 
			} else {
				LWIP_DEBUGF(L2CAP_DEBUG, ("lp_connect_cfm: No resources available\n")); 
				L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available */
			}
		} else {
			LWIP_DEBUGF(L2CAP_DEBUG, ("lp_connect_cfm: Connection falied\n")); 
			L2CA_ACTION_CONN_CFM(pcb,L2CAP_CONN_REF_RES,0x0000,ret); /* No resources available */
		}
	}
}

/* 
 * lp_connect_ind():
 * 
 * Indicates the lower protocol has successfully established a connection.
 */

	void
lp_connect_ind(struct bd_addr *bdaddr)
{
	LWIP_DEBUGF(L2CAP_DEBUG, ("lp_connect_ind\n")); 
}

/* 
 * lp_disconnect_ind():
 * 
 * Indicates the lower protocol (Baseband) has been shut down by LMP commands or a 
 * timeout event..
 */

	void
lp_disconnect_ind(struct bd_addr *bdaddr)
{
	struct l2cap_pcb *pcb, *tpcb;
	err_t ret;

	for(pcb = l2cap_active_pcbs; pcb != NULL;) {
		tpcb = pcb->next;
		LWIP_DEBUGF(L2CAP_DEBUG, ("lp_disconnect_ind: Find a pcb with a matching Bluetooth address\n"));
		/* All PCBs with matching Bluetooth address have been disconnected */
		if(bd_addr_cmp(&(pcb->remote_bdaddr), bdaddr)) {// && pcb->state != L2CAP_CLOSED) {
			pcb->state = L2CAP_CLOSED;
			LWIP_DEBUGF(L2CAP_DEBUG, ("lp_disconnect_ind: Notify application\n"));
			L2CA_ACTION_DISCONN_IND(pcb,ERR_OK,ret);
		}
		pcb = tpcb;
		}
	}

	/* 
	 * l2cap_next_sigid():
	 * 
	 * Issues a signal identifier that helps matching a request with the reply.
	 */

	u8_t
		l2cap_next_sigid(void)
		{
			++sigid_nxt;
			if(sigid_nxt == 0) {
				sigid_nxt = 1;
			}
			return sigid_nxt;
		}

	/* 
	 * l2cap_arg():
	 * 
	 * Used to specify the argument that should be passed callback functions.
	 */

	void
		l2cap_arg(struct l2cap_pcb *pcb, void *arg)
		{
			pcb->callback_arg = arg;
		}

	/* 
	 * l2cap_connect_ind():
	 * 
	 * Set the state of the connection to be LISTEN, which means that it is able to
	 * accept incoming connections. The protocol control block is reallocated in
	 * order to consume less memory. Setting the connection to LISTEN is an
	 * irreversible process. Also specify the function that should be called when
	 * the channel has received a connection request.
	 */
	err_t
	   l2cap_connect_ind(struct l2cap_pcb *npcb, u8_t psm,
			   err_t (* l2ca_connect_ind)(void *arg, struct l2cap_pcb *pcb, err_t err))
	{
		struct l2cap_pcb_listen *lpcb;

		lpcb = lwbt_memp_malloc(MEMP_L2CAP_PCB_LISTEN);
		if(lpcb == NULL) {
			LWIP_DEBUGF(L2CAP_DEBUG, ("l2cap_connect_ind: Could not allocate memory for lpcb\n"));
			return ERR_MEM;
		}
		lpcb->psm = psm;
		lpcb->l2ca_connect_ind = l2ca_connect_ind;
		lpcb->state = L2CAP_LISTEN;
		lpcb->callback_arg = npcb->callback_arg;
		lwbt_memp_free(MEMP_L2CAP_PCB, npcb);
		L2CAP_REG((struct l2cap_pcb **)&l2cap_listen_pcbs, (struct l2cap_pcb *)lpcb);
		return ERR_OK;
	}

	/* 
	 * l2cap_disconnect_ind():
	 * 
	 * Used to specify the a function to be called when a disconnection request has been 
	 * received from a remote device or the remote device has been disconnected because it 
	 * has failed to respond to a signalling request.
	 */
	void
	   l2cap_disconnect_ind(struct l2cap_pcb *pcb,
			   err_t (* l2ca_disconnect_ind)(void *arg, struct l2cap_pcb *newpcb, err_t err))
	{
		pcb->l2ca_disconnect_ind = l2ca_disconnect_ind;
	}

	/*
	 * l2cap_timeout_ind():
	 * 
	 * Used to specify the function to be called when RTX or ERTX timer has expired.
	 */
	void
	   l2cap_timeout_ind(struct l2cap_pcb *pcb,
			   err_t (* l2ca_timeout_ind)(void *arg, struct l2cap_pcb *newpcb, err_t err))
	{
		pcb->l2ca_timeout_ind = l2ca_timeout_ind;
	}

	/* 
	 * l2cap_recv():
	 * 
	 * Used to specify the function that should be called when a L2CAP connection receives 
	 * data.
	 */
	void
	   l2cap_recv(struct l2cap_pcb *pcb,
			   err_t (* l2ca_recv)(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err))
	{
		pcb->l2ca_recv = l2ca_recv;
	}

