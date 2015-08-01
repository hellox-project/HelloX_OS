//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb,25 2005
//    Module Name               : dim.h
//    Module Funciton           : 
//                                This module countains the pre-definition of Device
//                                Input Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DIM_H__
#define __DIM_H__


#include "ktmgr.h"


#ifdef __cplusplus
extern "C" {
#endif

//
//The pro-type's definition of Device Input Message.
//The hardware driver(s) use this structure to describe a message.
//
BEGIN_DEFINE_OBJECT(__DEVICE_MESSAGE)
    WORD           wDevMsgType;
    WORD           wDevMsgParam;
	DWORD          dwDevMsgParam;
END_DEFINE_OBJECT(__DEVICE_MESSAGE)

//
//The pro-type's definition of Device Input Manager.
//
BEGIN_DEFINE_OBJECT(__DEVICE_INPUT_MANAGER)
    __KERNEL_THREAD_OBJECT*    lpFocusKernelThread;
    __KERNEL_THREAD_OBJECT*    lpShellKernelThread;

	DWORD            (*SendDeviceMessage)(__COMMON_OBJECT*    lpThis,
		                                  __DEVICE_MESSAGE*   lpDevMsg,
							              __COMMON_OBJECT*    lpTarget);
	
	__COMMON_OBJECT* (*SetFocusThread)(__COMMON_OBJECT* lpThis,
		                               __COMMON_OBJECT* lpFocusThread);

	__COMMON_OBJECT* (*SetShellThread)(__COMMON_OBJECT* lpThis,
		                               __COMMON_OBJECT* lpShellThread);

	BOOL             (*Initialize)(__COMMON_OBJECT*    lpThis,
		                           __COMMON_OBJECT*    lpFocusThread,
								   __COMMON_OBJECT*    lpShellThread);
END_DEFINE_OBJECT(__DEVICE_INPUT_MANAGER)

//
//The definition of return values for above routine.
//

#define DEVICE_MANAGER_SUCCESS                0x00000001
#define DEVICE_MANAGER_FAILED                 0x00000000
#define DEVICE_MANAGER_NO_FOCUS_THREAD        0x00000002
#define DEVICE_MANAGER_NO_SHELL_THREAD        0x00000004

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The pre-definition of Global Object: Device Input Manager.
//

extern __DEVICE_INPUT_MANAGER DeviceInputManager;

#ifdef __cplusplus
}
#endif


#endif
