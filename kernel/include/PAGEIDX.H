//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,16 2005
//    Module Name               : PAGEIDX.H
//    Module Funciton           : 
//                                This module countains page index object
//                                definition and page index manager's definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PAGEIDX_H__
#define __PAGEIDX_H__

#ifdef __cplusplus
extern "C" {
#endif

//
//Page control flags for Intel IA32 CPU.
//
#define PTE_FLAG_PRESENT    0x001
#define PTE_FLAG_RW         0x002
#define PTE_FLAG_USER       0x004
#define PTE_FLAG_PWT        0x008
#define PTE_FLAG_PCD        0x010
#define PTE_FLAG_ACCESSED   0x020
#define PTE_FLAG_DIRTY      0x040
#define PTE_FLAG_PAT        0x080
#define PTE_FLAG_GLOBAL     0x100
#define PTE_FLAG_USER1      0x200
#define PTE_FLAG_USER2      0x400
#define PTE_FLAG_USER3      0x800

//
//Page directory entry's flags for Intel's CPU.
//
#define PDE_FLAG_PRESENT    0x001
#define PDE_FLAG_RW         0x002
#define PDE_FLAG_USER       0x004
#define PDE_FLAG_PWT        0x008
#define PDE_FLAG_PCD        0x010
#define PDE_FLAG_ACCESSED   0x020
#define PDE_FLAG_RESERVED   0x040
#define PDE_FLAG_PAGESIZE   0x080
#define PDE_FLAG_GLOBAL     0x100
#define PDE_FLAG_USER1      0x200
#define PDE_FLAG_USER2      0x400
#define PDE_FLAG_USER3      0x800

#define __PTE               __U32                      //Page table entry for Intel's IA32.
#define __GPDE              __U32
#define __MPDE              __U32
#define __PDE               __U32                      //Page directory entry for Intel's IA32.

#define NULL_PDE            0
#define NULL_PTE            0

#define EMPTY_PDE_ENTRY(pde)    (pde == NULL_PDE)    //To check if the page directory entry
                                                     //is NULL.
#define EMPTY_PTE_ENTRY(pte)    (pte == NULL_PTE)    //To check if the page table entry
                                                     //is NULL.

#define SET_PTE_FLAGS(pte,flags)     (pte)  |= flags
#define CLEAR_PTE_FLAGS(pte,flags)   (pte)  &= ~flags
#define SET_GPDE_FLAGS(gpde,flags)   (gpde) |= flags
#define CLEAR_GPDE_FLAGS(gpde,flags) (gpde) &= ~flags
#define SET_MPDE_FLAGS(mpde,flags)   (mpde) |= flags
#define CLEAR_MPDE_FLAGS(mpde,flags) (mpde) &= ~flags

#define DEFAULT_PTE_FLAGS        (PTE_FLAG_PRESENT | PTE_FLAG_RW)
#define DEFAULT_PDE_FLAGS        (PDE_FLAG_PRESENT | PDE_FLAG_RW)
#define PTE_FLAGS_FOR_NORMAL     DEFAULT_PTE_FLAGS
#define PTE_FLAGS_FOR_IOMAP      (DEFAULT_PTE_FLAGS | PTE_FLAG_PCD)

#define INIT_PTE(pte)            (pte  = NULL_PTE)   //Initialize a page table entry to NULL.
#define INIT_PTE_TO_DEFAULT(pte) (pte  = DEFAULT_PTE_FLAGS)
#define INIT_PTE_TO_NULL(pte)    (pte  = NULL_PTE)
#define INIT_PDE(pde)            (pde  = NULL_PDE)   //Initialize a page directory entry to NULL.
#define INIT_PDE_TO_DEFAULT(pde) (pde  = DEFAULT_PDE_FLAGS)
#define INIT_PDE_TO_NULL(pde)    (pde  = NULL_PDE)

#define PDE_ADDRESS_MASK     0xFFFFF000
#define PTE_ADDRESS_MASK     0xFFFFF000
#define PDE_FLAGS_MASK       0x00000FFF
#define PTE_FLAGS_MASK       0x00000FFF

#define PDE_INDEX_MASK      0xFFC00000  //Used to get the page directory index from a virtual
                                        //32 bits address.
#define PTE_INDEX_MASK      0x003FF000  //Used to get the page table index.

#define FORM_PDE_ENTRY(pde,pteaddr)    ((pde) = (pde) + ((pteaddr) & PDE_ADDRESS_MASK))
#define FORM_PTE_ENTRY(pte,pgaddr)     ((pte) = (pte) + ((pgaddr) & PTE_ADDRESS_MASK))
#define SET_PDE(addr,pde)              (*(__PDE*)addr = pde)
#define SET_PTE(addr,pte)              (*(__PTE*)addr = pte)

//
//The page directory start address of the current version.
//
#define PD_START                (0x00200000 - 0x00010000)

//
//First several page tables of os kernel.
//
#define PT_START                (0x00200000 - 0x00010000 + 1024*4)

#define PD_OFFSET_SHIFT    22    //To locate the page directory index,should shift these
                                 //bits to right.
#define PT_OFFSET_SHIFT    12    //To locate the page table index,should shift virtual address
                                 //12 bits to right.

#define PD_SIZE            1024  //How many page directory entries in one page directory,
                                 //for Intel CPU.
#define PT_SIZE            1024  //How many page table entries in one page table,for IA32.
#define PAGE_SIZE          4096  //One page memory's size,4K.

//
//The definition of page index object manager.
//This object is used to manage page index objects.Generally,different hardware platform
//implements different page mechanism,this object also be used to islote this difference.
//

BEGIN_DEFINE_OBJECT(__PAGE_INDEX_MANAGER)
    INHERIT_FROM_COMMON_OBJECT
	__PDE*                         lpPdAddress;    //Physical address of page directory.
	LPVOID                         (*GetPhysicalAddress)(__COMMON_OBJECT*,LPVOID);
	BOOL                           (*ReservePage)(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
	BOOL                           (*SetPageFlags)(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
	VOID                           (*ReleasePage)(__COMMON_OBJECT*,LPVOID);
END_DEFINE_OBJECT(__PAGE_INDEX_MANAGER)

//
//Declare initialize routine for page index manager object.
//

BOOL PageInitialize(__COMMON_OBJECT*);

//
//Declare uninitialize routine for page index manager object.
//

VOID PageUninitialize(__COMMON_OBJECT*);

#ifdef __cplusplus
}
#endif

#endif  //PAGEIDX.H

