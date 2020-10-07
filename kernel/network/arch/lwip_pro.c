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
	memset(pExt, 0, sizeof(__LWIP_EXTENSION));
	pExt->nIncomePktSize = 0;
	pExt->pIncomePktFirst = pExt->pIncomePktLast = NULL;
	pExt->nOutgoingPktSize = 0;
	pExt->pOutgoingPktFirst = pExt->pOutgoingPktLast = NULL;
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

/*
 * Low level output of a ethernet interface,which is called
 * by IP or other upper level protocols.
 */
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

	/* Create a new ethernet buffer to hold the packet. */
	pEthBuff = EthernetManager.CreateEthernetBuffer(p->tot_len, 0);
	if (NULL == pEthBuff)
	{
		_hx_printf("[%s]Failed to create ethernet buffer.\r\n", __func__);
		return ERR_BUF;
	}
	/* Convert pbuf to ethernet buffer. */
	if (!EthernetManager.pbuf_to_ebuf(p, pEthBuff))
	{
		return ERR_BUF;
	}

	pEthBuff->pOutInterface = pEthInt;
	/* Update interface statistics counter. */
	if (Eth_MAC_Multicast(&pEthBuff->Buffer[0]))
	{
		pEthInt->ifState.dwTxMcastNum++;
	}
	if (!pEthInt->SendFrame(pEthInt, pEthBuff))
	{
		/* Destroy the ethernet buffer. */
		EthernetManager.DestroyEthernetBuffer(pEthBuff);
		LINK_STATS_INC(link.err);
		return ERR_IF;
	}
	pEthInt->ifState.dwFrameSend += 1;
	pEthInt->ifState.dwTotalSendSize += pEthBuff->act_length;
	LINK_STATS_INC(link.xmit);
	return ERR_OK;
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

/*
 * Low level sending routine of netif,which will invoke
 * the output routine of genif.
 */
static err_t __genif_ll_output(struct netif* netif, struct pbuf* p)
{
	__GENERIC_NETIF* pGenif = NULL;

	BUG_ON((NULL == netif) || (NULL == p));
	pGenif = (__GENERIC_NETIF*)netif->state;
	BUG_ON(NULL == pGenif);
	
	/* Delegate the tx to genif's output routine. */
	return pGenif->genif_output(pGenif, p);
}

/* Local helper to init a netif when AddGenif is invoked. */
static err_t __genif_netif_init(struct netif* pIf)
{
	__GENERIC_NETIF* pGenif = NULL;

	BUG_ON(NULL == pIf);
	pGenif = (__GENERIC_NETIF*)pIf->state;
	BUG_ON(NULL == pGenif);

	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);

	/* 
	 * We directly use etharp_output() here to save a function call.
	 * You can instead declare your own function an call etharp_output()
	 * from it if you have to do some checks before sending (e.g. if link
	 * is available...) 
	 */
	pIf->output = etharp_output;
	pIf->hwaddr_len = ETH_MAC_LEN;
	pIf->linkoutput = __genif_ll_output;

	/* maximum transfer unit */
	pIf->mtu = pGenif->genif_mtu;
	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	pIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	/* Enable IGMP or multicasting in all ethernet interface(s). */
	pIf->flags |= NETIF_FLAG_IGMP;
	/* Set the MAC address of this interface. */
	memcpy(pIf->hwaddr, pGenif->genif_ha, ETH_MAC_LEN);

	return ERR_OK;
}

/*
 * Add a genif to the protocol. The interface specific state
 * will be returned if bind success,otherwise NULL.
 */
LPVOID lwipAddGenif(__GENERIC_NETIF* pGenif)
{
	struct netif* netif = NULL;
	int name_idx = 0;
	ip_addr_t ipAddr, ipMask, ipGw;

	BUG_ON(NULL == pGenif);
	/* Only ethernet/VLAN/ppp/loop are supported. */
	if ((pGenif->link_type != lt_ethernet) && 
		(pGenif->link_type != lt_vlanif) && 
		(pGenif->link_type != lt_pppoe) &&
		(pGenif->link_type != lt_loopback))
	{
		goto __TERMINAL;
	}

	/* Create netif and init it. */
	netif = (struct netif*)_hx_malloc(sizeof(struct netif));
	if (NULL == netif)
	{
		goto __TERMINAL;
	}
	memset(netif, 0, sizeof(struct netif));
	netif->state = pGenif;

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
		if (pGenif->genif_name[name_idx] == 0)
		{
			break;
		}
		name_idx++;
	}
	/* First byte of netif name is 0,bug. */
	BUG_ON(0 == name_idx);
	name_idx--;
	netif->name[0] = pGenif->genif_name[0];
	netif->name[1] = pGenif->genif_name[name_idx];

	/* Use first IPv4 configuration config the netif. */
	ipAddr = pGenif->ip_addr[0];
	ipMask = pGenif->ip_mask[0];
	ipGw = pGenif->ip_gw[0];

	/* Add the netif to lwIP. */
	netif_add(netif, &ipAddr, &ipMask, &ipGw, pGenif, __genif_netif_init, &tcpip_input);
	/* Enable dhcp client if no IP address specified. */
	if (0 == ipAddr.addr)
	{
		dhcp_start(netif);
	}
	/* Set as default interface if it's the first one. */
	if (0 == pGenif->if_index)
	{
		netif_set_default(netif);
	}

__TERMINAL:
	return (LPVOID)netif;
}

/* Add ip address into genif(then to netif in lwIP). */
BOOL lwipAddGenifAddress(__GENERIC_NETIF* pGenif,
	LPVOID pIfState,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	struct netif* pif = (struct netif*)pIfState;
	ip_addr_t addr, mask, gw;

	/* Validate the input parameters.*/
	BUG_ON((NULL == pif) || (NULL == comm_addr));
	if (addr_num != 3)
	{
		/* 
		 * address,mask,gw must be specified 
		 * in an array together. 
		 */
		return FALSE;
	}
	for (int i = 0; i < 3; i++) {
		if (comm_addr[i].AddressType != NETWORK_ADDRESS_TYPE_IPV4)
		{
			/* Only ipv4 addr is acceptable. */
			return FALSE;
		}
	}
	if (bSecondary) {
		/* Secondary address not supported yet. */
		return FALSE;
	}

	/* Convert common network address to IP address. */
	addr.addr = comm_addr[0].Address.ipv4_addr;
	mask.addr = comm_addr[1].Address.ipv4_addr;
	gw.addr = comm_addr[2].Address.ipv4_addr;

	/* stop dhcp client on the netif. */
	dhcp_stop(pif);

	/* Config the IP parameters into interface. */
	netif_set_down(pif);
	netif_set_addr(pif, &addr, &mask, &gw);
	netif_set_up(pif);
	return TRUE;
}

/* Delete a genif from the protocol. */
BOOL lwipDeleteGenif(__GENERIC_NETIF* pGenif, LPVOID pIfState)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return FALSE;
}

/* Check if a incoming packet belong to the protocol. */
BOOL lwipMatch(__GENERIC_NETIF* pGenif, LPVOID pIfState, unsigned long type, struct pbuf* pkt)
{
	BUG_ON((NULL == pGenif) || (NULL == pIfState) || (NULL == pkt));
	if ((ETH_FRAME_TYPE_IP == type) || (ETH_FRAME_TYPE_ARP == type))
	{
		/* IP or ARP packet. */
		return TRUE;
	}
	return FALSE;
}

/* Accept a incoming packet if Match returns TRUE. */
BOOL lwipAcceptPacket(__GENERIC_NETIF* pGenif, LPVOID pIfState,
	struct pbuf* pIncomingPkt)
{
	struct netif* pIf = (struct netif*)pIfState;
	int i = 0;

	BUG_ON((NULL == pGenif) || (NULL == pIfState) || (NULL == pIncomingPkt));

	/* Delivery the incoming packet to IP layer. */
	if (pIf->input(pIncomingPkt, pIf) != ERR_OK)
	{
		/*
		 * If IP layer deny to accept the frame,
		 * that maybe caused by out of memory or
		 * full of message queue,then we Should
		 * release the pbuf object,which is created
		 * by device driver.
		 */
		pbuf_free(pIncomingPkt);
		/* Record one packet drop counter. */
		IP_STATS_INC(ip.drop); 
	}
	return TRUE;
}

/* Check if the protocol can be bind a specific interface. */
BOOL lwipCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, 
	LPVOID pInterface, DWORD* ftm)
{
	__ETHERNET_INTERFACE* pEthInt = (__ETHERNET_INTERFACE*)pInterface;

	if ((NULL == pEthInt) || (NULL == ftm))
	{
		return FALSE;
	}
	/*
	 * Frame mask of the lwIP protocol,it tells the 
	 * network core of HelloX what kind of
	 * ethernet frames that lwIP is interesting.
	 * It's the combination of IP and ARP's type
	 * value in Ethernet Frame Header.
	 */
	*ftm = ETH_FRAME_TYPE_IP | ETH_FRAME_TYPE_ARP;
	return TRUE;
}

/*
 * Add one ethernet interface to the network protocol.
 * The state of the layer3 interface will be returned 
 * and be saved into pEthInt object.
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
	UNIMPLEMENTED_ROUTINE_CALLED;
	return;
}

/*
 * Delivery a Ethernet Frame to this protocol,
 * a dedicated L3 interface is also provided.
 */
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

/* Configure an IP address on the specified interface. */
BOOL lwipSetIPAddress(LPVOID pIfState, __ETH_IP_CONFIG* pConfig)
{
	struct netif* pif = (struct netif*)pIfState;
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
static BOOL lwipShutdownInterface(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	BUG_ON(NULL == pif);
	netif_set_down(pif);
	return TRUE;
}

//Unshutdown a layer3 interface.
static BOOL lwipUnshutdownInterface(LPVOID pL3Interface)
{
	struct netif* pif = (struct netif*)pL3Interface;
	BUG_ON(NULL == pif);
	netif_set_up(pif);
	return TRUE;
}

/* Handler of link status changed. */
BOOL lwipLinkStatusChange(LPVOID pIfState, BOOL bLinkDown)
{
	BUG_ON(NULL == pIfState);
	if (bLinkDown)
	{
		/* Link status change to down. */
		lwipShutdownInterface(pIfState);
	}
	else
	{
		/* Link status change to up from down. */
		lwipUnshutdownInterface(pIfState);
	}
	return TRUE;
}

//Reset a layer3 interface.
BOOL lwipResetInterface(LPVOID pL3Interface)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
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
