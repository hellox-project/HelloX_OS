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

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif


//Page Frame functions only available when VMM function is enabled.
#ifdef __CFG_SYS_VMM
#include "types.h"
#include "commobj.h"
#include "kapi.h"
#include "memmgr.h"
//------------------------------------------------------------------------
//
//    The implementation code of Page Frame Manager.
//
//------------------------------------------------------------------------
static DWORD FrameBlockSize[] = {    //This array is used to locate the page frame block's
	                                 //size according to it's index.
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

//
//Align one address to page frame boundary.
//

#define ALIGN_DOWN_TO_PAGE(address) \
    (LPVOID)(((DWORD)(address) % PAGE_FRAME_SIZE) ? ((DWORD)(address) + \
    (PAGE_FRAME_SIZE - (DWORD)(address) % PAGE_FRAME_SIZE)) : (DWORD)(address))

#define ALIGN_UP_TO_PAGE(address) \
    (LPVOID)(((DWORD)(address) % PAGE_FRAME_SIZE) ? ((DWORD)(address) - \
	(DWORD)(address) % PAGE_FRAME_SIZE) : (DWORD)(address))

//
//The following are some helper routines,used by the member functions of PageFrameManager.
//

//
//Set one bit in bit map,the bit's position is determined by dwBitPos.
//

static VOID SetBitmapBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*    lpdwMap    = NULL;
	DWORD     dwPos      = 0;
	DWORD     dwBitZero  = 0x00000001;

	if(NULL == lpdwBitmap)    //Parameter check.
		return;

	lpdwMap  = lpdwBitmap;
	lpdwMap += dwBitPos / (sizeof(DWORD)*8);  //Now,lpdwMap pointes to the
	                                          //DWORD the target bit exists.
	dwPos = dwBitPos % (sizeof(DWORD)*8);
	dwBitZero <<= dwPos;

	*lpdwMap |= dwBitZero;  //Set the target bit.
}

//
//Clear one bit in bit map,the bit's position is determined by dwBitPos.
//

static VOID ClearBitmapBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*    lpdwMap    = NULL;
	DWORD     dwPos      = 0;
	DWORD     dwBitZero  = 0x00000001;

	if(NULL == lpdwBitmap)    //Parameter check.
		return;

	lpdwMap  = lpdwBitmap;
	lpdwMap += dwBitPos / (sizeof(DWORD)*8);  //Now,lpdwMap pointes to the
	                                          //DWORD the target bit exists.
	dwPos = dwBitPos % (sizeof(DWORD)*8);
	dwBitZero <<= dwPos;

	*lpdwMap &= ~dwBitZero;  //Set the target bit.
}

//
//TestBit routine.
//The routine tests the bit whose position is indicated by dwBitPos,if the bit is one,then
//returns TRUE,else,returns FALSE.
//

static BOOL TestBit(DWORD* lpdwBitmap,DWORD dwBitPos)
{
	DWORD*                 lpdwMap     = NULL;
	DWORD                  dwBitZero   = 0x00000001;
	DWORD                  dwPos       = 0;

	if(NULL == lpdwBitmap)   //Parameter check.
		return FALSE;

	lpdwMap = lpdwBitmap;
	dwPos   = dwBitPos;

	lpdwMap += dwPos / (sizeof(DWORD) * 8);  //Now,lpdwMap pointes to the double word
	                                         //that the bit to be tested exists.
	dwPos      %= (sizeof(DWORD) * 8);
	dwBitZero <<= dwPos;

	if(*lpdwMap & dwBitZero)  //Bit is one.
		return TRUE;
	
	return FALSE;
}

//
//Initialize routine.
//The routine does the following:
// 1. Calculate total page frame numbers,and initializes the members of page frame manager,
// 2. Allocate a block of kernel memory,as page frame array,and initializes it;
// 3. Allocate a block of kernel memory,as page frame block bit map;
// 4. Split the physical memory into page frame blocks,and inserts them into page frame
//    block list;
// 5. If all above steps successfully,then return TRUE,else,return FALSE.
//

//
//The lpStartAddr is the start address of linear memory space,and the lpEndAddr is the
//end address of linear address space.
//For example:
//
//    -------------
//    |           |   <----- lpStartAddr
//    |           |
//    |           |
//       ... ...   
//    |           |
//    |           |   <----- lpEndAddr
//    -------------
//
// If the lpStartAddr is 0x00100000,and the memory's size is 4M,then,the lpEndAddr must be
// 0x004FFFFF,not 0x00500000.
//
// If the lpStartAddr and the lpEndAddr's value does not align with 4K boundary,then the
// routine will round them to 4k boundary.
//

static BOOL PageFrameMgrInit(__COMMON_OBJECT*  lpThis,
							 LPVOID            lpStartAddr,
							 LPVOID            lpEndAddr)
{
	__PAGE_FRAME_MANAGER*        lpFrameManager      = NULL;
	BOOL                         bResult             = FALSE;
	LPVOID                       lpStartAddress      = NULL;
	LPVOID                       lpEndAddress        = NULL;
	DWORD                        dwPageNum           = 0;
	__PAGE_FRAME*                lpPageFrameArray    = NULL;
	DWORD                        i                   = 0;
	DWORD                        j                   = 0;
	DWORD                        k                   = 0;
	DWORD                        dwTotalMemLen       = 0;
	DWORD                        dwIndBase           = 0;

	if(NULL == lpThis)    //Parameters check.
		goto __TERMINAL;

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;
	lpStartAddress = ALIGN_DOWN_TO_PAGE(lpStartAddr);
	lpEndAddress   = (LPVOID)((DWORD)lpEndAddr + 1);    //Adjust the end address.
	lpEndAddress   = ALIGN_UP_TO_PAGE(lpEndAddress);
	
	if((DWORD)((DWORD)lpEndAddress - (DWORD)lpStartAddress) < PAGE_FRAME_SIZE)
		goto __TERMINAL;

	dwPageNum = ((DWORD)lpEndAddress - (DWORD)lpStartAddress) / PAGE_FRAME_SIZE;
	lpPageFrameArray = (__PAGE_FRAME*)KMemAlloc(dwPageNum * sizeof(__PAGE_FRAME),
		KMEM_SIZE_TYPE_ANY);
	if(NULL == lpPageFrameArray)    //Failed to allocate memory.
		goto __TERMINAL;

	//
	//Initializes the members of page frame manager.
	//

	lpFrameManager->lpStartAddress   = lpStartAddress;
	lpFrameManager->dwTotalFrameNum  = dwPageNum;
	lpFrameManager->dwFreeFrameNum   = dwPageNum;
	lpFrameManager->lpPageFrameArray = lpPageFrameArray;

	//--------------- ** debug ** --------------------------
	//printf("Initialize: Total Page Frame number : %d\r\n",dwPageNum);
	//printf("Initialize: Start Address at        : %d\r\n",(DWORD)lpStartAddress);
	//printf("Initialize: Page Frame Array base   : %d\r\n",(DWORD)lpPageFrameArray);

	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)    //Initialize all bitmap to NULL.
	{
		lpFrameManager->FrameBlockArray[i].lpdwBitmap  = NULL;
		lpFrameManager->FrameBlockArray[i].lpNextBlock = NULL;
		lpFrameManager->FrameBlockArray[i].lpPrevBlock = NULL;
	}

	//
	//The following code initializes the bitmap of page frame block.
	//

	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		//dwPageNum /= sizeof(DWORD);
		lpFrameManager->FrameBlockArray[i].lpdwBitmap = (DWORD*)KMemAlloc((dwPageNum / sizeof(DWORD)
			+ 1)*sizeof(DWORD),KMEM_SIZE_TYPE_ANY);
		if(NULL == lpFrameManager->FrameBlockArray[i].lpdwBitmap)  //Failed to allocate memory.
			goto __TERMINAL;
		for(j = 0;j < (dwPageNum/sizeof(DWORD) + 1)*sizeof(DWORD);j ++)  //Clear to zero.
			((UCHAR*)lpFrameManager->FrameBlockArray[i].lpdwBitmap)[j] = 0;

		if(0 == dwPageNum)
			break;

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
			lpFrameManager->lpPageFrameArray[dwIndBase + j * k].lpPrevFrame = 
				NULL;

			lpFrameManager->FrameBlockArray[i - 1].lpNextBlock = 
				&(lpFrameManager->lpPageFrameArray[dwIndBase + j * k]);

			//
			//Set the appropriate bitmap bit,to indicate the block exists.
			//
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

static LPVOID FrameAlloc(__COMMON_OBJECT* lpThis,
						 DWORD            dwSize,
						 DWORD            dwPageFrameFlag)
{
	__PAGE_FRAME_MANAGER*        lpFrameManager  = NULL;
	//DWORD                        dwBlockIndex    = 0;
	BOOL                         bFind           = TRUE;
	LPVOID                       lpResult        = NULL;
	DWORD                        i               = 0;
	DWORD                        j               = 0;
	DWORD                        k               = 0;
	DWORD                        dwOffset        = 0;
	__PAGE_FRAME*                lpPageFrame     = NULL;
	__PAGE_FRAME*                lpTempFrame     = NULL;
	DWORD                        dwFlags         = 0;

	if((NULL == lpThis) || (0 == dwSize))//Parameter check
		goto __TERMINAL;

	if(dwSize > FrameBlockSize[PAGE_FRAME_BLOCK_NUM - 1]) //Request a too large block.
		goto __TERMINAL;

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;

	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		if(dwSize <= FrameBlockSize[i])    //Find which block list satisfy the request.
			break;
	}

	//ENTER_CRITICAL_SECTION(); //The following operation is a atomic operation.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	j = i;
	while(j < PAGE_FRAME_BLOCK_NUM)
	{
		if(NULL == lpFrameManager->FrameBlockArray[j].lpNextBlock)
		{
			j ++;
			continue;
		}
		else
			break;    //Find the fitable block list to allocate from.
	}

	if(PAGE_FRAME_BLOCK_NUM == j)  //There is not page frame block to fit the request.
	{
		//LEAVE_CRITICAL_SECTION();
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	//
	//Now,we have found the block list countains the correct block,the block can fit the
	//request,so we delete the first block from the block list,spit it into more small
	//blocks,insert the less block into appropriate block list,and return one correct to
	//the caller.
	//
	lpPageFrame = lpFrameManager->FrameBlockArray[j].lpNextBlock;
	if(NULL != lpPageFrame->lpNextFrame)
		lpPageFrame->lpPrevFrame = NULL;
	lpFrameManager->FrameBlockArray[j].lpNextBlock = lpPageFrame->lpNextFrame; //Delete the block.

	dwOffset = (DWORD)lpPageFrame - (DWORD)lpFrameManager->lpPageFrameArray;
	k        = dwOffset / sizeof(__PAGE_FRAME);

	//
	//The return value can be calculated as following:
	//

	lpResult = (LPVOID)((DWORD)lpFrameManager->lpStartAddress + k * PAGE_FRAME_SIZE);
	k       /= (FrameBlockSize[j] / PAGE_FRAME_SIZE);
	ClearBitmapBit(lpFrameManager->FrameBlockArray[j].lpdwBitmap,k);  //Clear the bit.

	while(j > i)    //Split the block into more small block,and insert into block list.
	{
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
		k        = dwOffset / sizeof(__PAGE_FRAME);
		k       /= FrameBlockSize[j - 1] / PAGE_FRAME_SIZE;
		SetBitmapBit(lpFrameManager->FrameBlockArray[j - 1].lpdwBitmap,k);  //Set bit.
		j --;
	}

	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

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

static VOID FrameFree(__COMMON_OBJECT*  lpThis,
					  LPVOID            lpStartAddr,
					  DWORD             dwSize)
{
	__PAGE_FRAME_MANAGER*               lpFrameManager  = NULL;
	__PAGE_FRAME*                       lpPageFrame     = NULL;
	__PAGE_FRAME*                       lpTempFrame     = NULL;
	DWORD                               i               = 0;
	//DWORD                               j               = 0;
	DWORD                               k               = 0;
	DWORD                               dwOffset        = 0;
	DWORD                               dwFlags         = 0;

	if((NULL == lpThis) || (NULL == lpStartAddr) ||
	   (0 == dwSize))   //Parameter check.
	{
		goto __TERMINAL;
	}

	if((DWORD)lpStartAddr % PAGE_FRAME_SIZE)   //The address is not page frame alignable.
	{
		goto __TERMINAL;
	}

	for(i = 0;i < PAGE_FRAME_BLOCK_NUM;i ++)
	{
		if(dwSize <= FrameBlockSize[i])
			break;
	}

	if(PAGE_FRAME_BLOCK_NUM == i)    //The dwSize is too large,invalid parameter.
		goto __TERMINAL;

	lpFrameManager = (__PAGE_FRAME_MANAGER*)lpThis;
	dwOffset       = (DWORD)lpStartAddr - (DWORD)lpFrameManager->lpStartAddress;
	k              = dwOffset / FrameBlockSize[i];  //The k-th block.

	lpPageFrame    = (__PAGE_FRAME*)((DWORD)lpFrameManager->lpPageFrameArray + 
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

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);

	if(TestBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k))  //If the current block's
		                                                          //buddy block also free,
																  //then combine the two blocks,
																  //and insert into up level
																  //block list.
	{
		while(i < PAGE_FRAME_BLOCK_NUM - 1)
		{
			if(k % 2)    //The behind block of lpPageFrame is it's buddy block.
			{
				lpTempFrame = (__PAGE_FRAME*)((DWORD)lpPageFrame + (FrameBlockSize[i] / 
					PAGE_FRAME_SIZE) * sizeof(__PAGE_FRAME));
				//lpLargeFrame = lpPageFrame;
			}
			else         //The previous block of lpPageFrame is it's buddy block.
			{
				lpTempFrame = (__PAGE_FRAME*)((DWORD)lpPageFrame - (FrameBlockSize[i] /
					PAGE_FRAME_SIZE) * sizeof(__PAGE_FRAME));
				//lpLargeFrame = lpTempFrame;
				lpPageFrame = lpTempFrame;
			}

			//The following code delete the buddy block from the current list,and insert
			//the combined block into up level block list.
			if(NULL == lpTempFrame->lpPrevFrame)    //This is the first block.
			{
				if(NULL == lpTempFrame->lpNextFrame)  //This is the last block.
				{
					//lpTempFrame->lpNextFrame->lpPrevFrame = NULL;
					lpFrameManager->FrameBlockArray[i].lpNextBlock = lpTempFrame->lpNextFrame;
				}
				else
				{
					lpTempFrame->lpNextFrame->lpPrevFrame = NULL;
					lpFrameManager->FrameBlockArray[i].lpNextBlock = lpTempFrame->lpNextFrame;
				}
			}
			else  //This is not the first block.
			{
				if(NULL == lpTempFrame->lpNextFrame) //This is the last block.
				{
					lpTempFrame->lpPrevFrame->lpNextFrame = NULL;
				}
				else  //This is not the last block.
				{
					lpTempFrame->lpPrevFrame->lpNextFrame = lpTempFrame->lpNextFrame;
					lpTempFrame->lpNextFrame->lpPrevFrame = lpTempFrame->lpPrevFrame;
				}
			}

			ClearBitmapBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k);  //Clear bit.

			i ++;  //To check up level block link.

			k  = (DWORD)lpPageFrame - (DWORD)lpFrameManager->lpPageFrameArray;
			k /= sizeof(__PAGE_FRAME);
			k /= (FrameBlockSize[i] / PAGE_FRAME_SIZE);

			if(k % 2)
				k -= 1;
			else
				k += 1;

			if(!TestBit(lpFrameManager->FrameBlockArray[i].lpdwBitmap,k))
				break;
		}

		//
		//The following code inserts the combined block into up level block list.
		//

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
	else      //The buddy of the current block is occupied.
		      //So,only insert the current block into the block
			  //list.
	{
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

	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	return;
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The definition of object PageFrameManager.
//

__PAGE_FRAME_MANAGER PageFrameManager = {
	NULL,                                  //lpPageFrameArray.
	{0},                                   //FrameBlockArray.
	0,                                    //dwTotalFrameNum.
	0,                                    //dwFreeFrameNum.
	NULL,                                  //lpStartAddress.
	PageFrameMgrInit,                      //Initialize routine.
	FrameAlloc,                            //FrameAlloc routine.
	FrameFree                              //FrameFree routine.
};

#endif
