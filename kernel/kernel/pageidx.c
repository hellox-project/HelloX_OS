//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : PAGEIDX.CPP
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

#include "StdAfx.h"

#include "stdio.h"

//Page Index function is available only if the VMM function is enabled.
#ifdef __CFG_SYS_VMM

//
//Declaration of member functions.
//
static LPVOID GetPhysicalAddress(__COMMON_OBJECT*,LPVOID);
static BOOL   ReservePage(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
static BOOL   SetPageFlags(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
static VOID   ReleasePage(__COMMON_OBJECT*,LPVOID);


//
//The implementation of PageInitialize routine.
//This routine does the following:
//
BOOL PageInitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER*            lpMgr     = NULL;
	__PDE                            pde;
	__PTE                            pte;
	DWORD                            dwPteStart = PT_START;
	DWORD                            dwPdeStart = PD_START;
	DWORD                            dwMemStart = 0;
	DWORD                            dwLoop     = 0;

	if(NULL == lpThis)    //Parameter check.
	{
		return FALSE;
	}

	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;

	lpMgr->lpPdAddress        = (__PDE*)PD_START;    //Page directory start 
	                                                 //address(physical address).
	lpMgr->GetPhysicalAddress = GetPhysicalAddress;
	lpMgr->ReservePage        = ReservePage;
	lpMgr->SetPageFlags       = SetPageFlags;
	lpMgr->ReleasePage        = ReleasePage;
	if(EMPTY_PDE_ENTRY(*(__PDE*)PD_START))    //This is the first time to call.
	{
		for(dwLoop = 0;dwLoop < 5;dwLoop ++)  //Initialize the first five PDE.
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
	else    //Not the first time,so must create page index object.
	{
	}
	return TRUE;
}

//
//The implementation of PageUninitialize routine.
//

VOID PageUninitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER*      lpMgr   = NULL;

	if(NULL == lpThis)   //Parameter check.
		return;
	lpMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	if((__PDE*)PD_START == lpMgr->lpPdAddress)      //This is the system page index manager.
	{
	}
	else    //This is a process page index manager,so should release all resources.
	{
		//Release source code here.
	}
	return;
}

//
//The implementation of GetPhysicalAddress.
//

static LPVOID GetPhysicalAddress(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__PAGE_INDEX_MANAGER*        lpIndexMgr    = (__PAGE_INDEX_MANAGER*)lpThis;
	__PDE                        pde;
	__PTE                        pte;
	__PTE*                       lpPte;
	DWORD                        dwIndex       = 0;
	LPVOID                       lpPhysical    = NULL;

	if((NULL == lpThis) || (NULL == lpVirtualAddr)) //Invalidate parameters.
		return NULL;
	if(NULL == lpIndexMgr->lpPdAddress)    //Page directory is NULL.
		return NULL;
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;  //Calculate the page directory index.
	pde = lpIndexMgr->lpPdAddress[dwIndex];
	if(EMPTY_PDE_ENTRY(pde)) //NULL pde entry.
		return NULL;
	if(!(pde & PDE_FLAG_PRESENT))    //Page table is not exists.
		return NULL;
	lpPte = (__PTE*)(pde & PDE_ADDRESS_MASK);    //Get the page table's physical address.
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;  //Get pte index.
	pte   = lpPte[dwIndex];
	if((EMPTY_PTE_ENTRY(pte)) || (!(pte & PTE_FLAG_PRESENT)))  //NULL pte or does not present.
		return NULL;
	lpPhysical = (LPVOID)(((DWORD)pte & ~PTE_FLAGS_MASK) + 
		((DWORD)lpVirtualAddr & PTE_FLAGS_MASK));    //Calculate the physical address.
	return lpPhysical;
}

//
//The implementation of ReservePage routine.
//

static BOOL ReservePage(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr,LPVOID lpPhysicalAddr,
						DWORD dwFlags)
{
	__PTE*                  lpNewPte    = NULL;
	__PAGE_INDEX_MANAGER*   lpIndexMgr  = NULL;
	DWORD                   dwIndex     = 0;
	__PTE                   pte;
	__PDE                   pde;
	DWORD                   dwFlags1    = 0;

	if((NULL == lpThis) || (NULL == lpVirtualAddr)) //Parameters check.
		return FALSE;
	if((dwFlags & PTE_FLAG_PRESENT) && (NULL == lpPhysicalAddr))  //Invalidate combination.
		return FALSE;
	if(lpPhysicalAddr && ((DWORD)lpPhysicalAddr & (~PTE_ADDRESS_MASK)))  //The physical
		                                                                 //address is not
																		 //page aligment.
	{
		return FALSE;
	}

	lpIndexMgr = (__PAGE_INDEX_MANAGER*)lpThis;
	dwFlags    = dwFlags & PTE_FLAGS_MASK;    //Only keep the flags bit.

	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;  //Get the page directory index.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags1);
	if(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[dwIndex])) //The page directory entry is NULL
	{
		lpNewPte = (__PTE*)KMemAlloc(sizeof(__PTE)*PT_SIZE,
			KMEM_SIZE_TYPE_4K);   //Allocate a new page table.
		if(NULL == lpNewPte)      //Failed to allocate.
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
			return FALSE;
		}
		memzero((LPVOID)lpNewPte,sizeof(__PTE)*PT_SIZE);    //Clear the newly allocated PTE.
		INIT_PDE_TO_DEFAULT(pde);
		FORM_PDE_ENTRY(pde,(DWORD)lpNewPte);
		lpIndexMgr->lpPdAddress[dwIndex] = pde;    //Set the PDE entry.

		dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if(dwFlags & PTE_FLAG_PRESENT)  //If present.
			FORM_PTE_ENTRY(pte,(DWORD)lpPhysicalAddr);
		lpNewPte[dwIndex] = pte;         //Set the page table entry.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
		return TRUE;
	}
	else    //The page directory entry is occupied.
	{
		lpNewPte = (__PTE*)((DWORD)(lpIndexMgr->lpPdAddress[dwIndex]) & PTE_ADDRESS_MASK);
		dwIndex  = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;  //Get index.
		pte      = lpNewPte[dwIndex];    //Now,the page table entry is got.
		INIT_PTE_TO_NULL(pte);
		SET_PTE_FLAGS(pte,dwFlags);
		if(dwFlags & PTE_FLAG_PRESENT)
			FORM_PTE_ENTRY(pte,(DWORD)lpPhysicalAddr);
		lpNewPte[dwIndex] = pte;    //Set the page table entry.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
		return TRUE;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
	return FALSE;
}

//
//The implementation of SetPageFlags routine.
//

static BOOL SetPageFlags(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr,LPVOID lpPhysicalAddr,
						 DWORD dwFlags)
{
	__PAGE_INDEX_MANAGER*         lpManager      = NULL;
	DWORD                         dwIndex        = 0;
	__PTE*                        lpPte          = NULL;
	__PTE                         pte;
	DWORD                         dwFlags1       = 0;

	if((NULL == lpThis) || (NULL == lpVirtualAddr)) //Invalidate parameters.
		return FALSE;
	if((NULL == lpPhysicalAddr) && (dwFlags & PTE_FLAG_PRESENT)) //Invalidate combination.
		return FALSE;
	if((dwFlags & PTE_FLAG_PRESENT) && ((DWORD)lpPhysicalAddr & (~PTE_ADDRESS_MASK)))
		return FALSE;

	lpManager = (__PAGE_INDEX_MANAGER*)lpThis;
	dwFlags  &= PTE_FLAGS_MASK;    //Only keep the flag bits.
	dwIndex   = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT; //Get the page direcroty's index.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags1);
	if(EMPTY_PDE_ENTRY(lpManager->lpPdAddress[dwIndex])) //The PDE is invalidate.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
		return FALSE;
	}
	lpPte = (__PTE*)((DWORD)lpManager->lpPdAddress[dwIndex] & PTE_ADDRESS_MASK);
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
	if(EMPTY_PTE_ENTRY(lpPte[dwIndex]))    //The page table entry is NULL.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
		return FALSE;
	}
	//
	//The following code initializes a new PTE,and replaces the old one.
	//
	INIT_PTE_TO_NULL(pte);
	SET_PTE_FLAGS(pte,dwFlags);
	if(dwFlags & PTE_FLAG_PRESENT)    //Should set a physical page frame.
		FORM_PTE_ENTRY(pte,(DWORD)lpPhysicalAddr);
	lpPte[dwIndex] = pte;    //Set the page table entry.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags1);
	return TRUE;
}

//
//The implementation of ReleasePage routine.
//

static VOID ReleasePage(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__PAGE_INDEX_MANAGER*        lpIndexMgr     = (__PAGE_INDEX_MANAGER*)lpThis;
	DWORD                        dwIndex        = 0;
	__PTE*                       lpPte          = NULL;
	DWORD                        dwFlags        = 0;

	if((NULL == lpIndexMgr) || (NULL == lpVirtualAddr)) //Invalidate parameters.
		return;
	dwIndex = (DWORD)lpVirtualAddr >> PD_OFFSET_SHIFT;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(EMPTY_PDE_ENTRY(lpIndexMgr->lpPdAddress[dwIndex])) //The PDE is NULL.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	lpPte   = (__PTE*)((DWORD)lpIndexMgr->lpPdAddress[dwIndex] & PTE_ADDRESS_MASK);
	dwIndex = ((DWORD)lpVirtualAddr & PTE_INDEX_MASK) >> PT_OFFSET_SHIFT;
	if(EMPTY_PTE_ENTRY(lpPte[dwIndex]))  //The page table entry is NULL.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	//
	//Clear the page table entry.
	//
	INIT_PTE_TO_NULL(lpPte[dwIndex]);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

#endif
