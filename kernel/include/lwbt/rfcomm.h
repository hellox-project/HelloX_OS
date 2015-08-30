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

#ifndef __LWBT_RFCOMM_H__
#define __LWBT_RFCOMM_H__

#include "lwbt/l2cap.h"


//struct rfcomm_pcb;

/* Functions for interfacing with RFCOMM: */

/* Lower layer interface to RFCOMM: */
void rfcomm_init(void); /* Must be called first to initialize RFCOMM */
void rfcomm_tmr(void); /* Must be called every 1s interval */

/* Application program's interface: */
struct rfcomm_pcb *rfcomm_new(struct l2cap_pcb *pcb);
void rfcomm_close(struct rfcomm_pcb *pcb);
void rfcomm_reset_all(void);
void rfcomm_arg(struct rfcomm_pcb *pcb, void *arg);
void rfcomm_recv(struct rfcomm_pcb *pcb, 
		 err_t (* recv)(void *arg, struct rfcomm_pcb *pcb, struct pbuf *p, err_t err));
void rfcomm_disc(struct rfcomm_pcb *pcb, 
		 err_t (* disc)(void *arg, struct rfcomm_pcb *pcb, err_t err));

#define rfcomm_cn(pcb) ((pcb)->cn)
#define rfcomm_cl(pcb) ((pcb)->cl)
#define rfcomm_local_credits(pcb) ((pcb)->k)
#define rfcomm_remote_credits(pcb) ((pcb)->rk)
#define rfcomm_fc(pcb) ((pcb)->fc)
#define rfcomm_mfs(pcb) ((pcb)->n)

err_t rfcomm_input(void *arg, struct l2cap_pcb *l2cappcb, struct pbuf *p, err_t err);

err_t rfcomm_connect(struct rfcomm_pcb *pcb, u8_t cn, 
		     err_t (* connected)(void *arg, struct rfcomm_pcb *tpcb, err_t err));
err_t rfcomm_disconnect(struct rfcomm_pcb *pcb);
err_t rfcomm_listen(struct rfcomm_pcb *npcb, u8_t cn, 
		    err_t (* accept)(void *arg, struct rfcomm_pcb *pcb, err_t err));
err_t rfcomm_pn(struct rfcomm_pcb *pcb,
		err_t (* pn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err));
err_t rfcomm_test(struct rfcomm_pcb *pcb, 
		  err_t (* test_rsp)(void *arg, struct rfcomm_pcb *tpcb, err_t err));
err_t rfcomm_msc(struct rfcomm_pcb *pcb, u8_t fc, 
		 err_t (* msc_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err));
err_t rfcomm_rpn(struct rfcomm_pcb *pcb, u8_t br,
	   err_t (* rpn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err));
err_t rfcomm_uih(struct rfcomm_pcb *pcb, u8_t cn, struct pbuf *q);
err_t rfcomm_uih_credits(struct rfcomm_pcb *pcb, u8_t credits, struct pbuf *q);

err_t rfcomm_lp_disconnected(struct l2cap_pcb *pcb);


/* Control field values */
#define RFCOMM_SABM 0x3F
#define RFCOMM_UA 0x73
#define RFCOMM_DM 0x0F
#define RFCOMM_DM_PF 0x1F
#define RFCOMM_DISC 0x53
#define RFCOMM_UIH 0xEF
#define RFCOMM_UIH_PF 0xFF 

/* Multiplexer message types */
#define RFCOMM_PN_CMD 0x83
#define RFCOMM_PN_RSP 0x81
#define RFCOMM_TEST_CMD 0x23
#define RFCOMM_TEST_RSP 0x21
#define RFCOMM_FCON_CMD 0xA3
#define RFCOMM_FCON_RSP 0xA1
#define RFCOMM_FCOFF_CMD 0x63
#define RFCOMM_FCOFF_RSP 0x61
#define RFCOMM_MSC_CMD 0xE3
#define RFCOMM_MSC_RSP 0xE1
#define RFCOMM_RPN_CMD 0x93
#define RFCOMM_RPN_RSP 0x91
#define RFCOMM_RLS_CMD 0x53
#define RFCOMM_RLS_RSP 0x51 
#define RFCOMM_NSC_RSP 0x11

/* Length of RFCOMM hdr with 1 or 2 byte lengh field excluding credit */
#define RFCOMM_HDR_LEN_1 3
#define RFCOMM_HDR_LEN_2 4

/* Length of a multiplexer message */
#define RFCOMM_MSGHDR_LEN 2
#define RFCOMM_PNMSG_LEN 8
#define RFCOMM_MSCMSG_LEN 2
#define RFCOMM_RPNMSG_LEN 8
#define RFCOMM_NCMSG_LEN 1

/* Length of a frame */
#define RFCOMM_DM_LEN 4
#define RFCOMM_SABM_LEN 4
#define RFCOMM_DISC_LEN 4
#define RFCOMM_UA_LEN 4
#define RFCOMM_UIH_LEN 3
#define RFCOMM_UIHCRED_LEN 4

/* Default convergence layer */
#define RFCOMM_CL 0xF /* Credit based flow control */

/* Default port settings for a communication link */
#define RFCOMM_COM_BR 0x03 /* Baud rate (9600 bit/s)*/
#define RFCOMM_COM_CFG 0x03 /* Data bits (8 bits), stop bits (1), parity (no parity) 
			       and parity type */
#define RFCOMM_COM_FC 0x00 /* Flow control (no flow ctrl) */
#define RFCOMM_COM_XON 0x00 /* No flow control (default DC1) */ 
#define RFCOMM_COM_XOFF 0x00 /* No flow control (default DC3) */

/* FCS calc */
#define RFCOMM_CODE_WORD 0xE0 /* pol = x8+x2+x1+1 */
#define RFCOMM_CRC_CHECK_LEN 3
#define RFCOMM_UIHCRC_CHECK_LEN 2

/* RFCOMM configuration parameter masks */
#define RFCOMM_CFG_IR 0x01
#define RFCOMM_CFG_FC 0x02
#define RFCOMM_CFG_MSC_OUT 0x04
#define RFCOMM_CFG_MSC_IN 0x08

enum rfcomm_state {
  RFCOMM_CLOSED, RFCOMM_LISTEN, W4_RFCOMM_MULTIPLEXER, W4_RFCOMM_SABM_RSP, RFCOMM_CFG, RFCOMM_OPEN, 
  W4_RFCOMM_DISC_RSP
};

/* The RFCOMM frame header */
struct rfcomm_hdr {
  u8_t addr;
  u8_t ctrl;
  u16_t len;
  u8_t k;
};

struct rfcomm_msg_hdr {
  u8_t type;
  u8_t len;
};

struct rfcomm_pn_msg {
  u8_t dlci; /* Data link connection id */
  u8_t i_cl; /* Type frame for information and Convergece layer to use */
  u8_t p; /* Priority */
  u8_t t; /* Value of acknowledgement timer */
  u16_t n; /* Maximum frame size */
  u8_t na; /* Maximum number of retransmissions */
  u8_t k; /* Initial credit value */
};

struct rfcomm_msc_msg {
  u8_t dlci; /* Data link connection id */
  u8_t rs232; /* RS232 control signals */
};

struct rfcomm_rpn_msg {
  u8_t dlci; /* Data link connection id */
  u8_t br; /* Baud Rate */
  u8_t cfg; /* Data bits, Stop bits, Parity, Parity type */
  u8_t fc; /* Flow control */
  u8_t xon;
  u8_t xoff;
  u16_t mask;
};

/* The RFCOMM protocol control block */
struct rfcomm_pcb {
  struct rfcomm_pcb *next; /* For the linked list */

  enum rfcomm_state state; /* RFCOMM state */

  struct l2cap_pcb *l2cappcb; /* The L2CAP connection */

  u8_t cn; /* Channel number */
  
  u8_t cl; /* Convergence layer */
  u8_t p; /* Connection priority */
  u16_t n; /* Maximum frame size */
  u8_t k; /* No of local credits */

  u8_t rk; /* No of remote credits */

  u8_t rfcommcfg; /* Bit 1 indicates if we are the initiator of this connection
		   * Bit 2 indicates if the flow control bit is set so that we are allowed to send data
		   * Bit 3 indicates if modem status has been configured for the incoming direction
		   * Bit 4 indicates if modem status has been configured for the outgoing direction
		   */

  u16_t to; /* Frame and cmd timeout */
  
  u8_t uih_in_fcs; /* Frame check sequence for uih frames (P/F bit = 0) */
  u8_t uihpf_in_fcs; /* Frame check sequence for uih frames (P/F bit = 1) */
  u8_t uih_out_fcs; /* Frame check sequence for uih frames (P/F bit = 0) */
  u8_t uihpf_out_fcs; /* Frame check sequence for uih frames (P/F bit = 1) */

  u8_t uih0_in_fcs; /* Frame check sequence for uih frames on the control channel (P/F bit = 0) */
  u8_t uih0_out_fcs; /* Frame check sequence for uih frames on the control channel (P/F bit = 0) */

#if RFCOMM_FLOW_QUEUEING
  struct pbuf *buf;
#endif
  void *callback_arg;
  
  /* RFCOMM Frame commands and responses */
  err_t (* connected)(void *arg, struct rfcomm_pcb *pcb, err_t err);
  err_t (* accept)(void *arg, struct rfcomm_pcb *pcb, err_t err);
  err_t (* disconnected)(void *arg, struct rfcomm_pcb *pcb, err_t err);

  /* RFCOMM Multiplexer responses */
  err_t (* pn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err);
  err_t (* test_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err);
  err_t (* msc_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err);
  err_t (* rpn_rsp)(void *arg, struct rfcomm_pcb *pcb, err_t err);

  err_t (* recv)(void *arg, struct rfcomm_pcb *pcb, struct pbuf *p, err_t err);
};

/* The server channel */
struct rfcomm_pcb_listen {
  struct rfcomm_pcb_listen *next; /* For the linked list */

  enum rfcomm_state state; /* RFCOMM state */

  u8_t cn; /* Channel number */

  void *callback_arg;

  err_t (* accept)(void *arg, struct rfcomm_pcb *pcb, err_t err);
};

#define RFCOMM_EVENT_CONNECTED(pcb,err,ret) \
                              if((pcb)->connected != NULL) \
                              (ret = (pcb)->connected((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_ACCEPT(pcb,err,ret) \
                              if((pcb)->accept != NULL) \
                              (ret = (pcb)->accept((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_DISCONNECTED(pcb,err,ret) \
                                 if((pcb)->disconnected != NULL) { \
                                   (ret = (pcb)->disconnected((pcb)->callback_arg,(pcb),(err))); \
                                 } else { \
                                   rfcomm_close(pcb); \
				 }
#define RFCOMM_EVENT_PN_RSP(pcb,err,ret) \
                       if((pcb)->pn_rsp != NULL) \
                       (ret = (pcb)->pn_rsp((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_TEST(pcb,err,ret) \
                       if((pcb)->test_rsp != NULL) \
                       (ret = (pcb)->test_rsp((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_MSC(pcb,err,ret) \
                        if((pcb)->msc_rsp != NULL) \
                        (ret = (pcb)->msc_rsp((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_RPN(pcb,err,ret) \
                        if((pcb)->rpn_rsp != NULL) \
                        (ret = (pcb)->rpn_rsp((pcb)->callback_arg,(pcb),(err)))
#define RFCOMM_EVENT_RECV(pcb,err,p,ret) \
                          if((pcb)->recv != NULL) { \
                            (ret = (pcb)->recv((pcb)->callback_arg,(pcb),(p),(err))); \
                          } else { \
                            pbuf_free(p); \
                          }


/* The RFCOMM PCB lists. */
extern struct rfcomm_pcb_listen *rfcomm_listen_pcbs; /* List of all RFCOMM PCBs listening for 
							 an incomming connection on a specific
							 server channel */
extern struct rfcomm_pcb *rfcomm_active_pcbs; /* List of all active RFCOMM PCBs */

extern struct rfcomm_pcb *rfcomm_tmp_pcb;      /* Only used for temporary storage. */

/* Define two macros, RFCOMM_REG and RFCOMM_RMV that registers a RFCOMM PCB
   with a PCB list or removes a PCB from a list, respectively. */

#define RFCOMM_REG(pcbs, npcb) \
	do { \
		(npcb)->next = *(pcbs); \
		*(pcbs) = (npcb); \
	} while(0)

#define RFCOMM_RMV(pcbs, npcb) \
	do { \
		if(*(pcbs) == (npcb)) { \
			*(pcbs) = (*(pcbs))->next; \
		} else for(rfcomm_tmp_pcb = *(pcbs); rfcomm_tmp_pcb != NULL; rfcomm_tmp_pcb = rfcomm_tmp_pcb->next) { \
			if(rfcomm_tmp_pcb->next != NULL && rfcomm_tmp_pcb->next == (npcb)) { \
				rfcomm_tmp_pcb->next = (npcb)->next; \
				break; \
			} \
		} \
		(npcb)->next = NULL; \
	} while(0)

#endif /* __RFCOMM_H__ */
