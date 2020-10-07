//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,22 2015
//    Module Name               : protos.c
//    Module Funciton           : 
//                                Each layer3 protocol in HelloX should create
//                                a global protocol object,and register it into
//                                a global array named NetworkProtocolArray in this
//                                file.
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

#include "netcfg.h"
#include "ethmgr.h"
#include "proto.h"

#ifdef __CFG_NET_IPv4
#include "arch/lwip_pro.h"
#endif

#ifdef __CFG_NET_PPPOE
#include "pppox/oe_pro.h"
#endif

/*
 * Network protocol array,each layer3 protocol 
 * in system should put one entry in this array.
 */
__NETWORK_PROTOCOL NetworkProtocolArray[] = {
#ifdef __CFG_NET_IPv4
	/* lwIP for IPv4 must be the first one in this array. */
	{
		LWIP_PROTOCOL_NAME,                    //protocol's name.
		NETWORK_PROTOCOL_TYPE_IPV4,            //L3 protocol's type.
		NULL,                                  //Protocol extension.
		lwipInitialize,                        //Initialize.
		lwipCanBindInterface,                  //CanBindInterface.
		lwipAddEthernetInterface,              //AddEthernetInterface.
		lwipDeleteEthernetInterface,           //DeleteEthernetInterface.
		lwipAddGenif,                          //AddGenif.
		lwipDeleteGenif,                       //DeleteGenif.
		lwipMatch,                             //Match.
		lwipAcceptPacket,                      //AcceptPacket.
		lwipDeliveryFrame,                     //DeliveryFrame.
		lwipLinkStatusChange,                  //LinkStatusChange.
		lwipAddGenifAddress,                   //AddGenifAddress.
		lwipSetIPAddress,                      //SetIPAddress.
		lwipResetInterface,                    //ResetInterface.
		lwipStartDHCP,                         //StartDHCP.
		lwipStopDHCP,                          //StopDHCP.
		lwipReleaseDHCP,                       //ReleaseDHCP.
		lwipRenewDHCP,                         //RenewDHCP.
		lwipGetDHCPConfig                      //GetDHCPConfig.
	},
#endif //__CFG_NET_IPv4
#ifdef __CFG_NET_PPPOE
	{
		PPPOE_PROTOCOL_NAME,                    //protocol's name.
		NETWORK_PROTOCOL_TYPE_PPPOE,            //L3 protocol's type.
		NULL,                                   //Protocol extension.
		pppoeInitialize,                        //Initialize.
		pppoeCanBindInterface,                  //CanBindInterface.
		pppoeAddEthernetInterface,              //AddEthernetInterface.
		pppoeDeleteEthernetInterface,           //DeleteEthernetInterface.
		pppoeAddGenif,                          //AddGenif.
		pppoeDeleteGenif,                       //DeleteGenif.
		pppoeMatch,                             //Match.
		pppoeAcceptPacket,                      //AcceptPacket.
		pppoeDeliveryFrame,                     //DeliveryFrame.
		pppoeLinkStatusChange,                  //LinkStatusChange.
		pppoeAddGenifAddress,                   //AddGenifAddress.
		pppoeSetIPAddress,                      //SetIPAddress.
		pppoeResetInterface,                    //ResetInterface.
		pppoeStartDHCP,                         //StartDHCP.
		pppoeStopDHCP,                          //StopDHCP.
		pppoeReleaseDHCP,                       //ReleaseDHCP.
		pppoeRenewDHCP,                         //RenewDHCP.
		pppoeGetDHCPConfig                      //GetDHCPConfig.
	},
#endif //__CFG_NET_PPPOE
	{ 0 }  //The last one must be 0.
};
