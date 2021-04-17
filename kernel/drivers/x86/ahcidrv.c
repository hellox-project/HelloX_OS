//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 17, 2021
//    Module Name               : ahcidrv.c
//    Module Funciton           : 
//                                Source code for AHCI(Advanced Host Controller
//                                Interface) driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PCI_DRV.H>

#include "ahcidef.h"
#include "ahci.h"

/* Helper routine to show out a port status. */
static void __show_ahci_port(__AHCI_PORT_OBJECT* port_obj)
{
	__HBA_PORT* pPort = port_obj->pConfigRegister;

	_hx_printf("  port index: %d\r\n", port_obj->port_index);
	_hx_printf("    port is/cmd: 0x%X/0x%X\r\n", pPort->is, pPort->cmd);
	_hx_printf("    port tfd/ie: 0x%X/0x%X\r\n", pPort->tfd, pPort->ie);
	_hx_printf("    port pxssts: 0x%X\r\n", pPort->ssts);
	_hx_printf("    port pxserr: 0x%X\r\n", pPort->serr);
	_hx_printf("    port i/o/xfer_err: %d/%d/%d\r\n", port_obj->sectors_in, 
		port_obj->sectors_out,
		port_obj->xfer_errors);
	_hx_printf("    port sts/dev type: 0x%X/%d\r\n", port_obj->port_status, port_obj->device_type);
	_hx_printf("    req q_len/int_num: %d/%d\r\n", port_obj->request_num, port_obj->int_raised);
	_hx_printf("    dhrs/pss/dss/ufs/dps/pcs: %d/%d/%d/%d/%d/%d\r\n",
		port_obj->int_dhrs_raised,
		port_obj->int_pss_raised,
		port_obj->int_dss_raised,
		port_obj->int_ufs_raised,
		port_obj->int_dps_raised,
		port_obj->int_pcs_raised);
}

/* Local helper to showout ahci controller's base information. */
static void __show_ahci_controller(__AHCI_CONTROLLER* pController)
{
	__HBA_MEM* base = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;

	BUG_ON(NULL == pController);
	base = pController->pConfigRegister;
	pPort = pController->pPortList;

	_hx_printf("AHCI controller information:\r\n");
	_hx_printf("  abr:%0x, size:%d, int:%d\r\n", pController->mem_start,
		pController->mem_size, pController->interrupt);
	_hx_printf("  Capabalities:0x%X/0x%X\r\n", base->cap, base->cap2);
	_hx_printf("  port implemented:%d\r\n", base->pi);
	_hx_printf("  ghc:0x%X\r\n", base->ghc);
	_hx_printf("  is/vs/bohc:0x%X/0x%X/0x%X\r\n", base->is, base->vs, base->bohc);
	
	/* Show out each implemented port of this device. */
	while (pPort)
	{
		__show_ahci_port(pPort);
		pPort = pPort->pNext;
	}
}

/* AHCI controller's specific show routine. */
static unsigned long __SpecificShow(__COMMON_OBJECT* pDriver, __COMMON_OBJECT* pDevice,
	__DRCB* pDrcb)
{
	__DEVICE_OBJECT* pAhciDevice = (__DEVICE_OBJECT*)pDevice;
	__AHCI_CONTROLLER* pController = NULL;

	BUG_ON(NULL == pAhciDevice);
	pController = pAhciDevice->lpDevExtension;
	BUG_ON(NULL == pController);
	__show_ahci_controller(pController);
	
	return 0;
}

/*
 * For each extension partition in ahci storage,
 * this function travels the extension partition table
 * and analyzes it, install partitons into system if the
 * corresponding entry is a valid one.
 * How many logical partition(s) is returned.
 */
static int InitExtension(
	__ATA_DISK_OBJECT* pHardDisk,
	__AHCI_PORT_OBJECT* pPort, 
	BYTE* pSector0,      /* First sector of extension partition. */
	DWORD dwStartSector, /* Position of this partition,in physical disk. */
	DWORD dwExtendStart, /* Start position of extension partition. */
	int nBaseNumber,     /* The partition base number. */
	__DRIVER_OBJECT* lpDrvObject)
{
	__DEVICE_OBJECT* pDevObject = NULL;
	__PARTITION_EXTENSION* pPe = NULL;
	BYTE* pStart = NULL;
	BYTE  buffer[512];
	DWORD dwNextStart;
	DWORD dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	DWORD dwFlags;
	int nPartitionNum = 0;

	BUG_ON((NULL == pSector0) || (NULL == lpDrvObject));

	/* Locate the partition table. */
	pStart = pSector0 + 0x1BE;
	if (0 == *(pStart + 4))
	{
		/* Invalid partition type. */
		goto __TERMINAL;
	}
	
	/* New a partition extension to manage it. */
	pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
	if (NULL == pPe)
	{
		goto __TERMINAL;
	}
	__INIT_SPIN_LOCK(pPe->spin_lock, "partition");
	pPe->dwCurrPos = 0;
	pPe->BootIndicator = *pStart;
	pStart += 4;
	pPe->PartitionType = *pStart;
	pStart += 4;
	pPe->dwStartSector = *(uint32_t*)pStart;
	pStart += 4;
	pPe->dwSectorNum = *(uint32_t*)pStart;
	pPe->pHardDisk = pHardDisk;
	/* Move to next entry. */
	pStart += 4;

	if ((pPe->dwStartSector == 0) || (pPe->dwSectorNum == 0))
	{
		/* Invalid value. */
		goto __TERMINAL;
	}
	switch (pPe->PartitionType)
	{
	case 0x0B:  //FAT32
	case 0x0C:  //FAT32
	case 0x0E:  //FAT32
		dwAttributes |= DEVICE_TYPE_FAT32;
		break;
	case 0x07:
		dwAttributes |= DEVICE_TYPE_NTFS;
		break;
	default:
		break;
	}
	/* Adjust the start sector to physical value. */
	pPe->dwStartSector += dwStartSector;

	/* Install it into system by associated with a device object. */
	strncpy(strDevName, ATA_PARTITION_NAME_BASE, MAX_DEV_NAME_LEN);
	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	strDevName[StrLen(ATA_PARTITION_NAME_BASE) - 1] += (CHAR)IOManager.dwPartitionNumber;
	IOManager.dwPartitionNumber += 1;
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	pDevObject = IOManager.CreateDevice(
		(__COMMON_OBJECT*)&IOManager,
		strDevName,
		dwAttributes,
		STORAGE_DEFAULT_SECTOR_SIZE,
		STORAGE_MAX_RW_SIZE,
		STORAGE_MAX_RW_SIZE,
		pPe,
		lpDrvObject);
	nPartitionNum += 1;

	/*
	 * Now check the next table entry to see if 
	 * there is another extension embeded.
	 */
	if (*(pStart + 4) == 0x05)
	{
		/* Is a extension partition, invoke self recursively. */
		dwNextStart = dwExtendStart + (*(DWORD*)(pStart + 8));
		if (!pPort->port_device_read(pPort,
			dwNextStart,
			0,
			1,
			buffer, NULL))
		{
			goto __TERMINAL;
		}
		nPartitionNum += InitExtension(
			pHardDisk,
			pPort,
			buffer,
			pPe->dwStartSector + pPe->dwSectorNum,
			dwExtendStart,
			nBaseNumber + 1,
			lpDrvObject);
	}

__TERMINAL:
	if (0 == nPartitionNum)
	{
		/* No partition created. */
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

/*
 * This function travels the partition table in sector 0
 * of the storage attaching ahci port, and install each primary 
 * partition into IOManager. For extension partitions,it invokes
 * InitExtension routine to analyze and install.
 */
static int InitPartitions(
	__ATA_DISK_OBJECT* pHardDisk,
	__AHCI_PORT_OBJECT* pPort,
	int nHdNum,
	BYTE* pSector0,
	__DRIVER_OBJECT* lpDrvObject)
{
	/* How many partitons in system. */
	static int nPartNum = 0;
	__DEVICE_OBJECT* pDevObject = NULL;
	__PARTITION_EXTENSION* pPe = NULL;
	BYTE* pStart = NULL;
	unsigned long dwStartSector = 0;
	unsigned long dwAttributes = DEVICE_TYPE_PARTITION;
	CHAR strDevName[MAX_DEV_NAME_LEN + 1];
	int i;
	unsigned long dwFlags;
	BYTE Buff[512];

	BUG_ON((NULL == pPort) || (NULL == pSector0) || (NULL == lpDrvObject));

	/* Partition table starts from 0x1be. */
	pStart = pSector0 + 0x1be;
	for (i = 0; i < 4; i++)
	{
		if (*(pStart + 4) == 0)
		{
			/* Empty slot. */
			break;
		}
		if (*(pStart + 4) == 0x0F)
		{
			/* Extension partiton. */
			dwStartSector = *(DWORD*)(pStart + 8);
			if(!pPort->port_device_read(pPort, dwStartSector, 0, 1, Buff, NULL))
			{
				_hx_printf("[%s]read sector fail\r\n", __func__);
				break;
			}
			nPartNum += InitExtension(
				pHardDisk,
				pPort,
				Buff,
				dwStartSector,
				dwStartSector,
				i,
				lpDrvObject);
			/* Move to next partition table entry. */
			pStart += 16;
			continue;
		}
		pPe = (__PARTITION_EXTENSION*)CREATE_OBJECT(__PARTITION_EXTENSION);
		if (NULL == pPe)
		{
			break;
		}
		__INIT_SPIN_LOCK(pPe->spin_lock, "partition");
		pPe->dwCurrPos = 0;
		pPe->BootIndicator = *pStart;
		pStart += 4;
		pPe->PartitionType = *pStart;
		pStart += 4;
		pPe->dwStartSector = *(uint32_t*)pStart;
		pStart += 4;
		pPe->dwSectorNum = *(uint32_t*)pStart;
		pPe->pHardDisk = pHardDisk;
		/* Move to next one. */
		pStart += 4;
		switch (pPe->PartitionType)
		{
		case 0x0B:   //FAT32.
		case 0x0C:   //FAT32.
		case 0x0E:   //FAT32.
			dwAttributes |= DEVICE_TYPE_FAT32;
			break;
		case 0x07:
			dwAttributes |= DEVICE_TYPE_NTFS;
			break;
		default:
			break;
		}

		/* 
		 * Create a partition device object and 
		 * install it into system. 
		 */
		strncpy(strDevName, ATA_PARTITION_NAME_BASE, MAX_DEV_NAME_LEN);
		__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		strDevName[StrLen(ATA_PARTITION_NAME_BASE) - 1] += (BYTE)IOManager.dwPartitionNumber;
		IOManager.dwPartitionNumber += 1;
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		/* Increment local partition number. */
		nPartNum++;
		pDevObject = IOManager.CreateDevice(
			(__COMMON_OBJECT*)&IOManager,
			strDevName,
			dwAttributes,
			STORAGE_DEFAULT_SECTOR_SIZE,
			STORAGE_MAX_RW_SIZE,
			STORAGE_MAX_RW_SIZE,
			pPe,
			lpDrvObject);
		if (NULL == pDevObject)
		{
			RELEASE_OBJECT(pPe);
			break;
		}
		/* Reset to default value since will be resued. */
		dwAttributes = DEVICE_TYPE_PARTITION;
	}
	if (0 == nPartNum)
	{
		/* No valid partition found. */
		__LOG("[%s]No valid partition found.\r\n", __func__);
	}
	return nPartNum;
}

/* 
 * Common xfer operations both for partition 
 * and for harddisk object.
 * The xfer can be input(read) when bInput is
 * TRUE, or output(write) when bInput is FALSE, the
 * direction of input is observed on the view of
 * CPU, not disk or partition.
 * the
 */
static unsigned long __device_xfer(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb, BOOL bInput)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	__ATA_DISK_OBJECT* pHardDisk = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;
	unsigned long dwResult = 0, dwStart = 0;
	unsigned long xfer_sz = 0;
	void* xfer_buffer = NULL;
	unsigned long dwFlags;

	if (bInput)
	{
		/* For read operation. */
		xfer_sz = lpDrcb->dwOutputLen;
		xfer_buffer = lpDrcb->lpOutputBuffer;
	}
	else {
		/* For write operation. */
		xfer_sz = lpDrcb->dwInputLen;
		xfer_buffer = lpDrcb->lpInputBuffer;
	}

	if (DEVICE_TYPE_PARTITION & pDevice->dwAttribute)
	{
		/* The device is a partition. */
		pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
		BUG_ON(NULL == pPe);
		pHardDisk = pPe->pHardDisk;
		BUG_ON(NULL == pHardDisk);
		pPort = (__AHCI_PORT_OBJECT*)pHardDisk->controller;
		BUG_ON(NULL == pPort);

		__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		if (pPe->dwCurrPos == pPe->dwSectorNum)
		{
			/* No data remained to xfer. */
			__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		dwResult = xfer_sz / pDevice->dwBlockSize;

		/* Check if exceed the device boundry after xfer. */
		if (pPe->dwCurrPos + dwResult >= pPe->dwSectorNum)
		{
			/* xfer partly. */
			dwResult = pPe->dwSectorNum - pPe->dwCurrPos;
		}
		/* xfer start position. */
		dwStart = pPe->dwCurrPos + pPe->dwStartSector;
		pPe->dwCurrPos += dwResult;
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	}
	if (DEVICE_TYPE_HARDDISK & pDevice->dwAttribute)
	{
		/* Device is a harddisk. */
		pHardDisk = (__ATA_DISK_OBJECT*)pDevice->lpDevExtension;
		BUG_ON(NULL == pHardDisk);
		pPort = (__AHCI_PORT_OBJECT*)pHardDisk->controller;
		BUG_ON(NULL == pPort);

		__ENTER_CRITICAL_SECTION_SMP(pHardDisk->spinlock, dwFlags);
		if (pHardDisk->curr_sector == pHardDisk->total_sectors)
		{
			/* Reach end of the disk. */
			__LEAVE_CRITICAL_SECTION_SMP(pHardDisk->spinlock, dwFlags);
			goto __TERMINAL;
		}
		dwResult = xfer_sz / pDevice->dwBlockSize;

		/* Check if exceed the device boundry after xfer. */
		if (pHardDisk->curr_sector + dwResult > pHardDisk->total_sectors)
		{
			/* xfer partly. */
			dwResult = pHardDisk->total_sectors - pHardDisk->curr_sector;
		}
		/* xfer start position. */
		dwStart = pHardDisk->curr_sector;
		pHardDisk->curr_sector += dwResult;
		__LEAVE_CRITICAL_SECTION_SMP(pHardDisk->spinlock, dwFlags);
	}

	/* Issue xfer command to device. */
	if (bInput)
	{
		/* for read operation. */
		if (!pPort->port_device_read(pPort, dwStart, 0, dwResult,
			(uint8_t*)xfer_buffer, lpDrcb))
		{
			dwResult = 0;
			goto __TERMINAL;
		}
		/* Adjust to byte counter. */
		dwResult *= pDevice->dwBlockSize;
	}
	else {
		/* For write operation. */
		if (!pPort->port_device_write(pPort, dwStart, 0, dwResult,
			(uint8_t*)xfer_buffer, lpDrcb))
		{
			dwResult = 0;
			goto __TERMINAL;
		}
		/* Adjust to byte counter. */
		dwResult *= pDevice->dwBlockSize;
	}

__TERMINAL:
	return dwResult;
}

/* Read raw data from partition object. */
static unsigned long __DeviceRead(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;

	/* Validates DRCB object. */
	if (DRCB_REQUEST_MODE_READ != lpDrcb->dwRequestMode)
	{
		return 0;
	}
	if ((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		return 0;
	}
	if (0 != lpDrcb->dwOutputLen % pDevice->dwBlockSize)
	{
		/* Only block size unit reading is valid. */
		return 0;
	}
	if (lpDrcb->dwOutputLen > pDevice->dwMaxReadSize)
	{
		/* Exceed the max read size. */
		return 0;
	}

	/* Just invoke the common xfer routine. */
	return __device_xfer(lpDrv, lpDev, lpDrcb, TRUE);
}

/* Write raw data into partition object. */
static unsigned long __DeviceWrite(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;

	/* Parameters checking. */
	BUG_ON((NULL == pDevice) || (NULL == lpDrcb));
	/* Validates DRCB object. */
	if (DRCB_REQUEST_MODE_WRITE != lpDrcb->dwRequestMode)
	{
		return 0;
	}
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	if (0 != lpDrcb->dwInputLen % pDevice->dwBlockSize)
	{
		/* Only block size unit reading is valid. */
		return 0;
	}

	/* Just invoke the common xfer routine. */
	return __device_xfer(lpDrv, lpDev, lpDrcb, FALSE);
}

/* Read sector(s) from a specified partition. */
static DWORD __CtrlSectorRead_Part(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	__ATA_DISK_OBJECT* pDisk = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;
	unsigned long dwStartSector = 0;
	unsigned long dwSectorNum = 0;
	unsigned long dwFlags;

	BUG_ON((NULL == lpDev) || (NULL == lpDrcb));
	/* Device must be a partition object. */
	BUG_ON(0 == (pDevice->dwAttribute & DEVICE_TYPE_PARTITION));
	/* Parameter validation. */
	if ((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		return 0;
	}
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	BUG_ON(NULL == pPe);
	pDisk = pPe->pHardDisk;
	BUG_ON(NULL == pDisk);
	pPort = (__AHCI_PORT_OBJECT*)pDisk->controller;
	BUG_ON(NULL == pPort);

	/*
	 * Get the start sector number of the HD,and how many
	 * sector(s) is(are) requested.
	 */
	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	/* Input buffer stores the start sector number. */
	dwStartSector = *(DWORD*)(lpDrcb->lpInputBuffer);
	if (lpDrcb->dwOutputLen % pDevice->dwBlockSize)
	{
		/* Request block size must be block aligned. */
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	dwSectorNum = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	/* Check if the requested data exceed the device boundry. */
	if ((dwStartSector + dwSectorNum) > pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	/* Convert to obsolute start address. */
	dwStartSector += pPe->dwStartSector;
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);

	/* Issue reading sector request. */
	if (!pPort->port_device_read(pPort, dwStartSector, 0, 
		dwSectorNum, lpDrcb->lpOutputBuffer, lpDrcb))
	{
		return 0;
	}
	
	/* Return the request sector number in success. */
	return dwSectorNum;
}

/* Write one or several sectors data into a specified partition. */
static DWORD __CtrlSectorWrite_Part(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	__SECTOR_INPUT_INFO* psii = NULL;
	__ATA_DISK_OBJECT* pDisk = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;
	unsigned long dwStartSector = 0;
	unsigned long dwSectorNum = 0;
	unsigned long dwFlags;
	BOOL bResult = FALSE;

	/* Validate parameters. */
	BUG_ON((NULL == pDevice) || (NULL == lpDrcb));
	/* Must be partition object. */
	BUG_ON(0 == (pDevice->dwAttribute & DEVICE_TYPE_PARTITION));
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	/* use sii stores input information. */
	psii = (__SECTOR_INPUT_INFO*)lpDrcb->lpInputBuffer;
	BUG_ON(NULL == psii);
	BUG_ON(NULL == psii->lpBuffer);
	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	BUG_ON(NULL == pPe);
	pDisk = pPe->pHardDisk;
	BUG_ON(NULL == pDisk);
	pPort = (__AHCI_PORT_OBJECT*)pDisk->controller;
	BUG_ON(NULL == pPort);

	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	/* Required data size must be aligned with block size. */
	if (psii->dwBufferLen % pDevice->dwBlockSize)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	dwSectorNum = psii->dwBufferLen / pDevice->dwBlockSize;
	/* Check if the writing data exceed the device boundry. */
	if ((psii->dwStartSector + dwSectorNum) > pPe->dwSectorNum)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	/* Calculates absolute start sector number. */
	dwStartSector = psii->dwStartSector + pPe->dwStartSector;
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);

	/* 
	 * Now carry out the writting request. 
	 * Please be noted that the data buffer is from psii.
	 */
	bResult = pPort->port_device_write(pPort, dwStartSector, 0, 
		dwSectorNum, psii->lpBuffer, lpDrcb);
	if (!bResult)
	{
		return 0;
	}
	/* Return the written sector number. */
	return dwSectorNum;
}

/* Read sector(s) from a specified DISK. */
static DWORD __CtrlSectorRead_Disk(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	__ATA_DISK_OBJECT* pDisk = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;
	unsigned long dwStartSector = 0;
	unsigned long dwSectorNum = 0;
	unsigned long dwFlags;

	BUG_ON((NULL == lpDev) || (NULL == lpDrcb));
	/* Device must be a partition object. */
	BUG_ON(0 == (pDevice->dwAttribute & DEVICE_TYPE_HARDDISK));
	/* Parameter validation. */
	if ((NULL == lpDrcb->lpOutputBuffer) || (0 == lpDrcb->dwOutputLen))
	{
		return 0;
	}
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	pDisk = (__ATA_DISK_OBJECT*)pDevice->lpDevExtension;
	BUG_ON(NULL == pDisk);
	pPort = (__AHCI_PORT_OBJECT*)pDisk->controller;
	BUG_ON(NULL == pPort);

	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	/* Input buffer stores the start sector number. */
	dwStartSector = *(DWORD*)(lpDrcb->lpInputBuffer);
	if (lpDrcb->dwOutputLen % pDevice->dwBlockSize)
	{
		/* Request block size must be block aligned. */
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	dwSectorNum = lpDrcb->dwOutputLen / pDevice->dwBlockSize;
	/* Check if the requested data exceed the device boundry. */
	if ((dwStartSector + dwSectorNum) > pDisk->total_sectors)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);

	/* Issue reading sector request. */
	if (!pPort->port_device_read(pPort, dwStartSector, 0, 
		dwSectorNum, lpDrcb->lpOutputBuffer, lpDrcb))
	{
		return 0;
	}

	/* Return the request sector number in success. */
	return dwSectorNum;
}

/* Write one or several sectors data into a specified DISK. */
static DWORD __CtrlSectorWrite_Disk(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	__SECTOR_INPUT_INFO* psii = NULL;
	__ATA_DISK_OBJECT* pDisk = NULL;
	__AHCI_PORT_OBJECT* pPort = NULL;
	unsigned long dwSectorNum = 0;
	unsigned long dwFlags;
	BOOL bResult = FALSE;

	/* Validate parameters. */
	BUG_ON((NULL == pDevice) || (NULL == lpDrcb));
	/* Must be partition object. */
	BUG_ON(0 == (pDevice->dwAttribute & DEVICE_TYPE_HARDDISK));
	if ((NULL == lpDrcb->lpInputBuffer) || (0 == lpDrcb->dwInputLen))
	{
		return 0;
	}
	/* use sii stores input information. */
	psii = (__SECTOR_INPUT_INFO*)lpDrcb->lpInputBuffer;
	BUG_ON(NULL == psii);
	BUG_ON(NULL == psii->lpBuffer);
	pDisk = (__ATA_DISK_OBJECT*)pDevice->lpDevExtension;
	BUG_ON(NULL == pDisk);
	pPort = (__AHCI_PORT_OBJECT*)pDisk->controller;
	BUG_ON(NULL == pPort);

	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	/* Required data size must be aligned with block size. */
	if (psii->dwBufferLen % pDevice->dwBlockSize)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	dwSectorNum = psii->dwBufferLen / pDevice->dwBlockSize;
	/* Check if the writing data exceed the device boundry. */
	if ((psii->dwStartSector + dwSectorNum) > pDisk->total_sectors)
	{
		__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
		return 0;
	}
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);

	/* Now carry out the writting request. */
	bResult = pPort->port_device_write(pPort, psii->dwStartSector, 0, dwSectorNum, 
		/* SII contains data buffer instead of drcb. */
		psii->lpBuffer, lpDrcb);
	if (!bResult)
	{
		return 0;
	}
	/* Return the written sector number. */
	return dwSectorNum;
}

/*
 * DeviceCtrl for hard disk object or partition object.
 * File systems may use this routine to obtain one or
 * several sectors, or write one or several sectors into device.
 * For sector read request, the dwOutputLen and lpOutputBuffer 
 * of DRCB object gives the output buffer, moreover,
 * the dwInputLen and lpInputBuffer associated together 
 * gives the input parameter, which is the start position.
 * The sector number can be calculated from
 * dwOutputLen,which must align with device's block size(512).
 * For sector write request,the lpInputBuffer gives 
 * the start sector number and the actual content
 * to write. The lpInputBuffer is a pointer of 
 * __SECTOR_INPUT_INFO structure, which stores the start
 * sector, sector number, and data buffer.
 * It will invoke partition sector operations or disk
 * sector operations according devie's attribute.
 */
static unsigned long __DeviceCtrl(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDevice = NULL;

	/* Parameters checking. */
	if ((NULL == lpDev) || (NULL == lpDrcb))
	{
		goto __TERMINAL;
	}
	if (DRCB_REQUEST_MODE_IOCTRL != lpDrcb->dwRequestMode)
	{
		goto __TERMINAL;
	}
	pDevice = (__DEVICE_OBJECT*)lpDev;

	switch (lpDrcb->dwCtrlCommand)
	{
		/* Read sector(s) from device. */
	case IOCONTROL_READ_SECTOR:
		/* Invoke approriate routines according device type. */
		if (pDevice->dwAttribute & DEVICE_TYPE_HARDDISK)
		{
			return __CtrlSectorRead_Disk(lpDrv, lpDev, lpDrcb);
		}
		if (pDevice->dwAttribute & DEVICE_TYPE_PARTITION)
		{
			return __CtrlSectorRead_Part(lpDrv, lpDev, lpDrcb);
		}
		break;

		/* Write sector(s) to device. */
	case IOCONTROL_WRITE_SECTOR:
		/* Dispatch to appropriate routines according device type. */
		if (pDevice->dwAttribute & DEVICE_TYPE_HARDDISK)
		{
			return __CtrlSectorWrite_Disk(lpDrv, lpDev, lpDrcb);
		}
		if (pDevice->dwAttribute & DEVICE_TYPE_PARTITION)
		{
			return __CtrlSectorWrite_Part(lpDrv, lpDev, lpDrcb);
		}
		break;

	default:
		break;
	}

__TERMINAL:
	return 0;
}

/* Helper routine to show one ATA disk's information. */
static void __show_ata_disk(__ATA_DISK_OBJECT* pDisk)
{
	__IDENTIFY_DEVICE_DATA* pIdentify = NULL;
	int i = 0;
	
	pIdentify = &pDisk->IdentifyData;

	/* Show serial number. */
	_hx_printf("  Serial number: ");
	for (i = 0; i < sizeof(pIdentify->SerialNumber); i++)
	{
		_hx_printf("%c", pIdentify->SerialNumber[i]);
	}
	_hx_printf("\r\n");

	/* Show model number. */
	_hx_printf("  Model number: ");
	for (i = 0; i < sizeof(pIdentify->ModelNumber); i++)
	{
		_hx_printf("%c", pIdentify->SerialNumber[i]);
	}
	_hx_printf("\r\n");

	/* Show out size information. */
	_hx_printf("  Current secotrs: %d\r\n", pIdentify->CurrentSectorCapacity);
	_hx_printf("  User addressable sector: %d\r\n", pIdentify->UserAddressableSectors);
	_hx_printf("  Total sectors: %d\r\n", pDisk->total_sectors);
	_hx_printf("  Current position: %d\r\n", pDisk->curr_sector);
}

/* 
 * Device specific show routine, it 
 * shows disk and logical partitions 
 * in one routine. 
 */
static unsigned long __specific_show(__COMMON_OBJECT* pDriver, 
	__COMMON_OBJECT* pDevObject,
	__DRCB* pDrcb)
{
	__ATA_DISK_OBJECT* pDisk = NULL;
	__DEVICE_OBJECT* pDev = (__DEVICE_OBJECT*)pDevObject;

	BUG_ON(NULL == pDev);
	if (pDev->dwAttribute & DEVICE_TYPE_HARDDISK)
	{
		/* Device is a disk. */
		pDisk = (__ATA_DISK_OBJECT*)pDev->lpDevExtension;
		BUG_ON(NULL == pDisk);
		__show_ata_disk(pDisk);
	}
	if (pDev->dwAttribute & DEVICE_TYPE_PARTITION)
	{
		/* Partiton object. */
	}
	return 0;
}

/* Open a device. */
static __COMMON_OBJECT* __DeviceOpen(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDeviceObject = (__DEVICE_OBJECT*)lpDev;
	__PARTITION_EXTENSION* pPartExt = NULL;
	__ATA_DISK_OBJECT* pDisk = NULL;
	__COMMON_OBJECT* pReturn = NULL;

	BUG_ON((NULL == pDeviceObject) || (NULL == lpDrv) || (NULL == lpDrcb));
	/* Open a partition object. */
	if (pDeviceObject->dwAttribute & DEVICE_TYPE_PARTITION)
	{
		pPartExt = (__PARTITION_EXTENSION*)pDeviceObject->lpDevExtension;
		BUG_ON(NULL == pPartExt);
		__ACQUIRE_SPIN_LOCK(pPartExt->spin_lock);
		if (pPartExt->open_counter > 0)
		{
			/* Device already opened. */
			__RELEASE_SPIN_LOCK(pPartExt->spin_lock);
			goto __TERMINAL;
		}
		/* Increase the open counter. */
		pPartExt->open_counter++;
		pPartExt->dwCurrPos = 0;
		pReturn = lpDev;
		__RELEASE_SPIN_LOCK(pPartExt->spin_lock);
		goto __TERMINAL;
	}
	/* Open a disk object. */
	if (pDeviceObject->dwAttribute & DEVICE_TYPE_HARDDISK)
	{
		pDisk = (__ATA_DISK_OBJECT*)pDeviceObject->lpDevExtension;
		BUG_ON(NULL == pDisk);
		__ACQUIRE_SPIN_LOCK(pDisk->spinlock);
		if (pDisk->open_counter > 0)
		{
			/* Already opened. */
			__RELEASE_SPIN_LOCK(pDisk->spinlock);
			goto __TERMINAL;
		}
		/* Increase open counter. */
		pDisk->open_counter++;
		pDisk->curr_sector = 0;
		pReturn = lpDev;
		__RELEASE_SPIN_LOCK(pDisk->spinlock);
		goto __TERMINAL;
	}

__TERMINAL:
	return pReturn;
}

/* Close a device. */
static unsigned long __DeviceClose(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__DEVICE_OBJECT* pDeviceObject = (__DEVICE_OBJECT*)lpDev;
	__PARTITION_EXTENSION* pPartExt = NULL;
	__ATA_DISK_OBJECT* pDisk = NULL;

	BUG_ON((NULL == pDeviceObject) || (NULL == lpDrv) || (NULL == lpDrcb));
	
	/* Close a partition object. */
	if (pDeviceObject->dwAttribute & DEVICE_TYPE_PARTITION)
	{
		pPartExt = (__PARTITION_EXTENSION*)pDeviceObject->lpDevExtension;
		BUG_ON(NULL == pPartExt);
		__ACQUIRE_SPIN_LOCK(pPartExt->spin_lock);
		if (pPartExt->open_counter > 0)
		{
			/* Device opened, decrease open counter. */
			pPartExt->open_counter--;
			pPartExt->dwCurrPos = 0;
		}
		__RELEASE_SPIN_LOCK(pPartExt->spin_lock);
		goto __TERMINAL;
	}

	/* Close a harddisk object. */
	if (pDeviceObject->dwAttribute & DEVICE_TYPE_HARDDISK)
	{
		pDisk = (__ATA_DISK_OBJECT*)pDeviceObject->lpDevExtension;
		BUG_ON(NULL == pDisk);
		__ACQUIRE_SPIN_LOCK(pDisk->spinlock);
		if (pDisk->open_counter > 0)
		{
			/* Already opened. */
			pDisk->open_counter--;
			pDisk->curr_sector = 0;
		}
		__RELEASE_SPIN_LOCK(pDisk->spinlock);
		goto __TERMINAL;
	}

__TERMINAL:
	return 0;
}

/* Seek current pointer to specified position. */
static unsigned long __DeviceSeek(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	return 0;
}

/* Get disk or partition's size. */
static unsigned long __DeviceSize(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev, __DRCB* lpDrcb)
{
	__PARTITION_EXTENSION* pPe = NULL;
	__DEVICE_OBJECT* pDevice = (__DEVICE_OBJECT*)lpDev;
	unsigned long ulHighPart = 0;
	unsigned long long total_size = 0;

	/* Parameters checking. */
	BUG_ON((NULL == lpDev) || (NULL == lpDrcb));
	if (!(pDevice->dwAttribute & DEVICE_TYPE_PARTITION))
	{
		/* Not a partition object. */
		lpDrcb->dwStatus = DRCB_STATUS_FAIL;
		return 0;
	}

	pPe = (__PARTITION_EXTENSION*)pDevice->lpDevExtension;
	BUG_ON(NULL == pPe);
	total_size = pPe->dwSectorNum;
	total_size *= STORAGE_DEFAULT_SECTOR_SIZE;
	/* get high part. */
	ulHighPart = (unsigned long)(total_size >> 32);
	if (lpDrcb->lpOutputBuffer)
	{
		*(unsigned long*)lpDrcb->lpOutputBuffer = ulHighPart;
	}
	/* Return the low part of size. */
	return (unsigned long)total_size;
}

/* 
 * Initializes the storage device attaching the 
 * specified HBA port. 
 * It reads the first sector of the storage, analyze
 * it and mount all partitions into system.
 */
BOOL InitPortStorage(__AHCI_PORT_OBJECT* pPort)
{
	__PARTITION_EXTENSION *pPe = NULL;
	UCHAR Buff[512];
	__DRIVER_OBJECT* pDriver = NULL;
	__ATA_DISK_OBJECT* pDisk = NULL;
	BOOL bResult = FALSE;
	/* total physical disks in system. */
	static unsigned long total_disks = 0;

	BUG_ON(NULL == pPort);
	/* Too many disks in system. */
	if (total_disks > 16384)
	{
		goto __TERMINAL;
	}

	/* 
	 * Create the driver object for both logical partitons
	 * and physical disk objects. 
	 */
	pDriver = (__DRIVER_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRIVER);
	if (!pDriver)
	{
		goto __TERMINAL;
	}
	if (!pDriver->Initialize((__COMMON_OBJECT*)pDriver))
	{
		goto __TERMINAL;
	}

	/* Initializes driver operations. */
	pDriver->DeviceRead = __DeviceRead;
	pDriver->DeviceWrite = __DeviceWrite;
	pDriver->DeviceCtrl = __DeviceCtrl;
	pDriver->DeviceSpecificShow = __specific_show;
	pDriver->DeviceOpen = __DeviceOpen;
	pDriver->DeviceClose = __DeviceClose;
	pDriver->DeviceSeek = __DeviceSeek;
	pDriver->DeviceSize = __DeviceSize;

	/* Identify the disk attaching on this port. */
	BUG_ON(NULL == pPort->port_device_identify);
	if (!pPort->port_device_identify(pPort, &Buff[0]))
	{
		_hx_printf("[%s]identify disk fail.\r\n", __func__);
		goto __TERMINAL;
	}
	/* 
	 * Create an ATA disk object to manage this 
	 * physical disk, this object will be set as
	 * disk object's extension.
	 */
	pDisk = (__ATA_DISK_OBJECT*)_hx_malloc(sizeof(__ATA_DISK_OBJECT));
	if (NULL == pDisk)
	{
		_hx_printf("[%s]out of kernel memory.\r\n", __func__);
		goto __TERMINAL;
	}
	/* Init the disk object. */
	memset(pDisk, 0, sizeof(__ATA_DISK_OBJECT));
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pDisk->spinlock, "ata_dsk");
#endif 
	unsigned long to_copy = sizeof(__IDENTIFY_DEVICE_DATA);
	if (to_copy > 512)
	{
		to_copy = 512;
	}
	memcpy(&pDisk->IdentifyData, Buff, to_copy);
	pDisk->curr_sector = 0;
	pDisk->total_sectors = pDisk->IdentifyData.UserAddressableSectors;
	pDisk->controller = (void*)pPort;

	/* Create the corresponding device object. */
	_hx_sprintf(Buff, ATA_DISK_NAME, total_disks);
	total_disks++;
	__DEVICE_OBJECT* pDiskDevice = IOManager.CreateDevice(
		(__COMMON_OBJECT*)&IOManager, Buff,
		DEVICE_TYPE_HARDDISK,
		STORAGE_DEFAULT_SECTOR_SIZE,
		STORAGE_MAX_RW_SIZE,
		STORAGE_MAX_RW_SIZE,
		pDisk,
		pDriver);
	if (NULL == pDiskDevice)
	{
		goto __TERMINAL;
	}

	/* Physical disk mount OK. */
	bResult = TRUE;

	/* Load first sector of the storage. */
	if(!pPort->port_device_read(pPort, 0, 0, 1, &Buff[0], NULL))
	{
		_hx_printf("[%s]Read MBR fail.\r\n", __func__);
		goto __TERMINAL;
	}

	/* 
	 * Analyze the MBR and try to find any 
	 * file partitions in the storage device, install
	 * them into system one by one.
	 */
	InitPartitions(pDisk, pPort, 0, (BYTE*)&Buff[0], pDriver);

__TERMINAL:
	if (!bResult)
	{
		if (pDriver)
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDriver);
		}
		if (pDisk)
		{
			_hx_free(pDisk);
		}
		if (pDiskDevice)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				pDiskDevice);
		}
	}
	return bResult;
}

/* Init driver object's operations. */
void SetDrvOperations(__DRIVER_OBJECT* pDriver)
{
	BUG_ON(NULL == pDriver);

	/* Set ahci controller's operations. */
	pDriver->DeviceSpecificShow = __SpecificShow;
}
