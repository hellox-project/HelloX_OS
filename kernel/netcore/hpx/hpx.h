//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec 16,2018
//    Module Name               : hpx.h
//    Module Funciton           : 
//                                Constants,objects,structures and 
//                                routines definition of HelloX Packet eXchange
//                                (HPX) protocol.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HPX_H__
#define __HPX_H__

/* Common headers for generic data type and configurations. */
#include <config.h>
#include <stdint.h>

/* Ethernet frame type for HPX protocol,not officially registered yet. */
#define ETH_FRAME_TYPE_HPX 0x9900

/* HPX address structure. */
typedef struct tag__HPX_ADDRESS {
	uint64_t endpoint_addr;
	uint64_t domain_addr[1];
}__HPX_ADDRESS;

#endif //__HPX_H__
