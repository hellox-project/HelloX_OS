//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 22,2017
//    Module Name               : tmo.h
//    Module Funciton           : 
//                                Timeout mechanism of network module.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __TMO_H__
#define __TMO_H__

/* Callback handler of time out. */
#ifndef sys_timeout_handler
typedef void(*sys_timeout_handler)(void *arg);
#endif

/*
* HelloX's implementation of sys_timeout and sys_untimeout
* routine,to replace
* the implementation of lwIP.This is mandatory since the lwIP's
* implementation is running in tcp_ip thread context,other modules such as
* PPPoE,is running in a dedicated kernel thread under HelloX.
* The HelloX's implementation is based on HelloX's timer mechanism
* in kernel,i.e,SetTimer,CancelTimer,...
* These routines must be called in a same kernel thread context,since
* it can not support multiple thread.
*/
void _hx_sys_timeout(HANDLE hTarget, unsigned long msecs, sys_timeout_handler handler, void* arg);
void _hx_sys_untimeout(sys_timeout_handler handler, void* arg);

/* Network timer object used to manage all network timers. */
typedef struct tag__network_timer_object{
	HANDLE hTimer;                 /* Handle of HelloX timer object. */
	sys_timeout_handler handler;
	void* handler_param;
	unsigned long msecs;
	DWORD dwObjectSignature;       /* Verify the object. */
	struct tag__network_timer_object* pNext;
	struct tag__network_timer_object* pPrev;
}__network_timer_object;

/* Release a network timer object. */
void _hx_release_network_tmo(__network_timer_object* pTmo);

#endif //__TMO_H__
