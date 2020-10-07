//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 18,2015
//    Module Name               : process.c
//    Module Funciton           : 
//                                Process mechanism related data types and
//                                operations.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <mlayout.h>
#include <process.h>
#include <../shell/shell.h>

#if defined(__CFG_SYS_PROCESS)

/* Helper routine to validate user agent. */
static BOOL ValidateUserAgent(LPVOID pUserAgent)
{
	return TRUE;
}

/* 
 * Helper routine to map the user agent page to current 
 * process's memory space.
 * Load the user agent into memory if this routine is
 * invoked the first time.
 */
static BOOL MapUserAgent(__PROCESS_OBJECT* pProcess)
{
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = NULL;
	HANDLE hUserAgentFile = NULL;
	LPVOID pUserAgent = NULL;
	LPVOID pNewUserAgent = NULL;
	LPVOID pUserApp = NULL;
	BOOL bResult = FALSE;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == pProcess);
	/* User agent's length must be aligned with page size. */
	BUG_ON(KMEM_USERAGENT_LENGTH % PAGE_SIZE);
	pVmmMgr = pProcess->pMemMgr;
	BUG_ON(NULL == pVmmMgr);

	/* Load the user agent file into memory,if not yet. */
	__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);
	pUserAgent = ProcessManager.pUserAgent;
	__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);
	if (NULL == pUserAgent)
	{
		pUserAgent = KMemAlloc(KMEM_USERAGENT_LENGTH, KMEM_SIZE_TYPE_4K);
		if (NULL == pUserAgent)
		{
			goto __TERMINAL;
		}
		hUserAgentFile = CreateFile(USER_AGENT_FILE_NAME, FILE_ACCESS_READ, 0, NULL);
		if (NULL == hUserAgentFile)
		{
			_hx_printf("  Failed to load user agent[%s]\r\n",
				USER_AGENT_FILE_NAME);
			goto __TERMINAL;
		}
		if (!ReadFile(hUserAgentFile, KMEM_USERAGENT_LENGTH, pUserAgent, NULL))
		{
			goto __TERMINAL;
		}
		/* Validate it. */
		if (!ValidateUserAgent(pUserAgent))
		{
			goto __TERMINAL;
		}
		/* 
		 * Save to process manager object. 
		 * Other process may initialized the user agent in case
		 * of race,so check it again.
		 */
		__ENTER_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);
		if (NULL == ProcessManager.pUserAgent)
		{
			ProcessManager.pUserAgent = pUserAgent;
		}
		else
		{
			/* The newly created one should be released. */
			pNewUserAgent = pUserAgent;
			pUserAgent = ProcessManager.pUserAgent;
		}
		__LEAVE_CRITICAL_SECTION_SMP(ProcessManager.spin_lock, ulFlags);
	}

#if 0 
	/*
	 * Allocate memory space in user space for user application's
	 * agent module. The first user instruction of the process
	 * is in this place.
	 */
	pUserApp = pVmmMgr->VirtualAlloc(
		(__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)KMEM_USERAPP_START,
		USER_AGENT_FILE_LENGTH,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"useragent",
		NULL);
	BUG_ON(pUserApp != (char*)KMEM_USERAPP_START);

	/* Map the user agent into user space. Just copy currently. */
	bResult = pVmmMgr->UserMemoryCopy((__COMMON_OBJECT*)pVmmMgr, 
		pUserAgent, pUserApp, USER_AGENT_FILE_LENGTH, FALSE);
	if (!bResult)
	{
		goto __TERMINAL;
	}
#endif
	/* Map the user agent into user space. */
	LPVOID pPhysicalAddr = lpVirtualMemoryMgr->GetPhysicalAddress(
		(__COMMON_OBJECT*)lpVirtualMemoryMgr, pUserAgent);
	BUG_ON(NULL == pPhysicalAddr);
	bResult = pVmmMgr->VirtualMap((__COMMON_OBJECT*)pVmmMgr,
		(LPVOID)KMEM_USERAPP_START, pPhysicalAddr, KMEM_USERAGENT_LENGTH, 
		VIRTUAL_AREA_ACCESS_READ, "usragent");
	BUG_ON(!bResult);

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (hUserAgentFile)
	{
		CloseFile(hUserAgentFile);
	}
	if (pNewUserAgent)
	{
		KMemFree(pNewUserAgent, KMEM_SIZE_TYPE_4K, KMEM_USERAGENT_LENGTH);
	}
	return bResult;
}

#if defined(__I386__)
/*
 * Construct user stack frame so the user application
 * could use it directly.
 * The command line that user specified,and system
 * environment information,are pushed into user stack
 * by this routine.
 * pszCmdLine has higher priority than pCmdObj if both
 * are specified.
 * User agent entry routine could refer these information elements
 * as parameters like argc and argv[] in standard C.
 */
static BOOL PrepareUserStack(__PROCESS_OBJECT* pProcessObject,
	LPVOID pCmdObj, char* pszCmdLine)
{
	__VIRTUAL_MEMORY_MANAGER* pvmmgr = NULL;
	char* pTmpStack = NULL, *pTmpStackTop = NULL;
	char* pUserStack = NULL, *pUserStackTop = NULL;
	char** argv = NULL;
	int argc = 0, i = 0;
	__CMD_PARA_OBJ* pCmdObject = (__CMD_PARA_OBJ*)pCmdObj;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pProcessObject);
	BUG_ON((NULL == pCmdObject) && (NULL == pszCmdLine));

	pvmmgr = pProcessObject->pMemMgr;
	BUG_ON(NULL == pvmmgr);

	/* 
	 * Allocate temporary memory in kernel space as scratch 
	 * to construct user stack,since we could not access user
	 * space stack directly.
	 */
	pTmpStack = _hx_malloc(PAGE_SIZE);
	if (NULL == pTmpStack)
	{
		goto __TERMINAL;
	}
	pTmpStackTop = pTmpStack;

	/* Get the start address of parameter block in user stack. */
	pUserStack = (char*)pProcessObject->lpMainThread->pUserStack;
	BUG_ON(NULL == pUserStack);
	pUserStack += pProcessObject->lpMainThread->user_stk_size;
	pUserStack -= PAGE_SIZE;
	pUserStackTop = pUserStack;

	/*
	 * Construct the main thread's user stack,as
	 * (int argc, char* argv[]) format for the entry
	 * of user application agent.
	 */
	if (pszCmdLine)
	{
		/* Use raw command line to construct user stack. */
		argc = 1;

		/*
		 * Push a NULL returen address to simulate a call
		 * instruction, so the user entry could access argc
		 * and argv[] use normal invoke syntax.
		 */
		*(unsigned long*)pTmpStackTop = 0;
		pTmpStackTop += sizeof(unsigned long);
		pUserStackTop += sizeof(unsigned long);

		/* Push argc. */
		*(unsigned long*)pTmpStackTop = argc;
		pTmpStackTop += sizeof(unsigned long);
		pUserStackTop += sizeof(unsigned long);
		
		/* Push argv. */
		pUserStackTop += sizeof(unsigned long);
		*(unsigned long*)pTmpStackTop = (unsigned long)pUserStackTop;
		pTmpStackTop += sizeof(unsigned long);

		/* Reserve space for argv[] array. */
		argv = (char**)pTmpStackTop;
		pTmpStackTop += sizeof(unsigned long) * argc;
		pUserStackTop += sizeof(unsigned long) * argc;

		/* Copy all parameters into stack. */
		for (i = 0; i < argc; i++)
		{
			argv[i] = (char*)pUserStackTop;
			strncpy(pTmpStackTop, pszCmdLine, MAX_CMD_LEN);
			pTmpStackTop += strlen(pszCmdLine) + 1;
			pUserStackTop += strlen(pszCmdLine) + 1;
		}
	}
	else /* Use formated command object. */
	{
		argc = pCmdObject->byParameterNum;
		if (argc > 16)
		{
			/* At most 16 parameters are transferd to user. */
			argc = 16;
		}
		/*
		 * Push a NULL returen address to simulate a call
		 * instruction, so the user entry could access argc
		 * and argv[] use normal invoke syntax.
		 */
		*(unsigned long*)pTmpStackTop = 0;
		pTmpStackTop += sizeof(unsigned long);
		pUserStackTop += sizeof(unsigned long);

		/* Push argc. */
		*(unsigned long*)pTmpStackTop = argc;
		pTmpStackTop += sizeof(unsigned long);
		pUserStackTop += sizeof(unsigned long);

		/* Push argv. */
		pUserStackTop += sizeof(unsigned long);
		*(unsigned long*)pTmpStackTop = (unsigned long)pUserStackTop;
		pTmpStackTop += sizeof(unsigned long);

		/* Reserve space for argv[] array. */
		argv = (char**)pTmpStackTop;
		pTmpStackTop += sizeof(unsigned long) * argc;
		pUserStackTop += sizeof(unsigned long) * argc;

		/* Copy all parameters into stack. */
		int cmd_len = 0;
		for (i = 0; i < argc; i++)
		{
			argv[i] = (char*)pUserStackTop;
			strncpy(pTmpStackTop, pCmdObject->Parameter[i], CMD_PARAMETER_LEN);
			cmd_len = strlen(pCmdObject->Parameter[i]);
			if (cmd_len > CMD_PARAMETER_LEN)
			{
				cmd_len = CMD_PARAMETER_LEN;
			}
			pTmpStackTop += cmd_len + 1;
			pUserStackTop += cmd_len + 1;
		}
	}
	/* Copy the stack frame into user's space. */
	pvmmgr->UserMemoryCopy((__COMMON_OBJECT*)pvmmgr,
		pTmpStack, pUserStack, PAGE_SIZE, FALSE);

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (pTmpStack)
	{
		_hx_free(pTmpStack);
	}
	return bResult;
}
#endif //__I386__

/*
 * User memory space is initialized in this routine.
 * The user agent is maped into user space,so as the 
 * main thread could jump to it directly.
 */
static BOOL PrepareUserSpace(__PROCESS_OBJECT* pProcessObject)
{
	BOOL bResult = FALSE;

	/* Map user agent to this process. */
	if (!MapUserAgent(pProcessObject))
	{
		goto __TERMINAL;
	}

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Get a handle from handle array. */
static __HANDLE GetHandle(__PROCESS_OBJECT* pProcess, __COMMON_OBJECT* objectPtr)
{
	__HANDLE_ARRAY* pHandleArray = NULL;
	__HANDLE handle = INVALID_HANDLE_VALUE;
	__HANDLE_ARRAY_ELEMENT* pArrayElement = NULL;
	__HANDLE_ARRAY_ELEMENT* pPrev = NULL;
	unsigned long ulFlags;

	BUG_ON((NULL == pProcess) || (NULL == objectPtr));
	pHandleArray = &pProcess->handleArray;

	/* Must operate in atomic. */
	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	pArrayElement = pHandleArray->pRoot;
	while (pArrayElement)
	{
		for (int i = 0; i < MAX_HANDLE_OBJECT_PTR; i++)
		{
			handle++;
			if (NULL == pArrayElement->objectPtr[i])
			{
				/* Found a free slot. */
				pArrayElement->objectPtr[i] = objectPtr;
				pHandleArray->handleNum++;
				if (handle > pHandleArray->maxHandleValue)
				{
					pHandleArray->maxHandleValue = handle;
				}
				__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
				goto __TERMINAL;
			}
		}
		/* Iterate next array element. */
		pPrev = pArrayElement;
		pArrayElement = pArrayElement->pNext;
	}
	
	/* No free handle slot, allocate a new array element. */
	pArrayElement = (__HANDLE_ARRAY_ELEMENT*)_hx_malloc(
		sizeof(__HANDLE_ARRAY_ELEMENT));
	if (NULL == pArrayElement)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		handle = INVALID_HANDLE_VALUE;
		goto __TERMINAL;
	}
	/* Clear the element and link to list. */
	memset(pArrayElement, 0, sizeof(__HANDLE_ARRAY_ELEMENT));
	if (NULL == pPrev)
	{
		pHandleArray->pRoot = pArrayElement;
	}
	else
	{
		pPrev->pNext = pArrayElement;
	}
	pHandleArray->elementNum++;
	pHandleArray->handleNum++;
	handle++;
	if (handle > pHandleArray->maxHandleValue)
	{
		pHandleArray->maxHandleValue = handle;
	}
	pArrayElement->objectPtr[0] = objectPtr;
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);

__TERMINAL:
	return handle;
}

/* Return the kernel object ptr given it's handle. */
static __COMMON_OBJECT* GetObjectByHandle(__PROCESS_OBJECT* pProcess, __HANDLE handle)
{
	__HANDLE_ARRAY* pHandleArray = NULL;
	__COMMON_OBJECT* objectPtr = NULL;
	__HANDLE_ARRAY_ELEMENT* pArrayElement = NULL;
	unsigned long ulFlags = 0;

	BUG_ON((NULL == pProcess) || (INVALID_HANDLE_VALUE == handle));
	pHandleArray = &pProcess->handleArray;

	/* Must operate in atomic. */
	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	if (handle > pHandleArray->maxHandleValue)
	{
		/* Invalid handle's value. */
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	unsigned int eleIndex = (handle - 1) / MAX_HANDLE_OBJECT_PTR;
	if (eleIndex >= pHandleArray->elementNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Slide to the corresponding array element. */
	unsigned int eleOffset = (handle - 1) % MAX_HANDLE_OBJECT_PTR;
	pArrayElement = pHandleArray->pRoot;
	while (pArrayElement)
	{
		if (0 == eleIndex)
		{
			break;
		}
		eleIndex--;
		pArrayElement = pArrayElement->pNext;
	}
	if (NULL == pArrayElement)
	{
		/* No array element matches the given handle. */
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Located the corresponding array index. */
	objectPtr = pArrayElement->objectPtr[eleOffset];
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);

__TERMINAL:
	return objectPtr;
}

/*
 * A helper routine to shrink handle array's element,
 * release the unused array element to save kernel memory.
 */
static void ShrinkHandleArray(__PROCESS_OBJECT* pProcess)
{
	__HANDLE_ARRAY* pHandleArray = NULL;
	__HANDLE secndLast = INVALID_HANDLE_VALUE;
	__HANDLE currHandle = INVALID_HANDLE_VALUE;
	__HANDLE_ARRAY_ELEMENT* pArrayElement = NULL;
	unsigned long eleIndex = 0;
	/* How many array element should be released. */
	unsigned long freeEleNum = 0;
	/* How many array element should be preserved. */
	unsigned long reserveEleNum = 0;
	/* Start position in list that should be released. */
	__HANDLE_ARRAY_ELEMENT* pFreeStart = NULL;
	unsigned long ulFlags;
	unsigned int i = 0;

	BUG_ON(NULL == pProcess);
	pHandleArray = &pProcess->handleArray;

	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	pArrayElement = pHandleArray->pRoot;
	while (pArrayElement && (currHandle < pHandleArray->maxHandleValue))
	{
		/* Travel the whole list to find the second last handle value. */
		for (i = 0; i < MAX_HANDLE_OBJECT_PTR; i++)
		{
			if (pArrayElement->objectPtr[i])
			{
				currHandle = eleIndex * MAX_HANDLE_OBJECT_PTR;
				currHandle += (i + 1);
				if (currHandle != pHandleArray->maxHandleValue)
				{
					secndLast = currHandle;
				}
			}
		}
		eleIndex++;
		pArrayElement = pArrayElement->pNext;
	}
	/*
	 * Calculate how many handle array element should be released.
	 * Handle slot from 0 to second last should be reserved,all others
	 * can be released.
	 */
	reserveEleNum = (secndLast / MAX_HANDLE_OBJECT_PTR);
	reserveEleNum += ((secndLast % MAX_HANDLE_OBJECT_PTR) ? 1 : 0);
	BUG_ON(reserveEleNum > pHandleArray->elementNum);
	freeEleNum = pHandleArray->elementNum - reserveEleNum;
	/* Locate the start position that should be released. */
	pFreeStart = pArrayElement = pHandleArray->pRoot;
	for (i = 0; i < reserveEleNum; i++)
	{
		/* pArrayElement points to the last reserved element. */
		pArrayElement = pFreeStart;
		pFreeStart = pFreeStart->pNext;
	}
	/* Detach the free list from handle array list. */
	if (pArrayElement)
	{
		pArrayElement->pNext = NULL;
	}
	/* If all array elements should be released. */
	if (pFreeStart == pHandleArray->pRoot)
	{
		pHandleArray->pRoot = NULL;
	}
	/* Update handle array's state. */
	pHandleArray->elementNum = reserveEleNum;
	pHandleArray->maxHandleValue = secndLast;
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);

	/* Release the free handle element list. */
	while (pFreeStart)
	{
		pArrayElement = pFreeStart;
		pFreeStart = pFreeStart->pNext;
		_hx_free(pArrayElement);
	}
	return;
}

/* Destroy the kernel object denoted by it's handle. */
static int CloseHandle(__PROCESS_OBJECT* pProcess, __HANDLE handle)
{
	__HANDLE_ARRAY* pHandleArray = NULL;
	__COMMON_OBJECT* objectPtr = NULL;
	__HANDLE_ARRAY_ELEMENT* pArrayElement = NULL;
	unsigned long ulFlags = 0;
	int result = 0;

	BUG_ON((NULL == pProcess) || (INVALID_HANDLE_VALUE == handle));
	pHandleArray = &pProcess->handleArray;

	/* Must operate in atomic. */
	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	if (handle > pHandleArray->maxHandleValue)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	unsigned int eleIndex = (handle - 1) / MAX_HANDLE_OBJECT_PTR;
	if (eleIndex >= pHandleArray->elementNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	unsigned int eleOffset = (handle - 1) % MAX_HANDLE_OBJECT_PTR;
	/* Slide to the corresponding array element. */
	pArrayElement = pHandleArray->pRoot;
	while (pArrayElement)
	{
		if (0 == eleIndex)
		{
			break;
		}
		eleIndex--;
		pArrayElement = pArrayElement->pNext;
	}
	if (NULL == pArrayElement)
	{
		/* The corresponding handle array element is not exist. */
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/*
	 * Now we located the handle array element,just
	 * destroy the corresponding kernel object and free
	 * the handle array element slot.
	 */
	objectPtr = pArrayElement->objectPtr[eleOffset];
	if (NULL == objectPtr)
	{
		/* No object in this slot,may caused by invalid handle value. */
		__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Reset the slot. */
	pArrayElement->objectPtr[eleOffset] = NULL;
	pHandleArray->handleNum--;
	if (handle == pHandleArray->maxHandleValue)
	{
		/* Potential exists to shrink the handle array. */
		ShrinkHandleArray(pProcess);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);

	/* Destroy the corresponding kernel object. */
	ObjectManager.DestroyObject(&ObjectManager, objectPtr);
	result = 1;

__TERMINAL:
	return result;
}

/* 
 * Helper routine to destroy a handle array,all kernel 
 * objects associated with it will be destroyed at this
 * process.
 * It will be invoked when one process object is to be
 * destroyed.
 */
static void DestroyHandleArray(__PROCESS_OBJECT* pProcess)
{
	__HANDLE_ARRAY* pHandleArray = NULL;
	__HANDLE_ARRAY_ELEMENT* pArrayElementNext = NULL;
	__HANDLE_ARRAY_ELEMENT* pArrayElementCurr = NULL;
	__COMMON_OBJECT* pObjectPtr = NULL;
	unsigned long ulFlags;
	int i = 0;

	BUG_ON(NULL == pProcess);
	pHandleArray = &pProcess->handleArray;

	__ENTER_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
	pArrayElementNext = pHandleArray->pRoot;
	while (pArrayElementNext)
	{
		pArrayElementCurr = pArrayElementNext;
		pArrayElementNext = pArrayElementNext->pNext;
		/* Destroy all kernel objects in this element. */
		for (i = 0; i < MAX_HANDLE_OBJECT_PTR; i++)
		{
			pObjectPtr = pArrayElementCurr->objectPtr[i];
			if (pObjectPtr)
			{
				ObjectManager.DestroyObject(&ObjectManager, pObjectPtr);
			}
		}
		/* Release the memory of this array element. */
		KMemFree(pArrayElementCurr, KMEM_SIZE_TYPE_ANY, 0);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pProcess->spin_lock, ulFlags);
}

#endif
