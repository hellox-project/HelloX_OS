//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 11,2017
//    Module Name               : oe_pro.h
//    Module Funciton           : 
//                                Ptotocol object's definition of PPP over 
//                                Ethernet.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __OE_PRO_H__
#define __OE_PRO_H__

#define PPPOE_PROTOCOL_NAME "PPP_over_Ethernet"

/* Ethernet frame type of PPPoE discovery. */
#define ETH_FRAME_TYPE_PPPOE_D 0x8864

/* Ethernet frame type of PPPoE session. */
#define ETH_FRAME_TYPE_PPPOE_S 0x8863

//Initializer of the protocol object.
BOOL pppoeInitialize(struct tag__NETWORK_PROTOCOL* pProtocol);

//Check if the protocol can be bind a specific interface.
BOOL pppoeCanBindInterface(struct tag__NETWORK_PROTOCOL* pProtocol, LPVOID pInterface, DWORD* l2proto);

//Add one ethernet interface to the network protocol.The state of the
//layer3 interface will be returned and be saved into pEthInt object.
LPVOID pppoeAddEthernetInterface(__ETHERNET_INTERFACE* pEthInt);

//Delete a ethernet interface.
VOID pppoeDeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt, LPVOID pL3Interface);

//Delivery a Ethernet Frame to this protocol,a dedicated L3 interface is also provided.
BOOL pppoeDeliveryFrame(__ETHERNET_BUFFER* pEthBuff, LPVOID pL3Interface);

//Set the network address of a given L3 interface.
BOOL pppoeSetIPAddress(LPVOID pL3Intface, __ETH_IP_CONFIG* addr);

//Shutdown a layer3 interface.
BOOL pppoeShutdownInterface(LPVOID pL3Interface);

//Unshutdown a layer3 interface.
BOOL pppoeUnshutdownInterface(LPVOID pL3Interface);

//Reset a layer3 interface.
BOOL pppoeResetInterface(LPVOID pL3Interface);

//Start DHCP protocol on a specific layer3 interface.
BOOL pppoeStartDHCP(LPVOID pL3Interface);

//Stop DHCP protocol on a specific layer3 interface.
BOOL pppoeStopDHCP(LPVOID pL3Interface);

//Release the DHCP configuration on a specific layer3 interface.
BOOL pppoeReleaseDHCP(LPVOID pL3Interface);

//Renew the DHCP configuration on a specific layer3 interface.
BOOL pppoeRenewDHCP(LPVOID pL3Interface);

//Get the DHCP configuration from a specific layer3 interface.
BOOL pppoeGetDHCPConfig(LPVOID pL3Interface, __DHCP_CONFIG* pConfig);

#endif //__OE_PRO_H__
