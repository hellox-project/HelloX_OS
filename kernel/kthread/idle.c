//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 18 DEC, 2011
//    Module Name               : idle.cpp
//    Module Funciton           : 
//                                This module contains the IDLE thread implementation code.IDLE thread
//                                is one of the kernel level threads and will be scheduled when no any
//                                other thread need CPU.
//                                Auxiliary functions such as battery management will also be placed in
//                                this thread.
//                                These code lines are moved from os_entry.cpp file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include "idle.h"

#ifdef __I386__
//A keep alive function,to indicate user that the system is running well,but no
//active input is detected.
//The keyboard or mouse will not work in some rare case,the system seems halt from
//the user's side,this routine will show user that the system is OK.

//Detect if there is effective human input.
static BOOL DetectInput()
{
	__INTERRUPT_VECTOR_STAT ivs;

	//Check if there is key board interrupt,i.e,key board input.
	if (!System.GetInterruptStat((__COMMON_OBJECT*)&System,
		INTERRUPT_VECTOR_KEYBOARD, &ivs))
	{
		return TRUE;
	}
	if (ivs.dwSuccHandledInt >= 2)  //At lease 1 key strokes detected.
	{
		return TRUE;
	}
	//Check if there is mouse input.
	if (!System.GetInterruptStat((__COMMON_OBJECT*)&System,
		INTERRUPT_VECTOR_MOUSE, &ivs))
	{
		return TRUE;
	}
	if (ivs.dwSuccHandledInt >= 6)
	{
		return TRUE;
	}
	return FALSE;
}

//Print out alive message routinely.
static void ShowAlive()
{
	DWORD dwMillionSecond = 0;

	dwMillionSecond =  System.GetClockTickCounter((__COMMON_OBJECT*)&System);
	dwMillionSecond += 1;  //Skip the first 0 clock tick.
	dwMillionSecond *= SYSTEM_TIME_SLICE;
	if (0 == dwMillionSecond % SHOW_ALIVE_TIMESPAN)
	{
		if (DetectInput())  //User input detected.
		{
			return;
		}
		//No active input untile then,show alive message.
		//_hx_printf("  System is alive,but no human input yet.\r\n");
	}
}
#endif

//
//System idle thread.
//When there is no kernel thread to schedule,the system idle thread will run.
//Some system level tasks can be processed in this thread,unless response time is not 
//senstive,since this thread will not be scheduled in case of busying.
//This kernel thread will never exit until the system is down.
//
DWORD SystemIdle(LPVOID lpData)
{
	static DWORD dwIdleCounter = 0;
	while(TRUE)
	{
#ifdef __I386__
		ShowAlive();
#endif
		dwIdleCounter ++;
		if(0xFFFFFFFF == dwIdleCounter)
		{
			dwIdleCounter = 0;
		}
		//Halt the current CPU.
		HaltSystem();
	}
}
