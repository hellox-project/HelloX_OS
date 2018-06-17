//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 04,2015
//    Module Name               : usbhd.c
//    Module Funciton           : 
//    Description               : USB storage(treated as HD in HelloX) driver
//                                source code are put into this file.
//    Last modified Author      :
//    Last modified Date        : 26 JAN,2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//
//***********************************************************************/

#include <StdAfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <kapi.h>

//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

#include "hxadapt.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "scsi.h"
#include "usbdev_storage.h"

//Only available when the USB Storage function is enabled.
#ifdef CONFIG_USB_STORAGE

//Global variables will be refered in this file.
extern block_dev_desc_t usb_dev_desc[];
extern int usb_max_devs;

//Global routines to access RAW USB storage deviceses.
extern unsigned long usb_stor_read(int device, lbaint_t blknr, lbaint_t blkcnt, void *buffer);
extern unsigned long usb_stor_write(int device, lbaint_t blknr, lbaint_t blkcnt, const void *buffer);

//Debug macro.
#define __USB_SECTOR_RW_DEBUG \
	_hx_printf("%s failed:dwStartSect = %d,dwSectNum = %d,pBuffer = 0x%X,dev = %d.\r\n", \
    __func__,dwStartSect,dwSectNum,pBuffer,dev);

//Local wraps of sector level reading and writing for USB device,it can not request data size
//exceeds USG_STORAGE_MAX_TRUNK_SIZE value.
static unsigned long __local_usbReadSector(int dev, DWORD dwStartSect, DWORD dwSectNum, BYTE* pBuffer)
{
	BYTE* pAlignedBuff = NULL;
	unsigned long ret = 0;

	if ((unsigned int)pBuffer != __ALIGN(((unsigned int)pBuffer), DEFAULT_CACHE_LINE_SIZE))
	{
		//Should allocate a buffer aligned with cache line.
		pAlignedBuff = _hx_aligned_malloc(dwSectNum * USB_STORAGE_SECTOR_SIZE, DEFAULT_CACHE_LINE_SIZE);
		if (NULL == pAlignedBuff)
		{
			return ret;
		}
	}
	if (pAlignedBuff)
	{
		ret = usb_stor_read(dev, dwStartSect, dwSectNum, pAlignedBuff);
		if (ret)
		{
			memcpy(pBuffer, pAlignedBuff, dwSectNum * USB_STORAGE_SECTOR_SIZE);
		}
		aligned_free(pAlignedBuff);
		if (!ret)
		{
			__USB_SECTOR_RW_DEBUG;
		}
		return ret;
	}
	
	//pBuffer is cache line aligned,just use it.
	ret = usb_stor_read(dev, dwStartSect, dwSectNum, pBuffer);
	if (!ret)
	{
		__USB_SECTOR_RW_DEBUG;
	}
	return ret;
}

/* 
 * Read one or several sector(s) from USB storage device. 
 * Any requested data length can be commited to this routine,it will be
 * splittered into several __local_usbReadSector's call in case
 * of the requested data length exceeds USG_STORAGE_MAX_TRUNK_SIZE.
 */
static unsigned long __usbReadSector(int dev, DWORD dwStartSect, DWORD dwSectNum, BYTE* pBuffer)
{
	unsigned long total_dl = dwSectNum * USB_STORAGE_SECTOR_SIZE;
	char* tmpBuff = pBuffer;
	unsigned long start_sect = dwStartSect;
	unsigned long sect_num = dwSectNum;
	unsigned long total_rd = 0, rd = 0;

	while (total_dl > USB_STORAGE_MAX_TRUNK_SIZE)
	{
		rd = __local_usbReadSector(dev, start_sect,
			USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE,
			tmpBuff);
		if (0 == rd)
		{
			return rd;
		}
		total_rd += rd;
		/* Update start sector number and data buffer's pointer. */
		start_sect += USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE;
		sect_num -= USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE;
		tmpBuff += USB_STORAGE_MAX_TRUNK_SIZE;
		total_dl -= USB_STORAGE_MAX_TRUNK_SIZE;
	}
	/* Read the remainding data. */
	rd = __local_usbReadSector(dev, start_sect, sect_num, tmpBuff);
	if (0 == rd)
	{
		return rd;
	}
	total_rd += rd; 
	return total_rd;
}

static unsigned long __local_usbWriteSector(int dev, DWORD dwStartSect, DWORD dwSectNum, BYTE* pBuffer)
{
	BYTE* pAlignedBuff = NULL;
	unsigned long ret = 0;

	if ((unsigned int)pBuffer != __ALIGN(((unsigned int)pBuffer), DEFAULT_CACHE_LINE_SIZE))
	{
		//Should allocate a buffer aligned with cache line.
		pAlignedBuff = _hx_aligned_malloc(dwSectNum * USB_STORAGE_SECTOR_SIZE, DEFAULT_CACHE_LINE_SIZE);
		if (NULL == pAlignedBuff)
		{
			return ret;
		}
	}
	if (pAlignedBuff)
	{
		memcpy(pAlignedBuff,pBuffer,dwSectNum * USB_STORAGE_SECTOR_SIZE);
		ret = usb_stor_write(dev, dwStartSect, dwSectNum, pAlignedBuff);
		aligned_free(pAlignedBuff);
		if (!ret)
		{
			__USB_SECTOR_RW_DEBUG;
		}
		return ret;
	}
	
	//pBuffer is aligned,just use it.
	ret = usb_stor_write(dev, dwStartSect, dwSectNum, pBuffer);
	if (!ret)
	{
		__USB_SECTOR_RW_DEBUG;
	}
	return ret;
}

/*
* Write one or several sector(s) from USB storage device.
* Any requested data length can be commited to this routine,it will be
* splittered into several __local_usbReadSector's call in case
* of the requested data length exceed USG_STORAGE_MAX_TRUNK_SIZE.
*/
static unsigned long __usbWriteSector(int dev, DWORD dwStartSect, DWORD dwSectNum, BYTE* pBuffer)
{
	unsigned long total_dl = dwSectNum * USB_STORAGE_SECTOR_SIZE;
	char* tmpBuff = pBuffer;
	unsigned long start_sect = dwStartSect;
	unsigned long sect_num = dwSectNum;
	unsigned long total_rt = 0, rt = 0;

	while (total_dl > USB_STORAGE_MAX_TRUNK_SIZE)
	{
		rt = __local_usbWriteSector(dev, start_sect,
			USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE,
			tmpBuff);
		if (0 == rt)
		{
			return rt;
		}
		total_rt += rt;
		/* Update start sector number and data buffer's pointer. */
		start_sect += USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE;
		sect_num -= USB_STORAGE_MAX_TRUNK_SIZE / USB_STORAGE_SECTOR_SIZE;
		tmpBuff += USB_STORAGE_MAX_TRUNK_SIZE;
		total_dl -= USB_STORAGE_MAX_TRUNK_SIZE;
	}
	/* Read the remainding data. */
	rt = __local_usbWriteSector(dev, start_sect, sect_num, tmpBuff);
	if (0 == rt)
	{
		return rt;
	}
	total_rt += rt;
	return total_rt;
}

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
	BYTE  buffer[USB_STORAGE_SECTOR_SIZE];  //Buffer used to read one sector.
	DWORD dwNextStart;  //Next extension's start sector if any.
	DWORD dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	DWORD dwFlags;
	int nPartitionNum = 0;

	if ((NULL == pSector0) || (NULL == lpDrvObject)) //Invalid parameters.
	{
		goto __TERMINAL;
	}
	//Locate the partition table's position.
	pStart = pSector0 + 0x1BE;
	if (0 == *(pStart + 4)) //Partition type is zero,invalid.
	{
		goto __TERMINAL;
	}
	//Now create the partition extension object and initialize it.
	pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
	if (NULL == pPe)
	{
		goto __TERMINAL;
	}
	pPe->nDiskNum = nHdNum;  //Harddisk number this partion resides.
	pPe->dwCurrPos = 0;
	pPe->BootIndicator = *pStart;
	pStart += 4;
	pPe->PartitionType = *pStart;
	pStart += 4;
	pPe->dwStartSector = *(DWORD*)pStart;
	pStart += 4;
	pPe->dwSectorNum = *(DWORD*)pStart;
	pStart += 4;  //Now pStart pointing to next partition entry.
	//Validate if all parameters are correct.
	if ((pPe->dwStartSector == 0) ||  //Should invalid.
		(pPe->dwSectorNum == 0))     //Impossible.
	{
		goto __TERMINAL;
	}
	switch (pPe->PartitionType)
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
	StrCpy(PARTITION_NAME_BASE, strDevName); //Form device name.
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	strDevName[StrLen(PARTITION_NAME_BASE) - 1] += (CHAR)IOManager.dwPartitionNumber;
	IOManager.dwPartitionNumber += 1;
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	pDevObject = IOManager.CreateDevice(
		(__COMMON_OBJECT*)&IOManager,
		strDevName,
		dwAttributes,
		USB_STORAGE_SECTOR_SIZE,
		16384,
		16384,
		pPe,
		lpDrvObject);
	nPartitionNum += 1;

	//Now check the next table entry to see if there is another extension embeded.
	if (*(pStart + 4) == 0x05)  //Is a extension partition.
	{
		dwNextStart = dwExtendStart + (*(DWORD*)(pStart + 8));
		if (!__usbReadSector(
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
	if (0 == nPartitionNum)  //Not any logical partiton is created.
	{
		if (pPe)
		{
			RELEASE_OBJECT(pPe);
		}
		if (pDevObject)
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
	__DEVICE_OBJECT* pDevObject = NULL;
	__PARTITION_EXTENSION* pPe = NULL;
	BYTE* pStart = NULL;
	DWORD dwStartSector;  //Used to seek next partition.
	DWORD dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	int i;
	DWORD dwFlags;
	BYTE Buff[USB_STORAGE_SECTOR_SIZE];

	if ((NULL == pSector0) || (NULL == lpDrvObject))  //Invalid parameter.
	{
		return 0;
	}
	pStart = pSector0 + 0x1be;  //Locate to the partition table start position.
	for (i = 0; i < 4; i++) //Analyze each partition table entry.
	{
		if (*(pStart + 4) == 0) //Table entry is empty.
		{
			break;
		}
		if (*(pStart + 4) == 0x0F)  //Extension partiton.
		{
			dwStartSector = *(DWORD*)(pStart + 8);
			if (!__usbReadSector(nHdNum, dwStartSector, 1, Buff))
			{
				break;
			}
			InitExtension(nHdNum,
				Buff,
				dwStartSector,
				dwStartSector,
				i,
				lpDrvObject);
			pStart += 16;  //Pointing to next partition table entry.
			continue;
		}
		pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
		if (NULL == pPe)  //Can not create object.
		{
			break;
		}
		pPe->nDiskNum = nHdNum;  //The harddisk number this partition resides.
		pPe->dwCurrPos = 0;
		pPe->BootIndicator = *pStart;
		pStart += 4;
		pPe->PartitionType = *pStart;
		pStart += 4;
		pPe->dwStartSector = *(DWORD*)pStart;
		pStart += 4;
		pPe->dwSectorNum = *(DWORD*)pStart;
		pStart += 4;  //Pointing to next partition table entry.
		switch (pPe->PartitionType)
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
		StrCpy(PARTITION_NAME_BASE, strDevName);
		__ENTER_CRITICAL_SECTION(NULL, dwFlags);
		strDevName[StrLen(PARTITION_NAME_BASE) - 1] += (BYTE)IOManager.dwPartitionNumber;
		IOManager.dwPartitionNumber += 1;
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		//nPartNum++;  //Increment the partition number.
		pDevObject = IOManager.CreateDevice(
			(__COMMON_OBJECT*)&IOManager,
			strDevName,
			dwAttributes,
			USB_STORAGE_SECTOR_SIZE,
			16384,
			16384,
			pPe,
			lpDrvObject);
		if (NULL == pDevObject)  //Can not create device object.
		{
			RELEASE_OBJECT(pPe);
			break;
		}
		dwAttributes = DEVICE_TYPE_PARTITION;  //Reset to default value,very important.
	}
	return 0;
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
	DWORD dwStart = 0;
	DWORD dwFlags;

	if ((NULL == lpDev) || (NULL == lpDrcb))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	if (NULL == pPe)  //Should not occur,or else the OS kernel may have fatal error.
	{
		goto __TERMINAL;
	}
	//Check the validity of DRCB object transferred.
	if (DRCB_REQUEST_MODE_READ != lpDrcb->dwRequestMode) //Invalid operation action.
	{
		goto __TERMINAL;
	}
	if ((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		goto __TERMINAL;
	}
	if (0 != lpDrcb->dwOutputLen % pDevice->dwBlockSize)  //Only block size request is
		//legally.
	{
		goto __TERMINAL;
	}
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	//Check if current position is in device end.
	if (pPe->dwCurrPos == pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		goto __TERMINAL;
	}
	dwResult = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	//Check if exceed the device boundry after read.
	if (pPe->dwCurrPos + dwResult >= pPe->dwSectorNum)  //Exceed end boundry.
	{
		dwResult = pPe->dwSectorNum - pPe->dwCurrPos;  //Only partial data of the
		//requested can be read.
	}
	dwStart = pPe->dwCurrPos + pPe->dwStartSector;     //Read start this position,in sector number.
	pPe->dwCurrPos += dwResult;  //Adjust current pointer.
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	//Now issue read command to read data from device.
	if (!__usbReadSector(pPe->nDiskNum, dwStart, dwResult, (BYTE*)lpDrcb->lpOutputBuffer))  //Can not read data.
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
static DWORD __CtrlSectorRead(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,__DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT*       pDevice = (__DEVICE_OBJECT*)lpDev;
	DWORD dwStartSector = 0;
	DWORD dwSectorNum = 0;
	DWORD dwFlags;

	//Parameter validity checking.
	if ((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		return 0;
	}
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	//Get start sector and sector number to read.
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	dwStartSector = *(DWORD*)(lpDrcb->lpInputBuffer);  //Input buffer stores the start pos.
	if (lpDrcb->dwOutputLen % pDevice->dwBlockSize)     //Always integral block size times is valid.
	{
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		_hx_printf("%s:dwOutputLen % pDevice->dwBlockSize != 0.\r\n", __FUNCTION__);
		return 0;
	}
	dwSectorNum = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	//Check if the reading data exceed the device boundry.
	//if((dwStartSector + dwSectorNum) > (pPe->dwStartSector + pPe->dwSectorNum))
	if ((dwStartSector + dwSectorNum) > pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		_hx_printf("%s:Exceed the partition boundary,StartSector = %d,SectorNum = %d,TotalSector = %d.\r\n",
			__FUNCTION__, dwStartSector, dwSectorNum, pPe->dwSectorNum);
		return 0;
	}
	dwStartSector += pPe->dwStartSector;
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	//Now issue the reading command.
	return __usbReadSector(pPe->nDiskNum, dwStartSector, dwSectorNum, (BYTE*)lpDrcb->lpOutputBuffer);
}

static DWORD __CtrlSectorWrite(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,__DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT*       pDevice = (__DEVICE_OBJECT*)lpDev;
	__SECTOR_INPUT_INFO*   psii = NULL;
	DWORD dwStartSector = 0;
	DWORD dwSectorNum = 0;
	DWORD dwFlags;

	//Parameter validity checking.
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	psii = (__SECTOR_INPUT_INFO*)lpDrcb->lpInputBuffer;

	//Get start sector and sector number to read.
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	if (psii->dwBufferLen % pDevice->dwBlockSize)   //Always integral block size times is valid.
	{
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		return 0;
	}
	dwSectorNum = psii->dwBufferLen / pDevice->dwBlockSize;
	//Check if the reading data exceed the device boundry.
	if ((psii->dwStartSector + dwSectorNum) > (pPe->dwStartSector + pPe->dwSectorNum))
	{
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		return 0;
	}
	dwStartSector = psii->dwStartSector + pPe->dwStartSector;
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	//Now issue the reading command.
	return __usbWriteSector(pPe->nDiskNum, dwStartSector, dwSectorNum, (BYTE*)psii->lpBuffer);
}

//DeviceCtrl for USBHD driver.
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

	if ((NULL == lpDev) || (NULL == lpDrcb))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	if (DRCB_REQUEST_MODE_IOCTRL != lpDrcb->dwRequestMode)  //Invalid request mode.
	{
		goto __TERMINAL;
	}
	switch (lpDrcb->dwCtrlCommand)
	{
	case IOCONTROL_READ_SECTOR:   //Read sector(s) from device.
		return __CtrlSectorRead(lpDrv, lpDev, lpDrcb);
		break;
	case IOCONTROL_WRITE_SECTOR:  //Write sector(s) into device.
		return __CtrlSectorWrite(lpDrv, lpDev, lpDrcb);
		break;
	default:
		break;
	}

__TERMINAL:
	return 0;
}

/* 
 * Probe if a USB device is a storage type device,and 
 * initialize it if so.
 * It's in usb_storage.c file,just be called in driver
 * entry routine of USB storage device,so just make it
 * visible in this file.
 */
extern int usb_stor_probe_device(__PHYSICAL_DEVICE* pPhyDev);

//The main entry point of WINHD driver.
BOOL USBStorage_DriverEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__PHYSICAL_DEVICE* pPhyDev = NULL;
	__IDENTIFIER id;
	UCHAR Buff[USB_STORAGE_SECTOR_SIZE];
	BOOL bResult = FALSE;
	struct usb_device* pUsbDev = NULL;
	int i = 0;

	/* 
	 * Search all USB storage device(s) in system,by specifying
	 * the device ID.
	 */
	id.dwBusType = BUS_TYPE_USB;
	id.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_INTERFACECLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACESUBCLASS;
	id.Bus_ID.USB_Identifier.bInterfaceClass = USB_CLASS_MASS_STORAGE;
	for (id.Bus_ID.USB_Identifier.bInterfaceSubClass = US_SC_MIN;
		id.Bus_ID.USB_Identifier.bInterfaceSubClass <= US_SC_MAX;
		id.Bus_ID.USB_Identifier.bInterfaceSubClass++)
	{
		pPhyDev = USBManager.GetUsbDevice(&id, NULL);
		while (pPhyDev)
		{
			_hx_printf("Get one USB storage device:[isc = %d].\r\n",
				id.Bus_ID.USB_Identifier.bInterfaceSubClass);
			pUsbDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
			BUG_ON(NULL == pUsbDev);
			/* Make farther probing of this device. */
			if (usb_stor_probe_device(pPhyDev))
			{
				/* 
				 * No more resource to hold the storage device if
				 * usb_stor_probe_device returns no zero,so just
				 * break out.
				 */
				break;
			}
			/* Try to locate next one. */
			pPhyDev = USBManager.GetUsbDevice(&id, pPhyDev);
		}
	}

	//Set operating functions for lpDrvObj first.
	lpDrvObj->DeviceRead = DeviceRead;
	lpDrvObj->DeviceWrite = DeviceWrite;
	lpDrvObj->DeviceCtrl = DeviceCtrl;

	//Trave each USB block storage device(s) in usb_dev_desc_t array and try to analyze it.
	for (i = 0; i < usb_max_devs; i++)
	{
		//Read the MBR from USB storage device.
		if (!__usbReadSector(i, 0, 1, (BYTE*)&Buff[0]))
		{
			_hx_printf("%s: Can not read MBR from USB HD [%d].\r\n", i);
			return FALSE;
		}

		/* 
		 * Analyze the MBR of USB HD and establish each partition
		 * object if there is. 
		 */
		InitPartitions(i, (BYTE*)&Buff[0], lpDrvObj);
		bResult = TRUE;
	}

	return bResult;
}

#endif  //CONFIG_USB_STORAGE

#endif  //__CFG_SYS_DDF
