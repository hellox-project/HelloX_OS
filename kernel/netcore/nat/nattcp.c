//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,03 2017
//    Module Name               : nattcp.c
//    Module Funciton           : 
//                                TCP protocol's NAT implementation code.
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
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "hash.h"
#include "nat.h"
#include "nattcp.h"

/* Adjust the TCP mss value according interface's MTU. */
static void AdjustMSS(__EASY_NAT_ENTRY* pEntry, struct tcp_hdr* pTcpHdr)
{
	unsigned char* tcp_opt = NULL;
	int data_off = 0, total_opt_len = 0;
	unsigned char opt_len = 0;
	u16_t mss = 0, old_mss = 0;

	/* Get data offset from TCP header. */
	data_off = TCPH_OFFSET(pTcpHdr);
	if (data_off <= TCP_FIXED_HEADER_LEN) /* No option or invalid TCP header. */
	{
		return;
	}
	total_opt_len = data_off - TCP_FIXED_HEADER_LEN;
	tcp_opt = ((unsigned char*)pTcpHdr + TCP_FIXED_HEADER_LEN);
	while (total_opt_len)
	{
		switch (*tcp_opt) /* Option kind value. */
		{
		case TCP_OPTION_KIND_END:
			return;
		case TCP_OPTION_KIND_NOP:
			tcp_opt++;
			total_opt_len--;
			break;
		case TCP_OPTION_KIND_MSS:
			/* Get the old MSS value. */
			tcp_opt += 2;
			old_mss = ntohs(*(u16_t*)tcp_opt);
			/* Get the new MSS value. */
			mss = pEntry->netif->mtu;
			mss -= TCP_FIXED_HEADER_LEN;
			mss -= IP_FIXED_HEADER_LEN;
			/* Change old MSS value if necessary. */
			if (old_mss > mss)
			{
				*(u16_t*)tcp_opt = htons(mss);
				//_hx_printf("Adjust TCP MSS: [%d]-->[%d]\r\n", old_mss, mss);
			}
			return;
		default:
			opt_len = *(tcp_opt + 1);
			/* Option's length can not be 0. */
			if (0 == opt_len)
			{
				return;
			}
			tcp_opt += opt_len;
			if (total_opt_len < opt_len) /* Ilegal case. */
			{
				return;
			}
			total_opt_len -= opt_len;
			break;
		}
	}
	return;
}

/* Entry point of TCP protocol's NAT processing. */
void TcpTranslation(__EASY_NAT_ENTRY* pEntry, struct tcp_hdr* pTcpHdr, __PACKET_DIRECTION dir)
{
	u8_t tcp_flags = 0;

	BUG_ON(NULL == pEntry);
	BUG_ON(NULL == pTcpHdr);

	/* 
	 * Check if the 2 direction final is over.
	 * If in and out directions' FIN bits are all set,it means
	 * the TCP tear down process is over,this is the last
	 * TCP segment that ack the last FIN req.
	 * So just set the NAT entry to timeout to be released
	 * as soon as possible.
	 */
	if ((pEntry->tp_header.tcp_flags_in & TCP_FIN) && (pEntry->tp_header.tcp_flags_out & TCP_FIN))
	{
		/* Set the NAT entry as timeout,to lead the purge ASAP. */
		pEntry->ms = NAT_ENTRY_TIMEOUT_TCP;
	}

	/* Get TCP flags in header. */
	tcp_flags = TCPH_FLAGS(pTcpHdr);
	/* Adjust TCP MSS,no matter in or out. */
	if (tcp_flags & TCP_SYN)
	{
		AdjustMSS(pEntry, pTcpHdr);
	}

	if (in == dir)
	{
		pTcpHdr->dest = htons(pEntry->srcPort_bef);
		/* Preserve the flags into NAT entry. */
		pEntry->tp_header.tcp_flags_in = tcp_flags;
	}
	else
	{
		pTcpHdr->src = htons(pEntry->srcPort_aft);
		/* Preserve the flags. */
		pEntry->tp_header.tcp_flags_out = tcp_flags;
	}

	/* 
	 * If RST bit is set,then should no more subsequent TCP packet in
	 * this connection,so just timeout the NAT entry.
	 */
	if (tcp_flags & TCP_RST)
	{
		pEntry->ms = NAT_ENTRY_TIMEOUT_TCP;
	}
}
