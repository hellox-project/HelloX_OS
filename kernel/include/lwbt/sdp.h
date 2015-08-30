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

#ifndef __LWBT_SDP_H__
#define __LWBT_SDP_H__

#include "lwbt/l2cap.h"

struct sdp_pcb;
struct sdp_record;
struct sdp_attribute;
struct sdp_element;

/* Functions for interfacing with SDP: */
void sdp_init(void);

/* Client API */
struct sdp_pcb *sdp_new(struct l2cap_pcb *l2cappcb);
void sdp_free(struct sdp_pcb *pcb);
void sdp_reset_all(void);
void sdp_arg(struct sdp_pcb *pcb, void *arg);
err_t sdp_service_search_req(struct sdp_pcb *pcb, u8_t *ssp, u8_t ssplen, u16_t max_src,
		   void (* service_searched)(void *arg, struct sdp_pcb *pcb, u16_t tot_src, 
					     u16_t curr_src, u32_t *rhdls));
err_t sdp_service_attrib_req(struct sdp_pcb *pcb, u32_t srhdl, u16_t max_abc, u8_t *attrids, u8_t attrlen,
			     void (* attributes_recv)(void *arg, struct sdp_pcb *pcb, u16_t attribl_bc, struct pbuf *p));
err_t sdp_service_search_attrib_req(struct sdp_pcb *pcb, u16_t max_abc, u8_t *ssp, u8_t ssplen, u8_t *attrids, 
				    u8_t attrlen, void (* attributes_searched)(void *arg, struct sdp_pcb *pcb, 
									       u16_t attribl_bc, 
									       struct pbuf *p));

/* Server API */
/* Functions to be used when adding and removing service records to and from the SDDB */
struct sdp_record *sdp_record_new(u8_t *record_de_list, u16_t rlen);
void sdp_record_free(struct sdp_record *record);
err_t sdp_register_service(struct sdp_record *record);
void sdp_unregister_service(struct sdp_record *record);

/* Lower layer API */
void sdp_lp_disconnected(struct l2cap_pcb *l2cappcb);
err_t sdp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err);

/* Type is constructed by ORing a type and size bitmask.
   Size is ignored for String, URL and sequence types.
   For String, URL types, the given value must be a char*,
   from which the size is calculated.
   For a sequence type the size is calculated directly from the
   list of elements added into the sequence.
   For integer types greater than 32 bit, and for 128 bit UUID
   types, the value is given as a byte array.
*/

#define SDP_DE_TYPE_NIL		(0<<3) /* Nil, the null type */
#define SDP_DE_TYPE_UINT	(1<<3) /* Unsigned Integer */
#define SDP_DE_TYPE_STCI	(2<<3) /* Signed, twos-complement integer */
#define SDP_DE_TYPE_UUID	(3<<3) /* UUID, a universally unique identifier */
#define SDP_DE_TYPE_STR		(4<<3) /* Text string */
#define SDP_DE_TYPE_BOOL	(5<<3) /* Boolean */
#define SDP_DE_TYPE_DES		(6<<3) /* Data Element Sequence */
#define SDP_DE_TYPE_DEA		(7<<3) /* Data Element Alternative */
#define SDP_DE_TYPE_URL		(8<<3) /* URL, a uniform resource locator */

#define SDP_DE_SIZE_8	0x0 /* 8 bit integer value */
#define SDP_DE_SIZE_16	0x1 /* 16 bit integer value */
#define SDP_DE_SIZE_32	0x2 /* 32 bit integer value */
#define SDP_DE_SIZE_64	0x3 /* 64 bit integer value */
#define SDP_DE_SIZE_128	0x4 /* 128 bit integer value */
#define SDP_DE_SIZE_N1	0x5 /* Data size is in next 1 byte */
#define SDP_DE_SIZE_N2	0x6 /* Data size is in next 2 bytes */
#define SDP_DE_SIZE_N4	0x7 /* Data size is in next 4 bytes */

#define SDP_DES_SIZE8	(SDP_DE_TYPE_DES  | SDP_DE_SIZE_N1)
#define SDP_DES_SIZE16	(SDP_DE_TYPE_DES  | SDP_DE_SIZE_N2)
#define SDP_UINT8		(SDP_DE_TYPE_UINT | SDP_DE_SIZE_8)
#define SDP_UINT16		(SDP_DE_TYPE_UINT | SDP_DE_SIZE_16)
#define SDP_UINT32		(SDP_DE_TYPE_UINT | SDP_DE_SIZE_32)
#define SDP_UUID16		(SDP_DE_TYPE_UUID | SDP_DE_SIZE_16)
#define SDP_UUID128		(SDP_DE_TYPE_UUID | SDP_DE_SIZE_128)

/* PDU identifiers */
#define SDP_ERR_PDU 0x01
#define SDP_SS_PDU 0x02
#define SDP_SSR_PDU 0x03
#define SDP_SA_PDU 0x04
#define SDP_SAR_PDU 0x05
#define SDP_SSA_PDU 0x06
#define SDP_SSAR_PDU 0x07

/* Response lengths and sizes */
#define SDP_PDUHDR_LEN 5
#define SDP_ATTRIBIDHDR_LEN 3
#define SDP_SSR_LEN 4
#define SDP_SRHDL_SIZE 4 /* Size of a service record handle */

/**/
#define SDP_MAX_SRHDLS 12

struct sdp_hdr {
  u8_t pdu;
  u16_t id;
  u16_t len;
} PACK_STRUCT_STRUCT;

struct sdp_record {
  struct sdp_record *next; /* For the linked list */
  
  u32_t hdl; /* Service Record Handle */
  u8_t *record_de_list;
  u16_t len;
};

/* The SDP protocol control block */
struct sdp_pcb {
  struct sdp_pcb *next; /* For the linked list */

  struct l2cap_pcb *l2cappcb; /* The L2CAP connection */

  u16_t tid; /* Transaction ID */
  
  void *callback_arg;

  void (* service_searched)(void *arg, struct sdp_pcb *pcb, u16_t tot_src, u16_t curr_src, u32_t *rhdls);
  void (* attributes_recv)(void *arg, struct sdp_pcb *pcb, u16_t attribl_bc,struct pbuf *p);
  void (* attributes_searched)(void *arg, struct sdp_pcb *pcb, u16_t attribl_bc, struct pbuf *p);
};

#define SDP_DE_TYPE(type_size) ((type_size) & 0xF8)
#define SDP_DE_SIZE(type_size) ((type_size) & 0x07)

#define SDP_ACTION_SERVICE_SEARCHED(pcb,tot_src,curr_src,rhdls) if((pcb)->service_searched != NULL) ((pcb)->service_searched((pcb)->callback_arg,(pcb),(tot_src),(curr_src),(rhdls)))
#define SDP_ACTION_ATTRIB_RECV(pcb,attribl_bc,p) if((pcb)->attributes_recv != NULL) ((pcb)->attributes_recv((pcb)->callback_arg,(pcb),(attribl_bc),(p)))
#define SDP_ACTION_ATTRIB_SEARCHED(pcb,attribl_bc,p) if((pcb)-> attributes_searched != NULL) ((pcb)->attributes_searched((pcb)->callback_arg,(pcb),(attribl_bc),(p)))

extern struct sdp_pcb *sdp_pcbs; /* List of all SDP PCBs awaiting incoming response to 
				    a request */
extern struct sdp_pcb *sdp_tmp_pcb; /* Only used for temporary storage. */

extern struct sdp_record *sdp_server_records;  /* List of all active service records in the
						  SDP server */
extern struct sdp_record *sdp_tmp_record;      /* Only used for temporary storage. */

#define SDP_REG(pcbs, npcb) do { \
                            npcb->next = *pcbs; \
                            *pcbs = npcb; \
                            } while(0)
#define SDP_RMV(pcbs, npcb) do { \
                            if(*pcbs == npcb) { \
                               *pcbs = (*pcbs)->next; \
                            } else for(sdp_tmp_pcb = *pcbs; sdp_tmp_pcb != NULL; sdp_tmp_pcb = sdp_tmp_pcb->next) { \
                               if(sdp_tmp_pcb->next != NULL && sdp_tmp_pcb->next == npcb) { \
                                  sdp_tmp_pcb->next = npcb->next; \
                                  break; \
                               } \
                            } \
                            npcb->next = NULL; \
                            } while(0)
#define SDP_RECORD_REG(records, record) do { \
                                        record->next = *records; \
                                        *records = record; \
                                        } while(0)
#define SDP_RECORD_RMV(records, record) do { \
                            if(*records == record) { \
                               *records = (*records)->next; \
                            } else for(sdp_tmp_record = *records; sdp_tmp_record != NULL; sdp_tmp_record = sdp_tmp_record->next) { \
                               if(sdp_tmp_record->next != NULL && sdp_tmp_record->next == record) { \
                                  sdp_tmp_record->next = record->next; \
                                  break; \
                               } \
                            } \
                            record->next = NULL; \
                            } while(0)

#endif /* __LWBT_SDP_H__ */
