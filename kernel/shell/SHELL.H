//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : shell.h
//    Module Funciton           : 
//                                This module countains the definations,
//                                data types,and procedures.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SHELL_H__
#define __SHELL_H__

/* Maximal length of command buffer. */
#define MAX_BUFFER_LEN 256
#define MAX_CMD_LEN    256

/* History command. */
typedef struct HIS_CMD_OBJ
{
	char CmdStr[MAX_CMD_LEN];
} HIS_CMD_OBJ;

/* 
 * Maximal parameters in one 
 * command parameter object. 
 */
#define CMD_PARAMETER_COUNT   64
/* Maximal length of one command string. */
#define CMD_PARAMETER_LEN     64

/* The max Count of one history command . */
#define HISCMD_MAX_COUNT      16

/* 
 * Command parameter object,each command line from user's 
 * input could be represented as one of this object.
 * Each string separated by space in command line corresponds
 * one parameter.
 */
typedef struct tag__CMD_PARA_OBJ
{
	BYTE byParameterNum;
	WORD wReserved;
	char* Parameter[CMD_PARAMETER_COUNT];
}__CMD_PARA_OBJ;

/* Handler proto-type of shell command. */
typedef DWORD (*__CMD_HANDLER)(__CMD_PARA_OBJ* pCmdParaObj);
typedef struct{
	LPSTR          CmdStr;               //Command text.
	__CMD_HANDLER  CmdHandler;           //Command handler.
} __CMD_OBJ;

/* Calculates the next parameter object's address. */
#define NextParaAddr(currentaddr,paranum)  (LPVOID)((BYTE*)currentaddr + 8 + (CMD_PARAMETER_LEN + 1) * paranum)

/* Create a parameter objects from a command line string. */
__CMD_PARA_OBJ* FormParameterObj(LPCSTR);

/* Copy one parameter objects to a new parameter object. */
__CMD_PARA_OBJ* CopyParameterObj(__CMD_PARA_OBJ* lpParamObj, BYTE nStart);

/* Clear all invisible Char in a string. */
VOID ClearUnVisableChar(LPCSTR pszStr);

/* Release the command line object created by FormParameterObj routine. */
VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj);

/* Entry point of text mode's shell. */
DWORD ShellEntryPoint(LPVOID pData);

typedef VOID* HISOBJ;

/* Routines to handle command history recording. */
HISOBJ His_CreateHisObj(INT nHisCount);
VOID   His_DeleteHisObj(HISOBJ hHisObj);
BOOL   His_SaveCmd(HISOBJ hHisObj,LPCSTR pCmdStr);
BOOL   His_LoadHisCmd(HISOBJ hHisObj,BOOL bUp,LPSTR pCmdBuf,INT nBufLen);

/* Execution result of shell command. */
#define SHELL_CMD_PARSER_TERMINAL      0x00000000
#define SHELL_CMD_PARSER_SUCCESS       0x00000001
#define SHELL_CMD_PARSER_INVALID       0x00000002
#define SHELL_CMD_PARSER_PROCESSING    0x00000003
#define SHELL_CMD_PARSER_FAILED        0x00000004
#define SHELL_CMD_PARSER_NULL          0xFFFFFFFF

#define  SHELL_QUERY_CONTINUE          0x1
#define  SHELL_QUERY_CANCEL            0x2

/* Command handler of the shell. */
typedef DWORD (*__SHELL_CMD_HANDLER)(LPSTR);
/* Complate command automatically. */
typedef DWORD (*__SHELL_NAMEQUERY_HANDLER)(LPSTR,INT);


/* Shell loop to process user input. */
DWORD Shell_Msg_Loop(const char* pPrompt, __SHELL_CMD_HANDLER pCmdRoute,
	__SHELL_NAMEQUERY_HANDLER pNameAuto);

#endif //__SHELL_H__
