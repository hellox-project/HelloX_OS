/***
*setjmp.h - definitions/declarations for setjmp/longjmp routines
*
*       Copyright (c) 1985-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the machine-dependent buffer used by
*       setjmp/longjmp to save and restore the program state, and
*       declarations for those routines.
*       [ANSI/System V]
*
*       [Public]
*
****/

#ifndef _INC_SETJMP
#define _INC_SETJMP

#include "__hxcomm.h"

#if _MSC_VER > 1000
#pragma once
#endif  /* _MSC_VER > 1000 */

#ifdef _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define setjmp  __setjmp
#define sigsetjmp __sigsetjmp
#define longjmp __longjmp

#define _JBLEN  16
#define _JBTYPE int

/*
 * Define jump buffer layout for x86 setjmp/longjmp.
 */
typedef struct __JUMP_BUFFER {
    unsigned long Ebp;
    unsigned long Ebx;
    unsigned long Edi;
    unsigned long Esi;
    unsigned long Esp;
    unsigned long Eip;
    unsigned long Registration;
    unsigned long TryLevel;
    unsigned long Cookie;
    unsigned long UnwindFunc;
    unsigned long UnwindData[6];
} _JUMP_BUFFER;

/* Define the buffer type for holding the state information */

#ifndef _JMP_BUF_DEFINED
typedef _JBTYPE jmp_buf[_JBLEN];
typedef _JBTYPE sigjmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif  /* _JMP_BUF_DEFINED */

/* Function prototypes */
int __CDECL setjmp(jmp_buf);
int __CDECL sigsetjmp(jmp_buf,int savings);
void __CDECL longjmp(jmp_buf env, int value);

//#if _MSC_VER >= 1200
//_CRTIMP __declspec(noreturn) void __cdecl longjmp(jmp_buf, int);
//#else  /* _MSC_VER >= 1200 */
//_CRTIMP void __cdecl longjmp(jmp_buf, int);
//#endif  /* _MSC_VER >= 1200 */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#ifdef _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_SETJMP */
