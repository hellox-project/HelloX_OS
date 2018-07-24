//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 7,2018
//    Module Name               : smpx86.c
//    Module Funciton           : 
//                                Symmentric Multiple Processor related source
//                                code,for x86 architecture.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "smpx86.h"

/* Only available under x86 architecture. */
#ifdef __I386__

/* Get CPU supported feature flags. */
uint32_t GetCPUFeature()
{
	uint32_t feature = 0;

	__asm {
		push eax
		push ebx
		push ecx
		push edx
		xor ecx, ecx
		mov eax, 0x01
		cpuid
		mov feature, edx
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	return feature;
}

/* Return current processor's ID. */
unsigned int __GetProcessorID()
{
	uint32_t __ebx = 0;

	/* Check if HTT feature is supported by current CPU. */
	if (0 == (GetCPUFeature() & CPU_FEATURE_MASK_HTT))
	{
		goto __TERMINAL;
	}

	/* 
	 * Fetch Initial local APIC ID from EBX,which will be filled 
	 * by CPUID instruction.
	 */
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		xor ecx, ecx
		mov eax, 0x01
		cpuid
		mov __ebx, ebx
		pop edx
		pop ecx
		pop ebx
		pop eax
	}

	/* 
	 * Use initial local APIC ID as processor ID,the 
	 * highest 8 bits(24~31) of EBX contains this value.
	 */
	__ebx >>= 24;
	__ebx &= 0xFF;

__TERMINAL:
	return __ebx;
}

/* Spin lock operations. */
void __raw_spin_lock(__SPIN_LOCK* sl)
{
	/* Use xchg instruction to guarantee the atomic operation. */
	__asm {
		push eax
		push ebx
		mov ebx,sl
	__TRY_AGAIN:
		mov eax,SPIN_LOCK_VALUE_LOCK
		lock xchg eax,dword ptr [ebx]
		test eax,eax
		jnz __TRY_AGAIN
		pop ebx
		pop eax
	}
}

void __raw_spin_unlock(__SPIN_LOCK* sl)
{
	/* 
	 * Just reset the spin lock to initial value. 
	 * Use xchg instruction instead of mov.
	 */
	__asm {
		push eax
		push ebx
		mov ebx,sl
		xor eax,eax
		lock xchg dword ptr [ebx],eax
		pop ebx
		pop eax
	}
}

/* Return the flags register and disable the interrupt,before acquire spin lock. */
unsigned long __smp_enter_critical_section(__SPIN_LOCK* sl)
{
	unsigned long dwFlags = 0;

	BUG_ON(NULL == sl);
	__asm {
		push eax
		pushfd
		pop eax
		mov dwFlags,eax
		pop eax
		cli
	}
	/* Acquire the spin lock. */
	__raw_spin_lock(sl);
	return dwFlags;
}

/* 
 * Restore the flags saved by previous routine. 
 * Spin lock will be released before that.
 */
unsigned long __smp_leave_critical_section(__SPIN_LOCK* sl, unsigned long dwFlags)
{
	unsigned long dwOldFlags = 0;

	BUG_ON(NULL == sl);
	__raw_spin_unlock(sl);
	/* Get the old value of eFlags before we restore it to dwFlags. */
	__asm{
		push eax
		pushfd
		pop eax
		mov dwOldFlags,eax
		pop eax
		push dwFlags
		popfd
	}
	return dwOldFlags;
}

/* Initialize the IOAPIC controller. */
BOOL Init_IOAPIC()
{
	__X86_CHIP_SPECIFIC* pSpec = NULL;
	uint32_t* pBase = NULL;

	/* 
	 * Obtain the chip specific information from ProcessorManager, then 
	 * reserve the IO memory of IOAPIC by calling VirtualAlloc,since paging
	 * is enabled when this routine is called.
	 */
	pSpec = ProcessorManager.GetChipSpecific(0, 0);
	BUG_ON(NULL == pSpec);
	pBase = (uint32_t*)VirtualAlloc(
		(LPVOID)pSpec->ioapic_base,
		0x100000, /* 1M memory space. */
		VIRTUAL_AREA_ALLOCATE_IO,
		VIRTUAL_AREA_ACCESS_RW,
		"IOAPIC");
	if (NULL == pBase)
	{
		_hx_printf("%s:failed to remap IOAPIC base.\r\n", __func__);
		return FALSE;
	}
	if (pBase != (LPVOID)pSpec->ioapic_base)
	{
		_hx_printf("%s:IO APIC base addr is occupied.\r\n", __func__);
		return FALSE;
	}
	/* Initialize the IOAPIC now. */
	_hx_printf("IOAPIC base:0x%X,value = 0x%X\r\n", pBase, __readl(pBase));
	return TRUE;
}

/* Initialize the local APIC controller,it will be invoked by each AP. */
BOOL Init_LocalAPIC()
{
	__X86_CHIP_SPECIFIC* pSpec = NULL;
	uint32_t* pBase = NULL;

	/*
	* Obtain the chip specific information from ProcessorManager, then
	* reserve the IO memory of local APIC by calling VirtualAlloc,since paging
	* is enabled when this routine is called.
	*/
	pSpec = ProcessorManager.GetChipSpecific(0, 0);
	BUG_ON(NULL == pSpec);
	pBase = (uint32_t*)VirtualAlloc(
		(LPVOID)pSpec->lapic_base,
		0x100000, /* 1M memory space. */
		VIRTUAL_AREA_ALLOCATE_IO,
		VIRTUAL_AREA_ACCESS_RW,
		"IOAPIC");
	if (NULL == pBase)
	{
		_hx_printf("%s:failed to remap local APIC base.\r\n", __func__);
		return FALSE;
	}
	if (pBase != (LPVOID)pSpec->lapic_base)
	{
		_hx_printf("%s:local APIC base addr is occupied.\r\n", __func__);
		return FALSE;
	}
	/* Initialize the IOAPIC now. */
	_hx_printf("Local APIC base:0x%X,value = 0x%X\r\n", pBase, __readl(pBase));
	return TRUE;
}

/* Start all application processors. */
BOOL Start_AP()
{
	return TRUE;
}

/* Stop all application processors. */
BOOL Stop_AP()
{
	return TRUE;
}

#endif //__I386__
