//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 3,2020
//    Module Name               : ethio.c
//    Module Funciton           : 
//                                Generic ethernet input and output process.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>

#include "hx_inet.h"
#include "hx_eth.h"
#include "ethmgr.h"
#include "proto.h"
#include "genif.h"

/* 
 * Helper routine that dispatch an ethernet frame
 * to local,i.e,the frame should be applied l3 routine,
 * not layer2 switching.
 */
static BOOL __dispatch_to_local(__GENERIC_NETIF* pGenif, struct pbuf* p)
{
	__NETWORK_PROTOCOL* pProtocol = NULL;
	__ETHERNET_II_HEADER* pEthHdr = NULL;
	uint16_t frame_type;
	BOOL bResult = FALSE;

	/* Get the ethernet frame's type. */
	pEthHdr = (__ETHERNET_II_HEADER*)p->payload;
	frame_type = _hx_ntohs(pEthHdr->frame_type);

	/* Dispatch to protocols bound to this genif. */
	for (int i = 0; i < GENIF_MAX_PROTOCOL_BINDING; i++)
	{
		pProtocol = pGenif->proto_binding[i].pProtocol;
		if (pProtocol)
		{
			BUG_ON(NULL == pGenif->proto_binding[i].pIfState);
			if (pProtocol->Match(pGenif, pGenif->proto_binding[i].pIfState,
				frame_type, p))
			{
				/* The protocol would like to accept it. */
				bResult = pProtocol->AcceptPacket(pGenif, 
					pGenif->proto_binding[i].pIfState, p);
				if (bResult)
				{
					/* The frame is eaten by the protocol. */
					goto __TERMINAL;
				}
			}
		}
	}

__TERMINAL:
	return bResult;
}

/*
 * General input process of ethernet,this routine
 * could be set as the default genif_input if device
 * driver does not specify a new one.
 */
int general_ethernet_input(__GENERIC_NETIF* pGenif, struct pbuf* p)
{
	__ETHERNET_II_HEADER* pEthHdr = NULL;
	int ret_val = ERR_UNKNOWN;

	BUG_ON((NULL == pGenif) || (NULL == p));
	if (p->len < sizeof(__ETHERNET_II_HEADER))
	{
		/* pbuf is illegal. */
		ret_val = ERR_ARG;
		goto __TERMINAL;
	}
	if (lt_ethernet != pGenif->link_type)
	{
		ret_val = ERR_ARG;
		goto __TERMINAL;
	}

	pEthHdr = (__ETHERNET_II_HEADER*)p->payload;
	if (Eth_MAC_Match(&pEthHdr->mac_dst[0], &pGenif->genif_ha[0]))
	{
		/* 
		 * The frame is destinaed to us, dispatch 
		 * to local and apply l3 routing. 
		 */
		if (__dispatch_to_local(pGenif, p))
		{
			pGenif->stat.rx_pkt += 1;
			pGenif->stat.rx_bytes += p->tot_len;
			ret_val = ERR_OK;
		}
		else
		{
			/* Local protocols deny to accept it. */
			ret_val = ERR_UNKNOWN;
		}
		goto __TERMINAL;
	}
	else
	{
		if (Eth_MAC_BM(pEthHdr->mac_dst))
		{
			/* The frame is broadcast or multicast. */
			if (Eth_MAC_Multicast(pEthHdr->mac_dst))
			{
				/* Broadcast and multicast all should dispatch to local. */
				if (__dispatch_to_local(pGenif, p))
				{
					/* Multicast, update multicast counter. */
					pGenif->stat.rx_mcast += 1;
					pGenif->stat.rx_pkt += 1;
					pGenif->stat.rx_bytes += p->tot_len;
					ret_val = ERR_OK;
				}
				else
				{
					/* Local protocols deny to accept it. */
					ret_val = ERR_UNKNOWN;
				}
			}
			else
			{
				/* Broadcast and multicast all should dispatch to local. */
				if (__dispatch_to_local(pGenif, p))
				{
					/* Broadcast, update broadcast counter. */
					pGenif->stat.rx_bcast += 1;
					pGenif->stat.rx_pkt += 1;
					pGenif->stat.rx_bytes += p->tot_len;
					ret_val = ERR_OK;
				}
				else
				{
					/* Local protocols deny to accept it. */
					ret_val = ERR_UNKNOWN;
				}
			}
		}
		else
		{
			/* 
			 * The frame's destination is unicast 
			 * but not for us. 
			 * We may switch it if bridging is enabled.
			 */
		}
	}

__TERMINAL:
	return ret_val;
}

/*
 * General output process of ethernet,this routine
 * could be set as the default genif_output if device
 * driver does not specify a new one.
 */
int general_ethernet_output(__GENERIC_NETIF* pGenif, struct pbuf* p)
{
	return ERR_OK;
}
