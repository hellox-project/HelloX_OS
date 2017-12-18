//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,03 2017
//    Module Name               : nattcp.h
//    Module Funciton           : 
//                                TCP protocol's NAT related constants,types,
//                                proto-types.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NATTCP_H__
#define __NATTCP_H__

/* NAT common header file. */
#include "nat.h"
#include "lwip/tcp_impl.h" /* TCP specific header. */

/* TCP option kind value. */
#define TCP_OPTION_KIND_END   0
#define TCP_OPTION_KIND_NOP   1
#define TCP_OPTION_KIND_MSS   2

/* TCP header's fixed length,without options. */
#define TCP_FIXED_HEADER_LEN  20
/* IP header's fixed length. */
#define IP_FIXED_HEADER_LEN   20

/* Entry point of TCP NAT. */
void TcpTranslation(__EASY_NAT_ENTRY* pEntry, struct tcp_hdr* pTcpHdr, __PACKET_DIRECTION dir);

#endif //__NATTCP__
