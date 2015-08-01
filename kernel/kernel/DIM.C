//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,25 2005
//    Module Name               : dim.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of Device
//                                Input Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include "ktmgr.h"
#include "commobj.h"
#include "dim.h"
#include "types.h"
#include "hellocn.h"
#include "kapi.h"
//
//The implementation of Initialize routine.
//
static BOOL DimInitialize(__COMMON_OBJECT* lpThis,
						  __COMMON_OBJECT* lpFocusThread,
						  __COMMON_OBJECT* lpShellThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;

	if(NULL == lpInputMgr)    //Parameter check.
	{
		return FALSE;
	}

	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;

	return TRUE;
}


//
//Send a device message to a specified kernel thread,or to the default shell thread
//if target thread is not specified.
//
static DWORD SendDeviceMessage(__COMMON_OBJECT*    lpThis,
							   __DEVICE_MESSAGE*   lpDevMsg,
							   __COMMON_OBJECT*    lpTarget)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr     = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_MESSAGE*   lpThreadMsg    = (__KERNEL_THREAD_MESSAGE*)lpDevMsg;
	DWORD                      dwFlags        = 0;

	if((NULL == lpInputMgr) || (NULL == lpThreadMsg))    //Parameter check.
	{
		return DEVICE_MANAGER_FAILED;
	}

	if(lpTarget != NULL)
	{
		KernelThreadManager.SendMessage(lpTarget,lpThreadMsg);
		return DEVICE_MANAGER_SUCCESS;
	}

	//The following code should not be interrupted.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(lpInputMgr->lpFocusKernelThread != NULL)  //DIM contains a valid current focus thread object.
	{
		if(KERNEL_THREAD_STATUS_TERMINAL == lpInputMgr->lpFocusKernelThread->dwThreadStatus)
		{
			//Clear the current focus thread since it's status is TERMINAL.
			lpInputMgr->lpFocusKernelThread = NULL;
			//Send to the default shell thread if exist.
			if(NULL != lpInputMgr->lpShellKernelThread)
			{
				KernelThreadManager.SendMessage((__COMMON_OBJECT*)(lpInputMgr->lpShellKernelThread),
					lpThreadMsg);
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return DEVICE_MANAGER_SUCCESS;
			}
			else    //The current shell kernel thread is not exists.
			{
				__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
				return DEVICE_MANAGER_NO_SHELL_THREAD;
			}
		}
		else  //Current focus kernel thread is not terminated,send the message to it.
		{
			KernelThreadManager.SendMessage((__COMMON_OBJECT*)lpInputMgr->lpFocusKernelThread,
				lpThreadMsg);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return DEVICE_MANAGER_SUCCESS;
		}
	}
	else //The current focus kernel thread is not exists,send to default shell thread.
	{
		if(NULL != lpInputMgr->lpShellKernelThread)
		{
			KernelThreadManager.SendMessage((__COMMON_OBJECT*)lpInputMgr->lpShellKernelThread,
				lpThreadMsg);
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return DEVICE_MANAGER_SUCCESS;
		}
		else
		{
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return DEVICE_MANAGER_NO_SHELL_THREAD;
		}
	}
}

//
//Set the current focus thread.
//
static __COMMON_OBJECT* SetFocusThread(__COMMON_OBJECT*  lpThis,
									   __COMMON_OBJECT*  lpFocusThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0;
	
	if(NULL == lpInputMgr)    //Parameter check.
	{
		return (__COMMON_OBJECT*)lpRetVal;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpRetVal                        = lpInputMgr->lpFocusKernelThread;
	lpInputMgr->lpFocusKernelThread = (__KERNEL_THREAD_OBJECT*)lpFocusThread;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}

//
//Set the default shell thread.Any device input message will be sent to this thread if no
//current focus thread exist.
//
static __COMMON_OBJECT* SetShellThread(__COMMON_OBJECT*  lpThis,
									   __COMMON_OBJECT*  lpShellThread)
{
	__DEVICE_INPUT_MANAGER*    lpInputMgr = (__DEVICE_INPUT_MANAGER*)lpThis;
	__KERNEL_THREAD_OBJECT*    lpRetVal   = NULL;
	DWORD                      dwFlags    = 0;
	
	if(NULL == lpInputMgr)    //Parameter check.
	{
		return (__COMMON_OBJECT*)lpRetVal;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpRetVal = lpInputMgr->lpShellKernelThread;
	lpInputMgr->lpShellKernelThread = (__KERNEL_THREAD_OBJECT*)lpShellThread;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (__COMMON_OBJECT*)lpRetVal;
}

/************************************************************************
*************************************************************************
*************************************************************************
*************************************************************************
************************************************************************/

//
//The definition of Global Object DeviceInputManager.
//
__DEVICE_INPUT_MANAGER DeviceInputManager = {
	NULL,                                     //lpFocusKernelThread.
	NULL,                                     //lpShellKernelThread.
	SendDeviceMessage,                        //SendDeviceMessage routine.
	SetFocusThread,                           //SetFocusThread routine.
	SetShellThread,                           //SetShellThread routine.
	DimInitialize                             //Initialize routine.
};
