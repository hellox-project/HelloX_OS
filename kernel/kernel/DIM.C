//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,25 2005
//    Module Name               : dim.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of Device
//                                Input Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include "ktmgr.h"
#include "commobj.h"
#include "dim.h"
#include "types.h"
#include "hellocn.h"

/* Initializer of DIM object. */
static BOOL DimInitialize(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFocusThread,
	__COMMON_OBJECT* lpShellThread)
{
	__DEVICE_INPUT_MANAGER* lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	BUG_ON(NULL == lpInputMgr);

	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;

	return TRUE;
}

/*
 * Send a device message to a specified kernel thread,
 * or to the default shell thread
 * if target thread is not specified.
 */
static DWORD SendDeviceMessage(__COMMON_OBJECT* lpThis,
	__DEVICE_MESSAGE* lpDevMsg,
	__COMMON_OBJECT* lpTarget)
{
	__DEVICE_INPUT_MANAGER* lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_MESSAGE* lpThreadMsg = (__KERNEL_THREAD_MESSAGE*)lpDevMsg;
	__KERNEL_THREAD_OBJECT* pMsgTarget = NULL;
	unsigned long dwFlags = 0;

	BUG_ON((NULL == lpInputMgr) || (NULL == lpThreadMsg));

	/* 
	 * Send device input message to target kernel 
	 * thread if specified. 
	 */
	if(lpTarget != NULL)
	{
		KernelThreadManager.SendMessage(lpTarget,lpThreadMsg);
		return DEVICE_MANAGER_SUCCESS;
	}

	/* 
	 * Send message to current focus or shell thread. 
	 * We get the target thread in critical section and
	 * send the message out of critical section,since 
	 * the SendMessage routine will lead reschedule.
	 */
	__ENTER_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
	if(lpInputMgr->lpFocusKernelThread != NULL)
	{
		/* 
		 * DIM contains a valid current focus thread object. 
		 * But device input message can not be sent to it if
		 * it's a terminaled kernel thread.
		 */
		if(KERNEL_THREAD_STATUS_TERMINAL == lpInputMgr->lpFocusKernelThread->dwThreadStatus)
		{
			/* Clear the current focus thread since it's status is TERMINAL. */
			lpInputMgr->lpFocusKernelThread = NULL;
			/* Send to the default shell thread if exist. */
			if(NULL != lpInputMgr->lpShellKernelThread)
			{
				pMsgTarget = lpInputMgr->lpShellKernelThread;
				__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
				goto __TERMINAL;
			}
			else
			{
				/* The current shell kernel thread is not exists. */
				__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
				goto __TERMINAL;
			}
		}
		else
		{
			/* Just send it to current focus kernel thread. */
			pMsgTarget = lpInputMgr->lpFocusKernelThread;
			__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
	}
	else
	{
		/* 
		 * The current focus kernel thread is not exists,
		 * send to default shell thread. 
		 */
		if(NULL != lpInputMgr->lpShellKernelThread)
		{
			pMsgTarget = lpInputMgr->lpShellKernelThread;
			__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		else
		{
			/* No kernel thread is available to receive device message. */
			__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
	}
__TERMINAL:
	if (pMsgTarget)
	{
		KernelThreadManager.SendMessage(
			(__COMMON_OBJECT*)pMsgTarget,
			lpThreadMsg);
		return DEVICE_MANAGER_SUCCESS;
	}
	return DEVICE_MANAGER_FAILED;
}

/* Set the current focus thread. */
static __COMMON_OBJECT* SetFocusThread(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFocusThread)
{
	__DEVICE_INPUT_MANAGER* lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT* lpRetVal = NULL;
	unsigned long dwFlags = 0;
	
	BUG_ON(NULL == lpInputMgr);

	__ENTER_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
	lpRetVal = lpInputMgr->lpFocusKernelThread;
	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}

/*
 * Set the default shell thread.Any device input message will 
 * be sent to this thread if no current focus thread exist.
 */
static __COMMON_OBJECT* SetShellThread(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpShellThread)
{
	__DEVICE_INPUT_MANAGER* lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT* lpRetVal = NULL;
	unsigned long dwFlags = 0;
	
	BUG_ON(NULL == lpInputMgr);

	__ENTER_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);
	lpRetVal = lpInputMgr->lpShellKernelThread;
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;
	__LEAVE_CRITICAL_SECTION_SMP(lpInputMgr->spin_lock, dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}

/* Globa device input manager object. */
__DEVICE_INPUT_MANAGER DeviceInputManager = {
	NULL,                   //lpFocusKernelThread.
	NULL,                   //lpShellKernelThread.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,
#endif
	SendDeviceMessage,      //SendDeviceMessage routine.
	SetFocusThread,         //SetFocusThread routine.
	SetShellThread,         //SetShellThread routine.
	DimInitialize           //Initialize routine.
};
