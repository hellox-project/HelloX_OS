//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 02,2017
//    Module Name               : dhcp_srv.c
//    Module Funciton           : 
//                                A simple dhcp server's implementation from
//                                lwIP,ported to HelloX by Garry.Xin,and revised 
//                                according HelloX's network framework.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

/*
 * Brief of the implementation:
 * 1. dhcp service could be enabled on multiple interfaces on routers
 *    running HelloX;
 * 2. A dedicated IP range(or pool) is assigned to each instance running
 *    on router's interface,the default IP range is 192.168.X.2 ~ 192.168.X.254,
 *    where the X denotes 169 minus interface's index value;
 * 3. A data structure named __DHCP_INTERFACE is allocated for each dhcp
 *    enabled interface,all of these objects are linked together named if_list_head;
 * 4. Interface index, IP range, allocation list, receiving buffer, and other
 *    variables bound to one dhcp interface are storaged in dhcp interface object;
 * 5. One mutex object is used to protect dhcp interface list,since it's accessed
 *    by multiple threads;
 * 6. Each dhcp interface has it's udp socket,listening on the interface with local
 *    IP address and viable to receive dhcp discover/request;
 * 7. One dedicated thread named 'dhcpd' is created and used to monitor all sockets,
 *    select() routine is used to make it available for muitiple sockets at the
 *    same time;
 * 8. dhcpd thread would be waken up when a dhcp message is received,the handler
 *    will be invoked in this thread;
 * 9. All IP address asigned to client are recorded in allocation list of each
 *    dhcp interface object;
 */

#include <stdio.h>
#include <stdint.h>
#include <KAPI.H>
#include <lwip/sockets.h>

#include "hx_inet.h"
#include "hx_eth.h"
#include "proto.h"
#include "dhcp_srv.h"
#include "netglob.h"   /* Obtain DNS server. */
#include "precfg.h"

/* Handle of dhcp server main thread. */
static __KERNEL_THREAD_OBJECT* pDhcpThread = NULL;
/* DHCP server enabled interface list. */
static __DHCP_INTERFACE* if_list_head = NULL;
/* Mutex to protect the if list. */
static HANDLE if_mtx = NULL;
/* Index of dhcp server enabled interface. */
static uint8_t if_index = 0;

/*
 * Add one interface to DHCP server enabled list.
 * Server will listen on this interface after added.
 */
static BOOL AddOneInterface(char* name);

/* Check if an IP address is used. */
static BOOL IpAddrIsUsed(__DHCP_INTERFACE* pInt, struct ip_addr* pAddr)
{
	__DHCP_ALLOCATION* pAlloc = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pAddr);

	pAlloc = pInt->alloc_head.pNext;
	while (pAlloc != &pInt->alloc_head)
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

/* 
 * Allocate a new IP address from the local pool 
 * of a given dhcp enabled interface. 
 */
static BOOL GetValidIpAddr(__DHCP_INTERFACE* pInt, struct ip_addr* pAddr)
{
	uint8_t last = 0;
	struct ip_addr addr;

	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pAddr);

	for (last = DHCPD_CLIENT_IP_MIN; last < DHCPD_CLIENT_IP_MAX; last++)
	{
		/* Construct an IP address. */
		IP4_ADDR(&addr, DHCPD_SERVER_IPADDR0, DHCPD_SERVER_IPADDR1, pInt->net_part3,
			last);
		/* Check if used. */
		if (!IpAddrIsUsed(pInt, &addr))
		{
			pAddr->addr = addr.addr;
			return TRUE;
		}
	}
	return FALSE;
}

/* Assign the specified IP addr to the host identified by MAC addr. */
static BOOL AssignIpAddrTo(__DHCP_INTERFACE* pInt, 
	struct ip_addr* pAddr, uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pAddr);
	BUG_ON(NULL == pMac);

	/* Validate if the IP addr is available. */
	if (IpAddrIsUsed(pInt, pAddr))
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
	/* Save to dhcp interface's allocation list. */
	pAlloc->pNext = pInt->alloc_head.pNext;
	pAlloc->pPrev = &pInt->alloc_head;
	pInt->alloc_head.pNext->pPrev = pAlloc;
	pInt->alloc_head.pNext = pAlloc;

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
static BOOL GetAssignedIpAddr(
	__DHCP_INTERFACE* pInt,
	uint8_t* pMac, struct ip_addr* pAddr)
{
	__DHCP_ALLOCATION* pAlloc = NULL;
	
	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pMac);
	BUG_ON(NULL == pAddr);

	pAlloc = pInt->alloc_head.pNext;
	while (pAlloc != &pInt->alloc_head)
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
static BOOL ReleaseIpAddr(__DHCP_INTERFACE* pInt, uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = NULL;

	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pMac);

	pAlloc = pInt->alloc_head.pNext;
	while (pAlloc != &pInt->alloc_head)
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
static BOOL CheckOwner(__DHCP_INTERFACE* pInt, 
	struct ip_addr* pAddr, uint8_t* pMac)
{
	__DHCP_ALLOCATION* pAlloc = NULL;

	BUG_ON(NULL == pInt);
	BUG_ON(NULL == pAddr);
	BUG_ON(NULL == pMac);

	pAlloc = pInt->alloc_head.pNext;
	while (pAlloc != &pInt->alloc_head)
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
	__DHCP_ALLOCATION* pAlloc = NULL;
	__DHCP_INTERFACE* pDhcpInt = NULL;

	if (NULL == pDhcpThread)
	{
		_hx_printf("service not started.\r\n");
		return;
	}

	_hx_printf("  DHCP allocations:\r\n");
	pDhcpInt = if_list_head;
	WaitForThisObject(if_mtx);
	while (pDhcpInt)
	{
		pAlloc = pDhcpInt->alloc_head.pNext;
		while (pAlloc != &pDhcpInt->alloc_head)
		{
			_hx_printf("  genif[%d]: MAC[%s] -- IP[%s]\r\n",
				pDhcpInt->genif_index,
				ethmac_ntoa(pAlloc->hwaddr),
				inet_ntoa(pAlloc->ipaddr.addr));
			pAlloc = pAlloc->pNext;
		}
		pDhcpInt = pDhcpInt->pNext;
	}
	ReleaseMutex(if_mtx);
}

/* Send dhcp response to client. */
static int _low_level_dhcp_send(
	__DHCP_INTERFACE* pInt,
	/* Destination addr of client if specified. */
	struct sockaddr_in* pclient_addr,
	const void *buffer,
	size_t size)
{
	struct sockaddr_in addr_to;
	int ret = -1;

	/* Parameters checking. */
	BUG_ON((NULL == pInt) || (NULL == buffer) || (0 == size));

	/* Send the reply through socket. */
	addr_to.sin_family = AF_INET;
	addr_to.sin_addr.s_addr = inet_addr("255.255.255.255"); /* broadcast. */
	addr_to.sin_len = sizeof(struct sockaddr_in);
	addr_to.sin_port = htons(DHCP_PORT_CLIENT);
	memset(&addr_to.sin_zero[0], 0, sizeof(addr_to.sin_zero));
	ret = lwip_sendto(pInt->sock,
		buffer,
		size,
		0,
		(const struct sockaddr*)&addr_to,
		sizeof(addr_to));
	if (-1 == ret)
	{
		_hx_printf("sendto failed with err[%d]\r\n", ret);
	}
	return ret;
}

/* Show out different message type. */
static void ShowDhcpMessage(uint8_t msg_type)
{
	switch (msg_type)
	{
	case 1:
	case 2:
	case 3:
		__LOG("Message type[%d] should be processed but not.\r\n", msg_type);
		break;
	case 4:
		__LOG("Decline message.\r\n");
		break;
	case 5:
	default:
		__LOG("Unknow message[%d].\r\n", msg_type);
		break;
	}
}

/* Processes DHCP messages. */
static int dhcp_msg_handler(
	/* Interface where the DHCP msg received. */
	__DHCP_INTERFACE* pInt,
	struct dhcp_msg *msg,
	struct sockaddr_in client_addr)
{
	uint8_t *dhcp_opt;
	uint8_t option;
	uint8_t length;
	uint8_t* pMac = NULL;
	uint8_t message_type = 0;
	uint8_t finished = 0;
	uint32_t request_ip = 0;
	struct ip_addr addr;
	__SYSTEM_NETWORK_INFO NetInfo;
	int result = 0;

	/* Validate parameters. */
	BUG_ON((NULL == pInt) || (NULL == msg));

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

	if ((request_ip) && (message_type != DHCP_DISCOVER))
	{
		/* 
		 * Check if the requested IP address already 
		 * owned by the requester. 
		 * Microsft client's discover message may
		 * with an IP address that it last used,we
		 * skip this scenario.
		 */
		pMac = &msg->chaddr[0];
		addr.addr = htonl(request_ip);
		if (!CheckOwner(pInt, &addr, pMac))
		{
			*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
			*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
			*dhcp_opt++ = DHCP_NAK;
			*dhcp_opt++ = DHCP_OPTION_END;

			addr.addr = ntohl(request_ip);
			DEBUG_PRINTF("requested IP[%s] invalid, reply DHCP_NAK\r\n", inet_ntoa(addr));
			if (pInt != NULL)
			{
				int send_byte = (dhcp_opt - (uint8_t *)msg);
				_low_level_dhcp_send(pInt,
					NULL,
					msg, send_byte);
				DEBUG_PRINTF("[%d] bytes sent.\r\n", send_byte);
			}
			result = 0;
			goto __TERMINAL;
		}
	}

	/* Build reply packet according message type. */
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

		msg->op = DHCP_BOOTREPLY;
		if (!GetAssignedIpAddr(pInt, &msg->chaddr[0], &addr))
		{
			/*
			 * The first time of DISCOVER message of this host,
			 * just allocate a new IP address to it.
			 */
			if (!GetValidIpAddr(pInt, &addr))
			{
				_hx_printf("DHCP:no IP on if[%d].\r\n", pInt->if_index);
				result = 0;
				goto __TERMINAL;
			}
			AssignIpAddrTo(pInt, &addr, &msg->chaddr[0]);
			msg->yiaddr.addr = addr.addr;
			DEBUG_PRINTF("Assign IP addr[%s] to host[%s].\r\n",
				inet_ntoa(addr), ethmac_ntoa(&msg->chaddr[0]));
		}
		else 
		{
			/* 
			 * IP address already assigned to this host. 
			 * Windows implementations of dhcp client may send several 
			 * discover at first,so we may receive duplicated discover,
			 * just reply the assignd IP address to it.
			 */
			msg->yiaddr.addr = addr.addr;
		}

		/* Append DHCP option END flag. */
		*dhcp_opt++ = DHCP_OPTION_END;
		client_addr.sin_addr.s_addr = INADDR_BROADCAST;

		/* Send reply to client. */
		if (pInt != NULL)
		{
			int send_byte = (dhcp_opt - (uint8_t *)msg);
			_low_level_dhcp_send(pInt, 
				NULL, 
				msg, send_byte);
			DEBUG_PRINTF("[%d] bytes sent.\r\n", send_byte);
		}
	}
	else if (message_type == DHCP_REQUEST)
	{
		DEBUG_PRINTF("request message = DHCP_REQUEST.\r\n");
		msg->op = DHCP_BOOTREPLY;

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

		/*
		 * DHCP_OPTION_ROUTER.
		 * Use the interface's IP address as client's gw.
		 */
		*dhcp_opt++ = DHCP_OPTION_ROUTER;
		*dhcp_opt++ = 4;
		*dhcp_opt++ = pInt->net_part1; // DHCPD_SERVER_IPADDR0;
		*dhcp_opt++ = pInt->net_part2; // DHCPD_SERVER_IPADDR1;
		*dhcp_opt++ = pInt->net_part3;
		*dhcp_opt++ = 1;

		/* Get dns servers from system. */
		memset(&NetInfo, 0, sizeof(__SYSTEM_NETWORK_INFO));
		NetInfo.version = 1;
		NetworkGlobal.GetNetworkInfo(&NetInfo);

		if ((0 == NetInfo.v4dns_p) &&
			(0 == NetInfo.v4dns_s))
		{
			/*
			 * DHCP_OPTION_DNS_SERVER.
			 * Use the default DNS server address if no 
			 *  DNS server address is obtained.
			 */
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
			if (NetInfo.v4dns_p)
			{
				*pOptLen += 4;
				memcpy(dhcp_opt, &NetInfo.v4dns_p, 4);
				dhcp_opt += 4;
			}
			if (NetInfo.v4dns_s)
			{
				*pOptLen += 4;
				memcpy(dhcp_opt, &NetInfo.v4dns_s, 4);
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

		if (!GetAssignedIpAddr(pInt, &msg->chaddr[0], &addr))
		{
			/* Client may response to other dhcp server. */
			_hx_printf("No IP assigned to host[%s] but request received.\r\n",
				ethmac_ntoa(&msg->chaddr[0]));
			*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE;
			*dhcp_opt++ = DHCP_OPTION_MESSAGE_TYPE_LEN;
			*dhcp_opt++ = DHCP_NAK;
		}
		else
		{
			/* 
			 * Should check if the requested IP address is the 
			 * same one allocated to this host in discover phase.
			 * The request may send to other dhcp server by client
			 * and the IP address in request is not the same one
			 * yet allocated,so should release it in this case.
			 */
			msg->yiaddr.addr = addr.addr;
			/* Just update the lease time. */
			DEBUG_PRINTF("Ack req[%s] to host[%s].\r\n",
				inet_ntoa(addr), ethmac_ntoa(&msg->chaddr[0]));
		}

		/* Append DHCP option END flag. */
		*dhcp_opt++ = DHCP_OPTION_END;
		client_addr.sin_addr.s_addr = INADDR_BROADCAST;

		/* Send reply to client. */
		if (pInt != NULL)
		{
			int send_byte = (dhcp_opt - (uint8_t *)msg);
			_low_level_dhcp_send(pInt, 
				NULL, 
				msg, send_byte);
			DEBUG_PRINTF("[%d] bytes sent.\r\n", send_byte);
		}
	}
	else if (message_type == DHCP_RELEASE)
	{
		ReleaseIpAddr(pInt, &msg->chaddr[0]);
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

	result = 1;

__TERMINAL:
	return result;
}

/* 
 * Load at most 6 genif names from system 
 * configuration file. 
 * The return value is the number of loaded,
 * all these interfaces will be applied dhcp
 * server functions.
 */
static int __GetPreconfigedGenif(char* genif_name, int genif_num)
{
	HANDLE hCfgProfile = NULL;
	char value_buff[SYSTEM_CONFIG_MAX_KVLENGTH];
	int if_num = 0;

	if (genif_num > 8)
	{
		return 0;
	}

	hCfgProfile = SystemConfigManager.GetConfigProfile("dhcpd");
	if (NULL == hCfgProfile)
	{
		_hx_printf("[%s]could not get config profile\r\n", __func__);
	}

	/* Retrieve key-value and show it. */
	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_1", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}
	else {
		goto __TERMINAL;
	}

	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_2", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}
	else {
		goto __TERMINAL;
	}

	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_3", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}
	else {
		goto __TERMINAL;
	}

	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_4", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}
	else {
		goto __TERMINAL;
	}

	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_5", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}
	else {
		goto __TERMINAL;
	}

	if (SystemConfigManager.GetConfigEntry(hCfgProfile, "default_if_6", value_buff, GENIF_NAME_LENGTH))
	{
		strncpy(genif_name + if_num * GENIF_NAME_LENGTH, value_buff, GENIF_NAME_LENGTH);
		if_num++;
	}

__TERMINAL:
	SystemConfigManager.ReleaseConfigProfile(hCfgProfile);
	return if_num;
}

/* 
 * Add pre-configured interface(s) to 
 * dhcp server enabled list.
 * This routine reads system configuration file,
 * get the interface list that should enable
 * dhcp server service on,then add it into
 * global list.
 */
static BOOL add_preconfig_interface()
{
#if (!NET_PRECONFIG_ENABLE)
	char genif_name[6][GENIF_NAME_LENGTH];
	int if_num = 0;

	if_num = __GetPreconfigedGenif(&genif_name[0][0], 6);
	for (int i = 0; i < if_num; i++)
	{
		if (AddOneInterface(genif_name[i]))
		{
			__LOG("[%s]enable dhcp server on genif[%s].\r\n",
				__func__, genif_name[i]);
		}
	}
	return TRUE;
#else

#if defined(DHCPD_ENABLED_GENIF_1)
	if (AddOneInterface(DHCPD_ENABLED_GENIF_1))
	{
		__LOG("[%s]enable dhcp server on genif[%s].\r\n",
			__func__, DHCPD_ENABLED_GENIF_1);
	}
#endif
#if defined(DHCPD_ENABLED_GENIF_2)
	if (AddOneInterface(DHCPD_ENABLED_GENIF_2))
	{
		__LOG("[%s]enable dhcp server on genif[%s].\r\n",
			__func__, DHCPD_ENABLED_GENIF_2);
	}
#endif
#if defined(DHCPD_ENABLED_GENIF_3)
	if (AddOneInterface(DHCPD_ENABLED_GENIF_3))
	{
		__LOG("[%s]enable dhcp server on genif[%s].\r\n",
			__func__, DHCPD_ENABLED_GENIF_3);
	}
#endif
#if defined(DHCPD_ENABLED_GENIF_4)
	if (AddOneInterface(DHCPD_ENABLED_GENIF_4))
	{
		__LOG("[%s]enable dhcp server on genif[%s].\r\n",
			__func__, DHCPD_ENABLED_GENIF_4);
	}
#endif
	return TRUE;
#endif
}

/* Main thread of DHCP server. */
static DWORD dhcpd_thread_entry(void *parameter)
{
	__DHCP_INTERFACE* pInt = NULL; //(__DHCP_INTERFACE*)parameter;
	int sock, max_sock;
	int bytes_read;
	char *recv_data;
	uint32_t addr_len;
	struct sockaddr_in client_addr;
	struct dhcp_msg *msg;
	struct timeval tv;
	fd_set readset;
	int sel_ret = 0;

	/* 
	 * Add pre-configured dhcp server enabled 
	 * interface(s) to if list. 
	 */
	add_preconfig_interface();

	/* No genif in dhcp server enabled list. */
	if (NULL == if_list_head)
	{
		__LOG("[%s]no genif in list.\r\n", __func__);
		return 0;
	}

	addr_len = sizeof(struct sockaddr);
	while (TRUE)
	{
		tv.tv_sec = DHCP_WAIT_TIMEOUT;
		tv.tv_usec = 0;

		/* Initializes readset. */
		FD_ZERO(&readset);
		/* Add all sockets into readset. */
		pInt = if_list_head;
		max_sock = 0;
		WaitForThisObject(if_mtx);
		while (pInt)
		{
			FD_SET(pInt->sock, &readset);
			if (pInt->sock > max_sock)
			{
				max_sock = pInt->sock;
			}
			pInt = pInt->pNext;
		}
		ReleaseMutex(if_mtx);

		/* Wait for incoming messages. */
		while (TRUE)
		{
			sel_ret = lwip_select(max_sock + 1, &readset, NULL, NULL, &tv);
			if (sel_ret <= 0)
			{
				/* Reinit readset and try again. */
				FD_ZERO(&readset);
				/* Add all sockets into readset. */
				pInt = if_list_head;
				max_sock = 0;
				WaitForThisObject(if_mtx);
				while (pInt)
				{
					FD_SET(pInt->sock, &readset);
					if (pInt->sock > max_sock)
					{
						max_sock = pInt->sock;
					}
					pInt = pInt->pNext;
				}
				ReleaseMutex(if_mtx);
				continue;
			}
			else /* At least 1 readable socket is selected. */
			{
				break;
			}
		}

		/* Selected a readable socket. */
		pInt = if_list_head;
		WaitForThisObject(if_mtx);
		while(pInt)
		{
			/* Handler for processing the client request. */
			sock = pInt->sock;
			recv_data = pInt->recv_buff;

			/* If the sock has incoming data. */
			if (FD_ISSET(sock, &readset))
			{
				FD_CLR(sock, &readset);
				bytes_read = lwip_recvfrom(sock, recv_data, DHCP_RECV_BUFSZ - 1, 0,
					(struct sockaddr *)&client_addr, &addr_len);
				if (bytes_read < DHCP_MSG_LEN)
				{
					DEBUG_PRINTF("packet too short[len = %d], wait for next!\r\n",
						bytes_read);
					continue;
				}

				msg = (struct dhcp_msg *)recv_data;
				/* check message type to make sure we can handle it */
				if ((msg->op != DHCP_BOOTREQUEST) || (msg->cookie != PP_HTONL(DHCP_MAGIC_COOKIE)))
				{
					continue;
				}

				/* Process the received DHCP message. */
				dhcp_msg_handler(pInt, msg, client_addr);
			}
			pInt = pInt->pNext;
		}
		ReleaseMutex(if_mtx);
	}
	return 0;
}

/* 
 * A local helper routine to get a genif's index 
 * by giving it's name. This routine obtains all
 * genif objects in system by invoking GetGenifInfo
 * routine,and returns one that matching the name.
 * NULL will be returned if no genif or no genif's
 * name matches the given name.
 */
static unsigned long __GetGenifByName(char* genif_name)
{
	unsigned long genif_index = -1;
	unsigned long buff_req = sizeof(__GENERIC_NETIF);
	__GENERIC_NETIF* pGenifSnap = NULL;
	int ret_val = -1;

	if (NULL == genif_name)
	{
		goto __TERMINAL;
	}

	/* Allocate genif snapshot memory pool. */
	pGenifSnap = (__GENERIC_NETIF*)_hx_malloc(buff_req);
	if (NULL == pGenifSnap)
	{
		__LOG("[%s]:out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	/* Try to capture all genifs in system. */
	while (TRUE)
	{
		ret_val = NetworkManager.GetGenifInfo(pGenifSnap, &buff_req);
		if (-1 == ret_val)
		{
			/* No genif in system at all. */
			ret_val = 0;
			goto __TERMINAL;
		}
		if (0 == ret_val)
		{
			/* Memory is not enough,realloc and try again. */
			_hx_free(pGenifSnap);
			pGenifSnap = (__GENERIC_NETIF*)_hx_malloc(buff_req);
			if (NULL == pGenifSnap)
			{
				__LOG("[%s]out of memory.\r\n", __func__);
				goto __TERMINAL;
			}
			continue;
		}
		/* Get genif info success. */
		break;
	}

	/* Locate the genif by it's name. */
	for (int i = 0; i < ret_val; i++)
	{
		if (0 == strncmp(genif_name, pGenifSnap[i].genif_name, 
			sizeof(pGenifSnap[i].genif_name)))
		{
			genif_index = pGenifSnap[i].if_index;
			break;
		}
	}

__TERMINAL:
	if (pGenifSnap)
	{
		/* Release the snapshot memory. */
		_hx_free(pGenifSnap);
	}
	return genif_index;
}

/* Creates and initializes a dhcp interface object. */
static __DHCP_INTERFACE* InitDhcpIf(char* netif_name)
{
	__DHCP_INTERFACE* dhcp_if = NULL;
	ip_addr_t addr;
	unsigned long genif_index = -1;
	int wait_count = 0;

	if (NULL == netif_name)
	{
		_hx_printf("No interface specified.\r\n");
		goto __TERMINAL;
	}

	/* 
	 * Get the corresponding genif object. 
	 * We try several times and wait a short time
	 * for each try since the genif may not be added
	 * into system when boot up.
	 */
	while (wait_count < 10)
	{
		genif_index = __GetGenifByName(netif_name);
		if (genif_index != -1)
		{
			/* got the genif. */
			break;
		}
		/* Wait 1s and try again. */
		Sleep(1000);
		wait_count++;
	}
	/* Could not get the genif after retry. */
	if (-1 == genif_index)
	{
		__LOG("[%s]no genif[%s] found after[%d] retries.\r\n",
			__func__, 
			netif_name,
			wait_count);
		goto __TERMINAL;
	}

	/* Interface index out of range. */
	if (if_index > DHCPD_SERVER_IPADDR2)
	{
		_hx_printf("If index out of range.\r\n");
		goto __TERMINAL;
	}

	/* Create a dhcp interface object and initializes it. */
	dhcp_if = (__DHCP_INTERFACE*)_hx_malloc(sizeof(__DHCP_INTERFACE));
	if (NULL == dhcp_if)
	{
		_hx_printf("[%s]:Out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	memset((void*)dhcp_if, 0, sizeof(__DHCP_INTERFACE));
	/* Allocate data recv buffer. */
	dhcp_if->recv_buff = (char*)_hx_malloc(DHCP_RECV_BUFSZ);
	if (NULL == dhcp_if->recv_buff)
	{
		_hx_printf("No memory for buff.\r\n");
		_hx_free(dhcp_if);
		dhcp_if = NULL;
		goto __TERMINAL;
	}

	/* Initializes the interface object. */
	dhcp_if->genif_index = genif_index;
	dhcp_if->sock = -1; /* Create later. */
	dhcp_if->alloc_head.pNext = dhcp_if->alloc_head.pPrev = &dhcp_if->alloc_head;
	dhcp_if->host_start = 2;
	dhcp_if->net_part1 = DHCPD_SERVER_IPADDR0;
	dhcp_if->net_part2 = DHCPD_SERVER_IPADDR1;
	dhcp_if->net_part3 = DHCPD_SERVER_IPADDR2 - if_index;
	/* Set and update interface index. */
	dhcp_if->if_index = if_index++;
	/* Init ip/mask/gw of this if. */
	addr.addr = dhcp_if->net_part1;
	addr.addr <<= 8;
	addr.addr += dhcp_if->net_part2;
	addr.addr <<= 8;
	addr.addr += dhcp_if->net_part3;
	addr.addr <<= 8;
	addr.addr += 1;
	addr.addr = _hx_htonl(addr.addr);
	dhcp_if->if_addr = addr;
	dhcp_if->if_gw = addr;
	addr.addr = 255;
	addr.addr <<= 8;
	addr.addr += 255;
	addr.addr <<= 8;
	addr.addr += 255;
	dhcp_if->if_mask = addr;

__TERMINAL:
	return dhcp_if;
}

/* 
 * Start dhcp service on an interface. 
 * It sets ipaddr,mask,and gateway on this interface,
 * create a server socket and listening on it.
 */
static BOOL StartDhcpIf(__DHCP_INTERFACE* pIf)
{
	BOOL bResult = FALSE;
	int sock = 0;
	int optval = 1;
	struct sockaddr_in server_addr;
	__COMMON_NETWORK_ADDRESS comm_addr[3];

	/* Configure ip address on the genif. */
	BUG_ON(NULL == pIf);
	comm_addr[0].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[0].Address.ipv4_addr = pIf->if_addr.addr;
	comm_addr[1].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[1].Address.ipv4_addr = pIf->if_mask.addr;
	comm_addr[2].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[2].Address.ipv4_addr = pIf->if_gw.addr;
	if (0 != NetworkManager.AddGenifAddress(pIf->genif_index,
		NETWORK_PROTOCOL_TYPE_IPV4,
		comm_addr, 3, FALSE))
	{
		__LOG("Set ip failed.\r\n");
		goto __TERMINAL;
	}

	/* New a server sock and listen on it. */
		/* create a socket with UDP */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		DEBUG_PRINTF("Sock fail[%d] on if[%d].\r\n", 
			errno, pIf->if_index);
		goto __TERMINAL;
	}

	/* set to receive broadcast packet */
	lwip_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
	/* Make the dhcp server port bound to multiple sockets. */
	lwip_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	/* initialize server address */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DHCP_PORT_SERVER);
	server_addr.sin_addr.s_addr = pIf->if_addr.addr; /* addr of if already in network order. */
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/* bind socket to the server address */
	if (bind(sock, (struct sockaddr *)&server_addr,
		sizeof(struct sockaddr)) == -1)
	{
		/* bind failed. */
		DEBUG_PRINTF("Bind server address fail[%d] on if[%d].\r\n", 
			errno, pIf->if_index);
		goto __TERMINAL;
	}
	/* Save sock to dhcp interface object. */
	pIf->sock = sock;

	DEBUG_PRINTF("Start dhcp server on if[%d].\r\n", pIf->genif_index);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Start routine of DHCP server. */
static BOOL dhcp_server_start(char* netif_name)
{
	__DHCP_INTERFACE* pIf = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == netif_name);

	/* Init the specified interface. */
	pIf = InitDhcpIf(netif_name);
	if (NULL == pIf)
	{
		goto __TERMINAL;
	}

	/* Start DHCP service on the interface. */
	if (!StartDhcpIf(pIf))
	{
		_hx_printf("Start service failed on if[%d]\r\n", pIf->if_index);
		goto __TERMINAL;
	}

	/* Create the mutex object to protect interface list. */
	if (NULL == if_mtx)
	{
		if_mtx = CreateMutex();
		if (NULL == if_mtx)
		{
			_hx_printf("Create mtx failed.\r\n");
			goto __TERMINAL;
		}
	}

	/*
	 * Now add the interface to list so as it
	 * can be monitored and processed by main thread.
	 */
	WaitForThisObject(if_mtx);
	pIf->pNext = if_list_head;
	if_list_head = pIf;
	ReleaseMutex(if_mtx);

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
			pIf,
			NULL,
			DHCP_SERVER_NAME);
		if (NULL == pDhcpThread)
		{
			_hx_printk("Start dhcp service failed.\r\n");
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

	/* Everything is in place. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Add one interface to DHCP server enabled list. 
 * Server will listen on this interface after added.
 */
static BOOL AddOneInterface(char* name)
{
	__DHCP_INTERFACE* pIf = NULL;
	
	BUG_ON(NULL == name);

	/* Init the specified interface. */
	pIf = InitDhcpIf(name);
	if (NULL == pIf)
	{
		return FALSE;
	}

	/* Start DHCP service on the interface. */
	if (!StartDhcpIf(pIf))
	{
		_hx_printf("Start service failed on if[%d]\r\n", pIf->if_index);
		return FALSE;
	}

	/* 
	 * Now add the interface to list so as it 
	 * can be monitored and processed by main thread. 
	 */
	WaitForThisObject(if_mtx);
	pIf->pNext = if_list_head;
	if_list_head = pIf;
	ReleaseMutex(if_mtx);

	return TRUE;
}

/* Start DHCP server on a given interface. */
BOOL DHCPSrv_Start_Onif(char* ifName)
{
	if (NULL == if_list_head)
	{
		/* 
		 * First time enabling dhcp on interface,dhcp
		 * server thread should be brought up.
		 */
		dhcp_server_start(ifName);
	}
	else
	{
		/* dhcp server started,just add one if. */
		AddOneInterface(ifName);
	}
	return TRUE;
}

/* 
 * start dhcp server thread in process of 
 * system initialization. 
 */
BOOL start_dhcp_server()
{
	BOOL bResult = FALSE;

	/* Create the mutex object to protect interface list. */
	if (NULL == if_mtx)
	{
		if_mtx = CreateMutex();
		if (NULL == if_mtx)
		{
			__LOG("[%s]create mtx failed.\r\n", __func__);
			goto __TERMINAL;
		}
	}

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
			NULL,
			NULL,
			DHCP_SERVER_NAME);
		if (NULL == pDhcpThread)
		{
			__LOG("[%s]start dhcp service failed.\r\n", __func__);
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

	/* Everything is in place. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}
