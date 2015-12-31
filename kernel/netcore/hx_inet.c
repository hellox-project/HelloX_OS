//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,22 2015
//    Module Name               : proto.h
//    Module Funciton           : 
//                                Network common functions or structures are
//                                implemented in this file.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include "hx_inet.h"

//Host order to network order.
__u32 _hx_htonl(__u32 n)
{
	return ((n & 0xff) << 24) |
		((n & 0xff00) << 8) |
		((n & 0xff0000UL) >> 8) |
		((n & 0xff000000UL) >> 24);
}

__u16 _hx_htons(__u16 n)
{
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

//Network order to host order.
__u32 _hx_ntohl(__u32 n)
{
	return _hx_htonl(n);
}

__u16 _hx_ntohs(__u16 n)
{
	return _hx_htons(n);
}