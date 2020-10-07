//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,28 2004
//    Module Name               : string.h
//    Module Funciton           : 
//                                This module and string.cpp countains the
//                                string operation method.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STRING__
#define __STRING__

#include "__hxcomm.h"
#include "stddef.h"  //For size_t definition.

//Maximal length of a string buffer.
#define MAX_STRING_LEN 512

//Memory manipulating functions.
void* memcpy(void* dst,const void* src,size_t count);
void* memset(void* dst,int val,size_t count);
void* memzero(void* dst,size_t count); 
int   memcmp(const void* p1,const void* p2,int count);
void* memchr (const void * buf,int chr,size_t cnt);
void *memmove(void *dst,const void *src,int n);

//Standard C Lib string operations.
char* strcat(char* dst,const char* src);
char* strcpy(char* dst,const char* src);
//char* strchr(const char* string,int ch);

char * strchr (const char *s, int c_in);
char * strrchr(const char * str,int ch);
char * strstr(const char *s1,const char *s2);

int strcmp(const char* src,const char* dst);
int strlen(const char* s);

//Array bound guaranteed string operations.
char* strncpy(char *dest,char *src,unsigned int n);
int strncmp ( char * s1, char * s2, size_t n);

//Flags to control the trimming.
#define TRIM_LEFT    0x1
#define TRIM_RIGHT   0x2

//Trim space in a string.
void strtrim(char * dst,int flag);

int strtol(const char *nptr, char **endptr, int base);

char* strtok(char* string_org, const char* demial);
size_t strcspn(const char* s1, register const char* s2);

/* Duplicate S, returning an identical malloc'd string.  */
char * __strdup(const char *s);
#define _strdup __strdup
#define strdup  __strdup

//Find the first bit in a given integer.
int ffs(int x);

#endif //string.h
