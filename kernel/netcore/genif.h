//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 19,2020
//    Module Name               : genif.h
//    Module Funciton           : 
//                                Generic network interface's definitions and
//                                constants.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

/*
 * Hirarchy of generic network interface in HelloX:
 * NetworkManager.pGenifRoot-->[Genif0]-->[Genif1]-->...-->[Genifn]
 *                               /
 *                              /
 * Sibling list:             [Genif]-->[Genif]-->[Genif]...
 *                                        /
 *                                       /
 *                                    [Genif]-->[Genif]-->...
 * Description:
 *   1. Each generic interface has four pointers pointing to it's parent,
 *      it's siblings, it's children,and also be linked into a global list;
 *   2. An example of generic network interface that follow this hirarchy 
 *      is ethernet,the physical ethernet interface is the first level,
 *      then the VLAN sub-interface based on this physical ethernet is the
 *      second level,there maybe pppoe session based on VLAN sub-interface,
 *      so the pppoe virtual interface is the third level,and so on;
 *   3. The parent pointer points to the based interface of current one;
 *   4. The sibling pointer links all generic interfaces belong to same
 *      parent interface;
 *   5. The child pointer points to the first child of current interface;
 *   6. All generic network interfaces are linked together by the next
 *      pointer of it's definition;
 *   7. pGenifRoot and pGenifLast pointers in Network Manager point to
 *      the first one and last one of generic network interfaces in list;
 *   8. Dedicated routines are defined in Network Manager to operate on
 *      generic network interfaces;
 *   9. The generic network interface is used to replace netif and ethernet
 *      interface in HelloX,and will become the unified network interface
 *      model for HelloX;
 */

#ifndef __GENIF_H__
#define __GENIF_H__

#include <TYPES.H>
#include "mii.h"
#include "netcfg.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"

/* Constants for generic network interface. */
#define GENIF_NAME_LENGTH  MAX_NETIF_NAME_LENGTH
#define GENIF_HARD_ADDRLEN 16

/* Maximal address could be asigned on genif. */
#define GENIF_MAX_ADDRESS 4

/* Address type definitions. */
#define NETWORK_ADDRESS_TYPE_NONE      0x00
#define NETWORK_ADDRESS_TYPE_ETHMAC    0x01
#define NETWORK_ADDRESS_TYPE_IPV4      0x02
#define NETWORK_ADDRESS_TYPE_IPV6      0x04
#define NETWORK_ADDRESS_TYPE_HPX       0x08

/* 
 * Common network address,used to 
 * contain any type of network address.
 */
typedef struct tag__COMMON_NETWORK_ADDRESS {
	/* Address type,see definitions above. */
	__u8 AddressType;

	/* Actual address value. */
	union {
		__u32 ipv4_addr;
		__u8  eth_mac[6];
		__u8  ipv6_addr[16];
		int   mask_length;
	}Address;
}__COMMON_NETWORK_ADDRESS;

/* Link types of genif. */
#define GENIF_LINKTYPE_UNKNOWN    0
#define GENIF_LINKTYPE_ETHERNET   1
#define GENIF_LINKTYPE_VLANIF     2
#define GENIF_LINKTYPE_PPPOE      3
#define GENIF_LINKTYPE_PPPOS      4
#define GENIF_LINKTYPE_TUNNEL     5
#define GENIF_LINKTYPE_LOOPBACK   6

/* Link types. */
enum __LINK_TYPE {
	lt_unknown = GENIF_LINKTYPE_UNKNOWN,
	lt_ethernet = GENIF_LINKTYPE_ETHERNET,
	lt_vlanif = GENIF_LINKTYPE_VLANIF,
	lt_pppoe = GENIF_LINKTYPE_PPPOE,
	lt_pppos = GENIF_LINKTYPE_PPPOS,
	lt_tunnel = GENIF_LINKTYPE_TUNNEL,
	lt_loopback = GENIF_LINKTYPE_LOOPBACK
};

/* Ethernet interface duplex mode. */
enum __DUPLEX {
	half = 0,
	full = 1
};

/* Ethernet speed options. */
enum __ETHERNET_SPEED {
	_10M = 0,
	_100M = 1,
	_1000M = 2,
	_10G = 3,
	_40G = 4,
	_80G = 5,
	_100G = 6
};

/* Link status. */
enum __LINK_STATUS {
	down = 0,
	up = 1
};

/* Statistics of genif. */
typedef struct tag__GENIF_STAT {
	/* Layer2 counters. */
	unsigned long tx_pkt;
	unsigned long rx_pkt;
	unsigned long tx_bytes;
	unsigned long rx_bytes;
	unsigned long tx_success;
	unsigned long rx_success;
	unsigned long tx_bcast;
	unsigned long rx_bcast;
	unsigned long tx_mcast;
	unsigned long rx_mcast;
	unsigned long tx_err;
	unsigned long rx_err;
#if defined(__CFG_NET_IPv4)
	unsigned long tx_ip;
	unsigned long rx_ip;
#endif
#if defined(__CFG_NET_IPv6)
	unsigned long tx_ip6;
	unsigned long rx_ip6;
#endif
#if defined(__CFG_NET_HPX)
	unsigned long tx_hpx;
	unsigned long rx_hpx;
#endif 
}__GENIF_STAT;

/*
 * Network protocol bound to current genif
 * and it's genif specific state. pProtocol points
 * to one entry in global protocol array.
 */
typedef struct tag__GENIF_PROTOCOL_BINDING {
	LPVOID pProtocol;
	LPVOID pIfState;
}__GENIF_PROTOCOL_BINDING;

/* Maximal protocol number can be bound to one genif. */
#define GENIF_MAX_PROTOCOL_BINDING 8

 /* Generic network interface. */
struct __GENERIC_NETIF {
	/* Counter of this object. */
	__atomic_t if_count;
	/* genif index. */
	unsigned long if_index;

	/* name,mtu,hw address,vlan ID. */
	char genif_name[GENIF_NAME_LENGTH];
	uint16_t genif_mtu;
	char genif_ha[GENIF_HARD_ADDRLEN];
	char ha_len;
	unsigned short vlan_id;

	/* Link type,genif's flags. */
	enum __LINK_TYPE link_type;
	unsigned long genif_flags;

	/* genif state. */
	enum __LINK_STATUS link_status;
	enum __DUPLEX link_duplex;
	enum __ETHERNET_SPEED link_speed;

	/* Statistic counters of the genif. */
	__GENIF_STAT stat;

	/* Private info for driver. */
	LPVOID pGenifExtension;

	/* 
	 * Protocols that bind to this genif, and it's
	 * interface specific state information.
	 * Each protocol that bind to this genif success will
	 * occupy one entry of this array.
	 */
	__GENIF_PROTOCOL_BINDING proto_binding[GENIF_MAX_PROTOCOL_BINDING];
	
#if defined(__CFG_NET_IPv4)
	/* IPv4 addresses. */
	ip_addr_t ip_addr[GENIF_MAX_ADDRESS];
	ip_addr_t ip_mask[GENIF_MAX_ADDRESS];
	ip_addr_t ip_gw[GENIF_MAX_ADDRESS];

	/* Protocol extension for this genif. */
	LPVOID ip_proto_ext;
#endif

#if defined(__CFG_NET_IPv6)
	/* IPv6 addresses. */
	ip6_addr_t ip6_addr[GENIF_MAX_ADDRESS];
	unsigned char ip6_pfxlen[GENIF_MAX_ADDRESS];
	ip6_addr_t ip6_gw[GENIF_MAX_ADDRESS];

	/* Protocol externsion for this genif. */
	LPVOID ip6_proto_ext;
#endif

#if defined(__CFG_NET_HPX)
	/* HelloX Packet eXchange addresses. */
	hpx_addr_t hpx_addr[GENIF_MAX_ADDRESS];
	hpx_addr_t hpx_gw[GENIF_MAX_ADDRESS];

	/* Protocol extension for this genif. */
	LPVOID hpx_proto_ext;
#endif

	/* Input and output routines. */
	int (*genif_output)(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt);
	int (*genif_input)(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt);
	/* Driver specific operation routine. */
	int (*genif_ioctrl)(struct __GENERIC_NETIF* pGenif, unsigned long operations,
		LPVOID pInputBuff, unsigned long input_sz,
		LPVOID pOutputBuff, unsigned long output_sz);
	/* Destructor of this genif. */
	int (*genif_destructor)(struct __GENERIC_NETIF* pGenif);

	/*
	 * Driver specific show routine,used for 
	 * device level debugging. 
	 */
	void (*specific_show)(struct __GENERIC_NETIF* pGenif);

	/* Points to next one in genif list. */
	struct __GENERIC_NETIF* pGenifNext;
	/* Parent,child,sibling pointers. */
	struct __GENERIC_NETIF* pGenifParent;
	struct __GENERIC_NETIF* pGenifChild;
	struct __GENERIC_NETIF* pGenifSibling;
};

typedef struct __GENERIC_NETIF __GENERIC_NETIF;

/* Destructor routine prototype. */
typedef int(*__GENIF_DESTRUCTOR)(__GENERIC_NETIF* pGenif);

#endif //__GENIF_H__
