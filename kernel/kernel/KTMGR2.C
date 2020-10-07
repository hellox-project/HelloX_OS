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
//                                by KTMGR.C file.
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

#include "StdAfx.h"
#include "ktmgr2.h"
#include "hellocn.h"
#include "mlayout.h"
#include "process.h"

/* Handler to process thread's flag(s). */
void HandleThreadFlags(__COMMON_OBJECT* pThread)
{
	__KERNEL_THREAD_OBJECT* pKernelThread = (__KERNEL_THREAD_OBJECT*)pThread;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == pKernelThread);

	/* 
	 * Flag(s) must be cleared before process,since 
	 * it may trigger re-enter of flag processing. 
	 */
	ulFlags = pKernelThread->ulThreadFlags;
	pKernelThread->ulThreadFlags = 0;
	if (ulFlags & KERNEL_THREAD_FLAGS_KILLED)
	{
		/* 
		 * Put current thread into killed queue,so it could 
		 * be destroyed by IDLE kernel thread later.
		 * The thread object is keep it's status without any
		 * change.
		 */
		KernelThreadManager.lpKilledQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)KernelThreadManager.lpKilledQueue,
			pThread,
			0);
	}
	if (ulFlags & KERNEL_THREAD_FLAGS_HUNG)
	{

	}
	if (ulFlags & KERNEL_THREAD_FLAGS_AFFINITYCHANGED)
	{

	}
	return;
}

/*
 * A helper routine,to check if a given kernel thread should be suspended.
 * It will be invoked by GetReadyKernelThread.
 */
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

/*
 * This routine tries to get a schedulable kernel thread from current processor's 
 * ready queue.
 * The target kernel thread's priority must larger or equal dwPriority.
 * If can not find,returns NULL.
 */
__KERNEL_THREAD_OBJECT* GetScheduleKernelThread(__COMMON_OBJECT* lpThis,
	DWORD dwPriority)
{
	__KERNEL_THREAD_OBJECT*   lpKernel = NULL;
	__KERNEL_THREAD_MANAGER*  lpMgr    = (__KERNEL_THREAD_MANAGER*)lpThis;
	__PRIORITY_QUEUE*         lpQueue  = NULL;
	int i = 0;
	unsigned long ulFlags = 0;
	int processorID = __CURRENT_PROCESSOR_ID;
	__KERNEL_THREAD_READY_QUEUE* pReadyQueue = NULL;
	
	/* Validate parameters. */
	if((NULL == lpThis) || (dwPriority > MAX_KERNEL_THREAD_PRIORITY))
	{
		return NULL;
	}

	/* Get the current processor's ready queue. */
#if defined(__CFG_SYS_SMP)
	BUG_ON(processorID >= MAX_CPU_NUM);
#endif
	
	pReadyQueue = lpMgr->KernelThreadReadyQueue[processorID];
	BUG_ON(NULL == pReadyQueue);

	/* 
	 * Should be protected since the ready queue maybe operated by other 
	 * thread on other processor,or interrupt handler.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pReadyQueue->spin_lock, ulFlags);
	for(i = dwPriority;i < MAX_KERNEL_THREAD_PRIORITY + 1;i ++)
	{
		lpQueue  = pReadyQueue->ThreadQueue[MAX_KERNEL_THREAD_PRIORITY - i + dwPriority];
		lpKernel = (__KERNEL_THREAD_OBJECT*)lpQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpQueue,
			NULL);
		/* Found one thread. */
		while(lpKernel)
		{
			/* It maybe suspended by other thread when in ready queue. */
			if(ShouldSuspend(lpKernel))
			{
				//Suspend the kernel thread.
				lpKernel->dwThreadStatus = KERNEL_THREAD_STATUS_SUSPENDED;
				lpMgr->lpSuspendedQueue->InsertIntoQueue(
					(__COMMON_OBJECT*)lpMgr->lpSuspendedQueue,
					(__COMMON_OBJECT*)lpKernel,
					lpKernel->dwThreadPriority);
			}
			else
			{
				/* Found,just return it. */
				__LEAVE_CRITICAL_SECTION_SMP(pReadyQueue->spin_lock, ulFlags);
				return lpKernel;
			}
			lpKernel = (__KERNEL_THREAD_OBJECT*)lpQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpQueue,
				NULL);
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pReadyQueue->spin_lock, ulFlags);
	/* No ready kernel thread found. */
	return NULL;
}

/* Code backup for debugging. */
__KERNEL_THREAD_OBJECT* __GetScheduleKernelThread(__COMMON_OBJECT* lpThis,
	DWORD dwPriority)
{
	__KERNEL_THREAD_OBJECT*   lpKernel = NULL;
	__KERNEL_THREAD_MANAGER*  lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;
	__PRIORITY_QUEUE*         lpQueue = NULL;
	int i = 0;
	unsigned long ulFlags = 0;
	int processorID = __CURRENT_PROCESSOR_ID;
	__KERNEL_THREAD_READY_QUEUE* pReadyQueue = NULL;

	/* Validate parameters. */
	if ((NULL == lpThis) || (dwPriority > MAX_KERNEL_THREAD_PRIORITY))
	{
		return NULL;
	}
	//_hx_printk("%s:processor id = %d\r\n", __func__, processorID);
	//__MicroDelay(3000000);
	/* Get the current processor's ready queue. */
#if defined(__CFG_SYS_SMP)
	BUG_ON(processorID >= MAX_CPU_NUM);
#endif

	pReadyQueue = lpMgr->KernelThreadReadyQueue[processorID];
	BUG_ON(NULL == pReadyQueue);

	/*
	* Should be protected since the ready queue maybe operated by other
	* thread on other processor,or interrupt handler.
	*/
	__ENTER_CRITICAL_SECTION(NULL, ulFlags);
	for (i = dwPriority; i < MAX_KERNEL_THREAD_PRIORITY + 1; i++)
	{
		lpQueue = pReadyQueue->ThreadQueue[MAX_KERNEL_THREAD_PRIORITY - i + dwPriority];
		lpKernel = (__KERNEL_THREAD_OBJECT*)lpQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpQueue,
			NULL);
		/* Found one thread. */
		if (lpKernel)
		{
			/* It maybe suspended by other thread when in ready queue. */
			if (ShouldSuspend(lpKernel))
			{
				//Suspend the kernel thread.
				lpKernel->dwThreadStatus = KERNEL_THREAD_STATUS_SUSPENDED;
				lpMgr->lpSuspendedQueue->InsertIntoQueue(
					(__COMMON_OBJECT*)lpMgr->lpSuspendedQueue,
					(__COMMON_OBJECT*)lpKernel,
					lpKernel->dwThreadPriority);
				continue;
			}
			__LEAVE_CRITICAL_SECTION(NULL, ulFlags);
			return lpKernel;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL, ulFlags);
	/* No ready kernel thread found. */
	return NULL;
}

/*
 * Add a kernel thread whose status is READY to ready queue.
 * The kernel thread's priority acts as index to locate the queue element
 * in ready queue array.
 */
VOID AddReadyKernelThread(__COMMON_OBJECT* lpThis, __KERNEL_THREAD_OBJECT* lpKernelThread)
{
	__PRIORITY_QUEUE* lpQueue = NULL;
	__KERNEL_THREAD_MANAGER* pManager = (__KERNEL_THREAD_MANAGER*)lpThis;
	unsigned int processorID = 0;
	unsigned long ulFlags = 0;
	__KERNEL_THREAD_READY_QUEUE* pReadyQueue = NULL;
#if defined(__CFG_SYS_SMP)
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
#endif

	BUG_ON((NULL == lpThis) || (NULL == lpKernelThread));

	/* 
	 * Just bounce the process to thread flag's handler,if 
	 * the flags value is set,and return immediately.
	 */
	if (lpKernelThread->ulThreadFlags)
	{
		HandleThreadFlags((__COMMON_OBJECT*)lpKernelThread);
		return;
	}

	/* Get the target processor's ready queue. */
#if defined(__CFG_SYS_SMP)
	processorID = lpKernelThread->cpuAffinity;
	BUG_ON(processorID >= MAX_CPU_NUM);
#else
	processorID = __CURRENT_PROCESSOR_ID;
#endif

	pReadyQueue = pManager->KernelThreadReadyQueue[processorID];
	BUG_ON(NULL == pReadyQueue);

	/* Validate the kernel thread's status and priority value. */
	if((lpKernelThread->dwThreadPriority > MAX_KERNEL_THREAD_PRIORITY) || 
		(lpKernelThread->dwThreadStatus != KERNEL_THREAD_STATUS_READY))
	{
		return;
	}

	/* 
	 * Should be protected since the ready queue maybe manipulated in other 
	 * kernel thread running on other processor,or by interrupt handler.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pReadyQueue->spin_lock, ulFlags);
	lpQueue = pReadyQueue->ThreadQueue[lpKernelThread->dwThreadPriority];
	/* The kernel thread must not in ready queue. */
	BUG_ON(lpQueue->ObjectInQueue((__COMMON_OBJECT*)lpQueue, (__COMMON_OBJECT*)lpKernelThread));
	lpQueue->InsertIntoQueue((__COMMON_OBJECT*)lpQueue, 
		(__COMMON_OBJECT*)lpKernelThread, 0);
	__LEAVE_CRITICAL_SECTION_SMP(pReadyQueue->spin_lock, ulFlags);
#if defined(__CFG_SYS_SMP)
	/*
	* Trigger an IPI interrupt to target CPU,
	* to tell it that a new kernel thread is added
	* into it's ready queue,so it can do a scheduling
	* immediately.
	* Skip this step if scheduling is disabled.
	*/
	if (KernelThreadManager.scheduling_enabled == KERNEL_THREAD_SCHEDULING_ENABLE)
	{
		if ((processorID != __CURRENT_PROCESSOR_ID) && (!IN_SYSINITIALIZATION()))
		{
			/*
			* Get the interrupt controller object from current
			* processor's specific information.
			*/
			pSpec = ProcessorManager.GetCurrentProcessorSpecific();
			BUG_ON(NULL == pSpec);
			pIntCtrl = pSpec->pIntCtrl;
			BUG_ON(NULL == pIntCtrl);
			BUG_ON(NULL == pIntCtrl->Send_IPI);
			pIntCtrl->Send_IPI(pIntCtrl, processorID, IPI_TYPE_NEWTHREAD);
		}
	}
#endif
	return;
}

/*
 * SetThreadHook routine,this routine sets appropriate hook routine
 * according to dwHookType, and returns the old one.
 */
__THREAD_HOOK_ROUTINE SetThreadHook(DWORD dwHookType,
	__THREAD_HOOK_ROUTINE lpRoutine)
{
	__THREAD_HOOK_ROUTINE lpOldRoutine = NULL;
	DWORD dwFlags;

	/* Can not be interrupted,use KernelThreadManager's spin lock for protection. */
	__ENTER_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, dwFlags);
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
	__LEAVE_CRITICAL_SECTION_SMP(KernelThreadManager.spin_lock, dwFlags);
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

/*
 * KernelThreadWrapper routine,all kernel threads start from
 * this routine,i.e,it is all kernel thread's entry porint.
 * The routine does the following:
 *  1. Calles the kernel thread's start routine;
 *  2. When the start routine is over,put the kernel thread object into terminal queue;
 *  3. Wakeup all kernel thread(s) waiting for this kernel thread object.
 *  4. Reschedule all kernel thread(s).
 * This routine will never return.
 */
void __KernelThreadWrapper(__COMMON_OBJECT* lpKThread)
{
	__KERNEL_THREAD_OBJECT*        lpKernelThread      = (__KERNEL_THREAD_OBJECT*)lpKThread;
	__KERNEL_THREAD_OBJECT*        lpWaitingThread     = NULL;
	__PRIORITY_QUEUE*              lpWaitingQueue      = NULL;
	DWORD                          dwRetValue          = 0;
	DWORD                          dwFlags             = 0;

	//Execute user defined kernel thread routine.
	dwRetValue = lpKernelThread->KernelThreadRoutine(lpKernelThread->lpRoutineParam);

	/* Clean up after finishing the user defined kernel thread routine. */
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	lpKernelThread->dwReturnValue    = dwRetValue;
	/* Change kernel thread status. */
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_TERMINAL;
	//Insert the current kernel thread object into TERMINAL queue.
	KernelThreadManager.lpTerminalQueue->InsertIntoQueue((__COMMON_OBJECT*)KernelThreadManager.lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	/*
	 * Wakeup all kernel thread(s) who waiting for this kernel thread 
	 * object.
	 */
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
			lpWaitingThread);
		lpWaitingThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);

	/* 
	 * Trigger a rescheduling,the current kernel thread will stay in 
	 * terminal queue.
	 */
	KernelThreadManager.ScheduleFromProc(NULL);
	return;
}

/* 
 * Half bottom part of a kernel thread,responds for the cleaning of a kernel thread's
 * context.
 * This routine is used for supporting the implementation of TerminateKernelThread,which
 * in furthar supports the implementations of exit() and abort() C lib routine.
 */
VOID KernelThreadClean(__COMMON_OBJECT* lpKThread,DWORD dwExitCode)
{
	__KERNEL_THREAD_OBJECT*        lpKernelThread      = (__KERNEL_THREAD_OBJECT*)lpKThread;
	__KERNEL_THREAD_OBJECT*        lpWaitingThread     = NULL;
	__PRIORITY_QUEUE*              lpWaitingQueue      = NULL;
	DWORD                          dwFlags             = 0;

	BUG_ON(NULL == lpKernelThread);

	//Set the exit code.
	lpKernelThread->dwReturnValue = dwExitCode;

	//Begin clean the execution context of the specified kernel thread.
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_TERMINAL;
	/* 
	 * Insert the current kernel thread object into TERMINAL queue. 
	 * Once the kernel thread appear in terminal queue,it's ready to be
	 * destroyed by other kernel threads.But no need to worry about
	 * race condition since the kernel thread's spin lock is acquired
	 * before do anything.
	 */
	KernelThreadManager.lpTerminalQueue->InsertIntoQueue((__COMMON_OBJECT*)KernelThreadManager.lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread,
		0);
	/*
	 * The following code wakeup all kernel thread(s) who waiting for this kernel thread 
	 * object.
	 */
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
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);

	//Re-schedule kernel thread.
	KernelThreadManager.ScheduleFromProc(NULL);
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
	
	if(NULL == lpKernelThread)
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	/* If the object's status is TERMINAL,the wait operation will success. */
	if(KERNEL_THREAD_STATUS_TERMINAL == lpKernelThread->dwThreadStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	//
	//If the waited object's status is not TERMINAL,then the waiting operation will
	//not secussful,the current kernel thread who want to wait will be blocked.
	//
	lpWaitingQueue = lpKernelThread->lpWaitingQueue;
	lpCurrent = __CURRENT_KERNEL_THREAD;
	lpCurrent->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	lpCurrent->ucAligment = KERNEL_THREAD_WAITTAG_KTHREAD;
	lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpWaitingQueue,
		(__COMMON_OBJECT*)lpCurrent,
		0);
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	KernelThreadManager.ScheduleFromProc(NULL);
	return 0;
}

//
//The initialization routine of Kernel Thread Object.
//In the current implementation of Hello China,we do the following task:
// 1. Create the waiting queue object of the kernel thread object;
// 2. Initialize the waiting queue object;
// 3. Set appropriaty value of the member functions,such as WaitForThisObject.
//
BOOL KernelThreadInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                    bResult = FALSE;
	__PRIORITY_QUEUE*       lpWaitingQueue = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	__PRIORITY_QUEUE*       lpMsgWaitingQueue = NULL;
	int                     i;

	BUG_ON(NULL == lpThis);

	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThis;
	/*
	* Create the waiting queue,to hold the kernel thread that wait
	* for this one to run end.
	*/
	lpWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == lpWaitingQueue)
	{
		goto __TERMINAL;
	}
	if (!lpWaitingQueue->Initialize((__COMMON_OBJECT*)lpWaitingQueue))
	{
		goto __TERMINAL;
	}

	/* Create message waiting queue. */
	lpMsgWaitingQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == lpMsgWaitingQueue)
	{
		goto __TERMINAL;
	}
	if (!lpMsgWaitingQueue->Initialize((__COMMON_OBJECT*)lpMsgWaitingQueue))
	{
		goto __TERMINAL;
	}

	lpKernelThread->lpWaitingQueue = lpWaitingQueue;
	lpKernelThread->lpMsgWaitingQueue = lpMsgWaitingQueue;
	lpKernelThread->WaitForThisObject = WaitForKernelThreadObject;
	lpKernelThread->lpKernelThreadContext = NULL;
	/*
	* Use ucAligment as waiting tag,i.e,record what kind of synchronous
	* object the kernel thread is waiting for,if it's status is
	* BLOCKED.
	*/
	lpKernelThread->ucAligment = 0;

	/* All thread flags are cleared. */
	lpKernelThread->ulThreadFlags = 0;

#if defined(__CFG_SYS_PROCESS)
	/* Initializes user stack. */
	lpKernelThread->pUserStack = NULL;
	lpKernelThread->user_stk_size = 0;
	__ATOMIC_SET(&lpKernelThread->in_user, 0);
#endif

	//Initialize the multiple object waiting related variables.
	lpKernelThread->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	lpKernelThread->dwMultipleWaitFlags = 0;
	for (i = 0; i < MAX_MULTIPLE_WAIT_NUM; i++)
	{
		lpKernelThread->MultipleWaitObjectArray[i] = NULL;
	}
#if defined(__CFG_SYS_SMP)
	/* Use current CPU as the cpuAffinity value. */
	__ATOMIC_SET(&lpKernelThread->cpuAffinity, __CURRENT_PROCESSOR_ID);
	/* Initialize spin lock. */
	__INIT_SPIN_LOCK(lpKernelThread->spin_lock, "kthread");
#endif

	/* OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Release all object(s) that created in case of failure. */
		if (lpWaitingQueue != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpWaitingQueue);
		}
		if (lpMsgWaitingQueue != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpMsgWaitingQueue);
		}
	}
	return bResult;
}

/*
* Release resource of a kernel thread object.
* The routine is invoked by Uninitialize routine of
* kernel thread object and does the following:
*  1. Remove the specified thread object from terminal queue;
*  2. Delete the kernel thread object from Terminal Queue;
*/
static BOOL __ReleaseThreadSrc(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpKernel)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpKernel;
	__KERNEL_THREAD_MANAGER* lpMgr = (__KERNEL_THREAD_MANAGER*)lpThis;
	__VIRTUAL_MEMORY_MANAGER* pvmmgr = NULL;
	__PROCESS_OBJECT* pProcess = NULL;
	__PRIORITY_QUEUE* lpTerminalQueue = NULL;
	LPVOID lpStack = NULL;
	unsigned long ulFlags = 0;

	BUG_ON((NULL == lpThis) || (NULL == lpKernel));

	/*
	* Delete from terminal queue,the kernel thread must be in terminal
	* queue if it's status is TERMINAL and ready to be destroyed.
	* Otherwise the kernel thread may not be TERMINAL state or invalid
	* object pointer specified.
	*/
	lpTerminalQueue = lpMgr->lpTerminalQueue;
	if (!lpTerminalQueue->DeleteFromQueue((__COMMON_OBJECT*)lpTerminalQueue,
		(__COMMON_OBJECT*)lpKernelThread))
	{
		return FALSE;
	}

	/*
	* Only the kernel thread with TERMINAL status can be destroyed,use
	* TerminateKernelThread routine to terminate a specified kernel thrad's
	* running and transmit to this status.
	*/
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
	BUG_ON(KERNEL_THREAD_STATUS_TERMINAL != lpKernelThread->dwThreadStatus);

	//Detach it from owner process's kernel thread list.
	ProcessManager.UnlinkKernelThread((__COMMON_OBJECT*)&ProcessManager,
		NULL, (__COMMON_OBJECT*)lpKernelThread);

	/* Call terminal hook routine. */
	lpMgr->CallThreadHook(THREAD_HOOK_TYPE_TERMINAL,
		lpKernelThread, NULL);

	/*
	 * Release the thread's kernel stack.
	 * User thread and kernel thread all have their own
	 * kernel stack.
	 */
	lpStack = lpKernelThread->lpInitStackPointer;
	lpStack = (LPVOID)((DWORD)lpStack - lpKernelThread->dwStackSize);
	KMemFree(lpStack, KMEM_SIZE_TYPE_ANY, 0);

	/* Release the user stack if the thread is a user one. */
	if (IS_USER_THREAD(lpKernelThread))
	{
		/* Release from it's user space. */
		pProcess = (__PROCESS_OBJECT*)lpKernelThread->lpOwnProcess;
		/* User thread must belong to a process. */
		BUG_ON(NULL == pProcess);
		pvmmgr = pProcess->pMemMgr;
		BUG_ON(NULL == pvmmgr);
		pvmmgr->VirtualFree((__COMMON_OBJECT*)pvmmgr, lpKernelThread->pUserStack);
	}

	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_DESTROYED;
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);

	return TRUE;
}

//
//The Uninitialize routine of kernel thread object.
//
BOOL KernelThreadUninitialize(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpThis;;

	BUG_ON(NULL == lpKernelThread);

	if (KERNEL_OBJECT_SIGNATURE != lpKernelThread->dwObjectSignature)
	{
		return FALSE;
	}
	if (!__ReleaseThreadSrc((__COMMON_OBJECT*)&KernelThreadManager,
		lpThis))
	{
		/* The object can not be destroyed yet. */
		return FALSE;
	}

	/* Kernel thread status must be destroyed. */
	BUG_ON(lpKernelThread->dwThreadStatus != KERNEL_THREAD_STATUS_DESTROYED);
	//Reset signature first.
	lpKernelThread->dwObjectSignature = 0;
	//Destroy waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpKernelThread->lpWaitingQueue);
	//Destroy message waiting queue.
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpKernelThread->lpMsgWaitingQueue);
	return TRUE;
}
