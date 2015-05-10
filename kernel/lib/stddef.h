//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 22,2014
//    Module Name               : stddef.h
//    Module Funciton           : 
//                                Stdand C library simulation code.It only implements a subset
//                                of C library,even much more simple.
//    Last modified Author      :
//    Last modified Date        : Jun 22,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDDEF_H__
#define __STDDEF_H__

//Definition of standard size_t type.
typedef unsigned int size_t;

//Definition of standard offset_t type.
typedef int offset_t;
typedef int off_t;

//uid_t.
typedef int uid_t;

//pid_t.
typedef int pid_t;

//Definition of standard wchar_t type.
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif  /* _WCHAR_T_DEFINED */

#endif
