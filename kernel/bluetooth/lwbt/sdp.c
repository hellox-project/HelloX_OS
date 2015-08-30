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


/* sdp.c
 *
 * Implementation of the service discovery protocol (SDP)
 */


#include "lwip/opt.h"
#include "lwbt/sdp.h"
#include "lwbt/lwbt_memp.h"

#include "lwip/err.h"
#include "lwip/pbuf.h"

#include "lwip/debug.h"
#include "lwip/inet.h"

/* Next service record handle to be used */
u32_t rhdl_next;

/* Next transaction id to be used */
u16_t tid_next;

/* The SDP PCB lists */
struct sdp_pcb *sdp_pcbs;
struct sdp_pcb *sdp_tmp_pcb;

/* List of all active service records in the SDP server */
struct sdp_record *sdp_server_records;
struct sdp_record *sdp_tmp_record; /* Only used for temp storage */


/* 
 * sdp_init():
 * 
 * Initializes the SDP layer.
 */
void sdp_init(void)
{
	/* Clear globals */
	sdp_server_records = NULL;
	sdp_tmp_record = NULL;

	/* Inialize service record handles */
	rhdl_next = 0x0000FFFF;

	/* Initialize transaction ids */
	tid_next = 0x0000;
}

/* Server API
*/


/* 
 * sdp_next_rhdl():
 * 
 * Issues a service record handler.
 */
u32_t sdp_next_rhdl(void)
{
	++rhdl_next;
	if(rhdl_next == 0) {
		rhdl_next = 0x0000FFFF;
	}
	return rhdl_next;
}

/* 
 * sdp_record_new():
 * 
 * Creates a new service record.
 */
struct sdp_record * sdp_record_new(u8_t *record_de_list, u16_t rlen)
{
	struct sdp_record *record;

	record = lwbt_memp_malloc(MEMP_SDP_RECORD);
	if(record != NULL) {
		record->hdl = sdp_next_rhdl();
		record->record_de_list = record_de_list;
		record->len = rlen;
		return record;
	}
	return NULL;
}

void sdp_record_free(struct sdp_record *record)
{
	lwbt_memp_free(MEMP_SDP_RECORD, record);
}

/* 
 * sdp_register_service():
 * 
 * Add a record to the list of records in the service record database, making it 
 * available to clients.
 */
err_t sdp_register_service(struct sdp_record *record)
{
	if(record == NULL) {
		return ERR_ARG;
	}
	SDP_RECORD_REG(&sdp_server_records, record);
	return ERR_OK;
}

/* 
 * sdp_unregister_service():
 * 
 * Remove a record from the list of records in the service record database, making it 
 * unavailable to clients.
 */
void sdp_unregister_service(struct sdp_record *record)
{
	SDP_RECORD_RMV(&sdp_server_records, record);
}

/* 
 * sdp_next_transid():
 * 
 * Issues a transaction identifier that helps matching a request with the reply.
 */
u16_t sdp_next_transid(void)
{
	++tid_next;
	return tid_next;
}

/* 
 * sdp_pattern_search():
 * 
 * Check if the given service search pattern matches the record.
 */
u8_t sdp_pattern_search(struct sdp_record *record, u8_t size, struct pbuf *p) 
{
	u8_t i, j;
	u8_t *payload = (u8_t *)p->payload;

	//TODO actually parse the request instead of going over each byte
	for(i = 0; i < size; ++i) {
		if(SDP_DE_TYPE(payload[i]) == SDP_DE_TYPE_UUID)  {
			switch(SDP_DE_SIZE(payload[i])) {
				case SDP_DE_SIZE_16:
					for(j = 0; j < record->len; ++j) {
						if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_UUID) {
							if(*((u16_t *)(payload + i + 1)) == *((u16_t *)(record->record_de_list + j + 1))) {
								return 1; /* Found a matching UUID in record */
							}
							++j;
						}
					}
					i += 2;
					break;
				case SDP_DE_SIZE_32:
					i += 4;
					break;
				case SDP_DE_SIZE_128:
					LWIP_DEBUGF(SDP_DEBUG, ("TODO: add support for 128-bit UUID\n"));
					i+= 16;
					break;
				default:
					break;
			}
		}
	}
	return 1; //TODO change back to 0
}

/*
 * sdp_attribute_search():
 * 
 * Searches a record for attributes and add them to a given packet buffer.
 */
struct pbuf * sdp_attribute_search(u16_t max_attribl_bc, struct pbuf *p, struct sdp_record *record) 
{
	struct pbuf *q = NULL;
	struct pbuf *r; 
	struct pbuf *s = NULL;
	u8_t *payload = (u8_t *)p->payload;
	u8_t size;
	u8_t i = 0, j;
	u16_t attr_id = 0, attr_id2 = 0;

	u16_t attribl_bc = 0; /* Byte count of the sevice attributes */
	u32_t hdl = htonl(record->hdl);

	if(SDP_DE_TYPE(payload[0]) == SDP_DE_TYPE_DES &&
			SDP_DE_SIZE(payload[0]) == SDP_DE_SIZE_N1) {
		/* Get size of attribute ID list */
		size = payload[1]; //TODO: correct to assume only one size byte in remote request? probably  

		while(i < size) {
			/* Check if this is an attribute ID or a range of attribute IDs */
			if(payload[2+i] == (SDP_DE_TYPE_UINT  | SDP_DE_SIZE_16)) {
				attr_id = ntohs(*((u16_t *)(payload+3+i)));
				attr_id2 = attr_id; /* For the range to cover this attribute ID only */
				i += 3;
			} else if(payload[2+i] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_32)) {
				attr_id = ntohs(*((u16_t *)(payload+3+i)));
				attr_id2 = ntohs(*((u16_t *)(payload+5+i)));
				i += 5;
			} else {
				/* ERROR: Invalid req syntax */
				LWIP_DEBUGF(SDP_DEBUG, ("sdp_attribute_search: Invalid req syntax\n"));
			}

			LWIP_DEBUGF(SDP_DEBUG, ("sdp_attribute_search: looking for %04x-%04x\n", attr_id, attr_id2));

			for(j = 0; j < record->len; ++j) {
				if(SDP_DE_TYPE(record->record_de_list[j]) == SDP_DE_TYPE_DES) {
					if(record->record_de_list[j + 2] == (SDP_DE_TYPE_UINT | SDP_DE_SIZE_16)) {
						u16_t rec_id = ntohs(*((u16_t *)(record->record_de_list + j + 3)));

						LWIP_DEBUGF(SDP_DEBUG, ("sdp_attribute_search: looking at rec %04x\n", rec_id));
						
						if(rec_id >= attr_id && rec_id <= attr_id2) {
							if(attribl_bc +  record->record_de_list[j + 1] + 2 > max_attribl_bc) {
								/* Abort attribute search since attribute list byte count must not 
								   exceed max attribute byte count in req */
								break;
							}
							/* Allocate a pbuf for the service attribute */
							;
							if((r = pbuf_alloc(PBUF_RAW, record->record_de_list[j + 1], PBUF_RAM)) == NULL)
							{
								LWIP_DEBUGF(SDP_DEBUG, ("couldn't alloc pbuf\n"));
								return NULL;
							}

							memcpy((u8_t *)r->payload, record->record_de_list + j + 2, r->len);
							attribl_bc += r->len;

							/* If request included a service record handle attribute id, add the correct
							 * id to the response */
							if(rec_id == 0x0000) {
								memcpy(((u8_t *)r->payload) + 4, &hdl, 4);
							}

							/* Add the attribute to the service attribute list */
							if(s == NULL) {
								s = r;
							} else {
								pbuf_chain(s, r);
								pbuf_free(r);
							}
						}
					}
				}
			} /* for */
		} /* while */
	} else {
		/* ERROR: Invalid req syntax */
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_attribute_search: Req not a data element list <255 bytes"));
	}
	/* Return service attribute list */
	if(s != NULL) {
		q = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM);
		((u8_t *)q->payload)[0] = SDP_DE_TYPE_DES | SDP_DE_SIZE_N1;
		((u8_t *)q->payload)[1] = s->tot_len;
		pbuf_chain(q, s);
		pbuf_free(s);
	}

	return q;
}

/*
 * SDP CLIENT API.
 */


/* 
 * sdp_new():
 * 
 * Creates a new SDP protocol control block but doesn't place it on
 * any of the SDP PCB lists.
 */
struct sdp_pcb * sdp_new(struct l2cap_pcb *l2cappcb)
{
	struct sdp_pcb *pcb;

	pcb = lwbt_memp_malloc(MEMP_SDP_PCB);
	if(pcb != NULL) {
		memset(pcb, 0, sizeof(struct sdp_pcb));
		pcb->l2cappcb = l2cappcb;
		return pcb;
	}
	return NULL;
}

/* 
 * sdp_free():
 * 
 * Free the SDP protocol control block.
 */
void sdp_free(struct sdp_pcb *pcb) 
{
	lwbt_memp_free(MEMP_SDP_PCB, pcb);
	pcb = NULL;
}

/* 
 * sdp_reset_all():
 * 
 * Free all SDP protocol control blocks and registered records.
 */
void sdp_reset_all(void) 
{
	struct sdp_pcb *pcb, *tpcb;
	struct sdp_record *record, *trecord;

	for(pcb = sdp_pcbs; pcb != NULL;) {
		tpcb = pcb->next;
		SDP_RMV(&sdp_pcbs, pcb);
		sdp_free(pcb);
		pcb = tpcb;
	}

	for(record = sdp_server_records; record != NULL;) {
		trecord = record->next;
		sdp_unregister_service(record);
		sdp_record_free(record);
		record = trecord;
	}

	sdp_init();
}

/* 
 * sdp_arg():
 *
 * Used to specify the argument that should be passed callback functions.
 */
void sdp_arg(struct sdp_pcb *pcb, void *arg)
{
	pcb->callback_arg = arg;
}

/* 
 * sdp_lp_disconnected():
 *
 * Called by the application to indicate that the lower protocol disconnected.
 */
void sdp_lp_disconnected(struct l2cap_pcb *l2cappcb)
{
	struct sdp_pcb *pcb, *tpcb;

	pcb = sdp_pcbs;
	while(pcb != NULL) {
		tpcb = pcb->next;
		if(bd_addr_cmp(&(l2cappcb->remote_bdaddr), &(pcb->l2cappcb->remote_bdaddr))) {
			/* We do not need to notify upper layer, free PCB */
			sdp_free(pcb);
		}
		pcb = tpcb;
	}
}

/*
 * sdp_service_search_req():
 * 
 * Sends a request to a SDP server to locate service records that match the service 
 * search pattern.
 */
err_t sdp_service_search_req(struct sdp_pcb *pcb, u8_t *ssp, u8_t ssplen,
		u16_t max_src,
		void (* service_searched)(void *arg, struct sdp_pcb *pcb, u16_t tot_src,
			u16_t curr_src, u32_t *rhdls)) 
{
	struct pbuf *p;
	struct sdp_hdr *sdphdr;

	/* Update PCB */
	pcb->tid = sdp_next_transid(); /* Set transaction id */

	/* Allocate packet for PDU hdr + service search pattern + max service record count +
	   continuation state */
	p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+ssplen+2+1, PBUF_RAM);
	sdphdr = p->payload;
	/* Add PDU header to packet */
	sdphdr->pdu = SDP_SS_PDU;
	sdphdr->id = htons(pcb->tid);
	sdphdr->len = htons(ssplen + 3); /* Seq descr + ServiceSearchPattern + MaxServiceRecCount + ContState */

	/* Add service search pattern to packet */
	memcpy(((u8_t *)p->payload) + SDP_PDUHDR_LEN, ssp, ssplen);

	/* Add maximum service record count to packet */
	*((u16_t *)(((u8_t *)p->payload) + ssplen + SDP_PDUHDR_LEN)) = htons(max_src);

	((u8_t *)p->payload)[SDP_PDUHDR_LEN+ssplen+2] = 0; /* No continuation */

	/* Update PCB */
	pcb->service_searched = service_searched; /* Set callback */
	SDP_REG(&sdp_pcbs, pcb); /* Register request */

	return l2ca_datawrite(pcb->l2cappcb, p);
}

/*
 * sdp_service_attrib_req():
 * 
 * Retrieves specified attribute values from a specific service record.
 */
err_t sdp_service_attrib_req(struct sdp_pcb *pcb, u32_t srhdl, u16_t max_abc,
		u8_t *attrids, u8_t attrlen,
		void (* attributes_recv)(void *arg, struct sdp_pcb *pcb,
			u16_t attribl_bc, struct pbuf *p))
{
	struct sdp_hdr *sdphdr;
	u8_t *payload;
	struct pbuf *p;

	LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_attrib_req"));

	/* Allocate packet for PDU hdr + service rec hdl + max attribute byte count +
	   attribute id data element sequense lenght  + continuation state */
	p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN + attrlen + 7, PBUF_RAM);

	/* Update PCB */
	pcb->tid = sdp_next_transid(); /* Set transaction id */

	/* Add PDU header to packet */
	sdphdr = p->payload;
	sdphdr->pdu = SDP_SA_PDU;
	sdphdr->id = htons(pcb->tid);
	sdphdr->len = htons((attrlen + 7)); /* Service rec hdl + Max attrib B count + Seq descr + Attribute sequence + ContState */

	payload = p->payload;

	/* Add service record handle to packet */
	*((u32_t *)(payload + SDP_PDUHDR_LEN)) = htonl(srhdl);

	/* Add maximum attribute count to packet */
	*((u16_t *)(payload + SDP_PDUHDR_LEN + 4)) = htons(max_abc);

	/* Add attribute id data element sequence to packet */
	memcpy(payload + SDP_PDUHDR_LEN + 6, attrids, attrlen);

	payload[SDP_PDUHDR_LEN + 6 + attrlen] = 0x00; /* No continuation */

	/* Update PCB */
	pcb->attributes_recv = attributes_recv; /* Set callback */
	SDP_REG(&sdp_pcbs, pcb); /* Register request */

	return l2ca_datawrite(pcb->l2cappcb, p);
}

/*
 * sdp_service_search_attrib_req():
 * 
 * Combines the capabilities of the SDP_ServiceSearchRequest and the 
 * SDP_ServiceAttributeRequest into a single request. Contains both a service search 
 * pattern and a list of attributes to be retrieved from service records that match 
 * the service search pattern.
 */
err_t sdp_service_search_attrib_req(struct sdp_pcb *pcb, u16_t max_abc,
		u8_t *ssp, u8_t ssplen, u8_t *attrids, u8_t attrlen,
		void (* attributes_searched)(void *arg, struct sdp_pcb *pcb,
			u16_t attribl_bc, struct pbuf *p))
{
	struct sdp_hdr *sdphdr;

	struct pbuf *p;
	u8_t *payload;
	u16_t pbuf_bc;

	/* Allocate packet for PDU hdr + service search pattern + max attribute byte count +
	   attribute id list + continuation state */
	p = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+ssplen+2+attrlen+1, PBUF_RAM);

	/* Update PCB */
	pcb->tid = sdp_next_transid(); /* Set transaction id */

	/* Add PDU header to packet */
	sdphdr = p->payload;
	sdphdr->pdu = SDP_SSA_PDU;
	sdphdr->id = htons(pcb->tid);
	sdphdr->len = htons(ssplen + 2 + attrlen + 1);

	pbuf_bc = SDP_PDUHDR_LEN;
	payload = (u8_t *)p->payload;

	/* Add service search pattern to packet */
	memcpy(((u8_t *)p->payload) + SDP_PDUHDR_LEN, ssp, ssplen);

	/* Add maximum attribute count to packet */
	*((u16_t *)(payload + SDP_PDUHDR_LEN + ssplen)) = htons(max_abc);

	/* Add attribute id data element sequence to packet */
	memcpy(payload + SDP_PDUHDR_LEN + ssplen + 2, attrids, attrlen);

	payload[SDP_PDUHDR_LEN + ssplen + 2 + attrlen] = 0x00; /* No continuation */

	pcb->attributes_searched = attributes_searched; /* Set callback */
	SDP_REG(&sdp_pcbs, pcb); /* Register request */

	return l2ca_datawrite(pcb->l2cappcb, p);
}

/*
 * SDP SERVER API.
 */


/*
 * sdp_service_search_rsp():
 * 
 * The SDP server sends a list of service record handles for service records that 
 * match the service search pattern given in the request.
 */
err_t sdp_service_search_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
	struct sdp_record *record;
	struct sdp_hdr *rsphdr;

	struct pbuf *q; /* response packet */
	struct pbuf *r; /* tmp buffer */

	u16_t max_src = 0;
	u16_t curr_src = 0;
	u16_t tot_src = 0;

	u8_t size = 0;

	err_t ret;

	if(SDP_DE_TYPE(((u8_t *)p->payload)[0]) == SDP_DE_TYPE_DES && 
			SDP_DE_SIZE(((u8_t *)p->payload)[0]) ==  SDP_DE_SIZE_N1) {
		/* Size of the search pattern must be in the next byte since only 
		   12 UUIDs are allowed in one pattern */
		size = ((u8_t *)p->payload)[1];

		/* Get maximum service record count that follows the service search pattern */
		max_src = ntohs(*((u16_t *)(((u8_t *)p->payload)+(2+size))));

		pbuf_header(p, -2);
	} else {
		//TODO: INVALID SYNTAX ERROR
	}

	/* Allocate header + Total service rec count + Current service rec count  */
	q  = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+4, PBUF_RAM);

	rsphdr = q->payload;
	rsphdr->pdu = SDP_SSR_PDU;
	rsphdr->id = reqhdr->id;

	for(record = sdp_server_records; record != NULL; record = record->next) {
		/* Check if service search pattern matches record */
		if(sdp_pattern_search(record, size, p)) {
			if(max_src > 0) {
				/* Add service record handle to packet */
				r = pbuf_alloc(PBUF_RAW, 4, PBUF_RAM);
				*((u32_t *)r->payload) = htonl(record->hdl);
				pbuf_chain(q, r);
				pbuf_free(r);
				--max_src;
				++curr_src;
			}
			++tot_src;
		}
	}

	/* Add continuation state to packet */
	r = pbuf_alloc(PBUF_RAW, 1, PBUF_RAM);
	((u8_t *)r->payload)[0] = 0x00;
	pbuf_chain(q, r);
	pbuf_free(r);

	/* Add paramenter length to header */
	rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);

	/* Add total service record count to packet */
	*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN)) = htons(tot_src);

	/* Add current service record count to packet */
	*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN + 2)) = htons(curr_src);


	{
		u16_t i;
		for(r = q; r != NULL; r = r->next) {
			for(i = 0; i < r->len; ++i) {
				LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_rsp: 0x%x\n", ((u8_t *)r->payload)[i]));
			}
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_rsp: STOP\n"));
		}
	}

	ret = l2ca_datawrite(pcb, q);
	pbuf_free(q);
	return ret;
}

/*
 * sdp_service_attrib_rsp():
 * 
 * Sends a response that contains a list of attributes (both attribute ID and 
 * attribute value) from the requested service record.
 */
err_t sdp_service_attrib_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
	struct sdp_record *record;
	struct sdp_hdr *rsphdr;

	struct pbuf *q;
	struct pbuf *r;

	u16_t max_attribl_bc = 0; /* Maximum attribute list byte count */

	err_t ret;

	/* Find record */
	for(record = sdp_server_records; record != NULL; record = record->next) {
		if(record->hdl == ntohl(*((u32_t *)p->payload))) {
			break;
		}
	} 
	if(record != NULL) { 
		/* Get maximum attribute byte count */
		max_attribl_bc = ntohs(((u16_t *)p->payload)[2]); 

		/* Allocate rsp packet header + Attribute list count */
		q  = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN+2, PBUF_RAM);
		rsphdr = q->payload;
		rsphdr->pdu = SDP_SAR_PDU;
		rsphdr->id = reqhdr->id;

		/* Search for attributes and add them to a pbuf */
		pbuf_header(p, -6);
		r = sdp_attribute_search(max_attribl_bc, p, record);
		if(r != NULL) {
			/* Add attribute list byte count length to header */
			*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN)) = htons(r->tot_len);
			pbuf_chain(q, r); /* Chain attribute id list for service to response packet */
			pbuf_free(r);
		} else {
			*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN)) = 0;
		}

		/* Add continuation state to packet */
		r = pbuf_alloc(PBUF_RAW, 1, PBUF_RAM);
		((u8_t *)r->payload)[0] = 0x00; //TODO: Is this correct?
		pbuf_chain(q, r);
		pbuf_free(r);

		/* Add paramenter length to header */
		rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);

		{
			u16_t i;
			for(r = q; r != NULL; r = r->next) {
				for(i = 0; i < r->len; ++i) {
					LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_attrib_rsp: 0x%02x\n", ((u8_t *)r->payload)[i]));
				}
				LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_attrib_rsp: STOP\n"));
			}
		}

		ret = l2ca_datawrite(pcb, q);
		pbuf_free(q);

		return ret;
	}
	//TODO: ERROR NO SERVICE RECORD MATCHING HANDLE FOUND
	return ERR_OK;
}

/*
 * sdp_service_search_attrib_rsp():
 * 
 * Sends a response that contains a list of attributes (both attribute ID and 
 * attribute value) from the service records that match the requested service
 * search pattern.
 */
err_t sdp_service_search_attrib_rsp(struct l2cap_pcb *pcb, struct pbuf *p, struct sdp_hdr *reqhdr)
{
	struct sdp_record *record;
	struct sdp_hdr *rsphdr;

	struct pbuf *q; /* response packet */
	struct pbuf *r = NULL; /* tmp buffer */
	struct pbuf *s = NULL; /* tmp buffer */

	err_t ret;

	u16_t max_attribl_bc = 0;
	u8_t size = 0;

	/* Get size of service search pattern */
	if(SDP_DE_TYPE(((u8_t *)p->payload)[0]) == SDP_DE_TYPE_DES && 
			SDP_DE_SIZE(((u8_t *)p->payload)[0]) ==  SDP_DE_SIZE_N1) {
		/* Size of the search pattern must be in the next byte since only 
		   12 UUIDs are allowed in one pattern */
		size = ((u8_t *)p->payload)[1];

		/* Get maximum attribute byte count that follows the service search pattern */
		max_attribl_bc = ntohs(*((u16_t *)(((u8_t *)p->payload)+(2+size))));

		pbuf_header(p, -2);
	} else {
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_attrib_rsp: invalid syntax error\n"));
		//TODO: INVALID SYNTAX ERROR
	}

	/* Allocate header + attribute list count */
	q = pbuf_alloc(PBUF_RAW, SDP_PDUHDR_LEN + 2, PBUF_RAM);

	rsphdr = q->payload;
	rsphdr->pdu = SDP_SSAR_PDU;
	rsphdr->id = reqhdr->id;

	for(record = sdp_server_records; record != NULL; record = record->next) {
		/* Check if service search pattern matches record */
		if(sdp_pattern_search(record, size, p)) {
			/* Search for attributes and add them to a pbuf */
			pbuf_header(p, -(size + 2));
			r = sdp_attribute_search(max_attribl_bc, p, record);
			if(r != NULL) {
				if(q->next == NULL) {
					if((s = pbuf_alloc(PBUF_RAW, 2, PBUF_RAM)) == NULL)
					{
						LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_attrib_rsp: couldn't alloc pbuf"));
						return ERR_MEM;
					}
					/* Chain attribute id list for service to response packet */
					pbuf_chain(q, s);
					pbuf_free(s);
				}
				/* Calculate remaining number of bytes of attribute data the
				 * server is to return in response to the request */
				max_attribl_bc -= r->tot_len; 
				/* Chain attribute id list for service to response packet */
				pbuf_chain(q, r);
				pbuf_free(r);
			}
			pbuf_header(p, size + 2);
		}
	}

	/* Add attribute list byte count length and length of all attribute lists
	 * in this PDU to packet */
	if(q->next != NULL ) {
		*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN)) = htons(q->tot_len - SDP_PDUHDR_LEN - 2);

		((u8_t *)q->next->payload)[0] = SDP_DES_SIZE8;
		((u8_t *)q->next->payload)[1] = q->tot_len - SDP_PDUHDR_LEN - 4;
	} else {
		*((u16_t *)(((u8_t *)q->payload) + SDP_PDUHDR_LEN)) = 0;
	}

	/* Add continuation state to packet */
	if((r = pbuf_alloc(PBUF_RAW, 1, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_attrib_rsp: error allocating new pbuf\n"));
		return ERR_MEM;
		//TODO: ERROR
	} else {
		((u8_t *)r->payload)[0] = 0x00; //TODO: Is this correct?
		pbuf_chain(q, r);
		pbuf_free(r);
	}

	/* Add paramenter length to header */
	rsphdr->len = htons(q->tot_len - SDP_PDUHDR_LEN);

#if SDP_DEBUG == LWIP_DBG_ON
	for(r = q; r != NULL; r = r->next) {
		u8_t i;
		for(i = 0; i < r->len; ++i) {
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_attrib_rsp: 0x%02x\n", ((u8_t *)r->payload)[i]));
		}
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_service_search_attrib_rsp: next pbuf\n"));
	}
#endif

	ret = l2ca_datawrite(pcb, q);
	pbuf_free(q);
	return ret; 
}

/* 
 * sdp_recv():
 * 
 * Called by the lower layer. Parses the header and handle the SDP message.
 */
err_t sdp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err)
{
	struct sdp_hdr *sdphdr;
	struct sdp_pcb *sdppcb;
	err_t ret = ERR_OK;
	u16_t i;
	struct pbuf *p, *q, *r;

	if(s->len != s->tot_len) {
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Fragmented packet received. Reassemble into one buffer\n"));
		if((p = pbuf_alloc(PBUF_RAW, s->tot_len, PBUF_RAM)) != NULL) {
			i = 0;
			for(r = s; r != NULL; r = r->next) {
				memcpy(((u8_t *)p->payload) + i, r->payload, r->len);
				i += r->len;
			}
			pbuf_free(s);
		} else {
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Could not allocate buffer for fragmented packet\n"));
			pbuf_free(s);
			return ERR_MEM; 
		}
	} else {
		p = s;
	}

	for(r = p; r != NULL; r = r->next) {
		for(i = 0; i < r->len; ++i) {
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: 0x%02x\n", ((u8_t *)r->payload)[i]));
		}
		LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: STOP\n"));
	}

	sdphdr = p->payload;
	pbuf_header(p, -SDP_PDUHDR_LEN);

	LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: pdu=%02x\n", sdphdr->pdu));

	switch(sdphdr->pdu) {
		case SDP_ERR_PDU:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Error response 0x%x\n", ntohs(*((u16_t *)p->payload))));
			pbuf_free(p);
			break;
		case SDP_SS_PDU: /* Client request */
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service search request\n"));
			ret = sdp_service_search_rsp(pcb, p, sdphdr);
			pbuf_free(p);
			break;
		case SDP_SSR_PDU: /* Server response */
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service search response\n"));
			/* Find the original request */
			for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next) {
				if(sdppcb->tid == ntohs(sdphdr->id)) {
					break; /* Found */
				} /* if */
			} /* for */
			if(sdppcb != NULL) {
				/* Unregister the request */
				SDP_RMV(&sdp_pcbs, sdppcb);
				/* Callback function for a service search response */
				SDP_ACTION_SERVICE_SEARCHED(sdppcb, ntohs(((u16_t *)p->payload)[0]), ntohs(((u16_t *)p->payload)[1]), ((u32_t *)p->payload) + 1);
			}
			pbuf_free(p);
			break;
		case SDP_SA_PDU:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service attribute request\n"));
			ret = sdp_service_attrib_rsp(pcb, p, sdphdr);
			pbuf_free(p);
			break;
		case SDP_SAR_PDU:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service attribute response\n"));
			/* Find the original request */
			for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next) {
				if(sdppcb->tid == ntohs(sdphdr->id)) {
					/* Unregister the request */
					SDP_RMV(&sdp_pcbs, sdppcb);
					/* If packet is divided into several pbufs we need to merge them */
					if(p->next != NULL) {
						r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
						i = 0;
						for(q = p; q != NULL; q = q->next) {
							memcpy(((u8_t *)r->payload)+i, q->payload, q->len);
							i += q->len;
						}
						pbuf_free(p);
						p = r;
					}
					i = *((u16_t *)p->payload);
					pbuf_header(p, -2);	
					/* Callback function for a service attribute response */
					SDP_ACTION_ATTRIB_RECV(sdppcb, i, p);
				} /* if */
			} /* for */
			pbuf_free(p);
			break;
		case SDP_SSA_PDU:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service search attribute request\n"));
			ret = sdp_service_search_attrib_rsp(pcb, p, sdphdr);
			pbuf_free(p);
			break;
		case SDP_SSAR_PDU:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: Service search attribute response\n"));
			/* Find the original request */
			for(sdppcb = sdp_pcbs; sdppcb != NULL; sdppcb = sdppcb->next) {
				if(sdppcb->tid == ntohs(sdphdr->id)) {
					/* Unregister the request */
					SDP_RMV(&sdp_pcbs, sdppcb);
					/* If packet is divided into several pbufs we need to merge them */
					if(p->next != NULL) {
						r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
						i = 0;
						for(q = p; q != NULL; q = q->next) {
							memcpy(((u8_t *)r->payload)+i, q->payload, q->len);
							i += q->len;
						}
						pbuf_free(p);
						p = r;
					}
					i = *((u16_t *)p->payload);
					pbuf_header(p, -2);
					/* Callback function for a service search attribute response */
					SDP_ACTION_ATTRIB_SEARCHED(sdppcb, i, p);
					break; /* Abort request search */
				} /* if */
			} /* for */
			pbuf_free(p);
			break;
		default:
			LWIP_DEBUGF(SDP_DEBUG, ("sdp_recv: syntax error\n"));
			ret = ERR_VAL;
			break;
	}
	return ret;
}

