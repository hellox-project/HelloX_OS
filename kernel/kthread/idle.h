//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 18 DEC, 2011
//    Module Name               : idle.h
//    Module Funciton           : 
//                                This module contains the IDLE thread declaration code.IDLE thread
//                                is one of the kernel level threads and will be scheduled when no any
//                                other thread need CPU.
//                                Auxiliary functions such as battery management will also be placed in
//                                this thread.
//                                These code lines are moved from os_entry.cpp file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __IDLE_H__
#define __IDLE_H__

DWORD SystemIdle(LPVOID lpData);

#endif  //__IDLE_H__
