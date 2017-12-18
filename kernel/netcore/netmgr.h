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

/* Network manager object. */
typedef struct tag__NETWORK_MANAGER{

	/* Operations that network manager can offer. */
	BOOL (*Initialize)(struct tag__NETWORK_MANAGER* pMgr);
}__NETWORK_MANAGER;

/* Global network manager object. */
extern __NETWORK_MANAGER NetworkManager;

#endif  //__NETMGR_H__
