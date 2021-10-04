//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 23,2021
//    Module Name               : dpiland.c
//    Module Funciton           : 
//                                Deep Packet Inspection for ANTI LAND attack.
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
#include "dpiland.h"

/* Filter function of ANTI LAND attack. */
BOOL dpiFilter_land(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;

	if (dir != dpi_in)
	{
		/* Only apply on incoming direction. */
		goto __TERMINAL;
	}
	pIpHdr = (struct ip_hdr*)p->payload;
	/* 
	 * Protocol is ICMP, source and destination address are same, 
	 * then assume it's a LAND attack from PPPoE server side.
	 */
	if ((IP_PROTO_ICMP == pIpHdr->_proto) &&
		(pIpHdr->src.addr == pIpHdr->dest.addr))
	{
		return TRUE;
	}

__TERMINAL:
	return FALSE;
}

/* 
 * Action function of ANTI LAND attack, just log a 
 * message and drop the packet. 
 */
struct pbuf* dpiAction_land(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;
	struct icmp_echo_hdr* pEchoHdr = NULL;
	int iph_len = 0;

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
	/* Get IP header from pbuf. */
	pIpHdr = (struct ip_hdr*)p->payload;
	iph_len = IPH_HL(pIpHdr);
	iph_len *= 4;
	int echo_pkt_len = (p->tot_len - iph_len);
	if (echo_pkt_len < sizeof(struct icmp_echo_hdr))
	{
		goto __TERMINAL;
	}
	/* Locate the echo header. */
	pEchoHdr = (struct icmp_echo_hdr*)((char*)p->payload + iph_len);
	/* Log a message. */
	__LOG("[%s]ICMP land may arrive[src:%s, type:%d, id:%d, code:%d, seqno:%d, dir:in]\r\n", 
		__func__,
		inet_ntoa(pIpHdr->src),
		pEchoHdr->type,
		pEchoHdr->id, pEchoHdr->code, pEchoHdr->seqno);

__TERMINAL:
	/*
	 * CAUTION!: pbuf p MUST NOT be destroyed here.
	 * The pbuf p will be released by dpiPacketIn/Out 
	 * routine of DPI Manager if NULL is returned. 
	 */
	return NULL;
}
