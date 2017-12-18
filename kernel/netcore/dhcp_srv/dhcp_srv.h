//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 02,2017
//    Module Name               : dhcp_srv.h
//    Module Funciton           : 
//                                DHCP Server related definitions,macros,types.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __DHCP_SRV_H__
#define __DHCP_SRV_H__

/* Common headers. */
#include <stdint.h>
#include <ethmgr.h>
#include <lwip/ip_addr.h>

/* Enable DHCP server debugging output. */
#define DHCP_DEBUG_PRINTF

#ifdef  DHCP_DEBUG_PRINTF
#define DEBUG_PRINTF __LOG /* Just call system log routine. */
#else
#define DEBUG_PRINTF(...)
#endif /* DHCP_DEBUG_PRINTF */

/* Common DHCP server options. */
#define DHCP_OPTION_DOMAIN_NAME 15

/* DHCP Server kernel thread's name. */
#define DHCP_SERVER_NAME "dhcpd"

/* allocated client ip range */
#ifndef DHCPD_CLIENT_IP_MIN
#define DHCPD_CLIENT_IP_MIN     2
#endif
#ifndef DHCPD_CLIENT_IP_MAX
#define DHCPD_CLIENT_IP_MAX     254
#endif

/* the DHCP server address */
#ifndef DHCPD_SERVER_IPADDR0
#define DHCPD_SERVER_IPADDR0      192UL
#define DHCPD_SERVER_IPADDR1      168UL
#define DHCPD_SERVER_IPADDR2      169UL
#define DHCPD_SERVER_IPADDR3      1UL
#endif

/* List node to record the allocation of one IP address. */
typedef struct tag__DHCP_ALLOCATION{
	struct tag__DHCP_ALLOCATION* pNext;
	struct tag__DHCP_ALLOCATION* pPrev;
	uint8_t hwaddr[ETH_MAC_LEN];
	struct ip_addr ipaddr;
}__DHCP_ALLOCATION;

/* 
 * Entry point of DHCP Server.
 * It will be invoked in process of network subsystem initialization.
 */
BOOL DHCPSrv_Start();

/* Start DHCP server on a given interface. */
BOOL DHCPSrv_Start_Onif(char* ifName);

/* Show out all IP allocations. */
void ShowDhcpAlloc();

#endif //__DHCP_SRV_H__
