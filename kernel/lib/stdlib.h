//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : stdlib.h
//    Module Funciton           : 
//                                Stdand C library simulation code.It only implements a subset
//                                of C library,even much more simple.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifndef __STDDEF_H__
#include "stddef.h"
#endif

//Standard C malloc/free/calloc routine.A dedicated prefix _hx_ is appended
//before the standard name to distinguish the HX version and standard version.
//The intention is to lead compiler link the code to the right version.It will
//lead conflict if we use standard name here,in some developing environment
//that has it's own lib source.
void* _hx_malloc(size_t size);
void  _hx_free(void* p);
void* _hx_calloc(size_t n,size_t s);
void* _hx_alloca(size_t size);  //Allocate memory from STACK.

//Environment related routine.
char *_hx_getenv(char *envvar);

//mmap and munmap routine to simulate the Linux API.
void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
int munmap(void* start,size_t length);

//Flags to control the mmap routine.
#define PROT_READ      0x00000001
#define PROT_WRITE     0x00000002

//Map flags.
#define MAP_PRIVATE   0x00000004
#define MAP_ANON      0x00000008

//Failed return value of mmap.
#define MAP_FAILED     NULL

//Convert a string to long.
long atol(const char *nptr);

//Convert a string to int.
int atoi(const char *nptr);

//Convert a int to string.
char* itoa(int value,char *buf,int radix);

//exit and abort.
void exit(int state);
void abort(void);

//Simulate standard C malloc and free routine.
//#ifndef malloc
#define malloc _hx_malloc
//#endif


#ifndef free
#define free _hx_free
#endif

#ifndef alloca
#define alloca _hx_alloca
#endif

#ifndef getenv
#define getenv _hx_getenv
#endif

#endif  //__STDLIB_H__
