//***********************************************************************/
//    Author                    : Erwin
//    Original Date             : 29th May, 2014
//    Module Name               : logcat.c
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

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include <kapi.h>
#include <debug.h>
#include <console.h>
#include <ktmgr.h>


//Only available when logging service is enabled.
#ifdef __CFG_SYS_LOGCAT

DWORD LogcatDaemon(LPVOID pData)
{
	char buf[256] = {'0'};
	while(TRUE)
	{
		DebugManager.Logcat(&DebugManager, buf, 0);	
		if(buf[0] != '0')
		{
			if(Console.bInitialized && Console.bLLInitialized)
			{
				Console.PrintLine(buf);
			}
		}
		KernelThreadManager.Sleep((__COMMON_OBJECT *)&KernelThreadManager, 500);
	}

}

#endif //__CFG_SYS_LOGCAT.
