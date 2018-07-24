//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 7,2018
//    Module Name               : smpx86.h
//    Module Funciton           : 
//                                Symmentric Multiple Processor related constants,
//                                structures and routines,for x86 architecture.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SMPX86_H__
#define __SMPX86_H__

/* For common SMP constants and structures. */
#include <smp.h>

/* CPU feature flags. */
#define CPU_FEATURE_MASK_HTT (1 << 28)

/* Get CPU supported feature flags. */
uint32_t GetCPUFeature();

/* Get chip id giving the processor ID. */
uint8_t __GetChipID(unsigned int processor_id);

/* Get the core ID giving the processor ID. */
uint8_t __GetCoreID(unsigned int processor_id);

/* Get the logical CPU ID giving the processor ID. */
uint8_t __GetLogicalCPUID(unsigned int processor_id);

/* x86 chip specific information. */
typedef struct tag__X86_CHIP_SPECIFIC {
	unsigned long ioapic_base; /* Base address of IOAPIC. */
	unsigned int global_intbase; /* Global interrupt base of IOAPIC. */
	unsigned long lapic_base;  /* Base address of local APIC. */
}__X86_CHIP_SPECIFIC;

/* Initialize the IOAPIC controller. */
BOOL Init_IOAPIC();

/* Initialize the local APIC controller,it will be invoked by each AP. */
BOOL Init_LocalAPIC();

/* Start all application processors. */
BOOL Start_AP();

/* Stop all application processors. */
BOOL Stop_AP();

#endif  //__SMPX86_H__
