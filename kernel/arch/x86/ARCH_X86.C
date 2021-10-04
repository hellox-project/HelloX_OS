//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,18 2006
//    Module Name               : ARCH_X86.CPP
//    Module Funciton           : 
//                                This module countains CPU specific code,in this file,
//                                Intel X86 series CPU's specific code is included.
//
//    Last modified Author      : Tywind
//    Last modified Date        : 30 JAN,2015
//    Last modified Content     :
//                                __GetTime routine is added,used to read system date and
//                                time from CMOS.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <mlayout.h>
#include <process.h>
#include <arch.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/utsname.h>

#include "pic.h"
#if defined(__CFG_SYS_SMP)
#include "acpi.h"
#include "smpx86.h"
#endif

#ifdef __I386__  //Only available in x86 based PC platform.

//8253 timer registers and constants.
#define IO_TIMER1        0x040
#define TIMER_FREQ       1193182
#define TIMER_CNTR       (IO_TIMER1 + 0)
#define TIMER_MODE       (IO_TIMER1 + 3)
#define TIMER_SEL0       0x00
#define TIMER_TCOUNT     0x00
#define TIMER_16BIT      0x30
#define TIMER_STAT       0xE0
#define TIMER_STAT0      (TIMER_STAT | 0x02)

//System clock frequency,round to up.
#define __HZ ((1000 + SYSTEM_TIME_SLICE - 1)/ SYSTEM_TIME_SLICE)

//Latch of 8253 timer's first slot,for system clock tick.
#define __LATCH ((TIMER_FREQ + __HZ / 2) / __HZ)

#if defined(__CFG_SYS_SMP)
/* Spin lock to protect the initialization of all CPU. */
static __SPIN_LOCK init_spinlock = SPIN_LOCK_INIT_VALUE;
#endif

/* CPU frequency. */
static uint64_t cpuFrequency = 0;

//A helper routine to convert __U64 to uint64_t.
static uint64_t __local_rdtsc()
{
	__U64 __tsc;
	uint64_t tsc;
	__GetTsc(&__tsc);
	tsc = __tsc.dwHighPart;
	tsc <<= 32;
	tsc += __tsc.dwLowPart;
	return tsc;
}

//Initialization of CPU frequency.
static void Frequency_Init(void)
{
	uint64_t xticks = 0x000000000000FFFF;
	uint64_t tsc = 0, s = 0, e = 0;
#if defined(__CFG_SYS_SMP)
	unsigned long ulFlags;
#endif

	/* Serialize the following code in case of SMP. */
#if defined(__CFG_SYS_SMP)
	__ENTER_CRITICAL_SECTION_SMP(init_spinlock, ulFlags);
#endif
	__outb(TIMER_SEL0 | TIMER_TCOUNT | TIMER_16BIT, TIMER_MODE);
	__outb((UCHAR)(xticks % 256), IO_TIMER1);
	__outb((UCHAR)(xticks / 256), IO_TIMER1);

	s = __local_rdtsc();
	do{
		__outb(TIMER_STAT0, TIMER_MODE);
		if (__local_rdtsc() - s >= 1ULL << 32)
		{
			_hx_printk("Warning: 8253 timer may unavailable,assume the CUP hz as 2G.\r\n");
			cpuFrequency = 2 * 1000 * 1000 * 1000;
#if defined(__CFG_SYS_SMP)
			__LEAVE_CRITICAL_SECTION_SMP(init_spinlock, ulFlags);
#endif
			return;
		}
	} while (!(__inb(TIMER_CNTR) & 0x80));
#if defined(__CFG_SYS_SMP)
	__LEAVE_CRITICAL_SECTION_SMP(init_spinlock, ulFlags);
#endif

	e = __local_rdtsc();
	cpuFrequency = ((e - s) * 10000000) / ((xticks * 10000000) / TIMER_FREQ);
	_hx_printk("CPU frequency: %u Hz.\r\n", (DWORD)cpuFrequency);
}

//Initialization of 8253 timer for system clock.
static void Init_Sys_Clock()
{
	__outb(0x34, 0x43);
	__outb((UCHAR)(((DWORD)__LATCH) & 0xFF), 0x40);
	__outb((UCHAR)(((DWORD)__LATCH) >> 8), 0x40);
}

/* Move a memory block from user space to kernel space. */
BOOL __copy_from_user(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, LPVOID pKernelStart, LPVOID pUserStart,
	unsigned long length)
{
	unsigned long __pdr = 0;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == pVmmMgr);
	BUG_ON(length > PAGE_SIZE);

	/* The operation must not be interupted. */
	__DISABLE_LOCAL_INTERRUPT(ulFlags);
	/* Use the specified VMM's pdr. */
	__pdr = (unsigned long)pVmmMgr->lpPageIndexMgr->lpPdAddress;
	__asm {
		push eax
		push ebx
		mov ebx, cr3
		mov eax, __pdr
		mov __pdr, ebx //Save old pdr.
		mov cr3, eax
		pop ebx
		pop eax
	}
	memcpy(pKernelStart, pUserStart, length);
	/* Restore the previous PDR. */
	__asm {
		push eax
		mov eax, __pdr
		mov cr3, eax
		pop eax
	}
	__RESTORE_LOCAL_INTERRUPT(ulFlags);
	return TRUE;
}

/* Move a memory block from kernel space to user space. */
BOOL __copy_to_user(__VIRTUAL_MEMORY_MANAGER* pVmmMgr, LPVOID pKernelStart, LPVOID pUserStart,
	unsigned long length)
{
	unsigned long __pdr = 0;
	unsigned long ulFlags = 0;

	BUG_ON(NULL == pVmmMgr);
	BUG_ON(length > PAGE_SIZE);

	/* The operation must not be interupted. */
	__DISABLE_LOCAL_INTERRUPT(ulFlags);
	/* Use the specified VMM's pdr. */
	__pdr = (unsigned long)pVmmMgr->lpPageIndexMgr->lpPdAddress;
	__asm {
		push eax
		push ebx
		mov ebx, cr3
		mov eax, __pdr
		mov __pdr, ebx //Save old pdr.
		mov cr3, eax
		pop ebx
		pop eax
	}
	memcpy(pUserStart, pKernelStart, length);
	/* Restore the previous PDR. */
	__asm {
		push eax
		mov eax, __pdr
		mov cr3, eax
		pop eax
	}
	__RESTORE_LOCAL_INTERRUPT(ulFlags);
	return TRUE;
}

/* Helper routine to construct a GDT entry. */
static void __SetGDTEntry(__GDT_ENTRY* pEntry, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	pEntry->base_low = (base & 0xFFFF);
	pEntry->base_middle = (base >> 16) & 0xFF;
	pEntry->base_high = (base >> 24) & 0xFF;
	pEntry->limit_low = (limit & 0xFFFF);
	pEntry->granularity = (limit >> 16) & 0x0F;
	pEntry->granularity |= gran & 0xF0;
	pEntry->access = access;
}

/* 
 * Helper routine to set a TSS into x86's GDT. 
 * It just delegate the operation to a routine in 
 * miniker.
 */
typedef BOOL (__cdecl *__SET_TSS_INTO_GDT)(int index, uint32_t low_dword, uint32_t high_dword);

BOOL __cdecl SetTSSToGDT(int index, uint32_t low_dword, uint32_t high_dword)
{
	__SET_TSS_INTO_GDT __set_tss = *((__SET_TSS_INTO_GDT*)__SET_GDT_ENTRY_BASE);
	BUG_ON(index >= MAX_CPU_NUM);
	return __set_tss(index, low_dword, high_dword);
}

/* Helper routine to load TR register. */
static __load_tr(int tss_index)
{
	int tr_val = tss_index * 8;
	tr_val += 10 * 8; /* 10 GDT entries above TSS. */
	tr_val |= 3; /* Set RPL as 3. */
	__asm {
		xor eax,eax
		mov eax,tr_val
		ltr ax
	}
}

/*
 * Initializes x86 specific task management function.
 * It creates a TSS for current processor and save it into
 * processor specific data structure,and set the TSS into
 * GDT entry corresponding current processor.
 * Then TR register is loaded.
 */
BOOL __InitCPUTask()
{
	__X86_TSS* pTss = NULL;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
	BOOL bResult = FALSE;
	uint32_t base = 0, limit = 0;
	__GDT_ENTRY Entry;

	/* Allocate TSS for current processor. */
	pTss = _hx_aligned_malloc(sizeof(__X86_TSS), DEFAULT_CACHE_LINE_SIZE);
	if (NULL == pTss)
	{
		goto __TERMINAL;
	}
	memset(pTss, 0, sizeof(__X86_TSS));
	/* Initializes the TSS. */
	pTss->ss0 = 0x10;
	/* Kernel code segment with RPL set to 3. */
	pTss->cs = 0x0b;
	pTss->ds = 0x13;
	pTss->es = 0x13;
	pTss->fs = 0x13;
	pTss->gs = 0x13;
	pTss->ss = 0x13;

	/* Save the TSS into processor's specific structure. */
	pSpec = (__LOGICALCPU_SPECIFIC*)ProcessorManager.GetCurrentProcessorSpecific();
	BUG_ON(NULL == pSpec);
	pSpec->vendorSpec = pTss;

	/* Save the TSS into GDT. */
	base = (uint32_t)pTss;
	limit = base + sizeof(__X86_TSS);
	__SetGDTEntry(&Entry, base, limit, 0xE9, 0x00);
	base = *(uint32_t*)&Entry;
	limit = *((uint32_t*)&Entry + 1);
	bResult = SetTSSToGDT(__CURRENT_PROCESSOR_ID, base, limit);
	BUG_ON(!bResult);

	/* Load TR register. */
	__load_tr(__CURRENT_PROCESSOR_ID);

	/* Init task successfully. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Architecture related initialization code,
 * this routine will be called in the
 * begining of system initialization.
 */
BOOL __HardwareInitialize()
{
	BOOL bResult = FALSE;

	/* Mask all maskable interrupts. */
	__Mask_All();

	/* Obtain CPU frequency and initialize system clock. */
	Frequency_Init();
	Init_Sys_Clock();

	/* Do SMP related initializations. */
#if defined(__CFG_SYS_SMP)
	/* Initialize ACPI subsystem. */
	if (!ACPI_Init())
	{
		goto __TERMINAL;
	}
#endif //__CFG_SYS_SMP

	bResult = TRUE;
#if defined(__CFG_SYS_SMP)
__TERMINAL:
#endif
	return bResult;
}

#if defined(__CFG_SYS_SMP)
/* 
 * Called by application processor in SMP to start the AP's initialization. 
 * It's wrapped by BeginAPInitialize of system object.
 */
BOOL __BeginAPInitialize()
{
	BOOL bResult = FALSE;

	/* Initialize local APIC. */
	if (!Init_LocalAPIC())
	{
		goto __TERMINAL;
	}

	/* Initialization process is OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}
#endif

/* 
 * Called after the OS kernel is finish the initialization phase. 
 * After this routine,the system is in running state.
 * Initialize IOAPIC and local APIC,start all APs,in SMP environment,
 * should be achieved in this routine.
 */
BOOL __EndHardwareInitialize()
{
	BOOL bResult = FALSE;

#if defined(__CFG_SYS_SMP)
	/* Initialize IOAPIC and local APIC. */
	if (!Init_IOAPIC())
	{
		goto __TERMINAL;
	}
	if (!Init_LocalAPIC())
	{
		goto __TERMINAL;
	}
	/* Start all APs. */
	if (!Start_AP())
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
#else
	bResult = TRUE;
#endif //__CFG_SYS_SMP
	return bResult;
}

//Exception table.
typedef struct{
	char*  description;
	BOOL   bErrorCode;
}__EXCEPTION_TABLE;

#define MAX_EXCEP_TABLE_SIZE 20  //Maximal exceptions available under current processor.

static __EXCEPTION_TABLE __ExcepTable[MAX_EXCEP_TABLE_SIZE] = {
	{ "Divide Error(#DE)", FALSE },
	{ "Reserved(#DB)", FALSE },
	{ "NMI Interrupt", FALSE },
	{ "Breakpoint(#BP)", FALSE },
	{ "Overflow(#OF)", FALSE },
	{ "Bound Range Exceed(#BR)", FALSE },
	{ "Invalid Opcode(#UD)", FALSE },
	{ "Device Not Available(#NM)", FALSE },
	{ "Double Fault(#DF)", TRUE },
	{ "Coprocessor Segment Overrun", FALSE },
	{ "Invalid TSS(#TS)", TRUE },
	{ "Segment Not Present(#NP)", TRUE },
	{ "Stack Segment Fault(#SS)", TRUE },
	{ "General Protection(#GP)", TRUE },
	{ "Page Fault(#PF)", TRUE },
	{ "Internal reserved", FALSE },
	{ "x87 Floating point error(#MF)", FALSE },
	{ "Alignment check(#AC)", TRUE },
	{ "Machine Check(#MC)", FALSE },
	{ "SIMD Floating-Point Exception(#XM)", FALSE }
};

//Exception specific operations.
static VOID ExcepSpecificOps(LPVOID pESP, UCHAR ucVector)
{
	DWORD excepAddr;

	/* Show exception address if page fault. */
	if (14 == ucVector)
	{
#ifdef __GCC__
		__asm__ __volatile__(
			".code32            \n\t"
			"pushl       %%eax     \n\t"
			"movl        %%cr2,     %%eax     \n\t"
			"movl        %%eax, %0                \n\t"
			"popl        %%eax                       \n\t"
			:"=g"(excepAddr) :  : "memory");
#else
		__asm{
			push eax
				mov eax, cr2
				mov excepAddr, eax
				pop eax
		}
#endif
		_hx_printf("  Exception addr: 0x%X.\r\n", excepAddr);
	}
}

/* Exception handler,for x86. */
void PSExcepHandler(LPVOID pESP, UCHAR ucVector)
{
	if (ucVector >= MAX_EXCEP_TABLE_SIZE)
	{
		/* Unknown exception. */
		_hx_printf("  Invalid exception number(#%d) for x86.\r\n", ucVector);
		return;
	}
	/* Show general information about the exception. */
	_hx_printf("  Exception Desc: %s.\r\n", __ExcepTable[ucVector].description);
	if (__ExcepTable[ucVector].bErrorCode)
	{
		/* Error code is pushed into stack by cpu. */
		_hx_printf("  Error Code: 0x%X.\r\n", *((DWORD*)pESP + 7));
		_hx_printf("  EIP: 0x%X.\r\n", *((DWORD*)pESP + 8));
		_hx_printf("  CS: 0x%X.\r\n", *((DWORD*)pESP + 9));
		_hx_printf("  EFlags: 0x%X.\r\n", *((DWORD*)pESP + 10));
	}
	else
	{
		/* No error code specified. */
		_hx_printf("  EIP: 0x%X.\r\n", *((DWORD*)pESP + 7));
		_hx_printf("  CS: 0x%X.\r\n", *((DWORD*)pESP + 8));
		_hx_printf("  EFlags: 0x%X.\r\n", *((DWORD*)pESP + 9));
	}

	/* Call exception specific handler. */
	ExcepSpecificOps(pESP, ucVector);
	return;
}

/*
 * Switch to a kernel thread,it just reload the general
 * purpose registers from target kernel's stack,and switch
 * to it using iret instruction.
 * The content of CS/DS/ES/FS/GS/SS registers are unchanged.
 */
#ifndef __GCC__
__declspec(naked)
#endif
static void __switchto_kernel(__KERNEL_THREAD_CONTEXT* lpContext)
{
#ifdef __GCC__
	__asm__ (
	".code32						\n\t "
	"pushl 	%%ebp					\n\t"
	"movl	%%esp,	%%ebp			\n\t"
	"movl	0x08(%%ebp),	%%esp	\n\t"
	"popl	%%ebp	\n\t"
	"popl	%%edi	\n\t"
	"popl	%%esi	\n\t"
	"popl	%%edx	\n\t"
	"popl	%%ecx	\n\t"
	"popl	%%ebx	\n\t"
	"popl	%%eax			\n\t"
	"iret					\n\t"
	:	:
	);
#else
	__asm{
		push ebp
		mov ebp,esp
		mov esp,dword ptr [ebp + 0x08]  //Restore ESP.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
#endif
}

/*
* Switch to a user thread.
* The content of CS/DS/ES/FS/GS/SS registers are changed
* to user,comparing to the __switchto_kernel routine.
*/
__declspec(naked) static void __switchto_user(__KERNEL_THREAD_CONTEXT* lpContext)
{
	__asm {
		push ebp
		mov ebp, esp
		mov esp, dword ptr[ebp + 0x08]  //Restore ESP.
		mov ax,0x4b
		mov ds,ax
		mov es,ax
		mov fs,ax
		mov gs,ax
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
}

/* 
 * A helper routine to reload the page directory base register 
 * from the target thread(process)'s page directory base value.
 * It will be called in process of context switching if the
 * target thread is a user one.
 */
static void __switch_pdr(unsigned long __pdr)
{
	/* Set it into CR3 register. */
	__asm {
		push eax
		mov eax, __pdr
		mov cr3, eax
		pop eax
	}
}

/*
 * Context switching routine,will be invoked in 
 * interrupt handler in most case,such as ScheduleFromInt
 * routine.
 * It restores the TSS and CR3 register if the next
 * thread is a user thread.
 */
void __SwitchTo(__KERNEL_THREAD_OBJECT* pPrev, __KERNEL_THREAD_OBJECT* pNext)
{
	__PROCESS_OBJECT* pProcess = NULL;
	__VIRTUAL_MEMORY_MANAGER* pvmmgr = NULL;
	__PAGE_INDEX_MANAGER* pPageIdxMgr = NULL;
	unsigned long __pdr = 0;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
	__X86_TSS* pTss = NULL;

	BUG_ON(NULL == pNext);
	if (IS_USER_THREAD(pNext))
	{
		/* 
		 * This thread is a user thread,has it's own memory
		 * space,so we should use it by reloading the CR3
		 * register.
		 */
		pProcess = (__PROCESS_OBJECT*)pNext->lpOwnProcess;
		BUG_ON(NULL == pProcess);
		pvmmgr = pProcess->pMemMgr;
		BUG_ON(NULL == pvmmgr);
		pPageIdxMgr = pvmmgr->lpPageIndexMgr;
		BUG_ON(NULL == pPageIdxMgr);
		/* Get the page directory base address from page index manager. */
		__pdr = (unsigned long)pPageIdxMgr->lpPdAddress;
		__switch_pdr(__pdr);

		/*
		 * Change kernel stack of current processor's TSS
		 * to this thread's kernel stack.
		 * This must be done even switch to kernel mode,
		 * issue raised(system crash) when this was not
		 * carried out before BUG FIEXED.
		 */
		pSpec = ProcessorManager.GetCurrentProcessorSpecific();
		BUG_ON(NULL == pSpec);
		pTss = (__X86_TSS*)pSpec->vendorSpec;
		BUG_ON(NULL == pTss);
		pTss->esp0 = (uint32_t)pNext->lpInitStackPointer;

		if (THREAD_IN_USER_MODE(pNext))
		{
			/*
			 * The thread may trap into kernel for handling
			 * interrupt,so we must switch to user space.
			 */
			__switchto_user(pNext->lpKernelThreadContext);
		}
		else
		{
			/* The thread is in interrim of system call. */
			__switchto_kernel(pNext->lpKernelThreadContext);
		}
	}
	else
	{
		/* 
		 * It's a kernel thread,so we use the system
		 * memory space to reload the CR3. 
		 * Please be noted the kernel threads can not see
		 * user thread's user part memory space,since
		 * the virtual memory allocated in user space only
		 * reflected in user thread(process)'s page table,
		 * access memory in user space by kernel thread may
		 * raise exception(PAGE FAULT).
		 */
		pPageIdxMgr = lpVirtualMemoryMgr->lpPageIndexMgr;
		__pdr = (unsigned long)pPageIdxMgr->lpPdAddress;
		__switch_pdr(__pdr);
		__switchto_kernel(pNext->lpKernelThreadContext);
	}
}

/* Acknowledge the default interrupt controller,which is PIC 8259. */
void __AckDefaultInterrupt()
{
	__asm {
		push eax
		mov al, 0x20  //Dismiss interrupt controller.
		out 0x20, al
		out 0xa0, al
		pop eax
	}
}

/*
 * Context switching routine for x86 architecture.
 * Saves the current kernel thread's stack top pointer into context pointer,
 * and switchs to the new kernel thread by calling __SwitchTo routine in
 * assembly language.
 * The three variables,are used as temporary space to store registers of
 * current kernel thread in process of saving context.
 */
static __declspec(naked) VOID __cdecl __local_SaveAndSwitch(
	__KERNEL_THREAD_CONTEXT** lppOldContext,
	__KERNEL_THREAD_OBJECT* pNextThread,
	unsigned long _t_ebp,
	unsigned long _t_eip,
	unsigned long _t_eax)
{
	__asm {
		/* Save current EBP into _t_ebp. */
		mov dword ptr[esp + 12], ebp
		/* Establish the stack frame. */
		mov ebp, esp
		/* Save current EAX register into _t_eax. */
		mov dword ptr[ebp + 20], eax
		/* Save the return address(EIP) into _t_eip. */
		pop dword ptr[ebp + 16]
		/* Save flags register. */
		pushfd
		/* Save CS. */
		xor eax, eax
		mov ax, cs
		push eax
		/* Store return address into stack. */
		push dword ptr[ebp + 16]
		/* Store EAX into stack. */
		push dword ptr[ebp + 20]
		/* Save all other registers except EBP. */
		push ebx
		push ecx
		push edx
		push esi
		push edi
		/* Save the original EBP register. */
		push dword ptr[ebp + 12]

		/*
		* Current kernel thread's stack frame built over,then
		* save the stack top pointer into kernel thread's context
		* pointer.
		* We can use any registers since all of them are saved.
		*/
		mov ebx, dword ptr[ebp + 0x04]
		mov dword ptr[ebx], esp

		/* Call the __SwitchTo routine to switch to the new thread. */
		mov ebx, dword ptr[ebp + 0x08]
		push ebx
		xor ebx,ebx
		push ebx
		call __SwitchTo
	}
}

#if 0
static __declspec(naked) VOID __cdecl __local_SaveAndSwitch(
	__KERNEL_THREAD_CONTEXT** lppOldContext,
	__KERNEL_THREAD_CONTEXT** lppNewContext,
	unsigned long _t_ebp,
	unsigned long _t_eip,
	unsigned long _t_eax)
{
	__asm {
		/* Save current EBP into _t_ebp. */
		mov dword ptr [esp + 12], ebp
		/* Establish the stack frame. */
		mov ebp, esp
		/* Save current EAX register into _t_eax. */
		mov dword ptr [ebp + 20], eax
		/* Save the return address(EIP) into _t_eip. */
		pop dword ptr [ebp + 16]
		/* Save flags register. */
		pushfd
		/* Save CS. */
		xor eax, eax
		mov ax, cs
		push eax
		/* Store return address into stack. */
		push dword ptr [ebp + 16]
		/* Store EAX into stack. */
		push dword ptr [ebp + 20]
		/* Save all other registers except EBP. */
		push ebx
		push ecx
		push edx
		push esi
		push edi
		/* Save the original EBP register. */
		push dword ptr [ebp + 12]

		/*
		 * Current kernel thread's stack frame built over,then
		 * save the stack top pointer into kernel thread's context
		 * pointer.
		 * We can use any registers since all of them are saved.
		 */
		 mov ebx, dword ptr [ebp + 0x04]
		 mov dword ptr [ebx], esp

		 /* Restore the new thread's context and switch to it. */
		 mov ebx, dword ptr[ebp + 0x08]
		 mov esp, dword ptr[ebx]  //Restore new stack.
		 pop ebp
		 pop edi
		 pop esi
		 pop edx
		 pop ecx
		 pop ebx
		 pop eax
		 iretd
	}
}
#endif

/* Context switching in process context. */
void __SaveAndSwitch(__KERNEL_THREAD_OBJECT* pCurrent, __KERNEL_THREAD_OBJECT* pNew)
{
	/* 
	 * Make room in local stack to be used by local SaveAndSwitch routine
	 * as temporary space to store current kernel thread's registers.
	 */
	unsigned long _t_ebp = 0;
	unsigned long _t_eax = 0;
	unsigned long _t_eip = 0;

	/* Commit context switching by invoke arch specific routine. */
	__local_SaveAndSwitch(&pCurrent->lpKernelThreadContext, pNew,
		_t_ebp, _t_eip, _t_eax);
}

/*
 * Old version of context switching routine in process context,it was
 * replaced by the above one since it can not support SMP.
 * Just for reference.
 */
#if 0
//
//These three global variables are used as temp variables
//by __SaveAndSwitch routine.
//
static DWORD dwTmpEip = 0;
static DWORD dwTmpEax = 0;
static DWORD dwTmpEbp = 0;

//
//This routine saves current kernel thread's context,and switch
//to the new kernel thread.
//
#ifndef __GCC__
__declspec(naked)
#endif
VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,__KERNEL_THREAD_CONTEXT** lppNewContext)
{
#ifdef __GCC__
	__asm__(
	".code32								\n\t"
	"movl	%%esp,	%0                  	\n\t"
	"popl	%1		                      	\n\t"
	"movl	%%eax,	%2   				\n\t"
	"pushf             					\n\t"
	"xorl	%%eax,	%%eax     				\n\t"
	"movw	%%cs,	%%ax          				\n\t"
	"pushl	%%eax           				\n\t"
	"pushl	%3      						\n\t"
	"pushl  %4      						\n\t"
	"pushl %%ebx                          	\n\t"
	"pushl %%ecx                          	\n\t"
	"pushl %%edx                          	\n\t"
	"pushl %%esi                          	\n\t"
	"pushl %%edi                          	\n\t"
	"pushl %%ebp                          	\n\t"
	"             	                     	\n\t"
	"movl %5, %%ebp                  		\n\t"
	"movl 0x04(%%ebp),	%%ebx				\n\t"
	"movl %%esp,	(%%ebx)					\n\t"
	"                                     	\n\t"
	"movl 0x08(%%ebp),	%%ebx				\n\t"
	"movl (%%ebx),		%%esp   			  \n\t"
	"popl %%ebp                              \n\t"
	"popl %%edi                              \n\t"
	"popl %%esi                              \n\t"
	"popl %%edx                            \n\t"
	"popl %%ecx                 			\n\t"
	"popl %%ebx         					\n\t"
    "popl %%eax							\n\t"
    "iret"
	:"=m"(dwTmpEbp),"=m"(dwTmpEip),"=m"(dwTmpEax)
	:"m"(dwTmpEip),"m"(dwTmpEax),"m"(dwTmpEbp)
	);
#else
	__asm{
		mov dwTmpEbp,esp
		pop dwTmpEip
		mov dwTmpEax,eax //Save EAX.
		pushfd           //Save EFlags.
		xor eax,eax
		mov ax,cs
		push eax         //Save CS.
		push dwTmpEip    //Save EIP.
		push dwTmpEax    //Save EAX.
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp

		//Now,we have build the target stack frame,then save it.
		mov ebp,dwTmpEbp
		mov ebx,dword ptr [ebp + 0x04]
		mov dword ptr [ebx],esp  //Save old stack frame.

		//Restore the new thread's context and switch to it.
		mov ebx,dword ptr [ebp + 0x08]
		mov esp,dword ptr [ebx]  //Restore new stack.
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		iretd
	}
#endif
}
#endif

/*
 * Enable Virtual Memory Management mechanism.
 * This routine will be called in process of OS 
 * initialization if __CFG_SYS_VMM flag is defined.
 * The physical address of page directory will be
 * passed to this routine,as the initial value
 * of page directory register.
 */
VOID EnableVMM(unsigned long* pdAddr)
{
	__asm{
		push eax
		mov eax,pdAddr
		mov cr3,eax
		mov eax,cr0
		or eax,0x80000000
		mov cr0,eax
		pop eax
	}
}

//Halt current CPU in case of IDLE,it will be called by IDLE thread.
VOID HaltSystem()
{
#ifdef __GCC__
	__asm__ __volatile__ ("hlt	\n\t");
#else
	__asm{
		hlt
	}
#endif
}

//Define a macro to make the code readable.
#define __PUSH(stackptr,val) \
	do{  \
	(DWORD*)(stackptr) --; \
	*((DWORD*)stackptr) = (DWORD)(val); \
	}while(0)

/*
 * Initializes a kernel thread's context.
 */
void InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
	__KERNEL_THREAD_WRAPPER lpStartAddr)
{
	unsigned long* lpStackPtr = NULL;
	unsigned long dwStackSize = 0;
	char* pUserStack = NULL;

	BUG_ON((NULL == lpKernelThread) || (NULL == lpStartAddr));
	BUG_ON(IS_USER_THREAD(lpKernelThread));

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;
	__PUSH(lpStackPtr, lpKernelThread);       //Push lpKernelThread to stack.
	__PUSH(lpStackPtr, NULL);                 //Push a new return address,simulate a call.
	__PUSH(lpStackPtr, INIT_EFLAGS_VALUE);    //Push EFlags.
	__PUSH(lpStackPtr, 0x00000008);           //Push CS.
	__PUSH(lpStackPtr, lpStartAddr);          //Push start address.
	__PUSH(lpStackPtr, 0);                    //Push general purpose registers.
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

/* Initializes a user thread's context. */
void InitUserThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
	__KERNEL_THREAD_WRAPPER lpStartAddr)
{
	unsigned long* lpStackPtr = NULL;
	unsigned long dwStackSize = 0;
	char* pUserStack = NULL;

	BUG_ON((NULL == lpKernelThread) || (NULL == lpStartAddr));
	BUG_ON(IS_KERNEL_THREAD(lpKernelThread));

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;
	pUserStack = (char*)lpKernelThread->pUserStack;
	pUserStack += lpKernelThread->user_stk_size;
	/* 
	 * One page of memory in user stack is used as 
	 * parameter(s) or environment information that 
	 * transfer to user agent or user application.
	 * This block of memory is initialized by PrepareUserStack
	 * routine in process module if the user thread is the
	 * main thread of own process.
	 */
	pUserStack -= PAGE_SIZE;

	__PUSH(lpStackPtr, 0x0000004B);           //User SS.
	__PUSH(lpStackPtr, pUserStack);           //User stack top ptr.
	__PUSH(lpStackPtr, INIT_EFLAGS_VALUE);    //EFlags.
	__PUSH(lpStackPtr, 0x00000043);           //User CS.
	__PUSH(lpStackPtr, lpStartAddr);          //Push start address.
	__PUSH(lpStackPtr, 0);                    //Push general purpose registers.
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);
	__PUSH(lpStackPtr, 0);

	/* The thread should be in user mode when first switch to. */
	__ATOMIC_SET(&lpKernelThread->in_user, 1);

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

#undef __PUSH

//Return CPU frequency.
uint64_t __GetCPUFrequency()
{
	if (0 == cpuFrequency)  //Not initialized yet,assume CPU freq is 2G hz.
	{
		return 2ULL * 1024 * 1024 * 1024;
	}
	return cpuFrequency;
}

//Return CPU's time stamp counter,same as __GetTSC but use uint64_t build in data
//type as return value.
uint64_t __GetCPUTSC()
{
	return __local_rdtsc();
}

//Get time stamp counter.
VOID __GetTsc(__U64* lpResult)
{
#ifdef __GCC__
	__asm__(
		".code32		\n\t"
		"pushl	%%ebp	\n\t"
		"movl	%%esp,	%%ebp	\n\t"
		"pushl 	%%eax	\n\t"
		"pushl	%%edx	\n\t"
		"pushl	%%ebx	\n\t"
		"rdtsc			\n\t"
		"movl	0x08(%%ebp),	%%ebx	\n\t"
		"movl	%%eax,			(%%ebx)	\n\t"
		"movl	%%edx,		0x04(%%ebx)	\n\t"
		"popl	%%ebx	\n\t"
		"popl	%%edx	\n\t"
		"popl	%%eax	\n\t"
		"popl	%%ebp	\n\t"
		"ret			\n\t"
			::
	);
#else
	__asm{
		push eax
		push edx
		push ebx
		rdtsc    //Read time stamp counter.
		mov ebx,dword ptr [ebp + 0x08]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		pop ebx
		pop edx
		pop eax
	}
#endif
}

/*
 * Local helper routine to get date and time
 * information from CMOS.
 */
static
#if defined(__MS_VC__)
__declspec(naked)
#else
#endif
__ReadCMOSData(BYTE* pData, BYTE nPort)
{
#if defined(__MS_VC__)
	__asm{
		    push ebp
			mov ebp,esp
			push ebx
			push edx

			mov ebx, dword ptr [ebp + 0x08]
		    xor ax, ax
			mov al, nPort
			out 70h, al
			in al, 71h
			mov byte ptr [ebx], al

			pop edx
			pop ebx
			leave
			retn
	}
#else
	/* Reserve for other compilers. */
#endif
}

/* Get system level date and time. */
void __GetTime(BYTE* pDate)
{
	__ReadCMOSData(&pDate[0], 9);  //read year
	__ReadCMOSData(&pDate[1], 8);  //read month
	__ReadCMOSData(&pDate[2], 7);  //read day

	__ReadCMOSData(&pDate[3], 4);  //read hour
	__ReadCMOSData(&pDate[4], 2);  //read Minute
	__ReadCMOSData(&pDate[5], 0);  //read second

	/* Convert to DEC from BCD. */
	pDate[0] = BCD_TO_DEC_BYTE(pDate[0]);
	pDate[1] = BCD_TO_DEC_BYTE(pDate[1]);
	pDate[2] = BCD_TO_DEC_BYTE(pDate[2]);
	pDate[3] = BCD_TO_DEC_BYTE(pDate[3]);
	pDate[4] = BCD_TO_DEC_BYTE(pDate[4]);
	pDate[5] = BCD_TO_DEC_BYTE(pDate[5]);
}

//Micro second level delay.
static void __udelay(unsigned long usec)
{
	uint64_t delay = (cpuFrequency * usec) / 1000000;
	uint64_t base  = __local_rdtsc();

	while (__local_rdtsc() - base < delay)
	{
		//Do nothing but busy waiting...
	}
}

//The alias of __udelay.
VOID __MicroDelay(DWORD dwmSeconds)
{
	__udelay(dwmSeconds);
}

VOID __outd(WORD wPort,DWORD dwVal)  //Write one double word to a port.
{
#ifdef __GCC__
	asm volatile("outl %0,%1" : : "a" (dwVal), "dN" (wPort));
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		mov eax,dwVal
		out dx,eax
		pop edx
		pop eax
	}
#endif
}

DWORD __ind(WORD wPort)    //Read one double word from a port.
{
	DWORD    dwRet       = 0;
#ifdef __GCC__
	asm volatile("inl %1,%0" : "=a" (dwRet) : "dN" (wPort));
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		in eax,dx
		mov dwRet,eax
		pop edx
		pop eax
	}
#endif
	return dwRet;
}

VOID __outb(UCHAR _bt,WORD _port)  //Send bt to port.
{
#ifdef __GCC__
	asm volatile("outb %0,%1" : : "a" (_bt), "dN" (_port));
#else
	__asm{
		push eax
		push edx
		mov al,_bt
		mov dx,_port
		out dx,al
		pop edx
		pop eax
	}
#endif
}

UCHAR __inb(WORD _port)  //Receive a byte from port.
{
	UCHAR uRet;
#ifdef __GCC__
	asm volatile("inb %1,%0" : "=a" (uRet) : "dN" (_port));
#else
	__asm{
		xor eax,eax
		push edx
		mov dx,_port
		in al,dx
		pop edx
		mov uRet,al
	}
#endif
	return uRet;
}

WORD __inw(WORD wPort)
{
	WORD    wRet       = 0;
#ifdef __GCC__
	asm volatile("inw %1,%0" : "=a" (wRet) : "dN" (wPort));
#else
	__asm{
		push eax
		push edx
		mov dx,wPort
		in ax,dx
		mov wRet,ax
		pop edx
		pop eax
	}
#endif
	return wRet;
}

VOID __outw(WORD wVal,WORD wPort)
{
#ifdef __GCC__
	asm volatile("outw %0,%1" : : "a" (wVal), "dN" (wPort));
#else
	__asm{
		push eax
		push edx
		mov ax,wVal
		mov dx,wPort
		out dx,ax
		pop edx
		pop eax
	}
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif
VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__

#ifdef __GCC__
	__asm__ (
	"	.code32								\n\t"
	"	pushl 	%%ebp                                      \n\t"
	"	movl 	%%esp,	%%ebp                               \n\t"
	"	pushl %%ecx                                      \n\t"
	"	pushl %%edx                                      \n\t"
	"	pushl %%edi                                      \n\t"
	"	movl 	0x08(%%ebp),	%%edi				     \n\t"
	"	movl 	0x0c(%%ebp),	%%ecx						\n\t"
	"	shrl 	$0x01,			%%ecx                          \n\t"
	"	movw 	0x10(%%ebp),	%%dx				\n\t"
	"	cld                                        	\n\t"
	"	rep 	insw                                    \n\t"
	"	popl %%edi                                     \n\t"
	"	popl %%edx                                     \n\t"
	"	popl %%ecx                                     \n\t"
	"	leave                                      	\n\t"
	"	ret                                		\n\t"	//retn->ret
			:	:
	);

#else
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		cld
		rep insw
		pop edi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
#endif
}

#ifndef __GCC__
__declspec(naked)
#endif
VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
#ifdef __I386__
#ifdef __GCC__
	__asm__ (
	".code32			  				\n\t"
	" pushl %%ebp                        \n\t"
	" movl %%esp, %%ebp                  \n\t"
	" pushl %%ecx                           \n\t"
	" pushl %%edx                           \n\t"
	" pushl %%esi                           \n\t"
	" movw 0x10(%%ebp), %%dx			\n\t"
	" rep outsw                        	\n\t"
	" popl %%esi                            \n\t"
	" popl %%edx                            \n\t"
	" popl %%ecx                            \n\t"
	" leave                             \n\t"
	" ret			                	\n\t"	//retn
			::
	);

#else
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
#endif
#endif
}

/* uname routine in C lib,which is hardware specified. */
int uname(struct utsname* __name)
{
	memset(__name,sizeof(struct utsname),0);
	strcpy(__name->sysname,"HelloX OS");
	strcpy(__name->version,OS_VERSION);
	strcpy(__name->machine,"IA32");
	strcpy(__name->release,OS_VERSION);
	return 0;
}

#endif //__I386__
