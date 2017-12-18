//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,10 2017
//    Module Name               : naticmp.h
//    Module Funciton           : 
//                                ICMP NAT related constants,object,types.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NATICMP_H__
#define __NATICMP_H__

/* Do ICMP specific initialization for a new NAT entry. */
void InitNatEntry_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr);

/* ICMP specific NAT translation for IN and OUT direction. */
void InTranslation_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct pbuf* pb);
void OutTranslation_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr, struct pbuf* pb);

/* Check if a packet matches the specified NAT entry. */
BOOL InPacketMatch_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr);
BOOL OutPacketMatch_ICMP(__EASY_NAT_ENTRY* pEntry, struct ip_hdr* pHdr);

/* Re calculate the checksum of ICMP packet. */
void _icmp_check_sum(struct ip_hdr* pHdr, struct pbuf* pb);

#endif //__NATICMP_H__
