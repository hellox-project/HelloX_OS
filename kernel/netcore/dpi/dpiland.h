//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpidns.h
//    Module Funciton           : 
//                                Deep Packet Inspection to anti-LAND attack.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DPILAND_H__
#define __DPILAND_H__

/* Filter function of ANTI LAND attack. */
BOOL dpiFilter_land(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

/* Action function of ANTI LAND attack. */
struct pbuf* dpiAction_land(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);

#endif //__DPILAND_H__
