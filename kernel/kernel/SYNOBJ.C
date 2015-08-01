//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 21, 2005
//    Module Name               : SYNOBJ.CPP
//    Module Funciton           : 
//                                This module countains synchronization object's implementation
//                                code.
//                                The following synchronization object(s) is(are) defined
//                                in this file:
//                                  1. EVENT
//                                  2. MUTEX
//                                  3. SEMAPHORE
//                                  4. TIMER
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      :
//    Last modified Date        : Mar 10,2012
//    Last modified Content     :
//                                1. Kernel object signature validation mechanism is added;
//                                2. WaitForMultipleObjects is implemented.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "types.h"
#include "commobj.h"
#include "ktmgr.h"
#include "system.h"
#include "hellocn.h"
#include "kapi.h"


//Timer handler routine for all synchronous object.
static DWORD WaitingTimerHandler(LPVOID lpData)
{
	__TIMER_HANDLER_PARAM*   lpHandlerParam = (__TIMER_HANDLER_PARAM*)lpData;
	DWORD                    dwFlags;

	if(NULL == lpHandlerParam)
	{
		BUG();
		return 0;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Acquire kernel thread object's spinlock.
	switch(lpHandlerParam->lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
	{
		case OBJECT_WAIT_RESOURCE:
		case OBJECT_WAIT_DELETED:
			break;
		case OBJECT_WAIT_WAITING:
			if(lpHandlerParam->TimeOutCallback)  //Call the specified time out call back.
			{
				lpHandlerParam->TimeOutCallback((VOID*)lpHandlerParam);
			}
			else  //Use the default implementation,it works for most synchronization kernel objects.
			{
				lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
				lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
				//Delete the lpKernelThread from waiting queue.
				lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
					(__COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
					(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread);
				//Also should decrement reference counter of the synchronization object if it has.
				//Add this kernel thread to ready queue.
				lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
				KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
					lpHandlerParam->lpKernelThread);
			}
			break;
		default:
			BUG();
			break;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return 0;
}

//
//TimeOutWaiting is a global routine used by any synchronous objects' WaitForThisObjectEx
//routine.
//
DWORD TimeOutWaiting(__COMMON_OBJECT* lpSynObject,      //Synchronous object.
					 __PRIORITY_QUEUE* lpWaitingQueue,  //Waiting queue.
					 __KERNEL_THREAD_OBJECT* lpKernelThread,  //Who want to wait.
					 DWORD dwMillionSecond,  //Time out value in millionsecond.
					 VOID (*TimeOutCallback)(VOID*))  //Call back routine when timeout.
{
	__TIMER_OBJECT*           lpTimerObj;
	__TIMER_HANDLER_PARAM     HandlerParam;

	if((NULL == lpSynObject) || (NULL == lpWaitingQueue) ||
	   (NULL == lpKernelThread) || (0 == dwMillionSecond))  //Invalid parameters.
	{
		BUG();
		return OBJECT_WAIT_FAILED;
	}

	//Initialize HandlerParam.
	HandlerParam.lpKernelThread  = lpKernelThread;
	HandlerParam.lpSynObject     = lpSynObject;
	HandlerParam.lpWaitingQueue  = lpWaitingQueue;
	HandlerParam.TimeOutCallback = TimeOutCallback;

	//lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	//lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;

	//Set a one time timer.
	lpTimerObj = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
		lpKernelThread,
#define TIMEOUT_WAITING_TIMER_ID 2048
		TIMEOUT_WAITING_TIMER_ID,
		dwMillionSecond,
		WaitingTimerHandler,
		(LPVOID)&HandlerParam,
		TIMER_FLAGS_ONCE);
	if(NULL == lpTimerObj)
	{
		return OBJECT_WAIT_FAILED;
	}

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.

	//Once reach here,it means the waiting kernel thread was waken up.
	switch(lpKernelThread->dwWaitingStatus & OBJECT_WAIT_MASK)
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

//
//Routines pre-declaration.
//
static DWORD kWaitForEventObject(__COMMON_OBJECT*);
static DWORD kSetEvent(__COMMON_OBJECT*);
static DWORD kResetEvent(__COMMON_OBJECT*);
static DWORD kWaitForEventObjectEx(__COMMON_OBJECT*,DWORD);

//---------------------------------------------------------------------------------
//
//                SYNCHRONIZATION OBJECTS
//
//----------------------------------------------------------------------------------

//
//Event object's initializing routine.
//This routine initializes the members of an event object.
//

BOOL EventInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                  bResult          = FALSE;
	__EVENT*              lpEvent          = NULL;
	__PRIORITY_QUEUE*     lpPriorityQueue  = NULL;

	if(NULL == lpThis)
		goto __TERMINAL;

	lpEvent = (__EVENT*)lpThis;

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

	lpEvent->lpWaitingQueue      = lpPriorityQueue;
	lpEvent->dwEventStatus       = EVENT_STATUS_OCCUPIED;
	lpEvent->SetEvent            = kSetEvent;
	lpEvent->ResetEvent          = kResetEvent;
	lpEvent->WaitForThisObjectEx = kWaitForEventObjectEx;
	lpEvent->WaitForThisObject   = kWaitForEventObject;
	lpEvent->dwObjectSignature   = KERNEL_OBJECT_SIGNATURE;
	bResult                      = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpPriorityQueue)    //Release the priority queue.
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpPriorityQueue);
	}
	return bResult;
}

//
//Event object's uninitializing routine.
//Safety deleted is support by EVENT object,so in this routine,
//if there are kernel threads waiting for this object,then wakeup
//all kernel threads,and then destroy the event object.
//

VOID EventUninitialize(__COMMON_OBJECT* lpThis)
{
	__EVENT*                 lpEvent          = NULL;
	__PRIORITY_QUEUE*        lpPriorityQueue  = NULL;
	__KERNEL_THREAD_OBJECT*  lpKernelThread   = NULL;
	DWORD                    dwFlags;

	if(NULL == lpThis)
	{
		BUG();
		return;
	}

	lpEvent = (__EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpPriorityQueue = lpEvent->lpWaitingQueue;
	if(EVENT_STATUS_FREE != EVENT_STATUS_FREE)
	{
		//Should wake up all kernel thread(s) who waiting for this object.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpPriorityQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpPriorityQueue,
			NULL);
		while(lpKernelThread)
		{
			lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			lpKernelThread = (__KERNEL_THREAD_OBJECT*)
				lpPriorityQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpPriorityQueue,
				NULL);
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Clear the kernel object's signature.
	lpEvent->dwObjectSignature = 0;

	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpPriorityQueue);          //*******CAUTION!!!************
	return;
}

//
//The implementation of SetEvent.
//This routine do the following:
// 1. Saves the previous status into a local variable;
// 2. Sets the current status of the event to EVENT_STATUS_FREE;
// 3. Wakes up all kernel thread(s) in it's waiting queue.
// 4. Returns the previous status.
//
//static
DWORD kSetEvent(__COMMON_OBJECT* lpThis)
{
	DWORD                     dwPreviousStatus     = EVENT_STATUS_OCCUPIED;
	__EVENT*                  lpEvent              = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread       = NULL;
	DWORD                     dwFlags              = 0;

	if(NULL == lpThis)
	{
		return dwPreviousStatus;
	}

	lpEvent = (__EVENT*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	lpEvent->dwEventStatus = EVENT_STATUS_FREE;    //Set the current status to free.

	//Wake up all kernel thread(s) waiting for this event.
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)
		lpEvent->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
		NULL);
	while(lpKernelThread)                         //Remove all kernel thread(s) from
		                                          //waiting queue.
	{
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
		//Set waiting result bit.
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);  //Add to ready queue.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)
			lpEvent->lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(IN_KERNELTHREAD())  //Current context is in process.
	{
		KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule.
	}
	return dwPreviousStatus;
}

//
//The implementation of ResetEvent.
//

static
DWORD kResetEvent(__COMMON_OBJECT* lpThis)
{
	__EVENT*          lpEvent          = (__EVENT*)lpThis;
	DWORD             dwPreviousStatus = 0;
	DWORD             dwFlags          = 0;

	if(NULL == lpEvent)
	{
		return dwPreviousStatus;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPreviousStatus = lpEvent->dwEventStatus;
	lpEvent->dwEventStatus = EVENT_STATUS_OCCUPIED;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return dwPreviousStatus;
}

//
//The implementation of WaitForEventObject.
//

//static
DWORD kWaitForEventObject(__COMMON_OBJECT* lpThis)
{
	__EVENT*                      lpEvent             = (__EVENT*)lpThis;
	__KERNEL_THREAD_OBJECT*       lpKernelThread      = NULL;
	//__KERNEL_THREAD_CONTEXT*      lpContext           = NULL;
	DWORD                         dwFlags             = 0;

	if(NULL == lpEvent)
	{
		return OBJECT_WAIT_FAILED;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	else
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);    //Leave critical section here is safety.
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return OBJECT_WAIT_RESOURCE;
}

//
//WaitForEventObjectEx's implementation.
//
//static
DWORD kWaitForEventObjectEx(__COMMON_OBJECT* lpObject,DWORD dwMillionSecond)
{
	__EVENT*                      lpEvent         = (__EVENT*)lpObject;
	__KERNEL_THREAD_OBJECT*       lpKernelThread  = NULL;
	DWORD                         dwFlags;
	DWORD                         dwTimeOutTick;
	DWORD                         dwTimeSpan;

	if(NULL == lpObject)
	{
		return OBJECT_WAIT_FAILED;
	}

	dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? 
		(dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
	dwTimeOutTick += System.dwClockTickCounter;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Should not be interrupted.
	if(EVENT_STATUS_FREE == lpEvent->dwEventStatus)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	//Should waiting now.
	if(0 == dwMillionSecond)  //Waiting zero time.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);
		return OBJECT_WAIT_TIMEOUT;
	}
	lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
	while(EVENT_STATUS_FREE != lpEvent->dwEventStatus)
	{
		if(dwTimeOutTick <= System.dwClockTickCounter)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return OBJECT_WAIT_TIMEOUT;
		}
		dwTimeSpan = (dwTimeOutTick - System.dwClockTickCounter) * SYSTEM_TIME_SLICE;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		//Add to event object's waiting queue.
		lpEvent->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpEvent->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		
		switch(TimeOutWaiting((__COMMON_OBJECT*)lpEvent,lpEvent->lpWaitingQueue,
			lpKernelThread,dwTimeSpan,NULL))
		{
			case OBJECT_WAIT_RESOURCE:  //Should loop to while again to check the status.
				__ENTER_CRITICAL_SECTION(NULL,dwFlags);
				break;
			case OBJECT_WAIT_TIMEOUT:
				return OBJECT_WAIT_TIMEOUT;
			case OBJECT_WAIT_DELETED:
				return OBJECT_WAIT_DELETED;
			default:
				BUG();
				return OBJECT_WAIT_FAILED;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return OBJECT_WAIT_RESOURCE;
}

////////////////////////////////////////////////////////////////////////////////////
//
// ------------------ ** The implementation of MUTEX object ** ---------------------
//
///////////////////////////////////////////////////////////////////////////////////

//
//The implementation of ReleaseMutex.
//
static
DWORD kReleaseMutex(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT*     lpKernelThread   = NULL;
	__MUTEX*                    lpMutex          = NULL;
	DWORD                       dwPreviousStatus = 0;
	DWORD                       dwFlags          = 0;

	if(NULL == lpThis)    //Parameter check.
		return 0;

	lpMutex = (__MUTEX*)lpThis;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMutex->dwWaitingNum > 0)    //If there are other kernel threads waiting for this object.
	{
		lpMutex->dwWaitingNum --;    //Decrement the counter.
	}
	if(0 == lpMutex->dwWaitingNum)   //There is no kernel thread waiting for the object.
	{
		dwPreviousStatus = lpMutex->dwMutexStatus;
		lpMutex->dwMutexStatus = MUTEX_STATUS_FREE;  //Set to free.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0;
	}
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpMutex->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
		0);  //Get one waiting kernel thread to run.
	lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
	KernelThreadManager.AddReadyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		lpKernelThread);  //Put the kernel thread to ready queue.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule kernel thread.
	return dwPreviousStatus;
}

//
//The implementation of WaitForMutexObject.
//

static DWORD WaitForMutexObject(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT*        lpKernelThread   = NULL;
	__MUTEX*                       lpMutex          = (__MUTEX*)lpThis;
	DWORD                          dwFlags          = 0;

	if(NULL == lpMutex)    //Parameter check.
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)    //If the current mutex is free.
	{
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;  //Modify the current status.
		lpMutex->dwWaitingNum  ++;    //Increment the counter.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return OBJECT_WAIT_RESOURCE;  //The current kernel thread successfully occupy
		                              //the mutex.
	}
	else    //The status of the mutex is occupied.
	{
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpMutex->dwWaitingNum          ++;    //Increment the waiting number.
		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);  //Leave critical section here is safety.
		//Reschedule all kernel thread(s).
		KernelThreadManager.ScheduleFromProc(NULL);
	}
	return OBJECT_WAIT_RESOURCE;
}

//Timeout call back for Mutex object,will be called in TimeOutWaiting routine,
//to remove the kernel thread waiting on the mutex from pending queue in case
//of waiting time out.
static VOID MutexTimeOutCallback(VOID* pData)
{
	__TIMER_HANDLER_PARAM*    lpHandlerParam = (__TIMER_HANDLER_PARAM*)pData;

	if(NULL == lpHandlerParam)  //Shoud not occur.
	{
		BUG();
	}
	lpHandlerParam->lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	lpHandlerParam->lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_TIMEOUT;
	//Delete the lpKernelThread from waiting queue.
	lpHandlerParam->lpWaitingQueue->DeleteFromQueue(
		(__COMMON_OBJECT*)lpHandlerParam->lpWaitingQueue,
		(__COMMON_OBJECT*)lpHandlerParam->lpKernelThread);
	//Also should decrement reference counter of the MUTEX object.
	((__MUTEX*)lpHandlerParam->lpSynObject)->dwWaitingNum --;
	//Add this kernel thread to ready queue.
	lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		lpHandlerParam->lpKernelThread);
}

//
//Implementation of WaitForThisObjectEx routine.
//This routine is a time out waiting routine,caller can give a time value
//to indicate how long want to wait,once exceed the time value,waiting operation
//will return,even in case of the resource is not released.
//If the time value is zero,then this routine will check the current status of
//mutex object,if free,then occupy the object and return RESOURCE,else return
//TIMEOUT,and a re-schedule is triggered.
//
static DWORD WaitForMutexObjectEx(__COMMON_OBJECT* lpThis,DWORD dwMillionSecond)
{
	__MUTEX*                      lpMutex        = (__MUTEX*)lpThis;
	__KERNEL_THREAD_OBJECT*       lpKernelThread = NULL;
	DWORD                         dwFlags;
	DWORD                         dwResult       = OBJECT_WAIT_FAILED;

	if(NULL == lpMutex)
	{
		return OBJECT_WAIT_FAILED;
	}
	
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(MUTEX_STATUS_FREE == lpMutex->dwMutexStatus)  //Free now.
	{
		lpMutex->dwMutexStatus = MUTEX_STATUS_OCCUPIED;
		lpMutex->dwWaitingNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		//KernelThreadManager.ScheduleFromProc(NULL);  //Re-schedule here.
		return OBJECT_WAIT_RESOURCE;
	}
	else  //The mutex is not free now.
	{
		if(0 == dwMillionSecond)
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.ScheduleFromProc(NULL); //Re-schedule here.
			return OBJECT_WAIT_TIMEOUT;
		}
		lpKernelThread = KernelThreadManager.lpCurrentKernelThread;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		//Waiting on mutex's waiting queue.
		lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_BLOCKED;
		lpMutex->dwWaitingNum ++;  //Added in 2015-04-06.
		lpMutex->lpWaitingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)lpMutex->lpWaitingQueue,
			(__COMMON_OBJECT*)lpKernelThread,
			0);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

		return TimeOutWaiting((__COMMON_OBJECT*)lpMutex,
			lpMutex->lpWaitingQueue,lpKernelThread,dwMillionSecond,MutexTimeOutCallback);
	}
}

//
//The implementation of MutexInitialize.
//
BOOL MutexInitialize(__COMMON_OBJECT* lpThis)
{
	__MUTEX*             lpMutex     = (__MUTEX*)lpThis;
	__PRIORITY_QUEUE*    lpQueue     = NULL;
	BOOL                 bResult     = FALSE;

	if(NULL == lpMutex) //Parameter check.
	{
		return bResult;
	}
	lpQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpQueue)    //Failed to create priority queue.
	{
		return bResult;
	}

	if(!lpQueue->Initialize((__COMMON_OBJECT*)lpQueue))  //Initialize the queue object.
	{
		goto __TERMINAL;
	}

	lpMutex->dwMutexStatus     = MUTEX_STATUS_FREE;
	lpMutex->lpWaitingQueue    = lpQueue;
	lpMutex->WaitForThisObject = WaitForMutexObject;
	lpMutex->dwWaitingNum      = 0;
	lpMutex->ReleaseMutex      = kReleaseMutex;
	lpMutex->WaitForThisObjectEx = WaitForMutexObjectEx;
	lpMutex->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	bResult = TRUE;    //Successful to initialize the mutex object.

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpQueue)    //Release the queue object.
		{
			ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpQueue);
		}
	}
	return bResult;
}

//
//The implementation of MutexUninitialize.
//This object support safety deleted,so in this routine,all kernel thread(s)
//must be waken up before this object is destroyed.
//
VOID MutexUninitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE*       lpWaitingQueue  = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread  = NULL;
	DWORD                   dwFlags;

	if(NULL == lpThis) //parameter check.
	{
		BUG();
		return;
	}

	lpWaitingQueue = ((__MUTEX*)lpThis)->lpWaitingQueue;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpWaitingQueue,
		NULL);
	while(lpKernelThread)
	{
		lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			lpKernelThread);
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpWaitingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpWaitingQueue,
			NULL);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Reset kernel object's signature.
	((__MUTEX*)lpThis)->dwObjectSignature = 0;
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpWaitingQueue);
	return;
}

//------------------------------------------------------------------------
//
//  The implementation of WaitForMultipleObjects,which is a new system
//  service added in version 1.80.
//
//------------------------------------------------------------------------

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

//Cancel the specified kernel thread from waiting queue of the given object array.
static VOID CancelWait(__COMMON_OBJECT** pObjectArray,__COMMON_OBJECT* pKernelThread)
{
	int                i;
	__PRIORITY_QUEUE*  pPriorityQueue = NULL;

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

//Timer handler for multiple waiting.
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
static DWORD MultiTimeOutWaiting(__KERNEL_THREAD_OBJECT* pKernelThread,int nObjectNum,DWORD dwMillionSeconds)
{
	__TIMER_OBJECT*           lpTimerObj;

	//pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	//pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;

	//Set a one time timer.
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

//A local helper routine,to process the case of waiting for all objects.
//This routine will be called by WaitForMultipleObjects.
static DWORD WaitForAll(__COMMON_OBJECT** pObjectArray,
						int nObjectNum,
						DWORD dwMillionSeconds,
						int* pnSignalObjectIndex)
{
	DWORD                         dwRetVal      = OBJECT_WAIT_FAILED;
	DWORD                         dwFlags;
	DWORD                         dwTimeToWait  = dwMillionSeconds;
	__KERNEL_THREAD_OBJECT*       pKernelThread = NULL;
	BOOL                          bTryAgain     = FALSE;
	int                           i;

	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
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

	//Should not reach here.
	//BUG();
	//return OBJECT_WAIT_FAILED;
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
