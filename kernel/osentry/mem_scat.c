//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 23 Feb, 2014
//    Module Name               : mem_scat.cpp
//    Module Funciton           : 
//                                This module contains hardware level system memory regions.Each memory region,
//                                for read and write,should be define an entry in SystemMemRegion array,thus it
//                                can be managed by operating system's memory management machanism.
//
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif
#include "buffmgr.h"

#ifndef __GCC__
__MEMORY_REGION SystemMemRegion[] = {
	//{Start address of memory region,memory region's length}
	{(LPVOID)KMEM_ANYSIZE_START_ADDRESS,0x00100000},  //1M memory,start from KMEM_ANYSIZE_START_ADDRESS
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00100000),0x00100000},
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00200000),0x00100000},
	{(LPVOID)(KMEM_ANYSIZE_START_ADDRESS + 0x00300000),0x00100000},
	//Please add more memory regions here.
	//The last entry must be NULL and zero,to indicate the end of this array.
	{NULL,0}
};
#endif
