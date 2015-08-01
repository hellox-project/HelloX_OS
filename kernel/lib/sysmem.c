//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : sysmem.c
//    Module Funciton           : 
//                                Stdand C library simulation code.It only implements a subset
//                                of C library,even much more simple.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "../include/StdAfx.h"
#endif

#ifndef __STDDEF_H__
#include "stddef.h"
#endif

#ifndef __STDLIB_H__
#include "stdlib.h"
#endif

//Implementation of C standard malloc routine.
void* _hx_malloc(size_t size)
{
	return KMemAlloc(size,KMEM_SIZE_TYPE_ANY);
}

//free.
void _hx_free(void* p)
{
	KMemFree(p,KMEM_SIZE_TYPE_ANY,0);
}

//calloc.
void* _hx_calloc(size_t n,size_t s)
{
	void*    p = KMemAlloc(n * s,KMEM_SIZE_TYPE_ANY);
	if(NULL == p)
	{
		return p;
	}
	memset(p,0,n * s);
	return p;
}

//mmap routine of POSIX.
void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset)
{
	return KMemAlloc(length,KMEM_SIZE_TYPE_ANY);
}

//munmap routine of POSIX.
int munmap(void* start,size_t length)
{
	KMemFree(start,KMEM_SIZE_TYPE_ANY,0);
	return 0;
}

//Allocate memory from STACK.Should be implemented later and put into arch related files.
void* _hx_alloca(size_t size)
{
	return NULL;
}
