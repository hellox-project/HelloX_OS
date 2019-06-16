//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : VMM.H
//    Module Funciton           : 
//                                This module countains virtual memory manager's definition
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __VMM_H__
#define __VMM_H__

/*
 * We implement the virtual memory model in Hello China's current version.
 * The following is the layout of virtual memory space:
 *  0x00000000 - 0x013FFFFF    Operating system kernel,including code and data.
 *  0x01400000 - 0x3FFFFFFF    Reserved,maybe used for loadable applications.
 *  0x40000000 - 0x7FFFFFFF    Reserved,maybe used for loadable modules or drivers.
 *  0x80000000 - 0xBFFFFFFF    Application's heap.
 *  0xC0000000 - 0xFFFFFFFF    Device I/O map zone.
 * The following macros define this constant.
 */
#define VIRTUAL_MEMORY_START               0x00000000
#define VIRTUAL_MEMORY_KERNEL_START        0x00000000
#define VIRTUAL_MEMORY_KERNEL_END          0x013FFFFF
#define VIRTUAL_MEMORY_APP_START           0x01400000
#define VIRTUAL_MEMORY_APP_END             0x3FFFFFFF
#define VIRTUAL_MEMORY_MODULE_START        0x40000000
#define VIRTUAL_MEMORY_MODULE_END          0x7FFFFFFF
#define VIRTUAL_MEMORY_HEAP_START          0x80000000
#define VIRTUAL_MEMORY_HEAP_END            0xBFFFFFFF
#define VIRTUAL_MEMORY_IOMAP_START         0xC0000000
#define VIRTUAL_MEMORY_IOMAP_END           0xFFFFFFFF
#define VIRTUAL_MEMORY_END                 0xFFFFFFFF

/* 
 * Object virtual memory manager will be refered 
 * by virtual area descriptor,so we declare it here.
 */
DECLARE_PREDEFINED_OBJECT(__VIRTUAL_MEMORY_MANAGER)

/* The maximal length of virtual area's name. */
#define MAX_VA_NAME_LEN 32

/* 
 * Virtual Area Descriptor,describes a virtual memory
 * range in process or kernel's virtual memory space. 
 */
BEGIN_DEFINE_OBJECT(__VIRTUAL_AREA_DESCRIPTOR)
	/* Pointing back to virtual memory manager. */
    struct tag__VIRTUAL_MEMORY_MANAGER* lpManager;
	/* Start virtual address. */
    LPVOID lpStartAddr;
	LPVOID lpEndAddr;
	/* Points to next one in list. */
	struct tag__VIRTUAL_AREA_DESCRIPTOR* lpNext;
	DWORD dwAccessFlags;   //Access flags.
	DWORD dwCacheFlags;    //Cache flags.
	DWORD dwAllocFlags;    //Allocate flags.
	__ATOMIC_T Reference;  //Reference counter.

	/* Left sub-tree of AVL. */
	struct tag__VIRTUAL_AREA_DESCRIPTOR* lpLeft;
	/* Right sub-tree of AVL. */
	struct tag__VIRTUAL_AREA_DESCRIPTOR* lpRight;
	/* Virtual area's name. */
    char strName[MAX_VA_NAME_LEN];
END_DEFINE_OBJECT(__VIRTUAL_AREA_DESCRIPTOR)

//
//Virtual area's access flags.
//
#define VIRTUAL_AREA_ACCESS_READ        0x00000001    //Read access.
#define VIRTUAL_AREA_ACCESS_WRITE       0x00000002    //Write access.
#define VIRTUAL_AREA_ACCESS_RW          (VIRTUAL_AREA_ACCESS_READ | VIRTUAL_AREA_ACCESS_WRITE)
#define VIRTUAL_AREA_ACCESS_EXEC        0x00000008    //Execute only.
#define VIRTUAL_AREA_ACCESS_NOACCESS    0x00000010    //Access is denied.

//
//Virtual area's cache flags.
//
#define VIRTUAL_AREA_CACHE_NORMAL       0x00000001    //Cachable.
#define VIRTUAL_AREA_CACHE_IO           0x00000002    //Not cached.
#define VIRTUAL_AREA_CACHE_VIDEO        0x00000004

/*
 * Virtual area's allocate flags.
 * The meaning of these flags and their difference
 * could be illustrated as following table:
 * 
 *             Reserve VAD  |  Reserve Page Tables  |  Reserve RAM pages    
 * ---------------------------------------------------------------------
 * RESERVE    |    *        |         N/A           |        N/A
 * ---------------------------------------------------------------------
 * COMMIT     |    N/A      |         *             |         *
 * ---------------------------------------------------------------------
 * I/O        |    *        |         *             |        N/A
 * ---------------------------------------------------------------------
 * ALL        |    *        |         *             |         *
 * ---------------------------------------------------------------------
 * IOCOMMIT   |    *        |         *             |         *
 * ---------------------------------------------------------------------
 * MAP        |    *        |         *             |        N/A
 * ---------------------------------------------------------------------
 * 
 * Notes:
 * 1. The combining effectiveness of RESERVE and COMMIT,is same as ALL flag;
 * 2. The difference between ALL and IOCOMMIT is,cache flag is disabled
 *    when set page tables for IOCOMMIT,but cache flag is keep enabled when
 *    allocates page tables for ALL;
 * 3. DEFAULT as alias of ALL flag.
 */
#define VIRTUAL_AREA_ALLOCATE_RESERVE   0x00000001
#define VIRTUAL_AREA_ALLOCATE_COMMIT    0x00000002
#define VIRTUAL_AREA_ALLOCATE_IO        0x00000004
#define VIRTUAL_AREA_ALLOCATE_ALL       0x00000008
#define VIRTUAL_AREA_ALLOCATE_IOCOMMIT  0x00000010
#define VIRTUAL_AREA_ALLOCATE_MAP       0x00000020
#define VIRTUAL_AREA_ALLOCATE_DEFAULT   VIRTUAL_AREA_ALLOCATE_ALL

/*
 * Switch virtual area number.
 * Once the virtual area's number exceed this value,
 * virtual memory manager will switch to AVL tree from list
 * to manage virtual areas.
 */
#define SWITCH_VA_NUM 2048

 /* Virtual memory manager object. */
BEGIN_DEFINE_OBJECT(__VIRTUAL_MEMORY_MANAGER)
	INHERIT_FROM_COMMON_OBJECT
	/* Lock to protect this object. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Page index manager. */
	__PAGE_INDEX_MANAGER* lpPageIndexMgr;
	/* List header of virtual area link. */
    __VIRTUAL_AREA_DESCRIPTOR* lpListHdr;
	/* AVL tree's root. */
	__VIRTUAL_AREA_DESCRIPTOR* lpTreeRoot;
	/* How many virtual areas allocated. */
	volatile unsigned long dwVirtualAreaNum;

	/* Operations the virtual memory manager can offer. */
	/* Allocate a block of virtual memory. */
	LPVOID (*VirtualAlloc)(__COMMON_OBJECT* pThis,
		LPVOID pDesiredAddr,
		unsigned long ulSize,
		unsigned long alloc_flags,
		unsigned long access_flags,
		char* va_name,
		LPVOID pReserved);
	/* Release the memory block allocated by VirtualAlloc. */
	VOID (*VirtualFree)(__COMMON_OBJECT* pThis, LPVOID pVirtualAddr);
	/* Map a physical memory block into virtual space. */
	BOOL (*VirtualMap)(__COMMON_OBJECT* pThis, LPVOID pVirtualAddr,
		LPVOID pPhysicalAddr, unsigned long length, unsigned long access_flags,
		char* va_name);
	/* Return page directory's address. */
	LPVOID (*GetPdAddress)(__COMMON_OBJECT*);
	/* Return the physical address by specifying the virtual address. */
	LPVOID (*GetPhysicalAddress)(__COMMON_OBJECT*, LPVOID);
	/* Copy memory from user to kernel,or from kernel to user. */
	BOOL (*UserMemoryCopy)(__COMMON_OBJECT* pThis,
		LPVOID pKernelStart,
		LPVOID pUserStart,
		unsigned long length,
		BOOL bUserToKernel);
END_DEFINE_OBJECT(__VIRTUAL_MEMORY_MANAGER)

/* Initializer of virtual memory manager. */
BOOL VmmInitialize(__COMMON_OBJECT*);

/* Unintializer of VMM. */
BOOL VmmUninitialize(__COMMON_OBJECT*);

/*
 * The first virtual memory manager in system,for OS
 * kernel space's management.
 */
extern __VIRTUAL_MEMORY_MANAGER* lpVirtualMemoryMgr;

/* Show all virtual areas of a process,for debugging. */
VOID PrintVirtualArea(__VIRTUAL_MEMORY_MANAGER*);

#endif  //__VMM_H__
