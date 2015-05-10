//***********************************************************************/
//    Author                    : Erwin
//    Original Date             : 29th May, 2014
//    Module Name               : logcat.h
//    Module Funciton           : 
//                                This module contains the logcat daemon thread declaration code.the logcat daemon thread
//                                is one of the kernel level threads and will print the log messages from other threads to the console.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __LOGCAT_H__
#define __LOGCAT_H__

DWORD TAEntry(LPVOID pData);
DWORD TBEntry(LPVOID pData);
DWORD TDEntry(LPVOID pData);
DWORD TCEntry(LPVOID pData);
DWORD LogcatDaemon(LPVOID pData);

#endif  //__IDLE_H__
