//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 15 FEB,2009
//    Module Name               : FS.CPP
//    Module Funciton           : 
//    Description               : Implementation code of fs application.
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
#include "fs.h"

#include "kapi.h"
#include "string.h"
#include "stdio.h"


#define  FS_PROMPT_STR   "[fs_view]"

static HISOBJ            s_hHiscmdInoObj   = NULL;

static struct __FS_GLOBAL_DATA{
	__FS_ARRAY_ELEMENT  FsArray[FILE_SYSTEM_NUM];
	BYTE                CurrentFs;                 //Current file system identifier.
	CHAR                CurrentDir[MAX_FILE_NAME_LEN];
	BOOL                bInitialized;              //Flag indicates if this structure has been inited.
}FsGlobalData = {0};
static CHAR   Buffer[256] = {0};                   //Local buffer used by this thread to print info out.

//
//Pre-declare routines.
//
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);        //help sub-command's handler.
static DWORD exit(__CMD_PARA_OBJ*);        //exit sub-command's handler.
static DWORD fslist(__CMD_PARA_OBJ*);
static DWORD dir(__CMD_PARA_OBJ*);
static DWORD cd(__CMD_PARA_OBJ*);
static DWORD vl(__CMD_PARA_OBJ*);
static DWORD md(__CMD_PARA_OBJ*);
static DWORD mkdir(__CMD_PARA_OBJ*);
static DWORD del(__CMD_PARA_OBJ*);
static DWORD rd(__CMD_PARA_OBJ*);
static DWORD ren(__CMD_PARA_OBJ*);
static DWORD type(__CMD_PARA_OBJ*);
static DWORD copy(__CMD_PARA_OBJ*);
static DWORD use(__CMD_PARA_OBJ*);
static DWORD init();                     //Initialize routine.

//
//The following is a map between command and it's handler.
//
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"fslist",     fslist,    "  fslist   : Show all available file systems."},
	{"dir",        dir,       "  dir      : Show current directory's file list."},
	{"cd",         cd,        "  cd       : Show or change current directory."},
	{"vl",         vl,        "  vl       : Change current fs's volume label."},
	{"md",         md,        "  md       : Create a new directory."},
	{"mkdir",      mkdir,     "  mkdir    : Alias command of md."},
	{"del",        del,       "  del      : Delete one file from current directory."},
	{"rd",         rd,        "  rd       : Delete one sub-directory from current directory."},
	{"ren",        ren,       "  ren      : Change file or directory's name."},
	{"type",       type,      "  type     : Show a specified file's content."},
	{"copy",       copy,      "  copy     : Copy file to other location,or reverse."},
	{"use",        use,       "  use      : Set current file system."},
	{"exit",       exit,      "  exit     : Exit the application."},
	{"help",       help,      "  help     : Print out this screen."},
	{NULL,		   NULL,      NULL}
};

static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}


	if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,SysDiagCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}
//
//The following routine processes the input command string.
//
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.If find,then calls the handler,else,return SYS_DIAG_CMD_PARSER_INVALID
	//to indicate the failure.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

//__TERMINAL:
	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

DWORD fsEntry(LPVOID p)
{
	if(0 == init())  //Can not finish the initialization work.
	{
		PrintLine("  Can not initialize the FS thread.");
		return 0;
	}

	return Shell_Msg_Loop(FS_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD exit(__CMD_PARA_OBJ* lpCmdObj)
{
	memzero(&FsGlobalData,sizeof(struct __FS_GLOBAL_DATA));

	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD fslist(__CMD_PARA_OBJ* pcpo)
{
	CHAR                Buffer[96];
	int                 i;

	PrintLine("  fs_id       fs_label     fs_type");
	PrintLine("  -----       --------     -------");

	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(0 == FsGlobalData.FsArray[i].FileSystemIdentifier)  //Not used yet.
		{
			continue;
		}
		_hx_sprintf(Buffer,"  %4c:    %11s    %8X",
			FsGlobalData.FsArray[i].FileSystemIdentifier,
			FsGlobalData.FsArray[i].VolumeLbl,
			FsGlobalData.FsArray[i].dwAttribute);
		PrintLine(Buffer);
	}
	return SHELL_CMD_PARSER_SUCCESS;;
}

//A local helper routine to print the directory list,used by dir command.
static VOID PrintDir(FS_FIND_DATA* pFindData)
{
	CHAR    Buffer[MAX_FILE_NAME_LEN] = {0};

	if(!pFindData->bGetLongName)
	{
		_hx_sprintf(Buffer,"%13s    %16d    %4s",
					pFindData->cAlternateFileName,
		            pFindData->nFileSizeLow,
		            (pFindData->dwFileAttribute & FILE_ATTR_DIRECTORY) ? "DIR" : "FILE");
	}
	else
	{
		_hx_sprintf(Buffer,"%s",strlen(pFindData->cFileName)?pFindData->cFileName:pFindData->cAlternateFileName);
	}

	PrintLine(Buffer);	
}

static DWORD dir(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	FS_FIND_DATA      ffd         = {0};
	__COMMON_OBJECT*  pFindHandle = NULL;
	BOOL              bFindNext   = FALSE;
	
	strcpy(Buffer,FsGlobalData.CurrentDir);
	/*if(pCmdObj->byParameterNum >= 2)  //Target directory specified.
	{
		strcat(Buffer,pCmdObj->Parameter[1]);
	}*/
	ToCapital(Buffer);  //Convert to capital.
	
	if(pCmdObj->byParameterNum >= 2 && strcmp(pCmdObj->Parameter[1],"-l") == 0)
	{
		ffd.bGetLongName = TRUE;
	}
		
	pFindHandle = IOManager.FindFirstFile((__COMMON_OBJECT*)&IOManager,	Buffer,	&ffd);
	if(NULL == pFindHandle)  //Can not open the target directory.
	{		
		PrintLine("Can not open the specified or current directory to display.");
		goto __TERMINAL;
	}

	//Dump out the directory's content.
	if(ffd.bGetLongName)	
	{		
		PrintLine("File_Name                                ");
	}
	else
	{	
		PrintLine("    File_Name            FileSize     F_D");
	}
	PrintLine("--------------------------------------------");

	do{
		PrintDir(&ffd);
		bFindNext = IOManager.FindNextFile((__COMMON_OBJECT*)&IOManager,
			Buffer,
			pFindHandle,
			&ffd);

	}while(bFindNext);

	PrintLine("--------------------------------------------");
	//Close the find handle.
	IOManager.FindClose((__COMMON_OBJECT*)&IOManager,
		Buffer,
		pFindHandle);

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD cd(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	CHAR     NewPath[MAX_FILE_NAME_LEN];
	CHAR*    pCurrPos  = NULL;
	DWORD    dwFileAttr = 0;

	if(1 == pCmdObj->byParameterNum)  //Print out the current directory.
	{
		_hx_sprintf(Buffer,"  Current directory is: %s",FsGlobalData.CurrentDir);
		PrintLine(Buffer);
		goto __TERMINAL;
	}
	if(0 == strcmp(".",pCmdObj->Parameter[1]))  //Current directory.
	{
		goto __TERMINAL;
	}
	if(0 == strcmp("..",pCmdObj->Parameter[1])) //Parent directory.
	{
		PrintLine("  Can not supported yet.:-)");
		goto __TERMINAL;
	}
	if(0 == strcmp("\\",pCmdObj->Parameter[1])) //Root directory is specified.
	{
		FsGlobalData.CurrentDir[0] = FsGlobalData.CurrentFs;
		FsGlobalData.CurrentDir[1] = ':';
		FsGlobalData.CurrentDir[2] = '\\';
		FsGlobalData.CurrentDir[3] = 0;
		goto __TERMINAL;
	}
	//Want to enter a new directory.
	strcpy(NewPath,FsGlobalData.CurrentDir);
	strcat(NewPath,pCmdObj->Parameter[1]);      //Form the new path.
	strcat(NewPath,"\\");                       //Specify a slash.
	ToCapital(NewPath);                         //Convert to capital.
	if((dwFileAttr = IOManager.GetFileAttributes((__COMMON_OBJECT*)&IOManager,
		NewPath)) == 0)  //Use GetFileAttributes to verify the validity of new directory.
	{
		PrintLine("  Bad target directory name.");
		goto __TERMINAL;
	}
	if((dwFileAttr & FILE_ATTR_DIRECTORY) == 0)   //Is not a directory,is a regular file.
	{
		PrintLine("  Please specify a DIRECTORY name,not a file name.");
		goto __TERMINAL;
	}
	//Validation is ok,set the new path as current directory.
	strcpy(FsGlobalData.CurrentDir,NewPath);
__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD md(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	CHAR  DirPath[MAX_FILE_NAME_LEN];

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("  Please specify the directory name to be created.");
		return SHELL_CMD_PARSER_SUCCESS;;
	}
	strcpy(DirPath,FsGlobalData.CurrentDir);
	strcat(DirPath,pCmdObj->Parameter[1]);
	ToCapital(DirPath);
	if(IOManager.CreateDirectory((__COMMON_OBJECT*)&IOManager,
		DirPath,
		NULL))
	{
		PrintLine("  Create directory successfully.");
	}
	else
	{
		PrintLine("  Create directory failed.");
	}
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

//mkdir is a alias name of md.
static DWORD mkdir(__CMD_PARA_OBJ* pCmdObj)
{
	return md(pCmdObj);
}

static DWORD del(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	CHAR     FullName[MAX_FILE_NAME_LEN];
	BOOL     bResult = FALSE;

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("  Please specify the file name to be deleted.");
		goto __TERMINAL;
	}
	strcpy(FullName,FsGlobalData.CurrentDir);
	strcat(FullName,pCmdObj->Parameter[1]);
	ToCapital(FullName);
	bResult = IOManager.DeleteFile((__COMMON_OBJECT*)&IOManager,
		FullName);
	if(!bResult)
	{
		PrintLine("  Can not delete the specified file.");
		goto __TERMINAL;
	}
__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD rd(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	CHAR     FullName[MAX_FILE_NAME_LEN];
	BOOL     bResult = FALSE;

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("  Please specify the directory name to be deleted.");
		goto __TERMINAL;
	}
	strcpy(FullName,FsGlobalData.CurrentDir);
	strcat(FullName,pCmdObj->Parameter[1]);
	ToCapital(FullName);
	bResult = IOManager.RemoveDirectory((__COMMON_OBJECT*)&IOManager,
		FullName);
	if(!bResult)
	{
		PrintLine("  Can not delete the specified directory.");
		goto __TERMINAL;
	}
__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD ren(__CMD_PARA_OBJ* pcpo)
{
	return SHELL_CMD_PARSER_SUCCESS;;
}

static DWORD vl(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	HANDLE   hFile = NULL;
	CHAR     Buffer[128];
	DWORD    dwWritten  = 0;
	CHAR     FullName[MAX_FILE_NAME_LEN];
	INT      dwCounter  = 6;

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("  Please specify the file name to be displayed.");
		goto __TERMINAL;
	}
	strcpy(FullName,FsGlobalData.CurrentDir);
	strcat(FullName,pCmdObj->Parameter[1]);
	ToCapital(FullName);
	//Try to open the target file.
	hFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		FullName,
		FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS,
		0,
		NULL);
	if(NULL == hFile)
	{
		PrintLine("  Please specify a valid and present file name.");
		goto __TERMINAL;
	}
	while(dwCounter!= 0)
	{
		_hx_sprintf(Buffer,"Wish it can work,this line number is %d.\r\n",dwCounter);

		if(!IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
			hFile,
			strlen(Buffer),
			Buffer,
			&dwWritten))
		{
			_hx_sprintf(Buffer,"  Can not write to file,The line number is %d.",dwCounter);
			PrintLine(Buffer);
			goto __TERMINAL;
		}
		dwCounter --;
	}
__TERMINAL:
	if(hFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hFile);
	}
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD type(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	HANDLE   hFile = NULL;
	CHAR     Buffer[128];
	DWORD    dwReadSize   = 0;
	DWORD    dwTotalRead  = 0;
	DWORD    i;
	CHAR     FullName[MAX_FILE_NAME_LEN];
	WORD     ch = 0x0700;

	if(pCmdObj->byParameterNum < 2)
	{
		PrintLine("  Please specify the file name to be displayed.");
		goto __TERMINAL;
	}
	strcpy(FullName,FsGlobalData.CurrentDir);
	strcat(FullName,pCmdObj->Parameter[1]);
	ToCapital(FullName);
		
	//Try to open the target file.
	hFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		FullName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hFile)
	{
		PrintLine("  Please specify a valid and present file name.");
		goto __TERMINAL;
	}
	//Try to read the target file and display it.
	GotoHome();
	ChangeLine();
	do{
		if(!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
			hFile,
			128,
			Buffer,
			&dwReadSize))
		{
			PrintLine("  Can not read the target file.");
			goto __TERMINAL;
		}
		for(i = 0;i < dwReadSize;i ++)
		{
			if('\r' == Buffer[i])
			{
				GotoHome();
				continue;
			}
			if('\n' == Buffer[i])
			{
				ChangeLine();
				continue;
			}
			ch += Buffer[i];
			PrintCh(ch);
			ch = 0x0700;
		}
		dwTotalRead += dwReadSize;
	}while(dwReadSize == 128);

	GotoHome();
	ChangeLine();
	_hx_sprintf(Buffer,"%d byte(s) read.",dwTotalRead);
	PrintLine(Buffer);

__TERMINAL:
	if(NULL != hFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hFile);
	}
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

static DWORD copy(__CMD_PARA_OBJ* pcpo)
{
	LPSTR    lpInfo1 = "  First parameter is Hello.";
	LPSTR    lpInfo2 = "  Second parameter is China.";
	LPSTR    lpInfo3 = "  Third parameter is Hello China 1.";
	LPSTR    lpInfo4 = "  Third parameter is Hello China 2.";
	LPSTR    lpInfo5 = "  Third parameter is Hello China 3.";
#ifdef __I386__
#ifdef __GCC__

	asm(
		".code32			;"
		"pushl	%9			;"
		"pushl	%8			;"
		"pushl	%7			;"
		"pushl	%6			;"
		"pushl	%5			;"
		"pushl	$0			;"
		"pushl	$0x2D		;"
		"int	$0x7F		;"
		"popl	%%eax		;"
		"popl	%%eax		;"
		"popl	%0			;"
		"popl	%1			;"
		"popl	%2			;"
		"popl	%3			;"
		"popl	%4			;"
		:"=r"(lpInfo1), "=r"(lpInfo2), "=r"(lpInfo3), "=r"(lpInfo4), "=r"(lpInfo5)
		:"r"(lpInfo1), "r"(lpInfo2), "r"(lpInfo3), "r"(lpInfo4), "r"(lpInfo5)
		 );
#else
	__asm{
		push lpInfo5
		push lpInfo4
		push lpInfo3
		push lpInfo2
		push lpInfo1
		push 0
		push 0x2D
		int 0x7f
		pop eax
		pop eax
		pop lpInfo1
		pop lpInfo2
		pop lpInfo3
		pop lpInfo4
		pop lpInfo5
	}
#endif
#endif
	PrintLine("  In copy of fs application.");
	PrintLine(lpInfo1);
	PrintLine(lpInfo2);
	return SHELL_CMD_PARSER_SUCCESS;;
}

static DWORD use(__CMD_PARA_OBJ* pCmdObj)
{
	CHAR     Info[4];
	int      i;
	BOOL     bMatched = FALSE;
	CHAR*    strError = "  Please specify a valid file system identifier.";

	if(1 == pCmdObj->byParameterNum)  //Without any parameter.
	{
		Info[0] = FsGlobalData.CurrentFs;
		Info[1] = ':';
		Info[2] = 0;
		_hx_sprintf(Buffer,"  Current file system is: %s",Info);
		PrintLine(Buffer);
		goto __TERMINAL;
	}
	//Parse the user input.
	if(strlen(pCmdObj->Parameter[1]) > 2)  //File system identifier can not exceed 2 characters.
	{
		PrintLine(strError);
		goto __TERMINAL;
	}
	Info[0] = pCmdObj->Parameter[1][0];
	Info[1] = pCmdObj->Parameter[1][1];
	Info[2] = 0;
	if(Info[1] != ':')
	{
		PrintLine(strError);
		goto __TERMINAL;
	}

	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(FsGlobalData.FsArray[i].FileSystemIdentifier == Info[0])  //FS is in system now.
		{
			bMatched = TRUE;
			break;
		}
	}
	if(!bMatched)  //The specified file system identifier is not in system now.
	{
		PrintLine("  The specified file system is not present now.");
		goto __TERMINAL;
	}
	//Valid file system,change current information.
	FsGlobalData.CurrentDir[0] = Info[0];
	FsGlobalData.CurrentDir[1] = Info[1];
	FsGlobalData.CurrentDir[2] = '\\';
	FsGlobalData.CurrentDir[3] = 0;
	FsGlobalData.CurrentFs     = Info[0];

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
}

//Initialize routine for FS thread.
static DWORD init()
{
#ifdef __CFG_SYS_DDF
	if(!FsGlobalData.bInitialized)  //FS array is not initialized yet,initialize it first.
	{
		memcpy((char*)&FsGlobalData.FsArray[0],(const char*)&IOManager.FsArray[0],sizeof(__FS_ARRAY_ELEMENT) * FILE_SYSTEM_NUM);
		if(FsGlobalData.FsArray[0].FileSystemIdentifier)  //Set the first available FS as current one.
		{
			FsGlobalData.CurrentFs     = FsGlobalData.FsArray[0].FileSystemIdentifier;
			FsGlobalData.CurrentDir[0] = FsGlobalData.FsArray[0].FileSystemIdentifier;
			FsGlobalData.CurrentDir[1] = ':';
			FsGlobalData.CurrentDir[2] = '\\';
			FsGlobalData.CurrentDir[3] = 0;     //Set string's terminator.
		}
	}
	return 1;
#else
	return 0;
#endif
}

