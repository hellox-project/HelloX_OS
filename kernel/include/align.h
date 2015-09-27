//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,26 2015
//    Module Name               : align.h
//    Module Funciton           : 
//                                Byte alignment related operations and macros are
//                                put into this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//
//***********************************************************************/

#ifndef __ALIGN_H__
#define __ALIGN_H__

//Round up or down to a specified value.
#define __ROUND_UP(x,y) ((((x) + (y - 1)) / y) * y)
#define __ROUND_DOWN(x,y) ((x) - ((x) % (y)))

//Round a to b's boundary.
#define __ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

//Align x to y.
#define __ALIGN(x,y) (((x) + (y - 1)) & (~(y - 1)))

//The following code is from u-boot.
/*
* The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
* stack that meets the minimum architecture alignment requirements for DMA.
* Such a buffer is useful for DMA operations where flushing and invalidating
* the cache before and after a read and/or write operation is required for
* correct operations.
*
* When called the macro creates an array on the stack that is sized such
* that:
*
* 1) The beginning of the array can be advanced enough to be aligned.
*
* 2) The size of the aligned portion of the array is a multiple of the minimum
*    architecture alignment required for DMA.
*
* 3) The aligned portion contains enough space for the original number of
*    elements requested.
*
* The macro then creates a pointer to the aligned portion of this array and
* assigns to the pointer the address of the first element in the aligned
* portion of the array.
*
* Calling the macro as:
*
*     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
*
* Will result in something similar to saying:
*
*     uint32_t    buffer[1024];
*
* The following differences exist:
*
* 1) The resulting buffer is guaranteed to be aligned to the value of
*    ARCH_DMA_MINALIGN.
*
* 2) The buffer variable created by the macro is a pointer to the specified
*    type, and NOT an array of the specified type.  This can be very important
*    if you want the address of the buffer, which you probably do, to pass it
*    to the DMA hardware.  The value of &buffer is different in the two cases.
*    In the macro case it will be the address of the pointer, not the address
*    of the space reserved for the buffer.  However, in the second case it
*    would be the address of the buffer.  So if you are replacing hard coded
*    stack buffers with this macro you need to make sure you remove the & from
*    the locations where you are taking the address of the buffer.
*
* Note that the size parameter is the number of array elements to allocate,
* not the number of bytes.
*
* This macro can not be used outside of function scope, or for the creation
* of a function scoped static buffer.  It can not be used to create a cache
* line aligned global buffer.
*/
#define PAD_COUNT(s, pad) (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)
#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)		\
	char __##name[__ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
		      + (align - 1)];					\
	type *name = (type *) __ALIGN((uintptr_t)__##name, align)

#define ALLOC_ALIGN_BUFFER(type, name, size, align)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
	ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

#endif //__ALIGN_H__
