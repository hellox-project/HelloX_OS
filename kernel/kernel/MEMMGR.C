//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar,09 2005
//    Module Name               : memmgr.cpp
//    Module Funciton           : 
//                                This module countains Memory Manager's implementation
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <mlayout.h>
#include <pmdesc.h>

/* 
 * Page frame manager functions is available 
 * only when VMM mechanism is enabled. 
 */
#ifdef __CFG_SYS_VMM

/* 
 * This array is used to locate the page frame block's
 * size according to it's index. 
 */
static DWORD FrameBlockSize[] = {
	PAGE_FRAME_BLOCK_SIZE_4K,
	PAGE_FRAME_BLOCK_SIZE_8K,

	PAGE_FRAME_BLOCK_SIZE_16K,
	PAGE_FRAME_BLOCK_SIZE_32K,
	PAGE_FRAME_BLOCK_SIZE_64K,

	PAGE_FRAME_BLOCK_SIZE_128K,
	PAGE_FRAME_BLOCK_SIZE_256K,
	PAGE_FRAME_BLOCK_SIZE_512K,

	PAGE_FRAME_BLOCK_SIZE_1024K,
	PAGE_FRAME_BLOCK_SIZE_2048K,
	PAGE_FRAME_BLOCK_SIZE_4096K,
	PAGE_FRAME_BLOCK_SIZE_8192K
};

/* Align a address value to page frame boundary. */
#define ALIGN_DOWN_TO_PAGE(address) \
    (LPVOID)(((DWORD)(address) % PAGE_FRAME_SIZE) ? ((DWORD)(address) + \
    (PAGE_FRAME_SIZE - (DWORD)(address) % PAGE_FRAME_SIZE)) : (DWORD)(address))

#define ALIGN_UP_TO_PAGE(address) \
    (LPVOID)(((DWORD)(address) % PAGE_FRAME_SIZE) ? ((DWORD)(address) - \
	(DWORD)(address) % PAGE_FRAME_SIZE) : (DWORD)(address))

/* Set one bit in bit map,the bit's position is determined by dwBitPos. */
static VOID SetBitmapBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*    lpdwMap    = NULL;
	DWORD     dwPos      = 0;
	DWORD     dwBitZero  = 0x00000001;

	if (NULL == lpdwBitmap)
	{
		return;
	}

	lpdwMap  = lpdwBitmap;
	lpdwMap += dwBitPos / (sizeof(DWORD)*8);  //Now,lpdwMap pointes to the
	                                          //DWORD the target bit exists.
	dwPos = dwBitPos % (sizeof(DWORD)*8);
	dwBitZero <<= dwPos;

	*lpdwMap |= dwBitZero;  //Set the target bit.
}

/* Clear one bit in bit map,the bit's position is determined by dwBitPos. */
static VOID ClearBitmapBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*    lpdwMap    = NULL;
	DWORD     dwPos      = 0;
	DWORD     dwBitZero  = 0x00000001;

	if (NULL == lpdwBitmap)
	{
		return;
	}

	lpdwMap  = lpdwBitmap;
	lpdwMap += dwBitPos / (sizeof(DWORD)*8);  //Now,lpdwMap pointes to the
	                                          //DWORD the target bit exists.
	dwPos = dwBitPos % (sizeof(DWORD)*8);
	dwBitZero <<= dwPos;

	*lpdwMap &= ~dwBitZero;  //Set the target bit.
}

/*
 * The routine tests the bit whose position is indicated by dwBitPos,if the bit is one,then
 * returns TRUE,else,returns FALSE.
 */
static BOOL TestBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*                 lpdwMap     = NULL;
	DWORD                  dwBitZero   = 0x00000001;
	DWORD                  dwPos       = 0;

	if (NULL == lpdwBitmap)
	{
		return FALSE;
	}

	lpdwMap = lpdwBitmap;
	dwPos   = dwBitPos;

	lpdwMap += dwPos / (sizeof(DWORD) * 8);  //Now,lpdwMap pointes to the double word
	                                         //that the bit to be tested exists.
	dwPos      %= (sizeof(DWORD) * 8);
	dwBitZero <<= dwPos;

	if (*lpdwMap & dwBitZero)  //Bit is one.
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Initialization routine of memory manager object.
 * The routine does the following:
 *  1. Calculate total page frame numbers,and initializes the members of page frame manager,
 *  2. Allocate a block of kernel memory,as page frame array,and initializes it;
 *  3. Allocate a block of kernel memory,as page frame block bit map;
 *  4. Split the physical memory into page frame blocks,and inserts them into page frame
 *     block list;
 *  5. If all above steps successfully,then return TRUE,else,return FALSE.
 */

/*
 * The lpStartAddr is the start address of paged memory space,
 * in HelloX it's the start address of RAM exclude kernel dedicate
 * occupied range,which is 0x1400000.
 * The lpEndAddr is the end address of paged memory space,
 * It's the end of RAM in most case.
 * If the lpStartAddr is 0x00100000,and the memory's 
 * size is 4M,then,the lpEndAddr must be 0x004FFFFF,
 * not 0x00500000.
 * If the lpStartAddr and the lpEndAddr's value are 
 * not aligned with page frame(4K) boundary,then the
 * routine will round them to page frame boundary.
*/
static BOOL PageFrameMgrInit(__COMMON_OBJECT* lpThis,
	LPVOID lpStartAddr,
	LPVOID lpEndAddr)
{
	__PAGE_FRAME_MANAGER* lpFrameManager = NULL;
	BOOL bResult = FALSE;
	LPVOID lpStartAddress = NULL;
	LPVOID lpEndAddress = NULL;
	DWORD dwPageNum = 0;
	__PAGE_FRAME* lpPageFrameArray = NULL;
	unsigned int i = 0, j = 0, k = 0;
	DWORD dwTotalMemLen = 0;
	DWORD dwIndBase = 0;

	BUG_ON(NULL == lpThis);

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;
	lpStartAddress = ALIGN_DOWN_TO_PAGE(lpStartAddr);
	lpEndAddress   = (LPVOID)((DWORD)lpEndAddr + 1);    //Adjust the end address.
	lpEndAddress   = ALIGN_UP_TO_PAGE(lpEndAddress);
	
	if ((DWORD)((DWORD)lpEndAddress - (DWORD)lpStartAddress) < PAGE_FRAME_SIZE)
	{
		goto __TERMINAL;
	}

	dwPageNum = ((DWORD)lpEndAddress - (DWORD)lpStartAddress) / PAGE_FRAME_SIZE;
	lpPageFrameArray = (__PAGE_FRAME*)KMemAlloc(dwPageNum * sizeof(__PAGE_FRAME),
		KMEM_SIZE_TYPE_ANY);
	if (NULL == lpPageFrameArray)
	{
		goto __TERMINAL;
	}

	//
	//Initializes the members of page frame manager.
	//
	lpFrameManager->lpStartAddress   = lpStartAddress;
	lpFrameManager->dwTotalFrameNum  = dwPageNum;
	lpFrameManager->dwFreeFrameNum   = dwPageNum;
	lpFrameManager->lpPageFrameArray = lpPageFrameArray;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpFrameManager->spin_lock, "frmmgr");
#endif

	/* Initialize all bitmap to NULL. */
	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		lpFrameManager->FrameBlockArray[i].lpdwBitmap  = NULL;
		lpFrameManager->FrameBlockArray[i].lpNextBlock = NULL;
		lpFrameManager->FrameBlockArray[i].lpPrevBlock = NULL;
	}

	/* Initializes the bitmap of page frame block. */
	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		lpFrameManager->FrameBlockArray[i].lpdwBitmap = (DWORD*)KMemAlloc((dwPageNum / sizeof(DWORD)
			+ 1)*sizeof(DWORD),KMEM_SIZE_TYPE_ANY);
		if (NULL == lpFrameManager->FrameBlockArray[i].lpdwBitmap)
		{
			goto __TERMINAL;
		}
		for (j = 0; j < (dwPageNum / sizeof(DWORD) + 1) * sizeof(DWORD); j++)
		{
			((UCHAR*)lpFrameManager->FrameBlockArray[i].lpdwBitmap)[j] = 0;
		}

		if (0 == dwPageNum)
		{
			break;
		}

		dwPageNum /= 2;
	}

	//
	//The following code splits the whole physical memory into page frame blocks,and insert
	//them into page frame block list of Page Frame Manager.
	//
	dwTotalMemLen = lpFrameManager->dwTotalFrameNum * PAGE_FRAME_SIZE;
	dwIndBase     = 0;
	for(i = PAGE_FRAME_BLOCK_NUM;i > 0;i --)
	{
		j = dwTotalMemLen / FrameBlockSize[i - 1];
		k = FrameBlockSize[i - 1] / PAGE_FRAME_SIZE;
		while(j)  //Insert the block into list.
		{
			j --;
			if(lpFrameManager->FrameBlockArray[i - 1].lpNextBlock)
			{
				lpFrameManager->FrameBlockArray[i - 1].lpNextBlock->lpPrevFrame = 
					&(lpFrameManager->lpPageFrameArray[dwIndBase + j * k]);
			}
			lpFrameManager->lpPageFrameArray[dwIndBase + j * k].lpNextFrame = 
				lpFrameManager->FrameBlockArray[i - 1].lpNextBlock;
			lpFrameManager->lpPageFrameArray[dwIndBase + j * k].lpPrevFrame = NULL;

			lpFrameManager->FrameBlockArray[i - 1].lpNextBlock = 
				&(lpFrameManager->lpPageFrameArray[dwIndBase + j * k]);

			/* Set the appropriate bitmap bit,to indicate the block exists. */
			SetBitmapBit(lpFrameManager->FrameBlockArray[i - 1].lpdwBitmap,
				dwIndBase / k + j);
		}
		j = dwTotalMemLen / FrameBlockSize[i - 1];
		dwIndBase += j * k;
		dwTotalMemLen -= j * FrameBlockSize[i - 1];

	}

	bResult = TRUE;    //Set the successful initialization flag.

__TERMINAL:
	if(!bResult)    //Failed to initialize the object.
	{
		if(lpFrameManager->lpPageFrameArray)
			KMemFree((LPVOID)lpFrameManager->lpPageFrameArray,KMEM_SIZE_TYPE_ANY,0);
		for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
		{
			if(lpFrameManager->FrameBlockArray[i].lpdwBitmap)
				KMemFree((void*)lpFrameManager->FrameBlockArray[i].lpdwBitmap,
				KMEM_SIZE_TYPE_ANY,0);
		}
	}
	return bResult;
}

/*
 * Initialization routine of Page Frame Manager object.
 * It analyzes the physical memory layout array constructed
 * in process of OS loading(by OS loader),and use the
 * first feasible RAM space to initialize the Page Frame
 * Manager object.
 * Currently we can not support the hardware system that many
 * discontinues RAM are installed,and only the first one
 * will be managed by this object.
 * We may solve this issue in the future.
 */
static BOOL __Initialize(__COMMON_OBJECT* pThis)
{
	BOOL bResult = FALSE;
	__SYSTEM_MLAYOUT_DESCRIPTOR* pLayout = NULL;
	uint32_t ramStart = 0;
	uint32_t ramLength = 0;
	uint32_t offset = 0;
	int i = MAX_MLAYOUT_ARRAY_SIZE;

	BUG_ON(NULL == pThis);
	pLayout = (__SYSTEM_MLAYOUT_DESCRIPTOR*)SYS_MLAYOUT_ADDR;

	/* Analyze the element in layout array one by one. */
	while (i)
	{
		if ((0 == pLayout->length_hi) && (0 == pLayout->length_lo))
		{
			/* Reach the end of memory layout array. */
			break;
		}
		i--;
		if ((pLayout->length_hi) || (pLayout->start_addr_hi))
		{
			/* Can not support 64 bits yet. */
			pLayout++;
			continue;
		}
		if (PHYSICAL_MEM_TYPE_RAM != pLayout->mem_type)
		{
			/* Not RAM memory. */
			pLayout++;
			continue;
		}
		if ((pLayout->start_addr_lo + pLayout->length_lo) < KMEM_KERNEL_EXCLUDE_END)
		{
			/* Covered by OS reserved range,ignore. */
			pLayout++;
			continue;
		}
		if (pLayout->start_addr_lo < KMEM_KERNEL_EXCLUDE_END)
		{
			/* 
			 * Overlap with kernel reserved range,deduct the 
			 * overlaped range.
			 */
			ramStart = KMEM_KERNEL_EXCLUDE_END;
			ramLength = KMEM_KERNEL_EXCLUDE_END - pLayout->start_addr_lo;
			ramLength = pLayout->length_lo - ramLength;
		}
		else
		{
			/* Independed RAM ranged. */
			ramStart = pLayout->start_addr_lo;
			ramLength = pLayout->length_lo;
		}
		if (ramLength < (16 * PAGE_FRAME_SIZE))
		{
			/* Too small block,ignore. */
			pLayout++;
			continue;
		}
		/* Make the start address is page frame aligned. */
		if (ramStart & (PAGE_FRAME_SIZE - 1))
		{
			offset = ramStart & (PAGE_FRAME_SIZE - 1);
			offset = PAGE_FRAME_SIZE - offset;
			ramLength -= offset;
			ramStart &= ~(PAGE_FRAME_SIZE - 1);
			ramStart += PAGE_FRAME_SIZE;
		}
		/* The lengh also should be page frame aligned. */
		if (ramLength & (PAGE_FRAME_SIZE - 1))
		{
			ramLength &= ~(PAGE_FRAME_SIZE - 1);
		}
		/* 
		 * Page frame manager can not manage too much 
		 * RAM currently,cap the exceed range.
		 */
		if (ramLength > MEM_MAX_PAGEPOOL_LEN)
		{
			ramLength = MEM_MAX_PAGEPOOL_LEN;
		}
		/* 
		 * Now use this RAM range to initialize the 
		 * Page Frame Manager object.
		 */
		bResult = PageFrameMgrInit(pThis,
			(LPVOID)ramStart,
			(LPVOID)(ramStart + ramLength - 1));
		if (bResult)
		{
			/* Only support 1 RAM range currently. */
			break;
		}
	}

	return bResult;
}

//
//FrameAlloc routine of PageFrameManager.
//The routine does the following:
// 1. Try to find a page frame block statisfy the request;
// 2. If can not find this kind of block,return NULL;
// 3. If can find this kind of block,then split the block into more small block(if
//    need),and insert the more small block into appropriate block list;
// 4. Update the appropriate bit of bitmap;
// 5. Return the result.
//
//CAUTION: The dwSize parameter must equal to the corresponding parameter of
//FrameFree routine of Page Frame Manager.
//
static LPVOID __FrameAlloc(__COMMON_OBJECT* lpThis,
	DWORD dwSize,
	DWORD dwPageFrameFlag)
{
	__PAGE_FRAME_MANAGER* lpFrameManager = NULL;
	BOOL bFind = TRUE;
	LPVOID lpResult = NULL;
	unsigned long i = 0, j = 0, k = 0;
	unsigned long dwOffset = 0, page_nums = 0;
	__PAGE_FRAME* lpPageFrame = NULL;
	__PAGE_FRAME* lpTempFrame = NULL;
	unsigned long dwFlags = 0;

	BUG_ON((NULL == lpThis) || (0 == dwSize));

	if (dwSize > FrameBlockSize[PAGE_FRAME_BLOCK_NUM - 1])
	{
		/* Requested block is too large. */
		goto __TERMINAL;
	}

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;

	/* Find which block list satisfy the request. */
	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		if(dwSize <= FrameBlockSize[i])
			break;
	}

	/* 
	 * How many page(s) will be deducted from the 
	 * free pages pool if success. 
	 */
	page_nums = FrameBlockSize[i] / PAGE_FRAME_SIZE;

	__ENTER_CRITICAL_SECTION_SMP(lpFrameManager->spin_lock, dwFlags);
	j = i;
	while(j < PAGE_FRAME_BLOCK_NUM)
	{
		if(NULL == lpFrameManager->FrameBlockArray[j].lpNextBlock)
		{
			j ++;
			continue;
		}
		else
		{
			/* Found the fitable block list to allocate from. */
			break;
		}
	}

	if(PAGE_FRAME_BLOCK_NUM == j)
	{
		/* No page frame block can fit the request. */
		__LEAVE_CRITICAL_SECTION_SMP(lpFrameManager->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	//
	//Now,we have found the block list countains the feasible block,so we 
	//delete the first block from the block list,spit it into more small
	//blocks,insert the less block into appropriate block list,and return
	//to the caller.
	//
	lpPageFrame = lpFrameManager->FrameBlockArray[j].lpNextBlock;
	if (NULL != lpPageFrame->lpNextFrame)
	{
		lpPageFrame->lpPrevFrame = NULL;
	}
	/* Delete the block from list. */
	lpFrameManager->FrameBlockArray[j].lpNextBlock = lpPageFrame->lpNextFrame;

	dwOffset = (DWORD)lpPageFrame - (DWORD)lpFrameManager->lpPageFrameArray;
	k = dwOffset / sizeof(__PAGE_FRAME);

	/* Get the return address and clear the corresponding bits. */
	lpResult = (LPVOID)((DWORD)lpFrameManager->lpStartAddress + k * PAGE_FRAME_SIZE);
	k /= (FrameBlockSize[j] / PAGE_FRAME_SIZE);
	ClearBitmapBit(lpFrameManager->FrameBlockArray[j].lpdwBitmap,k);

	/* Update free pages number. */
	lpFrameManager->dwFreeFrameNum -= page_nums;

	while(j > i)
	{
		/* Split the block into more small block,and insert into block list. */
		k  = FrameBlockSize[j - 1] / PAGE_FRAME_SIZE;
		k *= sizeof(__PAGE_FRAME);
		lpTempFrame = (__PAGE_FRAME*)((DWORD)lpPageFrame + k);
		if(NULL != lpFrameManager->FrameBlockArray[j - 1].lpNextBlock)
		{
			lpFrameManager->FrameBlockArray[j - 1].lpNextBlock->lpPrevFrame = 
				lpTempFrame;
		}
		lpTempFrame->lpNextFrame = lpFrameManager->FrameBlockArray[j - 1].lpNextBlock;
		lpTempFrame->lpPrevFrame = NULL;
		lpFrameManager->FrameBlockArray[j - 1].lpNextBlock = lpTempFrame;
		dwOffset = (DWORD)lpTempFrame - (DWORD)lpFrameManager->lpPageFrameArray;
		k = dwOffset / sizeof(__PAGE_FRAME);
		k /= FrameBlockSize[j - 1] / PAGE_FRAME_SIZE;
		SetBitmapBit(lpFrameManager->FrameBlockArray[j - 1].lpdwBitmap,k);  //Set bit.
		j --;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpFrameManager->spin_lock, dwFlags);

__TERMINAL:
	return lpResult;
}

//
//FrameFree routine of PageFrameManager.
//The routine does the following:
// 1. Check the buddy block's status of the block which will be freed;
// 2. If the status of buddy block is free,then combine the two blocks,and 
//    insert the combined block into up level block list;
// 3. Check the up level block list,this is a recursive process.
//
//CAUTION: The dwSize parameter of this routine,must equal to the dwSize
//parameter of FrameAlloc routine of Page Frame Manager.
//
static VOID __FrameFree(__COMMON_OBJECT*  lpThis,
	LPVOID lpStartAddr,
	DWORD dwSize)
{
	__PAGE_FRAME_MANAGER* lpFrameManager = NULL;
	__PAGE_FRAME* lpPageFrame = NULL;
	__PAGE_FRAME* lpTempFrame = NULL;
	unsigned long i = 0, k = 0, dwOffset = 0;
	unsigned long page_nums = 0;
	unsigned long dwFlags = 0;

	BUG_ON((NULL == lpThis) || (NULL == lpStartAddr) || (0 == dwSize));

	if((DWORD)lpStartAddr % PAGE_FRAME_SIZE)
	{
		/* The address is not page frame aligned. */
		goto __TERMINAL;
	}

	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		if(dwSize <= FrameBlockSize[i])
			break;
	}

	if (PAGE_FRAME_BLOCK_NUM == i)
	{
		/* The dwSize is too large. */
		goto __TERMINAL;
	}

	/* How many pages will be freed. */
	page_nums = FrameBlockSize[i] / PAGE_FRAME_SIZE;

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;
	dwOffset = (DWORD)lpStartAddr - (DWORD)lpFrameManager->lpStartAddress;
	k = dwOffset / FrameBlockSize[i];  //The k-th block.

	lpPageFrame = (__PAGE_FRAME*)((DWORD)lpFrameManager->lpPageFrameArray +
		(dwOffset / PAGE_FRAME_SIZE) * sizeof(__PAGE_FRAME));
	//Now,lpPageFrame pointes the page frame control block.

	if(k % 2)    //Get this block's buddy block index.
	{
		k -= 1;
	}
	else
	{
		k += 1;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpFrameManager->spin_lock, dwFlags);
	/* 
	 * If the current block's buddy block also free,
	 * then combine the two blocks,and insert into up level
	 * block list. 
	 */
	if(TestBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k))
	{
		while(i < PAGE_FRAME_BLOCK_NUM - 1)
		{
			if(k % 2) /* The behind block of lpPageFrame is it's buddy block. */
			{
				lpTempFrame = (__PAGE_FRAME*)((DWORD)lpPageFrame + (FrameBlockSize[i] / 
					PAGE_FRAME_SIZE) * sizeof(__PAGE_FRAME));
			}
			else /* The previous block of lpPageFrame is it's buddy block. */
			{
				lpTempFrame = (__PAGE_FRAME*)((DWORD)lpPageFrame - (FrameBlockSize[i] /
					PAGE_FRAME_SIZE) * sizeof(__PAGE_FRAME));
				lpPageFrame = lpTempFrame;
			}
			/*
			 * Deletes the buddy block from the current list,and insert
			 * the combined block into up level block list.
			 */
			if(NULL == lpTempFrame->lpPrevFrame)
			{
				if(NULL == lpTempFrame->lpNextFrame)
				{
					lpFrameManager->FrameBlockArray[i].lpNextBlock = lpTempFrame->lpNextFrame;
				}
				else
				{
					lpTempFrame->lpNextFrame->lpPrevFrame = NULL;
					lpFrameManager->FrameBlockArray[i].lpNextBlock = lpTempFrame->lpNextFrame;
				}
			}
			else  /* This is not the first block. */
			{
				if(NULL == lpTempFrame->lpNextFrame)
				{
					lpTempFrame->lpPrevFrame->lpNextFrame = NULL;
				}
				else
				{
					lpTempFrame->lpPrevFrame->lpNextFrame = lpTempFrame->lpNextFrame;
					lpTempFrame->lpNextFrame->lpPrevFrame = lpTempFrame->lpPrevFrame;
				}
			}

			/* Clear corresponding bits. */
			ClearBitmapBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k);
			/* check up level block link. */
			i ++;

			k = (DWORD)lpPageFrame - (DWORD)lpFrameManager->lpPageFrameArray;
			k /= sizeof(__PAGE_FRAME);
			k /= (FrameBlockSize[i] / PAGE_FRAME_SIZE);

			if(k % 2)
				k -= 1;
			else
				k += 1;

			if(!TestBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k))
				break;
		}

		/* Inserts the combined block into up level block list. */
		if(NULL != lpFrameManager->FrameBlockArray[i].lpNextBlock)
		{
			lpFrameManager->FrameBlockArray[i].lpNextBlock->lpPrevFrame = 
				lpPageFrame;
		}
		lpPageFrame->lpNextFrame = lpFrameManager->FrameBlockArray[i].lpNextBlock;
		lpPageFrame->lpPrevFrame = NULL;

		lpFrameManager->FrameBlockArray[i].lpNextBlock = lpPageFrame;

		if(k % 2)    //Adjust k.
			k -= 1;
		else
			k += 1;

		SetBitmapBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k); //Set bit.
	}
	else
	{
		/* 
		 * The buddy of the current block is occupied.
		 * So,only insert the current block into the block list.
		 */
		if(NULL != lpFrameManager->FrameBlockArray[i].lpNextBlock)
		{
			lpFrameManager->FrameBlockArray[i].lpNextBlock->lpPrevFrame = 
				lpPageFrame;
		}
		lpPageFrame->lpNextFrame = lpFrameManager->FrameBlockArray[i].lpNextBlock;
		lpPageFrame->lpPrevFrame = NULL;

		lpFrameManager->FrameBlockArray[i].lpNextBlock = lpPageFrame;

		if(k % 2)    //Adjust k.
			k -= 1;
		else
			k += 1;

		SetBitmapBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k);  //Set bit.
	}
	/* Update the total free page's number. */
	lpFrameManager->dwFreeFrameNum += page_nums;
	__LEAVE_CRITICAL_SECTION_SMP(lpFrameManager->spin_lock, dwFlags);

__TERMINAL:
	return;
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/
/*
 * PageFrameManager object,the global object to manage all physical memory 
 * frames.
 */
__PAGE_FRAME_MANAGER PageFrameManager = {
	NULL,                                  //lpPageFrameArray.
	{0},                                   //FrameBlockArray.
	0,                                     //dwTotalFrameNum.
	0,                                     //dwFreeFrameNum.
	NULL,                                  //lpStartAddress.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,                  //spin_lock.
#endif
	__Initialize,                            //Initialize routine.
	__FrameAlloc,                            //FrameAlloc routine.
	__FrameFree                              //FrameFree routine.
};

#endif
