//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : arch.h
//    Module Funciton           : 
//                                Common header for all architectures,arch
//                                specific definitions or routines should
//                                defined in arch specific header file,such
//                                as arch_x86.h.
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

#include <config.h>

/* Arch specific header files. */
#if defined(__I386__)
#include "arch_x86.h"
#endif

/* converts BCD code to decimal. */
#define BCD_TO_DEC_BYTE(bcd)   ((bcd >> 4) & 0xF) * 10 + ((bcd) & 0xF)

//Kernel thread wrapper routine.
typedef VOID (*__KERNEL_THREAD_WRAPPER)(__COMMON_OBJECT*);

/* 
 * Initializes the context of a kernel thread.
 * The initialization process is different on different platforms,so
 * implement this routine in ARCH directory.
 */
void InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
	__KERNEL_THREAD_WRAPPER lpStartAddr);

/* Initializes the context of a user thread. */
void InitUserThreadContext(__KERNEL_THREAD_OBJECT* pUserThread,
	__KERNEL_THREAD_WRAPPER pUserStartAddr);

//Enable VMM mechanism.
VOID EnableVMM(unsigned long* pdAddr);

//Halt the system in case of idle.
VOID HaltSystem();

/* Switch to the next thread. */
VOID __SwitchTo(__KERNEL_THREAD_OBJECT* pPrev, __KERNEL_THREAD_OBJECT* pNext);

/* Saves current kernel thread's context,and switches to the new one. */
VOID __SaveAndSwitch(__KERNEL_THREAD_OBJECT* pCurrent, __KERNEL_THREAD_OBJECT* pNext);

/*
 * Acknowledge interrupt from default interrupt controller,
 * it's 8259 PIC in PC.
 */
void __AckDefaultInterrupt();

//Get Time Stamp Counter of current CPU.
VOID __GetTsc(__U64*);

//Get time from CMOS of the PC.
VOID __GetTime(BYTE*);

//Microsecond level delay.
VOID __MicroDelay(DWORD dwmSeconds);

/* Return the frequency of CPU. */
uint64_t __GetCPUFrequency();

/* Initializes CPU specific task management function. */
BOOL __InitCPUTask();

/* Processor specified exception handler. */
VOID PSExcepHandler(LPVOID pESP, UCHAR ucVector);

/* Move a memory block from user space to kernel space. */
BOOL __copy_from_user(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, LPVOID pKernelStart, LPVOID pUserStart,
	unsigned long length);
/* Move a memory block from kernel space to user space. */
BOOL __copy_to_user(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, LPVOID pKernelStart, LPVOID pUserStart,
	unsigned long length);

/* 
 * Read or write to device maped space. 
 * They should be common in all architectures.
 */
#define __readb(addr) (*(volatile unsigned char*)(addr))
#define __readw(addr) (*(volatile unsigned short*)(addr))
#define __readl(addr) (*(volatile unsigned long*)(addr))
#define __writeb(v,addr) (*(volatile unsigned char*)(addr)  = (unsigned char)(v))
#define __writew(v,addr) (*(volatile unsigned short*)(addr) = (unsigned short)(v))
#define __writel(v,addr) (*(volatile unsigned long*)(addr)  = (unsigned long)(v))

#endif  //__ARCH_H__
