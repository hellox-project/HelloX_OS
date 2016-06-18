//***********************************************************************/
//    Author                    : tywind
//    Original Date             : 15 MAY,2016
//    Module Name               : telnet.c
//    Module Funciton           : 
//    Description               : Implementation code of telnet application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "shell.h"
#include "telnet2.h"

#include "kapi.h"
#include "string.h"
#include "stdio.h"

BOOL s_bTelnetOver = FALSE;

int PrintTelnetStr(const char* pStr)
{
	int  len = strlen(pStr);
	int  i;

	for(i=0;i<len;i++)
	{
		unsigned char  ch = (unsigned char)pStr[i] ;

		if(ch >= 0x20 && ch <= 0x7E)
		{
			CD_PrintChar(ch);
		}
		else if(ch == CRLF)
		{
			CD_GotoHome();
			CD_ChangeLine();
		}
	}

	return i;
}

int Telnet_Callback(UINT nMsg,LPCSTR pMgsBuf)
{
	switch(nMsg)
	{
		case TELMSG_CONNECT_FAILD:
			PrintLine("connect telnet server faild!");
			break;
		case TELMSG_CONNECT_OK:
			PrintLine("connect telnet server ok,now setup telnet options!");
			break;
		case TELMSG_SETUP_OK:
			PrintLine("setup options ok, please waiting telnet server response!");			
			break;
		case TELMSG_SETUP_FAILD:
			PrintLine("setup options faild");			
			break;
		case TELMSG_RESPONSE_OK:
			PrintTelnetStr(pMgsBuf);
			break;
	}

	return S_OK;
}

DWORD TelnetEntry(LPVOID pData)
{		

	while(1)
	{
		char   szOutput[TBL+1] = {0};
		int    recvlen         = 0;
		int    i;

		recvlen = telnet_recv_ret(pData,szOutput,TBL);
		if(recvlen < 0)
		{
			PrintLine("");
			s_bTelnetOver = TRUE;
			break;
		}

		if(recvlen == 0)
		{
			continue;
		}

		for(i=recvlen-1;i >= 0;i--)
		{
			if(szOutput[i] == VK_BACKSPACE)
			{
				CD_DelChar(DISPLAY_DELCHAR_PREV);
				szOutput[0] = 0;
				break;
				
			}
		}
		
		PrintTelnetStr(szOutput);			
	}


	return S_OK;
}

BOOL  IsReachPrompt()
{
	BYTE   szPromptFlage    = 0;
	WORD   CursorX          = 0;
	WORD   CursorY          = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	if(CursorX > 0)
	{
		CD_GetString(CursorX-1,CursorY,(LPSTR)&szPromptFlage,1);
		if(szPromptFlage == '>' || szPromptFlage == ']')
		{
			return TRUE;
		}
	}

	return FALSE;
}

DWORD TelnetSession(void* pTelnetObj)
{

	while(TRUE)
	{
		__KERNEL_THREAD_MESSAGE     Msg;

		if(GetMessage(&Msg))
		{
			BYTE bt = (BYTE)(Msg.dwParam);

			if(s_bTelnetOver)
			{
				goto __TERMINAL;
			}

			switch(Msg.wCommand)
			{
				case KERNEL_MESSAGE_AKEYDOWN:
					{	
						char szInput [2]  = {0};	
						int  ret          = 0;						

						if(bt == VK_BACKSPACE && IsReachPrompt())
						{
							continue;
						}

						szInput[0]  = bt;
						ret = telnet_send_input(pTelnetObj,szInput,1);
						if(ret < 0)
						{
							PrintLine("telnet disconnect!");
							goto __TERMINAL;
						}
						
					}
				break;
			}
		}
	}

__TERMINAL:

	  return S_OK;
}

DWORD TelnetHandler(__CMD_PARA_OBJ* lpCmdObj)
{
	void*    pTelnetObj  = NULL;
	int      nConnectOk  = 0;

	/*if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("params input error!");
		return SHELL_CMD_PARSER_TERMINAL;
	}*/
	
	
	s_bTelnetOver = FALSE;
	pTelnetObj    = telnet_new_seesion("192.168.110.177",23,Telnet_Callback);	
	//pTelnetObj  = telnet_new_seesion(lpCmdObj->Parameter[1],atoi(lpCmdObj->Parameter[2]),Telnet_Callback);	
	if(pTelnetObj)
	{
		nConnectOk = telnet_connect_seesion(pTelnetObj);
	}
	else
	{		
		return SHELL_CMD_PARSER_TERMINAL;
	}
		
	if(nConnectOk <= 0)
	{
		telnet_free_seesion(pTelnetObj);
		return SHELL_CMD_PARSER_TERMINAL;
	}

	
	KernelThreadManager.CreateKernelThread(   //Create shell thread.
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,PRIORITY_LEVEL_HIGH,	TelnetEntry,pTelnetObj,NULL,"TELNET");
		
	
	TelnetSession(pTelnetObj);

	telnet_free_seesion(pTelnetObj);

	return SHELL_CMD_PARSER_SUCCESS;	
}
