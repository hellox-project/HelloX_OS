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

#include "__hxcomm.h"
#include "stddef.h"
#include "ptmalloc/malloc.h"

/**
 * Generate random number,the standard C routine.
 * Here we just simulate it,since it should be supported
 * by hardware to generate TRUE randomly number.
 */
#define RAND_MAX 0x7FFFFFFF

long rand();
void srand(unsigned long seed);

//Environment related routine.
char *_hx_getenv(char *envvar);

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
char* itoa(int value, char *buf, int radix);

/*
 *  atob(vp,p,base)
 *      converts p to binary result in vp, rtn 1 on success
 */
int
atob(uint32_t *vp, char *p, int base);

/*
 *  char *btoa(dst,value,base)
 *      converts value to ascii, result in dst
 */
char *
btoa(char *dst, unsigned int value, int base);

/*
 *  gethex(vp,p,n)
 *      convert n hex digits from p to binary, result in vp,
 *      rtn 1 on success
 */
int
gethex(int32_t *vp, char *p, int n);

/*
 * The C library function double strtod(const char *str, char **endptr)
 * converts the string pointed to by the argument str to a floating - point
 * number(type double).If endptr is not NULL, a pointer to the character
 * after the last character used in the conversion is stored in the
 * location referenced by endptr.
 */
double strtod(const char *str, char **endptr);

/* strtoul function. */
unsigned long strtoul(const char* nptr, char** endptr, int base);

//exit and abort.
void exit(int state);
void abort(void);

/* Flags for exit. */
#define EXIT_SUCCESS ( 0)
#define EXIT_FAILURE (-1)

/* qsort. */
void qsort(void* a, size_t n, size_t es, int(*cmp)());

//Standard C routine,alias of HX version.
//#ifndef malloc
#define malloc _hx_malloc
//#endif

#ifndef calloc
#define calloc _hx_calloc
#endif

#ifndef free
#define free _hx_free
#endif

#ifndef alloca
#define alloca _hx_alloca
#endif

#ifndef realloc
#define realloc _hx_realloc
#endif

#ifndef getenv
#define getenv _hx_getenv
#endif

#ifndef aligned_malloc
#define aligned_malloc _hx_aligned_malloc
#endif

#ifndef memalign
#define memalign(align,size) _hx_aligned_malloc(size,align)  //Parameter order is different.
#endif

#ifndef aligned_free
#define aligned_free _hx_free
#endif

#endif  //__STDLIB_H__
