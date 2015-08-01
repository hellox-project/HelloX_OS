//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : TYPES.H
//    Module Funciton           : 
//                                This module countains the definition for several types
//                                supported by Hello China.
//                                The operating interface,i.e,the function that operate these
//                                data structures,also be defined here.
//    Last modified Author      : Garry
//    Last modified Date        : Jun 4,2013
//    Last modified Content     :
//                                1. All basic data types are moved into this file from hellocn.h;
//                                2. Redefined the essential data type 'BYTE',changed to
//                                   'unsigned char' from 'signed char';
//                                3. All basic data types are redefined by 'typedef' key
//                                   word instead of 'define'.
//    Lines number              :
//***********************************************************************/

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

//
//Hello China's basic data type.
//

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned long       BOOL;

typedef char                CHAR;
typedef short               SHORT;
typedef int                 INT;
typedef unsigned char       UCHAR;
typedef short               WCHAR;
typedef short               TCHAR;

typedef unsigned long       ULONG;
typedef unsigned int        UINT;
typedef unsigned short      USHORT;
typedef unsigned char       UCHAR;

typedef double              DOUBLE;
typedef float               FLOAT;

typedef char*               LPSTR;
typedef const char*         LPCTSTR;
typedef const char*         LPCSTR;

typedef void                VOID;
typedef void*               LPVOID;

//Extra definitions maybe used by some GPL code,or you can
//add your specific data type definitions here to comply your
//code requirement,such as __uint8,...
typedef unsigned char       __U8;
typedef unsigned short      __U16;
typedef unsigned long       __U32;

//size_t for ANSI C library.
typedef unsigned int size_t;

//Special values for basic data types defined above.
#define FALSE               0x00000000
#define TRUE                0x00000001

#define S_OK                0x00000000

#define NULL                ((void*)0x00000000)

#define MAX_DWORD_VALUE     0xFFFFFFFF
#define MAX_WORD_VALUE      0xFFFF
#define MAX_BYTE_VALUE      0xFF
#define MAX_QWORD_VALUE     0xFFFFFFFFFFFFFFFF

//Macros to simplify programming.
#define LOWORD(dw)          WORD(dw)
#define HIWORD(dw)          WORD(dw >> 16)

#define LOBYTE(wr)          BYTE(wr)
#define HIBYTE(wr)          BYTE(wr >> 16)

//
//The definition of unsigned 64 bit integer.
//
#define U64_ZERO {0,0}
#define U64_MAX  {0xFFFFFFFF,0xFFFFFFFF}

typedef struct{
	unsigned long dwLowPart;
	unsigned long dwHighPart;
}__U64;


//
//Add operations for unsigned 64 bits integer.
//lpu64_result = lpu64_1 + lpu64_2.
//
VOID u64Add(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Subtract operations for U64.
//lpu64_result = lpu64_1 - lpu64_2.
//
VOID u64Sub(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Compare operations for unsigned 64 bits integer.
//
BOOL EqualTo(__U64* lpu64_1,__U64* lpu64_2);    //TRUE if lpu64_1 == lpu64_2.
BOOL LessThan(__U64* lpu64_1,__U64* lpu64_2);   //TRUE if lpu64_1 < lpu64_2.
BOOL MoreThan(__U64* lpu64_1,__U64* lpu64_2);   //TRUE if lpu64_1 > lpu64_2.
VOID u64Div(__U64*,__U64*,__U64*,__U64*);

//
//Shift operation.
//
VOID u64RotateLeft(__U64* lpu64_1,DWORD dwTimes);  //Shift dwTimes bit(s) of lpu64_1 to left.
VOID u64RotateRight(__U64* lpu64_1,DWORD dwTimes); //Shift dwTimes bit(s) of lpu64_1 to right.

//
//Multiple operations.
//
// VOID u64Mul(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//
//Divide operations.
//
//VOID u64Div(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);

//Convert a 64 bits integer to string.
BOOL u64Hex2Str(__U64* lpu64,LPSTR lpszResult);

#ifdef __cplusplus
}
#endif

#endif  //__TYPES_H__
