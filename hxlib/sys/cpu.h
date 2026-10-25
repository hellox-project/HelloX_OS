//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 05,2017
//    Module Name               : cpu.h
//    Module Funciton           : 
//                                Declares target CPU related constants,such
//                                as CPU model,page size,etc.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C" {
#endif

//CPU endian.
#if (defined (i386) || defined (__i386) || defined (__i386__) || \
	 defined(I386) || defined(__I386) || defined(__I386__) || defined(_M_IX86) || \
     defined (i486) || defined (__i486) || defined (__i486__) || \
     defined (intel) || defined (x86) || defined (i86pc) || \
     defined (__alpha) || defined (__osf__) || defined(_M_X64) || \
     defined (__x86_64__) || defined (__arm__) || defined (__aarch64__))
#define __LITTLE_ENDIAN
#endif

#ifdef __cplusplus
}
#endif

#endif //__CPU_H__
