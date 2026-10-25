//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 08,2026
//    Module Name               : condvar.c
//    Module Funciton           : 
//                                Kernel synchronization object's implementation
//                                code in this file, for condition variable matching
//                                POSIX condition variable.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <system.h>
#include <types.h>
#include <ktmgr2.h>
#include <commobj.h>
#include <heap.h>
#include <hellocn.h>
#include <stdio.h>

/****************************************************************************************
/*
/*  The implementation of CONDITION object,which is used to synchronize multiple kernel
/*  threads,and conforms POSIX pthread standard.
/*
/***************************************************************************************/

/*
 * The following routine resides in synobj.c file,it's a
 * common used routine by all synchronizing objects,to apply
 * timeout waiting.
 */
extern DWORD TimeOutWaiting(__COMMON_OBJECT* pSynObject,
	__PRIORITY_QUEUE* pWaitingQueue,
	__KERNEL_THREAD_OBJECT* lpKernelThread,
	DWORD dwMillionSecond,
	VOID(*TimeOutCallback)(VOID*));

/*
 * WaitForConditionObject's implementation,this is a empty 
 * implementation just to satisfy kernel syn object's API spec, and
 * it does not conforms the operations offered by POSIX 
 * standard condition object.
 */
static DWORD WaitForConditionObject(__COMMON_OBJECT* pCondObj)
{
	return OBJECT_WAIT_FAILED;
}

/*
 * Timeout call back for Condition object,will be used in TimeOutWaiting routine,
 * and will be invoked in the timer object set by TimeOutWaiting routine.
 * to remove the kernel thread waiting on the condition from pending queue in case
 * of waiting time out.
 */
static VOID CondTimeOutCallback(VOID* pData)
{
	__TIMER_HANDLER_PARAM* lpHandlerParam = (__TIMER_HANDLER_PARAM*)pData;
	__CONDITION* pCond = NULL;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == lpHandlerParam);

	/*
	 * No need to acquire kernel thread's spin lock, since
	 * time out timer's handler already acquired it before call
	 * this routine.
	 * But condition object's spin lock should be obtained since
	 * we will modify it's variables.
	 */
	pCond = (__CONDITION*)lpHandlerParam->lpSynObject;
	BUG_ON(NULL == pCond);
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, ulFlags);
	//Delete the lpKernelThread from waiting queue.
	lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
		(__COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
		(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread);
	//Also should decrement reference counter of the MUTEX object.
	((__CONDITION*)lpHandlerParam->lpSynObject)->nThreadNum--;
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, ulFlags);
	//Add the just deleted kernel thread into ready queue.
	lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
	lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		lpHandlerParam->lpKernelThread);
}

/*
 * Release mutex object without re-schedule, it's
 * just for condition var requirement. If invoke
 * mutex's Release routine directly, it will trigger
 * reschedule without condition's spinlock release,
 * and then lead deadlock.
 * This routine MUST keep same as ReleaseMutex in
 * synobj.c file, it's better to write a common
 * routine to be used by these 2 places, but it
 * maybe future's work.
 */
static DWORD __cond_ReleaseMutex(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	__MUTEX* lpMutex = NULL;
	DWORD dwPreviousStatus = 0;
	unsigned long dwFlags = 0, dwFlags1 = 0;

	/* Object's handle must be specified. */
	BUG_ON(NULL == lpThis);

	lpMutex = (__MUTEX*)lpThis;
	__ENTER_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
	/* Only current owner can release it. */
	if (__CURRENT_KERNEL_THREAD != lpMutex->lpCurrentOwner)
	{
		/*
		 * Set a break point to capture BUG.
		 * TODO:
		 * This is mainly used to capture user land error
		 * now, should be removed in commercial release.
		 */
		BUG_ON(TRUE);
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
		return 0;
	}
	/*
	 * Check if is recursive obtaining.
	 */
	if (__CURRENT_KERNEL_THREAD == lpMutex->lpCurrentOwner)
	{
		lpMutex->nCurrOwnCount--;
		if (lpMutex->nCurrOwnCount > 0) /* Just return. */
		{
			dwPreviousStatus = lpMutex->dwMutexStatus;
			__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
			return dwPreviousStatus;
		}
	}
	if (lpMutex->dwWaitingNum > 0)
	{
		/*
		 * If there are other kernel threads waiting for
		 * this object,decrement the waiting number.
		 */
		lpMutex->dwWaitingNum--;
	}
	if (0 == lpMutex->dwWaitingNum)
	{
		/* No kernel thread waiting for the object,set to free. */
		dwPreviousStatus = lpMutex->dwMutexStatus;
		lpMutex->dwMutexStatus = MUTEX_STATUS_FREE;
		lpMutex->lpCurrentOwner = NULL;
		lpMutex->nCurrOwnCount = 0;
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
		return 0;
	}
	/* Wake up one kernel thread who waiting for this mutex. */
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpMutex->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
		0);
	BUG_ON(NULL == lpKernelThread);
	/*
	 * Set mutex's current owner as the new thread,
	 * since it will own this object automatically after
	 * woken up.
	 */
	lpMutex->lpCurrentOwner = lpKernelThread;
	lpMutex->nCurrOwnCount = 1;
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
	KernelThreadManager.AddReadyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		lpKernelThread);
	__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
	__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);

	/* Rescheduling. */
	//KernelThreadManager.ScheduleFromProc(NULL);
	return dwPreviousStatus;
}

/* Wait a condition object until the condition satisfied or time out. */
static DWORD CondWaitTimeout(__COMMON_OBJECT* pCondObj, __COMMON_OBJECT* pMutexObj,
	DWORD dwMillionSecond)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__MUTEX* pMutex = (__MUTEX*)pMutexObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags;
	DWORD dwResult = OBJECT_WAIT_FAILED;

	if ((NULL == pCond) || (NULL == pMutex))
	{
		goto __TERMINAL;
	}

	/* Optional: validates kernel object signature. */
	BUG_ON((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) ||
		(KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature));

	/*
	 * Put the current kernel thread into pending queue,and 
	 * release mutex,in one atomic operation.
	 * TODO:
	 *   Should consider the cast that time out value is 0,
	 *   in this case, the mutex object should be release
	 *   and obtain again, without putting the thread into
	 *   condition's waiting queue, according POSIX.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_COND;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	
	/* Put into waiting queue. */
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread, pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	pCond->nThreadNum++;

	/* Release the mutex object. */
	__cond_ReleaseMutex((__COMMON_OBJECT*)pMutex);

	/* Save current kernel thread. */
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	/* Issue a time out waiting. */
	dwResult = TimeOutWaiting((__COMMON_OBJECT*)pCond, pCond->lpPendingQueue, pKernelThread,
		dwMillionSecond, CondTimeOutCallback);

	/*
	 * The kernel thread is waken up when reach here, 
	 * then try to get the mutex object again.
	 */
	pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

__TERMINAL:
	return dwResult;
}

/* Wait a specified condition object. */
static DWORD CondWait(__COMMON_OBJECT* pCondObj, __COMMON_OBJECT* pMutexObj)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__MUTEX* pMutex = (__MUTEX*)pMutexObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags;
	DWORD dwResult = OBJECT_WAIT_FAILED;

	BUG_ON((NULL == pCond) || (NULL == pMutex));

	/* Validate the kernel objects. */
	BUG_ON((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) ||
		(KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature));

	/*
	 * Put the current kernel thread into pending queue,and release mutex,in
	 * one atomic operation.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_COND;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;

	/* Put into condition's waiting queue. */
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread, pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	pCond->nThreadNum++;

	/* 
	 * Release mutex object, use local version 
	 * of release mutex operation. 
	 */
	__cond_ReleaseMutex((__COMMON_OBJECT*)pMutex);

	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	/* Issue reschedule. */
	KernelThreadManager.ScheduleFromProc(NULL);

	/*
	 * Now the kernel thread must be waken up,
	 * try to occupy the mutex object again before return.
	 */
	dwResult = pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

	return dwResult;
}

/* Signal a condition object. */
static DWORD CondSignal(__COMMON_OBJECT* pCondObj)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags;

	if (NULL == pCond)
	{
		return 0;
	}
	//Validate the kernel object.
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature);

	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	/* Wakeup one kernel thread if there is. */
	if (pCond->nThreadNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue, NULL);
		BUG_ON(NULL == pKernelThread);
		//Put the kernel thread object into ready queue.
		__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
		//Update the pending kernel thread counter.
		pCond->nThreadNum--;
	}
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	//Reschedule.
	KernelThreadManager.ScheduleFromProc(NULL);
	return 1;
}

/* Broadcast a condition object. */
static DWORD CondBroadcast(__COMMON_OBJECT* pCondObj)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags;

	if (NULL == pCond)
	{
		return 0;
	}
	//Validate the kernel object.
	if (KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	/* Wakeup all kernel thread(s) waiting for condition object. */
	while (pCond->nThreadNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue, NULL);
		BUG_ON(NULL == pKernelThread);
		__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
		//Put the kernel thread object into ready queue.
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
		//Update the pending kernel thread counter.
		pCond->nThreadNum--;
	}
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	//Reschedule.
	KernelThreadManager.ScheduleFromProc(NULL);
	return 1;
}

/* Initializer of Condition object. */
BOOL ConditionInitialize(__COMMON_OBJECT* pCondObj)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__PRIORITY_QUEUE* pPendingQueue = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pCond);

	/*
	 * Create the pending queue object of condition,
	 * which is used to contain kernel thread(s) waiting 
	 * on the CONDITION object.
	 */
	pPendingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager, NULL, OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == pPendingQueue)
	{
		goto __TERMINAL;
	}
	if (!pPendingQueue->Initialize((__COMMON_OBJECT*)pPendingQueue))
	{
		goto __TERMINAL;
	}

	//Initialize the CONDITION object.
	pCond->nThreadNum = 0;
	pCond->lpPendingQueue = pPendingQueue;
	pCond->CondWait = CondWait;
	pCond->CondWaitTimeout = CondWaitTimeout;
	pCond->CondSignal = CondSignal;
	pCond->CondBroadcast = CondBroadcast;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pCond->spin_lock, "condition");
#endif

	//Set the signature of kernel object,to identify this object is a valid
	//kernel object.
	pCond->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;

	bResult = TRUE;

__TERMINAL:
	/* Recollec resource if failure. */
	if (!bResult)
	{
		if (!pPendingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pPendingQueue);
		}
	}
	return bResult;
}

/* Uninitializer of Condition object. */
BOOL ConditionUninitialize(__COMMON_OBJECT* pCondObj)
{
	__CONDITION* pCond = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags;

	BUG_ON(NULL == pCond);

	//Check if the specified condition object is a valid kernel thread.
	if (KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return FALSE;
	}

	//Wakeup all pending kernel thread(s) if there is(are).
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	while (pCond->nThreadNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue, NULL);
		BUG_ON(NULL == pKernelThread);
		//Set the waiting flags of the kernel thread and insert it into ready queue.
		__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
		pCond->nThreadNum--;
	}
	pCond->dwObjectSignature = 0;  //Clear the kernel object signature.
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	//Destroy the pending queue object.
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pCond->lpPendingQueue);
	return TRUE;
}
