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

#include <StdAfx.h>

#include <blkcache.h>
#include <fat32.h>
#include <kapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "fs.h"

#define FS_PROMPT_STR "[fs_view]"

static HISOBJ s_hHiscmdInoObj = NULL;

/* Global fs information. */
__FS_GLOBAL_DATA FsGlobalData = {0};

/* A local temporary buffer. */
static char Buffer[256] = {0};

/* command handler functions. */
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);
static DWORD fs_exit(__CMD_PARA_OBJ*);
static DWORD fslist(__CMD_PARA_OBJ*);
static DWORD cd(__CMD_PARA_OBJ*);
static DWORD vl(__CMD_PARA_OBJ*);
static DWORD md(__CMD_PARA_OBJ*);
static DWORD mkdir(__CMD_PARA_OBJ*);
static DWORD del(__CMD_PARA_OBJ*);
static DWORD rd(__CMD_PARA_OBJ*);
static DWORD ren(__CMD_PARA_OBJ*);
static DWORD type(__CMD_PARA_OBJ*);
static DWORD use(__CMD_PARA_OBJ*);
static DWORD trunc(__CMD_PARA_OBJ*);
/* Initialize routine of fs command. */
static DWORD init();

/* A map between command and it's handler. */
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"fslist",     fslist,            "  fslist   : Show all available file systems."},
	{"dir",        __handler_fs_dir,  "  dir      : Show current directory's file list."},
	{"cd",         cd,                "  cd       : Show or change current directory."},
	{"vl",         vl,                "  vl       : Change current fs's volume label."},
	{"md",         md,                "  md       : Create a new directory."},
	{"mkdir",      mkdir,             "  mkdir    : Alias command of md."},
	{"del",        __handler_fs_del,  "  del      : Delete file(s) from current directory."},
	{"rd",         rd,                "  rd       : Delete one sub-directory from current directory."},
	{"ren",        ren,               "  ren      : Change file or directory's name."},
	{"type",       type,              "  type     : Show a specified file's content."},
	{"copy",       __handler_fs_copy, "  copy     : Copy file to other location,or reverse."},
	{"trunc",      trunc,             "  trunc    : Truncate a file from the specified position."},
	{"use",        use,               "  use      : Set current file system."},
	{"exit",       fs_exit,           "  exit     : Exit the application."},
	{"help",       help,              "  help     : Print out this screen."},
	{NULL,		   NULL,              NULL}
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

/* 
 * Processes the input command string. 
 */
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj = NULL;

	if ((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	if (NULL == lpCmdParamObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if (0 == lpCmdParamObj->byParameterNum)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//
	//The following code looks up the command map,
	//to find the correct handler that handle
	//the current command.If find,then calls the 
	//handler,else,return SYS_DIAG_CMD_PARSER_INVALID
	//to indicate the failure.
	//
	while (TRUE)
	{
		if (NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if (StrCmp(SysDiagCmdMap[dwIndex].lpszCommand, lpCmdParamObj->Parameter[0]))
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex++;
		}
	}

	if (NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

/* Entry point of fs application. */
DWORD fsEntry(LPVOID p)
{
	if(0 == init())
	{
		PrintLine("  Init thread fail.");
		return 0;
	}
	/* Main message loop. */
	return Shell_Msg_Loop(FS_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD fs_exit(__CMD_PARA_OBJ* lpCmdObj)
{
	memzero(&FsGlobalData,sizeof(__FS_GLOBAL_DATA));
	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwIndex = 0;

	while (TRUE)
	{
		if (NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD fslist(__CMD_PARA_OBJ* pcpo)
{
	CHAR Buffer[96];
	int i;
	__DEVICE_OBJECT* pFileDevice = NULL;
	__FAT32_FS* pFileObject = NULL;

	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(0 == FsGlobalData.FsArray[i].FileSystemIdentifier)
		{
			continue;
		}

		/* 
		 * Get the corresponding file device object, to show out 
		 * cluster size or other information.
		 */
		pFileDevice = (__DEVICE_OBJECT*)FsGlobalData.FsArray[i].pFileSystemObject;
		BUG_ON(NULL == pFileDevice);
		pFileObject = (__FAT32_FS*)pFileDevice->lpDevExtension;
		BUG_ON(NULL == pFileObject);

		_hx_sprintf(Buffer,"  fs_id:[%c:]    label:[%s]    type:[%X]    clus_sz:[%d]",
			FsGlobalData.FsArray[i].FileSystemIdentifier,
			FsGlobalData.FsArray[i].VolumeLbl,
			FsGlobalData.FsArray[i].dwAttribute,
			pFileDevice->dwBlockSize);
		PrintLine(Buffer);
	}
	return SHELL_CMD_PARSER_SUCCESS;
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
		_hx_printf("Open dir[%s] fail[0x%X].\r\n",
			NewPath,
			GetLastError());
		goto __TERMINAL;
	}
	if((dwFileAttr & FILE_ATTR_DIRECTORY) == 0)   //Is not a directory,is a regular file.
	{
		PrintLine("  Directory name is desired.");
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

static DWORD rd(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	CHAR     FullName[MAX_FILE_NAME_LEN];
	BOOL     bResult = FALSE;

	if(pCmdObj->byParameterNum < 2)
	{
		_hx_printf("  No directory specified.\r\n");
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
		_hx_printf("  No file specified.\r\n");
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

/* 
 * Local helper to check that a user 
 * specified string is a device's name
 * or a regular file's name.
 */
static BOOL __is_device_name(const char* file_name)
{
	BUG_ON(NULL == file_name);

	/* Device name's prefix is "\\.\". */
	if (strlen(file_name) < 4)
	{
		return FALSE;
	}
	if (('\\' == file_name[0]) && ('\\' == file_name[1]) &&
		('.' == file_name[2]) && ('\\' == file_name[3]))
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Show out a file's or similiar object's
 * such as partion, storage disk, content
 * into screen.
 */
#define TYPE_READ_BLOCK_LENGTH 1024

static unsigned long type(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	HANDLE hFile = NULL;
	char FullName[MAX_FILE_NAME_LEN];
	char* pBuffer = NULL;
	unsigned long dwReadSize = 0, dwTotalRead = 0;
	unsigned long total_show = -1;
	unsigned int i = 0;
	unsigned short ch = 0x0700;
	MSG msg;

	if(pCmdObj->byParameterNum < 2)
	{
		_hx_printf("  No file specified.\r\n");
		goto __TERMINAL;
	}
	if (pCmdObj->byParameterNum > 2)
	{
		/* How many bytes to show is specified. */
		total_show = atoi(pCmdObj->Parameter[2]);
	}

	/* 
	 * If the target file is not device, 
	 * then append the current opened dir
	 * in front of file's name. 
	 */
	if (!__is_device_name(pCmdObj->Parameter[1]))
	{
		strcpy(FullName, FsGlobalData.CurrentDir);
		strcat(FullName, pCmdObj->Parameter[1]);
	}
	else {
		/* Device name. */
		strcpy(FullName, pCmdObj->Parameter[1]);
	}
	ToCapital(FullName);

	/* Allocate temporary buffer to hold file data. */
	pBuffer = (char*)_hx_malloc(TYPE_READ_BLOCK_LENGTH);
	if (NULL == pBuffer)
	{
		goto __TERMINAL;
	}

	/* Try to open the target file or device. */
	hFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		FullName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hFile)
	{
		_hx_printf("Open file [%s] error [0x%X].\r\n", 
			FullName, GetLastError());
		goto __TERMINAL;
	}

	/* Read and display it's content into console. */
	GotoHome();
	ChangeLine();
	do{
		if(!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
			hFile,
			TYPE_READ_BLOCK_LENGTH,
			pBuffer,
			&dwReadSize))
		{
			_hx_printf("Read file error [0x%X].\r\n",
				GetLastError());
			goto __TERMINAL;
		}
		for(i = 0;i < dwReadSize;i ++)
		{
			if('\r' == pBuffer[i])
			{
				GotoHome();
				continue;
			}
			if('\n' == pBuffer[i])
			{
				ChangeLine();
				continue;
			}
			if ('\t' == pBuffer[i])
			{
				/* Show a 'tab' key. */
				ch += ' ';
				PrintCh(ch);
				PrintCh(ch);
				PrintCh(ch);
				PrintCh(ch);
				ch = 0x0700;
				continue;
			}
			ch += pBuffer[i];
			PrintCh(ch);
			ch = 0x0700;

			total_show--;
			if (0 == total_show)
			{
				goto __TERMINAL;
			}
		}
		dwTotalRead += dwReadSize;

		/* Check if user want to break. */
		while (KernelThreadManager.PeekMessage(NULL, &msg))
		{
			if (msg.wCommand == KERNEL_MESSAGE_TERMINAL)
			{
				/* Show out how many bytes read. */
				GotoHome();
				ChangeLine();
				_hx_printf("%d byte(s) read.", dwTotalRead);

				goto __TERMINAL;
			}
		}
	}while(dwReadSize == TYPE_READ_BLOCK_LENGTH);

	/* Show out how many bytes read. */
	GotoHome();
	ChangeLine();
	_hx_printf("%d byte(s) read.", dwTotalRead);

__TERMINAL:
	if(NULL != hFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hFile);
	}
	if (pBuffer)
	{
		/* Release temp file buffer. */
		_hx_free(pBuffer);
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return FS_CMD_FAILED;
#endif
}

/* Truncate a file from the specified position. */
static unsigned long trunc(__CMD_PARA_OBJ* pCmdObj)
{
	HANDLE hFile = NULL;
	char FullName[MAX_FILE_NAME_LEN];
	unsigned long start_pos = -1;
	unsigned long file_sz = 0;

	if (pCmdObj->byParameterNum < 3)
	{
		_hx_printf("  No file or start position specified.\r\n");
		goto __TERMINAL;
	}

	/* The start position to trunc. */
	start_pos = atoi(pCmdObj->Parameter[2]);

	/*
	 * If the target file is not device,
	 * then append the current opened dir
	 * in front of file's name.
	 */
	if (!__is_device_name(pCmdObj->Parameter[1]))
	{
		strcpy(FullName, FsGlobalData.CurrentDir);
		strcat(FullName, pCmdObj->Parameter[1]);
	}
	else {
		/* Device name. */
		_hx_printf("  [trunc]only regular file supported.\r\n");
		goto __TERMINAL;
	}
	ToCapital(FullName);

	/* Try to open the target file. */
	hFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		FullName,
		FILE_ACCESS_READ | FILE_ACCESS_WRITE,
		0,
		NULL);
	if (NULL == hFile)
	{
		_hx_printf("  [trunc]Open file [%s] error [0x%X].\r\n",
			FullName, GetLastError());
		goto __TERMINAL;
	}

	/* Get the file's size. */
	file_sz = GetFileSize(hFile, NULL);
	if (-1 == file_sz)
	{
		_hx_printf("  [trunc]failed to get file size.\r\n");
		goto __TERMINAL;
	}
	if (start_pos > file_sz)
	{
		_hx_printf("  [trunc]start position must less file size.\r\n");
		goto __TERMINAL;
	}

	/* Move to target position. */
	if (SetFilePointer(hFile, &start_pos, NULL, FILE_FROM_BEGIN) == 0)
	{
		_hx_printf("  [trunc]move file pointer failed\r\n");
		goto __TERMINAL;
	}

	/* Truncate the file. */
	if (!SetEndOfFile(hFile))
	{
		_hx_printf("  [trunc]truncate file failed.\r\n");
	}
	else {
		_hx_printf("  Truncate file[%s] success.\r\n", FullName);
	}

__TERMINAL:
	if (NULL != hFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hFile);
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

static DWORD use(__CMD_PARA_OBJ* pCmdObj)
{
	CHAR     Info[4];
	int      i;
	BOOL     bMatched = FALSE;
	CHAR*    strError = "  No partition specified.";

	if (1 == pCmdObj->byParameterNum)
	{
		Info[0] = FsGlobalData.CurrentFs;
		Info[1] = ':';
		Info[2] = 0;
		_hx_sprintf(Buffer, "  Current partition: %s", Info);
		PrintLine(Buffer);
		goto __TERMINAL;
	}
	//Parse the user input.
	if (strlen(pCmdObj->Parameter[1]) > 2)
	{
		PrintLine(strError);
		goto __TERMINAL;
	}
	Info[0] = pCmdObj->Parameter[1][0];
	Info[1] = pCmdObj->Parameter[1][1];
	Info[2] = 0;
	if (Info[1] != ':')
	{
		PrintLine(strError);
		goto __TERMINAL;
	}

	for (i = 0; i < FILE_SYSTEM_NUM; i++)
	{
		if (FsGlobalData.FsArray[i].FileSystemIdentifier == Info[0])
		{
			bMatched = TRUE;
			break;
		}
	}
	if (!bMatched)
	{
		PrintLine("  No partition found.");
		goto __TERMINAL;
	}
	//Valid file system,change current information.
	FsGlobalData.CurrentDir[0] = Info[0];
	FsGlobalData.CurrentDir[1] = Info[1];
	FsGlobalData.CurrentDir[2] = '\\';
	FsGlobalData.CurrentDir[3] = 0;
	FsGlobalData.CurrentFs = Info[0];

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}

//Initialize routine for FS thread.
static DWORD init()
{
#ifdef __CFG_SYS_DDF
	if(!FsGlobalData.bInitialized)
	{
		/* Obtain FS objects from kernel. */
		memcpy((char*)&FsGlobalData.FsArray[0],
			(const char*)&IOManager.FsArray[0],
			sizeof(__FS_ARRAY_ELEMENT) * FILE_SYSTEM_NUM);

		/* Set default current directory. */
		if(FsGlobalData.FsArray[0].FileSystemIdentifier)
		{
			FsGlobalData.CurrentFs     = FsGlobalData.FsArray[0].FileSystemIdentifier;
			FsGlobalData.CurrentDir[0] = FsGlobalData.FsArray[0].FileSystemIdentifier;
			FsGlobalData.CurrentDir[1] = ':';
			FsGlobalData.CurrentDir[2] = '\\';
			FsGlobalData.CurrentDir[3] = 0;
		}
	}
	return 1;
#else
	return 0;
#endif
}
