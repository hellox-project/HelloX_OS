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

//Constants for x86 CPU.
#define DEFAULT_CACHE_LINE_SIZE 32
#define ARCH_DMA_MINALIGN DEFAULT_CACHE_LINE_SIZE

//A macro converting BCD code to decimal.
#define BCD_TO_DEC_BYTE(bcd)   ((bcd>>4)&0xF)*10+((bcd)&0xF)

//Unaligned memory accessing.The following macros must be adapted according CPU
//architecture,since it may lead exception when implemented unproper.
//We just get and put value as aligned memory accessing since x86 support unaligned
//memory accessing directly.
#define __GET_UNALIGNED(ptr) (*ptr)
#define __GET_UNALIGNED_I16(ptr) (*ptr)
#define __GET_UNALIGNED_I32(ptr) (*ptr)
#define __GET_UNALIGNED_I64(ptr) (*ptr)
#define __GET_UNALIGNED_U16(ptr) (*ptr)
#define __GET_UNALIGNED_U32(ptr) (*ptr)
#define __GET_UNALIGNED_U64(ptr) (*ptr)

#define __PUT_UNALIGNED(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_I16(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_I32(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_I64(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_U16(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_U32(val,ptr) (*ptr = val)
#define __PUT_UNALIGNED_U64(val,ptr) (*ptr = val)

//Kernel thread wrapper routine.
typedef VOID (*__KERNEL_THREAD_WRAPPER)(__COMMON_OBJECT*);

//Processor specified exception handler,for x86.
VOID PSExcepHandler(LPVOID pESP, UCHAR ucVector);

//
//Initializes the context of a kernel thread.
//The initialization process is different on different platforms,so
//implement this routine in ARCH directory.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr);

//Enable VMM mechanism.
VOID EnableVMM(void);

//Halt the system in case of idle.
VOID HaltSystem();

//Switch to a new kernel thread from interrupt context.
VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext);

//Saves current kernel thread's context,and switches to the new one.
VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
					 __KERNEL_THREAD_CONTEXT** lppNewContext);

//Get Time Stamp Counter of current CPU.
VOID __GetTsc(__U64*);

//Get time from CMOS of the PC.
VOID __GetTime(BYTE*);

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

//Read or write to device maped space,in some architecture,it may
//different compare to normal memory access.
#define __readb(addr) (*(volatile unsigned char*)(addr))
#define __readw(addr) (*(volatile unsigned short*)(addr))
#define __readl(addr) (*(volatile unsigned long*)(addr))
#define __writeb(v,addr) (*(volatile unsigned char*)(addr)  = (unsigned char)(v))
#define __writew(v,addr) (*(volatile unsigned short*)(addr) = (unsigned short)(v))
#define __writel(v,addr) (*(volatile unsigned long*)(addr)  = (unsigned long)(v))

#endif  //__ARCH_H__
