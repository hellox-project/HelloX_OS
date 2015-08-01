//***********************************************************************/
//    Author                    : Erwin
//	  Email						: erwin.wang@qq.com
//    Original Date             : 29th May, 2014
//    Module Name               : debug.c
//    Module Funciton           : 
//                                Designed for debug subsystem. The log functions 
//								  now have been implemented.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "../include/StdAfx.h"
#endif

#ifndef __KAPI_H__
#include "../include/kapi.h"
#endif

#include "debug.h"
#include "stdio.h"
#include "string.h"

//Enabled only the macro __CFG_SYS_LOGCAT is defined.
#ifdef __CFG_SYS_LOGCAT

static void __Log(__DEBUG_MANAGER *pThis, char *tag, char *msg)
{
	__DEBUG_MANAGER			*pDebugManager		= pThis;
	__LOG_MESSAGE			*pMsg				= NULL;
	__KERNEL_THREAD_OBJECT	*lpCurrentThread	= NULL;
	int						dwFlags				= 0;
	int						Result				= -1;

	pMsg = (__LOG_MESSAGE *)KMemAlloc(sizeof(__LOG_MESSAGE),KMEM_SIZE_TYPE_ANY);

	//
	// Set to default now
	//
	pMsg->code = 0;
	pMsg->format = 0;
	pMsg->len = 0;
	pMsg->pid = 0;
	pMsg->time = 0;

	//
	//*****XXX*******
	// FIXME
	//

	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;
	StrCpy(lpCurrentThread->KernelThreadName,pMsg->name);
	pMsg->tid = lpCurrentThread->dwThreadID;
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

	StrCpy(msg, pMsg->msg);
	StrCpy(tag, pMsg->tag);

	//
	// Get the authority to visit the bufferqueue
	//
	Result = pDebugManager->pMutexForBufferQueue->WaitForThisObject((__COMMON_OBJECT*)DebugManager.pMutexForBufferQueue);
	if (Result == OBJECT_WAIT_RESOURCE)
	{
		pDebugManager->pBufferQueue->Enqueue(
			pDebugManager->pBufferQueue,
			pMsg);
		pDebugManager->pMutexForBufferQueue->ReleaseMutex((__COMMON_OBJECT*)DebugManager.pMutexForBufferQueue);
	}

	KMemFree(pMsg, KMEM_SIZE_TYPE_ANY, 0);

	return;
}

static void Logcat(__DEBUG_MANAGER *pThis, char *buf, int len)
{
	__DEBUG_MANAGER *pDebugManager	=	pThis;

	__LOG_MESSAGE	*pMsg			=	NULL;
	__LOG_MESSAGE	*p				=	NULL;
	int				Result			=	-1;

	pMsg = (__LOG_MESSAGE *)KMemAlloc(sizeof(__LOG_MESSAGE),KMEM_SIZE_TYPE_ANY);

	//
	// Get the authority to visit the kernel bufferqueue
	//
	Result = DebugManager.pMutexForKRNLBufferQueue->WaitForThisObject((__COMMON_OBJECT*)DebugManager.pMutexForKRNLBufferQueue);
	if (Result == OBJECT_WAIT_RESOURCE)
	{
		p = pDebugManager->pKRNLBufferQueue->Dequeue(pDebugManager->pKRNLBufferQueue);
		pDebugManager->pMutexForKRNLBufferQueue->ReleaseMutex((__COMMON_OBJECT*)DebugManager.pMutexForKRNLBufferQueue);
	}

	if(p == NULL)
	{
		//
		// Get the authority to visit the user bufferqueue
		//
		Result = DebugManager.pMutexForBufferQueue->WaitForThisObject((__COMMON_OBJECT*)DebugManager.pMutexForBufferQueue);
		if (Result == OBJECT_WAIT_RESOURCE)
		{
			p = pDebugManager->pBufferQueue->Dequeue(pDebugManager->pBufferQueue);
			pDebugManager->pMutexForBufferQueue->ReleaseMutex((__COMMON_OBJECT*)DebugManager.pMutexForBufferQueue);
		}
	}

	if(p != NULL)
	{
		pMsg->pid = p->pid;
		pMsg->tid = p->tid;
		pMsg->time = p->time;
		StrCpy(p->tag, pMsg->tag);
		StrCpy(p->name, pMsg->name);
		StrCpy(p->msg, pMsg->msg);
		_hx_sprintf(buf, "tag:%s name:%s time:%d pid:%d tid:%d msg:%s", 
			pMsg->tag, pMsg->name, pMsg->time, 
			pMsg->pid, pMsg->tid, pMsg->msg);	
	}

	KMemFree(pMsg, KMEM_SIZE_TYPE_ANY, 0);

	return;	
}

static void __Logk(__DEBUG_MANAGER *pThis, char *tag, char *msg)
{
	__DEBUG_MANAGER			*pDebugManager		= pThis;
	__LOG_MESSAGE			*pMsg				= NULL;
	__KERNEL_THREAD_OBJECT	*lpCurrentThread	= NULL;

	int						Result				= -1;
	int						dwFlags				= 0;

	pMsg = (__LOG_MESSAGE *)KMemAlloc(sizeof(__LOG_MESSAGE),KMEM_SIZE_TYPE_ANY);

	//
	//Set to default now
	//
	pMsg->code = 0;
	pMsg->format = 0;
	pMsg->len = 0;
	pMsg->pid = 0;
	pMsg->time = 0;

	//
	//*****XXX*******
	// FIXME
	//
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;
	StrCpy(lpCurrentThread->KernelThreadName,pMsg->name);
	pMsg->tid = lpCurrentThread->dwThreadID;
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

	StrCpy(msg, pMsg->msg);
	StrCpy(tag, pMsg->tag);

	//
	//Get the authority to visit the bufferqueue
	//

	Result = pDebugManager->pMutexForKRNLBufferQueue->WaitForThisObject((__COMMON_OBJECT*)DebugManager.pMutexForKRNLBufferQueue);
	if (Result == OBJECT_WAIT_RESOURCE)
	{
		pDebugManager->pKRNLBufferQueue->Enqueue(
			pDebugManager->pKRNLBufferQueue,
			pMsg);
		pDebugManager->pMutexForKRNLBufferQueue->ReleaseMutex((__COMMON_OBJECT*)DebugManager.pMutexForKRNLBufferQueue);
	}

	KMemFree(pMsg, KMEM_SIZE_TYPE_ANY, 0);

	return;
}


static int Enqueue(__BUFFER_QUEUE *pThis, __LOG_MESSAGE *pMsg)
{

	__BUFFER_QUEUE	*pBufferQueue	=	pThis;
	int				saved			=	pBufferQueue->tail;
	DWORD			dwFlags			=	0;

	if(pBufferQueue->len >= 256) return -1;

	pBufferQueue->tail = (pBufferQueue->tail + 1) % 256;
	StrCpy(pMsg->name,pBufferQueue->BufferQueue[pBufferQueue->tail].name);
	StrCpy(pMsg->tag, pBufferQueue->BufferQueue[pBufferQueue->tail].tag);
	StrCpy(pMsg->msg, pBufferQueue->BufferQueue[pBufferQueue->tail].msg);
	pBufferQueue->BufferQueue[pBufferQueue->tail].tid = pMsg->tid;
	pBufferQueue->len++;

	return saved;
}

static __LOG_MESSAGE *Dequeue(__BUFFER_QUEUE *pThis)
{
	__BUFFER_QUEUE	*pBufferQueue	= pThis;
	int				saved			= pBufferQueue->head;

	if(pBufferQueue->len <= 0) return NULL;

	pBufferQueue->head = (pBufferQueue->head + 1) % 256;
	pBufferQueue->len--;

	return &pBufferQueue->BufferQueue[saved];
}

static void Initialize(__DEBUG_MANAGER *pThis)
{
	__DEBUG_MANAGER	*pDebugManager				=		pThis;
	__BUFFER_QUEUE	*pBufferQueue				=		NULL;
	__BUFFER_QUEUE	*pKRNLBufferQueue			=		NULL;
	__MUTEX			*pMutexForBufferQueue		=		NULL;
	__MUTEX			*pMutexForKRNLBufferQueue	=		NULL;

	int	i = 0;

	//
	//****************KERNEL BUFFERQUEUE*************
	// Intialize the sync object
	//
	pMutexForKRNLBufferQueue = (__MUTEX*)CreateMutex();
	if (!pMutexForKRNLBufferQueue)return;

	pDebugManager->pMutexForKRNLBufferQueue = pMutexForKRNLBufferQueue;

	//
	// Intialize the kernel BufferQueue.
	//
	pKRNLBufferQueue = (__BUFFER_QUEUE *)KMemAlloc(sizeof(__BUFFER_QUEUE),KMEM_SIZE_TYPE_ANY);
	if(!pKRNLBufferQueue)return;
	for(; i < 256; i++)
	{
		pKRNLBufferQueue->BufferQueue[i].code = 0;
		pKRNLBufferQueue->BufferQueue[i].format = 0;
		pKRNLBufferQueue->BufferQueue[i].len = 0;
		pKRNLBufferQueue->BufferQueue[i].time = 0;
		pKRNLBufferQueue->BufferQueue[i].pid = 0;// get pid
		pKRNLBufferQueue->BufferQueue[i].tid = 0;// get tid
	}

	//
	// FIXME
	//
	//pBufferQueue->head = pBufferQueue ->tail = -1;
	pKRNLBufferQueue->head = pBufferQueue ->tail = 0;
	pKRNLBufferQueue->len = 0;

	pKRNLBufferQueue->Enqueue = Enqueue;
	pKRNLBufferQueue->Dequeue = Dequeue;

	pDebugManager->pKRNLBufferQueue = pKRNLBufferQueue;

	//
	//**************USER BUFFERQUEUE**************
	// Intialize the sync object
	//
	pMutexForBufferQueue = (__MUTEX*)CreateMutex();
	if (!pMutexForBufferQueue)return;

	pDebugManager->pMutexForBufferQueue = pMutexForBufferQueue;

	//
	// Intialize the user BufferQueue.
	//
	pBufferQueue = (__BUFFER_QUEUE *)KMemAlloc(sizeof(__BUFFER_QUEUE),KMEM_SIZE_TYPE_ANY);
	if(!pBufferQueue)return;
	for(; i < 256; i++)
	{
		pBufferQueue->BufferQueue[i].code = 0;
		pBufferQueue->BufferQueue[i].format = 0;
		pBufferQueue->BufferQueue[i].len = 0;
		pBufferQueue->BufferQueue[i].time = 0;
		pBufferQueue->BufferQueue[i].pid = 0;// get pid
		pBufferQueue->BufferQueue[i].tid = 0;// get tid
	}
	//
	// FIXME
	//
	//pBufferQueue->head = pBufferQueue ->tail = -1;
	pBufferQueue->head = pBufferQueue ->tail = 0;
	pBufferQueue->len = 0;

	pBufferQueue->Enqueue = Enqueue;
	pBufferQueue->Dequeue = Dequeue;

	pDebugManager->pBufferQueue = pBufferQueue;

	return;
}

static void Unintialize(__DEBUG_MANAGER *pThis)
{

	__DEBUG_MANAGER *DebugManager = pThis;

	// Release the user BufferQueue
	if(!DebugManager->pBufferQueue)
	{
		KMemFree(DebugManager->pBufferQueue, KMEM_SIZE_TYPE_ANY, sizeof(__BUFFER_QUEUE));
	}

	// Release the kernel BufferQueue
	if(!DebugManager->pKRNLBufferQueue)
	{
		KMemFree(DebugManager->pKRNLBufferQueue, KMEM_SIZE_TYPE_ANY, sizeof(__BUFFER_QUEUE));
	}

	//
	// Uninitialize the sync object..
	//
	if (!DebugManager->pMutexForBufferQueue)
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)DebugManager->pMutexForBufferQueue);
	}

	if (!DebugManager->pMutexForKRNLBufferQueue)
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)DebugManager->pMutexForKRNLBufferQueue);
	}

	return;
} 

__DEBUG_MANAGER DebugManager =
{
	// For user 
	NULL,
	NULL,       //MODIFIED BY GARRY: Please keep same structure in header file's definition,only warning in C will be generated if
				//align is not enforced and can lead fatal run time error.
	// For kernel
	NULL,
	NULL,

	__Log,		//Log
	__Logk,		//Logk
	Logcat,		//Logcat
	Initialize,
	Unintialize,
};

#endif  //__CFG_SYS_LOGCAT.
