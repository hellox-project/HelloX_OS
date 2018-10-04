//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 09,2017
//    Module Name               : pppoe.c
//    Module Funciton           : 
//                                Implementation code of PPPoE function,
//                                mainly global variables,functions.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netif/ppp_oe.h"
#include "pppdebug.h"
#include "lwip/inet.h"

#include "netcfg.h"
#include "tmo.h"
#include "ethmgr.h"
#include "oemgr.h"
#include "oe_pro.h"
#include "netglob.h"
#include "ppp.h"

/* PPPoE session ID. */
static int session_id = 0;

/* Create a new PPPoE instance. */
static __PPPOE_INSTANCE* CreatePPPoEInstance(char* ethName, char* instName, char* user_name, 
	char* password,__PPPOE_AUTH_TYPE authType)
{
	__PPPOE_INSTANCE* pInstance = NULL;
	int i = 0;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	__PPPOE_ETHIF_BINDING* pBinding = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == instName);
	BUG_ON(NULL == user_name);
	BUG_ON(NULL == ethName);

	/* 
	 * Can not create PPPoE instance without any ethernet interface
	 * that bound to PPPoE protocol.
	 */
	if (0 == current_bind_num)
	{
		goto __TERMINAL;
	}

	pInstance = _hx_malloc(sizeof(__PPPOE_INSTANCE));
	if (NULL == pInstance)
	{
		goto __TERMINAL;
	}
	/* Initialize it. */
	memset(pInstance, 0, sizeof(*pInstance));
	pInstance->authType = authType;
	pInstance->instance_id = session_id++;
	pInstance->ppp_session_id = -1;
	pInstance->status = SESSION_IDLE;
	pInstance->pEthInt = NULL;

	/* Set PPPoE user name. */
	if (strlen(user_name) > PPPOE_USER_NAME_LEN)
	{
		goto __TERMINAL;
	}
	strcpy(pInstance->user_name, user_name);

	/* Set PPPoE instance name. */
	if (strlen(instName) > PPPOE_SESSION_NAME_LEN)
	{
		goto __TERMINAL;
	}
	strcpy(pInstance->session_name, instName);

	/* Set password if specified. */
	if (strlen(password) > PPPOE_PASSWORD_LEN)
	{
		goto __TERMINAL;
	}
	strcpy(pInstance->password, password);

#if 0
	/* Set the ethernet interface this session based on. */
	pEthInt = EthernetManager.GetEthernetInterface(ethName);
	if (NULL == pEthInt)
	{
		goto __TERMINAL;
	}
	pInstance->pEthInt = pEthInt;
#endif

	/* Find the ethernet interface that this instance based on. */
	for (i = 0; i < PPPOE_MAX_INSTANCE_NUM; i++)
	{
		pBinding = &pppoeBinding[i];
		if (NULL == pBinding->pEthInt)
		{
			continue;
		}
		if (0 == strcmp(ethName, pBinding->pEthInt->ethName))
		{
			pInstance->pEthInt = pBinding->pEthInt;
			pBinding->pInstance = pInstance;
			break;
		}
	}
	if (NULL == pInstance->pEthInt) /* Can not match ethernet interface. */
	{
		goto __TERMINAL;
	}

	/* Link the instance to global list. */
	pInstance->pNext = pppoeManager.pInstanceList;
	pppoeManager.pInstanceList = pInstance;

	/* Set success indictor. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult) /* Should release the instance. */
	{
		if (pInstance)
		{
			_hx_free(pInstance);
			pInstance = NULL;
		}
	}
	return pInstance;
}

/* PPP link status call back. */
static void pppoeStatusCallback(void* ctx, int errCode, void* arg)
{
	struct ppp_addrs* pAddrs = NULL;

	switch (errCode)
	{
	case PPPERR_NONE: /* PPP session OK,no error. */
		pAddrs = (struct ppp_addrs*)arg;
		/* Save DNS servers to network global object. */
		NetworkGlobal.dns_primary.addr = pAddrs->dns1.addr;
		NetworkGlobal.dns_secondary.addr = pAddrs->dns2.addr;
		/* Show out established PPP parameters. */
		_hx_printf("PPP connection established:\r\n");
		_hx_printf("  local addr: %s\r\n", inet_ntoa(pAddrs->our_ipaddr));
		_hx_printf("  peer addr: %s\r\n", inet_ntoa(pAddrs->his_ipaddr));
		_hx_printf("  primary DNS: %s\r\n", inet_ntoa(pAddrs->dns1));
		_hx_printf("  secondary DNS: %s\r\n", inet_ntoa(pAddrs->dns2));
		break;
		/* Log out error information,according error code. */
	case PPPERR_AUTHFAIL:
		__LOG("PPPoE authentication failed.\r\n");
		break;
	case PPPERR_CONNECT:
		__LOG("PPPoE connection lost.\r\n");
	case PPPERR_PARAM:
	case PPPERR_OPEN:
	case PPPERR_DEVICE:
	case PPPERR_ALLOC:
	case PPPERR_USER:
	case PPPERR_PROTOCOL:
		__LOG("PPP connect error[code = %d]\r\n", errCode);
		break;
	default:
		break;
	}
}

/* Start a PPPoE session by specifying the instance. */
static BOOL StartPPPoE(__PPPOE_INSTANCE* pInstance)
{
	BOOL bResult = FALSE;
	static int first_time = 0;
	enum pppAuthType auth = PPPAUTHTYPE_ANY;
	struct netif* pIf = NULL;

	BUG_ON(NULL == pInstance);

	if (pInstance->status != SESSION_IDLE)
	{
		goto __TERMINAL;
	}

	if (!first_time) /* Should initialize PPP stack of lwIP. */
	{
		pppInit();
		//first_time = 1;
	}

	/* Convert authentication type to lwIP defined. */
	switch (pInstance->authType)
	{
	case PPPOE_AUTH_ANY:
		auth = PPPAUTHTYPE_ANY;
		break;
	case PPPOE_AUTH_NONE:
		auth = PPPAUTHTYPE_NONE;
		break;
	case PPPOE_AUTH_PAP:
		auth = PPPAUTHTYPE_PAP;
		break;
	case PPPOE_AUTH_CHAP:
		auth = PPPAUTHTYPE_CHAP;
		break;
	default:
		BUG();
	}
	/* Set authentication type for the instance. */
	pppSetAuth(auth, pInstance->user_name, pInstance->password);

	/* Locate the lwIP netif. */
	pIf = pInstance->pEthInt->Proto_Interface[0].pL3Interface;

	/* Start PPPoE session. */
	pInstance->ppp_session_id = pppOverEthernetOpen(pIf, NULL, NULL, pppoeStatusCallback, NULL);
	if (pInstance->ppp_session_id < 0)
	{
		_hx_printf("Failed to start PPPoE session,err = %d.\r\n",
			pInstance->ppp_session_id);
		goto __TERMINAL;
	}
	/* Start PPPoE OK. */
	pInstance->status = SESSION_RUNNING;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Stop a PPPoE session. */
static BOOL StopPPPoE(__PPPOE_INSTANCE* pInstance)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pInstance);
	if (pInstance->status == SESSION_IDLE)
	{
		goto __TERMINAL;
	}
	/* 
	 * Just close PPP session here.
	 * The callback of PPPoE will destroy
	 * PPPoE control block,i.e,pppoe_soft_c.
	 */
	pppClose(pInstance->ppp_session_id);

	/* Reset DNS server to default. */
	NetworkGlobal.ResetDNSServer();

	pInstance->status = SESSION_IDLE;

__TERMINAL:
	return bResult;
}

/* Release a PPPoE instance object. */
static void DestroyPPPoEInstance(__PPPOE_INSTANCE* pInstance)
{
	__PPPOE_ETHIF_BINDING* pBinding = NULL;
	int i = 0;
	__PPPOE_INSTANCE* pPrev = NULL;
	__PPPOE_INSTANCE* pNext = NULL;

	BUG_ON(NULL == pInstance);
	
	if (pInstance->status != SESSION_IDLE)
	{
		return;
	}
	/* Unbind from ethernet interface. */
	for (i = 0; i < PPPOE_MAX_INSTANCE_NUM; i++)
	{
		pBinding = &pppoeBinding[i];
		if (pBinding->pInstance == pInstance)
		{
			pBinding->pInstance = NULL;
			break;
		}
	}
	BUG_ON(PPPOE_MAX_INSTANCE_NUM == i);

	/* Remove from instance list. */
	pPrev = pNext = pppoeManager.pInstanceList;
	while (pNext != pInstance)
	{
		if (NULL == pNext)
		{
			BUG(); /* The instance must be in list. */
		}
		pPrev = pNext;
		pNext = pNext->pNext;
	}
	pPrev->pNext = pNext->pNext;
	if (pppoeManager.pInstanceList == pInstance)
	{
		pppoeManager.pInstanceList = NULL;
	}

	/* Just release the instance object. */
	_hx_free(pInstance);
}

/* Start a PPPoE session by giving it's instance name. */
static BOOL StartPPPoEByName(char* instance)
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	BOOL bResult = FALSE;
	__KERNEL_THREAD_MESSAGE msg;

	BUG_ON(NULL == instance);
	/* Locate the instance object by comparing their name. */
	while (pInstance)
	{
		if (0 == strcmp(instance, pInstance->session_name))
		{
			break;
		}
		pInstance = pInstance->pNext;
	}
	if (NULL == pInstance)
	{
		_hx_printf("  Can not find the instance you specified.\r\n");
		goto __TERMINAL;
	}
	/* Start PPPoE session by sending message to main thread. */
	msg.wCommand = PPPOE_MSG_STARTSESSION;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pInstance;
	bResult = SendMessage(pppoeManager.hMainThread, &msg);

__TERMINAL:
	return bResult;
}

/* Stop a PPPoE session by giving it's name. */
static BOOL StopPPPoEByName(char* instance)
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	BOOL bResult = FALSE;
	__KERNEL_THREAD_MESSAGE msg;

	BUG_ON(NULL == instance);
	/* Locate the instance object by comparing their name. */
	while (pInstance)
	{
		if (0 == strcmp(instance, pInstance->session_name))
		{
			break;
		}
		pInstance = pInstance->pNext;
	}
	if (NULL == pInstance)
	{
		_hx_printf("  Can not find the instance you specified.\r\n");
		goto __TERMINAL;
	}

	/* Trigger stop process by sending message to main thread. */
	msg.wCommand = PPPOE_MSG_STOPSESSION;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pInstance;
	bResult = SendMessage(pppoeManager.hMainThread, &msg);

__TERMINAL:
	return bResult;
}

/* Destroy a PPPoE instance by giving it's name. */
static void DestroyPPPoEInstanceByName(char* instance)
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	BOOL bResult = FALSE;

	BUG_ON(NULL == instance);
	/* Locate the instance object by comparing their name. */
	while (pInstance)
	{
		if (0 == strcmp(instance, pInstance->session_name))
		{
			break;
		}
		pInstance = pInstance->pNext;
	}
	if (NULL == pInstance)
	{
		_hx_printf("  Can not find the instance you specified.\r\n");
		return;
	}
	DestroyPPPoEInstance(pInstance);
}

/* Authentication types that display. */
static char* auth_type_string[4] = {
	"none",
	"any",
	"pap",
	"chap"
};

/* List all PPPoE instance in system. */
void ListPPPoE()
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	while (pInstance)
	{
		_hx_printf("  Name: %s\r\n", pInstance->session_name);
		_hx_printf("    ethif name: %s\r\n", pInstance->pEthInt->ethName);
		_hx_printf("    username: %s\r\n", pInstance->user_name);
		_hx_printf("    password: ********\r\n");
		_hx_printf("    auth type: %s\r\n", auth_type_string[pInstance->authType]);
		_hx_printf("\r\n");
		pInstance = pInstance->pNext;
	}
}

/* 
 * Send out a packet through PPP/PPPoE main thread.
 * This routine is mainly used as sending routine of PPP interface,
 * which is called by IP layer.
 */
static BOOL pppSendPacket(struct netif* out_if, struct pbuf* pb, ip_addr_t* ipaddr)
{
	__PPP_SENDPACKET_BLOCK* pBlock = NULL;
	__KERNEL_THREAD_MESSAGE msg;
	DWORD dwFlags;
	BOOL bResult = FALSE;

	BUG_ON(NULL == out_if);
	BUG_ON(NULL == pb);

	/* Make sure the PPPoE main thread is in place. */
	if (NULL == pppoeManager.hMainThread)
	{
		goto __TERMINAL;
	}

	/* Allocate a parameter block object to hold all parameters. */
	pBlock = (__PPP_SENDPACKET_BLOCK*)_hx_malloc(sizeof(__PPP_SENDPACKET_BLOCK));
	if (NULL == pBlock)
	{
		goto __TERMINAL;
	}
	/* 
	 * Increase reference counter of pbuf,avoid to be deleted by 
	 * this routine's caller.
	 * The pbuf object will be released in the handler of SENDPACKET
	 * message.
	 */
	pbuf_ref(pb);

	pBlock->pNext = NULL;
	pBlock->out_if = out_if;
	pBlock->pkt_buff = pb; /* Should be confirmed if a new pbuf object should be allocated here. */
	pBlock->addr = *ipaddr;

	/*
	 * Hook the send packet block object into pppoeManager's out going list,
	 * and send a message to the PPPoE main thread if the list is empty,to
	 * trigger the sending process.
	 * Use critical section to protect the list operation since it maybe accessed
	 * in interrupt context.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
	if (0 == pppoeManager.nOutgSize) /* List is empty. */
	{
		BUG_ON(pppoeManager.pOutgFirst != NULL);
		BUG_ON(pppoeManager.pOutgLast != NULL);
		pppoeManager.pOutgFirst = pppoeManager.pOutgLast = pBlock;
		pBlock->pNext = NULL; /* Very important. */
		pppoeManager.nOutgSize++;
		/* 
		 * Send sending message to PPPoE main thread,exit the
		 * critical section since the SendMessage routine may lead
		 * kernel thread re-scheduling.
		 */
		__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
		msg.wCommand = PPPOE_MSG_SENDPACKET;
		msg.wParam = 0;
		msg.dwParam = 0;
		bResult = SendMessage(pppoeManager.hMainThread, &msg);
		if (!bResult) /* Msg queue is full? */
		{
			__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			/* Unlink from list. */
			pppoeManager.pOutgFirst = pppoeManager.pOutgLast = NULL;
			pppoeManager.nOutgSize = 0;
			__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
	}
	else /* The out going list is not empty. */
	{
		BUG_ON(NULL == pppoeManager.pOutgFirst);
		BUG_ON(NULL == pppoeManager.pOutgLast);
		if (pppoeManager.nOutgSize > PPPOE_MAX_PENDINGLIST_SIZE)
		{
			__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			bResult = FALSE;
			goto __TERMINAL;
		}
		/* Link the block into lsit. */
		pBlock->pNext = NULL;
		pppoeManager.pOutgLast->pNext = pBlock;
		pppoeManager.pOutgLast = pBlock;
		pppoeManager.nOutgSize++;
		__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
		bResult = TRUE;
	}

#if 0 /* Obsoleted. */
	/* Send message to PPP/PPPoE main thread to trigger sending. */
	msg.wCommand = PPPOE_MSG_SENDPACKET;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pBlock;
	bResult = SendMessage(pppoeManager.hMainThread, &msg);
#endif

__TERMINAL:
	if (!bResult)
	{
		/* Should release the parameter block object. */
		if (pBlock)
		{
			_hx_free(pBlock);
		}
		/* Free pbuf since we refered it. */
		pbuf_free(pb);
		LINK_STATS_INC(link.drop);
	}
	return bResult;
}

/* 
 * Handler of PPPOE_MSG_SENDPACKET message.
 * It gets parameter block objects from list,then
 * send the packet out by calling pppifOutput routine.
 * Parameter block object also be destroyed in
 * this routine.
 */
static void _SendPacketHandler(__PPP_SENDPACKET_BLOCK* pBlock)
{
	BUG_ON(NULL == pBlock);

	/* Send the packet out through PPP interface. */
	pppifOutput(pBlock->out_if, pBlock->pkt_buff, &pBlock->addr);

	/* Release pbuf object since we refered it. */
	pbuf_free(pBlock->pkt_buff);

	/* Destroy the parameter object. */
	_hx_free(pBlock);
}

/* 
 * Post a PPP over Ethernet frame to PPPoE main thread.
 * This routine is mainly called by EtherentManager,or
 * ethernet interface driver directly.
 */
static BOOL _PostFrame(__PPPOE_POSTFRAME_BLOCK* pBlock)
{
	__KERNEL_THREAD_MESSAGE msg;
	BOOL bResult = FALSE;
	DWORD dwFlags = 0;

	BUG_ON(NULL == pBlock);
	if (NULL == pppoeManager.hMainThread) /* Maybe not initialized yet. */
	{
		goto __TERMINAL;
	}

	/* 
	 * Hook the post frame block into incoming list, and send a message to
	 * PPPoE main thread if the list is empty.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
	if (pppoeManager.nIncomSize == 0) /* List is empty. */
	{
		BUG_ON(pppoeManager.pIncomFirst);
		BUG_ON(pppoeManager.pIncomLast);
		pBlock->pNext = NULL; /* Set as last. */
		pppoeManager.pIncomFirst = pppoeManager.pIncomLast = pBlock;
		pppoeManager.nIncomSize++;
		__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
		/* Send POST_FRAME message to PPPoE main thread. */
		msg.wCommand = PPPOE_MSG_POSTFRAME;
		msg.wParam = 0;
		msg.dwParam = 0;
		bResult = SendMessage(pppoeManager.hMainThread, &msg);
		if (!bResult)
		{
			__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			pppoeManager.pIncomFirst = pppoeManager.pIncomLast = NULL;
			pppoeManager.nIncomSize = 0;
			__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
	}
	else /* Incoming list is not empty. */
	{
		BUG_ON(NULL == pppoeManager.pIncomFirst);
		BUG_ON(NULL == pppoeManager.pIncomLast);
		if (pppoeManager.nIncomSize > PPPOE_MAX_PENDINGLIST_SIZE) /* List is full. */
		{
			__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
			bResult = FALSE;
			goto __TERMINAL;
		}
		pBlock->pNext = NULL;
		pppoeManager.pIncomLast->pNext = pBlock;
		pppoeManager.pIncomLast = pBlock;
		pppoeManager.nIncomSize++;
		__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
		bResult = TRUE;
	}

#if 0
	msg.wCommand = PPPOE_MSG_POSTFRAME;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pBlock;
	if (!SendMessage(pppoeManager.hMainThread, &msg))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
#endif

__TERMINAL:
	return bResult;
}

/* 
 * Handler of PPPOE_MSG_POSTFRAME.It unlinks and deliverys a ethernet
 * frame object into pppoe_disc_input or pppoe_data_input routine,
 * according the frame type value.
 */
static BOOL pppoePostFrameHandler(__PPPOE_MANAGER* pMgr, __PPPOE_POSTFRAME_BLOCK* pParam)
{
	__PPPOE_INSTANCE* pInstance = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	struct netif* netif = NULL;

	BUG_ON(NULL == pMgr);
	BUG_ON(NULL == pParam);

	pInstance = pParam->pInstance;
	pEthInt = pParam->pEthInt;
	BUG_ON(NULL == pParam->p);
	BUG_ON(NULL == pInstance);
	BUG_ON(NULL == pEthInt);

	/* Delivery the frame to PPPoE module. */
	netif = pEthInt->Proto_Interface[0].pL3Interface;

	/* Delivery to PPPoE module according frame type. */
	if (ETH_FRAME_TYPE_PPPOE_S == pParam->frame_type)
	{
		pppoe_disc_input(netif, pParam->p);
	}
	else if (ETH_FRAME_TYPE_PPPOE_D == pParam->frame_type)
	{
		pppoe_data_input(netif, pParam->p);
	}
	else
	{
		BUG();
	}

	/* Release the post frame parameter object. */
	_hx_free(pParam);
	return TRUE;
}

/* Main thread of PPPoE service. */
static DWORD pppoeMainThread(LPVOID* arg)
{
	__KERNEL_THREAD_MESSAGE msg;
	__network_timer_object* pTimerObject = NULL;
	__PPPOE_INSTANCE* pInstance = NULL;
	__PPP_SENDPACKET_BLOCK* pSendBlock = NULL;
	__PPPOE_POSTFRAME_BLOCK* pPostBlock = NULL;
	DWORD dwFlags;

	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				pTimerObject = (__network_timer_object*)msg.dwParam;
				BUG_ON(KERNEL_OBJECT_SIGNATURE != pTimerObject->dwObjectSignature);
				/* Call timer handler. */
				pTimerObject->handler(pTimerObject->handler_param);
				/* Destroy it. */
				_hx_release_network_tmo(pTimerObject);
				break;
			case PPPOE_MSG_STARTSESSION:
				pInstance = (__PPPOE_INSTANCE*)msg.dwParam;
				BUG_ON(NULL == pInstance);
				StartPPPoE(pInstance);
				break;
			case PPPOE_MSG_STOPSESSION:
				pInstance = (__PPPOE_INSTANCE*)msg.dwParam;
				BUG_ON(NULL == pInstance);
				StopPPPoE(pInstance);
				break;
			case PPPOE_MSG_SENDPACKET:
				/* Fetch all pending out going block from list and process it. */
				while (TRUE)
				{
					__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
					if (0 == pppoeManager.nOutgSize)
					{
						BUG_ON(pppoeManager.pOutgFirst);
						BUG_ON(pppoeManager.pOutgLast);
						__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
						break;
					}
					BUG_ON(NULL == pppoeManager.pOutgFirst);
					BUG_ON(NULL == pppoeManager.pOutgLast);
					pSendBlock = pppoeManager.pOutgFirst;
					pppoeManager.pOutgFirst = pSendBlock->pNext;
					pppoeManager.nOutgSize--;
					if (0 == pppoeManager.nOutgSize)
					{
						BUG_ON(pppoeManager.pOutgFirst);
						pppoeManager.pOutgLast = NULL;
					}
					__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
					/* Commit to send. */
					_SendPacketHandler(pSendBlock);
				}
				break;
			case PPPOE_MSG_POSTFRAME:
				/* Process all pending incoming frame(s). */
				while (TRUE)
				{
					__ENTER_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
					if (0 == pppoeManager.nIncomSize) /* List is empty. */
					{
						BUG_ON(pppoeManager.pIncomFirst);
						BUG_ON(pppoeManager.pIncomLast);
						__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
						break;
					}
					/* Fetch one pending block and process it. */
					BUG_ON(NULL == pppoeManager.pIncomFirst);
					BUG_ON(NULL == pppoeManager.pIncomLast);
					pPostBlock = pppoeManager.pIncomFirst;
					pppoeManager.pIncomFirst = pPostBlock->pNext;
					pppoeManager.nIncomSize--;
					if (0 == pppoeManager.nIncomSize)
					{
						BUG_ON(pppoeManager.pIncomFirst);
						pppoeManager.pIncomLast = NULL;
					}
					__LEAVE_CRITICAL_SECTION_SMP(pppoeManager.spin_lock, dwFlags);
					pppoePostFrameHandler(&pppoeManager, pPostBlock);
				}
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

/* Initializer of PPPoE manager object. */
static BOOL PPPoEInitialize(__PPPOE_MANAGER* pMgr)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pMgr);
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pMgr->spin_lock, "pppoe");
#endif
	pMgr->hMainThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		pppoeMainThread,
		NULL,
		NULL,
		PPPOE_MAIN_THREAD_NAME);
	if (NULL == pMgr->hMainThread)
	{
		goto __TERMINAL;
	}

	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/* Global PPPoE manager object. */
__PPPOE_MANAGER pppoeManager = {
	NULL,                               //pInstanceList.
	NULL,                               //pOutgFirst.
	NULL,                               //pOutgLast.
	0,                                  //nOutgSize.
	NULL,                               //pIncomFirst.
	NULL,                               //pIncomLast.
	0,                                  //nIncomSize.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,               //spin_lock.
#endif
	NULL,                               //hMainThread.

	PPPoEInitialize,                    //Initialize.
	CreatePPPoEInstance,                //CreatePPPoEInstance.
	StartPPPoE,                         //StartPPPoE.
	StartPPPoEByName,                   //StartPPPoEByName.

	/* Stop a PPPoE session. */
	StopPPPoE,                          //StopPPPoE.
	StopPPPoEByName,                    //StopPPPoEByName.

	/* Destroy a PPPoE instance. */
	DestroyPPPoEInstance,               //DestroyPPPoEInstance.
	DestroyPPPoEInstanceByName,         //DestroyPPPoEInstanceByName.

	/* Post frame to main thread. */
	_PostFrame,                         //PostFrame.

	pppSendPacket,                      //SendPacket.
};
