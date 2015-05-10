//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : EXTCMD.CPP
//    Module Funciton           : 
//                                This module countains Hello China's External 
//                                command's definition.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

typedef struct{
	LPSTR lpszCmdName;
	LPSTR lpszHelpInfo;
	BOOL  bBackground;
	DWORD (*ExtCmdHandler)(LPVOID);
}__EXTERNAL_COMMAND;

extern __EXTERNAL_COMMAND ExtCmdArray[];

