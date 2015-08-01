//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 28 DEC, 2008
//    Module Name               : FAT32.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of
//                                FAT32 file system.
//                                The day before yesterday I got the TOEIC testing
//                                result,which is 755.Shit!I think I can get 800 or
//                                above,but that afternoon I took the examination,I
//                                feel so tired and had a little headache,so the result
//                                may be impacted by all these factors.
//                                But I have passed the deadline of my company even 
//                                though,which is 650.
//                                -------- ABOVE CONTENT ONLY FOR MEMORY --------
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

#include "stdio.h"

//This module will be available if and only if DDF is enabled.
#ifdef __CFG_SYS_DDF

//Global variables used by FAT32.
static __DRIVER_OBJECT*    g_Fat32Driver  = NULL;
static __DEVICE_OBJECT*    g_Fat32Object  = NULL;

//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.


//A helper routine to dump out a FAT32 file system's global information.
static VOID DumpFat32(__FAT32_FS* pFat32Fs)
{
	__DEVICE_OBJECT* pDevObj = (__DEVICE_OBJECT*)pFat32Fs->pPartition;

	/*
	printf("  -------- FAT32 FILE SYSTEM INFO -------- \r\n");
	printf("  File system name        : %s\r\n",pDevObj->DevName);
	printf("  Sector per cluster      : %d\r\n",pFat32Fs->SectorPerClus);
	printf("  Number of FAT           : %d\r\n",pFat32Fs->FatNum);
	printf("  Reserved sector number  : %d\r\n",pFat32Fs->wReservedSector);
	printf("  Byte per sector         : %d\r\n",pFat32Fs->dwBytePerSector);
	printf("  Data sector start       : %d\r\n",pFat32Fs->dwDataSectorStart);
	printf("  Root dir start cluster  : %d\r\n",pFat32Fs->dwRootDirClusStart);
	printf("  Start sector for FAT    : %d\r\n",pFat32Fs->dwFatBeginSector);
	printf("  FAT sector number       : %d\r\n",pFat32Fs->dwFatSectorNum);
	*/
}

//Helper routine to obtain volume lable.
static BOOL GetVolumeLbl(__FAT32_FS* pFat32Fs,CHAR* pVolumeLbl)
{
	BOOL                bResult      = FALSE;
	__FAT32_SHORTENTRY* pfse         = NULL;
	BYTE*               pBuffer      = NULL;
	DWORD               dwCurrClus   = 0;
	DWORD               dwSector     = 0;	
	CHAR                Buffer[128]  = {0};
	INT                 i,j;

	if((NULL == pFat32Fs) || (NULL == pVolumeLbl))
	{
		goto __TERMINAL;
	}
	//Create local buffer to contain one cluster.
	pBuffer = (BYTE*)FatMem_Alloc(pFat32Fs->SectorPerClus * pFat32Fs->dwBytePerSector);
	if(NULL == pBuffer)
	{
		PrintLine("Can not allocate memory from heap.");
		goto __TERMINAL;
	}
	dwCurrClus = pFat32Fs->dwRootDirClusStart;
	while(!IS_EOC(dwCurrClus))  //Main loop to check the root directory.
	{
		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)  //Fatal error.
		{
			goto __TERMINAL;
		}
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pBuffer))  //Can not read the appropriate sector(s).
		{
			_hx_sprintf(Buffer," Read sector failed in GetVolumeLbl,Info: %d %d %d %d %d %d %d ",
				dwCurrClus,
				dwSector,
				pFat32Fs->dwClusterSize,
				pFat32Fs->dwDataSectorStart,
				pFat32Fs->dwFatSectorNum,
				pFat32Fs->dwFatBeginSector,
				pFat32Fs->SectorPerClus);
			PrintLine(Buffer);  //Only used for debugging.
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
				for(j = 0;j < 11;j ++)
				{
					pVolumeLbl[j] = pfse->FileName[j];
				}
				pVolumeLbl[j] = 0;  //Set the terminator.
				bResult = TRUE;
				goto __TERMINAL;
			}
			pfse += 1;
		}
		if(!GetNextCluster(pFat32Fs,&dwCurrClus))
		{
			_hx_printf(Buffer,"Current cluster number is %d\n",dwCurrClus);			
			break;
		}
	}

__TERMINAL:
	FatMem_Free(pBuffer);

	return bResult;
}

//Initialize FAT32 partition,this routine is called by CheckPartition,which is then
//called by CreateDevice of IOManager.
static __FAT32_FS* InitFat32(__COMMON_OBJECT* pPartObj)
{
	__DEVICE_OBJECT*  pPartition  = (__DEVICE_OBJECT*)pPartObj;
	__FAT32_FS*       pFatObject = NULL;
	BYTE              buff[SECTOR_SIZE];

	if(NULL == pPartition)
	{
		goto __TERMINAL;
	}

	//Check the validity of partition device object.
	if(DEVICE_OBJECT_SIGNATURE != pPartition->dwSignature)  //Invalid signature.
	{
		goto __TERMINAL;
	}

	if(!ReadDeviceSector(pPartObj,
		0,
		1,
		buff))
	{
		PrintLine("Can not read sector 0.");
		goto __TERMINAL;
	}
	pFatObject = (__FAT32_FS*)CREATE_OBJECT(__FAT32_FS);
	if(NULL == pFatObject)  //Can not create FAT32 object.
	{
		goto __TERMINAL;
	}
	pFatObject->pPartition = pPartObj;    //Very important.
	//Initialize the FAT32 file system.
	if(!Fat32Init(pFatObject,buff))
	{
		PrintLine("Can not initialize the FAT32 file system.");
		RELEASE_OBJECT(pFatObject);       //Release it.
		pFatObject = NULL;
		goto __TERMINAL;
	}
	GetVolumeLbl(pFatObject,pFatObject->VolumeLabel);  //This operation may failed,but we no
	                                                   //need to concern it.
	DumpFat32(pFatObject);
__TERMINAL:
	return pFatObject;
}

//Implementation of CheckPartition.
static BOOL CheckPartition(__COMMON_OBJECT* lpThis,  //FAT32 driver device object.
		                   __COMMON_OBJECT* pPartitionObject)
{
	__DEVICE_OBJECT*       pPartition = (__DEVICE_OBJECT*)pPartitionObject;
	__FAT32_FS*            pFatObject = NULL;
	BOOL                   bResult    = FALSE;
	__DEVICE_OBJECT*       pFatDevice = NULL;
	CHAR                   DevName[64];
	int                    nIndex;
	static CHAR            nNameIndex = 0 ;
	//__PARTITION_EXTENSION* ppe = NULL;

	/*
	//Debug only.
	ppe = (__PARTITION_EXTENSION*)pPartition->lpDevExtension;
	sprintf(DevName," PT is : %X, attr is : %X",
		ppe->PartitionType,
		pPartition->dwAttribute);
	PrintLine(DevName);
	*/
	
	if(pPartition == NULL)
	{
		goto __TERMINAL;
	}
	if(!(DEVICE_TYPE_FAT32 & pPartition->dwAttribute))  //Not a FAT32 partition.
	{
		goto __TERMINAL;
	}
	pFatObject = InitFat32(pPartitionObject);
	if(!pFatObject)
	{
		goto __TERMINAL;
	}
	//Now create fat32 device object.
	StrCpy(FAT32_DEVICE_NAME_BASE,DevName);
	nIndex = StrLen(FAT32_DEVICE_NAME_BASE);
	DevName[nIndex - 1] += nNameIndex;  //Form the device name string.
	nNameIndex += 1; //Increment name index.
	pFatDevice = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		DevName,
		DEVICE_TYPE_FAT32,
		DEVICE_BLOCK_SIZE_INVALID,  //FAT device object can not be accessed directly.
		DEVICE_BLOCK_SIZE_INVALID,
		DEVICE_BLOCK_SIZE_INVALID,
		(LPVOID)pFatObject,
		((__DEVICE_OBJECT*)lpThis)->lpDriverObject);
	if(NULL == pFatDevice)  //Failed to create device.
	{
		goto __TERMINAL;
	}

	IOManager.AddFileSystem((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)pFatDevice,
		pFatObject->dwAttribute,
		pFatObject->VolumeLabel);
	/*
	//Print out the volume label information,only for debugging.
	for(i = 0;i < 11;i ++)
	{
		DevName[i] = pFatObject->VolumeLabel[i];
	}
	DevName[i] = 0;
	if(StrLen(DevName) == 0)
	{
		PrintLine("Volume label is empty.");
	}
	else
	{
		PrintLine(DevName);
	}*/
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

//Implementation of CreateDirectory.
static BOOL _CreateDirectory(__COMMON_OBJECT* lpDev,
	                         CHAR*   pszFileName,
	                         LPVOID  pReserved)    //Create directory.
{
	__DEVICE_OBJECT*         pFatDevice = (__DEVICE_OBJECT*)lpDev;
	DWORD                    dwDirCluster = 0;
	CHAR                     DirName[MAX_FILE_NAME_LEN];
	CHAR                     SubDirName[MAX_FILE_NAME_LEN];
	__FAT32_SHORTENTRY       DirShortEntry;

	if((NULL == pFatDevice) || (NULL == pszFileName))
	{
		return FALSE;
	}
	if(!GetPathName(pszFileName,DirName,SubDirName))
	{
		return FALSE;
	}

	
	//Try to open the parent directory.
	if(!GetDirEntry((__FAT32_FS*)pFatDevice->lpDevExtension,	DirName,&DirShortEntry,NULL,NULL))
	{
		PrintLine("Can not get directory entry of parent dir.");
		return FALSE;
	}

	//return FALSE;
	if(!(DirShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))  //Is not a directory.
	{
		PrintLine("The parent directory is not a directory.");
		return FALSE;
	}
	dwDirCluster =   DirShortEntry.wFirstClusHi;
	dwDirCluster <<= 16;
	dwDirCluster +=  DirShortEntry.wFirstClusLow;
	

	return CreateFatDir((__FAT32_FS*)pFatDevice->lpDevExtension,	dwDirCluster,SubDirName,FILE_ATTR_DIRECTORY);
}

//Implementation of CreateFile for FAT file system.
static __COMMON_OBJECT* FatDeviceOpen(__COMMON_OBJECT* lpDrv,
									  __COMMON_OBJECT* lpDev,
									  __DRCB* lpDrcb)   //Open file.
{
	__FAT32_FILE*       pFat32File    = NULL;
	__FAT32_FS*         pFat32Fs      = NULL;
	__COMMON_OBJECT*    pFileDevice   = NULL;
	CHAR                FileName[MAX_FILE_NAME_LEN];
	__FAT32_SHORTENTRY  ShortEntry;
	DWORD               dwDirClus     = 0;
	DWORD               dwDirOffset   = 0;
	DWORD               dwFlags;
	static CHAR         NameIndex     = 0;
	BOOL                bResult       = FALSE;
	CHAR                FileDevName[16];

	if((NULL == lpDrv) || (NULL == lpDev))
	{
		goto __TERMINAL;
	}
	pFat32Fs = (__FAT32_FS*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	//strcpy(FileName,(LPSTR)lpDrcb->lpInputBuffer);
	StrCpy((LPSTR)lpDrcb->lpInputBuffer,FileName);
	ToCapital(FileName);
	if(!GetDirEntry(pFat32Fs,&FileName[0],&ShortEntry,&dwDirClus,&dwDirOffset))
	{		
		goto __TERMINAL;
	}
	
	if(FILE_ATTR_DIRECTORY & ShortEntry.FileAttributes)  //Is a directory.
	{
		goto __TERMINAL;
	}
	//Create a file object.
	pFat32File = (__FAT32_FILE*)CREATE_OBJECT(__FAT32_FILE);
	if(NULL == pFat32File)
	{
		goto __TERMINAL;
	}
	pFat32File->Attributes     = ShortEntry.FileAttributes;
	pFat32File->dwClusOffset   = 0;
	pFat32File->bInRoot        = FALSE;          //Caution,not check currently.
	pFat32File->dwCurrClusNum  = ((DWORD)ShortEntry.wFirstClusHi << 16)
		+ (DWORD)ShortEntry.wFirstClusLow;
	pFat32File->dwCurrPos      = 0;
	pFat32File->dwFileSize     = ShortEntry.dwFileSize;
	pFat32File->dwOpenMode     = lpDrcb->dwInputLen;     //dwInputLen is used to contain open mode.
	pFat32File->dwShareMode    = lpDrcb->dwOutputLen;    //dwOutputLen is used to contain share mode.
	pFat32File->dwStartClusNum = pFat32File->dwCurrClusNum;
	pFat32File->pFileSystem    = pFat32Fs;
	pFat32File->pPartition     = pFat32Fs->pPartition;   //CAUTION!!!
	pFat32File->dwParentClus   = dwDirClus;              //Save parent directory's information.
	pFat32File->dwParentOffset = dwDirOffset;
	pFat32File->pNext          = NULL;
	pFat32File->pPrev          = NULL;
	//Now insert the file object to file system object's file list.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pFat32Fs->pFileList == NULL)  //Not any file object in list yet.
	{
		pFat32Fs->pFileList = pFat32File;
		pFat32File->pNext = NULL;
		pFat32File->pPrev = NULL;
	}
	else
	{
		pFat32File->pNext = pFat32Fs->pFileList;
		pFat32Fs->pFileList->pPrev = pFat32File;

		pFat32File->pPrev = NULL;
		pFat32Fs->pFileList = pFat32File;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Now create file device object.
	//strcpy(FileDevName,FAT32_FILE_NAME_BASE);
	StrCpy(FAT32_FILE_NAME_BASE,FileDevName);
	FileDevName[13] += NameIndex;
	NameIndex ++;
	pFileDevice = (__COMMON_OBJECT*)IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		FileDevName,
		DEVICE_TYPE_FILE,
		1, //For file,block size is 1.
		DEVICE_BLOCK_SIZE_ANY,
		DEVICE_BLOCK_SIZE_ANY,
		pFat32File,
		(__DRIVER_OBJECT*)lpDrv);
	if(NULL == pFileDevice)
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	if(!bResult)  //The transaction has failed.
	{
		if(pFat32File)
		{
			RELEASE_OBJECT(pFat32File);
		}
		if(pFileDevice)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				(__DEVICE_OBJECT*)pFileDevice);
		}
		return NULL;
	}
	return pFileDevice;
}

//Implementation of DeviceClose routine.
static DWORD FatDeviceClose(__COMMON_OBJECT* lpDrv,
							__COMMON_OBJECT* lpDev,
							__DRCB* lpDrcb)
{
	__DEVICE_OBJECT*        pDeviceObject    = (__DEVICE_OBJECT*)lpDev;
	__FAT32_FS*             pFat32Fs         = NULL;
	__FAT32_FILE*           pFileObject      = NULL;
	DWORD                   _dwFlags;

	if((NULL == pDeviceObject) || (NULL == lpDrcb))
	{
		return 0;
	}
	pFileObject = (__FAT32_FILE*)pDeviceObject->lpDevExtension;
	pFat32Fs    = pFileObject->pFileSystem;
	//Delete the fat32 file object from file system.
	__ENTER_CRITICAL_SECTION(NULL, _dwFlags);
	if((pFileObject->pPrev == NULL) && (pFileObject->pNext == NULL))
	{
		pFat32Fs->pFileList = NULL;
	}
	else
	{
		if(pFileObject->pPrev == NULL)  //This is the first object in file list.
		{
			pFat32Fs->pFileList = pFileObject->pNext;
			pFileObject->pNext->pPrev = NULL;
		}
		else  //Not the fist file in list.
		{
			if(NULL == pFileObject->pNext)  //This is the last one in list.
			{
				pFileObject->pPrev->pNext = NULL;
			}
			else  //Neither is the first nor is the last one in list.
			{
				pFileObject->pPrev->pNext = pFileObject->pNext;
				pFileObject->pNext->pPrev = pFileObject->pPrev;
			}
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL, _dwFlags);
	//Release the file object.
	RELEASE_OBJECT(pFileObject);

	//Destroy file device object.
	IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,pDeviceObject);

	return 0;
}


//Implementation of _fat32DeleteFile routine.
static BOOL _fat32DeleteFile(__COMMON_OBJECT* lpDev,LPSTR pszFileName)       //Delete a file.
{
	__FAT32_FS*        pFat32Fs   = NULL;

	if((NULL == lpDev) || (NULL == pszFileName))
	{
		return FALSE;
	}
	pFat32Fs = (__FAT32_FS*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;

	return DeleteFatFile(pFat32Fs,pszFileName);
}


//Implementation of _fat32FindClose routine.
static BOOL _fat32FindClose(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* pHandle)  //Close find handle.
{
	__FAT32_FIND_HANDLE*  pFindHandle = (__FAT32_FIND_HANDLE*)pHandle;
	if(NULL == pFindHandle)
	{
		return FALSE;
	}
	//Release the resource now.
	while(pFindHandle->pClusterRoot)
	{
		pFindHandle->pCurrCluster = pFindHandle->pClusterRoot;
		pFindHandle->pClusterRoot = pFindHandle->pClusterRoot->pNext;
		if(pFindHandle->pCurrCluster->pCluster)
		{
			FatMem_Free(pFindHandle->pCurrCluster->pCluster);
		}

		FatMem_Free(pFindHandle->pCurrCluster);
	}
	//Release the find handle object.
	FatMem_Free(pFindHandle);

	return TRUE;
}

//Helper routine used to build the directory cluster list given a name.
//The appropriate find handle is returned if all successfully.
static __FAT32_FIND_HANDLE* BuildFindHandle(__FAT32_FS* pFat32Fs,CHAR* pszDirName)
{
	__FAT32_FIND_HANDLE*    pFindHandle     = NULL;
	__FAT32_DIR_CLUSTER*    pDirCluster     = NULL;
	__FAT32_SHORTENTRY      ShortEntry      = {0};     //Short entry of target dir.
	DWORD                   dwCurrClus      = 0;
	DWORD                   dwSector        = 0;
	BOOL                    bResult         = FALSE;

	if(!GetDirEntry(pFat32Fs,&pszDirName[0],&ShortEntry,NULL,NULL))
	{
		goto __TERMINAL;
	}
	if(!(ShortEntry.FileAttributes & FILE_ATTR_DIRECTORY))  //Not a directory.
	{
		goto __TERMINAL;
	}
	pFindHandle = (__FAT32_FIND_HANDLE*)CREATE_OBJECT(__FAT32_FIND_HANDLE);
	if(NULL == pFindHandle)
	{
		goto __TERMINAL;
	}
	//Initialize the find hanlde object.
	pFindHandle->dwClusterOffset = 0;
	pFindHandle->dwClusterSize   = pFat32Fs->dwClusterSize;
	pFindHandle->pClusterRoot    = NULL;
	pFindHandle->pCurrCluster    = NULL;
	dwCurrClus = ((DWORD)ShortEntry.wFirstClusHi << 16) + (DWORD)ShortEntry.wFirstClusLow;
	while(!IS_EOC(dwCurrClus))
	{
		pDirCluster = (__FAT32_DIR_CLUSTER*)CREATE_OBJECT(__FAT32_DIR_CLUSTER);
		if(NULL == pDirCluster)  //Can not allocate memory.
		{
			goto __TERMINAL;
		}
		pDirCluster->pCluster = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
		if(NULL == pDirCluster->pCluster)
		{
			goto __TERMINAL;
		}
		pDirCluster->pNext = NULL;
		//Now try to read the directory cluster data.
		dwSector = GetClusterSector(pFat32Fs,dwCurrClus);
		if(0 == dwSector)
		{
			goto __TERMINAL;
		}
		if(!ReadDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
			dwSector,
			pFat32Fs->SectorPerClus,
			pDirCluster->pCluster))  //Can not read directory cluster.
		{
			goto __TERMINAL;
		}
		//Attach this cluster into directory cluster list.
		if(NULL == pFindHandle->pClusterRoot)  //First cluster now.
		{
			pFindHandle->pClusterRoot = pDirCluster;
			pFindHandle->pCurrCluster = pDirCluster;
		}
		else  //Not the first cluster,pCurrCluster pointing to the last node.
		{
			pFindHandle->pCurrCluster->pNext = pDirCluster;
			pFindHandle->pCurrCluster        = pDirCluster;
		}
		if(!GetNextCluster(pFat32Fs,&dwCurrClus))
		{
			goto __TERMINAL;
		}
	}
	pFindHandle->pCurrCluster = pFindHandle->pClusterRoot;
	pDirCluster = NULL;  //Indicate the successful execution of above while block.
	bResult = TRUE;  //Mark the successful flag.

__TERMINAL:
	if(bResult)  //Successful.
	{
		return pFindHandle;
	}
	else  //Failed,should release the allocated resource.
	{
		if(pDirCluster)  //Directory cluster object has been allocated.
		{
			if(pDirCluster->pCluster)
			{
				FatMem_Free(pDirCluster->pCluster);
			}
			FatMem_Free(pDirCluster);
		}
		//Release the directory cluster object in list.
		if(NULL == pFindHandle)
		{
			goto __RETURN;
		}
		while(pFindHandle->pClusterRoot)
		{
			pFindHandle->pCurrCluster = pFindHandle->pClusterRoot;
			pFindHandle->pClusterRoot = pFindHandle->pClusterRoot->pNext;
			if(pFindHandle->pCurrCluster->pCluster)
			{
				FatMem_Free(pFindHandle->pCurrCluster->pCluster);
			}
			FatMem_Free(pFindHandle->pCurrCluster);
		}

		//Release the find handle object.
		FatMem_Free(pFindHandle);

__RETURN:

	return NULL;
	}
}

//Another helper routine to fill one find data structure according to find handle.
static BOOL FillFindData(__FAT32_FIND_HANDLE* pFindHandle,FS_FIND_DATA* pFindData)
{
	__FAT32_SHORTENTRY*    pShortEntry                   = NULL;
	CHAR                   szLongName[MAX_FILE_NAME_LEN] = {0};
	BOOL                   bResult                       = FALSE;	
	DWORD                  i;

	while(pFindHandle->pCurrCluster)  //Search from current cluster.
	{
		__FAT32_LONGENTRY*  szLongEntry[64]     = {0};
		INT                 nLongEntryNum       = 0;	

		pShortEntry = (__FAT32_SHORTENTRY*)pFindHandle->pCurrCluster->pCluster;
		pShortEntry += pFindHandle->dwClusterOffset / 32;
		i = pFindHandle->dwClusterOffset / 32;

		//Now try to find a valid directory short entry.
		for(i;i < pFindHandle->dwClusterSize / 32;i ++)
		{
			if(0xE5 == (BYTE)pShortEntry->FileName[0])
			{
				pShortEntry += 1;
				pFindHandle->dwClusterOffset += 32;
				continue;
			}
			if(0 == pShortEntry->FileName[0])
			{
				break;
			}
			if(FILE_ATTR_LONGNAME == pShortEntry->FileAttributes)
			{
				//record long entry info
				if(pFindData->bGetLongName)
				{
					szLongEntry[nLongEntryNum ++ ] =  (__FAT32_LONGENTRY*)pShortEntry;
				}
				
				pShortEntry += 1;
				pFindHandle->dwClusterOffset += 32;
				continue;
			}
			if(FILE_ATTR_VOLUMEID & pShortEntry->FileAttributes)
			{
				pShortEntry += 1;
				pFindHandle->dwClusterOffset += 32;
				continue;
			}

			if(pFindData->bGetLongName)
			{
				CombinLongFileName(szLongEntry,nLongEntryNum,szLongName);
			}
				
			//Normal short entry,return it.
			pFindHandle->dwClusterOffset += 32;
			bResult = TRUE;
			goto __FIND;
		}
		pFindHandle->pCurrCluster = pFindHandle->pCurrCluster->pNext;
		pFindHandle->dwClusterOffset = 0;
	}
__FIND:
	if(bResult)  //Find.
	{
		ConvertShortEntry(pShortEntry,pFindData);

		if(pFindData->bGetLongName)
		{
			strcpy(pFindData->cFileName,szLongName);
		}
	}
	return bResult;
}


//Implementation of _fat32FindFirstFile.
static __COMMON_OBJECT* _fat32FindFirstFile(__COMMON_OBJECT* lpThis,CHAR*  pszFileName,FS_FIND_DATA* pFindData)
{
	__FAT32_FIND_HANDLE*      pFindHandle = NULL;
	__DEVICE_OBJECT*          pDevice     = (__DEVICE_OBJECT*)lpThis;

	if((NULL == lpThis) || (NULL == pszFileName) || (NULL == pFindData))
	{
		return NULL;
	}

	pFindHandle = BuildFindHandle((__FAT32_FS*)pDevice->lpDevExtension,	pszFileName);
	if(NULL == pFindHandle)
	{
		return NULL;
	}

	if(!FillFindData(pFindHandle,pFindData))
	{
		return NULL;
	}

	return (__COMMON_OBJECT*)pFindHandle;
}

//Implementation of _FindNextFile.
static BOOL _FindNextFile(__COMMON_OBJECT* lpThis,
		                 __COMMON_OBJECT* pFindHandle,
		                 FS_FIND_DATA* pFindData)
{
	if((NULL == pFindHandle) || (NULL == pFindData))
	{
		return FALSE;
	}
	return FillFindData((__FAT32_FIND_HANDLE*)pFindHandle,pFindData);
}

//Implementation of FlushFileBuffers.
static DWORD FatDeviceFlush(__COMMON_OBJECT* lpDrv,
		                    __COMMON_OBJECT* lpDev,
							__DRCB* lpDrcb)
{
	return FALSE;
}


//Implementation of _fat32GetFileAttributes.
static DWORD _fat32GetFileAttributes(__COMMON_OBJECT* lpDev, LPCTSTR  pszFileName)  //Get file's attribute.
{
	__FAT32_FS*         pFat32Fs      = NULL;
	CHAR                FileName[MAX_FILE_NAME_LEN];
	__FAT32_SHORTENTRY  ShortEntry;
	DWORD               dwFileAttributes = 0;

	if((NULL == pszFileName) || (NULL == lpDev))
	{
		goto __TERMINAL;
	}
	pFat32Fs = (__FAT32_FS*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	//strcpy(FileName,(LPSTR)lpDrcb->lpInputBuffer);
	StrCpy((CHAR*)pszFileName,FileName);
	ToCapital(FileName);
	if(!GetDirEntry(pFat32Fs,&FileName[0],&ShortEntry,NULL,NULL))
	{
		_hx_printf("In _fat32GetFileAttributes: Can not get directory entry.\n%s\n",FileName);
		goto __TERMINAL;
	}
	dwFileAttributes = ShortEntry.FileAttributes;

__TERMINAL:
	return dwFileAttributes;
}

//Implementation of GetFileSize.
/*static DWORD GetFileSize(__COMMON_OBJECT* lpThis,
		                 __COMMON_OBJECT* pFileHandle,
						 DWORD* pdwSizeHigh)
{
	__FAT32_FS*         pFat32Fs      = NULL;
	
	pFat32Fs = (__FAT32_FS*)(((__DEVICE_OBJECT*)lpThis)->lpDevExtension);

	return pFat32Fs->pFileList->dwFileSize;
}*/

//Implementation of _RemoveDirectory.
static BOOL _RemoveDirectory(__COMMON_OBJECT* lpDev,
		                    LPSTR            pszFileName)
{
	__FAT32_FS*             pFat32Fs         = NULL;

	if((NULL == lpDev) || (NULL == pszFileName))
	{
		return FALSE;
	}
	pFat32Fs = (__FAT32_FS*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;

	return DeleteFatDir(pFat32Fs,pszFileName);
}

//Implementation of _SetEndOfFile.
static BOOL _SetEndOfFile( __COMMON_OBJECT* lpDev)
{
	__FAT32_FS*             pFat32Fs      = NULL;
	__FAT32_FILE*           pFat32File    = NULL;	
	BYTE*                   pClusBuffer   = NULL;
	__FAT32_SHORTENTRY*     pfse          = NULL;	
	DWORD                   dwNextCluster = 0;
	DWORD                   dwSector      = 0;
	BOOL                    bSetOk        = FALSE;


	pFat32File   = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	pFat32Fs     = pFat32File->pFileSystem;

	pClusBuffer  = (BYTE*)FatMem_Alloc(pFat32Fs->dwClusterSize);
	if(NULL == pClusBuffer)  //Can not allocate buffer.
	{
		goto __TERMINAL;
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
	pfse = (__FAT32_SHORTENTRY*)(pClusBuffer + pFat32File->dwParentOffset);

	//�ļ���СΪ��ǰλ��
	pFat32File->dwFileSize = pFat32File->dwCurrPos;
	pfse->dwFileSize       = pFat32File->dwFileSize;

	WriteDeviceSector((__COMMON_OBJECT*)pFat32Fs->pPartition,
		dwSector,
		pFat32Fs->SectorPerClus,
		pClusBuffer);

	//�ͷ�ʣ��Ŀռ�
	if(pFat32File->dwCurrPos < pFat32Fs->dwClusterSize )
	{
		dwNextCluster = 1;
	}
	else
	{
		dwNextCluster = pFat32File->dwCurrPos/pFat32Fs->dwClusterSize;
		if((pFat32File->dwCurrPos+1)%pFat32Fs->dwClusterSize)
		{
				dwNextCluster ++;
		}
	}	
	dwNextCluster ++;
	ReleaseClusterChain(pFat32Fs,dwNextCluster);

	bSetOk = TRUE;

__TERMINAL:
	FatMem_Free(pClusBuffer);

	return bSetOk;
}

//Implementation of SetFilePointer.
static DWORD FatDeviceSeek(__COMMON_OBJECT* lpDrv, __COMMON_OBJECT* lpDev,__DRCB* lpDrcb)
{	
	__FAT32_FILE*    pFatFile  = NULL;
	DWORD	         dwSeekRet = -1;

	if((NULL == lpDrv) || (NULL == lpDev) || (NULL == lpDrcb))
	{
		return dwSeekRet;
	}

	pFatFile = (__FAT32_FILE*)(((__DEVICE_OBJECT*)lpDev)->lpDevExtension);
	if( NULL != pFatFile)
	{
		__FAT32_FS* pFatFs       = pFatFile->pFileSystem;		
		DWORD       dwWhereBegin = (DWORD)lpDrcb->lpInputBuffer;//((DWORD*)lpDrcb->lpInputBuffer);
		DWORD       dwOffsetPos  = *((INT*)lpDrcb->dwExtraParam1);		
		DWORD       dwClusterNum = 0;		
		DWORD       i            = 0;
				
		//_hx_printf("FatDeviceSeek: where=%d,pos=%d\r\n",dwWhereBegin,dwOffsetPos);

		switch(dwWhereBegin)
		{
			case FILE_FROM_BEGIN:
			{
				pFatFile->dwCurrPos = dwOffsetPos;
			}
			break;
			case FILE_FROM_CURRENT:
			{			
				pFatFile->dwCurrPos += dwOffsetPos;				
			}
			break;
			case FILE_FROM_END:
			{
				pFatFile->dwCurrPos = pFatFile->dwFileSize;	
			}
			break;
		default:
			{
			return dwSeekRet;
			}
		}
		
		//λ���Ƿ����
		if(pFatFile->dwCurrPos > pFatFile->dwFileSize)
		{
			pFatFile->dwCurrPos = pFatFile->dwFileSize;		
		}

		//ȷ�� Cluster ����
		if(pFatFile->dwCurrPos < pFatFs->dwClusterSize )
		{
			dwClusterNum = 1;
		}
		else
		{
			dwClusterNum = pFatFile->dwCurrPos/pFatFs->dwClusterSize;
			if((pFatFile->dwCurrPos+1)%pFatFs->dwClusterSize)
			{
				dwClusterNum ++;
			}
		}	
								
		//����Cluster���� ����� �ļ�Cluster ʵ��λ��
		pFatFile->dwCurrClusNum = pFatFile->dwStartClusNum;
		for(i=0; i< (dwClusterNum-1); i++)
		{
			DWORD dwNextNum = pFatFile->dwCurrClusNum;

			if(!GetNextCluster(pFatFs,&dwNextNum))
			{
				return dwSeekRet;
			}

			pFatFile->dwCurrClusNum = dwNextNum;
		}

		//���� Cluster ��ƫ��
		pFatFile->dwClusOffset = pFatFile->dwCurrPos % pFatFs->dwClusterSize;
		dwSeekRet              = pFatFile->dwCurrPos;
	}
		
	return dwSeekRet;	
}

//Implementation of DeviceCtrl routine.
static DWORD FatDeviceCtrl(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
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
		pFindHandle = _fat32FindFirstFile((__COMMON_OBJECT*)lpDev,
			(CHAR*)lpDrcb->dwExtraParam1,
			(FS_FIND_DATA*)lpDrcb->dwExtraParam2);
		if(NULL == pFindHandle)  //Can not start the iterate.
		{
			return 0;
		}
		lpDrcb->lpOutputBuffer = (LPVOID)pFindHandle;
		return 1;
	case IOCONTROL_FS_FINDNEXTFILE:
		return _FindNextFile((__COMMON_OBJECT*)lpDev,
			(__COMMON_OBJECT*)lpDrcb->lpInputBuffer,
			(FS_FIND_DATA*)lpDrcb->dwExtraParam2) ? 1 : 0;
	case IOCONTROL_FS_FINDCLOSE:
		_fat32FindClose((__COMMON_OBJECT*)lpDev,
			(__COMMON_OBJECT*)lpDrcb->lpInputBuffer);
		break;
	case IOCONTROL_FS_CREATEDIR:
		if(_CreateDirectory(lpDev,
			(LPSTR)lpDrcb->lpInputBuffer,
			0))
		{
			lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;
			return 1;
		}
		else
		{
			lpDrcb->dwStatus = DRCB_STATUS_FAIL;
			return 0;
		}
		break;
	case IOCONTROL_FS_GETFILEATTR:
		lpDrcb->dwExtraParam2 = _fat32GetFileAttributes(
			lpDev,
			(LPCTSTR)lpDrcb->dwExtraParam1);
		lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;
		return 1;
		break;
	case IOCONTROL_FS_DELETEFILE:
		if(_DeleteFile(lpDev,
			(LPSTR)lpDrcb->lpInputBuffer))
		{
			lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;
			return 1;
		}
		else
		{
			lpDrcb->dwStatus = DRCB_STATUS_FAIL;
			return 0;
		}
		break;
	case IOCONTROL_FS_REMOVEDIR:
		if(_RemoveDirectory(lpDev,
			(LPSTR)lpDrcb->lpInputBuffer))
		{
			lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;
			return 1;
		}
		else
		{
			lpDrcb->dwStatus = DRCB_STATUS_FAIL;
			return 0;
		}
		break;
	case IOCONTROL_FS_SETENDFILE:
		{
		if(_SetEndOfFile(lpDev))
			{
				lpDrcb->dwStatus = DRCB_STATUS_SUCCESS;
				return 1;
			}
			else
			{
				lpDrcb->dwStatus = DRCB_STATUS_FAIL;
				return 0;
			}
		}
		break;
	default:
		break;
	}

__TERMINAL:
	return 0;
}

//Implementation of DriverEntry routine for FAT32 file system.
BOOL FatDriverEntry(__DRIVER_OBJECT* lpDriverObject)
{
	__DEVICE_OBJECT*  pFatObject  = NULL;

	//Initialize the driver object.
	lpDriverObject->DeviceClose   = FatDeviceClose;
	lpDriverObject->DeviceCtrl    = FatDeviceCtrl;
	lpDriverObject->DeviceFlush   = FatDeviceFlush;
	lpDriverObject->DeviceOpen    = FatDeviceOpen;
	lpDriverObject->DeviceRead    = FatDeviceRead;
	lpDriverObject->DeviceSeek    = FatDeviceSeek;
	lpDriverObject->DeviceWrite   = FatDeviceWrite;
	lpDriverObject->DeviceCreate  = FatDeviceCreate;
	lpDriverObject->DeviceSize    = FatDeviceSize;

	//Create FAT file system driver object now.
	pFatObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		FAT32_DRIVER_DEVICE_NAME,
		DEVICE_TYPE_FSDRIVER, //Attribute,this is a file system driver object.
		DEVICE_BLOCK_SIZE_INVALID, //File system driver object can not be read or written.
		DEVICE_BLOCK_SIZE_INVALID,
		DEVICE_BLOCK_SIZE_INVALID,
		NULL,  //With out any extension.
		lpDriverObject);
	if(NULL == pFatObject)  //Can not create file system object.
	{
		return FALSE;
	}
	//Register file system now.
	if(!IOManager.RegisterFileSystem((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)pFatObject))
	{
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			pFatObject);
		return FALSE;
	}
	//Record the driver and device objects.
	g_Fat32Driver = lpDriverObject;
	g_Fat32Object = pFatObject;
	return TRUE;
}

#endif
