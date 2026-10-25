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

/*
 * Principles of memory maping between user
 * and kernel space:
 * 1. WHY
 *    Different process has different virtual
 *    memory space, i.e, 4G in 32 bits architect.
 *    But the kernel space is common for all
 *    processes and is maped into all process's
 *    space. So when a system call is invoked
 *    and the memory used to hold result is in
 *    user space, when switch to another process,
 *    the kernel may write result to other
 *    process's memory, it may lead system crash.
 * 2. When to map
 *    1) If the system call is run in the same
 *       thread context of the caller, i.e, no
 *       kernel thread is used to execute this
 *       system call, and the user memory block
 *       offered to API is fixed in physical
 *       memory, it's no need to map memory;
 *    2) You must remap the memory from user to
 *       kernel, if the actual execution is
 *       happened in kernel thread. For example,
 *       the socket APIs most execute in tcp/ip
 *       kernel thread, it's not same with the
 *       user thread;
 *    3) Even the system call runs in the same
 *       thread context with the caller, but
 *       the memory block offered to system 
 *       API is not guaranteed fix in memory,
 *       for example the memory block may
 *       swaped out into external storage, 
 *       you MUST remap the memory block in
 *       user space to kernel space, unless
 *       you can guarantee the memory block
 *       is pinned in physical memory. The
 *       LockMemory routine could be used to
 *       lock the user memory block in memory.
 * After the mapping from user to kernel, you
 * MUST map it back to user from kernel when 
 * the system call run over.
 */

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
