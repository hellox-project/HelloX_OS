//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 17,2015
//    Module Name               : signal.h
//    Module Funciton           : 
//                                In order to port JamVM into HelloX,we must
//                                simulate POSIX SIGNAL operations,which
//                                are widely refered by JamVM.
//                                This file defines signal related data types.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SIGNAL_H__
#define __SIGNAL_H__

//A dedicated signal set type,to contain signal bits.
typedef unsigned long sigset_t;

//Signals which should be simulate when port JamVM.
#define SIGUSR1      0x00000001
#define SIGQUIT      0x00000002
#define SIGPIPE      0x00000004
#define SIGINT       0x00000008
#define SIGTERM      0x00000010

//Some structures to simulate signal operation.
union sigval{
	int sival_int;
	void* sival_ptr;
};

typedef struct{
	int si_signo;
	int si_code;
	union sigval si_value;
	int si_errno;
	pid_t si_pid;
	uid_t si_uid;
	void* si_addr;
	int si_status;
	int si_band;
}siginfo_t;

//Signal action structure.
struct sigaction{
	void (*sa_handler)(int);
	void (*sa_sigaction)(int,siginfo_t*,void*);
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};

//Set specified handling routine for a given signal,returns the old action,in
//process level.
int sigaction(int signum,const struct sigaction* act,struct sigaction* oldact);

//Replace the process's signal mask,and returns the old one.
//@how : indicates how to do,it can be the values defined above.
//return value:
// 0 : success.
// -1: Some error occurs.
int sigprocmask(int how,sigset_t* set,sigset_t* oldset);

//Set all supported signal value to set.
int sigfillset(sigset_t* set);

//Add the @sig into @mask.
int sigaddset(sigset_t* mask,int sig);

//Remove @sig from @set.
int sigdelset(sigset_t* set,int sig);

//Initialize a signal set to all zero.
int sigemptyset(sigset_t* set);

//Wait the signals in @set to be hung,and return it through
//@sig variable.
//All signals in set must be blocked,otherwise it will be processed
//by system without waking up the waiting process.
//Message mechnism can be used to simulate this routine.
int sigwait(const sigset_t* set,int* sig);

//Like the sigprocmask routine,but only apply for thread level.
int pthread_sigmask(int how,sigset_t* set,sigset_t* oldset);

//Suspend the current process to wait the signal that is NOT in
//mask.
//This routine replace the process's signal mask by usging the 
//@mask and suspend.
//Any signal that is NOT in mask will wake up the suspended process,
//other signals in mask can not wake up the process.
int sigsuspend(sigset_t* mask);

#endif
