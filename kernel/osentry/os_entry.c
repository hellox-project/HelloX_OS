//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 28 DEC, 2006
//    Module Name               : OS_ENTRY.cpp
//    Module Funciton           : 
//                                This module contains the OS initializating code,include but not limited:
//                                1. Global objects initialization;
//                                2. Enable paging and other system mechanism;
//                                3. Load drivers and modules;
//                                4. Create kernel level thread(s) and switch to them.
//
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 2013.06.04
//    Last modified Content     : 
//                                1. Re-wrote some codes in this file to make it looks well,added pre-defined
//                                   controlling switches to implement condition compiling;
//                                2. User entry mechanism is implemented,i.e,a /user directory has been added
//                                   to root and a fixed routine name,'_HCNMain',has been defined and called in
//                                   OS's initialization process,to isolate the user code and system code.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "statcpu.h"
#include "modmgr.h"
#include "console.h"
#include "lwip/tcpip.h"
#include "stdio.h"
#include "buffmgr.h"

#include "../shell/shell.h"
#include "../shell/stat_s.h"
#include "../kthread/idle.h"
#include "../network/ethernet/ethif.h"

#include "../kthread/logcat.h"
#include "ktmgr.h"

#ifdef __I386__
#include "../arch/x86/biosvga.h"
#endif

//Welcome information.
char* pszStartMsg1 = "HelloX is running now.If you have any question,";
char* pszStartMsg2 = "please send email to : garryxin@yahoo.com.cn.";
char* pszWelcome   = "Welcome to use HelloX!";

//Help information.
char* pszHelpInfo = "Any help please press 'help' + return.";

//Driver entry point array,this array resides in drventry.cpp file in the 
//same directory as os_entry.cpp,which is OSENTRY in current version.
extern __DRIVER_ENTRY DriverEntryArray[];

//A dead loop routine.
//static
static void DeadLoop(BOOL bDisableInt)
{
	//DWORD dwFlags;
	PrintLine("loop..."); GotoHome();
	if (bDisableInt)
	{
		//__ENTER_CRITICAL_SECTION(NULL, dwFlags);
		while (TRUE){
		}
		//__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	}
	else
	{
		while (TRUE){
		}
	}
}

//User entry point if used as EOS.
#ifdef __CFG_USE_EOS
extern DWORD _HCNMain(LPVOID);
#endif

//
//The main entry of OS.When the OS kernel is loaded into memory and all hardware
//context is initialized OK,Hello China OS's kernel will run from here.
//This is a never finish(infinite) routine and will never end unless powers off 
//the system or reboot the system.
//It's main functions are initializing all system level objects and modules,loading kernel
//mode hardware drivers,loading external function modules(such as GUI and network),then
//creating several kernel thread(s) and entering a dead loop.
//But the dead loop codes only run a short time since the kernel thread(s) will be 
//scheduled once system clock occurs and the dead loop will end.
//

void __OS_Entry()
{
	__KERNEL_THREAD_OBJECT*       lpIdleThread     = NULL;
	__KERNEL_THREAD_OBJECT*       lpShellThread    = NULL;
	__KERNEL_THREAD_OBJECT*		lpLogcatDaemonThread = NULL;
#ifdef __CFG_USE_EOS
	__KERNEL_THREAD_OBJECT*       lpUserThread     = NULL;
#endif
	DWORD                         dwIndex          = 0;
	CHAR                          strInfo[64];
	char*                         pszErrorMsg      = "INIT: OK,everything is done.";


	//Initialize display device under PC architecture,since the rest output will rely on this.
#ifdef __I386__
	InitializeVGA();
#endif

	//Print out welcome message.
	//Please note the output should put here that before the System.BeginInitialization routine,
	//since it may cause the interrupt enable,which will lead the failure of system initialization.
	ClearScreen();
	PrintStr(pszStartMsg1);
	PrintStr(pszStartMsg2);
	GotoHome();
	ChangeLine();

	PrintStr(pszHelpInfo); //Print out help information.
	GotoHome();
	ChangeLine();

	//Prepare the OS initialization environment.It's worth noting that even the System
	//object self is not initialized yet.

	if(!System.BeginInitialize((__COMMON_OBJECT*)&System))
	{
		pszErrorMsg = "INIT ERROR: System.BeginInitialization routine failed.";
		goto __TERMINAL;
	}

	//Initialize memory management object.This object must be initialized before any other
	//system level objects since it's function maybe required by them.
	if(!AnySizeBuffer.Initialize(&AnySizeBuffer))
	{
		pszErrorMsg = "INIT ERROR: Failed to initialize AnySizeBuffer object.";
		goto __TERMINAL;
	}

#ifdef __CFG_SYS_VMM  //Enable VMM.
	*(__PDE*)PD_START = NULL_PDE;    //Set the first page directory entry to NULL,to indicate
	//this location is not initialized yet.
#endif

	//********************************************************************************
	//
	//The following code initializes system level global objects.
	//
	//********************************************************************************

#ifdef __CFG_SYS_VMM    //Should enable virtual memory model.

	lpVirtualMemoryMgr = (__VIRTUAL_MEMORY_MANAGER*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER);    //Create virtual memory manager object.
	if(NULL == lpVirtualMemoryMgr)    //Failed to create this object.
	{
		pszErrorMsg = "INIT ERROR: Can not create VirtualMemoryManager object.";
		goto __TERMINAL;
	}

	if(!lpVirtualMemoryMgr->Initialize((__COMMON_OBJECT*)lpVirtualMemoryMgr))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize VirtualMemoryManager object.";
		goto __TERMINAL;
	}
#endif


	if(!ProcessManager.Initialize((__COMMON_OBJECT*)&ProcessManager))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize ProcessManager object.";
		goto __TERMINAL;
	}

	//Initialize Kernel Thread Manager object.
	//Initialize the process manager object.

	if(!KernelThreadManager.Initialize((__COMMON_OBJECT*)&KernelThreadManager))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize KernelThreadManager object.";
		goto __TERMINAL;
	}

	//Initialize System object.
	if(!System.Initialize((__COMMON_OBJECT*)&System))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize System object.";
		goto __TERMINAL;
	}

	//Set the general interrupt handler,after this action,interrupt and
	//exceptions in system can be handled by System object.
	//This routine must be called earlier than any modules who will use
	//system call.
#ifdef __I386__
	SetGeneralIntHandler(GeneralIntHandler);
#endif

#ifdef __CFG_SYS_VMM
	//Initialize PageFrmaeManager object.
	if(!PageFrameManager.Initialize((__COMMON_OBJECT*)&PageFrameManager,
		(LPVOID)0x02000000,
		(LPVOID)0x09FFFFFF))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize PageFrameManager object.";
		goto __TERMINAL;
	}
#endif

	//Device Driver Framework related global functions.
#ifdef __CFG_SYS_DDF
	//Initialize IOManager object.
	if(!IOManager.Initialize((__COMMON_OBJECT*)&IOManager))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize IOManager object.";
		goto __TERMINAL;
	}

	//Initialize DeviceManager object.
	if(!DeviceManager.Initialize(&DeviceManager))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize DeviceManager object.";
		goto __TERMINAL;
	}
#endif

	//Initialize CPU statistics object.
#ifdef __CFG_SYS_CPUSTAT
	if(!StatCpuObject.Initialize(&StatCpuObject))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize StatCpuObject.";
		goto __TERMINAL;
	}
#endif

	//Enable the virtual memory management mechanism if __CFG_SYS_VMM flag is defined.
#ifdef __CFG_SYS_VMM
	EnableVMM();
#endif

	//Initialize Ethernet Manager if it is enabled.
#ifdef __CFG_NET_ETHMGR

	if(!EthernetManager.Initialize(&EthernetManager))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize Ethernet Manager.\r\n";
		goto __TERMINAL;
	}
#endif

	//********************************************************************************
	//
	//The following code loads all inline device drivers,and external drivers implemented as
	//external module.
	//
	//********************************************************************************
#ifdef __CFG_SYS_DDF
	dwIndex = 0;
	while(DriverEntryArray[dwIndex])
	{
		if(!IOManager.LoadDriver(DriverEntryArray[dwIndex])) //Failed to load.
		{
			_hx_sprintf(strInfo,"Failed to load the %d th driver.", dwIndex); //Show an error.
			PrintLine(strInfo);
		}
		dwIndex++;  //Continue to load.
	}
#endif

	//Initialize Console object if necessary.
#ifdef __CFG_SYS_CONSOLE

	if(!Console.Initialize(&Console))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize Console object.";
		goto __TERMINAL;
	}
#endif

	//********************************************************************************
	//
	//The following code creates all system level kernel threads.
	//
	//********************************************************************************

	//The first one is IDLE thread,which will be scheduled when no thread need to schedule,
	//so it's priority is the lowest one in system,which is PRIORITY_LEVEL_LOWEST.
	//Also need to mention that this thread is mandatory and without any switch to turn off
	//it.

	lpIdleThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOWEST,
		SystemIdle,
		NULL,
		NULL,
		"IDLE");
	if(NULL == lpIdleThread)
	{
		pszErrorMsg = "INIT ERROR: Can not create SystemIdle kernel thread.";
		goto __TERMINAL;
	}
	//Disable suspend on this kernel thread,since it may lead system crash.
	KernelThreadManager.EnableSuspend((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpIdleThread,
		FALSE);

	//Create statistics kernel thread.
#ifdef __CFG_SYS_CPUSTAT

	lpStatKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,  //With high priority.
		StatThreadRoutine,
		NULL,
		NULL,
		"CPU STAT");
	if(NULL == lpStatKernelThread)
	{
		pszErrorMsg = "INIT ERROR: Can not create CPU_Stat kernel thread.";
		goto __TERMINAL;
	}
#endif

	//Create shell thread.The shell thread's implementation code resides in shell.cpp
	//file in shell directory.
#ifdef __CFG_SYS_SHELL  //Shell can be eleminated by turn off this switch.
	if(NULL == ModuleMgr.ShellEntry)  //Use default shell.
	{

		lpShellThread = KernelThreadManager.CreateKernelThread(   //Create shell thread.
			(__COMMON_OBJECT*)&KernelThreadManager,
			0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_HIGH,
			ShellEntryPoint,
			NULL,
			NULL,
			"SHELL");
		if(NULL == lpShellThread)
		{
			pszErrorMsg = "INIT ERROR: Can not create Shell kernel thread.";
			goto __TERMINAL;
		}
	}
	else    //Use other kernel module specified shell.
	{
		lpShellThread = KernelThreadManager.CreateKernelThread(   //Create shell thread.
			(__COMMON_OBJECT*)&KernelThreadManager,
			0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_NORMAL,
			ModuleMgr.ShellEntry,
			NULL,
			NULL,
			"SHELL");
		if(NULL == lpShellThread)
		{
			pszErrorMsg = "INIT ERROR: Can not create Shell kernel thread.";
			goto __TERMINAL;
		}
	}
	g_lpShellThread = lpShellThread;     //Initialize the shell thread global variable.
	//Print out the default system prompt,which can be changed by 'sysname' command.
	//strcpy(&HostName[0],"[system-view]");
#endif

	//Initialize DeviceInputManager object.
	if(!DeviceInputManager.Initialize((__COMMON_OBJECT*)&DeviceInputManager,
		NULL,
		(__COMMON_OBJECT*)lpShellThread))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize DeviceInputManager object.";
		goto __TERMINAL;
	}

	//********************************************************************************
	//
	//The following code creates user's main kernel thread if used as EOS.
	//
	//********************************************************************************

	//Create user kernel thread.
#ifdef __CFG_USE_EOS

	lpUserThread = KernelThreadManager.CreateKernelThread(   //Create shell thread.
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		__HCNMAIN_PRIORITY,
		_HCNMain,
		NULL,
		NULL,
		__HCNMAIN_NAME);
	if(NULL == lpUserThread)
	{
		pszErrorMsg = "INIT ERROR: Can not create User_Main kernel thread.";
		goto __TERMINAL;
	}
#endif

	//If log debugging functions is enabled.
#ifdef __CFG_SYS_LOGCAT

	lpLogcatDaemonThread = KernelThreadManager.CreateKernelThread(   //Create logcat daemon thread.
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		LogcatDaemon,
		NULL,
		NULL,
		"Logcat Daemon");
	if(NULL == lpLogcatDaemonThread)
	{
		pszErrorMsg = "INIT ERROR: Can not create Logcat_Daemon object.";
		goto __TERMINAL;
	}

#endif  //__CFG_SYS_LOGCAT.

#ifdef __CFG_NET_IPv4  //IPv4 network protocol is enabled.

	if(!IPv4_Entry(NULL))
	{
		pszErrorMsg = "INIT ERROR: Can not initialize IPv4 protocol function.";
		goto __TERMINAL;
	}
#endif
	System.EndInitialize((__COMMON_OBJECT*)&System);
	//Enter a dead loop to wait for the scheduling of kernel threads.
	DeadLoop(FALSE);

	//The following code will never be executed if anything is correct.
__TERMINAL:
	GotoHome();
	ChangeLine();
	PrintLine(pszErrorMsg);  //Show error msg.
	DeadLoop(TRUE);
}

//------------------------------------------------------------------------------
// A main routine to satisfy compiler under some platforms.
// This main routine will call __OS_Entry directly.
//------------------------------------------------------------------------------
#ifdef __STM32__
int main()
{
	__OS_Entry();  //Call OS entry routine.

	return 0;
}
#endif
