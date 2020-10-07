//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug 15, 2020
//    Module Name               : memxfer.c
//    Module Funciton           : 
//                                In HelloX's system call mechanism, memory
//                                in user mode should be moved into kernel,
//                                or from kernel to user. A group of routines
//                                are implemented to do these.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "mlayout.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "memxfer.h"

/*
 * allocate a block of memory in kernel,copy
 * data from user space and returns the pointer.
 * copy user data into kernel if dir is in.
 */
void* map_to_kernel(void* user_ptr, unsigned long size, __PTR_DIRECTION dir)
{
	char* pKernelMem = NULL;

	BUG_ON((NULL == user_ptr) || (0 == size));
	BUG_ON(!KMEM_IN_USER_SPACE(user_ptr));

	pKernelMem = (char*)_hx_malloc(size);
	if (NULL == pKernelMem)
	{
		_hx_printf("%s: out of memory[req = %d]\r\n", __func__, size);
		goto __TERMINAL;
	}

	/* Copy user memory's content into kernel if necessary. */
	if ((__IN == dir) || (__INOUT == dir))
	{
		memcpy(pKernelMem, user_ptr, size);
	}

__TERMINAL:
	return pKernelMem;
}

/*
 * Copy the kernel memory's content back
 * to user space, user space ptr will be
 * returned. The kernel memory space will
 * be released in this routine.
 */
void* map_to_user(void* user_ptr, unsigned long size, __PTR_DIRECTION dir, void* kernel_ptr)
{
	BUG_ON((NULL == user_ptr) || (0 == size) || (NULL == kernel_ptr));
	BUG_ON(!KMEM_IN_USER_SPACE(user_ptr));

	/* Copy back from kernel to user if necessary. */
	if ((__OUT == dir) || (__INOUT == dir))
	{
		memcpy(user_ptr, kernel_ptr, (size_t)size);
	}
	_hx_free(kernel_ptr);

	return user_ptr;
}
