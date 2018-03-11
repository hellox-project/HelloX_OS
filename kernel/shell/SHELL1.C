//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2011
//    Module Name               : shell1.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//                                Some functions in shell.cpp originally are moved to this file to reduce the
//                                shell.cpp's size.
//    Last modified Author      :tywind
//    Last modified Date        :Aug 25,2015
//    Last modified Content     :modify loadapp and gui route 
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "kapi.h"
#include "shell.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "../appldr/AppLoader.h"


//Handler of version command.
DWORD VerHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	PrintLine(HELLOX_VERSION_INFO);
	PrintLine(HELLOX_SLOGAN_INFO);
	PrintLine(HELLOX_SPECIAL_INFO);
	_hx_printf("Default k_thread stack sz: %d\r\n", DEFAULT_STACK_SIZE);
	return S_OK;
}

//Handler for memory,this routine print out the memory layout and memory usage status.
DWORD  MemHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	CHAR   buff[256];
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

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	dwPoolSize      = AnySizeBuffer.dwPoolSize;
	dwFreeSize      = AnySizeBuffer.dwFreeSize;
	dwFreeBlocks    = AnySizeBuffer.dwFreeBlocks;
	dwAllocTimesSuccL  = AnySizeBuffer.dwAllocTimesSuccL;
	dwAllocTimesSuccH  = AnySizeBuffer.dwAllocTimesSuccH;
	dwAllocTimesL      = AnySizeBuffer.dwAllocTimesL;
	dwAllocTimesH      = AnySizeBuffer.dwAllocTimesH;
	dwFreeTimesL       = AnySizeBuffer.dwFreeTimesL;
	dwFreeTimesH       = AnySizeBuffer.dwFreeTimesH;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	PrintLine("    Free block list algorithm is adopted:");
	//Get and dump out memory usage status.
	_hx_sprintf(buff,"    Total memory size     : %d(0x%X)",dwPoolSize,dwPoolSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory size      : %d(0x%X)",dwFreeSize,dwFreeSize);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free memory blocks    : %d",dwFreeBlocks);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc success times   : %d/%d",dwAllocTimesSuccH,dwAllocTimesSuccL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Alloc operation times : %d/%d",dwAllocTimesH,dwAllocTimesL);
	PrintLine(buff);
	_hx_sprintf(buff,"    Free operation times  : %d/%d",dwFreeTimesH,dwFreeTimesL);
	PrintLine(buff);

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

/* Load the specified application into memory and run it. */
DWORD LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj)
{	
	__CMD_PARA_OBJ* pAppParaObj        = NULL;
	CHAR            FullPathName[128]  = {0};  //Full name of binary file.
	int             i                  = 0; 

	if(pCmdParaObj->byParameterNum < 2)
	{
		_hx_printf("Please specify app module's path and name.\r\n");
		goto __TERMINAL;
	}
	
	/* Make sure the length of module's path and name is not too long. */
	BUG_ON(NULL == pCmdParaObj->Parameter[1]);
	if (strlen(pCmdParaObj->Parameter[1]) > 127)
	{
		_hx_printf("Module's path and name too long(should < 128).\r\n");
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
