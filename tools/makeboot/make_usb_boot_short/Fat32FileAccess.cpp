
#include "stdafx.h"
#include "Fat32FileAccess.h"

#define MAX_FILE_NAME_LEN 256

struct FAT32_FS;
typedef struct FAT32_FILE
{
	CHAR       FileName[13];     //Zero terminated file name.
	CHAR       FileExtension[4]; //Zero terminated file extension.
	BYTE       Attributes;       //File attributes.
	DWORD      dwStartClusNum;   //Start cluster number of this file.
	DWORD      dwCurrClusNum;    //Current cluster number.
	DWORD      dwCurrPos;        //Current file pointer position.
	DWORD      dwClusOffset;     //Current offset in current cluster.
	DWORD      dwFileSize;       //File size.
	DWORD      dwOpenMode;       //Opened mode,such as READ,WRITE,etc.
	DWORD      dwShareMode;      //Share mode.
	BOOL       bInRoot;          //If in root directory.
	DWORD      dwParentClus;     //Parent directory's cluster number.
	DWORD      dwParentOffset;   //File short entry's offset in directory.

	struct FAT32_FS*        pFileSystem;    //File system this file belong to.
	VOID*                    pFileCache;     //File cache object.
	VOID*                     pPartition;     //Partition this file belong to.
	struct FAT32_FILE*        pNext;          //Pointing to next one.
	struct FAT32_FILE*        pPrev;          //Pointing to previous one.
}__FAT32_FILE;


#define FAT_CACHE_LENGTH 1024 

typedef struct FAT32_FS
{
	VOID*               pPartition;      //Partition this file system based.
	DWORD               dwAttribute;     //File system attributes.
	BYTE                SectorPerClus;   //Sector per cluster.
	CHAR                VolumeLabel[13]; //Volume lable.
	BYTE                FatNum;          //How many FAT in this volume.
	BYTE                Reserved;        //Reserved to align DWORD.
	WORD                wReservedSector; //Reserved sector number.
	WORD                wFatInfoSector;  //FAT information sector number.
	DWORD               dwPartitionSatrt; //Partition start sector
	DWORD               dwBytePerSector; //Byte number per sector.
	DWORD               dwClusterSize;   //Byte number of one cluster.
	DWORD               dwDataSectorStart;          //Start sector number of data.
	DWORD               dwRootDirClusStart;         //Start cluster number of root dir.
	DWORD               dwFatBeginSector;           //Start sector number of FAT.
	DWORD               dwFatSectorNum;             //Sector number per FAT.
	DWORD               FatCache[FAT_CACHE_LENGTH]; //FAT entry cache.
	__FAT32_FILE*       pFileList;                  //File list header.
}__FAT32_FS;


BOOL ReadDeviceSector(VOID* pPartition,DWORD dwStartSectorNum, DWORD dwSectorNum,BYTE* pBuffer)
{
	DWORD dwRead = 0;

	SetFilePointer((HANDLE)pPartition,dwStartSectorNum*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	ReadFile((HANDLE)pPartition,pBuffer,dwSectorNum*HX_SECTOR_SIZE,&dwRead,NULL);
		
 	return (dwRead == dwSectorNum*HX_SECTOR_SIZE);
}

BOOL WriteDeviceSector(VOID* pPartition,DWORD dwStartSectorNum,	DWORD dwSectorNum,BYTE* pBuffer)
{
	DWORD dwWrite = 0;

	SetFilePointer((HANDLE)pPartition,dwStartSectorNum*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	WriteFile((HANDLE)pPartition,pBuffer,dwSectorNum*HX_SECTOR_SIZE,&dwWrite,NULL);

	return (dwWrite == dwSectorNum*HX_SECTOR_SIZE);

}

//set new file date and time
VOID SetFatFileDateTime(__FAT32_SHORTENTRY*  pDirEntry,DWORD dwTimeFlage)
{
	BYTE         time[6] = {0};
	SYSTEMTIME  szTime   = {0};
	WORD   wdate;
	WORD   wtime;
	WORD   wtemp; 

	GetLocalTime(&szTime);
	time[0] = (BYTE)szTime.wYear;
	time[1] = (BYTE)szTime.wMonth;
	time[2] = (BYTE)szTime.wDay;

	time[3] = (BYTE)szTime.wHour;
	time[4] = (BYTE)szTime.wMinute;
	time[5] = (BYTE)szTime.wSecond;
	
	//__GetTime(&time[0]);
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
	wtemp = (time[0])-1980;  
	wtemp = wtemp<<9;
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

//A helper routine to check the validity of a file name.
//For a regular file name,the first three characters must be file system identifier,a colon,
//and a slash character.
BOOL NameIsValid(CHAR* pFullName)
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
BOOL GetFullNameLevel(CHAR* pFullName,DWORD* pdwLevel)
{
	int i = 0;
	DWORD dwLevel = 0;

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
BOOL GetSubDirectory(CHAR* pFullName,DWORD dwLevel,CHAR* pSubDir)
{
	BOOL       bResult         = FALSE;
	DWORD      dwTotalLevel    = 0;
	DWORD      i = 0;
	DWORD      j = 0;
	BYTE       buffer[16];  //Contain sub-directory name temporary.

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
		strcpy((CHAR*)pSubDir,(CHAR*)&buffer[0]);
		
	}
	return bResult;
}

//Segment one full file name into directory part and file name part.
//pDir and pFileName must be long enough to contain the result.
//If the full name's level is zero,that no subdirectory in the full name,
//the pDir[0] will contain the file system identifier to indicate this.
//If the full name only contain a directory,i.e,the last character of
//the full name is a slash character,then the pFileName[0] will be set to 0.
BOOL GetPathName(CHAR* pFullName,CHAR* pDir,CHAR* pFileName)
{
	BYTE        DirName[MAX_FILE_NAME_LEN] = {0};
	BYTE        FileName[13];
	BYTE        tmp;
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
	strcpy((CHAR*)&DirName[0],(CHAR*)pFullName);
	
	i = strlen((CHAR*)pFullName);
	i -= 1;
	if(pFullName[i] == '\\') //The last character is a splitter,means only dir name present.
	{
		//DirName[i] = 0;      //Eliminate the slash.
		strcpy((CHAR*)pDir,(CHAR*)&DirName[0]);
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
	strcpy((CHAR*)pDir,(CHAR*)&DirName[0]);	
	strcpy((CHAR*)pFileName,(CHAR*)&FileName[0]);
	

	return TRUE;
}

//Convert the short name in FAT entry to dot format.
//  @pfse    : The short entry with the name to convert;
//  @pResult : The converting result,pointing to a string whose length must
//             longer or equal to 13.
BOOL ConvertName(__FAT32_SHORTENTRY* pfse,BYTE* pResult)
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

	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)  //Can not allocate temporary buffer.
	{
		return FALSE;
	}

	dwSector = pFat32Fs->dwFatBeginSector;
	while(dwSector < pFat32Fs->dwFatSectorNum + pFat32Fs->dwFatBeginSector)
	{
		if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+ dwSector,1,pBuffer))  //Can not read the sector from fat region.
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
				
				if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+ dwSector,	1,pBuffer))
				{
					goto __TERMINAL;
				}

				//Write to backup FAT region.
				WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+pFat32Fs->dwFatSectorNum+dwSector,	1,pBuffer);  //It is no matter if write to backup fat failed.
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
		LocalFree(pBuffer);
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

	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	//Read the fat sector where this cluster's index resides,modify to zero and write it back.
	if(!ReadDeviceSector(pFat32Fs->pPartition,	dwSector+pFat32Fs->dwPartitionSatrt,	1,pBuffer))
	{
		goto __TERMINAL;
	}

	*(DWORD*)(pBuffer + dwOffset) &= 0xF0000000;
	if(!WriteDeviceSector(pFat32Fs->pPartition,
		dwSector+pFat32Fs->dwPartitionSatrt,
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
		LocalFree(pBuffer);
	}
	return bResult;
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
	BYTE            buff[512];

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
	if(!ReadDeviceSector(pFat32Fs->pPartition,
		pFat32Fs->dwPartitionSatrt+ dwClusSector,
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

	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwBytePerSector);
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
	if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+ dwSector,1,pBuffer))
	{		
		goto __TERMINAL;
	}
	//Save the next cluster to chain.
	*(DWORD*)(pBuffer + dwOffset) &= 0xF0000000;  //Keep the leading 4 bits.
	*(DWORD*)(pBuffer + dwOffset) += (dwNextCluster & 0x0FFFFFFF);
	if(!WriteDeviceSector(pFat32Fs->pPartition,	pFat32Fs->dwPartitionSatrt+ dwSector,1,pBuffer))
	{
		ReleaseCluster(pFat32Fs,dwNextCluster);   //Release this cluster.		
		goto __TERMINAL;
	}

	dwEndCluster = *(DWORD*)(pBuffer + dwOffset);
	if(!GetNextCluster(pFat32Fs,&dwEndCluster))
	{
		
	}

	bResult = TRUE;  //Anything is in place.

__TERMINAL:
	if(pBuffer)  //Should release it.
	{
		LocalFree(pBuffer);
	}

	if(bResult)
	{
		*pdwCurrCluster = (dwNextCluster & 0x0FFFFFFF);
	}

	return bResult;
}

BOOL GetShortEntry(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pFileName,__FAT32_SHORTENTRY* pShortEntry, DWORD* pDirClus,DWORD* pDirOffset)
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
	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->SectorPerClus * pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)
	{
		
		goto __TERMINAL;
	}
	dwCurrClus = dwStartCluster;
	while(!IS_EOC(dwCurrClus))  //Main loop to check the root directory.
	{
		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)  //Fatal error.
		{
			//PrintLine("  In GetShortEntry: Can not get cluster sector.");
			goto __TERMINAL;
		}
		if(!ReadDeviceSector(pFat32Fs->pPartition,
			pFat32Fs->dwPartitionSatrt+dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))  //Can not read the appropriate sector(s).
		{
			//PrintLine("  In GetShortEntry: Can not read sector from device.");
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
				if(strcmpi((CHAR*)pFileName,(CHAR*)&FileName[0]) == 0)  //Found.
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
		LocalFree(pBuffer);
	}

	return bResult;
}


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

	ZeroMemory(pfse,sizeof(__FAT32_SHORTENTRY));
	
	if(strcmp(pszName,".") == 0)
	{
		InitDot(pfse);
		goto __INITOTHER;
	}
	if(strcmp(pszName,"..") == 0)
	{
		InitDotdot(pfse);
		goto __INITOTHER;
	}
	//Now convert the name to directory format.
	nNameLen = strlen(pszName);
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
		goto __TERMINAL;
	}
	if(!ConvertName(pDirEntry,(BYTE*)&DirName[0]))
	{
		goto __TERMINAL;
	}
	//Check if the directory to be created has already in directory.
	if(GetShortEntry(pFat32Fs,dwStartCluster,DirName,&DirEntry,NULL,NULL))  //Directory already exists.
	{
		goto __TERMINAL;
	}
	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwClusterSize);
	if(NULL == pBuffer)
	{
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
			//PrintLine("  In CreateDirEntry,Condition 2");
			goto __TERMINAL;
		}
		if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector, pFat32Fs->SectorPerClus,pBuffer))
		{			//PrintLine("  In CreateDirEntry,Condition 3");
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
			//PrintLine("  In CreateDirEntry,Condition 4");
			goto __TERMINAL;
		}
	}

	if(bFind)  //Has found a free directory slot.
	{
		memcpy((char*)pfse,(const char*)pDirEntry,sizeof(__FAT32_SHORTENTRY));
		if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pBuffer))
		{			
			goto __TERMINAL;
		}
	}
	else       //Can not find a free slot,allocate a new cluster for parent directory.
	{
		if(!AppendClusterToChain(pFat32Fs,&dwCurrCluster))
		{			
			goto __TERMINAL;
		}
		ZeroMemory(pBuffer,pFat32Fs->dwClusterSize);
		memcpy((char*)pBuffer,(const char*)pDirEntry,sizeof(__FAT32_SHORTENTRY));
		dwSector = GetClusterSector(pFat32Fs,dwCurrCluster);
		if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pBuffer))
		{			
			goto __TERMINAL;
		}
	}

	bResult = TRUE;

__TERMINAL:
	if(pBuffer)
	{
		LocalFree(pBuffer);
	}

	return bResult;
}

//Get the directory entry of a full file name.
//The difference between this routine and GetShortEntry is,the last one only
//search the directory designated by start cluster.
//But this one will search the whole file system tree to seek the target.
BOOL GetDirEntry(__FAT32_FS* pFat32Fs,CHAR* pFullName, __FAT32_SHORTENTRY* pfse,DWORD* pDirClus,DWORD* pDirOffset)
{
	__FAT32_SHORTENTRY   ShortEntry         = {0};
	CHAR                 SubDir[MAX_FILE_NAME_LEN];
	CHAR                 buffer[MAX_FILE_NAME_LEN];  //
	BOOL                 bResult            = FALSE;
	DWORD                dwLevel            = 0;	
	BYTE*                pBuffer            = 0;
	DWORD                dwStartClus        = 0;      //Start cluster of current directory to search.
	DWORD                dwSector           = 0;	
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
			//PrintLine("  In GetDirEntry: GetSubDirectory failed.");
			goto __TERMINAL;
		}
		if(!GetShortEntry(pFat32Fs,dwStartClus,SubDir,&ShortEntry,pDirClus,pDirOffset))
		{
		
			goto __TERMINAL;
		}
		dwStartClus = MAKELONG(ShortEntry.wFirstClusLow,ShortEntry.wFirstClusHi);
		//dwStartClus = (DWORD)ShortEntry.wFirstClusHi;
		//dwStartClus = dwStartClus << 16;
		//dwStartClus = dwStartClus + (DWORD)ShortEntry.wFirstClusLow;
		dwLevel --;
		i ++;
	}

	if(!GetPathName(pFullName,SubDir,buffer))
	{
	//	PrintLine("  In GetDirEntry: GetPathName failed.");
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
	
	memcpy((char*)pfse,(const char*)&ShortEntry,sizeof(__FAT32_SHORTENTRY));
	bResult = TRUE;

__TERMINAL:
	if(pBuffer)
	{
		free(pBuffer);
	}
	return bResult;
}
//Initialize one FAT32 file system given the first sector data.
//Return TRUE if sucessfully.
BOOL Fat32Init(__FAT32_FS* pFat32Fs,BYTE* pSector0)
{
	UCHAR*    pStart         = (UCHAR*)pSector0;
	BOOL      bResult        = FALSE;
	DWORD     dwCluster      = 0;
	WORD      RootDirSector  = 0;  //Following variables are used to judge the FAT type.
	WORD      wRootEntryCnt  = 0;
	DWORD     FATSz          = 0;
	DWORD     TotSec         = 0;
	DWORD     DataSec        = 0;
	DWORD     CountOfCluster = 0;


	if((NULL == pFat32Fs) || (NULL == pSector0)) //Invalid parameters.
	{
		goto __TERMINAL;
	}
	if((0x55 != pStart[510]) || (0xAA != pStart[511]))  //Check the signature.
	{
		return FALSE;
	}
	//Initialize the FAT32 extension object,pFat32Fs.
	pFat32Fs->dwAttribute         = FILE_SYSTEM_TYPE_FAT32;
	pFat32Fs->SectorPerClus       = pStart[BPB_SecPerClus];
	pFat32Fs->wReservedSector     = *(WORD*)(pStart + BPB_RsvdSecCnt);
	pFat32Fs->FatNum              = *(pStart + BPB_NumFATs);
	pFat32Fs->dwFatSectorNum      = *(DWORD*)(pStart + BPB_FAT32_FATSz32);
	pFat32Fs->dwRootDirClusStart  = *(DWORD*)(pStart + BPB_FAT32_RootClus);
	pFat32Fs->wFatInfoSector      = *(WORD*)(pStart + BPB_FAT32_FSInfo);
	pFat32Fs->dwFatBeginSector    = (DWORD)pFat32Fs->wReservedSector;
	pFat32Fs->dwDataSectorStart   = pFat32Fs->dwFatBeginSector + (pFat32Fs->FatNum * pFat32Fs->dwFatSectorNum);
	pFat32Fs->dwBytePerSector     = (DWORD)(*(WORD*)(pStart + BPB_BytsPerSec));
	pFat32Fs->dwClusterSize       = pFat32Fs->dwBytePerSector * pFat32Fs->SectorPerClus;
	pFat32Fs->pFileList           = NULL;

	if(0 == pFat32Fs->dwBytePerSector)
	{		
		goto __TERMINAL;
	}
	//goto __TERMINAL;
	//Now check if this partition is ACTUALLY a FAT32 partition.
	wRootEntryCnt = *(WORD*)(pStart + BPB_RootEntCnt);
	wRootEntryCnt *= 32;
	wRootEntryCnt += (WORD)pFat32Fs->dwBytePerSector - 1;
	RootDirSector = wRootEntryCnt / (WORD)pFat32Fs->dwBytePerSector;  //Get root directory's sector num.

	if(*(WORD*)(pStart + BPB_FATSz16) != 0)
	{
		FATSz = (DWORD)(*(WORD*)(pStart + BPB_FATSz16));
	}
	else
	{
		FATSz = *(DWORD*)(pStart + BPB_FAT32_FATSz32);
	}

	if(*(WORD*)(pStart + BPB_TotSec16) != 0)
	{
		TotSec = (DWORD)(*(WORD*)(pStart + BPB_TotSec16));
	}
	else
	{
		TotSec = *(DWORD*)(pStart + BPB_TotSec32);
	}

	DataSec  = TotSec - (DWORD)pFat32Fs->wReservedSector;
	DataSec -= (FATSz * pFat32Fs->FatNum);
	DataSec -= RootDirSector;  //We now have got the data sector number.
	CountOfCluster = DataSec / pFat32Fs->SectorPerClus;  //Get the total cluster counter.

	if(CountOfCluster < 4085)
	{		
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:

	return bResult;
}

HANDLE APIENTRY OpenVhdFile(LPCTSTR pVhdFile,DWORD dwPartitionSatrt)
{
	HANDLE             hVhdObj               = NULL;
	MAIN_PATION_INFO*  pMainPationInfo       = NULL;	
	__FAT32_FS*        pFat32Fs              = NULL;
	BYTE               szBuf[HX_SECTOR_SIZE] = {0};

	hVhdObj = CreateFile(pVhdFile,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
	if(hVhdObj == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}


	pFat32Fs = (__FAT32_FS*)LocalAlloc(LPTR,sizeof(__FAT32_FS));
	pFat32Fs->pPartition = hVhdObj;

	
	ReadDeviceSector(pFat32Fs->pPartition,0,1,szBuf);
	pMainPationInfo = (MAIN_PATION_INFO*)(szBuf+HX_BOOTSEC_MPS);

	pFat32Fs->dwPartitionSatrt = dwPartitionSatrt;/* pMainPationInfo->bStartClinder*HX_VDISK_CLINDER_PER_TRACK
		                         +pMainPationInfo->bStartTrack*HX_VDISK_TRACK_PER_SECTOR
								 +pMainPationInfo->bStartSector-1;*/

	ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt,1,szBuf);
	
	Fat32Init(pFat32Fs,szBuf);

	return (HANDLE)pFat32Fs;
}

VOID APIENTRY CloseVhdFile(HANDLE hVhdObj)
{
	__FAT32_FS*    pFat32Fs     = (__FAT32_FS*)hVhdObj;

	if(pFat32Fs)
	{
		CloseHandle((HANDLE)pFat32Fs->pPartition);
		LocalFree(pFat32Fs);
	}

	return ;
}

//Initializes one directory,i.e,write dot and dotdot directories into one directory newly created.
//  @dwParentCluster  : The start cluster number of target directory's parent directory;
//  @dwDirCluster     : The start cluster number of target directory.
BOOL InitDirectory(__FAT32_FS* pFat32Fs,DWORD dwParentCluster,DWORD dwDirCluster)
{
	__FAT32_SHORTENTRY* pfse        = NULL;
	BYTE*               pBuffer     = NULL;      //Temprory buffer used to contain first cluster.	
	BOOL                bResult     = FALSE;
	DWORD               dwDirSector = 0;
	

	if((NULL == pFat32Fs) || (dwParentCluster < 2) || (dwDirCluster < 2) || IS_EOC(dwParentCluster) ||
	   IS_EOC(dwDirCluster))
	{
		goto __TERMINAL;
	}
	//Allocate temporary buffer.
	pBuffer = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwClusterSize);
	if(NULL == pBuffer)
	{		
		goto __TERMINAL;
	}

	dwDirSector = GetClusterSector(pFat32Fs,dwDirCluster);  //Get the start cluster number of target dir.
	if(IS_EOC(dwDirSector))
	{
		
		goto __TERMINAL;
	}
	//Clear cluster's content to zero.
	ZeroMemory(pBuffer,pFat32Fs->dwClusterSize);
	pfse = (__FAT32_SHORTENTRY*)pBuffer;
	InitShortEntry(pfse,".",dwDirCluster,0,FILE_ATTR_DIRECTORY);
	pfse ++;

	//Create the dotdot directory.
	InitShortEntry(pfse,"..",dwDirCluster,0,FILE_ATTR_DIRECTORY);
	pfse ++;

	//Write the cluster into directory.
	if(!WriteDeviceSector(pFat32Fs->pPartition,	pFat32Fs->dwPartitionSatrt+dwDirSector,pFat32Fs->SectorPerClus,	pBuffer))
	{		
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if(pBuffer)
	{
		LocalFree(pBuffer);
	}

	return bResult;
}

BOOL CreateFatDir(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszDirName,BYTE Attributes)
{
	__FAT32_SHORTENTRY  DirEntry     = {0};
	DWORD               dwDirCluster = 0;	
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


	if(!InitShortEntry(&DirEntry,pszDirName,dwDirCluster,0,FILE_ATTR_DIRECTORY))
	{
		goto __TERMINAL;
	}

	DirEntry.CreateTimeTenth = 10;
	SetFatFileDateTime(&DirEntry,FAT32_DATETIME_CREATE|FAT32_DATETIME_WRITE);

	if(!CreateDirEntry(pFat32Fs,dwStartCluster,&DirEntry))
	{
	//	PrintLine("In CreateFatDir: Can not create directory entry in parent dir.");
		ReleaseCluster(pFat32Fs,dwDirCluster);
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}
//Implementation of CreateDirectory.
BOOL APIENTRY AddDirToVhd(HANDLE hVhdObj,LPCSTR pDirName)    //Create directory.
{
	DWORD                    dwDirCluster                  = 0;
	CHAR                     DirName[MAX_FILE_NAME_LEN]    = {0};
	CHAR                     SubDirName[MAX_FILE_NAME_LEN] = {0};
	__FAT32_SHORTENTRY       DirShortEntry                 = {0};
	__FAT32_FS*              pFat32Fs                      = (__FAT32_FS*)hVhdObj;
	

	if((NULL == pFat32Fs) || (NULL == pDirName))
	{
		return FALSE;
	}

	if(!GetPathName((LPSTR)pDirName,DirName,SubDirName))
	{
		return FALSE;
	}

	//Try to open the parent directory.
	if(!GetDirEntry(pFat32Fs,DirName,&DirShortEntry,NULL,NULL))
	{	
		return FALSE;
	}

	if(!(DirShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))  //Is not a directory.
	{
		return FALSE;
	}

	dwDirCluster = MAKELONG(DirShortEntry.wFirstClusLow,DirShortEntry.wFirstClusHi);

	return CreateFatDir(pFat32Fs,dwDirCluster,SubDirName,0);
}

//Create a new file in given directory.
BOOL CreateFatFile(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszFileName,BYTE Attributes)
{
	DWORD               dwInitCluster = 0;
	__FAT32_SHORTENTRY  DirEntry      = {0};
	BOOL                bResult       = FALSE;


	if((NULL == pFat32Fs) || (dwStartCluster < 2) || IS_EOC(dwStartCluster) || (NULL == pszFileName))
	{
		goto __TERMINAL;
	}

	//Allocate a free cluster for the new created file.
	if(!GetFreeCluster(pFat32Fs,0,&dwInitCluster))
	{
		goto __TERMINAL;
	}
	if(!InitShortEntry(&DirEntry,pszFileName,dwInitCluster,0,FILE_ATTR_ARCHIVE))
	{
		//PrintLine("In CreateFatFile: Can not initialize short entry.");
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


__FAT32_FILE*   OpenFat32File(__FAT32_FS*   pFat32Fs,CHAR*  pszFileName)
{
	__FAT32_FILE*     pFat32File      = NULL; 		
	__FAT32_SHORTENTRY  ShortEntry    = {0};
	CHAR                FileName[MAX_FILE_NAME_LEN];
	CHAR                DirName[MAX_FILE_NAME_LEN];
	DWORD               dwDirClus     = 0;
	DWORD               dwDirOffset   = 0;	
	BOOL                bResult       = FALSE;
	
	strcpy(FileName,pszFileName);	

		
	if(!GetDirEntry(pFat32Fs,FileName,&ShortEntry,&dwDirClus,&dwDirOffset))
	{
		goto __TERMINAL;
	}

	if(FILE_ATTR_DIRECTORY & ShortEntry.FileAttributes)  //Is a directory.
	{
		goto __TERMINAL;
	}
	//Create a file object.
	pFat32File = (__FAT32_FILE*)LocalAlloc(LPTR,sizeof(__FAT32_FILE));
	if(NULL == pFat32File)
	{
		goto __TERMINAL;
	}
	pFat32File->Attributes     = ShortEntry.FileAttributes;
	pFat32File->dwClusOffset   = 0;
	pFat32File->bInRoot        = FALSE;          //Caution,not check currently.
	pFat32File->dwCurrClusNum  = ((DWORD)ShortEntry.wFirstClusHi << 16) 	+ (DWORD)ShortEntry.wFirstClusLow;

	pFat32File->dwCurrPos      = 0;
	pFat32File->dwFileSize     = ShortEntry.dwFileSize;
	pFat32File->dwOpenMode     = 0;;
	pFat32File->dwShareMode    = 0;;
	pFat32File->dwStartClusNum = pFat32File->dwCurrClusNum;
	pFat32File->pFileSystem    = pFat32Fs;
	pFat32File->pPartition     = pFat32Fs->pPartition;   //CAUTION!!!
	pFat32File->dwParentClus   = dwDirClus;              //Save parent directory's information.
	pFat32File->dwParentOffset = dwDirOffset;
	pFat32File->pNext          = NULL;
	pFat32File->pPrev          = NULL;
	//Now insert the file object to file system object's file list.
	
	if(pFat32Fs->pFileList == NULL)  //Not any file object in list yet.
	{
		pFat32Fs->pFileList = pFat32File;
		pFat32File->pNext   = NULL;
		pFat32File->pPrev   = NULL;
	}
	else
	{
		pFat32File->pNext          = pFat32Fs->pFileList;
		pFat32Fs->pFileList->pPrev = pFat32File;
		pFat32File->pPrev          = NULL;
		pFat32Fs->pFileList        = pFat32File;
	}

__TERMINAL:

	return pFat32File;
}

DWORD WriteFileToVhd(__FAT32_FS*  pFat32Fs,__FAT32_FILE*   pFat32File,LPCSTR pSrcFile)
{
	__FAT32_SHORTENTRY*     pFat32Entry    = NULL;
	HANDLE                  hSrcFile       = NULL;	
	BYTE*                   pClusBuffer    = NULL;	
	BYTE*                   pStart         = NULL;
	DWORD                   dwCurrPos      = 0;
	DWORD                   dwSector       = 0;
	DWORD                   dwNextClus     = 0;
	DWORD                   dwWriteSize    = 0;
	DWORD                   dwFirstCluster = 0;	
	DWORD                   dwOnceSize     = 0;
	DWORD                   dwWritten      = 0;    //Record the written size.
	
	
	
	hSrcFile     = CreateFileA(pSrcFile,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
	if(hSrcFile == INVALID_HANDLE_VALUE)
	{
		return S_OK;
	}
	dwWriteSize   = GetFileSize(hSrcFile,NULL);
	
	pClusBuffer  = (BYTE*)LocalAlloc(LPTR,pFat32Fs->dwClusterSize);
	if(NULL == pClusBuffer)  //Can not allocate buffer.
	{
		goto __TERMINAL;
	}

	dwCurrPos  = pFat32File->dwCurrPos;
	dwNextClus = pFat32File->dwCurrClusNum;

	//if file null，first alloc a Cluster
	if(dwNextClus == 0 && GetFreeCluster(pFat32Fs,0,&dwNextClus))
	{
		pFat32File->dwCurrClusNum  = dwNextClus;
		pFat32File->dwStartClusNum = dwNextClus;
		dwFirstCluster             = dwNextClus;
	}

	dwSector   = GetClusterSector(pFat32Fs,dwNextClus);		
	if(0 == dwSector)
	{		
		goto __TERMINAL;
	}
	//Read the current cluster.
	if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+ dwSector,pFat32Fs->SectorPerClus,	pClusBuffer))
	{
		goto __TERMINAL;
	}

	while(dwWriteSize > 0)
	{
		DWORD      dwRead = 0;
		pStart     = pClusBuffer + pFat32File->dwClusOffset;
		dwOnceSize = pFat32Fs->dwClusterSize - pFat32File->dwClusOffset;

		if(dwOnceSize > dwWriteSize)
		{
			dwOnceSize = dwWriteSize;
		}
		//memcpy(pStart,pBuffer,dwOnceSize);
		ReadFile(hSrcFile,pStart,dwOnceSize,&dwRead,NULL);

		//Now write the cluster into memory.
		if(!WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pClusBuffer))
		{
			
			goto __TERMINAL;
		}

		//Adjust file object's status.
		pFat32File->dwClusOffset += dwOnceSize;
		pFat32File->dwCurrPos    += dwOnceSize;

		//2014.9.28 modified by tywind
		if(pFat32File->dwCurrPos >= pFat32File->dwFileSize)
		{
			pFat32File->dwFileSize = pFat32File->dwCurrPos;
		}

		if(0 == (pFat32File->dwClusOffset % pFat32Fs->dwClusterSize))
		{
			dwNextClus = pFat32File->dwCurrClusNum;
			if(!GetNextCluster(pFat32Fs,&dwNextClus))
			{				
				goto __TERMINAL;
			}
			if(IS_EOC(dwNextClus))  //Reach the end of file,so extend file.
			{
				if(!AppendClusterToChain(pFat32Fs,&pFat32File->dwCurrClusNum))
				{
					goto __TERMINAL;
				}
				dwNextClus = pFat32File->dwCurrClusNum;
			}
			pFat32File->dwCurrClusNum = dwNextClus;
			pFat32File->dwClusOffset  = 0;

			//Update dwSector to corespond current cluster.
			dwSector = GetClusterSector(pFat32Fs,dwNextClus);
			if(0 == dwSector)
			{				
				goto __TERMINAL;
			}
		}
		//Adjust the buffer position and local control variables.		
		dwWritten    += dwOnceSize;
		dwWriteSize  -= dwOnceSize;

		if(0 == dwWriteSize)  //Write over.
		{
			break;
		}
	}
	//Now update the file's directory entry.
	dwSector = GetClusterSector(pFat32Fs,pFat32File->dwParentClus);
	if(0 == dwSector)
	{		
		goto __TERMINAL;
	}

	if(!ReadDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pClusBuffer))
	{
		goto __TERMINAL;
	}

	pFat32Entry = (__FAT32_SHORTENTRY*)(pClusBuffer + pFat32File->dwParentOffset);

	//modify file First Cluster index 
	if(dwFirstCluster > 0 )
	{
		pFat32Entry->wFirstClusHi   = (WORD)(dwFirstCluster >> 16);
		pFat32Entry->wFirstClusLow  = (WORD)(dwFirstCluster&0x0000FFFF);
	}

	//update write date and time 
	SetFatFileDateTime(pFat32Entry,FAT32_DATETIME_WRITE);

	
	pFat32Entry->dwFileSize = pFat32File->dwFileSize;
	WriteDeviceSector(pFat32Fs->pPartition,pFat32Fs->dwPartitionSatrt+dwSector,pFat32Fs->SectorPerClus,pClusBuffer);
	

__TERMINAL:
	if(pClusBuffer)
	{
		LocalFree(pClusBuffer);
	}
	CloseHandle(hSrcFile);

	return dwWritten;

}
BOOL APIENTRY AddFileToVhd(HANDLE hVhdObj,LPCSTR pSrcFile,LPCSTR pDstFile)
{
	__FAT32_FS*         pFat32Fs                    = (__FAT32_FS*)hVhdObj;
	__FAT32_FILE*       pFat32File                  = NULL; 
	__FAT32_SHORTENTRY  DirShortEntry               = {0};
	CHAR                DirName[MAX_FILE_NAME_LEN]  = {0};
	CHAR                FileName[MAX_FILE_NAME_LEN] = {0};
	CHAR*               pszFileName                 = (CHAR*)pDstFile;	
	DWORD               dwDirCluster                = 0;
	DWORD               dwResult                    = 0;


	if(pFat32Fs == NULL)
	{
		return FALSE;
	}
		
	if(!GetPathName(pszFileName,DirName,FileName) || strlen(FileName) <= 0)
	{
		return FALSE;
	}
	
	//Try to open the parent directory.
	if(!GetDirEntry(pFat32Fs,DirName,&DirShortEntry,NULL,NULL))
	{	
		return FALSE;
	}

	if(!(DirShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))  //Is not a directory.
	{
		return FALSE;
	}
	dwDirCluster =   DirShortEntry.wFirstClusHi;
	dwDirCluster <<= 16;
	dwDirCluster +=  DirShortEntry.wFirstClusLow;

	if(!CreateFatFile(pFat32Fs,	dwDirCluster,FileName,0))
	{
		return FALSE;		
	}
		
	pFat32File  = OpenFat32File(pFat32Fs,pszFileName);	
	if(pFat32File)
	{
		//Now start writefile
		WriteFileToVhd(pFat32Fs,pFat32File,pSrcFile);
		
		LocalFree(pFat32File);
	}


	return TRUE;
}

BOOL DelFileFromVhd(HANDLE hVhdObj,LPCTSTR pDiskFile)
{
	return TRUE;
}

