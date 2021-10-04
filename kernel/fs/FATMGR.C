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

#include <StdAfx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fsstr.h"
#include "fat32.h"

#ifdef __CFG_SYS_DDF

#define  FILENAME_TYPE_SHORT      0
#define  FILENAME_TYPE_LONG       1
#define  FILENAME_TYPE_FAT_SHORT  2

VOID* FatMem_Alloc(INT nSize)
{
	VOID* p = KMemAlloc(nSize, KMEM_SIZE_TYPE_ANY);

	if (p)
	{
		memset(p, 0, nSize);
	}
	return p;
}

VOID FatMem_Free(VOID* p)
{
	if(p)
	{
		KMemFree(p,KMEM_SIZE_TYPE_ANY,0);
	}
}

INT  GetFileExtLen(CHAR* pFileName)
{
	INT    nExtLen   = 0;
	CHAR*  pDotPos   = strstr(pFileName,".");

	if(pDotPos)
	{
		pDotPos ++;
		nExtLen = strlen(pDotPos);
	}

	return nExtLen;
}

//Convert file name to  fat32 format
BOOL ConvertFatName(CHAR* pSrc,CHAR* pShortName)
{
	BOOL            bResult    = FALSE;
	CHAR*           pDotPos    = NULL;
	CHAR*           pStart     = NULL;
	INT             nNameLen   = strlen(pSrc);
	INT             nExtLen    = GetFileExtLen(pSrc);
	INT             i,j        = 0;
	
	if((NULL == pSrc) || nNameLen <= 0|| (NULL == pShortName))  //Invalid parameters.
	{		
		return FALSE;
	}

	//if file name is long.don't need to convert
	if(nNameLen > FAT32_SHORTDIR_FILENAME_LEN || nExtLen > FAT32_STANDARDEXT_NAME_LEN )
	{
		if(!strstr(pSrc,"~"))
		{
			strcpy(pShortName,pSrc);
			return TRUE;
		}
	}

	//Now convert the name to directory format.	
	pDotPos  = pSrc + (nNameLen - 1);
	while((*pDotPos != '.') && (pDotPos != pSrc))
	{
		pDotPos --;
	}
	if((pDotPos == pSrc) && (*pDotPos == '.')) //First character is dot,invalid.
	{
		goto __TERMINAL;
	}
	if(pDotPos == pSrc)  //Without extension.
	{
		pDotPos = pSrc + nNameLen;
	}
	i = 0;
	pStart = pSrc;
	while(pStart != pDotPos)  //Get the name's part.
	{
		pShortName[i] = *pStart;
		i ++;
		pStart ++;
		if(i == 8)
		{
			break;
		}
	}
	while(i < 8)  //Fill space.
	{
		pShortName[i] = ' '; //0x20.
		i ++;
	}
	//Process the extension.
	pStart = pSrc + (nNameLen - 1);  //Now pStart pointing to the tail of name.
	i = 10;
	while((pStart > pDotPos) && (i > 7))
	{
		pShortName[i] = *pStart;
		i --;
		pStart --;
	}
	while(i > 7)
	{
		pShortName[i --] = ' ';
	}
	
	ToCapital(pShortName);

__TERMINAL:
	return TRUE;
}

BYTE GetSumOfSFN (const BYTE *dir )
{
	BYTE sum = 0;
	UINT n = FAT32_SHORTDIR_FILENAME_LEN;
	do {
		sum = (sum >> 1) + (sum << 7) + *dir++;
	} while (--n);
	return sum;
}

VOID InitLongFileName(BYTE* pDstLongName,CHAR* pSrcLongName,INT nNameLen)
{
	INT  i,j;

	for(i=0;i<nNameLen;i++)
	{
		if(pSrcLongName[i] != 0)
		{
			pDstLongName[i*2] = pSrcLongName[i];
		}
		else
		{
			pDstLongName[i*2]   = 0;
			pDstLongName[i*2+1] = 0;

			for(j=i+1;j<nNameLen;j++)
			{
				pDstLongName[j*2]   = 0xFF;
				pDstLongName[j*2+1] = 0xFF;
			}
			break;
		}
	}

}

VOID  InitLongEntry(__FAT32_LONGENTRY* pLongEntry,INT nLongEntryNum,CHAR* pszName,BYTE bSumofSfn)
{
	INT    nNameLen = strlen(pszName);
	INT    nExtLen  = GetFileExtLen(pszName);
	INT    i        = 0;

	//not need long dir
	if(nNameLen <= FAT32_SHORTDIR_FILENAME_LEN && nExtLen <= FAT32_STANDARDEXT_NAME_LEN)
	{
		return ;
	}

	for(i = nLongEntryNum-1;i >= 0;i --)
	{
		__FAT32_LONGENTRY*  pTempEntry                               = &pLongEntry[nLongEntryNum-i-1];
		CHAR              szPartName[FAT32_LONGDIR_FILENAME_LEN+1] = {0};

		memcpy(szPartName,&pszName[i*FAT32_LONGDIR_FILENAME_LEN],FAT32_LONGDIR_FILENAME_LEN);

		pTempEntry->LongFlage  = 0x0F;  //long entry flage
		if(i == nLongEntryNum-1)
		{			
			pTempEntry->LongNum[0] = i+1|0x40;
		}
		else
		{
			pTempEntry->LongNum[0] = i+1;
		}

		pTempEntry->Checksum = bSumofSfn;

		InitLongFileName(pTempEntry->szName1,&szPartName[0],5);
		InitLongFileName(pTempEntry->szName2,&szPartName[5],6);
		InitLongFileName(pTempEntry->szName3,&szPartName[11],2);			
	}
}

VOID  MakeShortEntryName(CHAR* pShortName,CHAR* pLongName,INT nNum)
{
	CHAR*  pDotPos   = strstr(pLongName,".");
	INT    nNameLen  = strlen(pLongName);
	CHAR   szNum[32] = {0};


	if(nNameLen > FAT32_SHORTDIR_FILENAME_LEN)
	{
		INT  nShortNameLen = FAT32_SHORTDIR_PREFIX_LEN;
		INT  nSpaceLen     = 0;

		if(pDotPos)
		{
			nShortNameLen = pDotPos - pLongName;
			nShortNameLen = (nShortNameLen > FAT32_SHORTDIR_PREFIX_LEN)?FAT32_SHORTDIR_PREFIX_LEN:nShortNameLen;
		}

		memcpy(pShortName,pLongName,nShortNameLen);
		sprintf(szNum,"~%d",nNum);
		strcat(pShortName,szNum);

		AddSpace(pShortName,FAT32_SHORTDIR_PREFIX_LEN-nShortNameLen);  
	}
	else
	{
		INT  nNameLen = pDotPos-pLongName;
		
		memcpy(pShortName,pLongName,nNameLen);
		if(nNameLen <= 2)
		{			
			sprintf(szNum,"EDFD~%d",nNum);
		}
		else
		{
			sprintf(szNum,"~%d",nNum);
		}
		strcat(pShortName,szNum);

		//fill left space  use 
		AddSpace(pShortName, 8-strlen(pShortName));  		
	}

	if(pDotPos)
	{
		pDotPos ++;		
		memcpy(pShortName+strlen(pShortName),pDotPos,FAT32_STANDARDEXT_NAME_LEN);		
	}
	else
	{  
		// fill 3 space		
		AddSpace(pShortName, 3);  
	}
	ToCapital(pShortName);
}

BOOL IsLongFileName(CHAR* pDirFileName)
{
	BOOL bLongFileName = FALSE;

	if(strlen(pDirFileName) > FAT32_SHORTDIR_FILENAME_LEN)
	{
		bLongFileName= TRUE;
	}
	else
	{
		CHAR*  pDotPos   = strstr(pDirFileName,".");
		if(pDotPos)
		{
			pDotPos ++;
			bLongFileName = (strlen(pDotPos) > FAT32_STANDARDEXT_NAME_LEN) ? TRUE : FALSE;
		}
	}
	return bLongFileName;
}

VOID ConvertLongEntry(__FAT32_LONGENTRY* plongEntry,CHAR* pFileName)
{	
	CHAR  szBuf[32] = {0};
	BYTE  nValue    = 0;
	INT   i         = 0;

	for(i=0;i<5;i++)
	{	
		nValue = plongEntry->szName1[i*2];
		if(nValue!= 0 && nValue != 0xFF)
		{
			szBuf[i] = nValue;
		}		
	}
	strcat(pFileName,szBuf);
	memset(szBuf,0,sizeof(szBuf));

	for(i=0;i<FAT32_SHORTDIR_PREFIX_LEN;i++)
	{
		nValue = plongEntry->szName2[i*2];
		if(nValue!= 0 && nValue != 0xFF)
		{
			szBuf[i] = nValue;
		}		
	}
	strcat(pFileName,szBuf);
	memset(szBuf,0,sizeof(szBuf));

	for(i=0;i<2;i++)
	{
		nValue = plongEntry->szName3[i*2];
		if(nValue!= 0 && nValue != 0xFF)
		{
			szBuf[i] = nValue;
		}		
	}
	strcat(pFileName,szBuf);
}

VOID CombinLongFileName(__FAT32_LONGENTRY** plongEntry,INT nLongEntryNum, CHAR* pFileFullName)
{	
	INT  i = 0;

	for(i = nLongEntryNum-1;i >= 0 ;i--)
	{
		CHAR szTemp[32]  = {0};

		ConvertLongEntry(plongEntry[i],szTemp);

		ToCapital(szTemp);
		strcat(pFileFullName,szTemp);				
	}	
}

BOOL DirEntryIsExist(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pEntryName,BOOL bLongName)
{
	__FAT32_SHORTENTRY* pfse         = NULL;
	BOOL                bResult      = FALSE;	
	BYTE*               pBuffer      = NULL;
	DWORD               dwCurrClus   = 0;
	DWORD               dwSector     = 0;	
	int                 i            = 0;

	if((NULL == pFat32Fs) || (NULL == pEntryName) )
	{
		goto __TERMINAL;
	}
	
	//Create local buffer to contain one cluster.
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->SectorPerClus * pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	dwCurrClus = dwStartCluster;
	while(!IS_EOC(dwCurrClus))  //Main loop to check the root directory.
	{
		__FAT32_LONGENTRY*  szLongEntry[64]     = {0};
		INT                 nLongEntryNum       = 0;	

		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)  //Fatal error.
		{
			goto __TERMINAL;
		}
		if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pBuffer))  //Can not read the appropriate sector(s).
		{
			goto __TERMINAL;
		}
		//Now check the root directory to seek the volume ID entry.
		pfse = (__FAT32_SHORTENTRY*)pBuffer;
		for(i = 0;i < pFat32Fs->SectorPerClus * 16;i ++)
		{
			CHAR      szFileName[MAX_FILE_NAME_LEN] = {0};

			if(0xE5 == (BYTE)pfse->FileName[0])  //Empty entry.
			{
				pfse += 1;						//Seek to the next entry.
				continue;
			}
			if(0 == pfse->FileName[0])     //All rest part is zero,no need to check futher.
			{
				break;
			}
			if(FILE_ATTR_LONGNAME == pfse->FileAttributes)  //Long file name entry.
			{
				szLongEntry[nLongEntryNum ++ ] =  (__FAT32_LONGENTRY*)pfse;
				pfse += 1;				

				continue;
			}

			if(FILE_ATTR_VOLUMEID & pfse->FileAttributes)   //Volume label entry.
			{
				pfse += 1;
				continue;
			}

			if(bLongName)
			{
				CombinLongFileName(szLongEntry,nLongEntryNum,szFileName);
				nLongEntryNum  = 0;
			}			
			else
			{
				memcpy(szFileName,pfse->FileName,sizeof(pfse->FileName));				
			}

			if(strcmp(pEntryName,szFileName) == 0) 
			{
				bResult = TRUE;
				goto __TERMINAL;
			}


			pfse += 1;
		}
		if(!GetNextCluster(pFat32Fs,&dwCurrClus))
		{
			break;
		}
	}
__TERMINAL:

	FatMem_Free(pBuffer);

	return bResult;
}

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

/*
 * Helper routine to check if a fat sector
 * is already loaded into fat cache, return
 * the base address if it is, NULL will be
 * returned if is not in cache.
 */
static unsigned char* __get_sector_in_cache(__FAT32_FS* pFat32Fs, uint32_t fat_sector)
{
	unsigned char* pSectorStart = NULL;

	if ((fat_sector >= pFat32Fs->fc_start_sector) &&
		(fat_sector < (pFat32Fs->fc_start_sector + pFat32Fs->fc_size / pFat32Fs->dwBytePerSector)))
	{
		/* Already loaded into cache. */
		unsigned long offset = fat_sector - pFat32Fs->fc_start_sector;
		offset *= pFat32Fs->dwBytePerSector;
		pSectorStart = pFat32Fs->pFatCache + offset;
		return pSectorStart;
	}
	/* Not in cache. */
	return NULL;
}

/*
 * Get the next cluster number in a cluster chain 
 * given the current one.
 * pdwCluster contains the current cluster,if next 
 * cluster can be fetched,TRUE will be
 * returned and pdwCluster contains the next one,
 * otherwise FALSE will be returned.
 */
BOOL GetNextCluster(__FAT32_FS* pFat32Fs, uint32_t* pdwCluster)
{
	uint32_t dwNextCluster = EOC, dwClusSector = 0;
	uint32_t dwCurrCluster = 0, dwClusOffset = 0;
	BYTE buff[SECTOR_SIZE];
	BYTE* pSectorBuff = NULL;

	/* Check the parameters. */
	BUG_ON((NULL == pFat32Fs) || (NULL == pdwCluster));
	dwCurrCluster = *pdwCluster;
	if (0 == dwCurrCluster)
	{
		dwCurrCluster = 2;
	}

	/* Must be in range of partition. */
	if (dwCurrCluster > pFat32Fs->total_clusters)
	{
		return FALSE;
	}
	/* Get the FAT's sector number contains current cluster. */
	dwClusSector = dwCurrCluster / 128;
	dwClusSector += pFat32Fs->dwFatBeginSector;
	/* Try to load the fat sector from cache. */
	pSectorBuff = __get_sector_in_cache(pFat32Fs, dwClusSector);
	if (NULL == pSectorBuff)
	{
		/* Sector not loaded yet, just load it. */
		if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwClusSector, 1, buff))
		{
			/* Failed to read sector. */
			return FALSE;
		}
		pSectorBuff = &buff[0];
	}

	dwClusOffset = (dwCurrCluster - (dwCurrCluster / 128) * 128) * sizeof(uint32_t);
	dwNextCluster = *(uint32_t*)(pSectorBuff + dwClusOffset);
	/* Mask the leading 4 bits. */
	dwNextCluster &= FAT32_CLUSTER_ID_MASK;
	if (dwNextCluster == 0)
	{
		return FALSE;
	}
	*pdwCluster = dwNextCluster;
	return TRUE;
}

/* 
 * Returns the corresponding sector number of given cluster number.
 * Return value of 0 indicates operation failed.
 */
DWORD GetClusterSector(__FAT32_FS* pFat32Fs,DWORD dwCluster)
{
	BUG_ON((NULL == pFat32Fs) || (0 == dwCluster));
	if(dwCluster < 2)
	{
		return 0;
	}
	return pFat32Fs->dwDataSectorStart + (dwCluster - 2) * pFat32Fs->SectorPerClus;
}

/* 
 * Local heler routine to calculate the 
 * FAT cache's length. 
 * If FAT's length is less than MAX_FAT_CACHE_LENGTH,
 * then returns the actual FAT length, otherwise
 * just return the MAX_FAT_CACHE_LENGTH. More complicated
 * algorithm maybe adopted in future.
 */
static unsigned long __get_fatcache_length(__FAT32_FS* pFatFs)
{
	unsigned long fat_size = pFatFs->dwFatSectorNum * pFatFs->dwBytePerSector;

	BUG_ON(MAX_FAT_CACHE_LENGTH % pFatFs->dwBytePerSector);

	if (fat_size < MAX_FAT_CACHE_LENGTH)
	{
		return fat_size;
	}
	return MAX_FAT_CACHE_LENGTH;
}

/* 
 * Get one free cluster and mark the cluster as used.
 * If find,TRUE will be returned and the cluster number 
 * will be set in pdwFreeCluster, otherwise FALSE will 
 * be returned without any changing to pdwFreeCluster.
 */
BOOL GetFreeCluster(__FAT32_FS* pFat32Fs, DWORD dwStartToFind, DWORD* pdwFreeCluster)
{
	unsigned long dwCurrCluster = 0, dwSector = 0;
	uint32_t* pCluster = NULL;
	unsigned char* pFatSectorBuff = NULL;
	BOOL bResult = FALSE;
	unsigned long i = 0, round = 0;

	/* Init fat cache if not yet. */
	if (NULL == pFat32Fs->pFatCache)
	{
		unsigned long fc_size = __get_fatcache_length(pFat32Fs);
		pFat32Fs->pFatCache = (unsigned char*)_hx_malloc(fc_size);
		if (NULL == pFat32Fs->pFatCache)
		{
			_hx_printf("[%s]out of memory.\r\n", __func__);
			goto __TERMINAL;
		}
		pFat32Fs->fc_size = fc_size;
		pFat32Fs->fc_dirty = FALSE;
		pFat32Fs->fc_start_sector = 0;
	}

	dwSector = pFat32Fs->dwFatBeginSector;
	while (dwSector < pFat32Fs->dwFatSectorNum + pFat32Fs->dwFatBeginSector)
	{
		if (dwSector != pFat32Fs->fc_start_sector)
		{
			/* Should reload fat cache. */
			if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
				dwSector, pFat32Fs->fc_size / pFat32Fs->dwBytePerSector, 
				pFat32Fs->pFatCache))
			{
				/* Read fat fail. */
				_hx_printf("[%s]load fat cache fail.\r\n", __func__);
				goto __TERMINAL;
			}
			/* Update the fat cache variables. */
			pFat32Fs->fc_start_sector = dwSector;
			pFat32Fs->fc_dirty = FALSE;
		}
		/* Search the whole fat cache to find a free cluster. */
		pFatSectorBuff = pFat32Fs->pFatCache;
		unsigned long total_cache_sectors = pFat32Fs->fc_size / pFat32Fs->dwBytePerSector;
		unsigned long fat_sector = 0;
		//for (fat_sector = 0; fat_sector < total_cache_sectors; fat_sector++)
		while(fat_sector < total_cache_sectors)
		{
			pCluster = (uint32_t*)pFatSectorBuff;
			/* Analyze one sector to find a free cluster entry. */
			for (i = 0; i < pFat32Fs->dwBytePerSector / sizeof(uint32_t); i++)
			{
				if (0 == ((*pCluster) & FAT32_CLUSTER_ID_MASK))
				{
					/* Find one, mark as used. */
					(*pCluster) |= FAT32_CLUSTER_ID_MASK;
					/* Mark the whole cache as dirty. */
					pFat32Fs->fc_dirty = TRUE;
					if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
						dwSector + fat_sector, 1,
						pFatSectorBuff))
					{
						goto __TERMINAL;
					}
					else {
						/* Mark back the dirty flag. */
						pFat32Fs->fc_dirty = FALSE;
						//_hx_printf("[%s]fat sector[%d] written.\r\n", __func__,
						//	dwSector + fat_sector);
					}
					/* Write to backup FAT region. */
					WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
						dwSector + fat_sector + pFat32Fs->dwFatSectorNum, 
						1, pFatSectorBuff);
					bResult = TRUE;
					goto __TERMINAL;
				}
				pCluster++;
				dwCurrCluster++;
			}
			/* Next sector in fat. */
			pFatSectorBuff += pFat32Fs->dwBytePerSector;
			fat_sector++;
		}
		/* Skip the searched fat cache sectors. */
		dwSector += (pFat32Fs->fc_size / pFat32Fs->dwBytePerSector);
		/* Update iteration round. */
		round++;
	}

__TERMINAL:

	/* Update performance metric counter. */
	if (pFat32Fs->perf_metric.max_getfreeclus_round < round)
	{
		pFat32Fs->perf_metric.max_getfreeclus_round = round;
	}

	if (bResult)
	{
		/* Return the free clustor number. */
		*pdwFreeCluster = dwCurrCluster;
	}
	else {
		_hx_printf("[%s]get free cluster fail.\r\n", __func__);
	}
	return bResult;
}

/* Old one. */
BOOL __GetFreeCluster(__FAT32_FS* pFat32Fs, DWORD dwStartToFind, DWORD* pdwFreeCluster)
{
	unsigned long dwCurrCluster = 0, dwSector = 0;
	BYTE* pBuffer = NULL;
	DWORD*  pCluster = NULL;
	BOOL bResult = FALSE;
	unsigned long i = 0, round = 0;

	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwBytePerSector);
	if (NULL == pBuffer)
	{
		return FALSE;
	}

	dwSector = pFat32Fs->dwFatBeginSector;
	while (dwSector < pFat32Fs->dwFatSectorNum + pFat32Fs->dwFatBeginSector)
	{
		if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector, 1, pBuffer))
		{
			/* Read fat fail. */
			goto __TERMINAL;
		}
		pCluster = (DWORD*)pBuffer;
		/* Analyze the sector to find a free cluster entry. */
		for (i = 0; i < pFat32Fs->dwBytePerSector / sizeof(DWORD); i++)
		{
			if (0 == ((*pCluster) & FAT32_CLUSTER_ID_MASK))
			{
				/* Find one, mark as used. */
				(*pCluster) |= FAT32_CLUSTER_ID_MASK;
				if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
					dwSector, 1,
					pBuffer))
				{
					goto __TERMINAL;
				}
				/* Write to backup FAT region. */
				WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
					dwSector + pFat32Fs->dwFatSectorNum, 1,
					pBuffer);
				bResult = TRUE;
				goto __TERMINAL;
			}
			pCluster++;
			dwCurrCluster++;
		}
		/* Next sector in fat. */
		dwSector++;
		/* Update iteration round. */
		round++;
	}
__TERMINAL:

	FatMem_Free(pBuffer);
	/* Update performance metric counter. */
	if (pFat32Fs->perf_metric.max_getfreeclus_round < round)
	{
		pFat32Fs->perf_metric.max_getfreeclus_round = round;
	}

	if (bResult)
	{
		/* Return the free clustor number. */
		*pdwFreeCluster = dwCurrCluster;
	}
	return bResult;
}

/* Release one used cluster. */
BOOL ReleaseCluster(__FAT32_FS* pFat32Fs, DWORD dwCluster)
{
	BYTE*         pBuffer = NULL;
	DWORD         dwSector = 0;
	DWORD         dwOffset = 0;
	BOOL          bResult = FALSE;

	if ((NULL == pFat32Fs) || (dwCluster < 2) || IS_EOC(dwCluster))
	{
		goto __TERMINAL;
	}
	dwSector = dwCluster / 128;
	if (dwSector > pFat32Fs->dwFatSectorNum)
	{
		goto __TERMINAL;
	}
	dwSector += pFat32Fs->dwFatBeginSector;
	dwOffset = (dwCluster - (dwCluster / 128) * 128) * sizeof(DWORD);

	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwBytePerSector);
	if (NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	//Read the fat sector where this cluster's index resides,modify to zero and write it back.
	if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		goto __TERMINAL;
	}
	*(DWORD*)(pBuffer + dwOffset) &= 0xF0000000;
	if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		1,
		pBuffer))
	{
		goto __TERMINAL;
	}
	//All successfully.
	bResult = TRUE;
__TERMINAL:

	FatMem_Free(pBuffer);

	return bResult;
}

/* Append one free cluster to the tail of 
 * a cluster chain.
 * The pdwCurrCluster contains the last cluster 
 * of a cluster chain, returns TRUE if success and
 * pdwCurrCluster contains the cluster number value 
 * appended to chain right now. Otherwise FALSE will 
 * be returned and the pdwCurrCluster keep unchanged.
 */
BOOL AppendClusterToChain(__FAT32_FS* pFat32Fs, DWORD* pdwCurrCluster)
{
	uint32_t dwCurrCluster = 0, dwNextCluster = 0;
	uint32_t dwSector = 0, dwOffset = 0, dwEndCluster = 0;
	BOOL bResult = FALSE, bAlloc = FALSE;
	BYTE* pBuffer = NULL;

	BUG_ON((NULL == pFat32Fs) || (NULL == pdwCurrCluster));
	dwCurrCluster = *pdwCurrCluster;
	if ((2 > dwCurrCluster) || IS_EOC(dwCurrCluster))
	{
		/* Invalid cluster value. */
		goto __TERMINAL;
	}

	/* 128 cluster entries per sector. */
	dwSector = dwCurrCluster / 128;
	if (dwSector > pFat32Fs->dwFatSectorNum)
	{
		/* Exceed the FAT size. */
		goto __TERMINAL;
	}
	/* Get physical sector number of dwCurrCluster in fat. */
	dwSector += pFat32Fs->dwFatBeginSector;
	/* Get current cluster's offset in sector. */
	dwOffset = (dwCurrCluster - (dwCurrCluster / 128) * 128) * sizeof(uint32_t);

	/* Try to get a free cluster. */
	if (!GetFreeCluster(pFat32Fs, 0, &dwNextCluster))
	{
		goto __TERMINAL;
	}

	pBuffer = __get_sector_in_cache(pFat32Fs, dwSector);
	if (!pBuffer)
	{
		/* 
		 * The corresponding fat sector to dwSector is not
		 * loaded into cache yet, so just load the fat sector
		 * using temporary sector buffer. 
		 */
		pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwBytePerSector);
		if (NULL == pBuffer)
		{
			goto __TERMINAL;
		}
		bAlloc = TRUE;

		/* Load the fat sector. */
		if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector, 1,
			pBuffer))
		{
			_hx_printf("[%s]read sector error\r\n", __func__);
			goto __TERMINAL;
		}
	}
	/* Save the next cluster to chain. */
	*(uint32_t*)(pBuffer + dwOffset) &= (~FAT32_CLUSTER_ID_MASK); /* Keep leading bits. */
	*(uint32_t*)(pBuffer + dwOffset) += (dwNextCluster & FAT32_CLUSTER_ID_MASK);
	if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector, 1, pBuffer))
	{
		/* Write fail, release the reserved cluster. */
		ReleaseCluster(pFat32Fs, dwNextCluster);
		_hx_printf("[%s]write sector fail.\r\n", __func__);
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:

	if (bAlloc)
	{
		/* Should release the temporary fat buffer. */
		FatMem_Free(pBuffer);
	}
	if (bResult)
	{
		/* Return the next cluster number. */
		*pdwCurrCluster = (dwNextCluster & FAT32_CLUSTER_ID_MASK);
	}
	return bResult;
}

/* old one. */
BOOL __AppendClusterToChain(__FAT32_FS* pFat32Fs, DWORD* pdwCurrCluster)
{
	uint32_t dwCurrCluster = 0, dwNextCluster = 0;
	uint32_t dwSector = 0, dwOffset = 0, dwEndCluster = 0;
	BOOL bResult = FALSE;
	BYTE* pBuffer = NULL;

	BUG_ON((NULL == pFat32Fs) || (NULL == pdwCurrCluster));
	dwCurrCluster = *pdwCurrCluster;
	if ((2 > dwCurrCluster) || IS_EOC(dwCurrCluster))
	{
		/* Invalid cluster value. */
		goto __TERMINAL;
	}

	/* 128 cluster entries per sector. */
	dwSector = dwCurrCluster / 128;
	if (dwSector > pFat32Fs->dwFatSectorNum)
	{
		/* Exceed the FAT size. */
		goto __TERMINAL;
	}
	/* Get physical sector number of dwCurrCluster in fat. */
	dwSector += pFat32Fs->dwFatBeginSector;
	/* Get current cluster's offset in sector. */
	dwOffset = (dwCurrCluster - (dwCurrCluster / 128) * 128) * sizeof(uint32_t);

	/* Temporary sector buffer. */
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwBytePerSector);
	if (NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	/* Try to get a free cluster. */
	if (!GetFreeCluster(pFat32Fs, 0, &dwNextCluster))
	{
		goto __TERMINAL;
	}

	/*
	 * The following invoking must behind GetFreeCluster
	 * routine above,because GetFreeCluster routine will
	 * modify the content of FAT, and this change must
	 * be taken before the following read.
	 * One complicated problem has lead by this reason.
	 */
	if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector, 1,
		pBuffer))
	{
		_hx_printf("[%s]read sector error\r\n", __func__);
		goto __TERMINAL;
	}
	/* Save the next cluster to chain. */
	*(DWORD*)(pBuffer + dwOffset) &= (~FAT32_CLUSTER_ID_MASK); /* Keep leading 4 bits. */
	*(DWORD*)(pBuffer + dwOffset) += (dwNextCluster & FAT32_CLUSTER_ID_MASK);
	if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector, 1, pBuffer))
	{
		/* Write fail, release the reserved cluster. */
		ReleaseCluster(pFat32Fs, dwNextCluster);
		_hx_printf("[%s]write sector fail.\r\n", __func__);
		goto __TERMINAL;
	}
	dwEndCluster = *(DWORD*)(pBuffer + dwOffset);
	if (!GetNextCluster(pFat32Fs, &dwEndCluster))
	{
		_hx_printf("[%s]can not get next cluster.\r\n", __func__);
	}
	bResult = TRUE;

__TERMINAL:

	FatMem_Free(pBuffer);
	if (bResult)
	{
		/* Return the next cluster number. */
		*pdwCurrCluster = (dwNextCluster & FAT32_CLUSTER_ID_MASK);
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
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
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
	FatMem_Free(pBuffer);

	return bResult;
}

/*
 * Find one empty short directory entry in a cluster chain start from dwStartCluster,
 * and save the short entry pointed by pfse into this entry.If can not find a free one
 * in the whole cluster chain,then append a free cluster in the chain and save it.
 */
BOOL CreateDirEntry(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,__FAT32_LONGENTRY* pDirEntry,INT nEntryNum)
{
	__FAT32_SHORTENTRY*    pfse          = NULL;
	DWORD                  dwSector      = 0;
	DWORD                  dwCurrCluster = 0;
	DWORD                  dwNextCluster = 0;
	BYTE*                  pBuffer       = NULL;
	CHAR                   DirName[13]   = {0};	
	BOOL                   bFind         = FALSE;
	BOOL                   bResult       = FALSE;
	DWORD                  i;

	/* Parameters checking. */
	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pDirEntry))
	{
		goto __TERMINAL;
	}

	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}

	/*
	 * Try to find a free directory entry in the given directory,if can not find,then
	 * allocate a free cluster,append it to the given directory.
	 */
	dwNextCluster = dwStartCluster;
	while(!IS_EOC(dwNextCluster))
	{
		dwCurrCluster = dwNextCluster;
		dwSector = GetClusterSector(pFat32Fs,dwCurrCluster);
		if(0 == dwSector)
		{			
			goto __TERMINAL;
		}
		if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector, pFat32Fs->SectorPerClus,pBuffer))
		{
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
		//Find a free directory entry,no need to check further.
		if(bFind)
		{
			break;
		}
		//Can not find a free directory slot,try to search next cluster.
		if(!GetNextCluster(pFat32Fs,&dwNextCluster))
		{			
			goto __TERMINAL;
		}
	}

	//Has found a free directory slot.
	if(bFind)
	{
		memcpy(pfse, pDirEntry, sizeof(__FAT32_SHORTENTRY) * nEntryNum);
		if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,
			pFat32Fs->SectorPerClus,pBuffer))
		{
			_hx_printf("%s:failed to write sector[lc = %d].\r\n", __func__, __LINE__);
			goto __TERMINAL;
		}
	}
	//Can not find a free slot,allocate a new cluster for parent directory.
	else
	{
		if(!AppendClusterToChain(pFat32Fs,&dwCurrCluster))
		{
			_hx_printf("%s:failed to append cluster to chain.\r\n", __func__);
			goto __TERMINAL;
		}
		memset(pBuffer,0,pFat32Fs->dwClusterSize);
		memcpy((char*)pBuffer,(const char*)pDirEntry,sizeof(__FAT32_SHORTENTRY)*nEntryNum);
		dwSector = GetClusterSector(pFat32Fs,dwCurrCluster);

		if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,
			pFat32Fs->SectorPerClus,pBuffer))
		{
			_hx_printf("%s:failed to write sector[lc = %d].\r\n", __func__, __LINE__);
			goto __TERMINAL;
		}
	}
	bResult = TRUE;

__TERMINAL:
	FatMem_Free(pBuffer);
	return bResult;
}

/*
 * Create directory in a given uper level directory.
 * Steps as follows:
 *  1. One free cluster is allocated as data cluster by calling GetFreeCluster;
 *  2. Initialize this cluster by calling InitDirectory;
 *  3. Create the short directory entry by calling CreateDirEntry.
 */
BOOL CreateFatDir(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszDirName,BYTE Attributes)
{
	__FAT32_SHORTENTRY  ShortEntry                   = {0};
	__FAT32_LONGENTRY*  pEntryArry                   = NULL;
	CHAR                szDirName[MAX_FILE_NAME_LEN] = {0};
	CHAR                szShorDir[64]                = {0};
	INT                 nEntryNum                    = 0;  
	INT                 nNameLen                     = strlen(pszDirName);
	DWORD               dwDirCluster                 = 0;	
	BOOL                bResult                      = FALSE;
	BOOL                bLongDir                     = FALSE;
	
	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pszDirName))
	{
		goto __TERMINAL;
	}

	//PrintLine(pszDirName);
	bLongDir = IsLongFileName(pszDirName);
	if(bLongDir == FALSE)
	{
		ConvertFatName(pszDirName,szDirName);
	}
	else
	{
		strcpy(szDirName,pszDirName);	
	}
	
	if(DirEntryIsExist(pFat32Fs,dwStartCluster,szDirName,bLongDir))
	{
		bResult = TRUE;		
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
		
	//allocate LONGENTRY num 
	if(nNameLen > FAT32_SHORTDIR_FILENAME_LEN)
	{
		nEntryNum = nNameLen/FAT32_LONGDIR_FILENAME_LEN;
		if(nNameLen % FAT32_LONGDIR_FILENAME_LEN)
		{
			nEntryNum ++;
		}
	}
	else if(bLongDir)
	{
		//extension over len ,need LONGENTRY 	
		nEntryNum ++;		
	}
	nEntryNum ++ ;//  add last short entry
	pEntryArry = (__FAT32_LONGENTRY*)FatMem_Alloc(nEntryNum*sizeof(__FAT32_LONGENTRY));
	if(nEntryNum > 1)
	{
		BYTE bSfnSum = 0;
		INT  i       = 1;

		//check short exist 
		while(1)
		{			
			MakeShortEntryName(szShorDir,pszDirName,i++);	
			//_hx_printf("short=%s,long=%s\n",szShorDir,pszDirName);
			if(DirEntryIsExist(pFat32Fs,dwStartCluster,szShorDir,FALSE) == FALSE)
			{
				break;	
			}
			memset(szShorDir,0,sizeof(szShorDir));
		}

		bSfnSum = GetSumOfSFN((BYTE*)szShorDir);
		InitLongEntry(pEntryArry,nEntryNum-1,pszDirName,bSfnSum);
	}
	else
	{
		strcpy(szShorDir,pszDirName);
		ToCapital(szShorDir);
	}

	if(!InitShortEntry(&ShortEntry,szShorDir,dwDirCluster,0,Attributes))
	{		
		goto __TERMINAL;
	}
		
	ShortEntry.CreateTimeTenth = 10;	
	SetFatFileDateTime(&ShortEntry,FAT32_DATETIME_CREATE|FAT32_DATETIME_WRITE);

	//add short entry
	memcpy(&pEntryArry[nEntryNum-1],(__FAT32_LONGENTRY*)&ShortEntry,sizeof(__FAT32_LONGENTRY));

	if(!CreateDirEntry(pFat32Fs,dwStartCluster,pEntryArry,nEntryNum))
	{			
		ReleaseCluster(pFat32Fs,dwDirCluster);
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	FatMem_Free(pEntryArry);
	return bResult;
}

/*BOOL CreateFatDir(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszDirName,BYTE Attributes)
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
*/

/* Create a new file in given directory. */
BOOL CreateFatFile(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszFileName,BYTE Attributes)
{
	return CreateFatDir(pFat32Fs,dwStartCluster,pszFileName,Attributes | FILE_ATTR_ARCHIVE);
#if 0
	DWORD               dwInitCluster = 0;
	__FAT32_SHORTENTRY  DirEntry;
	BOOL                bResult       = FALSE;

	/* Parameters checking. */
	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pszFileName))
	{
		goto __TERMINAL;
	}

	/* Allocate a free cluster for the new created file. */
	if(!GetFreeCluster(pFat32Fs,0,&dwInitCluster))
	{
		_hx_printf("%s:failed to obtain a free cluster.\r\n", __func__);
		goto __TERMINAL;
	}
	
	if(!InitShortEntry(&DirEntry,pszFileName,dwInitCluster,0,FILE_ATTR_ARCHIVE))
	{
		_hx_printf("%s:failed to initialize dir entry.\r\n", __func__);
		goto __TERMINAL;
	}
	
	DirEntry.CreateTimeTenth = 10;
	SetFatFileDateTime(&DirEntry,FAT32_DATETIME_CREATE|FAT32_DATETIME_WRITE);

	if(!CreateDirEntry(pFat32Fs,dwStartCluster,&DirEntry,1))
	{
		_hx_printf("%s:failed to create dir entry.\r\n", __func__);
		ReleaseCluster(pFat32Fs,dwInitCluster);
		goto __TERMINAL;
	}

	bResult = TRUE;
	_hx_printf("Create file [%s] successful.\r\n", pszFileName);
__TERMINAL:
	return bResult;
#endif
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

	if(!GetDirEntry(pFat32Fs,pszFileName,&ShortEntry,
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
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
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

	if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,dwSector,pFat32Fs->SectorPerClus,pBuffer))
	{
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:

	FatMem_Free(pBuffer);

	return bResult;
}

BOOL GetShortEntry(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pFileName,__FAT32_SHORTENTRY* pShortEntry, DWORD* pDirClus,DWORD* pDirOffset)
{
	__FAT32_SHORTENTRY* pfse         = NULL;
	BOOL                bResult      = FALSE;	
	BYTE*               pBuffer      = NULL;
	DWORD               dwCurrClus   = 0;
	DWORD               dwSector     = 0;	
	int                 i            = 0;

	if((NULL == pFat32Fs) || (NULL == pFileName) || (pShortEntry == pfse))
	{
		goto __TERMINAL;
	}
	//Create local buffer to contain one cluster.
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->SectorPerClus * pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)
	{

		goto __TERMINAL;
	}
	dwCurrClus = dwStartCluster;
	while(!IS_EOC(dwCurrClus))  //Main loop to check the root directory.
	{
		__FAT32_LONGENTRY*  szLongEntry[64]     = {0};
		INT                 nLongEntryNum       = 0;		
		BOOL                bFind               = FALSE;

		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)  //Fatal error.
		{
			goto __TERMINAL;
		}
		if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pBuffer))  //Can not read the appropriate sector(s).
		{
			goto __TERMINAL;
		}
		//Now check the root directory to seek the volume ID entry.
		pfse = (__FAT32_SHORTENTRY*)pBuffer;
		for(i = 0;i < pFat32Fs->SectorPerClus * 16;i ++)
		{
			if(0xE5 == (BYTE)pfse->FileName[0])  //Empty entry.
			{
				pfse += 1;						//Seek to the next entry.
				continue;
			}
			if(0 == pfse->FileName[0])     //All rest part is zero,no need to check futher.
			{
				break;
			}
			if(FILE_ATTR_LONGNAME == pfse->FileAttributes)  //Long file name entry.
			{
				szLongEntry[nLongEntryNum ++ ] =  (__FAT32_LONGENTRY*)pfse;

				pfse += 1;				
				continue;
			}

			if(FILE_ATTR_VOLUMEID & pfse->FileAttributes)   //Volume label entry.
			{
				pfse += 1;
				continue;
			}
			
			if(nLongEntryNum > 0 && !strstr(pFileName,"~")) 
			{
				CHAR   szLongFileName[MAX_FILE_NAME_LEN] = {0};

				CombinLongFileName(szLongEntry,nLongEntryNum,szLongFileName);							

				//_hx_printf("CombinLongFileName \n%s,%s\n",szLongFileName,pFileName);

				nLongEntryNum  = 0;				
				
				if(strncmp(szLongFileName,pFileName,strlen(pFileName)) == 0)
				{
					bFind = TRUE;
				}
				else
				{					
					pfse += 1;
					continue;
				}
			}
			else
			{
				CHAR    szFileName[MAX_FILE_NAME_LEN] = {0};

				memcpy(szFileName,pfse->FileName,sizeof(pfse->FileName));
				if(strcmp(pFileName,szFileName) == 0) 
				{
					bFind = TRUE;				
				}									
			}

			if(bFind)  //Found.
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

			pfse += 1;
		}
		if(!GetNextCluster(pFat32Fs,&dwCurrClus))
		{
			break;
		}
	}
__TERMINAL:
	FatMem_Free(pBuffer);

	return bResult;
}

//Get the directory entry of a full file name.
//The difference between this routine and GetShortEntry is,the last one only
//search the directory designated by start cluster.
//But this one will search the whole file system tree to seek the target.
BOOL GetDirEntry(__FAT32_FS* pFat32Fs, CHAR* pFullName,	 __FAT32_SHORTENTRY* pfse, DWORD* pDirClus, DWORD* pDirOffset)
{
	__FAT32_SHORTENTRY   ShortEntry = {0};
	CHAR                 SubDir[MAX_FILE_NAME_LEN];
	CHAR                 buffer[MAX_FILE_NAME_LEN];
	CHAR                 szConvertName[MAX_FILE_NAME_LEN] = {0};
	BOOL                 bResult = FALSE;
	DWORD                dwLevel = 0;	
	DWORD                dwStartClus = 0;      //Start cluster of current directory to search.
	DWORD                dwSector = 0;	
	int                  i;

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
		goto __TERMINAL;
	}
	i           = 1;
	dwStartClus = pFat32Fs->dwRootDirClusStart;

	//Initialize the short entry as root directory.
	ShortEntry.FileAttributes = FILE_ATTR_DIRECTORY;
	ShortEntry.wFirstClusHi   = (WORD)(dwStartClus >> 16);
	ShortEntry.wFirstClusLow  = (WORD)dwStartClus;

	while(dwLevel)
	{	
		if(!GetSubDirectory(pFullName,i,SubDir))  //Get the sub-directory.
		{			
			goto __TERMINAL;
		}
		memset(szConvertName,0,sizeof(szConvertName));

		ConvertFatName(SubDir,szConvertName);
		//_hx_printf("ConvertFatName: src=%s,dst=%s\n",SubDir,szConvertName);

		if(!GetShortEntry(pFat32Fs,dwStartClus,szConvertName,&ShortEntry,pDirClus,pDirOffset))
		{		
			//_hx_printf("GetDirEntry: SubDir=%s,src=%s\n",SubDir,szConvertName);
			goto __TERMINAL;
		}
		dwStartClus   = ShortEntry.wFirstClusHi;
		dwStartClus   = (dwStartClus<<16)+ShortEntry.wFirstClusLow;
		//dwStartClus   = //MAKELONG(ShortEntry.wFirstClusLow,ShortEntry.wFirstClusHi);		
		dwLevel --;
		i ++;
	}

	if(!GetPathName(pFullName,SubDir,buffer))
	{		
		goto __TERMINAL;
	}

	memset(szConvertName,0,sizeof(szConvertName));
	ConvertFatName(buffer,szConvertName);	
	//_hx_printf("GetDirEntry: fat=%s,src=%s\n",buffer,szConvertName);

	if(0 == szConvertName[0])  //The target is a directory.
	{
		memcpy((char*)pfse,(const char*)&ShortEntry,sizeof(__FAT32_SHORTENTRY));
		bResult = TRUE;

		goto __TERMINAL;
	}

	if(!GetShortEntry(pFat32Fs,dwStartClus,szConvertName,&ShortEntry,pDirClus,pDirOffset))
	{	
		goto __TERMINAL;
	}

	memcpy((char*)pfse,(const char*)&ShortEntry,sizeof(__FAT32_SHORTENTRY));
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#endif
