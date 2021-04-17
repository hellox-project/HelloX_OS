//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : kapi.h
//    Module Funciton           : 
//                                Declares all kernel API functions that can be
//                                used by other modules in kernel or application
//                                running in kernel mode.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#define __KAPI_H__

#include "types.h"

#ifndef __CONFIG_H__
#include "../config/config.h"
#endif

#ifndef __HELLO_CHINA__
#include "hellocn.h"
#endif

#ifndef __COMMOBJ_H__
#include "commobj.h"
#endif

#ifndef __SYN_MECH_H__
#if defined(__I386__)
#include "../arch/x86/syn_mech.h"
#elif defined(__STM32__)
#include "../arch/stm32/syn_mech.h"
#endif
#endif

#ifndef __OBJQUEUE_H__
#include "objqueue.h"
#endif

#ifndef __KTMGR_H__
#include "ktmgr.h"
#endif

#ifndef __KTMGR2_H__
#include "ktmgr2.h"
#endif

#ifndef __PERF_H__
#include "perf.h"
#endif

#ifndef __RINGBUFF_H__
#include "ringbuff.h"
#endif

#ifndef __SYSTEM_H__
#include "system.h"
#endif

#ifndef __DIM_H__
#include "dim.h"
#endif

#ifndef __MEMMGR_H__
#include "memmgr.h"
#endif

#ifndef __PAGEIDX_H__
#include "pageidx.h"
#endif

#ifndef __VMM_H__
#include "vmm.h"
#endif

#ifndef __KMEMMGR__
#include "kmemmgr.h"
#endif

#ifndef __ARCH_H__
#if defined(__I386__)
#include "../arch/x86/arch.h"
#elif defined(__STM32__)
#include "../arch/stm32/arch.h"
#endif
#endif

#include "iomgr.h"
#include "DEVMGR.H"

#ifdef __cplusplus
extern "C" {
#endif

//Treat all system objects as HANDLE to simplify the programming.
typedef __COMMON_OBJECT* HANDLE;

//Redefine kernel message structure.
typedef __KERNEL_THREAD_MESSAGE MSG;

//Create a kernel thread by calling this routine.
HANDLE CreateKernelThread(DWORD dwStackSize,
	DWORD dwInitStatus,
	DWORD dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	LPVOID lpReserved,
	LPSTR lpszName);

//Destroy the kernel thread object created by CreateKernelThread.
VOID DestroyKernelThread(HANDLE hThread);

//Set error number of the system.
DWORD SetLastError(DWORD dwNewError);

//Get the error number set by SetLastError.
DWORD GetLastError(void);

//Return current kernel thread's ID.
DWORD GetThreadID(HANDLE hThread);

//Change one kernel thread's priority level.
DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority);

//Get one message from message queue.
BOOL GetMessage(MSG* lpMsg);

//Send a message to a specified thread.
BOOL SendMessage(HANDLE hThread,MSG* lpMsg);

/* Get message from queue without blocking if no message present. */
BOOL PeekMessage(MSG* lpMsg);

//Enable or disable suspend on a kernel thread.
//Enable the suspending operation on the specified thread
//if bEnable is TRUE,disable it if FALSE.
BOOL EnableSuspend(HANDLE hThread,BOOL bEnable);

//Suspend a kernel thread.
BOOL SuspendKernelThread(HANDLE hThread);

//Resume a kernel thread.
BOOL ResumeKernelThread(HANDLE hThread);

//Sleep a period specified by dwMillionSecond.
BOOL Sleep(DWORD dwMillionSecond);

/* Get a CPU(processor) to schedule a kernel thread to. */
unsigned int GetScheduleCPU();

/* Change a specified kernel thread's CPU affinity. */
unsigned int ChangeAffinity(HANDLE hThread, unsigned int nAffinity);

//Set a timer object.
HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags);

//Cancel the timer object set by SetTimer.
//Only ALWAYS timer object can be canceled.
BOOL CancelTimer(HANDLE hTimer);

//Create an event object to synchronize kernel thread's execution.
HANDLE CreateEvent(BOOL bInitialStatus);

//Destroy the event object.
VOID DestroyEvent(HANDLE hEvent);

//Set event object's status to signaling.This will wakeup all thread(s)
//waiting this event object.
DWORD SetEvent(HANDLE hEvent);

/* 
 * Palse an event object,this is the combination operations of 
 * SetEvent and ResetEvent,in atomic.
 * All kernel threads waiting on the event will be waken up,the
 * previous status is retuned.
 */
DWORD PulseEvent(HANDLE hEvent);

//Set event object's status to unsignal.
DWORD ResetEvent(HANDLE hEvent);

//Create a mutex object.
HANDLE CreateMutex(void);

//Destroy the mutex object created by CreateMutex.
VOID DestroyMutex(HANDLE hMutex);

//Set mutex object's status to signaling.This operation can wakeup all
//thread(s) waiting for this object.
DWORD ReleaseMutex(HANDLE hEvent);

/* Create a new mailbox object. */
HANDLE CreateMailbox(int size);

/* Destroy a mailbox object. */
VOID DestroyMailbox(HANDLE hMailbox);

/* Send a mail to mailbox. */
DWORD SendMail(HANDLE hMailbox, LPVOID pMsg, DWORD dwPriority, DWORD dwWaitTime, DWORD* pdwWaited);

/* Get mail from mailbox. */
DWORD GetMail(HANDLE hMailbox, LPVOID* ppMsg, DWORD dwWaitTime, DWORD* pdwWaited);

/* Create semaphore object. */
HANDLE CreateSemaphore(int max_count,int curr_count);

/* 
 * Release a semaphore object,i.e,increase it's counter and 
 * activate one kernel thread who is waiting for the semaphore
 * object if there is(are). 
 * Previous counter will be returned.
 * Rescheduling will not be triggered if bNoResched is set.
 */
int ReleaseSemaphore(HANDLE hSem, BOOL bNoResched);

/* Release the semaphore object. */
VOID DestroySemaphore(HANDLE hSem);

//Wait a synchronizing object,such as event,mutex,kernel thread,...
DWORD WaitForThisObject(HANDLE hObject);

/* 
 * Can specify a timeout value when wait a synchronizing object.
 * The actually wait time will be returned in case of wait sucess,if pdwWaited
 * is specified.
 */
DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond,DWORD* pdwWaited);

//Create a CONDITION object.
HANDLE CreateCondition(DWORD condAttr);

//Wait a CONDITION object infinitely.
DWORD WaitCondition(HANDLE hCond,HANDLE hMutex);

//Wait a CONDITION object,time out manner.
DWORD TimeoutWaitCondition(HANDLE hCond,HANDLE hMutex,DWORD dwMillionSecond);

//Signaling a CONDITION object.
DWORD SignalCondition(HANDLE hCond);

//Broadcast a CONDITION object.
DWORD BroadcastCondition(HANDLE hCond);

//Destroy a CONDITION object.
VOID DestroyCondition(HANDLE hCond);

//Bind interrupt handler and vector together,so that the handler can be
//called when the specified interrupt is raised.
HANDLE ConnectInterrupt(
	const char* int_name,
	__INTERRUPT_HANDLER lpInterruptHandler,
	LPVOID lpHandlerParam,
	UCHAR ucVector);

//Can the binding between interrupt handler and it's vector.
VOID DisconnectInterrupt(HANDLE hInterrupt);

//Allocate a memory region in system or application's memory space.
LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
	DWORD  dwSize,
	DWORD  dwAllocateFlags,
	DWORD  dwAccessFlags,
	UCHAR* lpszRegName);

//Release the memory region allocated by VirtualAlloc.
VOID VirtualFree(LPVOID lpVirtualAddr);

//Open or create a file in the specified path.
HANDLE CreateFile(LPSTR lpszFileName,
				  DWORD dwAccessMode,
				  DWORD dwShareMode,
				  LPVOID lpReserved);

//Read file's content.
BOOL ReadFile(HANDLE hFile,
			  DWORD dwReadSize,
			  LPVOID lpBuffer,
			  DWORD* lpdwReadSize);

//Write content into file.
BOOL WriteFile(HANDLE hFile,
			   DWORD dwWriteSize,
			   LPVOID lpBuffer,
			   DWORD* lpdwWrittenSize);

//Close the file opened or created by CreateFile.
VOID CloseFile(HANDLE hFile);

//Create a sub-directory under a specified directory.
BOOL CreateDirectory(LPSTR lpszDirName);

//Delete a specified file.
BOOL DeleteFile(LPSTR lpszFileName);

//Locate to the first file in a given directory,and return
//it's handle.
HANDLE FindFirstFile(LPSTR lpszDirName,
					 FS_FIND_DATA* pFindData);

//Locate the next file in a given directory.
BOOL FindNextFile(LPSTR lpszDirName,
				  HANDLE hFindHandle,
				  FS_FIND_DATA* pFindData);

//Close the transaction of travel a whole path(directory).
VOID FindClose(LPSTR lpszDirName,
			   HANDLE hFindHandle);

//Return a file's attributes.
DWORD GetFileAttributes(LPSTR lpszFileName);

//Return a given file's size,lpdwSizeHigh contains the high 32
//bits of size value,when file's size exceed 4G.
DWORD GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh);

//Remove a whole directory from file system.
BOOL RemoveDirectory(LPSTR lpszDirName);

//Set end flags in current file's current location.
BOOL SetEndOfFile(HANDLE hFile);

//Low level or hardware specified controlling operations.
BOOL IOControl(HANDLE hFile,
			   DWORD dwCommand,
			   DWORD dwInputLen,
			   LPVOID lpInputBuffer,
			   DWORD dwOutputLen,
			   LPVOID lpOutputBuffer,
			   DWORD* lpdwFilled);

//Move file pointer to a specified position.
DWORD SetFilePointer(HANDLE hFile,
					DWORD* lpdwDistLow,
					DWORD* lpdwDistHigh,
					DWORD dwMoveFlags);

//Force file system to write cached file content into storage.
BOOL FlushFileBuffers(HANDLE hFile);

//Create a device object,mainly called by device drivers.
HANDLE CreateDevice(LPSTR lpszDevName,
					DWORD dwAttributes,
					DWORD dwBlockSize,
					DWORD dwMaxReadSize,
					DWORD dwMaxWriteSize,
					LPVOID lpDevExtension,
					__DRIVER_OBJECT* lpDrvObject);

//Destroy the device object.
VOID DestroyDevice(HANDLE hDevice);

//Allocate memory from kernel memory pool.
LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType);

//Release the memory allocated by KMemAlloc.
VOID KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength);

/* Return system tick counter since boot. */
unsigned long GetClockTickCounter();

/* 
 * Get a device from system,the device is identified 
 * by ID parameter. 
 * The first matched device will be returned if pPrev is set
 * to NULL.Other matched devices following pPrev will be returned
 * if pPrev is set.
 */
__PHYSICAL_DEVICE* GetDevice(__IDENTIFIER* id, __PHYSICAL_DEVICE* pPrev);

//------------------------------------------------------------------------
HANDLE CreateRingBuff(DWORD dwBuffLength);
BOOL GetRingBuffElement(HANDLE hRb,DWORD* lpdwElement,DWORD dwMillionSecond);
BOOL AddRingBuffElement(HANDLE hRb,DWORD dwElement);
BOOL SetRingBuffLength(HANDLE hRb,DWORD dwNewLength);
VOID DestroyRingBuff(HANDLE hRb);

#ifdef __cplusplus
}
#endif

#endif  //__KAPI_H__
