//***********************************************************************/
//    Author                    : twind
//    Original Date             : Nov,02 2015
//    Module Name               : KAPI.H
//    Module Funciton           : 
//                                Declares all socket routines can be
//                                used by other modules in kernel.
//                                This file is used by user application,so all
//                                routines declared in it is system calls.
//
//    Last modified Author      : twind
//    Last modified Date        : Jan 09,2012
//    Last modified Content     :
//                                1. System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "KAPI.H"

#ifndef __K_IO_H__
#define __K_IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _FILE_TIME
{
	DWORD dwHighDateTime;
	DWORD dwLowDateTime;

}FILE_TIME;

typedef struct _FS_FIND_DATA
{
	DWORD           dwFileAttribute;
	FILE_TIME       ftCreationTime;    //-------- CAUTION!!! --------
	FILE_TIME       ftLastAccessTime;  //-------- CAUTION!!! --------
	FILE_TIME       ftLastWriteTime;   //-------- CAUTION!!! --------
	DWORD           nFileSizeHigh;
	DWORD           nFileSizeLow;
	DWORD           dwReserved0;
	DWORD           dwReserved1;
	CHAR            cFileName[MAX_FILE_NAME_LEN];
	CHAR            cAlternateFileName[13];
}FS_FIND_DATA;

/*struct FS_FIND_DATA
{
	DWORD           dwFileAttribute;
	struct FILE_TIME       ftCreationTime;    //-------- CAUTION!!! --------
	struct FILE_TIME       ftLastAccessTime;  //-------- CAUTION!!! --------
	struct FILE_TIME       ftLastWriteTime;   //-------- CAUTION!!! --------
	DWORD           nFileSizeHigh;
	DWORD           nFileSizeLow;
	DWORD           dwReserved0;
	DWORD           dwReserved1;
	CHAR            cFileName[MAX_FILE_NAME_LEN];
	CHAR            cAlternateFileName[13];
};*/

//Flags used to control CreateFile's action.
#define FILE_ACCESS_READ         0x00000001    //Read access.
#define FILE_ACCESS_WRITE        0x00000002    //Write access.
#define FILE_ACCESS_READWRITE    0x00000003    //Read and write access.
#define FILE_ACCESS_CREATE       0x00000004    //Create a new file.

#define FILE_OPEN_ALWAYS         0x80000000    //If can not open one,create a new one then open it.
#define FILE_OPEN_NEW            0x40000000    //Create a new file,overwrite existing if.
#define FILE_OPEN_EXISTING       0x20000000    //Open a existing file,return fail if does not exist.

HANDLE CreateFile(LPSTR lpszFileName,DWORD dwAccessMode,DWORD dwShareMode,LPVOID lpReserved);

BOOL   ReadFile(HANDLE hFile,DWORD dwReadSize,LPVOID lpBuffer,DWORD* lpdwReadSize);

BOOL   WriteFile(HANDLE hFile,DWORD dwWriteSize,LPVOID lpBuffer,DWORD* lpdwWrittenSize);
VOID   CloseFile(HANDLE hFile);
BOOL   CreateDirectory(LPSTR lpszDirName);
BOOL   DeleteFile(LPSTR lpszFileName);

HANDLE FindFirstFile(LPSTR lpszDirName,	FS_FIND_DATA* pFindData);
BOOL   FindNextFile(LPSTR lpszDirName,HANDLE hFindHandle,FS_FIND_DATA* pFindData);
VOID   FindClose(LPSTR lpszDirName,HANDLE hFindHandle);

//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20

DWORD   GetFileAttributes(LPSTR lpszFileName);
DWORD   GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh);
BOOL    RemoveDirectory(LPSTR lpszDirName);
BOOL    SetEndOfFile(HANDLE hFile);
BOOL    IOControl(HANDLE hFile,	DWORD dwCommand,DWORD dwInputLen,LPVOID lpInputBuffer,DWORD dwOutputLen,LPVOID lpOutputBuffer,DWORD* lpdwFilled);

//Flags to control SetFilePointer.
#define FILE_FROM_BEGIN        0x00000001
#define FILE_FROM_CURRENT      0x00000002

BOOL   SetFilePointer(HANDLE hFile,	DWORD* lpdwDistLow,	DWORD* lpdwDistHigh,	DWORD dwMoveFlags);
BOOL   FlushFileBuffers(HANDLE hFile);

HANDLE CreateDevice(LPSTR lpszDevName,	DWORD dwAttributes,	DWORD dwBlockSize,	
	                DWORD dwMaxReadSize,DWORD dwMaxWriteSize,
	                LPVOID lpDevExtension,
	                HANDLE hDrvObject
					);

VOID DestroyDevice(HANDLE hDevice);

VOID PrintLine(LPSTR lpszInfo);
VOID PrintChar(WORD ch);
int  _hx_printf(const char* fmt,...);
int  _hx_sprintf(char *buf, const char *fmt, ...);
//void* memset (void *dst,int val,size_t count);

#ifdef __cplusplus
}
#endif


#endif //__K_IO_H__
