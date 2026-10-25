//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apr 14, 2019
//    Module Name               : usragent.c
//    Module Funciton           : 
//                                Source code for user agent module.This module
//                                is loaded into kernel and is maped into process's
//                                user space when a new process is created.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "../include/mlayout.h"
#include "pteb.h"
#include "loader.h"
#include "fmt_pe.h"
#include "stdio.h"
#include "string.h"

/* Debug the system call function. */
static void ShowFile(char* pszFileName)
{
	HANDLE hFile = NULL;
	unsigned long read_sz = 0;
	char buff[128];
	MSG msg;

	/* Open the file. */
	hFile = CreateFile(pszFileName,
		FILE_ACCESS_READWRITE,
		0, NULL);
	if (NULL == hFile)
	{
		PrintLine("Failed to open the file.");
		goto __TERMINAL;
	}
	else
	{
		PrintLine("Open file OK.");
	}
	while (ReadFile(hFile, 128, buff, &read_sz))
	{
		if (0 == read_sz)
		{
			break;
		}
		if (PeekMessage(&msg))
		{
			/* Any key press will abort. */
			if ((msg.command == KERNEL_MESSAGE_AKDOWN) || 
				(msg.command == KERNEL_MESSAGE_VKDOWN))
			{
				break;
			}
		}
		/* Show out the whole file. */
		for (unsigned long i = 0; i < read_sz; i++)
		{
			if (buff[i] == '\r')
			{
				GotoHome();
				continue;
			}
			if (buff[i] == '\n')
			{
				ChangeLine();
				continue;
			}
			if (buff[i] == '\t')
			{
				PrintChar(' ');
				PrintChar(' ');
				continue;
			}
			PrintChar(buff[i]);
		}
	}

__TERMINAL:
	if (hFile)
	{
		CloseFile(hFile);
	}
	return;
}

/* Entry point of user agent. */
int __UsrAgent_Entry(int argc, char* argv[])
{
	__PE_ENTRY_POINT peEntry = NULL;
	__PEB* peb = NULL;
	unsigned long error = 0;
	int main_ret = 0;

	if (argc < 2)
	{
		/* No application specfied. */
		_hx_printf("[%s]Too few argv(argc = %d, argv[0] = %s)\r\n",
			__func__, argc, argv[0]);
		goto __TERMINAL;
	}

	/* Create PEB. */
	peb = CreatePEB(argc, argv);
	if (NULL == peb)
	{
		goto __TERMINAL;
	}

#if 0
	/* Load the PE's header into memory. */
	pFileHeader = LoadFileHeader(argv[1], &hFile);
	if (NULL == pFileHeader)
	{
		PrintLine("Failed to load PE header.");
		goto __TERMINAL;
	}
	/* Load image into memory after header is loaded. */
	peEntry = LoadImage(pFileHeader, hFile);
	if (NULL == peEntry)
	{
		PrintLine("Failed to load PE image.");
		goto __TERMINAL;
	}
#endif

	/* Load the application into memory. */
	error = LoadAllModules(peb);
	if (error)
	{
		_hx_printf("Loader error: code = %d\r\n", error);
#if (!LOADER_FORCE_RUN)
		goto __TERMINAL;
#endif
	}
	_hx_printf("%s:load all modules OK.\r\n", __func__);

#if (!LOADER_FORCE_RUN)
	/* 
	 * Carry out binding, since loading process 
	 * is just load the modules into memory.
	 */
	error = BindModulesSymbol(peb);
	if (error)
	{
		_hx_printf("Loader error: code = %d\r\n", error);
		goto __TERMINAL;
	}
#endif

	/* 
	 * Begin to launch the application, use the
	 * first module's entry as start point.
	 * PEB maybe the parameter of entry point.
	 */
	peEntry = (__PE_ENTRY_POINT)peb->mq[0].entry_point;
	_hx_printf("[%s]Load OK, start at:0x%X\r\n", 
		LOADER_VER,
		(unsigned long)peEntry);
	main_ret = peEntry(argc - 1, &argv[1]);

__TERMINAL:
	if (peb)
	{
		/* PEB created successfully. */
		UnloadModules(peb);
		VirtualFree(peb);
	}

	/* 
	 * Invoke the bottom half of process. 
	 * It responses for releasing of resources.
	 */
	ExitThread(main_ret);
	/* Should never reach here. */
	return main_ret;
}
