//***********************************************************************/
//    Author                    : Garry
//    Original Date             : March 25,2019
//    Module Name               : vmm2.c
//    Module Funciton           : 
//                                Second part of virtual memory manager object's
//                                implementation,this file is included by vmm.c.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <mlayout.h>
#include <stdio.h>

/* Check if a virtual address is within a virtual area. */
#define WITHIN_VIRTUAL_AREA(lpAddr,lpVa) \
	(((DWORD)((lpVa)->lpStartAddr) <= (DWORD)lpAddr) && \
     ((DWORD)((lpVa)->lpEndAddr)   >= (DWORD)lpAddr))

/*
 * Check if a virtual memory address is in the hole
 * between 2 virtual areas.
 */
#define BETWEEN_VIRTUAL_AREA(lpAddr,lpVa1,lpVa2) \
	(((DWORD)((lpVa1)->lpEndAddr) < (DWORD)lpAddr) && \
	 ((DWORD)((lpVa2)->lpStartAddr) > (DWORD)lpAddr))

 /* Check if a virtual memory address is between two virtual addresses. */
#define BETWEEN_VIRTUAL_ADDRESS(lpAddr,lpAddr1,lpAddr2) \
	(((DWORD)lpAddr1 <= (DWORD)lpAddr) && \
	 ((DWORD)lpAddr2 >=  (DWORD)lpAddr))

/*
 * A helper routine,used to search a proper virtual area gap in the
 * Virtual Memory Space,the "l" menas it searchs in the virtual area list.
 * CAUTION: This routine is not safe,it's the caller's responsibility to guarantee the
 * safety.
 * Please be noted that VIRTUAL_MEMORY_END will be returned in case of
 * failure,since the NULL address also available for desired address.
 */
static LPVOID SearchVirtualArea_l(__COMMON_OBJECT* lpThis, LPVOID lpDesiredAddr, DWORD dwSize)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	LPVOID pGapStartAddr = NULL;
	LPVOID pGapEndAddr = NULL;
	LPVOID lpDesiredEnd = NULL;
	LPVOID lpPotentialStart = NULL;
	LPVOID lpPotentialEnd = NULL;
	BOOL bFind = FALSE;

	/* Parameters checking. */
	BUG_ON((NULL == lpThis) || (0 == dwSize));
	lpVad = lpMemMgr->lpListHdr;
	if (NULL == lpVad)
	{
		/*
		 * No virtual area allocated yet,any desired
		 * address is OK.
		 */
		return lpDesiredAddr;
	}

	/* Search virtual memory space gaps one by one. */
	pGapStartAddr = NULL;
	pGapEndAddr = (LPVOID)((unsigned long)lpVad->lpStartAddr - 1);
	lpDesiredEnd = (LPVOID)((unsigned long)lpDesiredAddr + dwSize - 1);
	while (lpVad)
	{
		if (BETWEEN_VIRTUAL_ADDRESS(lpDesiredAddr, pGapStartAddr, pGapEndAddr) &&
			BETWEEN_VIRTUAL_ADDRESS(lpDesiredEnd, pGapStartAddr, pGapEndAddr))
		{
			/* The desired region fall into gap,so it's free. */
			return lpDesiredAddr;
		}
		if (!bFind)
		{
			/* check if the gap can statisfy the required size. */
			lpPotentialStart = pGapStartAddr;
			lpPotentialEnd = (LPVOID)((unsigned long)lpPotentialStart + dwSize - 1);
			if (BETWEEN_VIRTUAL_ADDRESS(lpPotentialEnd, pGapStartAddr, pGapEndAddr))
			{
				/* Find a potential VA that can statisfy the original required size. */
				bFind = TRUE;
			}
		}
		/* Check next space GAP. */
		pGapStartAddr = (LPVOID)((DWORD)lpVad->lpEndAddr + 1);
		lpVad = lpVad->lpNext;
		if (lpVad)
		{
			pGapEndAddr = (LPVOID)((unsigned long)lpVad->lpStartAddr - 1);
		}
		else
		{
			pGapEndAddr = (LPVOID)VIRTUAL_MEMORY_END;
		}
	}
	/*
	 * Check the last GAP,since it's not checked in
	 * above while loop(out of loop by cond "lpVad == NULL").
	 */
	if (BETWEEN_VIRTUAL_ADDRESS(lpDesiredAddr, pGapStartAddr, pGapEndAddr) &&
		BETWEEN_VIRTUAL_ADDRESS(lpDesiredEnd, pGapStartAddr, pGapEndAddr))
	{
		/* The desired region fall into gap,so it's free. */
		return lpDesiredAddr;
	}
	if (!bFind)
	{
		/* check if the gap can statisfy the required size. */
		lpPotentialStart = pGapStartAddr;
		lpPotentialEnd = (LPVOID)((unsigned long)lpPotentialStart + dwSize - 1);
		if (BETWEEN_VIRTUAL_ADDRESS(lpPotentialEnd, pGapStartAddr, pGapEndAddr))
		{
			/* Find a potential VA that can statisfy the original required size. */
			bFind = TRUE;
		}
	}

	if (bFind)
	{
		/*
		 * Have found a virtual area that statisfy the
		 * original request,though it's start address is
		 * not the desired one.
		 */
		return lpPotentialStart;
	}
	/* Otherwise no suitable range found for the request. */
	return (LPVOID)VIRTUAL_MEMORY_END;
}

//
//SearchVirtualArea_t is a same routine as SearchVirtualArea_l,the difference is,this
//routine searchs in the AVL tree.
//
static LPVOID SearchVirtualArea_t(__COMMON_OBJECT* lpThis, LPVOID lpStartAddr, DWORD dwSize)
{
	return (LPVOID)VIRTUAL_MEMORY_END;    //We will implement it in the future :-)
}

/*
 * Inserts a virtual area descriptor object into a process's
 * virtual area list.
 */
static VOID InsertIntoList(__COMMON_OBJECT* lpThis, __VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR* lpFirst = NULL, *lpSecond = NULL;

	BUG_ON((NULL == lpThis) || (NULL == lpVad));
	lpFirst = lpMemMgr->lpListHdr;
	if (NULL == lpFirst)
	{
		/* No element in the list. */
		lpMemMgr->lpListHdr = lpVad;
		lpVad->lpNext = NULL;
		lpMemMgr->dwVirtualAreaNum++;
		return;
	}
	lpSecond = lpFirst;
	while (lpFirst)
	{
		/* Locate the proper position. */
		if ((DWORD)lpFirst->lpStartAddr > (DWORD)lpVad->lpStartAddr)
		{
			break;
		}
		lpSecond = lpFirst;
		lpFirst = lpFirst->lpNext;
	}
	if (lpSecond == lpFirst)
	{
		/* Should be the first element in the list. */
		lpVad->lpNext = lpMemMgr->lpListHdr;
		lpMemMgr->lpListHdr = lpVad;
		lpMemMgr->dwVirtualAreaNum++;
		return;
	}
	else
	{
		lpVad->lpNext = lpSecond->lpNext;
		lpSecond->lpNext = lpVad;
	}
	lpMemMgr->dwVirtualAreaNum++;
}

//
//InsertIntoTree routine,the same as above except that this routine is used to insert
//a virtual memory area into AVL tree.
//
static VOID InsertIntoTree(__COMMON_OBJECT* lpThis, __VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	return;  //We will implement this routine in the future. :-)
}

/*
 * A helper routine used to get a virtual area descriptor object by
 * specifying a virtual address.
 * The label "l" means the search target is list.
 * It's the caller's responsibility that guarantee the racing,since
 * this routine does not apply spin lock.
 */
static __VIRTUAL_AREA_DESCRIPTOR* GetVaByAddr_l(__COMMON_OBJECT* lpThis, LPVOID lpAddr)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;

	BUG_ON(NULL == lpThis);

	lpVad = lpMemMgr->lpListHdr;
	while (lpVad)
	{
		if (WITHIN_VIRTUAL_AREA(lpAddr, lpVad))
		{
			return lpVad;
		}
		lpVad = lpVad->lpNext;
	}
	/* NULL will be returned if reach here. */
	return lpVad;
}

//
//Get a virtual area descriptor object from AVL tree according to a virtual 
//memory address.
//
static __VIRTUAL_AREA_DESCRIPTOR* GetVaByAddr_t(__COMMON_OBJECT* lpThis, LPVOID lpAddr)
{
	return NULL;  //We will complete this routine in the future. :-)
}

/*
 * Move memory between user and kernel space.
 * Copy memory content from user to kernel if bUserToKernel 
 * is TRUE,otherwise copy memory content from kernel to user.
 * The architecture specific routines,__copy_from_user and
 * __copy_to_user are invoked to support the memory moving
 * operation.
 */
static BOOL __UserMemoryCopy(__COMMON_OBJECT* pThis,
	LPVOID pKernelStart,
	LPVOID pUserStart,
	unsigned long length,
	BOOL bUserToKernel)
{
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = (__VIRTUAL_MEMORY_MANAGER*)pThis;
	unsigned long remaind = length;

	/* Validates the specified address. */
	BUG_ON(!KMEM_IN_KERNEL_SPACE(pKernelStart));
	BUG_ON(!KMEM_IN_KERNEL_SPACE((unsigned long)pKernelStart + length));
	BUG_ON(!KMEM_IN_USER_SPACE(pUserStart));
	BUG_ON(!KMEM_IN_USER_SPACE((unsigned long)pUserStart + length));

	/* 
	 * Call the arch specific memory moving routine.
	 * Copy at most PAGE_SIZE memory each time,since the
	 * underlay routines could not copy more than PAGE_SIZE
	 * one time.
	 */
	while (remaind > PAGE_SIZE)
	{
		if (bUserToKernel)
		{
			__copy_from_user(pVmmMgr, pKernelStart, pUserStart, PAGE_SIZE);
		}
		else
		{
			__copy_to_user(pVmmMgr, pKernelStart, pUserStart, PAGE_SIZE);
		}
		remaind -= PAGE_SIZE;
	}
	/* Copy the rest if any. */
	if (remaind)
	{
		if (bUserToKernel)
		{
			__copy_from_user(pVmmMgr, pKernelStart, pUserStart, remaind);
		}
		else
		{
			__copy_to_user(pVmmMgr, pKernelStart, pUserStart, remaind);
		}
	}
	return TRUE;
}

/*
 * Map a block of physical memory into virtual space.
 * The virtual and physical addresses must be specified,
 * the physical address will be maped to virtual address.
 * FALSE will return if fail,the virtual space is occupied
 * maybe the most possible reason.
 */
static BOOL __VirtualMap(__COMMON_OBJECT* pThis, LPVOID pVirtualAddr,
	LPVOID pPhysicalAddr, unsigned long length, unsigned long access_flags,
	char* va_name)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)pThis;
	LPVOID lpStartAddr = pVirtualAddr;
	LPVOID lpEndAddr = NULL;
	DWORD dwFlags = 0;
	LPVOID lpPhysical = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	DWORD dwPteFlags = 0;
	BOOL bResult = FALSE;

	BUG_ON((NULL == lpMemMgr) || (0 == length));
	/* 
	 * Length must be multiply of page size,and the 
	 * start virtual address must be page aligned.
	 */
	BUG_ON(length % PAGE_SIZE); 
	BUG_ON((unsigned long)pVirtualAddr % PAGE_SIZE);
	BUG_ON(VIRTUAL_MEMORY_END - length < (unsigned long)pVirtualAddr);
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	/* Create a virtual area object to manage this range. */
	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY);
	if (NULL == lpVad)
	{
		_hx_printf("[%s]:Out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	lpVad->lpManager = lpMemMgr;
	lpVad->lpStartAddr = pVirtualAddr;
	lpVad->lpEndAddr = (LPVOID)((unsigned long)pVirtualAddr + length - 1);
	lpVad->lpNext = NULL;
	lpVad->dwAccessFlags = access_flags;
	lpVad->dwAllocFlags = VIRTUAL_AREA_ALLOCATE_MAP;
	INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft = NULL;
	lpVad->lpRight = NULL;
	/* Copy virtual area name. */
	if (va_name)
	{
		if (StrLen(va_name) > MAX_VA_NAME_LEN)
			va_name[MAX_VA_NAME_LEN - 1] = 0;
		StrCpy((LPSTR)va_name, (LPSTR)&lpVad->strName[0]);
	}
	else
	{
		lpVad->strName[0] = 0;
	}
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_NORMAL;

	/*
	 * Check if the virtual address is occupied yet.
	 * Just give up if not free,and insert the VA into
	 * list or tree otherwise.
	 */
	lpStartAddr = pVirtualAddr;
	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)
	{
		lpStartAddr = SearchVirtualArea_l((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, length);
	}
	else
	{
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, length);
	}
	if (lpStartAddr != pVirtualAddr)
	{
		/* The specified range is occupied,give up. */
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* The range is free,just save it. */
	if (lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)
	{
		InsertIntoList((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}
	else
	{
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}

	/* Reserve corresponding page table entries. */
	dwPteFlags = 0;
	if (access_flags == VIRTUAL_AREA_ACCESS_READ)
	{
		dwPteFlags |= PTE_FLAGS_FOR_RO;
	}
	else
	{
		dwPteFlags |= PTE_FLAGS_FOR_NORMAL;
	}
	unsigned long total_length = length;
	lpPhysical = pPhysicalAddr;
	while (total_length)
	{
		if (!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr, lpPhysical, dwPteFlags))
		{
			/*
			 * Out of memory may lead the failure of page reserving,
			 * since page table itself also need physical memory.
			 */
			__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		total_length -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical = (LPVOID)((DWORD)lpPhysical + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
	}
	return bResult;
}

/*
 * A helper routine that release the virtual area that
 * is created by VirtualMap.
 */
static VOID ReleaseMap(__COMMON_OBJECT* lpThis,
	__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID lpStartAddr = NULL;
	LPVOID lpEndAddr = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	unsigned long dwSize = 0;

	BUG_ON((NULL == lpThis) || (NULL == lpVad));

	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	lpStartAddr = lpVad->lpStartAddr;
	lpEndAddr = lpVad->lpEndAddr;
	dwSize = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;

	/* Release page tables. */
	while (dwSize)
	{
		lpIndexMgr->ReleasePage((__COMMON_OBJECT*)lpIndexMgr, lpStartAddr);
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		dwSize -= PAGE_FRAME_SIZE;
	}
}
