//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : shell.cpp
//    Module Funciton           : 
//                                This module countains shell procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "kapi.h"
#include "shell.h"
#include "ioctrl_s.h"
#include "sysd_s.h"
#include "extcmd.h"
#include "debug.h"
#include "modmgr.h"
#include "console.h"
#include "stdio.h"
#include "string.h"
#include <buffmgr.h>


#if defined(__I386__)
#ifndef __BIOS_H__
#include "../arch/x86/bios.h"
#endif
#endif

#define  DEF_PROMPT_STR   "[system-view]"
#define  ERROR_STR        "  Incorrect command."

//shell input pos
#define  SHELL_INPUT_START_X       (strlen(s_szPrompt))   // 
#define  SHELL_INPUT_START_Y       1 
#define  SHELL_INPUT_START_Y_FIRST 4 

//Host name array of the system.
#define MAX_HOSTNAME_LEN           16
CHAR    s_szPrompt[64]             = {0};

//Shell thread's handle.
__KERNEL_THREAD_OBJECT*  g_lpShellThread   = NULL;
//static HISOBJ            s_hHiscmdInoObj   = NULL;


//The following handlers are moved to shell1.cpp.
extern DWORD VerHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handles the version command.
extern DWORD MemHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handles the memory command.
extern DWORD SysInfoHandler(__CMD_PARA_OBJ* pCmdParaObj);      //Handles the sysinfo command.
extern DWORD HlpHandler(__CMD_PARA_OBJ* pCmdParaObj);
extern DWORD LoadappHandler(__CMD_PARA_OBJ* pCmdParaObj);
extern DWORD GUIHandler(__CMD_PARA_OBJ* pCmdParaObj);          //Handler for GUI command,resides in
//extern DWORD FileWriteTest(__CMD_PARA_OBJ* pCmdParaObj); 

static DWORD CpuHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SptHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD ClsHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD RunTimeHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SysNameHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD TimeHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD IoCtrlApp(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD SysDiagApp(__CMD_PARA_OBJ* pCmdParaObj);
#ifdef __CFG_APP_JVM
static DWORD JvmHandler(__CMD_PARA_OBJ* pCmdParaObj);
#endif
static DWORD Reboot(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD Poweroff(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD ComDebug(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD DebugHandler(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD FileWriteTest(__CMD_PARA_OBJ* pCmdParaObj);
static DWORD FileReadTest(__CMD_PARA_OBJ* pCmdParaObj);

//Internal command handler array.
__CMD_OBJ  CmdObj[] = {
	{"version"  ,    VerHandler},
	{"memory"   ,    MemHandler},
	{"sysinfo"  ,    SysInfoHandler},
	{"sysname"  ,    SysNameHandler},
	{"help"     ,    HlpHandler},
	{"cpuinfo"  ,    CpuHandler},
	{"support"  ,    SptHandler},
	{"time"     ,    TimeHandler},
	{"runtime"  ,    RunTimeHandler},
	{"ioctrl"   ,    IoCtrlApp},
	{"sysdiag"  ,    SysDiagApp},
	{"loadapp"  ,    LoadappHandler},
	{"gui"      ,    GUIHandler},
#ifdef __CFG_APP_JVM  //Java VM is enabled.
	{"jvm"      ,    JvmHandler},
#endif
	{"reboot"   ,    Reboot},
	{"poff"     ,    Poweroff},
	{"comdebug" ,    ComDebug},
	{"cls"      ,    ClsHandler},
	//You can add your specific command and it's handler here.
	//{'yourcmd',    CmdHandler},
	 {"fw"      ,   FileWriteTest},
	 {"fr"      ,   FileReadTest},
	
	{"debug"    ,    DebugHandler},
//	{"test"    ,    FileWriteTest},
	//The last element of this array must be NULL.
	{NULL       ,    NULL}
};

//Com interface debugging application.
static DWORD ComDebug(__CMD_PARA_OBJ* pCmdParaObj)
{
	HANDLE hCom1 = NULL;
	CHAR   *pData = "Hello China V1.76";  //Data to write to COM1 interface.
	DWORD  dwWriteSize = 0;
	CHAR   buff[16];

	//Try to open COM1 interface to debugging.
	hCom1 = IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		"\\\\.\\COM1",
		0,
		0,
		NULL);
	if(NULL == hCom1)
	{
		PrintLine("ComDebug : Can not open COM1 interface.");
		goto __TERMINAL;
	}
	else
	{
		PrintLine("ComDebug: Open COM1 interface successfully.");
		if(IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
			hCom1,
			strlen(pData),
			pData,
			&dwWriteSize))
		{
			PrintLine("Write data to COM1 interface successfully.");
		}
		else
		{
			PrintLine("Can not write data to COM1 interface.");
		}
		PrintLine("ComDebug: Try to read data from COM interface...");
		if(!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
			hCom1,
			1,
			(LPVOID)&buff[0],
			NULL))
		{
			PrintLine("Can not read COM interface.");
		}
		else
		{
			PrintLine("Read COM interface sucessfully.");
		}
	}

__TERMINAL:
	if(NULL != hCom1)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hCom1);
	}

	return SHELL_CMD_PARSER_SUCCESS;
}

//
//sysname handler.
//This handler changes the system name,and save it to system config database.
//
static DWORD SaveSysName(__CMD_PARA_OBJ* pCmdParaObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD SysNameHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	//__CMD_PARA_OBJ*    pCmdObj = NULL;
	//pCmdParaObj = FormParameterObj(pszSysName);
	if(NULL == pCmdParaObj)
	{
		PrintLine("Not enough system resource to interpret the command.");
		goto __TERMINAL;
	}
	if((0 == pCmdParaObj->byParameterNum) || (0 == pCmdParaObj->Parameter[0][0]))
	{
		PrintLine("Invalid command parameter.");
		goto __TERMINAL;
	}

	if(StrLen(pCmdParaObj->Parameter[0]) >= MAX_HOSTNAME_LEN)
	{
		PrintLine("System name must not exceed 16 bytes.");
		goto __TERMINAL;
	}

	//SaveSysName(pCmdParaObj->Parameter[0]);
	//StrCpy(pCmdObj->Parameter[0],&s_szPrompt[0]);
	_hx_sprintf(s_szPrompt,"[%s]",pCmdParaObj->Parameter[0]);
	if(StrLen(s_szPrompt) <= 0)
	{
		StrCpy(DEF_PROMPT_STR,&s_szPrompt[0]);
	}

__TERMINAL:

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD TimeHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	BYTE   time[6];

	__GetTime(&time[0]);
	_hx_printf("  Current Date: %d-%d-%d(YYYY-MM-DD)\r\n",time[0] + 2000,time[1],time[2]);
	_hx_printf("  Current Time: %d:%d:%d(HH:MM:SS)\r\n",time[3],time[4],time[5]);
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'ioctrl' command.
DWORD IoCtrlApp(__CMD_PARA_OBJ* pCmdParaObj)
{
	__KERNEL_THREAD_OBJECT*    lpIoCtrlThread    = NULL;

	lpIoCtrlThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		IoCtrlStart,
		NULL,
		NULL,
		"IO CTRL");
	if(NULL == lpIoCtrlThread)    //Can not create the IO control thread.
	{
		PrintLine("Can not create IO control thread.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);    //Set the current focus to IO control
	//application.

	lpIoCtrlThread->WaitForThisObject((__COMMON_OBJECT*)lpIoCtrlThread);  //Block the shell
	//thread until
	//the IO control
	//application end.
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,NULL);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpIoCtrlThread);  //Destroy the thread object.

	return SHELL_CMD_PARSER_SUCCESS;
}

#ifdef __CFG_APP_JVM
//Main entry point of java VM.
extern int jvm_main(int argc, char* argv[]);
//Wrapper of Java VM.
static DWORD JvmEntryPoint(LPVOID pData)
{
	__CMD_PARA_OBJ* pCmdParaObj = (__CMD_PARA_OBJ*)pData;
	
	if (NULL == pData)
	{
		_hx_printf("  No parameter specified.\r\n");
		return 0;
	}
	
	//Call JVM's main entry.
	return jvm_main(pCmdParaObj->byParameterNum, pCmdParaObj->Parameter);
}

static DWORD JvmHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	__KERNEL_THREAD_OBJECT*    lpJVMThread = NULL;
	//char*                      className = "-version";

	//Create Java VM thread.
	lpJVMThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		JvmEntryPoint,
		pCmdParaObj,
		NULL,
		"JVM");
	if (NULL == lpJVMThread)    //Can not create the IO control thread.
	{
		PrintLine("Can not create Java VM thread.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpJVMThread);    //Set the current focus to IO control
	//application.

	lpJVMThread->WaitForThisObject((__COMMON_OBJECT*)lpJVMThread);  //Block the shell
	//thread until
	//the IO control
	//application end.
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager, NULL);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpJVMThread);  //Destroy the thread object.

	return SHELL_CMD_PARSER_SUCCESS;
}
#endif

//
//System diag application's shell start code.
//
DWORD SysDiagApp(__CMD_PARA_OBJ* pCmdParaObj)
{
	__KERNEL_THREAD_OBJECT*        lpSysDiagThread    = NULL;

	lpSysDiagThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		SysDiagStart,
		NULL,
		NULL,
		"SYS DIAG");
	if(NULL == lpSysDiagThread)    //Can not create the kernel thread.
	{
		PrintLine("Can not start system diag application,please retry again.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpSysDiagThread);

	lpSysDiagThread->WaitForThisObject((__COMMON_OBJECT*)lpSysDiagThread);
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,NULL);
	KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)lpSysDiagThread);  //Destroy the kernel thread object.

	return SHELL_CMD_PARSER_SUCCESS;
}

DWORD FileReadTest(__CMD_PARA_OBJ* pCmdParaObj)
{
	char   szFileBuf[512] = {0};
	char   szInfoBuf[128] = {0};
	DWORD  secondS        = 0;    
	DWORD  secondE        = 0;   
	DWORD  ReadRet         = 0;
	DWORD  ReadCount     = 20480;
	DWORD  dwFilePos      = 0;
	DWORD  i              = 0;
	HANDLE hFileObj       = NULL;

	secondS = System.GetSysTick(NULL);  //Get system tick counter.
	//Convert to second.
	secondS *= SYSTEM_TIME_SLICE;
	secondS /= 1000;

	_hx_printf("Raed Start....\r\n");

	hFileObj = CreateFile("C:\\wt.dat",FILE_ACCESS_READ|FILE_OPEN_EXISTING,0,NULL);

	if(hFileObj == NULL || hFileObj == (HANDLE)-1)
	{
		_hx_printf("read Faild!\r\n");		
		return S_OK;
	}
		
	for(i=0;i<ReadCount;i++)
	{
		ReadFile(hFileObj,sizeof(szFileBuf),szFileBuf,&ReadRet);

		if(ReadRet < sizeof(szFileBuf))
		{
			_hx_printf("Raed faild....step=%d\r\n",i);
		}
	}
		
	//_hx_printf("Close handle\r\n");
	CloseFile(hFileObj);

	secondE = System.GetSysTick(NULL);  //Get system tick counter.	
	secondE *= SYSTEM_TIME_SLICE;
	secondE /= 1000;

	_hx_printf("Read Complete,Use Time= %d sec",secondE-secondS);

	return S_OK;
}

DWORD FileModifyTest(__CMD_PARA_OBJ* pCmdParaObj)
{
	DWORD  dwFilePos      = 1024;
	HANDLE hFileObj       = NULL;

	hFileObj = CreateFile("C:\\test.bmp",FILE_ACCESS_WRITE|FILE_OPEN_ALWAYS,0,NULL);

	_hx_printf("Modify Start!\r\n");		

	if(hFileObj == NULL || hFileObj == (HANDLE)-1)
	{
		_hx_printf("Modify Faild!\r\n");		
		return S_OK;
	}

	//�ƶ��ļ�
	SetFilePointer(hFileObj,&dwFilePos,&dwFilePos,FILE_FROM_CURRENT);

	//�ض�
	SetEndOfFile(hFileObj);

	CloseFile(hFileObj);

	_hx_printf("Modify Complete!\r\n");		

	return S_OK;
}

//Entry point of FileWriteTest.
DWORD FileWriteTest(__CMD_PARA_OBJ* pCmdParaObj)
{
	static  char  s_temp = 'a';
	char   szFileBuf[512] = {0};
	char   szInfoBuf[128] = {0};
	DWORD  secondS        = 0;    
	DWORD  secondE        = 0;   
	DWORD  Writed         = 0;
	DWORD  WriteCount     = 10;
	DWORD  dwFilePos      = 0;
	DWORD  i              = 0;
	HANDLE hFileObj       = NULL;

	secondS = System.GetSysTick(NULL);  //Get system tick counter.
	//Convert to second.
	secondS *= SYSTEM_TIME_SLICE;
	secondS /= 1000;

	memset(szFileBuf,s_temp,sizeof(szFileBuf));
	s_temp += 1;
	_hx_printf("Write Start....\r\n");

	hFileObj = CreateFile("C:\\wt.dat",FILE_ACCESS_WRITE|FILE_OPEN_ALWAYS,0,NULL);

	if(hFileObj == NULL || hFileObj == (HANDLE)-1)
	{
		_hx_printf("Write Faild!\r\n");		
		return S_OK;
	}

	//�ƶ����ļ�β
	SetFilePointer(hFileObj,&dwFilePos,&dwFilePos,FILE_FROM_END);

	//	_hx_printf("start write \r\n");
	for(i=0;i<WriteCount;i++)
	{
		WriteFile(hFileObj,sizeof(szFileBuf),szFileBuf,&Writed);
	}

	WriteFile(hFileObj,2,"\r\n",&Writed);

	//_hx_printf("Close handle\r\n");
	CloseFile(hFileObj);

	secondE = System.GetSysTick(NULL);  //Get system tick counter.	
	secondE *= SYSTEM_TIME_SLICE;
	secondE /= 1000;

    _hx_printf("Write Complete,Use Time= %d sec\r\n",secondE-secondS);

	return S_OK;
}

//Entry point of reboot.
DWORD Reboot(__CMD_PARA_OBJ* pCmdParaObj)
{
	ClsHandler(NULL); //Clear screen first.
#ifdef __I386__
	BIOSReboot();
#endif
	return SHELL_CMD_PARSER_SUCCESS;
}

//Entry point of poweroff.
DWORD Poweroff(__CMD_PARA_OBJ* pCmdParaObj)
{
#ifdef __I386__
	BIOSPoweroff();
#endif
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'runtime' command.
DWORD RunTimeHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	CHAR  Buffer[190] = {0};
	DWORD week = 0,day = 0,hour = 0,minute = 0,second = 0;
	DWORD* Array[5] = {0};

	second = System.GetSysTick(NULL);  //Get system tick counter.
	//Convert to second.
	second *= SYSTEM_TIME_SLICE;
	second /= 1000;

	if(second >= 60)  //Use minute.
	{
		minute = second / 60;
		second = second % 60;
	}
	if(minute >= 60) //use hour.
	{
		hour   = minute / 60;
		minute = minute % 60;
	}
	if(hour >= 24) //Use day.
	{
		day  = hour / 24;
		hour = hour % 24;
	}
	if(day >= 7) //Use week.
	{
		week = day / 7;
		day  = day % 7;
	}

	Array[0] = &week;
	Array[1] = &day;
	Array[2] = &hour;
	Array[3] = &minute;
	Array[4] = &second;	
	FormString(Buffer,"System has running %d week(s), %d day(s), %d hour(s), %d minute(s), %d second(s).",(LPVOID*)Array);

	//Show out the result.
	/*sprintf(Buffer,"System has running %d week(s), %d day(s), %d hour(s), %d minute(s), %d second(s).",
		week,day,hour,minute,second);*/
	//sprintf(Buffer,"System has running %d,%d",(INT)week,(INT)day);//(INT)hour,(INT)minute,(INT)second
	//PrintLine(Buffer);
	CD_PrintString(Buffer,TRUE);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'cls' command.
DWORD ClsHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	ClearScreen();

	CD_SetCursorPos(0,SHELL_INPUT_START_Y);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'cpu' command.
DWORD CpuHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	/*GotoHome();
	ChangeLine();
	PrintStr("Cpu Handler called.");*/

	CD_PrintString("Cpu Handler called.",TRUE);

	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler for 'support' command.
DWORD SptHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	LPSTR strSupportInfo1 = "    For any technical support,send E-Mail to:";
	LPSTR strSupportInfo2 = "    garryxin@yahoo.com.cn.";
	LPSTR strSupportInfo3 = "    or join the QQ group : 38467832";

	PrintLine(strSupportInfo1);
	PrintLine(strSupportInfo2);
	PrintLine(strSupportInfo3);

	return SHELL_CMD_PARSER_SUCCESS;
}

//*********************************
// For log service
//Author :	Erwin
//Email  :	erwin.wang@qq.com
//Date	 :  9th June, 2014
//********************************
static DWORD Process1(LPVOID pData)
{
	int i = 0;
	for(i = 0;i < 5;i ++)
	{
		_hx_printf("  I'm process 1.\r\n");
		DumpProcess();
		Sleep(10);
	}
	return 0;
}

static DWORD Process2(LPVOID pData)
{
	int i = 0;
	for(i = 0;i < 5;i ++)
	{
		_hx_printf("  I'm process 2.\r\n");
		DumpProcess();
		Sleep(20);
	}
	return 0;
}

DWORD DebugHandler(__CMD_PARA_OBJ* pCmdParaObj)
{
	__PROCESS_OBJECT* pProcess1  = NULL;
	__PROCESS_OBJECT* pProcess2  = NULL;
	int round = 1000;

	for(round = 0;round < 20480;round ++)
	{
		pProcess1 = ProcessManager.CreateProcess(
			(__COMMON_OBJECT*)&ProcessManager,
			0,
			PRIORITY_LEVEL_NORMAL,
			Process1,
			NULL,
			NULL,
			"Process1");
		if(NULL == pProcess1)
		{
			_hx_printf("  Create Process1 failed.\r\n");
			return SHELL_CMD_PARSER_SUCCESS;
		}
		_hx_printf("  Create Process1 successfully.\r\n");

		pProcess2 = ProcessManager.CreateProcess(
			(__COMMON_OBJECT*)&ProcessManager,
			0,
			PRIORITY_LEVEL_NORMAL,
			Process2,
			NULL,
			NULL,
			"Process2");
		if(NULL == pProcess2)
		{
			_hx_printf("  Create Process2 failed.\r\n");
			return SHELL_CMD_PARSER_SUCCESS;
		}
		_hx_printf("  Create Process2 successfully.\r\n");
		
		pProcess1->WaitForThisObject((__COMMON_OBJECT*)pProcess1);
		pProcess2->WaitForThisObject((__COMMON_OBJECT*)pProcess2);
		ProcessManager.DestroyProcess((__COMMON_OBJECT*)&ProcessManager,(__COMMON_OBJECT*)pProcess1);
		ProcessManager.DestroyProcess((__COMMON_OBJECT*)&ProcessManager,(__COMMON_OBJECT*)pProcess2);
		_hx_printf("  Debug round [ %d ]run over.\r\n",round);
	}

	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex   = 0;
	static DWORD dwExIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;
		dwExIndex  = 0;
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL != CmdObj[dwIndex].CmdStr)
	{
		strncpy(pMatchBuf,CmdObj[dwIndex].CmdStr,nBufLen);
		dwIndex ++;

		return SHELL_QUERY_CONTINUE;
	}

	if(NULL != ExtCmdArray[dwExIndex].lpszCmdName)
	{
		strncpy(pMatchBuf,ExtCmdArray[dwExIndex].lpszCmdName,nBufLen);
		dwExIndex ++;

		return SHELL_QUERY_CONTINUE;
	}

	dwIndex    = 0;
	dwExIndex  = 0;

	return SHELL_QUERY_CANCEL;	
}
//Command analyzing routine,it analyzes user's input and search
//command array to find a proper handler,then call it.
//Default handler will be called if no proper command handler is
//located.
static DWORD  CommandParser(LPSTR pCmdBuf)
{
	__KERNEL_THREAD_OBJECT* hKernelThread = NULL;
	__CMD_PARA_OBJ*         lpCmdParamObj = NULL;
	DWORD   dwResult                      = SHELL_CMD_PARSER_INVALID;        //If find the correct command object,then
	DWORD   dwIndex                       = 0;          //Used for 'for' loop.
	

	lpCmdParamObj = FormParameterObj(pCmdBuf);
	if(NULL == lpCmdParamObj || lpCmdParamObj->byParameterNum < 1)    //Can not form a valid command parameter object.
	{
		CD_PrintString(pCmdBuf,TRUE);
		goto __END;
	}
	
	dwIndex = 0;
	while(CmdObj[dwIndex].CmdStr)
	{
		if(StrCmp(CmdObj[dwIndex].CmdStr,lpCmdParamObj->Parameter[0]))
		{			
			CmdObj[dwIndex].CmdHandler(lpCmdParamObj);  //Call the command handler.
			dwResult = SHELL_CMD_PARSER_SUCCESS;
			break;
		}
		dwIndex ++;
	}

	if(dwResult == SHELL_CMD_PARSER_SUCCESS)
	{
		goto __END;
	}
	
	dwIndex = 0;  //Now,should search external command array.
	while(ExtCmdArray[dwIndex].lpszCmdName)
	{		
		if(StrCmp(ExtCmdArray[dwIndex].lpszCmdName,lpCmdParamObj->Parameter[0]))  //Found.
		{	
			hKernelThread = KernelThreadManager.CreateKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				0,
				KERNEL_THREAD_STATUS_READY,
				PRIORITY_LEVEL_NORMAL,
				ExtCmdArray[dwIndex].ExtCmdHandler,
				(LPVOID)lpCmdParamObj, //?
				NULL,
				NULL);
			if(!ExtCmdArray[dwIndex].bBackground)  //Should wait.
			{
				DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
					(__COMMON_OBJECT*)hKernelThread);  //Give the current input focus to this thread.
				hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);
				DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,NULL);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)hKernelThread);  //Destroy it.
				//Set focus thread to shell.
			}
			
			dwResult = SHELL_CMD_PARSER_SUCCESS;
			goto __END;
		}
		dwIndex ++;
	}
	
	//DefaultHandler(NULL); //Call the default command handler.	
__END:

	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}
	
	return dwResult;		
}

//Entry point of the text mode shell.
//
DWORD ShellEntryPoint(LPVOID pData)
{
	WORD x, y;

	StrCpy(DEF_PROMPT_STR,&s_szPrompt[0]);
	CD_PrintString(VERSION_INFO,TRUE);
	//CD_SetCursorPos(0,SHELL_INPUT_START_Y_FIRST);
	CD_GetCursorPos(&x, &y);
	CD_SetCursorPos(0, y + 1);
	
	Shell_Msg_Loop(s_szPrompt,CommandParser,QueryCmdName);

	//When reach here,it means the shell thread will terminate.We will reboot
	//the system in current version's implementation,since there is no interact
	//mechanism between user and computer in case of no shell.
	//NOTE:System clean up operations should be put here if necessary.
#ifdef __I386__
	BIOSReboot();
#endif

	return 0;
}
