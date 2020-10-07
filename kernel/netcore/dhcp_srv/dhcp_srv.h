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
#include <netmgr.h>
#include <lwip/ip_addr.h>
#include <lwip/sockets.h>

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

/* dhcp server and client port. */
#define DHCP_PORT_SERVER 67
#define DHCP_PORT_CLIENT 68

/* buffer size for incoming DHCP packet */
#define DHCP_RECV_BUFSZ 1024

/* Timeout value for waiting client messages in second. */
#define DHCP_WAIT_TIMEOUT 5

#define DHCP_CHADDR_LEN 16U
#define DHCP_SNAME_LEN  64U
#define DHCP_FILE_LEN   128U

/** DHCP message item offsets and length */
#define DHCP_OP_OFS       0
#define DHCP_HTYPE_OFS    1
#define DHCP_HLEN_OFS     2
#define DHCP_HOPS_OFS     3
#define DHCP_XID_OFS      4
#define DHCP_SECS_OFS     8
#define DHCP_FLAGS_OFS    10
#define DHCP_CIADDR_OFS   12
#define DHCP_YIADDR_OFS   16
#define DHCP_SIADDR_OFS   20
#define DHCP_GIADDR_OFS   24
#define DHCP_CHADDR_OFS   28
#define DHCP_SNAME_OFS    44
#define DHCP_FILE_OFS     108
#define DHCP_MSG_LEN      236

#define DHCP_COOKIE_OFS   DHCP_MSG_LEN
#define DHCP_OPTIONS_OFS  (DHCP_MSG_LEN + 4)

/** DHCP client states */
#define DHCP_OFF          0
#define DHCP_REQUESTING   1
#define DHCP_INIT         2
#define DHCP_REBOOTING    3
#define DHCP_REBINDING    4
#define DHCP_RENEWING     5
#define DHCP_SELECTING    6
#define DHCP_INFORMING    7
#define DHCP_CHECKING     8
#define DHCP_PERMANENT    9
#define DHCP_BOUND        10
/** not yet implemented #define DHCP_RELEASING 11 */
#define DHCP_BACKING_OFF  12

/** AUTOIP cooperatation flags */
#define DHCP_AUTOIP_COOP_STATE_OFF  0
#define DHCP_AUTOIP_COOP_STATE_ON   1

#define DHCP_BOOTREQUEST  1
#define DHCP_BOOTREPLY    2

/** DHCP message types */
#define DHCP_DISCOVER 1
#define DHCP_OFFER    2
#define DHCP_REQUEST  3
#define DHCP_DECLINE  4
#define DHCP_ACK      5
#define DHCP_NAK      6
#define DHCP_RELEASE  7
#define DHCP_INFORM   8

/** DHCP hardware type, currently only ethernet is supported */
#define DHCP_HTYPE_ETH 1

#define DHCP_MAGIC_COOKIE 0x63825363UL

/* This is a list of options for BOOTP and DHCP, see RFC 2132 for descriptions */

/** BootP options */
#define DHCP_OPTION_PAD 0
#define DHCP_OPTION_SUBNET_MASK 1 /* RFC 2132 3.3 */
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_DNS_SERVER 6 
#define DHCP_OPTION_HOSTNAME 12
#define DHCP_OPTION_IP_TTL 23
#define DHCP_OPTION_MTU 26
#define DHCP_OPTION_BROADCAST 28
#define DHCP_OPTION_TCP_TTL 37
#define DHCP_OPTION_END 255

/** DHCP options */
#define DHCP_OPTION_REQUESTED_IP 50 /* RFC 2132 9.1, requested IP address */
#define DHCP_OPTION_LEASE_TIME 51 /* RFC 2132 9.2, time in seconds, in 4 bytes */
#define DHCP_OPTION_OVERLOAD 52 /* RFC2132 9.3, use file and/or sname field for options */

#define DHCP_OPTION_MESSAGE_TYPE 53 /* RFC 2132 9.6, important for DHCP */
#define DHCP_OPTION_MESSAGE_TYPE_LEN 1

#define DHCP_OPTION_SERVER_ID 54 /* RFC 2132 9.7, server IP address */
#define DHCP_OPTION_PARAMETER_REQUEST_LIST 55 /* RFC 2132 9.8, requested option types */

#define DHCP_OPTION_MAX_MSG_SIZE 57 /* RFC 2132 9.10, message size accepted >= 576 */
#define DHCP_OPTION_MAX_MSG_SIZE_LEN 2

#define DHCP_OPTION_T1 58 /* T1 renewal time */
#define DHCP_OPTION_T2 59 /* T2 rebinding time */
#define DHCP_OPTION_US 60
#define DHCP_OPTION_CLIENT_ID 61
#define DHCP_OPTION_TFTP_SERVERNAME 66
#define DHCP_OPTION_BOOTFILE 67

/** possible combinations of overloading the file and sname fields with options */
#define DHCP_OVERLOAD_NONE 0
#define DHCP_OVERLOAD_FILE 1
#define DHCP_OVERLOAD_SNAME  2
#define DHCP_OVERLOAD_SNAME_FILE 3

/* MUST be compiled with "packed". */
#pragma pack(push,1)
/** minimum set of fields of any DHCP message */
struct dhcp_msg
{
	u8_t op;
	u8_t htype;
	u8_t hlen;
	u8_t hops;
	u32_t xid;
	u16_t secs;
	u16_t flags;
	ip_addr_p_t ciaddr;
	ip_addr_p_t yiaddr;
	ip_addr_p_t siaddr;
	ip_addr_p_t giaddr;
	u8_t chaddr[DHCP_CHADDR_LEN];
	u8_t sname[DHCP_SNAME_LEN];
	u8_t file[DHCP_FILE_LEN];
	u32_t cookie;
#define DHCP_MIN_OPTIONS_LEN 68U
	/** make sure user does not configure this too small */
#if ((defined(DHCP_OPTIONS_LEN)) && (DHCP_OPTIONS_LEN < DHCP_MIN_OPTIONS_LEN))
#  undef DHCP_OPTIONS_LEN
#endif
/** allow this to be configured in lwipopts.h, but not too small */
#if (!defined(DHCP_OPTIONS_LEN))
/** set this to be sufficient for your options in outgoing DHCP msgs */
#  define DHCP_OPTIONS_LEN DHCP_MIN_OPTIONS_LEN
#endif
	u8_t options[DHCP_OPTIONS_LEN];
};
#pragma pack(pop)

/* 
 * List node to record the allocation of one 
 * IP address on a dhcp server enabled interface. 
 */
typedef struct tag__DHCP_ALLOCATION{
	struct tag__DHCP_ALLOCATION* pNext;
	struct tag__DHCP_ALLOCATION* pPrev;
	/* The owner's hardware address. */
	uint8_t hwaddr[ETH_MAC_LEN];
	struct ip_addr ipaddr;
}__DHCP_ALLOCATION;

/* 
 * DHCP interface object contains all DHCP server 
 * information and data members associated to one
 * layer 3 interface.
 */
typedef struct tag__DHCP_INTERFACE {
	/* interface index. */
	int if_index;
	/* Next one in list. */
	struct tag__DHCP_INTERFACE* pNext;
	/* Allocation list head. */
	__DHCP_ALLOCATION alloc_head;
	/* Server listening socket on this interface. */
	int sock;
	/* Associated generic netif's index in system. */
	unsigned long genif_index;
	/* Start address of IP pool on this if, default is 2. */
	uint8_t host_start;
	/* Network part of IP pool. */
	uint8_t net_part1;  /* 192L. */
	uint8_t net_part2;  /* 168L. */
	uint8_t net_part3;  /* Start from 169 in descending. */
	/* interface's ip addr, mask and gw. */
	ip_addr_t if_addr;
	ip_addr_t if_mask;
	ip_addr_t if_gw; /* Asign to client as defaut gw. */
	/* Incoming data buffer. */
	char* recv_buff;
}__DHCP_INTERFACE;

/* Start DHCP server on a given interface. */
BOOL DHCPSrv_Start_Onif(char* ifName);

/* start dhcp server after when system initializes. */
BOOL start_dhcp_server();

/* Show out all IP allocations. */
void ShowDhcpAlloc();

#endif //__DHCP_SRV_H__
