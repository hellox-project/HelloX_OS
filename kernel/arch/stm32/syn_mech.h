//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,26 2005
//    Module Name               : SYN_MECH.H
//    Module Funciton           : 
//                                This module countains synchronization code for system kernel.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/


#ifndef __SYN_MECH_H__
#define __SYN_MECH_H__


#ifdef __cplusplus
extern "C" {
#endif


//System level critical section operations definition.
extern DWORD Enter_Critical_Section(void);
extern void Leave_Critical_Section(DWORD dwFlags);


//Disable and enable of interrupt.
extern void EnableInterrupt(void);
extern void DisableInterrupt(void);


#define __ENTER_CRITICAL_SECTION(objptr,dwFlags) \
	dwFlags = Enter_Critical_Section();


#define __LEAVE_CRITICAL_SECTION(objptr,dwFlags) \
	Leave_Critical_Section(dwFlags);
	
//Interrupt enable and disable operation.
#ifdef __I386__

#ifdef __GCC__
	#define __ENABLE_INTERRUPT() {__asm__("sti \n\t"::);}
#else
	#define __ENABLE_INTERRUPT() {__asm sti};
#else

#define __ENABLE_INTERRUPT() EnableInterrupt()
#endif


#ifdef __I386__

#ifdef __GCC__
	#define __DISABLE_INTERRUPT() { __asm__ ("cli \n\t"::); }
#else
	#define __DISABLE_INTERRUPT() {__asm cli}
#endif

#else
#define __DISABLE_INTERRUPT() DisableInterrupt()
#endif


typedef unsigned long __ATOMIC_T;


#define __INIT_ATOMIC(t) (t) = 0


/*#ifdef __INLINE_ENABLE
#define INLINE inline
#else
#define INLINE
#endif*/
#ifdef __cplusplus
}
#endif


#endif    //__SYN_MECH_H__
