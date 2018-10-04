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

/* For system level configurations. */
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Critical section operating macros.
 * Just disable local interrupt since only
 * available for UP.
 */
#ifdef __I386__
#ifdef __GCC__
	#define __ENTER_CRITICAL_SECTION_UP(lpObj,dwFlags)	\
		__asm__ __volatile__("pushfl; popl %0 ; cli" : "=g" (dwFlags): /* no input */ :"memory");
#else
	#define __ENTER_CRITICAL_SECTION_UP(lpObj, dwFlags) \
		__asm	push eax			 \
		__asm	pushfd               \
		__asm	pop eax              \
		__asm	mov dwFlags,eax      \
		__asm	pop eax              \
		__asm	cli
#endif  //__GCC__
#else
#define __ENTER_CRITICAL_SECTION_UP(lpObj,dwFlags) \
	(dwFlags = 0);
#endif //__I386__

#ifdef __I386__
#ifdef __GCC__
#define __LEAVE_CRITICAL_SECTION_UP(lpObj,dwFlags)	\
	__asm__ __volatile__("pushl %0; popf " :	 : "g"(dwFlags): "memory")
#else
#define __LEAVE_CRITICAL_SECTION_UP(lpObj,dwFlags) \
    __asm push dwFlags	\
	__asm popfd

#endif
#else
#define __LEAVE_CRITICAL_SECTION_UP(lpObj,dwFlags)
	(dwFlags = 0);
#endif

/*
 * Alias of critical section operating macros in UP
 * environment.
 */
#define __DISABLE_LOCAL_INTERRUPT(dwFlags) __ENTER_CRITICAL_SECTION_UP(NULL,dwFlags)
#define __RESTORE_LOCAL_INTERRUPT(dwFlags) __LEAVE_CRITICAL_SECTION_UP(NULL,dwFlags)

/*
 * Unified critical section operation macros,they actually refer 
 * UP or SMP version of __ENTER_CRITICAL_SECTION/__LEAVECRITICAL_SECTION,
 * according to the configuration.
 */
//#if defined(__CFG_SYS_SMP)
//#define __ENTER_CRITICAL_SECTION(objptr,dwFlags) __ENTER_CRITICAL_SECTION_SMP((objptr)->spin_lock,dwFlags)
//#define __LEAVE_CRITICAL_SECTION(objptr,dwFlags) __LEAVE_CRITICAL_SECTION_SMP((objptr)->spin_lock,dwFlags)
//#else /* UP environment. */
#define __ENTER_CRITICAL_SECTION(objptr,dwFlags) __ENTER_CRITICAL_SECTION_UP(NULL,dwFlags)
#define __LEAVE_CRITICAL_SECTION(objptr,dwFlags) __LEAVE_CRITICAL_SECTION_UP(NULL,dwFlags)
//#endif

//Interrupt enable and disable operation.
#ifdef __I386__
#ifdef __GCC__
	#define __ENABLE_INTERRUPT() 	\
	{    							\
		__asm__("pushl %%eax	\n\t"::);			\
		__asm__("movb $0x20, %%al	\n\t"::);		\
		__asm__("outb %%al, $0x20	\n\t"::);		\
		__asm__("outb %%al, $0xa0	\n\t"::);		\
		__asm__("popl %%eax			\n\t"::);		\
		__asm__("sti	\n\t"	:::	"memory");					\
	}
#else
	#define __ENABLE_INTERRUPT() 	\
	{    							\
		__asm push eax    			\
		__asm mov al, 0x20    		\
		__asm out 0x20, al    		\
		__asm out 0xa0, al    		\
		__asm pop eax    			\
		__asm sti    				\
	}
#endif

#else
#define __ENABLE_INTERRUPT()
#endif

/* Flash TLB entry,specified by the virtual address. */
#if defined(__I386__)
#if defined(__CFG_SYS_VMM)
#define __FLUSH_PAGE_TLB(mem_ptr) \
{ \
__asm invlpg mem_ptr \
}
#else
#define __FLUSH_PAGE_TLB(ptr)
#endif //__CFG_SYS_VMM
#endif //__I386__

#ifdef __I386__
	#ifdef __GCC__
		#define __DISABLE_INTERRUPT() {__asm__("cli" : : :"memory"); }
	#else
		#define __DISABLE_INTERRUPT() {__asm cli}
	#endif
#else
	#define __DISABLE_INTERRUPT()
#endif

/*
* This macro is used to flush cache's content to memory,or just invalidate a range of
* cache content,according the flags parameter.
* Cache invalidating is not necessary under x86 platform,since the cache snooping is
* implemented in CPU,so we let it empty when the flags value is CACHE_FLUSH_INVALIDATE.
*/
//Flags to control the cache flushing.
#define CACHE_FLUSH_WRITEBACK   0x00000001
#define CACHE_FLUSH_INVALIDATE  0x00000002
#ifdef __I386__
#ifdef __GCC__
#define __FLUSH_CACHE(addr,leng,flags)  {__asm__ ("nop \n\t"::);}
#else
#define __FLUSH_CACHE(addr,leng,flags) \
    { \
        if(CACHE_FLUSH_WRITEBACK == flags) \
		        { \
			/* __asm wbinvd */ \
			__asm nop \
			__asm nop \
			__asm nop \
			__asm nop \
		        } \
		        else if(CACHE_FLUSH_INVALIDATE == flags) \
	        { \
	        } \
	} 
#endif
#else
#define __FLUSH_CACHE(addr,leng,flags)
#endif

/*
* This macros is used as a barrier.
* In some CPU,the write operations may not be committed into memory or cache,but 
* be queued in WRITE BUFFER on the CPU,so,in order to commit the write operation
* immediately,the following macro should be used.
* For example,the following code:
*  ... ...
*  WriteMemory(0xCFFFFFFF,90);
*  WriteMemory(0xCFFFFFFC,90);
*  ... ...
* 0xCFFFFFFF and 0xCFFFFFFC are device mapped control port,in order to commit the
* writing operations into device immediately,the following macro must be called.
*/
#ifdef __I386__
#ifdef __GCC__
#define __BARRIER() __asm__("LOCK addl $0, (%%esp) \n\t"::);
#else
#define __BARRIER() __asm LOCK add dword ptr [esp],0
#endif
#else
#define __BARRIER()
#endif

/* Atomic operations,the parameters must be volatile integer. */
/* Set a atomic integer to 0. */
__inline void __ATOMIC_INIT(__atomic_t* atomic)
{
	*atomic = 0;
}

/* Atomic increase. */
__inline void __ATOMIC_INCREASE(__atomic_t* atomic)
{
	__asm {
		push eax
		mov eax,atomic
		lock inc dword ptr [eax]
		pop eax
	}
}

/* Atomic decrease. */
__inline void __ATOMIC_DECREASE(__atomic_t* atomic)
{
	__asm {
		push eax
		mov eax,atomic
		lock dec dword ptr [eax]
		pop eax
	}
}

/* Add a value to atomic integer. */
__inline void __ATOMIC_ADD(__atomic_t* atomic, unsigned int toadd)
{
	__asm {
		push eax
		push ebx
		mov eax,toadd
		mov ebx,atomic
		lock add dword ptr [ebx],eax
		pop ebx
		pop eax
	}
}

/* Minus from atomic. */
__inline void __ATOMIC_MINUS(__atomic_t* atomic, unsigned int tominus)
{
	__asm {
		push eax
		push ebx
		mov eax, tominus
		mov ebx, atomic
		lock sub dword ptr [ebx], eax
		pop ebx
		pop eax
	}
}

/* Set the given value to atomic,and return the atomic's old value. */
__inline __atomic_t __ATOMIC_SET(__atomic_t* atomic, unsigned int new_val)
{
	volatile unsigned int old_val = 0;
	__asm {
		push eax
		push ebx
		mov eax,new_val
		mov ebx,atomic
		lock xchg dword ptr[ebx], eax
		mov old_val,eax
		pop ebx
		pop eax
	}
	return old_val;
}

//Atomic operations.
#define __INIT_ATOMIC(t)        do{t = 0;}while(0);
#define __INCREASE_ATOMIC(t)    do{t ++;}while(0);
#define __DECREASE_ATOMIC(t)    do{t --;}while(0);

#ifdef __cplusplus
}
#endif
#endif  //__SYN_MECH_H__
