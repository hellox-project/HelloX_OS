//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,10 2017
//    Module Name               : naticmp.c
//    Module Funciton           : 
//                                ICMP NAT implementations.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/icmp.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "hash.h"
#include "nat.h"
#include "naticmp.h"

/* Re calculate the ICMP checksum value. */
void _icmp_check_sum(struct ip_hdr* pHdr, struct pbuf* pb)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pb);

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	/* Validate the pbuf. */
	if (pb->tot_len < (iph_len + sizeof(u16_t) * 2))
	{
		/* pbuf length less than the sum of IP header and minimal ICMP hdr. */
		_hx_printf("%s:too short ICMP packet(len = %d)\r\n", pb->tot_len);
		return;
	}
	/* Locate ICMP header. */
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	/* Reset the original ICMP checksum value. */
	pIcmpHdr->chksum = 0;
	/* Move pb to ICMP header. */
	pbuf_header(pb, -iph_len);
	/* Calculate checksum. */
	pIcmpHdr->chksum = inet_chksum_pbuf(pb);
	/* Move back. */
	pbuf_header(pb, iph_len);
}

/* New ICMP echo ID,used to replace the old one in NAT. */
static u16_t GetICMPID()
{
	static u16_t icmp_id = NAT_ICMP_ID_BEGIN;
	return htons(icmp_id++);
}

/* Do ICMP specific initialization for a new NAT entry. */
void InitNatEntry_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pHdr);

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	/* Initialize NAT entry according ICMP type and code. */
	switch (ICMPH_TYPE(pIcmpHdr))
	{
	case ICMP_ECHO: /* ICMP echo request. */
		/*
		 * Translate as following:
		 * srcPort_bef = type(echo req) + code(0)
		 * srcPort_aft = type(echo rep) + code(0)
		 * dstPort_bef = ID in header
		 * dstPort_aft = New ICMP ID
		 */
		pEntry->srcPort_bef = (ICMP_ECHO << 8) + 0;
		pEntry->srcPort_aft = (ICMP_ER << 8) + 0;
		pEntry->dstPort_bef = ntohs(pIcmpHdr->id);
		pEntry->dstPort_aft = ntohs(GetICMPID());
		break;
	default:
		/* No more translation,just use IP address only. */
		break;
	}
}

/* ICMP specific NAT translation for IN and OUT direction. */
void InTranslation_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct pbuf* pb)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pHdr);

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	/* Do translation according ICMP type. */
	switch (ICMPH_TYPE(pIcmpHdr))
	{
	case ICMP_ER: /* echo reply. */
		pIcmpHdr->id = htons(pEntry->dstPort_bef); /* Recover the original ID. */
		break;
	default:
		break;
	}
}

void OutTranslation_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct pbuf* pb)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pHdr);

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	/* Do translation according ICMP type. */
	switch (ICMPH_TYPE(pIcmpHdr))
	{
	case ICMP_ECHO: /* echo request. */
		pIcmpHdr->id = htons(pEntry->dstPort_aft); /* Just change the ID. */
		break;
	default:
		break;
	}
}

/* Check if a packet matches the specified NAT entry. */
BOOL InPacketMatch_ICMP(__EASY_NAT_ENTRY* pNatEntry, struct ip_hdr* pHdr)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pNatEntry);
	BUG_ON(NULL == pHdr);

	/* Check if the IP addresses match. */
	if ((pHdr->src.addr != pNatEntry->dstAddr_aft.addr) ||
		(pHdr->dest.addr != pNatEntry->srcAddr_aft.addr))
	{
		return FALSE;
	}
	/* Do further check according to ICMP specific fields. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	switch (ICMPH_TYPE(pIcmpHdr))
	{
	case ICMP_ER:
		if (pIcmpHdr->id == htons(pNatEntry->dstPort_aft))
		{
			return TRUE;
		}
		return FALSE;
	default: 
		/* Only IP address is translated. */
		return TRUE;
	}
	return FALSE;
}

BOOL OutPacketMatch_ICMP(__EASY_NAT_ENTRY* pNatEntry, struct ip_hdr* pHdr)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pNatEntry);
	BUG_ON(NULL == pHdr);

	/* Check if IP addresses match. */
	if ((pHdr->src.addr != pNatEntry->srcAddr_bef.addr) ||
		(pHdr->dest.addr != pNatEntry->dstAddr_bef.addr))
	{
		return FALSE;
	}
	/* Do further check according ICMP specific field. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	switch (ICMPH_TYPE(pIcmpHdr))
	{
	case ICMP_ECHO:
		if (pIcmpHdr->id == htons(pNatEntry->dstPort_bef))
		{
			return TRUE;
		}
		return FALSE;
	default:
		/* Only IP address is translated. */
		return TRUE;
	}
	return FALSE;
}
