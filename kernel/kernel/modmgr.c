//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 21,2009
//    Module Name               : MODMGR.CPP
//    Module Funciton           : 
//                                This module countains module manager's
//                                implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "modmgr.h"
#include "kapi.h"
#include "string.h"
#include "types.h"
#include "commobj.h"
#include "comqueue.h"
#include "stdio.h"
#include "kapi.h"
#include "process.h"

//Implementation of InitModule routine.
static BOOL InitModule(__MODULE_INIT InitEntry)
{
	if(NULL == InitEntry)
	{
		return FALSE;
	}
	return InitEntry();
}

//Implementation of Initialize routine.
static BOOL ModMgrInit(__MODULE_MGR* lpThis)
{
	int i = 0;
	CHAR Buffer[128];
	BOOL bResult = FALSE;

	if(NULL == lpThis)
	{
		return FALSE;
	}
	//Initialize static kernel module(s).
	while(TRUE)
	{
		if(0 == KernelModule[i].dwLoadAddress)
		{
			break;
		}
		bResult = InitModule(KernelModule[i].InitRoutine);
		if(!bResult)
		{
			_hx_sprintf(Buffer,"  The module with start address 0x%08X can not be initialized.",
				KernelModule[i].dwLoadAddress);
			PrintLine(Buffer);
		}
		i += 1;
	}
	return TRUE;
}

//Replace shell routine,it is called by other kernel module to register a different
//shell.The default one is character shell,implemented in master module.
static BOOL ReplaceShell(__KERNEL_THREAD_ROUTINE shell)
{
	if(NULL == shell)
	{
		return FALSE;
	}
	ModuleMgr.ShellEntry = shell;
	return TRUE;
}

//A helper routine used to convert a ascii string into __EXTERNAL_MOD_DESC object.
static BOOL FetchModDesc(LPSTR pszLine,__EXTERNAL_MOD_DESC* pModDesc)
{
	BOOL    bResult      = FALSE;
	int     i;
	CHAR    numStr[9];

	while(' ' == *pszLine) //Skip the leading space.
	{
		pszLine ++;
	}
	if(0 == *pszLine)  //End of the string line.
	{
		goto __TERMINAL;
	}
	//Fetch module name.
	for(i = 0;i < MAX_MOD_FILENAME;i ++)
	{
		if(0 == *pszLine)  //End of line,invalid.
		{
			goto __TERMINAL;
		}
		if(' ' == *pszLine)
		{
			break;
		}
		pModDesc->ModFileName[i] = *pszLine ++;
	}
	pModDesc->ModFileName[i] = 0;
	//Skip non-space characters,suppose the scenario that file name's length larger than MAX_MOD_FILENAME.
	while(' ' != *pszLine)
	{
		if(0 == *pszLine)
		{
			goto __TERMINAL;
		}
		pszLine ++;
	}
	//Skip the space between file name and load address.
	while(' ' == *pszLine)
	{
		*pszLine ++;
	}
	if(0 == *pszLine)
	{
		goto __TERMINAL;
	}
	for(i = 0;i < 8;i ++)
	{
		if(0 == *pszLine)
		{
			goto __TERMINAL;
		}
		if(' ' == *pszLine)
		{
			break;
		}
		numStr[i] = *pszLine ++;
	}
	numStr[i] = 0;
	//Convert the address from string to numeric.
	if(!Str2Hex(numStr,&pModDesc->StartAddress))
	{
		goto __TERMINAL;
	}

	//Skip non-space characters,suppose the scenario that start address's length larger than 8.
	while(' ' != *pszLine)
	{
		if(0 == *pszLine)
		{
			goto __TERMINAL;
		}
		pszLine ++;
	}
	//Skip the space between load address and attribute.
	while(' ' == *pszLine)
	{
		*pszLine ++;
	}
	if(0 == *pszLine)
	{
		goto __TERMINAL;
	}
	for(i = 0;i < 8;i ++)
	{
		if(0 == *pszLine)
		{
			goto __TERMINAL;
		}
		if(' ' == *pszLine)
		{
			break;
		}
		numStr[i] = *pszLine ++;
	}
	numStr[i] = 0;
	//Convert the attribute into numeric.
	if(!Str2Hex(numStr,&pModDesc->dwModAttribute))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;  //All successful.

__TERMINAL:
	return bResult;
}

//A helper routine used to load a specified module described by module description.
static BOOL LoadModule(__EXTERNAL_MOD_DESC* pModDesc)
{
	CHAR    FullPathName[64];
	CHAR    Msg[128];
	HANDLE  hFile              = NULL;
	BYTE*   pStartAddr         = (BYTE*)pModDesc->StartAddress;
	BOOL    bResult            = FALSE;
	DWORD   dwReadSize         = 0;
	BOOL    bInitResult        = FALSE;
	__MODULE_INIT initRoutine  = (__MODULE_INIT)pModDesc->StartAddress;  //Initialize routine.

	strcpy(FullPathName,OS_ROOT_DIR);  //Append the root directory before module's file name.
	strcat(FullPathName,pModDesc->ModFileName);
	ToCapital(FullPathName);
	//Now try to open the module.
	hFile = CreateFile(FullPathName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hFile)  //Can not open the file.
	{
		_hx_sprintf(Msg,"Can not open the module named %s.",FullPathName);
		PrintLine(Msg);
		goto __TERMINAL;
	}
	//Now try to read the module file into memory,each for 8K byte.
	do{
		if(!ReadFile(hFile,
			8192,
			pStartAddr,
			&dwReadSize))  //Failed to read.
		{
			_hx_sprintf(Msg,"Can not read from module file %s.",FullPathName);
			PrintLine(Msg);
			goto __TERMINAL;
		}
		pStartAddr += 8192;  //Move to next 8k block.
	}while(dwReadSize == 8192);
	//Try to initialize the module.
	if(pModDesc->dwModAttribute & MOD_ATTR_BIN)  //BIN file.
	{
		bInitResult = initRoutine();
		if(bInitResult)
		{
			_hx_sprintf(Msg,"Initialize module %s at 0x%X successful.",FullPathName,pModDesc->StartAddress);
			PrintLine(Msg);
		}
		else
		{
			_hx_sprintf(Msg,"Initialize module %s at 0x%X failed.",FullPathName,pModDesc->StartAddress);
			PrintLine(Msg);
		}
	}
	if(pModDesc->dwModAttribute & MOD_ATTR_EXE)  //Will be implemented later.
	{
	}
	if(pModDesc->dwModAttribute & MOD_ATTR_EXE)  //Will be implemented later.
	{
	}
    _hx_sprintf(Msg,"Load module %s at 0x%X successful.",FullPathName,pModDesc->StartAddress);
	PrintLine(Msg);
	bResult = TRUE;

__TERMINAL:
	if(NULL != hFile)  //Should close it.
	{
		CloseFile(hFile);
	}
	return bResult;
}

//Fetch a line(may invalid) from a bulk memory read from MODCFG.INI file.
//If the line's first two characters are "\\",then this line is a comment line and is invalid.
static BOOL FetchLine(LPSTR pszBlock,CHAR* pszLine,int* pIndex)
{
	int     i;

	*pIndex = 0;
	while(' ' == *pszBlock)  //skip leading space.
	{
		pszBlock ++;
		(*pIndex) ++;
	}
	if(0 == *pszBlock)
	{
		return FALSE;
	}
	for(i = 0;i < MAX_LINE_LENGTH;i ++)
	{
		*pszLine ++ = *pszBlock ++;
		(*pIndex) ++;
		if((0 == *pszBlock) || ('\r' == *pszBlock) || ('\n' == *pszBlock))  //Terminator or enter reached.
		{
			break;
		}
	}
	*pszLine = 0;  //Set terminator.
	//Skip enter from block.
	while(('\r' == *pszBlock) || ('\n' == *pszBlock))
	{
		(*pIndex) ++;
		pszBlock ++;
	}
	return TRUE;
}

//Check if a given line is valid.
static BOOL IsLineValid(LPSTR pszLine)
{
	if('\\' == *pszLine)  //Invliad.
	{
		return FALSE;
	}
	return TRUE;
}

//Load external modules from file system.
static BOOL LoadExternalMod(LPSTR lpszModCfgFile)
{
	CHAR    FullPathName[128];
	CHAR*   pBuffer = NULL;
	CHAR*   pFileBuff = NULL;
	CHAR*   pLineBuff = NULL;
	HANDLE  hFile   = NULL;
	DWORD   dwReadSize = 0;
	BOOL    bResult    = FALSE;
	__EXTERNAL_MOD_DESC modDesc;
	int     index      = 0;

	if(NULL == lpszModCfgFile)  //Invalid file.
	{
		goto __TERMINAL;
	}
	//Form the valid path name.
	strcpy(FullPathName,OS_ROOT_DIR);
	strcat(FullPathName,lpszModCfgFile);
	//Try to open the file.
	hFile = CreateFile(FullPathName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if(NULL == hFile)
	{
		PrintLine("Can not open the module configure file.");
		goto __TERMINAL;
	}
	//Try to read the file.
	pBuffer   = (CHAR*)KMemAlloc(MAX_MODCF_LEN + 1,KMEM_SIZE_TYPE_ANY);
	pLineBuff = (CHAR*)KMemAlloc(MAX_LINE_LENGTH + 1,KMEM_SIZE_TYPE_ANY);
	if((NULL == pBuffer) || (NULL == pLineBuff))
	{
		PrintLine("In Module Manager object,LoadExternalMod: Allocate memory failed.");
		goto __TERMINAL;
	}
	pFileBuff = pBuffer;  //Use pFileBuff to operate the memory.
	if(!ReadFile(hFile,
		MAX_MODCF_LEN,
		pFileBuff,
		&dwReadSize))
	{
		PrintLine("Can not read MODCFG.INI file.");
		goto __TERMINAL;
	}
	pFileBuff[dwReadSize] = 0;  //Set the end of memory block.
	
	//Now try to read each line from pBuffer and load it.
	while(FetchLine(pFileBuff,pLineBuff,&index))
	{
		if(IsLineValid(pLineBuff))
		{
			if(FetchModDesc(pLineBuff,&modDesc))
			{
				//PrintLine(pLineBuff);
				LoadModule(&modDesc);  //Load the module into memory.
			}
		}
		pFileBuff += index;  //Skip the processed line.
	}
	bResult = TRUE;

__TERMINAL:
	if(NULL != hFile)
	{
		CloseFile(hFile);
	}
	if(NULL != pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	if(NULL != pLineBuff)
	{
		KMemFree(pLineBuff,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Definition of Module manager.
__MODULE_MGR ModuleMgr = {
	NULL,              //Shell thread entry.
	ModMgrInit,        //Initialize.
	InitModule,        //InitModule.
	ReplaceShell,      //ReplaceShell.
	LoadExternalMod    //LoadExternalMod.
};

