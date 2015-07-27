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

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __PCI_DRV_H__
#include "pci_drv.h"
#endif

#include "kmemmgr.h"

static BOOL PciBusProbe()    //Probe if there is(are) PCI bus(es).
{
	DWORD   dwInit     = 0x80000000;

	__outd(CONFIG_REGISTER,dwInit);

	dwInit = __ind(DATA_REGISTER);

	if(dwInit == 0xFFFFFFFF)    //The HOST-PCI bridge is not exist.
		return FALSE;
	return TRUE;
}

static DWORD GetRangeSize(DWORD dwValue)  //Get the length of one base address register.
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

//
//The following routine filles resource array of a physical device object.
//
static VOID PciFillDevResources(DWORD dwConfigReg,__PHYSICAL_DEVICE* lpPhyDev)
{
	DWORD            dwTmp      = 0;
	DWORD            dwOrg      = 0;
	DWORD            dwSize     = 0;
	DWORD            dwIndex    = 0;
	DWORD            dwLoop     = 0;

	if((0 == dwConfigReg) || (NULL == lpPhyDev)) //Invalid parameters.
		return;

	dwConfigReg &= 0xFFFFFF00;    //Clear the offset part.
	dwConfigReg += PCI_CONFIG_OFFSET_BASE1;  //Pointing to the first base address register.
	for(dwLoop = 0;dwLoop < 6;dwLoop ++)  //Read resources.
	{
		__outd(CONFIG_REGISTER,dwConfigReg);
		dwOrg = __ind(DATA_REGISTER);     //Read and save the original value.
		//__outd(CONFIG_REGISTER,0xFFFFFFFF);
		__outd(DATA_REGISTER,0xFFFFFFFF);
		dwTmp = __ind(DATA_REGISTER);
		if((0 == dwTmp) || (0xFFFFFFFF == dwTmp)) //This base address register is not used.
		{
			dwConfigReg += 4;
			__outd(DATA_REGISTER,dwOrg);        //Restore original value.
			continue;
		}

		__outd(DATA_REGISTER,dwOrg);            //Restore original value.
		if(dwOrg & 0x00000001)  //IO Port range.
		{
			dwSize = GetRangeSize(dwTmp);
			dwOrg &= 0xFFFFFFFE;   //Clear the lowest bit.
			lpPhyDev->Resource[dwIndex].dwResType = RESOURCE_TYPE_IO;
			lpPhyDev->Resource[dwIndex].Dev_Res.IOPort.wStartPort = (WORD)dwOrg;
			lpPhyDev->Resource[dwIndex].Dev_Res.IOPort.wEndPort   = 
				(WORD)(dwOrg + dwSize - 1);
		}
		else    //Memory map range.
		{
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

	//
	//Now,we should obtain interrupt vector information from configure space.
	//
	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_INTLINE;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);
	if(0xFF == (UCHAR)dwTmp)        //No interrupt vector is present.
		return;
	lpPhyDev->Resource[dwIndex].dwResType = RESOURCE_TYPE_INTERRUPT;
	lpPhyDev->Resource[dwIndex].Dev_Res.ucVector  = (UCHAR)dwTmp;
	return;
}

//
//This routine reads resource information from a type 1 header,and fills them into
//physical device's resource array.Primary bus number,secondary bus number and sub-ordinate
//bus number are the most three parameters for PCI-PCI bridge.
//
static VOID PciFillBridgeResources(DWORD dwConfigReg,__PHYSICAL_DEVICE* lpPhyDev)
{
	__PCI_DEVICE_INFO*             pDevInfo  = NULL;
	DWORD                          dwTmp     = 0;

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

//
//The following routine is used to add one PCI device(physical device) into a PCI
//bus.
//It first create a physical device and a PCI information structure,then initializes
//them by reading data from configure space.
//
static VOID PciAddDevice(DWORD dwConfigReg,__SYSTEM_BUS* lpSysBus)
{
	__PCI_DEVICE_INFO*                    lpDevInfo      = NULL;
	__PHYSICAL_DEVICE*                    lpPhyDev       = NULL;
	DWORD                                 dwFlags        = 0;
	BOOL                                  bResult        = FALSE;
	//DWORD                                 dwLoop         = 0;
	DWORD                                 dwTmp          = 0;

	if((0 == dwConfigReg) || (NULL == lpSysBus)) //Invalid parameters.
		return;

	lpPhyDev = (__PHYSICAL_DEVICE*)KMemAlloc(sizeof(__PHYSICAL_DEVICE), KMEM_SIZE_TYPE_ANY);  //Create physical device.
	if(NULL == lpPhyDev)
		goto __TERMINAL;

	lpDevInfo = (__PCI_DEVICE_INFO*)KMemAlloc(sizeof(__PCI_DEVICE_INFO), KMEM_SIZE_TYPE_ANY);
	if(NULL == lpDevInfo)  //Can not allocate information structure.
		goto __TERMINAL;
	
	lpDevInfo->DeviceNum   = (dwConfigReg >> 11) & 0x0000001F;  //Get device number.
	lpDevInfo->FunctionNum = (dwConfigReg >> 8) & 0x00000007;   //Get function number.
	lpPhyDev->lpPrivateInfo = (LPVOID)lpDevInfo;  //Link device information to physical device.

	//
	//The following code initializes identifier member of physical device.
	//

//	_hx_printf("Read vendor ID and device ID\n");
	dwConfigReg &= 0xFFFFFF00;    //Clear offset part.
	dwConfigReg += PCI_CONFIG_OFFSET_VENDOR;
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);  //Read vendor ID and device ID.

	lpPhyDev->DevId.dwBusType               = BUS_TYPE_PCI;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.ucMask   = PCI_IDENTIFIER_MASK_ALL;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.wVendor  = (WORD)dwTmp;
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.wDevice  = (WORD)(dwTmp >> 16);



	//_hx_printf("Get revision ID and class code\n");
	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_REVISION;  //Get revision ID and class code.
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);

	lpPhyDev->DevId.Bus_ID.PCI_Identifier.dwClass = dwTmp;
	lpDevInfo->dwClassCode                 = dwTmp;  //Save to information struct also.


	//_hx_printf("Get header type\n");
	dwConfigReg &= 0xFFFFFF00;
	dwConfigReg += PCI_CONFIG_OFFSET_CACHELINESZ;    //Get header type.
	__outd(CONFIG_REGISTER,dwConfigReg);
	dwTmp = __ind(DATA_REGISTER);
	
	lpPhyDev->DevId.Bus_ID.PCI_Identifier.ucHdrType = (UCHAR)(dwTmp >> 16); //Get header type.

	//
	//The following code initializes the resource information required by device.
	//
	switch((dwTmp >> 16) & 0x7F)
	{
	case 0:         //Normal PCI device.
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_NORMAL;
		dwConfigReg &= 0xFFFFFF00;
		PciFillDevResources(dwConfigReg,lpPhyDev);
		bResult = TRUE;
		break;
	case 1:         //PCI-PCI bridge.
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_BRIDGE;
		dwConfigReg &= 0xFFFFFF00;
		PciFillBridgeResources(dwConfigReg,lpPhyDev);
		bResult = TRUE;
		break;
	case 2:        //CardBus-PCI bridge.
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_CARDBUS;
		bResult = TRUE;
		break;
	default:       //Not supported yet.
		lpDevInfo->dwDeviceType = PCI_DEVICE_TYPE_UNSUPPORTED;
		bResult = TRUE;
		break;
	}

	//
	//Now,we have finished to initialize resource information,so we insert the physical device
	//object into system bus.
	//
	lpPhyDev->lpHomeBus = lpSysBus;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpPhyDev->lpNext = lpSysBus->lpDevListHdr;
	lpSysBus->lpDevListHdr = lpPhyDev;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	if(!bResult)
	{
		if(lpPhyDev)  //Release memory.
			KMemFree((LPVOID)lpPhyDev,KMEM_SIZE_TYPE_ANY,0);
		if(lpDevInfo)
			KMemFree((LPVOID)lpDevInfo,KMEM_SIZE_TYPE_ANY,0);
	}
	return;
}

//
//The following routine scans all devices on one system bus,and inserts them into
//system bus's device list.
//
static VOID PciScanDevices(__SYSTEM_BUS* lpSysBus)
{
	//__PHYSICAL_DEVICE*              lpPhyDev      = NULL;
	//DWORD                           dwFlags;
	DWORD                           dwConfigReg   = 0x80000000;
	DWORD                           dwLoop        = 0;
	DWORD                           dwTmp         = 0;

	if(NULL == lpSysBus) //Parameter check.
		return;

	dwTmp =  lpSysBus->dwBusNum;
	dwTmp &= 0x000000FF;    //Only reserve the lowest 7 bits.
	dwConfigReg += (dwTmp << 16);  //Now,dwConfigReg value countains the bus number.

	//
	//The following code scans all devices and functions in one PCI bus,if there is
	//a device or function,it calles PciAddDevice routine to initialize it and add it
	//to system bus's device list.
	//
	for(dwLoop = 0;dwLoop < 0x100;dwLoop ++)  //For every devices and functions.
	{
		dwConfigReg &= 0xFFFF0000;
		dwConfigReg += (dwLoop << 8);  //Now,dwConfigReg countains the bus number,device number,
		                               //and function number.

		__outd(CONFIG_REGISTER,dwConfigReg);
		dwTmp = __ind(DATA_REGISTER);

		if(0xFFFFFFFF == dwTmp)        //The device or function does not exist.
			continue;
		/*
		//
		//Now,find one PCI device,first,we check if it is a multi-function device,
		//if so,enumerate all sub-functions of this physical device,and add them to
		//system bus,otherwise,add the current physical device to system bus,and
		//increment 7 to dwLoop to skip all functions bound to this physical device.
		//
		dwConfigReg += PCI_CONFIG_OFFSET_CACHELINESZ;
		__outd(CONFIG_REGISTER,dwConfigReg);
		dwTmp = __ind(DATA_REGISTER);        //Read one double word countaining the header
		                                     //type segment.
		dwConfigReg -= PCI_CONFIG_OFFSET_CACHELINESZ;
		if(dwTmp & 0x00800000)               //This is a multiple function device.
		{
			PciAddDevice(dwConfigReg,lpSysBus);
		}
		else                                 //This is a single function device.
		{
			PciAddDevice(dwConfigReg,lpSysBus);
			dwLoop += 7;                     //Skip all other functions of current device.
		}*/
		PciAddDevice(dwConfigReg,lpSysBus);  //Add to system device.
	}
	return;
}

//
//The following routine scans a PCI bus and all it's child buses(if exist),for each bus,use
//one element of SystemBus array(in DeviceManager object) to record it.
//This routine returns the largest bus number in this bus tree,if failed,returns MAX_DWORD_VALUE,
//which is 0xFFFFFFFF currently.
//
static DWORD PciScanBus(__DEVICE_MANAGER* lpDevMgr, __PHYSICAL_DEVICE* lpBridge, DWORD dwBusNum)
{
	DWORD               dwLoop                     = 0;
	DWORD               dwFlags                    = 0;
	//__PCI_DEVICE_INFO*  lpDevInfo                  = NULL;
	__PHYSICAL_DEVICE*  lpPhyDev                   = NULL;
	DWORD               dwSubNum                   = dwBusNum;

	if(NULL == lpDevMgr)  //Parameter check.
	{
		return MAX_DWORD_VALUE;
	}
	if(255 <= dwBusNum)   //Maximal bus number should not exceed 255.
	{
		return MAX_DWORD_VALUE;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(dwLoop = 0;dwLoop < MAX_BUS_NUM;dwLoop ++)
	{
		if(BUS_TYPE_NULL == lpDevMgr->SystemBus[dwLoop].dwBusType)
			break;
	}
	if(MAX_BUS_NUM == dwLoop) //Can not find a free bus,the number of system buses exceed
		                      //MAX_BUS_NUM.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return MAX_DWORD_VALUE;
	}

	//
	//Now,we have found a free system bus element,so initialize it.
	//
	lpDevMgr->SystemBus[dwLoop].dwBusType      = BUS_TYPE_PCI;
	lpDevMgr->SystemBus[dwLoop].dwBusNum       = dwBusNum;
	lpDevMgr->SystemBus[dwLoop].lpHomeBridge   = lpBridge;
	if(lpBridge)    //If the current bus is not root bus.
	{
		lpDevMgr->SystemBus[dwLoop].lpParentBus = lpBridge->lpHomeBus;
		lpBridge->lpChildBus                    = &lpDevMgr->SystemBus[dwLoop];
	}

	PciScanDevices(&lpDevMgr->SystemBus[dwLoop]);  //Scan all devices on this bus.
	lpPhyDev = lpDevMgr->SystemBus[dwLoop].lpDevListHdr;
	while(lpPhyDev)    //Now,scan all child buses of the current bus.
	{
		if(PCI_DEVICE_TYPE_BRIDGE == 
			((__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo)->dwDeviceType) //This is a PCI-PCI
			                                                             //bridge.
		{
			dwSubNum = PciScanBus(lpDevMgr,lpPhyDev,//++dwBusNum); //Scan child bus.
				((__PCI_DEVICE_INFO*)lpPhyDev->lpPrivateInfo)->ucSecondary);
		}
		lpPhyDev = lpPhyDev->lpNext;
	}

	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return dwSubNum;
}

//
//The implementation of PciBusDriver routine.
//This is the entry routine for all system bus drivers,it is called by DeviceManager when
//this object is initializing.
//This routine checks if there is(are) PCI bus(es) in system,if not,returns FALSE,else,scan
//all PCI buses,configure all devices reside on the PCI buses,and returns TRUE.
//
BOOL PciBusDriver(__DEVICE_MANAGER* lpDevMgr)
{
	BOOL          bResult           = FALSE;

	if(NULL == lpDevMgr)
		return FALSE;

	//
	//First,probe if there is PCI bus present.
	//
	bResult = PciBusProbe();
	if(!bResult)  //No PCI bus.
		return FALSE;

	//
	//Now,should scan all PCI devices.
	//
	PciScanBus(lpDevMgr,NULL,0);
	return TRUE;
}
