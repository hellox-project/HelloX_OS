//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : PCI_DRV.CPP
//    Module Funciton           : 
//                                This module countains PCI local system bus driver's implement-
//                                ation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "pci_drv.h"
#include "kmemmgr.h"
#include "stdio.h"

//Read configuration from PCI bus.
static DWORD PciReadConfig(__SYSTEM_BUS* bus, DWORD dwConfigReg,int size)
{
	DWORD dwTmp = 0;
	DWORD _cfg  = 0x80000000;
	
	dwTmp = bus->dwBusNum;
	dwTmp &= 0x000000FF;    //Only reserve the lowest 7 bits.
	_cfg  += (dwTmp << 16);  //Now,_cfg value countains the bus number.
	_cfg  += (dwConfigReg & 0x0000FFFF); //Now the device and function number are combined into _cfg.

	//Read PCI configuration.
	__outd(CONFIG_REGISTER, _cfg);
	switch (size)
	{
	case 4:
		return __ind(DATA_REGISTER);
	case 2:
		//return __inw(DATA_REGISTER);
		return (__ind(DATA_REGISTER) & 0xFFFF);
	case 1:
		//return __inb(DATA_REGISTER);
		return (__ind(DATA_REGISTER) & 0xFF);
	default:
		return 0;
	}
}

//Write configuration to PCI bus.
static BOOL PciWriteConfig(__SYSTEM_BUS* bus, DWORD dwConfigReg, DWORD dwVal,int size)
{
	DWORD dwTmp = 0;
	DWORD _cfg = 0x80000000;
	DWORD result;

	//Combine the bus number,dev number,function number and offset together.
	dwTmp = bus->dwBusNum;
	dwTmp &= 0x000000FF;
	_cfg += (dwTmp << 16);
	_cfg += (dwConfigReg & 0x0000FFFF);

	//Issue writting.
	__outd(CONFIG_REGISTER, _cfg);
	switch (size)
	{
	case 4:
		__outd(DATA_REGISTER, dwVal);
		result = __ind(DATA_REGISTER);
		break;
	case 2:
		__outw((WORD)dwVal, DATA_REGISTER);
		result = __inw(DATA_REGISTER);
		dwVal &= 0xFFFF;
		break;
	case 1:
		__outb((UCHAR)dwVal, DATA_REGISTER);
		result = __inb(DATA_REGISTER);
		dwVal &= 0xFF;
		break;
	default:
		return FALSE;
	}

	//Check if writting is successful.
	if (dwVal == result)
	{
		return TRUE;
	}
	//For debugging.
	_hx_printf("PCI_DRV: Write data to PCI space failed,val = %d,result = %d.\r\n",dwVal,result);
	return FALSE;
}

//Read configuration from a PCI device.
static DWORD PciReadDeviceConfig(__PHYSICAL_DEVICE* dev, DWORD dwConfigReg,int size)
{
	__SYSTEM_BUS* bus = NULL;
	DWORD dwTmp = 0;

	if (NULL == dev)
	{
		return 0;
	}
	bus = dev->lpHomeBus;
	if (NULL == bus)
	{
		return 0;
	}
	
	//Construct the device number of the configuration register.
	dwTmp = dev->dwNumber;
	dwTmp <<= 8;
	dwTmp |= (dwConfigReg & 0x000000FF);

	return bus->ReadConfig(bus, dwTmp,size);
}

//Write configuration to device.
static BOOL PciWriteDeviceConfig(__PHYSICAL_DEVICE* dev, DWORD dwConfigReg, DWORD dwVal,int size)
{
	__SYSTEM_BUS* bus = NULL;
	DWORD dwTmp = 0;

	if (NULL == dev)
	{
		return FALSE;
	}
	bus = dev->lpHomeBus;
	if (NULL == bus)
	{
		return FALSE;
	}

	//Construct the device number of the configuration register.
	dwTmp = dev->dwNumber;
	dwTmp <<= 8;
	dwTmp |= (dwConfigReg & 0x000000FF);

	return bus->WriteConfig(bus, dwTmp, dwVal,size);
}

//Probe if there is(are) PCI bus(es).
static BOOL PciBusProbe()
{
	DWORD   dwInit     = 0x80000000;

	__outd(CONFIG_REGISTER,dwInit);

	dwInit = __ind(DATA_REGISTER);

	if(dwInit == 0xFFFFFFFF)    //The HOST-PCI bridge is not exist.
		return FALSE;
	return TRUE;
}

//Get the length of one base address register.
static DWORD GetRangeSize(DWORD dwValue)
{
	DWORD    dwTmp        = dwValue;

	if(dwTmp & 0x00000001)  //The range is IO port range.
	{
		dwTmp &= 0xFFFFFFFE;  //Clear the lowest bit.
		dwTmp  = ~dwTmp;      //NOT calculation.
		dwTmp += 1;           //Increment 1.
		dwTmp &= 0xFFFF;      //Clear the up 16 bits.
		return dwTmp;
	}
	else
	{
		dwTmp &= 0xFFFFFFF0;  //Clear the lowest 4 bits.
		dwTmp  = ~dwTmp;
		dwTmp += 1;
		return dwTmp;
	}
}

/* The following routine filles resource array of a physical device object. */
static VOID PciFillDevResources(DWORD dwConfigReg,__PHYSICAL_DEVICE* lpPhyDev)
{
	DWORD dwTmp = 0, dwOrg = 0;
	DWORD dwSize = 0;
	DWORD dwIndex = 0;
	DWORD dwLoop = 0;

	if ((0 == dwConfigReg) || (NULL == lpPhyDev))
	{
		return;
	}

	dwConfigReg &= 0xFFFFFF00;    //Clear the offset part.
	dwConfigReg += PCI_CONFIG_OFFSET_BASE1;  //Pointing to the first base address register.
	for(dwLoop = 0;dwLoop < 6;dwLoop ++)  //Read resources.
	{
		__outd(CONFIG_REGISTER,dwConfigReg);
		/* Read and save the original value. */
		dwOrg = __ind(DATA_REGISTER);
		__outd(DATA_REGISTER,0xFFFFFFFF);
		dwTmp = __ind(DATA_REGISTER);
		/* This base address register is not used. */
		if((0 == dwTmp) || (0xFFFFFFFF == dwTmp))
		{
			dwConfigReg += 4;
			/* Restore original value. */
			__outd(DATA_REGISTER,dwOrg);
			continue;
		}

		/* Restore original value. */
		__outd(DATA_REGISTER,dwOrg);
		if(dwOrg & 0x00000001)
		{
			/* IO Port range. */
			dwSize = GetRangeSize(dwTmp);
			dwOrg &= 0xFFFFFFFE;   //Clear the lowest bit.
			lpPhyDev->Resource[dwIndex].dwResType = RESOURCE_TYPE_IO;
			lpPhyDev->Resource[dwIndex].Dev_Res.IOPort.wStartPort = (WORD)dwOrg;
			lpPhyDev->Resource[dwIndex].Dev_Res.IOPort.wEndPort   = 
				(WORD)(dwOrg + dwSize - 1);
		}
		else
		{
			/* Memory map range. */
			dwOrg &= 0xFFFFFFF0;    //Clear the lowest 4 bits.
			dwSize = GetRangeSize(dwTmp);
			lpPhyDev->Resource[dwIndex].dwResType = RESOURCE_TYPE_MEMORY;
			lpPhyDev->Resource[dwIndex].Dev_Res.MemoryRegion.lpStartAddr = (LPVOID)dwOrg;
			lpPhyDev->Resource[dwIndex].Dev_Res.MemoryRegion.lpEndAddr   = 
				(LPVOID)(dwOrg + dwSize - 1);
		}
		dwIndex ++;
		dwConfigReg += 4;
	}

	/* Obtain interrupt vector information from configure space. */
	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_INTLINE;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);
	/* No interrupt vector is present. */
	if (0xFF == (UCHAR)dwTmp)
	{
		return;
	}
	lpPhyDev->Resource[dwIndex].dwResType = RESOURCE_TYPE_INTERRUPT;
	lpPhyDev->Resource[dwIndex].Dev_Res.ucVector  = (UCHAR)dwTmp;
	return;
}

/*
 * This routine reads resource information from a type 1 header,and fills them into
 * physical device's resource array.Primary bus number,secondary bus number and sub-ordinate
 * bus number are the most three parameters for PCI-PCI bridge.
 */
static VOID PciFillBridgeResources(DWORD dwConfigReg,__PHYSICAL_DEVICE* lpPhyDev)
{
	__PCI_DEVICE_INFO* pDevInfo = NULL;
	DWORD dwTmp = 0;

	if(NULL == lpPhyDev)
	{
		return;
	}
	pDevInfo = (__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo;
	//Read the bridge's primary,secondary and subordinate bus number respectly.
	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_PRIMARY;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);
	pDevInfo->ucPrimary      = (UCHAR)dwTmp;
	pDevInfo->ucSecondary    = (UCHAR)(dwTmp >> 8);
	pDevInfo->ucSubordinate  = (UCHAR)(dwTmp >> 16);
	return;
}

/*
 * Add one PCI device(physical device) into a PCI
 * bus.
 * It first create a physical device and a PCI information structure,then initializes
 * them by reading data from configure space.
 */
static VOID PciAddDevice(DWORD dwConfigReg,__SYSTEM_BUS* lpSysBus)
{
	__PCI_DEVICE_INFO* lpDevInfo = NULL;
	__PHYSICAL_DEVICE* lpPhyDev = NULL;
	BOOL bResult = FALSE;
	DWORD dwTmp = 0;

	/* Basic checking. */
	if ((0 == dwConfigReg) || (NULL == lpSysBus))
	{
		return;
	}
	/* Only available in process of system initialization. */
	BUG_ON(!IN_SYSINITIALIZATION());

	/* Create physical device. */
	lpPhyDev = (__PHYSICAL_DEVICE*)KMemAlloc(sizeof(__PHYSICAL_DEVICE), KMEM_SIZE_TYPE_ANY);
	if (NULL == lpPhyDev)
	{
		goto __TERMINAL;
	}
	memset(lpPhyDev, 0, sizeof(__PHYSICAL_DEVICE));

	/* Create PCI device information structure. */
	lpDevInfo = (__PCI_DEVICE_INFO*)KMemAlloc(sizeof(__PCI_DEVICE_INFO), KMEM_SIZE_TYPE_ANY);
	if (NULL == lpDevInfo)
	{
		goto __TERMINAL;
	}
	memset(lpDevInfo, 0, sizeof(__PCI_DEVICE_INFO));
	
	lpDevInfo->DeviceNum   = (dwConfigReg >> 11) & 0x0000001F;  //Get device number.
	lpDevInfo->FunctionNum = (dwConfigReg >> 8) & 0x00000007;   //Get function number.
	lpPhyDev->lpPrivateInfo = (LPVOID)lpDevInfo;  //Link device information to physical device.

	//Save device number to physical device object.
	lpPhyDev->dwNumber = dwConfigReg & 0x0000FF00;
	lpPhyDev->dwNumber >>= 8;

	/* Initializes identifier member of physical device. */
	dwConfigReg &= 0xFFFFFF00;    //Clear offset part.
	dwConfigReg += PCI_CONFIG_OFFSET_VENDOR;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);  //Read vendor ID and device ID.

	lpPhyDev->DevId.dwBusType = BUS_TYPE_PCI;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_ALL;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.wVendor = (WORD)dwTmp;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.wDevice = (WORD)(dwTmp >> 16);

	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_REVISION;  //Get revision ID and class code.
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);

	lpPhyDev->DevId.Bus_ID.PCI_Identifier.dwClass = dwTmp;
	/* Save to information struct also. */
	lpDevInfo->dwClassCode = dwTmp;

	dwConfigReg &= 0xFFFFFF00;
	/* Get header type. */
	dwConfigReg += PCI_CONFIG_OFFSET_CACHELINESZ;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);
	
	/* Get header type. */
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.ucHdrType = (UCHAR)(dwTmp >> 16);

	/* Initializes the resource information required by device. */
	switch((dwTmp >> 16) & 0x7F)
	{
	case 0:         
		/* Normal PCI device. */
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_NORMAL;
		dwConfigReg &= 0xFFFFFF00;
		PciFillDevResources(dwConfigReg,lpPhyDev);
		bResult = TRUE;
		break;
	case 1:
		/* PCI-PCI bridge. */
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_BRIDGE;
		dwConfigReg &= 0xFFFFFF00;
		PciFillBridgeResources(dwConfigReg,lpPhyDev);
		bResult = TRUE;
		break;
	case 2:
		/* CardBus-PCI bridge. */
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_CARDBUS;
		bResult = TRUE;
		break;
	default:
		/* Not supported yet. */
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_UNSUPPORTED;
		bResult = TRUE;
		break;
	}

	//Set up physical device's configuration reading/writting routine.
	lpPhyDev->ReadDeviceConfig  = PciReadDeviceConfig;
	lpPhyDev->WriteDeviceConfig = PciWriteDeviceConfig;

	/*
	 * Now,we have finished to initialize resource information,just insert the physical device
	 * object into system bus.
	 */
	lpPhyDev->lpHomeBus = lpSysBus;
	bResult = DeviceManager.AppendDevice(&DeviceManager, lpPhyDev);

__TERMINAL:
	if(!bResult)
	{
		if (lpPhyDev)
		{
			KMemFree((LPVOID)lpPhyDev, KMEM_SIZE_TYPE_ANY, 0);
		}
		if (lpDevInfo)
		{
			KMemFree((LPVOID)lpDevInfo, KMEM_SIZE_TYPE_ANY, 0);
		}
	}
	return;
}

/*
 * Scans all devices on one system bus,and inserts them into
 * system bus's device list.
 */
static VOID PciScanDevices(__SYSTEM_BUS* lpSysBus)
{
	DWORD dwConfigReg = 0x80000000;
	DWORD dwLoop = 0, dwTmp = 0;

	if (NULL == lpSysBus)
	{
		return;
	}

	dwTmp =  lpSysBus->dwBusNum;
	dwTmp &= 0x000000FF;    //Only reserve the lowest 7 bits.
	dwConfigReg += (dwTmp << 16);  //Now,dwConfigReg value countains the bus number.

	/*
	 * The following code scans all devices and functions in one PCI bus,if there is
	 * a device or function,it calles PciAddDevice routine to initialize it and add it
	 * to system bus's device list.
	 */
	for(dwLoop = 0;dwLoop < 0x100;dwLoop ++)
	{
		dwConfigReg &= 0xFFFF0000;
		/* Now,dwConfigReg countains the bus number,device number, and function number. */
		dwConfigReg += (dwLoop << 8);

		__outd(CONFIG_REGISTER,dwConfigReg);
		dwTmp = __ind(DATA_REGISTER);

		/* The device or function does not exist. */
		if (0xFFFFFFFF == dwTmp)
		{
			continue;
		}
		/* Add the probed device into system. */
		PciAddDevice(dwConfigReg,lpSysBus);
	}
	return;
}

/*
 * Scans a PCI bus and all it's child buses(if exist).For each bus,use
 * one element of SystemBus array(in DeviceManager object) to record it.
 * This routine returns the largest bus number in this bus tree,if failed,
 * returns MAX_DWORD_VALUE, which is 0xFFFFFFFF currently.
 */
static DWORD PciScanBus(__DEVICE_MANAGER* lpDevMgr, __PHYSICAL_DEVICE* lpBridge, DWORD dwBusNum)
{
	DWORD dwLoop = 0;
	__PHYSICAL_DEVICE* lpPhyDev = NULL;
	DWORD dwSubNum = dwBusNum;

	/* Basic checking. */
	BUG_ON(NULL == lpDevMgr);
	/* 
	 * This routine can be called only in process of system 
	 * initialization,so no locks are obtained when initialize
	 * the physial device list or other global data structures.
	 */
	BUG_ON(!IN_SYSINITIALIZATION());
	if(255 <= dwBusNum)
	{
		/* Maximal bus number should not exceed 255. */
		return MAX_DWORD_VALUE;
	}

	for(dwLoop = 0;dwLoop < MAX_BUS_NUM;dwLoop ++)
	{
		if (BUS_TYPE_NULL == lpDevMgr->SystemBus[dwLoop].dwBusType)
		{
			break;
		}
	}
	if(MAX_BUS_NUM == dwLoop)
	{
		/* Can not find a free bus,the number of system buses exceed MAX_BUS_NUM. */
		return MAX_DWORD_VALUE;
	}

	/* Now we have found a free system bus element,initialize it. */
	lpDevMgr->SystemBus[dwLoop].dwBusType = BUS_TYPE_PCI;
	lpDevMgr->SystemBus[dwLoop].dwBusNum = dwBusNum;
	lpDevMgr->SystemBus[dwLoop].lpHomeBridge = lpBridge;
	if(lpBridge)
	{
		/* If the current bus is not root bus. */
		lpDevMgr->SystemBus[dwLoop].lpParentBus = lpBridge->lpHomeBus;
		lpBridge->lpChildBus = &lpDevMgr->SystemBus[dwLoop];
	}
	//Set PCI bus operations.
	lpDevMgr->SystemBus[dwLoop].ReadConfig  = PciReadConfig;
	lpDevMgr->SystemBus[dwLoop].WriteConfig = PciWriteConfig;

	/* Scan all devices on this bus. */
	PciScanDevices(&lpDevMgr->SystemBus[dwLoop]);
	lpPhyDev = lpDevMgr->SystemBus[dwLoop].lpDevListHdr;
	/* Scan all child buses of the current bus. */
	while(lpPhyDev)
	{
		if(PCI_DEVICE_TYPE_BRIDGE == 
			((__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo)->dwDeviceType)
		{
			/* PCI bridge. */
			dwSubNum = PciScanBus(lpDevMgr,
				lpPhyDev,
				((__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo)->ucSecondary);
		}
		lpPhyDev = lpPhyDev->lpNext;
	}
	return dwSubNum;
}

/*
 * This is the entry routine for all system bus drivers,it is called by DeviceManager when
 * it is initializing.
 * This routine checks if there is(are) PCI bus(es) in system,if not,returns FALSE,else,scan
 * all PCI buses,configure all devices reside on the PCI buses,and returns TRUE.
 */
BOOL PciBusDriver(__DEVICE_MANAGER* lpDevMgr)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == lpDevMgr);
	/* Only available in process of system initialization. */
	BUG_ON(!IN_SYSINITIALIZATION());

	/* Probe if there is PCI bus present. */
	bResult = PciBusProbe();
	if (!bResult)
	{
		/* No PCI bus presents. */
		return FALSE;
	}

	/* Scan all PCI device(s) attaching on the PCI bus system. */
	PciScanBus(lpDevMgr,NULL,0);
	return TRUE;
}
