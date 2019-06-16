//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpidns.c
//    Module Funciton           : 
//                                Deep Packet Inspection for DNS protocol.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <KAPI.H>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "dpimgr.h"
#include "dpidns.h"

/* Filter function of DNS's DPI. */
BOOL dpiFilter_DNS(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;

	pIpHdr = (struct ip_hdr*)p->payload;
	if (IP_PROTO_UDP != pIpHdr->_proto)
	{
		return FALSE;
	}
	return FALSE;
}

/* Action function of DNS's DPI. */
struct pbuf* dpiAction_DNS(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	/* Drop the packet. */
	return p;
}
