//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec 29,2014
//    Module Name               : ethentry.c
//    Module Funciton           : 
//                                This module countains the definition of ethernet
//                                driver's entry point array.
//                                Each ethernet driver should provide an entry in 
//                                this array.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <stdio.h>
#include "ethmgr.h"

#ifdef __STM32__

#ifndef __MARVELIF_H__
#include "mrvlwifi/marvelif.h"
#endif

#ifndef __ETHERNET_H__
#include "enc_eth/ethernet.h"
#endif

#endif //__STM32__

#ifdef __CFG_NET_PCNET  //PCNet controller is enabled.
#include "drivers/pcnet.h"
#endif

#ifdef __CFG_NET_RTL8111 //Realtek 8111/8168 NIC
#include "drivers/rtl8111.h"
#endif

#ifdef __CFG_NET_E1000   //Intel e1000 NIC.
//#include "drivers/e1000e.h"
#include "drivers/e1000ex.h"
#endif

__ETHERNET_DRIVER_ENTRY EthernetDriverEntry[] = 
{
#ifdef __CFG_NET_MARVELLAN
    {Marvel_Initialize,NULL},
#endif

#ifdef __CFG_NET_ENC28J60
    {Ethernet_Initialize,NULL},
#endif

#ifdef __CFG_NET_PCNET
	{ PCNet_Drv_Initialize, NULL },
#endif

#ifdef __CFG_NET_RTL8111
	{ RTL8111_Drv_Initialize, NULL },
#endif

#ifdef __CFG_NET_E1000
	//{ E1000_Drv_Initialize, NULL },
	//{ E1000E_Drv_Initialize, NULL },
	{ E1000EX_Drv_Initialize, NULL },
#endif

  //Please add your ethernet driver's entry here.
  {NULL,NULL}
};
