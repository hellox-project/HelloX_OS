//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2009
//    Module Name               : SYSCALL.CPP
//    Module Funciton           : 
//                                System call implementation code.
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

#include "syscall.h"
#include "kapi.h"
#include "modmgr.h"
#include "stdio.h"
//devmgr.h
#include "devmgr.h"
#include "iomgr.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif

//A static array to contain system call range.
static __SYSCALL_RANGE SyscallRange[SYSCALL_RANGE_NUM] = {0};

//System call used by other kernel module to register their system
//call range.
BOOL RegisterSystemCall(DWORD dwStartNum,DWORD dwEndNum,
						__SYSCALL_DISPATCH_ENTRY sde)
{
	int i = 0;

	if((dwStartNum > dwEndNum) || (NULL == sde))
	{
		return FALSE;
	}
	while(i < SYSCALL_RANGE_NUM)
	{
		if((0 == SyscallRange[i].dwStartSyscallNum) && ( 0 == SyscallRange[i].dwEndSyscallNum))
		{
			break;
		}
		i += 1;
	}
	if(SYSCALL_RANGE_NUM == i)  //Can not find a empty slot.
	{
		return FALSE;
	}
	//Add the requested range to system call range array.
	SyscallRange[i].dwStartSyscallNum = dwStartNum;
	SyscallRange[i].dwEndSyscallNum   = dwEndNum;
	SyscallRange[i].sde               = sde;
	return TRUE;
}

//If can not find a proper service routine in master kernel,then try to dispatch
//the system call to other module(s) by looking system call range array.
static BOOL DispatchToModule(LPVOID lpEsp,LPVOID lpParam)
{
	__SYSCALL_PARAM_BLOCK*  pspb = (__SYSCALL_PARAM_BLOCK*)lpEsp;
	int i = 0;

	for(i = 0;i < SYSCALL_RANGE_NUM;i ++)
	{
		if((SyscallRange[i].dwStartSyscallNum <= pspb->dwSyscallNum) &&
		   (SyscallRange[i].dwEndSyscallNum   >= pspb->dwSyscallNum))
		{
			SyscallRange[i].sde(lpEsp,NULL);
			return TRUE;
		}
	}
	return FALSE;  //Can not find a system call range to handle it.
}

//System call entry point.
BOOL SyscallHandler(LPVOID lpEsp,LPVOID lpParam)
{
	__SYSCALL_PARAM_BLOCK*  pspb = (__SYSCALL_PARAM_BLOCK*)lpEsp;

	if(NULL == lpEsp)
	{
		return FALSE;
	}
	//Call the proper service routine according to system call number.
	//We use switch and case clauses to avoid the complaint problem in
	//assemble language.
	//But don't worry about the performance,clever compiler will generate
	//jumping table which can locate one service routine only by ONE array
	//locating.
#define PARAM(i) (pspb->lpParams[i])  //To simplify the programming.
	switch(pspb->dwSyscallNum)
	{
	case SYSCALL_CREATEKERNELTHREAD:
		pspb->lpRetValue = (LPVOID)CreateKernelThread(
			(DWORD)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2),
			(__KERNEL_THREAD_ROUTINE)PARAM(3),
			(LPVOID)PARAM(4),
			(LPVOID)PARAM(5),
			(LPSTR)PARAM(6));
		break;
	case SYSCALL_DESTROYKERNELTHREAD:
		DestroyKernelThread(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_SETLASTERROR:
		SetLastError(
			(DWORD)PARAM(0));
		break;
	case SYSCALL_GETLASTERROR:
		pspb->lpRetValue = (LPVOID)GetLastError();
		break;
	case SYSCALL_GETTHREADID:
		pspb->lpRetValue = (LPVOID)GetThreadID(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_SETTHREADPRIORITY:
		pspb->lpRetValue = (LPVOID)SetThreadPriority(
			(HANDLE)PARAM(0),
			(DWORD)PARAM(1));
		break;
	case SYSCALL_GETMESSAGE:
		pspb->lpRetValue = (LPVOID)GetMessage(
			(MSG*)PARAM(0));
		break;
	case SYSCALL_SENDMESSAGE:
		pspb->lpRetValue = (LPVOID)SendMessage(
			(HANDLE)PARAM(0),
			(MSG*)PARAM(1));
		break;
	case SYSCALL_SLEEP:
		pspb->lpRetValue = (LPVOID)Sleep(
			(DWORD)PARAM(0));
		break;
	case SYSCALL_SETTIMER:
		pspb->lpRetValue = (LPVOID)SetTimer(
			(DWORD)PARAM(0),
			(DWORD)PARAM(1),
			(__DIRECT_TIMER_HANDLER)PARAM(2),
			(LPVOID)PARAM(3),
			(DWORD)PARAM(4));
		break;
	case SYSCALL_CANCELTIMER:
		CancelTimer(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CREATEEVENT:
		pspb->lpRetValue = (LPVOID)CreateEvent(
			(BOOL)PARAM(0));
		break;
	case SYSCALL_DESTROYEVENT:
		DestroyEvent(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_SETEVENT:
		pspb->lpRetValue = (LPVOID)SetEvent(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_RESETEVENT:
		pspb->lpRetValue = (LPVOID)ResetEvent(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CREATEMUTEX:
		pspb->lpRetValue = (LPVOID)CreateMutex();
		break;
	case SYSCALL_DESTROYMUTEX:
		DestroyMutex(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_RELEASEMUTEX:
		pspb->lpRetValue = (LPVOID)ReleaseMutex(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_WAITFORTHISOBJECT:
		pspb->lpRetValue = (LPVOID)WaitForThisObject(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_WAITFORTHISOBJECTEX:
		pspb->lpRetValue = (LPVOID)WaitForThisObjectEx(
			(HANDLE)PARAM(0),
			(DWORD)PARAM(1));
		break;
	case SYSCALL_CONNECTINTERRUPT:
		pspb->lpRetValue = (LPVOID)ConnectInterrupt(
			(__INTERRUPT_HANDLER)PARAM(0),
			(LPVOID)PARAM(1),
			(UCHAR)PARAM(2));
		break;
	case SYSCALL_DISCONNECTINTERRUPT:
		DisconnectInterrupt(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_VIRTUALALLOC:
		pspb->lpRetValue = (LPVOID)VirtualAlloc(
			(LPVOID)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2),
			(DWORD)PARAM(3),
			(UCHAR*)PARAM(4));
		break;
	case SYSCALL_VIRTUALFREE:
		VirtualFree(
			(LPVOID)PARAM(0));
		break;
	case SYSCALL_CREATEFILE:
		pspb->lpRetValue = (LPVOID)CreateFile(
			(LPSTR)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2),
			(LPVOID)PARAM(3));
		break;
	case SYSCALL_READFILE:
		pspb->lpRetValue = (LPVOID)ReadFile(
			(HANDLE)PARAM(0),
			(DWORD)PARAM(1),
			(LPVOID)PARAM(2),
			(DWORD*)PARAM(3));
		break;
	case SYSCALL_WRITEFILE:
		pspb->lpRetValue = (LPVOID)WriteFile(
			(HANDLE)PARAM(0),
			(DWORD)PARAM(1),
			(LPVOID)PARAM(2),
			(DWORD*)PARAM(3));
		break;
	case SYSCALL_CLOSEFILE:
		CloseFile(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CREATEDIRECTORY:
		pspb->lpRetValue = (LPVOID)CreateDirectory(
			(LPSTR)PARAM(0));
		break;
	case SYSCALL_DELETEFILE:
		pspb->lpRetValue = (LPVOID)DeleteFile(
			(LPSTR)PARAM(0));
		break;
	case SYSCALL_FINDFIRSTFILE:
		pspb->lpRetValue = (LPVOID)FindFirstFile(
			(LPSTR)PARAM(0),
			(FS_FIND_DATA*)PARAM(1));
		break;
	case SYSCALL_FINDNEXTFILE:
		pspb->lpRetValue = (LPVOID)FindNextFile(
			(LPSTR)PARAM(0),
			(HANDLE)PARAM(1),
			(FS_FIND_DATA*)PARAM(2));
		break;
	case SYSCALL_FINDCLOSE:
		FindClose(
			(LPSTR)PARAM(0),
			(HANDLE)PARAM(1));
		break;
	case SYSCALL_GETFILEATTRIBUTES:
		pspb->lpRetValue = (LPVOID)GetFileAttributes(
			(LPSTR)PARAM(0));
		break;
	case SYSCALL_GETFILESIZE:
		pspb->lpRetValue = (LPVOID)GetFileSize(
			(HANDLE)PARAM(0),
			(DWORD*)PARAM(1));
		break;
	case SYSCALL_REMOVEDIRECTORY:
		pspb->lpRetValue = (LPVOID)RemoveDirectory(
			(LPSTR)PARAM(0));
		break;
	case SYSCALL_SETENDOFFILE:
		pspb->lpRetValue = (LPVOID)SetEndOfFile(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_IOCONTROL:
		pspb->lpRetValue = (LPVOID)IOControl(
			(HANDLE)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2),
			(LPVOID)PARAM(3),
			(DWORD)PARAM(4),
			(LPVOID)PARAM(5),
			(DWORD*)PARAM(6));
		break;
	case SYSCALL_SETFILEPOINTER:
		pspb->lpRetValue = (LPVOID)SetFilePointer(
			(HANDLE)PARAM(0),
			(DWORD*)PARAM(1),
			(DWORD*)PARAM(2),
			(DWORD)PARAM(3));
		break;
	case SYSCALL_FLUSHFILEBUFFERS:
		pspb->lpRetValue = (LPVOID)FlushFileBuffers(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CREATEDEVICE:
		pspb->lpRetValue = (LPVOID)CreateDevice(
			(LPSTR)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2),
			(DWORD)PARAM(3),
			(DWORD)PARAM(4),
			(LPVOID)PARAM(5),
			(__DRIVER_OBJECT*)PARAM(6));
		break;
	case SYSCALL_DESTROYDEVICE:
		DestroyDevice(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_KMEMALLOC:
		pspb->lpRetValue = (LPVOID)KMemAlloc(
			(DWORD)PARAM(0),
			(DWORD)PARAM(1));
		break;
	case SYSCALL_KMEMFREE:
		KMemFree(
			(LPVOID)PARAM(0),
			(DWORD)PARAM(1),
			(DWORD)PARAM(2));
		break;
	case SYSCALL_PRINTLINE:
		PrintLine(
			(LPSTR)PARAM(0));
		break;
	case SYSCALL_PRINTCHAR:
		PrintCh(
			(WORD)PARAM(0));
		break;
	case SYSCALL_REGISTERSYSTEMCALL:
		pspb->lpRetValue = (LPVOID)RegisterSystemCall(
			(DWORD)PARAM(0),
			(DWORD)PARAM(1),
			(__SYSCALL_DISPATCH_ENTRY)PARAM(2));
		break;
	case SYSCALL_REPLACESHELL:
		pspb->lpRetValue = (LPVOID)ModuleMgr.ReplaceShell(
			(__KERNEL_THREAD_ROUTINE)PARAM(0));
		break;
	case SYSCALL_LOADDRIVER:
#ifdef __CFG_SYS_DDF
		pspb->lpRetValue = (LPVOID)IOManager.LoadDriver(
			(__DRIVER_ENTRY)PARAM(0));
#endif
		break;
	case SYSCALL_GETCURRENTTHREAD:
		pspb->lpRetValue = (LPVOID)KernelThreadManager.lpCurrentKernelThread;
		break;
	case SYSCALL_GETDEVICE:
#ifdef __CFG_SYS_DDF
		pspb->lpRetValue = (LPVOID)DeviceManager.GetDevice(
			&DeviceManager,
			(DWORD)PARAM(0),
			(__IDENTIFIER*)PARAM(1),
			(__PHYSICAL_DEVICE*)PARAM(2));
#endif
		break;
#ifdef __I386__
	case SYSCALL_SWITCHTOGRAPHIC:
		pspb->lpRetValue = (LPVOID)SwitchToGraphic();
		break;
	case SYSCALL_SWITCHTOTEXT:
		SwitchToText();
		break;
#endif
	case SYSCALL_SETFOCUSTHREAD:
		pspb->lpRetValue = (LPVOID)DeviceInputManager.SetFocusThread(
			(__COMMON_OBJECT*)&DeviceInputManager,
			(__COMMON_OBJECT*)PARAM(0));
		break;
	default:
		if(!DispatchToModule(lpEsp,NULL))
		{
			PrintLine("  SyscallHandler: Unknown system call is requested.");
		}
		break;
	}
	return TRUE;
}
