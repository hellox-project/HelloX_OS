//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : pageidx.c
//    Module Funciton           : 
//                                This module countains page index object
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>

//Page Index function is available only if the VMM function is enabled.
#ifdef __CFG_SYS_VMM

//
//Declaration of member functions.
//
static LPVOID GetPhysicalAddress(__COMMON_OBJECT*,LPVOID);
static BOOL   ReservePage(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
static BOOL   SetPageFlags(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
static VOID   ReleasePage(__COMMON_OBJECT*,LPVOID);

/* Initializer of page index object. */
BOOL PageInitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER* lpMgr = NULL;
	__PDE pde;
	__PTE pte;
	DWORD dwPteStart = PT_START;
	DWORD dwPdeStart = PD_START;
	DWORD dwMemStart = 0;
	DWORD dwLoop = 0;

	BUG_ON(NULL == lpThis);
	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;

	/* Start physical address of page directory. */
	lpMgr->lpPdAddress = (__PDE*)PD_START;
	lpMgr->GetPhysicalAddress = GetPhysicalAddress;
	lpMgr->ReservePage        = ReservePage;
	lpMgr->SetPageFlags       = SetPageFlags;
	lpMgr->ReleasePage        = ReleasePage;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpMgr->spin_lock, "pgidxmgr");
#endif
	if(EMPTY_PDE_ENTRY(*(__PDE*)PD_START))
	{
		/* This is the first time to call, initialize the first 5 PDEs. */
		for(dwLoop = 0;dwLoop < 5;dwLoop ++)
		{
			INIT_PDE_TO_DEFAULT(pde);         //Init to default,that is read and write.
			FORM_PDE_ENTRY(pde,dwPteStart);   //Form a pde entry according to pte's address.
			SET_PDE(dwPdeStart,pde);          //Set the pde entry.
			dwPteStart += sizeof(__PTE) * PT_SIZE;
			dwPdeStart += sizeof(__PDE);
		}
		for(dwLoop = 5;dwLoop < PD_SIZE;dwLoop ++)  //Initialize the rest PDE to NULL_PDE.
		{
			INIT_PDE_TO_NULL(pde);
			SET_PDE(dwPdeStart,pde);
			dwPdeStart += sizeof(__PDE);
		}
		dwPteStart = PT_START;
		for(dwLoop = 0;dwLoop < PT_SIZE * 5;dwLoop ++)  //Initialize the first 5 page tables.
		{
			INIT_PTE_TO_DEFAULT(pte);
			FORM_PTE_ENTRY(pte,dwMemStart);
			SET_PTE(dwPteStart,pte);
			dwMemStart += PAGE_SIZE;
			dwPteStart += sizeof(__PTE);
		}
	}
	else
	{
		/* Not the first time,so must create page index object. */
	}
	return TRUE;
}

/* Uninitializer of page index manager. */
VOID PageUninitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER* lpMgr = NULL;

	BUG_ON(NULL == lpThis);
	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	if((__PDE*)PD_START == lpMgr->lpPdAddress)
	{
		/* This is the system page index manager. */
	}
	else
	{
		/* This is a process page index manager,so should release all resources. */
		//Release source code here.
	}
	return;
}

/* Return the physical address given the corresponding virtual address. */
static LPVOID GetPhysicalAddress(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__PAGE_INDEX_MANAGER*        lpIndexMgr    = (__PAGE_INDEX_MANAGER*)lpThis;
	__PDE                        pde;
	__PTE                        pte;
	__PTE*                       lpPte;
	DWORD                        dwIndex       = 0;
	LPVOID                       lpPhysical    = NULL;

	/* Basic checking. */
	if ((NULL == lpThis) || (NULL == lpVirtualAddr))
	{
		return NULL;
	}
	if (NULL == lpIndexMgr->lpPdAddress)
	{
		return NULL;
	}
	/* Calculate the page directory index. */
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	pde = lpIndexMgr->lpPdAddress[dwIndex];
	if (EMPTY_PDE_ENTRY(pde))
	{
		return NULL;
	}
	if (!(pde & PDE_FLAG_PRESENT))
	{
		/* Page table is not exists. */
		return NULL;
	}
	/* Get the page table's physical address. */
	lpPte = (__PTE*)(pde & PDE_ADDRESS_MASK);
	/* Get pte index. */
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
	pte = lpPte[dwIndex];
	if ((EMPTY_PTE_ENTRY(pte)) || (!(pte & PTE_FLAG_PRESENT)))
	{
		return NULL;
	}
	/* Calculate the physical address. */
	lpPhysical = (LPVOID)(((DWORD)pte & ~PTE_FLAGS_MASK) + 
		((DWORD)lpVirtualAddr & PTE_FLAGS_MASK));
	return lpPhysical;
}

/* Reserve page(s) for a range of virtual memory. */
static BOOL ReservePage(__COMMON_OBJECT* lpThis,
	LPVOID lpVirtualAddr,
	LPVOID lpPhysicalAddr,
	DWORD dwFlags)
{
	__PTE*                  lpNewPte    = NULL;
	__PAGE_INDEX_MANAGER*   lpIndexMgr  = NULL;
	DWORD                   dwIndex     = 0;
	__PTE                   pte;
	__PDE                   pde;
	DWORD                   dwFlags1    = 0;

	if ((NULL == lpThis) || (NULL == lpVirtualAddr))
	{
		return FALSE;
	}
	if ((dwFlags & PTE_FLAG_PRESENT) && (NULL == lpPhysicalAddr))
	{
		/* Invalidate combination. */
		return FALSE;
	}
	/* The physical address is not page aligment.*/
	if(lpPhysicalAddr && ((DWORD)lpPhysicalAddr & (~PTE_ADDRESS_MASK)))
	{
		return FALSE;
	}

	lpIndexMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	dwFlags    = dwFlags & PTE_FLAGS_MASK;
	/* Get the page directory index. */
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags1);
	if(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[dwIndex]))
	{
		/* Allocate a new page table. */
		lpNewPte = (__PTE*)KMemAlloc(sizeof(__PTE)*PT_SIZE,
			KMEM_SIZE_TYPE_4K);
		if(NULL == lpNewPte)
		{
			__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags1);
			return FALSE;
		}
		memzero((LPVOID)lpNewPte,sizeof(__PTE)*PT_SIZE);
		INIT_PDE_TO_DEFAULT(pde);
		FORM_PDE_ENTRY(pde,(DWORD)lpNewPte);
		/* Set the PDE entry. */
		lpIndexMgr->lpPdAddress[dwIndex] = pde;

		dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if (dwFlags & PTE_FLAG_PRESENT)
		{
			FORM_PTE_ENTRY(pte, (DWORD)lpPhysicalAddr);
		}
		/* Set the page table entry. */
		lpNewPte[dwIndex] = pte;
		/* Flush the just modified page TLB entry. */
		__FLUSH_PAGE_TLB(lpVirtualAddr);
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags1);
		return TRUE;
	}
	else /* The page directory entry is occupied. */
	{
		lpNewPte = (__PTE*)((DWORD)(lpIndexMgr->lpPdAddress[dwIndex]) & PTE_ADDRESS_MASK);
		dwIndex  = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
		pte      = lpNewPte[dwIndex];
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if (dwFlags & PTE_FLAG_PRESENT)
		{
			FORM_PTE_ENTRY(pte, (DWORD)lpPhysicalAddr);
		}
		/* Set the page table entry. */
		lpNewPte[dwIndex] = pte;
		/* Flush the just modified page TLB entry. */
		__FLUSH_PAGE_TLB(lpVirtualAddr);
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags1);
		return TRUE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags1);
	return FALSE;
}

/* Set flags for a given page entry. */
static BOOL SetPageFlags(__COMMON_OBJECT* lpThis,
	LPVOID lpVirtualAddr,
	LPVOID lpPhysicalAddr,
	DWORD dwFlags)
{
	__PAGE_INDEX_MANAGER*         lpManager      = NULL;
	DWORD                         dwIndex        = 0;
	__PTE*                        lpPte          = NULL;
	__PTE                         pte;
	DWORD                         dwFlags1       = 0;

	if ((NULL == lpThis) || (NULL == lpVirtualAddr))
	{
		return FALSE;
	}
	if ((NULL == lpPhysicalAddr) && (dwFlags & PTE_FLAG_PRESENT))
	{
		/* Invalidate combination. */
		return FALSE;
	}
	if ((dwFlags & PTE_FLAG_PRESENT) && ((DWORD)lpPhysicalAddr & (~PTE_ADDRESS_MASK)))
	{
		return FALSE;
	}

	lpManager = (__PAGE_INDEX_MANAGER*)lpThis;
	dwFlags &= PTE_FLAGS_MASK;
	/* Get the page direcroty's index. */
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION_SMP(lpManager->spin_lock, dwFlags1);
	if(EMPTY_PDE_ENTRY(lpManager->lpPdAddress[dwIndex]))
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpManager->spin_lock, dwFlags1);
		return FALSE;
	}
	lpPte = (__PTE*)((DWORD)lpManager->lpPdAddress[dwIndex] & PTE_ADDRESS_MASK);
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
	if(EMPTY_PTE_ENTRY(lpPte[dwIndex]))
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpManager->spin_lock, dwFlags1);
		return FALSE;
	}

	/* Initializes a new PTE,and replaces the old one. */
	INIT_PTE_TO_NULL(pte);
	SET_PTE_FLAGS(pte,dwFlags);
	if (dwFlags & PTE_FLAG_PRESENT)
	{
		/* Should set a physical page frame. */
		FORM_PTE_ENTRY(pte, (DWORD)lpPhysicalAddr);
	}
	/* Set the page table entry. */
	lpPte[dwIndex] = pte;
	/* Flush the corresponding page TLB entry. */
	__FLUSH_PAGE_TLB(lpVirtualAddr);
	__LEAVE_CRITICAL_SECTION_SMP(lpManager->spin_lock, dwFlags1);
	return TRUE;
}

/* Release the corresponding page designated by the given virtual address. */
static VOID ReleasePage(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__PAGE_INDEX_MANAGER*        lpIndexMgr     = (__PAGE_INDEX_MANAGER*)lpThis;
	DWORD                        dwIndex        = 0;
	__PTE*                       lpPte          = NULL;
	DWORD                        dwFlags        = 0;

	if ((NULL == lpIndexMgr) || (NULL == lpVirtualAddr))
	{
		return;
	}
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags);
	if(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[dwIndex]))
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags);
		return;
	}
	lpPte   = (__PTE*)((DWORD)lpIndexMgr->lpPdAddress[dwIndex] & PTE_ADDRESS_MASK);
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
	if(EMPTY_PTE_ENTRY(lpPte[dwIndex]))
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags);
		return;
	}
	/* Clear the page table entry. */
	INIT_PTE_TO_NULL(lpPte[dwIndex]);
	/* Flush the corresponding TLB entry. */
	__FLUSH_PAGE_TLB(lpVirtualAddr);
	__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags);
	return;
}

#endif
