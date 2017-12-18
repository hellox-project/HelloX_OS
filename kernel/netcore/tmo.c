//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 22,2017
//    Module Name               : tmo.c
//    Module Funciton           : 
//                                Timeout mechanism of network module.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "netcfg.h"
#include "ethmgr.h"
#include "tmo.h"

/* Global network timer object list. */
static __network_timer_object timer_list = {
	NULL,           //hTimer.
	NULL,           //handler.
	NULL,           //handler_param.
	0,              //msecs.
	KERNEL_OBJECT_SIGNATURE,    //dwObjectSignature.
	&timer_list,    //pNext.
	&timer_list,    //pPrev.
};

/* 
 * Release a network timer object,and unlink it from global list.
 * This routine only release the network timer object,does not
 * check the status of hTimer,even if the HelloX timer object(hTimer)
 * is not destroyed.
 * So please call this routine after make sure the hTimer is 
 * destroyed.
 */
void _hx_release_network_tmo(__network_timer_object* pTimerObject)
{
	BUG_ON(NULL == pTimerObject);
	if (pTimerObject->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return;
	}
	pTimerObject->pPrev->pNext = pTimerObject->pNext;
	pTimerObject->pNext->pPrev = pTimerObject->pPrev;
	/* Clear object signature. */
	pTimerObject->dwObjectSignature = 0;
	_hx_free(pTimerObject);
}

/* Set a network timer object. */
void _hx_sys_timeout(HANDLE hTarget, unsigned long msecs, sys_timeout_handler handler, void* arg)
{
	HANDLE hTimer = NULL;
	__network_timer_object* pTimerObject = NULL;

	pTimerObject = (__network_timer_object*)_hx_malloc(sizeof(__network_timer_object));
	if (NULL == pTimerObject)
	{
		goto __TERMINAL;
	}

	/* 
	 * Initialize the network timer object and 
	 * insert it into global list.
	 */
	pTimerObject->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	pTimerObject->handler = handler;
	pTimerObject->handler_param = arg;
	pTimerObject->msecs = msecs;

	pTimerObject->pNext = timer_list.pNext;
	pTimerObject->pPrev = &timer_list;
	timer_list.pNext->pPrev = pTimerObject;
	timer_list.pNext = pTimerObject;

	/* Set a HelloX timer object. */
	//hTimer = SetTimer((DWORD)pTimerObject, msecs, NULL, NULL,
	//	TIMER_FLAGS_ONCE);
	hTimer = System.SetTimer(
		(__COMMON_OBJECT*)&System,
		(__KERNEL_THREAD_OBJECT*)hTarget,
		(DWORD)pTimerObject, msecs, NULL, NULL,
		TIMER_FLAGS_ONCE);
	if (NULL == hTimer)
	{
		_hx_printf("%s:failed to set timer.\r\n", __func__);
		goto __TERMINAL;
	}
	pTimerObject->hTimer = hTimer;

__TERMINAL:
	if (NULL == hTimer) /* Failed to set timer. */
	{
		_hx_release_network_tmo(pTimerObject);
	}
	return;
}

/* Cancel a network timer object. */
void _hx_sys_untimeout(sys_timeout_handler handler, void* arg)
{
	__network_timer_object* pTimerObject = timer_list.pNext;
	BOOL bResult = FALSE;

	while (pTimerObject != &timer_list)
	{
		if ((pTimerObject->handler == handler) && (pTimerObject->handler_param == arg))
		{
			break;
		}
		pTimerObject = pTimerObject->pNext;
	}
	if (pTimerObject == &timer_list)
	{
		//_hx_printf("%s:can not find the timer to delete.\r\n", __func__);
		return;
	}
	/* Cancel the HelloX timer first. */
	bResult = CancelTimer(pTimerObject->hTimer);
	if (!bResult) /* The timer maybe triggered and cancelled before. */
	{
		return;
	}
	
	/* Release the network timer object. */
	_hx_release_network_tmo(pTimerObject);
}
