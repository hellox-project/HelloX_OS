//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 11,2017
//    Module Name               : oe_pro.c
//    Module Funciton           : 
//                                Ptotocol object's implementation of PPP over 
//                                Ethernet.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "netcfg.h"
#include "hx_inet.h"
#include "ethmgr.h"
#include "netmgr.h"
#include "oemgr.h"
#include "oe_pro.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/stats.h"

/* 
 * Binding objects between ppp over ethernet 
 * protocol and Ethernet interface(genif). 
 */
__PPPOE_ETHIF_BINDING pppoeBinding[PPPOE_MAX_INSTANCE_NUM] = { 0 };
int current_bind_num = 0;

/* Initializer of the protocol object. */
BOOL pppoeInitialize(struct tag__NETWORK_PROTOCOL* pProtocol)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pProtocol);

	/* Start PPPoE service in kernel. */
	bResult = pppoeManager.Initialize(&pppoeManager);
	if (!bResult)
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
 * Add a genif to the protocol. The interface specific state
 * will be returned if bind success,otherwise NULL.
 */
LPVOID pppoeAddGenif(__GENERIC_NETIF* pGenif)
{
	__PPPOE_ETHIF_BINDING* pBinding = NULL;
	int i = 0;
	struct netif* netif = NULL;

	BUG_ON(NULL == pGenif);
	if (current_bind_num >= PPPOE_MAX_INSTANCE_NUM)
	{
		/* No free binding slot. */
		goto __TERMINAL;
	}

	/* 
	 * lwIP must be bound to the genif, before 
	 * pppoe can bind to it,since pppoe will invoke the
	 * netif to rx/tx. 
	 */
	if (pGenif->proto_binding[0].pIfState == NULL)
	{
		goto __TERMINAL;
	}
	netif = (struct netif*)pGenif->proto_binding[0].pIfState;

	/* Find a free slot to hold the binding relationship. */
	for (i = 0; i < PPPOE_MAX_INSTANCE_NUM; i++)
	{
		if (NULL == pppoeBinding[i].pGenif)
		{
			pBinding = &pppoeBinding[i];
			break;
		}
	}
	if (NULL == pBinding) 
	{
		/* Can not find empty slot. */
		goto __TERMINAL;
	}

	/* Increase genif's refer counter. */
	NetworkManager.GetGenif(pGenif);
	pBinding->pGenif = pGenif;

	pBinding->netif = netif;
	pBinding->pInstance = NULL;  /* Unknow yet. */
	current_bind_num++;

__TERMINAL:
	return pBinding;
}

/* Add address to genif,it's no use for pppoe currently. */
BOOL pppoeAddGenifAddress(__GENERIC_NETIF* pGenif,
	LPVOID pIfState,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	return FALSE;
}

/* Delete a genif from the protocol. */
BOOL pppoeDeleteGenif(__GENERIC_NETIF* pGenif, LPVOID pIfState)
{
	BUG_ON((NULL == pGenif) || (NULL == pIfState));

	/* Decrease genif's refer counter. */
	NetworkManager.ReleaseGenif(pGenif);
	return FALSE;
}

/* Check if a incoming packet belong to the protocol. */
BOOL pppoeMatch(__GENERIC_NETIF* pGenif, LPVOID pIfState, 
	unsigned long type, 
	struct pbuf* pkt)
{
	BUG_ON((NULL == pGenif) || (NULL == pkt));
	if ((ETH_FRAME_TYPE_PPPOE_D == type) || (ETH_FRAME_TYPE_PPPOE_S == type))
	{
		/* PPPoE discovery or session packet. */
		return TRUE;
	}
	return FALSE;
}

/* Accept a incoming packet if Match returns TRUE. */
BOOL pppoeAcceptPacket(__GENERIC_NETIF* pGenif, LPVOID pIfState,
	struct pbuf* pIncomingPkt)
{
	__PPPOE_ETHIF_BINDING* pBinding = (__PPPOE_ETHIF_BINDING*)pIfState;
	__PPPOE_POSTFRAME_BLOCK* pBlock = NULL;
	__ETHERNET_II_HEADER* pEthHdr = NULL;
	BOOL bResult = FALSE;

	BUG_ON((NULL == pBinding) || (NULL == pIncomingPkt));

	if (NULL == pBinding->pInstance) 
	{
		/* No PPPoE instance yet. */
		//_hx_printf("pppoe frame received but no session yet.\r\n");
		goto __TERMINAL;
	}

	/* Get frame type. */
	pEthHdr = (__ETHERNET_II_HEADER*)pIncomingPkt->payload;

	/* Allocate post frame block object. */
	pBlock = (__PPPOE_POSTFRAME_BLOCK*)_hx_malloc(sizeof(__PPPOE_POSTFRAME_BLOCK));
	if (NULL == pBlock)
	{
		goto __TERMINAL;
	}
	pBlock->pInstance = pBinding->pInstance;
	pBlock->frame_type = _hx_ntohs(pEthHdr->frame_type);
	pBlock->p = NULL;
	pBlock->pNext = NULL;
	pBlock->p = pIncomingPkt;

	/* Delivery the frame to PPPoE main thread. */
	bResult = pppoeManager.PostFrame(pBlock);

__TERMINAL:
	/*
	 * pBlock and pbuf object should be released if delivery failed.
	 * They will be released in upper module if delivery success.
	 */
	if (!bResult)
	{
		if (pBlock)
		{
			_hx_free(pBlock);
			LINK_STATS_INC(link.drop);
		}
	}
	else
	{
		LINK_STATS_INC(link.recv);
	}
	return bResult;
}

//Check if the protocol can be bind a specific interface.
BOOL pppoeCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface, DWORD* l2proto)
{
	__ETHERNET_INTERFACE* pEthInt = (__ETHERNET_INTERFACE*)pInterface;

	BUG_ON(NULL == pProtocol);
	BUG_ON(NULL == pEthInt);
	BUG_ON(NULL == l2proto);

	if (current_bind_num >= PPPOE_MAX_INSTANCE_NUM)
	{
		return FALSE;
	}

	/*
	 * Frame mask of the ppp over ethernet protocol,
	 * it tells the network core of HelloX what kind of
	 * ethernet frames that pppoe is interesting.
	 * It's the combination of pppoe session and pppoe data
	 * values in Ethernet Frame Header.
	 */
	*l2proto = ETH_FRAME_TYPE_PPPOE_D | ETH_FRAME_TYPE_PPPOE_S;
	return TRUE;
}

/*
 * Add one ethernet interface to the ppp over 
 * network protocol.The state of the
 * layer3 interface will be returned and be 
 * saved into pEthInt object.
 */
LPVOID pppoeAddEthernetInterface(__ETHERNET_INTERFACE* pEthInt)
{
	__PPPOE_ETHIF_BINDING* pBinding = NULL;
	int i = 0;

	BUG_ON(NULL == pEthInt);
	BUG_ON(current_bind_num >= PPPOE_MAX_INSTANCE_NUM);

	/* Find a empty slot to hold the binding relationship. */
	for (i = 0; i < PPPOE_MAX_INSTANCE_NUM; i++)
	{
		if (NULL == pppoeBinding[i].pEthInt)
		{
			pBinding = &pppoeBinding[i];
			break;
		}
	}
	if (NULL == pBinding) /* Can not find empty slot. */
	{
		goto __TERMINAL;
	}

	pBinding->pEthInt = pEthInt;
	pBinding->pInstance = NULL;  /* Unknow yet. */
	current_bind_num++;

__TERMINAL:
	return pBinding;
}

//Delete a ethernet interface.
VOID pppoeDeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt, LPVOID pIfState)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return;
}

/*
 * Delivery a Ethernet Frame to this protocol,
 * a protocol dedicated state is also provided.
 */
BOOL pppoeDeliveryFrame(__ETHERNET_BUFFER* pEthBuff, LPVOID pIfState)
{
	__PPPOE_ETHIF_BINDING* pBinding = (__PPPOE_ETHIF_BINDING*)pIfState;
	__PPPOE_POSTFRAME_BLOCK* pBlock = NULL;
	struct pbuf *p = NULL, *q = NULL;
	int i = 0;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pEthBuff);
	BUG_ON(NULL == pBinding);
	
	/* Make sure the frame type is PPPoE discovery or session. */
	if ((pEthBuff->frame_type != ETH_FRAME_TYPE_PPPOE_S) &&
		(pEthBuff->frame_type != ETH_FRAME_TYPE_PPPOE_D))
	{
		LINK_STATS_INC(link.proterr);
		goto __TERMINAL;
	}

	if (NULL == pBinding->pInstance) /* No PPPoE instance yet. */
	{
		//_hx_printf("PPPoE frame received but no PPPoE session yet.\r\n");
		goto __TERMINAL;
	}

	/* Allocate post frame block object. */
	pBlock = (__PPPOE_POSTFRAME_BLOCK*)_hx_malloc(sizeof(__PPPOE_POSTFRAME_BLOCK));
	if (NULL == pBlock)
	{
		goto __TERMINAL;
	}
	pBlock->pInstance = pBinding->pInstance;
	//pBlock->pEthInt = pEthBuff->pInInterface;
	pBlock->frame_type = pEthBuff->frame_type;
	pBlock->p = NULL;
	pBlock->pNext = NULL;

	p = pbuf_alloc(PBUF_RAW, pEthBuff->act_length, PBUF_RAM);
	if (NULL == p)
	{
		_hx_printf("  %s:allocate pbuf object failed.\r\n", __func__);
		goto __TERMINAL;
	}
	i = 0;
	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, &pEthBuff->Buffer[i], q->len);
		i = i + q->len;
	}
	pBlock->p = p;

	//Delivery the frame to PPPoE main thread.
	bResult = pppoeManager.PostFrame(pBlock);

__TERMINAL:
	/* 
	 * pBlock and pbuf object should be released if delivery failed.
	 * They will be released in upper module if delivery success.
	 */
	if (!bResult)
	{
		if (p)
		{
			pbuf_free(p);
		}
		if (pBlock)
		{
			_hx_free(pBlock);
			LINK_STATS_INC(link.drop);
		}
	}
	else
	{
		LINK_STATS_INC(link.recv);
	}
	return bResult;
}

//Set the network address of a given L3 interface.
BOOL pppoeSetIPAddress(LPVOID pIfState, __ETH_IP_CONFIG* addr)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return FALSE;
}

//Shutdown a layer3 interface.
static BOOL pppoeShutdownInterface(LPVOID pIfState)
{
	return FALSE;
}

//Unshutdown a layer3 interface.
static BOOL pppoeUnshutdownInterface(LPVOID pIfState)
{
	return FALSE;
}

/* Link status change handler. */
BOOL pppoeLinkStatusChange(LPVOID pIfState, BOOL bLinkDown)
{
	BUG_ON(NULL == pIfState);
	if (bLinkDown)
	{
		/* Link changed to down. */
		pppoeShutdownInterface(pIfState);
	}
	else
	{
		/* Link changed to up from down. */
		pppoeUnshutdownInterface(pIfState);
	}
	return TRUE;
}

//Reset a layer3 interface.
BOOL pppoeResetInterface(LPVOID pIfState)
{
	return FALSE;
}

//Start DHCP protocol on a specific layer3 interface.
BOOL pppoeStartDHCP(LPVOID pIfState)
{
	return FALSE;
}

//Stop DHCP protocol on a specific layer3 interface.
BOOL pppoeStopDHCP(LPVOID pIfState)
{
	return FALSE;
}

//Release the DHCP configuration on a specific layer3 interface.
BOOL pppoeReleaseDHCP(LPVOID pIfState)
{
	return FALSE;
}

//Renew the DHCP configuration on a specific layer3 interface.
BOOL pppoeRenewDHCP(LPVOID pIfState)
{
	return FALSE;
}

//Get the DHCP configuration from a specific layer3 interface.
BOOL pppoeGetDHCPConfig(LPVOID pIfState, __DHCP_CONFIG* pConfig)
{
	return FALSE;
}
