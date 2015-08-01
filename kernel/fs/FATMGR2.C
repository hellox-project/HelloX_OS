//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 07 JAN,2008
//    Module Name               : FATMGR2.CPP
//    Module Funciton           : 
//                                FAT manager implementation code in this file.
//                                FAT manager is a object to manage FAT table for FAT file
//                                system.
//                                In order to reduce the size of one module(file),FAT related code
//                                is separated into several parts,named FATMGRx.CPP,where x start from
//                                2,and one file FATMGR.CPP,is the initial part of FAT driver.
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

//Read one or several sector(s) from device object.
//This is the lowest level routine used by all FAT32 driver code.
BOOL ReadDeviceSector(__COMMON_OBJECT* pPartition,
					  DWORD            dwStartSector,
					  DWORD            dwSectorNum,   //How many sector to read.
					  BYTE*            pBuffer)       //Must equal or larger than request.
{
	BOOL              bResult        = FALSE;
	__DRIVER_OBJECT*  pDrvObject     = NULL;
	__DEVICE_OBJECT*  pDevObject     = (__DEVICE_OBJECT*)pPartition;
	__DRCB*           pDrcb          = NULL;

	if((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	pDrcb = (__DRCB*)CREATE_OBJECT(__DRCB);
	if(NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pDevObject->dwSignature)
	{
		PrintLine("Invalid device object encountered.");
		goto __TERMINAL;
	}
	//Initialize the DRCB object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_READ_SECTOR;
	pDrcb->dwInputLen      = sizeof(DWORD);
	pDrcb->lpInputBuffer   = (LPVOID)&dwStartSector;    //Input buffer stores the start position pointer.
	pDrcb->dwOutputLen     = dwSectorNum * (pDevObject->dwBlockSize);
	pDrcb->lpOutputBuffer  = pBuffer;
	//Issue the IO control command to read data.
	if(0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))  //Can not read.
	{
		goto __TERMINAL;
	}
	bResult = TRUE;  //Indicate read successfully.

__TERMINAL:
	if(pDrcb)  //Should release it.
	{
		RELEASE_OBJECT(pDrcb);
	}
	return bResult;
}

//Write one or several sector(s) to device.
BOOL WriteDeviceSector(__COMMON_OBJECT* pPartition,
					  DWORD            dwStartSector,
					  DWORD            dwSectorNum,   //How many sector to write.
					  BYTE*            pBuffer)       //Must equal or larger than request.
{
	BOOL              bResult        = FALSE;
	__DRIVER_OBJECT*  pDrvObject     = NULL;
	__DEVICE_OBJECT*  pDevObject     = (__DEVICE_OBJECT*)pPartition;
	__DRCB*           pDrcb          = NULL;
	__SECTOR_INPUT_INFO ssi;

	if((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	pDrcb = (__DRCB*)CREATE_OBJECT(__DRCB);
	if(NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	ssi.dwBufferLen   = dwSectorNum * pDevObject->dwBlockSize;
	ssi.lpBuffer      = pBuffer;
	ssi.dwStartSector = dwStartSector;
	//Initialize the DRCB object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_WRITE_SECTOR;
	pDrcb->dwInputLen      = sizeof(__SECTOR_INPUT_INFO);
	pDrcb->lpInputBuffer   = (LPVOID)&ssi;
	pDrcb->dwOutputLen     = 0;
	pDrcb->lpOutputBuffer  = NULL;
	//Issue the IO control command to read data.
	if(0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))  //Can not read.
	{
		goto __TERMINAL;
	}
	bResult = TRUE;  //Indicate read successfully.

__TERMINAL:
	if(pDrcb)  //Should release it.
	{
		RELEASE_OBJECT(pDrcb);
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
		PrintLine("In Fat32Init: byte per sector is zero.");
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
		//This partition is FAT12,we can not handle it currently.
		goto __TERMINAL;
	}
	
	//The FAT partition is FAT32.

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#endif
