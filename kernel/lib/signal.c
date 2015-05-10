//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : signal.c
//    Module Funciton           : 
//                                In order to port JamVM into HelloX,we must
//                                simulate POSIX SIGNAL operations,which
//                                are widely refered by JamVM.
//                                This file contains the source code of signal
//                                simulation mechanism.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stddef.h>
#include <kapi.h>
#include <pthread.h>
#include <signal.h>

//Set specified handling routine for a given signal,returns the old action,in
//process level.
int sigaction(int signum,const struct sigaction* act,struct sigaction* oldact)
{
	return 0;
}

//Change current process's signal mask.
int sigprocmask(int how,sigset_t* set,sigset_t* oldset)
{
	return 0;
}

//Set all supported signal value to set.
int sigfillset(sigset_t* set)
{
	*set = ~(sigset_t)0;
	return 0;
}

//Add the @sig into @mask.
int sigaddset(sigset_t* mask,int sig)
{
	*mask |= sig;
	return 0;
}

//Initialize a signal set to all zero.
int sigemptyset(sigset_t* set)
{
	*set = 0;
	return 0;
}

//Remove the @sig from @set.
int sigdelset(sigset_t* set,int sig)
{
	*set &= ~sig;
	return 0;
}

//Wait the signals in @set to be hung,and return it through
//@sig variable.
//All signals in set must be blocked,otherwise it will be processed
//by system without waking up the waiting process.
//Message mechnism can be used to simulate this routine.
int sigwait(const sigset_t* set,int* sig)
{
	return 0;
}

//Like the sigprocmask routine,but only apply for thread level.
int pthread_sigmask(int how,sigset_t* set,sigset_t* oldset)
{
	return 0;
}

//Suspend the current process to wait the signal that is NOT in
//mask.
//This routine replace the process's signal mask by usging the 
//@mask and suspend.
//Any signal that is NOT in mask will wake up the suspended process,
//other signals in mask can not wake up the process.
int sigsuspend(sigset_t* mask)
{
	return 0;
}

