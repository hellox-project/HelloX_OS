//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 07 JAN,2008
//    Module Name               : FATMGR2.CPP
//    Module Funciton           : 
//                                FAT manager implementation code in this file.
//                                FAT manager is a object to manage FAT table 
//                                for FAT file system.
//                                In order to reduce the size of one module(file),
//                                FAT related code is separated into several 
//                                parts,named FATMGRx.CPP,where x start from
//                                2,and one file FATMGR.CPP,is the initial part of 
//                                FAT driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>

#include "fsstr.h"
#include "fat32.h"

/* Available if and only if the DDF function is enabled. */
#ifdef __CFG_SYS_DDF

/* 
 * Read one or several sector(s) from device object.
 * This is the lowest level routine used by all FAT32 driver code.
 */
BOOL ReadDeviceSector(__COMMON_OBJECT* pPartition,
	DWORD            dwStartSector,
	DWORD            dwSectorNum,
	BYTE*            pBuffer)
{
	BOOL              bResult = FALSE;
	__DRIVER_OBJECT*  pDrvObject = NULL;
	__DEVICE_OBJECT*  pDevObject = (__DEVICE_OBJECT*)pPartition;
	__DRCB*           pDrcb = NULL;

	if ((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	/* New a drcb object to carry request. */
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		goto __TERMINAL;
	}

	if (DEVICE_OBJECT_SIGNATURE != pDevObject->dwSignature)
	{
		PrintLine("Invalid device object encountered.");
		goto __TERMINAL;
	}

	//Initialize the DRCB object.
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_READ_SECTOR;
	pDrcb->dwInputLen = sizeof(DWORD);
	pDrcb->lpInputBuffer = (LPVOID)&dwStartSector;    //Input buffer stores the start position pointer.
	pDrcb->dwOutputLen = dwSectorNum * (pDevObject->dwBlockSize);
	pDrcb->lpOutputBuffer = pBuffer;

	//Issue the IO control command to read data.
	if (0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))
	{
		/* Failed to read data from physical device. */
		goto __TERMINAL;
	}
	/* Read OK. */
	bResult = TRUE;

__TERMINAL:
	if (pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* Write one or several sector(s) to device. */
BOOL WriteDeviceSector(__COMMON_OBJECT* pPartition,
	DWORD dwStartSector,
	DWORD dwSectorNum,
	BYTE* pBuffer)
{
	BOOL bResult = FALSE;
	__DRIVER_OBJECT* pDrvObject = NULL;
	__DEVICE_OBJECT* pDevObject = (__DEVICE_OBJECT*)pPartition;
	__DRCB* pDrcb = NULL;
	__SECTOR_INPUT_INFO ssi;

	/* Validate all parameters first. */
	if ((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	/* New a drcb object to carry request. */
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		goto __TERMINAL;
	}

	ssi.dwBufferLen = dwSectorNum * pDevObject->dwBlockSize;
	ssi.lpBuffer = pBuffer;
	ssi.dwStartSector = dwStartSector;
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_WRITE_SECTOR;
	pDrcb->dwInputLen = sizeof(__SECTOR_INPUT_INFO);
	pDrcb->lpInputBuffer = (LPVOID)&ssi;
	pDrcb->dwOutputLen = 0;
	pDrcb->lpOutputBuffer = NULL;

	/* Issue the IO control command to device to read data. */
	if (0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if (pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* 
 * Initialize one FAT32 file system given the first sector data.
 * Return TRUE if sucessfully.
 */
BOOL Fat32Init(__FAT32_FS* pFat32Fs, BYTE* pSector0)
{
	UCHAR*    pStart = (UCHAR*)pSector0;
	BOOL      bResult = FALSE;
	DWORD     dwCluster = 0;
	WORD      RootDirSector = 0;
	WORD      wRootEntryCnt = 0;
	DWORD     FATSz = 0;
	DWORD     TotSec = 0;
	DWORD     DataSec = 0;
	DWORD     CountOfCluster = 0;

	BUG_ON((NULL == pFat32Fs) || (NULL == pSector0));
	/* Verify the signature of sector. */
	if ((0x55 != pStart[510]) || (0xAA != pStart[511]))
	{
		_hx_printf("[%s]Invalid partition.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Initialize the FAT32 extension object,pFat32Fs. */
	pFat32Fs->dwAttribute = FILE_SYSTEM_TYPE_FAT32;
	pFat32Fs->SectorPerClus = pStart[BPB_SecPerClus];
	pFat32Fs->wReservedSector = *(WORD*)(pStart + BPB_RsvdSecCnt);
	pFat32Fs->FatNum = *(pStart + BPB_NumFATs);
	pFat32Fs->dwFatSectorNum = *(DWORD*)(pStart + BPB_FAT32_FATSz32);
	pFat32Fs->dwRootDirClusStart = *(DWORD*)(pStart + BPB_FAT32_RootClus);
	pFat32Fs->wFatInfoSector = *(WORD*)(pStart + BPB_FAT32_FSInfo);
	pFat32Fs->dwFatBeginSector = (DWORD)pFat32Fs->wReservedSector;
	pFat32Fs->dwDataSectorStart = pFat32Fs->dwFatBeginSector + (pFat32Fs->FatNum * pFat32Fs->dwFatSectorNum);
	pFat32Fs->dwBytePerSector = (DWORD)(*(WORD*)(pStart + BPB_BytsPerSec));
	pFat32Fs->dwClusterSize = pFat32Fs->dwBytePerSector * pFat32Fs->SectorPerClus;
	pFat32Fs->pFileList = NULL;

	if (STORAGE_DEFAULT_SECTOR_SIZE != pFat32Fs->dwBytePerSector)
	{
		_hx_printf("[%s]invalid byte/sector value[%d].\r\n",
			__func__, pFat32Fs->dwBytePerSector);
		goto __TERMINAL;
	}
	if (0 == pFat32Fs->SectorPerClus)
	{
		_hx_printf("[%s]invalid sector/clus value[%d].\r\n",
			__func__, pFat32Fs->SectorPerClus);
		goto __TERMINAL;
	}

	/* Check the partition type, only FAT32 is supported. */
	wRootEntryCnt = *(WORD*)(pStart + BPB_RootEntCnt);
	wRootEntryCnt *= 32;
	wRootEntryCnt += (WORD)pFat32Fs->dwBytePerSector - 1;
	/* Get root directory's sector num. */
	RootDirSector = wRootEntryCnt / (WORD)pFat32Fs->dwBytePerSector;

	/* According FAT file system specification. */
	if (*(WORD*)(pStart + BPB_FATSz16) != 0)
	{
		FATSz = (DWORD)(*(WORD*)(pStart + BPB_FATSz16));
	}
	else
	{
		FATSz = *(DWORD*)(pStart + BPB_FAT32_FATSz32);
	}

	if (*(WORD*)(pStart + BPB_TotSec16) != 0)
	{
		TotSec = (DWORD)(*(WORD*)(pStart + BPB_TotSec16));
	}
	else
	{
		TotSec = *(DWORD*)(pStart + BPB_TotSec32);
	}

	/* Get the data sector number. */
	DataSec = TotSec - (DWORD)pFat32Fs->wReservedSector;
	DataSec -= (FATSz * pFat32Fs->FatNum);
	DataSec -= RootDirSector;
	/* Get the total cluster counter. */
	CountOfCluster = DataSec / pFat32Fs->SectorPerClus;

	/* Can not support FAT12/FAT16 yet. */
	if (CountOfCluster < 4085)
	{
		_hx_printf("[%s]only FAT32 supported.\r\n", __func__);
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#endif
