//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug 15, 2020
//    Module Name               : memxfer.h
//    Module Funciton           : 
//                                Memory transfer routines and definitions.
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

#ifndef __MEMXFER_H__
#define __MEMXFER_H__

/* 
 * Enumerates used to describe a pointer's usage:
 *   __IN: should be used as input;
 *   __OUT: should be used as output;
 *   __INOUT: should be used as both.
 */
typedef enum {
	__IN = 0,
	__OUT = 1,
	__INOUT = 2
}__PTR_DIRECTION;

/*
 * allocate a block of memory in kernel,copy
 * data from user space and returns the pointer.
 * copy user data into kernel if dir is in.
 */
void* map_to_kernel(void* user_ptr, unsigned long size, __PTR_DIRECTION dir);

/* 
 * Copy the kernel memory's content back 
 * to user space, user space ptr will be
 * returned
 */
void* map_to_user(void* user_ptr, unsigned long size, __PTR_DIRECTION usr_ptr_dir, void* kernel_ptr);

#endif //__MEMXFER_H__
