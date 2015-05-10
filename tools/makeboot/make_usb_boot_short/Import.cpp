#include "stdafx.h"
#include <string.h>
#include <vector>
#include "make_usb_boot.h"

using namespace std;

#ifdef UNICODE
typedef vector<wstring>		FILE_LIST;
#else
typedef vector<string>		FILE_LIST;
#endif   


HANDLE APIENTRY   OpenVhdFile(LPCTSTR pVhdFile,DWORD dwPartitionSatrt);
VOID   APIENTRY   CloseVhdFile(HANDLE hObj);
BOOL   APIENTRY   AddDirToVhd(HANDLE hVhdObj, LPCSTR pDirName);
BOOL   APIENTRY   AddFileToVhd(HANDLE hVhdObj,LPCSTR pSrcFile,LPCSTR pDiskFile);


TCHAR s_szSrcDir[MAX_PATH]  = {0};

//枚举目录下的全部文件，并加入到导入列表
DWORD APIENTRY EnumDirFiles(LPCTSTR pDir,FILE_LIST* pFileList)
{
	WIN32_FIND_DATA FindFileData      = {0};
	TCHAR		    szFindPath[512]   = {0};	
	HANDLE		    hFind             = NULL;
	INT             nFindCount        = 0;


	lstrcpy(szFindPath,pDir);
	lstrcat(szFindPath,L"*.*");

	hFind = FindFirstFile(szFindPath, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		//不是目录，直接返回
		pFileList->push_back(pDir);		
		return S_FALSE;
	}

	while (TRUE)
	{
		TCHAR  szFullPath[512] = {0};

		if(FindNextFile(hFind, &FindFileData) == 0) 
		{
			break;			
		}

		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN || lstrcmpi(FindFileData.cFileName,L"..") ==0 ) 
		{
			continue;
		}

		
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
			wsprintf(szFullPath,_T("%s%s\\"),pDir,FindFileData.cFileName);
			EnumDirFiles(szFullPath,pFileList);
			}
		else{
			wsprintf(szFullPath,_T("%s%s"),pDir,FindFileData.cFileName);
			nFindCount ++;
			pFileList->push_back(szFullPath);				
			}		
		}

	FindClose(hFind);

	if(nFindCount == 0)
	{
		pFileList->push_back(pDir);	
	}

	return S_OK;
}


BOOL APIENTRY ImportFile(HANDLE hVhdFile,LPCTSTR pSrcFile,LPCTSTR pDstFile)
{
	CHAR  szDstFile[MFS] = {0};
	CHAR  szSrcFile[MFS] = {0};

	WideCharToMultiByte(CP_ACP, 0, pSrcFile, -1, szSrcFile, sizeof(szSrcFile), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, pDstFile, -1, szDstFile, sizeof(szDstFile), NULL, NULL);

	AddFileToVhd(hVhdFile,szSrcFile,szDstFile);

	return TRUE;
}

BOOL APIENTRY MakeDir(HANDLE hVhdFile,LPCTSTR pDirPath)
{
	CHAR  szDstDir[MFS] = {0};
	INT   i             = 0;

	WideCharToMultiByte(CP_ACP, 0, pDirPath, -1, szDstDir, sizeof(szDstDir), NULL, NULL);

	for(i=0;i<lstrlenA(szDstDir);i++)
	{
		if(szDstDir[i] == '\\')
		{
			CHAR  szShorDir[256] = {0};

			lstrcpynA(szShorDir,szDstDir,i+1);
			AddDirToVhd(hVhdFile,szShorDir);
		}
	}
	
	return TRUE;
}

//导入文件到Vhd文件
DWORD APIENTRY ImportFilesToVhd(LPCTSTR pVhdFile,LPCTSTR pSrcDir,DWORD dwPartitionSatrt)
{
	FILE_LIST   szFileList;
	HANDLE      hVhdObj    = NULL;
	UINT        nFileCount = 0;
	UINT        nFilePos   = 0;
		

	lstrcpy(s_szSrcDir,pSrcDir);
	EnumDirFiles(s_szSrcDir,&szFileList);

	nFileCount = szFileList.size();
	if(nFileCount == 0)
	{
		return nFileCount;
	}

	hVhdObj = OpenVhdFile(pVhdFile,dwPartitionSatrt);
	if(hVhdObj == NULL) 
	{
		return 0;
	}

	for(nFilePos=0;nFilePos <nFileCount;nFilePos ++)
	{
		TCHAR   szSrcFile[MFS] = {0};
		TCHAR   szDstFile[MFS] = {0};
		LPTSTR  pPos           = NULL;

		lstrcpy(szSrcFile,szFileList.at(nFilePos).c_str());
		
		//设置目标文件路径
		pPos   = szSrcFile+lstrlen(s_szSrcDir);
		lstrcpy(szDstFile,L"C:\\");
		lstrcat(szDstFile,pPos);

		//转大写
		CharUpper(szDstFile);

		//预先生成目录
		MakeDir(hVhdObj,szDstFile);

		//导入文件
		ImportFile(hVhdObj,szSrcFile,szDstFile);
	}

	CloseVhdFile(hVhdObj);

	return S_OK;
}

//枚举目录下的全部文件
DWORD APIENTRY EnumDirFiles(LPCTSTR pDir,PUINT64 pTotalSize)
{
	WIN32_FIND_DATA FindFileData      = {0};
	TCHAR		    szFindPath[512]   = {0};	
	HANDLE		    hFind             = NULL;
	INT             nFindCount        = 0;


	lstrcpy(szFindPath,pDir);
	lstrcat(szFindPath,L"*.*");

	hFind = FindFirstFile(szFindPath, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		//不是目录，直接返回
		return S_FALSE;
	}

	while (TRUE)
	{		

		if(FindNextFile(hFind, &FindFileData) == 0) 
		{
			break;			
		}

		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN || lstrcmpi(FindFileData.cFileName,L"..") ==0 ) 
		{
			continue;
		}
		
		if(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{	
			TCHAR  szFullPath[512] = {0};

			wsprintf(szFullPath,_T("%s%s\\"),pDir,FindFileData.cFileName);
			EnumDirFiles(szFullPath,pTotalSize);
		}
		else
		{			
			*pTotalSize +=  ((FindFileData.nFileSizeHigh<<32) +FindFileData.nFileSizeLow);			
		}		
	}

	FindClose(hFind);

	return S_OK;
}

//得到目录总大小
UINT64 APIENTRY GetDirSize(LPCTSTR pSrcDir)
{	
	UINT64   nTotalSize = 0;

	lstrcpy(s_szSrcDir,pSrcDir);
	EnumDirFiles(s_szSrcDir,&nTotalSize);

	return nTotalSize;
}