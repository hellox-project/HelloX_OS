//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 07 JAN,2008
//    Module Name               : FATMGR.CPP
//    Module Funciton           : 
//                                FAT manager implementation code in this file.
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

#ifndef __FSSTR_H__
#include "fsstr.h"
#endif

#ifndef __FAT32_H__
#include "fat32.h"
#endif

#include "../lib/stdio.h"

//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF


//set new file date and time
VOID SetFatFileDateTime(__FAT32_SHORTENTRY*  pDirEntry,DWORD dwTimeFlage)
{
	BYTE   time[6];
	WORD   wdate;
	WORD   wtime;
	WORD   wtemp; 

	__GetTime(&time[0]);
	
	//Fat32file 文件时间格式
	//date  fmt : yyyyyyymmmmddddd  (y年份 ，m月份 d日期) 注意: 实际年份值 = 1980+yyyyyyy
	//time  fmt : hhhhhhmmmmmsssss (h小时 ，m分钟 s秒 小时字段和秒字段需乘2为实际的数字)
	
	 /*
	 时间为两个字节的16bit被划分为3个部分
	 0~4bit为秒，以2秒为单位，有效值为0~29，可以表示的时刻为0~58
	 5~9bit为分，有效值为0~59
	 10~15bit为时，有效值为0~46，除以2后为正确的小时数(0-23)
	 */

	//year
	wtemp = (time[0]+2000)-1980;  wtemp = wtemp<<9;
	wdate = wtemp;

	//month
	wtemp = time[1]; wtemp = wtemp<<5;
	wdate = wdate|wtemp;
	
	//date
	wtemp = time[2]; 
	wdate = wdate|wtemp;

	//hour
	wtemp = time[3]*2; wtemp = wtemp<<10;
	wtime = wtemp;

	//minute
	wtemp = time[4]; wtemp = wtemp<<5;
	wtime |= wtemp;

	//second
	wtemp = time[5]/2; 
	wtime |= wtemp;

	if(dwTimeFlage&FAT32_DATETIME_CREATE)
	{
		pDirEntry->CreateDate       = wdate;
		pDirEntry->CreateTime       = wtime;
	}

	if(dwTimeFlage&FAT32_DATETIME_WRITE)
	{
		pDirEntry->WriteDate        = wdate;	
		pDirEntry->WriteTime        = wtime;
	}

	if(dwTimeFlage&FAT32_DATETIME_ACCEPT)
	{
		pDirEntry->LastAccessDate   = wdate;		
	}
	

}

//Implementation of GetNextCluster.
//pdwCluster contains the current cluster,if next cluster can be fetched,TRUE will be
//returned and pdwCluster contains the next one,or else FALSE will be returned.
BOOL GetNextCluster(__FAT32_FS* pFat32Fs,DWORD* pdwCluster)
{
	DWORD           dwNextCluster    = EOC;
	DWORD           dwClusSector     = 0;
	DWORD           dwCurrCluster    = 0;
	DWORD           dwClusOffset     = 0;
	BYTE            buff[SECTOR_SIZE];

	if((NULL == pFat32Fs) || (NULL == pdwCluster))  //Invalid parameter.
	{
		return FALSE;
	}
	dwCurrCluster = *pdwCluster;
	if(0 == dwCurrCluster)
	{
		dwCurrCluster = 2;
	}
	//Calculate the sector number of current cluster number.
	dwClusSector = dwCurrCluster / 128;  //128 fat cluster entry per sector.
	dwClusSector += pFat32Fs->dwFatBeginSector;
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwClusSector,
		1,
		buff))  //Can not read FAT sector.
	{
		return FALSE;
	}
	dwClusOffset  =   (dwCurrCluster - (dwCurrCluster / 128) * 128) * sizeof(DWORD);
	dwNextCluster =   *(DWORD*)(&buff[0] + dwClusOffset);
	dwNextCluster &=  0x0FFFFFFF;   //Mask the leading 4 bits.
	if(dwNextCluster == 0)  //Invalid cluster value.
	{
		return FALSE;
	}
	*pdwCluster = dwNextCluster;
	return TRUE;
}

//GetClusterSector,returns the appropriate sector number of given cluster number,0 indicates
//failed.
DWORD GetClusterSector(__FAT32_FS* pFat32Fs,DWORD dwCluster)
{
	if((NULL == pFat32Fs) || (0 == dwCluster))  //Invalid parameters.
	{
		return 0;
	}
	if(dwCluster < 2)  //Invalid value.
	{
		return 0;
	}
	return pFat32Fs->dwDataSectorStart + (dwCluster - 2) * pFat32Fs->SectorPerClus;
}

//Get one free cluster and mark the cluster as used.
//If find,TRUE will be returned and the cluster number will be set in pdwFreeCluster,
//else FALSE will be returned without any changing to pdwFreeCluster.
BOOL GetFreeCluster(__FAT32_FS* pFat32Fs,DWORD dwStartToFind,DWORD* pdwFreeCluster)
{
	DWORD           dwCurrCluster = 0;
	DWORD           dwSector      = 0;
	BYTE*           pBuffer       = NULL;
	DWORD*          pCluster      = NULL;
	BOOL            bResult       = FALSE;
	DWORD           i;

	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwBytePerSector,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)  //Can not allocate temporary buffer.
	{
		return FALSE;
	}

	dwSector = pFat32Fs->dwFatBeginSector;
	while(dwSector < pFat32Fs->dwFatSectorNum + pFat32Fs->dwFatBeginSector)
	{
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			1,
			pBuffer))  //Can not read the sector from fat region.
		{
			goto __TERMINAL;
		}
		pCluster = (DWORD*)pBuffer;
		//Analysis the sector to find a free cluster entry.
		for(i = 0;i < pFat32Fs->dwBytePerSector / sizeof(DWORD);i ++)
		{
			if(0 == ((*pCluster) & 0x0FFFFFFF))  //Find one free cluster.
			{
				(*pCluster) |= 0x0FFFFFFF;  //Mark the cluster to EOC,occupied.
				if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
					    dwSector,
						1,
						pBuffer))
				{
					goto __TERMINAL;
				}
				//Write to backup FAT region.
				WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
					dwSector + pFat32Fs->dwFatSectorNum,
					1,
					pBuffer);  //It is no matter if write to backup fat failed.
				bResult = TRUE;
				//_hx_sprintf(Buffer,"  In GetFreeCluster,success,cluster = %d,next = %d",dwCurrCluster,*pCluster);
				//PrintLine(Buffer);
				goto __TERMINAL;
			}
			pCluster ++;
			dwCurrCluster ++;
		}
		dwSector ++;  //Travel the FAT region in sector unit one by one.
	}
__TERMINAL:
		if(pBuffer)
		{
			KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
		}
		if(bResult)  //Found one free cluster successfully,return it.
		{
			*pdwFreeCluster = dwCurrCluster;
		}
		return bResult;
}

//Release one cluster.
BOOL ReleaseCluster(__FAT32_FS* pFat32Fs,DWORD dwCluster)
{
	BYTE*         pBuffer   = NULL;
	DWORD         dwSector  = 0;
	DWORD         dwOffset  = 0;
	BOOL          bResult   = FALSE;

	if((NULL == pFat32Fs) || (dwCluster < 2) || IS_EOC(dwCluster))
	{
		goto __TERMINAL;
	}
	dwSector = dwCluster / 128;
	if(dwSector > pFat32Fs->dwFatSectorNum)
	{
		goto __TERMINAL;
	}
	dwSector += pFat32Fs->dwFatBeginSector;
	dwOffset  = (dwCluster - (dwCluster / 128) * 128) * sizeof(DWORD);

	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwBytePerSector,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	//Read the fat sector where this cluster's index resides,modify to zero and write it back.
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		goto __TERMINAL;
	}
	*(DWORD*)(pBuffer + dwOffset) &= 0xF0000000;
	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		goto __TERMINAL;
	}
	//All successfully.
	bResult = TRUE;
__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Append one free cluster to the tail of a cluster chain.
//The pdwCurrCluster contains the laster cluster of a cluster chain,if this routine
//executes successfully,it will return TRUE and pdwCurrCluster contains the cluster
//number value appended to chain right now.Else FALSE will be returned and the pdwCurrCluster
//keep unchanged.
BOOL AppendClusterToChain(__FAT32_FS* pFat32Fs,DWORD* pdwCurrCluster)
{
	DWORD         dwCurrCluster   = 0;
	DWORD         dwNextCluster   = 0;
	DWORD         dwSector        = 0;
	DWORD         dwOffset        = 0;
	BOOL          bResult         = FALSE;
	BYTE*         pBuffer         = NULL;
	DWORD         dwEndCluster    = 0;
	

	if((NULL == pFat32Fs) || (NULL == pdwCurrCluster))
	{
		goto __TERMINAL;
	}
	dwCurrCluster = *pdwCurrCluster;
	if((2 > dwCurrCluster) || IS_EOC(dwCurrCluster))
	{
		goto __TERMINAL;
	}

	dwSector = dwCurrCluster / 128;
	if(dwSector > pFat32Fs->dwFatSectorNum)  //Exceed the FAT size.
	{
		goto __TERMINAL;
	}
	dwSector += pFat32Fs->dwFatBeginSector;  //Now dwSector is the physical sector number of dwCurrCluster in fat.
	dwOffset  = (dwCurrCluster - (dwCurrCluster / 128) * 128) * sizeof(DWORD); //Get sector offset.

	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwBytePerSector,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	//Try to get a free cluster.
	if(!GetFreeCluster(pFat32Fs,0,&dwNextCluster))
	{
		goto __TERMINAL;
	}
	//The following operation must behind GetFreeCluster routine above,because
	//GetFreeCluster routine will modify the content of FAT,and this change must
	//be taken before the following read.
	//One complicated problem has been caused by this reason.
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		PrintLine("AppendClusterToChain. ReadDeviceSector error");
		goto __TERMINAL;
	}
	//Save the next cluster to chain.
	*(DWORD*)(pBuffer + dwOffset) &= 0xF0000000;  //Keep the leading 4 bits.
	*(DWORD*)(pBuffer + dwOffset) += (dwNextCluster & 0x0FFFFFFF);
	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		ReleaseCluster(pFat32Fs,dwNextCluster);   //Release this cluster.

		PrintLine("AppendClusterToChain. WriteDeviceSector error");
		goto __TERMINAL;
	}
	dwEndCluster = *(DWORD*)(pBuffer + dwOffset);
	if(!GetNextCluster(pFat32Fs,&dwEndCluster))
	{
		PrintLine("  In AppendClusterToChain: Can not get next cluster.");
	}
	bResult = TRUE;  //Anything is in place.
__TERMINAL:
	if(pBuffer)  //Should release it.
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	if(bResult)
	{
		*pdwCurrCluster = (dwNextCluster & 0x0FFFFFFF);
	}
	return bResult;
}

//Release one cluster chain,the cluster chain starts from dwStartCluster.
BOOL ReleaseClusterChain(__FAT32_FS* pFat32Fs,DWORD dwStartCluster)
{
	DWORD   dwNextCluster = 0;
	DWORD   dwCurrCluster = 0;
	BOOL    bResult       = FALSE;

	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster))
	{
		goto __TERMINAL;
	}
	dwNextCluster = dwStartCluster;
	while(!IS_EOC(dwNextCluster))
	{
		dwCurrCluster = dwNextCluster;
		if(!GetNextCluster(pFat32Fs,&dwNextCluster))
		{
			goto __TERMINAL;
		}
		ReleaseCluster(pFat32Fs,dwCurrCluster);  //Release current one.
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Initializes one directory,i.e,write dot and dotdot directories into one directory newly created.
//  @dwParentCluster  : The start cluster number of target directory's parent directory;
//  @dwDirCluster     : The start cluster number of target directory.
BOOL InitDirectory(__FAT32_FS* pFat32Fs,DWORD dwParentCluster,DWORD dwDirCluster)
{
	BYTE*               pBuffer = NULL;       //Temprory buffer used to contain first cluster.
	__FAT32_SHORTENTRY* pfse    = NULL;
	BOOL                bResult = FALSE;
	DWORD               dwDirSector = 0;
	CHAR                Buffer[128];

	if((NULL == pFat32Fs) || (dwParentCluster < 2) || (dwDirCluster < 2) || IS_EOC(dwParentCluster) ||
	   IS_EOC(dwDirCluster))
	{
		_hx_sprintf(Buffer,"  In InitDirectory: parent clus = %d,dir clus = %d",
			dwParentCluster,
			dwDirCluster);
		PrintLine(Buffer);
		PrintLine("  In InitDirectory: Condition 0");
		goto __TERMINAL;
	}
	//Allocate temporary buffer.
	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwClusterSize,
		KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		PrintLine("  In InitDirectory: Condition 1");
		goto __TERMINAL;
	}

	dwDirSector = GetClusterSector(pFat32Fs,dwDirCluster);  //Get the start cluster number of target dir.
	if(IS_EOC(dwDirSector))
	{
		PrintLine("  In InitDirectory: Condition 2");
		goto __TERMINAL;
	}
	//Clear cluster's content to zero.
	memzero(pBuffer,pFat32Fs->dwClusterSize);
	pfse = (__FAT32_SHORTENTRY*)pBuffer;
	InitShortEntry(pfse,".",dwDirCluster,0,FILE_ATTR_DIRECTORY);
	//Create the dot directory.
	/*pfse->CreateDate        = 0;
	pfse->CreateTime        = 0;
	pfse->CreateTimeTenth   = 0;
	pfse->dwFileSize        = 0;
	pfse->FileAttributes    = FILE_ATTR_DIRECTORY;
	pfse->FileName[0]       = '.';
	pfse->LastAccessDate    = 0;
	pfse->wFirstClusHi      = (WORD)(dwDirCluster >> 16);
	pfse->wFirstClusLow     = (WORD)dwDirCluster;
	pfse->WriteDate         = 0;
	pfse->WriteTime         = 0;*/
	pfse ++;

	//Create the dotdot directory.
	InitShortEntry(pfse,"..",dwDirCluster,0,FILE_ATTR_DIRECTORY);
	/*pfse->CreateDate        = 0;
	pfse->CreateTime        = 0;
	pfse->CreateTimeTenth   = 0;
	pfse->dwFileSize        = 0;
	pfse->FileAttributes    = FILE_ATTR_DIRECTORY;
	pfse->FileName[0]       = '.';
	pfse->FileName[1]       = '.';
	pfse->LastAccessDate    = 0;
	pfse->wFirstClusHi      = (WORD)(dwParentCluster >> 16);
	pfse->wFirstClusLow     = (WORD)dwParentCluster;
	pfse->WriteDate         = 0;
	pfse->WriteTime         = 0;*/
	pfse ++;

	//Write the cluster into directory.
	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwDirSector,
		pFat32Fs->SectorPerClus,
		pBuffer))
	{
		PrintLine("  In InitDirectory: Condition 3");
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Find one empty short directory entry in a cluster chain start from dwStartCluster,
//and save the short entry pointed by pfse into this entry.If can not find a free one
//in the whole cluster chain,then append a free cluster in the chain and save it.
BOOL CreateDirEntry(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,__FAT32_SHORTENTRY* pDirEntry)
{
	__FAT32_SHORTENTRY     DirEntry;
	__FAT32_SHORTENTRY*    pfse       = NULL;
	DWORD                  dwSector   = 0;
	DWORD                  dwCurrCluster = 0;
	DWORD                  dwNextCluster = 0;
	BYTE*                  pBuffer       = NULL;
	CHAR                   DirName[13]   = {0};
	DWORD                  i;
	BOOL                   bFind      = FALSE;
	BOOL                   bResult    = FALSE;

	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pDirEntry))
	{
		PrintLine("  In CreateDirEntry,Condition 0");
		goto __TERMINAL;
	}
	if(!ConvertName(pDirEntry,(BYTE*)&DirName[0]))
	{
		PrintLine("  In CreateDirEntry,Condition 1");
		goto __TERMINAL;
	}
	//Check if the directory to be created has already in directory.
	if(GetShortEntry(pFat32Fs,dwStartCluster,DirName,&DirEntry,NULL,NULL))  //Directory already exists.
	{
		PrintLine("  In CreateDirEntry: The specified directory already exist.");
		goto __TERMINAL;
	}
	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwClusterSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		PrintLine("  In CreateDirEntry,can not allocate memory for temporary buffer.");
		goto __TERMINAL;
	}
	//Try to find a free directory entry in the given directory,if can not find,then
	//allocate a free cluster,append it to the given directory.
	dwNextCluster = dwStartCluster;
	while(!IS_EOC(dwNextCluster))
	{
		dwCurrCluster = dwNextCluster;
		dwSector = GetClusterSector(pFat32Fs,dwCurrCluster);
		if(0 == dwSector)
		{
			PrintLine("  In CreateDirEntry,Condition 2");
			goto __TERMINAL;
		}
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))
		{
			PrintLine("  In CreateDirEntry,Condition 3");
			goto __TERMINAL;
		}
		//Search this cluster from begin.
		pfse = (__FAT32_SHORTENTRY*)pBuffer;
		for(i = 0;i < pFat32Fs->dwClusterSize / sizeof(__FAT32_SHORTENTRY);i++)
		{
			if((0 == pfse->FileName[0]) || (0xE5 == (BYTE)pfse->FileName[0]))  //Find a free slot.
			{
				bFind = TRUE;
				break;
			}
			pfse ++;
		}
		if(bFind) //Find a free directory entry,no need to check further.
		{
			break;
		}
		//Can not find a free directory slot,try to search next cluster.
		if(!GetNextCluster(pFat32Fs,&dwNextCluster))
		{
			PrintLine("  In CreateDirEntry,Condition 4");
			goto __TERMINAL;
		}
	}
	if(bFind)  //Has found a free directory slot.
	{
		memcpy((char*)pfse,(const char*)pDirEntry,sizeof(__FAT32_SHORTENTRY));
		if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))
		{
			PrintLine("  In CreateDirEntry,Condition 5");
			goto __TERMINAL;
		}
	}
	else       //Can not find a free slot,allocate a new cluster for parent directory.
	{
		if(!AppendClusterToChain(pFat32Fs,&dwCurrCluster))
		{
			PrintLine("  In CreateDirEntry: Can not append a free cluster to this dir.");
			goto __TERMINAL;
		}
		memzero(pBuffer,pFat32Fs->dwClusterSize);
		memcpy((char*)pBuffer,(const char*)pDirEntry,sizeof(__FAT32_SHORTENTRY));
		dwSector = GetClusterSector(pFat32Fs,dwCurrCluster);
		if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))
		{
			PrintLine("  In CreateDirEntry,Condition 6");
			goto __TERMINAL;
		}
	}

	bResult = TRUE;
__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Create directory in a given uper level directory.
//Steps as follows:
// 1. One free cluster is allocated as data cluster by calling GetFreeCluster;
// 2. Initialize this cluster by calling InitDirectory;
// 3. Create the short directory entry by calling CreateDirEntry.
//
BOOL CreateFatDir(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszDirName,BYTE Attributes)
{
	DWORD               dwDirCluster = 0;
	__FAT32_SHORTENTRY  DirEntry;
	BOOL                bResult      = FALSE;

	

	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pszDirName))
	{
		goto __TERMINAL;
	}
	if(!GetFreeCluster(pFat32Fs,0,&dwDirCluster))
	{
		goto __TERMINAL;
	}

	if(!InitDirectory(pFat32Fs,dwStartCluster,dwDirCluster))
	{
		goto __TERMINAL;
	}
	//Initialize the directory entry.
	/*
	DirEntry.CreateDate       = 0;
	DirEntry.CreateTime       = 0;
	DirEntry.CreateTimeTenth  = 0;
	DirEntry.dwFileSize       = 0;
	DirEntry.FileAttributes   = FILE_ATTR_DIRECTORY;
	DirEntry.LastAccessDate   = 0;
	DirEntry.wFirstClusHi     = (WORD)(dwDirCluster >> 16);
	DirEntry.wFirstClusLow    = (WORD)dwDirCluster;
	DirEntry.WriteDate        = 0;
	DirEntry.WriteTime        = 0;
	for(i = 0;i < 11;i ++)
	{
		DirEntry.FileName[i] = pszDirName[i];
	}*/
	if(!InitShortEntry(&DirEntry,pszDirName,dwDirCluster,0,FILE_ATTR_DIRECTORY))
	{
		goto __TERMINAL;
	}

	DirEntry.CreateTimeTenth = 10;
	SetFatFileDateTime(&DirEntry,FAT32_DATETIME_CREATE|FAT32_DATETIME_WRITE);

	if(!CreateDirEntry(pFat32Fs,dwStartCluster,&DirEntry))
	{
		PrintLine("In CreateFatDir: Can not create directory entry in parent dir.");
		ReleaseCluster(pFat32Fs,dwDirCluster);
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}


//Create a new file in given directory.
BOOL CreateFatFile(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszFileName,BYTE Attributes)
{
	DWORD               dwInitCluster = 0;
	__FAT32_SHORTENTRY  DirEntry;
	BOOL                bResult       = FALSE;


	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pszFileName))
	{
		goto __TERMINAL;
	}
	//Allocate a free cluster for the new created file.
	if(!GetFreeCluster(pFat32Fs,0,&dwInitCluster))
	{
		PrintLine("In CreateFatFile: Can not get a free cluster.");
		goto __TERMINAL;
	}
	if(!InitShortEntry(&DirEntry,pszFileName,dwInitCluster,0,FILE_ATTR_ARCHIVE))
	{
		PrintLine("In CreateFatFile: Can not initialize short entry.");
		goto __TERMINAL;
	}

	DirEntry.CreateTimeTenth = 10;
	SetFatFileDateTime(&DirEntry,FAT32_DATETIME_CREATE|FAT32_DATETIME_WRITE);
		
	if(!CreateDirEntry(pFat32Fs,dwStartCluster,&DirEntry))
	{
		ReleaseCluster(pFat32Fs,dwInitCluster);
		goto __TERMINAL;
	}

	bResult = TRUE;
__TERMINAL:

	return bResult;
}

//Delete a given file in a given directory.
BOOL DeleteFatFile(__FAT32_FS* pFat32Fs,CHAR* pszFileName)
{
	__FAT32_SHORTENTRY         ShortEntry;
	__FAT32_SHORTENTRY*        pFileEntry     = NULL;
	DWORD                      dwParentClus   = 0;
	DWORD                      dwParentOffset = 0;
	DWORD                      dwStartClus    = 0;
	DWORD                      dwSector       = 0;
	BOOL                       bResult        = FALSE;
	BYTE*                      pBuffer        = NULL;

	if(!GetDirEntry(pFat32Fs,
		pszFileName,
		&ShortEntry,
		&dwParentClus,    //Parent directory's cluster where this entry resides.
		&dwParentOffset)) //Cluster offset.
	{
		goto __TERMINAL;
	}
	//Release the cluster chain this file occupies.
	dwStartClus =  (DWORD)(ShortEntry.wFirstClusHi) << 16;
	dwStartClus += (DWORD)ShortEntry.wFirstClusLow;
	ReleaseClusterChain(pFat32Fs,dwStartClus);
	//Remove the directory entry in it's parent directory.
	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwClusterSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	dwSector = GetClusterSector(pFat32Fs,dwParentClus);
	if(0 == dwSector)
	{
		goto __TERMINAL;
	}
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pBuffer))
	{
		goto __TERMINAL;
	}
	pFileEntry = (__FAT32_SHORTENTRY*)(pBuffer + dwParentOffset);
	memzero(pFileEntry,sizeof(__FAT32_SHORTENTRY));
	pFileEntry->FileName[0] = (CHAR)0xE5;   //Empty this short entry.
	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pBuffer))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Delete a subdirectory in a given directory.
BOOL DeleteFatDir(__FAT32_FS* pFat32Fs,CHAR* pszFileName)
{
	__FAT32_SHORTENTRY         ShortEntry;
	__FAT32_SHORTENTRY*        pFileEntry     = NULL;
	DWORD                      dwParentClus   = 0;
	DWORD                      dwParentOffset = 0;
	DWORD                      dwStartClus    = 0;
	DWORD                      dwSector       = 0;
	BOOL                       bResult        = FALSE;
	BYTE*                      pBuffer        = NULL;

	if(!GetDirEntry(pFat32Fs,
		pszFileName,
		&ShortEntry,
		&dwParentClus,    //Parent directory's cluster where this entry resides.
		&dwParentOffset)) //Cluster offset.
	{
		goto __TERMINAL;
	}
	if(0 == (FILE_ATTR_DIRECTORY & ShortEntry.FileAttributes))  //Not a directory.
	{
		goto __TERMINAL;
	}
	//Remove the directory entry in it's parent directory.
	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->dwClusterSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	dwSector = GetClusterSector(pFat32Fs,dwParentClus);
	if(0 == dwSector)
	{
		goto __TERMINAL;
	}
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pBuffer))
	{
		goto __TERMINAL;
	}
	pFileEntry = (__FAT32_SHORTENTRY*)(pBuffer + dwParentOffset);
	memzero(pFileEntry,sizeof(__FAT32_SHORTENTRY));
	pFileEntry->FileName[0] = (CHAR)0xE5;   //Empty this short entry.
	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pBuffer))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Get the file's directory entry given it's name.
//  @pFat32Fs        : File system extension object;
//  @dwStartCluster  : Start cluster of the target directory;
//  @pFileName       : File name to find,with extension;
//  @pShortEntry     : Contain the result if successfully.
//  @pDirClus[OUT]   : Returns the parent directory's start cluster.
//  @pDirOffset[OUT] : Returns the file entry's offset in parent.
//Or else FALSE will be returned.
BOOL GetShortEntry(__FAT32_FS* pFat32Fs,
				   DWORD dwStartCluster,
				   CHAR* pFileName,
				   __FAT32_SHORTENTRY* pShortEntry,
				   DWORD* pDirClus,
				   DWORD* pDirOffset)
{
	BOOL                bResult      = FALSE;
	__FAT32_SHORTENTRY* pfse         = NULL;
	BYTE*               pBuffer      = NULL;
	DWORD               dwCurrClus   = 0;
	DWORD               dwSector     = 0;
	BYTE                FileName[13];
	int                 i;

	if((NULL == pFat32Fs) || (NULL == pFileName) || (pShortEntry == pfse))
	{
		goto __TERMINAL;
	}
	//Create local buffer to contain one cluster.
	pBuffer = (BYTE*)KMemAlloc(pFat32Fs->SectorPerClus * pFat32Fs->dwBytePerSector,KMEM_SIZE_TYPE_ANY);
	if(NULL == pBuffer)
	{
		PrintLine("  In GetShortEntry: Can not allocate kernel memory.");
		goto __TERMINAL;
	}
	dwCurrClus = dwStartCluster;
	while(!IS_EOC(dwCurrClus))  //Main loop to check the root directory.
	{
		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)  //Fatal error.
		{
			PrintLine("  In GetShortEntry: Can not get cluster sector.");
			goto __TERMINAL;
		}
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))  //Can not read the appropriate sector(s).
		{
			PrintLine("  In GetShortEntry: Can not read sector from device.");
			goto __TERMINAL;
		}
		//Now check the root directory to seek the volume ID entry.
		pfse = (__FAT32_SHORTENTRY*)pBuffer;
		for(i = 0;i < pFat32Fs->SectorPerClus * 16;i ++)
		{
			if(0xE5 == (BYTE)pfse->FileName[0])  //Empty entry.
			{
				pfse += 1;  //Seek to the next entry.
				continue;
			}
			if(0 == pfse->FileName[0])     //All rest part is zero,no need to check futher.
			{
				break;
			}
			if(FILE_ATTR_LONGNAME == pfse->FileAttributes)  //Long file name entry.
			{
				pfse += 1;
				continue;
			}
			if(FILE_ATTR_VOLUMEID & pfse->FileAttributes)   //Volume label entry.
			{
				pfse += 1;
				continue;
			}
			if(ConvertName(pfse,FileName))  //Can not convert to regular file name string.
			{
				if(StrCmp((CHAR*)pFileName,(CHAR*)&FileName[0]))  //Found.
				{
					memcpy((char*)pShortEntry,(const char*)pfse,sizeof(__FAT32_SHORTENTRY));
					if(pDirClus)
					{
						*pDirClus = dwCurrClus;
					}
					if(pDirOffset)
					{
						*pDirOffset = (BYTE*)pfse - pBuffer;
					}
					bResult = TRUE;
					goto __TERMINAL;
				}
			}
			pfse += 1;
		}
		if(!GetNextCluster(pFat32Fs,&dwCurrClus))
		{
			break;
		}
	}
__TERMINAL:
	if(pBuffer)
	{
		KMemFree(pBuffer,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//Get the directory entry of a full file name.
//The difference between this routine and GetShortEntry is,the last one only
//search the directory designated by start cluster.
//But this one will search the whole file system tree to seek the target.
BOOL GetDirEntry(__FAT32_FS* pFat32Fs,
				 CHAR* pFullName,
				 __FAT32_SHORTENTRY* pfse,
				 DWORD* pDirClus,
				 DWORD* pDirOffset)
{
	BOOL                 bResult            = FALSE;
	DWORD                dwLevel            = 0;
	int                  i;
	BYTE*                pBuffer            = 0;
	DWORD                dwStartClus        = 0;      //Start cluster of current directory to search.
	DWORD                dwSector           = 0;
	__FAT32_SHORTENTRY   ShortEntry         = {0};
	CHAR                 SubDir[MAX_FILE_NAME_LEN];
	CHAR                 buffer[13];

	if((NULL == pFat32Fs) || (NULL == pFullName) || (NULL == pfse))
	{
		goto __TERMINAL;
	}
	if(!NameIsValid(pFullName))  //Is not a valid full file name.
	{
		goto __TERMINAL;
	}
	if(!GetFullNameLevel(pFullName,&dwLevel))
	{
		PrintLine("  In GetDirEntry: GetFullNameLevel failed.");
		goto __TERMINAL;
	}
	i = 1;
	dwStartClus = pFat32Fs->dwRootDirClusStart;
	//Initialize the short entry as root directory.
	ShortEntry.FileAttributes = FILE_ATTR_DIRECTORY;
	ShortEntry.wFirstClusHi   = (WORD)(dwStartClus >> 16);
	ShortEntry.wFirstClusLow  = (WORD)dwStartClus;

	while(dwLevel)
	{
		if(!GetSubDirectory(pFullName,i,SubDir))  //Get the sub-directory.
		{
			PrintLine("  In GetDirEntry: GetSubDirectory failed.");
			goto __TERMINAL;
		}
		if(!GetShortEntry(pFat32Fs,dwStartClus,SubDir,&ShortEntry,pDirClus,pDirOffset))
		{
			/*
			PrintLine("In GetDirEntry: GetShortEntry failed.");
			sprintf(Buffer,"  Parameters: start clus = %d,SubDir = %s",dwStartClus,SubDir);
			PrintLine(Buffer);*/
			goto __TERMINAL;
		}
		dwStartClus = (DWORD)ShortEntry.wFirstClusHi;
		dwStartClus = dwStartClus << 16;
		dwStartClus = dwStartClus + (DWORD)ShortEntry.wFirstClusLow;
		dwLevel --;
		i ++;
	}
	if(!GetPathName(pFullName,SubDir,buffer))
	{
		PrintLine("  In GetDirEntry: GetPathName failed.");
		goto __TERMINAL;
	}
	if(0 == buffer[0])  //The target is a directory.
	{
		memcpy((char*)pfse,(const char*)&ShortEntry,sizeof(__FAT32_SHORTENTRY));
		bResult = TRUE;
		goto __TERMINAL;
	}
	if(!GetShortEntry(pFat32Fs,dwStartClus,buffer,&ShortEntry,pDirClus,pDirOffset))
	{
		/*
		PrintLine("  In GetDirEntry: GetShortEntry failed,next one.");
		sprintf(Buffer,"  Parameters: start clus = %d,SubDir = %s",dwStartClus,SubDir);
		PrintLine(Buffer);*/
		goto __TERMINAL;
	}
	//memcpy(pfse,&ShortEntry,sizeof(__FAT32_SHORTENTRY));
	memcpy((char*)pfse,(const char*)&ShortEntry,sizeof(__FAT32_SHORTENTRY));
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

#endif
