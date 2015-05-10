//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,29 2005
//    Module Name               : COMQUEUE.CPP
//    Module Funciton           : 
//                                This module countains common queue's implementation code.
//                                The common queue is a FIFO queue,the queue element can
//                                be anything,i.e,the type is not limited.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "comqueue.h"

//
//Declare static routines in this module(file).
//
static BOOL    InsertIntoQueue(__COMMON_OBJECT* lpThis,LPVOID lpObject);
static LPVOID  GetFromQueue(__COMMON_OBJECT* lpThis);
static BOOL    CommQueueFull(__COMMON_OBJECT* lpThis);
static BOOL    CommQueueEmpty(__COMMON_OBJECT* lpThis);
static DWORD   SetQueueLength(__COMMON_OBJECT* lpThis,DWORD dwNewLen);
static DWORD   GetQueueLength(__COMMON_OBJECT* lpThis);
static DWORD   GetCurrLength(__COMMON_OBJECT* lpThis);

//
//The implementation of CommQueueInit routine.
//
BOOL CommQueueInit(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*                 lpComQueue = (__COMMON_QUEUE*)lpThis;

	if(NULL == lpThis)  //Parameter check.
		return FALSE;

	//AtomicSet(&lpComQueue->QueueLen,DEFAULT_COMMON_QUEUE_LEN);
	//AtomicSet(&lpComQueue->CurrentLen,0);
	lpComQueue->dwQueueLen         = DEFAULT_COMMON_QUEUE_LEN;
	lpComQueue->dwCurrentLen       = 0;
	INIT_COMMON_QUEUE_ELEMENT(&lpComQueue->QueueHdr); //Initialize queue's header.

	lpComQueue->InsertIntoQueue    = InsertIntoQueue;
	lpComQueue->GetFromQueue       = GetFromQueue;
	lpComQueue->QueueEmpty         = CommQueueEmpty;
	lpComQueue->QueueFull          = CommQueueFull;
	lpComQueue->SetQueueLength     = SetQueueLength;
	lpComQueue->GetQueueLength     = GetQueueLength;
	lpComQueue->GetCurrLength      = GetCurrLength;

	return TRUE;
}

//
//The implementation of CommQueueUninit routine.
//
VOID CommQueueUninit(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*           lpCommQueue   = (__COMMON_QUEUE*)lpThis;
	__COMMON_QUEUE_ELEMENT*   lpQueueEle    = NULL;
	__COMMON_QUEUE_ELEMENT*   lpCurrEle     = NULL;
	
	if(NULL == lpThis) //Invalidate parameter.
		return;
	lpQueueEle = &lpCommQueue->QueueHdr;
	while(lpQueueEle->lpNext != lpQueueEle)    //Delete all elements in the common queue.
	{
		lpCurrEle  = lpQueueEle;
		lpQueueEle = lpQueueEle->lpNext;
		FREE_QUEUE_ELEMENT(lpCurrEle);    //Release the queue element's memory space.
	}
	return;
}

//
//The implementation of InsertIntoQueue.
//This routine allocate a queue element object,initialize it,and 
//put the element object into queue.
//
static BOOL    InsertIntoQueue(__COMMON_OBJECT* lpThis,LPVOID lpObject)
{
	__COMMON_QUEUE*            lpCommQueue = (__COMMON_QUEUE*)lpThis;
	__COMMON_QUEUE_ELEMENT*    lpQueueEle  = NULL;
	DWORD                      dwFlags     = 0;

	if(NULL == lpThis)    //Invalidate parameter.
		return FALSE;
	if(CommQueueFull(lpThis)) //The current queue is full.
		return FALSE;

	lpQueueEle = ALLOCATE_QUEUE_ELEMENT();
	if(NULL == lpQueueEle)  //Can not allocate queue element.
		return FALSE;
	lpQueueEle->lpObject = lpObject;
	//
	//The following code puts the queue element into comnon queue,
	//and increase current queue element number.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpQueueEle->lpNext = lpCommQueue->QueueHdr.lpNext;
	lpQueueEle->lpPrev = &lpCommQueue->QueueHdr;
	lpCommQueue->QueueHdr.lpNext->lpPrev = lpQueueEle;
	lpCommQueue->QueueHdr.lpNext = lpQueueEle;
	lpCommQueue->dwCurrentLen ++;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return TRUE;
}

//
//The implementation of GetFromQueue routine.
//
static LPVOID  GetFromQueue(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*         lpCommQueue  = (__COMMON_QUEUE*)lpThis;
	__COMMON_QUEUE_ELEMENT* lpQueueEle   = NULL;
	DWORD                   dwFlags      = 0;
	LPVOID                  lpObject     = NULL;

	if(NULL == lpThis)
		return NULL;
	if(CommQueueEmpty(lpThis))  //The current queue is empty.
		return NULL;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpQueueEle = lpCommQueue->QueueHdr.lpPrev;
	lpQueueEle->lpNext->lpPrev = lpQueueEle->lpPrev;
	lpQueueEle->lpPrev->lpNext = lpQueueEle->lpNext;
	lpCommQueue->dwCurrentLen --;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	lpObject = lpQueueEle->lpObject;

	FREE_QUEUE_ELEMENT(lpQueueEle);    //Release the queue element object.
	return lpObject;
}

//
//The implementation of CommQueueEmpty.
//This routine check the current element number in the queue,if it is 0,then returns
//TRUE,else,returns FALSE.
//
static BOOL    CommQueueEmpty(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*         lpComQueue  = (__COMMON_QUEUE*)lpThis;
	DWORD                   dwFlags;
	BOOL                    bResult     = FALSE;

	if(NULL == lpThis)
		return FALSE;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	bResult = (lpComQueue->dwCurrentLen == 0);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return bResult;
}

//
//The implementation of CommQueueFull.
//This routine checks the current element number of current queue.
//If it equals the queue number,then returns TRUE,else,
//returns FALSE.
//
static BOOL    CommQueueFull(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*       lpComQueue = (__COMMON_QUEUE*)lpThis;
	DWORD                 dwFlags    = 0;
	BOOL                  bResult    = FALSE;
	if(NULL == lpThis)
		return FALSE;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	bResult = (lpComQueue->dwCurrentLen == lpComQueue->dwQueueLen);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return bResult;
}

//
//The implementation of SetQueueLength.
//This routine sets the queue's length to dwNewLen,and returns the old
//value of queue length.
//
static DWORD   SetQueueLength(__COMMON_OBJECT* lpThis,DWORD dwNewLen)
{
	__COMMON_QUEUE*          lpComQueue = (__COMMON_QUEUE*)lpThis;
	DWORD                    dwFlags    = 0;
	DWORD                    dwOldLen   = 0;

	if(NULL == lpThis)
		return 0;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwOldLen = lpComQueue->dwQueueLen;
	lpComQueue->dwQueueLen = dwNewLen;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return dwOldLen;
}

//
//The implementation of GetQueueLength.
//This routine returns the queue's length.
//
static DWORD   GetQueueLength(__COMMON_OBJECT* lpThis)
{
	__COMMON_QUEUE*           lpComQueue = (__COMMON_QUEUE*)lpThis;
	if(NULL == lpThis)
	{
		return 0;
	}
	return lpComQueue->dwQueueLen;
}

//
//The implementation of GetCurrLength.
//This routine returns the current length of a common queue object.
//
static DWORD   GetCurrLength(__COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)
	{
		return 0;
	}
	return ((__COMMON_QUEUE*)lpThis)->dwCurrentLen;
}


