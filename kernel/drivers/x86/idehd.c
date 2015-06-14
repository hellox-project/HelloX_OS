//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : EXTCMD.CPP
//    Module Funciton           : 
//    Description               : Hard disk driver object's dispatch routines 
//                                are implemented in this file.
//                                High level functions IDE HD related are implemented in
//                                this file.
//                                Low level routines,such as read sector and write sector,
//                                are implemented in another file.
//    Last modified Author      :
//    Last modified Date        : 26 JAN,2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : Today is 26 JAN,2009.In china's traditional canlendar,
//                                today is spring festival,i.e,new year.
//                                I like spring festival,in fact,most chinese like spring 
//                                festival,today can lead me recalling the time I was a child.
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "kapi.h"
#include "idehd.h"
#include "idebase.h"
#include "stdio.h"

//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//For each extension partition in hard disk,this function travels the
//extension partition table list,to install one by one into IOManager.
//How many logical partition(s) is returned.
static int InitExtension(int nHdNum,          //The hard disk number.
						 BYTE* pSector0,      //First sector of extension partition.
						 DWORD dwStartSector, //Position of this partition,in physical disk.
						 DWORD dwExtendStart, //Start position of extension partition.
						 int nBaseNumber,     //The partition base number.
						 __DRIVER_OBJECT* lpDrvObject)
{
	__DEVICE_OBJECT* pDevObject = NULL;
	__PARTITION_EXTENSION* pPe = NULL;
	BYTE* pStart = NULL;
	BYTE  buffer[512];  //Buffer used to read one sector.
	DWORD dwNextStart;  //Next extension's start sector if any.
	DWORD dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	int nPartitionNum = 0;

	if((NULL == pSector0) || (NULL == lpDrvObject)) //Invalid parameters.
	{
		goto __TERMINAL;
	}
	//Locate the partition table's position.
	pStart = pSector0 + 0x1BE;
	if(0 == *(pStart + 4)) //Partition type is zero,invalid.
	{
		goto __TERMINAL;
	}
	//Now create the partition extension object and initialize it.
	pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
	if(NULL == pPe)
	{
		goto __TERMINAL;
	}
	pPe->dwCurrPos     = 0;
	pPe->BootIndicator = *pStart;
	pStart += 4;
	pPe->PartitionType = *pStart;
	pStart += 4;
	pPe->dwStartSector = *(DWORD*)pStart;
	pStart += 4;
	pPe->dwSectorNum   = *(DWORD*)pStart;
	pStart += 4;  //Now pStart pointing to next partition entry.
	//Validate if all parameters are correct.
	if((pPe->dwStartSector == 0)  ||  //Should invalid.
	   (pPe->dwSectorNum   == 0))     //Impossible.
	{
		goto __TERMINAL;
	}
	switch(pPe->PartitionType)
	{
	case 0x0B:  //FAT32
	case 0x0C:  //FAT32
	case 0x0E:
		dwAttributes |= DEVICE_TYPE_FAT32;
		break;
	case 0x07:
		dwAttributes |= DEVICE_TYPE_NTFS;
		break;
	default:
		break;
	}
	pPe->dwStartSector += dwStartSector;  //Adjust the start sector to physical one.
	//Partiton information is OK,now create the device object.
	StrCpy(PARTITION_NAME_BASE,strDevName); //Form device name.
	strDevName[StrLen(PARTITION_NAME_BASE) - 1] += (CHAR)nBaseNumber;
	pDevObject = IOManager.CreateDevice(
		(__COMMON_OBJECT*)&IOManager,
		strDevName,
		dwAttributes,
		512,
		16384,
		16384,
		pPe,
		lpDrvObject);
	nPartitionNum += 1;

	//Now check the next table entry to see if there is another extension embeded.
	if(*(pStart + 4) == 0x05)  //Is a extension partition.
	{
		dwNextStart = dwExtendStart + (*(DWORD*)(pStart + 8));
		if(!ReadSector(
			nHdNum,
			dwNextStart,
			1,
			buffer))
		{
			goto __TERMINAL;
		}
		nPartitionNum += InitExtension(
			nHdNum,
			buffer,
			pPe->dwStartSector + pPe->dwSectorNum,//dwStartSector,
			dwExtendStart,
			nBaseNumber + 1,
			lpDrvObject);
	}

__TERMINAL:
	if(0 == nPartitionNum)  //Not any logical partiton is created.
	{
		if(pPe)
		{
			RELEASE_OBJECT(pPe);
		}
		if(pDevObject)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				pDevObject);
		}
	}
	return nPartitionNum;
}

//This function travels all partition table in sector 0 in current
//hard disk,from the MBR,to install each primary partition into IOManager.
//For extension partition,it calls InitExtension function to install.
static int InitPartitions(int nHdNum,
						  BYTE* pSector0,
						  __DRIVER_OBJECT* lpDrvObject)
{
	static int nPartNum = 0;  //How many partitons in current system.
	__DEVICE_OBJECT* pDevObject = NULL;
	__PARTITION_EXTENSION* pPe  = NULL;
	BYTE* pStart                = NULL;
	DWORD dwStartSector;  //Used to seek next partition.
	DWORD dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	int i;
	BYTE Buff[512];

	if((NULL == pSector0) || (NULL == lpDrvObject))  //Invalid parameter.
	{
		return 0;
	}
	pStart = pSector0 + 0x1be;  //Locate to the partition table start position.
	for(i = 0;i < 4;i ++) //Analyze each partition table entry.
	{
		if(*(pStart + 4) == 0) //Table entry is empty.
		{
			break;
		}
		if(*(pStart + 4) == 0x0F)  //Extension partiton.
		{
			dwStartSector = *(DWORD*)(pStart + 8);
			if(!ReadSector(nHdNum,dwStartSector,1,Buff))
			{
				break;
			}
			nPartNum += InitExtension(nHdNum,
				Buff,
				dwStartSector,
				dwStartSector,
				i,
				lpDrvObject);
			pStart += 16;  //Pointing to next partition table entry.
			continue;
		}
		pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
		if(NULL == pPe)  //Can not create object.
		{
			break;
		}
		pPe->dwCurrPos     = 0;
		pPe->BootIndicator = *pStart;
		pStart += 4;
		pPe->PartitionType = *pStart;
		pStart += 4;
		pPe->dwStartSector = *(DWORD*)pStart;
		pStart += 4;
		pPe->dwSectorNum   = *(DWORD*)pStart;
		pStart += 4;  //Pointing to next partition table entry.
		switch(pPe->PartitionType)
		{
		case 0x0B:   //FAT32.
		case 0x0C:   //Also FAT32.
		case 0x0E:   //FAT32 also.
			dwAttributes |= DEVICE_TYPE_FAT32;
			break;
		case 0x07:
			dwAttributes |= DEVICE_TYPE_NTFS;
			break;
		default:
			break;
		}

		//Create device object for this partition.
		//strcpy(strDevName,PARTITION_NAME_BASE);  // -------- CAUTION !!! ---------
		StrCpy(PARTITION_NAME_BASE,strDevName);
		strDevName[StrLen(PARTITION_NAME_BASE) - 1] += (BYTE)nPartNum;
		nPartNum ++;  //Increment the partition number.
		pDevObject = IOManager.CreateDevice(
			(__COMMON_OBJECT*)&IOManager,
			strDevName,
			dwAttributes,
			512,
			16384,
			16384,
			pPe,
			lpDrvObject);
		if(NULL == pDevObject)  //Can not create device object.
		{
			RELEASE_OBJECT(pPe);
			break;
		}
		dwAttributes = DEVICE_TYPE_PARTITION;  //Reset to default value,very important.
	}
	return nPartNum;
}

//------------------------------------------------------------------------

//DeviceRead for WINHD driver.
static DWORD DeviceRead(__COMMON_OBJECT* lpDrv,
						__COMMON_OBJECT* lpDev,
						__DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	DWORD dwResult = 0;
	DWORD dwStart  = 0;
	DWORD dwFlags;

	if((NULL == lpDev) || (NULL == lpDrcb))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	if(NULL == pPe)  //Should not occur,or else the OS kernel may have fatal error.
	{
		goto __TERMINAL;
	}
	//Check the validity of DRCB object transferred.
	if(DRCB_REQUEST_MODE_READ != lpDrcb->dwRequestMode) //Invalid operation action.
	{
		goto __TERMINAL;
	}
 	if((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		goto __TERMINAL;
	}
	if(0 != lpDrcb->dwOutputLen % pDevice->dwBlockSize)  //Only block size request is
		                                                 //legally.
	{
		goto __TERMINAL;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	//Check if current position is in device end.
	if(pPe->dwCurrPos == pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	dwResult = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	//Check if exceed the device boundry after read.
	if(pPe->dwCurrPos + dwResult >= pPe->dwSectorNum)  //Exceed end boundry.
	{
		dwResult = pPe->dwSectorNum - pPe->dwCurrPos;  //Only partial data of the
		                                               //requested can be read.
	}
	dwStart = pPe->dwCurrPos + pPe->dwStartSector;     //Read start this position,in sector number.
	pPe->dwCurrPos += dwResult;  //Adjust current pointer.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Now issue read command to read data from device.
	if(!ReadSector(0,dwStart,dwResult,(BYTE*)lpDrcb->lpOutputBuffer))  //Can not read data.
	{
 		dwResult = 0;
		goto __TERMINAL;
	}
	dwResult *= pDevice->dwBlockSize;  //dwResult now is the byte number just read.
__TERMINAL:
	return dwResult;
}

//DeviceWrite for WINHD driver.
static DWORD DeviceWrite(__COMMON_OBJECT* lpDrv,
						 __COMMON_OBJECT* lpDev,
						 __DRCB* lpDrcb)
{
	return 0;
}

//Several helper routines used by DeviceCtrl.
static DWORD __CtrlSectorRead(__COMMON_OBJECT* lpDrv,
							  __COMMON_OBJECT* lpDev,
							  __DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe     = NULL;
	__DEVICE_OBJECT*       pDevice = (__DEVICE_OBJECT*)lpDev;
	DWORD dwStartSector        = 0;
	DWORD dwSectorNum          = 0;
	int   nDiskNum             = 0;
	DWORD i;
	DWORD dwFlags;

	//Parameter validity checking.
	if((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		return 0;
	}
	if((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	//Get start sector and sector number to read.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	dwStartSector = *(DWORD*)(lpDrcb->lpInputBuffer);  //Input buffer stores the start pos.
	if(lpDrcb->dwOutputLen % pDevice->dwBlockSize)     //Always integral block size times is valid.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0;
	}
	dwSectorNum = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	//Check if the reading data exceed the device boundry.
	//if((dwStartSector + dwSectorNum) > (pPe->dwStartSector + pPe->dwSectorNum))
	if((dwStartSector + dwSectorNum) > pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0;
	}
	dwStartSector += pPe->dwStartSector;
	nDiskNum       = pPe->nDiskNum;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Now issue the reading command.
	for(i = 0;i < dwSectorNum;i ++)
	{
		if(!ReadSector(nDiskNum,dwStartSector + i,1,((BYTE*)lpDrcb->lpOutputBuffer) + 512*i))
		{
			return FALSE;
		}
	}
	//return ReadSector(nDiskNum,dwStartSector,dwSectorNum,(BYTE*)lpDrcb->lpOutputBuffer);
	return TRUE;
}

static DWORD __CtrlSectorWrite(__COMMON_OBJECT* lpDrv,
							   __COMMON_OBJECT* lpDev,
							   __DRCB* lpDrcb)
{	
	__PARTITION_EXTENSION* pPe     = NULL;
	__DEVICE_OBJECT*       pDevice = (__DEVICE_OBJECT*)lpDev;
	__SECTOR_INPUT_INFO*   psii    = NULL;
	DWORD dwStartSector        = 0;
	DWORD dwSectorNum          = 0;
	int   nDiskNum             = 0;
	DWORD i;
	DWORD dwFlags;

	//Parameter validity checking.
	if((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	psii = (__SECTOR_INPUT_INFO*)lpDrcb->lpInputBuffer;

	//Get start sector and sector number to read.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	if(psii->dwBufferLen % pDevice->dwBlockSize)   //Always integral block size times is valid.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0;
	}
	dwSectorNum = psii->dwBufferLen / pDevice->dwBlockSize;
	//Check if the reading data exceed the device boundry.
	if((psii->dwStartSector + dwSectorNum) > (pPe->dwStartSector + pPe->dwSectorNum))
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return 0;
	}
	dwStartSector = psii->dwStartSector + pPe->dwStartSector;
	nDiskNum = pPe->nDiskNum;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Now issue the reading command.
	for(i = 0;i < dwSectorNum;i ++)
	{
		if(!WriteSector(nDiskNum,dwStartSector + i,1,((BYTE*)psii->lpBuffer) + 512*i))
		{
			return FALSE;
		}
	}
	return TRUE;
}

//DeviceCtrl for WINHD driver.
//File systems based on this kind of device will use this routine to obtain one or
//several sectors,or write one or several sectors into device.
//For sector read,the dwOutputLen and lpOutputBuffer of DRCB object gives the output
//buffer,moreover,the dwInputLen and lpInputBuffer associated together gives the
//input parameter,which is the start position.The sector number can be rationed from
//dwOutputLen,which must be integral times of block size.
//For sector write,the lpInputBuffer gives the start sector number and the actual content
//to write.The lpInputBuffer is a pointer of __SECTOR_INPUT_INFO structure.
static DWORD DeviceCtrl(__COMMON_OBJECT* lpDrv,
						__COMMON_OBJECT* lpDev,
						__DRCB* lpDrcb)
{

	if((NULL == lpDev) || (NULL == lpDrcb))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	if(DRCB_REQUEST_MODE_IOCTRL != lpDrcb->dwRequestMode)  //Invalid request mode.
	{
		goto __TERMINAL;
	}
	switch(lpDrcb->dwCtrlCommand)
	{
	case IOCONTROL_READ_SECTOR:   //Read sector(s) from device.
		return __CtrlSectorRead(lpDrv,lpDev,lpDrcb);
		break;
	case IOCONTROL_WRITE_SECTOR:  //Write sector(s) into device.
		return __CtrlSectorWrite(lpDrv,lpDev,lpDrcb);
		break;
	default:
		break;
	}

__TERMINAL:
	return 0;
}

//A testing routine used to create hard disk partition table.
static VOID CreatePartition()
{
	BYTE Buff[512];
	__PARTITION_TABLE_ENTRY* pTableEntry = NULL;

	pTableEntry = (__PARTITION_TABLE_ENTRY*)(Buff + 0x1BE);

	pTableEntry->IfActive      = 0x00;
	pTableEntry->PartitionType = PARTITION_TYPE_NTFS;
	pTableEntry->dwStartSector = 16065;
	pTableEntry->dwTotalSector = 65535;
	pTableEntry ++;

	pTableEntry->IfActive      = 0x00;
	pTableEntry->PartitionType = PARTITION_TYPE_NTFS;
	pTableEntry->dwStartSector = 81600;
	pTableEntry->dwTotalSector = 65535;
	pTableEntry ++;

	pTableEntry->IfActive      = 0x00;
	pTableEntry->PartitionType = PARTITION_TYPE_NTFS;
	pTableEntry->dwStartSector = 147135;
	pTableEntry->dwTotalSector = 65535;
	pTableEntry ++;

	pTableEntry->IfActive      = 0x00;
	pTableEntry->PartitionType = PARTITION_TYPE_NTFS;
	pTableEntry->dwStartSector = 212670;
	pTableEntry->dwTotalSector = 65535;
	pTableEntry ++;

	//Set the terminate flag.
	Buff[510] = (BYTE)0xAA;
	Buff[511] = (BYTE)0x55;

	WriteSector(0,0,1,Buff);
}

//Interrupt handler for IDE.
static BOOL IDEIntHandler(LPVOID pParam,LPVOID pEsp)
{
	PrintLine("  IDE interrupt occurs.");
	return TRUE;
}

//The main entry point of WINHD driver.
BOOL IDEHdDriverEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;
	__PARTITION_EXTENSION *pPe = NULL;
	UCHAR Buff[512];
	WORD w = 0x0700;
	//DWORD dwLba;

	//Set operating functions for lpDrvObj first.
	lpDrvObj->DeviceRead    = DeviceRead;
	lpDrvObj->DeviceWrite   = DeviceWrite;
	lpDrvObj->DeviceCtrl    = DeviceCtrl;

	/*
	//Connect interrupt for IDE first.
	ConnectInterrupt(IDEIntHandler,
		NULL,
		0x2D);
	*/
	//Disable harddisk's directly reading functions since in it's will not work properly on
	//some platforms.
	//The IDE accessing will be conducted by BIOS trap instead.
	/*
	if(!IdeInitialize())
	{
		PrintLine("Can not initialize the IDE controller,you may not access HD directly.");
	}
	else
	{
		PrintLine("Initialize IDE controller successfully.");
	}

	//Identify the device.
	if(!Identify(0,(BYTE*)&Buff[0]))
	{
		PrintLine("Can not identify the hard disk device,you can not access IDE directly.");
	}
	else
	{
		PrintLine("Identify IDE controller successfully.");

		//Get the total sector number of current disk.
		dwLba = ((DWORD)Buff[123] << 24) + ((DWORD)Buff[122] << 16) 
			+ ((DWORD)Buff[121] << 8) + (DWORD)Buff[120];

		pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
		if(NULL == pPe)
		{
			PrintLine("Can not create RAW partition extension.");
			return FALSE;
		}
		//Initialize the partition extension object.
		pPe->BootIndicator  = 0x00;
		pPe->PartitionType  = PARTITION_TYPE_RAW;
		pPe->dwStartSector  = 0;
		pPe->dwSectorNum    = dwLba;  //Set the maximal sector number of this disk.
		pPe->nDiskNum       = 0;
		pPe->dwCurrPos      = 0;

		lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
			"\\\\.\\PHYSICALHARDDISK0",
			DEVICE_TYPE_STORAGE | DEVICE_TYPE_HARDDISK,  //Hard disk,as a kind of storage device.
			512,
			2048,
			2048,
			pPe,
			lpDrvObj);
		if(NULL == lpDevObject)  //Failed to create device object.
		{
			PrintLine("WINHD Driver: Failed to create device object for WINHD.");
			return FALSE;
		}
	}*/
	//Read the MBR from HD.
	if(!ReadSector(0,0,1,(BYTE*)&Buff[0]))
	{
		PrintLine("Can not read the MBR,all file system in this host may unavailable.");
		return FALSE;
	}
	InitPartitions(0,(BYTE*)&Buff[0],lpDrvObj);
	//A temporary routine used to establish the partition table.
	//CreatePartition();
	return TRUE;
}

#endif
