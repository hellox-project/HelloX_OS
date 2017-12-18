//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec,03 2015
//    Module Name               : lwip_pro.h
//    Module Funciton           : 
//                                Protocol object's definition corresponding
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

#ifndef __LWIP_PRO_H__
#define __LWIP_PRO_H__

#define LWIP_PROTOCOL_NAME "lwip_IP"

/* Ethernet frame type of IP. */
#define ETH_FRAME_TYPE_IP 0x0800

/* Ethernet frame type of ARP. */
#define ETH_FRAME_TYPE_ARP 0x0806

//Initializer of the protocol object.
BOOL lwipInitialize(struct tag__NETWORK_PROTOCOL* pProtocol);

//Check if the protocol can be bind a specific interface.
BOOL lwipCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface, DWORD* l2proto);

//Add one ethernet interface to the network protocol.The state of the
//layer3 interface will be returned and be saved into pEthInt object.
LPVOID lwipAddEthernetInterface(__ETHERNET_INTERFACE* pEthInt);

//Delete a ethernet interface.
VOID lwipDeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt, LPVOID pL3Interface);

//Delivery a Ethernet Frame to this protocol,a dedicated L3 interface is also provided.
BOOL lwipDeliveryFrame(__ETHERNET_BUFFER* pEthBuff, LPVOID pL3Interface);

//Set the network address of a given L3 interface.
BOOL lwipSetIPAddress(LPVOID pL3Intface, __ETH_IP_CONFIG* addr);

//Shutdown a layer3 interface.
BOOL lwipShutdownInterface(LPVOID pL3Interface);

//Unshutdown a layer3 interface.
BOOL lwipUnshutdownInterface(LPVOID pL3Interface);

//Reset a layer3 interface.
BOOL lwipResetInterface(LPVOID pL3Interface);

//Start DHCP protocol on a specific layer3 interface.
BOOL lwipStartDHCP(LPVOID pL3Interface);

//Stop DHCP protocol on a specific layer3 interface.
BOOL lwipStopDHCP(LPVOID pL3Interface);

//Release the DHCP configuration on a specific layer3 interface.
BOOL lwipReleaseDHCP(LPVOID pL3Interface);

//Renew the DHCP configuration on a specific layer3 interface.
BOOL lwipRenewDHCP(LPVOID pL3Interface);

//Get the DHCP configuration from a specific layer3 interface.
BOOL lwipGetDHCPConfig(LPVOID pL3Interface, __DHCP_CONFIG* pConfig);

#endif //__LWIP_PRO_H__
