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

#include "StdAfx.h"
#include "mlayout.h"
#include "stdio.h"
#include "types.h"
#include "system.h"
#include "commobj.h"
#include "process.h"
#include "stdio.h"
#include "stdlib.h"
#include "ktmgr.h"

//
//Pre-declare for extern global routines,these routines may
//be implemented in ktmgrx.c file,where x is 2,3,etc.
//
extern __THREAD_HOOK_ROUTINE SetThreadHook(DWORD dwHookType, 
	__THREAD_HOOK_ROUTINE lpRoutine);
extern VOID CallThreadHook(DWORD dwHookType, __KERNEL_THREAD_OBJECT* lpPrev,
	__KERNEL_THREAD_OBJECT* lpNext);
extern __KERNEL_THREAD_OBJECT* GetScheduleKernelThread(__COMMON_OBJECT* lpThis, 
	DWORD dwPriority);
extern VOID AddReadyKernelThread(__COMMON_OBJECT* lpThis,
	__KERNEL_THREAD_OBJECT* lpKernelThread);
extern VOID __KernelThreadWrapper(__COMMON_OBJECT*);
extern VOID KernelThreadClean(__COMMON_OBJECT*,DWORD);
/* Routines to handle thread flag(s),if set. */
extern void HandleThreadFlags(__COMMON_OBJECT* pThread);

/* Helper routine to initialize kernel thread's ready queues. */
static BOOL InitReadyQueue(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_MANAGER* pManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	BOOL bResult = FALSE;
	__PRIORITY_QUEUE* pPriorityQueue = NULL;
	__KERNEL_THREAD_READY_QUEUE* pReadyQueue = NULL;
#if defined(__CFG_SYS_SMP)
	//int processorNum = ProcessorManager.GetProcessorNum();
	int processorNum = (ProcessorManager.maxProcessorID + 1);
#else
	int processorNum = 1;
#endif

	BUG_ON(NULL == pManager);
	/* 
	 * Create one ready queue for each processor,and create MAX_KERNEL_THREAD_PRIORITY + 1 
	 * priority queue objects for each ready queue object.
	 * Any failure will lead this routine's failure.
	 */
	for (int qIndex = 0; qIndex < processorNum; qIndex++)
	{
		pReadyQueue = (__KERNEL_THREAD_READY_QUEUE*)_hx_calloc(1, sizeof(__KERNEL_THREAD_READY_QUEUE));
		if (NULL == pReadyQueue)
		{
			goto __TERMINAL;
		}
#if defined(__CFG_SYS_SMP)
		/* Initialize spin lock. */
		__INIT_SPIN_LOCK(pReadyQueue->spin_lock, "rdyq");
#endif
		/* Create priority queue for each priority level. */
		for (int i = 0; i < MAX_KERNEL_THREAD_PRIORITY + 1; i++)
		{
			pPriorityQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
				NULL,
				OBJECT_TYPE_PRIORITY_QUEUE);
			if (NULL == pPriorityQueue)
			{
				goto __TERMINAL;
			}
			if (!pPriorityQueue->Initialize((__COMMON_OBJECT*)pPriorityQueue))
			{
				goto __TERMINAL;
			}
			pReadyQueue->ThreadQueue[i] = pPriorityQueue;
		}
		/* Save the ready queue pointer to corresponding processor's slot. */
		pManager->KernelThreadReadyQueue[qIndex] = pReadyQueue;
	}
	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;;
}

/* 
 * Initializing routine of Kernel Thread Manager,it will be
 * invoked in process of OS initialization.The failure of
 * kernel thread manager's initialization will lead the failure
 * of OS initialization.
 */
static BOOL KernelThreadMgrInit(__COMMON_OBJECT* lpThis)
{
	BOOL bResult = FALSE;
	__KERNEL_THREAD_MANAGER* lpMgr = NULL;
	__PRIORITY_QUEUE* lpSuspendedQueue = NULL;
	__PRIORITY_QUEUE* lpSleepingQueue = NULL;
	__PRIORITY_QUEUE* lpTerminalQueue = NULL;
	__PRIORITY_QUEUE* lpKilledQueue = NULL;

	if(NULL == lpThis)
	{
		return bResult;
	}
	lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;

	/*
	 * The following code creates all objects required by Kernel Thread Manager.
	 * If any error occurs,the initializing process is ternimated and jump to fail.
	 */
	lpSuspendedQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSuspendedQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpSuspendedQueue->Initialize((__COMMON_OBJECT*)lpSuspendedQueue))
	{
		goto __TERMINAL;
	}
	lpSleepingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpSleepingQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpSleepingQueue->Initialize((__COMMON_OBJECT*)lpSleepingQueue))
	{
		goto __TERMINAL;
	}
	lpTerminalQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpTerminalQueue)
	{
		goto __TERMINAL;
	}
	if(FALSE == lpTerminalQueue->Initialize((__COMMON_OBJECT*)lpTerminalQueue))
	{
		goto __TERMINAL;
	}
	lpKilledQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager, NULL, 
		OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == lpKilledQueue)
	{
		goto __TERMINAL;
	}
	if (FALSE == lpKilledQueue->Initialize((__COMMON_OBJECT*)lpKilledQueue))
	{
		goto __TERMINAL;
	}

	//
	//Now,the objects required by Kernel Thread Manager are created and initialized success-
	//fully,initialize the kernel thread manager itself now.
	//
	lpMgr->lpSuspendedQueue = lpSuspendedQueue;
	lpMgr->lpSleepingQueue = lpSleepingQueue;
	lpMgr->lpTerminalQueue = lpTerminalQueue;
	lpMgr->lpKilledQueue = lpKilledQueue;

#if 0
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
#endif

	/* Initialize the ready queue array of kernel thread. */
	if (!InitReadyQueue(lpThis))
	{
		goto __TERMINAL;
	}

	/* Reset current kernel thread handle array. */
#if defined(__CFG_SYS_SMP)
	for (int i = 0; i < MAX_CPU_NUM; i++)
	{
		lpMgr->CurrentKernelThread[i] = NULL;
	}
#else
	lpMgr->CurrentKernelThread[0] = NULL;
#endif

	/* Set scheduling flag as enable. */
	__ATOMIC_SET(&lpMgr->scheduling_enabled, KERNEL_THREAD_SCHEDULING_ENABLE);

	bResult = TRUE;

__TERMINAL:
	if(!bResult)  //If failed to initialize the Kernel Thread Manager.
	{
		if (NULL != lpSuspendedQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpSuspendedQueue);
		}
		if (NULL != lpSleepingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpSleepingQueue);
		}
		if (NULL != lpTerminalQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpTerminalQueue);
		}
		if (NULL != lpKilledQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpKilledQueue);
		}
	}

	return bResult;
}

/*
 * Creates a new thread,either user thread or kernel.
 * The CreateUserThread and CreateKernelThread are wrappers
 * of this routine.
 */
static __KERNEL_THREAD_OBJECT* __CreateThread(
	__COMMON_OBJECT* lpThis,
	unsigned long dwStackSize,
	LPVOID pInitStack,
	unsigned long dwStatus,
	unsigned long dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	LPVOID lpOwnerProcess,
	LPVOID pUserStack,
	unsigned long user_stk_size,
	LPSTR lpszName)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	__KERNEL_THREAD_MANAGER* lpMgr = NULL;
	LPVOID lpStack = NULL;
	BOOL bSuccess = FALSE;
	DWORD i = 0;

	/* Basic parameters checking. */
	if((NULL == lpThis) || (NULL == lpStartRoutine))
	{
		goto __TERMINAL;
	}

	/* If user specified stack but it's size is too small. */
	if (pInitStack)
	{
		if (dwStackSize < MIN_STACK_SIZE)
		{
			goto __TERMINAL;
		}
	}

	/* User stack must be specified if to create a user thread. */
	if (lpOwnerProcess)
	{
		BUG_ON((NULL == pUserStack) || (0 == user_stk_size));
	}

	/* 
	 * The initation status of a kernel thread should only be READY or
	 * SUSPENDED.If the initation status is READY,then the kernel thread maybe
	 * scheduled to run in the NEXT schedule circle(please note the kernel thread
	 * does not be scheduled immediately),else,the kernel thread will be susp-
	 * ended,the kernel thread in this status can be activated by kResumeKernelThread
	 * system call.
	 */
	if ((KERNEL_THREAD_STATUS_READY != dwStatus) && 
		(KERNEL_THREAD_STATUS_SUSPENDED != dwStatus))
	{
		goto __TERMINAL;
	}

	lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_KERNEL_THREAD);
	if (NULL == lpKernelThread)
	{
		goto __TERMINAL;
	}
	if(!lpKernelThread->Initialize((__COMMON_OBJECT*)lpKernelThread))
	{
		goto __TERMINAL;
	}

	/* Use default kernel thread stack size if not specified. */
	if(0 == dwStackSize)
	{
		dwStackSize = DEFAULT_STACK_SIZE;
	}
	else
	{
		if(dwStackSize < MIN_STACK_SIZE)
		{
			dwStackSize = MIN_STACK_SIZE;
		}
	}

	/* Create kernel thread's stack,if not specified. */
	if (NULL == pInitStack)
	{
		lpStack = KMemAlloc(dwStackSize, KMEM_SIZE_TYPE_ANY);
		if (NULL == lpStack)
		{
			goto __TERMINAL;
		}
	}
	else
	{
		/* Just use the user specified stack. */
		lpStack = pInitStack;
	}

	/* initializes the kernel thread object created just now. */
	lpKernelThread->dwThreadID            = lpKernelThread->dwObjectID;
	lpKernelThread->dwThreadStatus        = dwStatus;
	lpKernelThread->dwThreadPriority      = dwPriority;
	lpKernelThread->dwScheduleCounter     = dwPriority;
	lpKernelThread->dwReturnValue         = 0;
	lpKernelThread->dwTotalRunTime        = 0;
	lpKernelThread->dwTotalMemSize        = 0;

	lpKernelThread->bUsedMath             = FALSE;
	lpKernelThread->dwStackSize           = dwStackSize ? dwStackSize : DEFAULT_STACK_SIZE;
	lpKernelThread->lpInitStackPointer    = (LPVOID)((unsigned long)lpStack + dwStackSize);
	lpKernelThread->KernelThreadRoutine   = lpStartRoutine;
	lpKernelThread->lpRoutineParam        = lpRoutineParam;

	lpKernelThread->ucMsgQueueHeader      = 0;
	lpKernelThread->ucMsgQueueTrial       = 0;
	lpKernelThread->ucCurrentMsgNum       = 0;
	lpKernelThread->nMsgReceived          = 0;
	lpKernelThread->nMsgDroped            = 0;

	lpKernelThread->dwLastError           = 0;
	lpKernelThread->dwWaitingStatus       = OBJECT_WAIT_WAITING;
	lpKernelThread->dwSuspendFlags        = 0;

	/* 
	 * Copy kernel thread's name into kernel thread object. 
	 * Use the stupid method instead of strcpy just for safety.:-)
	 */
	i = 0;
	if(lpszName)
	{
		for(i = 0;i < MAX_THREAD_NAME - 1;i ++)
		{
			if(lpszName[i] == 0)
			{
				break;
			}
			lpKernelThread->KernelThreadName[i] = lpszName[i];
		}
	}
	lpKernelThread->KernelThreadName[i] = 0;  //Set string's terminator.

	/* Initializes user stack. */
	if (lpOwnerProcess)
	{
		lpKernelThread->pUserStack = pUserStack;
		lpKernelThread->user_stk_size = user_stk_size;
	}

	/*
	 * Link the kernel thread to it's owner process.
	 * Must be set before call LinkKernelThread.
	 */
	lpKernelThread->lpOwnProcess = (__COMMON_OBJECT*)lpOwnerProcess;
	if(lpOwnerProcess)
	{
		if(!ProcessManager.LinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
			(__COMMON_OBJECT*)lpOwnerProcess,(__COMMON_OBJECT*)lpKernelThread))
		{
			goto __TERMINAL;
		}
	}

	/*
	 * Initializes the hardware context of the thread.
	 * Different implementations according to thread's type,
	 * so we call them respectively.
	 */
	if (IS_KERNEL_THREAD(lpKernelThread))
	{
		/* All kernel thread starts from Wrapper routine. */
		InitKernelThreadContext(lpKernelThread, __KernelThreadWrapper);
	}
	else
	{
		/* 
		 * User thread should start from routine in user space,
		 * which is specified through lpStartRoutine parameter.
		 */
		InitUserThreadContext(lpKernelThread, lpStartRoutine);
	}

	/* Add it into ready queue if status is READY. */
	if(KERNEL_THREAD_STATUS_READY == dwStatus)
	{
		lpMgr->AddReadyKernelThread((__COMMON_OBJECT*)lpMgr,
			lpKernelThread);
	}
	/* Add it into suspending queue otherwise. */
	else
	{
		if(!lpMgr->lpSuspendedQueue->InsertIntoQueue((__COMMON_OBJECT*)lpMgr->lpSuspendedQueue,
			(__COMMON_OBJECT*)lpKernelThread,dwPriority))
			goto __TERMINAL;
	}
	/* Call the corresponding creating hook routine. */
	lpMgr->CallThreadHook(THREAD_HOOK_TYPE_CREATE,lpKernelThread,
		NULL);
	/* God bless,anything is in place. */
	bSuccess = TRUE;

__TERMINAL:
	if(!bSuccess)
	{
		//Detach from owner process's list.
		if(lpOwnerProcess)
		{
			ProcessManager.UnlinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
				(__COMMON_OBJECT*)lpOwnerProcess,(__COMMON_OBJECT*)lpKernelThread);
		}
		if (NULL != lpKernelThread)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpKernelThread);
		}
		if (NULL == pInitStack)
		{
			if (NULL != lpStack)
			{
				KMemFree(lpStack, KMEM_SIZE_TYPE_ANY, 0);
			}
		}
		return NULL;
	}
	else
	{
		return lpKernelThread;
	}
}

/* Create a new kernel thread. */
static __KERNEL_THREAD_OBJECT* __CreateKernelThread(
	__COMMON_OBJECT* lpThis,
	unsigned long dwStackSize,
	unsigned long dwStatus,
	unsigned long dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	LPVOID lpOwnerProcess,
	LPSTR lpszName)
{
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;

	/* 
	 * Just call the __CreateThread routine,without 
	 * initial stack specified,and no process specified
	 * since the kernel thread belongs to no process.
	 */
	pKernelThread = __CreateThread(
		lpThis,
		dwStackSize,
		NULL,
		dwStatus,
		dwPriority,
		lpStartRoutine,
		lpRoutineParam,
		NULL,
		NULL,
		0,
		lpszName);
	BUG_ON(!IS_KERNEL_THREAD(pKernelThread));
	return pKernelThread;
}

/* Destroy a kernel thread,after it runs over. */
static BOOL __DestroyKernelThread(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpKernel)
{
	BUG_ON((NULL == lpThis) || (NULL == lpKernel));
	/*
	* Just delegate to DestroyObject of ObjectManager,since
	* kernel thread object is one type of kernel object.
	*/
	return ObjectManager.DestroyObject(&ObjectManager, lpKernel);
}

#if defined(__CFG_SYS_PROCESS)
/* 
 * Create a new user thread. 
 * The user stack must be created before call __CreateThread routine.
 */
static __KERNEL_THREAD_OBJECT* __CreateUserThread(
	__COMMON_OBJECT* lpThis,
	DWORD dwStackSize,
	DWORD dwStatus,
	DWORD dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	LPVOID lpOwnerProcess,
	LPSTR lpszName)
{
	__KERNEL_THREAD_OBJECT* pUserThread = NULL;
	unsigned long user_stk_size = dwStackSize;
	LPVOID pUserStack = NULL;
	__VIRTUAL_MEMORY_MANAGER* pvmmgr = NULL;
	__PROCESS_OBJECT* pOwnProcess = (__PROCESS_OBJECT*)lpOwnerProcess;

	/* Own process must be specified. */
	if (NULL == pOwnProcess)
	{
		goto __TERMINAL;
	}

	/* 
	 * Create user stack,one PAGE extra memory is 
	 * reserved for parameter(s) or env information
	 * that xfer to user.
	 */
	if (user_stk_size < DEFAULT_USER_STACK_SIZE)
	{
		user_stk_size = DEFAULT_USER_STACK_SIZE;
	}
	user_stk_size += PAGE_SIZE;

	pvmmgr = pOwnProcess->pMemMgr;
	BUG_ON(NULL == pvmmgr);
	pUserStack = pvmmgr->VirtualAlloc((__COMMON_OBJECT*)pvmmgr,
		(LPVOID)KMEM_USERSTACK_START,
		user_stk_size,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"user_stk",
		NULL);
	if (NULL == pUserStack)
	{
		/* Seldom occur,show a warning. */
		_hx_printk("%s:failed to create user stk.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Just call the worker routine. */
	pUserThread = __CreateThread(lpThis, 0, NULL,
		dwStatus, dwPriority, lpStartRoutine, lpRoutineParam,
		lpOwnerProcess, pUserStack, user_stk_size,
		lpszName);
	if (NULL == pUserThread)
	{
		goto __TERMINAL;
	}
	BUG_ON(!IS_USER_THREAD(pUserThread));

__TERMINAL:
	return pUserThread;
}

/* 
 * Destroy a user thread.
 * It just delegates the operation to DestroyObject routine of
 * ObjectManager.
 */
static BOOL __DestroyUserThread(__COMMON_OBJECT* pThis, __COMMON_OBJECT* pThread)
{
	BUG_ON((NULL == pThis) || (NULL == pThread));
	/*
	* Just delegate to DestroyObject of ObjectManager,since
	* kernel thread object is one type of kernel object.
	*/
	return ObjectManager.DestroyObject(&ObjectManager, pThread);
}

#endif //__CFG_SYS_PROCESS

//Enable or disable suspending on a specified kernel thread.
//If bEnable is TRUE,then enable the suspending on the specified thread,and do a suspending
//if there is pending one(suspending counter > 0).
//If bEnable is FALSE,then disable the suspending operation on the specified thread.
static BOOL kEnableSuspend(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread,BOOL bEnable)
{
	__KERNEL_THREAD_OBJECT*  lpKernelThread   = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KERNEL_THREAD_MANAGER* lpManager        = (__KERNEL_THREAD_MANAGER*)lpThis;
	DWORD                    dwFlags          = 0;

	if((NULL == lpKernelThread) || (NULL == lpManager))
	{
		return FALSE;
	}
	if(!bEnable)
	{
		/* Should disable the suspending of the specified kernel thread. */
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
		lpKernelThread->dwSuspendFlags |= SUSPEND_FLAG_DISABLE;
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
	}
	else
	{
		/* Enable the suspending. */
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
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
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
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
static BOOL kSuspendKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*    lpManager        = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*     lpKernelThread   = (__KERNEL_THREAD_OBJECT*)lpThread;
	DWORD                       dwFlags          = 0;

	if((NULL == lpManager) || (NULL == lpKernelThread))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
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
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
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
static BOOL kResumeKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*     lpManager      = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*      lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	DWORD                        dwFlags;

	if((NULL == lpManager) || (NULL == lpKernelThread))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
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
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
	//Do reschedule if in kernel thread context.
	if(!(IN_INTERRUPT() || IN_SYSINITIALIZATION()))
	{
		lpManager->ScheduleFromProc(NULL);
	}
	return TRUE;
}

#ifdef __CFG_SYS_IS

/* Try to combine the in-procedure and interrupt scheduling together. */
static void __ScheduleFromProc(__KERNEL_THREAD_CONTEXT* pContext)
{
	unsigned long ulFlags;
	__DISABLE_LOCAL_INTERRUPT(ulFlags);
	__asm {
		int 0x80
	}
	__RESTORE_LOCAL_INTERRUPT(ulFlags);
}

/* 
 * The following 2 routines apply only in inline schedule mode.
 * ScheduleFromProc's implementation.
 * This routine can be called anywhere that re-schedule is required,under process context.
 * The ScheduleFromInt(later one) should be called in interrupt context.
 * Critical section operations should be optimized. 
 */
static VOID ScheduleFromProc(__KERNEL_THREAD_CONTEXT* lpContext)
{
	__KERNEL_THREAD_OBJECT* lpCurrent = NULL;
	__KERNEL_THREAD_OBJECT* lpNew = NULL;
	unsigned long ulFlags = 0;

	/* 
	 * The following procedure must not be interrupted by local
	 * interrupt.
	 */
	__DISABLE_LOCAL_INTERRUPT(ulFlags);
	lpCurrent = __CURRENT_KERNEL_THREAD;
	/* Do different actions according to status,use kernel thread's spin lock. */
	__ACQUIRE_SPIN_LOCK(lpCurrent->spin_lock);
	switch(lpCurrent->dwThreadStatus)
	{
	case KERNEL_THREAD_STATUS_RUNNING:
		/* Get a ready kernel thread from local ready queue. */
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);
		if(NULL == lpNew)
		{
			/*
			* NO ready kernel thread's priority is higher than current one,
			* just let current continue to run.
			*/
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;
			__RELEASE_SPIN_LOCK(lpCurrent->spin_lock);
			__RESTORE_LOCAL_INTERRUPT(ulFlags);
			return;
		}
		else
		{
			/* Should swap out current kernel thread and run next ready one. */
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrent);
			__RELEASE_SPIN_LOCK(lpCurrent->spin_lock);
			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			__CURRENT_KERNEL_THREAD = lpNew;
			//Call schedule hook before switch.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);
			/* Save current kernel thread's context,and switch to next one. */
			__SaveAndSwitch(lpCurrent, lpNew);
			__RESTORE_LOCAL_INTERRUPT(ulFlags);
			return;
		}
	case KERNEL_THREAD_STATUS_READY:
		/* 
		 * The kernel thread is waken up immediately after block but 
		 * before the ScheduleFromProc is called.
		 * Try to get a ready thread with higher priority than current one.
		 * A essential assumption is,the kernel thread must be in ready
		 * queue,if it's status is READY.
		 */
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpCurrent->dwThreadPriority);
		/* At least one ready kernel exist in ready queue. */
		BUG_ON(NULL == lpNew);
		/* The same one,let it continue to run. */
		if(lpNew == lpCurrent)
		{
			lpCurrent->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			__RELEASE_SPIN_LOCK(lpCurrent->spin_lock);
			__RESTORE_LOCAL_INTERRUPT(ulFlags);
			return;
		}
		else
		{
			__RELEASE_SPIN_LOCK(lpCurrent->spin_lock);
			/* Switch to the new fetched kernel thread. */
			lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			__CURRENT_KERNEL_THREAD = lpNew;
			//Call schedule hook routine.
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
				lpCurrent,lpNew);
			__SaveAndSwitch(lpCurrent, lpNew);
			__RESTORE_LOCAL_INTERRUPT(ulFlags);
			return;
		}
	case KERNEL_THREAD_STATUS_BLOCKED:
	case KERNEL_THREAD_STATUS_SUSPENDED:
	case KERNEL_THREAD_STATUS_TERMINAL:
	case KERNEL_THREAD_STATUS_SLEEPING:
		/* 
		 * Current kernel thread is put into waiting queue,
		 * so get a new ready thread to run. 
		 */
		lpNew = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);
		/* At least idle thread is ready. */
		BUG_ON(NULL == lpNew);
		__RELEASE_SPIN_LOCK(lpCurrent->spin_lock);
		lpNew->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNew->dwTotalRunTime += SYSTEM_TIME_SLICE;
		__CURRENT_KERNEL_THREAD = lpNew;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_ENDSCHEDULE | THREAD_HOOK_TYPE_BEGINSCHEDULE,
			lpCurrent,lpNew);
		__SaveAndSwitch(lpCurrent, lpNew);
		__RESTORE_LOCAL_INTERRUPT(ulFlags);
		return;
	default:
		BUG();
		break;
	}
}

/* 
 * ScheduleFromInt,will be invoked in INTERRUPT context,to schedule all
 * kernel thread(s) in system.
 * The corresponding routine,ScheduleFromProc,is called in PROCESS context
 * to do schedule.
 */
static VOID ScheduleFromInt(__COMMON_OBJECT* lpThis,LPVOID lpESP)
{
	__KERNEL_THREAD_OBJECT* lpNextThread = NULL;
	__KERNEL_THREAD_OBJECT* lpCurrentThread = NULL;
	__KERNEL_THREAD_MANAGER* lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;

	/* Parameters checking. */
	if((NULL == lpThis) || (NULL == lpESP))
	{
		return;
	}

	lpCurrentThread = __CURRENT_KERNEL_THREAD;
	if(NULL == lpCurrentThread)
	{
		/* The routine is called first time at current processor. */
		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);
		if(NULL == lpNextThread)
		{
			/* 
			 * when the interrupt is enabled and in process of system initialization,
			 * there maybe not any kernel thread at all,so just let it go. 
			 */
			return;
		}
		__CURRENT_KERNEL_THREAD = lpNextThread;
		lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
		/* Switch to this thread. */
		//__SwitchTo(lpNextThread->lpKernelThreadContext);
		__SwitchTo(NULL, lpNextThread);
	}
	else if (lpCurrentThread->ulThreadFlags)
	{
		/* 
		 * Handle flag(s) of current thread. 
		 * In most case,the current thread will
		 * be killed,hang up,or change to another
		 * processor.
		 */
		HandleThreadFlags((__COMMON_OBJECT*)lpCurrentThread);

		/* Fetch another ready thread to run. */
		lpNextThread = KernelThreadManager.GetScheduleKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			0);
		/* At least idle thread is in ready queue. */
		BUG_ON(NULL == lpNextThread);

		/* Should Check flags of the ready thread. */
		/* Will be implemented later. */

		__CURRENT_KERNEL_THREAD = lpNextThread;
		lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
		lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
		//Call schedule hook.
		KernelThreadManager.CallThreadHook(
			THREAD_HOOK_TYPE_BEGINSCHEDULE, NULL, lpNextThread);
		/* Switch to this thread. */
		//__SwitchTo(lpNextThread->lpKernelThreadContext);
		__SwitchTo(NULL, lpNextThread);
	}
	else
	{
		lpCurrentThread = __CURRENT_KERNEL_THREAD;
		//Saves the context of current kernel thread.
		lpCurrentThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpESP;
		switch(lpCurrentThread->dwThreadStatus)
		{
			/* 
			 * If a kernel thread's status is one of the following,it means the
			 * kernel thread is interrupted in the time between CHANGING IT'S STATUS
			 * and SWAPE OUT.In this scenario we just let the current thread to go,
			 * it will be swaped out immediately in the ScheduleFromProc routine,which
			 * will be called by the current thread itself.
			 */
		case KERNEL_THREAD_STATUS_BLOCKED:     //In process of wait shared resource.
		case KERNEL_THREAD_STATUS_TERMINAL:    //In process of termination.
		case KERNEL_THREAD_STATUS_SLEEPING:    //In process of falling in sleep.
		case KERNEL_THREAD_STATUS_READY:       //Wakeup immediately in another interrupt or by another CPU.
			lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);
			//__SwitchTo(lpCurrentThread->lpKernelThreadContext);
			__SwitchTo(NULL, lpCurrentThread);
			break;
		case KERNEL_THREAD_STATUS_SUSPENDED:
			/* 
			 * But for kernel thread's suspending,it's a little different.
			 * When the suspending is generated by the thread itself,then
			 * we can follow the processing mechanism as BLOCK status.But if
			 * the suspending is made by other kernel thread running in other
			 * CPU,if we let the suspended thread to go,it may lead serious
			 * issue.
			 * So if the thread's status is SUSPENDED,we just swape it out and
			 * fetch one READY thread to run.
			 */
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0);
			/* At least idle thread exists in ready queue. */
			BUG_ON(NULL == lpNextThread);
			lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
			lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
			lpMgr->CurrentKernelThread[__CURRENT_PROCESSOR_ID] = lpNextThread;
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
			//__SwitchTo(lpNextThread->lpKernelThreadContext);
			__SwitchTo(lpCurrentThread, lpNextThread);
			break;
		case KERNEL_THREAD_STATUS_RUNNING:
			lpNextThread = KernelThreadManager.GetScheduleKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpCurrentThread->dwThreadPriority);
			if(NULL == lpNextThread)
			{
				/* Current running kernel thread has the top priority. */
				lpCurrentThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpCurrentThread);

				//__SwitchTo(lpCurrentThread->lpKernelThreadContext);
				__SwitchTo(NULL, lpCurrentThread);
				return;
			}
			else
			{
				/* 
				 * More priority kernel thread exists,switch to it and
				 * save current one into ready queue. 
				 */
				lpCurrentThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
				KernelThreadManager.AddReadyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					lpCurrentThread);

				lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
				lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
				lpMgr->CurrentKernelThread[__CURRENT_PROCESSOR_ID] = lpNextThread;
				KernelThreadManager.CallThreadHook(
					THREAD_HOOK_TYPE_BEGINSCHEDULE,NULL,lpNextThread);
				//__SwitchTo(lpNextThread->lpKernelThreadContext);
				__SwitchTo(lpCurrentThread, lpNextThread);
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
	if(NULL == __CURRENT_KERNEL_THREAD)   //The routine is called first time.
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
		__CURRENT_KERNEL_THREAD = lpNextThread;
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
		lpCurrentThread = __CURRENT_KERNEL_THREAD;
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
			__CURRENT_KERNEL_THREAD = lpNextThread;
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
				__CURRENT_KERNEL_THREAD = lpNextThread;
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
				__CURRENT_KERNEL_THREAD = lpNextThread;
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

/* 
 * Change the specified kernel thread's priority level,or change 
 * current kernel thread's priority level if no kernel thread is 
 * specified by lpKernelThread.
 * The old priority level will be returned.
 */
static DWORD  kSetThreadPriority(__COMMON_OBJECT* lpKernelThread,DWORD dwPriority)
{
	__KERNEL_THREAD_OBJECT*    lpThread = NULL;
	DWORD                      dwOldPri = PRIORITY_LEVEL_IDLE;
	DWORD                      dwFlags  = 0;

	if (NULL == lpKernelThread)
	{
		lpThread = __CURRENT_KERNEL_THREAD;
	}
	else
	{
		lpThread = (__KERNEL_THREAD_OBJECT*)lpKernelThread;
	}
	/* Validate the priority level's value. */
	if (dwPriority > MAX_KERNEL_THREAD_PRIORITY)
	{
		return dwOldPri;
	}
	/* Can not be interrupted when exchange the priority level value. */
	__ENTER_CRITICAL_SECTION_SMP(lpThread->spin_lock, dwFlags);
	dwOldPri = lpThread->dwThreadPriority;
	lpThread->dwThreadPriority = dwPriority;
	__LEAVE_CRITICAL_SECTION_SMP(lpThread->spin_lock, dwFlags);

	return dwOldPri;
}

/* 
 * Get the specified kernel thread's priority level. 
 * Current kernel thread's priority level will be returned
 * if lpKernelThread is NULL.
 */
static DWORD  GetThreadPriority(__COMMON_OBJECT* lpKernelThread)
{
	__KERNEL_THREAD_OBJECT*    lpThread = NULL;
	DWORD dwOldPriority = 0;
	unsigned long ulFlags = 0;

	if(NULL == lpKernelThread)
	{
		lpThread = __CURRENT_KERNEL_THREAD;
	}
	else
	{
		lpThread = (__KERNEL_THREAD_OBJECT*)lpKernelThread;
	}
	__ENTER_CRITICAL_SECTION_SMP(lpThread->spin_lock, ulFlags);
	dwOldPriority = lpThread->dwThreadPriority;
	__LEAVE_CRITICAL_SECTION_SMP(lpThread->spin_lock, ulFlags);

	return dwOldPriority;
}

/* 
 * Terminate a kernel thread.
 * It just sets the KILLED bit of the thread's
 * flag(ulFlags) as 1. Other routines of thread management
 * system,such as AddReadyKernelThread,will put the thread
 * into killed queue.
 * Idle thread(s) will destroy all thread object(s) in killed
 * queue.
 * Contention case will raise if we destroy thread object in
 * this routine directly,consider the scenario that target 
 * thread is running on other CPU.
 */
static DWORD  TerminateKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread,DWORD dwExitCode)
{
	__KERNEL_THREAD_MANAGER* lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT* pTargetThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KERNEL_THREAD_OBJECT* pCurrentThread = NULL;
	int ncpu = 0;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == lpManager);
	//Get current kernel thread object.
	pCurrentThread = lpManager->CurrentKernelThread[__CURRENT_PROCESSOR_ID];
	if ((NULL == pTargetThread) || (pCurrentThread == pTargetThread))
	{
		/* Current thread suicide:-) */
		KernelThreadClean((__COMMON_OBJECT*)pCurrentThread, dwExitCode);
		/* Should never reach the following clause. */
		return 0;
	}

	/* Kill other thread.The system IDLE thread(s) should never be killed. */
	for (ncpu = 0; ncpu < MAX_CPU_NUM; ncpu++)
	{
		if (pTargetThread == lpManager->IdleKernelThread[ncpu])
		{
			/* Attempt to kill IDLE kernel thread. */
			return 0;
		}
	}

	/* Just set the KILLED flag of target thread. */
	__ENTER_CRITICAL_SECTION_SMP(pTargetThread->spin_lock, ulFlags);
	pTargetThread->ulThreadFlags |= KERNEL_THREAD_FLAGS_KILLED;
	__LEAVE_CRITICAL_SECTION_SMP(pTargetThread->spin_lock, ulFlags);

	return 0;
}

/*
 * Sleep Routine.
 * This routine do the following:
 *  1. Updates the dwNextWakeupTick value of kernel thread manager;
 *  2. Modifies the current kernel thread's status to SLEEPING;
 *  3. Puts the current kernel thread into sleeping queue of kernel thread manager;
 *  4. Schedules another kernel thread to run;
 *  5. If the specified sleep time is 0,then do a re-schedule.
 */
static BOOL kSleep(__COMMON_OBJECT* lpThis,/*__COMMON_OBJECT* lpKernelThread,*/DWORD dwMillisecond)
{
	__KERNEL_THREAD_MANAGER*           lpManager      = (__KERNEL_THREAD_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*            lpKernelThread = NULL;
	DWORD                              dwTmpCounter   = 0;
	unsigned long ulFlags1, ulFlags2;

	if(NULL == lpManager)    //Parameters check.
	{
	     return FALSE;
	}
	
	//Just re-schedule all kernel thread(s) if the sleep time is less than
	//system slice,this may cause issues,suppose that one kernel thread want
	//to pausing execution for 10ms,to yeild CPU to other kernel threads,
	//then it calls Sleep routine by giving 10 as parameter.The objective
	//will not achieved since the Sleep routine will return immediately.
	//A feasible approach is round any sleep time less than system time
	//slice to one SYSTEM_TIME_SLICE.
	//But we keep it is since no problem encountered up to now.
	if(dwMillisecond < SYSTEM_TIME_SLICE)
	{
		lpManager->ScheduleFromProc(NULL);
		return TRUE;
	}

	lpKernelThread = lpManager->CurrentKernelThread[__CURRENT_PROCESSOR_ID];
	if(NULL == lpKernelThread)
	{
		/* The routine is called in system initializing process. */
		BUG();
		return FALSE;
	}
	dwTmpCounter =  dwMillisecond / SYSTEM_TIME_SLICE;
	
	/* 
	 * Obtain KernelThreadManager's lock and kernel thread object's lock
	 * respectively,since we will update these 2 objects.
	 * Get the container's lock first.
	 */
	__ENTER_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags1);
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags2);
	/* 
	 * dwTmpCounter countains the tick counter value 
	 * when this kernel thread who calls this routine
	 * must be waken up.
	 */
	dwTmpCounter += System.dwClockTickCounter;
	if((0 == lpManager->dwNextWakeupTick) || (lpManager->dwNextWakeupTick > dwTmpCounter))
	{
		lpManager->dwNextWakeupTick = dwTmpCounter;     //Update dwNextWakeupTick value.
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_SLEEPING;
	/* 
	 * Calculates the priority of the current kernel thread in 
	 * the sleeping queue.
	 */
	dwTmpCounter = MAX_DWORD_VALUE - dwTmpCounter;
	lpManager->lpSleepingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpManager->lpSleepingQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		dwTmpCounter);
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,ulFlags2);
	__LEAVE_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags1);

	/* Trigger a rescheduling. */
	lpManager->ScheduleFromProc(NULL);
	return TRUE;
}

//CancelSleep Routine.
static BOOL CancelSleep(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpKernelThread)
{
	return FALSE;
}

//SetLastError.
static DWORD  _SetLastError(/*__COMMON_OBJECT* lpKernelThread,*/DWORD dwNewError)
{
	DWORD  dwOldError = 0;
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	unsigned long ulFlags = 0;

	/* Maybe called in process of system initialization. */
	BUG_ON(NULL == pKernelThread);
	/* Can not be interrupted. */
	__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, ulFlags);
	dwOldError = __CURRENT_KERNEL_THREAD->dwLastError;
	__CURRENT_KERNEL_THREAD->dwLastError = dwNewError;
	__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, ulFlags);
	return dwOldError;
}

//GetLastError.
static DWORD _GetLastError(/*__COMMON_OBJECT* lpKernelThread*/)
{
	return __CURRENT_KERNEL_THREAD->dwLastError;
}

//kGetThreadID.
static DWORD kGetThreadID(__COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)  //Return current kernel thread's ID.
	{
		return __CURRENT_KERNEL_THREAD->dwThreadID;
	}

	return ((__KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadID;
}

//GetThreadStatus.
static DWORD GetThreadStatus(__COMMON_OBJECT* lpKernelThread)
{
	if(NULL == lpKernelThread)  //Return current kernel thread's ID.
	{
		return __CURRENT_KERNEL_THREAD->dwThreadStatus;
	}

	return ((__KERNEL_THREAD_OBJECT*)lpKernelThread)->dwThreadStatus;
}

//MsgQueueFull.
static BOOL MsgQueueFull(__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread = NULL;

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

	if(NULL == lpMsg)
	{
		goto __TERMINAL;
	}

	/* Send message to current kernel thread. */
	if(NULL == lpThread)
	{
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	else
	{
		/* Send message to the specified target thread. */
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	if(MsgQueueFull((__COMMON_OBJECT*)lpKernelThread))
	{
		lpKernelThread->nMsgDroped += 1;
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
		/* Show out a warning for the first time of message queue full. */
		if (lpKernelThread->nMsgDroped == 1)
		{
			_hx_printf("[k-warning]:Message queue of kthread [%s] is full,msg[cmd:%d,sendor:%s] droped.\r\n",
				lpKernelThread->KernelThreadName,
				lpMsg->wCommand,
				__CURRENT_KERNEL_THREAD->KernelThreadName);
		}
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
	lpKernelThread->nMsgReceived += 1;

	/* Wake up kernel thrad(s) who is(are) waiting for message. */
	lpNewThread = (__KERNEL_THREAD_OBJECT*)lpKernelThread->lpMsgWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
		NULL);
	if(lpNewThread)
	{
		lpNewThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		lpNewThread->ucAligment = 0;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpNewThread);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	
	/* If in kernel thread context,then re-schedule kernel thread. */
	if(IN_KERNELTHREAD())
	{
		if (KERNEL_THREAD_SCHEDULING_ENABLE == KernelThreadManager.scheduling_enabled)
		{
			KernelThreadManager.ScheduleFromProc(NULL);
		}
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

	if(NULL == lpMsg)
	{
		return FALSE;
	}
	/* Get message from current kernel thread's queue. */
	if(NULL == lpThread)
	{
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	else
	{
		/* Get message from the specified kernel thread. */
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock,dwFlags);
	if(MsgQueueEmpty((__COMMON_OBJECT*)lpKernelThread))
	{
		/* Current message queue is empty,should wait. */
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MESSAGE;
		lpKernelThread->lpMsgWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);
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
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
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

	if(NULL == lpMsg)
	{
		return FALSE;
	}
	if(NULL == lpThread)
	{
		/* Peek message from current kernel thread's queue. */
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	else
	{
		/* Peek message from the specified kernel thread. */
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	if(MsgQueueEmpty((__COMMON_OBJECT*)lpKernelThread))
	{
		/* Current message queue is empty. */
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		return FALSE;
	}
	else
	{
		/* Has message in queue,fetch one. */
		lpMsg->wCommand     = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wCommand;
		lpMsg->wParam       = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].wParam;
		lpMsg->dwParam      = lpKernelThread->KernelThreadMsg[lpKernelThread->ucMsgQueueHeader].dwParam;
		lpKernelThread->ucMsgQueueHeader ++;
		if(MAX_KTHREAD_MSG_NUM == lpKernelThread->ucMsgQueueHeader)
		{
			lpKernelThread->ucMsgQueueHeader = 0;
		}
		lpKernelThread->ucCurrentMsgNum --;
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
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

	if(NULL == lpThis)
	{
		return FALSE;
	}
	lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	//If lpThread is NULL,then lock the current kernel thread.
	lpKernelThread = (NULL == lpThread) ? lpManager->CurrentKernelThread[__CURRENT_PROCESSOR_ID] : 
		(__KERNEL_THREAD_OBJECT*)lpThread;
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	if(KERNEL_THREAD_STATUS_RUNNING != lpKernelThread->dwThreadStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		return FALSE;
	}
	/*
	 * Once mark the status of the target thread to BLOCKED,it will never be
	 * switched out.
	 */
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	return TRUE;
}

//
// Unlockes a kernel thread who is locked by LockKernelThread routine.
//
static VOID UnlockKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_MANAGER*               lpManager       = NULL;
	__KERNEL_THREAD_OBJECT*                lpKernelThread  = NULL;
	DWORD                                  dwFlags         = 0;

	if(NULL == lpThis)
	{
		return;
	}

	lpManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	lpKernelThread = (NULL == lpThread) ? lpManager->CurrentKernelThread[__CURRENT_PROCESSOR_ID] : 
		(__KERNEL_THREAD_OBJECT*)lpThread;
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	/* Just return if the kernel thread's status is not blocked. */
	if(KERNEL_THREAD_STATUS_BLOCKED != lpKernelThread->dwThreadStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		return;
	}
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	return;
}

/*
 * Return the current running kernel thread,in current CPU.
 * The current kernel threads are different in case of SMP,one kernel thread
 * runs on each CPU and the total number of current running kernel thread equal
 * to the CPU number in system.
 */
static __COMMON_OBJECT* _GetCurrentKernelThread()
{
	return (__COMMON_OBJECT*)__CURRENT_KERNEL_THREAD;
}

#if defined(__CFG_SYS_SMP)
/*
 * Change the specified kernel thread's cpu affinity value,the old 
 * affinity value will be returned.
 */
static unsigned int _ChangeAffinity(__COMMON_OBJECT* lpTargetThread, unsigned int newAffinity)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpTargetThread;
	int oldAffinity = -1;
	unsigned long ulFlags = 0;

	/* If no target kernel thread is specified,just apply on current one. */
	if (NULL == lpKernelThread)
	{
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	if (newAffinity >= MAX_CPU_NUM)
	{
		goto __TERMINAL;
	}

	/*
	 * Just change the affinity value.
	 * It will be moved to appropriate ready queue corresponding the processor
	 * when AddReadyKernelThread is called.
	 */
	oldAffinity = __ATOMIC_SET(&lpKernelThread->cpuAffinity, newAffinity);
#if 0
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
	oldAffinity = lpKernelThread->cpuAffinity;
	lpKernelThread->cpuAffinity = newAffinity;
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
#endif

__TERMINAL:
	return oldAffinity;
}
#endif

/* Start to schedule kernel thread(s) after OS's initialization phase. */
static BOOL StartScheduling()
{
	__KERNEL_THREAD_OBJECT* lpNextThread = NULL;
#if defined(__CFG_SYS_SMP)
	uint64_t cpuStartFlags = 0;
	unsigned long ulFlags = 0;
#endif 

	/* 
	 * Current kernel thread of current processor must be NULL,since the
	 * scheduling process is not started yet.
	 */
	BUG_ON(NULL != __CURRENT_KERNEL_THREAD);
#if defined(__CFG_SYS_SMP)
	/* 
	 * Mark the current CPU's start flag to 1,indicates the current
	 * CPU is ready to schedule.
	 */
	__ENTER_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags);
	KernelThreadManager.cpuStartFlags |= (uint64_t)(1 << __CURRENT_PROCESSOR_ID);
	__LEAVE_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags);

	/* Wait all CPU(s) in system ready to schedule. */
	cpuStartFlags = ProcessorManager.cpuMap;

	while (TRUE)
	{
		__ENTER_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags);
		if (KernelThreadManager.cpuStartFlags == cpuStartFlags)
		{
			__LEAVE_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags);
			break;
		}
		__LEAVE_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, ulFlags);
		__MicroDelay(10);
	}
#endif //__CFG_SYS_SMP.

	/* Mark the end of system initialization. */
	System.bSysInitialized = TRUE;
	/* Fetch a ready kernel thread from ready queue. */
	lpNextThread = KernelThreadManager.GetScheduleKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0);
	/* At least one kernel thread should in the ready queue. */
	BUG_ON(NULL == lpNextThread);

	__CURRENT_KERNEL_THREAD = lpNextThread;
	lpNextThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;
	lpNextThread->dwTotalRunTime += SYSTEM_TIME_SLICE;
	/* Call the begin scheduling hook routine. */
	KernelThreadManager.CallThreadHook(
		THREAD_HOOK_TYPE_BEGINSCHEDULE, NULL, lpNextThread);
	/* Switch to the target kernel thread. */
	_hx_printk("CPU[%d]: Switch to kernel thread[%s]\r\n", 
		__CURRENT_PROCESSOR_ID,
		lpNextThread->KernelThreadName);
	__SwitchTo(NULL, lpNextThread);

	/* 
	 * Should not reach here,since the executing path will divert to
	 * the kernel thread lpNextThread.
	 */
	BUG();
	return FALSE;
}

/* 
 * Enable or disable kernel thread scheduling function. 
 * It just change the scheduling_enable flag in kernel thread manager according
 * bEnabled,and the old flag will be returned.
 * A IPI will be sent to all processor(s) in system but current one,to trigger
 * a new schedule,if bEnabled is TRUE.
 * Kernel code or applications may disable the scheduling before
 * apply any kernel API call,and restore it's old value after that.
 */
static BOOL EnableScheduling(BOOL bEnable)
{
	__atomic_t schedule_flag = ATOMIC_INIT_VALUE;
#if defined(__CFG_SYS_SMP)
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
#endif

	/* Set scheduling enable flags according bEnable. */
	schedule_flag = bEnable ? KERNEL_THREAD_SCHEDULING_ENABLE : KERNEL_THREAD_SCHEDULING_DISABLE;
	schedule_flag = __ATOMIC_SET(&KernelThreadManager.scheduling_enabled, schedule_flag);

	/* Trigger a new scheduling for all processors if scheduling is enabled. */
	if (bEnable)
	{
#if defined(__CFG_SYS_SMP)
		/* Send IPI to all processor(s) but current one. */
		pSpec = ProcessorManager.GetCurrentProcessorSpecific();
		BUG_ON(NULL == pSpec);
		pIntCtrl = pSpec->pIntCtrl;
		BUG_ON(NULL == pIntCtrl);
		BUG_ON(NULL == pIntCtrl->Send_IPI);
		for (int processor = 0; processor < ProcessorManager.nProcessorNum; processor++)
		{
			/* Skip current processor. */
			if (__CURRENT_PROCESSOR_ID != processor)
			{
				pIntCtrl->Send_IPI(pIntCtrl, processor, IPI_TYPE_NEWTHREAD);
			}
		}
#endif
		/* Trigger a schedule on current processor. */
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return (KERNEL_THREAD_SCHEDULING_ENABLE == schedule_flag ? TRUE : FALSE);
}

/* Clean up a killed thread before destroy it. */
static VOID __KilledThreadCleanUp(__COMMON_OBJECT* lpThread)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KERNEL_THREAD_OBJECT* lpWaitingThread = NULL;
	__PRIORITY_QUEUE* lpWaitingQueue = NULL;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == lpKernelThread);

	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
	/* Set the killed thread's status as terminal and put it into terminal queue. */
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_TERMINAL;
	KernelThreadManager.lpTerminalQueue->InsertIntoQueue((__COMMON_OBJECT*)KernelThreadManager.lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	/*
	 * wakeup all kernel thread(s) who waiting for this kernel thread
	 * object.
	 */
	lpWaitingQueue = lpKernelThread->lpWaitingQueue;
	lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while (lpWaitingThread)
	{
		lpWaitingThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		lpWaitingThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		/* 
		 * The thread(s) waiting on the killed thread will get a 
		 * resource deleted result.
		 */
		lpWaitingThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpWaitingThread);  //Add to ready queue.
		lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);

	return;
}

/* 
 * Scan killed queue and destroy any threads in it. 
 * It will be invoked in IDLE thread(s).
 */
static void __ProcessKilledQueue(__COMMON_OBJECT* pThis)
{
	__PRIORITY_QUEUE* pKilledQueue = NULL;
	__KERNEL_THREAD_MANAGER* pMgr = (__KERNEL_THREAD_MANAGER*)pThis;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;

	BUG_ON(NULL == pMgr);
	pKilledQueue = pMgr->lpKilledQueue;
	pKernelThread = (__KERNEL_THREAD_OBJECT*)pKilledQueue->GetHeaderElement(
		(__COMMON_OBJECT*)pKilledQueue,
		NULL);
	while (pKernelThread)
	{
		/* Release all other resources belong to the killed thread. */
		__KilledThreadCleanUp((__COMMON_OBJECT*)pKernelThread);

		/* Destroy this thread. */
		if (IS_KERNEL_THREAD(pKernelThread))
		{
			pMgr->DestroyKernelThread((__COMMON_OBJECT*)pMgr,
				(__COMMON_OBJECT*)pKernelThread);
		}
		else
		{
			/* Must be user thread. */
			BUG_ON(!IS_USER_THREAD(pKernelThread));
			pMgr->DestroyUserThread((__COMMON_OBJECT*)pMgr,
				(__COMMON_OBJECT*)pKernelThread);
		}
		_hx_printk("Thread killed brutely.\r\n");
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pKilledQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pKilledQueue,
			NULL);
	}
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
	0,                                               //dwCurrentIRQL.
	ATOMIC_INIT_VALUE,                               //scheduling_enabled.
	{0},                                             //CurrentKernelThread.
	{0},                                             //IdelKernelThread.
#if defined(__CFG_SYS_SMP)
	0,                                               //cpuStartFlags.
	SPIN_LOCK_INIT_VALUE,                            //spin_lock.
#endif 
	NULL,                                            //lpSuspendedQueue.
	NULL,                                            //lpSleepingQueue.
	NULL,                                            //lpTerminalQueue.
	NULL,                                            //lpKilledQueue.
	{0},                                             //KernelThreadReadyQueue array.
	0,                                               //dwNextWakeupTick.

	NULL,                                            //lpCreateHook.
	NULL,                                            //lpEndScheduleHook.
	NULL,                                            //lpBeginScheduleHook.
	NULL,                                            //lpTerminalHook.
	SetThreadHook,                                   //SetThreadHook.
	CallThreadHook,                                  //CallThreadHook.

	GetScheduleKernelThread,                         //GetScheduleKernelThread.
	AddReadyKernelThread,                            //AddReadyKernelThread.
	KernelThreadMgrInit,                             //Initialize routine.
	__CreateKernelThread,                            //CreateKernelThread routine.
#if defined(__CFG_SYS_PROCESS)
	__CreateUserThread,                              //CreateUserThread.
	__DestroyUserThread,                             //DestroyUserThread.
#endif

	__DestroyKernelThread,                            //DestroyKernelThread routine.
	kEnableSuspend,                                   //EnableSuspend routine.
	kSuspendKernelThread,                             //SuspendKernelThread routine.
	kResumeKernelThread,                              //ResumeKernelThread routine.

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
	kSleep,                                          //Sleep routine.
	CancelSleep,                                     //CancelSleep routine.
	GetLastError,                                    //GetLastError routine.
	SetLastError,                                    //SetLastError routine.
	GetThreadID,                                     //GetThreadID routine.
	GetThreadStatus,                                 //GetThreadStatus routine.
	MgrSendMessage,                                  //SendMessage routine.
	MgrGetMessage,                                   //GetMessage routine.
	MgrPeekMessage,                                  //PeekMessage routine.
	MsgQueueFull,                                    //MsgQueueFull routine.
	MsgQueueEmpty,                                   //MsgQueueEmpty routine.
	LockKernelThread,                                //LockKernelThread routine.
	UnlockKernelThread,                              //UnlockKernelThread routine.
	_GetCurrentKernelThread,                         //GetCurrentKernelThread.
#if defined(__CFG_SYS_SMP)
	_ChangeAffinity,                                  //ChangeAffinity.
#endif
	StartScheduling,                                 //StartScheduling.
	EnableScheduling,                                //EnableScheduling.
	__ProcessKilledQueue,                            //ProcessKilledQueue.
};

/**************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/
//
//Dispatch a message to a message(event) handler.
//
DWORD DispatchMessage(__KERNEL_THREAD_MESSAGE* lpMsg,__KERNEL_THREAD_MESSAGE_HANDLER lpHandler)
{
	return lpHandler(lpMsg->wCommand,lpMsg->wParam,lpMsg->dwParam);
}
