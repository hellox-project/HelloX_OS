//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 30,2018
//    Module Name               : acpi.c
//    Module Funciton           : 
//                                ACPI(Advanced Configuration and Power Interface)
//                                related source code.
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
#include <stdlib.h>

#include "smpx86.h"
#include "acpi.h"

/* Only available under PC(or compitable) platforms. */
#if defined(__I386__)
#if defined(__CFG_SYS_SMP)

/* Helper routine to get a number's bit(s). */
static int GetNumberBit(uint32_t number)
{
	int bits = 0;
	while (number & 0x01)
	{
		bits++;
		number >>= 1;
	}
	return bits;
}

/* 
 * Calculate logical_cpu_bits and core_bits,which are used to
 * determine the CPU topology.
 */
static int GetLogicalCPUBits(int core_bits)
{
	int logical_cpu_bits = 0;
	uint32_t count = 0;

	/* Check if HTT feature is supported by current CPU. */
	if (0 == (GetCPUFeature() & CPU_FEATURE_MASK_HTT))
	{
		goto __TERMINAL;
	}

	/*
	* Get count field from EBX,which represents the maximal
	* logical CPU number in a chip.
	*/
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		xor ecx,ecx
		mov eax, 0x01
		cpuid
		mov count, ebx
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	count >>= 16;
	count &= 0xFF;

	logical_cpu_bits = GetNumberBit(count - 1);
	logical_cpu_bits -= core_bits;

__TERMINAL:
	return logical_cpu_bits;
}

static int GetCoreBits()
{
	int core_bits = 0;
	uint32_t count = 0;

	/* Check if HTT feature is supported by current CPU. */
	if (0 == (GetCPUFeature() & CPU_FEATURE_MASK_HTT))
	{
		goto __TERMINAL;
	}

	/*
	 * Get maximal core number in chip,which represents the maximal
	 * core number in a chip package.
	 */
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		xor ecx,ecx
		mov eax,0x04
		cpuid
		mov count,eax
		pop edx
		pop ecx
		pop ebx
		pop eax
	}
	count >>= 26;
	count &= 0x3F;

	core_bits = GetNumberBit(count);

__TERMINAL:
	return core_bits;
}

/* Get chip id giving the processor ID. */
uint8_t __GetChipID(unsigned int processor_id)
{
	int core_bits = GetCoreBits();
	int logical_cpu_bits = GetLogicalCPUBits(core_bits);
	return ((processor_id) & (~((1 << (logical_cpu_bits + core_bits)) - 1)));
}

/* Get the core ID giving the processor ID. */
uint8_t __GetCoreID(unsigned int processor_id)
{
	int core_bits = GetCoreBits();
	int logical_cpu_bits = GetLogicalCPUBits(core_bits);
	return (processor_id >> logical_cpu_bits) & ((1 << core_bits) - 1);
}

/* Get the logical CPU ID giving the processor ID. */
uint8_t __GetLogicalCPUID(unsigned int processor_id)
{
	int core_bits = GetCoreBits();
	int logical_cpu_bits = GetLogicalCPUBits(core_bits);
	return (processor_id) & ((1 << logical_cpu_bits) - 1);
}

/* RSDP descriptor signature. */
static char rsdp_signature[] = {
	'R','S','D',' ',
	'P','T','R',' ',
};

/* 
 * Helper routine to validate the checksum of ACPI data structures.
 * It can be used to validate such as RSDP,RSDT,XSDT,and other ACPI
 * defined structures.
 */
static BOOL acpiValidateChecksum(const char* begin, int length)
{
	char chk_sum = 0;

	if ((NULL == begin) || (length <= 0))
	{
		return FALSE;
	}
	/* Just add up all bytes. */
	for (int i = 0; i < length; i++)
	{
		chk_sum += begin[i];
	}
	return (0 == chk_sum) ? TRUE : FALSE;
}

/* 
 * Local helper routine to locate the RSDP descriptor from BIOS,
 * the start address will be returned in case of success,NULL will
 * be returned if can not locate the RSDP descriptor. 
 */
static struct __RSDP_Descriptor* LocateRSDPDescriptor()
{
	struct __RSDP_Descriptor* pRSDP = NULL;
	uint32_t* begin = (uint32_t*)ACPI_RSDP_RANGE_BEGIN;
	uint32_t* first_part = (uint32_t*)&rsdp_signature[0];
	uint32_t* last_part = (uint32_t*)&rsdp_signature[4];

	while (begin < (uint32_t*)ACPI_RSDP_RANGE_END)
	{
		if ((*begin == *first_part) && (*(begin + 1) == *last_part))
		{
			/* Signature matched,validate the checksum. */
			if (acpiValidateChecksum((const char*)begin, sizeof(struct __RSDP_Descriptor)))
			{
				/* Pass checksum validation,enforce v2.0's check. */
				struct __RSDP_Descriptor20* pRSDP2 = (struct __RSDP_Descriptor20*)begin;
				if (pRSDP2->Revision < 2)
				{
					/* Less than 2.0,find over with success. */
					break;
				}
				/* Validate V2.0 extension's checksum. */
				if (acpiValidateChecksum((const char*)pRSDP2, sizeof(struct __RSDP_Descriptor20)))
				{
					/* Pass checksum validation,find over with success. */
					break;
				}
				else
				{
					/* Abnormal,show out a warning. */
					_hx_printf("%s:extension part's checksum validation failed.\r\n");
				}
			}
		}
		/* RSDP signature always reside in 16 bytes boundary. */
		begin += (16 / sizeof(uint32_t));
	}
	if (begin < (uint32_t*)ACPI_RSDP_RANGE_END)
	{
		/* Located,just return it. */
		pRSDP = (struct __RSDP_Descriptor*)begin;
	}
	return pRSDP;
}

/* 
 * Locate the specified SDT(System Description Table) 
 * from APIC's RSDT(Root System Description Table).
 * The SDT type is specified by sdt_type,and return the
 * common ACPI table header if success,the caller just
 * cast the header to the specific SDT table.
 * NULL will be returned in case of failure.
 */
static struct ACPISDTHeader* LocateSDT(char* sdt_type)
{
	struct __RSDP_Descriptor* pRSDP = NULL;
	struct __RSDP_Descriptor20* pRSDP20 = NULL;
	struct RSDT* pRSDT = NULL;
	struct XSDT* pXSDT = NULL;
	struct ACPISDTHeader* pCommHdr = NULL;;

	/* Parameter checking. */
	if (NULL == sdt_type)
	{
		goto __TERMINAL;
	}
	if (strlen(sdt_type) < 4)
	{
		goto __TERMINAL;
	}

	/* Get RSDT descriptor first. */
	pRSDP = LocateRSDPDescriptor();
	if (NULL == pRSDP)
	{
		goto __TERMINAL;
	}

	/* Get RSDT/XSDT according revision. */
	if (pRSDP->Revision < 2)
	{
		/* Use RSDT. */
		pRSDT = (struct RSDT*)pRSDP->RsdtAddress;
		if (!IN_SYSINITIALIZATION())
		{
			pRSDT = VirtualAlloc(pRSDT, 1024 * 1024, VIRTUAL_AREA_ALLOCATE_IO, VIRTUAL_AREA_ACCESS_RW,
				"XSDT");
			if (NULL == pRSDT)
			{
				_hx_printf("Can not remap RSDT.\r\n");
				goto __TERMINAL;
			}
		}
		/* Validate RSDT table. */
		if (!acpiValidateChecksum((const char*)pRSDT, pRSDT->h.Length))
		{
			goto __TERMINAL;
		}
		/* Find MADT from entries in RSDT. */
		int entries = (pRSDT->h.Length - sizeof(pRSDT->h)) / 4;
		//_hx_printf("RSDT@0x%0X,length:%d,entry num:%d\r\n", pRSDT, pRSDT->h.Length, entries);
		for (int i = 0; i < entries; i++)
		{
			struct ACPISDTHeader* pHdr = (struct ACPISDTHeader*)pRSDT->PointerToOtherSDT[i];
#if 0
			_hx_printf("entry[%d]'s signature:%c%c%c%c\r\n", i,
				pHdr->Signature[0],
				pHdr->Signature[1],
				pHdr->Signature[2],
				pHdr->Signature[3]
			);
#endif
			if (0 == strncmp(pHdr->Signature, sdt_type, 4))
			{
				/* Found,return it. */
				pCommHdr = pHdr;
				goto __TERMINAL;
			}
		}
	}
	else
	{
		/* Use XSDT. */
		pRSDP20 = (struct __RSDP_Descriptor20*)pRSDP;
		pXSDT = (struct XSDT*)pRSDP20->XsdtAddress;
		if (!IN_SYSINITIALIZATION())
		{
			pXSDT = VirtualAlloc(pXSDT, 1024, VIRTUAL_AREA_ALLOCATE_IO, VIRTUAL_AREA_ACCESS_RW,
				"XSDT");
			if (NULL == pXSDT)
			{
				_hx_printf("Can not remap XSDT.\r\n");
				goto __TERMINAL;
			}
		}
		/* Validate XSDT table. */
		if (!acpiValidateChecksum((const char*)pXSDT, pXSDT->h.Length))
		{
			goto __TERMINAL;
		}
		/* Find MADT from entries in RSDT. */
		int entries = (pXSDT->h.Length - sizeof(pXSDT->h)) / 8;
		//_hx_printf("XSDT@0x%0X,length:%d,entry num:%d\r\n", pXSDT, pXSDT->h.Length, entries);
		for (int i = 0; i < entries; i++)
		{
			struct ACPISDTHeader* pHdr = (struct ACPISDTHeader*)pXSDT->PointerToOtherSDT[i];
#if 0
			_hx_printf("entry[%d]'s signature:%c%c%c%c\r\n", i,
				pHdr->Signature[0],
				pHdr->Signature[1],
				pHdr->Signature[2],
				pHdr->Signature[3]
			);
#endif
			if (0 == strncmp(pHdr->Signature, sdt_type, 4))
			{
				/* Found,return it. */
				pCommHdr = pHdr;
				goto __TERMINAL;
			}
		}
	}

__TERMINAL:
	/* Validate the checksum if the SDT is located. */
	if (pCommHdr)
	{
		if (!acpiValidateChecksum((const char*)pCommHdr, pCommHdr->Length))
		{
			pCommHdr = NULL;
		}
	}
	return pCommHdr;
}

/* Locate MADT table. */
static struct MADT* LocateMADT()
{
	return (struct MADT*)LocateSDT("APIC");
}

/* 
 * Parse MADT entries one by one. 
 * Returns TRUE if anything is in place,else
 * FALSE will be returned,and the system may
 * stop to initialize or just run in UP mode.
 * The following case may casue failure:
 * 1. Can not allocate interrupt controller object;
 * 2. Failed to add found processor into ProcessorManager;
 * 3. Too many processors detected(exceed MAX_CPU_NUM);
 * 4. There is no MADT entry at all.
 */
static BOOL ParseMADTEntry(struct MADT* pMADT, __X86_CHIP_SPECIFIC* pChipSpec)
{
	char* pEntryBegin = NULL;
	char* pEntryEnd = NULL;
	struct __IO_APIC* pIoApic = NULL;
	struct __LOCAL_APIC* pLocalApic = NULL;
	char entry_length = 0;
	int core_bits = GetCoreBits();
	int logical_cpu_bits = GetLogicalCPUBits(core_bits);
	int logical_cpu_id = 0, core_id = 0, chip_id = 0;
	__LOGICALCPU_SPECIFIC* plcpuSpecific = NULL;
	int processor_detected = 0;
	BOOL bResult = FALSE;

	/* Parameters checking. */
	BUG_ON(NULL == pMADT);
	BUG_ON(NULL == pChipSpec);

	pEntryBegin = &pMADT->entry_type;
	/* pEntryEnd points to the end of entries data. */
	pEntryEnd = pEntryBegin + (pMADT->h.Length - 0x2C);
	/* 
	 * Parse MADT entries one by one,move begin 
	 * pointer according to each entry's length.
	 */
	while (pEntryBegin < pEntryEnd)
	{
		switch (*pEntryBegin)
		{
		case MADT_ENTRY_TYPE_IOAPIC:
			pIoApic = (struct __IO_APIC*)pEntryBegin;
			pChipSpec->ioapic_base = pIoApic->address;
			pChipSpec->global_intbase = pIoApic->global_intbase;
//#if 0
			_hx_printf("IOAPIC:id = %d,addr = 0x%0X,int_base = %d\r\n",
				pIoApic->apic_id,
				pIoApic->address,
				pIoApic->global_intbase);
//#endif
			entry_length = pIoApic->entry_length;
			pEntryBegin += entry_length;
			break;
		case MADT_ENTRY_TYPE_LAPIC:
			pLocalApic = (struct __LOCAL_APIC*)pEntryBegin;
			/* Show out the information. */
			_hx_printf("LocalAPIC:processor_id = %d,apic_id = %d,flags = 0x%X\r\n",
				pLocalApic->processor_id,
				pLocalApic->apic_id,
				pLocalApic->flags);
			/* Move to next entry. */
			entry_length = pLocalApic->entry_length;
			pEntryBegin += entry_length;
			/* The corresponding processor is not enabled. */
			if (0 == pLocalApic->flags)
			{
				break;
			}
			processor_detected++;
			if (processor_detected > MAX_CPU_NUM)
			{
				_hx_printk("Too many processors detected,can not support yet.\r\n");
				goto __TERMINAL;
			}
			/* Create a logical CPU specific information object. */
			plcpuSpecific = (__LOGICALCPU_SPECIFIC*)_hx_calloc(1,
				sizeof(__LOGICALCPU_SPECIFIC));
			if (NULL == plcpuSpecific)
			{
				goto __TERMINAL;
			}
			/* Substract the chip ID,core ID,and logical CPU ID. */
			logical_cpu_id = (pLocalApic->apic_id) & ((1 << logical_cpu_bits) - 1);
			core_id = (pLocalApic->apic_id >> logical_cpu_bits) & ((1 << core_bits) - 1);
			chip_id = (pLocalApic->apic_id) & (~((1 << (logical_cpu_bits + core_bits)) - 1));
			/* 
			 * Add the found processor into system,using the parameters above. 
			 * Set domain ID to 0 forcefully since we do not support more than one domain yet.
			 */
			if (!ProcessorManager.AddProcessor(0, chip_id, core_id, logical_cpu_id))
			{
				_hx_printf("Failed to add processor[id = %d] into system.\r\n", pLocalApic->apic_id);
				goto __TERMINAL;
			}
			/* Set logical CPU specific information. */
			ProcessorManager.SetLogicalCPUSpecific(0, chip_id, core_id, logical_cpu_id, (void*)plcpuSpecific);
			/* Save chip specific information. */
			ProcessorManager.SetChipSpecific(0, chip_id, (void*)pChipSpec);
			break;
		case MADT_ENTRY_TYPE_ISO:
			entry_length = *(pEntryBegin + 1);
			pEntryBegin += entry_length;
			break;
		case MADT_ENTRY_TYPE_LAPICAO:
			entry_length = *(pEntryBegin + 1);
			pEntryBegin += entry_length;
			break;
		case MADT_ENTRY_TYPE_NMI:
			entry_length = *(pEntryBegin + 1);
			pEntryBegin += entry_length;
			break;
		default:
			entry_length = *(pEntryBegin + 1);
			pEntryBegin += entry_length;
			break;
		}
		if (0 == entry_length)
		{
			/* Should not be 0,exception case,just terminate. */
			_hx_printf("%s:MADT entry length is 0.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	bResult = TRUE;
	/* Program can jump here in case of exception case. */
__TERMINAL:
	return bResult;
}

/* Show out RSDP descriptor information. */
static void showRSDPDescriptor(struct __RSDP_Descriptor* pRSDP)
{
	struct __RSDP_Descriptor20* pRSDP2 = NULL;

	if (pRSDP->Revision == 2)
	{
		pRSDP2 = (struct __RSDP_Descriptor20*)pRSDP;
	}

	_hx_printf("RSDP Descriptor:\r\n");
	_hx_printf("   checksum:%X\r\n", pRSDP->Checksum);
	_hx_printf("   OEM_ID:%c%c%c%c%c%c\r\n",
		pRSDP->OEMID[0],
		pRSDP->OEMID[1],
		pRSDP->OEMID[2],
		pRSDP->OEMID[3],
		pRSDP->OEMID[4],
		pRSDP->OEMID[5]);
	_hx_printf("   Revision:%d\r\n", pRSDP->Revision);
	/* Different revision yields different SDT base address. */
	if (pRSDP->Revision == 0)
	{
		_hx_printf("   RSDT Address:0x%0X\r\n", pRSDP->RsdtAddress);
	}
	else if(pRSDP->Revision == 2)
	{
		_hx_printf("   RSDT Address:0x%0X\r\n", pRSDP->RsdtAddress);
		_hx_printf("   XSDT Address:0x%0X\r\n", (uint32_t)pRSDP2->XsdtAddress);
	}
	else
	{
		_hx_printf("  Invalid revision value.\r\n");
	}
	return;
}

/* 
 * Entry point of ACPI initialization process. 
 * It will be invoked in process of hardware initialization.
 */
BOOL ACPI_Init()
{
	struct MADT* pMADT = NULL;
	__X86_CHIP_SPECIFIC* pChipSpec = NULL;
	BOOL bResult = FALSE;

	/* Get and show out MADT. */
	pMADT = LocateMADT();
	if (NULL == pMADT)
	{
		_hx_printf("Can not locate MADT.\r\n");
		goto __TERMINAL;
	}
	else
	{
		_hx_printf("MADT@0x%0X:\r\n", pMADT);
	}
	/* 
	 * Allocate the chip specific object,it will be linked on the 
	 * chip specific pointer of processor node,managed by ProcessorManager object.
	 */
	pChipSpec = (__X86_CHIP_SPECIFIC*)_hx_calloc(1, sizeof(__X86_CHIP_SPECIFIC));
	if (NULL == pChipSpec)
	{
		goto __TERMINAL;
	}
	/* Save local APIC's base address to chip specific. */
	pChipSpec->lapic_base = pMADT->local_APIC_addr;

	/* 
	 * Parse the MADT table. 
	 * The processor topology will be build in this process,
	 * and processor related information will be collected also in this routine.
	 * The chip specific object will be linked into system by this routine.
	 */
	ParseMADTEntry(pMADT, pChipSpec);

	/* Mark as success. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}
#endif //__CFG_SYS_SMP

/* Entry point of sysinfo command's handler. */
void ShowSysInfo()
{
#if 0
	struct MADT* pMADT = NULL;
	__DECLARE_SPIN_LOCK(spin_lock);
	DWORD dwFlags = 0;

	/* Get and show out MADT. */
	pMADT = LocateMADT();
	if (NULL == pMADT)
	{
		_hx_printf("Can not locate MADT.\r\n");
	}
	else
	{
		_hx_printf("MADT@0x%0X:\r\n", pMADT);
		_hx_printf("   Length:%d\r\n", pMADT->h.Length);
		_hx_printf("   Local APIC addr:0x%0X\r\n", pMADT->local_APIC_addr);
		_hx_printf("   entry_type:%d\r\n", pMADT->entry_type);
		_hx_printf("   entry_length:%d\r\n", pMADT->entry_length);
	}
	ParseMADTEntry(pMADT);

	int core_bits = GetCoreBits();
	int logical_cpu_bits = GetLogicalCPUBits(core_bits);
#endif
#if defined(__CFG_SYS_SMP)
	ProcessorManager.ShowCPU(&ProcessorManager);
#endif
}

#endif //__I386__
