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

#endif //__I386__
