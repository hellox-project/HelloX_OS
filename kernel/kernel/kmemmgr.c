//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,31 2004
//    Module Name               : kmemmgr.cpp
//    Module Funciton           : 
//                                This module countains the kernal memory
//                                management code.
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
#include <types.h>
#include <kmemmgr.h>
#include "buffmgr.h"
#include "types.h"
#include "ktmgr.h"
#include <../config/config.h>

//KMEM_4K_START_ADDRESS = 0x00200000
//KMEM_4K_END_ADDRESS   = 0x00BFFFFF

//KMEM_ANYSIZE_START_ADDRESS = 0x00C00000
//KMEM_AYNSIZE_END_ADDRESS   = 0x013EFFEF

//KMEM_STACK_START_ADDRESS   = 0x013EFFF0
//KMEM_STACK_END_ADDRESS     = 0x013FFFF0

//
//4K block pool controller.
//
static __4KSIZE_BLOCK g_4kBlockPool[] = {
	{(LPVOID)(KMEM_4K_START_ADDRESS + 0x00000000),0x00100000,0x00000000},  //First 1M pool.
	{(LPVOID)(KMEM_4K_START_ADDRESS + 0x00100000),0x00100000,0x00000000},  //Second 1M pool.
	{(LPVOID)(KMEM_4K_START_ADDRESS + 0x00200000),0x00100000,0x00000000},
	{(LPVOID)(KMEM_4K_START_ADDRESS + 0x00300000),0x00100000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000},
	{0x00000000,0x00000000,0x00000000}};


//
//Some helper functions.
//
//The following function find a 0's string
//in an 8 DWORD element array,and returns the 
//index of first zero.
//If failed,returns 256.

#define BIT_STRING_SIZE 8            //The bit string's length to be searched
	                                 //is 8 DWORD,256 bits.

DWORD Find0String(DWORD dwNum,BYTE* pdwArray)
{
	DWORD dwIndex = BIT_STRING_SIZE * 8 * 4;
	DWORD i       = 0;
	DWORD j       = 1;
	DWORD k       = 0;
	BOOL  bFind   = FALSE;

	if((0 == dwNum) || (NULL == pdwArray))  //Parameters check.
		return dwIndex;

	while(i < BIT_STRING_SIZE * 8 * 4)
	{
		if((*pdwArray) & j)                //To find the first zero.
		{
			i ++;
			j <<= 1;
			if(256 == j)                   //Search over one byte,then the other byte.
			{
				j = 1;
				pdwArray ++;
			}
			continue;
		}
		else{                            //Found the first zero.
			dwIndex = i;                 //Assume this is the start position of 0's string.
			if(1 == dwNum)
			{
				bFind = TRUE;
				break;
			}
			bFind = TRUE;               //Assume we can find the correct 0 string.
			i ++;
			j <<= 1;
			if(256 == j)
			{
				j = 1;
				pdwArray ++;
			}
			for(k = 0;k < dwNum - 1;k ++)
			{
				if((*pdwArray) & j)     //If find one 1,then false.
				{
					bFind = FALSE;
					break;
				}
				i ++;
				j <<= 1;
				if( 256 == j)
				{
					j = 1;
					pdwArray ++;
				}
			}
			if(bFind)                  //Found the correct 0's string.
				break;
		}
	}
	return dwIndex;
}

//
//Set bit to 0 or 1.This function is used by allocater to set occupy map bit.
//

VOID SetBit(DWORD dwStart,DWORD dwBitNum,BYTE* pbArray)
{
	BYTE bt = 1;

	pbArray += dwStart / 8;             //Adjust the position pointer.
	dwStart =  dwStart % 8;
	while(dwStart)                  
	{
		bt <<= 1;
		dwStart --;
	}
	while(dwBitNum)
	{
		*pbArray |= bt;
		bt <<= 1;
		if(0 == bt)
		{
			bt = 1;
			pbArray ++;
		}
		dwBitNum --;
	}
}

//
//This function update the max usable block in 4k block pool.
//The dwIndex parameter gives the index of the 4k block pool.
//This function searchs the occupying map,find the longest
//1's string,and update the max usable block size.
//

VOID UpdateMaxBlock(DWORD dwIndex)
{
	DWORD dw0Num     = 0x0000;
	DWORD dwTmp      = 0x0001;
	DWORD i          = 0x0000;
	DWORD j          = 0x0000;
	DWORD* pdwMap    = &g_4kBlockPool[dwIndex].dwOccupMap[0];

	while(i < 32*8)
	{
		if(*pdwMap & dwTmp)               //To find the first 0 bit.
		{
			i ++;
			dwTmp <<= 1;
			if(0 == dwTmp)
			{
				dwTmp = 1;
				pdwMap ++;
			}
			continue;
		}
		
		/*j ++;                             //Find the first 0 bit.
		dwTmp <<= 1;
		if(0 == dwTmp)
		{
			dwTmp = 1;
			pdwMap ++;
		}*/
		while((!(*pdwMap & dwTmp)) && (i < 32*8))         //Now,we have found the first 0.
		{
			j ++;
			i ++;
			dwTmp <<= 1;
			if(0 == dwTmp)
			{
				dwTmp = 1;
				pdwMap ++;
			}
		}
		if(j > dw0Num)                    //If the current found 0's string
			                              //is longer than before,update the
										  //dw1Num value to indicate this.
			dw0Num = j;
		j = 0;
	}

	g_4kBlockPool[dwIndex].dwMaxBlockSize = 4096 * dw0Num;  //Now,the variable,dw0Num
	                                                        //countains the longest
	                                                        //zero string's length.
	
}

//
//4k memory allocation functions.
//

LPVOID _4kAllocate(DWORD dwSize)        //The parameter,dwSize must be 4k's times.
{
	LPVOID pStartAddress = NULL;
	BOOL   bFind         = FALSE;
	DWORD  dw0Num        = 0;
	DWORD  dwIndex       = 0;
	DWORD  i             = 0;
	DWORD  dwTmp         = 0;

	if((dwSize % 4096) || (!dwSize))    //If the size is not 4k's times,return false.
		return pStartAddress;
	if(dwSize > KMEM_MAX_BLOCK_SIZE)
		return pStartAddress;

	for(i = 0;i < KMEM_MAX_4K_POOL_NUM;i ++)
	{
		if(g_4kBlockPool[i].dwMaxBlockSize >= dwSize)  //To find the enough large block.
		{
			bFind = TRUE;
			break;
		}
	}
	if(FALSE == bFind)                  //Can not find a enough block to allocate.
		return pStartAddress;

	dw0Num = dwSize / 4096;             //How much 4K block must be allocated.
	dwIndex = Find0String(dw0Num,(BYTE*)&g_4kBlockPool[i].dwOccupMap[0]);
	if(BIT_STRING_SIZE * 8 * 4 == dwIndex)
		return pStartAddress;           //Can not found the fitable block.

	pStartAddress =  g_4kBlockPool[i].pStartAddress;
	dwTmp = (DWORD)pStartAddress;
	dwTmp += 4096 * dwIndex;
	pStartAddress = (LPVOID)dwTmp;
	//g_4kBlockPool[i].dwMaxBlockSize -= dwSize;    //Adjust the max block size.
	SetBit(dwIndex,dw0Num,(BYTE*)&g_4kBlockPool[i].dwOccupMap[0]);  //Set the occupy bit.
	UpdateMaxBlock(i);                    //Udjust the max block size.
	return pStartAddress;
}

//
//Free function,this function frees the memory allocated by KMemAlloc function.
//
VOID _4kFree(LPVOID pStartAddress,DWORD dwSize)
{
	BOOL bFind        = FALSE;
	DWORD dwLoop      = 0x0000;
	DWORD dwIndex     = 0x0000;
	DWORD dwStartPos  = 0x0000;
	DWORD dw1Num      = 0x0000;
	DWORD i           = 0;

	if((0 == dwSize)
		|| (NULL == pStartAddress)
		|| ((DWORD)pStartAddress % 4096))        //Parameters check.
		return;

	for(dwLoop = 0;dwLoop < KMEM_MAX_4K_POOL_NUM;dwLoop ++)
	{
		if((g_4kBlockPool[dwLoop].pStartAddress <= pStartAddress)
			&&((DWORD)g_4kBlockPool[dwLoop].pStartAddress + KMEM_MAX_BLOCK_SIZE > (DWORD)pStartAddress))
		{
			bFind = TRUE;
			break;
		}
	}
	if(FALSE == bFind)                    //Can not find the correct block.
		return;

	dwIndex =  ((DWORD)pStartAddress - (DWORD)g_4kBlockPool[dwLoop].pStartAddress) / 4096;
	dwStartPos = dwIndex % 32;            //Get the start position of the first 1.
	dwIndex /= 32;                        //Get the first map word of the block.
	RoundTo4k(dwSize);                    //Round to 4k.
	dw1Num = dwSize / 4096;               //How many 4k blocks will be freed.

	for(i = 0;i < dw1Num;i ++)
	{
		ClearBit(g_4kBlockPool[dwLoop].dwOccupMap[dwIndex],dwStartPos);  //Clear the bit.
		dwStartPos += 1;             //Get the start position of the 1 bit.
		if(32 == dwStartPos)
		{
			dwStartPos -= 32;
			dwIndex += 1;
		}
	}

	UpdateMaxBlock(dwLoop);        //Update the max block size.
}

//
//Allocate memory for kernel and applications.
//
LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType)
{
	LPVOID pMemAddress = NULL;
	DWORD  dwFlag      = 0;

	if(0 == dwSize)
	{
		return pMemAddress;               //Parameter check.
	}

	//Align size to alignment boundary,which is defined in config.h file.
	dwSize = ((dwSize + SYSTEM_BYTE_ALIGN - 1) & ~(SYSTEM_BYTE_ALIGN - 1));

	switch(dwSizeType)
	{
	case KMEM_SIZE_TYPE_ANY:
		dwFlag = AnySizeBuffer.GetControlBlockFlag(&AnySizeBuffer);
		//Check if the global buffer management object is initialied,allocation will
		//fail if not initialized.It should be initialized in OS entry.
		if((!(dwFlag & POOL_INITIALIZED)) || (!(dwFlag & OPERATIONS_INITIALIZED)))
		{
			return pMemAddress;
		}
        pMemAddress = AnySizeBuffer.Allocate(&AnySizeBuffer,dwSize);
		break;
	case KMEM_SIZE_TYPE_4K:
		RoundTo4k(dwSize);                //Round the dwSize to 4k times.
		pMemAddress = _4kAllocate(dwSize);
		break;
	default:
		break;
	}
	return pMemAddress;
}

//
//Free the memory allocated by KMemAlloc routine.
//
VOID KMemFree(LPVOID pStartAddress,DWORD dwSizeType,DWORD dwSize)
{
	DWORD  dwFlag = 0;
	switch(dwSizeType)
	{
	case KMEM_SIZE_TYPE_ANY:
		dwFlag = AnySizeBuffer.GetControlBlockFlag(&AnySizeBuffer);
		if((!(dwFlag & POOL_INITIALIZED)) || (!(dwFlag & OPERATIONS_INITIALIZED)))
		{
			return;
		}
		AnySizeBuffer.Free(&AnySizeBuffer,pStartAddress);
		break;
	case KMEM_SIZE_TYPE_4K:
		_4kFree(pStartAddress,dwSize);
	default:
		break;
	}
	return;
}

//
//Get the free memory size of the system.
//
DWORD GetFreeMemorySize()
{
	return AnySizeBuffer.dwFreeSize;
}

//Get the total memory size in system.
DWORD GetTotalMemorySize()
{
	return AnySizeBuffer.dwPoolSize;
}

