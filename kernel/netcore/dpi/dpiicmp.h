//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpiicmp.h
//    Module Funciton           : 
//                                Deep Packet Inspection for ICMP protocol.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DPIICMP_H__
#define __DPIICMP_H__

/* Filter function of ICMP's DPI. */
BOOL dpiFilter_ICMP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

/* Action function of ICMP's DPI. */
struct pbuf* dpiAction_ICMP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

#endif //__DPIICMP_H__
