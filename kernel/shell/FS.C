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
#include <kapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "fs.h"

#define FS_PROMPT_STR "[fs_view]"

static HISOBJ s_hHiscmdInoObj = NULL;

static struct __FS_GLOBAL_DATA{
	__FS_ARRAY_ELEMENT  FsArray[FILE_SYSTEM_NUM];
	BYTE                CurrentFs;                 //Current file system identifier.
	CHAR                CurrentDir[MAX_FILE_NAME_LEN];
	BOOL                bInitialized;              //Flag indicates if this structure has been inited.
}FsGlobalData = {0};
static char Buffer[256] = {0};

/* command handler functions. */
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);
static DWORD fs_exit(__CMD_PARA_OBJ*);
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
/* Initialize routine of fs command. */
static DWORD init();

/* A map between command and it's handler. */
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
	{"exit",       fs_exit,   "  exit     : Exit the application."},
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

	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

DWORD fsEntry(LPVOID p)
{
	if(0 == init())
	{
		PrintLine("  Can not initialize the FS thread.");
		return 0;
	}

	return Shell_Msg_Loop(FS_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD fs_exit(__CMD_PARA_OBJ* lpCmdObj)
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
		if(0 == FsGlobalData.FsArray[i].FileSystemIdentifier)
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
	ToCapital(Buffer);
	
	if(pCmdObj->byParameterNum >= 2 && strcmp(pCmdObj->Parameter[1],"-l") == 0)
	{
		ffd.bGetLongName = TRUE;
	}
		
	pFindHandle = IOManager.FindFirstFile((__COMMON_OBJECT*)&IOManager,	Buffer,	&ffd);
	if(NULL == pFindHandle)
	{		
		_hx_printf("Can not open directory[%s].\r\n", Buffer);
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
		_hx_printf("  No file specified.\r\n");
		goto __TERMINAL;
	}
	strcpy(FullName,FsGlobalData.CurrentDir);
	strcat(FullName,pCmdObj->Parameter[1]);
	ToCapital(FullName);
	bResult = IOManager.DeleteFile((__COMMON_OBJECT*)&IOManager,
		FullName);
	if(!bResult)
	{
		_hx_printf("  Can not delete the specified file.");
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
		_hx_printf("  Can not open[%s].\r\n", FullName);
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
			_hx_printf("  Failed to read file.\r\n");
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
				goto __TERMINAL;
			}
		}
	}while(dwReadSize == TYPE_READ_BLOCK_LENGTH);

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
	return SHELL_CMD_PARSER_SUCCESS;;
#else
	return FS_CMD_FAILED;
#endif
}

/* Helper routine to check if a given file name is relative. */
static BOOL IsRelative(char* pszFileName)
{
	BUG_ON(NULL == pszFileName);
	if (strlen(pszFileName) < 2)
	{
		return TRUE;
	}
	/* It's a absolately path if the second character is ':'. */
	if (':' == pszFileName[1])
	{
		/* Only FS identifier and ':',no more characters. */
		if (strlen(pszFileName) == 2)
		{
			return TRUE;
		}
		return FALSE;
	}
	if (strlen(pszFileName) > 4)
	{
		/* Device name also is not relative. */
		if ((pszFileName[0] == '\\') &&
			(pszFileName[1] == '\\') &&
			(pszFileName[2] == '.') &&
			(pszFileName[3] == '\\'))
		{
			return FALSE;
		}
	}
	/* All other cases are relative path. */
	return TRUE;
}

/*
 * Copy the content in source file to destination file.
 * Device, such as partition, also could be duplicated
 * by this command.
 */
static unsigned long copy(__CMD_PARA_OBJ* pCmdObj)
{
	HANDLE hSourceFile = NULL, hDestinationFile = NULL;
	char* pBuffer = NULL;
	DWORD dwReadSize = 0, dwWriteSize = 0, dwTotalRead = 0;
	char srcFullName[MAX_FILE_NAME_LEN];
	char dstFullName[MAX_FILE_NAME_LEN];
	unsigned long file_sz_high = 0;
	unsigned long long total_size = 0, batch_size = 0;

	if (pCmdObj->byParameterNum < 3)
	{
		_hx_printf("  No files specified.\r\n");
		goto __TERMINAL;
	}
	/* Construct source file's full name. */
	if (IsRelative(pCmdObj->Parameter[1]))
	{
		/* Append current directory into the relative file name. */
		strcpy(srcFullName, FsGlobalData.CurrentDir);
		strcat(srcFullName, pCmdObj->Parameter[1]);
	}
	else
	{
		strcpy(srcFullName, pCmdObj->Parameter[1]);
	}
	ToCapital(srcFullName);

	/* Construct destination file's full name. */
	if (IsRelative(pCmdObj->Parameter[2]))
	{
		/* Append current directory into the relative file name. */
		strcpy(dstFullName, FsGlobalData.CurrentDir);
		strcat(dstFullName, pCmdObj->Parameter[2]);
	}
	else
	{
		strcpy(dstFullName, pCmdObj->Parameter[2]);
	}
	ToCapital(dstFullName);

	/* Can not copy one file to itself. */
	if (0 == strcmp(srcFullName, dstFullName))
	{
		_hx_printf("  Files are same.\r\n");
		goto __TERMINAL;
	}

	/* Try to open the source file. */
	hSourceFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		srcFullName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if (NULL == hSourceFile)
	{
		_hx_printf("  Can not open[%s].\r\n", srcFullName);
		goto __TERMINAL;
	}

	/* Try to open or create the destination file name. */
	hDestinationFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		dstFullName,
		FILE_OPEN_ALWAYS,
		0,
		NULL);
	if (NULL == hDestinationFile)
	{
		_hx_printf("  Can not open[%s].\r\n",
			dstFullName);
		goto __TERMINAL;
	}

	/* Get the source file's size. */
	total_size = GetFileSize(hSourceFile, &file_sz_high);
	if (0 == total_size)
	{
		goto __TERMINAL;
	}
	total_size += (unsigned long long)file_sz_high << 32;
	batch_size = total_size / 64;

	/* Allocate a buffer to hold the file's data. */
#define __TMP_FILE_BUFFSZ (128 * 1024)
	pBuffer = (char*)_hx_malloc(__TMP_FILE_BUFFSZ);
	if (NULL == pBuffer)
	{
		_hx_printf("[%s]: Out of memory.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Copy data now. */
	do {
		/* Read the source file. */
		if (!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
			hSourceFile,
			__TMP_FILE_BUFFSZ,
			pBuffer,
			&dwReadSize))
		{
			_hx_printf("  Read file failure.\r\n");
			goto __TERMINAL;
		}

		/* 
		 * Write the data block into destination file. 
		 * read size maybe 0 in case of EOF.
		 */
		if (dwReadSize)
		{
			if (!IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
				hDestinationFile,
				dwReadSize,
				pBuffer,
				&dwWriteSize))
			{
				_hx_printf("  Write file failure\r\n");
				goto __TERMINAL;
			}
			dwTotalRead += dwReadSize;
		}

		/* Show out copying progress. */
		if (batch_size < dwReadSize)
		{
			_hx_printf(".");
			batch_size = total_size / 64;
		}
		else
		{
			batch_size -= dwReadSize;
		}
	} while (dwReadSize == __TMP_FILE_BUFFSZ);
#undef __TMP_FILE_BUFFSZ

__TERMINAL:
	_hx_printf("\r\n");
	_hx_printf("[copy]: %d byte(s) copied.\r\n", dwTotalRead);

	if (NULL != hSourceFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hSourceFile);
	}
	if (NULL != hDestinationFile)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			hDestinationFile);
	}
	if (NULL != pBuffer)
	{
		_hx_free(pBuffer);
	}
	return SHELL_CMD_PARSER_SUCCESS;;
}

static DWORD use(__CMD_PARA_OBJ* pCmdObj)
{
	CHAR     Info[4];
	int      i;
	BOOL     bMatched = FALSE;
	CHAR*    strError = "  Please specify a valid file system identifier.";

	if(1 == pCmdObj->byParameterNum)
	{
		Info[0] = FsGlobalData.CurrentFs;
		Info[1] = ':';
		Info[2] = 0;
		_hx_sprintf(Buffer,"  Current file system is: %s",Info);
		PrintLine(Buffer);
		goto __TERMINAL;
	}
	//Parse the user input.
	if(strlen(pCmdObj->Parameter[1]) > 2)
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
		if(FsGlobalData.FsArray[i].FileSystemIdentifier == Info[0])
		{
			bMatched = TRUE;
			break;
		}
	}
	if(!bMatched)
	{
		PrintLine("  The specified file system is not present.");
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
	if(!FsGlobalData.bInitialized)
	{
		memcpy((char*)&FsGlobalData.FsArray[0],(const char*)&IOManager.FsArray[0],sizeof(__FS_ARRAY_ELEMENT) * FILE_SYSTEM_NUM);
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
