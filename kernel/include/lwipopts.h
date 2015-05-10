//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : lwipopts.h
//    Module Funciton           : 
//                                Options definition file for lwIP,it is required
//                                by lwIP stack.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __STDLIB_H__
#include "stdlib.h"      //For memory allocation.
#endif

#ifndef __STDIO_H__
#include "stdio.h"
#endif

#ifndef __LWIPOPTS_H__

//Swich of debugging functions.
//#define LWIP_DEBUG           1

//Use binary semaphore for mutex.
#define LWIP_COMPAT_MUTEX    1

//Use light weight protection.
#define SYS_LIGHTWEIGHT_PROT 1

//Use standard C memory manipulation routines,such as malloc/free.
#define MEM_LIBC_MALLOC      1

//Use HelloX's memory allocator to allocate memory pool.
#define MEMP_MEM_MALLOC      1

//Enable loop back interface.
#define LWIP_HAVE_LOOPIF     1

//Enable receive timeout mechanism.
#define LWIP_SO_RCVTIMEO     1

//Enable or disable TCP functions in lwIP.
#define LWIP_TCP             1

//Enable or disable DHCP client functions.
#define LWIP_DHCP            1

//Enable or disable DNS functions in lwIP.
#define LWIP_DNS             1

//Change the default value(3) to larger number,since DHCP is enabled.
#define MEMP_NUM_SYS_TIMEOUT 8

//Disable the "purge oldest pbuf function" in IP fragment,since it
//will lead ASSERT failed in ip_reass_free_complete_datagram routine,
//in ip_frag.c file.
#define IP_REASS_FREE_OLDEST 0

//*-----------------------------------------------------------------------
//*
//*  Thread options for lwIP's internal threads.
//*
//*-----------------------------------------------------------------------

//TCPIP thread's name.
#define TCPIP_THREAD_NAME                "tcpip_thread"

//TCPIP thread's stack size,use default value.
#define TCPIP_THREAD_STACKSIZE           0x00000800  //2K stack.

//TCPIP thread's priority.
#define TCPIP_THREAD_PRIO                PRIORITY_LEVEL_HIGH

//Mail box's size for TCP/IP thread.Increase this value may improve performance.
#define TCPIP_MBOX_SIZE                  8

//SLIP interface's thread name.
#define SLIPIF_THREAD_NAME               "slipif_loop"

//Stack size of slipif.
#define SLIPIF_THREAD_STACKSIZE          0

//SLIPIF thread's priority.
#define SLIPIF_THREAD_PRIO               PRIORITY_LEVEL_NORMAL

//Thread name of PPP function.
#define PPP_THREAD_NAME                  "ppp_in_thrd"

//Stack size of PPP thread.
#define PPP_THREAD_STACKSIZE             0

//Priority level of PPP thread.
#define PPP_THREAD_PRIO                  PRIORITY_LEVEL_NORMAL

//Default value for other lwIP thread.
#define DEFAULT_THREAD_NAME              "lwIP stack"
#define DEFAULT_THREAD_STACKSIZE         0
#define DEFAULT_THREAD_PRIO              PRIORITY_LEVEL_NORMAL

//*-----------------------------------------------------------------------
//*
//*  Debugging switchs for each function module in lwIP.
//*
//*-----------------------------------------------------------------------
/**
* NETIF_DEBUG: Enable debugging in netif.c.
*/
#ifndef NETIF_DEBUG
#define NETIF_DEBUG                     LWIP_DBG_ON
#endif

/**
* PBUF_DEBUG: Enable debugging in pbuf.c.
*/
#ifndef PBUF_DEBUG
#define PBUF_DEBUG                      LWIP_DBG_ON
#endif

/**
* API_LIB_DEBUG: Enable debugging in api_lib.c.
*/
#ifndef API_LIB_DEBUG
#define API_LIB_DEBUG                   LWIP_DBG_ON
#endif

/**
* API_MSG_DEBUG: Enable debugging in api_msg.c.
*/
#ifndef API_MSG_DEBUG
#define API_MSG_DEBUG                   LWIP_DBG_ON
#endif

/**
* SOCKETS_DEBUG: Enable debugging in sockets.c.
*/
#ifndef SOCKETS_DEBUG
#define SOCKETS_DEBUG                   LWIP_DBG_ON
#endif

/**
* IP_DEBUG: Enable debugging for IP.
*/
#ifndef IP_DEBUG
#define IP_DEBUG                        LWIP_DBG_ON
#endif

//Turn DHCP debugging one.
#ifndef DHCP_DEBUG
#define DHCP_DEBUG                      LWIP_DBG_ON
#endif

//Debug output routine.
#define LWIP_PLATFORM_DIAG(x) _hx_printf x

//Format indicators of console output routine(_hx_printf).
#define U8_F  "c"
#define S8_F  "c"
#define X8_F  "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

#endif  //__LWIPOPTS_H__

