//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2006
//    Module Name               : KTMGR2.CPP
//    Module Funciton           : 
//                                This module countains kernel thread and kernel thread 
//                                manager's implementation code.
//                                This file is the second part of KTMGR.CPP,in order
//                                to reduce one file's size.
//                                The global routines in this file can only be refered
//                                by KTMGR.CPP file.
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Sep,30,2006
//    Last modified Content     :
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "ktmgr2.h"
#include "hellocn.h"
#include "kapi.h"


//A helper routine,to check if a given kernel thread should be suspended.
//It will be invoked by GetReadyKernelThread.
static BOOL ShouldSuspend(__KERNEL_THREAD_OBJECT* lpKernelThread)
{
	if(lpKernelThread->dwSuspendFlags & SUSPEND_FLAG_DISABLE)
	{
		return FALSE;
	}
	if(0 == (lpKernelThread->dwSuspendFlags & ~SUSPEND_FLAG_MASK))
	{
		return FALSE;
	}
	return TRUE;
}

//
//This routine tris to get a schedulable kernel thread from ready queue,
//the target kernel thread's priority must larger or equal dwPriority.
//If can not find,returns NULL.
//
__KERNEL_THREAD_OBJECT* GetScheduleKernelThread(__COMMON_OBJECT* lpThis,
												DWORD dwPriority)
{
	__KERNEL_THREAD_OBJECT*   lpKernel = NULL;
	__KERNEL_THREAD_MANAGER*  lpMgr    = (__KERNEL_THREAD_MANAGER*)lpThis;
	__PRIORITY_QUEUE*         lpQueue  = NULL;
	DWORD                     i        = 0;

	if((NULL == lpThis) || (dwPriority > MAX_KERNEL_THREAD_PRIORITY)) //Invalid parameters.
	{
		return NULL;
	}
	
	//Search a kernel thread from ready queue.
	for(i = dwPriority;i < MAX_KERNEL_THREAD_PRIORITY + 1;i ++)
	{
		lpQueue  = lpMgr->ReadyQueue[MAX_KERNEL_THREAD_PRIORITY - i + dwPriority];
		lpKernel = (__KERNEL_THREAD_OBJECT*)lpQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpQueue,
			NULL);
		if(lpKernel)  //Found one successfully.
		{
			if(ShouldSuspend(lpKernel))
			{
				//Suspend the kernel thread.
				lpKernel->dwThreadStatus = KERNEL_THREAD_STATUS_SUSPENDED;
				lpMgr->lpSuspendedQueue->InsertIntoQueue(
					(__COMMON_OBJECT*)lpMgr->lpSuspendedQueue,
					(__COMMON_OBJECT*)lpKernel,
					lpKernel->dwThreadPriority);
				continue;
			}
			return lpKernel;
		}
	}
	return lpKernel;  //If reach here,it means fail.
}

//
//Add a kernel thread whose status is READY to ready queue.
//The kernel thread's priority acts as index to locate the queue element
//in ready queue array.
//
VOID AddReadyKernelThread(__COMMON_OBJECT* lpThis,
						  __KERNEL_THREAD_OBJECT* lpKernelThread)
{
	__PRIORITY_QUEUE* lpQueue = NULL;

	if((NULL == lpThis) || (NULL == lpKernelThread)) //Invalid parameters.
	{
		BUG();
		return;
	}

	if((lpKernelThread->dwThreadPriority > MAX_KERNEL_THREAD_PRIORITY) ||
	   (lpKernelThread->dwThreadStatus != KERNEL_THREAD_STATUS_READY))
	{
		BUG();
		return;
	}

	lpQueue = ((__KERNEL_THREAD_MANAGER*)lpThis)->ReadyQueue[
		lpKernelThread->dwThreadPriority];

	lpQueue->InsertIntoQueue((__COMMON_OBJECT*)lpQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	return;
}

//
//SetThreadHook routine,this routine sets appropriate hook routine
//according to dwHookType, and returns the old one.
//
__THREAD_HOOK_ROUTINE SetThreadHook(DWORD dwHookType,
									__THREAD_HOOK_ROUTINE lpRoutine)
{
	__THREAD_HOOK_ROUTINE lpOldRoutine = NULL;
	DWORD                 dwFlags;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	switch(dwHookType)
	{
	case THREAD_HOOK_TYPE_CREATE:
		lpOldRoutine = KernelThreadManager.lpCreateHook;
		KernelThreadManager.lpCreateHook = lpRoutine;
		break;
	case THREAD_HOOK_TYPE_ENDSCHEDULE:
		lpOldRoutine = KernelThreadManager.lpEndScheduleHook;
		KernelThreadManager.lpEndScheduleHook = lpRoutine;
		break;
	case THREAD_HOOK_TYPE_BEGINSCHEDULE:
		lpOldRoutine = KernelThreadManager.lpBeginScheduleHook;
		KernelThreadManager.lpBeginScheduleHook = lpRoutine;
		break;
	case THREAD_HOOK_TYPE_TERMINAL:
		lpOldRoutine = KernelThreadManager.lpTerminalHook;
		KernelThreadManager.lpTerminalHook = lpRoutine;
		break;
	default:  //Should not reach here.
		BUG();
		break;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return lpOldRoutine;
}

//
//CallThreadHook,this routine calls proper hook routine according
//to the dwHookType value.
//
VOID CallThreadHook(DWORD dwHookType,
					__KERNEL_THREAD_OBJECT* lpPrev,
					__KERNEL_THREAD_OBJECT* lpNext)
{
	if(dwHookType & THREAD_HOOK_TYPE_CREATE)  //Should call create hook.
	{
		if(NULL == lpPrev)
		{
			BUG(); //Should not occur.
			return;
		}
		if(NULL == KernelThreadManager.lpCreateHook)  //Not set yet.
		{
			return;
		}
		//Call the create hook routine.
		KernelThreadManager.lpCreateHook(lpPrev,&lpPrev->dwUserData);
	}
	if(dwHookType & THREAD_HOOK_TYPE_ENDSCHEDULE) //Should call end hook.
	{
		if(NULL == lpPrev)
		{
			BUG();
			return;
		}
		if(NULL == KernelThreadManager.lpEndScheduleHook)
		{
			return;
		}
		//Call end schedule hook now.
		KernelThreadManager.lpEndScheduleHook(lpPrev,&lpPrev->dwUserData);
	}
	if(dwHookType & THREAD_HOOK_TYPE_BEGINSCHEDULE) //Should call begin hook.
	{
		if(NULL == lpNext)
		{
			BUG();
			return;
		}
		if(NULL == KernelThreadManager.lpBeginScheduleHook)
		{
			return;
		}
		//Call begin schedule hook now.
		KernelThreadManager.lpBeginScheduleHook(lpNext,&lpNext->dwUserData);
	}
	if(dwHookType & THREAD_HOOK_TYPE_TERMINAL) //Should call terminal hook.
	{
		if(NULL == lpPrev)
		{
			BUG();
			return;
		}
		if(NULL == KernelThreadManager.lpTerminalHook)
		{
			return;
		}
		//Call terminal hook now.
		KernelThreadManager.lpTerminalHook(lpPrev,&lpPrev->dwUserData);
	}
}

//
//KernelThreadWrapper routine.
//The routine is all kernel thread's entry porint.
//The routine does the following:
// 1. Calles the kernel thread's start routine;
// 2. When the start routine is over,put the kernel thread object into terminal queue;
// 3. Wakeup all kernel thread(s) waiting for this kernel thread object.
// 4. Reschedule all kernel thread(s).
//This routine will never return.
//
VOID KernelThreadWrapper(__COMMON_OBJECT* lpKThread)
{
	__KERNEL_THREAD_OBJECT*        lpKernelThread      = (__KERNEL_THREAD_OBJECT*)lpKThread;
	__KERNEL_THREAD_OBJECT*        lpWaitingThread     = NULL;
	__PRIORITY_QUEUE*              lpWaitingQueue      = NULL;
	DWORD                          dwRetValue          = 0;
	DWORD                          dwFlags             = 0;

	//lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpKThread;
	//Execute user defined kernel thread function.
	dwRetValue = lpKernelThread->KernelThreadRoutine(lpKernelThread->lpRoutineParam);

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpKernelThread->dwReturnValue    = dwRetValue;      //Set the return value of this thread.
	lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_TERMINAL;  //Change the status.
	//Insert the current kernel thread object into TERMINAL queue.
	KernelThreadManager.lpTerminalQueue->InsertIntoQueue((__COMMON_OBJECT*)KernelThreadManager.lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	//
	//The following code wakeup all kernel thread(s) who waiting for this kernel thread 
	//object.
	//
	lpWaitingQueue  = lpKernelThread->lpWaitingQueue;
	lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while(lpWaitingThread)
	{
		lpWaitingThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		lpWaitingThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpWaitingThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpWaitingThread);  //Add to ready queue.
		lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule kernel thread.
	return;        //***** CAUTION! ***** : This instruction will never reach.
}

//Half bottom part of a kernel thread,responds for the cleaning of a kernel thread's
//context.
//This routine is used for supporting the implementation of TerminateKernelThread,which
//in furthar supports the implementations of exit() and abort() C lib routine.
VOID KernelThreadClean(__COMMON_OBJECT* lpKThread,DWORD dwExitCode)
{
	__KERNEL_THREAD_OBJECT*        lpKernelThread      = (__KERNEL_THREAD_OBJECT*)lpKThread;
	__KERNEL_THREAD_OBJECT*        lpWaitingThread     = NULL;
	__PRIORITY_QUEUE*              lpWaitingQueue      = NULL;
	DWORD                          dwFlags             = 0;

	if(NULL == lpKernelThread)
	{
		return;
	}

	//Set the exit code.
	lpKernelThread->dwReturnValue = dwExitCode;

	//Begin clean the execution context of the specified kernel thread.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_TERMINAL;  //Change the status.
	//Insert the current kernel thread object into TERMINAL queue.
	KernelThreadManager.lpTerminalQueue->InsertIntoQueue((__COMMON_OBJECT*)KernelThreadManager.lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	//
	//The following code wakeup all kernel thread(s) who waiting for this kernel thread 
	//object.
	//
	lpWaitingQueue  = lpKernelThread->lpWaitingQueue;
	lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while(lpWaitingThread)
	{
		lpWaitingThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		lpWaitingThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpWaitingThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpWaitingThread);  //Add to ready queue.
		lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule kernel thread.
	return;
}

//
//The implementation of WaitForKernelThreadObject,because this routine calls ScheduleFromproc,
//so we implement it here(After the implementation of ScheduleFromProc).
//The routine does the following:
// 1. Check the current status of the kernel thread object;
// 2. If the current status is not KERNEL_THREAD_STATUS_TERMINAL,then block the
//    current kernel thread(who want to wait),put it into the object's waiting queue;
// 3. Call ScheduleFromProc to fetch next kernel thread whose status is READY to run.
//
DWORD WaitForKernelThreadObject(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT*           lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThis;
	__KERNEL_THREAD_OBJECT*           lpCurrent      = NULL;
	__PRIORITY_QUEUE*                 lpWaitingQueue = NULL;
	DWORD                             dwFlags        = 0;
	
	if(NULL == lpKernelThread)    //Parameter check.
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(KERNEL_THREAD_STATUS_TERMINAL == lpKernelThread->dwThreadStatus)  //If the object's
		                                                                 //status is TERMINAL,
																		 //the wait operation
																		 //will secussfully.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}

	//
	//If the waited object's status is not TERMINAL,then the waiting operation will
	//not secussful,the current kernel thread who want to wait will be blocked.
	//
	lpWaitingQueue = lpKernelThread->lpWaitingQueue;
	lpCurrent = KernelThreadManager.lpCurrentKernelThread;
	lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;

	lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpWaitingQueue,
		(__COMMON_OBJECT*)lpCurrent,
		0);    //Insert into the current kernel thread into waiting queue.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	KernelThreadManager.ScheduleFromProc(NULL);
	return 0;
}
