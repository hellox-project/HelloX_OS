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
#include "apic.h"

/* Only available under x86 architecture. */
#if (defined(__I386__) && defined(__CFG_SYS_SMP))

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

#if 0
	/* Check if HTT feature is supported by current CPU. */
	if (0 == (GetCPUFeature() & CPU_FEATURE_MASK_HTT))
	{
		goto __TERMINAL;
	}
#endif

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

//__TERMINAL:
	return __ebx;
}

/* 
 * Spin lock operations,wait on a spin lock by spinning. 
 * FALSE will be returned if wait time exceed the threshold,
 * this may lead by deadlock.
 */
BOOL __raw_spin_lock(__SPIN_LOCK* sl)
{
	unsigned long ulTimeout = SPIN_LOCK_DEBUG_MAX_SPINTIMES;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
	unsigned int apicID = 0;

	/* Use xchg instruction to guarantee the atomic operation. */
	__asm {
		push eax
		push ebx
		mov ebx,sl
	__TRY_AGAIN:
		dec ulTimeout;
		jz __FAIL_OUT;
		mov eax,SPIN_LOCK_VALUE_LOCK
		lock xchg eax,dword ptr [ebx]
		test eax,eax
		jnz __TRY_AGAIN
		pop ebx
		pop eax
	__FAIL_OUT:
	}
	if (0 == ulTimeout)
	{
		if (IN_SYSINITIALIZATION())
		{
			/* In process of system initialization. */
			_hx_printk("Spin lock time out,may lead by deadlock,curr CPU:[%d].\r\n",
				__CURRENT_PROCESSOR_ID);
		}
		else
		{
			_hx_printk("Spin lock time out,may lead by deadlock,curr thread:[%s],curr CPU:[%d].\r\n",
				__CURRENT_KERNEL_THREAD->KernelThreadName,
				__CURRENT_PROCESSOR_ID);
			if (IN_INTERRUPT())
			{
				/* Show interrupt information. */
				_hx_printk("Current interrupt vector:[%d]\r\n",	System.ucCurrInt[__CURRENT_PROCESSOR_ID]);
			}
		}
		/* Set CPU status as halted. */
		apicID = __CURRENT_PROCESSOR_ID;
		pSpec = ProcessorManager.GetLogicalCPUSpecific(
			0,
			__GetChipID(apicID),
			__GetCoreID(apicID),
			__GetLogicalCPUID(apicID));
		BUG_ON(NULL == pSpec);
		pSpec->cpuStatus = CPU_STATUS_HALTED;
		/* Halt the CPU. */
		while (TRUE)
		{
			HaltSystem();
		}
		return FALSE;
	}
	return TRUE;
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
		_hx_printk("%s:failed to remap IOAPIC base.\r\n", __func__);
		return FALSE;
	}
	if (pBase != (LPVOID)pSpec->ioapic_base)
	{
		_hx_printk("%s:IO APIC base addr is occupied.\r\n", __func__);
		return FALSE;
	}
	/* Initialize the IOAPIC now. */
	_hx_printk("IOAPIC base:0x%X,value = 0x%X\r\n", pBase, __readl(pBase));
	return TRUE;
}

/* Check if 2xAPIC is present in current CPU. */
static BOOL Isx2APIC()
{
	uint32_t features = 0;
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		xor ecx,ecx
		mov eax,0x01
		cpuid
		mov features,ecx
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	return (features & (1 << 21));
}

/* Just for debugging. */
//static __INTERRUPT_CONTROLLER* pAPICIntCtrl = NULL;

/* 
 * Initialize the local APIC controller,it will be invoked by BSP and each AP. 
 * IO mapped memory will be allocated by BSP and AP just use it.
 */
BOOL Init_LocalAPIC()
{
	__X86_CHIP_SPECIFIC* pSpec = NULL;
	uint8_t* pBase = NULL;
	__LOGICALCPU_SPECIFIC* plcpuSpec = NULL;
	unsigned int apicID = 0;
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;
	BOOL bResult = FALSE;

	/*
	* Obtain the chip specific information from ProcessorManager, then
	* reserve the IO memory of local APIC by calling VirtualAlloc,since paging
	* is enabled when this routine is called.
	*/
	pSpec = ProcessorManager.GetChipSpecific(0, 0);
	BUG_ON(NULL == pSpec);
	if (__CURRENT_PROCESSOR_IS_BSP())
	{
		/* Allocate IO mapped memory if invoked by BSP(and also is the first time). */
		pBase = (uint8_t*)VirtualAlloc(
			(LPVOID)pSpec->lapic_base,
			0x100000, /* 1M memory space. */
			VIRTUAL_AREA_ALLOCATE_IO,
			VIRTUAL_AREA_ACCESS_RW,
			"IOAPIC");
		if (NULL == pBase)
		{
			_hx_printk("%s:failed to remap local APIC base.\r\n", __func__);
			goto __TERMINAL;
		}
		if (pBase != (LPVOID)pSpec->lapic_base)
		{
			_hx_printk("%s:local APIC base addr is occupied.\r\n", __func__);
			goto __TERMINAL;
		}
	}
	else
	{
		/* Invoked by AP,just use the address. */
		pBase = (uint8_t*)pSpec->lapic_base;
	}

	/* Determine the APIC's type. */
	uint32_t apicType = APIC_TYPE_XAPIC;
	uint32_t apicVersion = 0;
	if (Isx2APIC())
	{
		apicType = APIC_TYPE_2XAPIC;
	}
	else
	{
		/* Check according to version number. */
		apicVersion = __readl(pBase + LAPIC_REGISTER_VERSION);
		apicVersion &= 0xFF;
		if (apicVersion < 0x10)
		{
			_hx_printk("Local APIC is too old[ver = %d]\r\n", apicVersion);
			goto __TERMINAL;
		}
		else
		{
			apicType = APIC_TYPE_XAPIC;
		}
	}

	/* Get APIC's ID,processor ID is same as APIC ID. */
	apicID = __CURRENT_PROCESSOR_ID;

	/* Create the corresponding interrupt controller object. */
	pIntCtrl = CreateInterruptController(pBase, apicID,
		APIC_TYPE_XAPIC,
		APIC_INT_VECTOR_START,
		APIC_INT_VECTOR_END);
	if (NULL == pIntCtrl)
	{
		goto __TERMINAL;
	}

#if 0
	/* 
	 * Just for debugging,will create one controller object for each 
	 * AP in the future.
	 */
	if (__CURRENT_PROCESSOR_IS_BSP())
	{
		pIntCtrl = CreateInterruptController(pBase, apicID,
			APIC_TYPE_XAPIC,
			APIC_INT_VECTOR_START,
			APIC_INT_VECTOR_END);
		if (NULL == pIntCtrl)
		{
			goto __TERMINAL;
		}
		pAPICIntCtrl = pIntCtrl;
	}
	else
	{
		BUG_ON(NULL == pAPICIntCtrl);
		pIntCtrl = pAPICIntCtrl;
	}
#endif

	/* Initialize the interrupt controller. */
	if (!pIntCtrl->Initialize(pIntCtrl))
	{
		goto __TERMINAL;
	}

	/* Save the controller object into logical CPU's specific information. */
	plcpuSpec = ProcessorManager.GetLogicalCPUSpecific(
		0,
		__GetChipID(apicID),
		__GetCoreID(apicID),
		__GetLogicalCPUID(apicID));
	if (NULL == plcpuSpec)
	{
		goto __TERMINAL;
	}
	plcpuSpec->pIntCtrl = pIntCtrl;

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Should release all resources. */
		if (pIntCtrl)
		{
			DestroyInterruptController(pIntCtrl);
		}
		if (plcpuSpec)
		{
			/*Reset interrupt controller pointer. */
			plcpuSpec->pIntCtrl = NULL;
		}
		if (pBase)
		{
			VirtualFree(pBase);
		}
	}
	return bResult;
}

/* Start all application processors. */
BOOL Start_AP()
{
	__LOGICALCPU_SPECIFIC* plcpuSpec = NULL;
	unsigned int processorID = __CURRENT_PROCESSOR_ID;
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;

	/* Get current CPU(BSP)'s specific information. */
	plcpuSpec = ProcessorManager.GetLogicalCPUSpecific(
		0,
		__GetChipID(processorID),
		__GetCoreID(processorID),
		__GetLogicalCPUID(processorID));
	BUG_ON(NULL == plcpuSpec);
	/* Get the corresponding interrupt controller object. */
	pIntCtrl = plcpuSpec->pIntCtrl;
	BUG_ON(NULL == pIntCtrl);

	/* 
	 * Start all AP processor(s) in system,according to the procedure
	 * specified in MP specification.
	 * Show a message first.
	 */
	//_hx_printk("Start all [%d] APs in system...\r\n", ProcessorManager.GetProcessorNum());
	/* Send Init IPI. */
	pIntCtrl->Send_Init_IPI(pIntCtrl, INTERRUPT_DESTINATION_ALL);
	/* Wait for 10ms. */
	__MicroDelay(10000);
	/* Send Startup IPI. */
	pIntCtrl->Send_Start_IPI(pIntCtrl, INTERRUPT_DESTINATION_ALL);
	/* Wait for 200us. */
	__MicroDelay(200);
	/* Send another Startup IPI. */
	pIntCtrl->Send_Start_IPI(pIntCtrl, INTERRUPT_DESTINATION_ALL);
	/* Wait for a short time. */
	__MicroDelay(100);
	return TRUE;
}

/* Stop all application processors. */
BOOL Stop_AP()
{
	return TRUE;
}

#endif //__I386__ && __CFG_SYS_SMP
