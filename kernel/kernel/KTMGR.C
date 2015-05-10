//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : ktmgr.cpp
//    Module Funciton           : 
//                                This module countains kernel thread and kernel thread 
//                                manager's implementation code.
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Mar 22,2009
//    Last modified Content     :
//                                1. Omit the BUG() line in 574 line.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "stdio.h"

//
//Pre-declare for extern global routines,these routines may
//be implemented in KTMGRx.CPP file,where x is 2,3,etc.
//
extern __THREAD_HOOK_ROUTINE SetThreadHook(DWORD dwHookType,
										   __THREAD_HOOK_ROUTINE lpRoutine);
extern VOID                  CallThreadHook(DWORD dwHookType,
											__KERNEL_THREAD_OBJECT* lpPrev,
											__KERNEL_THREAD_OBJECT* lpNext);
extern __KERNEL_THREAD_OBJECT* GetScheduleKernelThread(__COMMON_OBJECT* lpThis,
													   DWORD dwPriority);
extern VOID AddReadyKernelThread(__COMMON_OBJECT* lpThis,
								 __KERNEL_THREAD_OBJECT* lpKernelThread);
extern VOID KernelThreadWrapper(__COMMON_OBJECT*);
extern VOID KernelThreadClean(__COMMON_OBJECT*,DWORD);
extern DWORD WaitForKernelThreadObject(__COMMON_OBJECT* lpThis);

//
//Static global varaibles.
//

CHAR* lpszCriticalMsg = "CRITICAL ERROR : The internal data structure is not consecutive,\
                         please restart the system!!";

//
//The initialization routine of Kernel Thread Object.
//In the current implementation of Hello China,we do the following task:
// 1. Create the waiting queue object of the kernel thread object;
// 2. Initialize the waiting queue object;
// 3. Set appropriaty value of the member functions,such as WaitForThisObject.
//
BOOL KernelThreadInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                    bResult           = FALSE;
	__PRIORITY_QUEUE*       lpWaitingQueue    = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread    = NULL;
	__PRIORITY_QUEUE*       lpMsgWaitingQueue = NULL;
	int                     i;

	if(NULL == lpThis)  //Parameter check.
		goto __TERMINAL;

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThis;
	lpWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpWaitingQueue)    //Failed to create the waiting queue object.
		goto __TERMINAL;

	if(!lpWaitingQueue->Initialize((__COMMON_OBJECT*)lpWaitingQueue))
	    //Failed to initialize the waiting queue object.
		goto __TERMINAL;
	lpMsgWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpMsgWaitingQueue)  //Can not create message waiting queue.
		goto __TERMINAL;
	if(!lpMsgWaitingQueue->Initialize((__COMMON_OBJECT*)lpMsgWaitingQueue))
		goto __TERMINAL;

	lpKernelThread->lpWaitingQueue    = lpWaitingQueue;
	lpKernelThread->lpMsgWaitingQueue = lpMsgWaitingQueue;
	lpKernelThread->WaitForThisObject = WaitForKernelThreadObject;
	lpKernelThread->lpKernelThreadContext = NULL;

	//Initialize the multiple object waiting related variables.
	lpKernelThread->dwObjectSignature   = KERNEL_OBJECT_SIGNATURE;
	lpKernelThread->dwMultipleWaitFlags = 0;
	for(i = 0;i < MAX_MULTIPLE_WAIT_NUM;i ++)
	{
		lpKernelThread->MultipleWaitObjectArray[i] = NULL;
	}

	bResult = TRUE;

__TERMINAL:
	if(!bResult)        //Initialize failed.
	{
		if(lpWaitingQueue != NULL)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpWaitingQueue);
		if(lpMsgWaitingQueue != NULL)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpMsgWaitingQueue);
	}
	return bResult;
}

//
//The Uninitialize routine of kernel thread object.
//
VOID KernelThreadUninitialize(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT*   lpKernelThread = NULL;
	//__PRIORITY_QUEUE*         lpWaitingQueue = NULL;
	//__EVENT*                  lpMsgEvent     = NULL;

	if(NULL == lpThis)    //Parameter check.
	{
		return;
	}

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThis;

	//Reset signature first.
	lpKernelThread->dwObjectSignature = 0;

	//Destroy waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpKernelThread->lpWaitingQueue);

	//Destroy message waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue);

	return;
}

//
//The implementation of Kernel Thread Manager.
//

//Initializing routine of Kernel Thread Manager.
static BOOL KernelThreadMgrInit(__COMMON_OBJECT* lpThis)
{
	BOOL                       bResult          = FALSE;
	__KERNEL_THREAD_MANAGER*   lpMgr            = NULL;
	__PRIORITY_QUEUE*          lpRunningQueue   = NULL;
	__PRIORITY_QUEUE*          lpReadyQueue     = NULL;
	__PRIORITY_QUEUE*          lpSuspendedQueue = NULL;
	__PRIORITY_QUEUE*          lpSleepingQueue  = NULL;
	__PRIORITY_QUEUE*          lpTerminalQueue  = NULL;
	DWORD i;

	if(NULL == lpThis)
	{
		return bResult;
	}
	lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;

	//
	//The following code creates all objects required by Kernel Thread Manager.
	//If any error occurs,the initializing process is terminaled.
	//
	lpRunningQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpRunningQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpRunningQueue->Initialize((__COMMON_OBJECT*)lpRunningQueue))
	{
		goto __TERMINAL;
	}

	lpReadyQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpReadyQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpReadyQueue->Initialize((__COMMON_OBJECT*)lpReadyQueue))
	{
		goto __TERMINAL;
	}

	lpSuspendedQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSuspendedQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpSuspendedQueue->Initialize((__COMMON_OBJECT*)lpSuspendedQueue))
	{
		goto __TERMINAL;
	}

	lpSleepingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSleepingQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpSleepingQueue->Initialize((__COMMON_OBJECT*)lpSleepingQueue))
	{
		goto __TERMINAL;
	}

	lpTerminalQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpTerminalQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpTerminalQueue->Initialize((__COMMON_OBJECT*)lpTerminalQueue))
	{
		goto __TERMINAL;
	}


	//
	//Now,the objects required by Kernel Thread Manager are created and initialized success-
	//fully,initialize the kernel thread manager itself now.
	//
	lpMgr->lpRunningQueue    = lpRunningQueue;
	//lpMgr->lpReadyQueue      = lpReadyQueue;
	lpMgr->lpSuspendedQueue  = lpSuspendedQueue;
	lpMgr->lpSleepingQueue   = lpSleepingQueue;
	lpMgr->lpTerminalQueue   = lpTerminalQueue;

	//Initializes the ready queue array.Any element failure can cause the whole
	//process to fail.
	for(i = 0;i < MAX_KERNEL_THREAD_PRIORITY + 1;i ++)
	{
		lpReadyQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
			NULL,
			OBJECT_TYPE_PRIORITY_QUEUE);
		if(NULL == lpReadyQueue)
		{
			goto __TERMINAL;
		}
		if(!lpReadyQueue->Initialize((__COMMON_OBJECT*)lpReadyQueue))  //Can not initialize it.
		{
			goto __TERMINAL;
		}
		lpMgr->ReadyQueue[i] = lpReadyQueue;
	}

	lpMgr->lpCurrentKernelThread = NULL;

	bResult = TRUE;

__TERMINAL:
	if(!bResult)  //If failed to initialize the Kernel Thread Manager.
	{
		if(NULL != lpRunningQueue)  //Destroy the objects created just now.
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpRunningQueue);
		if(NULL != lpReadyQueue)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpReadyQueue);
		if(NULL != lpSuspendedQueue)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpSuspendedQueue);
		if(NULL != lpSleepingQueue)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpSleepingQueue);
		if(NULL != lpTerminalQueue)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpTerminalQueue);
	}

	return bResult;
}

//
//CreateKernelThread's implementation.
//This routine do the following:
// 1. Create a kernel thread object by calling CreateObject;
// 2. Initializes the kernel thread object;
// 3. Create the kernel thread's stack by calling KMemAlloc;
// 4. Insert the kernel thread object into proper queue.
//
static __KERNEL_THREAD_OBJECT* CreateKernelThread(__COMMON_OBJECT*             lpThis,
												  DWORD                        dwStackSize,
												  DWORD                        dwStatus,
												  DWORD                        dwPriority,
												  __KERNEL_THREAD_ROUTINE      lpStartRoutine,
												  LPVOID                       lpRoutineParam,
												  LPVOID                       lpOwnerProcess,
												  LPSTR                        lpszName)
{
	__KERNEL_THREAD_OBJECT*             lpKernelThread      = NULL;
	__KERNEL_THREAD_MANAGER*            lpMgr               = NULL;
	LPVOID                              lpStack             = NULL;
	BOOL                                bSuccess            = FALSE;
	DWORD                               i                   = 0;

	if((NULL == lpThis) || (NULL == lpStartRoutine))    //Parameter check.
	{
		goto __TERMINAL;
	}

	if((KERNEL_THREAD_STATUS_READY != dwStatus) &&      //The initation status of a kernel
		                                                //thread should only be READY or
														//SUSPENDED.If the initation status
														//is READY,then the kernel thread maybe
														//scheduled to run in the NEXT schedule
														//circle(please note the kernel thread
														//does not be scheduled immediately),
														//else,the kernel thread will be susp-
														//ended,the kernel thread in this status
														//can be activated by ResumeKernelThread
														//calls.
	   (KERNEL_THREAD_STATUS_SUSPENDED != dwStatus))
	    goto __TERMINAL;

	lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_KERNEL_THREAD);

	if(NULL == lpKernelThread)    //If failed to create the kernel thread object.
		goto __TERMINAL;

	if(!lpKernelThread->Initialize((__COMMON_OBJECT*)lpKernelThread))    //Failed to initialize.
	{
		goto __TERMINAL;
	}

	if(0 == dwStackSize)          //If the dwStackSize is zero,then allocate the default size's
		                          //stack.
	{
		dwStackSize = DEFAULT_STACK_SIZE;
	}
	else
	{
		if(dwStackSize < MIN_STACK_SIZE)    //If dwStackSize is too small.
		{
			dwStackSize = MIN_STACK_SIZE;
		}
	}

	lpStack = KMemAlloc(dwStackSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == lpStack)    //Failed to create kernel thread stack.
	{
		goto __TERMINAL;
	}

	//The following code initializes the kernel thread object created just now.
	lpKernelThread->dwThreadID            = lpKernelThread->dwObjectID;
	lpKernelThread->dwThreadStatus        = dwStatus;
	lpKernelThread->dwThreadPriority      = dwPriority;
	lpKernelThread->dwScheduleCounter     = dwPriority;  //***** CAUTION!!! *****
	lpKernelThread->dwReturnValue         = 0;
	lpKernelThread->dwTotalRunTime        = 0;
	lpKernelThread->dwTotalMemSize        = 0;

	lpKernelThread->bUsedMath             = FALSE;      //May be updated in the future.
	lpKernelThread->dwStackSize           = dwStackSize ? dwStackSize : DEFAULT_STACK_SIZE;
	lpKernelThread->lpInitStackPointer    = (LPVOID)((DWORD)lpStack + dwStackSize);
	lpKernelThread->KernelThreadRoutine   = lpStartRoutine;       //Will be updated.
	lpKernelThread->lpRoutineParam        = lpRoutineParam;

	lpKernelThread->ucMsgQueueHeader      = 0;
	lpKernelThread->ucMsgQueueTrial       = 0;
	lpKernelThread->ucCurrentMsgNum       = 0;

	lpKernelThread->dwLastError           = 0;
	lpKernelThread->dwWaitingStatus       = OBJECT_WAIT_WAITING;
	lpKernelThread->dwSuspendFlags        = 0;

	//Copy kernel thread name.
	i = 0;
	if(lpszName)
	{
		for(i = 0;i < MAX_THREAD_NAME - 1;i ++)
		{
			if(lpszName[i] == 0)  //End.
			{
				break;
			}
		lpKernelThread->KernelThreadName[i] = lpszName[i];
		}
	}
	lpKernelThread->KernelThreadName[i] = 0;  //Set string's terminator.

	//
	//The following routine initializes the hardware context
	//of the kernel thread.
	//It's implementation depends on the hardware platform,so
	//this routine is implemented in ARCH directory.
	//
	InitKernelThreadContext(lpKernelThread,KernelThreadWrapper);

	//Link the kernel thread to it's owner process.
	lpKernelThread->lpOwnProcess = (__COMMON_OBJECT*)lpOwnerProcess;  //Must be set before call LinkKernelThread.
	if(lpOwnerProcess)
	{
		if(!ProcessManager.LinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
			(__COMMON_OBJECT*)lpOwnerProcess,(__COMMON_OBJECT*)lpKernelThread))
		{
			goto __TERMINAL;
		}
	}

	if(KERNEL_THREAD_STATUS_READY == dwStatus)         //Add into Ready Queue.
	{
		lpMgr->AddReadyKernelThread((__COMMON_OBJECT*)lpMgr,
			lpKernelThread);
	}
	else                                               //Add into Suspended Queue.
	{
		if(!lpMgr->lpSuspendedQueue->InsertIntoQueue((__COMMON_OBJECT*)lpMgr->lpSuspendedQueue,
			(__COMMON_OBJECT*)lpKernelThread,dwPriority))
			goto __TERMINAL;
	}

	//Call the create hook.
	lpMgr->CallThreadHook(THREAD_HOOK_TYPE_CREATE,lpKernelThread,
		NULL);
	bSuccess = TRUE;  //Now,the TRANSACTION of create a kernel thread is successfully.

__TERMINAL:
	if(!bSuccess)
	{
		//Detach from owner process's list.
		if(lpOwnerProcess)
		{
			ProcessManager.UnlinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
				(__COMMON_OBJECT*)lpOwnerProcess,(__COMMON_OBJECT*)lpKernelThread);
		}
		if(NULL != lpKernelThread)
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpKernelThread);
		if(NULL != lpStack)
			KMemFree(lpStack,KMEM_SIZE_TYPE_ANY,0);
		return NULL;
	}
	else
		return lpKernelThread;
}

//
//DestroyKernelThread's implementation.
//The routine do the following:
// 1. Check the status of the kernel thread object will be destroyed,if the
//    status is KERNEL_THREAD_STATUS_TERMINAL,then does the rest steps,else,
//    simple return;
// 2. Delete the kernel thread object from Terminal Queue;
// 3. Destroy the kernel thread object by calling DestroyObject.
//
static VOID DestroyKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpKernel)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread   = (__KERNEL_THREAD_OBJECT*)lpKernel;
	__KERNEL_THREAD_MANAGER*    lpMgr            = (__KERNEL_THREAD_MANAGER*)lpThis;
	__PRIORITY_QUEUE*           lpTerminalQueue  = NULL;
	LPVOID                      lpStack          = NULL;

	if((NULL == lpThis) || (NULL == lpKernel))    //Parameter check.
	{
		return;
	}

	//Only the kernel thread with TERMINAL status can be destroyed,use
	//TerminateKernelThread routine to terminate a specified kernel thrad's
	//running and transmit to this status.
	if(KERNEL_THREAD_STATUS_TERMINAL != lpKernelThread->dwThreadStatus)
	{
		return;
	}

	//Detach it from owner process's kernel thread list.
	ProcessManager.UnlinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
		NULL,(__COMMON_OBJECT*)lpKernelThread);

	lpTerminalQueue = lpMgr->lpTerminalQueue;
	lpTerminalQueue->DeleteFromQueue((__COMMON_OBJECT*)lpTerminalQueue,
		                             (__COMMON_OBJECT*)lpKernelThread);  //Delete from terminal queue.

	//Call terminal hook routine.
	lpMgr->CallThreadHook(THREAD_HOOK_TYPE_TERMINAL,
		lpKernelThread,NULL);

	lpStack = lpKernelThread->lpInitStackPointer;
	lpStack = (LPVOID)((DWORD)lpStack - lpKernelThread->dwStackSize);
	KMemFree(lpStack,KMEM_SIZE_TYPE_ANY,0);    //Free the stack of the kernel thread.

	ObjectManager.DestroyObject(&ObjectManager,
		                        (__COMMON_OBJECT*)lpKernelThread);

}

//Enable or disable suspending on a specified kernel thread.
//If bEnable is TRUE,then enable the suspending on the specified thread,and do a suspending
//if there is pending one(suspending counter > 0).
//If bEnable is FALSE,then disable the suspending operation on the specified thread.
static BOOL EnableSuspend(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread,BOOL bEnable)
{
	__KERNEL_THREAD_OBJECT*  lpKernelThread   = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KERNEL_THREAD_MANAGER* lpManager        = (__KERNEL_THREAD_MANAGER*)lpThis;
	DWORD                    dwFlags          = 0;

	if((NULL == lpKernelThread) || (NULL == lpManager))
	{
		return FALSE;
	}
	if(!bEnable)  //Should disable the suspending of the specified kernel thread.
	{
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		lpKernelThread->dwSuspendFlags |= SUSPEND_FLAG_DISABLE;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	}
	else  //Enable the suspending.
	{
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		lpKernelThread->dwSuspendFlags &= ~SUSPEND_FLAG_DISABLE;
		//Check if there is pending suspend operation.
		if((lpKernelThread->dwSuspendFlags & ~SUSPEND_FLAG_MASK) > 0)
		{
			if(KERNEL_THREAD_STATUS_RUNNING == lpKernelThread->dwThreadStatus)
			{
				//Change status to SUSPENDED and put it into suspending queue.
				lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_SUSPENDED;
				lpManager->lpSuspendedQueue->InsertIntoQueue(
					(__COMMON_OBJECT*)lpManager->lpSuspendedQueue,
					(__COMMON_OBJECT*)lpKernelThread,
					lpKernelThread->dwThreadPriority);
			}
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	}
	//Do a reschedule,if in kernel thread context.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		lpManager->ScheduleFromProc(NULL);
	}
	return TRUE;
}

//Suspend one kernel thread.
//It increments the suspending counter of the specified kernel thread,checks
//if the specified kernel thread is suspending enable,and suspend it if so and it's
//current status is RUNNING.Otherwise just return.
static BOOL SuspendKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*    lpManager        = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*     lpKernelThread   = (__KERNEL_THREAD_OBJECT*)lpThread;
	DWORD                       dwFlags          = 0;

	if((NULL == lpManager) || (NULL == lpKernelThread))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if((lpKernelThread->dwSuspendFlags & ~SUSPEND_FLAG_MASK) < MAX_SUSPEND_COUNTER)
	{
		//Increment the suspending counter.
		lpKernelThread->dwSuspendFlags ++;
	}
	//Check if the specified kernel thread is permit suspending.
	if(!(lpKernelThread->dwSuspendFlags & SUSPEND_FLAG_DISABLE))
	{
		//Suspend the kernel thread.
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_SUSPENDED;
		lpManager->lpSuspendedQueue->InsertIntoQueue((__COMMON_OBJECT*)lpManager->lpSuspendedQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			lpKernelThread->dwThreadPriority);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Do a reschedule if in kernel thread context.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		lpManager->ScheduleFromProc(NULL);
	}
	return TRUE;
}

//Resume a specified kernel thread.
//It decrements the suspending counter of the given kernel thread,if the counter is reach
//zero,then check if the kernel thread's status is SUSPENDED,and move it into READY queue
//if so.
static BOOL ResumeKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*     lpManager      = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*      lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	DWORD                        dwFlags;

	if((NULL == lpManager) || (NULL == lpKernelThread))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if((lpKernelThread->dwSuspendFlags & ~SUSPEND_FLAG_MASK) > 0)
	{
		lpKernelThread->dwSuspendFlags --;
	}
	if(0 == (lpKernelThread->dwSuspendFlags & ~SUSPEND_FLAG_MASK))
	{
		//If the kernel thread is in suspending queue,then move it to ready queue.
		if(KERNEL_THREAD_STATUS_SUSPENDED == lpKernelThread->dwThreadStatus)
		{
			lpManager->lpSuspendedQueue->DeleteFromQueue(
				(__COMMON_OBJECT*)lpManager->lpSuspendedQueue,
				(__COMMON_OBJECT*)lpKernelThread);
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			lpManager->AddReadyKernelThread((__COMMON_OBJECT*)lpManager,
				lpKernelThread);
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Do reschedule if in kernel thread context.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		lpManager->ScheduleFromProc(NULL);
	}
	return TRUE;
}

#ifdef __CFG_SYS_IS  //The following 2 routines apply only in inline schedule mode.
//
//ScheduleFromProc's implementation.
//This routine can be called anywhere that re-schedule is required,under process context.
//The ScheduleFromInt(later one) should be called in interrupt context.
//
static VOID ScheduleFromProc(__KERNEL_THREAD_CONTEXT* lpContext)
{
	//__KERNEL_THREAD_OBJECT*          lpKernelThread     = NULL;
	__KERNEL_THREAD_OBJECT*          lpCurrent          = NULL;
	__KERNEL_THREAD_OBJECT*          lpNew              = NULL;
	//__KERNEL_THREAD_CONTEXT**        lppOldContext      = NULL;
	//__KERNEL_THREAD_CONTEXT**        lppNewContext      = NULL;
	DWORD                            dwFlags            = 0;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpCurrent = KernelThreadManager.lpCurrentKernelThread;
	switch(lpCurrent->dwThreadStatus)  //Do different actions according to status.
	{
	case KERNEL_THREAD_STATUS_RUNNING:
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);    //Get a ready kernel thread.
		if(NULL == lpNew)  //Current one is most priority whose status is READY.
		{
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;  //Let current kernel thread continue to run.
		}
		else  //Should swap out current kernel thread and run next ready one.
		{
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrent);  //Insert into ready queue.
			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			KernelThreadManager.lpCurrentKernelThread = lpNew;
			//Call schedule hook before swich.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);

			__SaveAndSwitch(&lpCurrent->lpKernelThreadContext,
				&lpNew->lpKernelThreadContext);  //Switch.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
	case KERNEL_THREAD_STATUS_READY:
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);  //Get a ready thread.

		if(NULL == lpNew)  //Should not occur.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			BUG();
			return;
		}
		if(lpNew == lpCurrent)  //The same one.
		{
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
		else  //Not the same one.
		{
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			KernelThreadManager.lpCurrentKernelThread = lpNew;
			//Call schedule hook routine.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);

			__SaveAndSwitch(&lpCurrent->lpKernelThreadContext,
				&lpNew->lpKernelThreadContext);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return;
		}
	case KERNEL_THREAD_STATUS_BLOCKED:
	case KERNEL_THREAD_STATUS_SUSPENDED:
	case KERNEL_THREAD_STATUS_TERMINAL:
	case KERNEL_THREAD_STATUS_SLEEPING:
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);  //Get a ready thread to run.

		if(NULL == lpNew)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			BUG();
			PrintLine("Fatal error: in ScheduleFromProc,lpNew == NULL.");
			return;
		}
		lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
		KernelThreadManager.lpCurrentKernelThread = lpNew;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
			lpCurrent,lpNew);

		__SaveAndSwitch(&lpCurrent->lpKernelThreadContext,
			&lpNew->lpKernelThreadContext);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	default:
		BUG();
		break;
	}
}

//ScheduleFromInt,will be invoked in INTERRUPT context,to schedule all
//kernel thread(s) in system.
//The corresponding routine,ScheduleFromProc,is called in PROCESS context
//to do schedule.
static VOID ScheduleFromInt(__COMMON_OBJECT* lpThis,LPVOID lpESP)
{
	__KERNEL_THREAD_OBJECT*         lpNextThread    = NULL;
	__KERNEL_THREAD_OBJECT*         lpCurrentThread = NULL;
	__KERNEL_THREAD_MANAGER*        lpMgr           = (__KERNEL_THREAD_MANAGER*)lpThis;

	if((NULL == lpThis) || (NULL == lpESP))    //Parameters check.
	{
		return;
	}

	if(NULL == lpMgr->lpCurrentKernelThread)   //The routine is called first time.
	{
		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);
		if(NULL == lpNextThread)               //This case can occur,when the interrupt is enabled and
			                                   //in process of system initialization,there is not any kernel thread.
		{
			//BUG();
			return;
		}
		KernelThreadManager.lpCurrentKernelThread = lpNextThread;
		lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);

		__SwitchTo(lpNextThread->lpKernelThreadContext);  //Switch to this thread.
	}
	else  //Not the first time be called.
	{
		lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;
		//This code line saves the context of current kernel thread.
		lpCurrentThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpESP;

		switch(lpCurrentThread->dwThreadStatus)
		{
		//If a kernel thread's status is one of the following,it means the
		//kernel thread is interrupted in the time between CHANGING IT'S STATUS
		//and SWAPE OUT.In this scenario we just let the current thread to go,
		//it will be swaped out immediately in the ScheduleFromProc routine,which
		//will be called by the current thread itself.
		case KERNEL_THREAD_STATUS_BLOCKED:     //Waiting shared object in process.
		case KERNEL_THREAD_STATUS_TERMINAL:    //In process of termination.
		case KERNEL_THREAD_STATUS_SLEEPING:    //In process of falling in sleep.
		//case KERNEL_THREAD_STATUS_SUSPENDED:   //In process of being suspended.
		case KERNEL_THREAD_STATUS_READY:       //Wakeup immediately in another interrupt.
			lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			//lpContext = lpCurrentThread->lpKernelThreadContext;
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);
			__SwitchTo(lpCurrentThread->lpKernelThreadContext);
			break; //This instruction will never reach.

		case KERNEL_THREAD_STATUS_SUSPENDED:
			//But for kernel thread's suspending,it's a little different.
			//When the suspending is generated by the thread itself,then
			//we can follow the processing mechanism as BLOCK status.But if
			//the suspending is made by other kernel thread running in other
			//CPU,if we let the suspended thread to go,it may lead serious
			//issue.
			//So if the thread's status is SUSPENDED,we just swape it out and
			//fetch one READY thread to run.
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0);
			if(NULL == lpNextThread)  //Should not occur.
			{
				BUG();
				break;
			}
			lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			lpMgr->lpCurrentKernelThread = lpNextThread;
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
			__SwitchTo(lpNextThread->lpKernelThreadContext);
			break;
		case KERNEL_THREAD_STATUS_RUNNING:
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrentThread->dwThreadPriority);
			if(NULL == lpNextThread)  //Current is most priority.
			{
				lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);

				__SwitchTo(lpCurrentThread->lpKernelThreadContext);
				return;
			}
			else
			{
				lpCurrentThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
				KernelThreadManager.AddReadyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					lpCurrentThread);  //Add to ready queue.

				lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				lpMgr->lpCurrentKernelThread = lpNextThread;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
				__SwitchTo(lpNextThread->lpKernelThreadContext);
				return;
			}
		default:
			BUG();
			break;
		}
	}
}

#else  //The following routine(s) apply in non-inline schedule mode.

//The following routines should be implemented in other module,for example,
//in ASM language,to trigger the uniform scheduler.
extern VOID ScheduleFromProc(__KERNEL_THREAD_CONTEXT* lpContext);
extern VOID ScheduleFromInt(__COMMON_OBJECT* lpThis,LPVOID lpESP);

//Uniform scheduler.This routine is used only in inline schedule mode,
//which should be enabled by comment off the __SYS_CFG_IS flat in config.h
//file.
//This routine get one kernel thread with most priority in ready queue and
//return it's context,then the interrupt hander can switch to this new thread.
//The old kernel thread's context also be saved in this routine,which is
//transfered to this routine by lpESP parameter.
//This routine should be called in critical section.
LPVOID UniSchedule(__COMMON_OBJECT* lpThis,LPVOID lpESP)
{
	__KERNEL_THREAD_OBJECT*         lpNextThread    = NULL;
	__KERNEL_THREAD_OBJECT*         lpCurrentThread = NULL;
	DWORD                           dwFlags;
	
	if(NULL == lpESP)    //Parameters check.
	{
		return lpESP;
	}

	//The following section should not be interruptted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == KernelThreadManager.lpCurrentKernelThread)   //The routine is called first time.
	{
		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);
		if(NULL == lpNextThread)               //This case can occur,when the interrupt is enabled and
			                                   //in process of system initialization,there is not any kernel thread.
		{
			BUG();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return NULL;
		}
		KernelThreadManager.lpCurrentKernelThread = lpNextThread;
		lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
		//Return the new kernel thread's context.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return (LPVOID)(lpNextThread->lpKernelThreadContext);
	}
	else  //Not the first time be called.
	{
		lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;
		//Saves the context of current kernel thread.
		lpCurrentThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpESP;

		switch(lpCurrentThread->dwThreadStatus)
		{
		case KERNEL_THREAD_STATUS_BLOCKED:     //Waiting shared object in process.
		case KERNEL_THREAD_STATUS_TERMINAL:    //In process of termination.
		case KERNEL_THREAD_STATUS_SLEEPING:    //In process of falling in sleep.
		case KERNEL_THREAD_STATUS_SUSPENDED:   //In process of being suspended.
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0);  //lpCurrentThread->dwThreadPriority); <-- BUG caused-->
			if(NULL == lpNextThread)  //Should not occur.
			{
				BUG();
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return NULL;
			}
			KernelThreadManager.lpCurrentKernelThread = lpNextThread;
			lpNextThread->dwThreadStatus  = KERNEL_THREAD_STATUS_RUNNING;
			lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			//lpContext = lpCurrentThread->lpKernelThreadContext;
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrentThread,lpNextThread);
			//Return the new thread's context.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return (LPVOID)(lpNextThread->lpKernelThreadContext);
		case KERNEL_THREAD_STATUS_READY:       //Wakeup immediately in another interrupt.
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrentThread->dwThreadPriority);  //Get a ready thread.
			if(NULL == lpNextThread)  //Should not occur.
			{
				BUG();  //Will cause the system into dead loop state.
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return NULL;
			}
			if(lpNextThread == lpCurrentThread)  //The same one.
			{
				lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				lpCurrentThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return lpESP;
			}
			else  //Not the same one.
			{
				lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				KernelThreadManager.lpCurrentKernelThread = lpNextThread;
				//Call schedule hook routine.
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
					lpCurrentThread,lpNextThread);
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return (LPVOID)(lpNextThread->lpKernelThreadContext);
			}
		case KERNEL_THREAD_STATUS_RUNNING:
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrentThread->dwThreadPriority);
			if(NULL == lpNextThread)  //Current is most priority.
			{
				lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return (LPVOID)lpESP;
			}
			else
			{
				lpCurrentThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
				KernelThreadManager.AddReadyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					lpCurrentThread);  //Add to ready queue.

				lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				KernelThreadManager.lpCurrentKernelThread = lpNextThread;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return (LPVOID)(lpNextThread->lpKernelThreadContext);
			}
		default:
			BUG();
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return NULL;
		}
	}
	//Should not reach here.
	//return (LPVOID)lpESP;
}

#endif  //__CFG_SYS_IS

//SetThreadPriority.
static DWORD SetThreadPriority(__COMMON_OBJECT* lpKernelThread,DWORD dwPriority)
{
	__KERNEL_THREAD_OBJECT*    lpThread = NULL;
	DWORD                      dwOldPri = PRIORITY_LEVEL_IDLE;
	DWORD                      dwFlags  = 0;

	if(NULL == lpKernelThread)
		return PRIORITY_LEVEL_IDLE;
	
	lpThread = (__KERNEL_THREAD_OBJECT*)lpKernelThread;
	dwOldPri = lpThread->dwThreadPriority;
	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpThread->dwThreadPriority = dwPriority;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)

	return dwOldPri;
}

//GetThreadPriority.
static DWORD GetThreadPriority(__COMMON_OBJECT* lpKernelThread)
{
	//__KERNEL_THREAD_OBJECT*    lpThread = NULL;

	if(NULL == lpKernelThread)
	{
		return PRIORITY_LEVEL_IDLE;
	}
	return ((__KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadPriority;
}

//Terminate a kernel thread.Only support the scenario that one kernel thread terminates itself,
//don't support one kernel thread terminates other kernel thread,for safety reason.
static DWORD TerminateKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread,DWORD dwExitCode)
{
	__KERNEL_THREAD_MANAGER* lpManager      = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*  lpKernelThread = NULL;

	if(NULL == lpManager)
	{
		return 0;
	}

	//Only support the scenario that a kernel thread terminates itself.
	if(lpThread != NULL)
	{
		return 0;
	}
	//Get current kernel thread object.
	lpKernelThread = lpManager->lpCurrentKernelThread;
	KernelThreadClean((__COMMON_OBJECT*)lpKernelThread,dwExitCode);
	return 0;  //This clause will never reach.
}

//
//Sleep Routine.
//This routine do the following:
// 1. Updates the dwNextWakeupTick value of kernel thread manager;
// 2. Modifies the current kernel thread's status to SLEEPING;
// 3. Puts the current kernel thread into sleeping queue of kernel thread manager;
// 4. Schedules another kernel thread to run;
// 5. If the specified sleep time is 0,then do a re-schedule.
//
static BOOL Sleep(__COMMON_OBJECT* lpThis,/*__COMMON_OBJECT* lpKernelThread,*/DWORD dwMillisecond)
{
	__KERNEL_THREAD_MANAGER*           lpManager      = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*            lpKernelThread = NULL;
	DWORD                              dwTmpCounter   = 0;
	//__KERNEL_THREAD_CONTEXT*           lpContext      = NULL;
	DWORD                              dwFlags        = 0;

	if(NULL == lpManager)    //Parameters check.
	{
	     return FALSE;
	}
	if(dwMillisecond < SYSTEM_TIME_SLICE)  //Just re-schedule all kernel thread(s).
	{
		lpManager->ScheduleFromProc(NULL);
		return TRUE;
	}

	//lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	lpKernelThread = lpManager->lpCurrentKernelThread;
	if(NULL == lpKernelThread)    //The routine is called in system initializing process.
	{
		BUG();
		return FALSE;
	}
	dwTmpCounter =  dwMillisecond / SYSTEM_TIME_SLICE;
	dwTmpCounter += System.dwClockTickCounter;         //Now,dwTmpCounter countains the 
	                                                   //tick counter value when this
	                                                   //kernel thread who calls this routine
	                                                   //must be waken up.
	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	if((0 == lpManager->dwNextWakeupTick) || (lpManager->dwNextWakeupTick > dwTmpCounter))
	{
		lpManager->dwNextWakeupTick = dwTmpCounter;     //Update dwNextWakeupTick value.
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_SLEEPING;
	dwTmpCounter = MAX_DWORD_VALUE - dwTmpCounter;     //Calculates the priority of the
	                                                   //current kernel thread in the sleeping
	                                                   //queue.
	lpManager->lpSleepingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpManager->lpSleepingQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		dwTmpCounter);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
	//lpContext = &lpKernelThread->KernelThreadContext;
	lpManager->ScheduleFromProc(NULL);
	return TRUE;
}

//CancelSleep Routine.
static BOOL CancelSleep(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpKernelThread)
{
	return FALSE;
}

//SetCurrentIRQL.
static DWORD SetCurrentIRQL(__COMMON_OBJECT* lpThis,DWORD dwNewIRQL)
{
	return 0;
}

//GetCurrentIRQL.
static DWORD GetCurrentIRQL(__COMMON_OBJECT* lpThis)
{
	return 0;
}

//SetLastError.
static DWORD SetLastError(/*__COMMON_OBJECT* lpKernelThread,*/DWORD dwNewError)
{
	DWORD  dwOldError = 0;

	dwOldError = KernelThreadManager.lpCurrentKernelThread->dwLastError;
	KernelThreadManager.lpCurrentKernelThread->dwLastError = dwNewError;
	return dwOldError;
}

//GetLastError.
static DWORD GetLastError(/*__COMMON_OBJECT* lpKernelThread*/)
{
	return KernelThreadManager.lpCurrentKernelThread->dwLastError;
}

//GetThreadID.
static DWORD GetThreadID(__COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)  //Return current kernel thread's ID.
	{
		return KernelThreadManager.lpCurrentKernelThread->dwThreadID;
	}

	return ((__KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadID;
}

//GetThreadStatus.
static DWORD GetThreadStatus(__COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)  //Return current kernel thread's ID.
	{
		return KernelThreadManager.lpCurrentKernelThread->dwThreadStatus;
	}

	return ((__KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadStatus;
}

//SetThreadStatus.
static DWORD SetThreadStatus(__COMMON_OBJECT* lpKernelThread,DWORD dwStatus)
{
	return 0;
}

//MsgQueueFull.
static BOOL MsgQueueFull(__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread  = NULL;

	if(NULL == lpThread)    //Parameter check.
	{
		return FALSE;
	}

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;

	return MAX_KTHREAD_MSG_NUM == lpKernelThread->ucCurrentMsgNum ? TRUE : FALSE;
}

//MsgQueueEmpty.
static BOOL MsgQueueEmpty(__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread = NULL;

	if(NULL == lpThread)   //Parameter check.
	{
		return FALSE;
	}

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;

	return 0 == lpKernelThread->ucCurrentMsgNum ? TRUE : FALSE;
}

//SendMessage.
static BOOL MgrSendMessage(__COMMON_OBJECT* lpThread,__KERNEL_THREAD_MESSAGE* lpMsg)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread = NULL;
	__KERNEL_THREAD_OBJECT*     lpNewThread    = NULL;
	BOOL                        bResult        = FALSE;
	DWORD                       dwFlags        = 0;

	if(NULL == lpMsg) //Parameters check.
	{
		goto __TERMINAL;
	}
	if(NULL == lpThread)  //Send message to current kernel thread.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	}
	else  //Send message to the specified target thread.
	{
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MsgQueueFull((__COMMON_OBJECT*)lpKernelThread))             //Message queue is full.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//Message queue not full,put the message to the queue.
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].wCommand
		= lpMsg->wCommand;
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].wParam
		= lpMsg->wParam;
	lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueTrial].dwParam
		= lpMsg->dwParam;
	lpKernelThread->ucMsgQueueTrial ++;
	if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueTrial)
	{
		lpKernelThread->ucMsgQueueTrial = 0;
	}
	lpKernelThread->ucCurrentMsgNum ++;

	lpNewThread = (__KERNEL_THREAD_OBJECT*)lpKernelThread->lpMsgWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
		NULL);
	if(lpNewThread)  //Should wakeup the target kernel thread.
	{
		lpNewThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpNewThread);  //Add to ready queue.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//
	//If in kernel thread context,then re-schedule kernel thread.
	//
	if(IN_KERNELTHREAD())  //---- !!!!!!!! PROBLEM CAUSED !!!!!!!! ----
	{
		KernelThreadManager.ScheduleFromProc(NULL);
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//GetMessage from current or a specified kernel thread's message queue,it's a blocking operation
//that will never return without message got.
//TRUE will be returned if message is got successfully,otherwise,FALSE,in case of bad parameter.
static BOOL MgrGetMessage(__COMMON_OBJECT* lpThread,__KERNEL_THREAD_MESSAGE* lpMsg)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread  = NULL;
	DWORD                       dwFlags         = 0;

	if(NULL == lpMsg) //Parameters check.
	{
		return FALSE;
	}
	if(NULL == lpThread)  //Get message from current kernel thread's queue.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	}
	else  //Get message from the specified kernel thread.
	{
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MsgQueueEmpty((__COMMON_OBJECT*)lpKernelThread))  //Current message queue is empty,should waiting.
	{
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->lpMsgWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.
		goto __TRY_AGAIN;
	}
	else
	{
		lpMsg->wCommand     = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wCommand;
		lpMsg->wParam       = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wParam;
		lpMsg->dwParam      = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].dwParam;
		lpKernelThread->ucMsgQueueHeader ++;
		if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueHeader)
		{
			lpKernelThread->ucMsgQueueHeader = 0;
		}
		lpKernelThread->ucCurrentMsgNum --;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	}
	return TRUE;
}

//GetMessage is not suitable in some case since it's a blocking operation,the PeekMessage
//operation is provided here.
//PeekMessage will check the current or specified kernel thread's message queue,if there
//is one or more message,it will fetch one and return TRUE,otherwise(the message queue is
//empty),FALSE will be returned.
static BOOL MgrPeekMessage(__COMMON_OBJECT* lpThread,__KERNEL_THREAD_MESSAGE* lpMsg)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread  = NULL;
	DWORD                       dwFlags         = 0;

	if(NULL == lpMsg) //Parameters check.
	{
		return FALSE;
	}
	if(NULL == lpThread)  //Get message from current kernel thread's queue.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	}
	else  //Get message from the specified kernel thread.
	{
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MsgQueueEmpty((__COMMON_OBJECT*)lpKernelThread))  //Current message queue is empty.
	{
		//KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	else  //Has message in queue,fetch one.
	{
		lpMsg->wCommand     = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wCommand;
		lpMsg->wParam       = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wParam;
		lpMsg->dwParam      = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].dwParam;
		lpKernelThread->ucMsgQueueHeader ++;
		if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueHeader)
		{
			lpKernelThread->ucMsgQueueHeader = 0;
		}
		lpKernelThread->ucCurrentMsgNum --;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return TRUE;
	}
}

//
//The following routine is used to lock a kernel thread,especially the current kernel thread.
//When a kernel thread is locked,it can not be interrupted,even a kernel thread with high priority
//ready to run.The different between lock kernel thread and lock interrupt is,when interrupt is 
//locked,the system will never be interrupted by hardware,and the schedule will never occur,because
//timer interrupt also be locked,but when lock a kernel thread,only disables the schedule of the
//system,interrupt is not locked,so,hardware interrupt can also be processed by system.
//CAUTION: When lock a kernel thread,you must unlock it by calling UnlockKernelThread routine to
//unlock the kernel thread,otherwise,others kernel thread(s) will never be scheduled.
//
static BOOL LockKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*                   lpManager        = NULL;
	__KERNEL_THREAD_OBJECT*                    lpKernelThread   = NULL;
	DWORD                                      dwFlags          = 0;

	if(NULL == lpThis)    //Parameter check.
	{
		return FALSE;
	}

	lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpKernelThread = (NULL == lpThread) ? lpManager->lpCurrentKernelThread : 
	(__KERNEL_THREAD_OBJECT*)lpThread;    //If lpThread is NULL,then lock the current kernel thread.

	if(KERNEL_THREAD_STATUS_RUNNING != lpKernelThread->dwThreadStatus)
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
		return FALSE;
	}

	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;    //Once mark the status of
	                                                                  //the target thread to
	                                                                  //BLOCKED,it will never be
	                                                                  //switched out.
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
	return TRUE;
}

//
//The following routine unlockes a kernel thread who is locked by LockKernelThread routine.
//

static VOID UnlockKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*               lpManager       = NULL;
	__KERNEL_THREAD_OBJECT*                lpKernelThread  = NULL;
	DWORD                                  dwFlags         = 0;

	if(NULL == lpThis)    //Parameter check.
	{
		return;
	}

	lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags)
	lpKernelThread = (NULL == lpThread) ? lpManager->lpCurrentKernelThread : 
	(__KERNEL_THREAD_OBJECT*)lpThread;
	if(KERNEL_THREAD_STATUS_BLOCKED != lpKernelThread->dwThreadStatus)  //If not be locked.
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags)
	return;
}

/**************************************************************
***************************************************************
***************************************************************
**************************************************************/
//
//The definition of Kernel Thread Manager.
//

__KERNEL_THREAD_MANAGER KernelThreadManager = {
	0,                                              //dwCurrentIRQL.
	NULL,                                            //CurrentKernelThread.

	NULL,                                            //lpRunningQueue.
	//NULL,                                            //lpReadyQueue.
	NULL,                                            //lpSuspendedQueue.
	NULL,                                            //lpSleepingQueue.
	NULL,                                            //lpTerminalQueue.

	{0},                                             //Ready queue array.
	//0,                                              //dwClockTickCounter.
	0,                                              //dwNextWakeupTick.

	NULL,                                            //lpCreateHook.
	NULL,                                            //lpEndScheduleHook.
	NULL,                                            //lpBeginScheduleHook.
	NULL,                                            //lpTerminalHook.
	SetThreadHook,                                   //SetThreadHook.
	CallThreadHook,                                  //CallThreadHook.

	GetScheduleKernelThread,                         //GetScheduleKernelThread.
	AddReadyKernelThread,                            //AddReadyKernelThread.
	KernelThreadMgrInit,                             //Initialize routine.

	CreateKernelThread,                              //CreateKernelThread routine.
	DestroyKernelThread,                             //DestroyKernelThread routine.

	EnableSuspend,                                   //EnableSuspend routine.
	SuspendKernelThread,                             //SuspendKernelThread routine.
	ResumeKernelThread,                              //ResumeKernelThread routine.

#ifdef __CFG_SYS_IS   //Inline schedule mode.
	ScheduleFromProc,                                //ScheduleFromProc routine.
	ScheduleFromInt,                                 //ScheduleFromInt routine.
	NULL,                                            //UniSchedule routine.
#else  //No-inline schedule mode.
	ScheduleFromProc,                                //ScheduleFromProc routine.
	ScheduleFromInt,                                 //ScheduleFromInt routine.
	UniSchedule,                                     //UniSchedule routine.
#endif  //__CFG_SYS_IS

	SetThreadPriority,                               //SetThreadPriority routine.
	GetThreadPriority,                               //GetThreadPriority routine.

	TerminateKernelThread,                           //TerminalKernelThread routine.
	Sleep,                                           //Sleep routine.
	CancelSleep,                                     //CancelSleep routine.

	SetCurrentIRQL,                                  //SetCurrentIRQL routine.
	GetCurrentIRQL,                                  //GetCurrentIRQL routine.

	GetLastError,                                    //GetLastError routine.
	SetLastError,                                    //SetLastError routine.

	GetThreadID,                                     //GetThreadID routine.

	GetThreadStatus,                                 //GetThreadStatus routine.
	SetThreadStatus,                                 //SetThreadStatus routine.

	MgrSendMessage,                                  //SendMessage routine.
	MgrGetMessage,                                   //GetMessage routine.
	MgrPeekMessage,                                  //PeekMessage routine.
	MsgQueueFull,                                    //MsgQueueFull routine.
	MsgQueueEmpty,                                   //MsgQueueEmpty routine.
	LockKernelThread,                                //LockKernelThread routine.
	UnlockKernelThread                               //UnlockKernelThread routine.
};

//
/**************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/
//
//Dispatch a message to an message(event) handler.
//
DWORD DispatchMessage(__KERNEL_THREAD_MESSAGE* lpMsg,__KERNEL_THREAD_MESSAGE_HANDLER lpHandler)
{
	return lpHandler(lpMsg->wCommand,lpMsg->wParam,lpMsg->dwParam);
}
