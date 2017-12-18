//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,30 2017
//    Module Name               : easynat.c
//    Module Funciton           : 
//                                Network Address Translation function related
//                                implementations code.
//                                The NAT function is implemented in HelloX as
//                                2 types,one is named easy NAT,abbreviated as
//                                eNAT,that use the interface's IP address as source
//                                address when do NAT.The other is traditional
//                                NAT,that use a pool of public IP address to replace
//                                the private IP address when forwarding.
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
#include <rdxtree.h>

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "hash.h"
#include "nat.h"
#include "nattcp.h"
#include "naticmp.h"

/* Local helper routine to show out an IP packet,for deubgging. */
#ifdef NAT_DEBUG
static void show_ip_pkt(struct pbuf *p)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
	u8_t *payload;

	payload = (u8_t *)iphdr + IP_HLEN;

	_hx_printf("[%s]IP header:\r\n",__func__);
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|%2"S16_F" |%2"S16_F" |  0x%02"X16_F" |     %5"U16_F"     | (v, hl, tos, len)\r\n",
		IPH_V(iphdr),
		IPH_HL(iphdr),
		IPH_TOS(iphdr),
		ntohs(IPH_LEN(iphdr)));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|    %5"U16_F"      |%"U16_F"%"U16_F"%"U16_F"|    %4"U16_F"   | (id, flags, offset)\r\n",
		ntohs(IPH_ID(iphdr)),
		ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
		ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
		ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
		ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK);
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |    0x%04"X16_F"     | (ttl, proto, chksum)\r\n",
		IPH_TTL(iphdr),
		IPH_PROTO(iphdr),
		ntohs(IPH_CHKSUM(iphdr)));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (src)\r\n",
		ip4_addr1_16(&iphdr->src),
		ip4_addr2_16(&iphdr->src),
		ip4_addr3_16(&iphdr->src),
		ip4_addr4_16(&iphdr->src));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (dest)\r\n",
		ip4_addr1_16(&iphdr->dest),
		ip4_addr2_16(&iphdr->dest),
		ip4_addr3_16(&iphdr->dest),
		ip4_addr4_16(&iphdr->dest));
	_hx_printf("+-------------------------------+\r\n");
}
#else
#define show_ip_pkt(x)
#endif //NAT_DEBUG

/* Local helper routine to show out one easy NAT entry. */
static void ShowNatEntry(__EASY_NAT_ENTRY* pEntry)
{
	/* Output as: [x.x.x.x : p1]->[y.y.y.y : p2], pro:17, ms:10, mt:1000 */
	_hx_printf("[%s:%d]->", inet_ntoa(pEntry->srcAddr_bef), pEntry->srcPort_bef);
	_hx_printf("[%s:%d], ", inet_ntoa(pEntry->srcAddr_aft), pEntry->srcPort_aft);
	_hx_printf("dst[%s:%d], pro:%d, ms:%d, mt:%d\r\n",
		inet_ntoa(pEntry->dstAddr_bef),
		pEntry->dstPort_bef,
		pEntry->protocol,
		pEntry->ms,
		pEntry->match_times);
}

/* Update UDP datagram's check sum accordingly. */
static void _udp_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = IPH_HL(p);

	LWIP_UNUSED_ARG(pb);

	iph_len *= 4;
	pUdpHdr = (struct udp_hdr*)((char*)p + iph_len);
	__NATDEBUG("%s: reset UDP chksum from [%x] to 0.\r\n",
		__func__, pUdpHdr->chksum);
	pUdpHdr->chksum = 0; /* Reset check sum. */
}

/* Update TCP segment's check sum. */
static void _tcp_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	struct tcp_hdr* pTcpHdr = NULL;
	int iph_len = IPH_HL(p);
	int chksum = 0;
	ip_addr_t src, dest;

	iph_len *= 4;
	/* Locate TCP header. */
	pTcpHdr = (struct tcp_hdr*)((char*)p + iph_len);

	/* Validate pbuf object. */
	if (pb->tot_len < (iph_len + sizeof(struct tcp_hdr)))
	{
		return;
	}

	/*
	* Reset TCP header's check sum since it's source address
	* or source port is changed after NATing.
	*/
	chksum = pTcpHdr->chksum;
	ip_addr_copy(src, p->src);
	ip_addr_copy(dest, p->dest);
	pbuf_header(pb, -iph_len); /* move to TCP header. */
	pTcpHdr->chksum = 0; /* Reset the original check sum. */
	pTcpHdr->chksum = inet_chksum_pseudo(pb, &src, &dest, IP_PROTO_TCP,
		pb->tot_len);
	pbuf_header(pb, iph_len); /* move back. */
	__NATDEBUG("%s: TCP check sum updated[%X] -> [%X]\r\n",
		__func__,
		chksum,
		pTcpHdr->chksum);
}

/* Local helper to re-calculate IP header's check sum. */
static void _iphdr_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	u16_t chksum = 0;

	LWIP_UNUSED_ARG(pb);
	BUG_ON(NULL == p);
	p->_chksum = 0; /* Reset check sum value. */
	chksum = _hx_checksum((__u16*)p, sizeof(struct ip_hdr));
	p->_chksum = chksum;
	switch (p->_proto)
	{
	case IP_PROTO_UDP:
		_udp_check_sum(p, pb);
		break;
	case IP_PROTO_ICMP:
		_icmp_check_sum(p, pb);
		break;
	case IP_PROTO_TCP:
		_tcp_check_sum(p, pb);
		break;
	case IP_PROTO_IGMP:
		break;
	case IP_PROTO_UDPLITE:
		break;
	default:
		break;
	}
}

/* Create and initialize a NAT entry. */
static __EASY_NAT_ENTRY* CreateNatEntry(__NAT_MANAGER* pMgr)
{
	__EASY_NAT_ENTRY* pEntry = NULL;

	BUG_ON(NULL == pMgr);

	/* 
	 * If too many NAT entries exist. Should in critical section,but
	 * now it's OK,since the routine will be called by tcp_ip thread only.
	 */
	if (MAX_NAT_ENTRY_NUM < pMgr->stat.entry_num)
	{
		_hx_printf("[NAT]: too many NAT entries.\r\n");
		goto __TERMINAL;
	}
	pMgr->stat.entry_num++;

	/* Create and initialize it. */
	pEntry = (__EASY_NAT_ENTRY*)_hx_malloc(sizeof(__EASY_NAT_ENTRY));
	if (NULL == pEntry)
	{
		goto __TERMINAL;
	}
	memset(pEntry, 0, sizeof(__EASY_NAT_ENTRY));

__TERMINAL:
	return pEntry;
}

/* Destroy an easy NAT entry. */
static void DestroyNatEntry(__NAT_MANAGER* pMgr, __EASY_NAT_ENTRY* pEntry)
{
	BUG_ON(NULL == pEntry);
	NatManager.stat.entry_num--;
	_hx_free(pEntry);
}

/* Get a new TCP/UDP source port number,to replace the original one. */
static u16_t GetTCPSrcPort()
{
	static u16_t tcp_port = NAT_SOURCE_PORT_BEGIN;
	return htons(tcp_port++);
}

static u16_t GetUDPSrcPort()
{
	static u16_t udp_port = NAT_SOURCE_PORT_BEGIN;
	return htons(udp_port++);
}

/* Initialize a new NAT entry giving the IP header. */
static void InitNatEntry(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct netif* pOutIf)
{
	struct udp_hdr* pUdpHdr = NULL;
	struct tcp_hdr* pTcpHdr = NULL;
	int iph_len = 0;

	/* Set NAT entry according IP header. */
	pEntry->srcAddr_bef.addr = pHdr->src.addr;
	pEntry->dstAddr_bef.addr = pHdr->dest.addr;
	pEntry->srcAddr_aft.addr = pOutIf->ip_addr.addr; /* Changed. */
	pEntry->dstAddr_aft.addr = pHdr->dest.addr;
	pEntry->protocol = pHdr->_proto;
	pEntry->netif = pOutIf; 
	pEntry->ms = 0;
	pEntry->match_times++;

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	switch (pHdr->_proto)
	{
	case IP_PROTO_TCP:
		pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
		pEntry->srcPort_bef = ntohs(pTcpHdr->src);
		pEntry->srcPort_aft = ntohs(GetTCPSrcPort());      /* Changed. */
		pEntry->dstPort_bef = ntohs(pTcpHdr->dest);
		pEntry->dstPort_aft = ntohs(pTcpHdr->dest);
		break;
	case IP_PROTO_UDP:
		pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
		pEntry->srcPort_bef = ntohs(pUdpHdr->src);
		pEntry->srcPort_aft = ntohs(GetUDPSrcPort());      /* Changed. */
		pEntry->dstPort_bef = ntohs(pUdpHdr->dest);
		pEntry->dstPort_aft = ntohs(pUdpHdr->dest);
		break;
	case IP_PROTO_ICMP:
		InitNatEntry_ICMP(pEntry, pHdr);
		break;
	default:
		break;
	}
}

/* 
 * Add a new NAT entry in system,when out direction entry can not
 * be found.
 */
static __EASY_NAT_ENTRY* AddNewNatEntry(__NAT_MANAGER* pMgr, struct ip_hdr* pHdr, struct netif* pOutIf)
{
	__EASY_NAT_ENTRY* pEntry = NULL;
	__EASY_NAT_ENTRY* pHashList = NULL;
	int err_code = -1;
	unsigned long hash_key = 0;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pMgr);
	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pOutIf);

	/* Try to create a new NAT entry. */
	pEntry = CreateNatEntry(pMgr);
	if (NULL == pEntry)
	{
		goto __TERMINAL;
	}
	/* Initialize it by using IP hdr. */
	InitNatEntry(pEntry, pHdr, pOutIf);

	/* 
	 * Calculate the corresponding hash key of the new NAT
	 * in radix tree.
	 */
	hash_key = enatGetHashKeyByHdr(pHdr,out);

	/* Add the new NAT entry into radix tree and global list. */
	WaitForThisObject(pMgr->lock);
	pHashList = pMgr->pTree->Lookup(pMgr->pTree, hash_key);
	if (NULL == pHashList) /* No entry with the same key. */
	{
		pEntry->pHashNext = NULL;
		err_code = pMgr->pTree->Insert(pMgr->pTree, hash_key, pEntry);
		if (ERR_OK != err_code)
		{
			ReleaseMutex(pMgr->lock);
			_hx_printf("Insert NAT entry into radix tree failed[code = %d]", err_code);
			goto __TERMINAL;
		}
	}
	else /* Hash collision,just link to hash list. */
	{
		pEntry->pHashNext = pHashList->pHashNext;
		pHashList->pHashNext = pEntry;
	}
	/* Add to global NAT entry list. */
	pEntry->pNext = pMgr->entryList.pNext;
	pEntry->pNext->pPrev = pEntry;
	pEntry->pPrev = &pMgr->entryList;
	pMgr->entryList.pNext = pEntry;
	ReleaseMutex(pMgr->lock);

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pEntry)
		{
			_hx_free(pEntry);
			pEntry = NULL;
		}
	}
	return pEntry;
}

/* Input direction translation. */
static void InTranslation(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct pbuf* pb)
{
	struct tcp_hdr* pTcpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pb);

	/*
	 * Update NAT entry state info.
	 * It must be done before actual translation,since it maybe
	 * altered in specific protocol's translation process,such
	 * as TCP,it may set the entry's timeout value to MAX to
	 * purge the entry as soon as possible,in case of the TCP
	 * connection released.
	 */
	pEntry->ms = 0;
	pEntry->match_times++;

	/* Translate address first. */
	pHdr->dest.addr = pEntry->srcAddr_bef.addr;
	/* Farther translation according protocol. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	switch (pHdr->_proto)
	{
	case IP_PROTO_TCP:
		pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
		//pTcpHdr->dest = htons(pEntry->srcPort_bef);
		TcpTranslation(pEntry, pTcpHdr, in);
		break;
	case IP_PROTO_UDP:
		pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
		pUdpHdr->dest = htons(pEntry->srcPort_bef);
		break;
	case IP_PROTO_ICMP:
		InTranslation_ICMP(pEntry, pHdr, pb);
		break;
	default:
		break;
	}
	/* Update the packet's checksum. */
	_iphdr_check_sum(pHdr, pb);
}

/* Output direction translation. */
static void OutTranslation(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr,struct pbuf* pb)
{
	struct tcp_hdr* pTcpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pb);

	/*
	* Update NAT entry state info.
	* It must be done before actual translation,since it maybe
	* altered in specific protocol's translation process,such
	* as TCP,it may set the entry's timeout value to MAX to
	* purge the entry as soon as possible,in case of the TCP
	* connection released.
	*/
	pEntry->ms = 0;
	pEntry->match_times++;

	/* Translate address first. */
	pHdr->src.addr = pEntry->srcAddr_aft.addr;
	/* Farther translation according protocol. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	switch (pHdr->_proto)
	{
	case IP_PROTO_TCP:
		pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
		//pTcpHdr->src = htons(pEntry->srcPort_aft); 
		TcpTranslation(pEntry, pTcpHdr, out);
		break;
	case IP_PROTO_UDP:
		pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
		pUdpHdr->src = htons(pEntry->srcPort_aft);
		break;
	case IP_PROTO_ICMP:
		OutTranslation_ICMP(pEntry, pHdr, pb);
		break;
	default:
		break;
	}
	/* Update the packet's checksum. */
	_iphdr_check_sum(pHdr, pb);
}

/* Check if a input direction packet matches the given NAT entry. */
static BOOL _InPacketMatch(struct ip_hdr* pHdr, __EASY_NAT_ENTRY* pNatEntry)
{
	struct tcp_hdr* pTcpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pNatEntry);

	/* Increment matching times counter. */
	NatManager.stat.match_times++;

	/* Check according protocol type. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	switch (pHdr->_proto)
	{
	case IP_PROTO_TCP:
		pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
		if ((pHdr->src.addr == pNatEntry->dstAddr_aft.addr) &&
			(pHdr->dest.addr == pNatEntry->srcAddr_aft.addr) &&
			(pTcpHdr->src == htons(pNatEntry->dstPort_aft)) &&
			(pTcpHdr->dest == htons(pNatEntry->srcPort_aft)))
		{
			return TRUE;
		}
		return FALSE;
		break;
	case IP_PROTO_UDP:
		pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
		if ((pHdr->src.addr == pNatEntry->dstAddr_aft.addr) &&
			(pHdr->dest.addr == pNatEntry->srcAddr_aft.addr) &&
			(pUdpHdr->src == htons(pNatEntry->dstPort_aft)) &&
			(pUdpHdr->dest == htons(pNatEntry->srcPort_aft)))
		{
			return TRUE;
		}
		return FALSE;
		break;
	case IP_PROTO_ICMP:
		return InPacketMatch_ICMP(pNatEntry,pHdr);
		break;
	default:
		return FALSE;
		break;
	}
	return FALSE;
}

/* Check if there is an entry that for a given output IP packet. */
static BOOL _OutPacketMatch(struct ip_hdr* pHdr, __EASY_NAT_ENTRY* pNatEntry)
{
	struct tcp_hdr* pTcpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pNatEntry);

	/* Increment total match times counter. */
	NatManager.stat.match_times++;

	/* Check according protocol type. */
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	switch (pHdr->_proto)
	{
	case IP_PROTO_TCP:
		pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
		if ((pHdr->src.addr == pNatEntry->srcAddr_bef.addr) &&
			(pHdr->dest.addr == pNatEntry->dstAddr_bef.addr) &&
			(pTcpHdr->src == htons(pNatEntry->srcPort_bef)) &&
			(pTcpHdr->dest == htons(pNatEntry->dstPort_bef)))
		{
			return TRUE;
		}
		return FALSE;
		break;
	case IP_PROTO_UDP:
		pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
		if ((pHdr->src.addr == pNatEntry->srcAddr_bef.addr) &&
			(pHdr->dest.addr == pNatEntry->dstAddr_bef.addr) &&
			(pUdpHdr->src == htons(pNatEntry->srcPort_bef)) &&
			(pUdpHdr->dest == htons(pNatEntry->dstPort_bef)))
		{
			return TRUE;
		}
		return FALSE;
		break;
	case IP_PROTO_ICMP:
		return OutPacketMatch_ICMP(pNatEntry, pHdr);
		break;
	default:
		return FALSE;
		break;
	}
	return FALSE;
}

/* Packet in direction process for easy NAT. */
static BOOL enatPacketIn(struct pbuf* p, struct netif* in_if)
{
	struct ip_hdr *pHdr = NULL;
	__EASY_NAT_ENTRY* pEntry = NULL;
	unsigned long hash_key = 0;
	int hash_deep = 0;
	BOOL bResult = FALSE;

	int i = 0;

	BUG_ON((NULL == p) || (NULL == in_if));

	/* Validate the packet before apply NAT. */
	if (!NAT_PACKET_VALIDATE(p, in))
	{
		goto __TERMINAL;
	}

	/* Increment total translation request times. */
	NatManager.stat.trans_times++;

	/* 
	 * Calculate hash key to locate the corresponding NAT entry
	 * in radix tree.
	 */
	pHdr = (struct ip_hdr*)p->payload;
	hash_key = enatGetHashKeyByHdr(pHdr,in);

	/* Try to locate the NAT entry. */
	WaitForThisObject(NatManager.lock);
	pEntry = NatManager.pTree->Lookup(NatManager.pTree, hash_key);
	if (NULL == pEntry) /* No NAT entry found. */
	{
		ReleaseMutex(NatManager.lock);
		goto __TERMINAL;
	}
	/* Search the NAT entry list. */
	while (pEntry)
	{
		if (_InPacketMatch(pHdr, pEntry))
		{
			InTranslation(pEntry, pHdr, p);
			if (hash_deep > NatManager.stat.hash_deep)
			{
				NatManager.stat.hash_deep = hash_deep;
				/* Save the NAT entry to stat object,for performance improvement purpuse. */
				memcpy(&NatManager.stat.deepNat, pEntry, sizeof(__EASY_NAT_ENTRY));
			}
			ReleaseMutex(NatManager.lock);
			bResult = TRUE;
			goto __TERMINAL;
		}
		hash_deep++;
		pEntry = pEntry->pHashNext;
	}
	/* Can not find the matched NAT entry when reach here. */
	ReleaseMutex(NatManager.lock);

__TERMINAL:
	return bResult;
}

#if 0
/* Packet in direction process for easy NAT. */
static BOOL __enatPacketIn(struct pbuf* p, struct netif* in_if)
{
	struct ip_hdr *pHdr = NULL;
	int i = 0;

	BUG_ON((NULL == p) || (NULL == in_if));
	pHdr = (struct ip_hdr*)p->payload;

	for (i = 0; i < ENAT_LOCAL_ENTRY_NUM; i++)
	{
		if (_InPacketMatch(pHdr,&enatEntry[i]))
		{
			//Matched,NAT it.
			__NATDEBUG("DST_NAT:[%s]", inet_ntoa(pHdr->dest));
			__NATDEBUG("->[%s],pkt_len = %d\r\n", 
				inet_ntoa(enatEntry[i].srcAddr_bef),
				ntohs(pHdr->_len));
			pHdr->dest.addr = enatEntry[i].srcAddr_bef.addr;
			/* Clear timeout counter and increment match times. */
			enatEntry[i].ms = 0;
			enatEntry[i].match_times++;
			_iphdr_check_sum(pHdr,p);
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return TRUE;
}
#endif

/* Packet out direction process for easy NAT. */
static BOOL enatPacketOut(struct pbuf* p, struct netif* out_if)
{
	struct ip_hdr *pHdr = NULL;
	__EASY_NAT_ENTRY* pEntry = NULL;
	unsigned long hash_key = 0;
	int hash_deep = 0;
	BOOL bResult = FALSE;

	BUG_ON((NULL == p) || (NULL == out_if));

	/* Validate the packet before apply NAT. */
	if (!NAT_PACKET_VALIDATE(p, out))
	{
		goto __TERMINAL;
	}

	/* Increment total translation request times. */
	NatManager.stat.trans_times++;
	
	/* Calculate hash key to locate the NAT entry. */
	pHdr = (struct ip_hdr*)p->payload;
	hash_key = enatGetHashKeyByHdr(pHdr,out);

	/* 
	 * Try to locate a corresponding NAT entry in
	 * radix tree by apply hash key.
	 * If can not locate,create a new one,otherwise,
	 * translate the IP packet by using the NAT
	 * entry.
	 */
	WaitForThisObject(NatManager.lock);
	pEntry = NatManager.pTree->Lookup(NatManager.pTree, hash_key);
	if (NULL == pEntry) /* No entry yet. */
	{
		pEntry = AddNewNatEntry(&NatManager, pHdr, out_if);
		if (NULL == pEntry)
		{
			ReleaseMutex(NatManager.lock);
			goto __TERMINAL;
		}
	}
	else /* Found the corresponding NAT entries. */
	{
		/* Travel the whole collision list. */
		while (pEntry)
		{
			if (_OutPacketMatch(pHdr, pEntry))
			{
				OutTranslation(pEntry, pHdr, p);
				if (hash_deep > NatManager.stat.hash_deep)
				{
					NatManager.stat.hash_deep = hash_deep;
					/* Save the NAT entry to stat object,for performance improvement purpuse. */
					memcpy(&NatManager.stat.deepNat, pEntry, sizeof(__EASY_NAT_ENTRY));
				}
				ReleaseMutex(NatManager.lock);
				bResult = TRUE;
				goto __TERMINAL;
			}
			hash_deep++;
			pEntry = pEntry->pHashNext;
		}
		/*
		* Also can not locate the NAT entry when reach here,
		* hash collision occur.
		*/
		pEntry = AddNewNatEntry(&NatManager, pHdr, out_if);
		if (NULL == pEntry)
		{
			ReleaseMutex(NatManager.lock);
			goto __TERMINAL;
		}
	}
	/* Translate the IP packet using the new created NAT entry. */
	OutTranslation(pEntry, pHdr, p);
	ReleaseMutex(NatManager.lock);
	bResult = TRUE;
	
__TERMINAL:
	return bResult;
}

/* Purge one NAT entry from NAT module. */
static BOOL PurgeNatEntry(__EASY_NAT_ENTRY* pEntry)
{
	BOOL bResult = FALSE;
	__EASY_NAT_ENTRY* pHashList = NULL;
	__EASY_NAT_ENTRY* pPrev = NULL;
	unsigned long hash_key = 0;
	int err_code = -1;

	BUG_ON(NULL == pEntry);
	hash_key = enatGetHashKeyByEntry(pEntry,in);
	WaitForThisObject(NatManager.lock);
	/* Get the hash list with the same key. */
	pHashList = NatManager.pTree->Lookup(NatManager.pTree,hash_key);
	if (NULL == pHashList) /* Maybe caused by invalid pEntry. */
	{
		ReleaseMutex(NatManager.lock);
		goto __TERMINAL;
	}
	if (pHashList == pEntry) /* First one is the delete target. */
	{
		if (NULL == pHashList->pHashNext) /* Only one NAT entry. */
		{
			NatManager.pTree->Delete(NatManager.pTree, hash_key);
		}
		else /* More NAT entry exist. */
		{
			/* Delete first and then insert the next NAT entry. */
			NatManager.pTree->Delete(NatManager.pTree, hash_key);
			err_code = NatManager.pTree->Insert(NatManager.pTree, hash_key, pHashList->pHashNext);
			if (ERR_OK != err_code)
			{
				ReleaseMutex(NatManager.lock);
				_hx_printf("[NAT]: failed to re-insert NAT entry into radix tree[err = %d].\r\n",
					err_code);
				goto __TERMINAL;
			}
		}
	}
	else /* Just delete the pEntry from hash list. */
	{
		pPrev = pHashList;
		pHashList = pHashList->pHashNext;
		while (pHashList)
		{
			if (pHashList == pEntry)
			{
				break;
			}
			pPrev = pHashList;
			pHashList = pHashList->pHashNext;
		}
		if (NULL == pHashList) /* Can not find pEntry in hash list. */
		{
			ReleaseMutex(NatManager.lock);
			goto __TERMINAL;
		}
		/* Delete pEntry from hash list. */
		pPrev->pHashNext = pPrev->pHashNext->pHashNext;
	}
	/* Just delete pEntry from global list. */
	pEntry->pNext->pPrev = pEntry->pPrev;
	pEntry->pPrev->pNext = pEntry->pNext;
	DestroyNatEntry(&NatManager, pEntry);
	ReleaseMutex(NatManager.lock);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
* Enable or disable easy NAT on a given interface.
* The interface is denoted by it's name.
*/
static BOOL enatEnable(char* if_name, BOOL bEnable, BOOL bSetDefault)
{
	struct netif* netif = netif_list;

	BUG_ON(NULL == if_name);
	if (strlen(if_name) < sizeof(netif->name))
	{
		return FALSE;
	}
	/* Find the netif in global list. */
	while (netif)
	{
		if ((netif->name[0] == if_name[0]) && (netif->name[1] == if_name[1]))
		{
			break;
		}
		netif = netif->next;
	}
	if (NULL == netif)
	{
		return FALSE;
	}
	/* Set or clear NAT flag of the interface according bEnable. */
	if (bEnable)
	{
		netif->flags |= NETIF_FLAG_NAT;
	}
	else
	{
		netif->flags &= (~NETIF_FLAG_NAT);
	}
	/* Set this interface as default one if need. */
	if (bSetDefault)
	{
		netif_set_default(netif);
	}
	return TRUE;
}

/* Periodic timer handler of NAT. */
static void PeriodicTimerHandler()
{
	__EASY_NAT_ENTRY* pEntry = NatManager.entryList.pNext;
	__EASY_NAT_ENTRY* pPurge = NULL;
	unsigned long time_out = 0;

	/* Scan every NAT entry in system. */
	WaitForThisObject(NatManager.lock);
	while (pEntry != &NatManager.entryList)
	{
		pEntry->ms += NAT_ENTRY_SCAN_PERIOD;
		switch (pEntry->protocol)
		{
		case IP_PROTO_TCP:
			time_out = NAT_ENTRY_TIMEOUT_TCP;
			break;
		case IP_PROTO_UDP:
			time_out = NAT_ENTRY_TIMEOUT_UDP;
			break;
		case IP_PROTO_ICMP:
			time_out = NAT_ENTRY_TIMEOUT_ICMP;
			break;
		default:
			time_out = NAT_ENTRY_TIMEOUT_DEF;
			break;
		}
		if (pEntry->ms > time_out) /* Should purge the entry. */
		{
			pPurge = pEntry;
			pEntry = pEntry->pNext;
			PurgeNatEntry(pPurge);
		}
		else
		{
			pEntry = pEntry->pNext;
		}
	}
	ReleaseMutex(NatManager.lock);
}

/* 
 * Background thread of NAT,timers,NAT entryies,
 * and other NAT related functions are handled in
 * this thread.
 */
static DWORD NatMainThread(LPVOID* pData)
{
	HANDLE hTimer = NULL;
	__KERNEL_THREAD_MESSAGE msg;

	/* Create periodic timer of NAT. */
	hTimer = SetTimer(NAT_PERIODIC_TIMER_ID, NAT_ENTRY_SCAN_PERIOD, NULL, NULL, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer)
	{
		_hx_printf("%s: failed to create periodic timer.\r\n", __func__);
		goto __TERMINAL;
	}
	/* Main message loop. */
	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				PeriodicTimerHandler();
				break;
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;
				break;
			default:
				break;
			}
		}
	}

__TERMINAL:
	return 0;
}

/* Initializer of NAT manager. */
static BOOL nmInitialize(__NAT_MANAGER* pMgr)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pMgr);
	/* Create radix tree object. */
	pMgr->pTree = CreateRadixTree();
	if (NULL == pMgr->pTree)
	{
		goto __TERMINAL;
	}
	/* Create the mutex object to protect NAT entry list. */
	pMgr->lock = CreateMutex();
	if (NULL == pMgr->lock)
	{
		goto __TERMINAL;
	}

	/* Initialize NAT entry list. */
	pMgr->entryList.pPrev = pMgr->entryList.pNext = &pMgr->entryList;

	/* Create background thread of NAT. */
	pMgr->hMainThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		NatMainThread,
		NULL,
		NULL,
		NAT_MAIN_THREAD_NAME);
	if (NULL == pMgr->hMainThread)
	{
		goto __TERMINAL;
	}

	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/* Uninitializer of NAT manager. */
static void nmUninitialize(__NAT_MANAGER* pMgr)
{
	__EASY_NAT_ENTRY* pEntry = NULL;

	BUG_ON(NULL == pMgr);
	BUG_ON(NULL == pMgr->pTree);
	BUG_ON(NULL == pMgr->lock);

	/* Destroy all NAT entries in system. */
	WaitForThisObject(pMgr->lock);
	pEntry = pMgr->entryList.pNext;
	while (pEntry != &pMgr->entryList)
	{
		/* Unlink it first. */
		pEntry->pNext->pPrev = pEntry->pPrev;
		pEntry->pPrev->pNext = pEntry->pNext;
		_hx_free(pEntry);
		pEntry = pMgr->entryList.pNext;
	}
	ReleaseMutex(pMgr->lock);

	/* Destroy lock. */
	DestroyMutex(pMgr->lock);

	/* Destroy the radix tree object. */
	DestroyRadixTree(pMgr->pTree);
	return;
}

/* Show out NAT session table. */
static void ShowNatSession(__NAT_MANAGER* pMgr, size_t ss_num)
{
	size_t top = ss_num;
	__EASY_NAT_ENTRY* pEntry = NULL;
	int show_cnt = 0;

	BUG_ON(NULL == pMgr);
	if (0 == top)
	{
		top = MAX_DWORD_VALUE;
	}
	WaitForThisObject(NatManager.lock);
	pEntry = NatManager.entryList.pNext;
	while (pEntry != &NatManager.entryList)
	{
		top--;
		ShowNatEntry(pEntry);
		show_cnt++;
		if (0 == top)
		{
			break;
		}
		pEntry = pEntry->pNext;
	}
	ReleaseMutex(NatManager.lock);
	_hx_printf("[%d] NAT entries showed.\r\n", show_cnt);
	return;
}

/* Global NAT manager object. */
__NAT_MANAGER NatManager = {
	NULL,                     //pTree.
	{ 0 },                    //entryList.
	{ 0 },                    //easy nat statistics.
	NULL,                     //lock.
	NULL,                     //hMainThread.
	enatEnable,               //enatEnable.
	enatPacketIn,             //enatPacketIn.
	enatPacketOut,            //enatPacketOut.

	nmInitialize,             //Initialize.
	nmUninitialize,           //Uninitialize.

	ShowNatSession,           //ShowNatSession.
};
