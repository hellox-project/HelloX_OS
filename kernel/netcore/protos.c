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

#include "ethmgr.h"
#include "proto.h"

#ifdef __CFG_NET_IPv4
#include "../network/arch/lwip_pro.h"
#endif

//Network protocol array,each layer3 protocol in system should put one entry
//in this array.
__NETWORK_PROTOCOL NetworkProtocolArray[] = {
#ifdef __CFG_NET_IPv4
	{
		LWIP_PROTOCOL_NAME,                    //protocol's name.
		NETWORK_PROTOCOL_TYPE_IPV4,            //L3 protocol's type.
		lwipInitialize,                        //Initialize.
		lwipCanBindInterface,                  //CanBindInterface.
		lwipAddEthernetInterface,              //AddEthernetInterface.
		lwipDeleteEthernetInterface,           //DeleteEthernetInterface.
		lwipDeliveryFrame,                     //DeliveryFrame.
		lwipSetIPAddress,                      //SetIPAddress.
		lwipShutdownInterface,                 //ShutdownInterface.
		lwipUnshutdownInterface,               //UnshutdownInterface.
		lwipResetInterface,                    //ResetInterface.
		lwipStartDHCP,                         //StartDHCP.
		lwipStopDHCP,                          //StopDHCP.
		lwipReleaseDHCP,                       //ReleaseDHCP.
		lwipRenewDHCP,                         //RenewDHCP.
		lwipGetDHCPConfig                      //GetDHCPConfig.
	},
#endif
	{ 0 }  //The last one must be 0.
};
