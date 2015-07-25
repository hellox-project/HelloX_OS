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
    INHERIT_FROM_COMMON_OBJECT                            //Inherit from __COMMON_OBJECT.
    __PRIORITY_QUEUE_ELEMENT     ElementHeader;
    DWORD                        dwCurrElementNum;
	BOOL                         (*InsertIntoQueue)(
		                         __COMMON_OBJECT* lpThis,
								 __COMMON_OBJECT* lpObject,
								 DWORD            dwPriority
								 );
	BOOL                         (*DeleteFromQueue)(
		                         __COMMON_OBJECT* lpThis,
								 __COMMON_OBJECT* lpObject
								 );
	__COMMON_OBJECT*             (*GetHeaderElement)(
		                         __COMMON_OBJECT* lpThis,
								 DWORD*           lpPriority
								 );
END_DEFINE_OBJECT(__PRIORITY_QUEUE)

//Initialize routine and Uninitialize's definition.

BOOL PriQueueInitialize(__COMMON_OBJECT* lpThis);
VOID PriQueueUninitialize(__COMMON_OBJECT* lpThis);

#ifdef __cplusplus
}
#endif

#endif  //__OBJQUEUE_H__
