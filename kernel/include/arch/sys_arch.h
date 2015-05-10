//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : sys_arch.h
//    Module Funciton           : 
//                                OS simulation layer for lwIP,it is required
//                                by lwIP stack.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

//NULL value for kernel objects.
#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL  NULL

//Redefinition for HelloX's kernel objects,required by lwIP.
typedef __KERNEL_THREAD_OBJECT*  sys_thread_t;
typedef DWORD                    sys_prot_t;
typedef __COMMON_OBJECT*         sys_mbox_t;
typedef __COMMON_OBJECT*         sys_sem_t;

#endif /* __ARCH_SYS_ARCH_H__ */
