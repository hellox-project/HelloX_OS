//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,30 2017
//    Module Name               : nat.h
//    Module Funciton           : 
//                                Network Address Translation function related
//                                definitions,objects,and constants...
//                                The NAT function is implemented in HelloX as
//                                2 types,one is named easy NAT,abbreviated as
//                                eNAT,that use the interface's IP address as source
//                                address when do NAT.The other is traditional
//                                NAT,that use a pool of public IP address to replace
//                                the private IP address when forwarding.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NAT_H__
#define __NAT_H__

#include <StdAfx.h>        /* For HelloX common data types and APIs. */
#include <rdxtree.h>       /* For radix tree. */

#include "lwip/pbuf.h"     /* For struct pbuf. */
#include "lwip/netif.h"    /* For struct netif. */

/* Enable or disable NAT debugging. */
//#define NAT_DEBUG

/* Output NAT debugging information. */
#ifdef NAT_DEBUG
#ifdef __MS_VC__
#define __NATDEBUG(fmt,...) _hx_printf(fmt,__VA_ARGS__)
#else
#define __NATDEBUG(fmt,args...) _hx_printf(fmt,##args)
#endif //__MS_VC__
#else
#ifdef __MS_VC__
#define __NATDEBUG(fmt,...)
#else
#define __NATDEBUG(fmt,args...)
#endif //__MS_VC__
#endif  //NAT_DEBUG

/* NAT entry for easy NAT. */
typedef struct tag__EASY_NAT_ENTRY{
	ip_addr_t srcAddr_bef;
	ip_addr_t dstAddr_bef;
	ip_addr_t srcAddr_aft;
	ip_addr_t dstAddr_aft;
	u8_t protocol;
	u16_t srcPort_bef;
	u16_t srcPort_aft;
	u16_t dstPort_bef;
	u16_t dstPort_aft;

	/* IP packet's ID in IP header. */
	u16_t identifier;

	/* Transportation header fields corresponding different protocol. */
	union {
		/* Identifier in ICMP echo req header. */
		u16_t icmp_id;
		/* TCP flags in TCP header,for IN and OUT directions. */
		u8_t tcp_flags_in;
		u8_t tcp_flags_out;
#define TCP_FLAG_MASK 0x3F /* Mask to obtain TCP flags. */
	}tp_header;

	/* Output network interface that easy NAT applied on. */
	struct netif* netif;

	unsigned long ms;
	unsigned long match_times;

	/* All NAT entries are linked into a global bidirection list. */
	struct tag__EASY_NAT_ENTRY* pPrev;
	struct tag__EASY_NAT_ENTRY* pNext;

	/* Vertical list pointer in radix tree. */
	struct tag__EASY_NAT_ENTRY* pHashNext;
}__EASY_NAT_ENTRY;

/* Packet direction,in or out. */
typedef enum {
	in = 0,
	out,
}__PACKET_DIRECTION;

/* Port range for TCP/UDP connection. */
#define NAT_SOURCE_PORT_BEGIN 1025
#define NAT_SOURCE_PORT_END   65535

/* Begin of new ICMP echo ID,each ID identifies one ICMP echo session. */
#define NAT_ICMP_ID_BEGIN 1025

/* 
 * Time periodic in millionsecond that NAT thread scan the NAT entries.
 * All NAT entries will be scaned by NAT manager thread every
 * this period,NAT entry's age out counter(ms in __EASY_NAT_ENTRY)
 * will be increased,and if they reach the pre-defined value,
 * then the NAT entry will be purged.
 */
#define NAT_ENTRY_SCAN_PERIOD    10000 //10s.

/* Predefined time out value for different protocol's NAT entry,
 * in millionseconds.
 */
#define NAT_ENTRY_TIMEOUT_DEF    30*1000
#define NAT_ENTRY_TIMEOUT_TCP    (7200 + 1000)*1000  //Default keepalive interval plus margin.
#define NAT_ENTRY_TIMEOUT_TCPSYN 60*1000
#define NAT_ENTRY_TIMEOUT_TCPRST 60*1000
#define NAT_ENTRY_TIMEOUT_UDP    300*1000
#define NAT_ENTRY_TIMEOUT_ICMP   30*1000
#define NAT_ENTRY_TIMEOUT_DNS    300*1000

/* 
 * Maximal NAT entry number in system,to limit the total NAT entry
 * number in case of abnormal,such as attacking.
 * No limitation of NAT entry number may lead system memory used out
 * and make the system crash.
 */
#define MAX_NAT_ENTRY_NUM 8192

/* NAT background thread's name. */
#define NAT_MAIN_THREAD_NAME "NatMain"

/* Timer ID of periodic timer of NAT. */
#define NAT_PERIODIC_TIMER_ID 2048

/* Packet validating before apply NAT. */
#define NAT_PACKET_VALIDATE(pkt,dir) validatePacket(pkt,dir)

/* Statistics variable of easy. */
typedef struct{
	int entry_num;     //How many NAT entries in system.
	int hash_deep;     //Hash collision deep,how many entries with same hash key.
	__EASY_NAT_ENTRY deepNat; //The nat entry with maximal hash deep value.
	int match_times;   //Total match times of NAT entry,no matter success or fail.
	int trans_times;   //Translate times,no matter success or fail.
}__EASY_NAT_STAT;

/* 
 * All things of NAT,include the global data structures,interfaces,and 
 * NAT related state information,are all arranged into a global object,
 * named NatManager.
 * This object oriented mechanism can make programming easy.
 */
typedef struct tag__NAT_MANAGER{
	/* Radix tree to assit NAT entry searching. */
	__RADIX_TREE* pTree;
	/* NAT entry list. */
	__EASY_NAT_ENTRY entryList;
	//int entry_num;
	__EASY_NAT_STAT stat;
	HANDLE lock; /* Lock to protect NAT entry list. */
	HANDLE hMainThread; /* Main NAT background thread. */

	/* 
	 * NAT interface.The prefix enat means easy NAT.
	 */
	/* Enable or disable easy NAT on a given interface. */
	BOOL (*enatEnable)(char* if_name, BOOL bEnable,BOOL bSetDefault);
	/*
	 * enatPacketIn will be called when a packet arrive to a network
	 * interface,and the bNAT flag of the interface is set.
	 * Return values:
	 * TRUE: the packet is NATed and modified according NAT session;
	 * FALSE: the packet can not be NATed,maybe caused by no session entry.
	 */
	BOOL (*enatPacketIn)(struct pbuf* p,struct netif* in_if);
	/*
	 * enatPacketOut will be called when a packet will be xmited out
	 * through the out_if,and the bNAT flag of interface is set.
	 * Return values:
	 * TRUE: the packet is NATed and the NAT session is created,the
	 *       packet also be modified accordingly.
	 * FALSE: the packet is not NATed for some reason.
	 */
	BOOL (*enatPacketOut)(struct pbuf* p,struct netif* out_if);

	/* Initializer and De-initializer of NAT manager. */
	BOOL (*Initialize)(struct tag__NAT_MANAGER* pMgr);
	VOID (*Uninitialize)(struct tag__NAT_MANAGER* pMgr);

	/* 
	 * Show out NAT session table,ss_num indicates how many 
	 * entry to show.A 0 value means to show out all NAT
	 * entries.
	 */
	VOID (*ShowNatSession)(struct tag__NAT_MANAGER* pMgr, size_t ss_num);
}__NAT_MANAGER;

/* Global NAT manager object. */
extern __NAT_MANAGER NatManager;

#endif //__NAT_H__
