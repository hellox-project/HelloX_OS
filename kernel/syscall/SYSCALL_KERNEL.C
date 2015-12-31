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

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "ethmgr.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif

static  void  SC_CreateKernelThread(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)CreateKernelThread(
		(DWORD)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(__KERNEL_THREAD_ROUTINE)PARAM(3),
		(LPVOID)PARAM(4),
		(LPVOID)PARAM(5),
		(LPSTR)PARAM(6));

		
}

static  void  SC_DestroyKernelThread(__SYSCALL_PARAM_BLOCK*  pspb)
{
	DestroyKernelThread((HANDLE)PARAM(0));
}

static  void  SC_SetLastError(__SYSCALL_PARAM_BLOCK*  pspb)
{
	SetLastError((DWORD)PARAM(0));
}

static  void  SC_GetLastError(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)GetLastError();
}

static  void  SC_GetThreadID(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)GetThreadID((HANDLE)PARAM(0));
}

static void   SC_SetThreadPriority(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SetThreadPriority((HANDLE)PARAM(0),(DWORD)PARAM(1));
}

static void   SC_GetMessage(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)GetMessage((MSG*)PARAM(0));
}

static void   SC_SendMessage(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SendMessage((HANDLE)PARAM(0),	(MSG*)PARAM(1));
}

static void   SC_Sleep(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)Sleep((DWORD)PARAM(0));
}

static void   SC_SetTimer(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SetTimer(	(DWORD)PARAM(0),(DWORD)PARAM(1),(__DIRECT_TIMER_HANDLER)PARAM(2),
		                                (LPVOID)PARAM(3),(DWORD)PARAM(4));
}

static void   SC_CancelTimer(__SYSCALL_PARAM_BLOCK*  pspb)
{
		CancelTimer((HANDLE)PARAM(0));
}

static void   SC_CreateEvent(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)CreateEvent(	(BOOL)PARAM(0));
}

static void   SC_DestroyEvent(__SYSCALL_PARAM_BLOCK*  pspb)
{
	DestroyEvent((HANDLE)PARAM(0));
}

static void   SC_SetEvent(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SetEvent(	(HANDLE)PARAM(0));
}

static void   SC_ResetEvent(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)ResetEvent(	(HANDLE)PARAM(0));
}

static void   SC_CreateMutex(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)CreateMutex();
}

static void   SC_DestroyMutex(__SYSCALL_PARAM_BLOCK*  pspb)
{
	DestroyMutex((HANDLE)PARAM(0));
}

static void   SC_ReleaseMutex(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)ReleaseMutex(	(HANDLE)PARAM(0));
}

static void   SC_WaitForThisObject(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)WaitForThisObject((HANDLE)PARAM(0));
}

static void   SC_WaitForThisObjectEx(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)WaitForThisObjectEx((HANDLE)PARAM(0),(DWORD)PARAM(1));
}

static void   SC_ConnectInterrupt(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)ConnectInterrupt(
		(__INTERRUPT_HANDLER)PARAM(0),
		(LPVOID)PARAM(1),
		(UCHAR)PARAM(2));
}

static void   SC_DisConnectInterrupt(__SYSCALL_PARAM_BLOCK*  pspb)
{
	DisconnectInterrupt((HANDLE)PARAM(0));
}

static void   SC_VirtualAlloc(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)VirtualAlloc(
		(LPVOID)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(DWORD)PARAM(3),
		(UCHAR*)PARAM(4));
}

static void   SC_VirtualFree(__SYSCALL_PARAM_BLOCK*  pspb)
{
	VirtualFree((LPVOID)PARAM(0));
}


static void   SC_KMemAlloc(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)KMemAlloc((DWORD)PARAM(0),(DWORD)PARAM(1));
}

static void   SC_KMemFree(__SYSCALL_PARAM_BLOCK*  pspb)
{
	KMemFree((LPVOID)PARAM(0),(DWORD)PARAM(1),(DWORD)PARAM(2));
}


static void   SC_RegisterSystemCall(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)RegisterSystemCall(
		(DWORD)PARAM(0),
		(DWORD)PARAM(1),
		(__SYSCALL_DISPATCH_ENTRY)PARAM(2));
}

static void   SC_ReplaceShell(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)ModuleMgr.ReplaceShell((__KERNEL_THREAD_ROUTINE)PARAM(0));
}


static void   SC_LoadDriver(__SYSCALL_PARAM_BLOCK*  pspb)
{
	#ifdef __CFG_SYS_DDF
		pspb->lpRetValue = (LPVOID)IOManager.LoadDriver(	(__DRIVER_ENTRY)PARAM(0));
	#endif
}

static void   SC_GetCurrentThread(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)KernelThreadManager.lpCurrentKernelThread;
}
	
static void   SC_GetDevice(__SYSCALL_PARAM_BLOCK*  pspb)
{
#ifdef __CFG_SYS_DDF
	pspb->lpRetValue = (LPVOID)DeviceManager.GetDevice(
		&DeviceManager,
		(DWORD)PARAM(0),
		(__IDENTIFIER*)PARAM(1),
		(__PHYSICAL_DEVICE*)PARAM(2));
#endif
}

static void   SC_SwitchToGraphic(__SYSCALL_PARAM_BLOCK*  pspb)
{
	#ifdef __I386__
		pspb->lpRetValue = (LPVOID)SwitchToGraphic();
	#endif
}

static void   SC_SwitchToText(__SYSCALL_PARAM_BLOCK*  pspb)
{
	#ifdef __I386__
		SwitchToText();
	#endif 
}

static void   SC_SetFocusThread(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)PARAM(0));
}

static void   SC_GetEthernetInterfaceState(__SYSCALL_PARAM_BLOCK*  pspb)
{
#ifdef __CFG_NET_ETHMGR
	pspb->lpRetValue = (LPVOID)EthernetManager.GetEthernetInterfaceState(
		(__ETH_INTERFACE_STATE*)PARAM(0),
		(int)PARAM(1),
		(int*)PARAM(2));
#endif
}

void  RegisterKernelEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	pSysCallEntry[SYSCALL_CREATEKERNELTHREAD]        = SC_CreateKernelThread;
	pSysCallEntry[SYSCALL_DESTROYKERNELTHREAD]       = SC_DestroyKernelThread;
	pSysCallEntry[SYSCALL_SETLASTERROR]              = SC_SetLastError;
	pSysCallEntry[SYSCALL_GETLASTERROR]              = SC_GetLastError;
	pSysCallEntry[SYSCALL_GETTHREADID]               = SC_GetThreadID;
	pSysCallEntry[SYSCALL_SETTHREADPRIORITY]         = SC_SetThreadPriority;
	pSysCallEntry[SYSCALL_GETMESSAGE]                = SC_GetMessage;
	pSysCallEntry[SYSCALL_SENDMESSAGE]               = SC_SendMessage;
	pSysCallEntry[SYSCALL_SLEEP]                     = SC_Sleep;
	pSysCallEntry[SYSCALL_SETTIMER]                  = SC_SetTimer;
	pSysCallEntry[SYSCALL_CANCELTIMER]               = SC_CancelTimer;

	pSysCallEntry[SYSCALL_CREATEEVENT]               = SC_CreateEvent;
	pSysCallEntry[SYSCALL_DESTROYEVENT]              = SC_DestroyEvent;	
	pSysCallEntry[SYSCALL_SETEVENT]                  = SC_SetEvent;
	pSysCallEntry[SYSCALL_RESETEVENT]                = SC_ResetEvent;

	pSysCallEntry[SYSCALL_CREATEMUTEX]               = SC_CreateMutex;
	pSysCallEntry[SYSCALL_DESTROYMUTEX]              = SC_DestroyMutex;
	pSysCallEntry[SYSCALL_RELEASEMUTEX]              = SC_ReleaseMutex;

	pSysCallEntry[SYSCALL_WAITFORTHISOBJECT]         = SC_WaitForThisObject;
	pSysCallEntry[SYSCALL_WAITFORTHISOBJECTEX]       = SC_WaitForThisObjectEx;

	pSysCallEntry[SYSCALL_CONNECTINTERRUPT]          = SC_ConnectInterrupt;
	pSysCallEntry[SYSCALL_DISCONNECTINTERRUPT]       = SC_DisConnectInterrupt;
	
	pSysCallEntry[SYSCALL_VIRTUALALLOC]              = SC_VirtualAlloc;
	pSysCallEntry[SYSCALL_VIRTUALFREE]               = SC_VirtualFree;
	
	
	pSysCallEntry[SYSCALL_KMEMALLOC]                 = SC_KMemAlloc;
	pSysCallEntry[SYSCALL_KMEMFREE]                  = SC_KMemFree;

	pSysCallEntry[SYSCALL_REGISTERSYSTEMCALL]        = SC_RegisterSystemCall;
	pSysCallEntry[SYSCALL_REPLACESHELL]              = SC_ReplaceShell;

	pSysCallEntry[SYSCALL_LOADDRIVER]                = SC_LoadDriver;
	pSysCallEntry[SYSCALL_GETCURRENTTHREAD]          = SC_GetCurrentThread;
	pSysCallEntry[SYSCALL_GETDEVICE]                 = SC_GetDevice;

	pSysCallEntry[SYSCALL_SWITCHTOGRAPHIC]           = SC_SwitchToGraphic;
	pSysCallEntry[SYSCALL_SWITCHTOTEXT]              = SC_SwitchToText;
	pSysCallEntry[SYSCALL_SETFOCUSTHREAD]            = SC_SetFocusThread;
	pSysCallEntry[SYSCALL_GETETHERNETINTERFACESTATE] = SC_GetEthernetInterfaceState;
}
