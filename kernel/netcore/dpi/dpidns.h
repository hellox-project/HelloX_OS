//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpidns.h
//    Module Funciton           : 
//                                Deep Packet Inspection for DNS protocol.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DPIDNS_H__
#define __DPIDNS_H__

/* Filter function of DNS's DPI. */
BOOL dpiFilter_DNS(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

/* Action function of DNS's DPI. */
struct pbuf* dpiAction_DNS(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

#endif //__DPIDNS_H__
