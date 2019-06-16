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
// and the second pool's range is from 0x00C00000 to 0x013FFFFF,8M space.
// The reason first pool is larger than second pool is,we recommand that the
// hardware device driver allocate memory from 4k pool.
//

/* 
 * How many 4K block pool there is in system. 
 * Each 4K block pool's size is 1M and at most
 * 10 pools(10M) in system currently.
 */
#define KMEM_MAX_4K_POOL_NUM 10

 /* Maximal allocation size of 4K memory pool. */
#define KMEM_MAX_4KBLOCK_SIZE 0x00100000

#define RoundTo16(x) (x = (x&15) ? x - x&15 + 16 : x)

#define RoundTo4k(x) (x = (x&4095) ? (x - x&4095) + 4096 : x)

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

typedef struct{
	LPVOID         pStartAddress;
	DWORD          dwMaxBlockSize;      //Max block size can be allocated in 
	                                    //current block pool.
	DWORD          dwOccupMap[8];       //Occupying map,one bit maps to one 4k
	                                    //block,if this bit is 1,this 4k block
	                                    //is occupied,otherwise,this 4k block
	                                    //is free.
}__4KSIZE_BLOCK;                        //4K size memory block.


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

#endif //kmemmgr.h
