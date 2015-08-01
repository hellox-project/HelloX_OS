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

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "idle.h"

//
//System idle thread.
//When there is no kernel thread to schedule,the system idle thread will run.
//In this kernel thread,some system level tasks can be done.
//This kernel thread will never exit until the system is down.
//
DWORD SystemIdle(LPVOID lpData)
{
	static DWORD dwIdleCounter = 0;
	while(TRUE)
	{
		dwIdleCounter ++;
		if(0xFFFFFFFF == dwIdleCounter)
		{
			dwIdleCounter = 0;
		}
		//Halt the current CPU.
		HaltSystem();
	}
}
