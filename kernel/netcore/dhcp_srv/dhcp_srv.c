/*
* File      : dhcp_server.c
*             A simple DHCP server implementation
*
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2013-2015, Shanghai Real-Thread Technology Co., Ltd
* http://www.rt-thread.com
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* Change Logs:
* Date           Author       Notes
* 2013-01-30     aozima       the first version
* 2013-08-08     aozima       support different network segments.
* 2015-01-30     bernard      release to RT-Thread RTOS.
*/

/*
 * Ported to HelloX by Garry.Xin,and revised according HelloX's network
 * framework.
 */

#include <StdAfx.h>
#include <stdio.h>
#include <stdint.h>

#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <lwip/inet_chksum.h>
#include <netif/etharp.h>
#include <lwip/ip.h>
#include <lwip/init.h>

#include "netcfg.h"
#include "hx_inet.h"
#include "hx_eth.h"
#include "dhcp_srv.h"
#include "netglob.h"   /* Obtain DNS server. */

/* we need some routines in the DHCP of lwIP */
#undef  LWIP_DHCP
#define LWIP_DHCP   1
#include <lwip/dhcp.h>

/* buffer size for receive DHCP packet */
#define BUFSZ 1024

/* 
 * List head of DHCP allocations,each IP address that assigned
 * to a host identified by hwaddr corresponding one entry in this
 * list.
 */
static __DHCP_ALLOCATION AllocList = { 0 };

/* Handle of DHCP server thread. */
static __KERNEL_THREAD_OBJECT* pDhcpThread = NULL;

/* Check if an IP address is used. */
static BOOL IpAddrIsUsed(struct ip_addr* pAddr)
{
	__DHCP_ALLOCATION* pAlloc = AllocList.pNext;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pAddr);
	while (pAlloc != &AllocList)
	{
		if (pAlloc->ipaddr.addr == pAddr->addr)
		{
			bResult = TRUE;
			break;
		}
		pAlloc = pAlloc->pNext;
	}
	return bResult;
}

/* Allocate a new IP address from pool. */
static BOOL GetValidIpAddr(struct ip_addr* pAddr)
{
	uint8_t last = 0;
	struct ip_addr addr;

	BUG_ON(NULL == pAddr);
	for (last = DHCPD_CLIENT_IP_MIN; last < DHCPD_CLIENT_IP_MAX; last++)
	{
		/* Construct an IP address. */
		IP4_ADDR(&addr, DHCPD_SERVER_IPADDR0, DHCPD_SERVER_IPADDR1, DHCPD_SERVER_IPADDR2,
			last);
		/* Check if used. */
		if (!IpAddrIsUsed(&addr))
		{
			pAddr->addr = addr.addr;
			return TRUE;
		}
	}
	return FALSE;
}

/* Assign the specified IP addr to the host identified by MAC addr. */
static BOOL AssignIpAddrTo(struct ip_addr* pAddr, uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pAddr);
	BUG_ON(NULL == pMac);

	/* Validate if the IP addr is available. */
	if (IpAddrIsUsed(pAddr))
	{
		goto __TERMINAL;
	}
	/* Create a new allocation. */
	pAlloc = _hx_malloc(sizeof(__DHCP_ALLOCATION));
	if (NULL == pAlloc)
	{
		goto __TERMINAL;
	}
	pAlloc->ipaddr.addr = pAddr->addr;
	memcpy(pAlloc->hwaddr, pMac, sizeof(pAlloc->hwaddr));
	/* Save to list. */
	pAlloc->pNext = AllocList.pNext;
	pAlloc->pPrev = &AllocList;
	AllocList.pNext->pPrev = pAlloc;
	AllocList.pNext = pAlloc;

	/* OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pAlloc)
		{
			_hx_free(pAlloc);
		}
	}
	return bResult;
}

/* 
 * Return the assigned IP address for the give host,if already
 * assigned.This routine is mainly called in process of DISCOVER
 * message.
 */
static BOOL GetAssignedIpAddr(uint8_t* pMac, struct ip_addr* pAddr)
{
	__DHCP_ALLOCATION* pAlloc = AllocList.pNext;
	
	BUG_ON(NULL == pMac);
	BUG_ON(NULL == pAddr);
	while (pAlloc != &AllocList)
	{
		if (Eth_MAC_Match(pMac, pAlloc->hwaddr))
		{
			pAddr->addr = pAlloc->ipaddr.addr;
			return TRUE;
		}
		pAlloc = pAlloc->pNext;
	}
	return FALSE;
}

/* Release a allocated IP address. */
static BOOL ReleaseIpAddr(uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = AllocList.pNext;

	BUG_ON(NULL == pMac);
	while (pAlloc != &AllocList)
	{
		if (Eth_MAC_Match(pMac,pAlloc->hwaddr))
		{
			pAlloc->pNext->pPrev = pAlloc->pPrev;
			pAlloc->pPrev->pNext = pAlloc->pNext;
			/* Release the allocation object. */
			DEBUG_PRINTF("Release IP[%s] from host[%s].\r\n",
				inet_ntoa(pAlloc->ipaddr), ethmac_ntoa(pMac));
			_hx_free(pAlloc);
			return TRUE;
		}
		pAlloc = pAlloc->pNext;
	}
	/* Can not find the specified IP address. */
	return FALSE;
}

/* Check if a given IP address already assigned to the specified host. */
static BOOL CheckOwner(struct ip_addr* pAddr, uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = AllocList.pNext;

	BUG_ON(NULL == pAddr);
	BUG_ON(NULL == pMac);
	while (pAlloc != &AllocList)
	{
		if (Eth_MAC_Match(pMac, pAlloc->hwaddr)) /* MAC address matched. */
		{
			if (pAddr->addr == pAlloc->ipaddr.addr) /* IP address also matched. */
			{
				DEBUG_PRINTF("Check owner success.\r\n");
				return TRUE;
			}
		}
		pAlloc = pAlloc->pNext;
	}
	return FALSE;
}

/* Show all allocations in system. */
void ShowDhcpAlloc()
{
	__DHCP_ALLOCATION* pAlloc = AllocList.pNext;

	if (NULL == pDhcpThread)
	{
		_hx_printf("Please start DHCP server first.\r\n");
		return;
	}

	_hx_printf("  DHCP allocations:\r\n");
	while (pAlloc != &AllocList)
	{
		_hx_printf("  MAC:[%s],IP:[%s]\r\n",
			ethmac_ntoa(pAlloc->hwaddr),
			inet_ntoa(pAlloc->ipaddr.addr));
		pAlloc = pAlloc->pNext;
	}
}

static int _low_level_dhcp_send(struct netif *netif,
	uint8_t* pMac, /* Destination MAC address if specified. */
	const void *buffer,
	size_t size)
{
	struct pbuf *p;
	struct eth_hdr *ethhdr;
	struct ip_hdr *iphdr;
	struct udp_hdr *udphdr;

	p = pbuf_alloc(PBUF_LINK,
		SIZEOF_ETH_HDR + sizeof(struct ip_hdr)
		+ sizeof(struct udp_hdr) + size,
		PBUF_RAM);
	if (p == NULL) return -ENOMEM;

	ethhdr = (struct eth_hdr *)p->payload;
	iphdr = (struct ip_hdr *)((char *)ethhdr + SIZEOF_ETH_HDR);
	udphdr = (struct udp_hdr *)((char *)iphdr + sizeof(struct ip_hdr));

	/* 
	 * Use the specified MAC as destination if specified,use
	 * broadcast MAC address otherwise.
	 */
	if (NULL == pMac)
	{
		ETHADDR32_COPY(&ethhdr->dest, (struct eth_addr *)&ethbroadcast);
	}
	else
	{
		memcpy(ethhdr->dest.addr, pMac, ETH_MAC_LEN);
	}
	ETHADDR16_COPY(&ethhdr->src, netif->hwaddr);
	ethhdr->type = PP_HTONS(ETHTYPE_IP);

	/* Use the out interface's addr as source. */
	iphdr->src.addr = netif->ip_addr.addr;
	iphdr->dest.addr = 0xFFFFFFFF; /* dst: 255.255.255.255 */

	IPH_VHLTOS_SET(iphdr, 4, IP_HLEN / 4, 0);
	//IPH_TOS_SET(iphdr, 0x00);
	IPH_LEN_SET(iphdr, htons(IP_HLEN + sizeof(struct udp_hdr) + size));
	IPH_ID_SET(iphdr, htons(2));
	IPH_OFFSET_SET(iphdr, 0);
	IPH_TTL_SET(iphdr, 255);
	IPH_PROTO_SET(iphdr, IP_PROTO_UDP);
	IPH_CHKSUM_SET(iphdr, 0);
	IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, IP_HLEN));

	udphdr->src = htons(DHCP_SERVER_PORT);
	udphdr->dest = htons(DHCP_CLIENT_PORT);
	udphdr->len = htons(sizeof(struct udp_hdr) + size);
	udphdr->chksum = 0;

	memcpy((char *)udphdr + sizeof(struct udp_hdr),
		buffer, size);

	netif->linkoutput(netif, p);
	pbuf_free(p);

	return 0;
}

/* Show out different message type. */
static void ShowDhcpMessage(uint8_t msg_type)
{
	switch (msg_type)
	{
	case 1:
	case 2:
	case 3:
		__LOG("DHCP message type[%d] should be processed but not.\r\n", msg_type);
		break;
	case 4:
		__LOG("DHCP Decline message.\r\n");
		break;
	case 5:
	default:
		__LOG("Unknow DHCP message type[%d].\r\n", msg_type);
		break;
	}
}

static DWORD dhcpd_thread_entry(void *parameter)
{
	struct netif *netif = NULL;
	int sock;
	int bytes_read;
	char *recv_data;
	uint32_t addr_len;
	struct sockaddr_in server_addr, client_addr;
	struct dhcp_msg *msg;
	int optval = 1;
	struct ip_addr addr;

	/* get ethernet interface. */
	netif = (struct netif*) parameter;
	BUG_ON(netif == NULL);

	/* Init IP address allocation list. */
	AllocList.pNext = AllocList.pPrev = &AllocList;

	/* our DHCP server information */
	_hx_printf("\r\n");
	DEBUG_PRINTF("DHCP server IP: %d.%d.%d.%d,IP pool: %d.%d.%d.%d-%d\r\n",
		DHCPD_SERVER_IPADDR0, DHCPD_SERVER_IPADDR1,
		DHCPD_SERVER_IPADDR2, DHCPD_SERVER_IPADDR3,
		DHCPD_SERVER_IPADDR0, DHCPD_SERVER_IPADDR1,
		DHCPD_SERVER_IPADDR2, DHCPD_CLIENT_IP_MIN, DHCPD_CLIENT_IP_MAX);

	/* allocate buffer for receive */
	recv_data = _hx_malloc(BUFSZ);
	if (recv_data == NULL)
	{
		/* No memory */
		DEBUG_PRINTF("Out of memory\r\n");
		return 0;
	}

	/* create a socket with UDP */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DEBUG_PRINTF("Create socket failed, errno = %d\r\n", errno);
		_hx_free(recv_data);
		return 0;
	}

	/* set to receive broadcast packet */
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));

	/* initialize server address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DHCP_SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/* bind socket to the server address */
	if (bind(sock, (struct sockaddr *)&server_addr,
		sizeof(struct sockaddr)) == -1)
	{
		/* bind failed. */
		DEBUG_PRINTF("Bind server address failed, errno = %d\r\n", errno);
		_hx_free(recv_data);
		return 0;
	}

	addr_len = sizeof(struct sockaddr);
	DEBUG_PRINTF("Start DHCP server on port %d.\r\n", DHCP_SERVER_PORT);

	while (1)
	{
		bytes_read = recvfrom(sock, recv_data, BUFSZ - 1, 0,
			(struct sockaddr *)&client_addr, &addr_len);
		if (bytes_read < DHCP_MSG_LEN)
		{
			DEBUG_PRINTF("packet too short, wait for next!\r\n");
			continue;
		}

		msg = (struct dhcp_msg *)recv_data;
		/* check message type to make sure we can handle it */
		if ((msg->op != DHCP_BOOTREQUEST) || (msg->cookie != PP_HTONL(DHCP_MAGIC_COOKIE)))
		{
			continue;
		}

		/* handler. */
		{
			uint8_t *dhcp_opt;
			uint8_t option;
			uint8_t length;
			uint8_t* pMac = NULL;

			uint8_t message_type = 0;
			uint8_t finished = 0;
			uint32_t request_ip = 0;

			dhcp_opt = (uint8_t *)msg + DHCP_OPTIONS_OFS;
			while (finished == 0)
			{
				option = *dhcp_opt;
				length = *(dhcp_opt + 1);

				switch (option)
				{
				case DHCP_OPTION_REQUESTED_IP:
					request_ip = *(dhcp_opt + 2) << 24 | *(dhcp_opt + 3) << 16
						| *(dhcp_opt + 4) << 8 | *(dhcp_opt + 5);
					break;

				case DHCP_OPTION_END:
					finished = 1;
					break;

				case DHCP_OPTION_MESSAGE_TYPE:
					message_type = *(dhcp_opt + 2);
					break;

				default:
					break;
				} /* switch(option) */

				dhcp_opt += (2 + length);
			}

			/* reply. */
			dhcp_opt = (uint8_t *)msg + DHCP_OPTIONS_OFS;

			/* check. */
			if (request_ip)
			{
				/* Check if the requested IP address already owned by the requester. */
				pMac = &msg->chaddr[0];
				addr.addr = htonl(request_ip);
				if (!CheckOwner(&addr,pMac))
				{
					*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
					*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
					*dhcp_opt++ = DHCP_NAK;
					*dhcp_opt++ = DHCP_OPTION_END;

					addr.addr = ntohl(request_ip);
					DEBUG_PRINTF("requested IP[%s] invalid, reply DHCP_NAK\r\n",inet_ntoa(addr));
					if (netif != NULL)
					{
						int send_byte = (dhcp_opt - (uint8_t *)msg);
						_low_level_dhcp_send(netif, pMac, msg, send_byte);
						DEBUG_PRINTF("DHCP server send %d bytes.\r\n", send_byte);
					}
					continue;
				}
			}

			if (message_type == DHCP_DISCOVER)
			{
				DEBUG_PRINTF("request message = DHCP_DISCOVER.\r\n");

				// DHCP_OPTION_MESSAGE_TYPE
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
				*dhcp_opt++ = DHCP_OFFER;

				// DHCP_OPTION_SERVER_ID
				*dhcp_opt++ = DHCP_OPTION_SERVER_ID;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR0;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR1;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR2;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR3;

				// DHCP_OPTION_LEASE_TIME
				*dhcp_opt++ = DHCP_OPTION_LEASE_TIME;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = 0x00;
				*dhcp_opt++ = 0x01;
				*dhcp_opt++ = 0x51;
				*dhcp_opt++ = 0x80;
			}
			else if (message_type == DHCP_REQUEST)
			{
				DEBUG_PRINTF("request message = DHCP_REQUEST.\r\n");

				// DHCP_OPTION_MESSAGE_TYPE
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
				*dhcp_opt++ = DHCP_ACK;

				// DHCP_OPTION_SERVER_ID
				*dhcp_opt++ = DHCP_OPTION_SERVER_ID;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR0;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR1;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR2;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR3;

				// DHCP_OPTION_SUBNET_MASK
				*dhcp_opt++ = DHCP_OPTION_SUBNET_MASK;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = 0xFF;
				*dhcp_opt++ = 0xFF;
				*dhcp_opt++ = 0xFF;
				*dhcp_opt++ = 0x00;

				// DHCP_OPTION_ROUTER
				*dhcp_opt++ = DHCP_OPTION_ROUTER;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR0;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR1;
				*dhcp_opt++ = DHCPD_SERVER_IPADDR2;
				*dhcp_opt++ = 1;

				if ((0 == NetworkGlobal.dns_primary.addr) &&
					(0 == NetworkGlobal.dns_secondary.addr))
				{
					// DHCP_OPTION_DNS_SERVER, use the default DNS server address if no 
					// DNS server address is obtained.
					*dhcp_opt++ = DHCP_OPTION_DNS_SERVER;
					*dhcp_opt++ = 4;
					*dhcp_opt++ = 208;
					*dhcp_opt++ = 67;
					*dhcp_opt++ = 222;
					*dhcp_opt++ = 222;
				}
				else
				{
					/* Use the obtained DNS server. */
					*dhcp_opt++ = DHCP_OPTION_DNS_SERVER;
					uint8_t* pOptLen = dhcp_opt++;
					*pOptLen = 0;
					if (NetworkGlobal.dns_primary.addr)
					{
						*pOptLen += 4;
						memcpy(dhcp_opt, &NetworkGlobal.dns_primary.addr, 4);
						dhcp_opt += 4;
					}
					if (NetworkGlobal.dns_secondary.addr)
					{
						*pOptLen += 4;
						memcpy(dhcp_opt, &NetworkGlobal.dns_secondary.addr, 4);
						dhcp_opt += 4;
					}
				}

				// DHCP_OPTION_LEASE_TIME
				*dhcp_opt++ = DHCP_OPTION_LEASE_TIME;
				*dhcp_opt++ = 4;
				*dhcp_opt++ = 0x00;
				*dhcp_opt++ = 0x01;
				*dhcp_opt++ = 0x51;
				*dhcp_opt++ = 0x80;

				// DHCP_OPTION_DOMAIN_NAME
				int dn_len = strlen(DEFAULT_DOMAIN_NAME);
				*dhcp_opt++ = DHCP_OPTION_DOMAIN_NAME;
				*dhcp_opt++ = dn_len;
				strcpy(dhcp_opt++, DEFAULT_DOMAIN_NAME);
				dhcp_opt += dn_len;
			}
			else if (message_type == DHCP_RELEASE)
			{
				ReleaseIpAddr(&msg->chaddr[0]);
				/* Reply ACK msg. */
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
				*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
				*dhcp_opt++ = DHCP_ACK;
				*dhcp_opt++ = DHCP_OPTION_END;
			}
			else
			{
				/* Unknown DHCP message,just show out. */
				ShowDhcpMessage(message_type);
			}

			/* send reply. */
			if ((message_type == DHCP_DISCOVER) || (message_type == DHCP_REQUEST))
			{
				msg->op = DHCP_BOOTREPLY;
				if (message_type == DHCP_DISCOVER)
				{
					if (!GetAssignedIpAddr(&msg->chaddr[0], &addr))
					{
						/* 
						 * The first time of DISCOVER message of this host, 
						 * just allocate a new IP address to it.
						 */
						if (!GetValidIpAddr(&addr))
						{
							_hx_printf("DHCP:no available IP address resource.\r\n");
							continue; /* Maybe next message is release... */
						}
						AssignIpAddrTo(&addr, &msg->chaddr[0]);
						msg->yiaddr.addr = addr.addr;
						DEBUG_PRINTF("Assign IP addr[%s] to host[%s].\r\n",
							inet_ntoa(addr), ethmac_ntoa(&msg->chaddr[0]));
					}
					else /* Already assigned IP address to this host. */
					{
						msg->yiaddr.addr = addr.addr;
						DEBUG_PRINTF("IP addr[%s] already assigned to[%s].\r\n",
							inet_ntoa(addr), ethmac_ntoa(&msg->chaddr[0]));
					}
				}
				else //message is DHCP_REQUEST. 
				{
					//msg->yiaddr.addr = htonl(request_ip);
					if (!GetAssignedIpAddr(&msg->chaddr[0], &addr))
					{
						_hx_printf("No IP assigned to host[%s] but request received.\r\n",
							ethmac_ntoa(&msg->chaddr[0]));
						*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
						*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
						*dhcp_opt++ = DHCP_NAK;
					}
					else
					{
						msg->yiaddr.addr = addr.addr;
						/* Just update the lease time. */
						DEBUG_PRINTF("Ack req[%s] to host[%s].\r\n",
							inet_ntoa(addr), ethmac_ntoa(&msg->chaddr[0]));
					}
				}
				/* Append DHCP option END flag. */
				*dhcp_opt++ = DHCP_OPTION_END;

				client_addr.sin_addr.s_addr = INADDR_BROADCAST;

				if (netif != NULL)
				{
					int send_byte = (dhcp_opt - (uint8_t *)msg);
					_low_level_dhcp_send(netif, NULL, msg, send_byte);
					DEBUG_PRINTF("DHCP server send %d bytes.\r\n", send_byte);
				}
			}
		} /* handler. */
	}
	return 0;
}

/* Start routine of DHCP server. */
static void dhcp_server_start(char* netif_name)
{
	struct netif *netif = netif_list;
	ip_addr_t addr, gw, mask;

	if (NULL == netif_name)
	{
		_hx_printf("Please specify an interface name.\r\n");
		goto __TERMINAL;
	}

	if (strlen(netif_name) > sizeof(netif->name))
	{
		_hx_printf("Network interface name too long!\r\n");
		goto __TERMINAL;
	}

	/* Find the netif object by it's name. */
	while (netif != NULL)
	{
		if (strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
		{
			break;
		}

		netif = netif->next;
		if (netif == NULL)
		{
			_hx_printf("network interface: %s not found!\r\n", netif_name);
			goto __TERMINAL;;
		}
	}

	/* Check if the DHCP server already enabled on this if. */
	if (netif->flags & NETIF_FLAG_DHCPSERVER)
	{
		_hx_printf("DHCP server already enabled on this interface.\r\n");
		goto __TERMINAL;
	}

	/* Config IP addr on the specified interface,as gateway of client. */
	addr.addr = DHCPD_SERVER_IPADDR0;
	addr.addr <<= 8;
	addr.addr += DHCPD_SERVER_IPADDR1;
	addr.addr <<= 8;
	addr.addr += DHCPD_SERVER_IPADDR2;
	addr.addr <<= 8;
	addr.addr += 1;
	addr.addr = _hx_htonl(addr.addr);
	mask.addr = 255;
	mask.addr <<= 8;
	mask.addr += 255;
	mask.addr <<= 8;
	mask.addr += 255;
	gw.addr = addr.addr; /* Use interface IP addr as gw. */
	netif_set_down(netif);
	/* Disable dhcp function on this if. */
	dhcp_stop(netif);
	/* Configure IP addr and gw. */
	netif_set_addr(netif, &addr, &mask, &gw);
	netif_set_up(netif);
	/* Set the DHCP server flag of this interface. */
	netif->flags |= NETIF_FLAG_DHCPSERVER;

	/* Start the DHCP server thread now,if it is not started yet. */
	if (NULL == pDhcpThread)
	{
		unsigned int nAffinity = 0;
		pDhcpThread = KernelThreadManager.CreateKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0,
			KERNEL_THREAD_STATUS_SUSPENDED,
			PRIORITY_LEVEL_NORMAL,
			dhcpd_thread_entry,
			netif,
			NULL,
			DHCP_SERVER_NAME);
		if (NULL == pDhcpThread)
		{
			_hx_printk("Failed to start DHCP task.\r\n");
			goto __TERMINAL;
		}
#if defined(__CFG_SYS_SMP)
		/* Get a CPU to schedule DHCP server thread to. */
		nAffinity = ProcessorManager.GetScheduleCPU();
#endif
		KernelThreadManager.ChangeAffinity((__COMMON_OBJECT*)pDhcpThread, nAffinity);
		/* Resume the kernel thread to ready to run. */
		KernelThreadManager.ResumeKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)pDhcpThread);
	}

__TERMINAL:
	return;
}

/* 
 * Entry point of DHCP Server.Do some system level initializations
 * in this routine.
 */
BOOL DHCPSrv_Start()
{
	return TRUE;
}

/* Start DHCP server on a given interface. */
BOOL DHCPSrv_Start_Onif(char* ifName)
{
	dhcp_server_start(ifName);
	return TRUE;
}
