//------------------------------------------------------------------------
//  Common functions for file system module.All file systems,include FAT32,
//  NTFS or others,will use several or all functions in this module.
//
//  Author                 : Garry.Xin
//  Initial date           : Jul 05,2013
//  Last updated           : Jul 05,2013
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------

#ifndef __FSSTR_H__
#define __FSSTR_H__

//Functions to manipulate file and directory name.
BOOL NameIsValid(CHAR* pFullName);
BOOL GetFullNameLevel(CHAR* pFullName,DWORD* pdwLevel);       //Get a full name's sub-dir level.
BOOL GetSubDirectory(CHAR* pFullName,DWORD dwLevel,CHAR* pSubDir);
BOOL GetPathName(CHAR* pFullName,CHAR* pDir,CHAR* pFileName);

int wstrcmp(const WCHAR* src,const WCHAR* dst);
WCHAR* wstrcpy (WCHAR* dest,const WCHAR* src);
int wstrlen(const WCHAR* src);
WCHAR* byte2unicode(WCHAR* dest,const char* src);
void tocapital(WCHAR* src);

//WCHAR version of functions to manipulate file and directory name.
BOOL wNameIsValid(WCHAR* pFullName);
BOOL wGetFullNameLevel(WCHAR* pFullName,DWORD* pdwLevel);
BOOL wGetSubDirectory(WCHAR* pFullName,DWORD dwLevel,WCHAR* pSubDir);
BOOL wGetPathName(WCHAR* pFullName,WCHAR* pDir,WCHAR* pFileName);

#endif //__FSSTR_H__
