//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,25 2004
//    Module Name               : mem_fbl.cpp
//    Module Funciton           : 
//                                This module countains the implementation code
//                                of free block list algorithm.
//    Last modified Author      : Garry
//    Last modified Date        : Jun 14,2013
//    Last modified Content     :
//                                1. Convert this module as a separated memory
//                                   management implementation file,other MM
//                                   modules may be added to system as this one;
//                                2. Changed file name to mem_fbl.cpp from buffmgr.cpp.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "types.h"
#include <buffmgr.h>

//Only __CFG_SYS_MMFBL switch is defined the following code is available.
#ifdef __CFG_SYS_MMFBL

//The following value will be asigned to memory block's header to identify the block
//is allocated legally.The purpose is to implement a safe memory management mechanism.
//The following identifier will be verified when free routine is called to make sure
//it's security.
#define MEM_BLOCK_IDENTIFIER1 0xAA55AA55
#define MEM_BLOCK_IDENTIFIER2 0x55AA55AA

//The following routine create a buffer pool by calling KMemAlloc.
static BOOL CreateBuffer1(__BUFFER_CONTROL_BLOCK* pControlBlock,DWORD dwPoolSize)
{
	return FALSE;
}

//
//The following routine create a buffer pool by using client created memory,and initialize
//the buffer control block.
//
static BOOL CreateBuffer2(__BUFFER_CONTROL_BLOCK* pControlBlock,
						  LPVOID lpBufferPool,
						  DWORD dwPoolSize)
{
	BOOL                     bResult      = FALSE;
	DWORD                    dwSize       = 0;
	__FREE_BUFFER_HEADER*    pFreeHdr     = NULL;

	dwSize = dwPoolSize;

	pControlBlock->dwPoolSize         = dwSize;  //Initialize the buffer control block.
	pControlBlock->dwFlags           |= CREATED_BY_CLIENT;
	pControlBlock->dwFlags           |= POOL_INITIALIZED;  //Set the pool initialized bit.
	pControlBlock->lpPoolStartAddress = lpBufferPool;

	pFreeHdr = (__FREE_BUFFER_HEADER*)lpBufferPool;
	pControlBlock->lpFreeBufferHeader = pFreeHdr;
	pFreeHdr->dwFlags = BUFFER_STATUS_FREE;
	pFreeHdr->lpNextBlock = NULL;
	pFreeHdr->lpPrevBlock = NULL; /*pControlBlock->lpFreeBufferHeader;*/
	pFreeHdr->dwBlockSize = dwSize - sizeof(__FREE_BUFFER_HEADER);
	pControlBlock->dwFreeSize   = pFreeHdr->dwBlockSize;
	pControlBlock->dwFreeBlocks = 1;

	bResult = TRUE;
	return bResult;
}


//
//Allocate buffer routine,this routine allocates a buffer from buffer pool,according
//the size of the client request,and returns the start address of the buffer.
//
static LPVOID Allocate(__BUFFER_CONTROL_BLOCK* pControlBlock,DWORD dwSize)
{
	LPVOID                   lpBuffer         = NULL;
	__FREE_BUFFER_HEADER*    lpFreeHeader     = NULL;
	__FREE_BUFFER_HEADER*    lpTmpHeader      = NULL;
	__USED_BUFFER_HEADER*    lpUsedHeader     = NULL;
	DWORD                    dwBoundrySize    = 0;
	DWORD                    dwFlags          = 0;

	if((NULL == pControlBlock) || (0 == dwSize))  //Parameters check.
	{
		goto __TERMINAL;
	}

	if(dwSize < MIN_BUFFER_SIZE)
	{
		dwSize = MIN_BUFFER_SIZE;
	}

	//If a free block's size is less this value and larger than dwSize,then allocate the whole
	//block to application.Otherwise will seperate the free block into 2 blocks and return one
	//to application.
	dwBoundrySize = dwSize + MIN_BUFFER_SIZE + sizeof(__FREE_BUFFER_HEADER);

	//The following operation must not be interrupted since it can run in any context.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	lpFreeHeader = pControlBlock->lpFreeBufferHeader;
	while(lpFreeHeader)
	{
		if(lpFreeHeader->dwBlockSize < dwSize)   //The current block is not fitable.
		{
			lpFreeHeader = lpFreeHeader->lpNextBlock;
			continue;
		}

		if(lpFreeHeader->dwBlockSize <= dwBoundrySize)  //Allocate the whole free block.
		{
			if(NULL == lpFreeHeader->lpPrevBlock)  //This block is the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					pControlBlock->lpFreeBufferHeader = NULL;
				}
				else    //This is not the last free block.
				{
					lpFreeHeader->lpNextBlock->lpPrevBlock = NULL;
					pControlBlock->lpFreeBufferHeader = lpFreeHeader->lpNextBlock;
				}
			}
			else        //This block is not the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = NULL;
				}
				else    //This is not the last free block.
				{
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpFreeHeader->lpPrevBlock;
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpFreeHeader->lpNextBlock;
				}
			}
			lpUsedHeader = (__USED_BUFFER_HEADER*)lpFreeHeader;
			lpUsedHeader->dwFlags &= ~BUFFER_STATUS_FREE;  //Clear the free bit.
			lpUsedHeader->dwFlags |= BUFFER_STATUS_USED;   //Set the used bit.
			                                               //The used block's size
			                                               //is the same as when the
			                                               //block is free.
			//Set memory block identifier.
			lpUsedHeader->dwReserved1 = MEM_BLOCK_IDENTIFIER1;
			lpUsedHeader->dwReserved2 = MEM_BLOCK_IDENTIFIER2;
            lpBuffer = (LPVOID)((UCHAR*)lpUsedHeader + sizeof(__USED_BUFFER_HEADER));
			pControlBlock->dwFreeSize   -= lpUsedHeader->dwBlockSize; //Update the free memory size.
			pControlBlock->dwFreeBlocks -= 1;  //Minus one free block.
			break;
		}
		else      //Can not allocate the whole free block,because it's size is enough to
			      //separate into two blocks,one block is allocated to client,and another
				  //is reserved as free.
		{
			//The lpTmpHeader will be the header pointer of new separated free block.
            lpTmpHeader = (__FREE_BUFFER_HEADER*)((DWORD)lpFreeHeader + sizeof(__FREE_BUFFER_HEADER) + dwSize);
			lpTmpHeader->lpPrevBlock = lpFreeHeader->lpPrevBlock;
			lpTmpHeader->lpNextBlock = lpFreeHeader->lpNextBlock;
			lpTmpHeader->dwBlockSize = lpFreeHeader->dwBlockSize - sizeof(__FREE_BUFFER_HEADER)- dwSize;
			lpTmpHeader->dwFlags = 0;
			lpTmpHeader->dwFlags |= BUFFER_STATUS_FREE;  //Set the free bit.

			//Now,the following code update the neighbor free blocks's member.
			if(NULL == lpFreeHeader->lpPrevBlock) //This block is the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					pControlBlock->lpFreeBufferHeader = lpTmpHeader;
				}
				else //This is not the last free block.
				{
					pControlBlock->lpFreeBufferHeader = lpTmpHeader;
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpTmpHeader;
				}
			}
			else    //This is not the first free block.
			{
				if(NULL == lpFreeHeader->lpNextBlock)  //This is the last free block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpTmpHeader;
				}
				else  //This is not the first,and also not the last free block.
				{
					lpFreeHeader->lpPrevBlock->lpNextBlock = lpTmpHeader;
					lpFreeHeader->lpNextBlock->lpPrevBlock = lpTmpHeader;
				}
			}

			lpUsedHeader = (__USED_BUFFER_HEADER*)lpFreeHeader;
			lpUsedHeader->dwFlags &= ~BUFFER_STATUS_FREE;  //Clear the free bit.
			lpUsedHeader->dwFlags |= BUFFER_STATUS_USED;   //Set the used bit.
			lpUsedHeader->dwBlockSize = dwSize;
			//Set memory block identifier.
			lpUsedHeader->dwReserved1 = MEM_BLOCK_IDENTIFIER1;
			lpUsedHeader->dwReserved2 = MEM_BLOCK_IDENTIFIER2;
			pControlBlock->dwFreeSize -= dwSize; //Update free memory size,in 20130613.
			pControlBlock->dwFreeSize -= sizeof(__FREE_BUFFER_HEADER); //Free header also included.

            lpBuffer = (LPVOID)((UCHAR*)lpUsedHeader + sizeof(__USED_BUFFER_HEADER));
			break;
		}
	}

	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	//Update successful allocation times number.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pControlBlock->dwAllocTimesL += 1;
	if(0 == pControlBlock->dwAllocTimesL)  //Overflow.
	{
		pControlBlock->dwAllocTimesH += 1;
	}
	if(lpBuffer)  //Successful,update success counter.
	{
		pControlBlock->dwAllocTimesSuccL += 1;
		if(0 == pControlBlock->dwAllocTimesSuccL)  //Overflow.
		{
			pControlBlock->dwAllocTimesSuccH += 1;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//For debugging.
	if(pControlBlock->dwAllocTimesSuccL > pControlBlock->dwAllocTimesL)
	{
		BUG();
	}
	return lpBuffer;
}

//A local helper routine to combine it's neighbor blocks given a free block.
static void CombineNeighbor(__BUFFER_CONTROL_BLOCK* pControlBlock,__FREE_BUFFER_HEADER* pFreeBlock)
{
	BOOL    bCombinePrev    = FALSE;
	BOOL    bCombineNext    = FALSE;
	__FREE_BUFFER_HEADER*   pCurrent = pFreeBlock;

	//Check if should combine current and previous one.
	if(NULL != pCurrent->lpPrevBlock)
	{
		if(pCurrent->lpPrevBlock->dwBlockSize + sizeof(__FREE_BUFFER_HEADER) + (DWORD)(pCurrent->lpPrevBlock) == (DWORD)pCurrent)
		{
			bCombinePrev = TRUE;
		}
	}
	//Checi if should combine current and next one.
	if(NULL != pCurrent->lpNextBlock)
	{
		if(pCurrent->dwBlockSize + sizeof(__FREE_BUFFER_HEADER) + (DWORD)pCurrent == (DWORD)(pCurrent->lpNextBlock))
		{
			bCombineNext = TRUE;
		}
	}
	if(bCombinePrev)  //Combine current one and previous block.
	{
		pCurrent->lpPrevBlock->lpNextBlock = pCurrent->lpNextBlock;
		if(pCurrent->lpNextBlock)
		{
			pCurrent->lpNextBlock->lpPrevBlock = pCurrent->lpPrevBlock;
		}
		pCurrent->lpPrevBlock->dwBlockSize += (pCurrent->dwBlockSize + sizeof(__FREE_BUFFER_HEADER));
		pControlBlock->dwFreeBlocks -= 1;
		pControlBlock->dwFreeSize += sizeof(__FREE_BUFFER_HEADER);
		//Update current block as the new combined one.
		pCurrent = pCurrent->lpPrevBlock;
	}
	if(bCombineNext)  //Combine current one and next block.
	{
		pCurrent->dwBlockSize += (pCurrent->lpNextBlock->dwBlockSize + sizeof(__FREE_BUFFER_HEADER));
		pControlBlock->dwFreeSize += sizeof(__FREE_BUFFER_HEADER);
		//Delete the next block from free block list.
		pCurrent->lpNextBlock = pCurrent->lpNextBlock->lpNextBlock;
		if(NULL != pCurrent->lpNextBlock)
		{
			pCurrent->lpNextBlock->lpPrevBlock = pCurrent;
		}
		pControlBlock->dwFreeBlocks -= 1;
	}
	return;
}

//A newly added function to put one free memory block into free list,and combine
//neighbor blocks if necessary.
static void ReleaseAndCombine(__BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer)
{
	__FREE_BUFFER_HEADER*     pFreeHeader     = NULL;
	__FREE_BUFFER_HEADER*     pInsertBehind   = NULL;  //Free block will be inserted into list after this block.
	__FREE_BUFFER_HEADER*     pInsertNext     = NULL;
	DWORD                     dwFlags;

	if((NULL == pControlBlock) || (NULL == lpBuffer))
	{
		return;
	}

	pFreeHeader = (__FREE_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(__FREE_BUFFER_HEADER));
	//Set flags properly.
	pFreeHeader->dwFlags &= ~BUFFER_STATUS_USED;
	pFreeHeader->dwFlags |= BUFFER_STATUS_FREE;

	//To find the proper position to insert the block into free block list.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == pControlBlock->lpFreeBufferHeader)  //Free block list is empty.
	{
		pControlBlock->lpFreeBufferHeader = pFreeHeader;
		pFreeHeader->lpNextBlock = NULL;
		pFreeHeader->lpPrevBlock = NULL;
		pControlBlock->dwFreeSize   += pFreeHeader->dwBlockSize;  //Update free memory size.
		pControlBlock->dwFreeBlocks += 1;  //Update free block number.
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	//Not the first free block,try to locate the position to insert.
	pInsertBehind = pControlBlock->lpFreeBufferHeader;
	pInsertNext   = pControlBlock->lpFreeBufferHeader;
	while(pInsertNext)
	{
		if((DWORD)pInsertNext > (DWORD)pFreeHeader)  //Found the position.
		{
			break;
		}
		pInsertBehind = pInsertNext;
		pInsertNext   = pInsertNext->lpNextBlock;
	}
	if(NULL == pInsertNext)  //Shoud put to the last position.
	{
		pInsertBehind->lpNextBlock = pFreeHeader;
		pFreeHeader->lpPrevBlock   = pInsertBehind;
		pFreeHeader->lpNextBlock   = NULL;
		pControlBlock->dwFreeSize += pFreeHeader->dwBlockSize;
		pControlBlock->dwFreeBlocks += 1;
		//Call combine routine try to combine the neighbor block.
		CombineNeighbor(pControlBlock,pFreeHeader);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	if(pInsertNext == pControlBlock->lpFreeBufferHeader)  //Should put into list as header.
	{
		pFreeHeader->lpNextBlock = pInsertNext;
		pFreeHeader->lpPrevBlock = NULL;
		pInsertNext->lpPrevBlock = pFreeHeader;
		pControlBlock->lpFreeBufferHeader = pFreeHeader;
		pControlBlock->dwFreeSize += pFreeHeader->dwBlockSize;
		pControlBlock->dwFreeBlocks += 1;
		//Combine neighbor blocks.
		CombineNeighbor(pControlBlock,pFreeHeader);
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return;
	}
	//Shoud put into the middle of free block list.
	pFreeHeader->lpNextBlock   = pInsertBehind->lpNextBlock;
	pFreeHeader->lpPrevBlock   = pInsertBehind;
	pFreeHeader->lpNextBlock->lpPrevBlock = pFreeHeader;
	pFreeHeader->lpPrevBlock->lpNextBlock = pFreeHeader;
	pControlBlock->dwFreeSize += pFreeHeader->dwBlockSize;
	pControlBlock->dwFreeBlocks += 1;
	//Combine neighbor blocks.
	CombineNeighbor(pControlBlock,pFreeHeader);
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

//
//Free buffer routine,this routine returns the buffer to buffer pool.
//
static VOID Free(__BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer)
{
	__USED_BUFFER_HEADER*       lpUsedHeader  = NULL;

	//Parameters check.
	if((NULL == pControlBlock) || (NULL == lpBuffer))
	{
		return;
	}

	//Verify the memory block.
	lpUsedHeader = (__USED_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(__USED_BUFFER_HEADER));
	if(!(lpUsedHeader->dwFlags & BUFFER_STATUS_USED))  //Flags is not correct.
	{
		return;
	}
	if((lpUsedHeader->dwReserved1 != MEM_BLOCK_IDENTIFIER1) || (lpUsedHeader->dwReserved2 != MEM_BLOCK_IDENTIFIER2))
	{
		return;
	}

	ReleaseAndCombine(pControlBlock,lpBuffer);
	//Update free routine's calling number.
	pControlBlock->dwFreeTimesL += 1;
	if(0 == pControlBlock->dwFreeTimesL)  //Overflow.
	{
		pControlBlock->dwFreeTimesH += 1;
	}
}


//
//Append memory buffer to buffer controller.
// @lpBuffer   : Start address of the free memory buffer to append;
// @dwBuffSize : Memory buffer's size.
//This routine act almost same as Free operation,it treat the memory buffer as a free block,
//and append it to free list.The only difference from Free is that the block size is give directly
//through parameter,not derive from memory block header.
static VOID AppendBuffer(__BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer,DWORD dwBuffSize)
{
	__FREE_BUFFER_HEADER*       lpFreeHeader  = NULL;
	
	if(!(pControlBlock->dwFlags & OPERATIONS_INITIALIZED))  //Buffer control block is not initialized yet.
	{
		return;
	}
	if((NULL == pControlBlock) || (NULL == lpBuffer))  //Invalid parameters.
	{
		return;
	}
	if(dwBuffSize < (sizeof(__FREE_BUFFER_HEADER) + MIN_BUFFER_SIZE)) //Buffer is too small.
	{
		return;
	}

	//Initialize the free buffer as a free memory block.
	lpFreeHeader = (__FREE_BUFFER_HEADER*)lpBuffer;
	lpFreeHeader->dwBlockSize = dwBuffSize - sizeof(__FREE_BUFFER_HEADER);
	lpFreeHeader->dwFlags &= ~BUFFER_STATUS_USED;
	lpFreeHeader->dwFlags |= BUFFER_STATUS_FREE;
	lpFreeHeader->lpNextBlock = lpFreeHeader->lpPrevBlock = NULL;
	
	//Insert the free block into free block list as the free operation.
	ReleaseAndCombine(pControlBlock,(LPVOID)((DWORD)lpBuffer + sizeof(__FREE_BUFFER_HEADER)));

	//Update total memory pool size.
	pControlBlock->dwPoolSize += dwBuffSize;
}

//
//Buffer flags operating routine,these routine set or get the buffer's flags.
//
static DWORD GetBufferFlag(__BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer)
{
	DWORD                  dwFlag       = 0;
	__USED_BUFFER_HEADER*  lpUsedHeader = NULL;

	if(NULL == lpBuffer)  //Parameter check.
	{
		return dwFlag;
	}

	lpUsedHeader = (__USED_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(__USED_BUFFER_HEADER));
	dwFlag = lpUsedHeader->dwFlags;
	return dwFlag;
}

//
//NOTICE : The following routine reset the used buffer's flags,overwrite the previous flags.
//
static BOOL SetBufferFlag(__BUFFER_CONTROL_BLOCK* pControlBlock,LPVOID lpBuffer,DWORD dwFlag)
{
	BOOL                   bResult      = FALSE;
	__USED_BUFFER_HEADER*  lpUsedHeader = NULL;

	if(NULL == lpBuffer)
	{
		return bResult;
	}

	lpUsedHeader = (__USED_BUFFER_HEADER*)((DWORD)lpBuffer - sizeof(__USED_BUFFER_HEADER));
	lpUsedHeader->dwFlags = dwFlag;
	bResult = TRUE;
	return bResult;
}

//
//Destroy buffer pool routine.
//
static VOID DestroyBuffer(__BUFFER_CONTROL_BLOCK* pControlBlock)
{
	return;
}

//
//The following routine get the buffer control block's flags.
//
static DWORD GetControlBlockFlag(__BUFFER_CONTROL_BLOCK* pControlBlock)
{
	return pControlBlock->dwFlags;
}

//
//The global routine used to initialize a buffer manager.
//This routine initializes buffer control block's members.
//
static BOOL InitBufferMgr(__BUFFER_CONTROL_BLOCK* pControlBlock)
{
	BOOL                  bResult = FALSE;

	if(NULL == pControlBlock)
	{
		goto __TERMINAL;
	}

	//Initialize buffer management variables.
	pControlBlock->GetControlBlockFlag = GetControlBlockFlag;
	pControlBlock->lpFreeBufferHeader  = NULL;
	pControlBlock->lpPoolStartAddress  = NULL;
	pControlBlock->dwFlags             = 0;
	pControlBlock->dwFlags            |= OPERATIONS_INITIALIZED;
	pControlBlock->dwFreeSize          = 0;
	pControlBlock->dwPoolSize          = 0;
	pControlBlock->dwFreeBlocks        = 0;
	pControlBlock->dwAllocTimesH       = 0;
	pControlBlock->dwAllocTimesL       = 0;
	pControlBlock->dwAllocTimesSuccH   = 0;
	pControlBlock->dwAllocTimesSuccL   = 0;
	pControlBlock->dwFreeTimesH        = 0;
	pControlBlock->dwFreeTimesL        = 0;
	pControlBlock->lpBuffExtension     = NULL;

	bResult = TRUE;  //Indicate the successful of initialization.

__TERMINAL:
	return bResult;
}

//Initialization oepration of buffer manager.
//It's initializes a given buffer manager and creates free memory regions according
//SystemMemRegion array residing in mem_scat.c file.
//So the global array SystemMemRegion should be decared first.

#ifdef __GCC__
__MEMORY_REGION SystemMemRegion[] = {
	//{Start address of memory region,memory region's length}
	{(LPVOID)KMEM_ANYSIZE_START_ADDRESS,0x00100000},  //1M memory,start from KMEM_ANYSIZE_START_ADDRESS
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00100000),0x00100000},
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00200000),0x00100000},
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00300000),0x00100000},
	//Please add more memory regions here.
	//The last entry must be NULL and zero,to indicate the end of this array.
	{NULL,0}
};
#endif
extern __MEMORY_REGION SystemMemRegion[];

static BOOL Initialize(__BUFFER_CONTROL_BLOCK* pControlBlock)
{
	int i = 1;

	if(NULL == pControlBlock)
	{
		return FALSE;
	}
	if(!pControlBlock->InitializeBuffer(pControlBlock))
	{
		return FALSE;
	}
	if(NULL == SystemMemRegion[0].lpBuffer)  //At least one memory region in system.
	{
		return FALSE;
	}
	if(!pControlBlock->CreateBuffer2(pControlBlock,
		SystemMemRegion[0].lpBuffer,SystemMemRegion[0].dwBufferSize))
	{
		return FALSE;
	}
	while(SystemMemRegion[i].lpBuffer)  //Should append as free memory buffer.
	{
		AppendBuffer(pControlBlock,SystemMemRegion[i].lpBuffer,  //CAUTION! AppendBuffer has bug.
			SystemMemRegion[i].dwBufferSize);
		i ++;
	}
	return TRUE;
}

//Any size memory allocation buffers,this object is mainly used by KMemAlloc routine,
//free memory block list algorithm is used in this object.
__BUFFER_CONTROL_BLOCK AnySizeBuffer = {
	OPERATIONS_INITIALIZED,       //dwFlags;
	NULL,                         //lpPoolStartAddress;

	0,                            //dwPoolSize;
	0,                            //dwFreeSize;
	0,                            //dwFreeBlocks;
	0,                            //dwAllocTimesH;
	0,                            //dwAllocTimesL;
	0,                            //dwAllocTimesSuccH;
	0,                            //dwAllocTimesSuccL;
	0,                            //dwFreeTimesH;
	0,                            //dwFreeTimesL;

	NULL,                         //lpBuffExtension;  //Extensions for special customization.This feild has same
	                                                  //functionality as device extension in device object.
	GetControlBlockFlag,          //(*GetControlBlockFlag)(__BUFFER_CONTROL_BLOCK*);
	NULL,                         //lpFreeBufferHeader;

	//Initializer of buffer object.
	Initialize,                   //Initialization operation.
	InitBufferMgr,                //(InitializeBuffer)(__BUFFER_CONTROL_BLOCK* pControlBlock);

	//Buffer operations.
	CreateBuffer1,                //CreateBuffer1;
	CreateBuffer2,                //CreateBuffer2;
	AppendBuffer,                 //AppendBuffer;
	Allocate,                     //Allocate;
	Free,                         //Free;
	GetBufferFlag,                //GetBufferFlag;
	SetBufferFlag,                //SetBufferFlag;
	DestroyBuffer                 //DestroyBuffer;
};

#endif  //__CFG_SYS_MMFBL
