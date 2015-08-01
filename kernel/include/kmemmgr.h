//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,31 2004
//    Module Name               : kmemmgr.h
//    Module Funciton           : 
//                                This module countains the kernal memory
//                                management code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KMEMMGR__
#define __KMEMMGR__

#ifdef __cplusplus
extern "C" {
#endif

//
// Here,we spend some time to descrip the kernal memory layout in detail.
// In Hello China,the kernal memory space is from 0x00000000 to 0x013FFFFF of the
// PHYSICAL memory,it's size is 20M,so,to run Hello China,the target machine's
// physical memory must larger than 20M.
// When the OS is loaded from DISK(floppy or other storage),it resides in low 1M
// memory,because in this time the CPU is working in Real Address Mode(RAM),only
// can address 1M memory.
// When the Hello China is loaded into memory,it begins to execute,first,it form
// the system tables(such as GDT,IDT,etc),fill them in system registers,and transform
// the CPU's working mode to Protected Mode(PM),then,CPU can address all of the 32 bit
// memory space,in that time,Hello China will move itself from low 1M space to up space,
// and the low 1M space is reserved to use in future.
// In currently,Hello China kernal's size would not exceed 1M,so we reserve 1M space to
// be used by Hello China kernal,that space is from 0x00100000 to 0x00200000 of physical
// memory.
// We also reserve 64K space as stack space,it's address is from 0x013EFFF0 to 0x013FFFF0,
// the 16 byte of the top 20M space (from 0x013FFFF0 to 0x013FFFFF) is reserved.
// The space between Hello China kernal to stack space is used to as kernal memory,so,the
// detail layout of the kernal memory is :
//
//                Range                  Size                   Description
//
//    0x00000000  ---------  0x000FFFFF  1M     BIOS data area,hardware buffer,such as VIDEO,
//                                              etc,and the other space of this space is reserved
//                                              for future use.
//    0x00100000  ---------  0x001FFFFF  1M     Hello China's kernal,including Mini-Kernal and 
//                                              Master.
//    0x00200000  ---------  0x013EFFFF  <18M   System kernal memory,buffers,etc.The hardware 
//                                              device driver is loaded into this space,this
//                                              space also used by device drivers.
//    0x013F0000  ---------  0x013FFFF0  <64K   Kernal stack.
//                                              In fact,the kernal stack's size is less than
//                                              64k,it's actually size is 64K - 15 byte.
//    0x01400000  ---------  0xXXXXXXXX  XXX    To be used as application.
//
// The system kernal memory is managed by kernal memory manager,this block of
// memory is separated into two memory pools,one is 4k block memory pool,and
// another is any size memory block pool.The memory client can commit memory
// allocation request to kernal memory manager,using the routine KMemAlloc,
// this function's parameter indicates which pool is used to allocate memory
// from.If the pool is 4k size pool,the request can only get 4k times memory
// blocks,if the request pool is any size,then the client can get any size 
// memory blocks(in fact,the kernal manager can only allocate memory blocks
// whose size is large than 16 byte).
// The first pool's range is from 0x00200000 to 0x00BFFFFF,total 10M space,
// and the second pool's range is from 0x00C00000 to 0x013EFFFF,about 8M space.
// The reason first pool is larger than second pool is,we recommand that the
// hardware device driver allocate memory from 4k pool.
//
//
//

#define KMEM_MAX_BLOCK_SIZE       0x00100000   //Max memory block size can be
                                               //allocated one time.
                                               //This restrict only apply to 4k
                                               //memory pool.

#define KMEM_4K_START_ADDRESS     0x00200000   //The start address of 4k memory pool.
#define KMEM_4K_END_ADDRESS       0x00BFFFFF   //The end address of 4k memory pool.

#define KMEM_ANYSIZE_START_ADDRESS 0x00C00000        //The start address of any size memory pool.
#define KMEM_ANYSIZE_END_ADDRESS   (0x00D00000 - 1)  //0x013EFFFF  //The end address of any size memory pool.
                                                     //NOTICE: In fact,the any size pool's length
                                                     //is less than 8M,it's actually size is:
                                                     //0x013EFFFF - 0x00C00000 + 1 = 127*64K.

#define KMEM_MAX_4K_POOL_NUM      16

#define RoundTo16(x) (x = (x&15) ? x - x&15 + 16 : x)

#define RoundTo4k(x) (x = (x&4095) ? (x - x&4095) + 4096 : x)

#define KMEM_MIN_ALOCATE_BLOCK 16
//
//Clear one bit of the DWORD.
//The position of the bit to be cleared is indicated by index.
//
#define ClearBit(dw,index)  (dw = dw & ~(0x01 << index))

//
//Size type.Any module of the kernal can allocate the three type memory blocks:
//Any Size,1K,and 4K.
//
#define KMEM_SIZE_TYPE_ANY       0x00000001
#define KMEM_SIZE_TYPE_1K        0x00000002
#define KMEM_SIZE_TYPE_4K        0x00000003

//
//Memory management structures.
//
/*struct __ANYSIZE_BLOCK{
	LPVOID         pStartAddress;        //Start address.
	DWORD          dwBlockSize;          //Block size.
	__ANYSIZE_BLOCK* pPrev;                //Previous object.
	__ANYSIZE_BLOCK* pNext;                //Next object.
};*/                                         //Any size block management structure.

/*typedef struct __tag1K_BLOCK{
	LPVOID*        pStartAddress;
	__tag1K_BLOCK* pPrev;
	__tag1K_BLOCK* pNext;
}__1KSIZE_BLOCK;                        //1K size memory block.*/

typedef struct{
	LPVOID         pStartAddress;
	DWORD          dwMaxBlockSize;      //Max block size can be allocated in 
	                                    //current block pool.
	DWORD          dwOccupMap[8];       //Occupying map,one bit maps to one 4k
	                                    //block,if this bit is 1,this 4k block
	                                    //is occupied,otherwise,this 4k block
	                                    //is free.
}__4KSIZE_BLOCK;                                      //4K size memory block.


//
//Global functions defination.
//
LPVOID KMemAlloc(DWORD,DWORD);          //Alocate a kernal memory block.
                                        //The first parameter gives the block size,
                                        //And the second gives the block type.

//VOID   KMemFree(LPVOID,DWORD dwSizeType = KMEM_SIZE_TYPE_4K,DWORD dwSize = 4096);
VOID  KMemFree(LPVOID,DWORD,DWORD);
                                        //Free a kernal memory block.
                                        //The first parameter gives the start address,
                                        //and the second gives memory block type.
//Get total memory size in system.
DWORD GetTotalMemorySize(void);

//Get free memory size in system.
DWORD GetFreeMemorySize(void);

#ifdef __cplusplus
}
#endif

#endif //kmemmgr.h
