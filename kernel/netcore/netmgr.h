//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 12, 2017
//    Module Name               : netmgr.h
//    Module Funciton           : 
//                                Network manager object's definition and
//                                related constants.
//                                Network manager is the core object of network
//                                subsystem in HelloX,it supplies network common
//                                service,such as timer,common task,...
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NETMGR_H__
#define __NETMGR_H__

/* For HelloX common types. */
#include <StdAfx.h>
#include "genif.h"

/* Network manager object. */
typedef struct tag__NETWORK_MANAGER{
	/* Operations that network manager can offer. */
	BOOL (*Initialize)(struct tag__NETWORK_MANAGER* pMgr);

#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/* Link list of generic network interfaces. */
	__GENERIC_NETIF* pGenifRoot;
	__GENERIC_NETIF* pGenifLast;
	/* How many genif in list. */
	unsigned int genif_num;
	/* Maximal index value of genif. */
	unsigned long genif_index;

	/* Operations on genif,creator and destructor. */
	__GENERIC_NETIF* (*CreateGenif)(__GENERIC_NETIF* parent,
		__GENIF_DESTRUCTOR destructor);
	BOOL (*ReleaseGenif)(__GENERIC_NETIF* pGenif);
	/* Increase refer counter of genif, returns the old value. */
	__atomic_t (*GetGenif)(__GENERIC_NETIF* pGenif);
	/* Get the genif by it's index. */
	__GENERIC_NETIF* (*GetGenifByIndex)(unsigned long genif_index);
	/* Return the genif by it's name. */
	__GENERIC_NETIF* (*GetGenifByName)(const char* genif_name);
	/* Register a generic netif into system. */
	BOOL (*RegisterGenif)(__GENERIC_NETIF* pGenif);

	/* 
	 * Returns all generic network interfaces info. 
	 * The caller should specify a bulk of memory,this routine
	 * will pack all genifs in system into this memory block,with
	 * operation routines set to NULL,buff_req should be set as
	 * the buffer's length by caller,genif's number will be returned
	 * as return value.
	 * 0 will be returned if buffer is not enough and buff_req will
	 * be set by this routine to indicate the required memory,caller
	 * should invoke this routine again with a larger memory buffer.
	 */
	int (*GetGenifInfo)(__GENERIC_NETIF* pGenifList, unsigned long* buff_req);

	/* 
	 * Configure address on the genif. 
	 * All network addresses must be packed into an array
	 * of common network address,and the addr_num specifies
	 * how many addresses in this array. For example,
	 * the ipv4 addr/mask/gw must be packed into an array /w
	 * 3 elements and addr_num must be assign 3.
	 */
	int (*AddGenifAddress)(unsigned long genif_index, 
		unsigned long protocol,
		__COMMON_NETWORK_ADDRESS* comm_addr,
		int addr_num,
		BOOL bSecondary);

	/* Show out one or all genif in system. */
	int (*ShowGenif)(int nGenifIndex);

	/* Handle genif's link status change event. */
	void (*LinkStatusChange)(__GENERIC_NETIF* pGenif,
		enum __LINK_STATUS link_status,
		enum __DUPLEX duplex,
		enum __ETHERNET_SPEED speed);

}__NETWORK_MANAGER;

/* Global network manager object. */
extern __NETWORK_MANAGER NetworkManager;

#endif  //__NETMGR_H__
