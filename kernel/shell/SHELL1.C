//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2011
//    Module Name               : shell1.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//    Last modified Author      :tywind
//    Last modified Date        :Aug 25,2015
//    Last modified Content     :modify loadapp and gui route 
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <KAPI.H>
#include <mlayout.h>
#include <pmdesc.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "../appldr/AppLoader.h"

//Handler of version command.
DWORD VerHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	_hx_printf("%s\r\n", HELLOX_VERSION_INFO);
	_hx_printf("Build date:%s,time:%s\r\n", __DATE__, __TIME__);
	_hx_printf("%s\r\n", HELLOX_SLOGAN_INFO);
	//_hx_printf("%s\r\n", HELLOX_SPECIAL_INFO);
	_hx_printf("Default k_thread stack sz: %d\r\n", DEFAULT_STACK_SIZE);
	return S_OK;
}

//Handler for memory,this routine print out the memory layout and memory usage status.
DWORD  MemHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	__SYSTEM_MLAYOUT_DESCRIPTOR* pLayout = NULL;

	DWORD  dwFlags;
	DWORD  dwPoolSize;
	DWORD  dwFreeSize;
	DWORD  dwFreeBlocks;
	DWORD  dwAllocTimesSuccL;
	DWORD  dwAllocTimesSuccH;
	DWORD  dwAllocTimesL;
	DWORD  dwAllocTimesH;
	DWORD  dwFreeTimesL;
	DWORD  dwFreeTimesH;

	__ENTER_CRITICAL_SECTION_SMP(AnySizeBuffer.spin_lock, dwFlags);
	dwPoolSize      = AnySizeBuffer.dwPoolSize;
	dwFreeSize      = AnySizeBuffer.dwFreeSize;
	dwFreeBlocks    = AnySizeBuffer.dwFreeBlocks;
	dwAllocTimesSuccL  = AnySizeBuffer.dwAllocTimesSuccL;
	dwAllocTimesSuccH  = AnySizeBuffer.dwAllocTimesSuccH;
	dwAllocTimesL      = AnySizeBuffer.dwAllocTimesL;
	dwAllocTimesH      = AnySizeBuffer.dwAllocTimesH;
	dwFreeTimesL       = AnySizeBuffer.dwFreeTimesL;
	dwFreeTimesH       = AnySizeBuffer.dwFreeTimesH;
	__LEAVE_CRITICAL_SECTION_SMP(AnySizeBuffer.spin_lock, dwFlags);

	/* Show out any size kernel pool's usage info. */
	_hx_printf("    Any size pool(FBL is adopted):\r\n");
	_hx_printf("    Total memory size     : %u(0x%X)\r\n",dwPoolSize,dwPoolSize);
	_hx_printf("    Total used size       : %u(0x%X)\r\n", (dwPoolSize - dwFreeSize), 
		(dwPoolSize - dwFreeSize));
	_hx_printf("    Free memory blocks    : %u\r\n",dwFreeBlocks);
	_hx_printf("    Alloc success times   : %u/%u\r\n",dwAllocTimesSuccH,dwAllocTimesSuccL);
	_hx_printf("    Alloc operation times : %u/%u\r\n",dwAllocTimesH,dwAllocTimesL);
	_hx_printf("    Free operation times  : %u/%u\r\n",dwFreeTimesH,dwFreeTimesL);

	/* Show out page frame resource usage info. */
	_hx_printf("\r\n    Page frame pool(Pagesz is %d):\r\n", PAGE_FRAME_SIZE);
	_hx_printf("    Total frames:         : %u(0x%X)\r\n",
		PageFrameManager.dwTotalFrameNum, PageFrameManager.dwTotalFrameNum);
	_hx_printf("    Free frames:          : %u(0x%X)\r\n",
		PageFrameManager.dwFreeFrameNum, PageFrameManager.dwFreeFrameNum);

	/* Show system memory layout information. */
	pLayout = (__SYSTEM_MLAYOUT_DESCRIPTOR*)SYS_MLAYOUT_ADDR;
	unsigned int count = 200;
	_hx_printf("\r\n    ------------ system RAM layout ------------ \r\n");
	while (count)
	{
		if ((0 == pLayout->length_lo) && (0 == pLayout->start_addr_lo))
		{
			break;
		}
		if (pLayout->mem_type == PHYSICAL_MEM_TYPE_RAM)
		{
			/* Only show out RAM information. */
			_hx_printf("    start_addr:0x%08X, length:0x%08X, type:%d\r\n",
				(uint32_t)pLayout->start_addr_lo,
				(uint32_t)pLayout->length_lo,
				pLayout->mem_type);
		}
		count--;
		pLayout++;
	}

	return S_OK;
}

//Handler for help command.
DWORD HlpHandler(__CMD_PARA_OBJ* pCmdParaObj)           //Command 'help' 's handler.
{
	LPSTR strHelpTitle   = "    The following commands are available currently:";
	LPSTR strHelpVer     = "    version      : Print out the version information.";
	LPSTR strHelpMem     = "    memory       : Print out current version's memory layout.";
	LPSTR strSysName     = "    sysname      : Change the system host name.";
	LPSTR strHelpHelp    = "    help         : Print out this screen.";
	LPSTR strSupport     = "    support      : Print out technical support information.";
	LPSTR strTime        = "    time         : Show system date and time.";
	LPSTR strRunTime     = "    runtime      : Display the total run time since last reboot.";
	LPSTR strIoCtrlApp   = "    ioctrl       : Start IO control application.";
	LPSTR strSysDiagApp  = "    sysdiag      : System or hardware diag application.";
	LPSTR strSysInfo     = "    sysinfo      : Show hardware system information.";
	LPSTR strFsApp       = "    fs           : File system operating application.";
	LPSTR strFdiskApp    = "    fdisk        : Hard disk operating application.";
	LPSTR strUsbVideo    = "    usbvideo     : USB video operations.";
	LPSTR strNetApp      = "    network      : Network diagnostic application.";
	LPSTR strLoadappApp  = "    loadapp      : Load application module and execute it.";
	LPSTR strGUIApp      = "    gui          : Load GUI module and enter GUI mode.";
#ifdef __CFG_APP_SSH
	LPSTR strSSH         = "    ssh          : Start a new SSH session.";
#endif

#ifdef __CFG_APP_JVM
	LPSTR strJvmApp      = "    jvm          : Start Java VM to run Java Application.";
#endif  //__CFG_APP_JVM
	LPSTR strReboot      = "    reboot       : Reboot the system.";
	LPSTR strCls         = "    cls          : Clear the whole screen.";

	PrintLine(strHelpTitle);              //Print out the help information line by line.
	PrintLine(strHelpVer);
	PrintLine(strHelpMem);
	PrintLine(strSysName);
	PrintLine(strHelpHelp);
	PrintLine(strSupport);
	PrintLine(strTime);
	PrintLine(strRunTime);
	PrintLine(strIoCtrlApp);
	PrintLine(strSysDiagApp);
	PrintLine(strSysInfo);
	PrintLine(strFsApp);
	PrintLine(strUsbVideo);
	PrintLine(strNetApp);
	PrintLine(strFdiskApp);
	PrintLine(strLoadappApp);
	PrintLine(strGUIApp);
#ifdef __CFG_APP_SSH
	PrintLine(strSSH);
#endif
#ifdef __CFG_APP_JVM
	PrintLine(strJvmApp);
#endif //__CFG_APP_JVM
	PrintLine(strReboot);
	PrintLine(strCls);

	return S_OK;
}

/* 
 * Load the specified user application and run it. 
 * The user application runs in user space as process.
 */
unsigned long LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	__PROCESS_OBJECT* pProcess = NULL;

	/* A module name must be specified. */
	if (pCmdParaObj->byParameterNum < 2)
	{
		_hx_printf("No module specified.\r\n");
		goto __TERMINAL;
	}

	/* 
	 * Make sure the length of module's path and name 
	 * is not too long. 
	 * The module's name as process's default name.
	 */
	BUG_ON(NULL == pCmdParaObj->Parameter[1]);
	if (strlen(pCmdParaObj->Parameter[1]) > MAX_FILE_NAME_LEN)
	{
		_hx_printf("Name is too long(should < %d).\r\n", MAX_FILE_NAME_LEN);
		goto __TERMINAL;
	}

	/* Launch a new process to run the module. */
	pProcess = ProcessManager.CreateProcess(
		(__COMMON_OBJECT*)&ProcessManager,
		0,
		PRIORITY_LEVEL_NORMAL,
		NULL,
		pCmdParaObj,
		NULL,
		"Process");
	if (NULL == pProcess)
	{
		_hx_printf("  Failed to launch the application.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/*
	 * Set the main thread as focus thread so as
	 * user input could be directed to this process.
	 */
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)pProcess->lpMainThread);
	pProcess->WaitForThisObject((__COMMON_OBJECT*)pProcess);
	/* Reset focus thread as default. */
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		NULL);
	ProcessManager.DestroyProcess((__COMMON_OBJECT*)&ProcessManager, (__COMMON_OBJECT*)pProcess);

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Load the specified application into memory and run it. */
DWORD __old_LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj)
{	
	__CMD_PARA_OBJ* pAppParaObj        = NULL;
	CHAR            FullPathName[128]  = {0};  //Full name of binary file.
	int             i                  = 0; 

	if(pCmdParaObj->byParameterNum < 2)
	{
		_hx_printf("No module specified.\r\n");
		goto __TERMINAL;
	}
	
	/* Make sure the length of module's path and name is not too long. */
	BUG_ON(NULL == pCmdParaObj->Parameter[1]);
	if (strlen(pCmdParaObj->Parameter[1]) > 127)
	{
		_hx_printf("Too long name(should < 128).\r\n");
		goto __TERMINAL;
	}

	/* 
	 * OK,just load the binary module from the given path and name.
	 * Convert the path and name to capital case since HelloX's FS
	 * is sensetive to string's case.
	 */
	strcpy(FullPathName, pCmdParaObj->Parameter[1]);
	ToCapital(FullPathName);
	/* 
	 * The app's filepath and name,and it's associated parameters,specified
	 * by the user when load the application,are all bounce to the application
	 * main kernel thread.
	 */
	pAppParaObj = CopyParameterObj(pCmdParaObj,1);
	RunDynamicAppModule(FullPathName,pAppParaObj);
	
__TERMINAL:
	ReleaseParameterObj(pAppParaObj);	
	return S_OK;
}

DWORD RunBatCmdLine(LPSTR pCmdLine)
{
	__CMD_PARA_OBJ* pAppParaObj  = NULL;
	
	ClearUnVisableChar(pCmdLine);

	pAppParaObj = FormParameterObj(pCmdLine);

	//exec app
	if(pAppParaObj->byParameterNum >= 2)
	{
		LoadappHandler(pAppParaObj);
	}

	ReleaseParameterObj(pAppParaObj);		
		
	return S_OK;
}

//Handler for Bat command.
DWORD BatHandler(__CMD_PARA_OBJ* pCmdParaObj)
{	
	HANDLE	hBatFile        = NULL;
	DWORD   dwFileSize      = 0;
	DWORD   dwReadSize      = 0;
	LPSTR   pShortName      = NULL;
	CHAR    szBatFile[128]  = {0};
	LPSTR   pBatBuf         = NULL;
	LPSTR   pCurrPos         = NULL;
	LPSTR   pNextPos         = NULL;
	DWORD   i                = 0;

	//Construct the bat full path and name.
	strcpy(szBatFile,"C:\\PTHOUSE\\");
	pShortName = strstr(pCmdParaObj->Parameter[0],"./");
	if(!pShortName)
	{
		PrintLine("error Batfile name.");
		goto __TERMINAL;
	}

	pShortName  += strlen("./");
	strcat(szBatFile,pShortName);
	
	hBatFile = CreateFile(szBatFile,FILE_ACCESS_READ,0,NULL);
	if(hBatFile == NULL)
	{		
		_hx_printf("Batfile open  error =%s \r\n",szBatFile);
		goto __TERMINAL;	
	}

	dwFileSize = GetFileSize(hBatFile,NULL);
	pBatBuf    = (LPSTR)_hx_malloc(dwFileSize+1);
	ReadFile(hBatFile,dwFileSize,pBatBuf,NULL);

	pCurrPos = pBatBuf;
	pNextPos = strstr(pCurrPos,"\n");

	while(pNextPos)
	{					
		*pNextPos = 0; 
				
		RunBatCmdLine(pCurrPos);
		
		pNextPos ++;
		pCurrPos = pNextPos;
		pNextPos = strstr(pCurrPos,"\n");		
	}
	
	if(pCurrPos)
	{
		RunBatCmdLine(pCurrPos);
	}

__TERMINAL:

	if(pBatBuf)
	{
		_hx_free(pBatBuf);
	}

	CloseFile(hBatFile);	

	return S_OK;
}

//Handler for GUI command,it only call LoadappHandler by given
//the GUI module's name and it's start address after loaded into
//memory.
DWORD GUIHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	//Set current mode indicator to GRAPHIC.
	System.ucReserved1 = 1;
	RunDynamicAppModule("C:\\PTHOUSE\\hcngui.dll",pCmdParaObj);
	//Restore current display mode to TEXT.
	System.ucReserved1 = 0;
	return S_OK;
}
