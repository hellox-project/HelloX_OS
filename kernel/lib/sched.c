//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : sched.c
//    Module Funciton           : 
//                                In order to port JamVM into HelloX,we must
//                                simulate POSIX schedule operations,which
//                                are widely refered by JamVM.
//                                This file implements the POSIX schedule
//                                routines.
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

#ifndef __KAPI_H__
#include "kapi.h"
#endif

#ifndef __SCHED_H__
#include "sched.h"
#endif

//Yield the CPU to other processes or threads.
void sched_yield()
{
	//A 0 argument of sleeping call will lead the re-schedule of OS.
	Sleep(0);
}

//exit routine.
void exit(int status)
{
	KernelThreadManager.TerminateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		NULL,status);
}

//abort routine.
void abort(void)
{
	KernelThreadManager.TerminateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		NULL,0);
}
