//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.CPP
//    Module Funciton           : 
//                                All routines in kernel module are wrapped
//                                in this file.
//
//    Last modified Author      : Garry
//    Last modified Date        : Jan 09,2012
//    Last modified Content     : 
//                                1.System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "K_SOCKET.H"
#include "K_KERNEL.H"

HANDLE CreateKernelThread(DWORD dwStackSize,
						  DWORD dwInitStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved,
						  LPSTR lpszName)
{
	SYSCALL_PARAM_7(SYSCALL_CREATEKERNELTHREAD,
		dwStackSize,dwInitStatus,
		dwPriority,lpStartRoutine,lpRoutineParam,
		lpReserved,lpszName);
}

VOID DestroyKernelThread(HANDLE hThread)
{
	SYSCALL_PARAM_1(SYSCALL_DESTROYKERNELTHREAD,hThread);
}

DWORD SetLastError(DWORD dwNewError)
{
	SYSCALL_PARAM_1(SYSCALL_SETLASTERROR,dwNewError);
}

DWORD GetLastError()
{
	SYSCALL_PARAM_0(SYSCALL_GETLASTERROR);
}

DWORD GetThreadID(HANDLE hThread)
{
	SYSCALL_PARAM_1(SYSCALL_GETTHREADID,hThread);
}

DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority)
{
	SYSCALL_PARAM_2(SYSCALL_SETTHREADPRIORITY,hThread,dwPriority);
}

BOOL GetMessage(MSG* lpMsg)
{
	SYSCALL_PARAM_1(SYSCALL_GETMESSAGE,lpMsg);
}

BOOL SendMessage(HANDLE hThread,MSG* lpMsg)
{
	SYSCALL_PARAM_2(SYSCALL_SENDMESSAGE,hThread,lpMsg);
}

BOOL Sleep(DWORD dwMillionSecond)
{
	SYSCALL_PARAM_1(SYSCALL_SLEEP,dwMillionSecond);
}

HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags)
{
	SYSCALL_PARAM_5(SYSCALL_SETTIMER,
		dwTimerID,
		dwMillionSecond,
		lpHandler,
		lpHandlerParam,
		dwTimerFlags);
}

VOID CancelTimer(HANDLE hTimer)
{
	SYSCALL_PARAM_1(SYSCALL_CANCELTIMER,hTimer);
}

HANDLE CreateEvent(BOOL bInitialStatus)
{
	SYSCALL_PARAM_1(SYSCALL_CREATEEVENT,bInitialStatus);
}

VOID DestroyEvent(HANDLE hEvent)
{
	SYSCALL_PARAM_1(SYSCALL_DESTROYEVENT,hEvent);
}

DWORD SetEvent(HANDLE hEvent)
{
	SYSCALL_PARAM_1(SYSCALL_SETEVENT,hEvent);
}

DWORD ResetEvent(HANDLE hEvent)
{
	SYSCALL_PARAM_1(SYSCALL_RESETEVENT,hEvent);
}

HANDLE CreateMutex()
{
	SYSCALL_PARAM_0(SYSCALL_CREATEMUTEX);
}

VOID DestroyMutex(HANDLE hMutex)
{
	SYSCALL_PARAM_1(SYSCALL_DESTROYMUTEX,hMutex);
}

DWORD ReleaseMutex(HANDLE hMutex)
{
	SYSCALL_PARAM_1(SYSCALL_RELEASEMUTEX,hMutex);
}

DWORD WaitForThisObject(HANDLE hObject)
{
	SYSCALL_PARAM_1(SYSCALL_WAITFORTHISOBJECT,hObject);
}

DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond)
{
	SYSCALL_PARAM_2(SYSCALL_WAITFORTHISOBJECTEX,hObject,
		dwMillionSecond);
}

HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
						LPVOID              lpHandlerParam,
						UCHAR               ucVector)
{
	__asm{
		push ebx
		mov bl,ucVector
		push ebx
		push lpHandlerParam
		push lpInterruptHandler
		push 0
		push SYSCALL_CONNECTINTERRUPT
		int 0x7F
		pop eax
		pop eax
		pop lpInterruptHandler
		pop lpHandlerParam
		pop ebx
		mov ucVector,bl
		pop ebx
	}
}

VOID DisconnectInterrupt(HANDLE hInterrupt)
{
	SYSCALL_PARAM_1(SYSCALL_DISCONNECTINTERRUPT,hInterrupt);
}

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
					DWORD  dwSize,
					DWORD  dwAllocateFlags,
					DWORD  dwAccessFlags,
					CHAR*  lpszRegName)
{
	SYSCALL_PARAM_5(SYSCALL_VIRTUALALLOC,
		lpDesiredAddr,
		dwSize,
		dwAllocateFlags,
		dwAccessFlags,
		lpszRegName);
}

VOID VirtualFree(LPVOID lpVirtualAddr)
{
	SYSCALL_PARAM_1(SYSCALL_VIRTUALFREE,lpVirtualAddr);
}

LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType)
{	
	SYSCALL_PARAM_2(SYSCALL_KMEMALLOC,dwSize,dwSizeType);
}

VOID KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength)
{
	SYSCALL_PARAM_3(SYSCALL_KMEMFREE,lpMemAddr,
		dwSizeType,
		dwMemLength);
}

VOID   KMemCpy(VOID* pDst,VOID* pSrc,INT nSize)
{
	uint8*  dst = (uint8*)pDst;
	uint8*  src = (uint8*)pSrc;
	INT     i;

	for(i=0; i < nSize;i++)
	{
		*dst = *src;
		dst ++;
		src ++;
	}
}

BOOL RegisterSystemCall(DWORD dwStartSyscallNum,DWORD dwEndSyscallNum,
						__SYSCALL_DISPATCH_ENTRY sde)
{
	SYSCALL_PARAM_3(SYSCALL_REGISTERSYSTEMCALL,
		dwStartSyscallNum,
		dwEndSyscallNum,
		sde);
}

BOOL ReplaceShell(__KERNEL_THREAD_ROUTINE shell)
{
	SYSCALL_PARAM_1(SYSCALL_REPLACESHELL,
		shell);
}

BOOL LoadDriver(__DRIVER_ENTRY de)
{
	SYSCALL_PARAM_1(SYSCALL_LOADDRIVER,	de);
}

HANDLE GetCurrentThread()
{
	SYSCALL_PARAM_0(SYSCALL_GETCURRENTTHREAD);
}

BOOL SwitchToGraphic()
{
	SYSCALL_PARAM_0(SYSCALL_SWITCHTOGRAPHIC);
}

VOID SwitchToText()
{
	SYSCALL_PARAM_0(SYSCALL_SWITCHTOTEXT);
}

HANDLE SetFocusThread(HANDLE hNewThread)
{
	SYSCALL_PARAM_1(SYSCALL_SETFOCUSTHREAD,	hNewThread);
}

BOOL GetEthernetInterfaceState(__ETH_INTERFACE_STATE* pState, int nIndex, int* pnNextInt)
{
	SYSCALL_PARAM_3(SYSCALL_GETETHERNETINTERFACESTATE, pState, nIndex, pnNextInt);
}
