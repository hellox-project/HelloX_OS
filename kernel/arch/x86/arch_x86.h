//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : March 23,2019
//    Module Name               : arch_x86.h
//    Module Funciton           : 
//                                x86 CPU specific routines,constants,and
//                                data structures,are defined in this header.
//                                It would be included by arch.h file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ARCH_X86_H__
#define __ARCH_X86_H__

/* Common data types. */
#include <TYPES.H>

/* Cache line size for x86. */
#define DEFAULT_CACHE_LINE_SIZE 64
#define ARCH_DMA_MINALIGN DEFAULT_CACHE_LINE_SIZE

/* 
 * Unaligned memory accessing.
 * The following macros must be adapted according CPU
 * architecture,since it may lead exception when implemented 
 * unproper.
 * We just get and put value as aligned memory accessing 
 * since x86 support unaligned memory accessing directly.
 */
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

 /* Port operating routines for x86. */
DWORD __ind(WORD wPort);
VOID __outd(WORD wPort, DWORD dwVal);
UCHAR __inb(WORD wPort);
VOID __outb(UCHAR ucVal, WORD wPort);
WORD __inw(WORD wPort);
VOID __outw(WORD wVal, WORD wPort);
VOID __inws(BYTE* pBuffer, DWORD dwBuffLen, WORD wPort);
VOID __outws(BYTE* pBuffer, DWORD dwBuffLen, WORD wPort);

/* GDT entry. */
#ifdef __MS_VC__
#pragma pack(push,1)
struct gdt_entry_struct
{
	uint16_t limit_low;           // The lower 16 bits of the limit.
	uint16_t base_low;            // The lower 16 bits of the base.
	uint8_t  base_middle;         // The next 8 bits of the base.
	uint8_t  access;              // Access flags, determine what ring this segment can be used in.
	uint8_t  granularity;
	uint8_t  base_high;           // The last 8 bits of the base.
} ;
#pragma pack(pop)
#else
struct gdt_entry_struct
{
	uint16_t limit_low;           // The lower 16 bits of the limit.
	uint16_t base_low;            // The lower 16 bits of the base.
	uint8_t  base_middle;         // The next 8 bits of the base.
	uint8_t  access;              // Access flags, determine what ring this segment can be used in.
	uint8_t  granularity;
	uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
#endif
typedef struct gdt_entry_struct __GDT_ENTRY;

/* 
 * Task structure for x86(TSS). 
 * Each processor(physical cores,or hyper-threads) has
 * one in HelloX,it's only usage is to tell CPU the
 * kernel stack(ring 0 stack) when interrupt/exception
 * raise in user mode.
 */
typedef struct {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eFlags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint32_t trap_iomap;
}__X86_TSS;

/* Set a TSS entry into x86's GDT. */
BOOL __cdecl SetTSSToGDT(int index, uint32_t low_dword, uint32_t high_dword);

/* Simulates a nop instruction. */
#define __NOP() \
__asm{    \
	nop   \
}

/*
 * System call parameter block,used to transfer parameters between user mode
 * and kernel mode.
 * It's just same as the stack frame established when interrupt raise.
 */
typedef struct SYSCALL_PARAM_BLOCK {
	union {
		uint32_t* param;
		uint32_t ebp;
	}ret_ptr;
	union {
		uint32_t param;
		uint32_t edi;
	}param_0;
	union {
		uint32_t param;
		uint32_t esi;
	}param_1;
	union {
		uint32_t param;
		uint32_t edx;
	}param_2;
	union {
		uint32_t param;
		uint32_t ecx;
	}param_3;
	union {
		uint32_t param;
		uint32_t ebx;
	}param_4;
	uint32_t syscall_num;
#if 0
	DWORD ebp;
	DWORD edi;
	DWORD esi;
	DWORD edx;
	DWORD ecx;
	DWORD ebx;
	DWORD eax;
	DWORD eip;
	DWORD cs;
	DWORD eflags;
	DWORD dwSyscallNum;
	LPVOID lpRetValue;
	LPVOID lpParams[1];
#endif
}__SYSCALL_PARAM_BLOCK;

#endif //__ARCH_X86_H__
