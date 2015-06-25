//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,20 2006
//    Module Name               : HEAP.CPP
//    Module Funciton           : 
//                                This module countains Heap's implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

//#ifndef __STDAFX_H__
#include "StdAfx.h"
//#endif

#include "types.h"
#include "commobj.h"

#include "kapi.h"
#include "heap.h"


//If and only if the VMM function is enabled,heap function is available since
//it's reley on the VMM mechanism.
#ifdef __CFG_SYS_VMM



//
//The implementation of CreateHeap routine.
//This routine does the following:
// 1. Allocate a virtual area according to dwInitSize;
// 3. Create a virtual area node object,to manage the virtual area;
// 2. Create a heap object,and initialize it;
// 4. Insert the virtual area node into heap's virtual area node list;
// 5. Insert the virtual area into heap object's free list;
// 6. Insert the heap object into kernel thread's list;
// 7. If all successful,return the help object's base address.
//

static __HEAP_OBJECT* CreateHeap(DWORD dwInitSize)
{
	__HEAP_OBJECT*              lpHeapObject   = NULL;
	__HEAP_OBJECT*              lpHeapRoot     = NULL;
	__VIRTUAL_AREA_NODE*        lpVirtualArea  = NULL;
	__FREE_BLOCK_HEADER*        lpFreeHeader   = NULL;
	LPVOID                      lpVirtualAddr  = NULL;
	BOOL                        bResult        = FALSE;
	DWORD                       dwFlags        = 0;

	if(dwInitSize > MAX_VIRTUAL_AREA_SIZE)  //Requested size too big.
		return NULL;
	if(dwInitSize < DEFAULT_VIRTUAL_AREA_SIZE)
		dwInitSize = DEFAULT_VIRTUAL_AREA_SIZE;

	//
	//Now,allocate the virtual area.
	//
	lpVirtualAddr = GET_VIRTUAL_AREA(dwInitSize);
	if(NULL == lpVirtualAddr)   //Can not get virtual area.
		goto __TERMINAL;
	lpFreeHeader = (__FREE_BLOCK_HEADER*)lpVirtualAddr;
	lpFreeHeader->dwFlags     = BLOCK_FLAGS_FREE;
	lpFreeHeader->dwBlockSize = dwInitSize - sizeof(__FREE_BLOCK_HEADER); //Caution!!!

	//
	//Now,create a virtual area node object,to manage virtual area.
	//
	lpVirtualArea = (__VIRTUAL_AREA_NODE*)GET_KERNEL_MEMORY(
		sizeof(__VIRTUAL_AREA_NODE));
	if(NULL == lpVirtualArea)  //Can not get memory.
		goto __TERMINAL;
	lpVirtualArea->lpStartAddress = lpVirtualAddr;
	lpVirtualArea->dwAreaSize     = dwInitSize;
	lpVirtualArea->lpNext         = NULL;

	//
	//Now,create a heap object,and initialize it.
	//
	lpHeapObject = (__HEAP_OBJECT*)GET_KERNEL_MEMORY(sizeof(__HEAP_OBJECT));
	if(NULL == lpHeapObject)  //Can not allocate memory.
		goto __TERMINAL;
	lpHeapObject->lpVirtualArea               = lpVirtualArea;  //Virtual area node list.
	lpHeapObject->FreeBlockHeader.dwFlags     |= BLOCK_FLAGS_FREE;
	lpHeapObject->FreeBlockHeader.dwFlags     &= ~BLOCK_FLAGS_USED;
	lpHeapObject->FreeBlockHeader.dwBlockSize = 0;
	lpHeapObject->lpPrev                      = lpHeapObject; //Pointing to itself.
	lpHeapObject->lpNext                      = lpHeapObject; //Pointing to itself.

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);  //Critical section here.
	lpHeapObject->lpKernelThread        = CURRENT_KERNEL_THREAD;
	lpHeapRoot = (__HEAP_OBJECT*)CURRENT_KERNEL_THREAD->lpHeapObject;
	if(NULL == lpHeapRoot)  //Has not any heap yet.
	{
		CURRENT_KERNEL_THREAD->lpHeapObject = (LPVOID)lpHeapObject;
	}
	else  //Has at least one heap object,so insert it into the list.
	{
		lpHeapObject->lpPrev = lpHeapRoot->lpPrev;
		lpHeapObject->lpNext = lpHeapRoot;
		lpHeapObject->lpNext->lpPrev = lpHeapObject;
		lpHeapObject->lpPrev->lpNext = lpHeapObject;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//
	//Now,add the virtual area into the heap object's free list.
	//
	lpFreeHeader->lpPrev = &(lpHeapObject->FreeBlockHeader);
	lpFreeHeader->lpNext = &(lpHeapObject->FreeBlockHeader);
	lpHeapObject->FreeBlockHeader.lpPrev      = lpFreeHeader;
	lpHeapObject->FreeBlockHeader.lpNext      = lpFreeHeader;

	bResult = TRUE;    //The whole operation is successful.

__TERMINAL:
	if(!bResult)    //Failed.
	{
		if(lpVirtualAddr)  //Should release it.
			RELEASE_VIRTUAL_AREA(lpVirtualAddr);
		if(lpHeapObject)
			RELEASE_KERNEL_MEMORY((LPVOID)lpHeapObject);
		if(lpVirtualArea)
			RELEASE_KERNEL_MEMORY((LPVOID)lpVirtualArea);
		lpHeapObject = NULL;  //Should return a NULL flags.
	}

	return lpHeapObject;
}

//
//The implementation of DestroyHeap routine.
//This routine does the following:
// 1. Delete the heap object from kernel thread's heap list;
// 2. Release all virtual areas belong to this heap;
// 3. Release the virtual area list;
// 4. Release the heap object itself.
//
static VOID DestroyHeap(__HEAP_OBJECT* lpHeapObject)
{
	__VIRTUAL_AREA_NODE*       lpVirtualArea  = NULL;
	__VIRTUAL_AREA_NODE*       lpVirtualTmp   = NULL;
	//LPVOID                     lpVirtualAddr  = NULL;
	DWORD                      dwFlags        = 0;

	if(NULL == lpHeapObject)  //Parameter check.
	{
		return;
	}

	if(lpHeapObject == lpHeapObject->lpNext)  //Only one heap object in current thread.
	{
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		lpHeapObject->lpKernelThread->lpHeapObject = NULL;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	}
	else  //Delete itself from the kernel thread's heap list.
	{
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		if(lpHeapObject->lpKernelThread->lpHeapObject == lpHeapObject)
		{
			lpHeapObject->lpKernelThread->lpHeapObject = (LPVOID)lpHeapObject->lpNext;
		}
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

		lpHeapObject->lpPrev->lpNext = lpHeapObject->lpNext;
		lpHeapObject->lpNext->lpPrev = lpHeapObject->lpPrev;
	}

	lpVirtualArea = lpHeapObject->lpVirtualArea;
	while(lpVirtualArea)
	{
		lpVirtualTmp = lpVirtualArea;
		lpVirtualArea = lpVirtualArea->lpNext;
		RELEASE_VIRTUAL_AREA(lpVirtualTmp->lpStartAddress);  //Release the virtual area.
		RELEASE_KERNEL_MEMORY((LPVOID)lpVirtualTmp);
	}

	//
	//Now,should release the heap object itself.
	//
	RELEASE_KERNEL_MEMORY((LPVOID)lpHeapObject);

	return;
}

//
//DestroyAllHeap's implementation.
//This routine destroys all heaps of the current kernel thread.
//
static VOID DestroyAllHeap(void)
{
	__HEAP_OBJECT*       lpHeapObj1 = NULL;
	__HEAP_OBJECT*       lpHeapObj2 = NULL;
	__HEAP_OBJECT*       lpHeapRoot = NULL;

	lpHeapRoot = (__HEAP_OBJECT*)CURRENT_KERNEL_THREAD->lpHeapObject;
	if(NULL == lpHeapRoot)  //Not any heap object.
		return;
	lpHeapObj1 = lpHeapRoot->lpNext;
	while(lpHeapRoot != lpHeapObj1)
	{
		lpHeapObj2 = lpHeapObj1->lpNext;
		DestroyHeap(lpHeapObj1);
		lpHeapObj1 = lpHeapObj2;
	}
	DestroyHeap(lpHeapRoot);  //Destroy the root heap.
}

//
//The following is a help routine,used to print out the heap information of
//a kernel thread.
//
VOID PrintHeapInfo(__KERNEL_THREAD_OBJECT* lpKernelThread)
{
	__HEAP_OBJECT*   lpHeapObject = NULL;
	__HEAP_OBJECT*   lpHeapTmp    = NULL;
	DWORD            dwFlags      = 0;
	__VIRTUAL_AREA_NODE*  lpVirtualArea = NULL;

	if(NULL == lpKernelThread)  //Parameter check.
		return;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpHeapObject = (__HEAP_OBJECT*)lpKernelThread->lpHeapObject;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	
	//printf("Start to print out the heap information.\r\n");
	lpHeapTmp = lpHeapObject;
	while(lpHeapTmp)
	{
		//printf("Begin a heap.......\r\n");
		lpVirtualArea = lpHeapTmp->lpVirtualArea;
		while(lpVirtualArea)
		{
			//printf("Start address is : %X\r\n",lpVirtualArea->lpStartAddress);
			lpVirtualArea = lpVirtualArea->lpNext;
		}
		lpHeapTmp = lpHeapTmp->lpNext;
		if(lpHeapObject == lpHeapTmp)  //The same heap object.
			break;
	}
}

//
//The following routine is a help routine,used to dumpout the free block list.
//
VOID DumpFreeList(__HEAP_OBJECT* lpHeapObj)
{
	__FREE_BLOCK_HEADER	*        lpFreeHeader = NULL;

	if(NULL == lpHeapObj)  //Invalid parameter
		return;
	//printf("\r\nBegin to dump out the free list:\r\n");
	lpFreeHeader = lpHeapObj->FreeBlockHeader.lpNext;
	while(lpFreeHeader != &lpHeapObj->FreeBlockHeader)
	{
		//printf("StartAddr: 0x%8X Size: %d\r\n",
		//	(DWORD)lpFreeHeader + sizeof(__FREE_BLOCK_HEADER),
		//	lpFreeHeader->dwBlockSize);
		lpFreeHeader = lpFreeHeader->lpNext;
	}
}

//
//The implementation of HeapAlloc routine.
//This routine does the following actions:
// 1. Check if the heap object given by user belong to current kernel thread,
//    that is,only the thread that heap object belong to can allocate memory
//    from it;
// 2. Check the free list of the heap,try to find a free block can statisfy
//    user's request;
// 3. If can find the block,then split the block in case of the block size is
//    big,and return one to user,or return the whole block to user in case of
//    the block is not too big;
// 4. If can not find the block,then allocate a virtual area according to user's
//    request,and split the virtual area,one part returned to user,insert another
//    part into free list;
// 5. If any failure,returns NULL to indicate failed.
//
static LPVOID HeapAlloc(__HEAP_OBJECT* lpHeapObject,DWORD dwSize)
{
	__VIRTUAL_AREA_NODE*             lpVirtualArea   = NULL;
	__FREE_BLOCK_HEADER*             lpFreeBlock     = NULL;
	__FREE_BLOCK_HEADER*             lpTmpHeader     = NULL;
	LPVOID                           lpResult        = NULL;
	DWORD                            dwFlags         = 0;
	DWORD                            dwFindSize      = 0;

	if((NULL == lpHeapObject) || (0 == dwSize)) //Parameter check.
		return lpResult;
	
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpHeapObject->lpKernelThread != CURRENT_KERNEL_THREAD) //Check the heap's owner.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return lpResult;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(dwSize < MIN_BLOCK_SIZE)
		dwSize = MIN_BLOCK_SIZE;
	dwFindSize = dwSize + MIN_BLOCK_SIZE + sizeof(__FREE_BLOCK_HEADER);

	//
	//Now,check the free list,try to find a free block.
	//
	lpFreeBlock = lpHeapObject->FreeBlockHeader.lpNext;
	while(lpFreeBlock != &lpHeapObject->FreeBlockHeader)
	{
		if(lpFreeBlock->dwBlockSize >= dwSize)  //Find one.
		{
			if(lpFreeBlock->dwBlockSize >= dwFindSize)  //Should split it into two free blocks.
			{
				lpTmpHeader = (__FREE_BLOCK_HEADER*)((DWORD)lpFreeBlock + dwSize
					+ sizeof(__FREE_BLOCK_HEADER));     //Pointing to second part.
				lpTmpHeader->dwFlags     = BLOCK_FLAGS_FREE;
				lpTmpHeader->dwBlockSize = lpFreeBlock->dwBlockSize - dwSize
					- sizeof(__FREE_BLOCK_HEADER);      //Calculate second part's size.
				//
				//Now,should replace the lpFreeBlock with lpTmpHeader.
				//
				lpTmpHeader->lpNext = lpFreeBlock->lpNext;
				lpTmpHeader->lpPrev = lpFreeBlock->lpPrev;
				lpTmpHeader->lpNext->lpPrev = lpTmpHeader;
				lpTmpHeader->lpPrev->lpNext = lpTmpHeader;

				lpFreeBlock->dwBlockSize = dwSize;
				lpFreeBlock->dwFlags     |= BLOCK_FLAGS_USED;
				lpFreeBlock->dwFlags     &= ~BLOCK_FLAGS_FREE;  //Clear the free flags.
				lpFreeBlock->lpPrev      = NULL;
				lpFreeBlock->lpNext      = NULL;
				lpResult                 = (LPVOID)((DWORD)lpFreeBlock
					+ sizeof(__FREE_BLOCK_HEADER));
				goto __TERMINAL;
			}
			else  //Now need to split,return the block is OK.
			{
				//
				//Delete the free block from free block list.
				//
				lpFreeBlock->lpNext->lpPrev = lpFreeBlock->lpPrev;
				lpFreeBlock->lpPrev->lpNext = lpFreeBlock->lpNext;
				lpFreeBlock->dwFlags        |= BLOCK_FLAGS_USED;
				lpFreeBlock->dwFlags        &= ~BLOCK_FLAGS_FREE;  //Clear free bit.
				lpFreeBlock->lpNext         = NULL;
				lpFreeBlock->lpPrev         = NULL;
				lpResult = (LPVOID)((DWORD)lpFreeBlock + sizeof(__FREE_BLOCK_HEADER));
				goto __TERMINAL;
			}
		}
		lpFreeBlock = lpFreeBlock->lpNext;  //Check the next block.
	}
	if(lpResult)    //Have found a block.
		goto __TERMINAL;
	//
	//If can not find a statisfying block in above process,we should allocate a
	//new virtual area according to dwSize,insert the virtual area into free list,
	//then repeat above process.
	//
	lpVirtualArea = (__VIRTUAL_AREA_NODE*)GET_KERNEL_MEMORY(sizeof(__VIRTUAL_AREA_NODE));
	if(NULL == lpVirtualArea)  //Can not allocate kernel memory.
		goto __TERMINAL;
	lpVirtualArea->dwAreaSize     = ((dwSize + sizeof(__FREE_BLOCK_HEADER))
		> DEFAULT_VIRTUAL_AREA_SIZE) ?
		(dwSize + sizeof(__FREE_BLOCK_HEADER)) : DEFAULT_VIRTUAL_AREA_SIZE;

	lpVirtualArea->lpStartAddress = GET_VIRTUAL_AREA(lpVirtualArea->dwAreaSize);
	if(NULL == lpVirtualArea->lpStartAddress)  //Can not get virtual area.
	{
		RELEASE_KERNEL_MEMORY((LPVOID)lpVirtualArea);
		goto __TERMINAL;
	}
	//
	//Insert the virtual area node object into virtual area list
	//of current heap object.
	//
	lpVirtualArea->lpNext       = lpHeapObject->lpVirtualArea;
	lpHeapObject->lpVirtualArea = lpVirtualArea;
	//
	//Now,insert the free block(virtual area) into free list of the
	//heap object.
	//
	lpTmpHeader = (__FREE_BLOCK_HEADER*)lpVirtualArea->lpStartAddress;
	lpTmpHeader->dwFlags |= BLOCK_FLAGS_FREE;
	lpTmpHeader->dwFlags &= ~BLOCK_FLAGS_USED;  //Clear the used flags.
	lpTmpHeader->dwBlockSize = lpVirtualArea->dwAreaSize - sizeof(__FREE_BLOCK_HEADER);

	lpTmpHeader->lpNext  = lpHeapObject->FreeBlockHeader.lpNext;
	lpTmpHeader->lpPrev  = &lpHeapObject->FreeBlockHeader;
	lpTmpHeader->lpNext->lpPrev = lpTmpHeader;
	lpTmpHeader->lpPrev->lpNext = lpTmpHeader;
	//
	//Now,call HeapAlloc again,this time must successful.
	//
	lpResult = HeapAlloc(lpHeapObject,dwSize);

__TERMINAL:
	return lpResult;
}

//
//The following routine is a helper routine,used to combine several small
//free blocks into one big free block.
//The combine unit is a virtual area,so,a problem may exists:if two virtual
//areas,one's end address is the same as the other's start address,so in normal
//case,these two virtual area should be combined into one big block.But in 
//current implementation,this function is implemented by Hello China,in fact,
//this function have very seldom influence the efficiency.
//This routine does the following actions:
// 1. From the first block,check if there are at least two blocks free and
//    consecutive;
// 2. If so,combine the two blocks into one block,and remove one from free
//    list;
// 3. Until reach the end position of the virtual area.
//
static VOID CombineBlock(__VIRTUAL_AREA_NODE* lpVirtualArea,__HEAP_OBJECT* lpHeapObj)
{
	__FREE_BLOCK_HEADER*           lpFirstBlock     = NULL;
	__FREE_BLOCK_HEADER*           lpSecondBlock    = NULL;
	LPVOID                         lpEndAddr        = NULL;

	if((NULL == lpVirtualArea) || (NULL == lpHeapObj)) //Invalid parameters.
		return;

	lpEndAddr     = (LPVOID)((DWORD)lpVirtualArea->lpStartAddress
		+ lpVirtualArea->dwAreaSize);

	lpFirstBlock  = (__FREE_BLOCK_HEADER*)lpVirtualArea->lpStartAddress;
	lpSecondBlock = (__FREE_BLOCK_HEADER*)((DWORD)lpFirstBlock
		+ sizeof(__FREE_BLOCK_HEADER)
		+ lpFirstBlock->dwBlockSize);  //Now,lpSecondBlock pointing to the second block.

	while(TRUE)
	{
		if(lpEndAddr == (LPVOID)lpSecondBlock) //Reach the end of the virtual area.
			break;
		if((lpFirstBlock->dwFlags & BLOCK_FLAGS_FREE) &&
		   (lpSecondBlock->dwFlags & BLOCK_FLAGS_FREE))  //Two blocks all free,combine it.
		{
			lpFirstBlock->dwBlockSize += lpSecondBlock->dwBlockSize;
			lpFirstBlock->dwBlockSize += sizeof(__FREE_BLOCK_HEADER);
			//
			//Delete the second block from free list.
			//
			lpSecondBlock->lpNext->lpPrev = lpSecondBlock->lpPrev;
			lpSecondBlock->lpPrev->lpNext = lpSecondBlock->lpNext;

			lpSecondBlock = (__FREE_BLOCK_HEADER*)((DWORD)lpFirstBlock
				+ sizeof(__FREE_BLOCK_HEADER)
				+ lpFirstBlock->dwBlockSize);  //Update the second block.
			continue;   //Continue to next round.
		}
		if((lpFirstBlock->dwFlags & BLOCK_FLAGS_USED) ||
		   (lpSecondBlock->dwFlags & BLOCK_FLAGS_USED)) //Any block is used.
		{
			lpFirstBlock  = lpSecondBlock;
			lpSecondBlock = (__FREE_BLOCK_HEADER*)((DWORD)lpFirstBlock
				+ sizeof(__FREE_BLOCK_HEADER)
				+ lpFirstBlock->dwBlockSize);
			continue;
		}	
	}
}

//
//The implementation of HeapFree routine.
//This routine does the following:
// 1. Insert the memory block freed by user into heap's free list;
// 2. Find which virtual area this address belong to;
// 3. Combine the virtual area by calling CombineBlock routine;
// 4. Check if the virtual area to be combined is ONE free block,
//    If so,then release the virtual area and virtual area node
//    object.
//
static VOID HeapFree(LPVOID lpStartAddr,__HEAP_OBJECT* lpHeapObj)
{
	__FREE_BLOCK_HEADER*    lpFreeHeader  = NULL;
	__VIRTUAL_AREA_NODE*    lpVirtualArea = NULL;
	__VIRTUAL_AREA_NODE*    lpVirtualTmp  = NULL;

	if((NULL == lpStartAddr) || (NULL == lpHeapObj)) //Invalid parameter.
		return;

	lpFreeHeader = (__FREE_BLOCK_HEADER*)((DWORD)lpStartAddr
		- sizeof(__FREE_BLOCK_HEADER));  //Get the block's header.
	if(!(lpFreeHeader->dwFlags & BLOCK_FLAGS_USED)) //Abnormal case.
		return;

	//
	//Now,check the block to be freed belong to which virtual area.
	//
	lpVirtualArea = lpHeapObj->lpVirtualArea;
	while(lpVirtualArea)
	{
		if(((DWORD)lpStartAddr > (DWORD)lpVirtualArea->lpStartAddress) &&
		   ((DWORD)lpStartAddr < (DWORD)lpVirtualArea->lpStartAddress
		   + lpVirtualArea->dwAreaSize))  //Belong to this virtual area.
		{
			break;
		}
		lpVirtualArea = lpVirtualArea->lpNext;
	}
	if(NULL == lpVirtualArea)  //Can not find a virtual area that countains
		                       //the free block to be released.
	{
		//printf("\r\nFree a invalid block: can not find virtual area.");
	    return;
	}

	//
	//Now,should insert the free block into heap object's free list.
	//
	lpFreeHeader->dwFlags |= BLOCK_FLAGS_FREE;
	lpFreeHeader->dwFlags &= ~BLOCK_FLAGS_USED;  //Clear the used flags.
	lpFreeHeader->lpPrev   = &lpHeapObj->FreeBlockHeader;
	lpFreeHeader->lpNext   = lpHeapObj->FreeBlockHeader.lpNext;
	lpFreeHeader->lpPrev->lpNext = lpFreeHeader;
	lpFreeHeader->lpNext->lpPrev = lpFreeHeader;

	CombineBlock(lpVirtualArea,lpHeapObj);      //Combine this virtual area.
	//
	//Now,should check if the whole virtual area is a free block.
	//If so,delete the free block from free list,then release the 
	//virtual area to system.
	//
	lpFreeHeader = (__FREE_BLOCK_HEADER*)(lpVirtualArea->lpStartAddress);
	if((lpFreeHeader->dwFlags & BLOCK_FLAGS_FREE) &&
	   (lpFreeHeader->dwBlockSize + sizeof(__FREE_BLOCK_HEADER)
	   == lpVirtualArea->dwAreaSize))
	{
		//
		//Delete the free block from free list.
		//
		lpFreeHeader->lpPrev->lpNext = lpFreeHeader->lpNext;
		lpFreeHeader->lpNext->lpPrev = lpFreeHeader->lpPrev;
		//
		//Now,should delete the virtual area node object from
		//heap object's virtual list.
		//
		lpVirtualTmp = lpHeapObj->lpVirtualArea;
		if(lpVirtualTmp == lpVirtualArea)  //The first virtual node.
		{
			lpHeapObj->lpVirtualArea = lpVirtualArea->lpNext;
		}
		else    //Not the first one.
		{
			while(lpVirtualTmp->lpNext != lpVirtualArea)
			{
				lpVirtualTmp = lpVirtualTmp->lpNext;
			}
			lpVirtualTmp->lpNext = lpVirtualArea->lpNext;  //Delete it.
		}
		//
		//Then,should release the virtual area and virtual area node 
		//object.
		//
		RELEASE_VIRTUAL_AREA((LPVOID)lpVirtualArea->lpStartAddress);
		RELEASE_KERNEL_MEMORY((LPVOID)lpVirtualArea);
	}

	return;
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/
//
//The declaration of HeapManager object.
//There is only one HeapManager object in whole system.
//

__HEAP_MANAGER HeapManager = {
	CreateHeap,                   //CreateHeap routine.
	DestroyHeap,                  //DestroyHeap routine.
	DestroyAllHeap,               //DestroyAllHeap routine.
	HeapAlloc,                    //HeapAlloc routine.
	HeapFree                      //HeapFree routine.
};

#endif

/*
//
//The following are malloc routine's implementation.
//This routine create a default heap,if not exists yet,then allocate
//memory from this default heap.
//
LPVOID malloc(DWORD dwSize)
{
#ifdef __CFG_SYS_VMM

	DWORD            dwFlags    = 0;
	LPVOID           lpResult   = NULL;
	__HEAP_OBJECT*   lpHeapObj  = NULL;
	DWORD            dwHeapSize = 0;

	if(0 == dwSize)  //Invalid parameter.
		return lpResult;

	if(NULL == CURRENT_KERNEL_THREAD->lpDefaultHeap)  //Should create one.
	{
		dwHeapSize = (dwSize + sizeof(__FREE_BLOCK_HEADER) > DEFAULT_VIRTUAL_AREA_SIZE) ?
			(dwSize + sizeof(__FREE_BLOCK_HEADER)) : DEFAULT_VIRTUAL_AREA_SIZE;
		lpHeapObj = CreateHeap(dwHeapSize);
		if(NULL == lpHeapObj)  //Can not create heap object.
			return lpResult;
		__ENTER_CRITICAL_SECTION(NULL,dwFlags);
		CURRENT_KERNEL_THREAD->lpDefaultHeap = (LPVOID)lpHeapObj;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	}
	else
	{
		lpHeapObj = (__HEAP_OBJECT*)CURRENT_KERNEL_THREAD->lpDefaultHeap;
	}
	return HeapAlloc(lpHeapObj,dwSize);
#else
	return KMemAlloc(dwSize,KMEM_SIZE_TYPE_ANY);
#endif
}

//
//The implementation of free routine.
//This routine frees a memory block,to the default heap object of
//current kernel thread.
//
VOID free(LPVOID lpMemory)
{
#ifdef __CFG_SYS_VMM

	__HEAP_OBJECT*      lpHeapObj = NULL;
	
	if(NULL == lpMemory)
		return;

	lpHeapObj = (__HEAP_OBJECT*)CURRENT_KERNEL_THREAD->lpDefaultHeap;
	if(NULL == lpHeapObj)    //Invalid case.
		return;
	HeapFree(lpMemory,lpHeapObj);
#else
	KMemFree(lpMemory,KMEM_SIZE_TYPE_ANY,0);
#endif
}
*/

