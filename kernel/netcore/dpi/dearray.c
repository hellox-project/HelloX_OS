//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dearray.c
//    Module Funciton           : 
//                                DPI engine array resides in this file.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <KAPI.H>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "dpimgr.h"

/* DPI engines header. */
#include "dpiicmp.h"
#include "dpidns.h"
#include "dpihttp.h"
#include "dpiland.h"

/* DPI engine array. */
__DPI_ENGINE DPIEngineArray[] = {
	{dpiFilter_land, dpiAction_land },
	//{dpiFilter_ICMP,dpiAction_ICMP},
	//{dpiFilter_DNS,dpiAction_DNS},
	//{dpiFilter_HTTP,dpiAction_HTTP},
	/* Terminator of this array. */
	{NULL,NULL},
};
