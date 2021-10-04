//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 19,2019
//    Module Name               : pmdesc.h
//    Module Funciton           : 
//                                Physical memory descriptor is defined
//                                in this file,which is used to describe
//                                each physical memory region(s) in system.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PMDESC_H__

#include <stdint.h>

/* Physical memory layout descriptor. */
typedef struct tag__SYSMEM_MLAYOUT_DESCRIPTOR {
	uint32_t start_addr_lo;
	uint32_t start_addr_hi;
	uint32_t length_lo;
	uint32_t length_hi;
	uint32_t mem_type;
}__SYSTEM_MLAYOUT_DESCRIPTOR;

/* Physical memory types. */
#define PHYSICAL_MEM_TYPE_UNKNOWN 0
#define PHYSICAL_MEM_TYPE_RAM     1
#define PHYSICAL_MEM_TYPE_RESERVE 2

/*
* System memory layout array's base address.
* System memory layout information is put here,
* one by one in format of __SYSTEM_MLAYOUT_DESCRIPTOR.
* The loader should do it and HelloX kernel will
* read this region to obtain physical memory layout
* information.
*/
#define SYS_MLAYOUT_ADDR 0x103000

/* Memory layout array's maximal size. */
#define MAX_MLAYOUT_ARRAY_SIZE 200

#endif //__PMDESC_H__
