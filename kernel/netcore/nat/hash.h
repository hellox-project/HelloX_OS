//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,23 2017
//    Module Name               : hash.h
//    Module Funciton           : 
//                                HASH algorithm that can be used by NAT module
//                                to construct a key value,which is used to locate
//                                a proper NAT entry.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HASH_H__
#define __HASH_H__

#include "lwip/ip.h"  //For struct ip_hdr.
#include "nat.h"  //For __EASY_NAT_ENTRY.

/* HASH key mask,to control the actual bits that hash key contains. */
#define HASH_KEY_MASK 0x00FFFFFF

/* Return a hash key,given an IP header. */
unsigned long GetHashKey(struct ip_hdr* pHdr);

/* Return a hash key according packet direction,given an IP header. */
unsigned long enatGetHashKeyByHdr(struct ip_hdr* pHdr, __PACKET_DIRECTION dir);

/* Return a hash key according packet direction,given an NAT entry. */
unsigned long enatGetHashKeyByEntry(__EASY_NAT_ENTRY* pEntry, __PACKET_DIRECTION dir);

#endif //__HASH_H__
