//***********************************************************************/
//    Author                    : tywind
//    Original Date             : 2 JUNE,2016
//    Module Name               : ssh2.c
//    Module Funciton           : 
//    Description               : Implementation code of ssh2 application.
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
#include "ssh/ssh.h"

#include "kapi.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#define  TIPS_SSHERR  " ssh error :err_id=%d\r\n"
#define  TIPS_AUTHERR   " username or password error\r\n"
#define  TIPS_NETERR  " ssh disconnect\r\n"


static  void* s_pSshObj        = NULL;
static  BOOL  s_bSsh2Exit      = FALSE;
static  BOOL  s_bLoginOk       = FALSE;
static  BYTE  s_KeyState[256]  = {0};

//BOOL  s_TestData = FALSE;
extern void terminal_init();
extern void terminal_uninit();
extern int  terminal_cmdline();
extern void terminal_analyze(const char* srcstr,int len);


static BOOL  IsReachPrompt()
{
	BYTE   szPromptFlage    = 0;
	WORD   CursorX          = 0;
	WORD   CursorY          = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	if(CursorX > 0)
	{
		CD_GetString(CursorX-1,CursorY,(LPSTR)&szPromptFlage,1);
		if(szPromptFlage == '>' || szPromptFlage == ']' || szPromptFlage == ':')
		{
			return TRUE;
		}
	}

	return FALSE;
}

static BYTE  GetKeyInput(BOOL *pVk)
{
	__KERNEL_THREAD_MESSAGE  Msg;
	BYTE  bKey = 0;

	GetMessage(&Msg);
	if(Msg.wCommand == KERNEL_MESSAGE_AKEYDOWN || Msg.wCommand == KERNEL_MESSAGE_VKEYDOWN)
	{		
		if(Msg.wCommand == KERNEL_MESSAGE_VKEYDOWN)
		{
			*pVk = TRUE;
		}
				
		bKey = (BYTE)(Msg.dwParam);			
		s_KeyState[bKey] = 1;

	}
	else if( Msg.wCommand == KERNEL_MESSAGE_VKEYUP)
	{
		s_KeyState[(BYTE)(Msg.dwParam)] = 0;
	}

	return bKey;
}

static BOOL  GetInputInfo(LPSTR pTips,BOOL bShow,LPSTR pInputBuf,INT len)
{	
	INT  pos      = 0; 
	BOOL bVk      = FALSE;
	BYTE KeyCode  = 0;

	CD_PrintString(pTips,FALSE);	

	while(1)
	{	
		KeyCode = GetKeyInput(&bVk);
		if(KeyCode == 0) //
		{		
			continue;
		}

		if(KeyCode == VK_RETURN)
		{
			CD_ChangeLine();
			return TRUE;			
		}
		else if(KeyCode == VK_BACKSPACE)
		{
			if(IsReachPrompt() == FALSE)
			{
				pos --; pInputBuf[pos] = 0;		
				if(bShow == TRUE) CD_DelChar(DISPLAY_DELCHAR_PREV);
			}			
		}
		else
		{
			if(pos >= len-1 || KeyCode<  0x20 ) 
			{
				continue;
			}
			if(bShow == TRUE) 
			{
				CD_PrintChar(KeyCode);
			}

			pInputBuf[pos ++] = (char)KeyCode;			
		}		
	}

	 return TRUE;
}



int Ssh2_CallBack(unsigned int code,const char* txtbuf,unsigned int len,unsigned int wp)
{

	switch(code)
	{
		case SSH_SVR_TEXT:
			{			
			terminal_analyze(txtbuf,len);
			s_bLoginOk = TRUE; 
			}
			break;
		case SSH_REQ_ACCOUNT:
			{
			}
			break;
		case SSH_USER_LOGOUT:
			{			
			s_bSsh2Exit = TRUE;			
			}
			break;
		case SSH_NETWORK_ERROR:
			{
			s_bSsh2Exit = TRUE;
			_hx_printf(TIPS_NETERR);						
			}
			break;
		case SSH_OTHER_ERROR:
			{			
			s_bSsh2Exit = TRUE;
			_hx_printf(TIPS_SSHERR,ssh_get_errid(s_pSshObj));			
			}
			break;
		case SSH_AUTH_ERROR:
			{
			s_bSsh2Exit = TRUE;
			_hx_printf(TIPS_AUTHERR);						
			}
			break;
	}
	
	return S_OK;
}

INT  MakeKeyMsg(BYTE bKeyCode,LPSTR pMsgBuf)
{
	INT nMsgLen = 0;

	/*if(bKeyCode == VK_BACKSPACE &&  IsReachPrompt())
	{
		return nMsgLen;
	}*/

	
	if(s_KeyState[VK_CONTROL] == 1)
	{
		if(bKeyCode >= 'a' && bKeyCode <= 'z')
		{
			pMsgBuf[0]  = bKeyCode-'a'+1;

		}
		else if(bKeyCode >= 'A' && bKeyCode <= 'Z')
		{
			pMsgBuf[0]  = bKeyCode-'A'+1;
		}
		else
		{
			pMsgBuf[0]  = bKeyCode;
		}

		nMsgLen = 1;
		
	}
	else if(s_KeyState[VK_ALT] == 1)	
	{
		pMsgBuf[0]  = 0x1B;
		pMsgBuf[1]  = bKeyCode;
		nMsgLen     = 2;
	}
	else 
	{
		pMsgBuf[0]  = bKeyCode;
		nMsgLen     = 1;
	}


	return nMsgLen;
}


INT  MakeVkeyMsg(BYTE bVKeyCode,LPSTR pMsgBuf)
{
	INT nMsgLen = 0;

	if(bVKeyCode >= VK_F1 && bVKeyCode >= VK_F12)
	{
		return 0;
	}
	
	if(bVKeyCode == VK_SHIFT || bVKeyCode == VK_CONTROL || bVKeyCode == VK_ALT || bVKeyCode == VK_CAPS_LOCK)
	{
		return 0;
	}

	if(bVKeyCode >= VK_INSERT && bVKeyCode <= VK_DELETE )
	{
		/*BYTE  bMap[6] = {0x32,0x31,0x35,0x36,0x34,0x33};

		pMsgBuf[0] = 0x1B;
		pMsgBuf[1] = 0x5B;
		pMsgBuf[1] = bMap[bVKeyCode-VK_INSERT];
		pMsgBuf[3] = 0x7E;
		nMsgLen    = 4;*/

	}else if(bVKeyCode >= VK_LEFTARROW && bVKeyCode <= VK_DOWNARROW )
	{
		BYTE  bMap[4] = {'D','A','C','B'};

		pMsgBuf[0]    = 0x1B;

		if(terminal_cmdline() == 1)
		{
			pMsgBuf[1]     = '[';
		}
		else 
		{
			pMsgBuf[1]    = 'O';
		}

		pMsgBuf[2] = bMap[bVKeyCode-VK_LEFTARROW];	
		nMsgLen    = 3; 
	}else
	{
		pMsgBuf[0] = bVKeyCode;
		nMsgLen    = 1; 
	}

	return nMsgLen;
}

DWORD Ssh2SessionEntry(void* pSshObj)
{	
	BYTE KeyCode  = 0;

	while(TRUE)
	{
		CHAR szInput [32]  = {0};
		BOOL bVk           = FALSE;
		INT  nMsgLen        = 0;						

		KeyCode  = GetKeyInput(&bVk);
		
		if(s_bSsh2Exit )     break;	
		if(KeyCode == 0) 	 continue;

		if(bVk == TRUE)
		{
			nMsgLen = MakeVkeyMsg(KeyCode,szInput);
		}
		else
		{
			nMsgLen = MakeKeyMsg(KeyCode,szInput);
		}
		
		if(nMsgLen == 0) continue;
		
		
		if(ssh_send_msg(pSshObj,szInput,nMsgLen) < 0)
		{
			PrintLine(" Ssh disconnect!");
			break;
		}	
	}

	return S_OK;
}

DWORD Ssh2Handler(__CMD_PARA_OBJ* lpCmdObj)
{	
	int    nConnectOk  = 0;
	int    remote      = 1;

	//return MemCheckHandler(lpCmdObj);

	if(lpCmdObj->byParameterNum < 3)
	{
		PrintLine("params input error!");
		return SHELL_CMD_PARSER_TERMINAL;
	}	
	
	s_bLoginOk  = FALSE;
	s_bSsh2Exit = FALSE;
	memset(s_KeyState,0,sizeof(s_KeyState));
		
	
	s_pSshObj = ssh_new_seesion(lpCmdObj->Parameter[1],atoi(lpCmdObj->Parameter[2]),Ssh2_CallBack,0);	

	//if(remote)
	//{
		//remote 
	//	s_pSshObj = ssh_new_seesion("123.57.191.213",2222,Ssh2_CallBack,0);	
	//}
	//else
	//{
		//local
	//	s_pSshObj = ssh_new_seesion("192.168.70.129",22,Ssh2_CallBack,0);	
	//}
	
	
	if(!s_pSshObj)
	{
		return SHELL_CMD_PARSER_TERMINAL;
	}
	else
	{	
		CHAR  szUser[64] = {0};
		CHAR  szPass[64] = {0};
		WORD  wRows,wCols;

		GetInputInfo(" Please Input Ssh Username:",TRUE,szUser,sizeof(szUser)-1);		
		GetInputInfo(" Please Input Ssh Password:",FALSE,szPass,sizeof(szPass)-1);
		
		CD_GetDisPlayRang(&wRows,&wCols);
		ssh_set_terminal(s_pSshObj,wRows,wCols);

		ssh_set_account(s_pSshObj,szUser,szPass);

		/*if(remote)
		{
			ssh_set_account(s_pSshObj,"root","HelloXAdmin321");
		}
		else
		{
			ssh_set_account(s_pSshObj,"root","hjf12345");
		}*/

		terminal_init();		
		nConnectOk = ssh_start_seesion(s_pSshObj);
	}
		
	if(nConnectOk <= 0)
	{
		PrintLine(" SSH2:connect faild");
	}
	else
	{
		Ssh2SessionEntry(s_pSshObj); //enter ssh session	
	}	

	ssh_free_seesion(s_pSshObj);
	terminal_uninit();
	
	return SHELL_CMD_PARSER_SUCCESS;	
}


