//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 23 Feb, 2014
//    Module Name               : mem_scat.c
//    Module Funciton           : 
//                                RAM's that can be used by OS kernel should
//                                be defined in this file,each element in
//                                SystemMemRegion should corresponding a physical
//                                memory range that can be used by OS.
//
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <buffmgr.h>
#include <mlayout.h>

#ifndef __GCC__
/* 
 * Kernel memory regions,allocated to kernel thread by 
 * KMemAlloc routine. 
 * Detail kernel memory layout information,please refer
 * mlayout.h file.
 */
__MEMORY_REGION SystemMemRegion[] = {
	//{Start address of memory region,memory region's length}
	{(LPVOID)KMEM_ANYSIZEPOOL_START,KMEM_ANYSIZEPOOL_LENGTH},
	//The last entry must be NULL and zero,to indicate the end of this array.
	{NULL,0}
};
#endif
