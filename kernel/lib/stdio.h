//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : L_STDIO.H
//    Module Funciton           : 
//                                Standard I/O libary header file.
//                                Please note it's name is a prefix "L_".
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

typedef char *  va_list;
typedef unsigned int     size_t;

//Flags to control file seeking operation.
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

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

