//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 12,2009
//    Module Name               : guientry.cpp
//    Module Funciton           : 
//                                Entry point code for GUI module.The code in this
//                                module initializes all GUI kernel global objects
//                                and variables,then start Raw Input Thread(RAWIT),
//                                GUI shell and other related kernel threads.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include ".\INCLUDE\KAPI.H"
#include ".\INCLUDE\stdio.h"
#include ".\INCLUDE\string.h"

#include ".\INCLUDE\VESA.H"
#include ".\INCLUDE\VIDEO.H"
#include ".\INCLUDE\GLOBAL.H"
#include ".\INCLUDE\CLIPZONE.H"
#include ".\INCLUDE\GDI.H"
#include ".\INCLUDE\WNDMGR.H"
#include ".\INCLUDE\RAWIT.H"
#include ".\INCLUDE\GUISHELL.H"
#include ".\syscall\syscall.h"

//Entry point of GUI module.This routine does the following tasks:
// 1. Switch to graphic mode(1024*768*32),failure will lead GUI mode fail to load;
// 2. Initializes the global video object - Video;
// 3. Create the RAWIT thread to handle any input in GUI mode;
// 4. Create GUI shell thread to display GUI shell;
// 5. Wait for GUI shell thread to terminal;
// 6. Switch back to text mode if user terminates the GUI shell.
//The pVbeInfo information block is initialized when switching to graphic mode.
//
extern "C"{
DWORD __GUIEntry(LPVOID)
{
	HANDLE     hRawInputThread   = NULL;  //RAW input thread handle.
	HANDLE     hLastFocusThread  = NULL;  //Hold last focusable thread.
	HANDLE     hGUIShell         = NULL;  //Shell thread in GUI mode.
	BOOL       bResult           = FALSE;
	BOOL       bVideoOK          = FALSE;

	if(!SwitchToGraphic())  //Can not switch to graphic mode.
	{
		PrintLine("  Can not switch to graphic mode.");
		PrintLine("  Please make sure the following graphic display mode is available:");
		PrintLine("  Demension: 1024 * 768,32 bits true color.");
		goto __TERMINAL;
	}

	//Initialize the floating point cooprocessor.
	__asm{
		finit
	}

	//Initialize the Video object.
	if(!Video.Initialize(&Video))
	{
		bVideoOK = FALSE;
		goto __TERMINAL;
	}
	bVideoOK = TRUE;
	//Initialize the GlobalParams object,system level variables are held by this object.
	if(!GlobalParams.Initialize(&GlobalParams,&Video))
	{
		goto __TERMINAL;
	}
	//Initialize the WindowManager object.
	if(!WindowManager.Initialize(&WindowManager))
	{
		goto __TERMINAL;
	}

	//Register system calls for GUI module.
	if(!RegisterSystemCall(SYSCALL_GUI_BEGIN,SYSCALL_GUI_END,SyscallHandler))
	{
		goto __TERMINAL;
	}

	//Now create RAWIT thread to receive and dispatch all events(input) in GUI mode.
	hRawInputThread = CreateKernelThread(
		0,  //Use default stack size.
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		RAWIT,
		NULL,
		NULL,
		"GUIRAWIT");
	if(NULL == hRawInputThread)  //Can not create the RAW input thread.
	{
		goto __TERMINAL;
	}
	//Set the RAW input thread as current input focus thread,so all external input,such as
	//keyboard,mouse,will be dispatched to RAW input thread.
	hLastFocusThread = SetFocusThread(hRawInputThread);
	GlobalParams.hRawInputThread = hRawInputThread;

	//Now Create GUI shell thread.
	hGUIShell = CreateKernelThread(
		16384,  //Use default stack size,it's 16K.
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,  //Normal priority.
		GuiShellEntry,
		NULL,
		NULL,
		"GUIShell");
	if(NULL == hGUIShell)
	{
		goto __TERMINAL;
	}
	GlobalParams.hGUIShell = hGUIShell;
	//Wait for GUI mode key threads to run over.
	WaitForThisObject(hRawInputThread);
	WaitForThisObject(hGUIShell);

	//Now GUI Shell is over,should return to TEXT mode.
	//Should set current focus thread as the last one.
	SetFocusThread(hLastFocusThread);

	bResult = TRUE;

__TERMINAL:
	//Destroy all kernel threads in GUI mode.
	if(hRawInputThread)
	{
		DestroyKernelThread(hRawInputThread);
	}
	if(hGUIShell)
	{
		DestroyKernelThread(hGUIShell);
	}
	//Should unregister system call.
	//UnregisterSystemCall(SYSCALL_GUI_BEGIN,SYSCALL_GUI_END);

	//Uninitialize all global objects in GUI mode.
	Video.Uninitialize(&Video);
	WindowManager.Uninitialize(&WindowManager);
	//Switch back to text mode.
	SwitchToText();
	if(!bVideoOK)  //Video initialization failed.
	{
		PrintLine("  GUI: Video initialization failed,only 1024 * 768 * 32 supported.");
	}
	return bResult;
}
}
