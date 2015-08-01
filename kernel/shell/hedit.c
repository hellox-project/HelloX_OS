//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 26 FEB,2009
//    Module Name               : HEDIT.CPP
//    Module Funciton           : 
//                                Implementation code of HEDIT application.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "../include/StdAfx.h"
#endif

#ifndef __KAPI_H__
#include "../include/kapi.h"
#endif

#include "shell.h"
#include "hedit.h"

#include "../lib/string.h"
#include "../lib/stdio.h"

//Create user specified file to write.
static HANDLE CreateEditFile(LPSTR lpszCmdLine)
{
#ifdef __CFG_SYS_DDF
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;
	HANDLE                 hFile             = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		goto __TERMINAL;
	}

	//Try to open or create the specified file to write.
	hFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		lpszCmdLine,
		FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS,
		0,
		NULL);

__TERMINAL:
	return hFile;
#else
	return NULL;
#endif
}

//Process user input.
//This is a private function can only be called by heditEntry.
static VOID __UserInput(HANDLE hFile)
{
#ifdef __CFG_SYS_DDF
	BYTE*                       pDataBuffer      = NULL;
	BYTE*                       pCurrPos         = NULL;
	DWORD                       dwDefaultSize    = 8192;      //Default file size is 8K.
	BOOL                        bCtrlDown        = FALSE;
	BYTE                        bt;
	WORD                        wr                            = 0x0700;
	__KERNEL_THREAD_MESSAGE     Msg;
	DWORD                       dwWrittenSize    = 0;

	pDataBuffer = (BYTE*)KMemAlloc(dwDefaultSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pDataBuffer)
	{
		PrintLine("  Can not allocate memory.");
		goto __TERMINAL;
	}
	pCurrPos = pDataBuffer;

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			if(MSG_KEY_DOWN == Msg.wCommand)    //This is a key down message.
			{
				bt = (BYTE)Msg.dwParam;
				switch(bt)
				{
				case VK_RETURN:                  //This is a return key.
					if((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize - 2)
					{
						*pCurrPos ++ = '\r';     //Append a return key.
						*pCurrPos ++ = '\n';
						GotoHome();
						ChangeLine();            //Change to next line.
					}
					break;
				case VK_BACKSPACE:
					if(*pCurrPos == '\n')  //Enter encountered.
					{
						pCurrPos -= 2;     //Skip the \r\n.
					}
					else
					{
						pCurrPos -= 1;
					}
					GotoPrev();
					break;
				default:
					if(('c' == bt) || ('C' == bt) || ('z' == bt) || ('Z' == bt))
					{
						if(bCtrlDown)  //CtrlC or CtrlZ encountered.
						{
							goto __TERMINAL;
						}
					}
					if((DWORD)(pCurrPos - pDataBuffer) < dwDefaultSize)
					{
						*pCurrPos ++ = bt; //Save this character.
						wr += bt;
						PrintCh(wr);
						wr  = 0x0700;
					}
					break;
				}
			}
			else
			{
				if(VIRTUAL_KEY_DOWN == Msg.wCommand)
				{
					bt = (BYTE)Msg.dwParam;
					if(VK_CONTROL == bt)
					{
						bCtrlDown = TRUE;
					}
				}
				if(VIRTUAL_KEY_UP   == Msg.wCommand)
				{
					bt = (BYTE)Msg.dwParam;
					if(VK_CONTROL == bt)    //Control key up.
					{
						bCtrlDown = FALSE;
					}
				}
			}
		}
	}

__TERMINAL:
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		hFile,
		(DWORD)(pCurrPos - pDataBuffer),
		pDataBuffer,
		&dwWrittenSize);
	if(pDataBuffer)
	{
		KMemFree(pDataBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return;
#else
#endif
}

//Internal routine to print version info.
static VOID __VersionInfo()
{
	LPSTR       lpszVersion   = "hedit - Hello China Editor,version 0.1";

	//Print out version info.
	GotoHome();
	ChangeLine();
	PrintLine(lpszVersion);
}

//Main entry of HEDIT application.
DWORD heditEntry(LPVOID lpParam)
{
#ifdef __CFG_SYS_DDF
	__CMD_PARA_OBJ* pCmdParaObj = (__CMD_PARA_OBJ*)lpParam;
	//LPSTR       lpszParam     = (LPSTR)lpParam;
	HANDLE      hFile         = NULL;
	LPSTR       lpszNoTarget  = "  Please specify the target file to write.";
	LPSTR       lpszFileName  = NULL;

	if(NULL == pCmdParaObj || pCmdParaObj->byParameterNum < 2)
	{
		PrintLine(lpszNoTarget);
		goto __TERMINAL;
	}

	hFile = CreateEditFile(pCmdParaObj->Parameter[1]);
	if(NULL == hFile)  //Can not create the target file.
	{
		PrintLine("  Can not create the target file.");
		PrintLine(pCmdParaObj->Parameter[1]);  //For debugging.
		goto __TERMINAL;
	}
	//Process user input and save to file.
	GotoHome();
	ChangeLine();
	__UserInput(hFile);
	//When user terminates this application,print out version information.
	__VersionInfo();

__TERMINAL:
	if(hFile)  //Close it.
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hFile);
	}
	return 0;
#else
	return 0;
#endif
}
