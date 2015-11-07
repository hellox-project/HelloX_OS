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

#ifdef __I386__

#ifdef __GCC__
	#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags)	\
		__asm__ __volatile__("pushfl; popl %0 ; cli" : "=g" (dwFlags): /* no input */ :"memory");
#else

	#define __ENTER_CRITICAL_SECTION(lpObj, dwFlags) \
		__asm	push eax			 \
		__asm	pushfd               \
		__asm	pop eax              \
		__asm	mov dwFlags,eax      \
		__asm	pop eax              \
		__asm	cli
#endif

#else
#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags) \
	(dwFlags = 0);
#endif



#ifdef __I386__

#ifdef __GCC__
#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags)	\
	__asm__ __volatile__("pushl %0; popf " :	 : "g"(dwFlags): "memory")
#else
#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags) \
    __asm push dwFlags	\
	__asm popfd

#endif
#else

#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags)
	(dwFlags = 0);
#endif

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

#ifdef __I386__
	#ifdef __GCC__
		#define __DISABLE_INTERRUPT() {__asm__("cli" : : :"memory"); }
	#else
		#define __DISABLE_INTERRUPT() {__asm cli}
	#endif
#else
	#define __DISABLE_INTERRUPT()
#endif

//
//This macro is used to flush cache's content to memory,or just invalidate a range of
//cache content,according the flags parameter.
//Cache invalidating is not necessary under x86 platform,since the cache snooping is
//implemented in CPU,so we let it empty when the flags value is CACHE_FLUSH_INVALIDATE.
//
//Flags to control the cache flushing.
#define CACHE_FLUSH_WRITEBACK   0x00000001
#define CACHE_FLUSH_INVALIDATE  0x00000002
#ifdef __I386__
#ifdef __GCC__
#define __FLUSH_CACHE(addr,leng,flags)  {__asm__ ("wbinvd \n\t"::);}
#else
#define __FLUSH_CACHE(addr,leng,flags) \
    { \
        if(CACHE_FLUSH_WRITEBACK == flags) \
		        { \
            __asm wbinvd \
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

//
//This macros is used as a barrier.
//In some CPU,the write operations may not be committed into memory or cache,but 
//be queued in WRITE BUFFER on the CPU,so,in order to commit the write operation
//immediately,the following macro should be used.
//For example,the following code:
// ... ...
// WriteMemory(0xCFFFFFFF,90);
// WriteMemory(0xCFFFFFFC,90);
// ... ...
//0xCFFFFFFF and 0xCFFFFFFC are device mapped control port,in order to commit the
//writing operations into device immediately,the following macro must be called.
//
#ifdef __I386__
#ifdef __GCC__
#define __BARRIER() __asm__("LOCK addl $0, (%%esp) \n\t"::);
#else
#define __BARRIER() __asm LOCK add dword ptr [esp],0
#endif
#else
#define __BARRIER()
#endif

//Atomic operations.
#define __INIT_ATOMIC(t)        do{t = 0;}while(0);
#define __INCREASE_ATOMIC(t)    do{t ++;}while(0);
#define __DECREASE_ATOMIC(t)    do{t --;}while(0);

#ifdef __cplusplus
}
#endif

#endif    //__SYN_MECH_H__
