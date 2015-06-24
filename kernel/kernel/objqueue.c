//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : objqueue.cpp
//    Module Funciton           : 
//                                This module countains Object Queue's implementation code.
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
#ifndef __TYPES_H__
#include "types.h"
#endif

#include "commobj.h"
#include "kmemmgr.h"
#include "objqueue.h"
#include "hellocn.h"
#include "kapi.h"

//
//Insert an element into Priority Queue.
//This routine insert an common object into priority queue,it's position in the queue is
//determined by the object's priority(dwPriority parameter).
//

static BOOL InsertIntoQueue(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpObject,DWORD dwPriority)
{
    __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    __PRIORITY_QUEUE_ELEMENT* lpTmpElement = NULL;
    __PRIORITY_QUEUE*         lpQueue   = (__PRIORITY_QUEUE*)lpThis;
    DWORD                     dwFlags   = 0;

	if((NULL == lpThis) || (NULL == lpObject)) //Invalid parameters.
    {
        return FALSE;
    }
    
    //Allocate a queue element,and initialize it.
    lpElement = (__PRIORITY_QUEUE_ELEMENT*)
		KMemAlloc(sizeof(__PRIORITY_QUEUE_ELEMENT),KMEM_SIZE_TYPE_ANY);
    if(NULL == lpElement)  //Can not allocate the memory.
    {
        return FALSE;
    }

    lpElement->lpObject       = lpObject;
    lpElement->dwPriority     = dwPriority;

    //Now,insert the element into queue list.
    __ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Atomic operation.
    lpQueue->dwCurrElementNum ++;  //Increment element number.
    lpTmpElement = lpQueue->ElementHeader.lpPrevElement;

    //Find the appropriate position according to priority to insert.
    while((lpTmpElement->dwPriority < dwPriority) &&
          (lpTmpElement != &lpQueue->ElementHeader))
    {
        lpTmpElement = lpTmpElement->lpPrevElement;
    }
    //Insert the element into list.
    lpElement->lpNextElement   = lpTmpElement->lpNextElement;
    lpElement->lpPrevElement   = lpTmpElement;
    lpTmpElement->lpNextElement->lpPrevElement = lpElement;
    lpTmpElement->lpNextElement = lpElement;
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);

    return TRUE;

}

//
//Delete an element from Priority Queue.
//This routine searchs the queue,to find the object to be deleted,
//if find,delete the object from priority queue,returns TRUE,else,
//returns FALSE.
//If the object is inserted into this queue for many times,this
//operation only deletes one time.
//

static BOOL DeleteFromQueue(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpObject)
{
    __PRIORITY_QUEUE*         lpQueue = (__PRIORITY_QUEUE*)lpThis;
    __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    DWORD                     dwFlags;

	if((NULL == lpThis) || (NULL == lpObject))  //Invalid parameters.
    {
        return FALSE;
    }

    __ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpElement = lpQueue->ElementHeader.lpNextElement;
    while((lpElement->lpObject != lpObject) && (lpElement != &lpQueue->ElementHeader))
    {
        lpElement = lpElement->lpNextElement;
    }
    if(lpObject == lpElement->lpObject)  //Found,delete it.
    {
        lpQueue->dwCurrElementNum --;
        lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
        lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
        __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
        KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0);  //Free memory.
        return TRUE;
    }
    //Not found the target object to delete.
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);  //No need barrier.
    return FALSE;
}

//
//Get the header element from Priority Queue.
//This routine get the first(header) object of the priority queue,
//and release the memory this element occupies.
//

static __COMMON_OBJECT* GetHeaderElement(__COMMON_OBJECT* lpThis,DWORD* lpdwPriority)
{
    __PRIORITY_QUEUE*          lpQueue = (__PRIORITY_QUEUE*)lpThis;
    __PRIORITY_QUEUE_ELEMENT*  lpElement = NULL;
    __COMMON_OBJECT*           lpCommObject = NULL;
    DWORD                      dwFlags;

	if(NULL == lpThis)
    {
        return FALSE;
    }

    __ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpElement = lpQueue->ElementHeader.lpNextElement;

    if(lpElement == &lpQueue->ElementHeader)  //Queue empty.
    {
        __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
        return NULL;
    }

    //Queue not empty,delete this element.
    lpQueue->dwCurrElementNum -= 1;
    lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
    lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
    __LEAVE_CRITICAL_SECTION(NULL,dwFlags);

    lpCommObject = lpElement->lpObject;
    if(lpdwPriority)  //Should return the priority value.
    {
        *lpdwPriority = lpElement->dwPriority;
    }
    KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0);  //Release the element.

    return lpCommObject;
}

//Initialize routine of the Priority Queue.
BOOL PriQueueInitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE*                lpPriorityQueue     = NULL;

	if(NULL == lpThis)           //Parameter check.
	{
		return FALSE;
	}

	//Initialize the Priority Queue.
	lpPriorityQueue = (__PRIORITY_QUEUE*)lpThis;
	lpPriorityQueue->ElementHeader.lpObject = NULL;
	lpPriorityQueue->ElementHeader.dwPriority = 0;
	lpPriorityQueue->ElementHeader.lpNextElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->ElementHeader.lpPrevElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->dwCurrElementNum = 0;
	lpPriorityQueue->InsertIntoQueue  = InsertIntoQueue;
	lpPriorityQueue->DeleteFromQueue  = DeleteFromQueue;
	lpPriorityQueue->GetHeaderElement = GetHeaderElement;

	return TRUE;
}

//
//Uninitialize routine of Priority Queue.
//This routine frees all memory this priority queue occupies.
//

VOID PriQueueUninitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE_ELEMENT*    lpElement    = NULL;
	__PRIORITY_QUEUE_ELEMENT*    lpTmpElement   = NULL;
	__PRIORITY_QUEUE*            lpPriorityQueue   = NULL;

	if(NULL == lpThis)
	{
		return;
	}

	lpPriorityQueue = (__PRIORITY_QUEUE*)lpThis;
	lpElement = lpPriorityQueue->ElementHeader.lpNextElement;
	//Delete all queue element(s).
	while(lpElement != &lpPriorityQueue->ElementHeader)
	{
		lpTmpElement = lpElement;
		lpElement = lpElement->lpNextElement;
		KMemFree(lpTmpElement,KMEM_SIZE_TYPE_ANY,0);
	}
}
