//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 13, 2017
//    Module Name               : lwipext.h
//    Module Funciton           : 
//                                Extension of lwIP protocol.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __LWIPEXT_H__
#define __LWIPEXT_H__

#include <config.h>
#include <TYPES.H>
#include "proto.h"  /* for __NETWORK_PROTOCOL object. */

/* 
 * Structure to hold one incoming IP packet,which is posted to TCP/IP main 
 * thread to process.All packets will be linked into one link list.
 */
typedef struct tag__INCOME_IP_PACKET{
	struct pbuf* p;
	struct netif* in_if;
	struct tag__INCOME_IP_PACKET* pNext;
}__INCOME_IP_PACKET;

/* Extension object of lwIP protocol stack. */
typedef struct tag__LWIP_EXTENSION{
	__INCOME_IP_PACKET* pIncomePktFirst;
	__INCOME_IP_PACKET* pIncomePktLast;
	volatile int nIncomePktSize;
	/* Global lock to protect the whole protocal intance. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
}__LWIP_EXTENSION;

/* Global lwIP protocol object. */
extern __NETWORK_PROTOCOL* plwipProto;

#endif //__LWIPEXT_H__
