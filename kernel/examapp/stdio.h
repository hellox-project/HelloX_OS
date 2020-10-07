//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : stdio.h
//    Module Funciton           : 
//                                Standard I/O libary header file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDIO_H__
#define __STDIO_H__

/* for size_t. */
#include "stddef.h"
/* for va list. */
#include "stdarg.h"

#if 0
typedef char* va_list;

#define _INTSIZEOF(n)   ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )
#endif

//Flags to control file seeking operation.
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define MAX_BUFFER_SIZE 512

//Kernel does not support floating point number yet.
#define NOFLOAT

/* Standard print routine of libc,hellox specific version. */
int _hx_sprintf(char* buf, const char* fmt, ...);
int _hx_printf(const char* fmt,...);
int _hx_vsprintf(char *buf, const char *fmt, va_list args);
int _hx_vfprintf(void* stream, const char* fmt, va_list args);
int _hx_snprintf(char* buf, size_t n, const char* fmt, ...);

/*
 * Can be called in any context include process of system
 * initialization.
 * It uses spin lock to protect the output device such as
 * VGA,thus several CPUs or threads can call it safety,
 * without worry about the outputs from different target
 * mixed together.
 * Avoid to using it since it wastes CPU cycles very much.
 */
int _hx_printk(const char* fmt, ...);

//How many space in a tab key.
#define TAB_SPACE_NUM 8

//Simulate standard printf routine.
#ifndef printf
#define printf _hx_printf
#endif

//Simulate standard sprintf routine.
#ifndef sprintf
#define sprintf _hx_sprintf
#endif

//vfprintf.
#ifndef vfprintf
#define vfprintf _hx_vfprintf
#endif

//snprintf.
#ifndef snprintf
#define snprintf _hx_snprintf
#endif

//printk.
#ifndef printk
#define printk _hx_printk
#endif

/*
 *  sscanf(buf,fmt,va_alist)
 */
int sscanf(const char *buf, const char *fmt, ...);

#endif //__STDIO_H__
