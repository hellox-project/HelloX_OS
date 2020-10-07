//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 22,2014
//    Module Name               : stddef.h
//    Module Funciton           : 
//                                Stdand C library simulation code.It only implements a subset
//                                of C library,even much more simple.
//    Last modified Author      :
//    Last modified Date        : Jun 22,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDDEF_H__
#define __STDDEF_H__

#include "__hxcomm.h"

//Definition of standard size_t type.
#ifndef __SIZE_T_DEFINED__
#define __SIZE_T_DEFINED__
typedef unsigned int size_t;
#endif

//Definition of standard offset_t type.
typedef int offset_t;
typedef int off_t;

//uid_t.
typedef int uid_t;

//pid_t.
typedef int pid_t;

//Definition of standard wchar_t type.
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif  /* _WCHAR_T_DEFINED */

//Data types for network module.
typedef unsigned char  u8_t;
typedef signed   char  s8_t;
typedef unsigned short u16_t;
typedef signed   short s16_t;
typedef unsigned long  u32_t;
typedef signed   long  s32_t;

typedef unsigned char  __u8;
typedef signed   char  __s8;
typedef unsigned short __u16;
typedef signed   short __s16;
typedef unsigned long  __u32;
typedef signed   long  __s32;

typedef unsigned long mem_ptr_t;

#endif
