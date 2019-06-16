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
#include <mlayout.h>
#include <stdio.h>
#include <stdlib.h>

/* 
 * Page Index function is available only if 
 * the VMM function is enabled.
 */
#ifdef __CFG_SYS_VMM

/*
 * Memory for kernel reserved space's page table.
 * The reserved kernel space will be maped to each process,
 * just link this buck of memory into each process's
 * page directoy is OK,no need to create reserved kernel
 * page table for every process,since they are same.
 * It will be initialized at the first time of
 * page index manager is created.
 * Other kernel space's page table exclude the reserved part
 * and I/O maped part 
 * is not initialized yet,since it's will be created on
 * demond.When new page table of kernel space is created,
 * it is mounted into just the required process's page
 * table,and is not mounted into other processes that
 * not the demond one.It will be installed into specific
 * process's page table on demond.
 */
static __PTE* kernel_rsved_pt = NULL;

/*
 * Start physical memory address of kernel space's page
 * directory.It's common for every process so we make it
 * standalone and will be copied into each process's
 * page directory.
 * It's size is 4K in 32 bits since the kernel space is
 * 1G.
 */
static __PDE* kernel_pdt_start = NULL;

/* I/O Maped space's page directory,same as kernel's. */
static __PDE* iomap_pdt_start = NULL;

/* How many page index managers have been created. */
__atomic_t pim_num = ATOMIC_INIT_VALUE;

/* Member functions of page index manager. */
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
	unsigned long pte_start = 0;
	unsigned long pde_start = 0;
	unsigned long mem_start = 0;
	unsigned int pd_num = 0, pt_num = 0;

	/* Page directory's length of kernel space,IO maped space and user space. */
	unsigned int kernel_pdt_len = KMEM_KRNLSPACE_LENGTH / PAGE_SIZE / PT_SIZE;
	kernel_pdt_len *= sizeof(__PDE);
	unsigned int iomap_pdt_len = KMEM_IOMAP_LENGTH / PAGE_SIZE / PT_SIZE;
	iomap_pdt_len *= sizeof(__PDE);
	unsigned int user_pdt_len = KMEM_USERSPACE_END - KMEM_USERSPACE_START;
	user_pdt_len /= (PAGE_SIZE * PT_SIZE);
	user_pdt_len *= sizeof(__PDE);

	/*
	 * How many page directory entries should be initialized
	 * to map the kernel space to process.
	 */
	unsigned int init_pdes = 0;
	BOOL bResult = FALSE;

	BUG_ON(NULL == lpThis);
	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;

	/*
	 * How many page directory entries that kernel
	 * reserved space will occupy.
	 */
	init_pdes = (KMEM_KERNEL_EXCLUDE_END / PAGE_SIZE);
	init_pdes /= PT_SIZE;

	/* 
	 * Create kernel's page table if not yet, 
	 * and initializes it by using the kernel
	 * reserved space.
	 */
	if (NULL == kernel_rsved_pt)
	{
		/* 
		 * Allocate memory space for kernel reserved space's page table,
		 * kernel PDT and IO maped PDT. 
		 * We seperate into 2 allocate operations since the page table 
		 * must be PAGE_SIZE aligned,we apply it in the 1st allocation.
		 */
		char* kernel_pde_pdt = (char*)_hx_aligned_malloc(
			init_pdes * PT_SIZE * sizeof(__PTE),
			PAGE_SIZE);
		BUG_ON(NULL == kernel_pde_pdt); /* Exception case. */
		memset(kernel_pde_pdt, 0, init_pdes * PT_SIZE * sizeof(__PTE));
		kernel_rsved_pt = (__PTE*)kernel_pde_pdt;

		kernel_pde_pdt = (char*)_hx_aligned_malloc(
			kernel_pdt_len + iomap_pdt_len,
			sizeof(__PDE)
		);
		BUG_ON(NULL == kernel_pde_pdt); /* Exception case. */
		/* 
		 * BUG caused at very first without the following 
		 * line of code. Reset to all zeros after allocation
		 * of memory is very very important.
		 */
		memset(kernel_pde_pdt, 0, (kernel_pdt_len + iomap_pdt_len));
		kernel_pdt_start = (__PDE*)kernel_pde_pdt;
		iomap_pdt_start = (__PDE*)(kernel_pde_pdt + kernel_pdt_len);

		pte_start = (unsigned long)kernel_rsved_pt;
		for (pd_num = 0; pd_num < init_pdes; pd_num++)
		{
			/* Set the corresponding PDE entry. */
			INIT_PDE_TO_DEFAULT(pde);
			FORM_PDE_ENTRY(pde, pte_start);
			kernel_pdt_start[pd_num] = pde;

			/* Initializes the corresponding page table. */
			for (pt_num = 0; pt_num < PT_SIZE; pt_num++)
			{
				INIT_PTE_TO_DEFAULT(pte);
				FORM_PTE_ENTRY(pte, mem_start);
				SET_PTE(pte_start, pte);
				mem_start += PAGE_SIZE;
				pte_start += sizeof(__PTE);
			}
		}
	}

	/*
	 * Create the top level page directory.
	 * Please be noted that the start address of
	 * page directory and page table must be PAGE_SIZE
	 * aligned,in most architecture.
	 */
	char* user_pdt = (char*)KMemAlloc(PD_FULL_SIZE, KMEM_SIZE_TYPE_4K);
	if (NULL == user_pdt)
	{
		goto __TERMINAL;
	}
	memset(user_pdt, 0, PD_FULL_SIZE);
	lpMgr->lpPdAddress = (__PDE*)user_pdt;
	/* Map the kernel and IO space to user. */
	memcpy(user_pdt, kernel_pdt_start, kernel_pdt_len);
	user_pdt += kernel_pdt_len;
	memset(user_pdt, 0, user_pdt_len);
	user_pdt += user_pdt_len;
	memcpy(user_pdt, iomap_pdt_start, iomap_pdt_len);

	/* Initializes operation routines. */
	lpMgr->GetPhysicalAddress = GetPhysicalAddress;
	lpMgr->ReservePage = ReservePage;
	lpMgr->SetPageFlags = SetPageFlags;
	lpMgr->ReleasePage = ReleasePage;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpMgr->spin_lock, "pgidxmgr");
#endif

#if 0
	/* Map the kernel reserved space to the new created page table. */
	pde_start = (unsigned long)lpMgr->lpPdAddress;
	pte_start = (unsigned long)kernel_rsved_pt;
	for (pd_num = 0; pd_num < init_pdes; pd_num++)
	{
		/* Set the page directory entry. */
		INIT_PDE_TO_DEFAULT(pde);
		FORM_PDE_ENTRY(pde, pte_start);
		SET_PDE(pde_start, pde);

		/* Move to next PDE entry. */
		pde_start += sizeof(__PDE);
		pte_start += PT_SIZE * sizeof(__PTE);
	}
#endif 
	/* Record the just created page index manager. */
	__ATOMIC_INCREASE(&pim_num);
	bResult = TRUE;
__TERMINAL:
	if (!bResult)
	{
		/* Recollection of memory in case of failure. */
		if (lpMgr->lpPdAddress)
		{
			KMemFree(lpMgr->lpPdAddress, KMEM_SIZE_TYPE_4K, PD_FULL_SIZE);
		}
	}
	return bResult;
}

/* Uninitializer of page index manager. */
BOOL PageUninitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER* lpMgr = NULL;
	unsigned long ptep = 0;
	unsigned long user_pde_num = 0;  /* How many PDE entries user space occupies. */
	unsigned long user_pde_start = 0; /* Start index of user PDE entries. */

	user_pde_num = KMEM_USERSPACE_END - KMEM_USERSPACE_START;
	user_pde_num /= (PAGE_SIZE * PT_SIZE);
	user_pde_start = KMEM_KRNLSPACE_LENGTH / PAGE_SIZE / PT_SIZE;

	BUG_ON(NULL == lpThis);
	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	BUG_ON(NULL == lpMgr->lpPdAddress);
	/* 
	 * Release all user page tables that exist in page directory. 
	 * The kernel space's page tables are unchanged now.
	 */
	for (unsigned long i = user_pde_start; i < user_pde_num; i++)
	{
		ptep = (unsigned long)lpMgr->lpPdAddress[i];
		ptep &= PTE_ADDRESS_MASK;
		if (ptep)
		{
			KMemFree((LPVOID)ptep, KMEM_SIZE_TYPE_4K, PD_SIZE * sizeof(unsigned long));
		}
	}
	/* Release the page directory. */
	KMemFree(lpMgr->lpPdAddress, KMEM_SIZE_TYPE_4K, PD_FULL_SIZE);
	/* Decrease page index manager's total number. */
	__ATOMIC_DECREASE(&pim_num);
	if (0 == pim_num)
	{
		/* 
		 * Last page index manager is destroyed,so
		 * release the page tables for kernel space.
		 */
		if (kernel_rsved_pt)
		{
			_hx_free(kernel_rsved_pt);
			kernel_rsved_pt = NULL;
		}
		if (kernel_pdt_start)
		{
			/* 
			 * Release kernel PDT and IO maped PDT. 
			 * They are in the same memory block.
			 */
			_hx_free(kernel_pdt_start);
			kernel_pdt_start = NULL;
			iomap_pdt_start = NULL;
		}
	}
	return TRUE;
}

/* 
 * Return the physical address, given the corresponding 
 * virtual address,in the memory space specified by page
 * index manager.
 */
static LPVOID GetPhysicalAddress(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__PAGE_INDEX_MANAGER* lpIndexMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	__PDE pde;
	__PTE pte;
	__PTE* lpPte;
	DWORD dwIndex = 0;
	LPVOID lpPhysical = NULL;

	/* Basic checking. */
	BUG_ON((NULL == lpIndexMgr) || (NULL == lpVirtualAddr));
	BUG_ON(NULL == lpIndexMgr->lpPdAddress);

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
	__PTE* lpNewPte = NULL;
	__PAGE_INDEX_MANAGER* lpIndexMgr = NULL;
	unsigned long index = 0;
	__PTE pte;
	__PDE pde;
	unsigned long ulFlags;

	BUG_ON((NULL == lpThis) || (NULL == lpVirtualAddr));
	if ((dwFlags & PTE_FLAG_PRESENT) && (NULL == lpPhysicalAddr))
	{
		/* 
		 * The corresponding physical address must be 
		 * specified if persent flag is set.
		 */
		return FALSE;
	}
	/* The physical address must be page aligned.*/
	if(lpPhysicalAddr && ((DWORD)lpPhysicalAddr & (~PTE_ADDRESS_MASK)))
	{
		return FALSE;
	}

	lpIndexMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	dwFlags = dwFlags & PTE_FLAGS_MASK;
	/* Set USER flag if the virtual address is in user space. */
	if (((unsigned long)lpVirtualAddr >= KMEM_USERSPACE_START) &&
		((unsigned long)lpVirtualAddr <= KMEM_USERSPACE_END))
	{
		dwFlags |= PTE_FLAG_USER;
	}
	/* Get the page directory index. */
	index = (unsigned long)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, ulFlags);
	if(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[index]))
	{
		/* Allocate a new page table,must be 4K aligned. */
		lpNewPte = (__PTE*)KMemAlloc(sizeof(__PTE)*PT_SIZE,
			KMEM_SIZE_TYPE_4K);
		if(NULL == lpNewPte)
		{
			__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, ulFlags);
			return FALSE;
		}
		BUG_ON((unsigned long)lpNewPte & (~PTE_ADDRESS_MASK));
		memzero((LPVOID)lpNewPte, sizeof(__PTE) * PT_SIZE);
		INIT_PDE_TO_DEFAULT(pde);
		FORM_PDE_ENTRY(pde,(DWORD)lpNewPte);
		/* Set user flag if in user space. */
		if (((unsigned long)lpVirtualAddr >= KMEM_USERSPACE_START) &&
			((unsigned long)lpVirtualAddr <= KMEM_USERSPACE_END))
		{
			pde |= PDE_FLAG_USER;
		}
		/* Set the PDE entry. */
		lpIndexMgr->lpPdAddress[index] = pde;
		/* 
		 * Set the corresponding PDE if the region is in kernel or 
		 * IO maped space,so the new created process can view it.
		 */
		if (KMEM_IN_KERNEL_SPACE(lpVirtualAddr))
		{
			kernel_pdt_start[index] = pde;
		}
		if (KMEM_IN_IOMAP_SPACE(lpVirtualAddr))
		{
			/* index should recalculated from start of IOMAP space. */
			index = ((unsigned long)lpVirtualAddr - KMEM_IOMAP_START);
			index >>= PD_OFFSET_SHIFT;
			iomap_pdt_start[index] = pde;
		}

		index = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if (dwFlags & PTE_FLAG_PRESENT)
		{
			FORM_PTE_ENTRY(pte, (DWORD)lpPhysicalAddr);
		}
		/* Set the page table entry. */
		lpNewPte[index] = pte;
		/* Flush the just modified page TLB entry. */
		__FLUSH_PAGE_TLB(lpVirtualAddr);
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, ulFlags);
		return TRUE;
	}
	else /* The page directory entry is occupied. */
	{
		lpNewPte = (__PTE*)((unsigned long)(lpIndexMgr->lpPdAddress[index]) & PTE_ADDRESS_MASK);
		index = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
		pte = lpNewPte[index];
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if (dwFlags & PTE_FLAG_PRESENT)
		{
			FORM_PTE_ENTRY(pte, (DWORD)lpPhysicalAddr);
		}
		/* Set the page table entry. */
		lpNewPte[index] = pte;
		/* Flush the just modified page TLB entry. */
		__FLUSH_PAGE_TLB(lpVirtualAddr);
		__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, ulFlags);
		return TRUE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, ulFlags);
	return FALSE;
}

/* Set flags for a given page entry. */
static BOOL SetPageFlags(__COMMON_OBJECT* lpThis,
	LPVOID lpVirtualAddr,
	LPVOID lpPhysicalAddr,
	DWORD dwFlags)
{
	__PAGE_INDEX_MANAGER* lpManager = NULL;
	DWORD dwIndex = 0;
	__PTE* lpPte = NULL;
	__PTE pte;
	DWORD dwFlags1 = 0;

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
	__PAGE_INDEX_MANAGER* lpIndexMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	DWORD dwIndex = 0;
	__PTE* lpPte = NULL;
	DWORD dwFlags = 0;

	if ((NULL == lpIndexMgr) || (NULL == lpVirtualAddr))
	{
		return;
	}
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION_SMP(lpIndexMgr->spin_lock, dwFlags);
	BUG_ON(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[dwIndex]));
	lpPte = (__PTE*)((DWORD)lpIndexMgr->lpPdAddress[dwIndex] & PTE_ADDRESS_MASK);
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
