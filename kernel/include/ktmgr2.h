//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 20, 2014
//    Module Name               : ktmgr2.h
//    Module Funciton           : 
//                                This is the second part of ktmgr.h file,since it's master part's
//                                size is too large.
//                                Semaphore and Mail Box objects are defined in this file.
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Jun 20,2014
//    Last modified Content     : 
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __KTMGR2_H__
#define __KTMGR2_H__

#include "types.h"
#include "ktmgr.h"
#include "commobj.h"

/* 
 * A helper structure used to transfer parameters 
 * to time out waiting handler, which is a basic 
 * routine to implements time out waiting for synchronization
 * kernel objects.
 */
typedef struct{
	/* Synchronous object. */
	__COMMON_OBJECT* lpSynObject;
	/* Waiting queue of the synchronous object. */
	__PRIORITY_QUEUE* lpWaitingQueue;
	/* Kernel thread who want to wait. */
	__KERNEL_THREAD_OBJECT* lpKernelThread;
	/* Synchronization object specified call back routine when time out. */
	VOID (*TimeOutCallback)(VOID*);
}__TIMER_HANDLER_PARAM;

//Definition of Semaphore objects.
BEGIN_DEFINE_OBJECT(__SEMAPHORE)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	/* Maximal semaphore counter,default is 1. */
	volatile DWORD dwMaxSem;
	/* Current value of semaphore,default same as dwMaxSem. */
    volatile DWORD dwCurrSem;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Kernel thread(s) waiting for the semaphore. */
	__PRIORITY_QUEUE* lpWaitingQueue;
	/*
	 * Change default sem counters,this routine should 
	 * be called before any WaitForThisObject or WaitForThisObjectEx
	 * operations since it can not wake up already blocked 
	 * kernel thread.
	 */
	BOOL (*SetSemaphoreCount)(__COMMON_OBJECT*,DWORD,DWORD); 
	/* 
	 * Release occupied semaphore resource,previous counter
	 * before this routine called will be returned.
	 * Rescheduling will not be triggered if bNoResched set,
	 * this is mainly used in interrupt context of critical
	 * sections,since rescheduling is not allowed.
	 */
	BOOL (*ReleaseSemaphore)(__COMMON_OBJECT*, DWORD* pdwPrevCount, BOOL bNoResched);
	DWORD (*WaitForThisObjectEx)(__COMMON_OBJECT*,DWORD dwMillionSecond,DWORD* pdwWait);
END_DEFINE_OBJECT(__SEMAPHORE)

//Initializer and Uninitializer of semaphore object.
BOOL SemInitialize(__COMMON_OBJECT* pSemaphore);
BOOL SemUninitialize(__COMMON_OBJECT* pSemaphore);

//Definition of mailbox's message.
typedef struct tag_MB_MESSAGE{
	LPVOID               pMessage;    //Point to any object.
	DWORD                dwPriority;  //Message priority in message box.
}__MB_MESSAGE;

//Definition of mail box object.
BEGIN_DEFINE_OBJECT(__MAIL_BOX)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	__MB_MESSAGE*        pMessageArray;    //Array contains the message sent to this mail box.
    DWORD                dwMaxMessageNum;  //The mail box's volume.
	DWORD                dwCurrMessageNum; //Current message number in mailbox.
	DWORD                dwMessageHeader;  //Pointing to message array's header.
	DWORD                dwMessageTail;    //Pointing to message array's tail.
	__PRIORITY_QUEUE*    lpSendingQueue;   //Kernel thread(s) try to send mail.
	__PRIORITY_QUEUE*    lpGettingQueue;   //Kernel thread(s) try to get mail.
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	//Set mailbox's size,default is 1.This routine should be called before any SendMail or
	//GetMail operations since it can not wake up any already blocked kernel thread.
	BOOL           (*SetMailboxSize)(__COMMON_OBJECT*,DWORD);
	DWORD          (*SendMail)(__COMMON_OBJECT*,LPVOID pMessage,
		                      DWORD dwPriority,DWORD dwMillionSecond,DWORD* pdwWait);  //Send message to mailbox.
	DWORD          (*GetMail)(__COMMON_OBJECT*,LPVOID* ppMessage,
		                     DWORD dwMillionSecond,DWORD* pdwWait);  //Get a mail from mail box.
END_DEFINE_OBJECT(__MAIL_BOX)

//Initializer and Uninitializer of mail box object.
BOOL MailboxInitialize(__COMMON_OBJECT* pMailbox);
BOOL MailboxUninitialize(__COMMON_OBJECT* pMailbox);

/* 
 * Definition of __CONDITION object.This object is conforms POSIX standard pthread_cond_xxx
 * operations,to fit the requirement of JamVM's porting.
 */
BEGIN_DEFINE_OBJECT(__CONDITION)
	INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	//How many thread(s) pending on the condition object.
	volatile int nThreadNum;
    __PRIORITY_QUEUE* lpPendingQueue;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	//Wait a condition object until the condition satisfied.
	DWORD (*CondWait)(__COMMON_OBJECT* pCond,__COMMON_OBJECT* pMutex);
	//Wait a condition object until the condition satisfied or time out.
	DWORD (*CondWaitTimeout)(__COMMON_OBJECT* pCond,__COMMON_OBJECT* pMutex,DWORD dwMillisonSecond);
	//Signal a condition object.
	DWORD (*CondSignal)(__COMMON_OBJECT* pCond);
	//Broadcast a condition object.
	DWORD (*CondBroadcast)(__COMMON_OBJECT* pCond);
END_DEFINE_OBJECT(__CONDITION)

//Initializer and Uninitializer of condition object.
BOOL ConditionInitialize(__COMMON_OBJECT* pCondObj);
BOOL ConditionUninitialize(__COMMON_OBJECT* pCondObj);

#endif //__KTMGR2_H__
