//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apr 14, 2019
//    Module Name               : hellox.c
//    Module Funciton           : 
//                                Source code for HelloX's API.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "hellox.h"

/* Macros to simplify the programming. */
#define __PARAM_0 edi
#define __PARAM_1 esi
#define __PARAM_2 edx
#define __PARAM_3 ecx
#define __PARAM_4 ebx

/* 
 * User thread wrapper routine,as the unified 
 * entry point of any user thread,ExitThread will
 * be invoked at the end of this routine to return
 * to kernel mode.
 */
static unsigned long __UserThreadWrapper(LPVOID pData)
{
	/* Invoke the user specified routine. */

	/* Return to OS kernel. */
	ExitThread(0);
}

/* APIs to manipulate user thread. */
HANDLE CreateUserThread(
	DWORD dwStatus,
	DWORD dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	char* pszName);
VOID DestroyUserThread(HANDLE hThread);
DWORD SetLastError(DWORD dwNewError);
DWORD GetLastError(void);
DWORD GetThreadID(HANDLE hThread);
DWORD SetThreadPriority(HANDLE hThread, DWORD dwPriority);
HANDLE GetCurrentThread(void);

/* Message operation. */
BOOL GetMessage(MSG* pMsg)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, pMsg
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_GETMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return bRet;
}

BOOL PeekMessage(MSG* pMsg)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, pMsg
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_PEEKMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return bRet;
}

BOOL SendMessage(HANDLE hThread, MSG* pMsg)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	__asm {
		push __PARAM_0
		push __PARAM_1
		push ebp
		push eax
		mov __PARAM_0, hThread
		mov __PARAM_1, pMsg
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_SENDMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_1
		pop __PARAM_0
	}
	return bRet;
}

/* Sleep a while. */
BOOL Sleep(DWORD dwMillionSecond)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;

	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, dwMillionSecond
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_SLEEP
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
}

HANDLE SetTimer(DWORD dwTimerID,
	DWORD dwMillionSecond,
	LPVOID lpHandlerParam,
	DWORD dwTimerFlags);
VOID CancelTimer(HANDLE hTimer);
HANDLE CreateEvent(BOOL bInitialStatus);
VOID DestroyEvent(HANDLE hEvent);
DWORD SetEvent(HANDLE hEvent);
DWORD ResetEvent(HANDLE hEvent);
HANDLE CreateMutex(void);
VOID DestroyMutex(HANDLE hMutex);
DWORD ReleaseMutex(HANDLE hEvent);
DWORD WaitForThisObject(HANDLE hObject);
DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond);

/* Allocate or free a virtual area from memory space. */
LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
	DWORD  dwSize,
	DWORD  dwAllocateFlags,
	DWORD  dwAccessFlags,
	CHAR*  lpszRegName)
{
	LPVOID pRet = NULL;
	LPVOID* pRetPtr = &pRet;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, lpDesiredAddr
		mov __PARAM_1, dwSize
		mov __PARAM_2, dwAllocateFlags
		mov __PARAM_3, dwAccessFlags
		mov __PARAM_4, lpszRegName
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_VIRTUALALLOC
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return pRet;
}

/* Release virtual memory allocated by VirtualAlloc. */
VOID VirtualFree(LPVOID lpVirtualAddr)
{
	__asm {
		mov __PARAM_0, lpVirtualAddr
		mov eax, SYSCALL_VIRTUALFREE
		int SYSCALL_VECTOR
	}
}

/* Routines to manipulate file or device. */
HANDLE CreateFile(LPSTR lpszFileName,
	DWORD dwAccessMode,
	DWORD dwShareMode,
	LPVOID lpReserved)
{
	HANDLE hRet = NULL;
	HANDLE* pRetHandle = &hRet;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push ebp
		push eax
		mov __PARAM_0, lpszFileName
		mov __PARAM_1, dwAccessMode
		mov __PARAM_2, dwShareMode
		mov __PARAM_3, lpReserved
		mov eax, pRetHandle
		mov ebp, eax
		mov eax, SYSCALL_CREATEFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return hRet;
}

BOOL ReadFile(HANDLE hFile,
	DWORD dwReadSize,
	LPVOID lpBuffer,
	DWORD* lpdwReadSize)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push ebp
		push eax
		mov __PARAM_0, hFile
		mov __PARAM_1, dwReadSize
		mov __PARAM_2, lpBuffer
		mov __PARAM_3, lpdwReadSize
		mov ebp, pRet
		mov eax, SYSCALL_READFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return bRet;
}

BOOL WriteFile(HANDLE hFile,
	DWORD dwWriteSize,
	LPVOID lpBuffer,
	DWORD* lpdwWrittenSize)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push ebp
		push eax
		mov __PARAM_0, hFile
		mov __PARAM_1, dwWriteSize
		mov __PARAM_2, lpBuffer
		mov __PARAM_3, lpdwWrittenSize
		mov ebp, pRet
		mov eax, SYSCALL_WRITEFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return bRet;
}

void CloseFile(HANDLE hFile)
{
	__asm {
		push __PARAM_0
		push eax
		mov __PARAM_0, hFile
		mov eax, SYSCALL_CLOSEFILE
		int SYSCALL_VECTOR
		pop eax
		pop __PARAM_0
	}
}

BOOL CreateDirectory(LPSTR lpszDirName);
BOOL DeleteFile(LPSTR lpszFileName);
HANDLE FindFirstFile(LPSTR lpszDirName,
	FS_FIND_DATA* pFindData);
BOOL FindNextFile(LPSTR lpszDirName,
	HANDLE hFindHandle,
	FS_FIND_DATA* pFindData);
VOID FindClose(LPSTR lpszDirName,
	HANDLE hFindHandle);

DWORD GetFileAttributes(LPSTR lpszFileName);

DWORD GetFileSize(HANDLE hFile, DWORD* lpdwSizeHigh)
{
	DWORD dwFileSize = 0;
	DWORD* pdwFileSize = &dwFileSize;

	if (NULL == hFile)
	{
		goto __TERMINAL;
	}

	__asm {
		push __PARAM_0
		push __PARAM_1
		push ebp
		push eax
		mov __PARAM_0, hFile
		mov __PARAM_1, lpdwSizeHigh
		mov eax, pdwFileSize
		mov ebp, eax
		mov eax, SYSCALL_GETFILESIZE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_1
		pop __PARAM_0
	}

__TERMINAL:
	return dwFileSize;
}

BOOL RemoveDirectory(LPSTR lpszDirName);
BOOL SetEndOfFile(HANDLE hFile);
BOOL IOControl(HANDLE hFile,
	DWORD dwCommand,
	DWORD dwInputLen,
	LPVOID lpInputBuffer,
	DWORD dwOutputLen,
	LPVOID lpOutputBuffer,
	DWORD* lpdwFilled);

/* Seek file's current pointer position. */
BOOL SetFilePointer(HANDLE hFile,
	DWORD* lpdwDistLow,
	DWORD* lpdwDistHigh,
	DWORD dwMoveFlags)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	
	if (NULL == hFile)
	{
		goto __TERMINAL;
	}

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push ebp
		push eax
		mov __PARAM_0, hFile
		mov __PARAM_1, lpdwDistLow
		mov __PARAM_2, lpdwDistHigh
		mov __PARAM_3, dwMoveFlags
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_SETFILEPOINTER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}

__TERMINAL:
	return bRet;
}

BOOL FlushFileBuffers(HANDLE hFile)
{
	return FALSE;
}

/* Exit a thread. */
void ExitThread(int errCode)
{
	__asm {
		mov eax, SYSCALL_PROCESSHALFBOTTOM
		int SYSCALL_VECTOR
	}
}

/* Print out a text line. */
void PrintLine(LPSTR pszInfo)
{
	__asm {
		mov __PARAM_0, pszInfo
		mov eax, SYSCALL_PRINTLINE
		int SYSCALL_VECTOR
	}
}

/* Print out a character. */
void PrintChar(char _char)
{
	/* While text and black background. */
	unsigned long lchar = 0x0700;
	lchar += _char;
	__asm {
		mov __PARAM_0, lchar
		mov eax, SYSCALL_PRINTCHAR
		int SYSCALL_VECTOR
	}
}

/* Go to home position. */
void GotoHome()
{
	__asm {
		mov eax, SYSCALL_GOTOHOME
		int SYSCALL_VECTOR
	}
}

/* Change to next line. */
void ChangeLine()
{
	__asm {
		mov eax, SYSCALL_CHANGELINE
		int SYSCALL_VECTOR
	}
}

#undef __PARAM_0
#undef __PARAM_1
#undef __PARAM_2
#undef __PARAM_3
#undef __PARAM_4
