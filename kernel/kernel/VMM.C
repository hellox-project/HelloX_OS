//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : vmm.c
//    Module Funciton           : 
//                                Virtual memory management functions and
//                                objects,are implemented in this file.
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

/* 
 * Virtual memory management function only available 
 * when this flag is defined. 
 */
#ifdef __CFG_SYS_VMM

/* Include other part of VMM. */
#include "vmm2.c"

/*
 * This routine is used to debug the virtual memory manager.
 * It prints out all virtual areas the current memory manager has.
 */
VOID PrintVirtualArea(__VIRTUAL_MEMORY_MANAGER* lpMemMgr)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	LPVOID lpAddr = NULL;

	BUG_ON(NULL == lpMemMgr);
	lpVad = lpMemMgr->lpListHdr;
	_hx_printf("  Virtuam memory manager's virtual area list:\r\n");
	while(lpVad)
	{
		_hx_printf("    start_addr:%08X, end_addr:%08X, length:%X, name:%s\r\n",
			(unsigned long)lpVad->lpStartAddr,
			(unsigned long)lpVad->lpEndAddr,
			(unsigned long)lpVad->lpEndAddr - (unsigned long)lpVad->lpStartAddr + 1,
			lpVad->strName);
		lpVad = lpVad->lpNext;
	}
}

/* Pre-declaration of local(member) routines. */
static LPVOID __VirtualAlloc(__COMMON_OBJECT*, LPVOID, DWORD, DWORD, DWORD, char*, LPVOID);
static LPVOID GetPdAddress(__COMMON_OBJECT*);
static BOOL __VirtualFree(__COMMON_OBJECT*, LPVOID);
static LPVOID _GetPhysicalAddress(__COMMON_OBJECT*, LPVOID);

/* Initializer of Virtual Memory Manager object. */
BOOL VmmInitialize(__COMMON_OBJECT* lpThis)
{
	__VIRTUAL_MEMORY_MANAGER* lpManager = NULL;
	__PAGE_INDEX_MANAGER* lpPageIndexMgr = NULL;
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	BOOL bResult = FALSE;
	LPVOID pKernelSpace = NULL;

	BUG_ON(NULL == lpThis);

	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	lpManager->VirtualAlloc = __VirtualAlloc;
	lpManager->VirtualFree = __VirtualFree;
	lpManager->VirtualMap = __VirtualMap;
	lpManager->GetPdAddress = GetPdAddress;
	lpManager->GetPhysicalAddress = _GetPhysicalAddress;
	lpManager->UserMemoryCopy = __UserMemoryCopy; /* In vmm2.c file. */
	__ATOMIC_INIT(&lpManager->va_num);
	lpManager->lpListHdr = NULL;
	lpManager->lpTreeRoot = NULL;
	lpManager->heap_sz = 0;
	lpManager->sbrk = __sbrk;
	lpManager->VirtualQuery = __VirtualQuery;

#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpManager->spin_lock, "vmmmgr");
#endif

	/* 
	 * Create and initialize the corresponding page index manager 
	 * object. Page mechanism for CPU are implemented in page index
	 * manager object.
	 */
	lpPageIndexMgr = (__PAGE_INDEX_MANAGER*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PAGE_INDEX_MANAGER);
	if(NULL == lpPageIndexMgr)
	{
		goto __TERMINAL;
	}
	if(!lpPageIndexMgr->Initialize((__COMMON_OBJECT*)lpPageIndexMgr))
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpPageIndexMgr);
		lpPageIndexMgr = NULL;
		goto __TERMINAL;
	}
	lpManager->lpPageIndexMgr = lpPageIndexMgr;

	if (IN_SYSINITIALIZATION())
	{
		/*
		 * Create the virtual area descriptor corresponding to
		 * kernel reserved space,it's corresponding page table
		 * is initialized by page index manager.
		 * The following code is invoked only once in process of
		 * system initialization.
		 */
		lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
			KMEM_SIZE_TYPE_ANY);
		if (NULL == lpVad)
		{
			goto __TERMINAL;
		}

#define SET(member,value) lpVad->member = value
		SET(lpManager, lpManager);
		SET(lpStartAddr, KMEM_KERNEL_EXCLUDE_START);
		SET(ulSize, (KMEM_KERNEL_EXCLUDE_END - KMEM_KERNEL_EXCLUDE_START));
		SET(lpEndAddr, (LPVOID)(KMEM_KERNEL_EXCLUDE_END - 1));
		SET(lpNext, NULL);
		SET(dwAccessFlags, VIRTUAL_AREA_ACCESS_RW);
		SET(dwAllocFlags, VIRTUAL_AREA_ALLOCATE_COMMIT);
		SET(lpLeft, NULL);
		SET(lpRight, NULL);
		SET(dwCacheFlags, VIRTUAL_AREA_CACHE_NORMAL);
#undef SET

		INIT_ATOMIC(lpVad->Reference);
		StrCpy((LPSTR)"Kernel_Rsved", (LPSTR)&lpVad->strName[0]);
		/* Insert the system kernel area into virtual memory manager's list. */
		InsertVirtualArea(lpManager, lpVad);
	}
	else
	{
		/* 
		 * The virtual memory manager is created for user process,
		 * so must exclude the kernel and IO maped space from user
		 * virtual memory space,to avoid allocating them to user.
		 */
		pKernelSpace = __VirtualAlloc((__COMMON_OBJECT*)lpManager,
			(LPVOID)KMEM_KRNLSPACE_START,
			KMEM_KRNLSPACE_LENGTH,
			VIRTUAL_AREA_ALLOCATE_RESERVE,
			VIRTUAL_AREA_ACCESS_RW,
			"krnl",
			NULL);
		BUG_ON((LPVOID)KMEM_KRNLSPACE_START != pKernelSpace);
		pKernelSpace = __VirtualAlloc((__COMMON_OBJECT*)lpManager,
			(LPVOID)KMEM_IOMAP_START,
			KMEM_IOMAP_LENGTH,
			VIRTUAL_AREA_ALLOCATE_RESERVE,
			VIRTUAL_AREA_ACCESS_RW,
			"iomap",
			NULL);
		BUG_ON((LPVOID)KMEM_IOMAP_START != pKernelSpace);
	}
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if (lpPageIndexMgr)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpPageIndexMgr);
		}
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
		return FALSE;
	}
	return TRUE;
}

/* Destroyer of virtual memory manager object. */
BOOL VmmUninitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER* lpPageIndexMgr = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpManager = NULL;
	__VIRTUAL_AREA_DESCRIPTOR *pVad = NULL, *pVadPrev = NULL;

	BUG_ON(NULL == lpThis);
	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	/* Release all the virtual areas belong to this virtual memory manager. */
	pVad = lpManager->lpListHdr;
	while (pVad)
	{
		pVadPrev = pVad;
		pVad = pVad->lpNext;
		__VirtualFree(lpThis, pVadPrev->lpStartAddr);
	}
	/* Destroy the page index manager object. */
	lpPageIndexMgr = lpManager->lpPageIndexMgr;
	if(lpPageIndexMgr)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpPageIndexMgr);
	}
	return TRUE;
}

/*
 * DoCommit routine.When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_COMMIT,
 * then is routine is called by VirtualAlloc.
 * The virtual area must be reserved by calling VirtualAlloc and specifying the
 * VIRTUAL_AREA_ALLOCATE_RESERVE flag.
 */
static LPVOID DoCommit(__COMMON_OBJECT* lpThis,
	LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	UCHAR* lpVaName,
	LPVOID lpReserved)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	LPVOID lpStartAddr = NULL;
	LPVOID lpPhysical = NULL;
	DWORD dwFlags = 0;
	DWORD dwPteFlags = 0;
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == lpThis);
	/* This routine only process commit. */
	BUG_ON((dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK) != VIRTUAL_AREA_ALLOCATE_COMMIT);

	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		/* Should search in list. */
		lpVad = GetVirtualArea(lpThis, lpDesiredAddr);
	}
	else
	{
		/* Should search in the AVL tree. */
		lpVad = GetVaByAddr_t(lpThis, lpDesiredAddr);
	}

	if(NULL == lpVad)
	{
		/* The virtual memory area is not reserved early. */
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	lpStartAddr = lpVad->lpStartAddr;
	dwSize = (DWORD)lpVad->lpEndAddr - (DWORD)lpStartAddr + 1;
	lpPhysical  = PageFrameManager.FrameAlloc((__COMMON_OBJECT*)&PageFrameManager,
		dwSize,
		0);
	if(NULL == lpPhysical)
	{
		/* Not enough physical memory frames. */
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	/* Reserves page table entries for this virtual area. */
	lpVad->dwAllocFlags = dwAllocFlags;
	dwPteFlags = PTE_FLAGS_FOR_NORMAL;
	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			/* Shoud not occur. */
			BUG();
			__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical  = (LPVOID)((DWORD)lpPhysical  + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if (lpPhysical)
		{
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
				lpPhysical,
				dwSize);
		}
		return NULL;
	}
	return lpDesiredAddr;
}

/*
 * Reserve a chunk of virtual memory space,no RAM memory allocated for
 * the reserved space,and no page table allocated for the reserved
 * range.
 * One scenario to use this routine is,kernel memory space should be
 * reserved in each user process's space,to avoid allocating kernel
 * memory space to user.
 * When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_RESERVE flag,
 * then this routine is invoked.
 */
static LPVOID DoReserve(__COMMON_OBJECT* lpThis,
	LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	UCHAR* lpVaName,
	LPVOID lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID lpStartAddr = lpDesiredAddr;
	LPVOID lpEndAddr = NULL;
	unsigned long ulFlags = 0;
	BOOL bResult = FALSE;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	DWORD dwPteFlags = 0;

	/* Validate parameters. */
	BUG_ON((NULL == lpThis) || (0 == dwSize));
	BUG_ON(VIRTUAL_AREA_ALLOCATE_RESERVE != (dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK));
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	/* Round up to page. */
	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1));
	/* Round down to page. */
	lpEndAddr = (LPVOID)((DWORD)lpDesiredAddr + dwSize);
	lpEndAddr = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ?
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1)
		: ((DWORD)lpEndAddr - 1));

	/* Get the actually size. */
	dwSize = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY);
	if (NULL == lpVad)
	{
		goto __TERMINAL;
	}
	lpVad->lpManager = lpMemMgr;
	lpVad->lpStartAddr = NULL;
	lpVad->ulSize = 0;
	lpVad->lpEndAddr = NULL;
	lpVad->lpNext = NULL;
	lpVad->dwAccessFlags = dwAccessFlags;
	lpVad->dwAllocFlags = dwAllocFlags;
	INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft = NULL;
	lpVad->lpRight = NULL;
	if(lpVaName)
	{
		if (StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
		{
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		}
		StrCpy((LPSTR)lpVaName, (LPSTR)&lpVad->strName[0]);
	}
	else
	{
		lpVad->strName[0] = 0;
	}
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_NORMAL;

	/*
	 * The following code searchs virtual area list or AVL tree,
	 * to check if the lpDesiredAddr
	 * is occupied,if so,then find a new one.
	 */
	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, ulFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		/* Should search in the list. */
		lpStartAddr = SearchVirtualArea((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, 
			dwSize,
			dwAllocFlags);
	}
	else
	{
		/* Should search in the AVL tree. */
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, dwSize);
	}
	if(VIRTUAL_MEMORY_END == (unsigned long)lpStartAddr)
	{
		/* No virtual area can satisfy the requirement. */
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, ulFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->ulSize = dwSize;
	lpVad->lpEndAddr = (LPVOID)((DWORD)lpStartAddr + dwSize -1);
	lpDesiredAddr = lpStartAddr;

	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		InsertVirtualArea(lpMemMgr, lpVad);
	}
	else
	{
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, ulFlags);
	/*
	 * Indicate that the whole operation is successfully.
	 * In this operation(only reserve),we do not commit page table entries,
	 * so,if access the memory space(read or write) of this range,will
	 * cause a page fault exception.
	 * The caller must call VirtualAlloc routine again to commit this
	 * block of area before access it.
	 */
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
		return NULL;
	}
	return lpDesiredAddr;
}

/*
 * DoReserveAndCommit routine.It reserves a memory range in virtual
 * memory space and allocates RAM memory pages to it.The caller can
 * use this this chunk of memory immediately.
 * When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_ALL or
 * VIRTUAL_AREA_ALLOCATE_DEFAULT,then this routine is called by 
 * VirtualAlloc.
 */
static LPVOID DoReserveAndCommit(__COMMON_OBJECT* lpThis,
	LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	UCHAR* lpVaName,
	LPVOID lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID lpStartAddr = lpDesiredAddr;
	LPVOID lpEndAddr = NULL;
	DWORD dwFlags = 0;
	BOOL bResult = FALSE;
	LPVOID lpPhysical = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	DWORD dwPteFlags = 0;

	BUG_ON((NULL == lpThis) || (0 == dwSize));
	if (VIRTUAL_AREA_ALLOCATE_ALL != (dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK))
	{
		return NULL;
	}
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	/* Round up to page,if not. */
	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1));
	lpEndAddr = (LPVOID)((DWORD)lpDesiredAddr + dwSize );
	lpEndAddr = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ? 
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1) 
		: ((DWORD)lpEndAddr - 1));

	/* Get the actually size. */
	dwSize = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY);
	if(NULL == lpVad)
	{
		_hx_printf("[%s]:Out of memory.\r\n",__func__);
		goto __TERMINAL;
	}
	lpVad->lpManager = lpMemMgr;
	lpVad->lpStartAddr = NULL;
	lpVad->ulSize = 0;
	lpVad->lpEndAddr = NULL;
	lpVad->lpNext = NULL;
	lpVad->dwAccessFlags = dwAccessFlags;
	lpVad->dwAllocFlags = dwAllocFlags;
	INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft = NULL;
	lpVad->lpRight = NULL;
	if(lpVaName)
	{
		if(StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		StrCpy((LPSTR)lpVaName, (LPSTR)&lpVad->strName[0]);
	}
	else
	{
		lpVad->strName[0] = 0;
	}
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_NORMAL;

	/* 
	 * Allocate physical memory pages.
	 * In order to reduce the time spend in critical section,
	 * we allocate physical memory pages here. 
	 */
	lpPhysical = PageFrameManager.FrameAlloc((__COMMON_OBJECT*)&PageFrameManager,
		dwSize,
		0);
	if(NULL == lpPhysical)
	{
		_hx_printf("[%s]:No enough RAM pages.\r\n",__func__);
		goto __TERMINAL;
	}

	/*
	 * The following code searchs virtual area list or AVL tree,to 
	 * check if the lpDesiredAddr is occupied,if so,then find a new one.
	 * Save the lpStartAddr first,because the lpStartAddr may changed
	 * after the SearchVirtualArea_X is called.
	 */
	lpEndAddr = lpStartAddr;
	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		lpStartAddr = SearchVirtualArea((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, 
			dwSize,
			dwAllocFlags);
	}
	else
	{
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, dwSize);
	}
	if(VIRTUAL_MEMORY_END == (unsigned long)lpStartAddr)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->ulSize = dwSize;
	lpVad->lpEndAddr = (LPVOID)((DWORD)lpStartAddr + dwSize - 1);
	if (!(lpStartAddr == lpEndAddr))
	{
		lpDesiredAddr = lpStartAddr;
	}

	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		InsertVirtualArea(lpMemMgr, lpVad);
	}
	else
	{
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}

	/* Reserves page table entries for the committed memory. */
	dwPteFlags = PTE_FLAGS_FOR_NORMAL;
	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			/* 
			 * Out of memory may lead the failure of page reserving,
			 * since page table itself also need physical memory.
			 */
			__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical = (LPVOID)((DWORD)lpPhysical + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
		if (lpPhysical)
		{
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
				lpPhysical,
				dwSize);
		}
		return NULL;
	}
	return lpDesiredAddr;
}

/*
 * DoIoCommit routine.
 * When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_IOCOMMIT,
 * this is routine is called by VirtualAlloc.It allocates a virtual area,initializes it,
 * insert it into virtual area list and reserve memory page for it.
 * The difference between this and DoReserveAndCommit is,the Page Table's cache flags is
 * set to disabled in this routine,thus the memory synchronization is guaranteed.
 * Must code lines are same between this and DoReserveAndCommit,we will combine them as
 * one routine in the future,but not now,just because of my lazy...:-)
 */
static LPVOID DoIoCommit(__COMMON_OBJECT* lpThis,
	LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	UCHAR* lpVaName,
	LPVOID lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID lpStartAddr = lpDesiredAddr;
	LPVOID lpEndAddr = NULL;
	unsigned long dwFlags = 0;
	BOOL bResult = FALSE;
	LPVOID lpPhysical = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	DWORD dwPteFlags = 0;

	/* Parameter checking. */
	if ((NULL == lpThis) || (0 == dwSize))
	{
		return NULL;
	}
	if (VIRTUAL_AREA_ALLOCATE_IOCOMMIT != (dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK))
	{
		return NULL;
	}
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1)); //Round up to page.

	lpEndAddr = (LPVOID)((DWORD)lpDesiredAddr + dwSize);
	lpEndAddr = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ?
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1)
		: ((DWORD)lpEndAddr - 1)); //Round down to page.

	dwSize = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;  //Get the actually size.

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY); //In order to avoid calling KMemAlloc routine in the
	//critical section,we first call it here.
	if (NULL == lpVad)
	{
		goto __TERMINAL;
	}
	lpVad->lpManager = lpMemMgr;
	lpVad->lpStartAddr = NULL;
	lpVad->ulSize = 0;
	lpVad->lpEndAddr = NULL;
	lpVad->lpNext = NULL;
	lpVad->dwAccessFlags = dwAccessFlags;
	lpVad->dwAllocFlags = dwAllocFlags;
	INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft = NULL;
	lpVad->lpRight = NULL;
	if (lpVaName)
	{
		/* Set the virtual area's name. */
		if (StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
		{
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		}
		StrCpy((LPSTR)lpVaName, (LPSTR)&lpVad->strName[0]);
	}
	else
	{
		lpVad->strName[0] = 0;
	}
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_IO;
	
	/*
	 * Allocate physical memory pages.
	 * In order to reduce the time in critical 
	 * section,we do it out of critical section.
	 */
	lpPhysical = PageFrameManager.FrameAlloc((__COMMON_OBJECT*)&PageFrameManager,
		dwSize,
		0);                  
	if (NULL == lpPhysical)
	{
		/* Can not allocate physical memory. */
		_hx_printk("%s: Can not allocate physical memory.", __func__);
		goto __TERMINAL;
	}

	/*
	 * Searchs virtual area list or AVL tree,to check if the lpDesiredAddr
	 * is occupied,if so,then find a new one.
	 * First we save the lpStartAddr,because the lpStartAddr may changed
	 * after the SearchVirtualArea_X is called.
	 */
	lpEndAddr = lpStartAddr;    
	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		lpStartAddr = SearchVirtualArea((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, 
			dwSize,
			dwAllocFlags);
	}
	else
	{
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, dwSize);
	}
	if (VIRTUAL_MEMORY_END == (unsigned long)lpStartAddr)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->ulSize = dwSize;
	lpVad->lpEndAddr = (LPVOID)((DWORD)lpStartAddr + dwSize - 1);
	if (!(lpStartAddr == lpEndAddr))
	{
		lpDesiredAddr = lpStartAddr;
	}

	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		InsertVirtualArea(lpMemMgr, lpVad);
	}
	else
	{
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}

	/* Reserves page table entries for the committed memory. */
	dwPteFlags = PTE_FLAGS_FOR_IOMAP;
	while (dwSize)
	{
		if (!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr, lpPhysical, dwPteFlags))
		{
			/* Should not occur. */
			BUG();
			__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical = (LPVOID)((DWORD)lpPhysical + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
		if (lpPhysical)
		{
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
				lpPhysical,
				dwSize);
		}
		return NULL;
	}
	return lpDesiredAddr;
}

/*
 * DoIoMap routine.
 * When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_IO,
 * then is routine is called by VirtualAlloc.
 * It's mostly used by device driver to reserve device configuration space,so
 * the flags in page table should be NO-CACHED.
 */
static LPVOID DoIoMap(__COMMON_OBJECT* lpThis,
	const LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	UCHAR* lpVaName,
	LPVOID lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID lpStartAddr = lpDesiredAddr;
	LPVOID lpEndAddr = NULL;
	unsigned long dwFlags = 0;
	BOOL bResult = FALSE;
	LPVOID lpPhysical = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	DWORD dwPteFlags  = 0;

	/* Parameters checking. */
	if ((NULL == lpThis) || (0 == dwSize))
	{
		return NULL;
	}
	BUG_ON(VIRTUAL_AREA_ALLOCATE_IO != (dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK));

	/* Obtain the page index manager object. */
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	/* Round the start and end address to page size,if not yet. */
	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1));
	lpEndAddr = (LPVOID)((DWORD)lpDesiredAddr + dwSize );
	lpEndAddr = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ? 
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1) 
		: ((DWORD)lpEndAddr - 1));
	dwSize = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;

	/* The requested range must be in I/O maped space. */
	if ((unsigned long)lpStartAddr < KMEM_IOMAP_START)
	{
		goto __TERMINAL;
	}

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY);
	if (NULL == lpVad)
	{
		goto __TERMINAL;
	}
	lpVad->lpManager = lpMemMgr;
	lpVad->lpStartAddr = NULL;
	lpVad->ulSize = 0;
	lpVad->lpEndAddr = NULL;
	lpVad->lpNext = NULL;
	lpVad->dwAccessFlags = dwAccessFlags;
	lpVad->dwAllocFlags = dwAllocFlags;
	INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft = NULL;
	lpVad->lpRight = NULL;
	if(lpVaName)
	{
		/* Set the virtual area's name. */
		if (StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
		{
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		}
		StrCpy((LPSTR)lpVaName, (LPSTR)&lpVad->strName[0]);
	}
	else
	{
		lpVad->strName[0] = 0;
	}
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_IO;

	/*
	 * Searchs virtual area list or AVL tree,to check if the lpDesiredAddr
	 * is occupied,if so,then find a new one.
	 */
	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		/* Should search in the list. */
		lpStartAddr = SearchVirtualArea((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, 
			dwSize,
			dwAllocFlags);
	}
	else
	{
		/* Should search in the AVL tree. */
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr, lpStartAddr, dwSize);
	}
	if(VIRTUAL_MEMORY_END == (unsigned long)lpStartAddr)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->ulSize = dwSize;
	lpVad->lpEndAddr = (LPVOID)((DWORD)lpStartAddr + dwSize -1);

	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		InsertVirtualArea(lpMemMgr, lpVad);
	}
	else
	{
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr, lpVad);
	}

	/* Reserves page table entries for the committed memory. */
	dwPteFlags = PTE_FLAGS_FOR_IOMAP;

	lpPhysical = lpStartAddr;
	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			BUG();
			__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical  = (LPVOID)((DWORD)lpPhysical  + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if (lpVad)
		{
			KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		}
		if (lpPhysical)
		{
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
				lpPhysical,
				dwSize);
		}
		return NULL;
	}
	return lpDesiredAddr;
}

/* 
 * Helper routine to validate allocation flags value,
 * invoked by VirtualAlloc routine. 
 */
static BOOL ValidateAllocFlags(unsigned long ulAllocFlags)
{
	if (0 == (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK))
	{
		return TRUE;
	}
	if (VIRTUAL_AREA_ALLOCATE_USERSPACE == (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK))
	{
		return TRUE;
	}
	if (VIRTUAL_AREA_ALLOCATE_IOSPACE == (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK))
	{
		return TRUE;
	}
	if (VIRTUAL_AREA_ALLOCATE_KERNELSPACE == (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK))
	{
		return TRUE;
	}
	if (VIRTUAL_AREA_ALLOCATE_STACK == (ulAllocFlags & VIRTUAL_AREA_ALLOCATE_SPACE_MASK))
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Allocate virtual memory region from current process's
 * lineary space.It will call the corresponding worker
 * routines according to dwAllocFlags.
 */
static LPVOID __VirtualAlloc(__COMMON_OBJECT* lpThis,
	LPVOID lpDesiredAddr,
	DWORD dwSize,
	DWORD dwAllocFlags,
	DWORD dwAccessFlags,
	char* lpVaName,
	LPVOID lpReserved)
{
	/* Validates allocation flags value. */
	if (!ValidateAllocFlags(dwAllocFlags))
	{
		return NULL;
	}
	
	switch(dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK)
	{
	case VIRTUAL_AREA_ALLOCATE_IO:
		return DoIoMap(lpThis,
			lpDesiredAddr,
			dwSize,
			dwAllocFlags,
			dwAccessFlags,
			lpVaName,
			lpReserved);
	case VIRTUAL_AREA_ALLOCATE_RESERVE:
		return DoReserve(lpThis,
			lpDesiredAddr,
			dwSize,
			dwAllocFlags,
			dwAccessFlags,
			lpVaName,
			lpReserved);
	case VIRTUAL_AREA_ALLOCATE_ALL:
		return DoReserveAndCommit(lpThis,
			lpDesiredAddr,
			dwSize,
			dwAllocFlags,
			dwAccessFlags,
			lpVaName,
			lpReserved);
	case VIRTUAL_AREA_ALLOCATE_COMMIT:
		return DoCommit(lpThis,
			lpDesiredAddr,
			dwSize,
			dwAllocFlags,
			dwAccessFlags,
			lpVaName,
			lpReserved);
	case VIRTUAL_AREA_ALLOCATE_IOCOMMIT:
		return DoIoCommit(lpThis,
			lpDesiredAddr,
			dwSize,
			dwAllocFlags,
			dwAccessFlags,
			lpVaName,
			lpReserved);
	default:
		return NULL;
	}
	return NULL;
}

//
//A helper routine,used to delete a virtual area descriptor object from AVL
//tree.
//
static VOID DelVaFromTree(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	return; //We will complete this routine in the future.
}

/*
 * A helper routine,used to release virtual area that committed.
 */
static VOID ReleaseCommit(__COMMON_OBJECT* lpThis,
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

	/* Get the physical memory address. */
	lpEndAddr = lpIndexMgr->GetPhysicalAddress((__COMMON_OBJECT*)lpIndexMgr,
		lpStartAddr);
	/* Release the corresponding physical page. */
	PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
		lpEndAddr, dwSize);

	while(dwSize)
	{
		lpIndexMgr->ReleasePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr);    //Release the page table entry that this area occupies.
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		dwSize -= PAGE_FRAME_SIZE;
	}
}

/*
 * A helper routine,used to release virtual area that committed,but without physical
 * memory pages allocated,such as IO map zone.
 * This routine only release the page table entry that this virtual area occupies.
 */
static VOID ReleaseIoMap(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER*             lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                lpStartAddr = NULL;
	LPVOID                                lpEndAddr   = NULL;
	__PAGE_INDEX_MANAGER*                 lpIndexMgr  = NULL;
	DWORD                                 dwSize      = 0;

	if ((NULL == lpThis) || (NULL == lpVad))
	{
		return;
	}
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	BUG_ON(NULL == lpIndexMgr);

	lpStartAddr = lpVad->lpStartAddr;
	lpEndAddr   = lpVad->lpEndAddr;
	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;
	while(dwSize)
	{
		/* Release the page table entry that this area occupies. */
		lpIndexMgr->ReleasePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr);
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		dwSize -= PAGE_FRAME_SIZE;
	}
}

/*
 * This routine frees the virtual area allocated by VirtualAlloc,and
 * frees any resource,such as physical memory page,page table entries,
 * and virtual area descriptor objects,associated with the virtual
 * area object,then returns TRUE.
 * If no virtual area found that matches the user specified lpVirtualAddr,
 * then FALSE will be returned.
 * The start address of the matching VA must be same as virtual address
 * user specified(lpVirtualAddr).
 */
static BOOL __VirtualFree(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__VIRTUAL_MEMORY_MANAGER* lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR* lpVad = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	BOOL bResult = FALSE;
	unsigned long dwFlags = 0;

	BUG_ON(NULL == lpThis);

	__ENTER_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		lpVad = GetVirtualArea(lpThis, lpVirtualAddr);
	}
	else
	{
		lpVad = GetVaByAddr_t(lpThis, lpVirtualAddr);
	}
	if(NULL == lpVad)
	{
		/* This virtual address is not allocated yet. */
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Check if the specified address is same as VA's start addr. */
	if (lpVad->lpStartAddr != lpVirtualAddr)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Delete it from list or tree. */
	if (lpMemMgr->va_num < SWITCH_VA_NUM)
	{
		DeleteVirtualArea(lpMemMgr, lpVad);
	}
	else
	{
		DelVaFromTree(lpThis, lpVad);
	}

	/* According to different allocating type,to do the different actions. */
	switch(lpVad->dwAllocFlags & VIRTUAL_AREA_ALLOCATE_FLAG_MASK)
	{
	case VIRTUAL_AREA_ALLOCATE_COMMIT:
	case VIRTUAL_AREA_ALLOCATE_ALL:
		ReleaseCommit(lpThis, lpVad);
		/* Release the memory occupied by virtual area descriptor.*/
		KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		break;
	case VIRTUAL_AREA_ALLOCATE_RESERVE:
		KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		break;
	case VIRTUAL_AREA_ALLOCATE_IO:
		/* Committed,but without physical memory pages. */
		ReleaseIoMap(lpThis, lpVad);
		KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		break;
	case VIRTUAL_AREA_ALLOCATE_MAP:
		ReleaseMap(lpThis, lpVad);
		KMemFree((LPVOID)lpVad, KMEM_SIZE_TYPE_ANY, 0);
		break;
	default:
		/* Should not reach here. */
		BUG_ON(TRUE);
		break;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpMemMgr->spin_lock, dwFlags);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* This routine returns the physical address of page directory. */
static LPVOID GetPdAddress(__COMMON_OBJECT* lpThis)
{
	__VIRTUAL_MEMORY_MANAGER*       lpManager = NULL;

	if (NULL == lpThis)
	{
		return NULL;
	}

	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;

	return (LPVOID)lpManager->lpPageIndexMgr->lpPdAddress;
}

//Get the physical address by giving a virtual address.
static LPVOID _GetPhysicalAddress(__COMMON_OBJECT* lpThis, LPVOID lpVirtualAddr)
{
	__VIRTUAL_MEMORY_MANAGER* lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;

	if (NULL == lpManager)
	{
		return NULL;
	}
	return lpManager->lpPageIndexMgr->GetPhysicalAddress(
		(__COMMON_OBJECT*)lpManager->lpPageIndexMgr,
		lpVirtualAddr);
}

/*
 * Virtual Memory Manager for system,to manage the kernel space.
 * This is one of the global objects,but it is created by 
 * CreateObject routine of ObjectManager.
 */
__VIRTUAL_MEMORY_MANAGER* lpVirtualMemoryMgr = NULL;

#endif
