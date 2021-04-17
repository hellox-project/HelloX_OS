//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.CPP
//    Module Funciton           : 
//                                All routines in kernel module are wrapped
//                                in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "kapi.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "usbasync.h"

HANDLE CreateKernelThread(DWORD dwStackSize,
						  DWORD dwInitStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved,
						  LPSTR lpszName)
{
	return (HANDLE)KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		dwStackSize,
		dwInitStatus,
		dwPriority,
		lpStartRoutine,
		lpRoutineParam,
		lpReserved,
		lpszName);
}

VOID DestroyKernelThread(HANDLE hThread)
{
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		hThread);
}

DWORD SetLastError(DWORD dwNewError)
{
	return KernelThreadManager.SetLastError(
		dwNewError);
}

DWORD GetLastError()
{
	return KernelThreadManager.GetLastError();
}

DWORD GetThreadID(HANDLE hThread)
{
	return KernelThreadManager.GetThreadID(
		hThread);
}

DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority)
{
	return KernelThreadManager.SetThreadPriority(
		hThread,
		dwPriority);
}

/* 
 * Get message from current kernel thread's message queue. 
 * Current kernel thread will be blocked if no message present.
 */
BOOL GetMessage(MSG* lpMsg)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = __CURRENT_KERNEL_THREAD;
	if (NULL == lpMsg)
	{
		return FALSE;
	}
	return KernelThreadManager.GetMessage((__COMMON_OBJECT*)lpKernelThread,lpMsg);
}

/* 
 * Get message from current kernel thread's message queue. 
 * Return immediately even no message in queue,FALSE will be
 * returned in this case.
 */
BOOL PeekMessage(MSG* pMsg)
{
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	if (NULL == pMsg)
	{
		return FALSE;
	}
	return KernelThreadManager.PeekMessage((__COMMON_OBJECT*)pKernelThread, pMsg);
}

BOOL SendMessage(HANDLE hThread,MSG* lpMsg)
{
	return KernelThreadManager.SendMessage(
		hThread,lpMsg);
}

BOOL Sleep(DWORD dwMillionSecond)
{
	return KernelThreadManager.Sleep(
		(__COMMON_OBJECT*)&KernelThreadManager,
		dwMillionSecond);
}

//EnableSuspend on a specified kernel thread.
BOOL EnableSuspend(HANDLE hThread,BOOL bEnable)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)hThread;

	if(NULL == lpKernelThread)
	{
		//Operate on the current kernel thread.
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	return KernelThreadManager.EnableSuspend((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpKernelThread,
		bEnable);
}

//Suspend a specified kernel thread.
BOOL SuspendKernelThread(HANDLE hThread)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)hThread;
	
	if(NULL == lpKernelThread)
	{
		//Suspend the current kernel thread.
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	return KernelThreadManager.SuspendKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpKernelThread);
}

//Resume a specified kernel thread.
BOOL ResumeKernelThread(HANDLE hThread)
{
	__KERNEL_THREAD_OBJECT* lpKernelThread = (__KERNEL_THREAD_OBJECT*)hThread;

	if(NULL == lpKernelThread)
	{
		//Operate on the current kernel thread.
		lpKernelThread = __CURRENT_KERNEL_THREAD;
	}
	return KernelThreadManager.ResumeKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpKernelThread);
}

/* Get a processor to schedule a new kernel thread to. */
unsigned int GetScheduleCPU()
{
	return ProcessorManager.GetScheduleCPU();
}

/* Change a specified kernel thread's CPU affinity. */
unsigned int ChangeAffinity(HANDLE hThread, unsigned int nAffinity)
{
#if defined(__CFG_SYS_SMP)
	return KernelThreadManager.ChangeAffinity((__COMMON_OBJECT*)hThread, nAffinity);
#else
	return 0;
#endif
}

HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags)
{
	return System.SetTimer(
		(__COMMON_OBJECT*)&System,
		__CURRENT_KERNEL_THREAD,
		dwTimerID,
		dwMillionSecond,
		lpHandler,
		lpHandlerParam,
		dwTimerFlags);
}

BOOL CancelTimer(HANDLE hTimer)
{
	return System.CancelTimer(
		(__COMMON_OBJECT*)&System,
		hTimer);
}

HANDLE CreateEvent(BOOL bInitialStatus)
{
	__COMMON_OBJECT*         lpCommonObject    = NULL;
	__EVENT*                 lpEvent           = NULL;

	lpCommonObject = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);
	if(NULL == lpCommonObject)
		goto __TERMINAL;

	if(!lpCommonObject->Initialize(lpCommonObject))
	{
		ObjectManager.DestroyObject(&ObjectManager,lpCommonObject);
		lpCommonObject = NULL;
		goto __TERMINAL;
	}

	lpEvent = (__EVENT*)lpCommonObject;
	if(bInitialStatus)
	{
		lpEvent->SetEvent((__COMMON_OBJECT*)lpEvent);
	}

__TERMINAL:
	return lpCommonObject;
}

VOID DestroyEvent(HANDLE hEvent)
{
	ObjectManager.DestroyObject(&ObjectManager,hEvent);
}

DWORD SetEvent(HANDLE hEvent)
{
	return ((__EVENT*)hEvent)->SetEvent(hEvent);
}

DWORD PulseEvent(HANDLE hEvent)
{
	return ((__EVENT*)hEvent)->PulseEvent(hEvent);
}

DWORD ResetEvent(HANDLE hEvent)
{
	return ((__EVENT*)hEvent)->ResetEvent(hEvent);
}

HANDLE CreateMutex()
{
	__MUTEX* lpMutex = NULL;

	lpMutex = (__MUTEX*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_MUTEX);
	if(NULL == lpMutex)
	{
		return NULL;
	}
	if(!lpMutex->Initialize((__COMMON_OBJECT*)lpMutex))  //Can not initialize.
	{
		ObjectManager.DestroyObject(
			&ObjectManager,
			(__COMMON_OBJECT*)lpMutex);
		return NULL;
	}

	return (__COMMON_OBJECT*)lpMutex;  //Create successfully.
}

VOID DestroyMutex(HANDLE hMutex)
{
	ObjectManager.DestroyObject(&ObjectManager,
		hMutex);
	return;
}

DWORD ReleaseMutex(HANDLE hEvent)
{
	return ((__MUTEX*)hEvent)->ReleaseMutex(hEvent);
}

/* Create a new mailbox object. */
HANDLE CreateMailbox(int size)
{
	__COMMON_OBJECT*   pMailbox = NULL;

	/* Mailbox's size must large than 0. */
	if (size <= 0)
	{
		goto __TERMINAL;
	}

	pMailbox = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_MAILBOX);
	if (NULL == pMailbox)
	{
		goto __TERMINAL;
	}
	if (!pMailbox->Initialize(pMailbox))
	{
		ObjectManager.DestroyObject(&ObjectManager, pMailbox);
		pMailbox = NULL;
		goto __TERMINAL;
	}

	//Set mailbox size accordingly.
	if (!((__MAIL_BOX*)pMailbox)->SetMailboxSize(pMailbox, size))
	{
		ObjectManager.DestroyObject(&ObjectManager, pMailbox);
		pMailbox = NULL;
		return 0;
	}

__TERMINAL:
	return (HANDLE)pMailbox;
}

/* Destroy a mailbox object. */
VOID DestroyMailbox(HANDLE hMailbox)
{
	__COMMON_OBJECT* pMailbox = (__COMMON_OBJECT*)hMailbox;
	
	/* Do some basic checking. */
	if (NULL == pMailbox)
	{
		return;
	}
	if (pMailbox->dwObjectType != OBJECT_TYPE_MAILBOX)
	{
		return;
	}
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)hMailbox);
}

/* Send a mail to mailbox. */
DWORD SendMail(HANDLE hMailbox, LPVOID pMsg, DWORD dwPriority, DWORD dwWaitTime, DWORD* pdwWaited)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)hMailbox;
	
	/* Basic checking. */
	if ((NULL == pMailbox) || (NULL == pMsg))
	{
		return OBJECT_WAIT_FAILED;
	}
	if (OBJECT_TYPE_MAILBOX != pMailbox->dwObjectType)
	{
		return OBJECT_WAIT_FAILED;
	}
	return pMailbox->SendMail((__COMMON_OBJECT*)pMailbox, pMsg, dwPriority, dwWaitTime, pdwWaited);
}

/* Get mail from mailbox. */
DWORD GetMail(HANDLE hMailbox, LPVOID* ppMsg, DWORD dwWaitTime, DWORD* pdwWaited)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)hMailbox;

	/* Basic checking. */
	if ((NULL == pMailbox) || (NULL == ppMsg))
	{
		return OBJECT_WAIT_FAILED;
	}
	if (OBJECT_TYPE_MAILBOX != pMailbox->dwObjectType)
	{
		return OBJECT_WAIT_FAILED;
	}
	return pMailbox->GetMail((__COMMON_OBJECT*)pMailbox, ppMsg, dwWaitTime, pdwWaited);
}

/* Create semaphore object. */
HANDLE CreateSemaphore(int max_count, int curr_count)
{
	__SEMAPHORE*  pSem = NULL;
	
	/* Checking parameters. */
	if ((max_count <= 0) || (curr_count < 0))
	{
		goto __TERMINAL;
	}
	if (curr_count > max_count)
	{
		goto __TERMINAL;
	}
	/* Create and initializes the semaphore object. */
	pSem = (__SEMAPHORE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_SEMAPHORE);
	if (NULL == pSem)
	{
		goto __TERMINAL;
	}
	if (!pSem->Initialize((__COMMON_OBJECT*)pSem))
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pSem);
		pSem = NULL;
		goto __TERMINAL;
	}
	/* Change initial counters. */
	if (!pSem->SetSemaphoreCount((__COMMON_OBJECT*)pSem, max_count, curr_count))
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pSem);
		pSem = NULL;
		goto __TERMINAL;
	}
__TERMINAL:
	return (HANDLE)pSem;
}

/*
* Release a semaphore object,i.e,increase it's counter and
* activate one kernel thread who is waiting for the semaphore
* object if there is(are).
*/
int ReleaseSemaphore(HANDLE hSem, BOOL bNoResched)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)hSem;
	int count = -1;

	/* Validate the object. */
	if (NULL == pSem)
	{
		goto __TERMINAL;
	}
	if (OBJECT_TYPE_SEMAPHORE != pSem->dwObjectType)
	{
		goto __TERMINAL;
	}
	/* Just delegates to object's member function. */
	if (pSem->ReleaseSemaphore((__COMMON_OBJECT*)pSem, &count, bNoResched))
	{
		goto __TERMINAL;
	}
	/* Release operation failed,set return value to -1. */
	count = -1;

__TERMINAL:
	return count;
}

/* Release the semaphore object. */
VOID DestroySemaphore(HANDLE hSem)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)hSem;

	/* Validate the semaphore object. */
	if (NULL == pSem)
	{
		return;
	}
	if (pSem->dwObjectType != OBJECT_TYPE_SEMAPHORE)
	{
		return;
	}
	/* Destroy it. */
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pSem);
}

DWORD WaitForThisObject(HANDLE hObject)
{
	__COMMON_OBJECT* lpCommonObject = (__COMMON_OBJECT*)hObject;
	__KERNEL_THREAD_OBJECT* lpThread = NULL;
	__EVENT* lpEvent = NULL;
	__MUTEX* lpMutex = NULL;
	__SEMAPHORE* pSem = NULL;

	if(NULL == lpCommonObject)
	{
		return OBJECT_WAIT_FAILED;
	}

	switch(lpCommonObject->dwObjectType)
	{
	case OBJECT_TYPE_KERNEL_THREAD:
		lpThread = (__KERNEL_THREAD_OBJECT*)lpCommonObject;
		return lpThread->WaitForThisObject((__COMMON_OBJECT*)lpThread);
	case OBJECT_TYPE_EVENT:
		lpEvent = (__EVENT*)lpCommonObject;
		return lpEvent->WaitForThisObject((__COMMON_OBJECT*)lpEvent);
	case OBJECT_TYPE_MUTEX:
		lpMutex = (__MUTEX*)lpCommonObject;
		return lpMutex->WaitForThisObject((__COMMON_OBJECT*)lpMutex);
	case OBJECT_TYPE_SEMAPHORE:
		pSem = (__SEMAPHORE*)lpCommonObject;
		return pSem->WaitForThisObject((__COMMON_OBJECT*)pSem);
	default:
		break;
	}
	return OBJECT_WAIT_FAILED;
}

DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond, DWORD* pdwWaited)
{
	__COMMON_OBJECT* lpCommonObject = (__COMMON_OBJECT*)hObject;
	__EVENT* lpEvent = NULL;
	__MUTEX* lpMutex = NULL;
	__SEMAPHORE* pSem = NULL;

	if(NULL == lpCommonObject)
	{
		return OBJECT_WAIT_FAILED;
	}

	switch(lpCommonObject->dwObjectType)
	{
	case OBJECT_TYPE_KERNEL_THREAD:
		/* Don't support timeout waiting yet. */
		return OBJECT_WAIT_FAILED;
	case OBJECT_TYPE_EVENT:
		/* Can not return the waited time yet. */
		if (pdwWaited)
		{
			return OBJECT_WAIT_FAILED;
		}
		lpEvent = (__EVENT*)lpCommonObject;
		return lpEvent->WaitForThisObjectEx((__COMMON_OBJECT*)lpEvent,dwMillionSecond);
	case OBJECT_TYPE_MUTEX:
		/* Can not return the waited time yet. */
		if (pdwWaited)
		{
			return OBJECT_WAIT_FAILED;
		}
		lpMutex = (__MUTEX*)lpCommonObject;
		return lpMutex->WaitForThisObjectEx((__COMMON_OBJECT*)lpMutex,dwMillionSecond);
	case OBJECT_TYPE_SEMAPHORE:
		pSem = (__SEMAPHORE*)lpCommonObject;
		return pSem->WaitForThisObjectEx((__COMMON_OBJECT*)pSem, dwMillionSecond, pdwWaited);
	default:
		break;
	}
	return OBJECT_WAIT_FAILED;
}

//Create a CONDITION object.
HANDLE CreateCondition(DWORD condAttr)
{
	__CONDITION*       lpCond = NULL;

	lpCond = (__CONDITION*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_CONDITION);
	if(NULL == lpCond)
	{
		return NULL;
	}
	if(!lpCond->Initialize((__COMMON_OBJECT*)lpCond))  //Can not initialize.
	{
		ObjectManager.DestroyObject(
			&ObjectManager,
			(__COMMON_OBJECT*)lpCond);
		return NULL;
	}
	return (HANDLE)lpCond;
}

//Wait a CONDITION object infinitely.
DWORD WaitCondition(HANDLE hCond,HANDLE hMutex)
{
	__CONDITION* pCond  = (__CONDITION*)hCond;

	if((NULL == pCond) || (NULL == hMutex))
	{
		return OBJECT_WAIT_FAILED;
	}
	//Call CONDITION object's waiting routine directly.
	return pCond->CondWait((__COMMON_OBJECT*)pCond,(__COMMON_OBJECT*)hMutex);
}

//Wait a CONDITION object,time out manner.
DWORD TimeoutWaitCondition(HANDLE hCond,HANDLE hMutex,DWORD dwMillionSecond)
{
	__CONDITION* pCond = (__CONDITION*)hCond;

	if((NULL == hCond) || (NULL == hMutex))
	{
		return OBJECT_WAIT_FAILED;
	}
	return pCond->CondWaitTimeout((__COMMON_OBJECT*)pCond,(__COMMON_OBJECT*)hMutex,dwMillionSecond);
}

//Signaling a CONDITION object.
DWORD SignalCondition(HANDLE hCond)
{
	__CONDITION* pCond = (__CONDITION*)hCond;

	if(NULL == pCond)
	{
		return 0;
	}
	return pCond->CondSignal((__COMMON_OBJECT*)pCond);
}

//Broadcast a CONDITION object.
DWORD BroadcastCondition(HANDLE hCond)
{
	__CONDITION* pCond = (__CONDITION*)hCond;

	if (NULL == pCond)
	{
		return 0;
	}
	return pCond->CondBroadcast((__COMMON_OBJECT*)pCond);
}

//Destroy a CONDITION object.
VOID DestroyCondition(HANDLE hCond)
{
	if(NULL == hCond)
	{
		return;
	}
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)hCond);
}

HANDLE ConnectInterrupt(
	const char* int_name,
	__INTERRUPT_HANDLER lpInterruptHandler,
	LPVOID lpHandlerParam,
	UCHAR ucVector)
{
	return System.ConnectInterrupt(
		(__COMMON_OBJECT*)&System,
		int_name,
		lpInterruptHandler,
		lpHandlerParam,
		ucVector,
		0,
		0,
		0,
		TRUE,
		0);
}

VOID DisconnectInterrupt(HANDLE hInterrupt)
{
	System.DisconnectInterrupt(
		(__COMMON_OBJECT*)&System,
		hInterrupt);
}

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
					DWORD  dwSize,
					DWORD  dwAllocateFlags,
					DWORD  dwAccessFlags,
					UCHAR* lpszRegName)
{
#ifdef __CFG_SYS_VMM
	return lpVirtualMemoryMgr->VirtualAlloc(
		(__COMMON_OBJECT*)lpVirtualMemoryMgr,
		lpDesiredAddr,
		dwSize,
		dwAllocateFlags,
		dwAccessFlags,
		lpszRegName,
		NULL);
#else
	return NULL;
#endif
}

VOID VirtualFree(LPVOID lpVirtualAddr)
{
#ifdef __CFG_SYS_VMM
	lpVirtualMemoryMgr->VirtualFree(
		(__COMMON_OBJECT*)lpVirtualMemoryMgr,
		lpVirtualAddr);
#else
#endif
}

HANDLE CreateFile(LPSTR lpszFileName,
				  DWORD dwAccessMode,
				  DWORD dwShareMode,
				  LPVOID lpReserved)
{
#ifdef __CFG_SYS_DDF
	return (HANDLE)IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		lpszFileName,
		dwAccessMode,
		dwShareMode,
		lpReserved);
#else
	return NULL;
#endif
}

BOOL ReadFile(HANDLE hFile,
			  DWORD dwReadSize,
			  LPVOID lpBuffer,
			  DWORD* lpdwReadSize)
{
#ifdef __CFG_SYS_DDF
	return IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile,
		dwReadSize,
		lpBuffer,
		lpdwReadSize);
#else
	return FALSE;
#endif
}

BOOL WriteFile(HANDLE hFile,
			   DWORD dwWriteSize,
			   LPVOID lpBuffer,
			   DWORD* lpdwWrittenSize)
{
#ifdef __CFG_SYS_DDF
	return IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile,
		dwWriteSize,
		lpBuffer,
		lpdwWrittenSize);
#else
	return FALSE;
#endif
}

VOID CloseFile(HANDLE hFile)
{
#ifdef __CFG_SYS_DDF
	IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile);
#else
	return;
#endif
}

BOOL CreateDirectory(LPSTR lpszDirName)
{
#ifdef __CFG_SYS_DDF
	return IOManager.CreateDirectory((__COMMON_OBJECT*)&IOManager,
		lpszDirName,
		NULL);
#else
	return FALSE;
#endif
}

BOOL DeleteFile(LPSTR lpszFileName)
{
#ifdef __CFG_SYS_DDF
	return IOManager.DeleteFile((__COMMON_OBJECT*)&IOManager,
		lpszFileName);
#else
	return FALSE;
#endif
}

HANDLE FindFirstFile(LPSTR lpszDirName,
					 FS_FIND_DATA* pFindData)
{
#ifdef __CFG_SYS_DDF
	return IOManager.FindFirstFile((__COMMON_OBJECT*)&IOManager,
		lpszDirName,
		pFindData);
#else
	return NULL;
#endif
}

BOOL FindNextFile(LPSTR lpszDirName,
				  HANDLE hFindHandle,
				  FS_FIND_DATA* pFindData)
{
#ifdef __CFG_SYS_DDF
	return IOManager.FindNextFile((__COMMON_OBJECT*)&IOManager,
		lpszDirName,
		(__COMMON_OBJECT*)hFindHandle,
		pFindData);
#else
	return FALSE;
#endif
}

VOID FindClose(LPSTR lpszDirName,
			   HANDLE hFindHandle)
{
#ifdef __CFG_SYS_DDF
	IOManager.FindClose((__COMMON_OBJECT*)&IOManager,
		lpszDirName,
		(__COMMON_OBJECT*)hFindHandle);
#else
	return;
#endif
}

DWORD GetFileAttributes(LPSTR lpszFileName)
{
#ifdef __CFG_SYS_DDF
	return IOManager.GetFileAttributes((__COMMON_OBJECT*)&IOManager,
		lpszFileName);
#else
	return 0;
#endif
}

DWORD GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh)
{
#ifdef __CFG_SYS_DDF
	return IOManager.GetFileSize((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile,
		lpdwSizeHigh);
#else
	return 0;
#endif
}

BOOL RemoveDirectory(LPSTR lpszDirName)
{
#ifdef __CFG_SYS_DDF
	return IOManager.RemoveDirectory((__COMMON_OBJECT*)&IOManager,
		lpszDirName);
#else
	return 0;
#endif
}

BOOL SetEndOfFile(HANDLE hFile)
{
#ifdef __CFG_SYS_DDF
	return IOManager.SetEndOfFile((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile);
#else
	return FALSE;
#endif
}

BOOL IOControl(HANDLE hFile,
			   DWORD dwCommand,
			   DWORD dwInputLen,
			   LPVOID lpInputBuffer,
			   DWORD dwOutputLen,
			   LPVOID lpOutputBuffer,
			   DWORD* lpdwFilled)
{
#ifdef __CFG_SYS_DDF
	return IOManager.IOControl((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile,
		dwCommand,
		dwInputLen,
		lpInputBuffer,
		dwOutputLen,
		lpOutputBuffer,
		lpdwFilled);
#else
	return FALSE;
#endif
}

DWORD SetFilePointer(HANDLE hFile,
					DWORD* lpdwDistLow,
					DWORD* lpdwDistHigh,
					DWORD dwMoveFlags)
{
#ifdef __CFG_SYS_DDF
	return IOManager.SetFilePointer((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile,
		lpdwDistLow,
		lpdwDistHigh,
		dwMoveFlags);
#else
	return FALSE;
#endif
}

BOOL FlushFileBuffers(HANDLE hFile)
{
#ifdef __CFG_SYS_DDF
	return IOManager.FlushFileBuffers((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)hFile);
#else
	return FALSE;
#endif
}

HANDLE CreateDevice(LPSTR lpszDevName,
					DWORD dwAttributes,
					DWORD dwBlockSize,
					DWORD dwMaxReadSize,
					DWORD dwMaxWriteSize,
					LPVOID lpDevExtension,
					__DRIVER_OBJECT* lpDrvObject)
{
#ifdef __CFG_SYS_DDF
	return (HANDLE)IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		lpszDevName,
		dwAttributes,
		dwBlockSize,
		dwMaxReadSize,
		dwMaxWriteSize,
		lpDevExtension,
		lpDrvObject);
#else
	return NULL;
#endif
}

VOID DestroyDevice(HANDLE hDevice)
{
#ifdef __CFG_SYS_DDF
	IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
		(__DEVICE_OBJECT*)hDevice);
#else
	return;
#endif
}

/* Return system tick counter since boot. */
unsigned long GetClockTickCounter()
{
	return System.GetClockTickCounter((__COMMON_OBJECT*)&System);
}

/*
* Get a device from system,the device is identified
* by ID parameter.
* The first matched device will be returned if pPrev is set
* to NULL.Other matched devices following pPrev will be returned
* if pPrev is set.
*/
__PHYSICAL_DEVICE* GetDevice(__IDENTIFIER* id, __PHYSICAL_DEVICE* pPrev)
{
	__PHYSICAL_DEVICE* pDevice = NULL;

	if (NULL == id)
	{
		goto __TERMINAL;
	}
	switch (id->dwBusType)
	{
	case BUS_TYPE_PCI:
		pDevice = DeviceManager.GetDevice(&DeviceManager,
			BUS_TYPE_PCI,
			id,
			pPrev);
		break;
	case BUS_TYPE_USB:
		pDevice = USBManager.GetUsbDevice(id, pPrev);
		break;
	default:
		break;
	}

__TERMINAL:
	return pDevice;
}

//------------------------------------------------------------------------
HANDLE CreateRingBuff(DWORD dwBuffLength)
{
	__RING_BUFFER* lprb = (__RING_BUFFER*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_RING_BUFFER);
	if (NULL == lprb)  //Can not create ring buffer.
	{
		return NULL;
	}
	if (!lprb->Initialize((__COMMON_OBJECT*)lprb)) //Failed to initialize.
	{
		return NULL;
	}
	if (0 != dwBuffLength)
	{
		lprb->SetBuffLength((__COMMON_OBJECT*)lprb, dwBuffLength);
	}
	return (HANDLE)lprb;
}

BOOL GetRingBuffElement(HANDLE hRb, DWORD* lpdwElement, DWORD dwMillionSecond)
{
	__RING_BUFFER* lprb = (__RING_BUFFER*)hRb;
	if (NULL == lprb) //Invalid parameter.
	{
		return FALSE;
	}
	return lprb->GetElement((__COMMON_OBJECT*)lprb,
		lpdwElement,
		dwMillionSecond);
}

BOOL AddRingBuffElement(HANDLE hRb, DWORD dwElement)
{
	__RING_BUFFER* lprb = (__RING_BUFFER*)hRb;
	if (NULL == lprb)
	{
		return FALSE;
	}
	return lprb->AddElement((__COMMON_OBJECT*)lprb,
		dwElement);
}

BOOL SetRingBuffLength(HANDLE hRb, DWORD dwNewLength)
{
	__RING_BUFFER* lprb = (__RING_BUFFER*)hRb;
	if (NULL == lprb)
	{
		return FALSE;
	}
	return lprb->SetBuffLength((__COMMON_OBJECT*)lprb,
		dwNewLength);
}

VOID DestroyRingBuff(HANDLE hRb)
{
	ObjectManager.DestroyObject(
		&ObjectManager,
		hRb);
}
