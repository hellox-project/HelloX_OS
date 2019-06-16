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

#include "StdAfx.h"
#include "syscall.h"
#include "kapi.h"
#include "modmgr.h"
#include "stdio.h"
#include "devmgr.h"
#include "iomgr.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif

/*
 * Macros to refer the parameter in system call parameter block.
 * Just to simplify the programming of system call's implementation.
 */
#define PARAM(idx) (pspb->param_##idx.param)
#define SYSCALL_RET (*pspb->ret_ptr.param)

/* Helper routine to show all registers. */
static void ShowRegisters()
{
	uint16_t __cs, __ds, __es, __fs, __gs, __ss;
	uint32_t __cr0, __cr3;

	__asm {
		xor eax, eax
		mov ax, cs
		mov __cs, ax
		mov ax, ds
		mov __ds, ax
		mov ax, es
		mov __es, ax
		mov ax, fs
		mov __fs, ax
		mov ax, gs
		mov __gs, ax
		mov ax, ss
		mov __ss, ax
		mov eax, cr0
		mov __cr0, eax
		mov eax, cr3
		mov __cr3, eax
	}
	_hx_printf("Segment registers:cs/ds/es/fs/gs/ss: 0x%X/0x%X/0x%X/0x%X/0x%X/0x%X\r\n",
		__cs, __ds, __es, __fs, __gs, __ss);
	_hx_printf("cr0/cr3: 0x%X/0x%X\r\n", __cr0, __cr3);
}

static void SC_CreateFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	HANDLE hFile = NULL;

	hFile = CreateFile(
		(LPSTR)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(LPVOID)PARAM(3));
	SYSCALL_RET = (uint32_t)hFile;
}

static void SC_ReadFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)ReadFile(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(LPVOID)PARAM(2),
		(DWORD*)PARAM(3));
}

static void SC_WriteFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)WriteFile(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(LPVOID)PARAM(2),
		(DWORD*)PARAM(3));
}
static void SC_CloseFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	CloseFile((HANDLE)PARAM(0));
}

static void SC_CreateDirectory(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)CreateDirectory((LPSTR)PARAM(0));
}

static void SC_DeleteFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)DeleteFile((LPSTR)PARAM(0));
}

static void SC_FindFirstFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)FindFirstFile((LPSTR)PARAM(0), 
		(FS_FIND_DATA*)PARAM(1));
}

static void SC_FindNextFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)FindNextFile((LPSTR)PARAM(0), 
		(HANDLE)PARAM(1), 
		(FS_FIND_DATA*)PARAM(2));
}

static void SC_FindClose(__SYSCALL_PARAM_BLOCK* pspb)
{
	FindClose((LPSTR)PARAM(0), (HANDLE)PARAM(1));
}

static void SC_GetFileAttributes(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetFileAttributes((LPSTR)PARAM(0));
}

static void SC_GetFileSize(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)GetFileSize((HANDLE)PARAM(0),
		(DWORD*)PARAM(1));
}

static void SC_RemoveDirectory(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)RemoveDirectory((LPSTR)PARAM(0));
}

static void SC_SetEndOfFile(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SetEndOfFile((HANDLE)PARAM(0));
}

static void SC_IOControl(__SYSCALL_PARAM_BLOCK* pspb)
{
	_hx_printf("SYSCALL[%d] not implemented yet.\r\n", SYSCALL_IOCONTROL);
	SYSCALL_RET = 0;
#if 0
	pspb->lpRetValue = (LPVOID)IOControl(
		(HANDLE)PARAM(0),
		(DWORD)PARAM(1),
		(DWORD)PARAM(2),
		(LPVOID)PARAM(3),
		(DWORD)PARAM(4),
		(LPVOID)PARAM(5),
		(DWORD*)PARAM(6));
#endif
}

static void SC_SetFilePointer(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)SetFilePointer(
		(HANDLE)PARAM(0),
		(DWORD*)PARAM(1),
		(DWORD*)PARAM(2),
		(DWORD)PARAM(3));
}

static void SC_FlushFileBuffers(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)FlushFileBuffers((HANDLE)PARAM(0));
}

static void SC_PrintLine(__SYSCALL_PARAM_BLOCK* pspb)
{
	PrintLine((LPSTR)PARAM(0));
}

static void SC_PrintCh(__SYSCALL_PARAM_BLOCK* pspb)
{
	PrintCh((WORD)PARAM(0));
}

static void SC_GotoHome(__SYSCALL_PARAM_BLOCK* pspb)
{
	GotoHome();
}

static void SC_ChangeLine(__SYSCALL_PARAM_BLOCK* pspb)
{
	ChangeLine();
}

static void SC_GotoPrev(__SYSCALL_PARAM_BLOCK* pspb)
{
	GotoPrev();
}

void RegisterIoEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	pSysCallEntry[SYSCALL_CREATEFILE] = SC_CreateFile;
	pSysCallEntry[SYSCALL_READFILE] = SC_ReadFile;
	pSysCallEntry[SYSCALL_WRITEFILE] = SC_WriteFile;
	pSysCallEntry[SYSCALL_CLOSEFILE] = SC_CloseFile;

	pSysCallEntry[SYSCALL_CREATEDIRECTORY] = SC_CreateDirectory;
	pSysCallEntry[SYSCALL_REMOVEDIRECTORY] = SC_RemoveDirectory;
	pSysCallEntry[SYSCALL_DELETEFILE] = SC_DeleteFile;

	pSysCallEntry[SYSCALL_FINDFIRSTFILE] = SC_FindFirstFile;
	pSysCallEntry[SYSCALL_FINDNEXTFILE] = SC_FindNextFile;
	pSysCallEntry[SYSCALL_FINDCLOSE] = SC_FindClose;

	pSysCallEntry[SYSCALL_GETFILEATTRIBUTES] = SC_GetFileAttributes;
	pSysCallEntry[SYSCALL_GETFILESIZE] = SC_GetFileSize;

	pSysCallEntry[SYSCALL_SETENDOFFILE] = SC_SetEndOfFile;

	pSysCallEntry[SYSCALL_IOCONTROL] = SC_IOControl;
	pSysCallEntry[SYSCALL_SETFILEPOINTER] = SC_SetFilePointer;
	pSysCallEntry[SYSCALL_FLUSHFILEBUFFERS] = SC_FlushFileBuffers;

	pSysCallEntry[SYSCALL_PRINTLINE] = SC_PrintLine;
	pSysCallEntry[SYSCALL_PRINTCHAR] = SC_PrintCh;
	pSysCallEntry[SYSCALL_GOTOHOME] = SC_GotoHome;
	pSysCallEntry[SYSCALL_CHANGELINE] = SC_ChangeLine;
	pSysCallEntry[SYSCALL_GOTOPREV] = SC_GotoPrev;
}

#undef PARAM
#undef SYSCALL_RET
