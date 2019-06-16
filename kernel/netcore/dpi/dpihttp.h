//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpihttp.h
//    Module Funciton           : 
//                                Deep Packet Inspection for HTTP protocol.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DPIHTTP_H__
#define __DPIHTTP_H__

/* Filter function of HTTP's DPI. */
BOOL dpiFilter_HTTP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

/* Action function of HTTP's DPI. */
struct pbuf* dpiAction_HTTP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

#endif //__DPIHTTP_H__
