//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpihttp.c
//    Module Funciton           : 
//                                Deep Packet Inspection for HTTP protocol.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <KAPI.H>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "dpimgr.h"
#include "dpihttp.h"

/* Response line with 200 OK. */
#define HTTP_RESPLINE_200OK "HTTP/1.1 200 OK"

/* 
 * Replace array,use the second one replace the 
 * first one,if it it present in HTTP response packet. 
 * Please make sure the two strings length must be
 * same.
 */
typedef struct tag__REPLACE_ARRAY {
	char* rep_src;
	char* rep_dst;
}__REPLACE_ARRAY;
static __REPLACE_ARRAY ReplaceArray[] = {
	{"www.baidu.com","www.dubai.com"},
	{"www.jd.com","www.dj.com"},
	{"www.google.com","www.elgoog.com"},
	{"www.hellox.com","www.hellox.org"},
};

/* 
 * Check if a HTTP response is 200 OK. 
 * pContent is the start address of HTTP response packet,without
 * IP or TCP header.
 */
static BOOL checkResponse(const char* pContent)
{
	int index = 0;
	char* _200ok = HTTP_RESPLINE_200OK;
	int head_len = strlen(_200ok);

	if (NULL == pContent)
	{
		return FALSE;
	}
	/* Check if the first line is 200OK. */
	while (head_len)
	{
		if (0 == pContent[index])
		{
			return FALSE;
		}
		if (pContent[index] != _200ok[index])
		{
			/* Can not match 200 OK string. */
			return FALSE;
		}
		index++;
		head_len--;
	}
	/* First line matches 200 OK when reach here. */
	return TRUE;
}

/* Filter function of HTTP's DPI. */
BOOL dpiFilter_HTTP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	struct ip_hdr* pIpHdr = NULL;
	struct tcp_hdr* pTcpHdr = NULL;
	char* pHttpContent = NULL;
	int iph_len = 0;

	pIpHdr = (struct ip_hdr*)p->payload;
	if (IP_PROTO_TCP != pIpHdr->_proto)
	{
		return FALSE;
	}
	iph_len = IPH_HL(pIpHdr);
	iph_len *= 4;
	pTcpHdr = (struct tcp_hdr*)((char*)p->payload + iph_len);
	if ((pTcpHdr->src == _hx_htons(80)) || (pTcpHdr->dest == _hx_htons(80)))
	{
		/* 
		 * If source port or destination port is 80,then assume the session 
		 * is HTTP,so do farther checking.
		 * We only have insteresting with HTTP 200 OK response.
		 */
		pHttpContent = (char*)((char*)p->payload + iph_len + 20);
		if (checkResponse(pHttpContent))
		{
			/* The response is 200 OK. */
			__LOG("%s:HTTP 200 OK response filtered.\r\n", __func__);
			return TRUE;
		}
	}
	return FALSE;
}

/* Action function of HTTP's DPI. */
struct pbuf* dpiAction_HTTP(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir)
{
	/* Do some modification then return it. */
	return p;
}
