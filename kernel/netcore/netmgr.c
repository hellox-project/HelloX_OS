//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 12, 2017
//    Module Name               : netmgr.h
//    Module Funciton           : 
//                                Network manager object's implementation.
//                                Network manager is the core object of network
//                                subsystem in HelloX,it supplies network common
//                                service,such as timer,common task,...
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>

#include "netcfg.h"
#include "proto.h"
#include "genif.h"
#include "netmgr.h"
#include "netglob.h"

#include "lwip/inet.h"

/* Initializer of network manager. */
static BOOL NetMgrInitialize(__NETWORK_MANAGER* pMgr)
{
	BOOL bResult = FALSE;
	int i = 0;

	BUG_ON(NULL == pMgr);

	/* Initialize network global object. */
	NetworkGlobal.Initialize(&NetworkGlobal);

	/* Initialize all network protocol object(s) in system. */
	i = 0;
	while ((NetworkProtocolArray[i].szProtocolName != NULL) && 
		(NetworkProtocolArray[i].ucProtocolType))
	{
		if (!NetworkProtocolArray[i].Initialize(&NetworkProtocolArray[i]))
		{
			/* Fail of any protocol object's initialization will lead whole failure. */
			goto __TERMINAL;
		}
		_hx_printf("Init [%s] OK.\r\n",NetworkProtocolArray[i].szProtocolName);
		i++;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Create a generic netif object and returns it's base,the
 * caller then should initialize it,and register it into
 * system by calling RegisterGenif routine.
 */
static __GENERIC_NETIF* __CreateGenif(__GENERIC_NETIF* parent,
	__GENIF_DESTRUCTOR destructor)
{
	__GENERIC_NETIF* pGenif = NULL;
	unsigned long ulFlags = 0;

	/* 
	 * Create genif from kerne heap. 
	 * We use aligned allocation and keep the last 2 bits as 0,
	 * so this 2 bits could be used by other modules,such as IP protocol,
	 * to distinguish it's a genif or an ethernet interface just 
	 * by given a pointer.
	 */
	pGenif = (__GENERIC_NETIF*)_hx_aligned_malloc(sizeof(__GENERIC_NETIF), 4);
	if (NULL == pGenif)
	{
		goto __TERMINAL;
	}
	/* Clear it and initiailze it's refer counter. */
	memset((void*)pGenif, 0, sizeof(__GENERIC_NETIF));

	__ATOMIC_INCREASE(&pGenif->if_count);
	pGenif->genif_destructor = destructor;
	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	pGenif->if_index = NetworkManager.genif_index;
	NetworkManager.genif_index++;
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

__TERMINAL:
	return pGenif;
}

/* 
 * Local helper to verify a genif object's member member's 
 * value before register it into system. 
 */
static BOOL __VerifyGenifMembers(__GENERIC_NETIF* pGenif)
{
	if (0 == pGenif->if_count)
	{
		return FALSE;
	}
	if (NULL == pGenif->genif_input)
	{
		/* Packet input routine must be specified. */
		return FALSE;
	}
	if (NULL == pGenif->genif_output)
	{
		/* Packet sending routine must be present. */
		return FALSE;
	}
	if (0 == pGenif->genif_name[0])
	{
		/* Name must be specified. */
		return FALSE;
	}
	if (pGenif->pGenifNext)
	{
		/* Re-register may raise. */
		return FALSE;
	}
	return TRUE;
}

/*
 * Verify a genif object,to check if it's a
 * legal generic netif object in system, or just 
 * a illegal pointer.
 * NOTE: No protection applied to this routine,since
 * the caller must obtain network manager's spin lock
 * before call it.
 */
static BOOL __VerifyGenif(__GENERIC_NETIF* pGenif)
{
	__GENERIC_NETIF* pNext = NetworkManager.pGenifRoot;

	while (pNext)
	{
		if (pGenif == pNext)
		{
			/* Valid object. */
			return TRUE;
		}
		pNext = pNext->pGenifNext;
	}
	return FALSE;
}

/*
 * A local helper to bind a genif to network protocols in
 * system.
 * The AddGenif routine of all protocols will be invoked,
 * and the protocol object's base will be saved to protocol
 * bingding array of the genif,if bind success.
 */
static BOOL __ProtocolBind(__GENERIC_NETIF* pGenif)
{
	BOOL bResult = FALSE;
	__NETWORK_PROTOCOL* pProtocol = NULL;
	int proto_idx = 0, bind_idx = 0;

	/* Clear the protocol binding array. */
	memset(pGenif->proto_binding, 0, sizeof(pGenif->proto_binding));
	while ((NetworkProtocolArray[proto_idx].szProtocolName) &&
		bind_idx < GENIF_MAX_PROTOCOL_BINDING)
	{
		pProtocol = &NetworkProtocolArray[proto_idx];
		proto_idx++;
		pGenif->proto_binding[bind_idx].pIfState = pProtocol->AddGenif(pGenif);
		if (pGenif->proto_binding[bind_idx].pIfState)
		{
			/* Genif added to protocol success,bind OK. */
			pGenif->proto_binding[bind_idx].pProtocol = pProtocol;
			bind_idx++;
			bResult = TRUE;
		}
	}

	return bResult;
}

/*
 * Register a genif object created by CreateGenif routine into system.
 * The genif is visiable to system only after registration through this
 * routine.
 */
static BOOL __RegisterGenif(__GENERIC_NETIF* pGenif)
{
	unsigned long ulFlags;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pGenif);
	/* Validates the generic netif object. */
	BUG_ON(0 == pGenif->if_count);
	if (!__VerifyGenifMembers(pGenif))
	{
		/* Invalid genif object. */
		goto __TERMINAL;
	}

	/* Bind the genif to all network protocols. */
	if (!__ProtocolBind(pGenif))
	{
		/* Just show a warning. */
		_hx_printf("No protocol bind to genif[%s].\r\n", pGenif->genif_name);
	}

	/* Register it into proper location according to it's parent. */
	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	if (pGenif->pGenifParent)
	{
		/* Insert into parent genif's children list. */
		pGenif->pGenifSibling = pGenif->pGenifParent->pGenifChild;
		pGenif->pGenifParent->pGenifChild = pGenif;
		/* Increase refer counter since parent refers it. */
		__ATOMIC_INCREASE(&pGenif->if_count);
	}
	/* Insert into the genif global list of network manager. */
	if (NULL == NetworkManager.pGenifLast)
	{
		/* The first one in global list. */
		BUG_ON(NetworkManager.pGenifRoot);
		NetworkManager.pGenifRoot = pGenif;
		NetworkManager.pGenifLast = pGenif;
		pGenif->pGenifNext = NULL;
		NetworkManager.genif_num++;
	}
	else
	{
		/* Append to existing genif's back. */
		BUG_ON(NULL == NetworkManager.pGenifRoot);
		pGenif->pGenifNext = NULL;
		NetworkManager.pGenifLast->pGenifNext = pGenif;
		NetworkManager.pGenifLast = pGenif;
		NetworkManager.genif_num++;
	}
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
 * Local helper routine to destroy a genif.
 * It's the reverse operation of RegisterGenif.
 * Operation steps:
 * 1. Delete it from Network manager's global genif list;
 * 2. Clear all children's parent pointer and sibling pointer;
 * 3. Destroy children's refer counter,and destroy it if no reference;
 * 4. Release the memory.
 * Note:no protection applied to this routine,the caller
 * must obtain network manager's spin lock before invoke it.
 */
static BOOL __DestroyGenif(__GENERIC_NETIF* pGenif)
{
	BOOL bResult = FALSE;
	__GENERIC_NETIF* pSibling = NULL, *pParent = NULL, *pChild = NULL;
	__GENERIC_NETIF* pCurrChild = NULL;

	/* Remove from network manager's global list. */
	if (pGenif == NetworkManager.pGenifRoot)
	{
		/* First one. */
		if (pGenif == NetworkManager.pGenifLast)
		{
			/* Also is the last one. */
			NetworkManager.pGenifRoot = NULL;
			NetworkManager.pGenifLast = NULL;
			NetworkManager.genif_num--;
		}
		else
		{
			/* Not last one. */
			NetworkManager.pGenifRoot = pGenif->pGenifNext;
			NetworkManager.genif_num--;
		}
	}
	else
	{
		/* Not the first one. */
		pCurrChild = NetworkManager.pGenifRoot;
		while (pCurrChild)
		{
			if (pCurrChild->pGenifNext == pGenif)
			{
				/* Located the genif in list. */
				pCurrChild->pGenifNext = pGenif->pGenifNext;
				if (NULL == pGenif->pGenifNext)
				{
					/* genif is the last one. */
					NetworkManager.pGenifLast = pCurrChild;
				}
				NetworkManager.genif_num--;
				break;
			}
			pCurrChild = pCurrChild->pGenifNext;
		}
		if (NULL == pCurrChild)
		{
			/* Show a warning. */
			_hx_printk("%s:genif not in global list.\r\n", __func__);
		}
	}

	/* Dismiss all children if has any. */
	if (pGenif->pGenifChild)
	{
		pCurrChild = pGenif->pGenifChild;
		while (pCurrChild)
		{
			pChild = pCurrChild;
			pCurrChild = pCurrChild->pGenifSibling;
			/* Clear child's parent and sibling. */
			pChild->pGenifParent = NULL;
			pChild->pGenifSibling = NULL;
			/* 
			 * Decrease child's refer counter since 
			 * parent(pGenif) no longer refer it.
			 */
			__ATOMIC_DECREASE(&pChild->if_count);
			if (0 == pChild->if_count)
			{
				__DestroyGenif(pChild);
			}
		}
	}
	
	/* Destroy the genif object. */
	_hx_free(pGenif);

	bResult = TRUE;
	return bResult;
}

/* 
 * Decrease reference counter of generic netif, and 
 * destroy it if refer counter reaches 0.
 * It's children branch is also released recursively.
 */
static BOOL __ReleaseGenif(__GENERIC_NETIF* pGenif)
{
	unsigned long ulFlags = 0;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pGenif);
	
	/* Must obtain network manager's spin lock. */
	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	/* Verify the object. */
	if (!__VerifyGenif(pGenif))
	{
		__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Decrease refer counter. */
	__ATOMIC_DECREASE(&pGenif->if_count);
	if (0 == pGenif->if_count)
	{
		/* 
		 * Just release it if does not refered yet. 
		 * Unlink it from parent's child list if it has
		 * parent.
		 */
		if (pGenif->pGenifParent)
		{
			__GENERIC_NETIF* pParent = pGenif->pGenifParent;
			__GENERIC_NETIF* pSibling = NULL;

			/* Remove itself from sibling list. */
			pSibling = pParent->pGenifChild;
			if (pSibling == pGenif)
			{
				/* First one in list. */
				pParent->pGenifChild = pGenif->pGenifSibling;
			}
			else
			{
				while (pSibling)
				{
					if (pSibling->pGenifSibling == pGenif)
					{
						pSibling->pGenifSibling = pGenif->pGenifSibling;
						break;
					}
					pSibling = pSibling->pGenifSibling;
				}
				/* Not in sibling list but parent set,show a warning. */
				_hx_printk("%s:not in sibling list with parent set.\r\n", __func__);
			}
		}
		/* Reset parent/sibling pointers. */
		pGenif->pGenifParent = pGenif->pGenifSibling = NULL;

		/* 
		 * It's children branch is also released in
		 * DestroyGenif routine,since it's a recursive routine.
		 */
		bResult = __DestroyGenif(pGenif);
	}
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

__TERMINAL:
	return bResult;
}

/* Increase refer counter of genif, returns the old value. */
static __atomic_t __GetGenif(__GENERIC_NETIF* pGenif)
{
	unsigned long ulFlags = 0;
	__atomic_t ret_val = 0;

	BUG_ON(NULL == pGenif);

	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	if (!__VerifyGenif(pGenif))
	{
		/* Invalid genif object. */
		__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Increase it's refer counter. */
	ret_val = pGenif->if_count;
	__ATOMIC_INCREASE(&pGenif->if_count);
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

__TERMINAL:
	return ret_val;
}

/*
 * Returns all generic network interfaces info.
 * The caller should specify a bulk of memory,this routine
 * will pack all genifs in system into this memory block,with
 * operation routines set to NULL,buff_req should be set as
 * the buffer's length by caller,genif's number will be returned
 * as return value.
 * 0 will be returned if buffer is not enough and buff_req will
 * be set by this routine to indicate the required memory,caller
 * should invoke this routine again with a larger memory buffer.
 * -1 will be returned in case of bad parameters,or no genif
 * in system at all.
 */
static int __GetGenifInfo(__GENERIC_NETIF* pGenifList, unsigned long* buff_req)
{
	unsigned long genif_num = 0;
	unsigned long ulFlags = 0;
	int ret_val = -1;
	__GENERIC_NETIF* pCopyStart = NULL;

	if ((NULL == pGenifList) || (NULL == buff_req))
	{
		goto __TERMINAL;
	}

	/* Pack all genif objects into user specified buffer. */
	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	genif_num = NetworkManager.genif_num;
	if (0 == genif_num)
	{
		/* No genif in system. */
		ret_val = -1;
		__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
		goto __TERMINAL;
	}
	if (*buff_req < genif_num * sizeof(__GENERIC_NETIF))
	{
		/* Buffer too small. */
		*buff_req = genif_num * sizeof(__GENERIC_NETIF);
		ret_val = 0;
		__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
		goto __TERMINAL;
	}
	pCopyStart = NetworkManager.pGenifRoot;
	while (pCopyStart)
	{
		memcpy(pGenifList, pCopyStart, sizeof(__GENERIC_NETIF));
		/* Clear the internal used members. */
		pGenifList->pGenifNext = NULL;
		/* Move to next. */
		pGenifList++;
		pCopyStart = pCopyStart->pGenifNext;
	}
	ret_val = genif_num;
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

__TERMINAL:
	return ret_val;
}

/* Local helper routine used to retrieve genif by it's index. */
static __GENERIC_NETIF* __GetGenifByIndex(unsigned long genif_index)
{
	unsigned long ulFlags;
	__GENERIC_NETIF* pGenif = NULL;

	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	pGenif = NetworkManager.pGenifRoot;
	while (pGenif)
	{
		if (genif_index == pGenif->if_index)
		{
			break;
		}
		pGenif = pGenif->pGenifNext;
	}
	/* Increase the refer counter. */
	if (pGenif)
	{
		__ATOMIC_INCREASE(&pGenif->if_count);
	}
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

	return pGenif;
}

/* Local helper routine used to retrieve genif by it's name. */
static __GENERIC_NETIF* __GetGenifByName(const char* genif_name)
{
	unsigned long ulFlags;
	__GENERIC_NETIF* pGenif = NULL;

	/* Verify the name's length. */
	BUG_ON(strlen(genif_name) >= GENIF_NAME_LENGTH);

	__ENTER_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);
	pGenif = NetworkManager.pGenifRoot;
	while (pGenif)
	{
		if (0 == strcmp(genif_name, pGenif->genif_name))
		{
			break;
		}
		pGenif = pGenif->pGenifNext;
	}
	/* Increase the refer counter. */
	if (pGenif)
	{
		__ATOMIC_INCREASE(&pGenif->if_count);
	}
	__LEAVE_CRITICAL_SECTION_SMP(NetworkManager.spin_lock, ulFlags);

	return pGenif;
}

 /* Local helper to add an ipv4 address to genif. */
static int __AddGenifAddress_ipv4(__GENERIC_NETIF* pGenif,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	int ret_val = ERR_ARG;
	__NETWORK_PROTOCOL* pProtocol = NULL;
	LPVOID pIfState = NULL;

	/* Validate the input parameters. */
	if (bSecondary)
	{
		UNIMPLEMENTED_ROUTINE_CALLED;
		goto __TERMINAL;
	}
	if (addr_num != 3)
	{
		goto __TERMINAL;
	}
	for (int i = 0; i < 3; i++)
	{
		if (comm_addr[i].AddressType != NETWORK_ADDRESS_TYPE_IPV4)
		{
			goto __TERMINAL;
		}
	}

	/* Locate the ipv4 protocol bind to this genif. */
	for (int i = 0; i < GENIF_MAX_PROTOCOL_BINDING; i++)
	{
		pProtocol = pGenif->proto_binding[i].pProtocol;
		if (NULL == pProtocol)
		{
			continue;
		}
		if (pProtocol->ucProtocolType == NETWORK_PROTOCOL_TYPE_IPV4)
		{
			pIfState = pGenif->proto_binding[i].pIfState;
			BUG_ON(NULL == pIfState);
			break;
		}
	}
	if (!pIfState)
	{
		/* No ipv4 protocol bind to the genif. */
		ret_val = ERR_BIND;
		goto __TERMINAL;
	}
	/* 
	 * Tell the ipv4 protocol that a new address is
	 * configured to this genif. 
	 */
	if (!pProtocol->AddGenifAddress(pGenif, pIfState,
		comm_addr, addr_num, bSecondary))
	{
		ret_val = ERR_UNKNOWN;
		goto __TERMINAL;
	}

	/* Set genif's ipv4 address. */
	pGenif->ip_addr[0].addr = comm_addr[0].Address.ipv4_addr;
	pGenif->ip_mask[0].addr = comm_addr[1].Address.ipv4_addr;
	pGenif->ip_gw[0].addr = comm_addr[2].Address.ipv4_addr;

	/* Set return value as OK. */
	ret_val = ERR_OK;

__TERMINAL:
	return ret_val;
}

/* Local helper to add an ipv6 address to genif. */
static int __AddGenifAddress_ipv6(__GENERIC_NETIF* pGenif,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	return ERR_UNKNOWN;
}

/* Local helper to add an hpx address to genif. */
static int __AddGenifAddress_hpx(__GENERIC_NETIF* pGenif,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	return ERR_UNKNOWN;
}

/* Configure address on the genif. */
static int __AddGenifAddress(unsigned long genif_index, 
	unsigned long protocol,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	__GENERIC_NETIF* pGenif = NULL;
	int ret_val = ERR_ARG;

	BUG_ON((NULL == comm_addr));
	/* Obtain the generic netif. */
	pGenif = __GetGenifByIndex(genif_index);
	if (NULL == pGenif)
	{
		goto __TERMINAL;
	}

	/* Dispatch to different handler by protocol type. */
	if (protocol == NETWORK_PROTOCOL_TYPE_IPV4)
	{
		if (addr_num != 3)
		{
			/* addr,mask,gw must be specified. */
			goto __TERMINAL;
		}
		ret_val = __AddGenifAddress_ipv4(pGenif,
			comm_addr,
			addr_num,
			bSecondary);
		goto __TERMINAL;
	}
	if (protocol == NETWORK_PROTOCOL_TYPE_IPV6)
	{
		ret_val = __AddGenifAddress_ipv6(pGenif,
			comm_addr,
			addr_num,
			bSecondary);
		goto __TERMINAL;
	}
	if (protocol == NETWORK_PROTOCOL_TYPE_HPX)
	{
		ret_val = __AddGenifAddress_hpx(pGenif,
			comm_addr,
			addr_num,
			bSecondary);
		goto __TERMINAL;
	}

__TERMINAL:
	/* genif should be released. */
	if (pGenif) {
		__ReleaseGenif(pGenif);
	}
	return ret_val;
}

/* 
 * Print out one genif. The genif object is obtained 
 * by the caller before call this routine,so it can be
 * gaurantee that the genif is not released.
 * Another scenario is, the caller captures all genifs
 * in system by calling GetGenifInfo into a snapshot,then
 * call this routine to show out all genif snapshot one
 * by one.
 */
static void __ShowOneGenif(__GENERIC_NETIF* pGenif)
{
	char* link_status = NULL;
	char* duplex = NULL;
	char* speed = NULL;

	/* Link status to string. */
	if (pGenif->link_status == up)
	{
		link_status = "UP";
	}
	else
	{
		link_status = "DOWN";
	}
	/* Duplex to string. */
	if (pGenif->link_duplex == full)
	{
		duplex = "FULL";
	}
	else
	{
		duplex = "HALF";
	}
	/* Speed to string. */
	switch (pGenif->link_speed)
	{
	case _10M:
		speed = "10Mbps";
		break;
	case _100M:
		speed = "100Mbps";
		break;
	case _1000M:
		speed = "1000Mbps";
		break;
	case _10G:
		speed = "10Gbps";
		break;
	case _40G:
		speed = "40Gbps";
		break;
	case _100G:
		speed = "100Gbps";
		break;
	default:
		speed = "UNKNOWN";
		break;
	}

	/* Show out general information. */
	_hx_printf("  --------------------------\r\n");
	_hx_printf("  genif name: %s\r\n", pGenif->genif_name);
	_hx_printf("  link status: %s\r\n", link_status);
	_hx_printf("  link duplex/speed: %s/%s\r\n", duplex, speed);

	/* Show out hard address. */
	_hx_printf("  hard addr: [%.2X-%.2X-%.2X-%.2X-%.2X-%.2X]\r\n",
		(unsigned char)pGenif->genif_ha[0],
		(unsigned char)pGenif->genif_ha[1],
		(unsigned char)pGenif->genif_ha[2],
		(unsigned char)pGenif->genif_ha[3],
		(unsigned char)pGenif->genif_ha[4],
		(unsigned char)pGenif->genif_ha[5]);

	/* Show out network address. */
#if defined(__CFG_NET_IPv4)
	/* Show IPv4 address. */
	_hx_printf("  ip addr/mask: %s/%s\r\n", inet_ntoa(pGenif->ip_addr[0]),
		inet_ntoa(pGenif->ip_mask[0]));
	_hx_printf("  gateway: %s\r\n", inet_ntoa(pGenif->ip_gw[0]));
#endif

	/* Show statistics counter of the genif. */
	_hx_printf("  rx/tx/rx_bytes/tx_bytes: %d/%d/%d/%d\r\n",
		pGenif->stat.rx_pkt,
		pGenif->stat.tx_pkt,
		pGenif->stat.rx_bytes,
		pGenif->stat.tx_bytes);
	_hx_printf("  rx_m/tx_m/rx_b/tx_b: %d/%d/%d/%d\r\n",
		pGenif->stat.rx_mcast,
		pGenif->stat.tx_mcast,
		pGenif->stat.rx_bcast,
		pGenif->stat.tx_bcast);
	_hx_printf("  rx_err/tx_err: %d/%d\r\n",
		pGenif->stat.rx_err,
		pGenif->stat.tx_err);

	/* Invoke driver specific show out routine. */
	if (pGenif->specific_show)
	{
		pGenif->specific_show(pGenif);
	}
	return;
}

/* 
 * Show all genifs in system. 
 * It captures all genifs in system by invoking
 * GetGenifInfo routine,then show out one by one.
 */
static unsigned long __ShowAllGenif()
{
	__GENERIC_NETIF* pGenif = NULL;
	unsigned long buff_req = sizeof(__GENERIC_NETIF);
	int ret_val = -1;

	/* Allocate genif snapshot memory pool. */
	pGenif = (__GENERIC_NETIF*)_hx_malloc(buff_req);
	if (NULL == pGenif)
	{
		_hx_printf("out of memory.\r\n");
		goto __TERMINAL;
	}
	/* Try to capture all genifs in system. */
	while (TRUE)
	{
		ret_val = NetworkManager.GetGenifInfo(pGenif, &buff_req);
		if (-1 == ret_val)
		{
			/* No genif in system at all. */
			ret_val = 0;
			goto __TERMINAL;
		}
		if (0 == ret_val)
		{
			/* Memory is not enough,realloc and try again. */
			_hx_free(pGenif);
			BUG_ON(0 == buff_req);
			pGenif = (__GENERIC_NETIF*)_hx_malloc(buff_req);
			if (NULL == pGenif)
			{
				_hx_printf("[%s]out of memory.\r\n", __func__);
				goto __TERMINAL;
			}
			continue;
		}
		/* Get genif info success. */
		break;
	}
	/* Show all genifs' brief info one by one. */
	_hx_printf("        genif_name    genif_index    parent_index\r\n");
	_hx_printf("  ----------------    -----------    ------------\r\n");
	for(int i = 0; i < ret_val; i++)
	{
		_hx_printf("  %16s    %11d    %11d\r\n",
			pGenif[i].genif_name,
			pGenif[i].if_index,
			pGenif[i].pGenifParent ? pGenif[i].pGenifParent->if_index : -1);
	}
__TERMINAL:
	/* Release pGenif first. */
	if (pGenif)
	{
		_hx_free(pGenif);
		pGenif = NULL;
	}
	return ret_val;
}

/* Show out one or all genif in system. */
static int __ShowGenif(int nGenifIndex)
{
	int nShowed = 0;
	__GENERIC_NETIF* pGenif = NULL;

	if (-1 == nGenifIndex)
	{
		/* Show out all genifs in system. */
		nShowed = __ShowAllGenif();
	}
	else
	{
		/* Show out one generic netif. */
		pGenif = __GetGenifByIndex((unsigned long)nGenifIndex);
		if (NULL == pGenif)
		{
			_hx_printf("No genif[index = %d] found.\r\n", nGenifIndex);
		}
		else
		{
			__ShowOneGenif(pGenif);
			__ReleaseGenif(pGenif);
			nShowed = 1;
		}
	}

	return nShowed;
}

/* 
 * Local helper to notify all protocols bound to 
 * one genif that the link's status has change.
 */
static void __NotifyLinkStatusChange(__GENERIC_NETIF* pGenif, BOOL link_down)
{
	__NETWORK_PROTOCOL* pProtocol = NULL;
	int i = 0;

	for (i = 0; i < GENIF_MAX_PROTOCOL_BINDING; i++)
	{
		pProtocol = pGenif->proto_binding[i].pProtocol;
		if (pProtocol)
		{
			pProtocol->LinkStatusChange(
				pGenif->proto_binding[i].pIfState,
				link_down);
		}
	}
}

/* Handle link status change event of an genif. */
static VOID __LinkStatusChange(__GENERIC_NETIF* pGenif,
	enum __LINK_STATUS link_status,
	enum __DUPLEX duplex,
	enum __ETHERNET_SPEED speed)
{
	if (NULL == pGenif)
	{
		goto __TERMINAL;
	}
	if (down == link_status)
	{
		if (down == pGenif->link_status)
		{
			/* Interface already down. */
			goto __TERMINAL;
		}
		pGenif->link_status = down;
		/* 
		 * Notify the protocols bound to this genif,so
		 * the protocol may do some actions,such as stopping
		 * forward on this interface.
		 */
		__NotifyLinkStatusChange(pGenif, TRUE);
		goto __TERMINAL;
	}
	if (up == link_status)
	{
		if (pGenif->link_status == up)
		{
			/* Status already is up. */
			goto __TERMINAL;
		}
		pGenif->link_status = up;
		pGenif->link_duplex = duplex;
		pGenif->link_speed = speed;
		/* Notify all protocols bound to this genif. */
		__NotifyLinkStatusChange(pGenif, FALSE);
		goto __TERMINAL;
	}

__TERMINAL:
	return;
}

/* Network manager object. */
__NETWORK_MANAGER NetworkManager = {
	NetMgrInitialize,       //Initialize.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,   //spin_lock.
#endif
	NULL,                   //pGenifRoot.
	NULL,                   //pGenifLast.
	0,                      //genif_num.
	0,                      //genif_index.
	__CreateGenif,          //CreateGenif.
	__ReleaseGenif,         //ReleaseGenif.
	__GetGenif,             //GetGenif.
	__GetGenifByIndex,      //GetGenifByIndex.
	__GetGenifByName,       //GetGenifByName.
	__RegisterGenif,        //RegisterGenif.
	__GetGenifInfo,         //GetGenifInfo.
	__AddGenifAddress,      //AddGenifAddress.
	__ShowGenif,            //ShowGenif.
	__LinkStatusChange,     //LinkStatusChange.
};
