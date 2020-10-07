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
#include "stdio.h"

/* Macros to simplify the programming. */
#define __PARAM_0 edi
#define __PARAM_1 esi
#define __PARAM_2 edx
#define __PARAM_3 ecx
#define __PARAM_4 ebx

/* 
 * Local structure to pass parameters to user thread 
 * wrapper routine from CreateUserThread.
 * The UserThreadWrapper must be used as entry point
 * when a new user thread is created,in which the
 * user specified start routine point will be invoked.
 */
typedef struct tag__USER_THREAD_WRAPPER_PARAMBLOCK {
	__THREAD_START_ROUTINE pStartRoutine;
	LPVOID pRoutineParam;
}__USER_THREAD_WRAPPER_PARAMBLOCK;

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

/* Create a new user thread. */
HANDLE CreateUserThread(
	DWORD dwStatus,
	DWORD dwPriority,
	__THREAD_START_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	char* pszName)
{
	HANDLE hRet = NULL;
	HANDLE* pRetHandle = &hRet;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, dwStatus
		mov __PARAM_1, dwPriority
		mov __PARAM_2, lpStartRoutine
		mov __PARAM_3, lpRoutineParam
		mov __PARAM_4, pszName
		mov eax, pRetHandle
		mov ebp, eax
		mov eax, SYSCALL_CREATEUSERTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return hRet;
}

VOID DestroyUserThread(HANDLE hThread);
DWORD SetLastError(DWORD dwNewError);
DWORD GetLastError(void);
DWORD GetThreadID(HANDLE hThread);
DWORD SetThreadPriority(HANDLE hThread, DWORD dwPriority);
HANDLE GetCurrentThread(void);

/* Get current thread's ID. */
unsigned long GetCurrentThreadID()
{
	unsigned long thread_id = 0;
	unsigned long* pthread_id = &thread_id;

	/* 
	 * emit asm instructions to query current thread's 
	 * ID from OS kernel. 
	 */
	__asm {
		push ebp
		push eax
		mov eax, pthread_id
		mov ebp, eax
		mov eax, SYSCALL_GETCURRENTTHREADID
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}

	return thread_id;
}

/* Terminates current kernel thread. */
unsigned long TerminateKernelThread(HANDLE hTarget, int status)
{
	unsigned long ret_val = 0;
	unsigned long* pret_val = &ret_val;

	/*
	 * emit asm instructions to terminates the current
	 * thread.
	 */
	__asm {
		push __PARAM_0
		push __PARAM_1
		push ebp
		push eax
		mov __PARAM_0, hTarget
		mov __PARAM_1, status
		mov eax, pret_val
		mov ebp, eax
		mov eax, SYSCALL_TERMINATEKERNELTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_1
		pop __PARAM_0
	}

	return ret_val;
}

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
	return bRet;
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

/* Create a new mutex object and returns the handle. */
HANDLE CreateMutex(void)
{
	HANDLE mtx_obj = 0;
	HANDLE* pmtx_obj = &mtx_obj;

	__asm {
		push ebp
		push eax
		mov eax, pmtx_obj
		mov ebp, eax
		mov eax, SYSCALL_CREATEMUTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}

	return mtx_obj;
}

/* Keep space and will be replaced by CloseHandle routine. */
VOID DestroyMutex(HANDLE hMutex);

/* Release a mutex object. */
DWORD ReleaseMutex(HANDLE hEvent)
{
	unsigned long ret_val = 0;
	unsigned long* pret_val = &ret_val;

	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, hEvent
		mov eax, pret_val
		mov ebp, eax
		mov eax, SYSCALL_RELEASEMUTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return ret_val;
}

/* Infinity waiting for a kernel object. */
DWORD WaitForThisObject(HANDLE hObject)
{
	unsigned long ret_val = 0;
	unsigned long* pret_val = &ret_val;

	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, hObject
		mov eax, pret_val
		mov ebp, eax
		mov eax, SYSCALL_WAITFORTHISOBJECT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return ret_val;
}

/* Timeout waiting for a kernel object. */
DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond)
{
	unsigned long ret_val = 0;
	unsigned long* pret_val = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push ebp
		push eax
		mov __PARAM_0, hObject
		mov __PARAM_1, dwMillionSecond
		mov eax, pret_val
		mov ebp, eax
		mov eax, SYSCALL_WAITFORTHISOBJECTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

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
BOOL VirtualFree(LPVOID lpVirtualAddr)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;

	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, lpVirtualAddr
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_VIRTUALFREE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return bRet;
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

/* Get system information. */
BOOL GetSystemInfo(SYSTEM_INFO* pSysInfo)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, pSysInfo
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_GETSYSTEMINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return bRet;
}

/* Query virtual memory's basic information. */
size_t VirtualQuery(LPVOID pStartAddr,
	MEMORY_BASIC_INFORMATION* pMemInfo,
	size_t info_sz)
{
	size_t ret_val = 0;
	size_t* pRet = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push ebp
		push eax
		mov __PARAM_0, pStartAddr
		mov __PARAM_1, pMemInfo
		mov __PARAM_2, info_sz
		mov ebp, pRet
		mov eax, SYSCALL_VIRTUALQUERY
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/*
 * Exclusively operations for user mode spinlock
 * and other purpose.
 */
long __InterlockedCompareExchange(long volatile* destination,
	long exchange,
	long comparand)
{
	__asm {
		mov eax, comparand
		mov ecx, destination
		mov edx, exchange
		lock cmpxchg dword ptr[ecx], edx
	}
}

long __InterlockedExchange(long volatile* destination, long exchange)
{
	__asm {
		mov eax, exchange
		mov ecx, destination
		lock xchg dword ptr[ecx], eax
	}
}

/* Test the parameter xfering mechanism of system call. */
BOOL XferMoreParam(char* pszInfo0, char* pszInfo1, char* pszInfo2,
	int nInfo3, unsigned int nInfo4, char* pszInfo5, char* pszInfo6,
	char* pszInfo7)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	BOOL bRet = FALSE;
	BOOL* pRetPtr = &bRet;

	/* Init extension param block. */
	ext_block.ext_param0 = (uint32_t)pszInfo0;
	ext_block.ext_param1 = (uint32_t)pszInfo1;
	ext_block.ext_param2 = (uint32_t)pszInfo2;
	ext_block.ext_param3 = (uint32_t)nInfo3;
	ext_block.ext_param4 = (uint32_t)nInfo4;
	ext_block.ext_param5 = (uint32_t)pszInfo5;
	ext_block.ext_param6 = (uint32_t)pszInfo6;
	ext_block.ext_param7 = (uint32_t)pszInfo7;

	/* Launch the system call. */
	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, 0
		mov __PARAM_1, 0
		mov __PARAM_2, 0
		mov __PARAM_3, 0
		mov __PARAM_4, pext_block
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_TESTPARAMXFER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return bRet;
}

#undef __PARAM_0
#undef __PARAM_1
#undef __PARAM_2
#undef __PARAM_3
#undef __PARAM_4
