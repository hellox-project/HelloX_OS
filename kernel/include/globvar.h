//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : globvar.h
//    Module Funciton           : 
//                                This module countains the defination of
//                                global variables.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GLOBAL_VAR__
#define __GLOBAL_VAR__

#ifdef __cplusplus
extern "C" {
#endif

//Shell thread's handle.
extern __KERNEL_THREAD_OBJECT*   g_lpShellThread;

//Host name.
extern CHAR                      HostName[16];

#ifdef __cplusplus
extern "C" {
#endif

#endif //globvar.h
