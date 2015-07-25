//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : VMM.CPP
//    Module Funciton           : 
//                                This module countains virtual memory manager's implementation
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"

//Virtual memory management function only available when this flag is defined.
#ifdef __CFG_SYS_VMM

//
//This routine is used to debug the virtual memory manager.
//It prints out all virtual areas the current memory manager has.
//

VOID PrintVirtualArea(__VIRTUAL_MEMORY_MANAGER* lpMemMgr)
{
	CHAR                               strBuff[12];
	__VIRTUAL_AREA_DESCRIPTOR*         lpVad          = NULL;
	LPVOID                             lpAddr         = NULL;

	if(NULL == lpMemMgr)
		return;
	lpVad = lpMemMgr->lpListHdr;
	PrintLine("    Virtuam memory manager's reserved area :");
	while(lpVad)
	{
		PrintLine("---------------------");
		//PrintLine((LPSTR)&lpVad->strName[0]);
		lpAddr = lpVad->lpStartAddr;
		Hex2Str((DWORD)lpAddr,strBuff);
		PrintLine(strBuff);
		lpAddr = lpVad->lpEndAddr;
		Hex2Str((DWORD)lpAddr,strBuff);
		PrintLine(strBuff);
		lpVad = lpVad->lpNext;
	}
	PrintLine("    Finished to print out.");
}

//
//Declaration for member routines.
//

static LPVOID kVirtualAlloc(__COMMON_OBJECT*,LPVOID,DWORD,DWORD,DWORD,UCHAR*,LPVOID);
static LPVOID GetPdAddress(__COMMON_OBJECT*);
static VOID   kVirtualFree(__COMMON_OBJECT*,LPVOID);
static VOID InsertIntoList(__COMMON_OBJECT*,__VIRTUAL_AREA_DESCRIPTOR*);

//
//The implementation of VmmInitialize routine.
//

BOOL VmmInitialize(__COMMON_OBJECT* lpThis)
{
	__VIRTUAL_MEMORY_MANAGER*       lpManager      = NULL;
	__PAGE_INDEX_MANAGER*           lpPageIndexMgr = NULL;
	__VIRTUAL_AREA_DESCRIPTOR*      lpVad          = NULL;
	BOOL                            bResult        = FALSE;

	if(NULL == lpThis)    //Parameter check.
	{
		return FALSE;
	}

	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	lpManager->VirtualAlloc     = kVirtualAlloc;
	lpManager->VirtualFree      = kVirtualFree;
	lpManager->GetPdAddress     = GetPdAddress;
	lpManager->dwVirtualAreaNum = 0;

	//
	//The following code creates the page index manager object.
	//
	lpPageIndexMgr = (__PAGE_INDEX_MANAGER*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PAGE_INDEX_MANAGER);
	if(NULL == lpPageIndexMgr)    //Failed to create the page index manager object.
	{
		goto __TERMINAL;
	}

	if(!lpPageIndexMgr->Initialize((__COMMON_OBJECT*)lpPageIndexMgr))  //Can not initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpPageIndexMgr);
		lpPageIndexMgr = NULL;
		goto __TERMINAL;
	}
	lpManager->lpPageIndexMgr = lpPageIndexMgr;

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY);
	if(NULL == lpVad)
		goto __TERMINAL;

#define SET(member,value) lpVad->member = value

	SET(lpManager,      lpManager);
	SET(lpStartAddr,    VIRTUAL_MEMORY_KERNEL_START);
	SET(lpEndAddr,      (LPVOID)VIRTUAL_MEMORY_KERNEL_END);
	SET(lpNext,         NULL);
	SET(dwAccessFlags,  VIRTUAL_AREA_ACCESS_RW);
	SET(dwAllocFlags,   VIRTUAL_AREA_ALLOCATE_COMMIT);
	SET(lpLeft,		    NULL);
	SET(lpRight,        NULL);
	SET(dwCacheFlags,   VIRTUAL_AREA_CACHE_NORMAL);

#undef SET

	__INIT_ATOMIC(lpVad->Reference);
	StrCpy((LPSTR)"System Kernel",(LPSTR)&lpVad->strName[0]);

	//
	//Insert the system kernel area into virtual memory manager's list.
	//
	InsertIntoList((__COMMON_OBJECT*)lpManager,lpVad);
	bResult = TRUE;    //Commit the whole transaction.

__TERMINAL:
	if(!bResult)
	{
		if(lpPageIndexMgr)
			ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpPageIndexMgr);    //Destroy the page index manager object.
		if(lpVad)
			KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);  //Free memory.
		return FALSE;
	}
	return TRUE;
}

//
//The implementation of VmmUninitialize routine.
//

VOID VmmUninitialize(__COMMON_OBJECT* lpThis)
{
	__PAGE_INDEX_MANAGER*       lpPageIndexMgr  = NULL;
	__VIRTUAL_MEMORY_MANAGER*   lpManager       = NULL;
	__VIRTUAL_AREA_DESCRIPTOR*  lpVad           = NULL;

	if(NULL == lpThis)    //Parameter check.
		return;

	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	//
	//Here,we should delete all the virtual areas belong to this virtual memory manager.
	//

	lpPageIndexMgr = lpManager->lpPageIndexMgr;
	if(lpPageIndexMgr)  //Destroy the page index manager object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpPageIndexMgr);
	}
	return;
}

//
//This macros is used to determin if a virtual address is within a virtual area.
//
#define WITHIN_VIRTUAL_AREA(lpAddr,lpVa) \
	(((DWORD)((lpVa)->lpStartAddr) <= (DWORD)lpAddr) && \
     ((DWORD)((lpVa)->lpEndAddr)   >= (DWORD)lpAddr))

//
//Check if a virtual memory address is between two virtual areas.
//
#define BETWEEN_VIRTUAL_AREA(lpAddr,lpVa1,lpVa2) \
	(((DWORD)((lpVa1)->lpEndAddr) < (DWORD)lpAddr) && \
	 ((DWORD)((lpVa2)->lpStartAddr) > (DWORD)lpAddr))

//
//Check if a virtual memory address is between two virtual addresses.
//
#define BETWEEN_VIRTUAL_ADDRESS(lpAddr,lpAddr1,lpAddr2) \
	(((DWORD)lpAddr1 <= (DWORD)lpAddr) && \
	 ((DWORD)lpAddr2 >  (DWORD)lpAddr))

//
//SearchVirtualArea_l is a helper routine,used to search a proper virtual area in the
//Virtual Memory Space,the "l" menas it searchs in the virtual area list.
//CAUTION: This routine is not safe,it's the caller's responsibility to guarantee the
//safety.
//
static LPVOID SearchVirtualArea_l(__COMMON_OBJECT* lpThis,LPVOID lpDesiredAddr,DWORD dwSize)
{
	__VIRTUAL_MEMORY_MANAGER*       lpMemMgr           = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR*      lpVad              = NULL;
	LPVOID                          lpStartAddr        = NULL;
	LPVOID                          lpEndAddr          = NULL;
	LPVOID                          lpDesiredEnd       = NULL;
	LPVOID                          lpPotentialStart   = NULL;
	LPVOID                          lpPotentialEnd     = NULL;
	BOOL                            bFind              = FALSE;

	if((NULL == lpThis) || (0 == dwSize)) //Invalidate parameters.
		return NULL;
	lpVad       = lpMemMgr->lpListHdr;
	if(NULL == lpVad)    //There is not any virtual area in the space.
		return lpDesiredAddr;    //Successfully.
	lpEndAddr       = lpVad->lpStartAddr;
	lpDesiredEnd    = (LPVOID)((DWORD)lpDesiredAddr + dwSize - 1);
	//lpPotentialEnd  = (LPVOID)((DWORD)lpPotentialStart + dwSize);
	while(lpVad)
	{
		if(BETWEEN_VIRTUAL_ADDRESS(lpDesiredAddr,lpStartAddr,lpEndAddr) && 
		   BETWEEN_VIRTUAL_ADDRESS(lpDesiredEnd,lpStartAddr,lpEndAddr))  //Find one.
		{
			return lpDesiredAddr;
		}
		if(!bFind)                      //To check if the gap can statisfy the required size,
			                            //that is,if the gap can be a potential virtual area.
		{
			lpPotentialStart = lpStartAddr;
			lpPotentialEnd   = (LPVOID)((DWORD)lpPotentialStart + dwSize - 1);
			if(BETWEEN_VIRTUAL_ADDRESS(lpPotentialEnd,lpStartAddr,lpEndAddr))
				                                                //Find a VA statisfy the
				                                                //original required size.
				bFind = TRUE;    //Set potential address to NULL.
		}
		lpStartAddr = (LPVOID)((DWORD)lpVad->lpEndAddr + 1);
		lpVad       = lpVad->lpNext;
		if(lpVad)
			lpEndAddr = lpVad->lpStartAddr;
		else
			lpEndAddr = (LPVOID)VIRTUAL_MEMORY_END;
	}
	if(BETWEEN_VIRTUAL_ADDRESS(lpDesiredAddr,lpStartAddr,lpEndAddr) &&
	   BETWEEN_VIRTUAL_ADDRESS(lpDesiredEnd,lpStartAddr,lpEndAddr))
	   return lpDesiredAddr;
	if(!bFind)    //Have not find a potential address,so try the last once.
	{
		lpPotentialStart = lpStartAddr;
		lpPotentialEnd   = (LPVOID)((DWORD)(lpPotentialStart) + dwSize - 1);
		if(BETWEEN_VIRTUAL_ADDRESS(lpPotentialEnd,lpStartAddr,lpEndAddr))  //Can statisfy.
			return lpPotentialStart;
		else
			return NULL;
	}
	else    //Have found a virtual area that statisfy the original request,though the start
		    //address not the original desired one.
	{
		return lpPotentialStart;
	}
	return NULL;
}

//
//SearchVirtualArea_t is a same routine as SearchVirtualArea_l,the difference is,this
//routine searchs in the AVL tree.
//
static LPVOID SearchVirtualArea_t(__COMMON_OBJECT* lpThis,LPVOID lpStartAddr,DWORD dwSize)
{
	return NULL;    //We will implement it in the future :-)
}

//
//InsertIntoList routine,this routine inserts a virtual area descriptor object into
//virtual area list.
//
static VOID InsertIntoList(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER*       lpMemMgr         =	(__VIRTUAL_MEMORY_MANAGER*)lpThis;
	DWORD                           dwFlags          = 0;
	__VIRTUAL_AREA_DESCRIPTOR*      lpFirst          =  NULL;
	__VIRTUAL_AREA_DESCRIPTOR*      lpSecond         =  NULL;

	if((NULL == lpThis) || (NULL == lpVad)) //Invalidate parameters.
		return;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpFirst  = lpMemMgr->lpListHdr;
	if(NULL == lpFirst)    //There is not any element in the list.
	{
		lpMemMgr->lpListHdr        = lpVad;
		lpMemMgr->dwVirtualAreaNum ++;          //Increment the reference counter.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	lpSecond = lpFirst;
	while(lpFirst)
	{
		if((DWORD)lpFirst->lpStartAddr > (DWORD)lpVad->lpStartAddr)    //Find the proper position.
			break;
		lpSecond = lpFirst;
		lpFirst  = lpFirst->lpNext;
	}
	if(lpSecond == lpFirst)  //Should be the first element in the list.
	{
		lpVad->lpNext       = lpMemMgr->lpListHdr;
		lpMemMgr->lpListHdr = lpVad;
		lpMemMgr->dwVirtualAreaNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	else    //Should not be the first element.
	{
		lpVad->lpNext    = lpSecond->lpNext;
		lpSecond->lpNext = lpVad;
	}
	lpMemMgr->dwVirtualAreaNum ++;    //Increment the virtual area's total number.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
}

//
//InsertIntoTree routine,the same as above except that this routine is used to insert
//a virtual memory area into AVL tree.
//
static VOID InsertIntoTree(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	return;  //We will implement this routine in the future. :-)
}

//
//A helper routine used to get a virtual area descriptor object by a virtual address.
//The label "l" means the search target is list.
//
static __VIRTUAL_AREA_DESCRIPTOR* GetVaByAddr_l(__COMMON_OBJECT* lpThis,LPVOID lpAddr)
{
	__VIRTUAL_MEMORY_MANAGER*     lpMemMgr = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR*    lpVad    = NULL;

	if((NULL == lpThis) || (NULL == lpAddr)) //Invalidate parameters.
		return NULL;
	lpVad = lpMemMgr->lpListHdr;
	while(lpVad)
	{
		if(WITHIN_VIRTUAL_AREA(lpAddr,lpVad))
			return lpVad;
		lpVad = lpVad->lpNext;
	}
	return lpVad;    //If can not search a proper VA,then NULL is returned.
}

//
//Get a virtual area descriptor object from AVL tree according to a virtual 
//memory address.
//
static __VIRTUAL_AREA_DESCRIPTOR* GetVaByAddr_t(__COMMON_OBJECT* lpThis,LPVOID lpAddr)
{
	return NULL;  //We will complete this routine in the future. :-)
}

//
//DoCommit routine.When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_COMMIT,
//then is routine is called by VirtualAlloc.
//
static LPVOID DoCommit(__COMMON_OBJECT* lpThis,
					   LPVOID           lpDesiredAddr,
					   DWORD            dwSize,
					   DWORD            dwAllocFlags,
					   DWORD            dwAccessFlags,
					   UCHAR*           lpVaName,
					   LPVOID           lpReserved)
{
	__VIRTUAL_MEMORY_MANAGER*           lpMemMgr       = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__PAGE_INDEX_MANAGER*               lpIndexMgr     = NULL;
	LPVOID                              lpStartAddr    = NULL;
	LPVOID                              lpPhysical     = NULL;
	DWORD                               dwFlags        = 0;
	DWORD                               dwPteFlags     = 0;
	__VIRTUAL_AREA_DESCRIPTOR*          lpVad          = NULL;
	BOOL                                bResult        = FALSE;

	if((NULL == lpThis) || (NULL == lpDesiredAddr))
		return NULL;
	if(dwAllocFlags != VIRTUAL_AREA_ALLOCATE_COMMIT)   //This routine only process commit.
		return NULL;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)     //Should search in list.
		lpVad = GetVaByAddr_l(lpThis,lpDesiredAddr);
	else                                               //Should search in the AVL tree.
		lpVad = GetVaByAddr_t(lpThis,lpDesiredAddr);

	if(NULL == lpVad)    //The virtual memory area is not reserved before.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	lpStartAddr = lpVad->lpStartAddr;
	dwSize      = (DWORD)lpVad->lpEndAddr - (DWORD)lpStartAddr + 1;
	lpPhysical  = PageFrameManager.FrameAlloc((__COMMON_OBJECT*)&PageFrameManager,
		dwSize,
		0);
	if(NULL == lpPhysical)    //Can not allocate physical memory.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//
	//The following code reserves page table entries for this virtual area.
	//
	lpVad->dwAllocFlags = VIRTUAL_AREA_ALLOCATE_COMMIT;

	dwPteFlags = PTE_FLAGS_FOR_NORMAL;    //Normal flags.
	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			PrintLine("Fatal Error : Internal data structure is not consist.");
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical  = (LPVOID)((DWORD)lpPhysical  + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;    //Indicate that the whole operation is successfully.

__TERMINAL:
	if(!bResult)    //The transaction is failed.
	{
		if(lpPhysical)
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
			lpPhysical,
			dwSize);
		return NULL;
	}
	return lpDesiredAddr;    //The lpDesiredAddr should never be changed.
}

//
//DoReserve routine.When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_RESERVE,
//then is routine is called by VirtualAlloc.
//
static LPVOID DoReserve(__COMMON_OBJECT* lpThis,
					   LPVOID           lpDesiredAddr,
					   DWORD            dwSize,
					   DWORD            dwAllocFlags,
					   DWORD            dwAccessFlags,
					   UCHAR*           lpVaName,
					   LPVOID           lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR*              lpVad       = NULL;
	__VIRTUAL_MEMORY_MANAGER*               lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                  lpStartAddr = lpDesiredAddr;
	LPVOID                                  lpEndAddr   = NULL;
	DWORD                                   dwFlags     = 0;
	BOOL                                    bResult     = FALSE;
	LPVOID                                  lpPhysical  = NULL;
	__PAGE_INDEX_MANAGER*                   lpIndexMgr  = NULL;
	DWORD                                   dwPteFlags  = 0;

	if((NULL == lpThis) || (0 == dwSize))    //Parameter check.
		return NULL;
	if(VIRTUAL_AREA_ALLOCATE_RESERVE != dwAllocFlags)    //Invalidate flags.
		return NULL;
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	if(NULL == lpIndexMgr)    //Validate.
		return NULL;

	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1)); //Round up to page.

	lpEndAddr   = (LPVOID)((DWORD)lpDesiredAddr + dwSize );
	lpEndAddr   = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ? 
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1) 
		: ((DWORD)lpEndAddr - 1)); //Round down to page.

	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;  //Get the actually size.

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY); //In order to avoid calling KMemAlloc routine in the
	                         //critical section,we first call it here.
	if(NULL == lpVad)        //Can not allocate memory.
		goto __TERMINAL;
	lpVad->lpManager       = lpMemMgr;
	lpVad->lpStartAddr     = NULL;
	lpVad->lpEndAddr       = NULL;
	lpVad->lpNext          = NULL;
	lpVad->dwAccessFlags   = dwAccessFlags;
	lpVad->dwAllocFlags    = dwAllocFlags;
	__INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft          = NULL;
	lpVad->lpRight         = NULL;
	if(lpVaName)
	{
		if(StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		StrCpy((LPSTR)lpVad->strName[0],(LPSTR)lpVaName);    //Set the virtual area's name.
	}
	else
		lpVad->strName[0] = 0;
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_NORMAL;

	//
	//The following code searchs virtual area list or AVL tree,to check if the lpDesiredAddr
	//is occupied,if so,then find a new one.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)  //Should search in the list.
		lpStartAddr = SearchVirtualArea_l((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	else    //Should search in the AVL tree.
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	if(NULL == lpStartAddr)    //Can not find proper virtual area.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->lpEndAddr   = (LPVOID)((DWORD)lpStartAddr + dwSize -1);
	lpDesiredAddr      = lpStartAddr;

	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)
		InsertIntoList((__COMMON_OBJECT*)lpMemMgr,lpVad);  //Insert into list or tree.
	else
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr,lpVad);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;    //Indicate that the whole operation is successfully.
	                   //In this operation(only reserve),we do not commit page table entries,
	                   //so,if access the memory space(read or write) of this range,will
	                   //cause a page fault exception.
	                   //The caller must call VirtualAlloc routine again to commit this
	                   //block of area before access it.

__TERMINAL:
	if(!bResult)   //Process failed.
	{
		if(lpVad)
			KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);
		if(lpPhysical)
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
			lpPhysical,
			dwSize);
		return NULL;
	}
	return lpDesiredAddr;
}

//
//DoReserveAndCommit routine.When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_ALL or
//VIRTUAL_AREA_ALLOCATE_DEFAULT,then is routine is called by VirtualAlloc.
//
static LPVOID DoReserveAndCommit(__COMMON_OBJECT* lpThis,
								 LPVOID           lpDesiredAddr,
								 DWORD            dwSize,
								 DWORD            dwAllocFlags,
								 DWORD            dwAccessFlags,
								 UCHAR*           lpVaName,
								 LPVOID           lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR*              lpVad       = NULL;
	__VIRTUAL_MEMORY_MANAGER*               lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                  lpStartAddr = lpDesiredAddr;
	LPVOID                                  lpEndAddr   = NULL;
	DWORD                                   dwFlags     = 0;
	BOOL                                    bResult     = FALSE;
	LPVOID                                  lpPhysical  = NULL;
	__PAGE_INDEX_MANAGER*                   lpIndexMgr  = NULL;
	DWORD                                   dwPteFlags  = 0;

	if((NULL == lpThis) || (0 == dwSize))    //Parameter check.
		return NULL;
	if(VIRTUAL_AREA_ALLOCATE_ALL != dwAllocFlags)    //Invalidate flags.
		return NULL;
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	if(NULL == lpIndexMgr)    //Validate.
		return NULL;

	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1)); //Round up to page.

	lpEndAddr   = (LPVOID)((DWORD)lpDesiredAddr + dwSize );
	lpEndAddr   = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ? 
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1) 
		: ((DWORD)lpEndAddr - 1)); //Round down to page.

	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;  //Get the actually size.

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY); //In order to avoid calling KMemAlloc routine in the
	                         //critical section,we first call it here.
	if(NULL == lpVad)        //Can not allocate memory.
	{
		PrintLine("In DoReserveAndCommit: Can not allocate memory for VAD.");
		goto __TERMINAL;
	}
	lpVad->lpManager       = lpMemMgr;
	lpVad->lpStartAddr     = NULL;
	lpVad->lpEndAddr       = NULL;
	lpVad->lpNext          = NULL;
	lpVad->dwAccessFlags   = dwAccessFlags;
	lpVad->dwAllocFlags    = VIRTUAL_AREA_ALLOCATE_COMMIT; //dwAllocFlags;
	__INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft          = NULL;
	lpVad->lpRight         = NULL;
	if(lpVaName)
	{
		if(StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		StrCpy((LPSTR)lpVad->strName[0],(LPSTR)lpVaName);    //Set the virtual area's name.
	}
	else
		lpVad->strName[0] = 0;
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_NORMAL;

	lpPhysical = PageFrameManager.FrameAlloc((__COMMON_OBJECT*)&PageFrameManager,
		dwSize,
		0);                  //Allocate physical memory pages.In order to reduce the time
	                          //in critical section,we allocate physical memory here.
	if(NULL == lpPhysical)    //Can not allocate physical memory.
	{
		PrintLine("In DoReserveAndCommit: Can not allocate physical memory.");
		goto __TERMINAL;
	}

	//
	//The following code searchs virtual area list or AVL tree,to check if the lpDesiredAddr
	//is occupied,if so,then find a new one.
	//
	lpEndAddr = lpStartAddr;    //Save the lpStartAddr,because the lpStartAddr may changed
	                            //after the SearchVirtualArea_X is called.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)  //Should search in the list.
		lpStartAddr = SearchVirtualArea_l((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	else    //Should search in the AVL tree.
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	if(NULL == lpStartAddr)    //Can not find proper virtual area.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		PrintLine("In DoReserveAndCommit: SearchVirtualArea failed.");
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->lpEndAddr   = (LPVOID)((DWORD)lpStartAddr + dwSize -1);
	if(!(lpStartAddr == lpEndAddr))    //Have not get the desired area.
		lpDesiredAddr      = lpStartAddr;

	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)
		InsertIntoList((__COMMON_OBJECT*)lpMemMgr,lpVad);  //Insert into list or tree.
	else
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr,lpVad);

	//
	//The following code reserves page table entries for the committed memory.
	//
	dwPteFlags = PTE_FLAGS_FOR_NORMAL;    //Normal flags.

	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			PrintLine("Fatal Error : Internal data structure is not consist.");
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical  = (LPVOID)((DWORD)lpPhysical  + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;    //Indicate that the whole operation is successfully.

__TERMINAL:
	if(!bResult)   //Process failed.
	{
		if(lpVad)
			KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);
		if(lpPhysical)
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
			lpPhysical,
			dwSize);
		return NULL;
	}
	return lpDesiredAddr;
}

//
//DoIoMap routine.When VirtualAlloc is called with VIRTUAL_AREA_ALLOCATE_IO,
//then is routine is called by VirtualAlloc.
//
static LPVOID DoIoMap(__COMMON_OBJECT* lpThis,
					  LPVOID           lpDesiredAddr,
					  DWORD            dwSize,
					  DWORD            dwAllocFlags,
					  DWORD            dwAccessFlags,
					  UCHAR*           lpVaName,
					  LPVOID           lpReserved)
{
	__VIRTUAL_AREA_DESCRIPTOR*              lpVad       = NULL;
	__VIRTUAL_MEMORY_MANAGER*               lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                  lpStartAddr = lpDesiredAddr;
	LPVOID                                  lpEndAddr   = NULL;
	DWORD                                   dwFlags     = 0;
	BOOL                                    bResult     = FALSE;
	LPVOID                                  lpPhysical  = NULL;
	__PAGE_INDEX_MANAGER*                   lpIndexMgr  = NULL;
	DWORD                                   dwPteFlags  = 0;

	if((NULL == lpThis) || (0 == dwSize))    //Parameter check.
		return NULL;
	if(VIRTUAL_AREA_ALLOCATE_IO != dwAllocFlags)    //Invalidate flags.
		return NULL;
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	if(NULL == lpIndexMgr)    //Validate.
		return NULL;

	lpStartAddr = (LPVOID)((DWORD)lpStartAddr & ~(PAGE_FRAME_SIZE - 1)); //Round up to page.

	lpEndAddr   = (LPVOID)((DWORD)lpDesiredAddr + dwSize );
	lpEndAddr   = (LPVOID)(((DWORD)lpEndAddr & (PAGE_FRAME_SIZE - 1)) ? 
		(((DWORD)lpEndAddr & ~(PAGE_FRAME_SIZE - 1)) + PAGE_FRAME_SIZE - 1) 
		: ((DWORD)lpEndAddr - 1)); //Round down to page.

	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;  //Get the actually size.

	lpVad = (__VIRTUAL_AREA_DESCRIPTOR*)KMemAlloc(sizeof(__VIRTUAL_AREA_DESCRIPTOR),
		KMEM_SIZE_TYPE_ANY); //In order to avoid calling KMemAlloc routine in the
	                         //critical section,we first call it here.
	if(NULL == lpVad)        //Can not allocate memory.
		goto __TERMINAL;
	lpVad->lpManager       = lpMemMgr;
	lpVad->lpStartAddr     = NULL;
	lpVad->lpEndAddr       = NULL;
	lpVad->lpNext          = NULL;
	lpVad->dwAccessFlags   = dwAccessFlags;
	lpVad->dwAllocFlags    = dwAllocFlags;
	__INIT_ATOMIC(lpVad->Reference);
	lpVad->lpLeft          = NULL;
	lpVad->lpRight         = NULL;
	if(lpVaName)
	{
		if(StrLen((LPSTR)lpVaName) > MAX_VA_NAME_LEN)
			lpVaName[MAX_VA_NAME_LEN - 1] = 0;
		StrCpy((LPSTR)lpVad->strName[0],(LPSTR)lpVaName);    //Set the virtual area's name.
	}
	else
		lpVad->strName[0] = 0;
	lpVad->dwCacheFlags = VIRTUAL_AREA_CACHE_IO;

	//
	//The following code searchs virtual area list or AVL tree,to check if the lpDesiredAddr
	//is occupied,if so,then find a new one.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)  //Should search in the list.
		lpStartAddr = SearchVirtualArea_l((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	else    //Should search in the AVL tree.
		lpStartAddr = SearchVirtualArea_t((__COMMON_OBJECT*)lpMemMgr,lpStartAddr,dwSize);
	if(NULL == lpStartAddr)    //Can not find proper virtual area.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	lpVad->lpStartAddr = lpStartAddr;
	lpVad->lpEndAddr   = (LPVOID)((DWORD)lpStartAddr + dwSize -1);
	lpDesiredAddr      = lpStartAddr;

	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)
		InsertIntoList((__COMMON_OBJECT*)lpMemMgr,lpVad);  //Insert into list or tree.
	else
		InsertIntoTree((__COMMON_OBJECT*)lpMemMgr,lpVad);

	//
	//The following code reserves page table entries for the committed memory.
	//
	dwPteFlags = PTE_FLAGS_FOR_IOMAP;    //IO map flags,that is,this memory range will not
	                                     //use hardware cache.
	lpPhysical = lpStartAddr;

	while(dwSize)
	{
		if(!lpIndexMgr->ReservePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr,lpPhysical,dwPteFlags))
		{
			PrintLine("Fatal Error : Internal data structure is not consist.");
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			goto __TERMINAL;
		}
		dwSize -= PAGE_FRAME_SIZE;
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		lpPhysical  = (LPVOID)((DWORD)lpPhysical  + PAGE_FRAME_SIZE);
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;    //Indicate that the whole operation is successfully.

__TERMINAL:
	if(!bResult)   //Process failed.
	{
		if(lpVad)
			KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);
		if(lpPhysical)
			PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
			lpPhysical,
			dwSize);
		return NULL;
	}
	return lpDesiredAddr;
}

//
//The implementation of VirtualAlloc routine.
//

static LPVOID kVirtualAlloc(__COMMON_OBJECT* lpThis,
						   LPVOID           lpDesiredAddr,
						   DWORD            dwSize,
						   DWORD            dwAllocFlags,
						   DWORD            dwAccessFlags,
						   UCHAR*           lpVaName,
						   LPVOID           lpReserved)
{
	switch(dwAllocFlags)
	{
	case VIRTUAL_AREA_ALLOCATE_IO:    //Call DoIoMap only.
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
		DoCommit(lpThis,
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
//A helper routine,used to delete a virtual area descriptor object from list.
//

static VOID DelVaFromList(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER*      lpMemMgr  = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR*     lpNext    = NULL;
	__VIRTUAL_AREA_DESCRIPTOR*     lpPrev    = NULL;

	if((NULL == lpThis) || (NULL == lpVad)) //Invalidate parameters.
		return;
	lpPrev = lpMemMgr->lpListHdr;
	lpNext = lpMemMgr->lpListHdr;
	if(NULL == lpPrev)   //There is not any VAD in list.
	{
		//__POTENTIAL_BUT();
		return;
	}
	if(lpPrev == lpVad)  //The vad to be deleted is the first one.
	{
		lpMemMgr->lpListHdr = lpPrev->lpNext;
		lpMemMgr->dwVirtualAreaNum --;
		return;
	}
	while(lpPrev)
	{
		if(lpVad == lpPrev)
			break;
		lpNext = lpPrev;
		lpPrev = lpPrev->lpNext;
	}
	if(NULL == lpPrev)    //Can not find the virtual area descriptor object in list.
		return;
	lpNext->lpNext = lpPrev->lpNext; //Delete the VAD object.
	lpMemMgr->dwVirtualAreaNum --;
	return;
}

//
//A helper routine,used to delete a virtual area descriptor object from AVL
//tree.
//
static VOID DelVaFromTree(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	return; //We will complete this routine in the future.
}

//
//A helper routine,used to release virtual area that committed and with physical memory
//pages allocated.
//
static VOID ReleaseCommit(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER*             lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                lpStartAddr = NULL;
	LPVOID                                lpEndAddr   = NULL;
	__PAGE_INDEX_MANAGER*                 lpIndexMgr  = NULL;
	DWORD                                 dwSize      = 0;

	if((NULL == lpThis) || (NULL == lpVad)) //Invalidate parameters.
		return;
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	if(NULL == lpIndexMgr)    //Fatal error.
		return;
	lpStartAddr = lpVad->lpStartAddr;
	lpEndAddr   = lpVad->lpEndAddr;
	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;

	lpEndAddr   = lpIndexMgr->GetPhysicalAddress((__COMMON_OBJECT*)lpIndexMgr,
		lpStartAddr);    //Get the physical memory address.
	PageFrameManager.FrameFree((__COMMON_OBJECT*)&PageFrameManager,
		lpEndAddr,
		dwSize);    //Release the physical page.

	while(dwSize)
	{
		lpIndexMgr->ReleasePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr);    //Release the page table entry that this area occupies.
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		dwSize -= PAGE_FRAME_SIZE;
	}
}

//
//A helper routine,used to release virtual area that committed,but without physical
//memory pages allocated,such as IO map zone.
//This routine only release the page table entry that this virtual area occupies.
//
static VOID ReleaseIoMap(__COMMON_OBJECT* lpThis,__VIRTUAL_AREA_DESCRIPTOR* lpVad)
{
	__VIRTUAL_MEMORY_MANAGER*             lpMemMgr    = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	LPVOID                                lpStartAddr = NULL;
	LPVOID                                lpEndAddr   = NULL;
	__PAGE_INDEX_MANAGER*                 lpIndexMgr  = NULL;
	DWORD                                 dwSize      = 0;

	if((NULL == lpThis) || (NULL == lpVad)) //Invalidate parameters.
		return;
	lpIndexMgr = lpMemMgr->lpPageIndexMgr;
	if(NULL == lpIndexMgr)    //Fatal error.
		return;
	lpStartAddr = lpVad->lpStartAddr;
	lpEndAddr   = lpVad->lpEndAddr;
	dwSize      = (DWORD)lpEndAddr - (DWORD)lpStartAddr + 1;
	while(dwSize)
	{
		lpIndexMgr->ReleasePage((__COMMON_OBJECT*)lpIndexMgr,
			lpStartAddr);    //Release the page table entry that this area occupies.
		lpStartAddr = (LPVOID)((DWORD)lpStartAddr + PAGE_FRAME_SIZE);
		dwSize -= PAGE_FRAME_SIZE;
	}
}

//
//The implementation of VirtualFree routine.
//This routine frees the virtual area allocated by VirtualAlloc,and
//frees any resource,such as physical memory page,page table entries,
//and virtual area descriptor objects,associated with the virtual
//area object.
//

static VOID kVirtualFree(__COMMON_OBJECT* lpThis,LPVOID lpVirtualAddr)
{
	__VIRTUAL_MEMORY_MANAGER*            lpMemMgr   = (__VIRTUAL_MEMORY_MANAGER*)lpThis;
	__VIRTUAL_AREA_DESCRIPTOR*           lpVad      = NULL;
	__PAGE_INDEX_MANAGER*                lpIndexMgr = NULL;
	BOOL                                 bResult    = FALSE;
	DWORD                                dwFlags    = 0;

	if((NULL == lpThis) || (NULL == lpVirtualAddr)) //Invalidate parameters.
		return;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)  //Should search in the list.
		lpVad = GetVaByAddr_l(lpThis,lpVirtualAddr);    //Get the virtual area descriptor.
	else
		lpVad = GetVaByAddr_t(lpThis,lpVirtualAddr);
	if(NULL == lpVad)    //This virtual address is not allocated yet.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	//
	//Now,we have got the virtual area descriptor object,so,
	//first delete it from list or tree.
	//
	if(lpMemMgr->dwVirtualAreaNum < SWITCH_VA_NUM)  //Delete from list.
		DelVaFromList(lpThis,lpVad);
	else                                            //Delete from tree.
		DelVaFromTree(lpThis,lpVad);

	//
	//According to different allocating type,to do the different actions.
	//
	switch(lpVad->dwAllocFlags)
	{
	case VIRTUAL_AREA_ALLOCATE_COMMIT:    //The virtual memory area is committed.
		ReleaseCommit(lpThis,lpVad);
		KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);  //Release the memory occupied by
		                                                //virtual area descriptor.
		break;
	case VIRTUAL_AREA_ALLOCATE_RESERVE:   //Only reserved.
		KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);  //Only release the memory.
		break;
	case VIRTUAL_AREA_ALLOCATE_IO:        //Committed,but without physical memory pages.
		ReleaseIoMap(lpThis,lpVad);
		KMemFree((LPVOID)lpVad,KMEM_SIZE_TYPE_ANY,0);
		break;
	default:
		break;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;    //Set the completing flags.

__TERMINAL:
	return;
}

//
//The implementation of GetPdAddress routine.
//This routine returns the physical address of page directory.
//

static LPVOID GetPdAddress(__COMMON_OBJECT* lpThis)
{
	__VIRTUAL_MEMORY_MANAGER*       lpManager = NULL;

	if(NULL == lpThis)    //Parameter check.
		return NULL;

	lpManager = (__VIRTUAL_MEMORY_MANAGER*)lpThis;

	return (LPVOID)lpManager->lpPageIndexMgr->lpPdAddress;
}

/***********************************************************************************
************************************************************************************
************************************************************************************
************************************************************************************
***********************************************************************************/

//
//The definition of object Virtual Memory Manager.
//This is one of the global objects,but it is created by CreateObject routine of
//ObjectManager.
//

__VIRTUAL_MEMORY_MANAGER*    lpVirtualMemoryMgr    = NULL;

#endif
