//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 30 Apri, 2024
//    Module Name               : fs_copy.c
//    Module Funciton           : 
//    Description               : copy command's implementation code.
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
#include <wildcard.h>

/* For file name string operation. */
#include <../fs/fsstr.h>

#include "shell.h"
#include "fs.h"

/* 
 * Helper routine to check if a file
 * name's path is relative or absolate.
 */
static BOOL IsRelative(char* pszFileName)
{
	BUG_ON(NULL == pszFileName);
	if (strlen(pszFileName) < 2)
	{
		return TRUE;
	}
	/* It's a absolate path if the second character is ':'. */
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
	/* Relative path. */
	return TRUE;
}

/*
 * Copy one file(source file) to destination file.
 * Device, such as partition, even physical disk, also 
 * could be duplicated by this command.
 */
static BOOL __copy_one_file(const char* srcFullName, const char* dstFullName,
	const char* src_fname, const char* dst_fname)
{
	HANDLE hSourceFile = NULL, hDestinationFile = NULL;
	char* pBuffer = NULL;
	DWORD dwReadSize = 0, dwWriteSize = 0;
	unsigned long file_sz_high = 0;
	unsigned long long total_size = 0, batch_size = 0;
	unsigned long long total_copied = 0;
	BOOL bResult = FALSE;

	/* Try to open the source file. */
	hSourceFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		(LPSTR)srcFullName,
		FILE_ACCESS_READ,
		0,
		NULL);
	if (NULL == hSourceFile)
	{
		_hx_printf("Open source [%s] error [0x%X].\r\n", srcFullName,
			GetLastError());
		goto __TERMINAL;
	}

	/* Try to open or create the destination file name. */
	hDestinationFile = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		(LPSTR)dstFullName,
		FILE_OPEN_ALWAYS,
		0,
		NULL);
	if (NULL == hDestinationFile)
	{
		_hx_printf("Open dest [%s] error [0x%X].\r\n",
			dstFullName, GetLastError());
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
#define __TMP_FILE_BUFFSZ (512 * 1024)
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
			_hx_printf("Read file error [0x%X].\r\n",
				GetLastError());
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
				_hx_printf("Write file error [0x%X].\r\n",
					GetLastError());
				goto __TERMINAL;
			}
			total_copied += dwReadSize;
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

	/* Set the end position of target file. */
	SetEndOfFile(hDestinationFile);

	bResult = TRUE;

__TERMINAL:
	_hx_printf("\r\n");
	_hx_printf("[%lld] byte(s) copied from [%s] to [%s].\r\n", 
		total_copied, src_fname, dst_fname);

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
	return bResult;
}

/*
 * Local helper routine, copy one directory
 * into another. All files under the source
 * directory, include sub-directories, will
 * be copied into destination directory.
 * Wildcard can be specifed, then files will
 * be filtered by the wildcard string.
 * Use short name in default, can be switched
 * to long name using bLongName flag.
 */
static DWORD __directory_copy(const char* src_path, const char* wc_string,
	const char* dest_path, BOOL bLongName)
{
	DWORD ret_val = SHELL_CMD_PARSER_SUCCESS;
	BOOL has_wc = (wc_string[0] ? TRUE : FALSE);
	int wc_matches = 0;
	BOOL bFindNext = FALSE;
	HANDLE hFindHandle = NULL;
	FS_FIND_DATA ffd = { 0 };
	char srcFullName[MAX_FILE_NAME_LEN];
	char dstFullName[MAX_FILE_NAME_LEN];
	int total_copied = 0;
	const char* dest_file_name = NULL;

	/* Start iteration of directory searching. */
	ffd.bGetLongName = bLongName;
	dest_file_name = bLongName ? ffd.cFileName : ffd.cAlternateFileName;
	hFindHandle = FindFirstFile((LPSTR)src_path, &ffd);
	if (NULL == hFindHandle)
	{
		_hx_printf("Open source dir [%s] error [0x%X].\r\n", 
			src_path, GetLastError());
		goto __TERMINAL;
	}
	do {
		/* Apply wildcard string matching. */
		wc_matches = 1;
		if (has_wc)
		{
			wc_matches = wc_match(wc_string, dest_file_name);
		}
		/* Apply file copying. */
		if (wc_matches)
		{
			if (ffd.dwFileAttribute & FILE_ATTR_DIRECTORY)
			{
				/* Copy sub directory. */
			}
			else {
				/* Normal file. */
				strncpy(srcFullName, src_path, MAX_FILE_NAME_LEN);
				strcat(srcFullName, dest_file_name);
				strncpy(dstFullName, dest_path, MAX_FILE_NAME_LEN);
				strcat(dstFullName, dest_file_name);

				if (!__copy_one_file(srcFullName, dstFullName, 
					dest_file_name, dest_file_name))
				{
					goto __TERMINAL;
				}
				total_copied++;
			}
		}
		/* Next finding iteration. */
		bFindNext = FindNextFile((LPSTR)src_path, hFindHandle, &ffd);
	} while (bFindNext);

__TERMINAL:
	_hx_printf("\r\nTotal [%d] file(s) copied.\r\n", total_copied);
	/* Close find handle. */
	if (hFindHandle)
	{
		FindClose((LPSTR)src_path, hFindHandle);
	}
	return ret_val;
}

/* Check if a name string contains wildcard. */
static BOOL HasWildCard(const char* name_string)
{
	const char* begin = name_string;
	while (*begin)
	{
		if ('*' == *begin)
		{
			return TRUE;
		}
		if ('?' == *begin)
		{
			return TRUE;
		}
		if ('[' == *begin)
		{
			return TRUE;
		}
		if (']' == *begin)
		{
			return TRUE;
		}
		begin++;
	}
	return FALSE;
}

/*
 * copy command's handler.
 * Usage:
 *   copy src_path/src_file [dst_path/dst_file] [/l]
 *
 * The destination is optional, current directory
 * will be used as destination path.
 * src_file could contain wildcards, such as * or ?,
 * more than one files will be copied in this case.
 * Option [/l] can be specified to copy long names,
 * only short name is copied from source to dest
 * by default.
 */
DWORD __handler_fs_copy(__CMD_PARA_OBJ* pCmdObj)
{
	char srcFullName[MAX_FILE_NAME_LEN];
	char dstFullName[MAX_FILE_NAME_LEN];
	char src_path[MAX_FILE_NAME_LEN];
	char src_name[MAX_FILE_NAME_LEN];
	char dst_path[MAX_FILE_NAME_LEN];
	char dst_name[MAX_FILE_NAME_LEN];
	BOOL bUseLongName = FALSE;
	DWORD ret_val = SHELL_CMD_PARSER_SUCCESS;

	if (pCmdObj->byParameterNum < 2)
	{
		_hx_printf("No source file specified.\r\n");
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
	if (!GetPathName((const char*)srcFullName, src_path, src_name))
	{
		_hx_printf("Invalid source name[%s]\r\n", srcFullName);
		goto __TERMINAL;
	}

	/* Construct destination file's full name. */
	dst_path[0] = dst_name[0] = 0;
	if (pCmdObj->byParameterNum >= 3)
	{
		if (pCmdObj->byParameterNum >= 4)
		{
			/* Option is specified. */
			if (0 == strncmp(pCmdObj->Parameter[3], "/l", 4))
			{
				bUseLongName = TRUE;
			}
		}
		else {
			/* Dest path/file or option specified. */
			if (0 == strncmp(pCmdObj->Parameter[2], "/l", 4))
			{
				/* Only option is specified. */
				bUseLongName = TRUE;
				/* Use current dir as dest directory. */
				strncpy(dstFullName, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
				strncpy(dst_path, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
			}
			else
			{
				/* Only dest path/file specified. */
				if (IsRelative(pCmdObj->Parameter[2]))
				{
					/* Append current directory into the relative file name. */
					strncpy(dstFullName, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
					strcat(dstFullName, pCmdObj->Parameter[2]);
				}
				else
				{
					strncpy(dstFullName, pCmdObj->Parameter[2], MAX_FILE_NAME_LEN);
				}
				ToCapital(dstFullName);
				if (!GetPathName((const char*)dstFullName, dst_path, dst_name))
				{
					_hx_printf("Invalid dest file name[%s]\r\n", dstFullName);
					goto __TERMINAL;
				}
			}
		}
	}
	else {
		/* 
		 * Dest path/file is not specified, 
		 * use current dir as dest path. 
		 */
		strncpy(dstFullName, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
		strncpy(dst_path, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
	}

	/* Can not copy one file to itself. */
	if (0 == strcmp(srcFullName, dstFullName))
	{
		_hx_printf("Files are same.\r\n");
		goto __TERMINAL;
	}

	/* Directory to directory copying. */
	if ((src_name[0] == 0) || (HasWildCard(src_name)))
	{
		if (dst_name[0])
		{
			_hx_printf("Can not combine files.\r\n");
			goto __TERMINAL;
		}
		__directory_copy(src_path, src_name, dst_path, bUseLongName);
		goto __TERMINAL;
	}

	/* File to file copying. */
	if (dst_name[0] == 0)
	{
		/* 
		 * Dest file name is not specified, 
		 * use source name. 
		 */
		strcat(dstFullName, src_name);
		strncpy(dst_name, src_name, MAX_FILE_NAME_LEN);
	}
	__copy_one_file(srcFullName, dstFullName, (const char*)src_name, (const char*)dst_name);

__TERMINAL:
	return ret_val;
}
