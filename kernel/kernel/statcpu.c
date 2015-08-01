//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STATCPU.H
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

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "statcpu.h"
#include "stdio.h"
#include "hellocn.h"
#include "kapi.h"



__THREAD_HOOK_ROUTINE  lpCreateHook        = NULL;
__THREAD_HOOK_ROUTINE  lpBeginScheduleHook = NULL;
__THREAD_HOOK_ROUTINE  lpEndScheduleHook   = NULL;
__THREAD_HOOK_ROUTINE  lpTerminalHook      = NULL;

//
//Create hook,when a kernel thread is created,this routine is called.
//
static DWORD CreateHook(__KERNEL_THREAD_OBJECT*  lpKernelThread,DWORD* lpdwUserData)
{
	__THREAD_STAT_OBJECT*        lpStatObj = &StatCpuObject.IdleThreadStatObj;
	DWORD                        dwFlags;

	if((NULL == lpdwUserData) || (NULL == lpKernelThread))  //Invalid parameter.
	{
		return 0;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpStatObj->lpKernelThread)  //This stat object was not used yet.
	{
		lpStatObj->lpKernelThread = lpKernelThread;
		*lpdwUserData             = (DWORD)lpStatObj;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 1;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//
	//Should create a kernel stat object.
	//
	lpStatObj = (__THREAD_STAT_OBJECT*)GET_KERNEL_MEMORY(sizeof(__THREAD_STAT_OBJECT));
	if(NULL == lpStatObj)  //Can not allocate memory.
	{
		return 0;
	}
	//Initialize this object.
	lpStatObj->lpKernelThread     = lpKernelThread;
	lpStatObj->CurrPeriodCycle.dwHighPart = 0;
	lpStatObj->CurrPeriodCycle.dwLowPart  = 0;
	lpStatObj->TotalCpuCycle.dwHighPart   = 0;
	lpStatObj->TotalCpuCycle.dwLowPart    = 0;

	lpStatObj->wQueueHdr   = 0;
	lpStatObj->wQueueTail  = 0;
	lpStatObj->lpPrev      = NULL;
	lpStatObj->lpNext      = NULL;
	lpStatObj->wMaxStatRatio    = 0;
	lpStatObj->wCurrPeriodRatio = 0;
	lpStatObj->wOneMinuteRatio  = 0;
	memzero((LPVOID)lpStatObj->RatioQueue,sizeof(lpStatObj->RatioQueue));  //Clear memory.

	*lpdwUserData             = (DWORD)lpStatObj;  //Save this object.

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	//Insert this object into stat list.
	lpStatObj->lpNext  = StatCpuObject.IdleThreadStatObj.lpNext;
	lpStatObj->lpPrev  = &StatCpuObject.IdleThreadStatObj;
	
	StatCpuObject.IdleThreadStatObj.lpNext->lpPrev = lpStatObj;
	StatCpuObject.IdleThreadStatObj.lpNext         = lpStatObj;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return 1L;
}

//
//Begin Schedule Hook,when a thread will be scheduled to run,this routine
//is called.
//
static DWORD BeginScheduleHook(__KERNEL_THREAD_OBJECT* lpKernelThread,
							   DWORD*                  lpdwUserData)
{
	__THREAD_STAT_OBJECT* lpStatObj = (__THREAD_STAT_OBJECT*)(*lpdwUserData);

	if((NULL == lpKernelThread) || (NULL == lpdwUserData))
	{
		return 0;
	}

	__GetTsc(&lpStatObj->PreviousTsc);  //Save current time stamp counter.

	return 1L;
}

//
//End Schedule Hook,when a kernel thread was scheduled to give the CPU,this routine
//is called.
//
static DWORD EndScheduleHook(__KERNEL_THREAD_OBJECT*   lpKernelThread,
							 DWORD*                    lpdwUserData)
{
	__THREAD_STAT_OBJECT* lpStatObj = (__THREAD_STAT_OBJECT*)(*lpdwUserData);
	__U64                 currtsc;

	if((NULL == lpKernelThread) || (NULL == lpdwUserData))
	{
		return 0;
	}

	__GetTsc(&currtsc);  //Get current time stamp counter.
	u64Sub(&currtsc,&lpStatObj->PreviousTsc,&currtsc);  //Get the difference.

	//Add the difference to current period cycle counter.
	u64Add(&lpStatObj->CurrPeriodCycle,&currtsc,&lpStatObj->CurrPeriodCycle);

	//Add the difference to total CPU cycle counter.
	u64Add(&lpStatObj->TotalCpuCycle,&currtsc,&lpStatObj->TotalCpuCycle);
	return 0;
}

//
//Terminal Hook,when a kernel thread was destroyed,this routine will be called.
//
static DWORD TerminalHook(__KERNEL_THREAD_OBJECT*      lpKernelThread,
						  DWORD*                       lpdwUserData)
{
	__THREAD_STAT_OBJECT* lpStatObj = (__THREAD_STAT_OBJECT*)(*lpdwUserData);
	DWORD                 dwFlags;

	if((NULL == lpKernelThread) || (NULL == lpdwUserData)) //Invalid parameters.
	{
		return 0;
	}

	//Delete this statistics object from stat object list.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpStatObj->lpNext->lpPrev = lpStatObj->lpPrev;
	lpStatObj->lpPrev->lpNext = lpStatObj->lpNext;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Free this object.
	FREE_KERNEL_MEMORY(lpStatObj);
	return 1L;
}

static BOOL Initialize(__STAT_CPU_OBJECT*  lpStatObj)
{
	lpCreateHook          = CreateHook;
	lpBeginScheduleHook   = BeginScheduleHook;
	lpEndScheduleHook     = EndScheduleHook;
	lpTerminalHook        = TerminalHook;

	//Install hook routines.
	KernelThreadManager.SetThreadHook(THREAD_HOOK_TYPE_CREATE,CreateHook);
	KernelThreadManager.SetThreadHook(THREAD_HOOK_TYPE_BEGINSCHEDULE,BeginScheduleHook);
	KernelThreadManager.SetThreadHook(THREAD_HOOK_TYPE_ENDSCHEDULE,EndScheduleHook);
	KernelThreadManager.SetThreadHook(THREAD_HOOK_TYPE_TERMINAL,TerminalHook);

	//Initialize the StatCpuObject.
	StatCpuObject.IdleThreadStatObj.lpNext = &StatCpuObject.IdleThreadStatObj;
	StatCpuObject.IdleThreadStatObj.lpPrev = &StatCpuObject.IdleThreadStatObj;

	//Save current CPU cycle counter.
	__GetTsc(&lpStatObj->PreviousTsc);

	return TRUE;
}

static __THREAD_STAT_OBJECT*  GetFirstThreadStat(void)
{
	return NULL;
}

static __THREAD_STAT_OBJECT*  GetNextThreadStat(__THREAD_STAT_OBJECT* lpStatObj)
{
	if(NULL == lpStatObj)
	{
		return NULL;
	}
	return lpStatObj->lpNext;
}

static VOID ShowStat(void)
{
}

//
//Do statistics action.
//This routine does the following:
// 1. Calculate total CPU cycle past since previous statistics action;
// 2. For each thread statistics object,calculate CPU occupancy ratio;
// 3. Save the result to thread statistics object,as ring queue;
// 4. Update total CPU cycle counter;
// 5. Record current CPU cycle counter.
//
static VOID DoStat(void)
{
	__STAT_CPU_OBJECT*    lpStatObj       = &StatCpuObject;
	__THREAD_STAT_OBJECT* lpThreadStatObj = NULL;
	__U64                 currtsc;
	__U64                 temp;
	__U64                 remainder;
	WORD                  ratio;
	int                   i,j;

	__GetTsc(&currtsc);
	u64Sub(&currtsc,&lpStatObj->PreviousTsc,&currtsc);  //Get CPU cycle difference.
	__GetTsc(&lpStatObj->PreviousTsc);  //Save current time stamp counter.
	lpStatObj->CurrPeriodCycle.dwHighPart = currtsc.dwHighPart;
	lpStatObj->CurrPeriodCycle.dwLowPart  = currtsc.dwLowPart;

	//Accumulate total CPU cycle since startup.
	u64Add(&lpStatObj->TotalCpuCycle,&currtsc,&lpStatObj->TotalCpuCycle);

	//Calculate each thread's CPU occupancy.
	remainder.dwHighPart = 0;
	remainder.dwLowPart  = 1000;  //Divisor used to shrink currtsc later.
	u64Div(&currtsc,&remainder,&currtsc,&remainder); //currtsc = currtsc / 1000.
	lpThreadStatObj = &lpStatObj->IdleThreadStatObj;
	do{
		temp = lpThreadStatObj->CurrPeriodCycle;
		u64Div(&temp,&currtsc,&temp,&remainder); //temp = temp / currtsc.
		ratio = (WORD)(temp.dwLowPart);
		//Clear this period's counter of the kernel thread.
		lpThreadStatObj->CurrPeriodCycle.dwHighPart = 0;
		lpThreadStatObj->CurrPeriodCycle.dwLowPart  = 0;

		//Save calculation result to kernel thread's statistics object.
		lpThreadStatObj->RatioQueue[lpThreadStatObj->wQueueHdr] = ratio;
		lpThreadStatObj->wCurrPeriodRatio                       = ratio;
		//Update ratio queue's pointers.
		lpThreadStatObj->wQueueHdr += 1;
		if(lpThreadStatObj->wQueueHdr == MAX_STAT_PERIOD)  //Exceed queue length.
		{
			lpThreadStatObj->wQueueHdr = 0;
		}
		if(lpThreadStatObj->wQueueHdr == lpThreadStatObj->wQueueTail)
		{
			lpThreadStatObj->wQueueTail += 1;
			if(MAX_STAT_PERIOD == lpThreadStatObj->wQueueTail)
			{
				lpThreadStatObj->wQueueTail = 0;
			}
		}
		//Calculate CPU occupancy ration in last one minute.
		lpThreadStatObj->wOneMinuteRatio   = 0;
		lpThreadStatObj->wMaxStatRatio     = 0;
		for(i = 0,j = lpThreadStatObj->wQueueHdr;i < MAX_STAT_PERIOD;i ++,j --)
		{
			if(j <= 0)
			{
				j = MAX_STAT_PERIOD;
			}

			if(i < ONE_MINUTE_PERIOD)  //Accumulate last ONE_MINUTE_PERIOD
				                                         //result.
			{
				lpThreadStatObj->wOneMinuteRatio += lpThreadStatObj->RatioQueue[j - 1];
			}
			lpThreadStatObj->wMaxStatRatio   += lpThreadStatObj->RatioQueue[j - 1];
		}
		lpThreadStatObj->wOneMinuteRatio /= ONE_MINUTE_PERIOD;
		lpThreadStatObj->wMaxStatRatio   /= MAX_STAT_PERIOD;
		//Process next thread statistics object.
		lpThreadStatObj = lpThreadStatObj->lpNext;
	}while(lpThreadStatObj != &lpStatObj->IdleThreadStatObj);
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/
//
//Global object StatCpuObject's declaration.
//
__STAT_CPU_OBJECT StatCpuObject = {
	{0},                         //PreviousTsc.
	{0},                         //CurrPeriodCycle.
	{0},                         //TotalCpuCycle.
	{0},                         //IdelThreadStatObj.

	Initialize,                  //Initialize.
	GetFirstThreadStat,          //GetFirstThreadStatObj.
	GetNextThreadStat,           //GetNextThreadStatObj.
	DoStat,                      //DoStat.
	ShowStat                     //ShowStat.
};

