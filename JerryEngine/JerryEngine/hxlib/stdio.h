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

#include "__hxcomm.h"
#include "kapi.h"

#ifndef __VA_LIST_DEFINED__
#define __VA_LIST_DEFINED__
typedef char *  va_list;
#endif

#ifndef __SIZE_T_DEFINED__
#define __SIZE_T_DEFINED__
typedef unsigned int size_t;
#endif

//Flags to control file seeking operation.
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#ifndef __VA_START_DEFINED__
#define __VA_START_DEFINED__
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#endif //__VA_START_DEFINED__

#ifndef __VA_ARG_DEFINED__
#define __VA_ARG_DEFINED__
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#endif //__VA_ARG_DEFINED__

#ifndef __VA_END_DEFINED__
#define __VA_END_DEFINED__
#define va_end(ap)      ( ap = (va_list)0 )
#endif //__VA_END_DEFINED__

#define MAX_BUFFER_SIZE 512

#define NOFLOAT  //Kernel does not support floating point number yet.

int _hx_sprintf(char* buf,const char* fmt,...);
int _hx_printf(const char* fmt,...);
int _hx_vsprintf(char *buf, const char *fmt, va_list args);
int _hx_vfprintf(void* stream,const char* fmt,va_list args);

int _hx_snprintf(char* buf,size_t n,const char* fmt,...);

#define TAB_SPACE_NUM 8  //How many space in a tab key.

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

#endif //__STDIO_H__
