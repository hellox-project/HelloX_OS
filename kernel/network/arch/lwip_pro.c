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
	__ETHERNET_INTERFACE*   pEthInt = NULL;
	struct pbuf* q = NULL;

	if ((NULL == netif) || (NULL == p))
	{
		return !ERR_OK;
	}
	pEthInt = (__ETHERNET_INTERFACE*)netif->state;
	if (NULL == pEthInt)  //Should not occur.
	{
		BUG();
	}
	if (p->tot_len > ETH_DEFAULT_MTU + ETH_HEADER_LEN)  //Too large packet.
	{
		_hx_printf("%s:Packet to send out[size = %d] of size.\r\n", __func__,
			p->tot_len);
		return !ERR_OK;
	}
	if (p->tot_len > pEthInt->SendBuffer.buff_length)
	{
		_hx_printf("%s:Packet to send out[size = %d] of size.\r\n", __func__,
			p->tot_len);
		return !ERR_OK;
	}
	//Convert pbuf to ethernet buffer.
	pbuf_to_ethbuf(p, &pEthInt->SendBuffer);
	//_hx_printf("  %s:send out a eth_frame,act_length = %d,tot_length = %d.\r\n", 
	//	__func__, pEthInt->SendBuffer.act_length,p->tot_len);
	if (EthernetManager.SendFrame(pEthInt, NULL))
	{
		return ERR_OK;
	}
	return !ERR_OK;
}

//A local helper routine,called by lwIP to initialize a netif object.
static err_t _ethernet_if_init(struct netif *netif)
{
	__ETHERNET_INTERFACE*   pEthInt = NULL;

	if (NULL == netif)
	{
		return !ERR_OK;
	}
	pEthInt = netif->state;
	if (NULL == pEthInt)
	{
		return !ERR_OK;
	}

	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
	netif->name[0] = pEthInt->ethName[0];
	netif->name[1] = pEthInt->ethName[1];

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
	//Set the MAC address of this interface.
	memcpy(netif->hwaddr, pEthInt->ethMac, ETH_MAC_LEN);

	return ERR_OK;
}

//Initializer of the protocol object.
BOOL lwipInitialize(struct tag_NETWORK_PROTOCOL* pProtocol)
{
	return TRUE;
}

//Check if the protocol can be bind a specific interface.
BOOL lwipCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface, DWORD* ftm)
{
	__ETHERNET_INTERFACE* pEthInt = (__ETHERNET_INTERFACE*)pInterface;

	if ((NULL == pEthInt) || (NULL == ftm))
	{
		return FALSE;
	}
	*ftm = LWIP_FRAME_TYPE_MASK;  //IP protocol's frame type value in Ethernet.
	return TRUE;
}

//Add one ethernet interface to the network protocol.The state of the
//layer3 interface will be returned and be saved into pEthInt object.
LPVOID lwipAddEthernetInterface(__ETHERNET_INTERFACE* pEthInt)
{
	static int ifIndex = 0;
	struct netif* pIf = NULL;
	ip_addr_t ipAddr, ipMask, ipGw;

	//Allocate layer 3 interface for this ethernet interface.
	pIf = (struct netif*)KMemAlloc(sizeof(struct netif), KMEM_SIZE_TYPE_ANY);
	if (NULL == pIf)
	{
		_hx_printf("  Allocate netif failed.\r\n");
		return NULL;
	}
	memset(pIf, 0, sizeof(struct netif));
	pIf->state = pEthInt;     //Point to the layer 2 interface.

	//Initialize name of layer 3 interface,since it only occupies 2 bytes.
	pIf->name[0] = pEthInt->ethName[0];
	pIf->name[1] = pEthInt->ethName[1];

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
	p = pbuf_alloc(PBUF_RAW, pEthBuff->act_length, PBUF_POOL);
	if (NULL == p)
	{
		_hx_printf("  %s:allocate pbuf object failed.\r\n", __func__);
		return FALSE;
	}
	i = 0;
	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, &pEthBuff->Buffer[i], q->len);
		i = i + q->len;
	}
	//Delivery the packet to IP layer.
	pIf->input(p, pIf);
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
