//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apri 15,2017
//    Module Name               : hx_eth.h
//    Module Funciton           : 
//                                Ethernet related definitions and data structures,
//                                for HelloX OS,are put here.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HX_ETH_H__
#define __HX_ETH_H__

#include "../stddef.h"
#include "../stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETH_MAC_LEN 6

/*
 * Convert a MAC address to host endian.
 */
int _hx_ntoh_mac(__u8* mac);

/*
 * Compare 2 MAC addresses,returns 0 if equal,-1 otherwise.
 */
BOOL Eth_MAC_Match(__u8* srcMac, __u8* dstMac);

/*
 * Check if a given MAC address is broadcast MAC address.
 */
BOOL Eth_MAC_Broadcast(__u8* mac);

/*
 * Check if a given MAC address is multicast MAC address.
 */
BOOL Eth_MAC_Multicast(__u8* mac);

/*
 * Compare if the given MAC address is broadcast MAC address or
 * multicast MAC address.
 */
BOOL Eth_MAC_BM(__u8* srcMac);

/* Convert MAC to string. */
char* ethmac_ntoa(uint8_t* mac);

#ifdef __cplusplus
}
#endif

#endif //__HX_ETH_H__
