//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 24 FEB, 2009
//    Module Name               : FAT322.CPP
//    Module Funciton           : 
//                                This module countains the implementation code of
//                                FAT32 file system.
//                                This is the second part of FAT32x.CPP,which contain
//                                the implementation code of FAT32 file system.
//                                If one source code file's size exceed 1000 lines,
//                                I will separate it into more fragments.
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



//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
//static
VOID ToCapital(LPSTR lpszString)
{
	int nIndex = 0;

	if(NULL == lpszString)
	{
		return;
	}
	while(lpszString[nIndex++])
	{
		if((lpszString[nIndex] >= 'a') && (lpszString[nIndex] <= 'z'))
		{
			lpszString[nIndex] += 'A' - 'a';
		}
	}
}

//Create a new file in the specified directory.
//Input of lpDrcb:
//  lpInputBuffer  :     Pointing to new file's name and path.
//  dwInputLen     :     Target file name's length.
//  dwExtraParam1  :     Target file's attributes.
//Output if successful:
//  lpOutputBuffer :     Pointing to the new file's handle(opened).
//  dwStatus       :     DRCB_STATUS_SUCCESS.
//Output if failed:
//  dwStatus       :     DRCB_STATUS_FAIL.
//
DWORD FatDeviceCreate(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev, __DRCB*          lpDrcb)
{
	CHAR                DirName[MAX_FILE_NAME_LEN]  = {0};
	CHAR                FileName[MAX_FILE_NAME_LEN] = {0};
	CHAR*               pszFileName                 = NULL;
	__DEVICE_OBJECT*    pFatDevice                  = (__DEVICE_OBJECT*)lpDev;
	__FAT32_SHORTENTRY  DirShortEntry               = {0};
	DWORD               dwDirCluster                = 0;
	DWORD               dwResult                    = 0;

	if((NULL == lpDev) || (NULL == lpDrcb))
	{
		goto __TERMINAL;
	}
	if(NULL == lpDrcb->lpInputBuffer)
	{
		goto __TERMINAL;
	}
	if(MAX_FILE_NAME_LEN - 1 < lpDrcb->dwInputLen)
	{
		goto __TERMINAL;
	}
	pszFileName = (CHAR*)lpDrcb->lpInputBuffer;
	if(!GetPathName(pszFileName,DirName,FileName))
	{
		return FALSE;
	}
	//Try to open the parent directory.
	if(!GetDirEntry((__FAT32_FS*)pFatDevice->lpDevExtension,	DirName,&DirShortEntry,NULL,NULL))
	{
		PrintLine("Can not get directory entry of parent dir.");
		return FALSE;
	}
	if(!(DirShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))  //Is not a directory.
	{
		PrintLine("The parent directory is not a directory.");
		return FALSE;
	}
	dwDirCluster =   DirShortEntry.wFirstClusHi;
	dwDirCluster <<= 16;
	dwDirCluster +=  DirShortEntry.wFirstClusLow;

	if(!CreateFatFile((__FAT32_FS*)pFatDevice->lpDevExtension,dwDirCluster,FileName,	0))
	{		
		goto __TERMINAL;
	}
		
	dwResult = 1;
__TERMINAL:
	return dwResult;
}

//Implementation of DeviceWrite routine.
DWORD FatDeviceWrite(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__FAT32_FS*             pFat32Fs       = NULL;
	__FAT32_FILE*           pFat32File     = NULL;
	__FAT32_SHORTENTRY*     pFat32Entry    = NULL;
	BYTE*                   pBuffer        = NULL;
	BYTE*                   pBufferEnd     = NULL;
	BYTE*                   pClusBuffer    = NULL;
	BYTE*                   pStart         = NULL;
	DWORD                   dwCurrPos      = 0;
	DWORD                   dwSector       = 0;
	DWORD                   dwNextClus     = 0;
	DWORD                   dwWriteSize    = 0;
	DWORD                   dwFirstCluster = 0;	
	DWORD                   dwOnceSize     = 0;
	DWORD                   dwWritten      = 0;    //Record the written size.
	

	if((NULL == lpDev) || (NULL == lpDrcb))
	{
		goto __TERMINAL;
	}

	pFat32File   = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	pFat32Fs     = pFat32File->pFileSystem;
	dwWriteSize  = lpDrcb->dwInputLen;
	pBuffer      = (BYTE*)lpDrcb->lpInputBuffer;
	pBufferEnd   = pBuffer + dwWriteSize;

	pClusBuffer  = (BYTE*) FatMem_Alloc(pFat32Fs->dwClusterSize);
	if(NULL == pClusBuffer)  //Can not allocate buffer.
	{
		goto __TERMINAL;
	}
	dwCurrPos  = pFat32File->dwCurrPos;
	dwNextClus = pFat32File->dwCurrClusNum;

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
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,	dwSector,pFat32Fs->SectorPerClus,pClusBuffer))
	{	
		goto __TERMINAL;
	}

	while(pBuffer < pBufferEnd)
	{
		pStart = pClusBuffer + pFat32File->dwClusOffset;
		dwOnceSize = pFat32Fs->dwClusterSize - pFat32File->dwClusOffset;
		if(dwOnceSize > dwWriteSize)
		{
			dwOnceSize = dwWriteSize;
		}

		memcpy(pStart,pBuffer,dwOnceSize);
		//Now write the cluster into memory.
		if(!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pClusBuffer))
		{
			PrintLine("  In FatDeviceWrite: Condition 4");
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
		pBuffer      += dwOnceSize;
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
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pClusBuffer))
	{
		goto __TERMINAL;
	}

	pFat32Entry = (__FAT32_SHORTENTRY*)(pClusBuffer + pFat32File->dwParentOffset);
	
	//modify  file start Cluster index 
	if(dwFirstCluster > 0 )
	{
		pFat32Entry->wFirstClusHi   = (WORD)(dwFirstCluster >> 16);
		pFat32Entry->wFirstClusLow  = (WORD)(dwFirstCluster&0x0000FFFF);
	}
	
	//update write date and time 
	SetFatFileDateTime(pFat32Entry,FAT32_DATETIME_WRITE);

	//pfse->
	pFat32Entry->dwFileSize = pFat32File->dwFileSize;
	WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pClusBuffer);
	lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;

__TERMINAL:
	if(pClusBuffer)
	{
		KMemFree(pClusBuffer,KMEM_SIZE_TYPE_ANY,0);
	}

	return dwWritten;
}


//Implementation of DeviceRead routine.
DWORD FatDeviceSize(__COMMON_OBJECT* lpDrv,	__COMMON_OBJECT* lpDev,	__DRCB* lpDrcb)
{
	__FAT32_FILE*          pFatFile     = NULL;
	DWORD                  dwFileSize   = 0;   
	
	if((NULL == lpDrv) || (NULL == lpDev))
	{
		return dwFileSize;
	}

	pFatFile = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	if( NULL != pFatFile)
	{
		dwFileSize =  pFatFile->dwFileSize;
	}

	return dwFileSize;	
}
//Implementation of DeviceRead routine.
DWORD FatDeviceRead(__COMMON_OBJECT* lpDrv,
		                   __COMMON_OBJECT* lpDev,
				           __DRCB* lpDrcb)
{
	__FAT32_FS*            pFatFs           = NULL;
	__FAT32_FILE*          pFatFile         = NULL;
	DWORD                  dwNextClus       = 0;
	DWORD                  dwSector         = 0;
	DWORD                  dwHasRead        = 0;
	DWORD                  dwToRead         = 0;
	DWORD                  dwTotalRead      = 0;
	BYTE*                  pBuffer          = NULL;  //Temporary buffer.
	BYTE*                  pStartCopy       = NULL;
	BYTE*                  pDestination     = NULL;
	BOOL                   bHasRead         = FALSE;
	//BYTE                   Buffer[128];

	//PrintLine("FatDeviceRead Call");
	if((NULL == lpDrv) || (NULL == lpDev))
	{
		PrintLine("FatDeviceRead error 1");
		goto __TERMINAL;
	}
	pFatFile = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	pFatFs   = pFatFile->pFileSystem;
	dwToRead = lpDrcb->dwOutputLen;  //dwOutputLen contains the desired read size.
	pDestination = (BYTE*)lpDrcb->lpOutputBuffer;
	//Check if request too much.
	if(dwToRead > pFatFile->dwFileSize - pFatFile->dwCurrPos)  //Exceed the file size.
	{
		dwToRead = (pFatFile->dwFileSize - pFatFile->dwCurrPos);
	}
	pBuffer = (BYTE*)FatMem_Alloc(pFatFs->dwClusterSize);
	if(NULL == pBuffer)  //Can not allocate memory.
	{
		_hx_printf("FatDeviceRead times:ClusterSize=%d",(INT)pFatFs->dwClusterSize);		
		goto __TERMINAL;
	}
	dwHasRead   = pFatFs->dwClusterSize;
	dwNextClus  = pFatFile->dwCurrClusNum;  //From current position.
	dwSector    = GetClusterSector(pFatFs,dwNextClus);  //Get the sector specific to dwNextClus.
	if(0 == dwSector)  //Can not get sector.
	{
		PrintLine("FatDeviceRead error 3");
		goto __TERMINAL;
	}
	while(dwToRead > dwHasRead)
	{
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
			dwSector,
			pFatFs->SectorPerClus,
			pBuffer))  //Can not read file data.
		{
			PrintLine("FatDeviceRead error= 4");
			goto __TERMINAL;
		}
		pStartCopy = pBuffer + pFatFile->dwClusOffset;
		memcpy(pDestination,pStartCopy,dwHasRead - pFatFile->dwClusOffset);
		pDestination += (dwHasRead - pFatFile->dwClusOffset);  //Update destination buffer.
		dwTotalRead  += (dwHasRead - pFatFile->dwClusOffset);
		//Update file related variables.
		dwToRead -= (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwCurrPos      += (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwClusOffset   =  0;
		//Update next cluster and the appropriate sector number.
		if(!GetNextCluster(pFatFs,&dwNextClus))
		{
			PrintLine("FatDeviceRead error 5");
			goto __TERMINAL;
		}
		pFatFile->dwCurrClusNum = dwNextClus;
		dwSector = GetClusterSector(pFatFs,dwNextClus);
		if(0 == dwSector)  //Invalid sector number.
		{
			PrintLine("FatDeviceRead error 6");
			goto __TERMINAL;
		}
	}
	//Handle the resting data.
	if(!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
		dwSector,
		pFatFs->SectorPerClus,
		pBuffer))
	{
		PrintLine("FatDeviceRead error 7");
		goto __TERMINAL;
	}
	pStartCopy = pBuffer + pFatFile->dwClusOffset;
	if(dwToRead < dwHasRead - pFatFile->dwClusOffset)
	{
		memcpy(pDestination,pStartCopy,dwToRead);
		pDestination += dwToRead;
		dwTotalRead  += dwToRead;
		pFatFile->dwClusOffset += dwToRead;
		pFatFile->dwClusOffset  = pFatFile->dwClusOffset % dwHasRead;  //Round to cluster size.
		//pFatFile->dwCurrClusNum += 0;  //No need to update current cluster number.
		pFatFile->dwCurrPos += dwToRead;
	}
	else  //Consider the desired reading size less than one cluster,but spans on 2 clusters.
	{
		memcpy(lpDrcb->lpOutputBuffer,pStartCopy,dwHasRead - pFatFile->dwClusOffset);
		pDestination += (dwHasRead - pFatFile->dwClusOffset);
		dwTotalRead  += (dwHasRead - pFatFile->dwClusOffset);
		dwToRead -= (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwCurrPos += (dwHasRead - pFatFile->dwClusOffset);  //Should be updated here.
		pFatFile->dwClusOffset += (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwClusOffset =  pFatFile->dwClusOffset % dwHasRead; //Round to cluster size.
		//Read the remainding section in next cluster.
		if(!GetNextCluster(pFatFs,&dwNextClus))
		{
			PrintLine("FatDeviceRead error 8");
			goto __TERMINAL;
		}
		pFatFile->dwCurrClusNum = dwNextClus;
		if(dwToRead != 0)  //Has not over.
		{
			dwSector = GetClusterSector(pFatFs,dwNextClus);
			if(0 == dwSector)
			{
				PrintLine("FatDeviceRead error 9");
				goto __TERMINAL;
			}
			if(!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
				dwSector,
				pFatFs->SectorPerClus,
				pBuffer))
			{
				PrintLine("FatDeviceRead error 10");
				goto __TERMINAL;
			}
			memcpy(lpDrcb->lpOutputBuffer,pBuffer,dwToRead);
			dwTotalRead += dwToRead;  //No need to update pDestination since no use further.
			//Update the file object's statistics variables.
			pFatFile->dwClusOffset  += dwToRead;
			pFatFile->dwClusOffset  =  pFatFile->dwClusOffset % dwHasRead; //Round to cluster size.
			pFatFile->dwCurrPos     += dwToRead;
		}
	}
__TERMINAL:

	
	FatMem_Free(pBuffer);

	return dwTotalRead;
}

#endif
