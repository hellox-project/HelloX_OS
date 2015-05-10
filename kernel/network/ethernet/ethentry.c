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

#ifndef __STDAFX_H__
 #include "StdAfx.h"
 #endif
 
 #ifndef __KAPI_H__
 #include "kapi.h"
 #endif

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "string.h"
#include "stdio.h"

#include "ethif.h"

#ifdef __STM32__

#ifndef __MARVELIF_H__
#include "mrvlwifi/marvelif.h"
#endif

#ifndef __ETHERNET_H__
#include "enc_eth/ethernet.h"
#endif

#endif //__STM32__

__ETHERNET_DRIVER_ENTRY EthernetDriverEntry[] = 
{
#ifdef __CFG_NET_MARVELLAN
	{Marvel_Initialize,NULL},
#endif

#ifdef __CFG_NET_ENC28J60
  {Ethernet_Initialize,NULL},
#endif

  //Please add your ethernet driver's entry here.
  {NULL,NULL}
};
