#include <stm32f10x.h>                       /* STM32F10x definitions         */

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include <stdio.h>

extern void SER_PutString(char*);  //For debugging.

//A helper routine to dumpout memory usage information.
static VOID ShowMemory(__COMMON_OBJECT* hUsart)
{
	CHAR   buff[256];
	DWORD  dwFlags;
	DWORD  dwWriteLen;   //How many bytes written to usart.
	CHAR*  pszHdr = "\r\n    Free block list algorithm is adopted:\r\n";

	DWORD  dwPoolSize;
	DWORD  dwFreeSize;
	DWORD  dwFreeBlocks;
	DWORD  dwAllocTimesSuccL;
	DWORD  dwAllocTimesSuccH;
	DWORD  dwAllocTimesL;
	DWORD  dwAllocTimesH;
	DWORD  dwFreeTimesL;
	DWORD  dwFreeTimesH;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPoolSize      = AnySizeBuffer.dwPoolSize;
	dwFreeSize      = AnySizeBuffer.dwFreeSize;
	dwFreeBlocks    = AnySizeBuffer.dwFreeBlocks;
	dwAllocTimesSuccL  = AnySizeBuffer.dwAllocTimesSuccL;
	dwAllocTimesSuccH  = AnySizeBuffer.dwAllocTimesSuccH;
	dwAllocTimesL      = AnySizeBuffer.dwAllocTimesL;
	dwAllocTimesH      = AnySizeBuffer.dwAllocTimesH;
	dwFreeTimesL       = AnySizeBuffer.dwFreeTimesL;
	dwFreeTimesH       = AnySizeBuffer.dwFreeTimesH;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

  //Write out statistic form's header.
	IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(pszHdr),
	   pszHdr,
	   &dwWriteLen);
	//Get and dump out memory usage status.
	_hx_sprintf(buff,"    Total memory size     : %d(0x%X)\r\n",dwPoolSize,dwPoolSize);
	IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
	_hx_sprintf(buff,"    Free memory size      : %d(0x%X)\r\n",dwFreeSize,dwFreeSize);
	IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
	_hx_sprintf(buff,"    Free memory blocks    : %d\r\n",dwFreeBlocks);
	IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
	_hx_sprintf(buff,"    Alloc success times   : %d/%d\r\n",dwAllocTimesSuccH,dwAllocTimesSuccL);
		IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
	_hx_sprintf(buff,"    Alloc operation times : %d/%d\r\n",dwAllocTimesH,dwAllocTimesL);
		IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
	_hx_sprintf(buff,"    Free operation times  : %d/%d\r\n",dwFreeTimesH,dwFreeTimesL);
		IOManager.WriteFile(
	   (__COMMON_OBJECT*)&IOManager,
	   hUsart,
	   strlen(buff),
	   buff,
	   &dwWriteLen);
}

//------------------------------------------------------------------------------
// User's application entry.
//------------------------------------------------------------------------------
DWORD _HCNMain(LPVOID pData)
{
	__COMMON_OBJECT* hUsart = NULL;
	DWORD           dwWriteLen = 0;
	CHAR            chCmd;
	CHAR            strInfo[64];
	
	SER_PutString("_HCNMain thread is running...\r\n");
	
	hUsart = IOManager.CreateFile(
	    (__COMMON_OBJECT*)&IOManager,
	    "\\\\.\\USART1",
	    0,
	    0,
	    NULL);
	if(NULL == hUsart)
	{
		SER_PutString("_HCNMain: Can not open USART object.\r\n");
		return 0;
	}
	SER_PutString("_HCNMain: Open device USART successfully.\r\n");
	while(TRUE)
	{
		if(IOManager.ReadFile(
			(__COMMON_OBJECT*)&IOManager,
		  hUsart,
		  1,
		  &chCmd,
		  NULL))
		{
			IOManager.WriteFile(
		    (__COMMON_OBJECT*)&IOManager,
		    hUsart,
		    1,
		    &chCmd,
		    &dwWriteLen);
			if('m' == chCmd)
			{
				ShowMemory(hUsart);
			}
		}
	}
	//return 1;
}
