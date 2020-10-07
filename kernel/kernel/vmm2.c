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

/* Check if a specified address is in range. */
#define ADDRESS_IN_RANGE(addr, start, end) \
	(((unsigned long)(addr) >= (unsigned long)(start)) && \
	((unsigned long)(addr) <= (unsigned long)(end)))

/*
 * A helper routine to calculate the intersection part of
 * 2 virtual area.
 * The intersection size will be returned if these 2 VA
 * overlaps,and the start address of overlaped area will
 * be filled into overlap_start.
 * 0 wil be returned if no intersection.
 */
static unsigned long VirtualAreaInterSection(__VIRTUAL_AREA_DESCRIPTOR* pVa,
	unsigned long range_start,
	unsigned long range_end,
	unsigned long* overlap_start)
{
	unsigned long gap_start = 0, gap_end = 0;
	unsigned long overlap_size = 0;

	/* Validates parameters. */
	BUG_ON((NULL == pVa) || (NULL == overlap_start));
	BUG_ON(range_end <= range_start);

	if (0 == pVa->ulSize)
	{
		goto __TERMINAL;
	}
	gap_start = (unsigned long)pVa->lpStartAddr;
	gap_end = pVa->ulSize;
	gap_end -= 1;
	gap_end += gap_start;
	BUG_ON(gap_end <= gap_start);

	if ((gap_start >= range_end) || (gap_end <= range_start))
	{
		/* No overlaped section. */
		goto __TERMINAL;
	}
	if ((gap_start >= range_start) && (gap_end <= range_end))
	{
		/* Range covers the whole virtual area. */
		*overlap_start = gap_start;
		overlap_size = gap_end - gap_start + 1;
		goto __TERMINAL;
	}
	if ((range_start >= gap_start) && (range_end <= gap_end))
	{
		/* The virtual area covers the whole range. */
		*overlap_start = range_start;
		overlap_size = range_end - range_start + 1;
		goto __TERMINAL;
	}
	if ((gap_start >= range_start) && (gap_end >= range_end))
	{
		/* Upper overlaps. */
		*overlap_start = gap_start;
		overlap_size = range_end - gap_start + 1;
		goto __TERMINAL;
	}
	if ((gap_start < range_start) && (gap_end < range_end))
	{
		/* Bottom overlaps. */
		*overlap_start = range_start;
		overlap_size = gap_end - range_start + 1;
		goto __TERMINAL;
	}

__TERMINAL:
	return overlap_size;
}

/*
 * Find a free gap in the given gap in lineary memory space.
 * The lpDesiredAddr is prefered when available in space,other
 * wise this routine will return a free gap that fits into
 * the range specifed by range_start and range_end,but with
 * a start address different than lpDesiredAddr.
 * If bMustSatisfied is TRUE,then the routine will fail if
 * the lpDesiredAddr could not satisfied.
 * This routine must be used with InsertVirtualArea routine
 * as pair and must be protected by spin lock in virtual memory
 * manager object.
 */
static LPVOID __SearchVirtualArea(__VIRTUAL_MEMORY_MANAGER* pVmmMgr,
	unsigned long range_start,
	unsigned long range_end,
	LPVOID pDesiredAddr,
	size_t size,
	BOOL bMustSatisfied)
{
	__VIRTUAL_AREA_DESCRIPTOR* pVaRoot = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* pVaNext = NULL;
	long potential_addr = -1;
	unsigned long intersection_sz = 0;
	unsigned long intersection_start = 0;
	__VIRTUAL_AREA_DESCRIPTOR vad, vad_desired;
	unsigned long desired_start = 0;

	/* Parameters checking. */
	BUG_ON(NULL == pVmmMgr);
	BUG_ON((0 == size) || (!PAGE_ALIGNED(size)));
	BUG_ON(!PAGE_ALIGNED(range_start));
	BUG_ON(range_end <= range_start);
	BUG_ON(size > range_end - range_start);
	if (pDesiredAddr)
	{
		/* Desired address must be in the specified range if given. */
		BUG_ON(!ADDRESS_IN_RANGE(pDesiredAddr, range_start, range_end));
		BUG_ON(!ADDRESS_IN_RANGE((unsigned long)pDesiredAddr + size, range_start, range_end));
		/* Construct a persude desired VAD for simplify programming. */
		vad_desired.lpNext = NULL;
		vad_desired.lpStartAddr = pDesiredAddr;
		vad_desired.ulSize = size;
	}

	pVaRoot = pVmmMgr->lpListHdr;
	if (NULL == pVaRoot)
	{
		/* No virtual area in list yet,whole space is free. */
		return pDesiredAddr;
	}

	/*
	 * Check if the first gap between 0 and the first VAD
	 * in list could satisfy original requirement,if it exists.
	 */
	if ((unsigned long)pVaRoot->lpStartAddr > 0)
	{
		vad.lpStartAddr = NULL;
		vad.ulSize = (unsigned long)pVaRoot->lpStartAddr;
		/*
		 * Calculate the intersection part between the first
		 * free gap VA and user specified range.
		 */
		intersection_sz = VirtualAreaInterSection(&vad, range_start, range_end, &intersection_start);
#if 0
		_hx_printf("%s:intersection_sz = 0x%X, intersection_start = 0x%X, req_sz = 0x%X\r\n", __FUNCTION__,
			intersection_sz,
			intersection_start,
			size);
#endif
		if (intersection_sz >= size)
		{
			/*
			 * The length of intersection part between range and
			 * gap COULD satisfied the requirement.
			 */
			potential_addr = intersection_start;
			if (NULL == pDesiredAddr)
			{
				/* No intention address specified. */
				goto __TERMINAL;
			}
			/* Check if the intersection could satisify desired. */
			desired_start = 0;
			if (VirtualAreaInterSection(&vad_desired,
				intersection_start,
				(intersection_start + intersection_sz - 1),
				&desired_start) >= size)
			{
				potential_addr = desired_start;
				goto __TERMINAL;
			}
		}
	}

	/* Search every free gap in memory space. */
	while (pVaRoot)
	{
		pVaNext = pVaRoot->lpNext;
		/* Construct a persude VA for the gap betwen root and next. */
		if (pVaNext)
		{
			vad.lpStartAddr = (LPVOID)((unsigned long)pVaRoot->lpStartAddr + pVaRoot->ulSize);
			vad.ulSize = (unsigned long)pVaNext->lpStartAddr - (unsigned long)vad.lpStartAddr;
		}
		else
		{
			vad.lpStartAddr = (LPVOID)((unsigned long)pVaRoot->lpStartAddr + pVaRoot->ulSize);
			vad.ulSize = (unsigned long)0 - (unsigned long)vad.lpStartAddr;
		}
		/* Calculate the intersection part of gap VA and specified range. */
		intersection_sz = VirtualAreaInterSection(&vad, range_start, range_end, &intersection_start);
#if 0
		_hx_printf("%s:intersection_sz = 0x%X, intersection_start = 0x%X, req_sz = 0x%X\r\n", __FUNCTION__,
			intersection_sz,
			intersection_start,
			size);
#endif
		if (intersection_sz >= size)
		{
			/*
			 * The length of intersection part between range and
			 * gap COULD satisfied the requirement.
			 */
			potential_addr = intersection_start;
			if (NULL == pDesiredAddr)
			{
				/* No intention address specified. */
				goto __TERMINAL;
			}
			/* Check if the intersection could satisify desired. */
			desired_start = 0;
			if (VirtualAreaInterSection(&vad_desired,
				intersection_start,
				(intersection_start + intersection_sz - 1),
				&desired_start) >= size)
			{
				potential_addr = desired_start;
				goto __TERMINAL;
			}
		}
		/* Check next gap sapce. */
		pVaRoot = pVaNext;
	}
	if (bMustSatisfied)
	{
		/* Could not satisfy the original request. */
		if (potential_addr != (long)pDesiredAddr)
		{
			_hx_printf("%s:potential_addr = 0x%X, desired_addr = 0x%X\r\n",
				__func__,
				potential_addr, (long)pDesiredAddr);
			potential_addr = -1;
		}
	}

	/* Just for debugging. */
	if (-1 == potential_addr)
	{
		_hx_printf("%s:No fit virtual memory address found.\r\n", __func__);
	}

__TERMINAL:
	return (LPVOID)potential_addr;
}

/*
 * Insert a virtual area descriptor into a process's
 * virtual area list.
 * This routine must be invoked with SearchVirtualArea
 * together as a pair,in atomic operation protected by
 * spin lock.
 */
static BOOL InsertVirtualArea(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, __VIRTUAL_AREA_DESCRIPTOR* pVa)
{
	BOOL bResult = FALSE;
	__VIRTUAL_AREA_DESCRIPTOR* pVaRoot = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* pVaPrev = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* pVaNext = NULL;

	/* Validates parameters. */
	BUG_ON((NULL == pVa) || (NULL == pVmmMgr));
	BUG_ON(!PAGE_ALIGNED(pVa->lpStartAddr));
	BUG_ON(!PAGE_ALIGNED(pVa->ulSize));

	pVaRoot = pVmmMgr->lpListHdr;
	if (NULL == pVaRoot)
	{
		/* The first virtual area object. */
		pVmmMgr->lpListHdr = pVa;
		__ATOMIC_INCREASE(&pVmmMgr->va_num);
		bResult = TRUE;
		goto __TERMINAL;
	}

	/* Travel the whole list to locates inserting position. */
	pVaPrev = pVaNext = pVaRoot;
	if ((unsigned long)pVaNext->lpStartAddr > (unsigned long)pVa->lpStartAddr)
	{
		/* Insert the VA into virtual memory manager's list as first one. */
		pVa->lpNext = pVaNext;
		pVmmMgr->lpListHdr = pVa;
		__ATOMIC_INCREASE(&pVmmMgr->va_num);
		bResult = TRUE;
		goto __TERMINAL;
	}
	while (pVaNext)
	{
		if ((unsigned long)pVaNext->lpStartAddr < (unsigned long)pVa->lpStartAddr)
		{
			pVaPrev = pVaNext;
			pVaNext = pVaNext->lpNext;
		}
		else
		{
			/* Found the proper location. */
			break;
		}
	}
	pVaPrev->lpNext = pVa;
	pVa->lpNext = pVaNext;
	__ATOMIC_INCREASE(&pVmmMgr->va_num);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* A helper macro to compare 2 VADs. */
#define IS_SAME_VAD(vad1, vad2) \
	(((unsigned long)(vad1).lpStartAddr == (unsigned long)(vad2).lpStartAddr) && \
	((vad1).ulSize == (vad2).ulSize))

/*
 * Delete a VAD from virtual memory manager's list.
 * TRUE will be returned if the VAD is located and removed
 * from list,FALSE otherwise.
 */
static BOOL DeleteVirtualArea(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, __VIRTUAL_AREA_DESCRIPTOR* pVad)
{
	__VIRTUAL_AREA_DESCRIPTOR* pVadNext = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* pVadRoot = NULL;
	BOOL bResult = FALSE;

	/* Validates parameters. */
	BUG_ON((NULL == pVmmMgr) || (NULL == pVad));
	BUG_ON(!PAGE_ALIGNED(pVad->lpStartAddr));
	BUG_ON(!PAGE_ALIGNED(pVad->ulSize));

	pVadRoot = pVmmMgr->lpListHdr;
	if (NULL == pVadRoot)
	{
		goto __TERMINAL;
	}
	if (pVad == pVadRoot)
	{
		/* The VAD to be deleted is the first one. */
		BUG_ON(!IS_SAME_VAD(*pVadRoot, *pVad));
		pVmmMgr->lpListHdr = pVadRoot->lpNext;
		__ATOMIC_DECREASE(&pVmmMgr->va_num);
		bResult = TRUE;
		goto __TERMINAL;
	}
	/* Locate the VAD in list. */
	while (pVadRoot)
	{
		pVadNext = pVadRoot->lpNext;
		if (NULL == pVadNext)
		{
			goto __TERMINAL;
		}
		if (pVad == pVadNext)
		{
			BUG_ON(!IS_SAME_VAD(*pVadNext, *pVad));
			pVadRoot->lpNext = pVad->lpNext;
			__ATOMIC_DECREASE(&pVmmMgr->va_num);
			bResult = TRUE;
			goto __TERMINAL;
		}
		pVadRoot = pVadNext;
	}

__TERMINAL:
	return bResult;
}

/*
 * Helper macro to check if a given memory block falls
 * into the specified virtual area by vad.
 */
#define MEMORY_BLOCK_IN_VIRTUAL_AREA(vad, start_addr, mem_size) \
	(((unsigned long)(start_addr) >= (unsigned long)((vad)->lpStartAddr)) && \
	((unsigned long)(start_addr) + mem_size <= (unsigned long)((vad)->lpStartAddr) + (vad)->ulSize))

 /*
  * Remove a virtual area that contains the specified memory block
  * from virtual memory's list and return it back.
  * NULL will be returned if no matched virtual area found.
  */
static __VIRTUAL_AREA_DESCRIPTOR* RemoveVirtualArea(__VIRTUAL_MEMORY_MANAGER* pVmmMgr,
	LPVOID pStartAddr,
	size_t va_size)
{
	__VIRTUAL_AREA_DESCRIPTOR* pVadRoot = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* pVadNext = NULL;

	BUG_ON(NULL == pVmmMgr);
	pVadRoot = pVmmMgr->lpListHdr;

	/* The first one matches the request. */
	if (MEMORY_BLOCK_IN_VIRTUAL_AREA(pVadRoot, pStartAddr, va_size))
	{
		pVmmMgr->lpListHdr = pVadRoot->lpNext;
		pVadNext = pVadRoot;
		__ATOMIC_DECREASE(&pVmmMgr->va_num);
		goto __TERMINAL;
	}

	/* Search the whole list to locate the desired one. */
	pVadNext = pVadRoot->lpNext;
	while (pVadNext)
	{
		if (MEMORY_BLOCK_IN_VIRTUAL_AREA(pVadNext, pStartAddr, va_size))
		{
			pVadRoot->lpNext = pVadNext->lpNext;
			__ATOMIC_DECREASE(&pVmmMgr->va_num);
			goto __TERMINAL;
		}
		pVadRoot = pVadNext;
		pVadNext = pVadNext->lpNext;
	}

__TERMINAL:
	return pVadNext;
}

/*
 * A helper routine,used to search a proper virtual area gap in the
 * Virtual Memory Space,the "l" menas it searchs in the virtual area list.
 * CAUTION: This routine is not safe,it's the caller's responsibility to guarantee the
 * safety.
 * Please be noted that VIRTUAL_MEMORY_END will be returned in case of
 * failure,since the NULL address also available for desired address.
 */
#if 0
/* Old one,obsoleted. */
static LPVOID __SearchVirtualArea_l(__COMMON_OBJECT* lpThis, LPVOID lpDesiredAddr, DWORD dwSize)
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
#endif

/* Just wrappers of underlay SearchVirtualArea routine. */
static LPVOID SearchVirtualArea(__COMMON_OBJECT* lpThis, LPVOID lpDesiredAddr, 
	DWORD dwSize,
	unsigned long ulAllocFlags)
{
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	unsigned long range_start = 0, range_end = -1;

	BUG_ON(NULL == pVmmMgr);
	BUG_ON(0 == dwSize);

	/* Set range accordingly. */
	switch (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK)
	{
	case VIRTUAL_AREA_ALLOCATE_USERSPACE:
		/* Allocate from user space. */
		range_start = KMEM_USERSPACE_START;
		range_end = KMEM_USERSPACE_END - 1;
		break;
	case VIRTUAL_AREA_ALLOCATE_KERNELSPACE:
		/* Allocate from kernel space. */
		break;
	case VIRTUAL_AREA_ALLOCATE_IOSPACE:
		/* Allocate from IO space. */
		break;
	case VIRTUAL_AREA_ALLOCATE_STACK:
		/* Allocate from stack space. */
		range_start = KMEM_USERSTACK_START;
		range_end = KMEM_USERSTACK_END - 1;
		break;
	default:
		//BUG_ON(TRUE);
		break;
	}
	return __SearchVirtualArea(pVmmMgr, range_start, range_end, lpDesiredAddr, dwSize, FALSE);
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
static VOID __InsertIntoList(__COMMON_OBJECT* lpThis, __VIRTUAL_AREA_DESCRIPTOR* lpVad)
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
		__ATOMIC_INCREASE(&lpMemMgr->va_num);
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
		__ATOMIC_INCREASE(&lpMemMgr->va_num);
	}
	else
	{
		lpVad->lpNext = lpSecond->lpNext;
		lpSecond->lpNext = lpVad;
		__ATOMIC_INCREASE(&lpMemMgr->va_num);
	}
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
static __VIRTUAL_AREA_DESCRIPTOR* GetVirtualArea(__COMMON_OBJECT* lpThis, LPVOID lpAddr)
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
 * Map a block of physical memory into virtual space that
 * managed by pThis(points to a virtual memory manager object).
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
	lpVad->ulSize = length;
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
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		lpStartAddr = SearchVirtualArea((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, 
			length,
			VIRTUAL_AREA_ALLOCATE_MAP);
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
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		InsertVirtualArea(lpMemMgr, lpVad);
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

/* Simulation of sbrk routine of POSIX. */
static void* __sbrk(__COMMON_OBJECT* pMemManager, size_t increment)
{
	return NULL;
}

/* 
 * Return the attributes of a block of 
 * virtual memory allocated by VirtualAlloc routine. 
 * The pMemInfo object's size(__MEMORY_BASIC_INFORMATION's size)
 * will be returned if success,0 will be returned if
 * failure encountered.
 */
static size_t __VirtualQuery(__COMMON_OBJECT* lpThis,
	LPVOID pStartAddr,
	__MEMORY_BASIC_INFORMATION* pMemInfo,
	size_t info_sz)
{
	__VIRTUAL_MEMORY_MANAGER* pVmmMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	size_t ret_val = 0;
	__VIRTUAL_AREA_DESCRIPTOR* pVad = NULL;
	unsigned long ulFlags = 0;

	/* Parameters checking. */
	BUG_ON(NULL == pVmmMgr);
	BUG_ON((NULL == pMemInfo) || (0 == info_sz));

	/* Must not be interrupted. */
	__ENTER_CRITICAL_SECTION_SMP(pVmmMgr->spin_lock, ulFlags);
	pVad = GetVirtualArea(lpThis, pStartAddr);
	if (NULL == pVad)
	{
		/* No matching VAD found. */
		__LEAVE_CRITICAL_SECTION_SMP(pVmmMgr->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	/* Found the VAD,set memory basic information accordingly. */
	pMemInfo->AllocationBase = pVad->lpStartAddr;
	pMemInfo->BaseAddress = pVad->lpStartAddr;
	pMemInfo->RegionSize = pVad->ulSize;
	pMemInfo->State = pVad->dwAllocFlags;
	pMemInfo->Type = 0;
	pMemInfo->Protect = 0;
	__LEAVE_CRITICAL_SECTION_SMP(pVmmMgr->spin_lock, ulFlags);
	/* Set return value. */
	ret_val = sizeof(__MEMORY_BASIC_INFORMATION);
	goto __TERMINAL;

__TERMINAL:
	return ret_val;
}
