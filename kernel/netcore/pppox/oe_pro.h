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
VOID pppoeDeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt, LPVOID pIfState);

/*
 * Add a genif to the protocol. The interface specific state
 * will be returned if bind success,otherwise NULL.
 */
LPVOID pppoeAddGenif(__GENERIC_NETIF* pGenif);

/* Add address to genif. */
BOOL pppoeAddGenifAddress(__GENERIC_NETIF* pGenif,
	LPVOID pIfState,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary);

/* Delete a genif from the protocol. */
BOOL pppoeDeleteGenif(__GENERIC_NETIF* pGenif, LPVOID pIfState);

/* Check if a incoming packet belong to the protocol. */
BOOL pppoeMatch(__GENERIC_NETIF* pGenif, LPVOID pIfState, unsigned long type, struct pbuf* pkt);

/* Accept a incoming packet if Match returns TRUE. */
BOOL pppoeAcceptPacket(__GENERIC_NETIF* pGenif, LPVOID pIfState,
	struct pbuf* pIncomingPkt);

//Delivery a Ethernet Frame to this protocol,a dedicated L3 interface is also provided.
BOOL pppoeDeliveryFrame(__ETHERNET_BUFFER* pEthBuff, LPVOID pIfState);

/* Handler of link's status change. */
BOOL pppoeLinkStatusChange(LPVOID pIfState, BOOL bLinkDown);

//Set the network address of a given L3 interface.
BOOL pppoeSetIPAddress(LPVOID pIfState, __ETH_IP_CONFIG* addr);

//Reset a layer3 interface.
BOOL pppoeResetInterface(LPVOID pIfState);

//Start DHCP protocol on a specific layer3 interface.
BOOL pppoeStartDHCP(LPVOID pIfState);

//Stop DHCP protocol on a specific layer3 interface.
BOOL pppoeStopDHCP(LPVOID pIfState);

//Release the DHCP configuration on a specific layer3 interface.
BOOL pppoeReleaseDHCP(LPVOID pIfState);

//Renew the DHCP configuration on a specific layer3 interface.
BOOL pppoeRenewDHCP(LPVOID pIfState);

//Get the DHCP configuration from a specific layer3 interface.
BOOL pppoeGetDHCPConfig(LPVOID pIfState, __DHCP_CONFIG* pConfig);

#endif //__OE_PRO_H__
