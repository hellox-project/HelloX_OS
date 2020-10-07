//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,22 2015
//    Module Name               : hx_inet.c
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

#include "inet.h"

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

/*
* Calculate check sum,IP header's checksum and TCP/UDP header's
* check sum will be generated by this routine.
*/
__u16 _hx_checksum(__u16* buffer, int size)
{
	unsigned long cksum = 0;
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(__u16);
	}
	if (size)
	{
		cksum += *(unsigned char*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (__u16)(~cksum);
}
