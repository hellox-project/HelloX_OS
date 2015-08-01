//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,20 2006
//    Module Name               : HEAP.H
//    Module Funciton           : 
//                                This module countains Heap's definitions
//                                and protypes.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HEAP_H__
#define __HEAP_H__


#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__CFG_SYS_HEAP) && defined(__CFG_SYS_VMM))  //Only available when VMM and HEAP are defined.

//
//The definition of Virtual Area Node,which is used to manage virtual areas
//allocated by the heap manager for certain heap object.
//

BEGIN_DEFINE_OBJECT(__VIRTUAL_AREA_NODE)
//struct __VIRTUAL_AREA_NODE{
    LPVOID                lpStartAddress;    //The start address.
    DWORD                 dwAreaSize;        //Virtual area's size.
	struct VIRTUAL_AREA_NODE*  lpNext;            //Pointing to next node.
END_DEFINE_OBJECT(__VIRTUAL_AREA_NODE)
//};

#define MAX_VIRTUAL_AREA_SIZE      1024*1024*4  //4M
#define DEFAULT_VIRTUAL_AREA_SIZE  1024*64      //64K

#define MIN_BLOCK_SIZE             16
#define CURRENT_KERNEL_THREAD      (KernelThreadManager.lpCurrentKernelThread)

//
//Some operating system routines wrapper.
//
#define GET_KERNEL_MEMORY(size)   KMemAlloc(size,KMEM_SIZE_TYPE_ANY)
#define RELEASE_KERNEL_MEMORY(p)  KMemFree(p,KMEM_SIZE_TYPE_ANY,0)
#define GET_VIRTUAL_AREA(size)    lpVirtualMemoryMgr->VirtualAlloc( \
	(__COMMON_OBJECT*)lpVirtualMemoryMgr,                           \
	NULL,                                                           \
	size,                                                           \
	VIRTUAL_AREA_ALLOCATE_ALL,                                      \
	VIRTUAL_AREA_ACCESS_RW,                                         \
	NULL,                                                           \
	NULL)

#define RELEASE_VIRTUAL_AREA(p)  lpVirtualMemoryMgr->VirtualFree(   \
	(__COMMON_OBJECT*)lpVirtualMemoryMgr,                           \
	p)


//
//The definition of free block header,which is used to manage
//each free block in a free block list.
//
BEGIN_DEFINE_OBJECT(__FREE_BLOCK_HEADER)
//struct __FREE_BLOCK_HEADER{
    DWORD                 dwFlags;                 //Flags.
    DWORD                 dwBlockSize;             //The size of this block.
	struct tag__FREE_BLOCK_HEADER*  lpPrev;                  //Pointing to previous free block.
	struct tag__FREE_BLOCK_HEADER*  lpNext;                  //Pointing to next free block.
END_DEFINE_OBJECT(__FREE_BLOCK_HEADER)
//};

#define BLOCK_FLAGS_FREE   0x00000001
#define BLOCK_FLAGS_USED   0x00000002

//
//The definition of heap object.
//A heap object is a memory heap,which can be requested by kernel thread.
//
BEGIN_DEFINE_OBJECT(__HEAP_OBJECT)
//struct __HEAP_OBJECT{
    __KERNEL_THREAD_OBJECT*     lpKernelThread;    //Kernel thread owning this heap.
    __FREE_BLOCK_HEADER         FreeBlockHeader;   //Free list header.
	__VIRTUAL_AREA_NODE*        lpVirtualArea;     //Virtual area list.
	struct tag__HEAP_OBJECT*              lpPrev;            //Pointing to previous heap object.
	struct tag__HEAP_OBJECT*              lpNext;            //Pointing to next heap object.
END_DEFINE_OBJECT(__HEAP_OBJECT)
//};

//
//The definition of heap manager object.
//This object is used to manage heap object,such as create,destroy,allocate
//memory from a heap,free memory,etc.
//This is the user interface of heap module.
//
BEGIN_DEFINE_OBJECT(__HEAP_MANAGER)
    __HEAP_OBJECT*        (*CreateHeap)(DWORD dwInitSize);  //Create a heap object.
    VOID                  (*DestroyHeap)(__HEAP_OBJECT*);   //Destroy a heap object.
	VOID                  (*DestroyAllHeap)(void);              //Destroy all heaps.
	LPVOID                (*HeapAlloc)(__HEAP_OBJECT*,DWORD);  //Allocate memory from heap.
	VOID                  (*HeapFree)(LPVOID,__HEAP_OBJECT*);  //Free the memory block.
END_DEFINE_OBJECT(__HEAP_MANAGER)

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/
//
//The declaration of HeapManager object.
//
extern __HEAP_MANAGER HeapManager;

//
//The malloc and free routines are standard C library routine,
//we also implement these routines in Hello China.
//
//LPVOID malloc(DWORD dwSize);    //Allocate a block of memory from heap.
//VOID free(LPVOID);              //Release a block of memory.

VOID PrintHeapInfo(__KERNEL_THREAD_OBJECT*);
VOID DumpFreeList(__HEAP_OBJECT*);

#endif  //defined(__CFG_SYS_VMM) && defined(__CFG_SYS_HEAP)

#ifdef __cplusplus
}
#endif

#endif //__HEAP_H__
