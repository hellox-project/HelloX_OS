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

#ifdef __cplusplus
extern "C" {
#endif

#define __ENABLE_VIRTUAL_MEMORY    //If this marcos is defined,the virtual memory model
                                   //is enabled.
                                   //Else,the virtual memory model is not permitted.

//
//We implement the virtual memory model in Hello China's current version.
//The following is the layout of virtual memory space:
// 0x00000000 - 0x013FFFFF    Operating system kernel,including code and data.
// 0x01400000 - 0x3FFFFFFF    Reserved,maybe used for loadable applications.
// 0x40000000 - 0x7FFFFFFF    Reserved,maybe used for loadable modules or drivers.
// 0x80000000 - 0xBFFFFFFF    Application's heap.
// 0xC0000000 - 0xFFFFFFFF    Device I/O map zone.
//The following macros define this constant.
//

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

//
//The definition of virtual area.
//

DECLARE_PREDEFINED_OBJECT(__VIRTUAL_MEMORY_MANAGER)

#define MAX_VA_NAME_LEN    32      //The maximal length of virtual area's name.

BEGIN_DEFINE_OBJECT(__VIRTUAL_AREA_DESCRIPTOR)
    struct tag__VIRTUAL_MEMORY_MANAGER*      lpManager;       //Pointing back to virtual memory manager.
    LPVOID                         lpStartAddr;     //Start virtual address.
	LPVOID                         lpEndAddr;       //End virtual address.
	struct tag__VIRTUAL_AREA_DESCRIPTOR*     lpNext;          //Virtual area list header.
	DWORD                          dwAccessFlags;   //Access flags.
	DWORD                          dwCacheFlags;    //Cache flags.
	DWORD                          dwAllocFlags;    //Allocate flags.
	__ATOMIC_T                     Reference;       //Reference counter.

	//DWORD                          dwTreeHeight;    //AVL tree's height.
	struct tag__VIRTUAL_AREA_DESCRIPTOR*     lpLeft;          //Left sub-tree of AVL.
	struct tag__VIRTUAL_AREA_DESCRIPTOR*     lpRight;         //Right sub-tree of AVL.
    UCHAR                          strName[MAX_VA_NAME_LEN];
	//__FILE*                        lpMappedFile;
	//DWORD                          dwOffset;
	//__FILE_OPERATIONS*             lpOperations;
END_DEFINE_OBJECT(__VIRTUAL_AREA_DESCRIPTOR)    //End of virtual area descriptor's definition.

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

//
//Virtual area's allocate flags.
//
#define VIRTUAL_AREA_ALLOCATE_RESERVE   0x00000001    //Reserved only.
#define VIRTUAL_AREA_ALLOCATE_COMMIT    0x00000002    //Has been committed.
#define VIRTUAL_AREA_ALLOCATE_IO        0x00000004    //Allocated as IO mapping area.
#define VIRTUAL_AREA_ALLOCATE_ALL       0x00000008    //Committed.Only be used by VirtualAlloc
                                                      //routine,the dwAllocFlags variable of
													  //virtual area descriptor never set this
													  //value.
#define VIRTUAL_AREA_ALLOCATE_DEFAULT   VIRTUAL_AREA_ALLOCATE_ALL


//
//The definition of virtual memory manager object.
//

#define SWITCH_VA_NUM    64    //Switch virtual area number.
                               //Once the virtual area's number exceed this value,
							   //virtual memory manager will switch to AVL tree from list
							   //to manage virtual areas.

BEGIN_DEFINE_OBJECT(__VIRTUAL_MEMORY_MANAGER)
    INHERIT_FROM_COMMON_OBJECT                         //Inherit from common object.
	__PAGE_INDEX_MANAGER*            lpPageIndexMgr;   //Page index manager.
    __VIRTUAL_AREA_DESCRIPTOR*       lpListHdr;        //List header of virtual area link.
	__VIRTUAL_AREA_DESCRIPTOR*       lpTreeRoot;       //AVL tree's root.

	DWORD                            dwVirtualAreaNum; //How many virtual areas.
	//__LOCK_T                         SpinLock;         //Used in SMP enviroment.

	LPVOID                           (*VirtualAlloc)(__COMMON_OBJECT*,
		                                             LPVOID,    //Desired start virtual addr.
													 DWORD,     //Size.
													 DWORD,     //Allocate flags.
													 DWORD,     //Access flags.
													 UCHAR*,    //Virtual area's name.
													 LPVOID     //Reserved.
													 );
	VOID                             (*VirtualFree)(__COMMON_OBJECT*,
		                                            LPVOID      //Start virtual address.
													);
	LPVOID                           (*GetPdAddress)(__COMMON_OBJECT*);
END_DEFINE_OBJECT(__VIRTUAL_MEMORY_MANAGER)    //End definition of virtual memory manager object.

//
//The declaration of VmmInitialize routine.
//

BOOL VmmInitialize(__COMMON_OBJECT*);

//
//The definition of VmmUninitialize routine.

VOID VmmUninitialize(__COMMON_OBJECT*);

/***********************************************************************************
************************************************************************************
************************************************************************************
************************************************************************************
***********************************************************************************/

//
//The declaration of lpVirtualMemoryMgr.
//

extern __VIRTUAL_MEMORY_MANAGER* lpVirtualMemoryMgr;

VOID PrintVirtualArea(__VIRTUAL_MEMORY_MANAGER*);

#ifdef __cplusplus
}
#endif

#endif  //__VMM_H__
