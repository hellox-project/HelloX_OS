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

#ifndef __STRING_H__
#define __STRING_H__

/* For basic HelloX data types. */
#include "hellox.h"

/* Maximal string's length. */
#define MAX_STRING_LEN 512

//Memory manipulating functions.
void* memcpy(void* dst, const void* src, size_t count);
void* memset(void* dst, int val, size_t count);
void* memzero(void* dst, size_t count);
int   memcmp(const void* p1, const void* p2, int count);
void* memchr(const void * buf, int chr, size_t cnt);
void *memmove(void *dst, const void *src, int n);

//Standard C Lib string operations.
char* strcat(char* dst, const char* src);
char* strcpy(char* dst, const char* src);
//char* strchr(const char* string,int ch);

char * strchr(const char *s, int c_in);
char * strrchr(const char * str, int ch);
char * strstr(const char *s1, const char *s2);

int strcmp(const char* src, const char* dst);
int strlen(const char* s);

//Array bound guaranteed string operations.
char* strncpy(char *dest, const char *src, unsigned int n);
int strncmp(char * s1, char * s2, size_t n);

//Flags to control the trimming.
#define TRIM_LEFT    0x1
#define TRIM_RIGHT   0x2

//Trim space in a string.
void strtrim(char * dst, int flag);

int strtol(const char *nptr, char **endptr, int base);

char*  strtok(char* string_org, const char* demial);

void ToCapital(LPSTR lpszString);

//Find the first bit in a given integer.
int ffs(int x);

#endif //__STRING_H__
