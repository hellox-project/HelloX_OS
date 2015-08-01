//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 07 JAN,2008
//    Module Name               : FATSTR.CPP
//    Module Funciton           : 
//                                FAT related string manipulating routines are in this module.
//                                FAT manager is a object to manage FAT table for FAT file
//                                system.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#ifndef __FAT32_H__
#include "fat32.h"
#endif

#ifndef __FSSTR_H__
#include "fsstr.h"
#endif

#include "../lib/stdio.h"

//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//A helper routine to initialize the dot and dotdot directory's name.
static VOID InitDot(__FAT32_SHORTENTRY* pfse)
{
	int i;

	if(NULL == pfse)
	{
		return;
	}
	pfse->FileName[0] = '.';
	for(i = 1;i < 11;i ++)
	{
		pfse->FileName[i] = ' ';
	}
	return;
}

static VOID InitDotdot(__FAT32_SHORTENTRY* pfse)
{
	int i;

	if(NULL == pfse)
	{
		return;
	}
	pfse->FileName[0] = '.';
	pfse->FileName[1] = '.';
	for(i = 2;i < 11;i ++)
	{
		pfse->FileName[i] = ' ';
	}
	return;
}

VOID  AddSpace(CHAR* pStrBuf,INT nCount)
{
	INT nSpaceCount = nCount;

	while(nSpaceCount)
	{
		strcat(pStrBuf," ");
		nSpaceCount --;
	}
}
//Initialize a FAT32 shortentry given it's name,start cluster number,and other
//information.
BOOL InitShortEntry(__FAT32_SHORTENTRY* pfse,CHAR* pszName,DWORD dwFirstClus,DWORD dwInitSize,BYTE FileAttr)
{
	BOOL            bResult    = FALSE;
	CHAR*           pDotPos    = NULL;
	CHAR*           pStart     = NULL;
	int             i,nNameLen;

	if((NULL == pfse) || (NULL == pszName))
	{
		goto __TERMINAL;
	}

	memzero(pfse,sizeof(__FAT32_SHORTENTRY));
	if(StrCmp(pszName,"."))
	{
		InitDot(pfse);
		goto __INITOTHER;
	}
	if(StrCmp(pszName,".."))
	{
		InitDotdot(pfse);
		goto __INITOTHER;
	}

	//long file name. don't change 
	if(strstr(pszName,"~"))
	{
		memcpy(pfse->FileName,pszName,FAT32_SHORTDIR_FILENAME_LEN);
		goto __INITOTHER;
	}
	//Now convert the name to directory format.
	nNameLen = StrLen(pszName);
	pDotPos  = pszName + (nNameLen - 1);
	while((*pDotPos != '.') && (pDotPos != pszName))
	{
		pDotPos --;
	}
	if((pDotPos == pszName) && (*pDotPos == '.')) //First character is dot,invalid.
	{
		goto __TERMINAL;
	}
	if(pDotPos == pszName)  //Without extension.
	{
		pDotPos = pszName + nNameLen;
	}
	i = 0;
	pStart = pszName;
	while(pStart != pDotPos)  //Get the name's part.
	{
		pfse->FileName[i] = *pStart;
		i ++;
		pStart ++;
		if(i == 8)
		{
			break;
		}
	}
	while(i < 8)  //Fill space.
	{
		pfse->FileName[i] = ' '; //0x20.
		i ++;
	}
	//Process the extension.
	pStart = pszName + (nNameLen - 1);  //Now pStart pointing to the tail of name.
	i = 10;
	while((pStart > pDotPos) && (i > 7))
	{
		pfse->FileName[i] = *pStart;
		i --;
		pStart --;
	}
	while(i > 7)
	{
		pfse->FileName[i --] = ' ';
	}
	
__INITOTHER:
	pfse->FileAttributes   = FileAttr;
	pfse->dwFileSize       = dwInitSize;
	pfse->wFirstClusHi     = (WORD)(dwFirstClus >> 16);
	pfse->wFirstClusLow    = (WORD)dwFirstClus;
	
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Convert a short entry to general file struct,which is FS_FIND_DATA.
BOOL ConvertShortEntry(__FAT32_SHORTENTRY* pfse,FS_FIND_DATA* pffd)
{
	BOOL    bResult    = FALSE;
	CHAR*   pExtStart  = NULL;
	CHAR*   pStart     = NULL;
	int     i,end,j;	
	WORD    ch = 0x0700;

	if((NULL == pfse) || (NULL == pffd))
	{
		goto __TERMINAL;
	}

	pffd->dwFileAttribute   = pfse->FileAttributes;
	pffd->nFileSizeLow      = pfse->dwFileSize;
		
	//Process file's name now.
	pStart    = &pfse->FileName[0];
	pExtStart = &pfse->FileName[8];
	i = 0;
	while(pStart < pExtStart)
	{
		if((*pStart == ' ') || (0 == *pStart))
		{
			pStart ++;
			continue;
		}
		pffd->cAlternateFileName[i] = *pStart;
		pStart ++;
		i ++;
	}
	end = i;
	i += 1;
	for(j = 0;j < 3;j++)
	{
		if((' ' == *pExtStart) || (0 == *pExtStart)) //Skip the space.
		{
			pExtStart ++;
			continue;
		}
		pffd->cAlternateFileName[i] = *pExtStart;
		i ++;
		pExtStart ++;
	}
	if(i == end + 1)  //Without any extension.
	{
		bResult = TRUE;
		pffd->cAlternateFileName[end] = 0;
		goto __TERMINAL;
	}
	pffd->cAlternateFileName[i]   = 0;
	pffd->cAlternateFileName[end] = '.';
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Convert the short name in FAT entry to dot format.
//  @pfse    : The short entry with the name to convert;
//  @pResult : The converting result,pointing to a string whose length must
//             longer or equal to 13.
/*BOOL ConvertName(__FAT32_SHORTENTRY* pfse,BYTE* pResult)
{
	int i,j = 0;

	if((NULL == pfse) || (NULL == pResult))  //Invalid parameters.
	{
		return FALSE;
	}
	if(pfse->FileAttributes & FILE_ATTR_DIRECTORY)  //Directory name.
	{
		for(i = 0;i < 11;i ++)
		{
			if(' ' == pfse->FileName[i])  //Skip the space.
			{
				continue;
			}
			pResult[j ++] = pfse->FileName[i];
		}
		pResult[j] = 0;
		return TRUE;
	}
	//Special process for file.
	for(i = 0;i < 8;i ++)  //Convert the file name first.
	{
		if(' ' == pfse->FileName[i])  //Skip space.
		{
			continue;
		}
		pResult[j ++] = pfse->FileName[i];
	}
	//Now convert the file name extension.
	if((pfse->FileName[8] == ' ') &&
	   (pfse->FileName[9] == ' ') &&
	   (pfse->FileName[10] == ' '))  //If no extension.
    {
		pResult[j] = 0;  //Terminate the file string.
		return TRUE;
	}
	pResult[j ++] = '.'; //Append the dot of name string.
	for(i = 8;i < 11;i ++)
	{
		if(' ' == pfse->FileName[i])
		{
			continue;
		}
		pResult[j ++] = pfse->FileName[i];
	}
	pResult[j] = 0;
	return TRUE;
}*/

#endif
