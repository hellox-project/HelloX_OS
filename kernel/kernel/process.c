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
//                                devices than legacy embedded OS,and JamVM
//                                also requires some process features,such as
//                                signaling and TLS,so we implements part of
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


//WaitForProcessObject routine,wait the process run over.
//We just wait on the main kernel thread,since it's life-cycle
//covers all kernel thread(s) belong to this process.
static DWORD WaitForProcessObject(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* pProcessObject = (__PROCESS_OBJECT*)lpThis;

	if(NULL == pProcessObject)
	{
		return OBJECT_WAIT_FAILED;
	}
	if(NULL == pProcessObject->lpMainKernelThread)
	{
		return OBJECT_WAIT_FAILED;
	}
	//Wait on the main thread.
	return pProcessObject->lpMainKernelThread->WaitForThisObject(
		(__COMMON_OBJECT*)pProcessObject->lpMainKernelThread);
}

//Initializer of process object.
BOOL ProcessInitialize(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* lpProcess = (__PROCESS_OBJECT*)lpThis;

	if(NULL == lpProcess)
	{
		return FALSE;
	}
	//Initialize the process's kernel thread list.
	lpProcess->KernelThreadList.prev = &lpProcess->KernelThreadList;
	lpProcess->KernelThreadList.next = &lpProcess->KernelThreadList;
	lpProcess->KernelThreadList.pKernelObject = NULL;
	lpProcess->nKernelThreadNum      = 0;

	//Initialize TLS flags.
	lpProcess->dwTLSFlags         = 0;
	lpProcess->dwObjectSignature  = KERNEL_OBJECT_SIGNATURE;
	lpProcess->lpMainKernelThread = NULL;
	lpProcess->WaitForThisObject  = WaitForProcessObject;
	lpProcess->ProcessName[0]     = 0;
	return TRUE;
}

//Uninitializer of process object.
VOID ProcessUninitialize(__COMMON_OBJECT* lpThis)
{
	__PROCESS_OBJECT* lpProcess = (__PROCESS_OBJECT*)lpThis;

	if(NULL == lpProcess)
	{
		return;
	}

	//Just clear the kernel object signature.
	lpProcess->dwObjectSignature = 0;
	return;
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
	pListNode = pProcessObject->KernelThreadList.next;
	while(pListNode != &pProcessObject->KernelThreadList)
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

//Create a new process.
static __PROCESS_OBJECT* CreateProcess(__COMMON_OBJECT*                 lpThis,
											  DWORD                     dwMainThreadStackSize,
											  DWORD                     dwMainThreadPriority,
											  __KERNEL_THREAD_ROUTINE   lpMainStartRoutine,
											  LPVOID                    lpMainRoutineParam,
											  LPVOID                    lpReserved,
											  LPSTR                     lpszName)
{
	__PROCESS_MANAGER*        pProcessManager   = (__PROCESS_MANAGER*)lpThis;
	__PROCESS_OBJECT*         pProcessObject    = NULL;
	DWORD                     dwFlags;
	int                       i                 = 0;
	BOOL                      bResult           = FALSE;
	__KOBJ_LIST_NODE*         pProcessNode      = NULL;

	if((NULL == pProcessManager) || (NULL == lpMainStartRoutine))
	{
		goto __TERMINAL;
	}

	//Now create process object.
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

	//Set the status to INITIALIZING in process of initialization.
	pProcessObject->dwProcessStatus = PROCESS_STATUS_INITIALIZING;

	//Set the process's name.
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
	//Set process status and other parameters.
	pProcessObject->lpMainStartRoutine = lpMainStartRoutine;
	pProcessObject->lpMainRoutineParam = lpMainRoutineParam;

	//Create main kernel thread now,set it's status to SUSPENDED since the process's
	//initialization is not over.And will resume the main kernel thread after init.
	pProcessObject->lpMainKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		dwMainThreadStackSize,
		KERNEL_THREAD_STATUS_SUSPENDED,
		dwMainThreadPriority,
		ProcessEntry,
		(LPVOID)pProcessObject,
		pProcessObject,  //Set the owner process of the kernel thread.
		lpszName);
	if(NULL == pProcessObject->lpMainKernelThread)
	{
		goto __TERMINAL;
	}

	//Update kernel thread's total number.
	pProcessObject->nKernelThreadNum ++;

	//Insert the process object into global process list.
	pProcessNode = (__KOBJ_LIST_NODE*)KMemAlloc(sizeof(__KOBJ_LIST_NODE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pProcessNode)
	{
		goto __TERMINAL;
	}
	pProcessNode->pKernelObject = (__COMMON_OBJECT*)pProcessObject;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pProcessNode->prev = &pProcessManager->ProcessList;
	pProcessNode->next = pProcessManager->ProcessList.next;
	pProcessManager->ProcessList.next->prev = pProcessNode;
	pProcessManager->ProcessList.next = pProcessNode;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Set the process's status to READY.
	pProcessObject->dwProcessStatus = PROCESS_STATUS_READY;

	//Resume the main thread to run.
	KernelThreadManager.ResumeKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)pProcessObject->lpMainKernelThread);

	bResult = TRUE;
__TERMINAL:
	if(!bResult)
	{
		if(pProcessObject)  //Process object is created.
		{
			if(pProcessObject->lpMainKernelThread)
			{
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)pProcessObject->lpMainKernelThread);
			}
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

//Destroy process.
static VOID DestroyProcess(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpObject)
{
	__PROCESS_MANAGER*    pProcessManager = (__PROCESS_MANAGER*)lpThis;
	__PROCESS_OBJECT*     pProcessObject  = (__PROCESS_OBJECT*)lpObject;
	__KOBJ_LIST_NODE*     pListNode       = NULL;
	DWORD                 dwFlags;

	if((NULL == pProcessManager) || (NULL == pProcessObject))
	{
		return;
	}
	//Only the process with TERMINAL status can be destroyed,please use TerminateProcess
	//routine to terminate process's normal execution and put it into this status.
	if(PROCESS_STATUS_TERMINAL != pProcessObject->dwProcessStatus)
	{
		return;
	}

	//Destroy the main kernel thread first.
	if(pProcessObject->lpMainKernelThread)
	{
		KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)pProcessObject->lpMainKernelThread);
	}

	//Delete it from the global process list.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pListNode = pProcessManager->ProcessList.next;
	while(pListNode != &pProcessManager->ProcessList)
	{
		if((__COMMON_OBJECT*)pProcessObject == pListNode->pKernelObject)  //Find.
		{
			break;
		}
		pListNode = pListNode->next;
	}
	if(pListNode != &pProcessManager->ProcessList)
	{
		pListNode->next->prev = pListNode->prev;
		pListNode->prev->next = pListNode->next;
		pListNode->next = pListNode->prev = NULL;  //Set as flags.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(pListNode->next != NULL)  //Does not find the process object in global list.
	{
		BUG();
	}
	else
	{
		KMemFree(pListNode,KMEM_SIZE_TYPE_ANY,0);
	}

	//Now destroy the process object itself.Memory leaking caused by the missing of this
	//coding line.
	ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pProcessObject);
	return;
}

//Link a kernel thread to it's own process.This kernel thread is created by the own process.
static BOOL LinkKernelThread(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpProcess,__COMMON_OBJECT* lpThread)
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
	pListNode->next          = NULL;
	pListNode->prev          = NULL;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	//Check if the kernel thread is the first kernel thread of the process.
	if(NULL == lpProcessObject->lpMainKernelThread)
	{
		lpProcessObject->lpMainKernelThread = lpKernelThread;
	}
	else  //Not the main thread,link to process's thread list.NOTE: The main kernel thread is not linked to list.
	{
		pListNode->prev = lpProcessObject->KernelThreadList.prev;
		pListNode->next = &lpProcessObject->KernelThreadList;
		lpProcessObject->KernelThreadList.prev->next = pListNode;
		lpProcessObject->KernelThreadList.prev       = pListNode;
		lpProcessObject->nKernelThreadNum ++;  //Update total thread number.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

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
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pListNode = lpProcessObject->KernelThreadList.next;
	while(pListNode != &lpProcessObject->KernelThreadList)
	{
		if(pListNode->pKernelObject == (__COMMON_OBJECT*)lpKernelThread)  //Find.
		{
			break;
		}
		pListNode = pListNode->next;
	}
	if(pListNode != &lpProcessObject->KernelThreadList)
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
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

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
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
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
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

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
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pProcessObject->dwTLSFlags &= ~(0x01 << TLSKey);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

static BOOL GetTLSValue(__COMMON_OBJECT* lpThis,DWORD TLSKey,LPVOID* ppValue)
{
	__PROCESS_OBJECT*       pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__KERNEL_THREAD_OBJECT* pKernelThread  = KernelThreadManager.lpCurrentKernelThread;
	BOOL                    bResult        = FALSE;
	DWORD                   dwFlags;

	if((NULL == pProcessObject) || (TLSKey > MAX_TLS_NUM))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pProcessObject->dwTLSFlags & (0x01 << TLSKey))
	{
		*ppValue = pKernelThread->TLS[TLSKey];
		bResult  = TRUE;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return bResult;
}

static BOOL SetTLSValue(__COMMON_OBJECT* lpThis,DWORD TLSKey,LPVOID pValue)
{
	__PROCESS_OBJECT*       pProcessObject = ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager);
	__KERNEL_THREAD_OBJECT* pKernelThread  = KernelThreadManager.lpCurrentKernelThread;
	BOOL                    bResult        = FALSE;
	DWORD                   dwFlags;

	if((NULL == pProcessObject) || (TLSKey > MAX_TLS_NUM))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pProcessObject->dwTLSFlags & (0x01 << TLSKey))
	{
		pKernelThread->TLS[TLSKey] = pValue;
		bResult  = TRUE;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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

	//Initialize the global process list.
	pProcessManager->ProcessList.prev = &pProcessManager->ProcessList;
	pProcessManager->ProcessList.next = &pProcessManager->ProcessList;

	return TRUE;
}

//Get current process from system.
static __PROCESS_OBJECT* GetCurrentProcess(__COMMON_OBJECT* lpThis)
{
	__KERNEL_THREAD_OBJECT* lpCurrentThread = KernelThreadManager.lpCurrentKernelThread;

	if(NULL == lpCurrentThread)
	{
		return NULL;
	}
	return (__PROCESS_OBJECT*)lpCurrentThread->lpOwnProcess;
}

//Global process manager object,one and only one in system,to manages all
//processes.
__PROCESS_MANAGER ProcessManager = {
	{0},

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

	PMInitialize                //Initialize.
};

//A helper routine used to dumpout all process(es) and it's kernel thread(s)
//information,used to debugging.
VOID DumpProcess()
{
	__KOBJ_LIST_NODE*    pProcessNode   = NULL;
	//__KOBJ_LIST_NODE*    pThreadNode    = NULL;
	__PROCESS_OBJECT*    pProcessObject = NULL;
	//__KERNEL_THREAD_OBJECT* pKernelThread = NULL;
	int count = 0;

	_hx_printf("  All process(es) information as follows:\r\n");
	pProcessNode = ProcessManager.ProcessList.next;
	while(pProcessNode != &ProcessManager.ProcessList)
	{
		pProcessObject = (__PROCESS_OBJECT*)pProcessNode->pKernelObject;
		_hx_printf("  Process[%d]: name = [%s],mthread_name = [%s]\r\n",
			count,pProcessObject->ProcessName,pProcessObject->lpMainKernelThread->KernelThreadName);
		count ++;
		pProcessNode = pProcessNode->next;
	}
}
