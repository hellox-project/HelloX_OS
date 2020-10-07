//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : sched.c
//    Module Funciton           : 
//                                Simultes abort/exit/sched_yield and other
//                                POSIX APIs.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "sched.h"
#include "stdio.h"

/* Yield the CPU to other processes or threads. */
void sched_yield()
{
	/* 
	 * Call Sleep routine with parameter set to 0
	 * will lead the OS kernel to re-schedule all
	 * thread(s).
	 */
	Sleep(0);
}

/* 
 * Exit current thread forcely. 
 * It just calls the TerminateKernelThread routine
 * to terminate current thread brutely.
 * Please be noted that all other thread(s) belong
 * to same process will not be affected,it's unproper
 * in most case,so it's not suggested to call this
 * routine directly.
 * The difference between exit and abort is that
 * the destructors of global objects will be invoked
 * in exit but not in abort.
 */
void exit(int status)
{
	/* Show out a warning msg. */
	_hx_printf("[thread ID:%d]: exit is invoked.\r\n",
		GetCurrentThreadID());

	/* Invoke all destructors of global object. */
	/* ...... */
	/* Terminate current thread. */
	TerminateKernelThread(NULL, status);
}

/*
 * Abort current thread.
 * It just calls the TerminateKernelThread routine
 * to terminate current thread brutely.
 * Please be noted that all other thread(s) belong
 * to same process will not be affected,it's unproper
 * in most case,so it's not suggested to call this
 * routine directly.
 * The difference between exit and abort is that
 * the destructors of global objects will be invoked
 * in exit but not in abort.
 */
void abort(void)
{
	/* Show out a warning msg. */
	_hx_printf("[thread ID:%d]: abort is invoked.\r\n",
		GetCurrentThreadID());

	TerminateKernelThread(NULL, 0);
}
