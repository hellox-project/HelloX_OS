//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apr 14, 2019
//    Module Name               : hellox.c
//    Module Funciton           : 
//                                Source code for HelloX's API.
//                                The AT&T style asm code is generated
//                                by AI according Intel style.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "stdio.h"

/*
 * Disable of showing out the C4731 warning.
 * Inline assembly is used in syscall's source
 * code, and EBP is modified but properly saved,
 * so supress the warning is clear.
 */
#ifdef _MSC_VER
#pragma warning(disable:4731)
#endif

/* Create a new user thread. */
HANDLE CreateUserThread(
	DWORD dwStatus,
	DWORD dwPriority,
	__THREAD_START_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	char* pszName)
{
	HANDLE hRet = NULL;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, dwStatus
		mov esi, dwPriority
		mov edx, lpStartRoutine
		mov ecx, lpRoutineParam
		mov ebx, pszName
		lea eax, hRet
		mov ebp, eax
		mov eax, SYSCALL_CREATEUSERTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"movl %4, %%ebx\n\t"
		"leal %5, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ebx\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(dwStatus), "+m"(dwPriority), "+m"(lpStartRoutine),
		"+m"(lpRoutineParam), "+m"(pszName), "+m"(hRet)
		: "i"(SYSCALL_CREATEUSERTHREAD), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ebx", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return hRet;
}

/*
 * Create a new process and returns the handle.
 * The handle must be closed by invoking CloseHandle
 * routine.
 */
HANDLE CreateProcess(
	unsigned long stack_size, /* Main thread's stack size, 0 as default. */
	unsigned long priority,   /* Main thread's priority level. */
	char* pszCmdLine,         /* User input command line. */
	LPVOID pReserved,         /* Reserved for future. */
	char* pszProcName         /* Process's name. */
) {
	HANDLE hRet = NULL;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, stack_size
		mov esi, priority
		mov edx, pszCmdLine
		mov ecx, pReserved
		mov ebx, pszProcName
		lea eax, hRet
		mov ebp, eax
		mov eax, SYSCALL_CREATEPROCESS
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
#endif
	return hRet;
}

/* Close a general handle. */
void CloseHandle(HANDLE _handle)
{
#ifdef _MSC_VER
	__asm {
		push edi
		mov edi, _handle
		mov eax, SYSCALL_CLOSEHANDLE
		int SYSCALL_VECTOR
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
#endif
}

/* Get a process's exit code. */
int GetProcessExitCode(HANDLE hProcess)
{
	int exit_code = -2;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hProcess
		lea eax, exit_code
		mov ebp, eax
		mov eax, SYSCALL_GETPROCESSEXITCODE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
#endif
	return exit_code;
}

/*
 * Get flash message from a given
 * process or current proccess.
 * hProcess is the process that flash
 * message get from, NULL for current
 * process.
 */
BOOL GetFlashMessage(HANDLE hProcess, int* flash_message_sz,
	LPVOID flash_message)
{
	BOOL bResult = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, hProcess
		mov esi, flash_message_sz
		mov edx, flash_message
		lea eax, bResult
		mov ebp, eax
		mov eax, SYSCALL_GETFLASHMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
#endif
	return bResult;
}

/*
 * Put flash message into a process or
 * current process, if the handle is NULL.
 */
BOOL PutFlashMessage(HANDLE hProcess, int flash_message_sz,
	LPVOID flash_message)
{
	BOOL bResult = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, hProcess
		mov esi, flash_message_sz
		mov edx, flash_message
		lea eax, bResult
		mov ebp, eax
		mov eax, SYSCALL_PUTFLASHMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
#endif
	return bResult;
}

VOID DestroyUserThread(HANDLE hThread)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hThread
		mov eax, SYSCALL_DESTROYUSERTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hThread), "i"(SYSCALL_DESTROYUSERTHREAD), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Resume a thread from suspended state. */
VOID ResumeThread(HANDLE thread_handle)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, thread_handle
		mov eax, SYSCALL_RESUMETHREAD
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(thread_handle), "i"(SYSCALL_RESUMETHREAD), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Set errcode of current thread. */
void SetLastError(DWORD dwNewError)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, dwNewError
		mov eax, SYSCALL_SETLASTERROR
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(dwNewError), "i"(SYSCALL_SETLASTERROR), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Get error code of current thread. */
DWORD GetLastError(void)
{
	unsigned long err_code = 0;

	/*
	 * emit asm instructions to query current thread's
	 * last error code from OS kernel.
	 */
#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, err_code
		mov ebp, eax
		mov eax, SYSCALL_GETLASTERROR
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(err_code)
		: "i"(SYSCALL_GETLASTERROR), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return err_code;
}

/* Get current thread's handle. */
HANDLE GetCurrentThread(void)
{
	HANDLE curr_handle = NULL;

	/*
	 * emit asm instructions to query current thread's
	 * HANDLE from OS kernel.
	 */
#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, curr_handle
		mov ebp, eax
		mov eax, SYSCALL_GETCURRENTTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(curr_handle)
		: "i"(SYSCALL_GETCURRENTTHREAD), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return curr_handle;
}

/* Get current thread's ID. */
unsigned long GetCurrentThreadID()
{
	unsigned long thread_id = 0;

	/*
	 * emit asm instructions to query current thread's
	 * ID from OS kernel.
	 */
#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, thread_id
		mov ebp, eax
		mov eax, SYSCALL_GETCURRENTTHREADID
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(thread_id)
		: "i"(SYSCALL_GETCURRENTTHREADID), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return thread_id;
}

/* Terminates current kernel thread. */
unsigned long TerminateKernelThread(HANDLE hTarget, int status)
{
	unsigned long ret_val = 0;

	/*
	 * emit asm instructions to terminates the current
	 * thread.
	 */
#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hTarget
		mov esi, status
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_TERMINATEKERNELTHREAD
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hTarget), "+m"(status), "+m"(ret_val)
		: "i"(SYSCALL_TERMINATEKERNELTHREAD), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif

	return ret_val;
}

/* Message operation. */
BOOL GetMessage(MSG* pMsg)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pMsg
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(pMsg), "+m"(bRet)
		: "i"(SYSCALL_GETMESSAGE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

BOOL PeekMessage(MSG* pMsg)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pMsg
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_PEEKMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(pMsg), "+m"(bRet)
		: "i"(SYSCALL_PEEKMESSAGE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

BOOL SendMessage(HANDLE hThread, MSG* pMsg)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hThread
		mov esi, pMsg
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_SENDMESSAGE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hThread), "+m"(pMsg), "+m"(bRet)
		: "i"(SYSCALL_SENDMESSAGE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

/* Sleep a while. */
BOOL Sleep(DWORD dwMillionSecond)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, dwMillionSecond
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_SLEEP
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(dwMillionSecond), "+m"(bRet)
		: "i"(SYSCALL_SLEEP), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Set a timer for current thread. */
HANDLE SetTimer(DWORD dwTimerID,
	DWORD dwMillionSecond,
	DWORD dwTimerFlags)
{
	HANDLE ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, dwTimerID
		mov esi, dwMillionSecond
		mov edx, dwTimerFlags
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SETTIMER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"leal %3, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(dwTimerID), "+m"(dwMillionSecond), "+m"(dwTimerFlags), "+m"(ret_val)
		: "i"(SYSCALL_SETTIMER), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edx", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Cancel a timer object. */
VOID CancelTimer(HANDLE hTimer)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hTimer
		mov eax, SYSCALL_CANCELTIMER
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hTimer), "i"(SYSCALL_CANCELTIMER), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Event operations. */
HANDLE CreateEvent(BOOL bInitialStatus);
VOID DestroyEvent(HANDLE hEvent);
DWORD SetEvent(HANDLE hEvent);
DWORD ResetEvent(HANDLE hEvent);

/* Create a new mutex object and returns the handle. */
HANDLE CreateMutex(void)
{
	HANDLE mtx_obj = 0;

#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, mtx_obj
		mov ebp, eax
		mov eax, SYSCALL_CREATEMUTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(mtx_obj)
		: "i"(SYSCALL_CREATEMUTEX), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return mtx_obj;
}

/* Destroy mutex object. */
VOID DestroyMutex(HANDLE hMutex)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hMutex
		mov eax, SYSCALL_DESTROYMUTEX
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hMutex), "i"(SYSCALL_DESTROYMUTEX), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Release a mutex object. */
DWORD ReleaseMutex(HANDLE hMutex)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hMutex
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_RELEASEMUTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hMutex), "+m"(ret_val)
		: "i"(SYSCALL_RELEASEMUTEX), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Create semaphore object. */
HANDLE CreateSemaphore(int max_count, int curr_count)
{
	HANDLE ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, max_count
		mov esi, curr_count
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CREATESEMAPHORE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(max_count), "+m"(curr_count), "+m"(ret_val)
		: "i"(SYSCALL_CREATESEMAPHORE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Destroy semaphore object. */
VOID DestroySemaphore(HANDLE hSemaphore)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hSemaphore
		mov eax, SYSCALL_DESTROYSEMAPHORE
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hSemaphore), "i"(SYSCALL_DESTROYSEMAPHORE), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Release a semaphore object. */
DWORD ReleaseSemaphore(HANDLE hSemaphore)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hSemaphore
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_RELEASESEMAPHORE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hSemaphore), "+m"(ret_val)
		: "i"(SYSCALL_RELEASESEMAPHORE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Get semaphore object's current value. */
DWORD GetSemaphoreValue(HANDLE hSemaphore)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hSemaphore
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_GETSEMAPHOREVALUE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hSemaphore), "+m"(ret_val)
		: "i"(SYSCALL_GETSEMAPHOREVALUE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Infinity waiting for a kernel object. */
DWORD WaitForThisObject(HANDLE hObject)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hObject
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_WAITFORTHISOBJECT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hObject), "+m"(ret_val)
		: "i"(SYSCALL_WAITFORTHISOBJECT), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Timeout waiting for a kernel object. */
DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hObject
		mov esi, dwMillionSecond
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_WAITFORTHISOBJECTEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hObject), "+m"(dwMillionSecond), "+m"(ret_val)
		: "i"(SYSCALL_WAITFORTHISOBJECTEX), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Create a new condition object and returns the handle. */
HANDLE CreateCondition(void)
{
	HANDLE con_obj = 0;

#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, con_obj
		mov ebp, eax
		mov eax, SYSCALL_CREATECONDITION
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(con_obj)
		: "i"(SYSCALL_CREATECONDITION), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return con_obj;
}

/* 
 * Destroy a condition object.
 * Keep space here and could be replaced by CloseHandle. 
 */
VOID DestroyCondition(HANDLE hCondition)
{
#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hCondition
		mov eax, SYSCALL_CONDITIONDESTROY
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hCondition), "i"(SYSCALL_CONDITIONDESTROY), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Wait for a condition object. */
DWORD ConditionWait(HANDLE hCondition, HANDLE hMutex)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hCondition
		mov esi, hMutex
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CONDITIONWAIT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hCondition), "+m"(hMutex), "+m"(ret_val)
		: "i"(SYSCALL_CONDITIONWAIT), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/*
 * Wait condition in time out maner.
 */
DWORD ConditionTimeoutWait(HANDLE hCondition, HANDLE hMutex,
	unsigned long million_seconds)
{
	size_t ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, hCondition
		mov esi, hMutex
		mov edx, million_seconds
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CONDITIONTIMEOUTWAIT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"leal %3, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hCondition), "+m"(hMutex), "+m"(million_seconds), "+m"(ret_val)
		: "i"(SYSCALL_CONDITIONTIMEOUTWAIT), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edx", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Signal a condtion object. */
DWORD ConditionSignal(HANDLE hCondition)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hCondition
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CONDITIONSIGNAL
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hCondition), "+m"(ret_val)
		: "i"(SYSCALL_CONDITIONSIGNAL), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_val;
}

/* Broadcast condition object. */
DWORD ConditionBroadcast(HANDLE hCondition)
{
	unsigned long ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, hCondition
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CONDITIONBROADCAST
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(hCondition), "+m"(ret_val)
		: "i"(SYSCALL_CONDITIONBROADCAST), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
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

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, lpDesiredAddr
		mov esi, dwSize
		mov edx, dwAllocateFlags
		mov ecx, dwAccessFlags
		mov ebx, lpszRegName
		lea eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_VIRTUALALLOC
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"movl %4, %%ebx\n\t"
		"leal %5, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ebx\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpDesiredAddr), "+m"(dwSize), "+m"(dwAllocateFlags),
		"+m"(dwAccessFlags), "+m"(lpszRegName), "+m"(pRet)
		: "i"(SYSCALL_VIRTUALALLOC), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ebx", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return pRet;
}

/* Release virtual memory allocated by VirtualAlloc. */
BOOL VirtualFree(LPVOID lpVirtualAddr)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, lpVirtualAddr
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_VIRTUALFREE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(lpVirtualAddr), "+m"(bRet)
		: "i"(SYSCALL_VIRTUALFREE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Routines to manipulate file or device. */
HANDLE CreateFile(LPSTR lpszFileName,
	DWORD dwAccessMode,
	DWORD dwShareMode,
	LPVOID lpReserved)
{
	HANDLE hRet = NULL;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, lpszFileName
		mov esi, dwAccessMode
		mov edx, dwShareMode
		mov ecx, lpReserved
		lea eax, hRet
		mov ebp, eax
		mov eax, SYSCALL_CREATEFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"leal %4, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszFileName), "+m"(dwAccessMode), "+m"(dwShareMode),
		"+m"(lpReserved), "+m"(hRet)
		: "i"(SYSCALL_CREATEFILE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return hRet;
}

BOOL ReadFile(HANDLE hFile,
	DWORD dwReadSize,
	LPVOID lpBuffer,
	DWORD* lpdwReadSize)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, hFile
		mov esi, dwReadSize
		mov edx, lpBuffer
		mov ecx, lpdwReadSize
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_READFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"leal %4, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hFile), "+m"(dwReadSize), "+m"(lpBuffer),
		"+m"(lpdwReadSize), "+m"(bRet)
		: "i"(SYSCALL_READFILE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

BOOL WriteFile(HANDLE hFile,
	DWORD dwWriteSize,
	LPVOID lpBuffer,
	DWORD* lpdwWrittenSize)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, hFile
		mov esi, dwWriteSize
		mov edx, lpBuffer
		mov ecx, lpdwWrittenSize
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_WRITEFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"leal %4, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hFile), "+m"(dwWriteSize), "+m"(lpBuffer),
		"+m"(lpdwWrittenSize), "+m"(bRet)
		: "i"(SYSCALL_WRITEFILE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

void CloseFile(HANDLE hFile)
{
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push eax
		mov edi, hFile
		mov eax, SYSCALL_CLOSEFILE
		int SYSCALL_VECTOR
		pop eax
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%edi\n\t"
		:
	: "m"(hFile), "i"(SYSCALL_CLOSEFILE), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Create a new directory. */
BOOL CreateDirectory(LPCSTR lpszPathName,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	BOOL bRet = FALSE;

	if (NULL == lpszPathName)
	{
		return FALSE;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, lpszPathName
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_CREATEDIRECTORY
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszPathName), "+m"(bRet)
		: "i"(SYSCALL_CREATEDIRECTORY), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Delete a given file. */
BOOL DeleteFile(LPSTR lpszFileName)
{
	return FALSE;
}

/* Open a directory and begin to iterate it. */
HANDLE FindFirstFile(LPSTR lpszDirName,
	FS_FIND_DATA* pFindData)
{
	HANDLE hFindHandle = NULL;

	if ((NULL == lpszDirName) || (NULL == pFindData))
	{
		return NULL;
	}
#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, lpszDirName
		mov esi, pFindData
		lea eax, hFindHandle
		mov ebp, eax
		mov eax, SYSCALL_FINDFIRSTFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszDirName), "+m"(pFindData), "+m"(hFindHandle)
		: "i"(SYSCALL_FINDFIRSTFILE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif

	return hFindHandle;
}

BOOL FindNextFile(LPSTR lpszDirName,
	HANDLE hFindHandle,
	FS_FIND_DATA* pFindData)
{
	BOOL bRet = FALSE;

	if ((NULL == lpszDirName) ||
		(NULL == hFindHandle) ||
		(NULL == pFindData))
	{
		return FALSE;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, lpszDirName
		mov esi, hFindHandle
		mov edx, pFindData
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_FINDNEXTFILE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"leal %3, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszDirName), "+m"(hFindHandle), "+m"(pFindData), "+m"(bRet)
		: "i"(SYSCALL_FINDNEXTFILE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edx", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

VOID FindClose(LPSTR lpszDirName,
	HANDLE hFindHandle)
{
	if ((NULL == lpszDirName) ||
		(NULL == hFindHandle))
	{
		return;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, lpszDirName
		mov esi, hFindHandle
		mov eax, SYSCALL_FINDCLOSE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszDirName), "+m"(hFindHandle)
		: "i"(SYSCALL_FINDCLOSE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
}

DWORD GetFileAttributes(LPSTR lpszFileName);

BOOL GetFileAttributesEx(LPCSTR lpFileName,
	GET_FILEEX_INFO_LEVELS fInfoLevel,
	LPVOID lpFileInformation)
{
	BOOL bRet = FALSE;

	if ((NULL == lpFileName) ||
		(NULL == lpFileInformation))
	{
		return FALSE;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, lpFileName
		mov esi, fInfoLevel
		mov edx, lpFileInformation
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETFILEATTRIBUTESEX
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"leal %3, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(lpFileName), "+m"(fInfoLevel), "+m"(lpFileInformation), "+m"(bRet)
		: "i"(SYSCALL_GETFILEATTRIBUTESEX), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edx", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

BOOL GetFileInformationByHandle(HANDLE hFile,
	BY_HANDLE_FILE_INFORMATION* pFileInfo)
{
	BOOL bRet = FALSE;

	if ((NULL == hFile) || (NULL == pFileInfo))
	{
		return FALSE;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hFile
		mov esi, pFileInfo
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETFILEINFORMATIONBYHANDLE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hFile), "+m"(pFileInfo), "+m"(bRet)
		: "i"(SYSCALL_GETFILEINFORMATIONBYHANDLE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

DWORD GetFileSize(HANDLE hFile, DWORD* lpdwSizeHigh)
{
	DWORD dwFileSize = 0;

	if (NULL == hFile)
	{
		goto __TERMINAL;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, hFile
		mov esi, lpdwSizeHigh
		lea eax, dwFileSize
		mov ebp, eax
		mov eax, SYSCALL_GETFILESIZE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hFile), "+m"(lpdwSizeHigh), "+m"(dwFileSize)
		: "i"(SYSCALL_GETFILESIZE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif

__TERMINAL:
	return dwFileSize;
}

/* Remove a empty directory. */
BOOL RemoveDirectory(LPCSTR lpszDirName)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, lpszDirName
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_REMOVEDIRECTORY
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(lpszDirName), "+m"(bRet)
		: "i"(SYSCALL_REMOVEDIRECTORY), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Set current position as end of the file. */
BOOL SetEndOfFile(HANDLE hFile)
{
	return FALSE;
}

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

	if (NULL == hFile)
	{
		goto __TERMINAL;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, hFile
		mov esi, lpdwDistLow
		mov edx, lpdwDistHigh
		mov ecx, dwMoveFlags
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_SETFILEPOINTER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"movl %3, %%ecx\n\t"
		"leal %4, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(hFile), "+m"(lpdwDistLow), "+m"(lpdwDistHigh),
		"+m"(dwMoveFlags), "+m"(bRet)
		: "i"(SYSCALL_SETFILEPOINTER), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ecx", "edx", "esi", "edi", "memory"
		);
#endif

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
#ifdef _MSC_VER
	__asm {
		mov eax, SYSCALL_PROCESSHALFBOTTOM
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%eax\n\t"
		"int %1\n\t"
		:
	: "i"(SYSCALL_PROCESSHALFBOTTOM), "i"(SYSCALL_VECTOR)
		: "eax"
		);
#endif
}

/* Print out a text line. */
void PrintLine(LPSTR pszInfo)
{
#ifdef _MSC_VER
	__asm {
		mov edi, pszInfo
		mov eax, SYSCALL_PRINTLINE
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		:
	: "m"(pszInfo), "i"(SYSCALL_PRINTLINE), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Print out a character. */
void PrintChar(char _char)
{
	/* While text and black background. */
	unsigned long lchar = 0x0700;
	lchar += _char;
#ifdef _MSC_VER
	__asm {
		mov edi, lchar
		mov eax, SYSCALL_PRINTCHAR
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		:
	: "m"(lchar), "i"(SYSCALL_PRINTCHAR), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/* Go to home position. */
void GotoHome()
{
#ifdef _MSC_VER
	__asm {
		mov eax, SYSCALL_GOTOHOME
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%eax\n\t"
		"int %1\n\t"
		:
	: "i"(SYSCALL_GOTOHOME), "i"(SYSCALL_VECTOR)
		: "eax"
		);
#endif
}

/* Change to next line. */
void ChangeLine()
{
#ifdef _MSC_VER
	__asm {
		mov eax, SYSCALL_CHANGELINE
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%eax\n\t"
		"int %1\n\t"
		:
	: "i"(SYSCALL_CHANGELINE), "i"(SYSCALL_VECTOR)
		: "eax"
		);
#endif
}

/* Get system time information. */
void GetSystemTime(unsigned char* time_info)
{
#ifdef _MSC_VER
	__asm {
		mov edi, time_info
		mov eax, SYSCALL_GETSYSTEMTIME
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%edi\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		:
	: "m"(time_info), "i"(SYSCALL_GETSYSTEMTIME), "i"(SYSCALL_VECTOR)
		: "eax", "edi", "memory"
		);
#endif
}

/*
 * Get the system clock counter.
 * This routine returns the low 32 bits of
 * system clock counter, and the high 32 bits
 * will be set in *high if specified.
 */
unsigned long GetClockCounter(unsigned long* high_32)
{
	unsigned long _clock = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, high_32
		lea eax, _clock
		mov ebp, eax
		mov eax, SYSCALL_GETCLOCKCOUNTER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(high_32), "+m"(_clock)
		: "i"(SYSCALL_GETCLOCKCOUNTER), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif

	return _clock;
}

/* Get system information. */
BOOL GetSystemInfo(SYSTEM_INFO* pSysInfo)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pSysInfo
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETSYSTEMINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(pSysInfo), "+m"(bRet)
		: "i"(SYSCALL_GETSYSTEMINFO), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Get CPU information. */
BOOL GetCPUInfo(CPU_INFO* pCpuInfo)
{
	BOOL bRet = FALSE;

	/* Check structure. */
	if (pCpuInfo->cbSize != sizeof(CPU_INFO))
	{
		return FALSE;
	}

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pCpuInfo
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETCPUINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(pCpuInfo), "+m"(bRet)
		: "i"(SYSCALL_GETCPUINFO), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

/* Switch to graphic mode. */
LPVOID SwitchToGraphic(__VBE_INFO* pVbeInfo)
{
	LPVOID ret_addr = NULL;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pVbeInfo
		lea eax, ret_addr
		mov ebp, eax
		mov eax, SYSCALL_SWITCHTOGRAPHIC
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(pVbeInfo), "+m"(ret_addr)
		: "i"(SYSCALL_SWITCHTOGRAPHIC), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return ret_addr;
}

/* Switch to text mode. */
void SwitchToText()
{
#ifdef _MSC_VER
	__asm {
		mov eax, SYSCALL_SWITCHTOTEXT
		int SYSCALL_VECTOR
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"movl %0, %%eax\n\t"
		"int %1\n\t"
		:
	: "i"(SYSCALL_SWITCHTOTEXT), "i"(SYSCALL_VECTOR)
		: "eax"
		);
#endif
}

/* Query virtual memory's basic information. */
unsigned long VirtualQuery(LPVOID pStartAddr,
	MEMORY_BASIC_INFORMATION* pMemInfo,
	unsigned long info_sz)
{
	size_t ret_val = 0;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, pStartAddr
		mov esi, pMemInfo
		mov edx, info_sz
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_VIRTUALQUERY
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"movl %2, %%edx\n\t"
		"leal %3, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(pStartAddr), "+m"(pMemInfo), "+m"(info_sz), "+m"(ret_val)
		: "i"(SYSCALL_VIRTUALQUERY), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edx", "esi", "edi", "memory"
		);
#endif
	return ret_val;
}

/* TLS manipulation routines. */
DWORD TlsAlloc()
{
	unsigned long key = 0;

	/*
	 * emit asm instructions to query current thread's
	 * last error code from OS kernel.
	 */
#ifdef _MSC_VER
	__asm {
		push ebp
		push eax
		lea eax, key
		mov ebp, eax
		mov eax, SYSCALL_TLSALLOC
		int SYSCALL_VECTOR
		pop eax
		pop ebp
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %1, %%eax\n\t"
		"int %2\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		: "+m"(key)
		: "i"(SYSCALL_TLSALLOC), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "memory"
		);
#endif

	return key;
}

BOOL TlsFree(DWORD dwTlsIndex)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, dwTlsIndex
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_TLSFREE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(dwTlsIndex), "+m"(bRet)
		: "i"(SYSCALL_TLSFREE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return bRet;
}

LPVOID TlsGetValue(DWORD dwTlsIndex)
{
	LPVOID pRet = NULL;

#ifdef _MSC_VER
	__asm {
		push edi
		push ebp
		push eax
		mov edi, dwTlsIndex
		lea eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_TLSGETVALUE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%edi\n\t"
		: "+m"(dwTlsIndex), "+m"(pRet)
		: "i"(SYSCALL_TLSGETVALUE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "edi", "memory"
		);
#endif
	return pRet;
}

BOOL TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
	BOOL bRet = FALSE;

#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, dwTlsIndex
		mov esi, lpTlsValue
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_TLSSETVALUE
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%esi\n\t"
		"leal %2, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(dwTlsIndex), "+m"(lpTlsValue), "+m"(bRet)
		: "i"(SYSCALL_TLSSETVALUE), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "esi", "edi", "memory"
		);
#endif
	return bRet;
}

/*
 * Exclusively operations for user mode spinlock
 * and other purpose.
 */
long __InterlockedCompareExchange(long volatile* destination,
	long exchange,
	long comparand)
{
#ifdef _MSC_VER
	long result = 0;
	__asm {
		mov eax, comparand
		mov ecx, destination
		mov edx, exchange
		lock cmpxchg dword ptr[ecx], edx
		mov result, eax
	}
	return result;
#elif defined(__GNUC__) || defined(__clang__)
	long result;
	__asm__ volatile (
		"lock cmpxchgl %2, (%1)"
		: "=a"(result)
		: "r"(destination), "r"(exchange), "a"(comparand)
		: "memory"
		);
	return result;
#endif
}

long __InterlockedExchange(long volatile* destination, long exchange)
{
#ifdef _MSC_VER
	long result = 0;
	__asm {
		mov eax, exchange
		mov ecx, destination
		lock xchg dword ptr[ecx], eax
		mov result, eax
	}
	return result;
#elif defined(__GNUC__) || defined(__clang__)
    long result;
    __asm__ volatile (
        "xchgl %0, %1"
        : "=r" (result), "+m" (*destination)
        : "0" (exchange)
        : "memory"
    );
    return result;
#endif
}

/* Exclusively add and return the original value. */
unsigned long __InterlockedAdd(unsigned long volatile* destination,
	unsigned long added)
{
#ifdef _MSC_VER
	unsigned long result = 0;
	__asm {
		mov eax, added
		mov ecx, destination
		lock xadd dword ptr[ecx], eax
		mov result, eax
	}
	return result;
#elif defined(__GNUC__) || defined(__clang__)
	unsigned long old;
	__asm__ volatile (
		"lock xaddl %0, (%1)"
		: "+r"(added)
		: "r"(destination)
		: "memory"
		);
	old = added;
	return old;
#endif
}

/* Exclusively sub and return the original value. */
unsigned long __InterlockedSub(unsigned long volatile* destination,
	unsigned long subed)
{
#ifdef _MSC_VER
	unsigned long result = 0;
	__asm {
		mov eax, subed
		neg eax
		mov ecx, destination
		lock xadd dword ptr[ecx], eax
		mov result, eax
	}
	return result;
#elif defined(__GNUC__) || defined(__clang__)
	unsigned long old;
	unsigned long neg = (unsigned long)(0 - subed);
	__asm__ volatile (
		"lock xaddl %0, (%1)"
		: "+r"(neg)
		: "r"(destination)
		: "memory"
		);
	old = neg;
	return old;
#endif
}

/* Test the parameter xfering mechanism of system call. */
BOOL XferMoreParam(char* pszInfo0, char* pszInfo1, char* pszInfo2,
	int nInfo3, unsigned int nInfo4, char* pszInfo5, char* pszInfo6,
	char* pszInfo7)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	BOOL bRet = FALSE;

	/* Init extension param block. */
	ext_block.ext_param0 = (UINT32)pszInfo0;
	ext_block.ext_param1 = (UINT32)pszInfo1;
	ext_block.ext_param2 = (UINT32)pszInfo2;
	ext_block.ext_param3 = (UINT32)nInfo3;
	ext_block.ext_param4 = (UINT32)nInfo4;
	ext_block.ext_param5 = (UINT32)pszInfo5;
	ext_block.ext_param6 = (UINT32)pszInfo6;
	ext_block.ext_param7 = (UINT32)pszInfo7;

	/* Launch the system call. */
#ifdef _MSC_VER
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, 0
		mov esi, 0
		mov edx, 0
		mov ecx, 0
		mov ebx, pext_block
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_TESTPARAMXFER
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__GNUC__) || defined(__clang__)
	__asm__ volatile (
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebx\n\t"
		"push %%ebp\n\t"
		"push %%eax\n\t"
		"movl $0, %%edi\n\t"
		"movl $0, %%esi\n\t"
		"movl $0, %%edx\n\t"
		"movl $0, %%ecx\n\t"
		"movl %0, %%ebx\n\t"
		"leal %1, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"pop %%eax\n\t"
		"pop %%ebp\n\t"
		"pop %%ebx\n\t"
		"pop %%ecx\n\t"
		"pop %%edx\n\t"
		"pop %%esi\n\t"
		"pop %%edi\n\t"
		: "+m"(pext_block), "+m"(bRet)
		: "i"(SYSCALL_TESTPARAMXFER), "i"(SYSCALL_VECTOR)
		: "eax", "ebp", "ebx", "ecx", "edx", "esi", "edi", "memory"
		);
#endif
	return bRet;
}
