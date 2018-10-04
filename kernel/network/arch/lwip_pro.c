//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec,03 2015
//    Module Name               : lwip_pro.c
//    Module Funciton           : 
//                                Protocol object's implementation corresponding
//                                lwIP network stack.Each L3 network protocol
//                                in HelloX should implement a protocol object and
//                                put one entry in NetworkProtocolArray in
//                                protos.c file.
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
#include <stdio.h>

#include "hx_eth.h"
#include "ethmgr.h"
#include "proto.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"

#include "lwip_pro.h"
#include "lwipext.h"
#ifdef __CFG_NET_DHCP_SERVER
#include "dhcp_srv/dhcp_srv.h"
#endif
#ifdef __CFG_NET_NAT
#include "nat/nat.h"
#endif

/* Global lwIP protocol object. */
__NETWORK_PROTOCOL* plwipProto = NULL;

//Initializer of the protocol object.
BOOL lwipInitialize(struct tag__NETWORK_PROTOCOL* pProtocol)
{
	__LWIP_EXTENSION* pExt = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pProtocol);

	/* Create and initialize lwIP extension object. */
	pExt = (__LWIP_EXTENSION*)_hx_malloc(sizeof(__LWIP_EXTENSION));
	if (NULL == pExt)
	{
		goto __TERMINAL;
	}
	pExt->nIncomePktSize = 0;
	pExt->pIncomePktFirst = pExt->pIncomePktLast = NULL;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pExt->spin_lock, "lwIP");
#endif

	/* Save the extension object to global lwIP protocol object. */
	pProtocol->pProtoExtension = pExt;

	/* Save the lwIP protocol object. */
	plwipProto = pProtocol;

	/* Initialize lwIP stack if enabled. */
	bResult = IPv4_Entry(pExt);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	/* Start DHCP Server damon if enabled. */
#ifdef __CFG_NET_DHCP_SERVER
	bResult = DHCPSrv_Start(NULL);
	if (!bResult)
	{
		goto __TERMINAL;
	}
#endif

	/* Initialize NAT functions. */
#ifdef __CFG_NET_NAT
	if (!NatManager.Initialize(&NatManager))
	{
		goto __TERMINAL;
	}
#endif

	/* Mark initialization OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Copy a pbuf object to ethernet frame buffer.
static VOID pbuf_to_ethbuf(struct pbuf* p, __ETHERNET_BUFFER* pEthBuff)
{
	struct pbuf* q = p;
	int i = 0;

	if ((NULL == p) || (NULL == pEthBuff))
	{
		BUG();
	}
	for (q = p; q != NULL; q = q->next)
	{
		if (i > pEthBuff->buff_length)
		{
			BUG();
		}
		memcpy(&pEthBuff->Buffer[i], q->payload, q->len);
		i += (int)q->len;
	}
	pEthBuff->act_length = i;
	pEthBuff->buff_status = ETHERNET_BUFFER_STATUS_INITIALIZED;
	pEthBuff->frame_type = 0x800;  //Set frame type to IP.
	return;
}

//A helper routine called by lwIP,to send out an ethernet frame in a given interface.
static err_t eth_level_output(struct netif* netif, struct pbuf* p)
{
	__ETHERNET_INTERFACE* pEthInt = NULL;
	struct pbuf* q = NULL;
	__ETHERNET_BUFFER* pEthBuff = NULL;

	if ((NULL == netif) || (NULL == p))
	{
		return ERR_ARG;
	}

	pEthInt = (__ETHERNET_INTERFACE*)netif->state;
	BUG_ON(NULL == pEthInt);
	if (p->tot_len > ETH_DEFAULT_MTU + ETH_HEADER_LEN)  //Too large packet.
	{
		_hx_printf("%s:Packet to send out[size = %d] of size.\r\n", __func__,
			p->tot_len);
		return ERR_VAL;
	}

	/* Use ethernet interface's default sending buffer to send the frame. */
	pEthBuff = &pEthInt->SendBuffer;
	pbuf_to_ethbuf(p, pEthBuff);
	pEthBuff->pOutInterface = pEthInt;
	/* Update interface statistics counter. */
	if (Eth_MAC_Multicast(&pEthBuff->Buffer[0]))
	{
		pEthInt->ifState.dwTxMcastNum++;
	}
	if(!pEthInt->SendFrame(pEthInt))
	{
		LINK_STATS_INC(link.err);
		return ERR_IF;
	}
	pEthInt->ifState.dwFrameSend += 1;
	pEthInt->ifState.dwTotalSendSize += pEthBuff->act_length;
	LINK_STATS_INC(link.xmit);
	return ERR_OK;

#if 0
	/*
	 * Create a new ethernet buffer to hold the sending packet.
	 */
	pEthBuff = EthernetManager.CreateEthernetBuffer(p->tot_len);
	if (NULL == pEthBuff)
	{
		return ERR_MEM;
	}
	//Convert pbuf to ethernet buffer.
	pbuf_to_ethbuf(p, pEthBuff);
	//pEthBuff->pEthernetInterface = pEthInt;
	pEthBuff->pOutInterface = pEthInt;
	//_hx_printf("  %s:send out a eth_frame,act_length = %d,tot_length = %d.\r\n", 
	//	__func__, pEthInt->SendBuffer.act_length,p->tot_len);
	if (EthernetManager.SendFrame(pEthInt, pEthBuff))
	{
		return ERR_OK;
	}
	else  /* Send fail. */
	{
		EthernetManager.DestroyEthernetBuffer(pEthBuff);
		return ERR_IF;
	}
#endif
}

//A local helper routine,called by lwIP to initialize a netif object.
static err_t _ethernet_if_init(struct netif *netif)
{
	__ETHERNET_INTERFACE*   pEthInt = NULL;
	int name_idx = 0;

	if (NULL == netif)
	{
		return ERR_ARG;
	}
	pEthInt = netif->state;
	if (NULL == pEthInt)
	{
		return ERR_VAL;
	}

	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
	/*
	* Initialize name of lwIP l3 interface.We use the first byte and
	* the last byte of ethernet interface name to construct the lwIP
	* netif name,since it only has 2 bytes.
	* This mechanism can guarantee that the lwIP netif names do not
	* conflict as possible.
	* We do not use string operations for security reason.
	*/
	while (name_idx < MAX_ETH_NAME_LEN)
	{
		if (pEthInt->ethName[name_idx] == 0)
		{
			break;
		}
		name_idx++;
	}
	BUG_ON(0 == name_idx); /* First byte of ethernet name is 0,bug. */
	name_idx--;
	netif->name[0] = pEthInt->ethName[0];
	netif->name[1] = pEthInt->ethName[name_idx];
	//netif->name[0] = pEthInt->ethName[0];
	//netif->name[1] = pEthInt->ethName[1];

	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */
	netif->output = etharp_output;
	netif->hwaddr_len = ETH_MAC_LEN;
	netif->linkoutput = eth_level_output;

	/* maximum transfer unit */
	netif->mtu = ETH_DEFAULT_MTU;
	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	/* Enable IGMP or multicasting in all ethernet interface(s). */
	netif->flags |= NETIF_FLAG_IGMP;
	//Set the MAC address of this interface.
	memcpy(netif->hwaddr, pEthInt->ethMac, ETH_MAC_LEN);

	return ERR_OK;
}

//Check if the protocol can be bind a specific interface.
BOOL lwipCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface, DWORD* ftm)
{
	__ETHERNET_INTERFACE* pEthInt = (__ETHERNET_INTERFACE*)pInterface;

	if ((NULL == pEthInt) || (NULL == ftm))
	{
		return FALSE;
	}
	//Frame mask of the lwIP protocol,it tells the network core of HelloX what kind of
	//ethernet frames that lwIP is interesting.It's the combination of IP and ARP's type
	//value in Ethernet Frame Header.
	*ftm = ETH_FRAME_TYPE_IP | ETH_FRAME_TYPE_ARP;
	return TRUE;
}

/*
 * Add one ethernet interface to the network protocol.The state of the
 * layer3 interface will be returned and be saved into pEthInt object.
 */
LPVOID lwipAddEthernetInterface(__ETHERNET_INTERFACE* pEthInt)
{
	static int ifIndex = 0;
	struct netif* pIf = NULL;
	ip_addr_t ipAddr, ipMask, ipGw;
	int name_idx = 0;

	//Allocate layer 3 interface for this ethernet interface.
	pIf = (struct netif*)KMemAlloc(sizeof(struct netif), KMEM_SIZE_TYPE_ANY);
	if (NULL == pIf)
	{
		_hx_printf("  Allocate netif failed.\r\n");
		return NULL;
	}
	memset(pIf, 0, sizeof(struct netif));
	pIf->state = pEthInt;     //Point to the layer 2 interface.

	/* 
	 * Initialize name of layer 3 interface.We use the first byte and
	 * the last byte of ethernet interface name to construct the lwIP
	 * netif name,since it only has 2 bytes.
	 * This mechanism can guarantee that the lwIP netif names do not
	 * conflict as possible.
	 * We do not use string operations for security reason.
	 */
	while (name_idx < MAX_ETH_NAME_LEN)
	{
		if (pEthInt->ethName[name_idx] == 0)
		{
			break;
		}
		name_idx++;
	}
	BUG_ON(0 == name_idx); /* First byte of ethernet name is 0,bug. */
	name_idx--;
	pIf->name[0] = pEthInt->ethName[0];
	pIf->name[1] = pEthInt->ethName[name_idx];

	//Convert common network address to IPv4 address.
	ipAddr.addr = pEthInt->ifState.IpConfig.ipaddr.Address.ipv4_addr;
	ipMask.addr = pEthInt->ifState.IpConfig.mask.Address.ipv4_addr;
	ipGw.addr = pEthInt->ifState.IpConfig.mask.Address.ipv4_addr;

	//Add the netif to lwIP.
	netif_add(pIf, &ipAddr, &ipMask,&ipGw,pEthInt,_ethernet_if_init, &tcpip_input);
	if (0 == ifIndex)
	{
		netif_set_default(pIf);
		ifIndex += 1;
	}

	return pIf;
}

//Delete a ethernet interface.
VOID lwipDeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt, LPVOID pL3Interface)
{
	return;
}

//Delivery a Ethernet Frame to this protocol,a dedicated L3 interface is also provided.
BOOL lwipDeliveryFrame(__ETHERNET_BUFFER* pEthBuff, LPVOID pL3Interface)
{
	struct netif* pIf = (struct netif*)pL3Interface;
	struct pbuf*  p, *q;
	int i = 0;

	if ((NULL == pEthBuff) | (NULL == pL3Interface))
	{
		BUG();
	}
	//Validate Ethernet buffer object.
	if (pEthBuff->act_length > pEthBuff->buff_length)
	{
		BUG();
	}
	/* Check the frame type again. */
	if ((pEthBuff->frame_type != ETH_FRAME_TYPE_IP) && 
		(pEthBuff->frame_type != ETH_FRAME_TYPE_ARP))
	{
		return FALSE;
	}

	p = pbuf_alloc(PBUF_RAW, pEthBuff->act_length, PBUF_POOL);
	if (NULL == p)
	{
		IP_STATS_INC(ip.memerr);
		return FALSE;
	}
	i = 0;
	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, &pEthBuff->Buffer[i], q->len);
		i = i + q->len;
	}
	//Delivery the packet to IP layer.
	if (pIf->input(p, pIf) != ERR_OK)
	{
		/* 
		 * If IP layer deny to accept the frame,
		 * that maybe caused by out of memory or
		 * full of message queue,then we Should
		 * release the pbuf object. 
		 */
		pbuf_free(p);
		IP_STATS_INC(ip.drop); /* Record one packet drop counter. */
	}
	return TRUE;
}

//Set the network address of a given L3 interface.
BOOL lwipSetIPAddress(LPVOID pL3Intface, __ETH_IP_CONFIG* pConfig)
{
	struct netif* pif = (struct netif*)pL3Intface;
	ip_addr_t addr, mask, gw;

	if ((NULL == pif) || (NULL == pConfig))
	{
		return FALSE;
	}
	//Convert common network address to IP address.
	addr.addr = pConfig->ipaddr.Address.ipv4_addr;
	mask.addr = pConfig->mask.Address.ipv4_addr;
	gw.addr = pConfig->defgw.Address.ipv4_addr;

	//Config the IP parameters into interface.
	netif_set_down(pif);
	netif_set_addr(pif, &addr, &mask, &gw);
	netif_set_up(pif);
	return TRUE;
}

//Shutdown a layer3 interface.
BOOL lwipShutdownInterface(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	netif_set_down(pif);
	return TRUE;
}

//Unshutdown a layer3 interface.
BOOL lwipUnshutdownInterface(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	netif_set_up(pif);
	return TRUE;
}

//Reset a layer3 interface.
BOOL lwipResetInterface(LPVOID pL3Interface)
{
	return FALSE;
}

//Start DHCP protocol on a specific layer3 interface.
BOOL lwipStartDHCP(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	dhcp_start(pif);
	return TRUE;
}

//Stop DHCP protocol on a specific layer3 interface.
BOOL lwipStopDHCP(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	dhcp_stop(pif);
	return TRUE;
}

//Release the DHCP configuration on a specific layer3 interface.
BOOL lwipReleaseDHCP(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	dhcp_release(pif);
	return TRUE;
}

//Renew the DHCP configuration on a specific layer3 interface.
BOOL lwipRenewDHCP(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	if (NULL == pif)
	{
		BUG();
	}
	dhcp_renew(pif);
	return TRUE;
}

//Get the DHCP configuration from a specific layer3 interface.
BOOL lwipGetDHCPConfig(LPVOID pL3Interface, __DHCP_CONFIG* pConfig)
{
	struct netif* pif = (struct netif*)pL3Interface;
	
	if ((NULL == pL3Interface) || (NULL == pConfig))
	{
		return FALSE;
	}
	//Try to obtain DHCP configurations.
	if (pif->dhcp)
	{
		if ((pif->dhcp->offered_ip_addr.addr != 0) &&
			(pif->dhcp->offered_sn_mask.addr != 0) &&
			(pif->dhcp->offered_gw_addr.addr != 0))
		{
			pConfig->network_addr.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			pConfig->network_addr.Address.ipv4_addr = pif->dhcp->offered_ip_addr.addr;
			pConfig->network_mask.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			pConfig->network_mask.Address.ipv4_addr = pif->dhcp->offered_sn_mask.addr;
			pConfig->default_gw.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			pConfig->default_gw.Address.ipv4_addr = pif->dhcp->offered_gw_addr.addr;
			//pConfig->dns_server1.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			//pConfig->dns_server1.Address.ipv4_addr = pif->dhcp->
			return TRUE;
		}
	}
	return FALSE;
}
