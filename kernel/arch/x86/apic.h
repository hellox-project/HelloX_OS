//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug 03,2018
//    Module Name               : apic.h
//    Module Funciton           : 
//                                APIC(Advanced Programmable Interrupt Controller)
//                                related structures and constants,functions.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __APIC_H__
#define __APIC_H__

#include <config.h>
#include <TYPES.H>
#include <stdint.h>
#include <smp.h>

/* APIC types. */
#define APIC_TYPE_XAPIC   0x01
#define APIC_TYPE_2XAPIC  0x02
#define APIC_TYPE_IO      0x03

/* APIC type name. */
#define APIC_NAME_XAPIC   "xAPIC"
#define APIC_NAME_2XAPIC  "2xAPIC"
#define APIC_NAME_IO      "ioAPIC"

/* Registers offset of local APIC. */
#define LAPIC_REGISTER_APICID         0x0020
#define LAPIC_REGISTER_VERSION        0x0030
#define LAPIC_REGISTER_TASKPRIO       0x0080
#define LAPIC_REGISTER_EOI            0x00B0
#define LAPIC_REGISTER_LDR            0x00D0
#define LAPIC_REGISTER_DFR            0x00E0
#define LAPIC_REGISTER_SIVR           0x00F0  /* Suprious Interrupt Vector Register. */
#define LAPIC_REGISTER_ESR            0x0280
#define LAPIC_REGISTER_ICR_LOW        0x0300
#define LAPIC_REGISTER_ICR_HIGH       0x0310
#define LAPIC_REGISTER_LVT_TIMER      0x0320
#define LAPIC_REGISTER_LVT_PERF       0x0340
#define LAPIC_REGISTER_LVT_INT0       0x0350
#define LAPIC_REGISTER_LVT_INT1       0x0360
#define LAPIC_REGISTER_LVT_ERR        0x0370
#define LAPIC_REGISTER_TIMER_INITCNT  0x0380
#define LAPIC_REGISTER_TIMER_CURRCNT  0x0390
#define LAPIC_REGISTER_TIMER_DIV      0x03E0

/* LVT control bits. */
#define LAPIC_LVT_INT_DISABLE         0x10000
#define LAPIC_TIMER_MODE_PERIODIC     0x20000

/* APIC timer's interrupt vector value. */
#define INTERRUPT_VECTOR_APICTMR  0x2E
/* IPI interrupt vector,for instant scheduling. */
#define INTERRUPT_VECTOR_IPI_IS   0x2F
/* IPI interrupt vector,for TLB flushing. */
#define INTERRUPT_VECTOR_IPI_TLBF 0x30

/* How many times of SYSTEM_SLICE_TIME for every APIC timer interrupt. */
#define LAPIC_TIMER_INT_MULTIPLIXER 4

/* APIC supported vector range. */
#define APIC_INT_VECTOR_START 0x2E
#define APIC_INT_VECTOR_END   0x30

#endif //__APIC_H__
