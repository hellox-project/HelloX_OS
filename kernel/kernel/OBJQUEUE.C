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

#include <StdAfx.h>
#include <types.h>
#include <commobj.h>
#include <kmemmgr.h>
#include <objqueue.h>
#include <hellocn.h>

/*
 * Insert an element into Priority Queue.
 * This routine insert an common object into priority queue,it's position in the queue is
 * determined by the object's priority(dwPriority parameter).
 */
static BOOL InsertIntoQueue(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpObject,DWORD dwPriority)
{
    __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    __PRIORITY_QUEUE_ELEMENT* lpTmpElement = NULL;
    __PRIORITY_QUEUE*         lpQueue   = (__PRIORITY_QUEUE*)lpThis;
    DWORD                     dwFlags   = 0;

	/* Validate parameters. */
	if((NULL == lpThis) || (NULL == lpObject))
    {
        return FALSE;
    }
	/* Validate the queue object. */
	if (lpQueue->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}
    
    /* Allocate a queue element,and initialize it. */
    lpElement = (__PRIORITY_QUEUE_ELEMENT*)KMemAlloc(sizeof(__PRIORITY_QUEUE_ELEMENT),
		KMEM_SIZE_TYPE_ANY);
    if(NULL == lpElement)
    {
		/* May no enough memory. */
        return FALSE;
    }

    lpElement->lpObject = lpObject;
    lpElement->dwPriority = dwPriority;

    //Now,insert the element into queue list.
    __ENTER_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
	//Increment element number.
    lpQueue->dwCurrElementNum ++;
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
    __LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);

    return TRUE;
}

/*
 * Delete an element from Priority Queue.
 * This routine searchs the queue,to find the object to be deleted,
 * if find,delete the object from priority queue,returns TRUE,else,
 * returns FALSE.
 * If the object is inserted into this queue for many times,this
 * operation only deletes one time.
 */
static BOOL DeleteFromQueue(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpObject)
{
    __PRIORITY_QUEUE*         lpQueue = (__PRIORITY_QUEUE*)lpThis;
    __PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
    DWORD                     dwFlags;

	/* Check the parameters. */
	if((NULL == lpThis) || (NULL == lpObject))
    {
        return FALSE;
    }
	/* Validate the queue object. */
	if (KERNEL_OBJECT_SIGNATURE != lpQueue->dwObjectSignature)
	{
		return FALSE;
	}

    __ENTER_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
	lpElement = lpQueue->ElementHeader.lpNextElement;
    while((lpElement->lpObject != lpObject) && (lpElement != &lpQueue->ElementHeader))
    {
        lpElement = lpElement->lpNextElement;
    }
    if(lpObject == lpElement->lpObject)
    {
		/* Found the corresponding element,delete it. */
        lpQueue->dwCurrElementNum --;
        lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
        lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
        __LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
        KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0);
        return TRUE;
    }
    /* Don't found the target object to delete. */
    __LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
    return FALSE;
}

/* Check if a specified object is in the queue. */
static BOOL ObjectInQueue(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpObject)
{
	__PRIORITY_QUEUE*         lpQueue = (__PRIORITY_QUEUE*)lpThis;
	__PRIORITY_QUEUE_ELEMENT* lpElement = NULL;
	DWORD                     dwFlags;

	/* Check the parameters. */
	if ((NULL == lpThis) || (NULL == lpObject))
	{
		return FALSE;
	}
	/* Validate the queue object. */
	if (KERNEL_OBJECT_SIGNATURE != lpQueue->dwObjectSignature)
	{
		return FALSE;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpQueue->spin_lock, dwFlags);
	lpElement = lpQueue->ElementHeader.lpNextElement;
	while ((lpElement->lpObject != lpObject) && (lpElement != &lpQueue->ElementHeader))
	{
		lpElement = lpElement->lpNextElement;
	}
	if (lpObject == lpElement->lpObject)
	{
		/* Object is in the queue. */
		__LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock, dwFlags);
		return TRUE;
	}
	/* Can not find the target object in queue. */
	__LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock, dwFlags);
	return FALSE;
}

/*
 * Get the header element from Priority Queue.
 * This routine get the first(header) object of the priority queue,
 * and release the memory this element occupies.
 */
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
	/* Validate the object. */
	if (lpQueue->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}

    __ENTER_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
	lpElement = lpQueue->ElementHeader.lpNextElement;
    if(lpElement == &lpQueue->ElementHeader)
    {
		/* Queue is empty. */
        __LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);
        return NULL;
    }

    //Queue not empty,delete this element.
    lpQueue->dwCurrElementNum -= 1;
    lpElement->lpNextElement->lpPrevElement = lpElement->lpPrevElement;
    lpElement->lpPrevElement->lpNextElement = lpElement->lpNextElement;
    __LEAVE_CRITICAL_SECTION_SMP(lpQueue->spin_lock,dwFlags);

    lpCommObject = lpElement->lpObject;
    if(lpdwPriority)  //Should return the priority value.
    {
        *lpdwPriority = lpElement->dwPriority;
    }
    KMemFree(lpElement,KMEM_SIZE_TYPE_ANY,0);  //Release the element.

    return lpCommObject;
}

/* 
 * Initializer of the priority queue ,called by ObjectManager when
 * a new priority queue is first created.
 */
BOOL PriQueueInitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;

	BUG_ON(NULL == lpThis);

	//Initialize the Priority Queue.
	lpPriorityQueue = (__PRIORITY_QUEUE*)lpThis;
	lpPriorityQueue->ElementHeader.lpObject = NULL;
	lpPriorityQueue->ElementHeader.dwPriority = 0;
	lpPriorityQueue->ElementHeader.lpNextElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->ElementHeader.lpPrevElement = &lpPriorityQueue->ElementHeader;
	lpPriorityQueue->dwCurrElementNum = 0;
	lpPriorityQueue->InsertIntoQueue = InsertIntoQueue;
	lpPriorityQueue->DeleteFromQueue = DeleteFromQueue;
	lpPriorityQueue->GetHeaderElement = GetHeaderElement;
	lpPriorityQueue->ObjectInQueue = ObjectInQueue;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpPriorityQueue->spin_lock, "pri_queue");
#endif
	lpPriorityQueue->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;

	return TRUE;
}

/*
 * Uninitialize routine of Priority Queue.
 * This routine frees all memory this priority queue occupies.
 */
BOOL PriQueueUninitialize(__COMMON_OBJECT* lpThis)
{
	__PRIORITY_QUEUE_ELEMENT*    lpElement    = NULL;
	__PRIORITY_QUEUE_ELEMENT*    lpTmpElement   = NULL;
	__PRIORITY_QUEUE*            lpPriorityQueue   = NULL;

	BUG_ON(NULL == lpThis);

	lpPriorityQueue = (__PRIORITY_QUEUE*)lpThis;
	/* Validate the object. */
	if (lpPriorityQueue->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}
	lpElement = lpPriorityQueue->ElementHeader.lpNextElement;
	//Delete all queue element(s).
	while(lpElement != &lpPriorityQueue->ElementHeader)
	{
		lpTmpElement = lpElement;
		lpElement = lpElement->lpNextElement;
		KMemFree(lpTmpElement,KMEM_SIZE_TYPE_ANY,0);
	}
	/* Reset object signature. */
	lpPriorityQueue->dwObjectSignature = 0;
	return TRUE;
}
