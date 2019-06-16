//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,03 2004
//    Module Name               : objqueue.h
//    Module Funciton           : 
//                                This module countains Object Queue structure's definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __OBJQUEUE_H__
#define __OBJQUEUE_H__

/* For spin lock. */
#include <TYPES.H>

#ifdef __cplusplus
extern "C" {
#endif

//Priority Queue's element.
BEGIN_DEFINE_OBJECT(__PRIORITY_QUEUE_ELEMENT)
    __COMMON_OBJECT*          lpObject;
    DWORD                     dwPriority;
	struct tag__PRIORITY_QUEUE_ELEMENT* lpNextElement;
	struct tag__PRIORITY_QUEUE_ELEMENT* lpPrevElement;
END_DEFINE_OBJECT(__PRIORITY_QUEUE_ELEMENT)

//The definition of Priority Queue.

BEGIN_DEFINE_OBJECT(__PRIORITY_QUEUE)
    INHERIT_FROM_COMMON_OBJECT //Inherit from __COMMON_OBJECT.
    __PRIORITY_QUEUE_ELEMENT ElementHeader;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Validate the object. */
	DWORD dwObjectSignature;
	/* How many queue element(s) in the queue. */
    DWORD dwCurrElementNum;
	/* Routines to operate the queue. */
	BOOL (*InsertIntoQueue)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpObject,
		DWORD dwPriority);
	BOOL (*DeleteFromQueue)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpObject);
	__COMMON_OBJECT* (*GetHeaderElement)(__COMMON_OBJECT* lpThis,
		DWORD* lpPriority);
	BOOL (*ObjectInQueue)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpObject);
END_DEFINE_OBJECT(__PRIORITY_QUEUE)

//Initialize routine and Uninitialize's definition.
BOOL PriQueueInitialize(__COMMON_OBJECT* lpThis);
BOOL PriQueueUninitialize(__COMMON_OBJECT* lpThis);

#ifdef __cplusplus
}
#endif

#endif  //__OBJQUEUE_H__
