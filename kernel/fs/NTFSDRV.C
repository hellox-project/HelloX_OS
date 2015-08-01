//------------------------------------------------------------------------
//  NTFS file system implementation code.
//
//  Author                 : Garry.Xin
//  Initial date           : Dec 10,2011
//  Last updated           : Dec 10,2011
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif
#ifndef __NTFS_H__
#include "ntfs.h"
#endif

#ifndef __FSSTR_H__
#include "fsstr.h"
#endif

#include "../lib/stdio.h"
#include "../lib/string.h"



//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//A local helper routine used to dumpout file system's key parameter.
static void DumpFileSystem(__NTFS_FILE_SYSTEM* pFileSystem)
{
	CHAR    buff[512];

	PrintLine("  The file system's key information as follows:");
    PrintLine("  -----------------------------------");
    _hx_sprintf(buff,"  Serial number    : %X%X%X%X-%X%X%X%X",
		pFileSystem->serialNumber[0],
        pFileSystem->serialNumber[1],
        pFileSystem->serialNumber[2],
        pFileSystem->serialNumber[3],
        pFileSystem->serialNumber[4],
        pFileSystem->serialNumber[5],
        pFileSystem->serialNumber[6],
        pFileSystem->serialNumber[7]);
	PrintLine(buff);
    _hx_sprintf(buff,"  Bytes per sector : %d",pFileSystem->bytesPerSector);
	PrintLine(buff);
    _hx_sprintf(buff,"  Sector per clus  : %d",pFileSystem->sectorPerClus);
	PrintLine(buff);
    _hx_sprintf(buff,"  Hidden sector    : %d",pFileSystem->hiddenSector);
	PrintLine(buff);
    _hx_sprintf(buff,"  Total sector num : %d",pFileSystem->totalSectorLow);
	PrintLine(buff);
    _hx_sprintf(buff,"  MFT start clust  : %d",pFileSystem->mftStartClusLow);
	PrintLine(buff);
    _hx_sprintf(buff,"  Cluster size     : %d",pFileSystem->clusSize);
	PrintLine(buff);
    _hx_sprintf(buff,"  File record size : %d",pFileSystem->FileRecordSize);
	PrintLine(buff);
    _hx_sprintf(buff,"  Dir record size  : %d",pFileSystem->DirBlockSize);
	PrintLine(buff);
    //DumpFileObject(pFileSystem->pMFT);
    //DumpFileObject(pFileSystem->pRootDir);
}

//Implementation of CheckPartiton routine.This routine will be called by
//IOManager when a storage partition object is created in system.The NTFS
//file system driver code should detect if the partition is formated as 
//NTFS file system,and should create a NTFS device object if so,RegisterFileSystem
//also should be called under this scenario to tell system that a NTFS partition
//existing.
static BOOL CheckPartition(__COMMON_OBJECT* pThis,__COMMON_OBJECT* pPartitionObject)
{
	__DEVICE_OBJECT*       pNtfsDeviceObject      = NULL;  //NTFS device object.
	__DEVICE_OBJECT*       pPartitionDevice       = (__DEVICE_OBJECT*)pPartitionObject;
	__NTFS_FILE_SYSTEM*    pFileSystem            = NULL;  //NTFS file system object.
	CHAR                   DevName[64];                    //NTFS device's name.
	int                    nIndex                 = 0;     //Used to locate device name number.
	static int             nNameIndex             = 0;     //Start name index number.
	BOOL                   bResult                = FALSE;
	BYTE*                  pSector0               = NULL;  //Buffer to contain sector of this part.

	if(NULL == pPartitionDevice)
	{
		goto __TERMINAL;
	}
	//Allocate buffer.
	pSector0 = (BYTE*)KMemAlloc(SECTOR_SIZE,KMEM_SIZE_TYPE_ANY);
	if(NULL == pSector0)
	{
		PrintLine("NTFS CheckPartition : Can not allocate memory for reading sector0.");
		goto __TERMINAL;
	}
	//Read the first sector of partition object now.
	if(!NtfsReadDeviceSector(pPartitionObject,
		0,
		1,
		pSector0))
	{
		PrintLine("NTFS CheckPartition : Can not read first sector from partition object.");
		goto __TERMINAL;
	}
	//OK,try to create NTFS object.
	pFileSystem = CreateNtfsFileSystem(pPartitionObject,pSector0);
	if(NULL == pFileSystem)
	{
		PrintLine("NTFS CheckPartition : Can not create file system object.");
		goto __TERMINAL;
	}
	//NTFS file system object created successfully and we should create the corresponding
	//device object.
	strcpy(DevName,NTFS_DEVICE_NAME_BASE);
	nIndex = strlen(DevName);
	DevName[nIndex - 1] += nNameIndex;
	nNameIndex ++;
	pNtfsDeviceObject = IOManager.CreateDevice(
		(__COMMON_OBJECT*)&IOManager,
		DevName,
		DEVICE_TYPE_NTFS,
		DEVICE_BLOCK_SIZE_INVALID,
		DEVICE_BLOCK_SIZE_INVALID,
		DEVICE_BLOCK_SIZE_INVALID,
		(LPVOID)pFileSystem,
		((__DEVICE_OBJECT*)pThis)->lpDriverObject);
	if(NULL == pNtfsDeviceObject)
	{
		PrintLine("NTFS CheckPartition : Can not create device object.");
		goto __TERMINAL;
	}
	//Now Add the file system object to system.
	if(!IOManager.AddFileSystem(
		(__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)pNtfsDeviceObject,
		0,
		pFileSystem->volLabel))
	{
		goto __TERMINAL;
	}
	//Everything is OK.
	bResult = TRUE;

__TERMINAL:
	if(NULL != pSector0)
	{
		KMemFree(pSector0,KMEM_SIZE_TYPE_ANY,0);
	}
	if(!bResult)
	{
		if(NULL != pNtfsDeviceObject)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				pNtfsDeviceObject);
		}
		if(pFileSystem)
		{
			DestroyNtfsFileSystem(pFileSystem);
		}
	}
	return bResult;
}

//Implementation of _FindFirstFile,it is only a wrapper of NtfsFindFirstFile routine.
static DWORD _FindFirstFile(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	WCHAR               dirName[NTFS_FILENAME_SIZE];
	__NTFS_FILE_SYSTEM* pFileSystem = NULL;
	__NTFS_FIND_HANDLE* pFindHandle = NULL;

	//Convert traditional string to unicode format.
	byte2unicode(dirName,(const char*)pDrcb->dwExtraParam1);
	tocapital(dirName);
	pFileSystem = (__NTFS_FILE_SYSTEM*)((__DEVICE_OBJECT*)pDev)->lpDevExtension;
	pFindHandle = NtfsFindFirstFile(pFileSystem,dirName,
		(FS_FIND_DATA*)pDrcb->dwExtraParam2);
	if(pFindHandle)
	{
		pDrcb->lpOutputBuffer = (LPVOID)pFindHandle;
		return 1;
	}
	return 0;
}

//Implementation of _FindNextFile.
static DWORD _FindNextFile(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	if(NtfsFindNextFile((__NTFS_FIND_HANDLE*)pDrcb->lpInputBuffer,
		(FS_FIND_DATA*)pDrcb->dwExtraParam2))
	{
		return 1;
	}
	return 0;
}

//Implementation of _FindClose routine.
static DWORD _FindClose(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	NtfsCloseFind((__NTFS_FIND_HANDLE*)pDrcb->lpInputBuffer);
	return 1;
}

//Implementation of DeviceOpen routine,this routine will be called by IOManager to
//open a existing file.
//It's only the wrapper of NtfsCreateFile in current version.
static __COMMON_OBJECT* NtfsDeviceOpen(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	WCHAR                   FullName[NTFS_FILENAME_SIZE];
	__NTFS_FILE_SYSTEM*     pFileSystem      = NULL;
	__NTFS_FILE_OBJECT*     pFileObject      = NULL;
	__DEVICE_OBJECT*        pFileDevice      = NULL;
	BOOL                    bResult          = FALSE;
	CHAR                    FileDevName[16];
	static CHAR             nameIndex        = 0;

	if((NULL == pDev) || (NULL == pDrcb))
	{
		goto __TERMINAL;
	}
	//Get file's name.
	byte2unicode(FullName,(const char*)pDrcb->lpInputBuffer);
	tocapital(FullName);
	pFileSystem = (__NTFS_FILE_SYSTEM*)((__DEVICE_OBJECT*)pDev)->lpDevExtension;
	pFileObject = NtfsCreateFile(pFileSystem,FullName);
	if(NULL == pFileObject)  //Failed to open file.
	{
		goto __TERMINAL;
	}
	//OK,now create the corresponding device object.But we should first construct the
	//device's name before create it.
	strcpy(FileDevName,NTFS_FILE_NAME_BASE);
	FileDevName[14] += nameIndex ++;
	pFileDevice = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		FileDevName,
		DEVICE_TYPE_FILE,
		1, //For file,block size is 1.
		DEVICE_BLOCK_SIZE_ANY,
		DEVICE_BLOCK_SIZE_ANY,
		pFileObject,
		(__DRIVER_OBJECT*)pDrv);
	if(NULL == pFileDevice)
	{
		goto __TERMINAL;
	}
	//Everything is OK.
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(NULL != pFileObject)
		{
			NtfsDestroyFile(pFileObject);
		}
	}
	return (__COMMON_OBJECT*)pFileDevice;
}

//Implementation of DeviceClose routine,this routine will be called by
//application or IOManager to destroy one file.
static DWORD NtfsDeviceClose(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	__NTFS_FILE_OBJECT*      pFileObject   = NULL;
	__DEVICE_OBJECT*         pDeviceObject = (__DEVICE_OBJECT*)pDev;

	if((NULL == pDev) || (NULL == pDrcb))
	{
		return 0;
	}
	pFileObject = (__NTFS_FILE_OBJECT*)pDeviceObject->lpDevExtension;
	NtfsDestroyFile(pFileObject);
	//Destroy the device object.
	IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
		pDeviceObject);
	return 1;
}

//Implementation of DeviceRead routine.
static DWORD NtfsDeviceRead(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	__NTFS_FILE_OBJECT*     pFileObject      = NULL;
	BYTE*                   pBuffer          = NULL;
	UINT_32                 toReadSize       = 0;
	UINT_32                 ReadSize         = 0;

	if((NULL == pDev) || (NULL == pDrcb))
	{
		return 0;
	}
	pFileObject = (__NTFS_FILE_OBJECT*)((__DEVICE_OBJECT*)pDev)->lpDevExtension;
	toReadSize  = pDrcb->dwOutputLen;
	pBuffer     = (BYTE*)pDrcb->lpOutputBuffer;
	if(NtfsReadFile(pFileObject,pBuffer,toReadSize,&ReadSize,NULL))
	{
		return ReadSize;
	}
	else
	{
		return 0;
	}
}

//Implementation of DeviceSeek routine.
static DWORD NtfsDeviceSeek(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	__NTFS_FILE_OBJECT*     pFileObject = NULL;

	if((NULL == pDev) || (NULL == pDrcb))
	{
		return 0;
	}
	pFileObject = (__NTFS_FILE_OBJECT*)((__DEVICE_OBJECT*)pDev)->lpDevExtension;
	return NtfsSetFilePointer(pFileObject,
		pDrcb->dwExtraParam1,
		NULL,
		0);
}

//Implementation of GetFileAttribute.
static DWORD _GetFileAttributes(__COMMON_OBJECT* pDrv,__COMMON_OBJECT* pDev,__DRCB* pDrcb)
{
	__NTFS_FILE_SYSTEM*   pFileSystem  = NULL;
	__NTFS_FILE_OBJECT*   pFileObject  = NULL;
	WCHAR                 FullName[NTFS_FILENAME_SIZE];
	DWORD                 dwAttribute  = 0;

	if((NULL == pDev) || (NULL == pDrcb))
	{
		return 0;
	}
	pFileSystem = (__NTFS_FILE_SYSTEM*)((__DEVICE_OBJECT*)pDev)->lpDevExtension;
	byte2unicode(FullName,(const char*)pDrcb->dwExtraParam1);
	tocapital(FullName);
	pFileObject = NtfsCreateFile(pFileSystem,FullName);
	if(NULL == pFileObject)
	{
		return 0;
	}
	dwAttribute = NtfsGetFileAttr(pFileObject);
	if(dwAttribute & 0x10000000)  //Is a directory,should convert to standard file attribute value.
	{
		dwAttribute |= 0x10;
	}
	pDrcb->dwStatus = DRCB_STATUS_SUCCESS;
	pDrcb->dwExtraParam2 = dwAttribute;
	//Destroy the file object,if it is not the ROOT and MFT file object.
	if((pFileObject != pFileSystem->pRootDir) && (pFileObject != pFileSystem->pMFT))
	{
		NtfsDestroyFile(pFileObject);
	}
	return 1;
}

//Implementation of DeviceCtrl routine,this routine will be called
//by applications through IOControl interface routine of IOManager.
static DWORD NtfsDeviceCtrl(__COMMON_OBJECT* lpDrv,
						   __COMMON_OBJECT* lpDev,
						   __DRCB* lpDrcb)
{
	__COMMON_OBJECT*       pFindHandle = NULL;

	if((NULL == lpDev) || (NULL == lpDrcb))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	//Dispatch the request to appropriate routines according to control command.
	switch(lpDrcb->dwCtrlCommand)
	{
	case IOCONTROL_FS_CHECKPARTITION:
		return CheckPartition(lpDev,(__COMMON_OBJECT*)lpDrcb->lpInputBuffer) ? 1 : 0;
	case IOCONTROL_FS_FINDFIRSTFILE:
		return _FindFirstFile(lpDrv,lpDev,lpDrcb);
	case IOCONTROL_FS_FINDNEXTFILE:
		return _FindNextFile(lpDrv,lpDev,lpDrcb);
	case IOCONTROL_FS_FINDCLOSE:
		return _FindClose(lpDrv,lpDev,lpDrcb);
	case IOCONTROL_FS_CREATEDIR:
		break;
	case IOCONTROL_FS_GETFILEATTR:
		return _GetFileAttributes(lpDrv,lpDev,lpDrcb);
	case IOCONTROL_FS_DELETEFILE:
	case IOCONTROL_FS_REMOVEDIR:
	default:
		break;
	}

__TERMINAL:
	return 0;
}


//Implementation of NtfsDriverEntry routine for NTFS file system.
//This routine will be placed in DriverEntryArray in DrvEntry.CPP file,and
//NTFS file system's driver will be loaded when OS start.
BOOL NtfsDriverEntry(__DRIVER_OBJECT* lpDriverObject)
{
	__DEVICE_OBJECT*  pNtfsObject  = NULL;

	//Initialize the driver object.
	lpDriverObject->DeviceClose   = NtfsDeviceClose;
	lpDriverObject->DeviceCtrl    = NtfsDeviceCtrl;
	lpDriverObject->DeviceFlush   = NULL; //NtfsDeviceFlush;
	lpDriverObject->DeviceOpen    = NtfsDeviceOpen;
	lpDriverObject->DeviceRead    = NtfsDeviceRead;
	lpDriverObject->DeviceSeek    = NtfsDeviceSeek;
	lpDriverObject->DeviceWrite   = NULL; //NtfsDeviceWrite;
	lpDriverObject->DeviceCreate  = NULL; //NtfsDeviceCreate;

	//Create FAT file system driver object now.
	pNtfsObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		NTFS_DRIVER_DEVICE_NAME,
		DEVICE_TYPE_FSDRIVER, //Attribute,this is a file system driver object.
		DEVICE_BLOCK_SIZE_INVALID, //File system driver object can not be read or written.
		DEVICE_BLOCK_SIZE_INVALID,
		DEVICE_BLOCK_SIZE_INVALID,
		NULL,  //With out any extension.
		lpDriverObject);
	if(NULL == pNtfsObject)  //Can not create file system object.
	{
		return FALSE;
	}
	//Register file system now.
	if(!IOManager.RegisterFileSystem((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)pNtfsObject))
	{
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			pNtfsObject);
		return FALSE;
	}
	return TRUE;
}

#endif
