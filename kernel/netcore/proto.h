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

#include "ethmgr.h"  /* For ethernet manager related definitions. */

//Protocol object's definition,each network protocol in HelloX,is corresponding
//one of this object.
typedef struct tag__NETWORK_PROTOCOL{
	char*      szProtocolName;
	UCHAR      ucProtocolType;  //Protocol types.
#define NETWORK_PROTOCOL_TYPE_UNSPEC    0x00
#define NETWORK_PROTOCOL_TYPE_IPV4      0x01
#define NETWORK_PROTOCOL_TYPE_IPV6      0x02
#define NETWORK_PROTOCOL_TYPE_6LOWPAN   0x04
#define NETWORK_PROTOCOL_TYPE_PPPOE     0x05
#define NETWORK_PROTOCOL_TYPE_USERDEF   0xFF

	/* Protocol private extension. */
	void*      pProtoExtension;

	/*
	 * Interfaces that one protocol object should expose to
	 * HelloX kernel. 
	 */
	//Initializer of the protocol object.
	BOOL      (*Initialize)(struct tag__NETWORK_PROTOCOL* pProtocol);

	//Check if the protocol can be bind a specific interface.
	BOOL      (*CanBindInterface)(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface,DWORD* ftm);

	//Add one ethernet interface to the network protocol.The state of the
	//layer3 interface will be returned and be saved into pEthInt object.
	LPVOID    (*AddEthernetInterface)(__ETHERNET_INTERFACE* pEthInt);

	//Delete a ethernet interface.
	VOID      (*DeleteEthernetInterface)(__ETHERNET_INTERFACE* pEthInt, LPVOID pL3Interface);

	//Delivery a Ethernet Frame to this protocol,a dedicated L3 interface is also provided.
	BOOL      (*DeliveryFrame)(__ETHERNET_BUFFER* pEthBuff, LPVOID pL3Interface);

	//Set the network address of a given L3 interface.
	BOOL      (*SetIPAddress)(LPVOID pL3Intface, __ETH_IP_CONFIG* addr);

	//Shutdown a layer3 interface.
	BOOL      (*ShutdownInterface)(LPVOID pL3Interface);

	//Unshutdown a layer3 interface.
	BOOL      (*UnshutdownInterface)(LPVOID pL3Interface);

	//Reset a layer3 interface.
	BOOL      (*ResetInterface)(LPVOID pL3Interface);

	//Start DHCP protocol on a specific layer3 interface.
	BOOL      (*StartDHCP)(LPVOID pL3Interface);

	//Stop DHCP protocol on a specific layer3 interface.
	BOOL      (*StopDHCP)(LPVOID pL3Interface);

	//Release the DHCP configuration on a specific layer3 interface.
	BOOL      (*ReleaseDHCP)(LPVOID pL3Interface);

	//Renew the DHCP configuration on a specific layer3 interface.
	BOOL      (*RenewDHCP)(LPVOID pL3Interface);

	//Get the DHCP configuration from a specific layer3 interface.
	BOOL      (*GetDHCPConfig)(LPVOID pL3Interface,__DHCP_CONFIG* pConfig);
}__NETWORK_PROTOCOL;

//A global protocol object array to contain all layer3 protocol bojects
//in HelloX.
extern __NETWORK_PROTOCOL NetworkProtocolArray[];

#endif  //__PROTO_H__
