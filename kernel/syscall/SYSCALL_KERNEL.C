//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 08, 25,2015
//    Module Name               : SYSCALL_KERNEL.CPP
//    Module Funciton           : 
//                                System KERNEL call implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "syscall.h"
#include "kapi.h"
#include "modmgr.h"
#include "stdio.h"
#include "devmgr.h"
#include "iomgr.h"
#include "process.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif

/*
 * Macros to refer the parameter in system call parameter block.
 * Just to simplify the programming of system call's implementation.
 */
#define PARAM(idx) (pspb->param_##idx.param)
#define SYSCALL_RET (*pspb->ret_ptr.param)

static void SC_CreateKernelThread(__SYSCALL_PARAM_BLOCK* pspb)
{
#if 0
	SYSCALL_RET = (uint32_t)CreateKernelThread(
		(DWORD)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(__KERNEL_THREAD_ROUTINE)PARAM(3),
		(uint32_t)PARAM(4),
		(uint32_t)PARAM(5),
		(LPSTR)PARAM(6));		
#endif
}

static void SC_DestroyKernelThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	DestroyKernelThread((HANDLE)PARAM(0));
}

static void SC_SetLastError(__SYSCALL_PARAM_BLOCK* pspb)
{
	SetLastError((DWORD)PARAM(0));
}

static void SC_GetLastError(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetLastError();
}

static void SC_GetThreadID(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetThreadID((HANDLE)PARAM(0));
}

static void SC_SetThreadPriority(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SetThreadPriority((HANDLE)PARAM(0),(DWORD)PARAM(1));
}

static void SC_GetMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetMessage((MSG*)PARAM(0));
}

static void SC_SendMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SendMessage((HANDLE)PARAM(0),	(MSG*)PARAM(1));
}

static void SC_PeekMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)KernelThreadManager.PeekMessage(
		NULL,
		(__KERNEL_THREAD_MESSAGE*)PARAM(0));
}

static void SC_Sleep(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)Sleep((DWORD)PARAM(0));
}

static void SC_SetTimer(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SetTimer(
		(DWORD)PARAM(0), (DWORD)PARAM(1), (__DIRECT_TIMER_HANDLER)PARAM(2),
		(LPVOID)PARAM(3), (DWORD)PARAM(4));
}

static void SC_CancelTimer(__SYSCALL_PARAM_BLOCK* pspb)
{
	CancelTimer((HANDLE)PARAM(0));
}

static void SC_CreateEvent(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)CreateEvent((BOOL)PARAM(0));
}

static void SC_DestroyEvent(__SYSCALL_PARAM_BLOCK* pspb)
{
	DestroyEvent((HANDLE)PARAM(0));
}

static void SC_SetEvent(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SetEvent((HANDLE)PARAM(0));
}

static void SC_ResetEvent(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ResetEvent((HANDLE)PARAM(0));
}

static void SC_CreateMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)CreateMutex();
}

static void SC_DestroyMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	DestroyMutex((HANDLE)PARAM(0));
}

static void SC_ReleaseMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ReleaseMutex((HANDLE)PARAM(0));
}

static void SC_WaitForThisObject(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)WaitForThisObject((HANDLE)PARAM(0));
}

static void SC_WaitForThisObjectEx(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)WaitForThisObjectEx((HANDLE)PARAM(0),(DWORD)PARAM(1),NULL);
}

static void SC_ConnectInterrupt(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ConnectInterrupt(
		(__INTERRUPT_HANDLER)PARAM(0),
		(LPVOID)PARAM(1),
		(UCHAR)PARAM(2));
}

static void SC_DisConnectInterrupt(__SYSCALL_PARAM_BLOCK* pspb)
{
	DisconnectInterrupt((HANDLE)PARAM(0));
}

static void SC_VirtualAlloc(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	SYSCALL_RET = (uint32_t)pVmmMgr->VirtualAlloc(
		(__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(DWORD)PARAM(3),
		(UCHAR*)PARAM(4),
		NULL);
}

static void SC_VirtualFree(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	pVmmMgr->VirtualFree((__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)PARAM(0));
}

static void SC_KMemAlloc(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)KMemAlloc((DWORD)PARAM(0),(DWORD)PARAM(1));
}

static void SC_KMemFree(__SYSCALL_PARAM_BLOCK* pspb)
{
	KMemFree((LPVOID)PARAM(0),(DWORD)PARAM(1),(DWORD)PARAM(2));
}


static void SC_RegisterSystemCall(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)RegisterSystemCall(
		(DWORD)PARAM(0),
		(DWORD)PARAM(1),
		(__SYSCALL_DISPATCH_ENTRY)PARAM(2));
}

static void SC_ReplaceShell(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ModuleMgr.ReplaceShell((__KERNEL_THREAD_ROUTINE)PARAM(0));
}

static void SC_LoadDriver(__SYSCALL_PARAM_BLOCK* pspb)
{
#ifdef __CFG_SYS_DDF
	SYSCALL_RET = (uint32_t)IOManager.LoadDriver(	(__DRIVER_ENTRY)PARAM(0));
#endif
}

static void SC_GetCurrentThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)__CURRENT_KERNEL_THREAD;
}
	
static void SC_GetDevice(__SYSCALL_PARAM_BLOCK* pspb)
{
#ifdef __CFG_SYS_DDF
	SYSCALL_RET = (uint32_t)DeviceManager.GetDevice(
		&DeviceManager,
		(DWORD)PARAM(0),
		(__IDENTIFIER*)PARAM(1),
		(__PHYSICAL_DEVICE*)PARAM(2));
#endif
}

static void SC_SwitchToGraphic(__SYSCALL_PARAM_BLOCK* pspb)
{
#ifdef __I386__
		SYSCALL_RET = (uint32_t)SwitchToGraphic();
#endif
}

static void SC_SwitchToText(__SYSCALL_PARAM_BLOCK* pspb)
{
#ifdef __I386__
		SwitchToText();
#endif 
}

static void SC_SetFocusThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)PARAM(0));
}

static void SC_GetSystemTime(__SYSCALL_PARAM_BLOCK* pspb)
{
	__GetTime((BYTE*)PARAM(0));
}

static void SC_GetCursorPos(__SYSCALL_PARAM_BLOCK* pspb)
{
	CD_GetCursorPos((WORD*)PARAM(0),
		(WORD*)PARAM(1));
}

static void SC_SetCursorPos(__SYSCALL_PARAM_BLOCK* pspb)
{
	CD_SetCursorPos((WORD)PARAM(0),
		(WORD)PARAM(1));
}

static void SC_TerminateKernelThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	KernelThreadManager.TerminateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)PARAM(0),
		(DWORD)PARAM(1));
}

/* Call half bottom part of a process. */
static void SC_ProcessHalfBottom(__SYSCALL_PARAM_BLOCK* pspb)
{
	ProcessManager.ProcessHalfBottom();
}

void  RegisterKernelEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	pSysCallEntry[SYSCALL_CREATEKERNELTHREAD] = SC_CreateKernelThread;
	pSysCallEntry[SYSCALL_DESTROYKERNELTHREAD] = SC_DestroyKernelThread;
	pSysCallEntry[SYSCALL_SETLASTERROR] = SC_SetLastError;
	pSysCallEntry[SYSCALL_GETLASTERROR] = SC_GetLastError;
	pSysCallEntry[SYSCALL_GETTHREADID] = SC_GetThreadID;
	pSysCallEntry[SYSCALL_SETTHREADPRIORITY] = SC_SetThreadPriority;
	pSysCallEntry[SYSCALL_GETMESSAGE] = SC_GetMessage;
	pSysCallEntry[SYSCALL_SENDMESSAGE] = SC_SendMessage;
	pSysCallEntry[SYSCALL_SLEEP] = SC_Sleep;
	pSysCallEntry[SYSCALL_SETTIMER] = SC_SetTimer;
	pSysCallEntry[SYSCALL_CANCELTIMER] = SC_CancelTimer;

	pSysCallEntry[SYSCALL_CREATEEVENT] = SC_CreateEvent;
	pSysCallEntry[SYSCALL_DESTROYEVENT] = SC_DestroyEvent;
	pSysCallEntry[SYSCALL_SETEVENT] = SC_SetEvent;
	pSysCallEntry[SYSCALL_RESETEVENT] = SC_ResetEvent;
	pSysCallEntry[SYSCALL_CREATEMUTEX] = SC_CreateMutex;
	pSysCallEntry[SYSCALL_DESTROYMUTEX] = SC_DestroyMutex;
	pSysCallEntry[SYSCALL_RELEASEMUTEX] = SC_ReleaseMutex;
	pSysCallEntry[SYSCALL_WAITFORTHISOBJECT] = SC_WaitForThisObject;
	pSysCallEntry[SYSCALL_WAITFORTHISOBJECTEX] = SC_WaitForThisObjectEx;

	pSysCallEntry[SYSCALL_CONNECTINTERRUPT] = SC_ConnectInterrupt;
	pSysCallEntry[SYSCALL_DISCONNECTINTERRUPT] = SC_DisConnectInterrupt;
	
	pSysCallEntry[SYSCALL_VIRTUALALLOC] = SC_VirtualAlloc;
	pSysCallEntry[SYSCALL_VIRTUALFREE] = SC_VirtualFree;
	pSysCallEntry[SYSCALL_KMEMALLOC] = SC_KMemAlloc;
	pSysCallEntry[SYSCALL_KMEMFREE] = SC_KMemFree;
	pSysCallEntry[SYSCALL_REGISTERSYSTEMCALL] = SC_RegisterSystemCall;
	pSysCallEntry[SYSCALL_REPLACESHELL] = SC_ReplaceShell;
	pSysCallEntry[SYSCALL_LOADDRIVER] = SC_LoadDriver;
	pSysCallEntry[SYSCALL_GETCURRENTTHREAD] = SC_GetCurrentThread;
	pSysCallEntry[SYSCALL_GETDEVICE] = SC_GetDevice;

	pSysCallEntry[SYSCALL_SWITCHTOGRAPHIC] = SC_SwitchToGraphic;
	pSysCallEntry[SYSCALL_SWITCHTOTEXT] = SC_SwitchToText;
	pSysCallEntry[SYSCALL_SETFOCUSTHREAD] = SC_SetFocusThread;
	pSysCallEntry[SYSCALL_GETSYSTEMTIME] = SC_GetSystemTime;
	pSysCallEntry[SYSCALL_GETCURSORPOS] = SC_GetCursorPos;
	pSysCallEntry[SYSCALL_SETCURSORPOS] = SC_SetCursorPos;
	pSysCallEntry[SYSCALL_TERMINATEKERNELTHREAD] = SC_TerminateKernelThread;

	pSysCallEntry[SYSCALL_PEEKMESSAGE] = SC_PeekMessage;
	pSysCallEntry[SYSCALL_PROCESSHALFBOTTOM] = SC_ProcessHalfBottom;
}

#undef PARAM
#undef SYSCALL_RET
