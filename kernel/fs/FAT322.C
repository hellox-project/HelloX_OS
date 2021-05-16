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

#include <StdAfx.h>
#include <stdio.h>

#include "fsstr.h"
#include "fat32.h"

#ifdef __CFG_SYS_DDF

/* 
 * A helper routine used to convert a string from 
 * lowercase to capital.
 */
void ToCapital(LPSTR lpszString)
{
	int nIndex = 0;

	BUG_ON(NULL == lpszString);
	while(lpszString[nIndex])
	{
		if((lpszString[nIndex] >= 'a') && (lpszString[nIndex] <= 'z'))
		{
			lpszString[nIndex] += 'A' - 'a';
		}
		nIndex++;
	}
}

/* 
 * Create a new file in the specified directory.
 * Input of lpDrcb:
 *   lpInputBuffer  :     Pointing to new file's name and path.
 *   dwInputLen     :     Target file name's length.
 *   dwExtraParam1  :     Target file's attributes.
 * Output if successful:
 *   lpOutputBuffer :     Pointing to the new file's handle(opened).
 *   dwStatus       :     DRCB_STATUS_SUCCESS.
 * Output if failed:
 *   dwStatus       :     DRCB_STATUS_FAIL.
 */
DWORD FatDeviceCreate(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	CHAR                DirName[MAX_FILE_NAME_LEN] = { 0 };
	CHAR                FileName[MAX_FILE_NAME_LEN] = { 0 };
	CHAR*               pszFileName = NULL;
	__DEVICE_OBJECT*    pFatDevice = (__DEVICE_OBJECT*)lpDev;
	__FAT32_SHORTENTRY  DirShortEntry = { 0 };
	DWORD               dwDirCluster = 0;
	DWORD               dwResult = 0;

	if ((NULL == lpDev) || (NULL == lpDrcb))
	{
		goto __TERMINAL;
	}
	if (NULL == lpDrcb->lpInputBuffer)
	{
		goto __TERMINAL;
	}
	if (MAX_FILE_NAME_LEN - 1 < lpDrcb->dwInputLen)
	{
		goto __TERMINAL;
	}
	pszFileName = (CHAR*)lpDrcb->lpInputBuffer;
	if (!GetPathName(pszFileName, DirName, FileName))
	{
		return FALSE;
	}
	/* Try to open the parent directory. */
	if (!GetDirEntry((__FAT32_FS*)pFatDevice->lpDevExtension, 
		DirName, &DirShortEntry, NULL, NULL))
	{
		PrintLine("Get parent dir fail.");
		return FALSE;
	}
	if (!(DirShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))
	{
		PrintLine("Not a valid dir entry.");
		return FALSE;
	}
	dwDirCluster = DirShortEntry.wFirstClusHi;
	dwDirCluster <<= 16;
	dwDirCluster += DirShortEntry.wFirstClusLow;

	if (!CreateFatFile((__FAT32_FS*)pFatDevice->lpDevExtension, dwDirCluster, FileName, 0))
	{
		_hx_printf("%s:failed to create file.\r\n", __func__);
		goto __TERMINAL;
	}

	dwResult = 1;
__TERMINAL:
	return dwResult;
}

/* Write data into the specified file,from current location. */
DWORD FatDeviceWrite(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__FAT32_FS* pFat32Fs = NULL;
	__FAT32_FILE* pFat32File = NULL;
	__FAT32_SHORTENTRY* pFat32Entry = NULL;
	BYTE* pBuffer = NULL, *pBufferEnd = NULL, *pClusBuffer = NULL;
	BYTE* pStart = NULL;
	unsigned long dwCurrPos = 0, dwSector = 0, dwNextClus = 0;
	unsigned long dwWriteSize = 0, dwFirstCluster = 0;
	unsigned long dwOnceSize = 0;
	unsigned long dwWritten = 0;

	BUG_ON((NULL == lpDev) || (NULL == lpDrcb));

	/* Get the file object, fs object and others. */
	pFat32File = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	BUG_ON(NULL == pFat32File);
	pFat32Fs = pFat32File->pFileSystem;
	BUG_ON(NULL == pFat32Fs);
	dwWriteSize = lpDrcb->dwInputLen;
	if (0 == dwWriteSize)
	{
		goto __TERMINAL;
	}
	pBuffer = (BYTE*)lpDrcb->lpInputBuffer;
	BUG_ON(NULL == pBuffer);
	pBufferEnd = pBuffer + dwWriteSize;

	/* Allocate a cluster buffer. */
	pClusBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
	if (NULL == pClusBuffer)
	{
		goto __TERMINAL;
	}
	dwCurrPos = pFat32File->dwCurrPos;
	dwNextClus = pFat32File->dwCurrClusNum;

	if (dwNextClus == 0 && GetFreeCluster(pFat32Fs, 0, &dwNextClus))
	{
		pFat32File->dwCurrClusNum = dwNextClus;
		pFat32File->dwStartClusNum = dwNextClus;
		dwFirstCluster = dwNextClus;
	}

	dwSector = GetClusterSector(pFat32Fs, dwNextClus);
	if (0 == dwSector)
	{
		goto __TERMINAL;
	}

	/* 
	 * Read current cluster into memory, since the 
	 * start position of writting may not begin from
	 * start of cluster, the data between cluster start
	 * and writting start should be preserved.
	 */
	if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition, dwSector, 
		pFat32Fs->SectorPerClus, pClusBuffer))
	{
		goto __TERMINAL;
	}

	while (pBuffer < pBufferEnd)
	{
		pStart = pClusBuffer + pFat32File->dwClusOffset;
		dwOnceSize = pFat32Fs->dwClusterSize - pFat32File->dwClusOffset;
		if (dwOnceSize > dwWriteSize)
		{
			dwOnceSize = dwWriteSize;
		}

		memcpy(pStart, pBuffer, dwOnceSize);
		/* Write data into storage. */
		if (!WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pClusBuffer))
		{
			_hx_printf("[%s]write sector fail\r\n", __func__);
			goto __TERMINAL;
		}
		/* Adjust file object's status. */
		pFat32File->dwClusOffset += dwOnceSize;
		pFat32File->dwCurrPos += dwOnceSize;

		/* Adjust file's size if necessary. */
		if (pFat32File->dwCurrPos >= pFat32File->dwFileSize)
		{
			pFat32File->dwFileSize = pFat32File->dwCurrPos;
		}

		if (0 == (pFat32File->dwClusOffset % pFat32Fs->dwClusterSize))
		{
			dwNextClus = pFat32File->dwCurrClusNum;
			if (!GetNextCluster(pFat32Fs, &dwNextClus))
			{
				goto __TERMINAL;
			}
			if (IS_EOC(dwNextClus))
			{
				/* Extend file if end of cluster. */
				if (!AppendClusterToChain(pFat32Fs, &pFat32File->dwCurrClusNum))
				{
					goto __TERMINAL;
				}
				dwNextClus = pFat32File->dwCurrClusNum;
			}
			pFat32File->dwCurrClusNum = dwNextClus;
			pFat32File->dwClusOffset = 0;
			/* Update dwSector to corespond current cluster. */
			dwSector = GetClusterSector(pFat32Fs, dwNextClus);
			if (0 == dwSector)
			{
				goto __TERMINAL;
			}
		}
		/* 
		 * Adjust the buffer position and 
		 * local control variables. 
		 */
		pBuffer += dwOnceSize;
		dwWritten += dwOnceSize;
		dwWriteSize -= dwOnceSize;
		if (0 == dwWriteSize)
		{
			/* Write over. */
			break;
		}
	}

	/* Update file's corresponding dir entry. */
	dwSector = GetClusterSector(pFat32Fs, pFat32File->dwParentClus);
	if (0 == dwSector)
	{
		goto __TERMINAL;
	}
	if (!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pClusBuffer))
	{
		goto __TERMINAL;
	}

	pFat32Entry = (__FAT32_SHORTENTRY*)(pClusBuffer + pFat32File->dwParentOffset);

	/* Update first cluster's value. */
	if (dwFirstCluster > 0)
	{
		pFat32Entry->wFirstClusHi = (WORD)(dwFirstCluster >> 16);
		pFat32Entry->wFirstClusLow = (WORD)(dwFirstCluster & 0x0000FFFF);
	}

	/* Update the last writting date time of the file. */
	SetFatFileDateTime(pFat32Entry, FAT32_DATETIME_WRITE);

	/* Update file's size attribute. */
	pFat32Entry->dwFileSize = pFat32File->dwFileSize;
	WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pClusBuffer);
	lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;

__TERMINAL:
	if (pClusBuffer)
	{
		KMemFree(pClusBuffer, KMEM_SIZE_TYPE_ANY, 0);
	}

	return dwWritten;
}

/* Return the specified file's size. */
DWORD FatDeviceSize(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__FAT32_FILE*          pFatFile = NULL;
	DWORD                  dwFileSize = 0;

	BUG_ON((NULL == lpDrv) || (NULL == lpDev));

	pFatFile = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	if (NULL != pFatFile)
	{
		dwFileSize = pFatFile->dwFileSize;
	}

	return dwFileSize;
}

/* 
 * Read file from a specified file object.
 * The object already opened,maybe by CreateFile routine,it's
 * handle is specified by lpDev.
 */
DWORD FatDeviceRead(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__FAT32_FS* pFatFs = NULL;
	__FAT32_FILE* pFatFile = NULL;
	uint32_t dwNextClus = 0, dwSector = 0, dwHasRead = 0, dwToRead = 0;
	DWORD dwTotalRead = 0;
	BYTE* pBuffer = NULL;
	BYTE* pStartCopy = NULL;
	BYTE* pDestination = NULL;
	BOOL bHasRead = FALSE;

	/* Parameters checking. */
	if ((NULL == lpDrv) || (NULL == lpDev))
	{
		goto __TERMINAL;
	}

	pFatFile = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	pFatFs = pFatFile->pFileSystem;
	dwToRead = lpDrcb->dwOutputLen;  //dwOutputLen contains the desired read size.
	pDestination = (BYTE*)lpDrcb->lpOutputBuffer;

	/*
	 * If the requested data size exceeds the file size minus current pointer,
	 * then just feed back the rest data(file size - current pointer).
	 */
	if (dwToRead > pFatFile->dwFileSize - pFatFile->dwCurrPos)
	{
		dwToRead = (pFatFile->dwFileSize - pFatFile->dwCurrPos);
	}

	pBuffer = (BYTE*)FatMem_Alloc(pFatFs->dwClusterSize);
	if (NULL == pBuffer)
	{
		goto __TERMINAL;
	}
	dwHasRead = pFatFs->dwClusterSize;
	dwNextClus = pFatFile->dwCurrClusNum;
	dwSector = GetClusterSector(pFatFs, dwNextClus);
	if (0 == dwSector)
	{
		goto __TERMINAL;
	}
	while (dwToRead > dwHasRead)
	{
		if (!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
			dwSector,
			pFatFs->SectorPerClus,
			pBuffer))
		{
			_hx_printf("[%s]read sector fail.\r\n", __func__);
			goto __TERMINAL;
		}
		pStartCopy = pBuffer + pFatFile->dwClusOffset;
		memcpy(pDestination, pStartCopy, dwHasRead - pFatFile->dwClusOffset);
		/* Update destination buffer. */
		pDestination += (dwHasRead - pFatFile->dwClusOffset);
		dwTotalRead += (dwHasRead - pFatFile->dwClusOffset);
		//Update file related variables.
		dwToRead -= (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwCurrPos += (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwClusOffset = 0;
		//Update next cluster and the appropriate sector number.
		if (!GetNextCluster(pFatFs, &dwNextClus))
		{
			_hx_printf("[%s]get next cluster fail.\r\n", __func__);
			goto __TERMINAL;
		}
		pFatFile->dwCurrClusNum = dwNextClus;
		dwSector = GetClusterSector(pFatFs, dwNextClus);
		if (0 == dwSector)
		{
			_hx_printf("[%s]get cluster sector fail.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	/* Handle the rest data. */
	if (!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
		dwSector,
		pFatFs->SectorPerClus,
		pBuffer))
	{
		_hx_printf("[%s]read fail,dwSector = %d,SectorPerClus = %d,pBuffer = 0x%X.\r\n",
			__func__, dwSector, pFatFs->SectorPerClus, pBuffer);
		goto __TERMINAL;
	}
	pStartCopy = pBuffer + pFatFile->dwClusOffset;
	if (dwToRead < dwHasRead - pFatFile->dwClusOffset)
	{
		memcpy(pDestination, pStartCopy, dwToRead);
		pDestination += dwToRead;
		dwTotalRead += dwToRead;
		pFatFile->dwClusOffset += dwToRead;
		/* Round to cluster size. */
		pFatFile->dwClusOffset %= dwHasRead;
		pFatFile->dwCurrPos += dwToRead;
	}
	/*
	 * Consider the requested reading size less than one cluster,
	 * but spans on 2 clusters,since the current cluster offset value
	 * is near the value of one cluster size.
	 */
	else
	{
		memcpy(pDestination, pStartCopy, dwHasRead - pFatFile->dwClusOffset);
		pDestination += (dwHasRead - pFatFile->dwClusOffset);
		dwTotalRead += (dwHasRead - pFatFile->dwClusOffset);
		dwToRead -= (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwCurrPos += (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwClusOffset += (dwHasRead - pFatFile->dwClusOffset);
		pFatFile->dwClusOffset %= dwHasRead; //Round to cluster size.
		//Read the remainding section in next cluster.
		if (!GetNextCluster(pFatFs, &dwNextClus))
		{
			PrintLine("FatDeviceRead error 8");
			goto __TERMINAL;
		}
		pFatFile->dwCurrClusNum = dwNextClus;
		if (dwToRead != 0)  //Has not over.
		{
			dwSector = GetClusterSector(pFatFs, dwNextClus);
			if (0 == dwSector)
			{
				PrintLine("FatDeviceRead error 9");
				goto __TERMINAL;
			}
			if (!ReadDeviceSector((__COMMON_OBJECT*)pFatFile->pPartition,
				dwSector,
				pFatFs->SectorPerClus,
				pBuffer))
			{
				PrintLine("FatDeviceRead error 10");
				goto __TERMINAL;
			}
			memcpy(pDestination, pBuffer, dwToRead);
			dwTotalRead += dwToRead;  //No need to update pDestination since no use further.
			//Update the file object's statistics variables.
			pFatFile->dwClusOffset += dwToRead;
			pFatFile->dwClusOffset = pFatFile->dwClusOffset % dwHasRead; //Round to cluster size.
			pFatFile->dwCurrPos += dwToRead;
		}
	}

__TERMINAL:
	FatMem_Free(pBuffer);
	return dwTotalRead;
}

#endif //__SYS_CFG_DDF
