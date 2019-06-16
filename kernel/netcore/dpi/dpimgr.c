//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpimgr.c
//    Module Funciton           : 
//                                Deep Packet Inspection function of HelloX,encapsulated
//                                as DPI Manager and implemented in this file.
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

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "lwip/stats.h"
#include "lwip/sockets.h"

#include "hx_inet.h"
#include "netcfg.h"
#include "ethmgr.h"
#include "dpimgr.h"

/* Initializer of this object. */
static BOOL _Initialize(__DPI_MANAGER* pMgr)
{
	BUG_ON(NULL == pMgr);
	return TRUE;
}

/* Will be invoked when pakcet into an interface. */
static struct pbuf* _dpiPacketIn(struct pbuf* p, struct netif* in_if)
{
	__DPI_ENGINE* pEngine = &DPIEngineArray[0];
	struct pbuf* retp = NULL;

	while (pEngine->dpiFilter)
	{
		if (pEngine->dpiFilter(p, in_if, dpi_in))
		{
			/* Pass the filter,apply corresponding action. */
			retp = pEngine->dpiAction(p, in_if, dpi_in);
			/* First match works. */
			break;
		}
		pEngine++;
	}
	if (retp == NULL)
	{
		/* Should release the pbuf object. */
		pbuf_free(p);
	}
	return retp;
}

/* Output direction of DPI processing. */
static struct pbuf* _dpiPacketOut(struct pbuf* p, struct netif* out_if)
{
	__DPI_ENGINE* pEngine = &DPIEngineArray[0];
	struct pbuf* retp = NULL;

	while (pEngine->dpiFilter)
	{
		if (pEngine->dpiFilter(p, out_if, dpi_out))
		{
			/* Pass the filter,apply corresponding action. */
			retp = pEngine->dpiAction(p, out_if, dpi_out);
			/* First match works. */
			break;
		}
		pEngine++;
	}
	if (retp == NULL)
	{
		/* Should release the pbuf object. */
		pbuf_free(p);
	}
	return retp;
}

/* Enable or disable DPI on a specified interface. */
static BOOL _dpiEnable(char* if_name, BOOL bEnable)
{
	struct netif* netif = netif_list;

	BUG_ON(NULL == if_name);
	if (strlen(if_name) < sizeof(netif->name))
	{
		return FALSE;
	}
	/* Find the netif in global list. */
	while (netif)
	{
		if ((netif->name[0] == if_name[0]) && (netif->name[1] == if_name[1]))
		{
			break;
		}
		netif = netif->next;
	}
	if (NULL == netif)
	{
		return FALSE;
	}
	/* Set or clear NAT flag of the interface according bEnable. */
	if (bEnable)
	{
		netif->flags |= NETIF_FLAG_DPI;
	}
	else
	{
		netif->flags &= (~NETIF_FLAG_DPI);
	}
	return TRUE;
}

/*
 * Send a raw IP packet to the specified IP address.
 * The source IP address can be specified even it's
 * not any one of local interface's IP address.
 * The protocol value also must be specified,it's can
 * be any value of 8 bits.
 * This routine can be used to construct and send out
 * any IP packet without almost any restrict.
 */
static BOOL __dpiSendRaw(ip_addr_t* src_addr, ip_addr_t* dst_addr, int protocol, void* pBuffer, int buff_length)
{
	BOOL bResult = FALSE;
#if 0 /* Issue pending to resolve. */
	int sock = 0, ret = 0;
	struct sockaddr_in local_addr;
	struct sockaddr_in to_addr;

	/* Use raw socket to send the packet. */
	if ((sock = lwip_socket(AF_INET, SOCK_RAW, protocol)) < 0)
	{
		_hx_printf("  dpiSendRaw: failed to create socket.\r\n");
		goto __TERMINAL;
	}
	/* 
	 * Bind socket to the src_addr so this address can 
	 * be set as source IP address of the packet. 
	 */
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = src_addr->addr;
	memset(local_addr.sin_zero, 0, sizeof(local_addr.sin_zero));
	ret = lwip_bind(sock, (const struct sockaddr*)&local_addr, sizeof(local_addr));
	if (ret < 0)
	{
		_hx_printf("  dpiSendRaw: fail to bind local addr[%d].\r\n", ret);
		goto __TERMINAL;
	}
	/* Send the raw packet now. */
	to_addr.sin_family = AF_INET;
	to_addr.sin_addr.s_addr = dst_addr->addr;
	to_addr.sin_port = 0;
	memset(to_addr.sin_zero, 0, sizeof(to_addr.sin_zero));
	ret = lwip_sendto(sock, pBuffer, buff_length, 0, (const struct sockaddr*)&to_addr, sizeof(to_addr));
	if (ret < 0)
	{
		_hx_printf("  dpiSendRaw: failed to send the packet[%d].\r\n", ret);
		goto __TERMINAL;
	}
	/* Destroy the socket. */
	lwip_close(sock);
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (sock >= 0)
		{
			lwip_close(sock);
		}
	}
#endif
	return bResult;
}

/*
* Send a raw IP packet to the specified IP address.
* The source IP address can be specified even it's
* not any one of local interface's IP address.
* The protocol value also must be specified,it's can
* be any value of 8 bits.
* This routine can be used to construct and send out
* any IP packet without almost any restrict.
*/
static BOOL _dpiSendRaw(ip_addr_t* src_addr, ip_addr_t* dst_addr, 
	uint8_t protocol, uint8_t ttl, uint8_t tos,
	uint16_t hdr_id, char* pBuffer, int buff_length)
{
	BOOL bResult = FALSE;
	struct pbuf* p = NULL;
	struct ip_hdr *iphdr;
	u32_t chk_sum = 0;
	int i = 0;
	struct pbuf* q = NULL;

	/* Mandatory parameters checking. */
	if ((NULL == src_addr) || (NULL == dst_addr) || (NULL == pBuffer) || (0 == buff_length))
	{
		goto __TERMINAL;
	}

	/*
	 * Create a new pbuf to hold the data and IP header. 
	 * The extra space for link layer header also is reserved.
	 */
	p = pbuf_alloc(PBUF_RAW, IP_HLEN + buff_length + MAX_L2_HEADER_LEN, PBUF_POOL);
	if (NULL == p)
	{
		IP_STATS_INC(ip.memerr);
		goto __TERMINAL;
	}

	/* Make room for IP header. */
	if (pbuf_header(p, -(IP_HLEN + MAX_L2_HEADER_LEN)))
	{
		IP_STATS_INC(ip.err);
		goto __TERMINAL;
	}

	/* Copy user user specified data into pbuf. */
	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, &pBuffer[i], q->len);
		i = i + q->len;
	}

	/* Generate IP header. */
	{
		u16_t ip_hlen = IP_HLEN;
		if (pbuf_header(p, IP_HLEN)) {
			LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("ip_output: not enough room for IP header in pbuf\r\n"));
			IP_STATS_INC(ip.err);
			goto __TERMINAL;
		}

		iphdr = (struct ip_hdr *)p->payload;
		LWIP_ASSERT("check that first pbuf can hold struct ip_hdr",
			(p->len >= sizeof(struct ip_hdr)));

		IPH_TTL_SET(iphdr, ttl);
		IPH_PROTO_SET(iphdr, protocol);
		chk_sum += LWIP_MAKE_U16(protocol, ttl);

		/* dest cannot be NULL here */
		ip_addr_copy(iphdr->dest, *dst_addr);
		chk_sum += ip4_addr_get_u32(&iphdr->dest) & 0xFFFF;
		chk_sum += ip4_addr_get_u32(&iphdr->dest) >> 16;

		IPH_VHLTOS_SET(iphdr, 4, ip_hlen / 4, tos);
		chk_sum += iphdr->_v_hl_tos;
		IPH_LEN_SET(iphdr, htons(p->tot_len));
		chk_sum += iphdr->_len;
		IPH_OFFSET_SET(iphdr, 0);
		IPH_ID_SET(iphdr, htons(hdr_id));
		chk_sum += iphdr->_id;

		/* src cannot be NULL here */
		ip_addr_copy(iphdr->src, *src_addr);

		chk_sum += ip4_addr_get_u32(&iphdr->src) & 0xFFFF;
		chk_sum += ip4_addr_get_u32(&iphdr->src) >> 16;
		chk_sum = (chk_sum >> 16) + (chk_sum & 0xFFFF);
		chk_sum = (chk_sum >> 16) + chk_sum;
		chk_sum = ~chk_sum;
		iphdr->_chksum = (u16_t)chk_sum; /* network order */
	}

	/* Everything is OK, send the pbuf. */
	bResult = (tcpip_output(p, NULL) == ERR_OK);

__TERMINAL:
	return bResult;
}

/* Copy data in pbuf to a consecutive buffer. */
static char* _copy_from_pbuf(struct pbuf* p)
{
	struct pbuf* q = p;
	int i = 0;
	char* pBuffer = NULL;

	BUG_ON(NULL == p);
	/* Allocate the consecutive buffer. */
	pBuffer = (char*)_hx_malloc(p->tot_len + 16);
	if (NULL == pBuffer)
	{
		return NULL;
	}
	for (q = p; q != NULL; q = q->next)
	{
		memcpy(&pBuffer[i], q->payload, q->len);
		i += (int)q->len;
	}
	return pBuffer;
}

/* The global DPI Manager object. */
__DPI_MANAGER DPIManager = {
	_Initialize,      /* Initialize. */
	_dpiEnable,       /* dpiEnable. */
	_dpiPacketIn,     /* dpiPacketIn. */
	_dpiPacketOut,    /* dpiPacketOut. */
	_dpiSendRaw,      /* dpiSendRaw. */
	_copy_from_pbuf,  /* copy_from_pbuf. */
};
