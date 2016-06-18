//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2016
//    Module Name               : usbvideo.c
//    Module Funciton           : 
//    Description               : 
//                                USB video diagnostic application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include <StdAfx.h>

#include "kapi.h"
#include "shell.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "usbvideo.h"

#define  USBVIDEO_PROMPT_STR   "[uvc_view]"

#if defined(__CFG_DRV_UVC) && defined(__CFG_SYS_USB)

//
//Pre-declare routines.
//
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);        //help sub-command's handler.
static DWORD _exit(__CMD_PARA_OBJ*);       //exit sub-command's handler.
static DWORD fmtlist(__CMD_PARA_OBJ*);     //Handler of fmtlist.
static DWORD fmtinfo(__CMD_PARA_OBJ*);     //Handler of fmtinfo.
static DWORD setcurr(__CMD_PARA_OBJ*);     //Hanlder of setcurr.
static DWORD start(__CMD_PARA_OBJ*);       //Handler of start.

//
//The following is a map between command and it's handler.
//
static struct __USBVIDEO_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD(*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}UsbVideoCmdMap[] = {
	{ "exit", _exit,      "  exit    : Exit the application." },
	{ "help", help,       "  help    : Print out this screen." },
	{ "fmtlist", fmtlist, "  fmtlist : Show all available video format." },
	{ "fmtinfo", fmtinfo, "  fmtinfo : Show verbose information of a specified format." },
	{ "setcurr", setcurr, "  setcurr : Set current format and frame type." },
	{ "start", start,     "  start   : Start to streaming." },
	{ NULL, NULL, NULL }
};

static DWORD QueryCmdName(LPSTR pMatchBuf, INT nBufLen)
{
	static DWORD dwIndex = 0;

	if (pMatchBuf == NULL)
	{
		dwIndex = 0;
		return SHELL_QUERY_CONTINUE;
	}

	if (NULL == UsbVideoCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;
	}

	strncpy(pMatchBuf, UsbVideoCmdMap[dwIndex].lpszCommand, nBufLen);
	dwIndex++;

	return SHELL_QUERY_CONTINUE;
}

//
//The following routine processes the input command string.
//
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj = NULL;

	if ((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);


	if (NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	//{
	//	return SHELL_CMD_PARSER_FAILED;
	//}
	//CD_PrintString(lpCmdParamObj->Parameter[0],TRUE);

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.Calls the corresponding command handler if found,otherwise SYS_DIAG_CMD_PARSER_INVALID
	//will be returned to indicate this case.
	//
	while (TRUE)
	{
		if (NULL == UsbVideoCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if (StrCmp(UsbVideoCmdMap[dwIndex].lpszCommand, lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = UsbVideoCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex++;
		}
	}

	//Release parameter object.
	if (NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

//
//This is the application's entry point.
//
DWORD UsbVideoStart(LPVOID p)
{
	return Shell_Msg_Loop(USBVIDEO_PROMPT_STR, CommandParser, QueryCmdName);
}

//
//The exit command's handler.
//
static DWORD _exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while (TRUE)
	{
		if (NULL == UsbVideoCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(UsbVideoCmdMap[dwIndex].lpszHelpInfo);
		dwIndex++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

//External routines that handle the actual user request.
extern void uvcFormatList();
extern void uvcFormatInfo(int index);
extern void uvcSetCurrent(int fmt, int frm);
extern void uvcStart();

//Handler of fmtlist command.
static DWORD fmtlist(__CMD_PARA_OBJ* pCmdObj)
{
	uvcFormatList();
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler of fmtinfo command.
static DWORD fmtinfo(__CMD_PARA_OBJ* pCmdObj)
{
	int fmt = 1;
	if (pCmdObj->byParameterNum < 2)
	{
		_hx_printf("  Please specify the format index you want to show.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	fmt = atol(pCmdObj->Parameter[1]);
	uvcFormatInfo(fmt);
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler of setcurr command.
static DWORD setcurr(__CMD_PARA_OBJ* pCmdObj)
{
	int fmt = 1, frm = 1;
	if (pCmdObj->byParameterNum < 3)
	{
		_hx_printf("  Please specify the format and frame index you want to set.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	fmt = atol(pCmdObj->Parameter[1]);
	frm = atol(pCmdObj->Parameter[2]);
	uvcSetCurrent(fmt,frm);
	return SHELL_CMD_PARSER_SUCCESS;
}

//Handler of start command.
static DWORD start(__CMD_PARA_OBJ* pCmdObj)
{
	uvcStart();
	return SHELL_CMD_PARSER_SUCCESS;
}

#endif //#if defined(__CFG_DRV_UVC) && defined(__CFG_SYS_USB)
