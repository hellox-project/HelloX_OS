//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 07 FEB,2009
//    Module Name               : FDISK2.CPP
//    Module Funciton           : 
//    Description               : Implementation code of fdisk application.
//                                This file is the second part of FDISK application source code.In this part,
//                                format command's source code is included.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include "../include/StdAfx.h"
#include "shell.h"
#include "fdisk.h"

#include "../include/kapi.h"
#include "../lib/stdio.h"

typedef struct BPB_FAT32BS{  //BPB and fat32 boot sector.
	UCHAR      BS_jmpBoot1;
	UCHAR      BS_jmpBoot2;
	UCHAR      BS_jmpBoot3;

	BYTE       BS_OEMName[8];
	UCHAR      BPB_BytsPerSec[2];
	BYTE       BPB_SecPerClus;
	UCHAR      BPB_RsvdSecCnt[2];
	BYTE       BPB_NumFATs;
	UCHAR      BPB_RootEntCnt[2];
	UCHAR      BPB_TotSec16[2];
	UCHAR      BPB_Media;
	UCHAR      BPB_FATSz16[2];
	UCHAR      BPB_SecPerTrk[2];
	UCHAR      BPB_NumHeads[2];
	UCHAR      BPB_HiddSec[4];
	UCHAR      BPB_TotSec32[4];

	//From here is the BS for FAT32. FAT16/12 is skiped in current version.
	UCHAR      BPB_FATSz32[4];
	UCHAR      BPB_ExtFlags[2];
	UCHAR      BPB_FSVer[2];
	UCHAR      BPB_RootClus[4];
	UCHAR      BPB_FSInfo[2];
	UCHAR      BPB_BkBootSec[2];
	UCHAR      BPB_Reserved[12];
	UCHAR      BS_DrvNum;
	UCHAR      BS_Reserved1;
	UCHAR      BS_BootSig;
	UCHAR      BS_VolID[4];
	UCHAR      BS_VolLab[11];
	UCHAR      BS_FilSysType[8];
}__BPB_FAT32BS;

//Typical instance of __BPB_FAT32BS,in most case of format,only modify several variables
//of this instance and write it into first sector of target partition is ok.
static __BPB_FAT32BS Fat32Sector0 = {
	0xEB,                               //BS_jmpBoot1,2,3
	0x00,
	0x90,

	{'M','S','W','I','N','4','.','1'},  //BS_OEMName,get these values from MS's FAT32 SPEC.
	{0x00,0x02},                        //Bytes per sector,512.
	0,                                  //Sector per cluster,should be determined when execute format.
	{32,0},                             //Reserved sector counter,32 is get from MS's FAT32 SPEC.
	2,                                  //Number of FAT.
	{0,0},                              //Root entry counter,0 for FAT32.
	{0,0},                              //Total sector 16,0 for FAT32.
	0xF8,                               //Media type,0xF8 for fixed.
	{0,0},                              //FAT size 16,0 for FAT32.
	{0,0},                              //Sector per track.
	{0,0},                              //Number of heads.
	{0,0,0,0},                          //Hidden sector counter.
	{0,0,0,0},                          //Total sector 32,should be modified when format.

	{0,0,0,0},                          //FAT size 32,should be modified when format.
	{0,0},                              //Extend flags,mirroring at run time.
	{0,0},                              //FS Version.
	{2,0,0,0},                          //Root directory cluster number,modified later.
	{1,0},                              //FSInfo sector number,default is 1.
	{6,0},                              //Backup sector number for boot sector,6 is default.
	{0},                                //Reserved for FAT32.
	0,                                  //Driver number for int 13h call in DOS.
	0,                                  //Reserved1.
	0x29,                               //Boot signature.
	{0,0,0,0},                          //Volume ID,modified late.
	{'N','O',' ','N','A','M','E',' ',' ',' ',' '},  //Volume label.
	{'F','A','T','3','2',' ',' ',' '}   //File system type,no meaning.
};

//The following code is captured from MS's FAT32 SPEC,to determine the default sector per cluster number
//given one partition's size.
typedef struct{
	DWORD	DiskSize;
	BYTE	SecPerClusVal;
}DSKSZTOSECPERCLUS;

/* 
* This is the table for FAT32 drives. NOTE that this table includes
* entries for disk sizes smaller than 512 MB even though typically
* only the entries for disks >= 512 MB in size are used.
* The way this table is accessed is to look for the first entry
* in the table for which the disk size is less than or equal
* to the DiskSize field in that table entry. For this table to
* work properly BPB_RsvdSecCnt must be 32, and BPB_NumFATs
* must be 2. Any of these values being different may require the first 
* table entries DiskSize value to be changed otherwise the cluster count 
* may be to low for FAT32.
*/
DSKSZTOSECPERCLUS DskTableFAT32 [] = {
        {       66600,   0},  /* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
        {     532480,   1},   /* disks up to 260 MB,  .5k cluster */
        { 16777216,   8},     /* disks up to     8 GB,    4k cluster */
        { 33554432, 16},      /* disks up to   16 GB,    8k cluster */
        { 67108864, 32},      /* disks up to   32 GB,  16k cluster */
        { 0xFFFFFFFF, 64}/* disks greater than 32GB, 32k cluster */
};

//Searchs the DskTableFAT32 array by using dwPartSectNum,returns the appropriate SecPerClus counter.
//This is a local helper routine.
static BYTE GetSecPerClus(DWORD dwPartSectNum)
{
	int i    = 0;

	while(dwPartSectNum > DskTableFAT32[i].DiskSize)
	{
		i ++;  //Check the next one.
	}
	return DskTableFAT32[i].SecPerClusVal;
}

//Calculate sector number occupied by one FAT.This is a local helper routine,only suitable for FAT32.
//This algorithm is copied from MS's FAT32 SPEC,and only keep the FAT32 related part.
static DWORD FatSize32(DWORD dwDiskSect,DWORD dwReservedSect,DWORD dwSecPerClus,DWORD dwNumFats)
{
	DWORD    TmpVal1 = dwDiskSect - dwReservedSect;
	DWORD    TmpVal2 = 256 * dwSecPerClus + dwNumFats;

	//if(FATType == FAT32)
	TmpVal2 = TmpVal2 / 2;
	return (TmpVal1 + TmpVal2 - 1) / TmpVal2;
}

//Two helper routines used to read or write sector(s) from or into disk.
//Read one or several sector(s) from device object.
//This is the lowest level routine used by all FAT32 driver code.
static BOOL ReadDeviceSector(__DEVICE_OBJECT* pPartition,
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
static BOOL WriteDeviceSector(__DEVICE_OBJECT* pPartition,
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

//Local helper routine,to print out process information.
static VOID InProcess()
{
	WORD w = 0x0700;
	w += '!';
	PrintCh(w);
	w = 0x0700;
}

//Helper routine to initialize the FAT zone of target partition,and root directory also be created by 
//this routine.
static BOOL InitFat_Root(__DEVICE_OBJECT* pPartition,    //Target partition object.
						 DWORD            dwFatStart,    //FAT's start sector number.
						 DWORD            dwFatSize,     //One FAT's size,in sector number.
						 DWORD            dwSecPerClus,  //Sector number per cluster.
						 BYTE             VolLabel[])    //Volume label.
{
	__FAT32_SHORTENTRY* pfse;  //Directory entry for volume label.
	BYTE                Buffer[512];  //Temporary local buffer.
	DWORD*              pFatEntry = NULL;
	DWORD               dwRootStart = 0; //Root dir's start data sector number.
	DWORD               i = 0;
	int                 j = 0;
	BOOL                bResult = FALSE;

	//Write the FAT table into partition,the first three entry(cluster 0,1,2) are initialized to EOC.
	memzero(&Buffer[0],512);
	pFatEntry = (DWORD*)&Buffer[0];
	*pFatEntry = 0xFFFFFFFF;
	pFatEntry ++;
	*pFatEntry = 0xFFFFFFFF;
	pFatEntry ++;
	*pFatEntry = 0xFFFFFFFF;
	if(!WriteDeviceSector(pPartition,  //Write the master FAT table.
		dwFatStart,
		1,
		Buffer))
	{
		return FALSE;
	}
	InProcess();

	if(!WriteDeviceSector(pPartition,  //Write the backup FAT table.
		dwFatStart + dwFatSize,
		1,
		Buffer))
	{
		return FALSE;
	}
	InProcess();

	//Now initialize the rest part of FAT table,all to zero.
	memzero(&Buffer[0],512);
	for(i = 1;i < dwFatSize;i ++)
	{
		if(!WriteDeviceSector(pPartition,
			dwFatStart + i,
			1,
			Buffer))
		{
			return FALSE;
		}
		if(!WriteDeviceSector(pPartition,
			dwFatStart + dwFatSize + i,
			1,
			Buffer))
		{
			return FALSE;
		}
		InProcess();
	}
	//Now initialize the root directory,assume the start cluster number of root is 2.
	dwRootStart = dwFatStart + dwFatSize * 2;
	memzero(&Buffer[0],512);
	pfse = (__FAT32_SHORTENTRY*)&Buffer[0];
	pfse->CreateDate       = 0;
	pfse->CreateTime       = 0;
	pfse->CreateTimeTenth  = 0;
	pfse->dwFileSize       = 0;
	pfse->FileAttributes   = FILE_ATTR_VOLUMEID;
	pfse->LastAccessDate   = 0;
	pfse->wFirstClusHi     = 0xFFFF;
	pfse->wFirstClusLow    = 0xFFFF;
	for(j = 0;j < 11;j ++)
	{
		pfse->FileName[j] = VolLabel[j];
	}
	//Now try to write the first sector of root directory's first cluster.
	if(!WriteDeviceSector(pPartition,
		dwRootStart,
		1,
		Buffer))
	{
		return FALSE;
	}
	InProcess();

	//Now clear the rest sector to zero.
	memzero(&Buffer[0],512);
	for(i = 1;i < dwSecPerClus;i ++)
	{
		if(!WriteDeviceSector(pPartition,
			dwRootStart + i,
			1,
			Buffer))
		{
			return FALSE;
		}
		InProcess();
	}
	return TRUE;
}

//Helper routine,print out format's usage.
static VOID formatusage()
{
	PrintLine("  Usage:");
	PrintLine("    format partition_name fs_type [vol_label]");
	PrintLine("    Where:");
	PrintLine("      partition_name : The name of the partition to be formatted.");
	PrintLine("      fs_type        : File system type,only FAT32 is available now.");
	PrintLine("      vol_label      : Volume label,default value is 'NO NAME    '");
}

//Format executing routine for FAT32 file system.
//This routine takes the following actions:
// 1. Get the Sector per cluster value by looking up DksTableFAT32 array,using partition's sector number;
// 2. Fill sector per cluster value into BPB_SecPerClus of Fat32Sector0;
// 3. Fill the partition's sector number into BPB_TotSec32 of Fat32Sector0;
// 4. Calculate the sector counter occupied by one FAT table and fill it into BPB_FATSz32 of Fat32Sector0;
// 5. Set the BS_VolID of Fat32Sector0 to AAAA,the simplied way:-);
// 6. Copy VolLabel into BS_VolLab of Fat32Sector0;
// 
// 7. After all above steps,sector 0 of target partition is constructed;
// 8. Then write the sector 0 into target partition;
// 9. Write the sector 0 into target partition's backup sector,usually 6;
// 10. Write all zero to FAT table area;
// 11. Mark the root cluster entry's next cluster number as EOC(End of cluster);
// 12. Create the volume label short directory entry in root directory.
//
static BOOL Fat32Format(__DEVICE_OBJECT* pPartition,BYTE VolLabel[],DWORD dwSecPerClus)
{
	__PARTITION_EXTENSION*  pPe = (__PARTITION_EXTENSION*)pPartition->lpDevExtension;
	DWORD                   dwPartSect = pPe->dwSectorNum;
	DWORD                   dwFatSize  = 0;
	DWORD*                  pdwTmp     = NULL;
	BYTE                    Buffer[512];
	int                     i;

	//Initialize BPB_SecPerClus of Fat32Sector0.
	Fat32Sector0.BPB_SecPerClus = GetSecPerClus(dwPartSect);
	//Initialize BPB_TotSec32.
	pdwTmp  = (DWORD*)&Fat32Sector0.BPB_TotSec32[0];
	*pdwTmp = dwPartSect;
	//Initialize BPB_FATSz32.
	pdwTmp  = (DWORD*)&Fat32Sector0.BPB_FATSz32[0];
	*pdwTmp = FatSize32(dwPartSect,32,Fat32Sector0.BPB_SecPerClus,2);
	dwFatSize = *pdwTmp;

	//Initialize BS_VolID.
	Fat32Sector0.BS_VolID[0]    = 0x0A;
	Fat32Sector0.BS_VolID[1]    = 0x0A;
	Fat32Sector0.BS_VolID[2]    = 0x0A;
	Fat32Sector0.BS_VolID[3]    = 0x0A;
	//Initialize volume label.
	for(i = 0;i < 11;i ++)
	{
		Fat32Sector0.BS_VolLab[i] = VolLabel[i];
	}
	//Copy to local buffer.
	memzero(&Buffer[0],512);
	memcpy((char*)&Buffer[0],(const char*)&Fat32Sector0,sizeof(Fat32Sector0));
	//Set the 0x55 and 0xAA flags.
	Buffer[510] = (UCHAR)0x55;
	Buffer[511] = (UCHAR)0xAA;

	//Write boot sector into target partition.
	if(!WriteDeviceSector(pPartition,
		0,
		1,
		Buffer))
	{
		return FALSE;
	}
	InProcess();

	if(!WriteDeviceSector(pPartition,
		6,     //Default backup sector number.
		1,
		Buffer))
	{
		return FALSE;
	}
	InProcess();

	//Initialize FAT table and root directory.
	return InitFat_Root(pPartition,
		32,
		dwFatSize,
		Fat32Sector0.BPB_SecPerClus,
		VolLabel);
	return FALSE;
}

//Entry point of format command.
DWORD format(__CMD_PARA_OBJ* pCmdParam)
{
#ifdef __CFG_SYS_DDF

	__DEVICE_OBJECT*    pPartition   = NULL;
	BYTE                VolLabel[11] = {'N','O',' ','N','A','M','E',' ',' ',' ',' '};
	int                 nVolLen,i;

	if(pCmdParam->byParameterNum < 3)  //At least three parameters should be specified.
	{
		formatusage();
		return FDISK_CMD_SUCCESS;
	}
	if(!StrCmp(pCmdParam->Parameter[2],"fat32"))
	{
		if(!StrCmp(pCmdParam->Parameter[2],"FAT32"))
		{
			formatusage();
			return FDISK_CMD_SUCCESS;
		}
	}
	if(pCmdParam->byParameterNum == 4)  //Volume label is specified.
	{
		nVolLen = StrLen(pCmdParam->Parameter[3]);
		if(nVolLen > 11)  //Trunk the first 11 characters as volume label.
		{
			for(i = 0;i < 11;i ++)
			{
				VolLabel[i] = pCmdParam->Parameter[3][i];
			}
		}
		else
		{
			for(i = 0;i < nVolLen;i ++)
			{
				VolLabel[i] = pCmdParam->Parameter[3][i];
			}
			for(i = nVolLen;i < 11;i ++)  //Pad the rest character as space.
			{
				VolLabel[i] = ' ';
			}
		}
	}
	pPartition = (__DEVICE_OBJECT*)IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		pCmdParam->Parameter[1],
		FILE_ACCESS_WRITE,
		0,
		NULL);
	if(NULL == pPartition)
	{
		PrintLine("  Can not open the specified partition object.");
		PrintLine("  Please make sure the partition object do exist.");
		return FDISK_CMD_SUCCESS;
	}
	//Verify the validity of partition oblect.
	if(0 == (pPartition->dwAttribute & DEVICE_TYPE_PARTITION))  //Not a partition object.
	{
		PrintLine("  The specified object is not a partition.");
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			(__COMMON_OBJECT*)pPartition);
		return FDISK_CMD_SUCCESS;
	}
	//Now try to format the partition object.
	GotoHome();
	ChangeLine();
	if(!Fat32Format(pPartition,VolLabel,0))  //Can not format the target partition object.
	{
		PrintLine("  Can not format the specified partition object.");
	}
	else
	{
		PrintLine("  Format the target partition successfully.");
	}
	IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
		(__COMMON_OBJECT*)pPartition);
	return FDISK_CMD_SUCCESS;
#else
	return FDISK_CMD_FAILED;
#endif
}

