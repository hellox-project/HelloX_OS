//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,22 2015
//    Module Name               : proto.h
//    Module Funciton           : 
//                                Network protocol related definitions are
//                                put into this file.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PROTO_H__
#define __PROTO_H__

/* General network framework headers. */
#include "ethmgr.h"
#include "genif.h"

/* Network protocol types. */
#define NETWORK_PROTOCOL_TYPE_UNSPEC    0x00
#define NETWORK_PROTOCOL_TYPE_IPV4      0x01
#define NETWORK_PROTOCOL_TYPE_IPV6      0x02
#define NETWORK_PROTOCOL_TYPE_6LOWPAN   0x04
#define NETWORK_PROTOCOL_TYPE_PPPOE     0x05
#define NETWORK_PROTOCOL_TYPE_HPX       0x06
#define NETWORK_PROTOCOL_TYPE_USERDEF   0xFF

/*
 * Protocol object's definition,each network protocol 
 * in HelloX,is corresponding one of this object.
 */
typedef struct tag__NETWORK_PROTOCOL{
	char* szProtocolName;
	/* Protocol types. */
	UCHAR ucProtocolType;

	/* 
	 * Protocol private extension. 
	 * Each protocol should has one global object to
	 * manage it self,such as tx/rx list, counters,etc.
	 * The protocol could put this global object into
	 * this pointer.
	 */
	void* pProtoExtension;

	/*
	 * Interfaces that a protocol object should expose to
	 * HelloX network framework. 
	 */
	BOOL (*Initialize)(struct tag__NETWORK_PROTOCOL* pProtocol);

	/* Check if the protocol can be bound to a specified interface. */
	BOOL (*CanBindInterface)(struct tag__NETWORK_PROTOCOL* pProtocol, 
		LPVOID pInterface, DWORD* ftm);

	/*
	 * Add one ethernet interface to the network protocol.
	 * The state of the protocol specific will be returned 
	 * and be saved into interface object.
	 */
	LPVOID (*AddEthernetInterface)(__ETHERNET_INTERFACE* pEthInt);

	/* Delete a ethernet interface. */
	VOID (*DeleteEthernetInterface)(__ETHERNET_INTERFACE* pEthInt, 
		LPVOID pL3Interface);

	/*
	 * Add a genif to the protocol. The interface specific state
	 * will be returned if bind success,otherwise NULL.
	 */
	LPVOID (*AddGenif)(__GENERIC_NETIF* pGenif);

	/* Delete a genif from the protocol. */
	BOOL (*DeleteGenif)(__GENERIC_NETIF* pGenif, LPVOID pIfState);

	/* Check if a incoming packet belong to the protocol. */
	BOOL (*Match)(__GENERIC_NETIF* pGenif, LPVOID pIfState, 
		unsigned long type, struct pbuf* pkt);

	/* Accept a incoming packet if Match returns TRUE. */
	BOOL (*AcceptPacket)(__GENERIC_NETIF* pGenif, LPVOID pIfState,
		struct pbuf* pIncomingPkt);

	/*
	 * Delivery a Ethernet Frame to this protocol,
	 * the state specific to an interface is also provided.
	 */
	BOOL (*DeliveryFrame)(__ETHERNET_BUFFER* pEthBuff, LPVOID pIfState);

	/* Notified by device driver that the bound link's status changed. */
	BOOL (*LinkStatusChange)(LPVOID pIfState, BOOL bLinkDown);

	/* Add network address to a genif. */
	BOOL (*AddGenifAddress)(__GENERIC_NETIF* pGenif,
		LPVOID pIfState,
		__COMMON_NETWORK_ADDRESS* comm_addr,
		int addr_num,
		BOOL bSecondary);

	/*
	 * Set the network address of a given interface,the
	 * pIfState is the protocol state associated with current interface.
	 */
	BOOL (*SetIPAddress)(LPVOID pIfState, __ETH_IP_CONFIG* addr);

	/* Notify the protocol that a netif is reset. */
	BOOL (*ResetInterface)(LPVOID pIfState);

	/* Start DHCP protocol on a specific layer3 interface. */
	BOOL (*StartDHCP)(LPVOID pIfState);

	/* Stop DHCP protocol on a specific layer3 interface. */
	BOOL (*StopDHCP)(LPVOID pIfState);

	/* Release the DHCP configuration on a specific layer3 interface. */
	BOOL (*ReleaseDHCP)(LPVOID pIfState);

	/* Renew the DHCP configuration on a specific layer3 interface. */
	BOOL (*RenewDHCP)(LPVOID pIfState);

	/* Get the DHCP configuration from a specific layer3 interface. */
	BOOL (*GetDHCPConfig)(LPVOID pIfState, __DHCP_CONFIG* pConfig);
}__NETWORK_PROTOCOL;

/*
 * A global protocol object array contains all layer3 
 * protocol bojects built in HelloX kernel.
 */
extern __NETWORK_PROTOCOL NetworkProtocolArray[];

#endif  //__PROTO_H__
