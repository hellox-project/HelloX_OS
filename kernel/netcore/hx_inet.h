//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,22 2015
//    Module Name               : proto.h
//    Module Funciton           : 
//                                Network common functions or structures are
//                                defined in this file,and implemented in hx_inet.c.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HX_INET_H__
#define __HX_INET_H__

//Host order to network order.
__u32 _hx_htonl(__u32 n);
__u16 _hx_htons(__u16 n);

//Network order to host order.
__u32 _hx_ntohl(__u32 n);
__u16 _hx_ntohs(__u16 n);

#endif  //__HX_INET_H__
