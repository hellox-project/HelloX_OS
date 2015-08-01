//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 20,2014
//    Module Name               : synobj2.c
//    Module Funciton           : 
//                                This module countains synchronization object's implementation
//                                code,it's the second part of synobj.c file,since the master
//                                part's size is too large.
//                                The following synchronization object(s) is(are) implemented
//                                in this file:
//                                  1. SEMAPHORE
//                                  2. MAIL BOX
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      :
//    Last modified Date        : Jun 20,2014
//    Last modified Content     :
//                                1. Kernel object signature validation mechanism is added;
//                                2. WaitForMultipleObjects is implemented.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif
#include "system.h"
#include "types.h"
#include "ktmgr2.h"
#include "commobj.h"
#include <heap.h>
#include "hellocn.h"
#include "kapi.h"


//Change semaphore's default counter value.
//static
 BOOL SetSemaphoreCount(__COMMON_OBJECT* pSemaphore,DWORD dwMaxSem,DWORD dwCurrSem)
{
	__SEMAPHORE*  pSem = (__SEMAPHORE*)pSemaphore;

	if(NULL == pSem)
	{
		return FALSE;
	}
	if(0 == dwMaxSem)  //Maximal count can not be zero.
	{
		return FALSE;
	}

	if(dwMaxSem < dwCurrSem)  //Invalid value.
	{
		return FALSE;
	}
	//Change the default value.
	pSem->dwMaxSem  = dwMaxSem;
	pSem->dwCurrSem = dwCurrSem;
	return TRUE;
}

//ReleaseSemaphore's implementation.It increase the dwCurrSem's value,and wake up one 
//kernel thread if exist.The previous current counter will be returned in pdwPrevCount.
//static
 BOOL ReleaseSemaphore(__COMMON_OBJECT* pSemaphore,DWORD* pdwPrevCount)
{
	__SEMAPHORE*             pSem          = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT*  pKernelThread = NULL;
	DWORD                    dwFlags       = 0;

	if(NULL == pSem)
	{
		return FALSE;
	}

	//The following operation must not be interruptted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pdwPrevCount)  //Return the previous counter value.
	{
		*pdwPrevCount = pSem->dwCurrSem;
	}
	if(pSem->dwCurrSem < pSem->dwMaxSem)
	{
		pSem->dwCurrSem ++;
	}
	else  //Reach the maximal value,invalid operation.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	//Try to wake up one kernel thread.
	pKernelThread = (__KERNEL_THREAD_OBJECT*)pSem->lpWaitingQueue->GetHeaderElement(
		(__COMMON_OBJECT*)pSem->lpWaitingQueue,NULL);
	if(pKernelThread)  //Wakeup the kernel thread.
	{
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	KernelThreadManager.ScheduleFromProc(NULL);  //Reschedule all kernel threads.
	return TRUE;
}

//WaitForThisObject's implementation,it only calls the WaitForThisObjectEx routine by setting 
//the wait time to infinite.
//static
 DWORD WaitForSemObject(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE*            pSem          = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD                   dwRetValue    = OBJECT_WAIT_FAILED;
	DWORD                   dwFlags;

	if(NULL == pSem)
	{
		goto __TERMINAL;
	}

	//The following operation should not be interruptted.
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pSem->dwCurrSem > 0)  //Resource available.
	{
		pSem->dwCurrSem --;
		dwRetValue = OBJECT_WAIT_RESOURCE;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//Resource unavailable,wait it.
	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;  //Set waiting flag.
	pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
	//Add into semaphore's waiting queue.
	pSem->lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)pSem->lpWaitingQueue,
		(__COMMON_OBJECT*)pKernelThread,
		pKernelThread->dwThreadPriority);  //Will be waken up earlier if has higher priority.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Reschedule all kernel thread(s).
	KernelThreadManager.ScheduleFromProc(NULL);

	//The kernel thread is waken up when reach here.
	if(pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)  //Semaphore object is destroyed.
	{
		dwRetValue = OBJECT_WAIT_DELETED;
		goto __TERMINAL;
	}
	else  //Try again otherwise.
	{
		goto __TRY_AGAIN;
	}

__TERMINAL:
	return dwRetValue;
}

//The following routine resides in synobj.c file,it's a common used routine by all synchronizing
//objects.
extern DWORD TimeOutWaiting(__COMMON_OBJECT* pSynObject,__PRIORITY_QUEUE* pWaitingQueue,
							__KERNEL_THREAD_OBJECT* lpKernelThread,DWORD dwMillionSecond,
							VOID (*TimeOutCallback)(VOID*));

//Implementation of WaitForThisObjectEx routine,it decrement the dwCurrSem value,and block
//the current kernel thread when the current counter is zero.
//static
 DWORD WaitForSemObjectEx(__COMMON_OBJECT* pSemaphore,DWORD dwMillionSecond,DWORD* pdwWait)
{
	__SEMAPHORE*                pSem             = (__SEMAPHORE*)pSemaphore;
	__KERNEL_THREAD_OBJECT*     pKernelThread    = NULL;
	DWORD                       dwCalledTick     = System.dwClockTickCounter;  //Record the tick.
	DWORD                       dwTimeOutTick    = 0;
	//DWORD                       dwTimeSpan       = 0;
	DWORD                       dwRetValue       = OBJECT_WAIT_FAILED;
	DWORD                       dwFlags;

	if(NULL == pSem)
	{
		goto __TERMINAL;
	}

	//Calculate the timeout tick counter.
	dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
	dwTimeOutTick += System.dwClockTickCounter;

__TRY_AGAIN:
	//The following operation should not be interruptted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pSem->dwCurrSem)  //Resource available.
	{
		pSem->dwCurrSem --;
		dwRetValue = OBJECT_WAIT_RESOURCE;
		//Return the spent time.
		if(pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//Resource unavailable.
	if(0 == dwMillionSecond)  //Return immediately.
	{
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//Block the current kernel thread to wait.
	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
	pKernelThread->dwThreadStatus      = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->dwWaitingStatus    &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus    |= OBJECT_WAIT_WAITING;
	//Add to semaphore's waiting queue.
	pSem->lpWaitingQueue->InsertIntoQueue((__COMMON_OBJECT*)pSem->lpWaitingQueue,
		(__COMMON_OBJECT*)pKernelThread,
		pKernelThread->dwThreadPriority);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Call TimeOutWaiting routine.
	switch(TimeOutWaiting((__COMMON_OBJECT*)pSem,pSem->lpWaitingQueue,pKernelThread,dwMillionSecond,NULL))
	{
	case OBJECT_WAIT_RESOURCE:
		goto __TRY_AGAIN;     //Try to acquire resource again.
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

__TERMINAL:
	return dwRetValue;
}

//Semaphore's initializer.
BOOL SemInitialize(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE*      pSem            = (__SEMAPHORE*)pSemaphore;
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;
	BOOL              bResult         = FALSE;

	if(NULL == pSem)
	{
		goto __TERMINAL;
	}

	//Create waiting queue for kernel thread(s) want to wait for this object.
	lpPriorityQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,
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

	//Set default semaphore's counter.
	pSem->dwMaxSem       = 1;
	pSem->dwCurrSem      = 1;
	pSem->lpWaitingQueue = lpPriorityQueue;
	pSem->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;

	//Set operation functions accordingly.
	pSem->WaitForThisObject   = WaitForSemObject;
	pSem->WaitForThisObjectEx = WaitForSemObjectEx;
	pSem->SetSemaphoreCount   = SetSemaphoreCount;
	pSem->ReleaseSemaphore    = ReleaseSemaphore;

__TERMINAL:
	if(!bResult)
	{
		if(NULL != lpPriorityQueue)    //Release the priority queue.
		{
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpPriorityQueue);
		}
	}
	return bResult;
}

//Unitializer of semaphore object.
VOID SemUninitialize(__COMMON_OBJECT* pSemaphore)
{
	__SEMAPHORE*             pSem             = (__SEMAPHORE*)pSemaphore;
	__PRIORITY_QUEUE*        lpPriorityQueue  = NULL;
	__KERNEL_THREAD_OBJECT*  lpKernelThread   = NULL;
	DWORD                    dwFlags;

	if(NULL == pSem)
	{
		BUG();
		return;
	}

	//The following operation must not be interruptted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpPriorityQueue = pSem->lpWaitingQueue;
	if(0 == pSem->dwCurrSem)
	{
		//Should wake up all kernel thread(s) who waiting for this object.
		lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpPriorityQueue->GetHeaderElement((__COMMON_OBJECT*)lpPriorityQueue,NULL);
		while(lpKernelThread)
		{
			lpKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			lpKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			lpKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpPriorityQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpPriorityQueue,NULL);
		}
	}
	//Clear the kernel object's signature.
	pSem->dwObjectSignature = 0;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Destroy prirority queue of the semaphore.
	ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpPriorityQueue);
	return;
}

//**------------------------------------------------------------------------------------------------
//**
//**  Implementation of Mailbox object.
//**
//**------------------------------------------------------------------------------------------------

//WaitForMailboxObject's implementation,this is a empty implementation since it's not allowed
//to wait a mailbox object.
//static
 DWORD WaitForMailboxObject(__COMMON_OBJECT* pMailbox)
{
	return OBJECT_WAIT_FAILED;
}

//Implementation of SetMailboxSize,it release the previous allocated message array memory
//and allocates a new one accordingly.
//This routine must be called before any mailbox's sending or getting operation.
//static
 BOOL SetMailboxSize(__COMMON_OBJECT* pMailboxObj,DWORD dwNewSize)
{
	__MAIL_BOX*    pMailbox         = (__MAIL_BOX*)pMailboxObj;
	BOOL           bResult          = FALSE;
	__MB_MESSAGE*  pNewMessageArray = NULL;
	__MB_MESSAGE*  pOldMessageArray = NULL;
	DWORD          dwFlags;
	
	if((NULL == pMailbox) || (0 == dwNewSize))  //Invalid parameters.
	{
		goto __TERMINAL;
	}

	//Check if there is mail in mailbox.
	if(pMailbox->dwCurrMessageNum)
	{
		goto __TERMINAL;
	}

	//Check if there is pending kernel thread in getting queue.
	if(pMailbox->lpGettingQueue->dwCurrElementNum)
	{
		goto __TERMINAL;
	}

	//Allocate memory first.
	pNewMessageArray = (__MB_MESSAGE*)KMemAlloc(dwNewSize * sizeof(__MB_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pNewMessageArray)
	{
		goto __TERMINAL;
	}
	memset(pNewMessageArray,0,dwNewSize * sizeof(__MB_MESSAGE));
	pOldMessageArray = pMailbox->pMessageArray;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pMailbox->pMessageArray    = pNewMessageArray;
	pMailbox->dwMaxMessageNum  = dwNewSize;
	pMailbox->dwCurrMessageNum = 0;
	pMailbox->dwMessageHeader  = 0;
	pMailbox->dwMessageTail    = 0;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Release the old message queue.
	KMemFree(pOldMessageArray,KMEM_SIZE_TYPE_ANY,0);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(pNewMessageArray)  //Should release it.
		{
			KMemFree(pNewMessageArray,KMEM_SIZE_TYPE_ANY,0);
		}
	}
	return bResult;
}

//Get a message from mailbox.
//static
 DWORD GetMail(__COMMON_OBJECT* pMailboxObj,LPVOID* ppMessage,DWORD dwMillionSecond,DWORD* pdwWait)
{
	__MAIL_BOX*               pMailbox      = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT*   pKernelThread = NULL;
	DWORD                     dwFlags;
	DWORD                     dwCalledTick  = System.dwClockTickCounter;
	DWORD                     dwTimeOutTick = 0;
	//DWORD                     dwTimeSpan    = 0;
	DWORD                     dwTimeoutWait = 0;
	DWORD                     dwRetValue    = OBJECT_WAIT_FAILED;

	//Parameters checking.
	if((NULL == pMailbox) || (NULL == ppMessage))
	{
		goto __TERMINAL;
	}

	//Calculate the timeout tick counter.
	if(WAIT_TIME_INFINITE != dwMillionSecond)
	{
		dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
		dwTimeOutTick += System.dwClockTickCounter;
	}

	//Try to get mail
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pMailbox->dwCurrMessageNum)  //Mailbox contains message,just return it.
	{
		//There maybe blocked kernel thread waiting to send mail to box,so wake up one
		//if there exist.
		if(pMailbox->dwCurrMessageNum == pMailbox->dwMaxMessageNum)
		{
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpSendingQueue,NULL);
			if(pKernelThread)  //Wakeup the kernel thread.
			{
				pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
				pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
				pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
				KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
					pKernelThread);
			}
		}
		//Return the first message and update status variables.
		*ppMessage = pMailbox->pMessageArray[pMailbox->dwMessageHeader].pMessage;
		pMailbox->dwCurrMessageNum --;
		pMailbox->dwMessageHeader += 1;
		pMailbox->dwMessageHeader = pMailbox->dwMessageHeader % pMailbox->dwMaxMessageNum;
		//Return waiting time if necessary.
		if(pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}

	//Resource is unavailable,handle this scenario according to dwMillionSecond's value.
	if(0 == dwMillionSecond)  //Return immediately.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}

	if(WAIT_TIME_INFINITE == dwMillionSecond)  //Wait until mail available.
	{
		pKernelThread = KernelThreadManager.lpCurrentKernelThread;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;  //Set waiting flag.
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
		//Add into Mailbox's getting queue.
		pMailbox->lpGettingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);  //Reschedule,current thread will be wakeup when
		                                             //mail is available.
	}
	else  //Wait a specific time.
	{
		pKernelThread = KernelThreadManager.lpCurrentKernelThread;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
		//Add current kernel thread into mailbox's getting queue.
		pMailbox->lpGettingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwTimeoutWait = TimeOutWaiting((__COMMON_OBJECT*)pMailbox,
			pMailbox->lpGettingQueue,
			pKernelThread,
			dwMillionSecond,NULL);
	}

	//The kernel thread is waken up when reach here.
	if(WAIT_TIME_INFINITE != dwMillionSecond)
	{
		switch(dwTimeoutWait)
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
		if(pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)  //Semaphore object is destroyed.
		{
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		}
		else  //Try again otherwise.
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
//static
 VOID __SendMail(__MAIL_BOX* pMailbox,LPVOID pMessage,DWORD dwPriority)
{
	__MB_MESSAGE*           pMessageArray = pMailbox->pMessageArray;
	DWORD                   dwIndex       = pMailbox->dwMessageTail;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;

	//Check if there is waiting kernel thread pending in getting queue.
	//Wake up one if there exist.
	if(0 == pMailbox->dwCurrMessageNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,NULL);
		if(pKernelThread)  //Wakeup the kernel thread.
		{
			pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
		}
	}

	pMessageArray[dwIndex].dwPriority = dwPriority;
	pMessageArray[dwIndex].pMessage   = pMessage;
	pMailbox->dwCurrMessageNum  += 1;
	pMailbox->dwMessageTail     += 1;
	pMailbox->dwMessageTail     %= pMailbox->dwMaxMessageNum;
	return;
}

//Send a message to mailbox.
//static
 DWORD SendMail(__COMMON_OBJECT* pMailboxObj,LPVOID pMessage,DWORD dwPriority,DWORD dwMillionSecond,DWORD* pdwWait)
{
	__MAIL_BOX*               pMailbox      = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT*   pKernelThread = NULL;
	DWORD                     dwFlags;
	DWORD                     dwCalledTick  = System.dwClockTickCounter;
	DWORD                     dwTimeOutTick = 0;
	//DWORD                     dwTimeSpan    = 0;
	DWORD                     dwTimeoutWait = 0;
	DWORD                     dwRetValue    = OBJECT_WAIT_FAILED;

	//Parameters checking.
	if(NULL == pMailbox)
	{
		goto __TERMINAL;
	}

	//Calculate the timeout tick counter.
	if(WAIT_TIME_INFINITE != dwMillionSecond)
	{
		dwTimeOutTick = (dwMillionSecond / SYSTEM_TIME_SLICE) ? (dwMillionSecond / SYSTEM_TIME_SLICE) : 1;
		dwTimeOutTick += System.dwClockTickCounter;
	}

	//Try to put message into mailbox.
__TRY_AGAIN:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pMailbox->dwCurrMessageNum < pMailbox->dwMaxMessageNum)  //Mailbox has space to put.
	{
		__SendMail(pMailbox,pMessage,dwPriority);
		//Return wait time if necessary.
		if(pdwWait)
		{
			*pdwWait = (System.dwClockTickCounter - dwCalledTick) * SYSTEM_TIME_SLICE;
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwRetValue = OBJECT_WAIT_RESOURCE;
		goto __TERMINAL;
	}

	//It means there is no space in mailbox to contain new message when reach here.
	if(0 == dwMillionSecond)  //Return immediately.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwRetValue = OBJECT_WAIT_TIMEOUT;
		goto __TERMINAL;
	}

	if(WAIT_TIME_INFINITE == dwMillionSecond)  //Wait until mailbox has empty slot to contain mail.
	{
		pKernelThread = KernelThreadManager.lpCurrentKernelThread;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;  //Set waiting flag.
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
		//Add into Mailbox's getting queue.
		pMailbox->lpSendingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		KernelThreadManager.ScheduleFromProc(NULL);  //Reschedule,current thread will be wakeup when
		                                             //mail is available.
	}
	else  //Wait a specific time.
	{
		pKernelThread = KernelThreadManager.lpCurrentKernelThread;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
		//Add current kernel thread into mailbox's getting queue.
		pMailbox->lpSendingQueue->InsertIntoQueue(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue,
			(__COMMON_OBJECT*)pKernelThread,
			pKernelThread->dwThreadPriority);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		dwTimeoutWait = TimeOutWaiting((__COMMON_OBJECT*)pMailbox,
			pMailbox->lpSendingQueue,
			pKernelThread,
			dwMillionSecond,NULL);
	}

	//The kernel thread is waken up when reach here.
	if(WAIT_TIME_INFINITE != dwMillionSecond)
	{
		switch(dwTimeoutWait)
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
		if(pKernelThread->dwWaitingStatus & OBJECT_WAIT_DELETED)  //Semaphore object is destroyed.
		{
			dwRetValue = OBJECT_WAIT_DELETED;
			goto __TERMINAL;
		}
		else  //Try again otherwise.
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
	__MB_MESSAGE*      pMbMessage    = NULL;
	__MAIL_BOX*        pMailbox      = (__MAIL_BOX*)pMailboxObj;
	__PRIORITY_QUEUE*  pSendingQueue = NULL;
	__PRIORITY_QUEUE*  pGettingQueue = NULL;
	BOOL               bResult       = FALSE;

	if(NULL == pMailbox)
	{
		goto __TERMINAL;
	}

	//Allocate resources for mail box.
	pMbMessage = (__MB_MESSAGE*)KMemAlloc(sizeof(__MB_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pMbMessage)
	{
		goto __TERMINAL;
	}
	//Create waiting queues for sending and getting threads.
	pSendingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == pSendingQueue)
	{
		goto __TERMINAL;
	}
	bResult = pSendingQueue->Initialize((__COMMON_OBJECT*)pSendingQueue);
	if(!bResult)
	{
		goto __TERMINAL;
	}

	pGettingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == pGettingQueue)
	{
		goto __TERMINAL;
	}
	bResult = pGettingQueue->Initialize((__COMMON_OBJECT*)pGettingQueue);
	if(!bResult)
	{
		goto __TERMINAL;
	}

	//Assign members to mailbox object.
	pMailbox->pMessageArray       = pMbMessage;
	pMailbox->dwMaxMessageNum     = 1;    //Default is 1,can be changed by SetMailboxSize routine.
	pMailbox->dwCurrMessageNum    = 0;
	pMailbox->dwMessageHeader     = 0;
	pMailbox->dwMessageTail       = 0;
	pMailbox->lpSendingQueue      = pSendingQueue;
	pMailbox->lpGettingQueue      = pGettingQueue;
	pMailbox->WaitForThisObject   = WaitForMailboxObject;
	pMailbox->SetMailboxSize      = SetMailboxSize;
	pMailbox->SendMail            = SendMail;
	pMailbox->GetMail             = GetMail;

	pMailbox->dwObjectSignature   = KERNEL_OBJECT_SIGNATURE;

	bResult = TRUE;

__TERMINAL:
	if(!bResult)  //Initialization failed,destroy any resource successfully allocated.
	{
		if(pMbMessage)
		{
			KMemFree(pMbMessage,KMEM_SIZE_TYPE_ANY,0);
		}
		if(!pSendingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pSendingQueue);
		}
		if(!pGettingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pGettingQueue);
		}
	}
	return bResult;
}

//Uninitializer of mailbox object.
VOID MailboxUninitialize(__COMMON_OBJECT* pMailboxObj)
{
	__MAIL_BOX*              pMailbox       = (__MAIL_BOX*)pMailboxObj;
	__KERNEL_THREAD_OBJECT*  pKernelThread  = NULL;
	DWORD                    dwFlags        = 0;

	if(NULL == pMailbox)
	{
		return;
	}

	//Wake up all kernel thread(s) pending on the mail box.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(0 == pMailbox->dwCurrMessageNum)  //Getting queue may contains pending kernel thread(s).
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpGettingQueue,NULL);
		while(pKernelThread)
		{
			pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpGettingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpGettingQueue,NULL);
		}
	}
	if(pMailbox->dwCurrMessageNum == pMailbox->dwMaxMessageNum)  //Sending queue may contains pending kernel thread(s).
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pMailbox->lpSendingQueue,NULL);
		while(pKernelThread)
		{
			pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
			KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pMailbox->lpSendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pMailbox->lpSendingQueue,NULL);
		}
	}
	pMailbox->dwObjectSignature = 0;  //Clear signature of the object.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Release all resources of mailbox object.
	ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pMailbox->lpGettingQueue);
	ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pMailbox->lpSendingQueue);
	KMemFree(pMailbox->pMessageArray,KMEM_SIZE_TYPE_ANY,0);
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

//WaitForConditionObject's implementation,this is a empty implementation since it does not
//conforms the operations offered by POSIX standard condition object.
//static
 DWORD WaitForConditionObject(__COMMON_OBJECT* pCondObj)
{
	return OBJECT_WAIT_FAILED;
}

//Wait a specified condition object.
//static
 DWORD CondWait(__COMMON_OBJECT* pCondObj,__COMMON_OBJECT* pMutexObj)
{
	__CONDITION*               pCond         = (__CONDITION*)pCondObj;
	__MUTEX*                   pMutex        = (__MUTEX*)pMutexObj;
	__KERNEL_THREAD_OBJECT*    pKernelThread = NULL;
	DWORD                      dwFlags;
	DWORD                      dwResult      = OBJECT_WAIT_FAILED;

	if((NULL == pCond) || (NULL == pMutex))
	{
		goto __TERMINAL;
	}
	//Validate the kernel objects.
	if((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) || (KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature))
	{
		goto __TERMINAL;
	}

	//Put the current kernel thread into pending queue,and release mutex,in
	//one atomic operation.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
	pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	//Put the kernel thread into condition object's pending queue.
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread,pKernelThread->dwThreadPriority);
	pCond->nThreadNum ++;

	//Release the mutex object.
	if(pMutex->dwWaitingNum > 0)
	{
		pMutex->dwWaitingNum --;
		if(0 == pMutex->dwWaitingNum)
		{
			pMutex->dwMutexStatus = MUTEX_STATUS_FREE;
		}
		else  //Wakeup one kernel thread.
		{
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pCond->lpPendingQueue,
				NULL);
			if(NULL == pKernelThread)
			{
				BUG();
			}
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
		}
	}
	else  //This scenario should not exist,since at least current kernel thread is occupying it.
	{
		BUG();
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	KernelThreadManager.ScheduleFromProc(NULL);

	//Now the kernel thread must be waken up,then try to occupy the mutex object again
	//before return.
	dwResult = pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

__TERMINAL:
	return dwResult;
}

//Timeout call back for Condition object,will be called in TimeOutWaiting routine,
//to remove the kernel thread waiting on the condition from pending queue in case
//of waiting time out.
//static
 VOID CondTimeOutCallback(VOID* pData)
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
	((__CONDITION*)lpHandlerParam->lpSynObject)->nThreadNum --;
	//Add this kernel thread to ready queue.
	lpHandlerParam->lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
	KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		lpHandlerParam->lpKernelThread);
}

//Wait a condition object until the condition satisfied or time out.
//static
 DWORD CondWaitTimeout(__COMMON_OBJECT* pCondObj,__COMMON_OBJECT* pMutexObj,DWORD dwMillionSecond)
{
	__CONDITION*               pCond         = (__CONDITION*)pCondObj;
	__MUTEX*                   pMutex        = (__MUTEX*)pMutexObj;
	__KERNEL_THREAD_OBJECT*    pKernelThread = NULL;
	DWORD                      dwFlags;
	DWORD                      dwResult      = OBJECT_WAIT_FAILED;

	if((NULL == pCond) || (NULL == pMutex))
	{
		goto __TERMINAL;
	}
	//Validate the kernel objects.
	if((KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature) || (KERNEL_OBJECT_SIGNATURE != pMutex->dwObjectSignature))
	{
		goto __TERMINAL;
	}

	//Put the current kernel thread into pending queue,and release mutex,in
	//one atomic operation.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
	pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_BLOCKED;
	pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
	pKernelThread->dwWaitingStatus |= OBJECT_WAIT_WAITING;
	//Put the kernel thread into condition object's pending queue.
	pCond->lpPendingQueue->InsertIntoQueue((__COMMON_OBJECT*)pCond->lpPendingQueue,
		(__COMMON_OBJECT*)pKernelThread,pKernelThread->dwThreadPriority);
	pCond->nThreadNum ++;

	//Release the mutex object.
	if(pMutex->dwWaitingNum > 0)
	{
		pMutex->dwWaitingNum --;
		if(0 == pMutex->dwWaitingNum)
		{
			pMutex->dwMutexStatus = MUTEX_STATUS_FREE;
		}
		else  //Wakeup one kernel thread.
		{
			pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)pCond->lpPendingQueue,
				NULL);
			if(NULL == pKernelThread)
			{
				BUG();
			}
			pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
			pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
			pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				pKernelThread);
		}
	}
	else  //This scenario should not exist,since at least current kernel thread is occupying it.
	{
		BUG();
	}
	//Record the current kernel thread.
	pKernelThread = KernelThreadManager.lpCurrentKernelThread;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	
	dwResult = TimeOutWaiting((__COMMON_OBJECT*)pCond,pCond->lpPendingQueue,pKernelThread,
		dwMillionSecond,CondTimeOutCallback);

	//Now the kernel thread must be waken up,then try to occupy the mutex object again
	//before return.
	pMutex->WaitForThisObject((__COMMON_OBJECT*)pMutex);

__TERMINAL:
	return dwResult;
}

//Signal a condition object.
//static
 DWORD CondSignal(__COMMON_OBJECT* pCondObj)
{
	__CONDITION*            pCond         = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD                   dwFlags;

	if(NULL == pCond)
	{
		return 0;
	}
	//Validate the kernel object.
	if(KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pCond->nThreadNum)  //There is(are) pending kernel thread(s).
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue,NULL);
		if(NULL == pKernelThread)  //Should not occur.
		{
			BUG();
		}
		//Put the kernel thread object into ready queue.
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		//Update the pending kernel thread counter.
		pCond->nThreadNum --;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Reschedule.
	KernelThreadManager.ScheduleFromProc(NULL);
	return 1;
}

//Broadcast a condition object.
//static
 DWORD CondBroadcast(__COMMON_OBJECT* pCondObj)
{
	__CONDITION*            pCond         = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	DWORD                   dwFlags;

	if(NULL == pCond)
	{
		return 0;
	}
	//Validate the kernel object.
	if(KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return 0;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	while(pCond->nThreadNum)  //There is(are) pending kernel thread(s).
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue,NULL);
		if(NULL == pKernelThread)  //Should not occur.
		{
			BUG();
		}
		//Put the kernel thread object into ready queue.
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_RESOURCE;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		//Update the pending kernel thread counter.
		pCond->nThreadNum --;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Reschedule.
	KernelThreadManager.ScheduleFromProc(NULL);
	return 1;
}

//Initialization of Condition object.
BOOL ConditionInitialize(__COMMON_OBJECT* pCondObj)
{
	__CONDITION*         pCond          = (__CONDITION*)pCondObj;
	__PRIORITY_QUEUE*    pPendingQueue  = NULL;
	BOOL                 bResult        = FALSE;

	if(NULL == pCond)
	{
		goto __TERMINAL;
	}

	//Create the pending queue object of condition,which is used to contain
	//kernel thread(s) waiting on the CONDITION object.
	pPendingQueue = (__PRIORITY_QUEUE*)
		ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == pPendingQueue)
	{
		goto __TERMINAL;
	}
	if(!pPendingQueue->Initialize((__COMMON_OBJECT*)pPendingQueue))
	{
		goto __TERMINAL;
	}

	//Initialize the CONDITION object.
	pCond->nThreadNum       = 0;
	pCond->lpPendingQueue   = pPendingQueue;
	pCond->CondWait         = CondWait;
	pCond->CondWaitTimeout  = CondWaitTimeout;
	pCond->CondSignal       = CondSignal;
	pCond->CondBroadcast    = CondBroadcast;

	//Set the signature of kernel object,to identify this object is a valid
	//kernel object.
	pCond->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;

	bResult = TRUE;
__TERMINAL:
	if(!bResult)  //Release all resources allocated to the CONDITION object.
	{
		if(!pPendingQueue)
		{
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pPendingQueue);
		}
	}
	return bResult;
}

//Uninitialization of Condition object.
VOID ConditionUninitialize(__COMMON_OBJECT* pCondObj)
{
	__CONDITION*             pCond          = (__CONDITION*)pCondObj;
	__KERNEL_THREAD_OBJECT*  pKernelThread  = NULL;
	DWORD                    dwFlags;

	if(NULL == pCond)
	{
		return;
	}
	//Check if the specified condition object is a valid kernel thread.
	if(KERNEL_OBJECT_SIGNATURE != pCond->dwObjectSignature)
	{
		return;
	}

	//Wakeup all pending kernel thread(s) if there is(are).
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	while(pCond->nThreadNum)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pCond->lpPendingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)pCond->lpPendingQueue,NULL);
		if(NULL == pKernelThread)  //Should not occur.
		{
			BUG();
		}
		//Set the waiting flags of the kernel thread and insert it into ready queue.
		pKernelThread->dwThreadStatus   = KERNEL_THREAD_STATUS_READY;
		pKernelThread->dwWaitingStatus &= ~OBJECT_WAIT_MASK;
		pKernelThread->dwWaitingStatus |= OBJECT_WAIT_DELETED;
		KernelThreadManager.AddReadyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pKernelThread);
		pCond->nThreadNum --;
	}
	pCond->dwObjectSignature = 0;  //Clear the kernel object signature.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Destroy the pending queue object.
	ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pCond->lpPendingQueue);
	return;
}
