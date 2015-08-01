//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug,25 2004
//    Module Name               : buffmgr.h
//    Module Funciton           : 
//                                This module countains the buffer manager data
//                                structures and data types definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __BUFFMGR_H__
#define __BUFFMGR_H__

#ifdef __cplusplus
extern "C" {
#endif

//
//Basic concepts:
//1. Buffer block : A buffer object,used by application to store data temporarily,
//                  when the temporarily data is freed,the buffer block can be
//                  freed too.
//
//2. Buffer pool : A big block of memory,the buffer blocks are allocated from
//                 this big block memory.

#define MIN_BUFFER_SIZE  16  //The min buffer can be allocated.

//Buffer control block.

#define CREATED_BY_SELF           0x00000001  //When the buffer pool is created by lpCreateBuffer1 function,
                                              //set this bit.
#define CREATED_BY_CLIENT         0x00000002  //When the buffer pool is created by lpCreateBuffer2 function,
                                              //set this bit.
#define POOL_INITIALIZED          0x00000004  //If set,the buffer's pool is created.
                                              //This bit must be set before the buffer manager
                                              //can be used.
#define OPERATIONS_INITIALIZED    0x00000008  //If set,the operations is initialized.
                                              //This bit must be set before the buffer manager
                                              //can be used.

typedef struct tag__BUFFER_CONTROL_BLOCK{
	volatile DWORD                    dwFlags;
	volatile LPVOID                   lpPoolStartAddress;
	volatile DWORD                    dwPoolSize;       //Total memory size.
	volatile DWORD                    dwFreeSize;       //Free memory size.
	volatile DWORD                    dwFreeBlocks;     //Free memory block number.
	volatile DWORD                    dwAllocTimesH;    //How many times the allocation routine is called,high DWORD.
	volatile DWORD                    dwAllocTimesL;    //Low DWORD.
	volatile DWORD                    dwAllocTimesSuccH; //Successful allocation times,high DWORD.
	volatile DWORD                    dwAllocTimesSuccL; //Low DWORD.
	volatile DWORD                    dwFreeTimesH;      //How many times the free routine is called,high DWORD.
	volatile DWORD                    dwFreeTimesL;      //Low DWORD.
	volatile LPVOID                   lpBuffExtension;   //Extensions for special customization.This feild has same
	                                                     //functionality as device extension in device object.
	DWORD                         (*GetControlBlockFlag)(struct tag__BUFFER_CONTROL_BLOCK*);
	struct tag__FREE_BUFFER_HEADER*         lpFreeBufferHeader;

	//Initializer of buffer object.
	BOOL     (*Initialize)(struct tag__BUFFER_CONTROL_BLOCK* pControlBlock);
	BOOL     (*InitializeBuffer)(struct tag__BUFFER_CONTROL_BLOCK* pControlBlock);

	//Buffer operations.
	BOOL     (*CreateBuffer1)(struct tag__BUFFER_CONTROL_BLOCK*,DWORD);
	BOOL     (*CreateBuffer2)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
	VOID     (*AppendBuffer)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
	LPVOID   (*Allocate)(struct tag__BUFFER_CONTROL_BLOCK*,DWORD);
	VOID     (*Free)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID);
	DWORD    (*GetBufferFlag)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID);
	BOOL     (*SetBufferFlag)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
	VOID     (*DestroyBuffer)(struct tag__BUFFER_CONTROL_BLOCK*);
} __BUFFER_CONTROL_BLOCK;

//
//Free buffer's control header.
//All of the free buffers are linked into a bi-direct list.
//
#define BUFFER_STATUS_FREE        0x00000001  //If this bit set,the buffer block is free.
#define BUFFER_STATUS_USED        0x00000002  //If this bit set,the buffer block is used.
#define BUFFER_STATUS_MODIFIED    0x00000004  //If this buffer's content is modified,
                                              //set this bit.

typedef struct tag__FREE_BUFFER_HEADER{
	DWORD                  dwFlags;
	DWORD                  dwBlockSize;
	struct tag__FREE_BUFFER_HEADER*  lpNextBlock;
	struct tag__FREE_BUFFER_HEADER*  lpPrevBlock;
}__FREE_BUFFER_HEADER;

typedef struct tag__USED_BUFFER_HEADER{
	DWORD                  dwFlags;
	DWORD                  dwBlockSize;
	DWORD                  dwReserved1; //Reserved dword,currently used as block identifier.
	DWORD                  dwReserved2;
}__USED_BUFFER_HEADER;

//Structure to describe system memory regions,one for each region.
//This object is used to configure system memory,please modify the
//mem_scat.c file under /osentry directory.
typedef struct{
	LPVOID        lpBuffer;
	DWORD         dwBufferSize;
}__MEMORY_REGION;

//Global object for memory management.Memory allocations routine,such as
//KMemAlloc or KMemFree will call this object's member functions to achieve
//memory management.


extern __BUFFER_CONTROL_BLOCK AnySizeBuffer;
extern __MEMORY_REGION SystemMemRegion[];
#ifdef __cplusplus
}
#endif

#endif //End of buffmgr.h
