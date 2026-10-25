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
#include "../arch/x86/vesa.h"
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

/* Create a new user thread. */
static void SC_CreateUserThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* pOwnProcess = __CURRENT_PROCESS;
	unsigned long init_state = 0;
	__COMMON_OBJECT* pUserThread = NULL;
	__HANDLE thread_handle = INVALID_HANDLE_VALUE;

	BUG_ON(NULL == pOwnProcess);

	/* Save and validate the requested initial state of the thread. */
	init_state = (unsigned long)PARAM(0);
	if ((KERNEL_THREAD_STATUS_SUSPENDED != init_state) &&
		(KERNEL_THREAD_STATUS_READY != init_state))
	{
		goto __TERMINAL;
	}

	/* Distribute the user thread to proper CPU. */
	unsigned int affinity = 0;
#if defined(__CFG_SYS_SMP)
	affinity = ProcessorManager.GetScheduleCPU();
#endif

	/* Create the user thread. */
	pUserThread = (__COMMON_OBJECT*)KernelThreadManager.CreateUserThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0, /* Use default stack size. */
		KERNEL_THREAD_STATUS_SUSPENDED, /* Suspend it at first. */
		(uint32_t)PARAM(1), /* Priority level. */
		(__KERNEL_THREAD_ROUTINE)PARAM(2),
		(LPVOID)PARAM(3),
		pOwnProcess,
		(LPSTR)PARAM(4)
	);
	if (NULL == pUserThread)
	{
		goto __TERMINAL;
	}

	/* Save the thread object into own process's handle slot. */
	thread_handle = ProcessManager.GetHandle(pOwnProcess, pUserThread);
	if (INVALID_HANDLE_VALUE == thread_handle)
	{
		/* Exception case. */
		KernelThreadManager.TerminateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pUserThread, 0);
		goto __TERMINAL;
	}

	/* Change it's default affinity. */
	KernelThreadManager.ChangeAffinity(pUserThread, affinity);
	/* Resume it if necessary. */
	if (KERNEL_THREAD_STATUS_READY == init_state)
	{
		KernelThreadManager.ResumeKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			pUserThread);
	}

__TERMINAL:
	SYSCALL_RET = (uint32_t)thread_handle;
	return;
}

/*
 * Create a new process.
 * A handle will be returned to the caller process
 * and the caller process should close handle explictly.
 */
static void SC_CreateProcess(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* proc_obj = NULL;
	__PROCESS_OBJECT* curr_proc = __CURRENT_PROCESS;
	__HANDLE hProc = INVALID_HANDLE_VALUE;

	/* Must be invoked in process context. */
	BUG_ON(NULL == curr_proc);

	/* Create process object. */
	proc_obj = ProcessManager.CreateProcess(
		(__COMMON_OBJECT*)&ProcessManager,
		PARAM(0), /* Main thread stack size. */
		PARAM(1), /* Main thread priority level. */
		(char*)PARAM(2), /* Command line. */
		NULL,     /* command obj, only for kernel. */
		(LPVOID)PARAM(3),     /* lpReserved. */
		(char*)PARAM(4)       /* Process name. */
	);
	if (NULL == proc_obj)
	{
		goto __TERMINAL;
	}

	/* Get a free handle slot for the process object. */
	hProc = ProcessManager.GetHandle(curr_proc,
		(__COMMON_OBJECT*)proc_obj);
	if (INVALID_HANDLE_VALUE == hProc)
	{
		/*
		 * TODO: Should add exception processing here,
		 * just to destroy or terminate the process.
		 */
		goto __TERMINAL;
	}

	/* Increase the ref-counter, since handle refer it. */
	ObjectManager.AddRefCount(&ObjectManager,
		(__COMMON_OBJECT*)proc_obj);

__TERMINAL:
	SYSCALL_RET = (uint32_t)hProc;
	return;
}

/* 
 * Destroy a kernel thread object. 
 * Just delegates this operation to CloseHandle.
 * This routine is only used to destroy a kernel
 * thread object.
 * It's has BUG now since kernel thread is not
 * bound to process, adjust it later.
 */
static void SC_DestroyKernelThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE thread_handle = (__HANDLE)PARAM(0);
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);
	if (INVALID_HANDLE_VALUE == thread_handle)
	{
		return;
	}
	ProcessManager.CloseHandle(pProcess, thread_handle);
}

/*
 * Destroy a user thread object.
 * Just delegates this operation to CloseHandle.
 */
static void SC_DestroyUserThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE thread_handle = (__HANDLE)PARAM(0);
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);
	if (INVALID_HANDLE_VALUE == thread_handle)
	{
		return;
	}
	ProcessManager.CloseHandle(pProcess, thread_handle);
}

/* Resume a thread from suspended status. */
static void SC_ResumeThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* kernel_thread_obj = NULL;
	__HANDLE thread_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* curr_proc = __CURRENT_PROCESS;

	BUG_ON(NULL == curr_proc);
	thread_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == thread_handle)
	{
		goto __TERMINAL;
	}

	/* Retrieve the corresponding kernel thread object. */
	kernel_thread_obj = ProcessManager.GetObjectByHandle(
		curr_proc, thread_handle);
	if (NULL == kernel_thread_obj)
	{
		goto __TERMINAL;
	}

	/* Resume it. */
	KernelThreadManager.ResumeKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		kernel_thread_obj);

__TERMINAL:
	return;
}

/* Get system clock counter. */
static void SC_GetClockCounter(__SYSCALL_PARAM_BLOCK* pspb)
{
	/* Set the high 32 bits. */
	if (PARAM(0))
	{
		*((unsigned long*)PARAM(0)) = 0;
	}
	/* Set the low 32 bits. */
	SYSCALL_RET = System.GetClockTickCounter((__COMMON_OBJECT*)&System);
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
	SYSCALL_RET = (uint32_t)SetThreadPriority((HANDLE)PARAM(0), (DWORD)PARAM(1));
}

static void SC_GetMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetMessage((MSG*)PARAM(0));
}

static void SC_SendMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SendMessage((HANDLE)PARAM(0), (MSG*)PARAM(1));
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
		(DWORD)PARAM(0), (DWORD)PARAM(1), NULL, NULL,
		(DWORD)PARAM(2));
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

/* Create a mutex object and return it's handle. */
static void SC_CreateMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pMutex = NULL;
	__HANDLE mtx_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	/* Create mutex object as kernel object. */
	pMutex = CreateMutex();
	if (pMutex)
	{
		mtx_handle = ProcessManager.GetHandle(pProcess, pMutex);
		if (INVALID_HANDLE_VALUE == mtx_handle)
		{
			/* Exception case,destroy the mutex object and write a log. */
			DestroyMutex(pMutex);
			goto __TERMINAL;
		}
	}

__TERMINAL:
	/* Return the handle of mutex object. */
	SYSCALL_RET = (uint32_t)mtx_handle;
}

/* 
 * Destroy a mutex object denoted by it's handle. 
 * Just delegates the operation to CloseHandle routine.
 */
static void SC_DestroyMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE mtx_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	mtx_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == mtx_handle)
	{
		goto __TERMINAL;
	}
	ProcessManager.CloseHandle(pProcess, mtx_handle);

__TERMINAL:
	return;
}

/* Release a mutex object. */
static void SC_ReleaseMutex(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pMutex = NULL;
	__HANDLE mtx_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long rel_result = 0;

	BUG_ON(NULL == pProcess);
	mtx_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == mtx_handle)
	{
		goto __TERMINAL;
	}
	/* Get the corresponding mutex object. */
	pMutex = ProcessManager.GetObjectByHandle(pProcess, mtx_handle);
	if (NULL == pMutex)
	{
		goto __TERMINAL;
	}
	/* Just destroy it. */
	rel_result = (uint32_t)ReleaseMutex(pMutex);

__TERMINAL:
	SYSCALL_RET = rel_result;
	return;
}

/* Create a semaphore object and return it's handle. */
static void SC_CreateSemaphore(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pSemaphore = NULL;
	__HANDLE sem_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	/* Create semaphore object as kernel object. */
	pSemaphore = CreateSemaphore((int)PARAM(0), (int)PARAM(1));
	if (pSemaphore)
	{
		sem_handle = ProcessManager.GetHandle(pProcess, pSemaphore);
		if (INVALID_HANDLE_VALUE == sem_handle)
		{
			/* Exception case,destroy the semaphore object and write a log. */
			DestroySemaphore(pSemaphore);
			goto __TERMINAL;
		}
	}

__TERMINAL:
	/* Return the handle of semaphore object. */
	SYSCALL_RET = (uint32_t)sem_handle;
}

/*
 * Destroy a semaphore object denoted by it's handle.
 * Just delegates the operation to CloseHandle routine.
 */
static void SC_DestroySemaphore(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE sem_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	sem_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == sem_handle)
	{
		goto __TERMINAL;
	}
	ProcessManager.CloseHandle(pProcess, sem_handle);

__TERMINAL:
	return;
}

/* Release a semaphore object. */
static void SC_ReleaseSemaphore(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pSemaphore = NULL;
	__HANDLE sem_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long rel_result = 0;

	BUG_ON(NULL == pProcess);
	sem_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == sem_handle)
	{
		goto __TERMINAL;
	}
	/* Get the corresponding mutex object. */
	pSemaphore = ProcessManager.GetObjectByHandle(pProcess, sem_handle);
	if (NULL == pSemaphore)
	{
		goto __TERMINAL;
	}
	/* 
	 * Just destroy it. 
	 * Launch a reschedule after release, since the release
	 * operation is from user and only interrupt context is
	 * required to disable reschedule.
	 */
	rel_result = (uint32_t)ReleaseSemaphore(pSemaphore, TRUE);

__TERMINAL:
	SYSCALL_RET = rel_result;
	return;
}

/* Get semaphore object's current value. */
static void SC_GetSemaphoreValue(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pSemaphore = NULL;
	__HANDLE sem_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long curr_val = 0;

	BUG_ON(NULL == pProcess);
	sem_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == sem_handle)
	{
		goto __TERMINAL;
	}
	/* Get the corresponding mutex object. */
	pSemaphore = ProcessManager.GetObjectByHandle(pProcess, sem_handle);
	if (NULL == pSemaphore)
	{
		goto __TERMINAL;
	}

	curr_val = ((__SEMAPHORE*)pSemaphore)->GetSemaphoreValue(pSemaphore);

__TERMINAL:
	SYSCALL_RET = curr_val;
	return;
}

/* Create a condition object and return it's handle. */
static void SC_CreateCondition(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pCond = NULL;
	__HANDLE cond_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	/* Create condition object as kernel object. */
	pCond = CreateCondition(0);
	if (pCond)
	{
		cond_handle = ProcessManager.GetHandle(pProcess, pCond);
		if (INVALID_HANDLE_VALUE == cond_handle)
		{
			/* Destroy the condition object and write a log. */
			DestroyCondition(pCond);
			goto __TERMINAL;
		}
	}

__TERMINAL:
	/* Return the handle of condition object. */
	SYSCALL_RET = (uint32_t)cond_handle;
}

/*
 * Destroy a condition object denoted by it's handle.
 * Just delegates the operation to CloseHandle routine.
 */
static void SC_DestroyCondition(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE cond_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	cond_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == cond_handle)
	{
		goto __TERMINAL;
	}
	ProcessManager.CloseHandle(pProcess, cond_handle);

__TERMINAL:
	return;
}

/* Wait condition object. */
static void SC_ConditionWait(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pCondition = NULL;
	__COMMON_OBJECT* pMutex = NULL;
	__HANDLE cond_handle = INVALID_HANDLE_VALUE, mtx_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long wait_result = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == pProcess);

	/* Get handle values. */
	cond_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == cond_handle)
	{
		goto __TERMINAL;
	}
	mtx_handle = (__HANDLE)PARAM(1);
	if (INVALID_HANDLE_VALUE == mtx_handle)
	{
		goto __TERMINAL;
	}

	/* Get the corresponding kernel objects. */
	pCondition = ProcessManager.GetObjectByHandle(pProcess, cond_handle);
	if (NULL == pCondition)
	{
		goto __TERMINAL;
	}
	pMutex = ProcessManager.GetObjectByHandle(pProcess, mtx_handle);
	if (NULL == pMutex)
	{
		goto __TERMINAL;
	}

	/* Now just invoke condwait routine. */
	wait_result = ((__CONDITION*)pCondition)->CondWait(pCondition, pMutex);

__TERMINAL:
	SYSCALL_RET = wait_result;
	return;
}

/* Time out wait condition object. */
static void SC_ConditionTimeoutWait(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pCondition = NULL;
	__COMMON_OBJECT* pMutex = NULL;
	unsigned long million_seconds = -1;
	__HANDLE cond_handle = INVALID_HANDLE_VALUE, mtx_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long wait_result = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == pProcess);

	/* Get handle values. */
	cond_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == cond_handle)
	{
		goto __TERMINAL;
	}
	mtx_handle = (__HANDLE)PARAM(1);
	if (INVALID_HANDLE_VALUE == mtx_handle)
	{
		goto __TERMINAL;
	}

	million_seconds = (unsigned long)PARAM(2);
	if (million_seconds == -1)
	{
		/* Use cond_wait instead. */
		goto __TERMINAL;
	}

	/* Get the corresponding kernel objects. */
	pCondition = ProcessManager.GetObjectByHandle(pProcess, cond_handle);
	if (NULL == pCondition)
	{
		goto __TERMINAL;
	}
	pMutex = ProcessManager.GetObjectByHandle(pProcess, mtx_handle);
	if (NULL == pMutex)
	{
		goto __TERMINAL;
	}

	/* Now just invoke condwait routine. */
	wait_result = ((__CONDITION*)pCondition)->CondWaitTimeout(pCondition,
		pMutex, million_seconds);

__TERMINAL:
	SYSCALL_RET = wait_result;
	return;
}

/* Signal condition object. */
static void SC_ConditionSignal(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pCondition = NULL;
	__HANDLE cond_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long signal_result = -1;

	BUG_ON(NULL == pProcess);

	/* Get handle values. */
	cond_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == cond_handle)
	{
		goto __TERMINAL;
	}

	/* Get the corresponding kernel objects. */
	pCondition = ProcessManager.GetObjectByHandle(pProcess, cond_handle);
	if (NULL == pCondition)
	{
		goto __TERMINAL;
	}

	/* Now just invoke cond_signal routine. */
	signal_result = ((__CONDITION*)pCondition)->CondSignal(pCondition);

__TERMINAL:
	SYSCALL_RET = signal_result;
	return;
}

/* Broadcast condition object. */
static void SC_ConditionBroadcast(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pCondition = NULL;
	__HANDLE cond_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long bcast_result = -1;

	BUG_ON(NULL == pProcess);

	/* Get handle values. */
	cond_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == cond_handle)
	{
		goto __TERMINAL;
	}

	/* Get the corresponding kernel objects. */
	pCondition = ProcessManager.GetObjectByHandle(pProcess, cond_handle);
	if (NULL == pCondition)
	{
		goto __TERMINAL;
	}

	/* Now just invoke cond_signal routine. */
	bcast_result = ((__CONDITION*)pCondition)->CondBroadcast(pCondition);

__TERMINAL:
	SYSCALL_RET = bcast_result;
	return;
}

/* Wait for a kernel object infinitely. */
static void SC_WaitForThisObject(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pObject = NULL;
	__HANDLE obj_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long wait_result = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == pProcess);
	obj_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == obj_handle)
	{
		goto __TERMINAL;
	}
	/* Get the corresponding kernel object. */
	pObject = ProcessManager.GetObjectByHandle(pProcess, obj_handle);
	if (NULL == pObject)
	{
		goto __TERMINAL;
	}

	/* Now just wait it. */
	wait_result = WaitForThisObject(pObject);

__TERMINAL:
	SYSCALL_RET = wait_result;
	return;
}

/* Timeout wait for a kernel object. */
static void SC_WaitForThisObjectEx(__SYSCALL_PARAM_BLOCK* pspb)
{
	__COMMON_OBJECT* pObject = NULL;
	__HANDLE obj_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	unsigned long wait_result = OBJECT_WAIT_FAILED;

	BUG_ON(NULL == pProcess);
	obj_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == obj_handle)
	{
		goto __TERMINAL;
	}
	/* Get the corresponding kernel object. */
	pObject = ProcessManager.GetObjectByHandle(pProcess, obj_handle);
	if (NULL == pObject)
	{
		goto __TERMINAL;
	}

	/* Now just wait it. */
	wait_result = WaitForThisObjectEx(pObject, PARAM(1), NULL);

__TERMINAL:
	SYSCALL_RET = wait_result;
	return;
}

static void SC_ConnectInterrupt(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ConnectInterrupt(
		"int_user",
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
	unsigned long ulAllocFlags = 0, permitted = 0;
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	/* 
	 * Check allocat flags, only user space
	 * is permitted since. 
	 */
	permitted = VIRTUAL_AREA_ALLOCATE_USERSPACE |
		VIRTUAL_AREA_ALLOCATE_STACK |
		VIRTUAL_AREA_ALLOCATE_USERAPP |
		VIRTUAL_AREA_ALLOCATE_USERHEAP |
		VIRTUAL_AREA_ALLOCATE_USERLIB,
	ulAllocFlags = (unsigned long)PARAM(2);
	if ((ulAllocFlags & permitted) == 0)
	{
		/* 
		 * Allocate flags not specified, use
		 * user space as default.
		 */
		ulAllocFlags |= VIRTUAL_AREA_ALLOCATE_USERSPACE;
	}
	//ulAllocFlags &= (~VIRTUAL_AREA_ALLOCATE_SPACE_MASK);
	//ulAllocFlags |= VIRTUAL_AREA_ALLOCATE_USERSPACE;

	SYSCALL_RET = (uint32_t)pVmmMgr->VirtualAlloc(
		(__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)PARAM(0),
		(DWORD)PARAM(1),
		ulAllocFlags,
		(DWORD)PARAM(3),
		(UCHAR*)PARAM(4),
		NULL);
}

static void SC_VirtualQuery(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	unsigned long ulAllocFlags = 0;
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	SYSCALL_RET = (uint32_t)pVmmMgr->VirtualQuery(
		(__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)PARAM(0),
		(__MEMORY_BASIC_INFORMATION*)PARAM(1),
		(size_t)PARAM(2));
}

static void SC_VirtualFree(__SYSCALL_PARAM_BLOCK* pspb)
{
	__PROCESS_OBJECT* pProcess = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;

	BUG_ON(NULL == pProcess);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	SYSCALL_RET = pVmmMgr->VirtualFree((__COMMON_OBJECT*)pVmmMgr,
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

/* Register a new system call into kernel. */
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
	SYSCALL_RET = (uint32_t)IOManager.LoadDriver((__DRIVER_ENTRY)PARAM(0));
#endif
}

static void SC_GetCurrentThread(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)__CURRENT_KERNEL_THREAD;
}

/* Return current thread's ID. */
static void SC_GetCurrentThreadID(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (unsigned long)__CURRENT_KERNEL_THREAD->dwThreadID;
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
		SYSCALL_RET = (uint32_t)SwitchToGraphic((__VBE_INFO*)PARAM(0));
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
	__HANDLE thread_handle = (__HANDLE)PARAM(0);
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;
	__COMMON_OBJECT* pThread = NULL;
	unsigned long result = 0;

	BUG_ON(NULL == pProcess);
	if (INVALID_HANDLE_VALUE == thread_handle)
	{
		/* Terminate current thread if no thread specified. */
		pThread = (__COMMON_OBJECT*)__CURRENT_KERNEL_THREAD;
	}
	else {
		pThread = ProcessManager.GetObjectByHandle(pProcess, thread_handle);
		if (NULL == pThread)
		{
			/* Thread is not exist, giveup. */
			goto __TERMINAL;
		}
	}
	/* Just terminates the thread. */
	SYSCALL_RET = KernelThreadManager.TerminateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		pThread,
		(DWORD)PARAM(1));

	/* Should not call CloseHandle... */

__TERMINAL:
	SYSCALL_RET = (uint32_t)result;
	return;
}

/* Call half bottom part of a process. */
static void SC_ProcessHalfBottom(__SYSCALL_PARAM_BLOCK* pspb)
{
	ProcessManager.ProcessHalfBottom((int)PARAM(0));
}

/* Allocate a TLS slot. */
static void SC_TLSAlloc(__SYSCALL_PARAM_BLOCK* pspb)
{
	unsigned long key = 0;
	if (ProcessManager.GetTLSKey((__COMMON_OBJECT*)&ProcessManager,
		&key, NULL))
	{
		SYSCALL_RET = key;
	}
	else
	{
		SYSCALL_RET = -1;
	}
}

/* Get TLS slot's value. */
static void SC_TLSGetValue(__SYSCALL_PARAM_BLOCK* pspb)
{
	unsigned long key = PARAM(0);
	void* value = NULL;

	if (ProcessManager.GetTLSValue((__COMMON_OBJECT*)&ProcessManager,
		key, &value))
	{
		SYSCALL_RET = (uint32_t)value;
		return;
	}
	SYSCALL_RET = 0;
	return;
}

/* Set a TLS slot's value. */
static void SC_TLSSetValue(__SYSCALL_PARAM_BLOCK* pspb)
{
	unsigned long key = PARAM(0);
	void* value = (void*)PARAM(1);

	if (ProcessManager.SetTLSValue((__COMMON_OBJECT*)&ProcessManager,
		key, value))
	{
		SYSCALL_RET = TRUE;
		return;
	}
	SYSCALL_RET = FALSE;
	return;
}

/* Release a TLS slot. */
static void SC_TLSFree(__SYSCALL_PARAM_BLOCK* pspb)
{
	unsigned long key = PARAM(0);

	if (key > MAX_TLS_NUM)
	{
		SYSCALL_RET = FALSE;
		return;
	}

	ProcessManager.ReleaseTLSKey((__COMMON_OBJECT*)&ProcessManager, key, NULL);
	SYSCALL_RET = TRUE;
	return;
}

/* Get system level information. */
static void SC_GetSystemInfo(__SYSCALL_PARAM_BLOCK* pspb)
{
	__SYSTEM_INFO* pSysInfo = NULL;
	BOOL bResult = FALSE;
	
	/* Map user memory to kernel. */
	pSysInfo = (__SYSTEM_INFO*)map_to_kernel((void*)PARAM(0), sizeof(__SYSTEM_INFO), __OUT);
	if (NULL == pSysInfo)
	{
		goto __TERMINAL;
	}

	bResult = System.GetSystemInfo(pSysInfo);
	if (bResult)
	{
		/* Map back to user. */
		map_to_user((void*)PARAM(0), sizeof(__SYSTEM_INFO), __OUT, pSysInfo);
	}

__TERMINAL:
	SYSCALL_RET = bResult;
}

/* Get current CPU information. */
static void SC_GetCPUInfo(__SYSCALL_PARAM_BLOCK* pspb)
{
	__CPU_INFO* pCpuInfo = NULL;
	BOOL bResult = FALSE;

	/* Map user memory to kernel. */
	pCpuInfo = (__CPU_INFO*)map_to_kernel((void*)PARAM(0), sizeof(__CPU_INFO), __INOUT);
	if (NULL == pCpuInfo)
	{
		goto __TERMINAL;
	}

	bResult = GetCPUInfo(pCpuInfo);
	if (bResult)
	{
		/* Map back to user. */
		map_to_user((void*)PARAM(0), sizeof(__CPU_INFO), __INOUT, pCpuInfo);
	}

__TERMINAL:
	SYSCALL_RET = bResult;
}

/*
 * General CloseHandle routine, close a handle that
 * opened by CreateProcess, CreateThread, CreateMutex, ...
 * any kernel object that map to a handle. 
 * CloseHandle will invoke ObjectManager.DestroyObject
 * routine.
 */
static void SC_CloseHandle(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE general_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* pProcess = __CURRENT_PROCESS;

	BUG_ON(NULL == pProcess);

	general_handle = (__HANDLE)PARAM(0);
	if (INVALID_HANDLE_VALUE == general_handle)
	{
		goto __TERMINAL;
	}
	ProcessManager.CloseHandle(pProcess, general_handle);

__TERMINAL:
	return;
}

/*
 * Get a process's exit code given it's handle.
 */
static void SC_GetProcessExitCode(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE proc_handle = INVALID_HANDLE_VALUE;
	__PROCESS_OBJECT* curr_process = __CURRENT_PROCESS;
	__PROCESS_OBJECT* exit_process = NULL;
	int exit_code = -1;

	/* 
	 * Must be invoked in user space, so current 
	 * process must be here. 
	 */
	BUG_ON(NULL == curr_process);

	/* Get the process object. */
	proc_handle = (__HANDLE)PARAM(0);
	exit_process = (__PROCESS_OBJECT*)ProcessManager.GetObjectByHandle(
		curr_process, proc_handle);
	if (NULL == exit_process)
	{
		SYSCALL_RET = (uint32_t)exit_code;
		return;
	}
	SYSCALL_RET = exit_process->exit_code;
	return;
}

/* 
 * Put flash message to a specified process. 
 * If the target process handle is not specified,
 * then send to itself.
 */
static void SC_PutFlashMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE hProcess = INVALID_HANDLE_VALUE;
	int flash_message_sz = 0;
	LPVOID flash_message = NULL;
	__PROCESS_OBJECT* pProcess = NULL;
	LPVOID kernel_msg = NULL;
	BOOL bResult = FALSE;

	hProcess = (__HANDLE)PARAM(0);
	flash_message_sz = (int)PARAM(1);
	flash_message = (LPVOID)PARAM(2);

	/* Check parameters. */
	if ((flash_message_sz > PROCESS_MAX_FLASHMESSAGE_LEN) || 
		(0 == flash_message_sz) ||
		(NULL == flash_message))
	{
		goto __TERMINAL;
	}

	if (INVALID_HANDLE_VALUE == hProcess)
	{
		/* To self. */
		pProcess = __CURRENT_PROCESS;
	}
	else {
		/* Get the process object given it's handle. */
		pProcess = (__PROCESS_OBJECT*)ProcessManager.GetObjectByHandle(
			__CURRENT_PROCESS, hProcess);
		if (NULL == pProcess)
		{
			goto __TERMINAL;
		}
	}

	/* Map user memory into kernel. */
	kernel_msg = (LPVOID)map_to_kernel(flash_message, flash_message_sz, __IN);
	if (NULL == kernel_msg)
	{
		goto __TERMINAL;
	}

	/* Invoke the put flash message routine. */
	bResult = ProcessManager.PutFlashMessage(pProcess, 
		flash_message_sz, kernel_msg);

__TERMINAL:
	if (flash_message && flash_message_sz && kernel_msg)
	{
		map_to_user(flash_message, flash_message_sz, __IN, kernel_msg);
	}
	SYSCALL_RET = (uint32_t)bResult;
}

/*
 * Get flash message from a given process. 
 * If the handle of target process is not specified(NULL),
 * then get flash message from self.
 */
static void SC_GetFlashMessage(__SYSCALL_PARAM_BLOCK* pspb)
{
	__HANDLE hProcess = INVALID_HANDLE_VALUE;
	int* flash_message_sz = 0;
	int original_sz = 0;
	LPVOID flash_message = NULL;
	__PROCESS_OBJECT* pProcess = NULL;
	LPVOID kernel_msg = NULL;
	BOOL bResult = FALSE;

	hProcess = (__HANDLE)PARAM(0);
	flash_message_sz = (int*)PARAM(1);
	flash_message = (LPVOID)PARAM(2);

	/* Check parameters. */
	if ((NULL == flash_message_sz) ||
		(*flash_message_sz > PROCESS_MAX_FLASHMESSAGE_LEN) || 
		(0 == *flash_message_sz) || 
		(NULL == flash_message))
	{
		goto __TERMINAL;
	}

	original_sz = *flash_message_sz;

	if (INVALID_HANDLE_VALUE == hProcess)
	{
		pProcess = __CURRENT_PROCESS;
	}
	else {
		/* Get the process object given it's handle. */
		pProcess = (__PROCESS_OBJECT*)ProcessManager.GetObjectByHandle(
			__CURRENT_PROCESS, hProcess);
		if (NULL == pProcess)
		{
			goto __TERMINAL;
		}
	}

	/* Map user memory into kernel. */
	kernel_msg = (LPVOID)map_to_kernel(flash_message, original_sz, __OUT);
	if (NULL == kernel_msg)
	{
		goto __TERMINAL;
	}

	/* Invoke the put flash message routine. */
	bResult = ProcessManager.GetFlashMessage(pProcess,
		flash_message_sz, kernel_msg);

__TERMINAL:
	if (flash_message && original_sz && kernel_msg)
	{
		map_to_user(flash_message, original_sz, __OUT, kernel_msg);
	}
	SYSCALL_RET = (uint32_t)bResult;
}

/*
 * Register all system calls into system call entry
 * array, using the syscall number.
 */
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
	pSysCallEntry[SYSCALL_CREATEUSERTHREAD] = SC_CreateUserThread;
	pSysCallEntry[SYSCALL_DESTROYUSERTHREAD] = SC_DestroyUserThread;
	pSysCallEntry[SYSCALL_GETSYSTEMINFO] = SC_GetSystemInfo;
	pSysCallEntry[SYSCALL_VIRTUALQUERY] = SC_VirtualQuery;
	pSysCallEntry[SYSCALL_GETCURRENTTHREADID] = SC_GetCurrentThreadID;
	pSysCallEntry[SYSCALL_GETCLOCKCOUNTER] = SC_GetClockCounter;
	pSysCallEntry[SYSCALL_TLSALLOC] = SC_TLSAlloc;
	pSysCallEntry[SYSCALL_TLSFREE] = SC_TLSFree;
	pSysCallEntry[SYSCALL_TLSGETVALUE] = SC_TLSGetValue;
	pSysCallEntry[SYSCALL_TLSSETVALUE] = SC_TLSSetValue;
	pSysCallEntry[SYSCALL_RESUMETHREAD] = SC_ResumeThread;
	pSysCallEntry[SYSCALL_CREATEPROCESS] = SC_CreateProcess;
	pSysCallEntry[SYSCALL_CLOSEHANDLE] = SC_CloseHandle;
	pSysCallEntry[SYSCALL_GETPROCESSEXITCODE] = SC_GetProcessExitCode;
	pSysCallEntry[SYSCALL_PUTFLASHMESSAGE] = SC_PutFlashMessage;
	pSysCallEntry[SYSCALL_GETFLASHMESSAGE] = SC_GetFlashMessage;
	pSysCallEntry[SYSCALL_CREATECONDITION] = SC_CreateCondition;
	pSysCallEntry[SYSCALL_CONDITIONDESTROY] = SC_DestroyCondition;
	pSysCallEntry[SYSCALL_CONDITIONWAIT] = SC_ConditionWait;
	pSysCallEntry[SYSCALL_CONDITIONTIMEOUTWAIT] = SC_ConditionTimeoutWait;
	pSysCallEntry[SYSCALL_CONDITIONSIGNAL] = SC_ConditionSignal;
	pSysCallEntry[SYSCALL_CONDITIONBROADCAST] = SC_ConditionBroadcast;
	pSysCallEntry[SYSCALL_GETCPUINFO] = SC_GetCPUInfo;
	pSysCallEntry[SYSCALL_CREATESEMAPHORE] = SC_CreateSemaphore;
	pSysCallEntry[SYSCALL_DESTROYSEMAPHORE] = SC_DestroySemaphore;
	pSysCallEntry[SYSCALL_RELEASESEMAPHORE] = SC_ReleaseSemaphore;
	pSysCallEntry[SYSCALL_GETSEMAPHOREVALUE] = SC_GetSemaphoreValue;
}

#undef PARAM
#undef SYSCALL_RET
