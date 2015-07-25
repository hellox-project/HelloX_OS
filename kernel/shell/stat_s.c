//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STAT_S.CPP
//    Module Funciton           : 
//                                Countains CPU overload ratio statistics related
//                                data structures and routines.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "statcpu.h"
#include "stdio.h"
#include "stat_s.h"

__KERNEL_THREAD_OBJECT*  lpStatKernelThread = NULL;  //Used to save statistics kernel
                                                     //thread's object.

//Print out memory usage information.
static VOID ShowMemInfo()
{
	CHAR   buff[256];
	DWORD  dwFlags;

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

	PrintLine("    Free block list algorithm is adopted:");
	//Get and dump out memory usage status.
	_hx_sprintf(buff,"    Total memory size     : %d(0x%X)",dwPoolSize,dwPoolSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory size      : %d(0x%X)",dwFreeSize,dwFreeSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory blocks    : %d",dwFreeBlocks);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc success times   : %d/%d",dwAllocTimesSuccH,dwAllocTimesSuccL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc operation times : %d/%d",dwAllocTimesH,dwAllocTimesL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free operation times  : %d/%d",dwFreeTimesH,dwFreeTimesL);
	PrintLine(buff);
}

//
//The following routine is used to print out all devices information
//in system.
//
static VOID ShowDevList()
{
#ifdef __CFG_SYS_DDF
	__DEVICE_OBJECT*   lpDevObject = IOManager.lpDeviceRoot;
	CHAR strInfo[256];
	
	PrintLine("                  DeviceName   Attribute   BlockSize       RdSize/WrSize");
	while(lpDevObject)
	{
		_hx_sprintf(strInfo,"    %24s   %8X    %8d    %8d/%8d",
			lpDevObject->DevName,
			lpDevObject->dwAttribute,
			lpDevObject->dwBlockSize,
			lpDevObject->dwMaxReadSize,
			lpDevObject->dwMaxWriteSize);
		PrintLine(strInfo);
		lpDevObject = lpDevObject->lpNext;
	}
#endif
}

//
//This routine is used to print out CPU statistics information.
//
static VOID ShowStatInfo()  //Display the statistics information.
{
	__THREAD_STAT_OBJECT* lpStatObj = &StatCpuObject.IdleThreadStatObj;
	CHAR Buff[256];

	//Print table header.
	PrintLine("      Thread Name  Thread ID  1s usage  60s usage  5m usage");
	PrintLine("    -------------  ---------  --------  ---------  --------");
	//For each kernel thread,print out statistics information.
	do{
		_hx_sprintf(Buff,"    %13s  %9X  %6d.%d  %7d.%d  %6d.%d",
			lpStatObj->lpKernelThread->KernelThreadName,
			lpStatObj->lpKernelThread->dwThreadID,
			lpStatObj->wCurrPeriodRatio / 10,
			lpStatObj->wCurrPeriodRatio % 10,
			lpStatObj->wOneMinuteRatio  / 10,
			lpStatObj->wOneMinuteRatio  % 10,
			lpStatObj->wMaxStatRatio    / 10,
			lpStatObj->wMaxStatRatio    % 10
			//lpStatObj->TotalCpuCycle.dwHighPart,
			//lpStatObj->TotalCpuCycle.dwLowPart
			);
		PrintLine(Buff);

		lpStatObj = lpStatObj->lpNext;
	}while(lpStatObj != &StatCpuObject.IdleThreadStatObj);
}

//
//Entry point of the statistics kernel thread.
//
DWORD StatThreadRoutine(LPVOID lpData)
{
	__TIMER_OBJECT*           lpTimer  = NULL;
	__KERNEL_THREAD_MESSAGE   msg;

	//Set a timer,to calculate statistics information periodic.
	lpTimer = (__TIMER_OBJECT*)System.SetTimer((__COMMON_OBJECT*)&System,
		KernelThreadManager.lpCurrentKernelThread,
		1024,
		1000,
		NULL,
		NULL,
		TIMER_FLAGS_ALWAYS);

	while(TRUE)
	{
		if(KernelThreadManager.GetMessage((__COMMON_OBJECT*)KernelThreadManager.lpCurrentKernelThread,&msg)) //Get message to process.
		{
			switch(msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				StatCpuObject.DoStat();  //Do statistics.
				break;
			case STAT_MSG_SHOWSTATINFO:
				ShowStatInfo();
				break;
			case STAT_MSG_SHOWMEMINFO:
				ShowMemInfo();
				break;
			case STAT_MSG_LISTDEV:  //List the device information.
				ShowDevList();
				break;
			case STAT_MSG_TERMINAL:  //Should exit.
				goto __EXIT;
			default:
				break;
			}
		}
	}

__EXIT:
	//Cancel the timer first.
	System.CancelTimer((__COMMON_OBJECT*)&System,
		(__COMMON_OBJECT*)lpTimer);
	return 0;
}
