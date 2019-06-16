//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpiicmp.c
//    Module Funciton           : 
//                                Deep Packet Inspection for ICMP protocol.
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
#include "lwip/icmp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "dpimgr.h"
#include "dpiicmp.h"

/* Filter function of ICMP's DPI. */
BOOL dpiFilter_ICMP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;

	pIpHdr = (struct ip_hdr*)p->payload;
	if (IP_PROTO_ICMP == pIpHdr->_proto)
	{
		return TRUE;
	}
	return FALSE;
}

/* Action function of ICMP's DPI. */
struct pbuf* dpiAction_ICMP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;
	struct icmp_echo_hdr* pEchoHdr = NULL;
	char* pRawPkt = NULL;
	int iph_len = 0;
	BOOL bShouldDrop = FALSE;
	int echo_pkt_len = 0;
	ip_addr_t src, dst;

	/* Only apply for incoming ICMP packet. */
	if (dir == dpi_out)
	{
		goto __TERMINAL;
	}
	/* Validate the pbuf object. */
	if (p->tot_len < sizeof(struct ip_hdr))
	{
		goto __TERMINAL;
	}
	/* Get raw IP packet from pbuf. */
	pRawPkt = DPIManager.copy_from_pbuf(p);
	if (NULL == pRawPkt)
	{
		_hx_printf("%s:copy raw packet failed.\r\n", __func__);
		goto __TERMINAL;
	}
	pIpHdr = (struct ip_hdr*)pRawPkt;
	iph_len = IPH_HL(pIpHdr);
	iph_len *= 4;
	echo_pkt_len = (p->tot_len - iph_len);
	if (echo_pkt_len < sizeof(struct icmp_echo_hdr))
	{
		goto __TERMINAL;
	}
	/* Locate the echo header. */
	pEchoHdr = (struct icmp_echo_hdr*)(pRawPkt + iph_len);
	if (ICMP_ECHO == ICMPH_TYPE(pEchoHdr))
	{
		/* Just bounce it back. */
		pEchoHdr->type = ICMP_ER;
		pEchoHdr->chksum = 0;
		pEchoHdr->chksum = inet_chksum(pEchoHdr, echo_pkt_len);
		/* Change source and destination IP addr. */
		src.addr = pIpHdr->dest.addr;
		dst.addr = pIpHdr->src.addr;
		/* Send out raw IP packet directly,bypass the IP stack's process. */
		if(DPIManager.dpiSendRaw(&src,&dst,IP_PROTO_ICMP,64,0,pIpHdr->_id,(char*)pEchoHdr,echo_pkt_len))
		{
			bShouldDrop = TRUE;
		}
	}

__TERMINAL:
	if (pRawPkt)
	{
		_hx_free(pRawPkt);
	}
	if (bShouldDrop)
	{
		/* Drop the original packet. */
		return NULL;
	}
	else
	{
		/* Just let the packet go. */
		return p;
	}
}
