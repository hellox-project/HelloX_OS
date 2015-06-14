//
//Application simple of Hello China.
//
#ifndef __STDAFX_H__
#include "../include/StdAfx.h"
#endif

#ifndef __KAPI_H__
#include "../include/kapi.h"
#endif

#include "shell.h"
#include "fibonacci.h"

typedef struct{
    DWORD dwInitNum;
    DWORD dwResult;
}__FIBONACCI_CONTROL_BLOCK;

DWORD CalculateThread(LPVOID lpParam)  //Calculate fibonacci sequence's element.
{
    __FIBONACCI_CONTROL_BLOCK* lpControlBlock = (__FIBONACCI_CONTROL_BLOCK*)lpParam;
    DWORD dwS0 = 0;
    DWORD dwS1 = 1;
    DWORD dwS2 = 0;
	DWORD i    = 0;

    if(NULL == lpControlBlock)
        return 0;
    for(i = lpControlBlock->dwInitNum;i > 0;i --)
    {
        dwS2 = dwS0 + dwS1;
        dwS0 = dwS1;
        dwS1 = dwS2;
    }
    lpControlBlock->dwResult = dwS0;
    return 1L;
}


DWORD Fibonacci(LPVOID lpParam)
{
    //LPSTR lpszParam = (LPSTR)lpParam;
	__CMD_PARA_OBJ* pCmdParaObj               = (__CMD_PARA_OBJ*)lpParam;
    __FIBONACCI_CONTROL_BLOCK ControlBlock[5] = {0};
    HANDLE hThread[5] = {NULL};
    CHAR Buffer[12];
    DWORD dwCounter;
    DWORD dwIndex,i;

	PrintLine("Fibonacci application running...");
	GotoHome();
	ChangeLine();

	if(NULL == pCmdParaObj || pCmdParaObj->byParameterNum < 2)
	{
		return 0;
	}

    dwCounter = 0;
    for(i = 0;i < 5;i ++)
    {     
        dwIndex = 0;
        while(pCmdParaObj->Parameter[1][dwCounter])
		{
			Buffer[dwIndex] =  pCmdParaObj->Parameter[1][dwCounter];
			dwIndex ++;
			dwCounter ++;
		}

        Buffer[dwIndex] = 0;
        Str2Hex(Buffer,&ControlBlock[i].dwInitNum);  //Convert the parameter to integer.

        if(pCmdParaObj->Parameter[1][dwCounter])
		{
			break;
		}
    }

	i = 5;
    for(i;i > 0;i --)
    {
        hThread[i - 1] = CreateKernelThread(
			0,    //Stack size,use default.
            KERNEL_THREAD_STATUS_READY,  //Status.
			PRIORITY_LEVEL_NORMAL,
			CalculateThread,  //Start routine.
			(LPVOID)&ControlBlock[i - 1],
            NULL,
			"FIBONACCI");

        if(NULL == hThread[i - 1])  //Failed to create kernel thread.
        {
            PrintLine("Create kernel thread failed.");
            break;
        }
    }

    //
    //Waiting for the kernel thread to over.
    //
    WaitForThisObject(hThread[0]);
    WaitForThisObject(hThread[1]);
    WaitForThisObject(hThread[2]);
    WaitForThisObject(hThread[3]);
    WaitForThisObject(hThread[4]);

    //
    //Now,we have calculated the fibonacci number,print them out.
    //
    for(i = 0;i < 5;i ++)
    {
        Int2Str(ControlBlock[i].dwInitNum,Buffer);
        PrintStr(Buffer);
        PrintStr("'s result is: ");
        Int2Str(ControlBlock[i].dwResult,Buffer);
        PrintStr(Buffer);
		GotoHome();
        ChangeLine();
    }

    //
    //Close the kernel thread.
    //
    DestroyKernelThread(hThread[0]);
    DestroyKernelThread(hThread[1]);
    DestroyKernelThread(hThread[2]);
    DestroyKernelThread(hThread[3]);
    DestroyKernelThread(hThread[4]);
    
    return 1L;
}

