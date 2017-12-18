//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,23 2017
//    Module Name               : hash.c
//    Module Funciton           : 
//                                HASH algorithm that can be used by NAT module
//                                to construct a key value,which is used to locate
//                                a proper NAT entry.
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

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "hash.h"

/* Return a hash key according packet direction. */
unsigned long enatGetHashKeyByHdr(struct ip_hdr* pHdr, __PACKET_DIRECTION dir)
{
	struct udp_hdr* pUdpHdr = NULL;
	struct tcp_hdr* pTcpHdr = NULL;
	int iph_len = 0;
	unsigned long hash_key = 0;

	BUG_ON(NULL == pHdr);
	iph_len = IPH_HL(pHdr);
	iph_len *= 4;

	if (in == dir) /* In direction,from Internet to internal network. */
	{

		/* Add src IP addr and protocol into hash key. */
		hash_key += pHdr->src.addr;
		hash_key += pHdr->_proto;

		if (IP_PROTO_UDP == pHdr->_proto)
		{
			/* Add UDP source port into hash key. */
			pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
			hash_key += ntohs(pUdpHdr->src);
		}
		if (IP_PROTO_TCP == pHdr->_proto)
		{
			/* Add TCP source port into hash key. */
			pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
			hash_key += ntohs(pTcpHdr->src);
		}
		if (IP_PROTO_ICMP == pHdr->_proto)
		{
			/* Add packet ID into hash key. */
		}
		goto __TERMINAL;
	}

	/* Output direction,from internal network to Internet. */
	if (out == dir)
	{
		/* Add dst IP addr into hash key. */
		hash_key += pHdr->dest.addr;
		hash_key += pHdr->_proto;

		if (IP_PROTO_UDP == pHdr->_proto)
		{
			/* Add UDP destination port into hash key. */
			pUdpHdr = (struct udp_hdr*)((char*)pHdr + iph_len);
			hash_key += ntohs(pUdpHdr->dest);
		}
		if (IP_PROTO_TCP == pHdr->_proto)
		{
			/* Add TCP destination port into hash key. */
			pTcpHdr = (struct tcp_hdr*)((char*)pHdr + iph_len);
			hash_key += ntohs(pTcpHdr->dest);
		}
		if (IP_PROTO_ICMP == pHdr->_proto)
		{
			/* Add ICMP packet ID into hash key. */
		}
		goto __TERMINAL;
	}

__TERMINAL:
	/* Preserve necessary bits determined by mask. */
	hash_key &= HASH_KEY_MASK;
	return hash_key;
}

/* Return a hash key according packet direction,given an NAT entry. */
unsigned long enatGetHashKeyByEntry(__EASY_NAT_ENTRY* pEntry, __PACKET_DIRECTION dir)
{
	unsigned long hash_key = 0;

	BUG_ON(NULL == pEntry);


	/* Add src IP addr and protocol into hash key. */
	hash_key += pEntry->dstAddr_bef.addr;
	hash_key += pEntry->protocol;
	if (IP_PROTO_UDP == pEntry->protocol)
	{
		/* Add UDP source port into hash key. */
		hash_key += pEntry->dstPort_bef;
	}
	if (IP_PROTO_TCP == pEntry->protocol)
	{
		/* Add TCP source port into hash key. */
		hash_key += pEntry->dstPort_bef;
	}
	if (IP_PROTO_ICMP == pEntry->protocol)
	{
		/* Add packet ID into hash key. */
		//hash_key += pHdr->_id;
	}

	/* Preserve necessary bits determined by mask. */
	hash_key &= HASH_KEY_MASK;
	return hash_key;
}
