//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 02,2017
//    Module Name               : netcfg.h
//    Module Funciton           : 
//                                Network subsystem configure switches,enable
//                                or disable a certain network function by
//                                defining a macro in this file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __NETCFG_H__
#define __NETCFG_H__

#include <StdAfx.h>  /* For system level configuration. */

/* IPv4 or IPv6 stack should be enabled before all other modules. */
#if defined(__CFG_NET_IPv4) || defined(__CFG_NET_IPv6)

/* Default domain name if not specified by user. */
#define DEFAULT_DOMAIN_NAME "HelloX.COM"

/* Domain name length. */
#define DOMAIN_NAME_LENGTH  64

/* Enable DHCP Server module. */
#define __CFG_NET_DHCP_SERVER

/* Enable PPPoE support in HelloX kernel. */
#define __CFG_NET_PPPOE

/* Enable Routing functions in system. */
#define __CFG_NET_ROUTING

/* Enable NAT functions in system. */
#define __CFG_NET_NAT

#endif //defined(__CFG_NET_IPv4) || defined(__CFG_NET_IPv6)

#endif //__NETCFG_H__
