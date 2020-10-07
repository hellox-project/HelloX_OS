//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 21, 2005
//    Module Name               : synobj.c
//    Module Funciton           : 
//                                Kernel synchronization object's implementation
//                                code.
//    Last modified Author      :
//    Last modified Date        : Mar 10,2012
//    Last modified Content     :
//                                1. Kernel object signature validation mechanism is added;
//                                2. WaitForMultipleObjects is implemented.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "types.h"
#include "commobj.h"
#include "ktmgr.h"
#include "system.h"
#include "hellocn.h"
#include "stdio.h"

//Timer handler routine for all synchronous object.
static DWORD WaitingTimerHandler(LPVOID lpData)
{
	__TIMER_HANDLER_PARAM* lpHandlerParam = (__TIMER_HANDLER_PARAM*)lpData;
	unsigned long dwFlags;

	BUG_ON(NULL == lpHandlerParam);

	/* Should be protected by kernel thread's spinlock. */
	__ENTER_CRITICAL_SECTION_SMP(lpHandlerParam->lpKernelThread->spin_lock,dwFlags);
	switch(lpHandlerParam->lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:
		case OBJECT_WAIT_DELETED:
			break;
		case OBJECT_WAIT_WAITING:
			if(lpHandlerParam->TimeOutCallback)
			{
				/* Call the specified time out call back. */
				lpHandlerParam->TimeOutCallback((VOID*)lpHandlerParam);
			}
			else
			{
				/* 
				 * Delete the lpKernelThread from waiting queue. 
				 * The deletion may fail if the kernel is already
				 * deleted by other CPU,since there maybe a contention
				 * exist,for we don't acquire the spin lock of
				 * corresponding synchronous object.
				 */
				if (lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
					(__COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
					(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread))
				{
					/* Use the default implementation,it works for most synchronization kernel objects. */
					lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
					lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
					//Also should decrement reference counter of the synchronization object if it has.
					//Add this kernel thread to ready queue.
					lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
					KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
						lpHandlerParam->lpKernelThread);
				}
			}
			break;
		default:
			BUG();
			break;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpHandlerParam->lpKernelThread->spin_lock, dwFlags);
	return 0;
}

/*
 * TimeOutWaiting is a global routine used by any synchronous objects' 
 * WaitForThisObjectEx routine.
 */
DWORD TimeOutWaiting(__COMMON_OBJECT* lpSynObject,      //Synchronous object.
	__PRIORITY_QUEUE* lpWaitingQueue,                   //Waiting queue.
	__KERNEL_THREAD_OBJECT* lpKernelThread,             //Who want to wait.
	DWORD dwMillionSecond,                              //Time out value in millionsecond.
	VOID (*TimeOutCallback)(VOID*))                     //Call back routine when timeout.
{
	__TIMER_OBJECT* lpTimerObj = NULL;
	__TIMER_HANDLER_PARAM HandlerParam;
	unsigned long ulFlags;

	/* Validate parameters. */
	if((NULL == lpSynObject) || (NULL == lpWaitingQueue) ||
	   (NULL == lpKernelThread) || (0 == dwMillionSecond))
	{
		BUG();
	}

	//Initialize HandlerParam.
	HandlerParam.lpKernelThread  = lpKernelThread;
	HandlerParam.lpSynObject     = lpSynObject;
	HandlerParam.lpWaitingQueue  = lpWaitingQueue;
	HandlerParam.TimeOutCallback = TimeOutCallback;

	/* 
	 * Use one shot timer to wake up the waiting kernel thread
	 * when timeout.Use TIMER_FLAGS_NOAUTODEL flag to avoid deleting
	 * of timer by kernel,since this may lead 'twice deleting' of
	 * timer object as follows:
	 * 1. The kernel thread put itself into waiting queue of syn object;
	 * 2. One shot timer is set as follows;
	 * 3. The synchronous object is signal and all kernel threads waiting
	 *    for it are waken up and ready to run(but not run immediately),
	 *    it's waiting status is set to OBJECT_WAIT_RESOURCE;
	 * 4. Then the timer is triggered and timeout handler is called;
	 * 5. Suppose the timer object is automatically destroyed at the moment by
	 *    interrupt handler;
	 * 6. When the kernel thread is put into run,it will delete the timer
	 *    object again,this lead 'twice deleting' of timer object.
	 * So we use NOAUTODEL flag to avoid this contention.
	 */
	lpTimerObj = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
		lpKernelThread,
#define TIMEOUT_WAITING_TIMER_ID 2048
		TIMEOUT_WAITING_TIMER_ID,
		dwMillionSecond,
		WaitingTimerHandler,
		(LPVOID)&HandlerParam,
		TIMER_FLAGS_ONCE | TIMER_FLAGS_NOAUTODEL);
	if(NULL == lpTimerObj)
	{
		_hx_printk("%s:failed to set timer.\r\n", __func__);
		return OBJECT_WAIT_FAILED;
	}

	KernelThreadManager.ScheduleFromProc(NULL);

	/* The waiting kernel thread was waken up. */
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
	switch(lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:
			/* Got resource successfully. */
			__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObj);  //Cancel timer.
			return OBJECT_WAIT_RESOURCE;

		case OBJECT_WAIT_DELETED:
			/* Synchronous object was deleted. */
			__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObj);
			return OBJECT_WAIT_DELETED;

		case OBJECT_WAIT_TIMEOUT:
			/* 
			 * Wait timeout,no need to cancel timer since it already be 
			 * destroyed in process of system timer interrupt handler.
			 */
			__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, ulFlags);
			/* Destroy the timer object since it's no auto delete timer. */
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObj);
			return OBJECT_WAIT_TIMEOUT;
		default:
			break;
	}
	//Once reach here,it means error encountered.
	BUG();
	return OBJECT_WAIT_FAILED;
}

//
//Routines pre-declaration.
//
static DWORD kWaitForEventObject(__COMMON_OBJECT*);
static DWORD kSetEvent(__COMMON_OBJECT*);
static DWORD kResetEvent(__COMMON_OBJECT*);
static DWORD kWaitForEventObjectEx(__COMMON_OBJECT*,DWORD);
static DWORD PulseEvent(__COMMON_OBJECT*);

//---------------------------------------------------------------------------------
//
//                SYNCHRONIZATION OBJECTS
//
//----------------------------------------------------------------------------------

/*
 * Event object's initializing routine.
 * This routine initializes the members of an event object.
 */
BOOL EventInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                  bResult          = FALSE;
	__EVENT*              lpEvent          = (__EVENT*)lpThis;
	__PRIORITY_QUEUE*     lpPriorityQueue  = NULL;

	BUG_ON(NULL == lpEvent);

	/* Create waiting queue of event object. */
	lpPriorityQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpPriorityQueue)
	{
		goto __TERMINAL;
	}
	bResult = lpPriorityQueue->Initialize((__COMMON_OBJECT*)lpPriorityQueue);
	if(!bResult)
	{
		goto __TERMINAL;
	}

	/* Initialize the event object. */
	lpEvent->lpWaitingQueue = lpPriorityQueue;
	lpEvent->dwEventStatus = EVENT_STATUS_OCCUPIED;
	lpEvent->SetEvent = kSetEvent;
	lpEvent->ResetEvent = kResetEvent;
	lpEvent->PulseEvent = PulseEvent;
	lpEvent->WaitForThisObjectEx = kWaitForEventObjectEx;
	lpEvent->WaitForThisObject = kWaitForEventObject;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpEvent->spin_lock,"event");
#endif
	lpEvent->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpPriorityQueue)    //Release the priority queue.
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpPriorityQueue);
	}
	return bResult;
}

/* 
 * Event object's uninitializing routine.
 * Safety deleted is support by EVENT object,so in this routine,
 * if there are kernel threads waiting for this object,then wakeup
 * all kernel threads,and then destroy the event object.
 */
BOOL EventUninitialize(__COMMON_OBJECT* lpThis)
{
	__EVENT* lpEvent = (__EVENT*)lpThis;
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	unsigned long dwFlags, dwFlags1;

	BUG_ON(NULL == lpEvent);

	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock,dwFlags);
	lpPriorityQueue = lpEvent->lpWaitingQueue;
	if (EVENT_STATUS_FREE != lpEvent->dwEventStatus)
	{
		//Should wake up all kernel thread(s) who waiting for this object.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpPriorityQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpPriorityQueue,
			NULL);
		while(lpKernelThread)
		{
			__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
			lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
			lpKernelThread = (__KERNEL_THREAD_OBJECT*)
				lpPriorityQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpPriorityQueue,
				NULL);
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);

	/* Clear the kernel object's signature and destroy it. */
	lpEvent->dwObjectSignature = 0;
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpPriorityQueue);
	return TRUE;
}

/*
 * PulseEvent,it as the combination of SetEvent and ResetEvent in
 * atomic,the previous status will be retuned.
 */
static DWORD PulseEvent(__COMMON_OBJECT* lpThis)
{
	DWORD                     dwPreviousStatus = EVENT_STATUS_OCCUPIED;
	__EVENT*                  lpEvent = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread = NULL;
	unsigned long dwFlags = 0, dwFlags1 = 0;

	if (NULL == lpThis)
	{
		return dwPreviousStatus;
	}
	lpEvent = (__EVENT*)lpThis;
	/* Validate the event object. */
	if (lpEvent->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return dwPreviousStatus;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	if (EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		/* Current status is free,just return. */
		__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
		return dwPreviousStatus;
	}
	BUG_ON(EVENT_STATUS_OCCUPIED != lpEvent->dwEventStatus);
	/*
	 * The event status is occupied,so try to wake up all 
	 * kernel thread(s) waiting for this event,if there are.
	 */
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)
		lpEvent->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			NULL);
	while (lpKernelThread)
	{
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		//Set waiting result bit.
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);  //Add to ready queue.
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpEvent->lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
				NULL);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);

	/* Triggeer rescheduling if in process context. */
	if (IN_KERNELTHREAD())
	{
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return dwPreviousStatus;
}

/*
 * SetEvent,this routine do the following:
 *  1. Saves the previous status into a local variable;
 *  2. Sets the current status of the event to EVENT_STATUS_FREE;
 *  3. Wakes up all kernel thread(s) in it's waiting queue.
 *  4. Returns the previous status.
 */
static DWORD kSetEvent(__COMMON_OBJECT* lpThis)
{
	DWORD                     dwPreviousStatus     = EVENT_STATUS_OCCUPIED;
	__EVENT*                  lpEvent              = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread       = NULL;
	unsigned long dwFlags = 0, dwFlags1 = 0;

	if(NULL == lpThis)
	{
		return dwPreviousStatus;
	}

	lpEvent = (__EVENT*)lpThis;
	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	/* Set the current status to free. */
	lpEvent->dwEventStatus = EVENT_STATUS_FREE;
	/* Wake up all kernel thread(s) waiting for this event. */
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)
		lpEvent->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
		NULL);
	while(lpKernelThread)
	{
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		//Set waiting result bit.
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);  //Add to ready queue.
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpEvent->lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);

	/* Triggeer rescheduling if in process context. */
	if(IN_KERNELTHREAD())
	{
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return dwPreviousStatus;
}

/* 
 * ResetEvent routine.
 * Change the event's status to occupied,and return the status
 * before this operation.
 */
static DWORD kResetEvent(__COMMON_OBJECT* lpThis)
{
	__EVENT*          lpEvent          = (__EVENT*)lpThis;
	DWORD             dwPreviousStatus = 0;
	DWORD             dwFlags          = 0;

	if(NULL == lpEvent)
	{
		return dwPreviousStatus;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	lpEvent->dwEventStatus = EVENT_STATUS_OCCUPIED;
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	return dwPreviousStatus;
}

/* Wait for a event object. */
static DWORD kWaitForEventObject(__COMMON_OBJECT* lpThis)
{
	__EVENT*                      lpEvent             = (__EVENT*)lpThis;
	__KERNEL_THREAD_OBJECT*       lpKernelThread      = NULL;
	unsigned long dwFlags = 0, dwFlags1 = 0;

	if(NULL == lpEvent)
	{
		return OBJECT_WAIT_FAILED;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	else
	{
		lpKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_EVENT;
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	/* Trigger a rescheduling. */
	KernelThreadManager.ScheduleFromProc(NULL);
	/* 
	 * The kernel thread is wakenup when reach here,it has obtained 
	 * the resource succesfully.
	 */
	return OBJECT_WAIT_RESOURCE;
}

/* Timeout waiting for event object. */
static DWORD kWaitForEventObjectEx(__COMMON_OBJECT* lpObject,DWORD dwMillionSecond)
{
	__EVENT* lpEvent = (__EVENT*)lpObject;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	unsigned long dwFlags, dwFlags1;
	DWORD dwTimeOutTick;
	DWORD dwTimeSpan;

	if(NULL == lpObject)
	{
		return OBJECT_WAIT_FAILED;
	}

	/* Calculate the tick counter that timeout occur. */
	dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? 
		(dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
	dwTimeOutTick += System.dwClockTickCounter;

	__ENTER_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
		return OBJECT_WAIT_RESOURCE;
 	}
	/* Wait the event object. */
	if(0 == dwMillionSecond)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);
		return OBJECT_WAIT_TIMEOUT;
	}
	lpKernelThread = __CURRENT_KERNEL_THREAD;
	while(EVENT_STATUS_FREE != lpEvent->dwEventStatus)
	{
		if(dwTimeOutTick <= System.dwClockTickCounter)
		{
			__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
			return OBJECT_WAIT_TIMEOUT;
		}
		dwTimeSpan = (dwTimeOutTick - System.dwClockTickCounter) * SYSTEM_TIME_SLICE;
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_EVENT;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		//Add to event object's waiting queue.
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
		
		/* Apply timeout waiting. */
		switch(TimeOutWaiting((__COMMON_OBJECT*)lpEvent,lpEvent->lpWaitingQueue,
			lpKernelThread,dwTimeSpan,NULL))
		{
			case OBJECT_WAIT_RESOURCE:
				/* 
				 * No need to check again,since the event is triggered,i.e,
				 * the event has occured,it's different from mutex or other
				 * synchronous objects.
				 */
				return OBJECT_WAIT_RESOURCE;
			case OBJECT_WAIT_TIMEOUT:
				return OBJECT_WAIT_TIMEOUT;
			case OBJECT_WAIT_DELETED:
				return OBJECT_WAIT_DELETED;
			default:
				BUG();
				break;
		}
	}
	/* Should not reach here. */
	__LEAVE_CRITICAL_SECTION_SMP(lpEvent->spin_lock, dwFlags);
	return OBJECT_WAIT_RESOURCE;
}

////////////////////////////////////////////////////////////////////////////////////
//
// ------------------ ** The implementation of MUTEX object ** ---------------------
//
///////////////////////////////////////////////////////////////////////////////////

/*
 * ReleaseMutex,it wake up all kernel threads that waiting for the mutex
 * object and set the mutex's status to FREE.
 */
static DWORD __ReleaseMutex(__COMMON_OBJECT* lpThis)
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
	if(lpMutex->dwWaitingNum > 0)
	{
		/* 
		 * If there are other kernel threads waiting for
		 * this object,decrement the waiting number. 
		 */
		lpMutex->dwWaitingNum --;
	}
	if(0 == lpMutex->dwWaitingNum)
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
	KernelThreadManager.ScheduleFromProc(NULL);
	return dwPreviousStatus;
}

/* Wait for mutex object,infinite waiting. */
static DWORD WaitForMutexObject(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	__MUTEX* lpMutex = (__MUTEX*)lpThis;
	unsigned long dwFlags = 0, dwFlags1 = 0;
	unsigned long dwRetValue = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == lpMutex);

	__ENTER_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)
	{
		/* Mutex is free. */
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;
		/* Increment the waiting number. */
		lpMutex->dwWaitingNum  ++;
		/* Save owner. */
		lpMutex->lpCurrentOwner = __CURRENT_KERNEL_THREAD;
		lpMutex->nCurrOwnCount  = 1;
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}
	else
	{
		/* the mutex is occupied,check if recursive obtain. */
		if (lpMutex->lpCurrentOwner == __CURRENT_KERNEL_THREAD)
		{
			lpMutex->nCurrOwnCount += 1;
			__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
			dwRetValue = OBJECT_WAIT_RESOURCE;
			goto __TERMINAL;
		}
		/*
		 * The mutex object is occupied and the current owner is not the
		 * one try to obtain it.
		 */
		lpKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MUTEX;
		/* Increment the waiting number. */
		lpMutex->dwWaitingNum++;
		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
		/* Trigger rescheduling of threads. */
		KernelThreadManager.ScheduleFromProc(NULL);
	}

	/* The kernel thread is waken up when reach here. */
	__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
	if (lpKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)
	{
		/* Mutex object is destroyed. */
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		dwRetValue = OBJECT_WAIT_DELETED;
		goto __TERMINAL;
	}
	else
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags);
		/*
		 * The thread is woken up by ReleaseMutex routine,
		 */
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}

__TERMINAL:
	return dwRetValue;
}

/* 
 * Timeout call back for Mutex object,will be called in TimeOutWaiting routine,
 * to remove the kernel thread waiting on the mutex from pending queue in case
 * of waiting time out.
 * No need to obatin kernel thread's spin lock,since it already been acquired
 * in TimeoutWaiting routine.But mutex's spin lock should be obtained.
 */
static VOID MutexTimeOutCallback(VOID* pData)
{
	__TIMER_HANDLER_PARAM* lpHandlerParam = (__TIMER_HANDLER_PARAM*)pData;
	__MUTEX* pMutex = NULL;

	BUG_ON(NULL == lpHandlerParam);
	pMutex = (__MUTEX*)lpHandlerParam->lpSynObject;
	BUG_ON(NULL == pMutex);

	__ACQUIRE_SPIN_LOCK(pMutex->spin_lock);
	/* 
	 * Delete the lpKernelThread from waiting queue.
	 * It may already deleted in case of race condition.
	 */
	if (lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
		(__COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
		(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread))
	{
		lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
		//Also should decrement reference counter of the MUTEX object.
		((__MUTEX*)lpHandlerParam->lpSynObject)->dwWaitingNum--;
		//Add this kernel thread to ready queue.
		lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			lpHandlerParam->lpKernelThread);
	}
	__RELEASE_SPIN_LOCK(pMutex->spin_lock);
}

/*
 * WaitForThisObjectEx,timeout waiting for mutex object.
 * This routine is a time out waiting routine,caller can give a time value
 * to indicate how long it want to wait,once exceed the time value,waiting operation
 * will return,even in the resource is not released.
 * If the time value is zero,then this routine will check the current status of
 * mutex object,if free,then occupy the object and return RESOURCE,else return
 * TIMEOUT,and a re-schedule is triggered.
 */
static DWORD WaitForMutexObjectEx(__COMMON_OBJECT* lpThis, DWORD dwMillionSecond)
{
	__MUTEX* lpMutex = (__MUTEX*)lpThis;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	unsigned long dwFlags = 0, dwFlags1 = 0;
	DWORD dwResult = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == lpMutex);
	
	__ENTER_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)
	{
		/* Mutex is free,just occupy it. */
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;
		lpMutex->dwWaitingNum ++;
		lpMutex->lpCurrentOwner = __CURRENT_KERNEL_THREAD;
		lpMutex->nCurrOwnCount = 1;
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
		dwResult = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}
	else
	{
		/*
		 * Check if recursive obtaining.
		 */
		if (lpMutex->lpCurrentOwner == __CURRENT_KERNEL_THREAD)
		{
			lpMutex->nCurrOwnCount += 1;
			__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
			dwResult = OBJECT_WAIT_RESOURCE;
			goto __TERMINAL;
		}
		if(0 == dwMillionSecond)
		{
			__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);
			KernelThreadManager.ScheduleFromProc(NULL);
			dwResult = OBJECT_WAIT_TIMEOUT;
			goto __TERMINAL;
		}
		lpKernelThread = __CURRENT_KERNEL_THREAD;
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		//Waiting on mutex's waiting queue.
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MUTEX;
		lpMutex->dwWaitingNum ++;  //Added in 2015-04-06.
		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		__LEAVE_CRITICAL_SECTION_SMP(lpMutex->spin_lock, dwFlags);

		/* 
		 * Apply timeout waiting,use callback to manipulate 
		 * mutex object when timeout. 
		 */
		switch (TimeOutWaiting((__COMMON_OBJECT*)lpMutex,
			lpMutex->lpWaitingQueue, 
			lpKernelThread, 
			dwMillionSecond, 
			MutexTimeOutCallback))
		{
		case OBJECT_WAIT_RESOURCE:
			/* The thread is woken up by ReleaseMutex routine. */
			dwResult = OBJECT_WAIT_RESOURCE;
			goto __TERMINAL;
		case OBJECT_WAIT_DELETED:
			dwResult = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		case OBJECT_WAIT_TIMEOUT:
			dwResult = OBJECT_WAIT_TIMEOUT;
			goto __TERMINAL;
		default:
			BUG();
			dwResult = OBJECT_WAIT_FAILED;
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return dwResult;
}

/* Initializer of mutex object. */
BOOL MutexInitialize(__COMMON_OBJECT* lpThis)
{
	__MUTEX* lpMutex = (__MUTEX*)lpThis;
	__PRIORITY_QUEUE* lpQueue = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == lpMutex);
	/* Create waiting queue of mutex object. */
	lpQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpQueue)
	{
		return bResult;
	}
	if(!lpQueue->Initialize((__COMMON_OBJECT*)lpQueue))
	{
		goto __TERMINAL;
	}

	/* Initialize the newly created mutex object. */
	lpMutex->dwMutexStatus = MUTEX_STATUS_FREE;
	lpMutex->lpWaitingQueue = lpQueue;
	lpMutex->WaitForThisObject = WaitForMutexObject;
	lpMutex->dwWaitingNum = 0;
	lpMutex->ReleaseMutex = __ReleaseMutex;
	lpMutex->WaitForThisObjectEx = WaitForMutexObjectEx;
	lpMutex->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	lpMutex->lpCurrentOwner = NULL;
	lpMutex->nCurrOwnCount = 0;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpMutex->spin_lock, "mutex");
#endif
	
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		/* Release all allocated resources if fail. */
		if(NULL != lpQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpQueue);
		}
	}
	return bResult;
}

/*
 * Uninitializer of mutex object.
 * This object support safety deleted,so in this routine,all kernel thread(s)
 * must be waken up before this object is destroyed.
 */
BOOL MutexUninitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE* lpWaitingQueue = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	__MUTEX* pMutex = (__MUTEX*)lpThis;
	unsigned long dwFlags, dwFlags1;

	BUG_ON(NULL == pMutex);

	lpWaitingQueue = pMutex->lpWaitingQueue;
	__ENTER_CRITICAL_SECTION_SMP(pMutex->spin_lock, dwFlags);
	/* Wakeup all kernel thread(s) waiting the mutex. */
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while(lpKernelThread)
	{
		__ENTER_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);
		__LEAVE_CRITICAL_SECTION_SMP(lpKernelThread->spin_lock, dwFlags1);
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pMutex->spin_lock, dwFlags);

	//Reset kernel object's signature.
	pMutex->dwObjectSignature = 0;
	/* Destroy the waiting queue. */
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpWaitingQueue);
	return TRUE;
}

//------------------------------------------------------------------------
//
//  The implementation of WaitForMultipleObjects,which is a new system
//  service added in version 1.80. Now only available under non SMP
//  environment,since spin lock mechanism is not supported in this routine.
//  But it's easy to adapt to SMP,if true requirement exist.
//
//------------------------------------------------------------------------

#if !defined(__CFG_SYS_SMP)

//A local helper routine to check if a given object is a sychronization object.
static BOOL IsSynObject(__COMMON_OBJECT* pObject)
{
	if(NULL == pObject)
	{
		return FALSE;
	}

	if(OBJECT_TYPE_EVENT == pObject->dwObjectType)  //Is a event object.
	{
		if(((__EVENT*)pObject)->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
		{
			return FALSE;
		}
		return TRUE;
	}

	if(OBJECT_TYPE_MUTEX == pObject->dwObjectType)  //Is a mutex object.
	{
		if(((__MUTEX*)pObject)->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
		{
			return FALSE;
		}
		return TRUE;
	}

	if(OBJECT_TYPE_KERNEL_THREAD == pObject->dwObjectType)  //Is a kernelthread object.
	{
		if(((__KERNEL_THREAD_OBJECT*)pObject)->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
		{
			return FALSE;
		}
		return TRUE;
	}
	//Other type of kernel object's checking should be append here.
	return FALSE;
}

//A local helper routine,check if a given object's status is signal.
static BOOL ObjectIsSignal(__COMMON_OBJECT* pObject)
{
	if(OBJECT_TYPE_EVENT == pObject->dwObjectType)
	{
		if(EVENT_STATUS_FREE == ((__EVENT*)pObject)->dwEventStatus)
		{
			return TRUE;
		}
		return FALSE;
	}

	if(OBJECT_TYPE_MUTEX == pObject->dwObjectType)
	{
		if(MUTEX_STATUS_FREE == ((__MUTEX*)pObject)->dwMutexStatus)
		{
			return TRUE;
		}
		return FALSE;
	}

	if(OBJECT_TYPE_KERNEL_THREAD == pObject->dwObjectType)
	{
		if(KERNEL_THREAD_STATUS_TERMINAL == ((__KERNEL_THREAD_OBJECT*)pObject)->dwThreadStatus)
		{
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

//A local helper to check if all objects in an array are in signal status.
static BOOL ObjectsAreSignal(__COMMON_OBJECT** pObjectArray,int nObjectNum)
{
	int     i;

	for(i = 0;i < nObjectNum;i ++)
	{
		if(!ObjectIsSignal(pObjectArray[i]))
		{
			return FALSE;
		}
	}
	return TRUE;
}

//A helper routine,to put the given thread into synchronization object's waiting queue,
//and modify it's status according the object's type.The synchronization object must be in
//non-signaled status.
static BOOL WaitThisObject(__COMMON_OBJECT* pSynObject,__COMMON_OBJECT* pKernelThread)
{
	__PRIORITY_QUEUE*      pThreadQueue     = NULL;

	switch(pSynObject->dwObjectType)
	{
	case OBJECT_TYPE_EVENT:
		pThreadQueue = ((__EVENT*)pSynObject)->lpWaitingQueue;
		if(!pThreadQueue->InsertIntoQueue((__COMMON_OBJECT*)pThreadQueue,pKernelThread,0))
		{
			return FALSE;
		}
		break;
	case OBJECT_TYPE_MUTEX:
		pThreadQueue = ((__MUTEX*)pSynObject)->lpWaitingQueue;
		if(!pThreadQueue->InsertIntoQueue((__COMMON_OBJECT*)pThreadQueue,pKernelThread,0))
		{
			return FALSE;
		}
		((__MUTEX*)pSynObject)->dwWaitingNum ++;
		break;
	case OBJECT_TYPE_KERNEL_THREAD:
		pThreadQueue = ((__KERNEL_THREAD_OBJECT*)pSynObject)->lpWaitingQueue;
		if(!pThreadQueue->InsertIntoQueue((__COMMON_OBJECT*)pThreadQueue,pKernelThread,0))
		{
			return FALSE;
		}
		break;
	default:
		BUG();
		break;
	}
	return TRUE;
}

//A local helper routine,used to wait on all non-signaled objects in the specified object array.
static BOOL WaitTheseObjects(__COMMON_OBJECT** pObjectArray,int nObjectNum,__COMMON_OBJECT* pKernelThread)
{
	int i;

	if((NULL == pObjectArray) || (0 == nObjectNum))
	{
		return FALSE;
	}
	for(i = 0;i < nObjectNum;i ++)
	{
		if(!ObjectIsSignal(pObjectArray[i]))
		{
			if(!WaitThisObject(pObjectArray[i],pKernelThread))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//Obtain all objects in the given array.All objects in the array must be in signal status.
static BOOL ObtainAllObjects(__COMMON_OBJECT** pObjectArray,int nObjectNum,__COMMON_OBJECT* pKernelThread)
{
	int     i;

	if((NULL == pObjectArray) || (0 == nObjectNum) || (NULL == pKernelThread))
	{
		BUG();
		return FALSE;
	}
	for(i = 0;i < nObjectNum;i ++)
	{
		switch(pObjectArray[i]->dwObjectType)
		{
		case OBJECT_TYPE_EVENT:
			if(((__EVENT*)pObjectArray[i])->dwEventStatus != EVENT_STATUS_FREE)
			{
				BUG();
				return FALSE;
			}
			return TRUE;
		case OBJECT_TYPE_MUTEX:
			if(((__MUTEX*)pObjectArray[i])->dwMutexStatus != MUTEX_STATUS_FREE)
			{
				BUG();
				return FALSE;
			}
			((__MUTEX*)pObjectArray[i])->dwMutexStatus = MUTEX_STATUS_OCCUPIED;
			((__MUTEX*)pObjectArray[i])->dwWaitingNum ++;
			return TRUE;
		case OBJECT_TYPE_KERNEL_THREAD:
			if(((__KERNEL_THREAD_OBJECT*)pObjectArray[i])->dwThreadStatus != KERNEL_THREAD_STATUS_TERMINAL)
			{
				BUG();
				return FALSE;
			}
			return TRUE;
		default:
			BUG();
			break;
		}
	}
	return FALSE;
}

/*
 * Cancel the specified kernel thread from all 
 * waiting queues of the kernel objects in object array.
 */
static VOID CancelWait(__COMMON_OBJECT** pObjectArray,__COMMON_OBJECT* pKernelThread)
{
	int i;
	__PRIORITY_QUEUE* pPriorityQueue = NULL;

	for(i = 0;i < MAX_MULTIPLE_WAIT_NUM;i ++)
	{
		if(NULL == pObjectArray[i])
		{
			continue;  //Skip the NULL object.
		}
		switch(pObjectArray[i]->dwObjectType)
		{
		case OBJECT_TYPE_EVENT:
			pPriorityQueue = ((__EVENT*)pObjectArray[i])->lpWaitingQueue;
			pPriorityQueue->DeleteFromQueue((__COMMON_OBJECT*)pPriorityQueue,pKernelThread);
			break;
		case OBJECT_TYPE_MUTEX:
			pPriorityQueue = ((__MUTEX*)pObjectArray[i])->lpWaitingQueue;
			if(pPriorityQueue->DeleteFromQueue((__COMMON_OBJECT*)pPriorityQueue,pKernelThread))
			{
				((__MUTEX*)pObjectArray[i])->dwWaitingNum --;
			}
			break;
		case OBJECT_TYPE_KERNEL_THREAD:
			pPriorityQueue = ((__KERNEL_THREAD_OBJECT*)pObjectArray[i])->lpWaitingQueue;
			pPriorityQueue->DeleteFromQueue((__COMMON_OBJECT*)pPriorityQueue,pKernelThread);
			break;
		default:
			BUG();
			break;
		}
	}
}

/* 
 * Handler of timer object for multiple waiting.
 */
static DWORD MultiWaitTimerHandler(LPVOID pData)
{
	__KERNEL_THREAD_OBJECT* pKernelThread = (__KERNEL_THREAD_OBJECT*)pData;

	switch(pKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:
		case OBJECT_WAIT_DELETED:
			break;
		case OBJECT_WAIT_WAITING:
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
			//Delete the kernel thread object from all synobjects in waiting array.
			CancelWait(pKernelThread->MultipleWaitObjectArray,(__COMMON_OBJECT*)pKernelThread);
			//Add this kernel thread to ready queue.
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			break;
		default:
			BUG();
			break;
	}
	return 0;
}

//Timeout waiting process routine.
static DWORD MultiTimeOutWaiting(__KERNEL_THREAD_OBJECT* pKernelThread,int nObjectNum,
	DWORD dwMillionSeconds)
{
	__TIMER_OBJECT* lpTimerObj;

	//pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	//pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;

	//Set up a one shot timer.
	lpTimerObj = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
		pKernelThread,
#define MULTIPLE_TIMEOUT_WAIT_TIMERID 4096
		MULTIPLE_TIMEOUT_WAIT_TIMERID,
		dwMillionSeconds,
		MultiWaitTimerHandler,
		(LPVOID)&pKernelThread,
		TIMER_FLAGS_ONCE);
	if(NULL == lpTimerObj)
	{
		return OBJECT_WAIT_FAILED;
	}

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.

	//Once reach here,it means the waiting kernel thread was waken up.
	switch(pKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:  //Got resource.
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObj);  //Cancel timer.
			return OBJECT_WAIT_RESOURCE;

		case OBJECT_WAIT_DELETED:   //Synchronous object was deleted.
			System.CancelTimer((__COMMON_OBJECT*)&System,
				(__COMMON_OBJECT*)lpTimerObj);
			return OBJECT_WAIT_DELETED;
		case OBJECT_WAIT_TIMEOUT:   //Time out.
			return OBJECT_WAIT_TIMEOUT;
		default:
			break;
	}
	//Once reach here,it means error encountered.
	BUG();
	return OBJECT_WAIT_FAILED;
}

//A helper routine to clear the multiple waiting related flags and array in 
//kernel thread object.
static VOID ClearMultipleWaitStatus(__KERNEL_THREAD_OBJECT* pKernelThread)
{
	int     i;

	pKernelThread->dwMultipleWaitFlags = 0;
	for(i = 0;i < MAX_MULTIPLE_WAIT_NUM;i ++)
	{
		pKernelThread->MultipleWaitObjectArray[i] = NULL;
	}
}

/*
 * A local helper routine,to process the 
 * case of 'waiting all' objects.
 * This routine will be called by WaitForMultipleObjects.
 */
static DWORD WaitForAll(__COMMON_OBJECT** pObjectArray,
						int nObjectNum,
						DWORD dwMillionSeconds,
						int* pnSignalObjectIndex)
{
	DWORD dwRetVal = OBJECT_WAIT_FAILED;
	DWORD dwFlags;
	DWORD dwTimeToWait = dwMillionSeconds;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	BOOL bTryAgain = FALSE;
	int i;

	pKernelThread = __CURRENT_KERNEL_THREAD;
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	while(TRUE)
	{
		if(ObjectsAreSignal(pObjectArray,nObjectNum))
		{
			ObtainAllObjects(pObjectArray,nObjectNum,(__COMMON_OBJECT*)pKernelThread);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return OBJECT_WAIT_RESOURCE;
		}
		if(0 == dwTimeToWait)  //Time out.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.
			return OBJECT_WAIT_TIMEOUT;
		}
		//Should wait on the objects whose status is non-signal.
		//Set the multiple waiting flags first.
		pKernelThread->dwMultipleWaitFlags |= MULTIPLE_WAITING_STATUS;
		pKernelThread->dwMultipleWaitFlags |= MULTIPLE_WAITING_WAITALL;
		//Save all waited objects.
		for(i = 0;i < nObjectNum;i ++)
		{
			pKernelThread->MultipleWaitObjectArray[i] = pObjectArray[i];
		}
		for(i = nObjectNum;i < MAX_MULTIPLE_WAIT_NUM;i ++)
		{
			pKernelThread->MultipleWaitObjectArray[i] = NULL;
		}
		//Set current kernel thread's status to BLOCKED.
		pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		pKernelThread->ucAligment = KERNEL_THREAD_WAITTAG_MULTIPLE;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		//Cancel wait for re-enter.
		if(bTryAgain)
		{
			CancelWait(pObjectArray,(__COMMON_OBJECT*)pKernelThread);
		}
		//Wait on all non-signal objects.
		if(!WaitTheseObjects(pObjectArray,nObjectNum,(__COMMON_OBJECT*)pKernelThread))  //Exception case,maybe caused by lack memory.
		{
			pKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_RUNNING;  //Restore.
			ClearMultipleWaitStatus(pKernelThread);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return OBJECT_WAIT_FAILED;
		}
		if(WAIT_TIME_INFINITE == dwTimeToWait)  //Should wait forever.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.ScheduleFromProc(NULL);
			switch(pKernelThread->dwWaitingStatus)
			{
			case OBJECT_WAIT_DELETED:
				*pnSignalObjectIndex = (pKernelThread->dwMultipleWaitFlags & 0x000000FF);
				ClearMultipleWaitStatus(pKernelThread);
				//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return OBJECT_WAIT_DELETED;
			case OBJECT_WAIT_RESOURCE:
				ClearMultipleWaitStatus(pKernelThread);
				bTryAgain = TRUE;
				goto __TRY_AGAIN;
			default:
				BUG();  //Should not occur this case.
				ClearMultipleWaitStatus(pKernelThread);
				//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return OBJECT_WAIT_FAILED;
			}
		}
		else  //Timeout waiting.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			dwRetVal = MultiTimeOutWaiting(pKernelThread,nObjectNum,dwTimeToWait);
			switch(dwRetVal)
			{
			case OBJECT_WAIT_RESOURCE:
				ClearMultipleWaitStatus(pKernelThread);
				bTryAgain = TRUE;
				goto __TRY_AGAIN;
			case OBJECT_WAIT_DELETED:
				*pnSignalObjectIndex = (pKernelThread->dwMultipleWaitFlags & 0x000000FF);
				ClearMultipleWaitStatus(pKernelThread);
				//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return OBJECT_WAIT_DELETED;
			case OBJECT_WAIT_TIMEOUT:
				ClearMultipleWaitStatus(pKernelThread);
				//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return OBJECT_WAIT_TIMEOUT;
			default:
				BUG();  //Should not occur this case.
				ClearMultipleWaitStatus(pKernelThread);
				//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return OBJECT_WAIT_FAILED;
			}
		}
	}
}

//A local helper routine,to process the case of waiting for any objects.
//This routine will be called by WaitForMultipleObjects.
static DWORD WaitForAny(__COMMON_OBJECT** pObjectArray,
						int nObjectNum,
						DWORD dwMillionSeconds,
						int* pnSignalObjectIndex)
{
	DWORD               dwRetVal  = OBJECT_WAIT_FAILED;

//__TERMINAL:
	return dwRetVal;
}

//The implementation of WaitForMultipleObjects.
DWORD WaitForMultipleObjects(__COMMON_OBJECT** pObjectArray,
							 int nObjectNum,
							 BOOL bWaitAll,
							 DWORD dwMillionSeconds,
							 int* pnSignalObjectIndex)
{
	DWORD                    dwRetVal       = OBJECT_WAIT_FAILED;
	int                      i;

	//Parameters' validation checking.
	if((NULL == pObjectArray) || (nObjectNum > MAX_MULTIPLE_WAIT_NUM))
	{
		goto __TERMINAL;
	}
	for(i = 0;i < nObjectNum;i ++)
	{
		if(!IsSynObject(pObjectArray[i]))
		{
			goto __TERMINAL;
		}
	}

	if(bWaitAll)
	{
		dwRetVal = WaitForAll(pObjectArray,nObjectNum,dwMillionSeconds,pnSignalObjectIndex);
	}
	else
	{
		dwRetVal = WaitForAny(pObjectArray,nObjectNum,dwMillionSeconds,pnSignalObjectIndex);
	}

__TERMINAL:
	return dwRetVal;
}
#endif
