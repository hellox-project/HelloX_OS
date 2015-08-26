//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2009
//    Module Name               : SYSCALL.CPP
//    Module Funciton           : 
//                                System call implementation code.
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

#include "syscall.h"
#include "kapi.h"
#include "modmgr.h"
#include "stdio.h"
#include "devmgr.h"
#include "iomgr.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif


static void   SC_CreateFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
		pspb->lpRetValue = (LPVOID)CreateFile(
				(LPSTR)PARAM(0),
				(DWORD)PARAM(1),
				(DWORD)PARAM(2),
				(LPVOID)PARAM(3));
	
}

static void   SC_ReadFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)ReadFile(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(LPVOID)PARAM(2),
		(DWORD*)PARAM(3));

}

static void   SC_WriteFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)WriteFile(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(LPVOID)PARAM(2),
		(DWORD*)PARAM(3));
}
static void   SC_CloseFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	CloseFile((HANDLE)PARAM(0));
}

static void   SC_CreateDirectory(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)CreateDirectory(	(LPSTR)PARAM(0));
}

static void   SC_DeleteFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)DeleteFile((LPSTR)PARAM(0));
}

static void   SC_FindFirstFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)FindFirstFile((LPSTR)PARAM(0),(FS_FIND_DATA*)PARAM(1));
}

static void   SC_FindNextFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)FindNextFile(	(LPSTR)PARAM(0),(HANDLE)PARAM(1),(FS_FIND_DATA*)PARAM(2));
}

static void   SC_FindClose(__SYSCALL_PARAM_BLOCK*  pspb)
{
	FindClose((LPSTR)PARAM(0),(HANDLE)PARAM(1));
}

static void   SC_GetFileAttributes(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)GetFileAttributes((LPSTR)PARAM(0));
}

static void   SC_GetFileSize(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)GetFileSize((HANDLE)PARAM(0),	(DWORD*)PARAM(1));

}

static void   SC_RemoveDirectory(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)RemoveDirectory((LPSTR)PARAM(0));
}

static void   SC_SetEndOfFile(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SetEndOfFile(	(HANDLE)PARAM(0));
}

static void   SC_IOControl(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)IOControl(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(LPVOID)PARAM(3),
		(DWORD)PARAM(4),
		(LPVOID)PARAM(5),
		(DWORD*)PARAM(6));
}

static void   SC_SetFilePointer(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)SetFilePointer(
		(HANDLE)PARAM(0),
		(DWORD*)PARAM(1),
		(DWORD*)PARAM(2),
		(DWORD)PARAM(3));
}

static void   SC_FlushFileBuffers(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)FlushFileBuffers(	(HANDLE)PARAM(0));
}

static void   SC_CreateDevice(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)CreateDevice(
		(LPSTR)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(DWORD)PARAM(3),
		(DWORD)PARAM(4),
		(LPVOID)PARAM(5),
		(__DRIVER_OBJECT*)PARAM(6));
}

static void   SC_DestroyDevice(__SYSCALL_PARAM_BLOCK*  pspb)
{
	DestroyDevice((HANDLE)PARAM(0));
}

static void   SC_PrintLine(__SYSCALL_PARAM_BLOCK*  pspb)
{
	PrintLine((LPSTR)PARAM(0));
}

static void   SC_PrintCh(__SYSCALL_PARAM_BLOCK*  pspb)
{
	PrintCh((WORD)PARAM(0));
}

void  RegisterIoEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	
	pSysCallEntry[SYSCALL_CREATEFILE]                = SC_CreateFile;
	pSysCallEntry[SYSCALL_READFILE]                  = SC_ReadFile;
	pSysCallEntry[SYSCALL_WRITEFILE]                 = SC_WriteFile;
	pSysCallEntry[SYSCALL_CLOSEFILE]                 = SC_CloseFile;

	pSysCallEntry[SYSCALL_CREATEDIRECTORY]           = SC_CreateDirectory;
	pSysCallEntry[SYSCALL_REMOVEDIRECTORY]           = SC_RemoveDirectory;
	pSysCallEntry[SYSCALL_DELETEFILE]                = SC_DeleteFile;

	pSysCallEntry[SYSCALL_FINDFIRSTFILE]             = SC_FindFirstFile;
	pSysCallEntry[SYSCALL_FINDNEXTFILE]              = SC_FindNextFile;
	pSysCallEntry[SYSCALL_FINDCLOSE]                 = SC_FindClose;

	pSysCallEntry[SYSCALL_GETFILEATTRIBUTES]         = SC_GetFileAttributes;
	pSysCallEntry[SYSCALL_GETFILESIZE]               = SC_GetFileSize;

	pSysCallEntry[SYSCALL_SETENDOFFILE]              = SC_SetEndOfFile;

	pSysCallEntry[SYSCALL_IOCONTROL]                 = SC_IOControl;
	pSysCallEntry[SYSCALL_SETFILEPOINTER]            = SC_SetFilePointer;
	pSysCallEntry[SYSCALL_FLUSHFILEBUFFERS]          = SC_FlushFileBuffers;

	pSysCallEntry[SYSCALL_CREATEDEVICE]              = SC_CreateDevice;
	pSysCallEntry[SYSCALL_DESTROYDEVICE]             = SC_DestroyDevice;

	pSysCallEntry[SYSCALL_PRINTLINE]                 = SC_PrintLine;
	pSysCallEntry[SYSCALL_PRINTCHAR]                 = SC_PrintCh;
}
