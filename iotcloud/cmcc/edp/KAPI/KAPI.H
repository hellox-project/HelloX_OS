//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.H
//    Module Funciton           : 
//                                Declares all kernel service routines can be
//                                used by other modules in kernel.
//                                This file is used by user application,so all
//                                routines declared in it is system calls.
//
//    Last modified Author      : Garry
//    Last modified Date        : Jan 09,2012
//    Last modified Content     :
//                                1. System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#define __KAPI_H__

#include "K_API_DEF.H"

//Types.
typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef char            CHAR;
typedef short           WCHAR;
typedef unsigned short  uint16;
typedef char*           LPSTR;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef signed short    SHORT;
typedef int             INT;
typedef unsigned int    UINT;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef long            LONG;
typedef unsigned long   BOOL;
typedef void*           LPVOID;
typedef void            VOID;
typedef LPVOID          HANDLE;                             //Use handle to refer any kernel object.

typedef unsigned char   uint8;
typedef char            int8;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned int    uint32;
typedef int             int32;
typedef unsigned int    size_t;
typedef long time_t;

//If WIDE char is used.
#ifdef __WIDE_CHAR
#define TCHAR WCHAR
#else
#define TCHAR CHAR
#endif

//Declare as wide character or wide char string.
#define WIDE_CHAR L
#define WIDE_STR  L

//Constants.
#define MAX_DWORD_VALUE    0xFFFFFFFF
#define TRUE               0xFFFFFFFF
#define FALSE              0x00000000
#define NULL               0
#define MAX_FILE_NAME_LEN  256

#define MOUSE_DIM_X   511  //Maximal x dimension of mouse.
#define MOUSE_DIM_Y   511  //y dimension.

//The following macros are defined to simply the programming.
#define SYSCALL_PARAM_0(num) \
	{                \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	}

#define SYSCALL_PARAM_1(num,p1) \
	{                \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	}

#define SYSCALL_PARAM_2(num,p1,p2) \
	{                \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	}

#define SYSCALL_PARAM_3(num,p1,p2,p3) \
	{                \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	}

#define SYSCALL_PARAM_4(num,p1,p2,p3,p4) \
	{                \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	}                

#define SYSCALL_PARAM_5(num,p1,p2,p3,p4,p5) \
	{                \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	}

#define SYSCALL_PARAM_6(num,p1,p2,p3,p4,p5,p6) \
	{                \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	}

#define SYSCALL_PARAM_7(num,p1,p2,p3,p4,p5,p6,p7) \
	{                \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
	}

#define SYSCALL_PARAM_8(num,p1,p2,p3,p4,p5,p6,p7,p8) \
	{                \
	__asm push p8    \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
	__asm pop p8     \
	}

#define SYSCALL_PARAM_9(num,p1,p2,p3,p4,p5,p6,p7,p8,p9) \
	{                \
	__asm push p9    \
	__asm push p8    \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
	__asm pop p8     \
	__asm pop p9     \
	}

#define SYSCALL_PARAM_10(num,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10) \
	{                \
	__asm push p10   \
	__asm push p9    \
	__asm push p8    \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
	__asm pop p8     \
	__asm pop p9     \
	__asm pop p10    \
	}

#define SYSCALL_PARAM_11(num,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11) \
	{                \
	__asm push p11   \
	__asm push p10   \
	__asm push p9    \
	__asm push p8    \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
	__asm pop p8     \
	__asm pop p9     \
	__asm pop p10    \
	__asm pop p11    \
	}



#endif //__KAPI_H__
