//------------------------------------------------------------------------
//  NTFS file system implementation code,the third part,since it's code line
//  number is too large that can not be hold in one file.
//
//  Author                 : Garry.Xin
//  Initial date           : Dec 06,2011
//  Last updated           : Dec 06,2011
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
 
#ifndef __NTFS_H__
#include "NTFS.H"
#endif
 
//
//UNICODE string operations.
//
 
//Convert a byte string to UNICODE string,return the unicode string's pointer.
//Make sure the dest buffer's length must equal or larger than src length.
WCHAR* byte2unicode(WCHAR* dest,const char* src)
{
         int index = 0;
 
         if((NULL == src) || (NULL == dest))
         {
                   return NULL;
         }
         while(src[index])
         {
                   dest[index] = src[index];
                   index ++;
         }
         dest[index] = 0;
         return dest;
}
 
//Campare two unicode strings check if they are equal,0 will be returned
//if equal,otherwise return -1.
int wstrcmp(const WCHAR* src,const WCHAR* dst)
{
         if((NULL == src) && (NULL == dst))
         {
                   return 0;
         }
         if((NULL == src) || (NULL == dst))
         {
                   return -1;
         }
         while((*src) && (*dst) && (*src == *dst))
         {
                   src ++;
                   dst ++;
         }
         return (*src == *dst) ? 0 : -1;
}
 
//Copy src to dest,return the dest.
WCHAR* wstrcpy (WCHAR* dest,const WCHAR* src)
{
  WCHAR c;
  WCHAR *s = (WCHAR *)src;
  const int off = dest - s - 1;
 
  do{
           c = *s++;
           s[off] = c;
    }while (c != '\0');
  return dest;
}
 
//Returns one unicode string's length.
int wstrlen(const WCHAR* src)
{
         WCHAR* s = (WCHAR*)src;
         int length = 0;
         if(NULL == s)
         {
                   return -1;
         }
         while(*s)
         {
                   length ++;
                   s ++;
         }
         return length;
}
 
//A helper routine to check the validity of a file name.
//For a regular file name,the first three characters must be file system identifier,a colon,
//and a slash character.
BOOL NameIsValid(WCHAR* pFullName)
{
         if(NULL == pFullName)
         {
                   return FALSE;
         }
         if(0 == pFullName[0])
         {
                   return FALSE;
         }
         if(0 == pFullName[1])
         {
                   return FALSE;
         }
         if(0 == pFullName[2])
         {
                   return FALSE;
         }
 
         if(':' != pFullName[1])  //File system identifier colon must exist.
         {
                   return FALSE;
         }
         if('\\' != pFullName[2]) //The third character must be splitter.
         {
                   return FALSE;
         }
         return TRUE;
}
 
//Get the directory level given a string.
//For the string only contains the file system identifier and only one file name,
//which resides in root directory,the level is 0.
BOOL GetFullNameLevel(WCHAR* pFullName,UINT_32* pdwLevel)
{
         int i = 0;
         UINT_32 dwLevel = 0;
 
         if((NULL == pFullName) || (NULL == pdwLevel)) //Invalid parameters.
         {
                   return FALSE;
         }
         if(!NameIsValid(pFullName))  //Is not a valid file name.
         {
                   return FALSE;
         }
 
         while(pFullName[i])
         {
                   if(pFullName[i] == '\\')
                   {
                            dwLevel += 1;
                   }
                   i ++;
         }
         *pdwLevel = dwLevel - 1;      //Substract the splitter between FS identifier and name.
         return TRUE;
}
 
//Get the desired level subdirectory from a full name.
//pSubDir will contain the sub-directory name if successfully,so it must long
//enough to contain the result.
BOOL GetSubDirectory(WCHAR* pFullName,UINT_32 dwLevel,WCHAR* pSubDir)
{
         BOOL       bResult         = FALSE;
         UINT_32      dwTotalLevel    = 0;
         UINT_32      i = 0;
         UINT_32      j = 0;
         WCHAR      buffer[256];  //Contain sub-directory name temporary.
 
         if((NULL == pFullName) || (NULL == pSubDir) || (0 == dwLevel))  //Level shoud not be zero.
         {
                   return FALSE;
         }
         if(!GetFullNameLevel(pFullName,&dwTotalLevel))
         {
                   return FALSE;
         }
         if(dwLevel > dwTotalLevel)  //Exceed the total level.
         {
                   return FALSE;
         }
         dwTotalLevel = 0;
         while(pFullName[i])
         {
                   if(pFullName[i] == '\\')  //Splitter encountered.
                   {
                            j = 0;
                            i ++;  //Skip the slash.
                            while((pFullName[i] != '\\') && (pFullName[i]))
                            {
                                     buffer[j] = pFullName[i];
                                     i ++;
                                     j ++;
                            }
                            buffer[j] = 0;               //Set the terminator.
                            dwTotalLevel += 1;
                            if(dwLevel == dwTotalLevel)  //Subdirectory found.
                            {
                                     bResult = TRUE;
                                     break;
                            }
                            i --;  //If the slash is skiped,the next sub-directory maybe eliminated.
                   }
                   i ++;
         }
         if(bResult)
         {
                   //strcpy((CHAR*)pSubDir,(CHAR*)&buffer[0]);
                   wstrcpy(pSubDir,&buffer[0]);
         }
         return bResult;
}
 
//Segment one full file name into directory part and file name part.
//pDir and pFileName must be long enough to contain the result.
//If the full name's level is zero,that no subdirectory in the full name,
//the pDir[0] will contain the file system identifier to indicate this.
//If the full name only contain a directory,i.e,the last character of
//the full name is a slash character,then the pFileName[0] will be set to 0.
BOOL GetPathName(WCHAR* pFullName,WCHAR* pDir,WCHAR* pFileName)
{
         WCHAR       DirName[MAX_FILE_NAME_LEN];
         WCHAR       FileName[13];
         WCHAR       tmp;
         int         i = 0;
         int         j = 0;
 
         if((NULL == pFullName) || (NULL == pDir) || (NULL == pFileName))  //Invalid parameters.
         {
                   return FALSE;
         }
         if(!NameIsValid(pFullName))  //Not a valid file name.
         {
                   return FALSE;
         }
         //strcpy((CHAR*)&DirName[0],(CHAR*)pFullName);
         wstrcpy(DirName,pFullName);
         i = wstrlen(pFullName);
         i -= 1;
         if(pFullName[i] == '\\') //The last character is a splitter,means only dir name present.
         {
                   //DirName[i] = 0;      //Eliminate the slash.
                   //strcpy((CHAR*)pDir,(CHAR*)&DirName[0]);
                   wstrcpy(pDir,&DirName[0]);
                   pFileName[0] = 0;
                   return TRUE;
         }
         j = 0;
         while(pFullName[i] != '\\')  //Get the file name part.
         {
                   FileName[j ++] = pFullName[i --];
         }
         DirName[i + 1]  = 0;  //Keep the slash.
         FileName[j] = 0;
         //Now reverse the file name string.
         for(i = 0;i < (j / 2);i ++)
         {
                   tmp = FileName[i];
                   FileName[i] = FileName[j - i - 1];
                   FileName[j - i - 1] = tmp;
         }
         //strcpy((CHAR*)pDir,(CHAR*)&DirName[0]);
         wstrcpy(pDir,&DirName[0]);
         //strcpy((CHAR*)pFileName,(CHAR*)&FileName[0]);
         wstrcpy(pFileName,&FileName[0]);
         return TRUE;
}

