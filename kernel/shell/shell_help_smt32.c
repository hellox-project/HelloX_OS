//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct,27 2014
//    Module Name               : SHELL_HELP.c
//    Module Funciton           : 
//                                This module countains shell input  routes
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "kapi.h"
#include "chardisplay.h"
#include "shell.h"


#define  ERROR_STR   "You entered incorrect command name."
#define  ERROR_STR2  "Failed to process the command."

static char   s_szCurCmd[CMD_MAX_LEN] = {0};
static DWORD  s_nCurCmdPos            = 0;

typedef struct tag__SHELL_MSG_INFO
{
	HISOBJ                    hHisInfoObj;
	LPSTR                     pPrompt;
	__SHELL_CMD_HANDLER       pCmdRoute;
	__SHELL_NAMEQUERY_HANDLER pNameQuery;

}SHELL_MSG_INFO;


static VOID PrintPrompt(SHELL_MSG_INFO*  pShellInfo)
{
	CD_PrintString(pShellInfo->pPrompt,FALSE);
}

//The following function form the command parameter object link from the command
//line string.
__CMD_PARA_OBJ* FormParameterObj(LPCSTR pszCmd)
{
	__CMD_PARA_OBJ*     pObjBuffer    = NULL;  
	LPSTR               pCmdPos       = (LPSTR)pszCmd;
	BYTE                byParamPos    = 0;
	BYTE                byParamNum    = 0;

	if(NULL == pCmdPos)    //Parameter check.
	{
		return NULL;
	}

	pObjBuffer = (__CMD_PARA_OBJ*)KMemAlloc(sizeof(__CMD_PARA_OBJ),KMEM_SIZE_TYPE_ANY);
	if(NULL == pObjBuffer)
	{
		return NULL;
	}
	memzero(pObjBuffer,sizeof(__CMD_PARA_OBJ));

	while(*pCmdPos)
	{
		if(' ' == *pCmdPos)
		{
			pCmdPos ++;
			continue; 
		}    

		// add papam
		while((' ' != *pCmdPos) && (*pCmdPos) && (byParamPos <= CMD_PARAMETER_LEN))
		{
			if(pObjBuffer->Parameter[byParamNum] == NULL)
			{
				pObjBuffer->Parameter[byParamNum] = (CHAR*)KMemAlloc(CMD_PARAMETER_LEN+1,KMEM_SIZE_TYPE_ANY);
				memzero(pObjBuffer->Parameter[byParamNum],CMD_PARAMETER_LEN+1);
			}

			pObjBuffer->Parameter[byParamNum][byParamPos] = *pCmdPos;
			pCmdPos   ++;
			byParamPos ++;
		}

		byParamNum ++;                                     
		byParamPos = 0;

		if(byParamNum >= CMD_PARAMETER_COUNT)
		{
			break;
		}

		//Skip the no space characters if the parameter's length
		while(' ' != *pCmdPos && *pCmdPos)
		{					
			pCmdPos ++;          
		}	
	}

	pObjBuffer->byParameterNum  = byParamNum;

	return pObjBuffer;
}

//
//Releases the parameter object created by FormParameterObj routine.
//
VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj)
{	
	BYTE     Index      = 0;

	if(NULL == lpParamObj)  //Parameter check.
	{
		return;
	}

	for(Index = 0;Index < lpParamObj->byParameterNum;Index ++)
	{
		KMemFree((LPVOID)lpParamObj->Parameter[Index],KMEM_SIZE_TYPE_ANY,0);
	}

	KMemFree((LPVOID)lpParamObj,KMEM_SIZE_TYPE_ANY,0);
	
	return;
}

//回退一个字符
static void BackChar()
{
	CHAR  szBs[4]     = {8,' ',8,0};
	
    CD_PrintString(szBs,FALSE);
}

//load history cmd to current cmd line
static 	void LoadHisCmd(SHELL_MSG_INFO*  pShellInfo,BOOL bUp)
{
	CHAR   szHisCmd[CMD_MAX_LEN] = {0};   
	WORD   CursorX               = 0;
	WORD   CursorY               = 0;

	if(His_LoadHisCmd(pShellInfo->hHisInfoObj,bUp,szHisCmd,sizeof(szHisCmd)) == FALSE)
	{		
		return; 
	}
	
	//清除原有输入
	if(s_nCurCmdPos)
	{		
		DWORD   i   = 0;
				
		//字符回退
		for(i=0;i<s_nCurCmdPos;i++)
		{
			BackChar();
		}		
	}
			
	CD_PrintString(szHisCmd,FALSE);
	strcpy(s_szCurCmd,szHisCmd);
	s_nCurCmdPos = strlen(s_szCurCmd);

}


//查找字符串是否存在
static BOOL FindSub(const char*  srcstr,const char* substr)
{
	char*  srcp    = (char*)srcstr;
	char*  subp    = (char*)substr;
	BOOL   bFound  = TRUE;

	while(*srcp != 0 && *subp != 0)
	{
		if(*srcp != *subp)
		{
			bFound = FALSE;

			break;
		}

		srcp ++;
		subp ++;
	}

	return bFound;
}



static BOOL OnAutoComplete(SHELL_MSG_INFO*  pShellInfo)
{
		
	//检查是否有匹配项
	if(strlen(s_szCurCmd) > 0 && pShellInfo->pNameQuery)
	{	
		//清空
		pShellInfo->pNameQuery(NULL,0);

		//逐个询问匹配
		while(TRUE)
		{
			CHAR  szCmdNmae[CMD_MAX_LEN] = {0};
						
			if(pShellInfo->pNameQuery(szCmdNmae,sizeof(szCmdNmae)) == SHELL_QUERY_CANCEL)
			{
				break;
			}
			
			if(FindSub(szCmdNmae,s_szCurCmd) && strlen(szCmdNmae) > strlen(s_szCurCmd))
			{	
				DWORD  i ;
					
				for(i=0;i<s_nCurCmdPos;i++)
				{
					BackChar();
				}
				strcpy(s_szCurCmd,szCmdNmae);
				s_nCurCmdPos = strlen(s_szCurCmd);
				CD_PrintString(s_szCurCmd,FALSE);
					
				break;
			}				
		}				
	}

	return TRUE;
}
//com Control msg
static INT OnComControl(SHELL_MSG_INFO*  pShellInfo)
{
	__KERNEL_THREAD_MESSAGE     Msg;

	GetMessage(&Msg);
	GetMessage(&Msg);

	switch(Msg.dwParam)
	{
		case 'A':
		{
			//up arraw=0x1b,'A','A'						
			LoadHisCmd(pShellInfo,TRUE);
		}
		break;
		case 'B':
		{
			//down arraw=0x1b,'B','B'
			LoadHisCmd(pShellInfo,FALSE);
		}
		break;
		case 'C':
		{
			//right arraw=0x1b,'C','C'
			OnAutoComplete(pShellInfo);			
		}
		break;
	}

	return TRUE;
}

static INT OnInputReturn(SHELL_MSG_INFO*  pShellInfo)
{
	CHAR   szCmdBuffer[CMD_MAX_LEN] = {0};			

	strcpy(szCmdBuffer,s_szCurCmd);
	strtrim(szCmdBuffer,TRIM_LEFT|TRIM_RIGHT);
	CD_ChangeLine();

	memset(s_szCurCmd,0,sizeof(s_szCurCmd));
	s_nCurCmdPos = 0;

	if(strlen(szCmdBuffer) > 0)
	{
		His_SaveCmd(pShellInfo->hHisInfoObj,szCmdBuffer);

		switch(pShellInfo->pCmdRoute(szCmdBuffer))
		{
		case SHELL_CMD_PARSER_TERMINAL: //Exit command is entered.
			{
				return FALSE;
			}
			break;
		case SHELL_CMD_PARSER_INVALID:  //Can not parse the command.
			{
				CD_PrintString(ERROR_STR,TRUE);
			}							
			break;
		case SHELL_CMD_PARSER_FAILED:
			{
				CD_PrintString(ERROR_STR2,TRUE);
			}
			break;
		default:
			break;
		}
	}						
	PrintPrompt(pShellInfo);

	return TRUE;
}

static INT OnBackSpace(SHELL_MSG_INFO*  pShellInfo)
{
	if(s_nCurCmdPos > 0)
	{				
		s_szCurCmd[s_nCurCmdPos ] = 0;
		s_nCurCmdPos --;

		BackChar();				
	}

	return TRUE;
}

static INT OnInputChar(SHELL_MSG_INFO*  pShellInfo,BYTE bt)
{
	//缓冲已满或非可见字符则略过
	if(s_nCurCmdPos < sizeof(s_szCurCmd)-1 && bt >= 0x20 && bt <= 0x7e)
	{
		s_szCurCmd[s_nCurCmdPos ++] = bt;
		CD_PrintChar(bt);
	}

	return TRUE;
}

//key  msg 
static INT OnKeyControl(SHELL_MSG_INFO*  pShellInfo,BYTE   bt )
{
    #define VK_COM_CONTROL 0x1B     //串口输入控制符前缀
	INT  nRet  = TRUE; 

	switch(bt)
	{
		case VK_COM_CONTROL:
		{
			nRet = OnComControl(pShellInfo);
		}
		break;
		case VK_RETURN:
		{
			nRet = OnInputReturn(pShellInfo);
		}
		break;
		case VK_BACKSPACE:
		{
			nRet = OnBackSpace(pShellInfo);
		}
		break;
		default:
		{
			nRet = OnInputChar(pShellInfo,bt);
		}
	}

	return nRet;
}

//shell 输入 循环处理
DWORD Shell_Msg_Loop(const char* pPrompt,__SHELL_CMD_HANDLER pCmdRoute,__SHELL_NAMEQUERY_HANDLER pNameQuery)
{
	SHELL_MSG_INFO*  pShellInfo  = NULL;
	

	if(NULL == pPrompt || NULL == pCmdRoute)
	{
		return 0;
	}

	pShellInfo = (SHELL_MSG_INFO*)KMemAlloc(sizeof(SHELL_MSG_INFO),KMEM_SIZE_TYPE_ANY);
	if(NULL == pShellInfo)
	{
		return 0;
	}

	pShellInfo->pPrompt      = (LPSTR)pPrompt;
	pShellInfo->pCmdRoute    = pCmdRoute;
	pShellInfo->pNameQuery   = pNameQuery;

	pShellInfo->hHisInfoObj  = His_CreateHisObj(HISCMD_MAX_COUNT);//HISCMD_MAX_COUNT

	CD_ChangeLine();
	PrintPrompt(pShellInfo);

	while(TRUE)
	{
		__KERNEL_THREAD_MESSAGE     Msg;

		if(GetMessage(&Msg))
		{
		  BYTE bt = (BYTE)(Msg.dwParam);

			switch(Msg.wCommand)
			{
			case MSG_KEY_DOWN:
				{				
				 if(OnKeyControl(pShellInfo,bt) == FALSE)	
					{
						goto __TERMINAL;
					}
				}
				break;
			case KERNEL_MESSAGE_TIMER:
				{
					PrintLine("Timer message received.");
				}
				break;
			case KERNEL_MESSAGE_TERMINAL: 
				{
				goto __TERMINAL;
				}
				break;
			}
		}
	}

__TERMINAL:

	His_DeleteHisObj(pShellInfo->hHisInfoObj);
	KMemFree(pShellInfo,KMEM_SIZE_TYPE_ANY,0);

	return 0;
}
