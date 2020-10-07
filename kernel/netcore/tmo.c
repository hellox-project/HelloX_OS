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

/*
 * NOTE:
 * The network timer mechanism is not thread safe,i.e,
 * the network timer object must be used in one thread,
 * since it's not protected by synchronous object.
 * But the management functions,such as create(timeout),
 * release, are thread safe.
 */

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

/* How many network timer object in system. */
static int timer_object_num = 0;

/* Mutex object to protect timer object list. */
static HANDLE timer_object_mutex = NULL;

/* 
 * Release a network timer object, cancel the kernel timer
 * object and unlink it from global list.
 */
void _hx_release_network_tmo(__network_timer_object* pTimerObject)
{
	BUG_ON(NULL == pTimerObject);
	BUG_ON(NULL == timer_object_mutex);

	/* 
	 * Validate the timer object, since it maybe 
	 * destroyed already in some race condition. 
	 */
	WaitForThisObject(timer_object_mutex);
	if (!_hx_sys_validate_timer(pTimerObject))
	{
		ReleaseMutex(timer_object_mutex);
		__LOG("[%s]timer[0x%X] maybe destroyed already.\r\n", 
			pTimerObject, __func__);
		return;
	}
	BUG_ON(pTimerObject->dwObjectSignature != KERNEL_OBJECT_SIGNATURE);
	BUG_ON(NULL == pTimerObject->hTimer);
	
	/* 
	 * Cancel the HelloX timer first. 
	 * The timer is not auto delete,so it must
	 * be success when cancel it.
	 */
	BUG_ON(!CancelTimer(pTimerObject->hTimer));
	pTimerObject->pPrev->pNext = pTimerObject->pNext;
	pTimerObject->pNext->pPrev = pTimerObject->pPrev;
	/* Clear object signature. */
	pTimerObject->dwObjectSignature = 0;
	/* decrease timer object number. */
	timer_object_num--;
	BUG_ON(timer_object_num < 0);
	_hx_free(pTimerObject);

	ReleaseMutex(timer_object_mutex);
}

/* Set a network timer object. */
void _hx_sys_timeout(HANDLE hTarget, unsigned long msecs, sys_timeout_handler handler, void* arg)
{
	HANDLE hTimer = NULL;
	__network_timer_object* pTimerObject = NULL;

	/* Create mutex object if not yet. */
	if (NULL == timer_object_mutex)
	{
		timer_object_mutex = CreateMutex();
		if (NULL == timer_object_mutex)
		{
			__LOG("[%s]faild to create mutex.\r\n", __func__);
			return;
		}
	}
	/* Get mutex to protect the global list. */
	WaitForThisObject(timer_object_mutex);

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
	/* Record to system. */
	timer_object_num++;

	/* 
	 * Set a HelloX timer object. 
	 * The timer is one shot,but should not be
	 * deleted automatically by kernel,which
	 * may lead race condition.
	 */
	hTimer = System.SetTimer(
		(__COMMON_OBJECT*)&System,
		(__KERNEL_THREAD_OBJECT*)hTarget,
		(DWORD)pTimerObject, msecs, NULL, NULL,
		TIMER_FLAGS_ONCE | TIMER_FLAGS_NOAUTODEL);
	if (NULL == hTimer)
	{
		_hx_printf("%s:failed to set timer.\r\n", __func__);
		goto __TERMINAL;
	}
	pTimerObject->hTimer = hTimer;

__TERMINAL:
	if (NULL == hTimer) /* Failed to set timer. */
	{
		if (pTimerObject)
		{
			/* network timer created,destroy it. */
			_hx_free(pTimerObject);
		}
	}
	ReleaseMutex(timer_object_mutex);
	return;
}

/* Cancel a network timer object. */
void _hx_sys_untimeout(sys_timeout_handler handler, void* arg, int code_line)
{
	__network_timer_object* pTimerObject = NULL;
	
	WaitForThisObject(timer_object_mutex);
	pTimerObject = timer_list.pNext;

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
		ReleaseMutex(timer_object_mutex);
		__LOG("[%s/%d]:timer is not exist.\r\n", __func__, code_line);
		return;
	}
	
	/* Release the network timer object. */
	_hx_release_network_tmo(pTimerObject);
	ReleaseMutex(timer_object_mutex);
}

/*
 * Check if a given network timer object is valid,i.e,
 * this object is in network timer list.
 * In some race condition, the network timer maybe released
 * in advance before it's handler is invoked,so the hander
 * must verify if the network timer object exist.
 */
BOOL _hx_sys_validate_timer(__network_timer_object* pTimerObject)
{
	__network_timer_object* temp_timer_obj = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pTimerObject);
	/* Check signature first. */
	if (KERNEL_OBJECT_SIGNATURE != pTimerObject->dwObjectSignature)
	{
		return bResult;
	}

	WaitForThisObject(timer_object_mutex);
	temp_timer_obj = timer_list.pNext;
	while (temp_timer_obj != &timer_list)
	{
		if (pTimerObject == temp_timer_obj)
		{
			/* network timer in list. */
			bResult = TRUE;
			break;
		}
		temp_timer_obj = temp_timer_obj->pNext;
	}
	ReleaseMutex(timer_object_mutex);
	return bResult;
}
