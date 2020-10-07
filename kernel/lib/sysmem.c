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

#include <StdAfx.h>
#include "stddef.h"
#include "stdlib.h"
#include "stdint.h"

//Local macros used to control the release of different memory blocks.
//There are 2 memory block types,named NORMAL and ALIGNED,corresponding the
//result of normal malloc and aligned malloc.
#define MEMORY_BLOCK_TYPE_NORMAL  0x00
#define MEMORY_BLOCK_TYPE_ALIGNED 0x08

//Local routine declarations.
static void _hx_aligned_free(void* ptr);

//Corresponding the C standard malloc routine.
void* _hx_malloc(size_t size)
{
	unsigned long* mem_ptr = NULL;

	//Allocate one long space to store the memory block type value.
	mem_ptr = (unsigned long*)KMemAlloc((size + sizeof(unsigned long)), KMEM_SIZE_TYPE_ANY);
	if (NULL == mem_ptr)
	{
		return NULL;
	}
	//Set the memory block type value.
	mem_ptr[0] = MEMORY_BLOCK_TYPE_NORMAL;

	//Return the memory pointer can be used by caller,it skips the long of original block.
	return (void*)(mem_ptr + 1);
}

//Corresponding the C standard free routine,it can release normal memory block and aligned
//memory block,which is allocated by aligned_malloc routine.
void _hx_free(void* p)
{
	unsigned long* mem_ptr = (unsigned long*)p;
	if (NULL == mem_ptr)
	{
		return;
	}
	//Check the memory block types.
	mem_ptr -= 1;
	if (MEMORY_BLOCK_TYPE_NORMAL == mem_ptr[0])
	{
		KMemFree(mem_ptr, KMEM_SIZE_TYPE_ANY, 0);
		return;
	}
	if (MEMORY_BLOCK_TYPE_ALIGNED == mem_ptr[0])  //Aligned block.
	{
		_hx_aligned_free(p);
		return;
	}
	//Other value of memory type is invalid.
	BUG();
	return;
}

/*
 * Allocates n objects with each object's size is s.
 * returns the start address if success,otherwise
 * NULL.
 * This routine is used to avoid overflow of n*s in
 * revoking of malloc(n*s),since it checks the n/s
 * values.
 */
void* _hx_calloc(size_t n, size_t s)
{
	void* p = NULL;

	/* zero value just lead failure. */
	if ((0 == n) || (s == 0))
	{
		goto __TERMINAL;
	}

	/* 
	 * n multiplies s may overflow if at least one
	 * larger than 65535,we never support the 16 bits
	 * machine.
	 */
	if ((n > 65535) || (s > 65535))
	{
		/* 
		 * Do our best to avoid the dividing since it's 
		 * costly,so we do above checking.
		 */
		if ((size_t)-1 / n < s)
		{
			/* overflow. */
			goto __TERMINAL;
		}
	}

	/* Just invoke the malloc to get memory. */
	p = _hx_malloc(n * s);
	if(NULL == p)
	{
		goto __TERMINAL;
	}
	memset(p, 0, n * s);

__TERMINAL:
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

//Allocate a block of aligned memory,mainly used by device drivers.
void* _hx_aligned_malloc(int size, int align)
{
	void* mem_ptr = NULL;
	unsigned char* block_ptr = NULL;
	unsigned long alloc_size = size + align + 2 * sizeof(unsigned long) + sizeof(void*);
	//Allocate memory from kernel pool.
	mem_ptr = _hx_malloc(alloc_size);
	if (NULL == mem_ptr)
	{
		return NULL;
	}
	block_ptr = mem_ptr;
	block_ptr += (sizeof(void*) + 2 * sizeof(unsigned long) + align);
	block_ptr = (unsigned char*)(((long)block_ptr) & (~(align - 1)));
	((unsigned long*)block_ptr)[-1] = MEMORY_BLOCK_TYPE_ALIGNED;
	((unsigned long*)block_ptr)[-2] = (unsigned long)mem_ptr;
	((unsigned long*)block_ptr)[-3] = KERNEL_OBJECT_SIGNATURE;
	return block_ptr;
}

static void _hx_aligned_free(void* block_ptr)
{
	unsigned long* mem_ptr = block_ptr;
	if (NULL == mem_ptr)
	{
		return;
	}
	//Validate the memory block's type.
	BUG_ON(mem_ptr[-1] != MEMORY_BLOCK_TYPE_ALIGNED);
	//Validate the memory block's signature.
	BUG_ON(mem_ptr[-3] != KERNEL_OBJECT_SIGNATURE);
	//Release the corresponding normal memory block.
	_hx_free((LPVOID)mem_ptr[-2]);
}

/* reallocate the specified memory. */
void* _hx_realloc(void* ptr, size_t new_sz)
{
	void* ptr_ret = ptr;

	if ((0 == new_sz) || (NULL == ptr))
	{
		return ptr_ret;
	}

	/* Just allocate a new memory block. */
	ptr_ret = _hx_malloc(new_sz);
	if (NULL == ptr_ret)
	{
		return ptr_ret;
	}
	/* Copy the original content into new block. */
	memcpy(ptr_ret, (const void*)ptr, new_sz);
	/* Release the old one. */
	_hx_free(ptr);

	return ptr_ret;
}
