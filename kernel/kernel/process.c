//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 18,2015
//    Module Name               : process.c
//    Module Funciton           : 
//                                Process mechanism related data types and
//                                operations.
//                                As a embedded operating system,it's no
//                                strong need to implement process,since it
//                                should be supported by VMM mechanism,and
//                                VMM feature is not supported in most
//                                embedded OS.But a clear difference is exist
//                                between traditional embedded OS and HelloX
//                                that HelloX will support more powerful smart
//                                devices than legacy embedded OS,so we implements 
//                                process mechanism here,to support current's
//                                application.
//                                It's worth noting that the architecture of
//                                HelloX's process framework is extensible and
//                                add new features in the future according to
//                                real application.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <process.h>
#include <stdio.h>
#include <types.h>
#include <mlayout.h>

/* Routines and objects belong to process module. */
#include "process2.c"

/* Only available when process is enabled. */
#if defined(__CFG_SYS_PROCESS)

//WaitForProcessObject routine,wait the process run over.
//We just wait on the main kernel thread,since it's life-cycle
//covers all kernel thread(s) belong to this process.
static DWORD WaitForProcessObject(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* pProcessObject = (__PROCESS_OBJECT*)lpThis;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == pProcessObject);
	__ENTER_CRITICAL_SECTION_SMP(pProcessObject->spin_lock, ulFlags);
	if (PROCESS_STATUS_TERMINAL == pProcessObject->dwProcessStatus)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pProcessObject->spin_lock, ulFlags);
		return OBJECT_WAIT_RESOURCE;
	}
	BUG_ON(NULL == pProcessObject->lpMainThread);
	__LEAVE_CRITICAL_SECTION_SMP(pProcessObject->spin_lock, ulFlags);

	/* 
	 * Just wait on the main thread for simplicity. 
	 * Race condition may raise here,consider the main thread
	 * is destroyed before commiting the wait.Dedicated wait
	 * queue should be implemented in process level,we will
	 * do this in future.:-)
	 */
	return pProcessObject->lpMainThread->WaitForThisObject(
		(__COMMON_OBJECT*)pProcessObject->lpMainThread);
}

//Initializer of process object.
BOOL ProcessInitialize(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* lpProcess = (__PROCESS_OBJECT*)lpThis;

	BUG_ON(NULL == lpProcess);

	//Initialize the process's kernel thread list.
	lpProcess->ThreadObjectList.prev = &lpProcess->ThreadObjectList;
	lpProcess->ThreadObjectList.next = &lpProcess->ThreadObjectList;
	lpProcess->ThreadObjectList.pKernelObject = NULL;
	lpProcess->nKernelThreadNum = 0;

	/* Set virtual memory manager to NULL. */
	lpProcess->pMemMgr = NULL;

	//Initialize TLS flags.
	lpProcess->dwTLSFlags         = 0;
	lpProcess->dwObjectSignature  = KERNEL_OBJECT_SIGNATURE;
	lpProcess->lpMainThread = NULL;
	lpProcess->WaitForThisObject  = WaitForProcessObject;
	lpProcess->ProcessName[0]     = 0;

#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpProcess->spin_lock, "proc");
#endif 

	return TRUE;
}

/*
 * Destroy a process object.
 * Just delegates to DestroyObject routine of ObjectManager,
 * since process object is a kernel object.
 */
static VOID DestroyProcess(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpObject)
{
	BUG_ON((NULL == lpThis) || (NULL == lpObject));
	/* Now destroy the process object itself. */
	ObjectManager.DestroyObject(&ObjectManager, lpObject);
	return;
}

//Uninitializer of process object.
BOOL ProcessUninitialize(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* pProcessObject = (__PROCESS_OBJECT*)lpThis;
	__KOBJ_LIST_NODE* pListNode = NULL;
	unsigned long ulFlags = 0;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;

	BUG_ON(NULL == pProcessObject);

	//Only the process with TERMINAL status can be destroyed,please use TerminateProcess
	//routine to terminate process's normal execution and put it into this status.
	if (PROCESS_STATUS_TERMINAL != pProcessObject->dwProcessStatus)
	{
		return FALSE;
	}

	//Delete it from the global process list.
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);
	pListNode = ProcessManager.ProcessList.next;
	while (pListNode != &ProcessManager.ProcessList)
	{
		if ((__COMMON_OBJECT*)pProcessObject == pListNode->pKernelObject)  //Find.
		{
			break;
		}
		pListNode = pListNode->next;
	}
	if (pListNode != &ProcessManager.ProcessList)
	{
		pListNode->next->prev = pListNode->prev;
		pListNode->prev->next = pListNode->next;
		pListNode->next = pListNode->prev = NULL;  //Set as flags.
	}
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);

	/* Does not find the process object in global list. */
	BUG_ON(pListNode->next != NULL);
	KMemFree(pListNode, KMEM_SIZE_TYPE_ANY, 0);

	/* 
	 * Destroy all kernel thread(s) this process own. 
	 * No need to release the memory of list node since it will
	 * be released when the thread is destroyed.
	 */
	pListNode = pProcessObject->ThreadObjectList.next;
	while (pListNode != &pProcessObject->ThreadObjectList)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pListNode->pKernelObject;
		BUG_ON(NULL == pKernelThread);
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pKernelThread);
		/* 
		 * Iterate from begining since the list node is removed in 
		 * process of thread's destroying. 
		 */
		pListNode = pProcessObject->ThreadObjectList.next;
	}

	/*
	 * Destroy the corresponding virtual memory manager.
	 * It must be the last step,since other routines may refer it.
	 */
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pProcessObject->pMemMgr);
	return TRUE;
}

/*************************************************************************/
//**
//**    Process Manager's implementation.
//**
/*************************************************************************/

//Entry point of all process,like the KernelThreadWrapper routine.This routine
//acts as the main kernel thread's running routine,it's lifecycle covers all
//kernel thread(s) created by this process.So in the end of the routine,it will
//wait for all kernel thread(s) belong to this process to run over.
static DWORD ProcessEntry(LPVOID pData)
{
	__PROCESS_OBJECT*         pProcessObject = (__PROCESS_OBJECT*)pData;
	__KOBJ_LIST_NODE*         pListNode      = NULL;
	__KERNEL_THREAD_OBJECT*   pKernelThread  = NULL;
	DWORD                     dwRetVal       = 0;

	if(NULL == pProcessObject)
	{
		return 0;
	}
	//Call the user specified main kernel thread function.
	if(pProcessObject->lpMainStartRoutine)
	{
		dwRetVal = pProcessObject->lpMainStartRoutine(pProcessObject->lpMainRoutineParam);
	}

	//Now wait for all kernel thread(s) except the main kernel thread to run over.
	pListNode = pProcessObject->ThreadObjectList.next;
	while(pListNode != &pProcessObject->ThreadObjectList)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pListNode->pKernelObject;
		if(NULL == pKernelThread)
		{
			BUG();
		}
		pKernelThread->WaitForThisObject((__COMMON_OBJECT*)pKernelThread);
		pListNode = pListNode->next;
	}

	//Set the process's status to TERMINAL;
	pProcessObject->dwProcessStatus = PROCESS_STATUS_TERMINAL;

	return dwRetVal;
}

/*
 * Half bottom of a process,at the end of each user thread,
 * this routine will be invoked(through system call in user space).
 * It waits all work thread(s) run over if current thread is the
 * main thread,then release all process level resources and exit
 * by calling KernelThreadClean routine.
 * It just invoke the KernelThreadClean routine if current thread
 * is not the main thread.
 */
extern void KernelThreadClean(__COMMON_OBJECT* lpKThread, DWORD dwExitCode);
static void ProcessHalfBottom()
{
	__PROCESS_OBJECT* pProcess = NULL;
	__KERNEL_THREAD_OBJECT* currentThread = __CURRENT_KERNEL_THREAD;
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	__KOBJ_LIST_NODE* pListNode = NULL;
	unsigned long ulFlags = 0;

	/* Only user thread is permited to invoke this routine. */
	BUG_ON(IS_KERNEL_THREAD(currentThread));
	/* Obtain the process object of current thread. */
	pProcess = (__PROCESS_OBJECT*)currentThread->lpOwnProcess;
	BUG_ON(NULL == pProcess);
	if (currentThread != pProcess->lpMainThread)
	{
		/* Work thread,just clean up and exit. */
		KernelThreadClean((__COMMON_OBJECT*)currentThread, 0);
		/* Should never reach here. */
		BUG_ON(TRUE);
	}
	/*
	 * Current thread is the main thread of process,so
	 * do some clean up work for current process.Should 
	 * wait all work thread(s) run over.
	 */
	pListNode = pProcess->ThreadObjectList.next;
	while (pListNode != &pProcess->ThreadObjectList)
	{
		pKernelThread = (__KERNEL_THREAD_OBJECT*)pListNode->pKernelObject;
		BUG_ON(NULL == pKernelThread);
		if (currentThread != pKernelThread)
		{
			/* Only wait other thread(s),since current thread is also in list. */
			pKernelThread->WaitForThisObject((__COMMON_OBJECT*)pKernelThread);
		}
		pListNode = pListNode->next;
	}

	/* Release all process level resources. */
	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	/* Set the process's status to TERMINAL so it could be destroyed. */
	pProcess->dwProcessStatus = PROCESS_STATUS_TERMINAL;
	pProcess->lpMainThread = NULL;
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);

	/* Clean up main thread. */
	KernelThreadClean((__COMMON_OBJECT*)currentThread, 0);
	/* Should never reach here. */
	BUG_ON(TRUE);
}

/* 
 * Create a new process. 
 * This routine is invoked by HelloX's shell,to
 * load a binary module and execute it in user
 * space.
 */
static __PROCESS_OBJECT* CreateProcess(__COMMON_OBJECT* lpThis,
	DWORD dwMainThreadStackSize,
	DWORD dwMainThreadPriority,
	char* pszCmdLine,
	LPVOID pCmdObject,
	LPVOID lpReserved,
	LPSTR lpszName)
{
	__PROCESS_MANAGER* pProcessManager   = (__PROCESS_MANAGER*)lpThis;
	__PROCESS_OBJECT* pProcessObject = NULL;
	unsigned long ulFlags;
	int i = 0;
	BOOL bResult = FALSE;
	__KOBJ_LIST_NODE* pProcessNode = NULL;
	__VIRTUAL_MEMORY_MANAGER* pvmmgr = NULL;

	BUG_ON(NULL == pProcessManager);
	BUG_ON((NULL == pCmdObject) && (NULL == pszCmdLine));

	/* Create a new process kernel object. */
	pProcessObject = (__PROCESS_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,OBJECT_TYPE_PROCESS);
	if(NULL == pProcessObject)
	{
		goto __TERMINAL;
	}
	if(!pProcessObject->Initialize((__COMMON_OBJECT*)pProcessObject))
	{
		goto __TERMINAL;
	}

	/* In process of initialization. */
	pProcessObject->dwProcessStatus = PROCESS_STATUS_INITIALIZING;

	/* Insert the process object into global process list. */
	pProcessNode = (__KOBJ_LIST_NODE*)KMemAlloc(sizeof(__KOBJ_LIST_NODE),
		KMEM_SIZE_TYPE_ANY);
	if (NULL == pProcessNode)
	{
		goto __TERMINAL;
	}
	pProcessNode->pKernelObject = (__COMMON_OBJECT*)pProcessObject;
	__ENTER_CRITICAL_SECTION_SMP(pProcessManager->spin_lock, ulFlags);
	pProcessNode->prev = &pProcessManager->ProcessList;
	pProcessNode->next = pProcessManager->ProcessList.next;
	pProcessManager->ProcessList.next->prev = pProcessNode;
	pProcessManager->ProcessList.next = pProcessNode;
	__LEAVE_CRITICAL_SECTION_SMP(pProcessManager->spin_lock, ulFlags);

	/* Set the process's name in safe maner. */
	for(i = 0;i < MAX_THREAD_NAME;i ++)
	{
		if(lpszName[i])
		{
			pProcessObject->ProcessName[i] = lpszName[i];
		}
		else
		{
			break;
		}
	}
	pProcessObject->ProcessName[i] = 0;

	/* 
	 * Create the corresponding virtual memory manager object. 
	 * Each process has it's own virtual memory space,so a dedicated
	 * virtual manager is created to manage the whole memory space.
	 */
	pvmmgr = (__VIRTUAL_MEMORY_MANAGER*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER);
	if (NULL == pvmmgr)
	{
		goto __TERMINAL;
	}
	if (!pvmmgr->Initialize((__COMMON_OBJECT*)pvmmgr))
	{
		goto __TERMINAL;
	}
	pProcessObject->pMemMgr = pvmmgr;

	/* Initializes the user space of this process. */
	if (!PrepareUserSpace(pProcessObject))
	{
		goto __TERMINAL;
	}

	/* 
	 * Create main kernel thread now,set it's status to SUSPENDED 
	 * since the process's initialization is not over,the start address
	 * contains nothing now,since the location will be loaded by
	 * PrepareProcessCtx routine.
	 * And will resume the main kernel thread after the initialization.
	 */
	pProcessObject->lpMainThread = KernelThreadManager.CreateUserThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		dwMainThreadStackSize,
		KERNEL_THREAD_STATUS_SUSPENDED,
		dwMainThreadPriority,
		(__KERNEL_THREAD_ROUTINE)KMEM_USERAPP_START,
		NULL,
		pProcessObject, /* Own process must be specified. */
		lpszName);
	if(NULL == pProcessObject->lpMainThread)
	{
		goto __TERMINAL;
	}

	/* How many user thread(s) created yet. */
	pProcessObject->nKernelThreadNum ++;

	/* 
	 * Construct user stack frame. 
	 * It should be invoked after the creation of
	 * main thread,since the user stack is created in
	 * CreateUserThread routine.
	 */
	if (!PrepareUserStack(pProcessObject, pCmdObject, pszCmdLine))
	{
		goto __TERMINAL;
	}

	/* Everything is in place so set it's tatus to READY. */
	pProcessObject->dwProcessStatus = PROCESS_STATUS_READY;

	/* Resume the main thread to run. */
	KernelThreadManager.ResumeKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)pProcessObject->lpMainThread);

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(pProcessObject)
		{
			if(pProcessObject->lpMainThread)
			{
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)pProcessObject->lpMainThread);
			}
			/* 
			 * Set the process's status as terminal,so that it could be 
			 * destroyed by Object Manager.Please refer the ProcessUninitialize
			 * routine for detail.
			 */
			pProcessObject->dwProcessStatus = PROCESS_STATUS_TERMINAL;
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pProcessObject);
			pProcessObject = NULL;
		}

		if(pProcessNode)
		{
			KMemFree(pProcessNode,KMEM_SIZE_TYPE_ANY,0);
		}
	}
	return pProcessObject;
}

/* 
 * Link a kernel thread to it's own process.
 * This kernel thread is created by the own process.
 */
static BOOL LinkKernelThread(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpProcess,
	__COMMON_OBJECT* lpThread)
{
	__PROCESS_MANAGER*       lpProcessManager   = (__PROCESS_MANAGER*)lpThis;
	__PROCESS_OBJECT*        lpProcessObject    = (__PROCESS_OBJECT*)lpProcess;
	__KERNEL_THREAD_OBJECT*  lpKernelThread     = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KOBJ_LIST_NODE*        pListNode          = NULL;
	DWORD                    dwFlags;

	if((NULL == lpProcessManager) || (NULL == lpProcessObject) || (NULL == lpKernelThread))
	{
		return FALSE;
	}

	//Check if the objects are valid.
	if(lpProcessObject->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}
	if(lpKernelThread->dwObjectSignature != KERNEL_OBJECT_SIGNATURE)
	{
		return FALSE;
	}

	//Set the own process of kernel thread.
	lpKernelThread->lpOwnProcess = (__COMMON_OBJECT*)lpProcessObject;

	//Create the list node to contain the kernel thread.
	pListNode = (__KOBJ_LIST_NODE*)KMemAlloc(sizeof(__KOBJ_LIST_NODE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pListNode)
	{
		return FALSE;
	}
	pListNode->pKernelObject = (__COMMON_OBJECT*)lpKernelThread;
	pListNode->next = NULL;
	pListNode->prev = NULL;

	__ENTER_CRITICAL_SECTION_SMP(lpProcessManager->spin_lock, dwFlags);
	pListNode->prev = lpProcessObject->ThreadObjectList.prev;
	pListNode->next = &lpProcessObject->ThreadObjectList;
	lpProcessObject->ThreadObjectList.prev->next = pListNode;
	lpProcessObject->ThreadObjectList.prev       = pListNode;
	lpProcessObject->nKernelThreadNum ++;  //Update total thread number.
	__LEAVE_CRITICAL_SECTION_SMP(lpProcessManager->spin_lock, dwFlags);

	if(NULL == pListNode->next)
	{
		//The list node is not used since the kernel thread is the fist one,which is saved
		//into lpProcessObject's lpMainKernelThread member.
		//Only the non-main kernel thread will be linked into KernelThreadList.
		KMemFree(pListNode,KMEM_SIZE_TYPE_ANY,0);
	}
	return TRUE;
}

//Detach a kernel thread from the kernel thread list of the specified process.
static BOOL UnlinkKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpProcess,__COMMON_OBJECT* lpThread)
{
	__PROCESS_MANAGER*       lpProcessManager   = (__PROCESS_MANAGER*)lpThis;
	__PROCESS_OBJECT*        lpProcessObject    = (__PROCESS_OBJECT*)lpProcess;
	__KERNEL_THREAD_OBJECT*  lpKernelThread     = (__KERNEL_THREAD_OBJECT*)lpThread;
	__KOBJ_LIST_NODE*        pListNode          = NULL;
	BOOL                     bResult            = FALSE;
	DWORD                    dwFlags;

	if((NULL == lpProcessManager) || (NULL == lpKernelThread))
	{
		return FALSE;
	}
	//The own process can be obtained from kernel thread object.
	if(NULL == lpProcessObject)
	{
		lpProcessObject = (__PROCESS_OBJECT*)lpKernelThread->lpOwnProcess;
		if(NULL == lpProcessObject)
		{
			return FALSE;
		}
	}
	else
	{
		//Should make sure the given process is the exactly one kept in kernel thread object.
		if(lpProcessObject != (__PROCESS_OBJECT*)lpKernelThread->lpOwnProcess)
		{
			return FALSE;
		}
	}

	//Now travel the whole kernel thread list of the process,find the list node and detach it
	//from list.
	__ENTER_CRITICAL_SECTION_SMP(lpProcessManager->spin_lock, dwFlags);
	pListNode = lpProcessObject->ThreadObjectList.next;
	while(pListNode != &lpProcessObject->ThreadObjectList)
	{
		if(pListNode->pKernelObject == (__COMMON_OBJECT*)lpKernelThread)  //Find.
		{
			break;
		}
		pListNode = pListNode->next;
	}
	if(pListNode != &lpProcessObject->ThreadObjectList)
	{
		//Delete the node from list.
		pListNode->next->prev = pListNode->prev;
		pListNode->prev->next = pListNode->next;
		lpProcessObject->nKernelThreadNum --;
		bResult = TRUE;
	}
	else
	{
		bResult = FALSE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpProcessManager->spin_lock, dwFlags);

	//Release list node.
	if(bResult)
	{
		KMemFree(pListNode,KMEM_SIZE_TYPE_ANY,0);
	}

	return bResult;
}

static BOOL GetTLSKey(__COMMON_OBJECT* lpThis,DWORD* pTLSKey,LPVOID pReserved)
{
	__PROCESS_OBJECT*   pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	DWORD               dwFlags,i;

	if((NULL == pProcessObject) || (NULL == pTLSKey))
	{
		return FALSE;
	}

	//Try to find a unused TLS slot,each slot corresponding to the index of
	//TLS array in Kernel Thread Object.The TLS key is the index value.
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	for(i = 0;i < MAX_TLS_NUM;i ++)
	{
		if((pProcessObject->dwTLSFlags >> i) & 0x01)
		{
			continue;
		}
		else  //Find a empty slot,it's corresponding flag bit is 0.
		{
			pProcessObject->dwTLSFlags |= (0x01 << i);  //Set the bit to 1.
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);

	if(MAX_TLS_NUM == i)  //Can not find free TLS slot.
	{
		return FALSE;
	}
	*pTLSKey = i;
	return TRUE;
}

static VOID ReleaseTLSKey(__COMMON_OBJECT* lpThis,DWORD TLSKey,LPVOID pReserved)
{
	__PROCESS_OBJECT*     pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	DWORD                 dwFlags;

	if((NULL == pProcessObject) || (TLSKey > MAX_TLS_NUM))
	{
		return;
	}
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	pProcessObject->dwTLSFlags &= ~(0x01 << TLSKey);
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	return;
}

static BOOL GetTLSValue(__COMMON_OBJECT* lpThis,DWORD TLSKey,LPVOID* ppValue)
{
	__PROCESS_OBJECT*       pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	BOOL                    bResult        = FALSE;
	DWORD                   dwFlags;

	if((NULL == pProcessObject) || (TLSKey > MAX_TLS_NUM))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	if(pProcessObject->dwTLSFlags & (0x01 << TLSKey))
	{
		*ppValue = pKernelThread->TLS[TLSKey];
		bResult  = TRUE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	return bResult;
}

static BOOL SetTLSValue(__COMMON_OBJECT* lpThis,DWORD TLSKey,LPVOID pValue)
{
	__PROCESS_OBJECT*       pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	BOOL                    bResult        = FALSE;
	DWORD                   dwFlags;

	if((NULL == pProcessObject) || (TLSKey > MAX_TLS_NUM))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	if(pProcessObject->dwTLSFlags & (0x01 << TLSKey))
	{
		pKernelThread->TLS[TLSKey] = pValue;
		bResult  = TRUE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, dwFlags);
	return bResult;
}

//Initialization routine of Process Manager.
static BOOL PMInitialize(__COMMON_OBJECT* lpThis)
{
	__PROCESS_MANAGER*  pProcessManager = (__PROCESS_MANAGER*)lpThis;

	if(NULL == pProcessManager)
	{
		return FALSE;
	}

#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pProcessManager->spin_lock, "processmgr");
#endif
	//Initialize the global process list.
	pProcessManager->ProcessList.prev = &pProcessManager->ProcessList;
	pProcessManager->ProcessList.next = &pProcessManager->ProcessList;

	return TRUE;
}

//Get current process from system.
static __PROCESS_OBJECT* GetCurrentProcess(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT* lpCurrentThread = __CURRENT_KERNEL_THREAD;

	BUG_ON(NULL == lpThis);

	if(NULL == lpCurrentThread)
	{
		return NULL;
	}
	return (__PROCESS_OBJECT*)lpCurrentThread->lpOwnProcess;
}

/*
* Intializes CPU specific task management functions.
* Task(process) related mechanism should be established
* in this routine,a typical example is, create TSS for
* current processor and initializes the TR register using
* the TSS.
* It just calls architecture specific task initialization
* routine,which is a global routine resides in arch_xxx.c
* file.
*/
static BOOL __InitializeCPUTask(__COMMON_OBJECT* pThis)
{
	BUG_ON(NULL == pThis);
	return __InitCPUTask();
}

//Global process manager object,one and only one in system,to manages all
//processes.
__PROCESS_MANAGER ProcessManager = {
	{0},
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,       //spin_lock.
#endif
	NULL,                       //pUserAgent.
	//Operation routines.
	CreateProcess,              //CreateProcess.
	DestroyProcess,             //DestroyProcess.
	LinkKernelThread,           //LinkKernelThread.
	UnlinkKernelThread,         //UnlinkKernelThread.
	GetTLSKey,                  //SetTLSKey routine.
	ReleaseTLSKey,              //ReleaseTLSKey.
	GetTLSValue,                //GetTLSValue.
	SetTLSValue,                //SetTLSValue.
	GetCurrentProcess,          //GetCurrentProcess.
	PMInitialize,               //Initialize.
	__InitializeCPUTask,        //InitCPUTask.
	ProcessHalfBottom,          //ProcessHalfBottom.
};

/*
 * A helper routine used to dumpout all process(es) and it's kernel thread(s)
 * information,used for debugging.
 */
VOID DumpProcess()
{
	__KOBJ_LIST_NODE*    pProcessNode   = NULL;
	__PROCESS_OBJECT*    pProcessObject = NULL;
	int count = 0;

	_hx_printf("  All process(es) information as follows:\r\n");
	pProcessNode = ProcessManager.ProcessList.next;
	while(pProcessNode != &ProcessManager.ProcessList)
	{
		pProcessObject = (__PROCESS_OBJECT*)pProcessNode->pKernelObject;
		_hx_printf("  Process[%d]: name = [%s],mthread_name = [%s]\r\n",
			count,pProcessObject->ProcessName,pProcessObject->lpMainThread->KernelThreadName);
		count ++;
		pProcessNode = pProcessNode->next;
	}
}

#endif //__CFG_SYS_PROCESS
