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
#include "stdio.h"  //For _hx_printf routine.

/* Implementation code for each system call. */

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
#ifdef __GCC__
	__asm__ __volatile__(
			".code32			;"
			"pushl 	%%ebx		;"
			"movb	%3,	%%bl	;"
			"pushl	%%ebx				;"
			"pushl	%4					;"
			"pushl	%5					;"
			"pushl	$0					;"
			"pushl	SYSCALL_CONNECTINTERRUPT;"
			"int	$0x7F					;"
			"popl	%%eax					;"
			"popl	%%eax					;"
			"popl	%0						;"
			"popl	%1						;"
			"popl	%%ebx					;"
			"movb	%%bl, %2				;"
			"popl	%%ebx					;"
			:"=r"(lpInterruptHandler), "=r"(lpHandlerParam), "=r"(ucVector)
			:"r"(ucVector), "r"(lpHandlerParam), "r"(lpInterruptHandler)
			:
			);
#else
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
#endif
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

HANDLE CreateFile(LPSTR lpszFileName,
				  DWORD dwAccessMode,
				  DWORD dwShareMode,
				  LPVOID lpReserved)
{
	SYSCALL_PARAM_4(SYSCALL_CREATEFILE,
		lpszFileName,
		dwAccessMode,
		dwShareMode,
		lpReserved);
}

BOOL ReadFile(HANDLE hFile,
			  DWORD dwReadSize,
			  LPVOID lpBuffer,
			  DWORD* lpdwReadSize)
{
	SYSCALL_PARAM_4(SYSCALL_READFILE,
		hFile,
		dwReadSize,
		lpBuffer,
		lpdwReadSize);
}

BOOL WriteFile(HANDLE hFile,
			   DWORD dwWriteSize,
			   LPVOID lpBuffer,
			   DWORD* lpdwWrittenSize)
{
	SYSCALL_PARAM_4(SYSCALL_WRITEFILE,
		hFile,
		dwWriteSize,
		lpBuffer,
		lpdwWrittenSize);
}

VOID CloseFile(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_CLOSEFILE,
		hFile);
}

BOOL CreateDirectory(LPSTR lpszDirName)
{
	SYSCALL_PARAM_1(SYSCALL_CREATEDIRECTORY,lpszDirName);
}

BOOL DeleteFile(LPSTR lpszFileName)
{
	SYSCALL_PARAM_1(SYSCALL_DELETEFILE,lpszFileName);
}

HANDLE FindFirstFile(LPSTR lpszDirName,
					 FS_FIND_DATA* pFindData)
{
	SYSCALL_PARAM_2(SYSCALL_FINDFIRSTFILE,
		lpszDirName,
		pFindData);
}

BOOL FindNextFile(LPSTR lpszDirName,
				  HANDLE hFindHandle,
				  FS_FIND_DATA* pFindData)
{
	SYSCALL_PARAM_3(SYSCALL_FINDNEXTFILE,lpszDirName,
		hFindHandle,
		pFindData);
}

VOID FindClose(LPSTR lpszDirName,
			   HANDLE hFindHandle)
{
	SYSCALL_PARAM_2(SYSCALL_FINDCLOSE,
		lpszDirName,
		hFindHandle);
}

DWORD GetFileAttributes(LPSTR lpszFileName)
{
	SYSCALL_PARAM_1(SYSCALL_GETFILEATTRIBUTES,lpszFileName);
}

DWORD GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh)
{
	SYSCALL_PARAM_2(SYSCALL_GETFILESIZE,hFile,
		lpdwSizeHigh);
}

BOOL RemoveDirectory(LPSTR lpszDirName)
{
	SYSCALL_PARAM_1(SYSCALL_REMOVEDIRECTORY,
		lpszDirName);
}

BOOL SetEndOfFile(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_SETENDOFFILE,hFile);
}

BOOL IOControl(HANDLE hFile,
			   DWORD dwCommand,
			   DWORD dwInputLen,
			   LPVOID lpInputBuffer,
			   DWORD dwOutputLen,
			   LPVOID lpOutputBuffer,
			   DWORD* lpdwFilled)
{
	SYSCALL_PARAM_7(SYSCALL_IOCONTROL,hFile,
		dwCommand,dwInputLen,
		lpInputBuffer,
		dwOutputLen,lpOutputBuffer,
		lpdwFilled);
}

BOOL SetFilePointer(HANDLE hFile,
					DWORD* lpdwDistLow,
					DWORD* lpdwDistHigh,
					DWORD dwMoveFlags)
{
	SYSCALL_PARAM_4(SYSCALL_SETFILEPOINTER,hFile,
		lpdwDistLow,
		lpdwDistHigh,
		dwMoveFlags);
}

BOOL FlushFileBuffers(HANDLE hFile)
{
	SYSCALL_PARAM_1(SYSCALL_FLUSHFILEBUFFERS,
		hFile);
}

HANDLE CreateDevice(LPSTR lpszDevName,
					DWORD dwAttributes,
					DWORD dwBlockSize,
					DWORD dwMaxReadSize,
					DWORD dwMaxWriteSize,
					LPVOID lpDevExtension,
					HANDLE hDrvObject)
{
	SYSCALL_PARAM_7(SYSCALL_CREATEDEVICE,
		lpszDevName,
		dwAttributes,
		dwBlockSize,
		dwMaxReadSize,
		dwMaxWriteSize,
		lpDevExtension,
		hDrvObject);
}

VOID DestroyDevice(HANDLE hDevice)
{
	SYSCALL_PARAM_1(SYSCALL_DESTROYDEVICE,hDevice);
}

LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType)
{
	SYSCALL_PARAM_2(SYSCALL_KMEMALLOC,
		dwSize,dwSizeType);
}

VOID KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength)
{
	SYSCALL_PARAM_3(SYSCALL_KMEMFREE,lpMemAddr,
		dwSizeType,
		dwMemLength);
}

VOID PrintLine(LPSTR lpszInfo)
{
	SYSCALL_PARAM_1(SYSCALL_PRINTLINE,
		lpszInfo);
}

VOID PrintChar(WORD ch)
{
	DWORD dwCh = (DWORD)ch;
	SYSCALL_PARAM_1(SYSCALL_PRINTCHAR,
		dwCh);
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
	SYSCALL_PARAM_1(SYSCALL_LOADDRIVER,
		de);
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
	SYSCALL_PARAM_1(SYSCALL_SETFOCUSTHREAD,
		hNewThread);
}

VOID GetSystemTime(BYTE* time)
{
	SYSCALL_PARAM_1(SYSCALL_GETSYSTEMTIME,
		time);
}

VOID GotoHome()
{
	SYSCALL_PARAM_0(SYSCALL_GOTOHOME);
}

VOID ChangeLine()
{
	SYSCALL_PARAM_0(SYSCALL_CHANGELINE);
}

VOID GotoPrev()
{
	SYSCALL_PARAM_0(SYSCALL_GOTOPREV);
}

VOID GetCursorPos(WORD* x, WORD* y)
{
	SYSCALL_PARAM_2(SYSCALL_GETCURSORPOS,
		x, y);
}

VOID SetCursorPos(WORD x, WORD y)
{
	SYSCALL_PARAM_2(SYSCALL_SETCURSORPOS,
		x, y);
}

VOID TerminateKernelThread(HANDLE hThread,int status)
{
	SYSCALL_PARAM_2(SYSCALL_TERMINATEKERNELTHREAD,
		hThread,
		status);
}

//
//The following routine prints out bug's information.
//
VOID __BUG(LPSTR lpszFileName, DWORD dwLineNum)
{
	DWORD   dwFlags;

	//Print out fatal error information.
	_hx_printf("\r\nBUG oencountered.\r\nFile name: %s\r\nCode Lines:%d", lpszFileName, dwLineNum);

	//Enter infinite loop.
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	while (TRUE);
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
}
