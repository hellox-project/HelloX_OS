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

#ifndef __SHELL__
#define __SHELL__

#define MAX_BUFFER_LEN 256       //The max length of command buffer.
#define MAX_CMD_LEN    16        //The max length of one command.

//
//Shell data structures.


typedef struct HIS_CMD_OBJ
{
	CHAR    CmdStr[64];  //Command text.

} HIS_CMD_OBJ;



#define CMD_PARAMETER_COUNT   16             //The max count of parameter.

#define CMD_PARAMETER_LEN     64            //The max length of one parameter.

#define CMD_MAX_LEN           64            //The max length of one command input. 

#define HISCMD_MAX_COUNT      16             //The max Count of one history command . 

//Command paramter object,countains the parameters and function labels of one
//command.
//

typedef struct tag__CMD_PARA_OBJ
{
	BYTE         byParameterNum;         //How many parameters  followed.
	WORD         wReserved;

	CHAR*       Parameter[CMD_PARAMETER_COUNT];
	//CHAR         Parameter[CMD_PARAMETER_COUNT][CMD_PARAMETER_LEN + 1]; //The parameters.

}__CMD_PARA_OBJ;

typedef DWORD (*__CMD_HANDLER)(__CMD_PARA_OBJ* pCmdParaObj);  //Command handler.
typedef struct{
	LPSTR          CmdStr;               //Command text.
	//__tagCMD_OBJ*  pPrev;                //Point to the previous command object.
	//__tagCMD_OBJ*  pNext;                //Point to the next.
	//__tagCMD_OBJ*  pParent;              //Point to the parent.
	__CMD_HANDLER  CmdHandler;           //Command handler.
} __CMD_OBJ;


//
//The following macro calculates the next parameter object's address.
//
#define NextParaAddr(currentaddr,paranum)  (LPVOID)((BYTE*)currentaddr + 8 + (CMD_PARAMETER_LEN + 1) * paranum)

//
//Form the parameter objects link from a command line string.
//
__CMD_PARA_OBJ* FormParameterObj(LPCSTR);

//
//Release the command line object created by FormParameterObj routine.
//
VOID ReleaseParameterObj(__CMD_PARA_OBJ* lpParamObj);

//Text mode shell's entry point.
DWORD ShellEntryPoint(LPVOID pData);

typedef VOID* HISOBJ;
//history command routine;
HISOBJ His_CreateHisObj(INT nHisCount);
VOID   His_DeleteHisObj(HISOBJ hHisObj);
BOOL   His_SaveCmd(HISOBJ hHisObj,LPCSTR pCmdStr);
BOOL   His_LoadHisCmd(HISOBJ hHisObj,BOOL bUp,LPSTR pCmdBuf,INT nBufLen);

//shell help 

#define SHELL_CMD_PARSER_TERMINAL      0x00000000
#define SHELL_CMD_PARSER_SUCCESS       0x00000001
#define SHELL_CMD_PARSER_INVALID       0x00000002
#define SHELL_CMD_PARSER_PROCESSING    0x00000003
#define SHELL_CMD_PARSER_FAILED        0x00000004
#define SHELL_CMD_PARSER_NULL          0xFFFFFFFF


#define  SHELL_QUERY_CONTINUE          0x1
#define  SHELL_QUERY_CANCEL            0x2

typedef DWORD (*__SHELL_CMD_HANDLER)(LPSTR);             //shell Command handler.
typedef DWORD (*__SHELL_NAMEQUERY_HANDLER)(LPSTR,INT);  //shell name auto complete.


// shell 输入循环处理
DWORD Shell_Msg_Loop(const char* pPrompt,__SHELL_CMD_HANDLER pCmdRoute,__SHELL_NAMEQUERY_HANDLER pNameAuto);

#endif //shell.h
