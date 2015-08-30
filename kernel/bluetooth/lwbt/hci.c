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


/* hci.c
 *
 * Implementation of the Host Controller Interface (HCI). A command interface
 * to the baseband controller and link manager, and gives access to hardware
 * status and control registers.
 *
 */


#include "lwbt/lwbtopts.h"
#include "lwbt/phybusif.h"
#include "lwbt/hci.h"
#include "lwbt/l2cap.h"
#include "lwbt/lwbt_memp.h"
#include "lwip/debug.h"
#include "lwip/mem.h"

/* The HCI LINK lists. */
struct hci_link *hci_active_links;  /* List of all active HCI LINKs */
struct hci_link *hci_tmp_link;

struct hci_pcb *pcb;


/* 
 * hci_init():
 *
 * Initializes the HCI layer.
 */
err_t hci_init(void)
{
	if((pcb = lwbt_memp_malloc(MEMP_HCI_PCB)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_init: Could not allocate memory for pcb\n"));
		return ERR_MEM;
	}
	MEMSET(pcb, 0, sizeof(struct hci_pcb));

	/* Clear globals */
	hci_active_links = NULL;
	hci_tmp_link = NULL;
	return ERR_OK;
}

/* 
 * hci_new():
 *
 * Creates a new HCI link control block
 */
struct hci_link * hci_new(void)
{
	struct hci_link *link;

	link = lwbt_memp_malloc(MEMP_HCI_LINK);
	if(link != NULL) {
		MEMSET(link, 0, sizeof(struct hci_link));
		return link;
	}
	LWIP_DEBUGF(HCI_DEBUG, ("hci_new: Could not allocate memory for link\n"));
	return NULL;
}

/* 
 * hci_close():
 *
 * Close the link control block.
 */
err_t hci_close(struct hci_link *link)
{ 
#if RFCOMM_FLOW_QUEUEING
	if(link->p != NULL) {
		pbuf_free(link->p);
	}
#endif
	HCI_RMV(&(hci_active_links), link);
	lwbt_memp_free(MEMP_HCI_LINK, link);
	link = NULL;
	return ERR_OK;
}

/* 
 * hci_reset_all():
 *
 * Closes all active link control blocks.
 */
void hci_reset_all(void)
{ 
	struct hci_link *link, *tlink;
	struct hci_inq_res *ires, *tires;

	for(link = hci_active_links; link != NULL;) {
		tlink = link->next;
		hci_close(link);
		link = tlink;
	}
	hci_active_links = NULL;

	for(ires = pcb->ires; ires != NULL;) {
		tires = ires->next;
		lwbt_memp_free(MEMP_HCI_INQ, ires);
		ires = tires;
	}
	lwbt_memp_free(MEMP_HCI_PCB, pcb);

	hci_init();
}

/* 
 * hci_arg():
 *
 * Used to specify the argument that should be passed callback
 * functions.
 */
void hci_arg(void *arg)
{
	pcb->callback_arg = arg;
}

/* 
 * hci_cmd_complete():
 *
 * Used to specify the function that should be called when HCI has received a 
 * command complete event.
 */
void hci_cmd_complete(err_t (* cmd_complete)(void *arg, struct hci_pcb *pcb, u8_t ogf, u8_t ocf, u8_t result)) 
{
	pcb->cmd_complete = cmd_complete;
}

/* 
 * hci_pin_req():
 *
 * Used to specify the function that should be called when HCI has received a 
 * PIN code request event.
 */
void hci_pin_req(err_t (* pin_req)(void *arg, struct bd_addr *bdaddr))
{
	pcb->pin_req = pin_req;
}

/* 
 * hci_link_key_not():
 *
 * Used to specify the function that should be called when HCI has received a 
 * link key notification event.
 */
void hci_link_key_not(err_t (* link_key_not)(void *arg, struct bd_addr *bdaddr, u8_t *key))
{
	pcb->link_key_not = link_key_not;
}

/* 
 * hci_connection_complete():
 *
 * Used to specify the function that should be called when HCI has received a 
 * connection complete event.
 */
void hci_connection_complete(err_t (* conn_complete)(void *arg, struct bd_addr *bdaddr))
{
	pcb->conn_complete = conn_complete;
}

/* 
 * hci_wlp_complete():
 *
 * Used to specify the function that should be called when HCI has received a 
 * successful write link policy complete event.
 */
void hci_wlp_complete(err_t (* wlp_complete)(void *arg, struct bd_addr *bdaddr))
{
	pcb->wlp_complete = wlp_complete;
}

/* 
 * hci_get_link():
 *
 * Used to get the link structure for that represents an ACL connection.
 */
struct hci_link * hci_get_link(struct bd_addr *bdaddr)
{
	struct hci_link *link;

	for(link = hci_active_links; link != NULL; link = link->next) {
		if(bd_addr_cmp(&(link->bdaddr), bdaddr)) {
			break;
		}
	}
	return link;
}

/* 
 * hci_acl_input():
 *
 * Called by the physical bus interface. Handles host controller to host flow
 * control, finds a bluetooth address that correspond to the connection handle
 * and forward the packet to the L2CAP layer.
 */
void hci_acl_input(struct pbuf *p)
{
	struct hci_acl_hdr *aclhdr;
	struct hci_link *link;
	u16_t conhdl;

	pbuf_header(p, HCI_ACL_HDR_LEN);
	aclhdr = p->payload;
	pbuf_header(p, -HCI_ACL_HDR_LEN);

	conhdl = aclhdr->conhdl_pb_bc & 0x0FFF; /* Get the connection handle from the first
											   12 bits */
	if(pcb->flow) {
		//TODO: XXX??? DO WE SAVE NUMACL PACKETS COMPLETED IN LINKS LIST?? SHOULD WE CALL 
		//hci_host_num_comp_packets from the main loop when no data has been received from the 
		//serial port???
		--pcb->host_num_acl;
		if(pcb->host_num_acl == 0) {
			hci_host_num_comp_packets(conhdl, HCI_HOST_MAX_NUM_ACL);
			pcb->host_num_acl = HCI_HOST_MAX_NUM_ACL;
		}
	}

	for(link = hci_active_links; link != NULL; link = link->next) {
		if(link->conhdl == conhdl) {
			break;
		}
	}

	if(link != NULL) {
		if(aclhdr->len) {
			LWIP_DEBUGF(HCI_DEBUG, ("hci_acl_input: Forward ACL packet to higher layer p->tot_len = %d\n", p->tot_len));
			l2cap_input(p, &(link->bdaddr));
		} else {
			pbuf_free(p); /* If length of ACL packet is zero, we silently discard it */
		}
	} else {
		pbuf_free(p); /* If no acitve ACL link was found, we silently discard the packet */
	}
}

#if HCI_EV_DEBUG
char * hci_get_error_code(u8_t code) {
	switch(code) {
		case HCI_SUCCESS:
			return("Success");
		case HCI_UNKNOWN_HCI_COMMAND:
			return("Unknown HCI Command");
		case HCI_NO_CONNECTION:
			return("No Connection");
		case HCI_HW_FAILURE:
			return("Hardware Failure");
		case HCI_PAGE_TIMEOUT:
			return("Page Timeout");
		case HCI_AUTHENTICATION_FAILURE:
			return("Authentication Failure");
		case HCI_KEY_MISSING:
			return("Key Missing");
		case HCI_MEMORY_FULL:
			return("Memory Full");
		case HCI_CONN_TIMEOUT:
			return("Connection Timeout");
		case HCI_MAX_NUMBER_OF_CONNECTIONS:
			return("Max Number Of Connections");
		case HCI_MAX_NUMBER_OF_SCO_CONNECTIONS_TO_DEVICE:
			return("Max Number Of SCO Connections To A Device");
		case HCI_ACL_CONNECTION_EXISTS:
			return("ACL connection already exists");
		case HCI_COMMAND_DISSALLOWED:
			return("Command Disallowed");
		case HCI_HOST_REJECTED_DUE_TO_LIMITED_RESOURCES:
			return("Host Rejected due to limited resources");
		case HCI_HOST_REJECTED_DUE_TO_SECURITY_REASONS:
			return("Host Rejected due to security reasons");
		case HCI_HOST_REJECTED_DUE_TO_REMOTE_DEVICE_ONLY_PERSONAL_SERVICE:
			return("Host Rejected due to remote device is only a personal device");
		case HCI_HOST_TIMEOUT:
			return("Host Timeout");
		case HCI_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE:
			return("Unsupported Feature or Parameter Value");
		case HCI_INVALID_HCI_COMMAND_PARAMETERS:
			return("Invalid HCI Command Parameters");
		case HCI_OTHER_END_TERMINATED_CONN_USER_ENDED:
			return("Other End Terminated Connection: User Ended Connection");
		case HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES:
			return("Other End Terminated Connection: Low Resources");
		case HCI_OTHER_END_TERMINATED_CONN_ABOUT_TO_POWER_OFF:
			return("Other End Terminated Connection: About to Power Off");
		case HCI_CONN_TERMINATED_BY_LOCAL_HOST:
			return("Connection Terminated by Local Host");
		case HCI_REPETED_ATTEMPTS:
			return("Repeated Attempts");
		case HCI_PAIRING_NOT_ALLOWED:
			return("Pairing Not Allowed");
		case HCI_UNKNOWN_LMP_PDU:
			return("Unknown LMP PDU");
		case HCI_UNSUPPORTED_REMOTE_FEATURE:
			return("Unsupported Remote Feature");
		case HCI_SCO_OFFSET_REJECTED:
			return("SCO Offset Rejected");
		case HCI_SCO_INTERVAL_REJECTED:
			return("SCO Interval Rejected");
		case HCI_SCO_AIR_MODE_REJECTED:
			return("SCO Air Mode Rejected");
		case HCI_INVALID_LMP_PARAMETERS:
			return("Invalid LMP Parameters");
		case HCI_UNSPECIFIED_ERROR:
			return("Unspecified Error");
		case HCI_UNSUPPORTED_LMP_PARAMETER_VALUE:
			return("Unsupported LMP Parameter Value");
		case HCI_ROLE_CHANGE_NOT_ALLOWED:
			return("Role Change Not Allowed");
		case HCI_LMP_RESPONSE_TIMEOUT:
			return("LMP Response Timeout");
		case HCI_LMP_ERROR_TRANSACTION_COLLISION:
			return("LMP Error Transaction Collision");
		case HCI_LMP_PDU_NOT_ALLOWED:
			return("LMP PDU Not Allowed");
		case HCI_ENCRYPTION_MODE_NOT_ACCEPTABLE:
			return("Encryption Mode Not Acceptable");
		case HCI_UNIT_KEY_USED:
			return("Unit Key Used");
		case HCI_QOS_NOT_SUPPORTED:
			return("QoS is Not Supported");
		case HCI_INSTANT_PASSED:
			return("Instant Passed");
		case HCI_PAIRING_UNIT_KEY_NOT_SUPPORTED:
			return("Pairing with Unit Key Not Supported");
		default:
			return("Error code unknown");
	}
}
#else
u8_t * hci_get_error_code(u8_t code) 
{
	return 0;
}
#endif /* HCI_EV_DEBUG */

/* hci_event_input():
 *
 * Called by the physical bus interface. Parses the received event packet to
 * determine which event occurred and handles it.
 */
void hci_event_input(struct pbuf *p)
{
	struct hci_inq_res *inqres;
	struct hci_event_hdr *evhdr;
	struct hci_link *link;
	u8_t i, j;
	struct bd_addr *bdaddr;
	u8_t resp_offset;
	err_t ret;
	u8_t ocf, ogf;

	pbuf_header(p, HCI_EVENT_HDR_LEN);
	evhdr = p->payload;
	pbuf_header(p, -HCI_EVENT_HDR_LEN);
	LWIP_DEBUGF(HCI_EV_DEBUG, ("\n"));

	switch(evhdr->code) {
		case HCI_INQUIRY_COMPLETE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Inquiry complete, 0x%x %s\n", ((u8_t *)p->payload)[0], hci_get_error_code(((u8_t *)p->payload)[0])));
			HCI_EVENT_INQ_COMPLETE(pcb,((u8_t *)p->payload)[0],ret);
			break;
		case HCI_INQUIRY_RESULT:
			for(i=0;i<((u8_t *)p->payload)[0];i++) {
				resp_offset = i*14;
				LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Inquiry result %d\nBD_ADDR: 0x", i));
				for(i = 0; i < BD_ADDR_LEN; i++) {
					LWIP_DEBUGF(HCI_EV_DEBUG, ("%x",((u8_t *)p->payload)[1+resp_offset+i]));
				}
				LWIP_DEBUGF(HCI_EV_DEBUG, ("\n"));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Page_Scan_Rep_Mode: 0x%x\n",((u8_t *)p->payload)[7+resp_offset]));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Page_Scan_Per_Mode: 0x%x\n",((u8_t *)p->payload)[8+resp_offset]));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Page_Scan_Mode: 0x%x\n",((u8_t *)p->payload)[9+resp_offset]));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Class_of_Dev: 0x%x 0x%x 0x%x\n",((u8_t *)p->payload)[10+resp_offset],
							((u8_t *)p->payload)[11+resp_offset], ((u8_t *)p->payload)[12+resp_offset]));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Clock_Offset: 0x%x%x\n",((u8_t *)p->payload)[13+resp_offset],
							((u8_t *)p->payload)[14+resp_offset]));

				bdaddr = (void *)(((u8_t *)p->payload)+(1+resp_offset));
				if((inqres = lwbt_memp_malloc(MEMP_HCI_INQ)) != NULL) {
					bd_addr_set(&(inqres->bdaddr), bdaddr);
					inqres->psrm = ((u8_t *)p->payload)[7+resp_offset];
					inqres->psm = ((u8_t *)p->payload)[9+resp_offset];
					MEMCPY(inqres->cod, ((u8_t *)p->payload)+10+resp_offset, 3);
					inqres->co = *((u16_t *)(((u8_t *)p->payload)+13+resp_offset));
					HCI_REG(&(pcb->ires), inqres);
				} else {
					LWIP_DEBUGF(HCI_DEBUG, ("hci_event_input: Could not allocate memory for inquiry result\n"));
				}
			}
			break;
		case HCI_CONNECTION_COMPLETE:
			bdaddr = (void *)(((u8_t *)p->payload)+3); /* Get the Bluetooth address */
			link = hci_get_link(bdaddr);
			switch(((u8_t *)p->payload)[0]) {
				case HCI_SUCCESS:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Conn successfully completed\n"));
					if(link == NULL) {
						if((link = hci_new()) == NULL) {
							/* Could not allocate memory for link. Disconnect */
							LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Could not allocate memory for link. Disconnect\n"));
							hci_disconnect(bdaddr, HCI_OTHER_END_TERMINATED_CONN_LOW_RESOURCES);
							/* Notify L2CAP */
							lp_disconnect_ind(bdaddr);
							break;
						}
						bd_addr_set(&(link->bdaddr), bdaddr);
						link->conhdl = *((u16_t *)(((u8_t *)p->payload)+1)); 
						HCI_REG(&(hci_active_links), link);
						HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /* Allow applicaton to do optional configuration of link */
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Calling lp_connect_ind\n"));
						lp_connect_ind(&(link->bdaddr)); /* Notify L2CAP */
					} else {
						link->conhdl = *((u16_t *)(((u8_t *)p->payload)+1));
						HCI_EVENT_CONN_COMPLETE(pcb,bdaddr,ret); /* Allow applicaton to do optional configuration of link */
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Calling lp_connect_cfm\n"));
						lp_connect_cfm(bdaddr, ((u8_t *)p->payload)[10], ERR_OK); /* Notify L2CAP */
					}
					//TODO: MASTER SLAVE SWITCH??
					break;
				default:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Conn failed to complete, 0x%x %s\n"
								, ((u8_t *)p->payload)[0], hci_get_error_code(((u8_t *)p->payload)[0])));
					if(link != NULL) {
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Link exists. Notify upper layer\n"));
						hci_close(link);
						lp_connect_cfm(bdaddr, ((u8_t *)p->payload)[10], ERR_CONN);
					} else {
						/* silently discard */
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Silently discard. Link does not exist\n"));
					}
					break;
			} /* switch */
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Conn_hdl: 0x%x 0x%x\n", ((u8_t *)p->payload)[1], ((u8_t *)p->payload)[2]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("BD_ADDR: 0x"));
			for(j=0;j<BD_ADDR_LEN;j++) {
				LWIP_DEBUGF(HCI_EV_DEBUG, ("%x",((u8_t *)p->payload)[3+j]));
			}
			LWIP_DEBUGF(HCI_EV_DEBUG, ("\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Link_type: 0x%x\n",((u8_t *)p->payload)[9]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Encryption_Mode: 0x%x\n",((u8_t *)p->payload)[10]));
			break;
		case HCI_DISCONNECTION_COMPLETE:
			switch(((u8_t *)p->payload)[0]) {
				case HCI_SUCCESS:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Disconn has occurred\n"));
					for(link = hci_active_links; link != NULL; link = link->next) {
						if(link->conhdl == *((u16_t *)(((u8_t *)p->payload)+1))) {
							break; /* Found */
						}
					}
					if(link != NULL) {
						lp_disconnect_ind(&(link->bdaddr)); /* Notify upper layer */
						hci_close(link);
					}
					/* else silently discard */
					break;
				default:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Disconn failed to complete, 0x%x %s\n"
								, ((u8_t *)p->payload)[0], hci_get_error_code(((u8_t *)p->payload)[0])));
					return;
			}
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Conn_hdl: 0x%x%x\n", ((u8_t *)p->payload)[1], ((u8_t *)p->payload)[2]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Reason: 0x%x %s\n", ((u8_t *)p->payload)[3], hci_get_error_code(((u8_t *)p->payload)[3])));
			break;
		case HCI_ENCRYPTION_CHANGE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Encryption changed. Status = 0x%x, Encryption enable = 0x%x\n", ((u8_t *)p->payload)[0], ((u8_t *)p->payload)[3]));
			break;
		case HCI_QOS_SETUP_COMPLETE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: QOS setup complete result = 0x%x\n", ((u8_t *)p->payload)[0]));
			break;
		case HCI_COMMAND_COMPLETE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Command Complete\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Num_HCI_Command_Packets: 0x%x\n", ((u8_t *)p->payload)[0]));

			pcb->numcmd += ((u8_t *)p->payload)[0]; /* Add number of completed command packets to the
													   number of command packets that the BT module 
													   can buffer */
			pbuf_header(p, -1); /* Adjust payload pointer not to cover
								   Num_HCI_Command_Packets parameter */
			ocf = *((u16_t *)p->payload) & 0x03FF;
			ogf = *((u16_t *)p->payload) >> 10;

			LWIP_DEBUGF(HCI_EV_DEBUG, ("OCF == 0x%x OGF == 0x%x\n", ocf, ogf));

			pbuf_header(p, -2); /* Adjust payload pointer not to cover Command_Opcode
								   parameter */
			if(ogf == HCI_INFO_PARAM) {
				if(ocf == HCI_READ_BUFFER_SIZE) {
					if(((u8_t *)p->payload)[0] == HCI_SUCCESS) {
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Read_Buffer_Size command succeeded\n"));
						LWIP_DEBUGF(HCI_EV_DEBUG, ("HC_ACL_Data_Packet_Length: 0x%x%x\n", ((u8_t *)p->payload)[1], ((u8_t *)p->payload)[2]));
						LWIP_DEBUGF(HCI_EV_DEBUG, ("HC_SCO_Data_Packet_Length: 0x%x\n", ((u8_t *)p->payload)[3]));
						LWIP_DEBUGF(HCI_EV_DEBUG, ("HC_Total_Num_ACL_Data_Packets: %d\n", *((u16_t *)(((u8_t *)p->payload)+4))));
						pcb->maxsize = *((u16_t *)(((u8_t *)p->payload)+1)); /* Maximum size of an ACL packet 
																				that the BT module is able to 
																				accept */
						pcb->hc_num_acl = *((u16_t *)(((u8_t *)p->payload)+4)); /* Number of ACL packets that the 
																				   BT module can buffer */
						LWIP_DEBUGF(HCI_EV_DEBUG, ("HC_Total_Num_SCO_Data_Packets: 0x%x%x\n", ((u8_t *)p->payload)[6], ((u8_t *)p->payload)[7]));
					} else {
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Read_Buffer_Size command failed, 0x%x %s\n", ((u8_t *)p->payload)[0], hci_get_error_code(((u8_t *)p->payload)[0])));
						return;
					}
				}
				if(ocf == HCI_READ_BD_ADDR) {
					if(((u8_t *)p->payload)[0] == HCI_SUCCESS) {
						bdaddr = (void *)(((u8_t *)p->payload) + 1); /* Get the Bluetooth address */
						HCI_EVENT_RBD_COMPLETE(pcb, bdaddr, ret); /* Notify application.*/
					}
				}
			}
			if(ogf == HCI_HOST_C_N_BB && ocf == HCI_SET_HC_TO_H_FC) {
				if(((u8_t *)p->payload)[0] == HCI_SUCCESS) {
					pcb->flow = 1;
				}
			}
			if(ogf == HCI_LINK_POLICY) {
				if(ocf == HCI_W_LINK_POLICY) {
					if(((u8_t *)p->payload)[0] == HCI_SUCCESS) {
						LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Successful HCI_W_LINK_POLICY.\n"));
						for(link = hci_active_links; link != NULL; link = link->next) {
							if(link->conhdl == *((u16_t *)(((u8_t *)p->payload)+1))) {
								break;
							}
						}
						if(link == NULL) {
							LWIP_DEBUGF(HCI_DEBUG, ("hci_event_input: Connection does not exist\n"));
							return; /* Connection does not exist */ 
						}
						HCI_EVENT_WLP_COMPLETE(pcb, &link->bdaddr, ret); /* Notify application.*/
					} else {
						LWIP_DEBUGF(HCI_EV_DEBUG, ("Unsuccessful HCI_W_LINK_POLICY.\n"));
						return;
					}
				}
			}

			HCI_EVENT_CMD_COMPLETE(pcb,ogf,ocf,((u8_t *)p->payload)[0],ret);
			break;
		case HCI_COMMAND_STATUS:
			switch(((u8_t *)p->payload)[0]) {
				case HCI_SUCCESS:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Command Status\n"));
					break;
				default:
					LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Command failed, %s\n", hci_get_error_code(((u8_t *)p->payload)[0])));
					pbuf_header(p, -2); /* Adjust payload pointer not to cover
										   Num_HCI_Command_Packets and status
										   parameter */
					ocf = *((u16_t *)p->payload) & 0x03FF;
					ogf = *((u16_t *)p->payload) >> 10;
					pbuf_header(p, -2); /* Adjust payload pointer not to cover
										   Command_Opcode parameter */
					HCI_EVENT_CMD_COMPLETE(pcb,ogf,ocf,((u8_t *)p->payload)[0],ret);
					pbuf_header(p, 4);
					break;
			}
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Num_HCI_Command_Packets: 0x%x\n", ((u8_t *)p->payload)[1]));
			/* Add number of completed command packets to the number of command
			 * packets that the BT module can buffer */
			pcb->numcmd += ((u8_t *)p->payload)[1]; 
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Command_Opcode: 0x%x 0x%x\n", ((u8_t *)p->payload)[2], ((u8_t *)p->payload)[3]));
			break;
		case HCI_HARDWARE_ERROR:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Hardware Error\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Hardware_code: 0x%x\n\n", ((u8_t *)p->payload)[0]));
			hci_reset();
			//TODO: IS THIS FATAL??
			break; 
		case HCI_ROLE_CHANGE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Role change\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Status: 0x%x\n", ((u8_t *)p->payload)[0]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("New Role: 0x%x\n", ((u8_t *)p->payload)[7]));
			break;
		case HCI_NBR_OF_COMPLETED_PACKETS:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Number Of Completed Packets\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Number_of_Handles: 0x%x\n", ((u8_t *)p->payload)[0]));
			for(i=0;i<((u8_t *)p->payload)[0];i++) {
				resp_offset = i*4;
				LWIP_DEBUGF(HCI_EV_DEBUG, ("Conn_hdl: 0x%x%x\n", ((u8_t *)p->payload)[1+resp_offset], ((u8_t *)p->payload)[2+resp_offset]));
				LWIP_DEBUGF(HCI_EV_DEBUG, ("HC_Num_Of_Completed_Packets: 0x%x\n",*((u16_t *)(((u8_t *)p->payload)+3+resp_offset))));
				/* Add number of completed ACL packets to the number of ACL packets that the 
				   BT module can buffer */
				pcb->hc_num_acl += *((u16_t *)(((u8_t *)p->payload) + 3 + resp_offset));
#if HCI_FLOW_QUEUEING
				{
					u16_t conhdl = *((u16_t *)(((u8_t *)p->payload) + 1 + resp_offset));
					struct pbuf *q;
					for(link = hci_active_links; link != NULL; link = link->next) {
						if(link->conhdl == conhdl) {
							break;
						}
					}
					q = link->p;
					/* Queued packet present? */
					if (q != NULL) {
						/* NULL attached buffer immediately */
						link->p = NULL;
						LWIP_DEBUGF(RFCOMM_DEBUG, ("rfcomm_input: Sending queued packet.\n"));
						/* Send the queued packet */
						lp_acl_write(&link->bdaddr, q, link->len, link->pb); 
						/* Free the queued packet */
						pbuf_free(q);
					}
				}
#endif /* RFCOMM_FLOW_QUEUEING */	
			}
			break;
		case HCI_MODE_CHANGE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Mode change\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Status: 0x%x\n", ((u8_t *)p->payload)[0]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Conn_hdl: 0x%x\n", ((u16_t *)(((u8_t *)p->payload) + 1))[0]));
			break;
		case HCI_DATA_BUFFER_OVERFLOW:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Data Buffer Overflow\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Link_Type: 0x%x\n", ((u8_t *)p->payload)[0]));
			//TODO: IS THIS FATAL????
			break;
		case HCI_MAX_SLOTS_CHANGE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Max slots changed\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("Conn_hdl: 0x%x\n", ((u16_t *)p->payload)[0]));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("LMP max slots: 0x%x\n", ((u8_t *)p->payload)[2]));
			break; 
		case HCI_PIN_CODE_REQUEST:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Pin request\n"));
			bdaddr = (void *)((u8_t *)p->payload); /* Get the Bluetooth address */
			HCI_EVENT_PIN_REQ(pcb, bdaddr, ret); /* Notify application. If event is not registered, 
													send a negative reply */
			break;
		case HCI_LINK_KEY_NOTIFICATION:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Link key notification\n"));
			bdaddr = (void *)((u8_t *)p->payload); /* Get the Bluetooth address */
			LWIP_DEBUGF(HCI_EV_DEBUG, ("bdaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
						((u8_t *)p->payload)[5], ((u8_t *)p->payload)[4], ((u8_t *)p->payload)[3],
						((u8_t *)p->payload)[2],  ((u8_t *)p->payload)[1],  ((u8_t *)p->payload)[0]
						));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("key_type: %d\n", ((u8_t *)p->payload)[16+6]));

			HCI_EVENT_LINK_KEY_NOT(pcb, bdaddr, ((u8_t *)p->payload) + 6, ret); /* Notify application.*/
			break;
		case HCI_PAGE_SCAN_REP_MODE_CHANGE:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Page Scan Repetition Mode changed\n"));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("bdaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
						((u8_t *)p->payload)[5], ((u8_t *)p->payload)[4], ((u8_t *)p->payload)[3],
						((u8_t *)p->payload)[2],  ((u8_t *)p->payload)[1],  ((u8_t *)p->payload)[0]
						));
			LWIP_DEBUGF(HCI_EV_DEBUG, ("rep mode: %d\n", ((u8_t *)p->payload)[6]));
			break;
		default:
			LWIP_DEBUGF(HCI_EV_DEBUG, ("hci_event_input: Undefined event code 0x%x\n", evhdr->code));
			break;
	}/* switch */
}

/* HCI Commands */ 


/* hci_cmd_ass():
 *
 * Assemble the command header.
 */
struct pbuf * hci_cmd_ass(struct pbuf *p, u8_t ocf, u8_t ogf, u8_t len) 
{
	((u8_t *)p->payload)[0] = HCI_COMMAND_DATA_PACKET; /* cmd packet type */
	((u8_t *)p->payload)[1] = (ocf & 0xff); /* OCF & OGF */
	((u8_t *)p->payload)[2] = (ocf >> 8)|(ogf << 2);
	((u8_t *)p->payload)[3] = len-HCI_CMD_HDR_LEN-1; /* Param len = plen - cmd hdr - ptype */
	if(pcb->numcmd != 0) {
		--pcb->numcmd; /* Reduce number of cmd packets that the host controller can buffer */
	}
	return p;
}

/* hci_inquiry():
 *
 * Cause the Host contoller to enter inquiry mode to discovery other nearby
 * Bluetooth devices.
 */
err_t hci_inquiry(u32_t lap, u8_t inq_len, u8_t num_resp, err_t (* inq_complete)(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires, u16_t result))
{
	struct pbuf *p;
	struct hci_inq_res *tmpres;

	/* Free any previous inquiry result list */
	while(pcb->ires != NULL) {
		tmpres = pcb->ires;
		pcb->ires = pcb->ires->next;
		lwbt_memp_free(MEMP_HCI_INQ, tmpres);
	}

	pcb->inq_complete = inq_complete;

	if((p = pbuf_alloc(PBUF_RAW, HCI_INQUIRY_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_inquiry: Could not allocate memory for pbuf\n"));
		return ERR_MEM; /* Could not allocate memory for pbuf */
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_INQUIRY_OCF, HCI_LINK_CTRL_OGF, HCI_INQUIRY_PLEN);
	/* Assembling cmd prameters */
	((u8_t *)p->payload)[4] = lap & 0xFF;
	((u8_t *)p->payload)[5] = lap >> 8;
	((u8_t *)p->payload)[6] = lap >> 16;
	//MEMCPY(((u8_t *)p->payload)+4, inqres->cod, 3);
	((u8_t *)p->payload)[7] = inq_len;
	((u8_t *)p->payload)[8] = num_resp;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_disconnect():
 *
 * Used to terminate an existing connection.
 */
err_t hci_disconnect(struct bd_addr *bdaddr, u8_t reason)
{
	struct pbuf *p;
	struct hci_link *link;

	link = hci_get_link(bdaddr);

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_disconnect: Connection does not exist\n"));
		return ERR_CONN; /* Connection does not exist */ 
	}
	if((p = pbuf_alloc(PBUF_RAW, HCI_DISCONN_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_disconnect: Could not allocate memory for pbuf\n"));
		return ERR_MEM; /* Could not allocate memory for pbuf */
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_DISCONN_OCF, HCI_LINK_CTRL_OGF, HCI_DISCONN_PLEN);

	/* Assembling cmd prameters */
	((u16_t *)p->payload)[2] = link->conhdl;
	((u8_t *)p->payload)[6] = reason;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_reject_connection_request():
 *
 * Used to decline a new incoming connection request.
 */
err_t hci_reject_connection_request(struct bd_addr *bdaddr, u8_t reason)
{
	struct pbuf *p;
	LWIP_DEBUGF(HCI_DEBUG, ("hci_reject_connection_request: reject from %02x:%02x:%02x:%02x:%02x:%02x\n",
		bdaddr->addr[5], bdaddr->addr[4], bdaddr->addr[3],
		bdaddr->addr[2], bdaddr->addr[1], bdaddr->addr[0] ));
	if((p = pbuf_alloc(PBUF_RAW, HCI_REJECT_CONN_REQ_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_reject_connection_request: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_REJECT_CONN_REQ_OCF, HCI_LINK_CTRL_OGF, HCI_REJECT_CONN_REQ_PLEN);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload) + 4, bdaddr->addr, 6);
	((u8_t *)p->payload)[10] = reason;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_pin_code_request_reply():
 *
 * Used to reply to a PIN Code Request event from the Host Controller and
 * specifies the PIN code to use for a connection.
 */
err_t hci_pin_code_request_reply(struct bd_addr *bdaddr, u8_t pinlen, u8_t *pincode)
{
	struct pbuf *p;

	if((p = pbuf_alloc(PBUF_RAW, HCI_PIN_CODE_REQ_REP_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_pin_code_request_reply: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Reset buffer content just to make sure */
	MEMSET((u8_t *)p->payload, 0, HCI_PIN_CODE_REQ_REP_PLEN);

	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_PIN_CODE_REQ_REP, HCI_LINK_CTRL_OGF, HCI_PIN_CODE_REQ_REP_PLEN);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload) + 4, bdaddr->addr, 6);
	((u8_t *)p->payload)[10] = pinlen;
	MEMCPY(((u8_t *)p->payload) + 11, pincode, pinlen);

	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_pin_code_request_neg_reply():
 *
 * Used to reply to a PIN Code Request event from the Host Controller when the
 * Host cannot specify a PIN code to use for a connection.
 */
err_t hci_pin_code_request_neg_reply(struct bd_addr *bdaddr)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_PIN_CODE_REQ_NEG_REP_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_pin_code_request_neg_reply: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_PIN_CODE_REQ_NEG_REP, HCI_LINK_CTRL_OGF, HCI_PIN_CODE_REQ_NEG_REP_PLEN);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload)+4, bdaddr->addr, 6);
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_sniff_mode():
 *
 * Sets an ACL connection to low power Sniff mode.
 */
err_t hci_sniff_mode(struct bd_addr *bdaddr, u16_t max_interval, u16_t min_interval, u16_t attempt, u16_t timeout)
{
	struct pbuf *p;
	struct hci_link *link;

	/* Check if an ACL connection exists */ 
	link = hci_get_link(bdaddr);

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_sniff_mode: ACL connection does not exist\n"));
		return ERR_CONN;
	}

	if((p = pbuf_alloc(PBUF_TRANSPORT, HCI_SNIFF_PLEN, PBUF_RAM)) == NULL) { /* Alloc len of packet */
		LWIP_DEBUGF(HCI_DEBUG, ("hci_sniff_mode: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_SNIFF_MODE, HCI_LINK_POLICY, HCI_SNIFF_PLEN);
	/* Assembling cmd prameters */
	((u16_t *)p->payload)[2] = link->conhdl;
	((u16_t *)p->payload)[3] = max_interval;
	((u16_t *)p->payload)[4] = min_interval;
	((u16_t *)p->payload)[5] = attempt;
	((u16_t *)p->payload)[6] = timeout;

	phybusif_output(p, p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/* hci_write_link_policy_settings():
 *
 * Control the modes (park, sniff, hold) that an ACL connection can take.
 *
 */
err_t hci_write_link_policy_settings(struct bd_addr *bdaddr, u16_t link_policy)
{
	struct pbuf *p;
	struct hci_link *link;

	/* Check if an ACL connection exists */ 
	link = hci_get_link(bdaddr);

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_link_policy_settings: ACL connection does not exist\n"));
		return ERR_CONN;
	}

	if( (p = pbuf_alloc(PBUF_TRANSPORT, HCI_W_LINK_POLICY_PLEN, PBUF_RAM)) == NULL) { /* Alloc len of packet */
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_link_policy_settings: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_W_LINK_POLICY, HCI_LINK_POLICY, HCI_W_LINK_POLICY_PLEN);

	/* Assembling cmd prameters */
	((u16_t *)p->payload)[2] = link->conhdl;
	((u16_t *)p->payload)[3] = link_policy;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/* hci_reset():
 *
 * Reset the Bluetooth host controller, link manager, and radio module.
 */
err_t hci_reset(void)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_RESET_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_reset: Could not allocate memory for pbuf\n")); 
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_RESET_OCF, HCI_HC_BB_OGF, HCI_RESET_PLEN);
	/* Assembling cmd prameters */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_set_event_filter():
 *
 * Used by the host to specify different event filters.
 */
err_t hci_set_event_filter(u8_t filter_type, u8_t filter_cond_type, u8_t* cond)
{
	u8_t cond_len = 0x00;
	struct pbuf *p;
	LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: filter_type=%d cond_type=%d\n", filter_type, filter_cond_type));
	switch(filter_type) {
		case HCI_SET_EV_FILTER_CLEAR:
			LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: Clearing all filters\n"));
			cond_len = 0x00;
			break;
		case HCI_SET_EV_FILTER_INQUIRY:
			switch(filter_cond_type) {
				case HCI_SET_EV_FILTER_ALLDEV:
					cond_len = 0x00;
					break;
				case HCI_SET_EV_FILTER_COD:
					cond_len = 0x06;
					break;
				case HCI_SET_EV_FILTER_BDADDR:
					cond_len = 0x06;
					break;
				default:
					LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: Entered an unspecified filter condition type!\n"));
					break;
			}
			break;
		case HCI_SET_EV_FILTER_CONNECTION:
			switch(filter_cond_type) {
				case HCI_SET_EV_FILTER_ALLDEV:
					cond_len = 0x01;
					break;
				case HCI_SET_EV_FILTER_COD:
					cond_len = 0x07;
					break;
				case HCI_SET_EV_FILTER_BDADDR:
					cond_len = 0x07;
					break;
				default:
					LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: Entered an unspecified filter condition type!\n"));
					break;
			}
			break;
		default:
			LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: Entered an unspecified filter type!\n"));
			break;
	}
	if((p = pbuf_alloc(PBUF_RAW, HCI_SET_EV_FILTER_PLEN+cond_len, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_set_event_filter: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_SET_EV_FILTER_OCF, HCI_HC_BB_OGF, HCI_SET_EV_FILTER_PLEN+cond_len);
	((u8_t *)p->payload)[4] = filter_type;
	((u8_t *)p->payload)[5] = filter_cond_type;
	/* Assembling cmd prameters */
	if(cond_len) {
		MEMCPY(((u8_t *)p->payload)+6, cond, cond_len);
	}
	phybusif_output(p, p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/* hci_write_stored_link_key():
 *
 * Writes a link key to be stored in the Bluetooth host controller.
 */
err_t hci_write_stored_link_key(struct bd_addr *bdaddr, u8_t *link)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_WRITE_STORED_LINK_KEY_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_stored_link_key: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_WRITE_STORED_LINK_KEY, HCI_HC_BB_OGF, HCI_WRITE_STORED_LINK_KEY_PLEN);
	/* Assembling cmd prameters */
	((u8_t *)p->payload)[4] = 0x01;
	MEMCPY(((u8_t *)p->payload) + 5, bdaddr->addr, 6);
	MEMCPY(((u8_t *)p->payload) + 11, link, 16);
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_change_local_name():
 *
 * Writes a link key to be stored in the Bluetooth host controller.
 */
err_t hci_change_local_name(u8_t *name, u8_t len)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_CHANGE_LOCAL_NAME_PLEN + len, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_change_local_name: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_CHANGE_LOCAL_NAME, HCI_HC_BB_OGF, HCI_CHANGE_LOCAL_NAME_PLEN + len);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload) + 4, name, len);
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_write_page_timeout():
 *
 * Define the amount of time a connection request will wait for the remote device
 * to respond before the local device returns a connection failure.
 */
err_t hci_write_page_timeout(u16_t page_timeout)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_W_PAGE_TIMEOUT_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_page_timeout: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_W_PAGE_TIMEOUT_OCF, HCI_HC_BB_OGF, HCI_W_PAGE_TIMEOUT_PLEN);
	/* Assembling cmd prameters */
	((u16_t *)p->payload)[2] = page_timeout;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_write_scan_enable():
 *
 * Controls whether or not the Bluetooth device will periodically scan for page 
 * attempts and/or inquiry requests from other Bluetooth devices.
 */
err_t hci_write_scan_enable(u8_t scan_enable)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_W_SCAN_EN_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_scan_enable: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_W_SCAN_EN_OCF, HCI_HC_BB_OGF, HCI_W_SCAN_EN_PLEN);
	/* Assembling cmd prameters */
	((u8_t *)p->payload)[4] = scan_enable;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_write_cod():
 *
 * Write the value for the Class_of_Device parameter, which is used to indicate
 * its capabilities to other devices.
 */
err_t hci_write_cod(u8_t *cod)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_W_COD_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_write_cod: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_W_COD_OCF, HCI_HC_BB_OGF, HCI_W_COD_PLEN);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload)+4, cod, 3);
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_set_hc_to_h_fc():
 *
 * Used by the Host to turn flow control on or off in the direction from the Host 
 * Controller to the Host.
 */
err_t hci_set_hc_to_h_fc(void)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_SET_HC_TO_H_FC_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_set_hc_to_h_fc: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_SET_HC_TO_H_FC_OCF, HCI_HC_BB_OGF, HCI_SET_HC_TO_H_FC_PLEN);
	/* Assembling cmd prameters */
	((u8_t *)p->payload)[4] = 0x01; /* Flow control on for HCI ACL Data Packets and off for HCI 
									   SCO Data Packets in direction from Host Controller to 
									   Host */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_host_buffer_size():
 *
 * Used by the Host to notify the Host Controller about the maximum size of the
 * data portion of HCI ACL Data Packets sent from the Host Controller to the Host.
 */
err_t hci_host_buffer_size(void)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_H_BUF_SIZE_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_host_buffer_size: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_H_BUF_SIZE_OCF, HCI_HC_BB_OGF, HCI_H_BUF_SIZE_PLEN); 
	((u16_t *)p->payload)[2] = HCI_HOST_ACL_MAX_LEN; /* Host ACL data packet maximum length */
	((u8_t *)p->payload)[6] = 255; /* Host SCO Data Packet Length */
	*((u16_t *)(((u8_t *)p->payload)+7)) = HCI_HOST_MAX_NUM_ACL; /* Host max total num ACL data packets */
	((u16_t *)p->payload)[4] = 1; /* Host Total Num SCO Data Packets */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	pcb->host_num_acl = HCI_HOST_MAX_NUM_ACL;

	return ERR_OK;
}

/* hci_host_num_comp_packets():
 *
 * Used by the Host to indicate to the Host Controller the number of HCI Data
 * Packets that have been completed for each Connection Handle since the previous 
 * Host_Number_Of_Completed_Packets command was sent to the Host Controller.
 */
err_t hci_host_num_comp_packets(u16_t conhdl, u16_t num_complete)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_H_NUM_COMPL_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_host_num_comp_packets: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_H_NUM_COMPL_OCF, HCI_HC_BB_OGF, HCI_H_NUM_COMPL_PLEN); 
	((u16_t *)p->payload)[2] = conhdl;
	((u16_t *)p->payload)[3] = num_complete; /* Number of completed acl packets */

	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	pcb->host_num_acl += num_complete;

	return ERR_OK;
}

/* hci_read_buffer_size():
 *
 * Used to read the maximum size of the data portion of HCI ACL packets sent from the 
 * Host to the Host Controller.
 */
err_t hci_read_buffer_size(void)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_R_BUF_SIZE_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_read_buffer_size: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_R_BUF_SIZE_OCF, HCI_INFO_PARAM_OGF, HCI_R_BUF_SIZE_PLEN);
	/* Assembling cmd prameters */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_read_local_features()
 *
 * Read the features of the connected module
 */
err_t hci_read_local_features(void)
{
	struct pbuf *p;
	if((p = pbuf_alloc(PBUF_RAW, HCI_R_BUF_SIZE_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_read_buffer_size: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_R_SUPPORTED_LOCAL_FEATURES_OCF, HCI_INFO_PARAM_OGF, HCI_R_SUPPORTED_LOCAL_FEATURES_PLEN);
	/* Assembling cmd prameters */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* hci_read_bd_addr():
 *
 * Used to retreive the Bluetooth address of the host controller.
 */
err_t hci_read_bd_addr(err_t (* rbd_complete)(void *arg, struct bd_addr *bdaddr))
{
	struct pbuf *p;

	pcb->rbd_complete = rbd_complete;

	if((p = pbuf_alloc(PBUF_RAW, HCI_R_BD_ADDR_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("hci_read_buffer_size: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	} 
	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_READ_BD_ADDR, HCI_INFO_PARAM_OGF, HCI_R_BD_ADDR_PLEN);
	/* Assembling cmd prameters */
	phybusif_output(p, p->tot_len);
	pbuf_free(p);

	return ERR_OK;
}

/* lp_write_flush_timeout():
 *
 * Called by L2CAP to set the flush timeout for the ACL link.
 */
err_t lp_write_flush_timeout(struct bd_addr *bdaddr, u16_t flushto)
{
	struct hci_link *link;
	struct pbuf *p;

	/* Check if an ACL connection exists */ 
	link = hci_get_link(bdaddr);

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("lp_write_flush_timeout: ACL connection does not exist\n"));
		return ERR_CONN;
	}

	if((p = pbuf_alloc(PBUF_TRANSPORT, HCI_W_FLUSHTO_PLEN, PBUF_RAM)) == NULL) { /* Alloc len of packet */
		LWIP_DEBUGF(HCI_DEBUG, ("lp_write_flush_timeout: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}

	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_W_FLUSHTO, HCI_HC_BB_OGF, HCI_W_FLUSHTO_PLEN);
	/* Assembling cmd prameters */
	((u16_t *)p->payload)[2] = link->conhdl;
	((u16_t *)p->payload)[3] = flushto;

	phybusif_output(p, p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/* lp_connect_req():
 *
 * Called by L2CAP to cause the Link Manager to create a connection to the 
 * Bluetooth device with the BD_ADDR specified by the command parameters.
 */
err_t lp_connect_req(struct bd_addr *bdaddr, u8_t allow_role_switch)
{
	u8_t page_scan_repetition_mode, page_scan_mode;
	u16_t clock_offset;
	struct pbuf *p;
	struct hci_link *link = hci_new();
	struct hci_inq_res *inqres;

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("lp_connect_req: Could not allocate memory for link\n")); 
		return ERR_MEM; /* Could not allocate memory for link */
	}

	bd_addr_set(&(link->bdaddr), bdaddr);
	HCI_REG(&(hci_active_links), link);


	/* Check if module has been discovered in a recent inquiry */
	for(inqres = pcb->ires; inqres != NULL; inqres = inqres->next) {
		if(bd_addr_cmp(&inqres->bdaddr, bdaddr)) {
			page_scan_repetition_mode = inqres->psrm;
			page_scan_mode = inqres->psm;
			clock_offset = inqres->co;
			break;
		}
	}
	if(inqres == NULL) {
		/* No information on parameters from an inquiry. Using default values */
		page_scan_repetition_mode = 0x01; /* Assuming worst case: time between
											 successive page scans starting 
											 <= 2.56s */
		page_scan_mode = 0x00; /* Assumes the device uses mandatory scanning, most
								  devices use this. If no conn is established, try
								  again w this parm set to optional page scanning */
		clock_offset = 0x00; /* If the device was not found in a recent inquiry
								this  information is irrelevant */
	}    

	if((p = pbuf_alloc(PBUF_RAW, HCI_CREATE_CONN_PLEN, PBUF_RAM)) == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("lp_connect_req: Could not allocate memory for pbuf\n"));
		return ERR_MEM; /* Could not allocate memory for pbuf */
	}

	/* Assembling command packet */
	p = hci_cmd_ass(p, HCI_CREATE_CONN_OCF, HCI_LINK_CTRL_OGF, HCI_CREATE_CONN_PLEN);
	/* Assembling cmd prameters */
	MEMCPY(((u8_t *)p->payload)+4, bdaddr->addr, 6);

	((u16_t *)p->payload)[5] = HCI_PACKET_TYPE;
	((u8_t *)p->payload)[12] = page_scan_repetition_mode;
	((u8_t *)p->payload)[13] = page_scan_mode;
	((u16_t *)p->payload)[7] = clock_offset;
	((u8_t *)p->payload)[16] = allow_role_switch;
	phybusif_output(p, p->tot_len);
	pbuf_free(p);
	return ERR_OK;
}

/* lp_acl_write():
 *
 * Called by L2CAP to send data to the Host Controller that will be transfered over
 * the ACL link from there.
 */
err_t lp_acl_write(struct bd_addr *bdaddr, struct pbuf *p, u16_t len, u8_t pb)
{
	struct hci_link *link;
	static struct hci_acl_hdr *aclhdr;
	struct pbuf *q;

	/* Check if an ACL connection exists */ 
	link = hci_get_link(bdaddr);

	if(link == NULL) {
		LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: ACL connection does not exist\n"));
		return ERR_CONN;
	}

	LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: HC num ACL %d\n", pcb->hc_num_acl));
	if(pcb->hc_num_acl == 0) {
		LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: HC out of buffer space\n"));
#if HCI_FLOW_QUEUEING
		if(p != NULL) {
			/* Packet can be queued? */
			if(link->p != NULL) {
				LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: Host buffer full. Dropped packet\n"));
				return ERR_OK; /* Drop packet */
			} else {
				/* Copy PBUF_REF referenced payloads into PBUF_RAM */
				p = pbuf_take(p);
				/* Remember pbuf to queue, if any */
				link->p = p;
				link->len = len;
				link->pb = pb;
				/* Pbufs are queued, increase the reference count */
				pbuf_ref(p);
				LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: Host queued packet %p\n", (void *)p));
			}
		}
#else
		LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: Dropped packet\n"));
#endif
		return ERR_OK;
	}

	if((q = pbuf_alloc(PBUF_RAW, 1+HCI_ACL_HDR_LEN, PBUF_RAM)) == NULL) {
		/* Could not allocate memory for pbuf */
		LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: Could not allocate memory for pbuf\n"));
		return ERR_MEM;
	}
	pbuf_chain(q, p);

	((u8_t*)q->payload)[0] = HCI_ACL_DATA_PACKET;
	aclhdr = (void *)(((u8_t*)q->payload)+1);
	aclhdr->conhdl_pb_bc = link->conhdl; /* Received from connection complete event */
	aclhdr->conhdl_pb_bc |= pb << 12; /* Packet boundary flag */
	aclhdr->conhdl_pb_bc &= 0x3FFF; /* Point-to-point */
	aclhdr->len = len;

	LWIP_DEBUGF(HCI_DEBUG, ("lp_acl_write: q->tot_len = %d aclhdr->len + q->len = %d\n", q->tot_len, aclhdr->len + q->len));

	phybusif_output(q, aclhdr->len + q->len);

	--pcb->hc_num_acl;

	/* Free ACL header. Upper layers will handle rest of packet */
	p = pbuf_dechain(q);
	pbuf_free(q);
	return ERR_OK;
}

/* lp_is_connected():
 *
 * Called by L2CAP to check if an active ACL connection exists for the specified 
 * Bluetooth address.
 */
u8_t lp_is_connected(struct bd_addr *bdaddr)
{
	struct hci_link *link;

	link = hci_get_link(bdaddr);

	if(link == NULL) {
		return 0;
	}
	return 1;
}

/* lp_pdu_maxsize():
 *
 * Called by L2CAP to check the maxsize of the PDU. In this case it is the largest
 * ACL packet that the Host Controller can buffer.
 */
u16_t lp_pdu_maxsize(void)
{
	return pcb->maxsize;
}


