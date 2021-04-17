//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 20,2014
//    Module Name               : synobj2.c
//    Module Funciton           : 
//                                Kernel synchronization object's implementation
//                                code in this file.
//    Last modified Author      :
//    Last modified Date        : Jun 20,2014
//    Last modified Content     :
//                                1. Kernel object signature validation mechanism is added;
//                                2. WaitForMultipleObjects is implemented.
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

/* Change semaphore's default counter value. */
static BOOL SetSemaphoreCount(__COMMON_OBJECT* pSemaphore, DWORD dwMaxSem, DWORD dwCurrSem)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	unsigned long ulFlags;

	if (NULL == pSem)
	{
		return FALSE;
	}
	/* Maximal count can not be zero. */
	if (0 == dwMaxSem)
	{
		return FALSE;
	}

	/* Invalid value. */
	if (dwMaxSem < dwCurrSem)
	{
		return FALSE;
	}
	//Change the default value.
	__ENTER_CRITICAL_SECTION_SMP(pSem->spin_lock, ulFlags);
	pSem->dwMaxSem = dwMaxSem;
	pSem->dwCurrSem = dwCurrSem;
	__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, ulFlags);
	return TRUE;
}

/*
 * ReleaseSemaphore.It increase the dwCurrSem's value,and
 * wake up one kernel thread if exist.
 * The previous current counter will be returned in pdwPrevCount.
 */
static BOOL _ReleaseSemaphore(__COMMON_OBJECT* pSemaphore,
	DWORD* pdwPrevCount,
	BOOL bNoResched)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD dwFlags = 0;

	BUG_ON(NULL == pSem);
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pSem->dwObjectSignature);

	__ENTER_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
	/* Return the previous counter value. */
	if (pdwPrevCount)
	{
		*pdwPrevCount = pSem->dwCurrSem;
	}
	if (pSem->dwCurrSem < pSem->dwMaxSem)
	{
		pSem->dwCurrSem++;
	}
	else
	{
		/* Reach the maximal value,invalid operation. */
		__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
		return FALSE;
	}
	/* Wake up one kernel thread. */
	pKernelThread = (__KERNEL_THREAD_OBJECT*)pSem->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)pSem->lpWaitingQueue, NULL);
	if (pKernelThread)
	{
		__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);

	/* Trigger a rescheduling if called does not deny it explicity. */
	if (!bNoResched)
	{
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return TRUE;
}

/* Wait for semaphore object,infinite wait. */
static DWORD WaitForSemObject(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD dwRetValue = OBJECT_WAIT_FAILED;
	DWORD dwFlags;

	BUG_ON(NULL == pSem);
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pSem->dwObjectSignature);

__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
	if (pSem->dwCurrSem > 0)
	{
		/* Resource available,got it. */
		pSem->dwCurrSem--;
		dwRetValue = OBJECT_WAIT_RESOURCE;
		__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	//Resource unavailable,wait it.
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_SEMAPHORE;
	/*
	 * Add into semaphore's waiting queue.
	 * Priority value specified,so as will be waken up
	 * earlier if has higher priority.
	 */
	pSem->lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)pSem->lpWaitingQueue,
		(__COMMON_OBJECT*)pKernelThread,
		pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
	/* Reschedule all kernel thread(s). */
	KernelThreadManager.ScheduleFromProc(NULL);

	/* The kernel thread is waken up when reach here. */
	__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
	if (pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)
	{
		/* Semaphore object is destroyed. */
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_DELETED;
		goto __TERMINAL;
	}
	else
	{
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
		/*
		 * Try to obtain resource again,since the resource maybe
		 * acquired by other kernel thread(s).
		 */
		goto __TRY_AGAIN;
	}

__TERMINAL:
	return dwRetValue;
}

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

/* WaitForThisObjectEx routine,timeout waiting for semaphore. */
static DWORD WaitForSemObjectEx(__COMMON_OBJECT* pSemaphore, DWORD dwMillionSecond, DWORD* pdwWait)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD dwCalledTick = System.dwClockTickCounter;
	DWORD dwTimeOutTick = 0;
	DWORD dwRetValue = OBJECT_WAIT_FAILED;
	DWORD dwFlags;

	if (NULL == pSem)
	{
		goto __TERMINAL;
	}
	if (KERNEL_OBJECT_SIGNATURE != pSem->dwObjectSignature)
	{
		goto __TERMINAL;
	}

	if (WAIT_TIME_INFINITE != dwMillionSecond)
	{
		/*
		 * Calculates the time out tick counter.
		 * It means time out when current system tick
		 * counter is larger than time out counter.
		 */
		dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
		dwTimeOutTick += System.dwClockTickCounter;
	}

__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
	if (pSem->dwCurrSem)
	{
		/* Resource available. */
		pSem->dwCurrSem--;
		dwRetValue = OBJECT_WAIT_RESOURCE;
		if (pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/*
	 * Resource unavailable and no timeout value specified,
	 * so just return immediately with timeout value.
	 */
	if (0 == dwMillionSecond)
	{
		if (pdwWait)
		{
			*pdwWait = 0;
		}
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	
	/* User specified waiting time out. */
	if ((dwMillionSecond != WAIT_TIME_INFINITE) && 
		(dwTimeOutTick <= System.dwClockTickCounter))
	{
		/* Set waited time if specified. */
		if (pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Block current kernel thread to wait. */
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_SEMAPHORE;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	//Add to semaphore's waiting queue.
	pSem->lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)pSem->lpWaitingQueue,
		(__COMMON_OBJECT*)pKernelThread,
		pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);

	if (dwMillionSecond != WAIT_TIME_INFINITE)
	{
		/* Apply timeout waiting. */
		switch (TimeOutWaiting((__COMMON_OBJECT*)pSem, pSem->lpWaitingQueue,
			pKernelThread,
			dwMillionSecond, NULL))
		{
		case OBJECT_WAIT_RESOURCE:
			/* Try to acquire resource again. */
			goto __TRY_AGAIN;
		case OBJECT_WAIT_TIMEOUT:
			/* Set waitted time if specified. */
			if (pdwWait)
			{
				*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
			}
			dwRetValue = OBJECT_WAIT_TIMEOUT;
			goto __TERMINAL;
		case OBJECT_WAIT_DELETED:
			/* Set waited time if specified. */
			if (pdwWait)
			{
				*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
			}
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		default:
			BUG();
		}
	}
	else /* dwMillionSecond == WAIT_TIME_INFINITE. */
	{
		/* Let other threads run. */
		KernelThreadManager.ScheduleFromProc(NULL);

		/* The thread is waken up now. */
		__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
		if (pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)
		{
			/* Semaphore is destroyed. */
			__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
			if (pdwWait)
			{
				*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
			}
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		}
		else
		{
			__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags);
			/* Try to obtain semaphore again. */
			goto __TRY_AGAIN;
		}
	}

__TERMINAL:
	return dwRetValue;
}

/* Initializer of semaphore object. */
BOOL SemInitialize(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pSem);

	/* Create waiting queue. */
	lpPriorityQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(
		&ObjectManager, 
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == lpPriorityQueue)
	{
		goto __TERMINAL;
	}
	bResult = lpPriorityQueue->Initialize((__COMMON_OBJECT*)lpPriorityQueue);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	//Set default semaphore's counter.
	pSem->dwMaxSem = 1;
	pSem->dwCurrSem = 1;
	pSem->lpWaitingQueue = lpPriorityQueue;
	pSem->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pSem->spin_lock, "semaphore");
#endif

	//Set operation functions accordingly.
	pSem->WaitForThisObject = WaitForSemObject;
	pSem->WaitForThisObjectEx = WaitForSemObjectEx;
	pSem->SetSemaphoreCount = SetSemaphoreCount;
	pSem->ReleaseSemaphore = _ReleaseSemaphore;

__TERMINAL:
	if (!bResult)
	{
		if (NULL != lpPriorityQueue)
		{
			/* Release the priority queue. */
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpPriorityQueue);
		}
	}
	return bResult;
}

//Unitializer of semaphore object.
BOOL SemUninitialize(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)pSemaphore;
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	DWORD dwFlags;

	BUG_ON(NULL == pSem);
	if (pSem->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}

	__ENTER_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);
	lpPriorityQueue = pSem->lpWaitingQueue;
	if (0 == pSem->dwCurrSem)
	{
		//Should wake up all kernel thread(s) who waiting for this object.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpPriorityQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpPriorityQueue, NULL);
		while (lpKernelThread)
		{
			__ACQUIRE_SPIN_LOCK(lpKernelThread->spin_lock);
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			__RELEASE_SPIN_LOCK(lpKernelThread->spin_lock);
			lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpPriorityQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpPriorityQueue, NULL);
		}
	}
	//Clear the kernel object's signature.
	pSem->dwObjectSignature = 0;
	__LEAVE_CRITICAL_SECTION_SMP(pSem->spin_lock, dwFlags);

	//Destroy prirority queue of the semaphore.
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpPriorityQueue);
	return TRUE;
}

//**------------------------------------------------------------------------------------------------
//**
//**  Implementation of Mailbox object.
//**
//**------------------------------------------------------------------------------------------------

/*
 * WaitForMailboxObject's implementation,this is a empty implementation 
 * since it's not allowed to wait a mailbox object.
 */
DWORD WaitForMailboxObject(__COMMON_OBJECT* pMailbox)
{
	return OBJECT_WAIT_FAILED;
}

/* Show out a mailbox object for kernel's debugging. */
void __ShowMailboxObject(__COMMON_OBJECT* pObject)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)pObject;
	unsigned long getq_num = 0, sendq_num = 0;
	unsigned long object_id, max_msg_num, curr_msg_num;
	unsigned long ulFlags;

	BUG_ON(NULL == pMailbox);
	/* Lock the mailbox object and get all it's state. */
	__ENTER_CRITICAL_SECTION_SMP(pMailbox->spin_lock, ulFlags);
	getq_num = pMailbox->lpGettingQueue->dwCurrElementNum;
	sendq_num = pMailbox->lpSendingQueue->dwCurrElementNum;
	object_id = pMailbox->dwObjectID;
	max_msg_num = pMailbox->dwMaxMessageNum;
	curr_msg_num = pMailbox->dwCurrMessageNum;
	__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, ulFlags);

	/* Then show out. */
	_hx_printf("Mailbox: id[%d], max_msg_num[%d], curr_msg_num[%d], gq_num[%d], sq_num[%d]\r\n",
		object_id, max_msg_num, curr_msg_num,
		getq_num, sendq_num);
	return;
}

/*
 * SetMailboxSize,it release the previous allocated message array memory
 * and allocates a new one accordingly.
 * This routine must be called before any mailbox's sending or getting operation.
 */
BOOL SetMailboxSize(__COMMON_OBJECT* pMailboxObj, DWORD dwNewSize)
{
	__MAIL_BOX*    pMailbox = (__MAIL_BOX*)pMailboxObj;
	BOOL           bResult = FALSE;
	__MB_MESSAGE*  pNewMessageArray = NULL;
	__MB_MESSAGE*  pOldMessageArray = NULL;
	unsigned long  dwFlags;

	/* Validate parameters. */
	if ((NULL == pMailbox) || (0 == dwNewSize))
	{
		goto __TERMINAL;
	}
	if (pMailbox->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}

	__ENTER_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
	/* The mailbox must be empty. */
	if (pMailbox->dwCurrMessageNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	//Check if there is pending kernel thread in getting queue.
	if (pMailbox->lpGettingQueue->dwCurrElementNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Allocate a new array to hold message. */
	pNewMessageArray =
		(__MB_MESSAGE*)KMemAlloc(dwNewSize * sizeof(__MB_MESSAGE), KMEM_SIZE_TYPE_ANY);
	if (NULL == pNewMessageArray)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	memset(pNewMessageArray, 0, dwNewSize * sizeof(__MB_MESSAGE));
	pOldMessageArray = pMailbox->pMessageArray;

	/* Update mailbox's members accordingly. */
	pMailbox->pMessageArray = pNewMessageArray;
	pMailbox->dwMaxMessageNum = dwNewSize;
	pMailbox->dwCurrMessageNum = 0;
	pMailbox->dwMessageHeader = 0;
	pMailbox->dwMessageTail = 0;
	__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);

	//Release the old message queue.
	KMemFree(pOldMessageArray, KMEM_SIZE_TYPE_ANY, 0);
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Release allocated resources. */
		if (pNewMessageArray)
		{
			KMemFree(pNewMessageArray, KMEM_SIZE_TYPE_ANY, 0);
		}
	}
	return bResult;
}

/*
 * Get a message from mailbox.
 * The kernel thread will be blocked if no message in box,
 * tiemout will be returned if dwMillionSecond is set and 
 * no message available after this period of time.
 * The processing time from the call emit to successful return 
 * will be set if pdwWait is not NULL.
 */
static DWORD _GetMail(__COMMON_OBJECT* pMailboxObj, LPVOID* ppMessage, 
	DWORD dwMillionSecond, DWORD* pdwWait)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags, dwFlags1;
	DWORD dwCalledTick = System.dwClockTickCounter;
	DWORD dwTimeOutTick = 0;
	DWORD dwTimeoutWait = 0;
	DWORD dwRetValue = OBJECT_WAIT_FAILED;

	//Parameters checking.
	BUG_ON((NULL == pMailbox) || (NULL == ppMessage));
	BUG_ON(pMailbox->dwObjectSignature != KERNEL_OBJECT_SIGNATURE);

	//Calculate the timeout tick counter.
	if (WAIT_TIME_INFINITE != dwMillionSecond)
	{
		/* Wait time must not less than one system tick. */
		if (dwMillionSecond < SYSTEM_TIME_SLICE)
		{
			dwMillionSecond = SYSTEM_TIME_SLICE;
		}
		dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
		dwTimeOutTick += System.dwClockTickCounter;
	}

	//Try to get mail.
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
	if (pMailbox->dwCurrMessageNum)
	{
		/*
		 * There maybe blocked kernel thread(s) waiting 
		 * to send mail to box,so wake up ALL of them since at least
		 * one slot is free after get mail.
		 */
		if (pMailbox->dwCurrMessageNum == pMailbox->dwMaxMessageNum)
		{
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpSendingQueue, NULL);
			while (pKernelThread)
			{
				/* Wakeup the kernel thread. */
				__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
				pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
				pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
				pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
				KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
					pKernelThread);
				__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
				/* Check if there are more kernel threads. */
				pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
					(__COMMON_OBJECT*)pMailbox->lpSendingQueue, NULL);
			}
		}
		/* Return the first message. */
		*ppMessage = pMailbox->pMessageArray[pMailbox->dwMessageHeader].pMessage;
		pMailbox->dwCurrMessageNum--;
		pMailbox->dwMessageHeader += 1;
		pMailbox->dwMessageHeader = pMailbox->dwMessageHeader % pMailbox->dwMaxMessageNum;
		/* Return waiting time if necessary. */
		if (pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}

	/*
	 * Resource is unavailable,handle this scenario according
	 * to dwMillionSecond's value.
	 */
	if (0 == dwMillionSecond)
	{
		/* Should return immediately.*/
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}

#if 0
	/* Requested time already out. */
	if (dwTimeOutTick <= System.dwClockTickCounter)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}
#endif

	if (WAIT_TIME_INFINITE == dwMillionSecond)
	{
		/* Wait until mail available. */
		pKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MAILBOX_GET;
		//Add into Mailbox's getting queue.
		pMailbox->lpGettingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		/* Reschedule,current thread will be wakeup when mail is available. */
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	else
	{
		/* Wait the specified period of time. */
		pKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MAILBOX_GET;
		//Add current kernel thread into mailbox's getting queue.
		pMailbox->lpGettingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwTimeoutWait = TimeOutWaiting((__COMMON_OBJECT*)pMailbox,
			pMailbox->lpGettingQueue,
			pKernelThread,
			dwMillionSecond, NULL);
	}

	//The kernel thread is waken up when reach here.
	if (WAIT_TIME_INFINITE != dwMillionSecond)
	{
		switch (dwTimeoutWait)
		{
		case OBJECT_WAIT_RESOURCE:
			goto __TRY_AGAIN;
		case OBJECT_WAIT_TIMEOUT:
			dwRetValue = OBJECT_WAIT_TIMEOUT;
			goto __TERMINAL;
		case OBJECT_WAIT_DELETED:
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		default:
			BUG();
			dwRetValue = OBJECT_WAIT_FAILED;
			goto __TERMINAL;
		}
	}
	else  //Wait INFINITE.
	{
		if (pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)
		{
			/* The mailbox is destroyed. */
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		}
		else
		{
			goto __TRY_AGAIN;
		}
	}

__TERMINAL:
	return dwRetValue;
}

//A helper routine used by SendMail routine,put a mail into mailbox,in sort of Priority.
//****NOTE****:The current implementation is not consider the priority sort of messages,
//since it no explicit requirement of this function,and the implementation of this function
//is complicated.:-)
static VOID __SendMail(__MAIL_BOX* pMailbox, LPVOID pMessage, DWORD dwPriority)
{
	__MB_MESSAGE* pMessageArray = pMailbox->pMessageArray;
	DWORD dwIndex = pMailbox->dwMessageTail;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long ulFlags;

	//Check if there is waiting kernel thread pending in getting queue.
	//Wake up one if there exist.
	if (0 == pMailbox->dwCurrMessageNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue, NULL);
		if (pKernelThread)
		{
			//Wakeup the kernel thread.
			__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, ulFlags);
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, ulFlags);
		}
	}

	pMessageArray[dwIndex].dwPriority = dwPriority;
	pMessageArray[dwIndex].pMessage = pMessage;
	pMailbox->dwCurrMessageNum += 1;
	pMailbox->dwMessageTail += 1;
	pMailbox->dwMessageTail %= pMailbox->dwMaxMessageNum;
	return;
}

/*
 * Send a message to mailbox.
 * The kernel thread will be blocked in case of mail box is full,caller can
 * specify a wait period by setting dwMillionSecond.
 * A 0 value of dwMillionSecond will make the routine return immediately,
 * even in case of failure(queue full,timeout flag will be returned).
 * The waited time will be returned if pdwWait is set(not NULL).
 */
static DWORD _SendMail(__COMMON_OBJECT* pMailboxObj, LPVOID pMessage, DWORD dwPriority, 
	DWORD dwMillionSecond, DWORD* pdwWait)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	unsigned long dwFlags, dwFlags1;
	DWORD dwCalledTick = System.dwClockTickCounter;
	DWORD dwTimeOutTick = 0;
	DWORD dwTimeoutWait = 0;
	DWORD dwRetValue = OBJECT_WAIT_FAILED;

	//Parameters checking.
	BUG_ON(NULL == pMailbox);
	BUG_ON(pMailbox->dwObjectSignature != KERNEL_OBJECT_SIGNATURE);

	/*
	 * Calculate the timeout tick counter,when the operation
	 * timeout value is not infinite and 0.
	 */
	if ((WAIT_TIME_INFINITE != dwMillionSecond) && (0 != dwMillionSecond))
	{
		/* Wait time must not less than one time slice. */
		if (dwMillionSecond < SYSTEM_TIME_SLICE)
		{
			dwMillionSecond = SYSTEM_TIME_SLICE;
		}
		dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
		dwTimeOutTick += System.dwClockTickCounter;
	}

	//Try to put message into mailbox.
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
	if (pMailbox->dwCurrMessageNum < pMailbox->dwMaxMessageNum)
	{
		/* Mailbox is not full,just put the message. */
		__SendMail(pMailbox, pMessage, dwPriority);
		//Return wait time if necessary.
		if (pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}

	/* No space in mailbox to contain new message when reach here. */
	if (0 == dwMillionSecond)
	{
		/* Return immediately if no timeout specified. */
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}

#if 0
	/* Requested time already out. */
	if (dwTimeOutTick <= System.dwClockTickCounter)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}
#endif

	if (WAIT_TIME_INFINITE == dwMillionSecond)
	{
		/* Wait until mailbox has empty slot to contain mail. */
		pKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;  //Set waiting flag.
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MAILBOX_PUT;
		/* Add into Mailbox's sending queue. */
		pMailbox->lpSendingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		/* Reschedule,current thread will be wakeup when mailbox is not full.*/
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	else
	{
		/* Wait the specified period of time. */
		pKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MAILBOX_PUT;
		//Add current kernel thread into mailbox's getting queue.
		pMailbox->lpSendingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
		dwTimeoutWait = TimeOutWaiting((__COMMON_OBJECT*)pMailbox,
			pMailbox->lpSendingQueue,
			pKernelThread,
			dwMillionSecond, NULL);
	}

	//The kernel thread is waken up when reach here.
	if (WAIT_TIME_INFINITE != dwMillionSecond)
	{
		switch (dwTimeoutWait)
		{
		case OBJECT_WAIT_RESOURCE:
			goto __TRY_AGAIN;
		case OBJECT_WAIT_TIMEOUT:
			dwRetValue = OBJECT_WAIT_TIMEOUT;
			goto __TERMINAL;
		case OBJECT_WAIT_DELETED:
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		default:
			BUG();
			dwRetValue = OBJECT_WAIT_FAILED;
			goto __TERMINAL;
		}
	}
	else
	{
		if (pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)
		{
			/* Mailbox is destroyed. */
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		}
		else
		{
			goto __TRY_AGAIN;
		}
	}

__TERMINAL:
	return dwRetValue;
}

//Initializer of Mail Box object.
BOOL MailboxInitialize(__COMMON_OBJECT* pMailboxObj)
{
	__MB_MESSAGE*      pMbMessage = NULL;
	__MAIL_BOX*        pMailbox = (__MAIL_BOX*)pMailboxObj;
	__PRIORITY_QUEUE*  pSendingQueue = NULL;
	__PRIORITY_QUEUE*  pGettingQueue = NULL;
	BOOL               bResult = FALSE;

	if (NULL == pMailbox)
	{
		goto __TERMINAL;
	}

	//Allocate resources for mail box.
	pMbMessage = (__MB_MESSAGE*)KMemAlloc(sizeof(__MB_MESSAGE), KMEM_SIZE_TYPE_ANY);
	if (NULL == pMbMessage)
	{
		goto __TERMINAL;
	}
	//Create waiting queues for sending and getting threads.
	pSendingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager, NULL, OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == pSendingQueue)
	{
		goto __TERMINAL;
	}
	bResult = pSendingQueue->Initialize((__COMMON_OBJECT*)pSendingQueue);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	pGettingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager, NULL, OBJECT_TYPE_PRIORITY_QUEUE);
	if (NULL == pGettingQueue)
	{
		goto __TERMINAL;
	}
	bResult = pGettingQueue->Initialize((__COMMON_OBJECT*)pGettingQueue);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	//Assign members to mailbox object.
	pMailbox->pMessageArray = pMbMessage;
	pMailbox->dwMaxMessageNum = 1;    //Default is 1,can be changed by SetMailboxSize routine.
	pMailbox->dwCurrMessageNum = 0;
	pMailbox->dwMessageHeader = 0;
	pMailbox->dwMessageTail = 0;
	pMailbox->lpSendingQueue = pSendingQueue;
	pMailbox->lpGettingQueue = pGettingQueue;
	pMailbox->WaitForThisObject = WaitForMailboxObject;
	pMailbox->SetMailboxSize = SetMailboxSize;
	pMailbox->SendMail = _SendMail;
	pMailbox->GetMail = _GetMail;
	pMailbox->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
#if defined(__CFG_SYS_SMP)
	/* Initialize spin lock. */
	__INIT_SPIN_LOCK(pMailbox->spin_lock, "mbox");
#endif

	bResult = TRUE;

__TERMINAL:
	if (!bResult)  //Initialization failed,destroy any resource successfully allocated.
	{
		if (pMbMessage)
		{
			KMemFree(pMbMessage, KMEM_SIZE_TYPE_ANY, 0);
		}
		if (!pSendingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pSendingQueue);
		}
		if (!pGettingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pGettingQueue);
		}
	}
	return bResult;
}

//Uninitializer of mailbox object.
BOOL MailboxUninitialize(__COMMON_OBJECT* pMailboxObj)
{
	__MAIL_BOX*              pMailbox = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT*  pKernelThread = NULL;
	unsigned long dwFlags = 0, dwFlags1 = 0;

	BUG_ON(NULL == pMailbox);

	/* Verify object signature. */
	if (pMailbox->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}

	//Wake up all kernel thread(s) pending on the mail box.
	__ENTER_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);
	if (0 == pMailbox->dwCurrMessageNum)
	{
		/* Wake up all pending kernel thread(s) who waiting for getting message. */
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue, NULL);
		while (pKernelThread)
		{
			/* Should be protected by spin lock in SMP. */
			__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpGettingQueue, NULL);
		}
	}
	if (pMailbox->dwCurrMessageNum == pMailbox->dwMaxMessageNum)
	{
		/* Wakeup all pending kernel thread(s) who waiting for sending message. */
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue, NULL);
		while (pKernelThread)
		{
			__ENTER_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			__LEAVE_CRITICAL_SECTION_SMP(pKernelThread->spin_lock, dwFlags1);
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpSendingQueue, NULL);
		}
	}
	pMailbox->dwObjectSignature = 0;  //Clear signature of the object.
	__LEAVE_CRITICAL_SECTION_SMP(pMailbox->spin_lock, dwFlags);

	//Release all resources of mailbox object.
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pMailbox->lpGettingQueue);
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pMailbox->lpSendingQueue);
	KMemFree(pMailbox->pMessageArray, KMEM_SIZE_TYPE_ANY, 0);
	return TRUE;
}

/****************************************************************************************
/*
/*  The implementation of CONDITION object,which is used to synchronize multiple kernel
/*  threads,and conforms POSIX pthread standard.
/*  Honestly,I don't think this mechanism is a good choice,since all scenarios that CONDITION
/*  works can be simulated by EVENT or MUTEX object.The only reason to implements this
/*  object is to fit the requirement of JamVM's porting,which use POSIX features and APIs
/*  widely.
/*
/***************************************************************************************/

/*
 * WaitForConditionObject's implementation,this is a empty implementation since it does not
 * conforms the operations offered by POSIX standard condition object.
 */
static DWORD WaitForConditionObject(__COMMON_OBJECT* pCondObj)
{
	return OBJECT_WAIT_FAILED;
}

/* Wait a specified condition object. */
static DWORD CondWait(__COMMON_OBJECT* pCondObj, __COMMON_OBJECT* pMutexObj)
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
	//Validate the kernel objects.
	if ((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) ||
		(KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature))
	{
		goto __TERMINAL;
	}

	/*
	 * Put the current kernel thread into pending queue,and release mutex,in
	 * one atomic operation.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	/* Obtain mutex's spin lock at the same time. */
	__ACQUIRE_SPIN_LOCK(pMutex->spin_lock);
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_COND;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	//Put the kernel thread into condition object's pending queue.
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread, pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	pCond->nThreadNum++;

	//Release the mutex object.
	if (pMutex->dwWaitingNum > 0)
	{
		pMutex->dwWaitingNum--;
		if (0 == pMutex->dwWaitingNum)
		{
			pMutex->dwMutexStatus = MUTEX_STATUS_FREE;
		}
		else
		{
			/* Wakeup one kernel thread. */
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pCond->lpPendingQueue,
				NULL);
			BUG_ON(NULL == pKernelThread);
			__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
		}
	}
	else
	{
		/*
		 * This scenario should not exist,since at least
		 * current kernel thread is occupying it.
		 */
		BUG();
	}
	__RELEASE_SPIN_LOCK(pMutex->spin_lock);
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	KernelThreadManager.ScheduleFromProc(NULL);

	/*
	 * Now the kernel thread must be waken up,
	 * try to occupy the mutex object again before return.
	 */
	dwResult = pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

__TERMINAL:
	return dwResult;
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

/* Wait a condition object until the condition satisfied or time out. */
static DWORD CondWaitTimeout(__COMMON_OBJECT* pCondObj, __COMMON_OBJECT* pMutexObj, DWORD dwMillionSecond)
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
	//Validate the kernel objects.
	if ((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) ||
		(KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature))
	{
		goto __TERMINAL;
	}

	/*
	 * Put the current kernel thread into pending queue,and release mutex,in
	 * one atomic operation.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);
	/* Obatin mutex's spin lock simultaneously. */
	__ACQUIRE_SPIN_LOCK(pMutex->spin_lock);
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
	pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_COND;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	//Put the kernel thread into condition object's pending queue.
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread, pKernelThread->dwThreadPriority);
	__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
	pCond->nThreadNum++;

	//Release the mutex object.
	if (pMutex->dwWaitingNum > 0)
	{
		pMutex->dwWaitingNum--;
		if (0 == pMutex->dwWaitingNum)
		{
			pMutex->dwMutexStatus = MUTEX_STATUS_FREE;
		}
		else
		{
			/* Wakeup one kernel thread. */
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pCond->lpPendingQueue,
				NULL);
			BUG_ON(NULL == pKernelThread);
			__ACQUIRE_SPIN_LOCK(pKernelThread->spin_lock);
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			__RELEASE_SPIN_LOCK(pKernelThread->spin_lock);
		}
	}
	else
	{
		/* This scenario should not exist,since at least current kernel thread is occupying it. */
		BUG();
	}
	//Record the current kernel thread.
	pKernelThread = __CURRENT_KERNEL_THREAD;
	__RELEASE_SPIN_LOCK(pMutex->spin_lock);
	__LEAVE_CRITICAL_SECTION_SMP(pCond->spin_lock, dwFlags);

	dwResult = TimeOutWaiting((__COMMON_OBJECT*)pCond, pCond->lpPendingQueue, pKernelThread,
		dwMillionSecond, CondTimeOutCallback);

	/*
	 * Now the kernel thread must be waken up,then try to
	 * occupy the mutex object again before return.
	 */
	pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

__TERMINAL:
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
	if (KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return 0;
	}

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
	 * Create the pending queue object of condition,which is used to contain
	 * kernel thread(s) waiting on the CONDITION object.
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
	if (!bResult)  //Release all resources allocated to the CONDITION object.
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
