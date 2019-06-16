//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 1,2019
//    Module Name               : mlayout.h
//    Module Funciton           : 
//                                HelloX's memory layout description is put
//                                into this file.
//                                Memory layout related constants,structures,
//                                objects and routines prototype are also in
//                                this file.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MLAYOUT_H__
#define __MLAYOUT_H__

/*
 * Here,we spend some time to describe the kernal's memory 
 * layout in detail.
 * In HelloX,the kernal memory space is from 0x00000000 to 
 * 0x013FFFFF of the PHYSICAL memory,it's size is 20M.To run
 * HelloX,the target machine's physical memory must larger 
 * than 20M.
 */

/*
 * HelloX's physical memory layout after loaded into memory:
 *
 *     | ~~~~~~~~~~~~~~~~~ |
 *     |                   |<---- 1M
 *     |  BIOS AREA        |
 *     |                   |
 *     | ----------------- |<---- 640K
 *     |                   |
 *     |                   |
 *     |                   |
 *     |  MASTER.BIN       |
 *     |  567K at most     |
 *     |                   |
 *     |                   |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 72K
 *     |                   |
 *     |  MINIKER.BIN      |
 *     |  64K(16K space)   |
 *     |                   |
 *     | ----------------- |<---- 8K
 *     |                   |
 *     |  REALINIT.BIN     |
 *     |  4K
 *     |                   |
 *     | ----------------- |<---- 4K
 *     |                   |
 *     |  BIOS AREA        |
 *     |                   |
 *     | ----------------- |<---- 0
 *
 *  The MINIKER.BIN module's layout as follows:
 *    0x0000(0x100000) - 0x1FFF [8K]: Initialization code and sys
 *                                    tables,such as IDT,GDT...
 *    0x2000(0x102000) - 0x2FFF [4K]: Trampoline code for AP in SMP
 *    0x3000(0x103000) - 0x3FFF [4k]: Memory layout table.
 *    0x4000(0x104000) - 0x7FFF [16k]: System stack for system init.
 *    0x8000(0x108000) - 0xBFFF [16k]: Reserved.
 *
 *  The REALINIT.BIN module's layout as follows:
 *    0x0000(0x1000) - 0x03FF(0x13FF): 16 bits real mode initializatio code
 *    0x0400(0x1400) - 0x07FF(0x17FF): 32 bits protected mode code
 *    0x0800(0x1800) - 0x0BFF(0x1BFF): BIOS service agent for Kernel
 *    0x0C00(0x1C00) - 0x0FFF(0x1FFF): Reserved for furture's extension.
 *
 *  The memory space from 0xA0000 down ward will be used as
 *  application processor's initialization stack after BSP boot
 *  over.
 */

 /*
 * HelloX's physical memory layout after initialized and 
 * ready to run:
 *
 *     | ----------------- |<---- 0xFFFFFFFF
 *     |                   |
 *     |  DEVICE MAPED I/O |
 *     |                   |
 *     | ----------------- |<---- 0xC0000000
 *       
 *       ~~~~~~~~~~~~~~~~~ 
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- END of RAM
 *     |                   |
 *     |                   |
 *     |  USABLE MEMORY    |
 *     |  PAGED MEMORY     |
 *     |                   |
 *     | ----------------- |<---- 64M(KMEM_KERNEL_EXCLUDE_END)
 *     |                   |
 *     |  USABLE MEMORY    |
 *     |  ALLOC AT ANYSIZE |
 *     |                   |
 *     | ----------------- |<---- 12M(KMEM_ANYSIZEPOOL_START)
 *     |                   |
 *     |  USABLE MEMORY    |
 *     |  ALLOC AT 4K      |
 *     |                   |
 *     | ----------------- |<---- 2M(KMEM_4KPOOL_START)
 *     |                   |
 *     |                   |
 *     |  HELLOX KERNEL    |
 *     |  (MASTER.BIN)     |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 1M + 64K
 *     |                   |
 *     |  MINIKER.BIN      |
 *     |  (16K SPACE)      |
 *     |                   |
 *     | ----------------- |<---- 1M
 *     |                   |
 *     |  BIOS AREA        |
 *     |  RESERVED         |
 *     |                   |
 *     | ----------------- |<---- 0
 *
 *  HelloX kernel is the binary code and static data of OS kernel,
 *  but does not include the dynamic data part,which will be allocated
 *  from USABLE MEMORY,by calling KMemAlloc,in unit of
 *  4K or any size.
 *  The kernel reserved memory space's page table is created in
 *  process of virtual memory manager's initialization,
 *  which calls page index manager's init routine accordingly,
 *  so OS kernel can access these KMEM_KERNEL_EXCLUDE_END
 *  (64M) space freely without warry about the page fault.
 *  
 *  The USSABLE MEMORY is seperated into 2 parts farther:
 *  1. The 4K allocation unit part,starts from KMEM_4KPOOL_START
 *     and it's length is KMEM_4KPOOL_LENGTH,currently is 10M;
 *  2. The any size allocation unit part,from KMEM_ANYSIZEPOOL_START
*      and it's length is KMEM_ANYSIZEPOOL_LENGTH,currently
*      the start address of any size pool is 0xC00000.
 *  The 4K unit part can be used by I/O driver buffer,
 *  and other functions that need large buck of memories.
 *  The any size unit part can be used by kernel objects
 *  that need fragmented memory blocks,such as thread object,
 *  synchronous object,file object,kernel thread's stack,etc.
 *  All these 2 regions always reside in memory and are not
 *  managemend by PageFrameManager,so aren't be swapped out
 *  from memory.
 *  
 *  SO THE MINIMAL RAM SPACE THAT HELLOX CAN RUN MUST NOT 
 *  LESS THAN 64M.
 *
 *  The MINIKER.BIN's code size is 48K,but there is a 16K
 *  space from history,we just keep it.:-)
 *  The physical system layout table is in 12K offset from
 *  MINIKER.BIN's start address,so it's memory address is
 *  0x103000 since MINIKER.BIN is relocated into 1M memory.
 *  
 *  Other RAM space start from KMEM_KERNEL_EXCLUDE_END is
 *  page frames that managed by PageFrameManager and alloc
 *  to applications or device drivers,in maner of using
 *  VirtualAlloc routine.The pages in this range can be
 *  swapped out.
 *  
 *  OS kernel developers could adjust the actual memory
 *  layout by changing the corresponding macros with
 *  prefix of 'KMEM',such as KMEM_ANYSIZEPOOL_START,
 *  KMEM_KERNEL_EXCLUDE_END,etc.
 */

/*
 *  HelloX's logical(lineary) memory space layout as 
 *  follows,in 32 bits:
 *  
 *     | ----------------- |<---- 0xFFFFFFFF
 *     |                   |
 *     |  DEVICE MAPED I/O |
 *     |                   |
 *     | ----------------- |<---- 0xC0000000(KMEM_USERSPACE_END/KMEM_IOMAP_START)
 *     |                   |  
 *     |                   |
 *     |                   |
 *     |  USER SPACE       |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 0x40000000(KMEM_USERSPACE_START/KMEM_KRNLSPACE_END)
 *     |                   |
 *     |                   |
 *     |  RESERVED KERNEL  |
 *     |  SPACE            |
 *     |                   |
 *     | ----------------- |<---- 64M(KMEM_KERNEL_EXCLUDE_END/KMEM_KRNLSPACE_START)
 *     |                   |
 *     |  USABLE MEMORY    |
 *     |  ALLOC AT ANYSIZE |
 *     |                   |
 *     | ----------------- |<---- 12M(KMEM_ANYSIZEPOOL_START)
 *     |                   |
 *     |  USABLE MEMORY    |
 *     |  ALLOC AT 4K      |
 *     |                   |
 *     | ----------------- |<---- 2M(KMEM_4KPOOL_START)
 *     |                   |
 *     |                   |
 *     |  HELLOX KERNEL    |
 *     |  (MASTER.BIN)     |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 1M + 64K
 *     |                   |
 *     |  MINIKER.BIN      |
 *     |  (16K SPACE)      |
 *     |                   |
 *     | ----------------- |<---- 1M
 *     |                   |
 *     |  BIOS AREA        |
 *     |  RESERVED         |
 *     |                   |
 *     | ----------------- |<---- 0
 *
 *  Same as most OS,HelloX's memory space is divided
 *  into kernel space and user space.
 *  Applications running in user space can not access
 *  kernel space directly without system call.But
 *  the kernel code can access user space freely.
 *  The kernel space is from the start lineary address
 *  to KMEM_KRNLSPACE_END,or KMEM_USERSPACE_START,these
 *  2 macros are same.The kernel space is seperated
 *  into 2 parts farther:the kernel reserved part,
 *  which is from 0 to KMEM_KRNLSPACE_START,and the
 *  rest part,which is from KMEM_KRNLSPACE_START to
 *  KMEM_KRNLSPACE_END.
 *  The reserved part of kernel space resides in
 *  memory always,and initialized in process of
 *  system initialization,with physical RAM space
 *  allocated to it.The rest part of kernel
 *  space is reserved the lineary space only and
 *  no physical RAM is allocated.Kernel code can
 *  use this space by calling VirtualAlloc routine
 *  to reserve page table and physical memory(from
 *  paged pool).Kernel libraries that common used
 *  will be loaded into this range.
 *
 *  The user space start from KMEM_USERSPACE_START
 *  to KMEM_USERSPACE_END,currently this range's
 *  length is 2G in 32 bits.More detailed layout of
 *  user space as follows:
 *
 *     | ----------------- |<---- 0xC0000000(KMEM_USERSPACE_END/KMEM_IOMAP_START/
 *     |                   |                 KMEM_USERSTACK_END)
 *     |                   |
 *     |  USER STACK       |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 0xA0000000(KMEM_USERSTACK_START/KMEM_USERLIB_END)
 *     |                   |
 *     |                   |
 *     |  USER LIBRARY     |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 0x80000000(KMEM_USERHEAP_END/KMEM_USERLIB_START)
 *     |                   |
 *     |                   |
 *     |  USER HEAP        |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 0x60000000(KMEM_USERHEAP_START/KMEM_USERAPP_END)
 *     |                   |
 *     |                   |
 *     |                   |
 *     |  USER APPLICATION |
 *     |                   |
 *     |                   |
 *     |                   |
 *     | ----------------- |<---- 0x40000000(KMEM_USERSPACE_START/KMEM_KRNLSPACE_END/
 *     |                   |                 KMEM_USERAPP_START)
 *
 *  The whole space is seperated into 4 parts:
 *  User application,the executable image of the application
 *  is loaed into this region;
 *  User heap,dynamic allocated memory from application
 *  by using malloc is reserved in this region;
 *  User library region,the libraries that for this application
 *  use only will be loaded into this region;
 *  User stack,stack region for user application.Each user
 *  thread has it's own stack,these stacks are all allocated
 *  from this region.
 *  Memories of these regions are allocated on demand,i.e,when
 *  a new process is created,only certain number of pages of
 *  these region are allocated,the actual number of page is defined
 *  by KMEM_USERXXXX_DEFAULTPGNUM macros,or indicated by the
 *  input parameters of create process APIs.
 *  APIs are offered by kernel to shrink or enlarge the default
 *  size of all these regions.
 *  Currently all these 4 regions' total length is same,that
 *  is 512M.This value can be changed by redefine the corresponding
 *  macros and recompile HelloX's kernel.
 */

/* Start logical address of kernel. */
#define KMEM_KRNLSPACE_START 0x00000000

/* End logical address of kernel space. */
#define KMEM_KRNLSPACE_END  0x40000000

/* Check if one address is in kernel space. */
#define KMEM_IN_KERNEL_SPACE(addr) ((unsigned long)addr < KMEM_KRNLSPACE_END)

/* Start logical address of user space. */
#define KMEM_USERSPACE_START 0x40000000

/* Total length of kernel space. */
#define KMEM_KRNLSPACE_LENGTH 0x40000000

/* Start logical address of user application. */
#define KMEM_USERAPP_START KMEM_USERSPACE_START

/* Start logical address of user agent. */
#define KMEM_USERAGENT_START KMEM_USERAPP_START

/* Length of user agent module. */
#define KMEM_USERAGENT_LENGTH (4096 * 8)

/* End logical address of user application. */
#define KMEM_USERAPP_END 0x60000000

/* Start logical address of user heap. */
#define KMEM_USERHEAP_START KMEM_USERAPP_END

/* End logical address of user heap. */
#define KMEM_USERHEAP_END 0x80000000

/* Start address of user static library. */
#define KMEM_USERLIB_START KMEM_USERHEAP_END

/* End logical address of user static library. */
#define KMEM_USERLIB_END 0xA0000000

/* Start logical address of user stack. */
#define KMEM_USERSTACK_START KMEM_USERLIB_END

/* End logical address of user stack. */
#define KMEM_USERSTACK_END 0xC0000000

/* End logical address of user space. */
#define KMEM_USERSPACE_END KMEM_USERSTACK_END

/* Check if one address is in user space. */
#define KMEM_IN_USER_SPACE(addr) (((unsigned long)addr >= KMEM_USERSPACE_START) && \
	((unsigned long)addr < KMEM_USERSPACE_END))

/* Start address of device's I/O maped memory. */
#define KMEM_IOMAP_START KMEM_USERSPACE_END

/* Total length of IO maped space. */
#define KMEM_IOMAP_LENGTH 0x40000000

/* Check if one address is in IO maped space. */
#define KMEM_IN_IOMAP_SPACE(addr) ((unsigned long)addr > KMEM_IOMAP_START)

/* 
 * Default page numbers allocated to user heap. 
 * So the initial heap memory that alloted to
 * a process is this number multiplies PAGE_SIZE.
 */
#define KMEM_USERHEAP_DEFAULTPGNUM 128

/*
 * Default page numbers allocated to user stack,
 * by default,if no stack size is specified when
 * a new user thread is created.
 */
#define KMEM_USERSTACK_DEFAULTPGNUM 16

/* 
 * End address of memory dedicate occupied memory,
 * please refer above figure to get the meaning of
 * this macro.
 */
#define KMEM_KERNEL_EXCLUDE_END  0x04000000

/* Start address of kernel reserved space. */
#define KMEM_KERNEL_EXCLUDE_START 0x00000000

/* start address of 4k memory pool. */
#define KMEM_4KPOOL_START 0x00200000

/* start address of any size memory pool. */
#define KMEM_ANYSIZEPOOL_START 0x00C00000

/* 4K pool memory's length. */
#define KMEM_4KPOOL_LENGTH (KMEM_ANYSIZEPOOL_START - KMEM_4KPOOL_START)

/* Any size pool memory's total length. */
#define KMEM_ANYSIZEPOOL_LENGTH (KMEM_KERNEL_EXCLUDE_END - KMEM_ANYSIZEPOOL_START)

#endif //__MLAYOUT_H__
