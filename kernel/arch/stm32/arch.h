//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : ARCH_X86.H
//    Module Funciton           : 
//                                This module countains CPU specific code,in this file,
//                                Intel X86 series CPU's specific code is included.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ARCH_H__
#define __ARCH_H__

//Kernel thread wrapper routine.
typedef VOID (*__KERNEL_THREAD_WRAPPER)(__COMMON_OBJECT*);

//
//Initializes the context of a kernel thread.
//The initialization process is different on different platforms,so
//implement this routine in ARCH directory.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr);

//Enable VMM mechanism.
VOID EnableVMM(void);

//Switch to a new kernel thread from interrupt context.
VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext);

//Saves current kernel thread's context,and switches to the new one.
VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
					 __KERNEL_THREAD_CONTEXT** lppNewContext);

//Get Time Stamp Counter of current CPU.
VOID __GetTsc(__U64*);

//Microsecond level delay.
VOID __MicroDelay(DWORD dwmSeconds);

//Simulate a NOP instruction.
#ifdef __I386__
#define NOP() \
__asm{    \
	nop   \
}
#else
#define NOP()
#endif

//Port operating routines for x86 architecture.
DWORD __ind(WORD wPort);
VOID __outd(WORD wPort,DWORD dwVal);
UCHAR __inb(WORD wPort);
VOID __outb(UCHAR ucVal,WORD wPort);
WORD __inw(WORD wPort);
VOID __outw(WORD wVal,WORD wPort);
VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort);
VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort);

#endif  //__ARCH_H__
