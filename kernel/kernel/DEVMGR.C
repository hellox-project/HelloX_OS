//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : DEVMGR.CPP
//    Module Funciton           : 
//                                This module countains device manager object's implemen-
//                                tation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "pci_drv.h"
#include "kapi.h"


//Only DDF(Device Driver Framework) is enabled the following code
//will be included in OS kernel.
#ifdef __CFG_SYS_DDF

//
//Pre-declaration of local routines.
//
static BOOL DevMgrInitialize(__DEVICE_MANAGER*);
static BOOL CheckPortRegion(__DEVICE_MANAGER*,__RESOURCE*);
static BOOL ReservePortRegion(__DEVICE_MANAGER*,__RESOURCE*);
static VOID ReleasePortRegion(__DEVICE_MANAGER*,__RESOURCE*);
static VOID DeleteDevice(__DEVICE_MANAGER*,__PHYSICAL_DEVICE*);
static BOOL AppendDevice(__DEVICE_MANAGER*,__PHYSICAL_DEVICE*);
static __PHYSICAL_DEVICE* GetDevice(__DEVICE_MANAGER*,
	DWORD,
	__IDENTIFIER*,
	__PHYSICAL_DEVICE*);

/* Initializer of device manager. */
static BOOL DevMgrInitialize(__DEVICE_MANAGER* lpDevMgr)
{
	__RESOURCE* lpRes = NULL;
	int dwLoop = 0;

	BUG_ON(NULL == lpDevMgr);
	/* Only be called in process of system initialization. */
	BUG_ON(!IN_SYSINITIALIZATION());

#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(lpDevMgr->spin_lock,"devmgr");
#endif

	lpRes = (__RESOURCE*)KMemAlloc(sizeof(__RESOURCE),KMEM_SIZE_TYPE_ANY);
	if(NULL == lpRes)
	{
		return FALSE;
	}

	lpRes->dwResType = RESOURCE_TYPE_IO;
	lpRes->Dev_Res.IOPort.wStartPort = MIN_IO_PORT;
	lpRes->Dev_Res.IOPort.wEndPort   = MAX_IO_PORT;

	lpDevMgr->FreePortResource.dwResType = RESOURCE_TYPE_EMPTY;
	lpDevMgr->FreePortResource.lpNext    = lpRes;
	lpDevMgr->FreePortResource.lpPrev    = lpRes;
	lpRes->lpNext                        = &lpDevMgr->FreePortResource;
	lpRes->lpPrev                        = &lpDevMgr->FreePortResource;

	lpDevMgr->UsedPortResource.dwResType = RESOURCE_TYPE_EMPTY;
	lpDevMgr->UsedPortResource.lpNext    = &lpDevMgr->UsedPortResource;
	lpDevMgr->UsedPortResource.lpPrev    = &lpDevMgr->UsedPortResource;

	for(dwLoop = 0;dwLoop < MAX_BUS_NUM;dwLoop ++)
	{
		lpDevMgr->SystemBus[dwLoop].dwBusType    = BUS_TYPE_NULL;
		lpDevMgr->SystemBus[dwLoop].lpDevListHdr = NULL;
		lpDevMgr->SystemBus[dwLoop].lpHomeBridge = NULL;
		lpDevMgr->SystemBus[dwLoop].lpParentBus  = NULL;
	}

	//
	//Load system bus drivers here.
	//
	#ifdef __CFG_SYS_BM
		PciBusDriver(lpDevMgr);
	#endif
	return TRUE;
}

//
//A helper routine,used to insert a IO port region into list,and keeps all port region
//in order.
//
static VOID InsertIntoList(__RESOURCE* lpListHdr,__RESOURCE* lpRes)
{
	__RESOURCE*            lpBefore              = NULL;

	/* Validate parameters. */
	if ((NULL == lpListHdr) || (NULL == lpRes))
	{
		return;
	}

	/* Travel the whole list to find a statisfying position. */
	lpBefore = lpListHdr->lpNext;
	while(lpBefore != lpListHdr)
	{
		if(lpBefore->Dev_Res.IOPort.wStartPort >= lpRes->Dev_Res.IOPort.wEndPort)
		{
			break;
		}
		lpBefore = lpBefore->lpNext;
	}
	//
	//Insert lpRes into the list.
	//
	lpRes->lpNext            = lpBefore;
	lpRes->lpPrev            = lpBefore->lpPrev;
	lpBefore->lpPrev->lpNext = lpRes;
	lpBefore->lpPrev         = lpRes;

	return;
}

//
//Some macros used to operate bi-direction link list.
//
#define INSERT_INTO_LIST(listhdr,node)        \
	InsertIntoList(listhdr,node)

#define DELETE_FROM_LIST(node)                \
	(node)->lpNext->lpPrev = (node)->lpPrev;  \
	(node)->lpPrev->lpNext = (node)->lpNext;

/*
 * The following routine is used to merge continues IO port region into one region.
 * This is a helper routine,used by ReleasePortRegion to merge continues port region
 * into one region.
 * This routine searches the whole list,once find two continues region(wEndPort of the
 * first region equals to wStartPort - 1 of the second region),then modify the first
 * region's wEndPort to the second region's wEndPort,and delete the second region from
 * list.
 */
static VOID MergeRegion(__RESOURCE* lpListHdr)
{
	__RESOURCE*         lpFirst     = NULL;
	__RESOURCE*         lpSecond    = NULL;

	if(NULL == lpListHdr)
	{
		return;
	}
	lpFirst = lpListHdr->lpNext;
	while(lpFirst != lpListHdr)
	{
		lpSecond = lpFirst->lpNext;
		if(lpSecond == lpListHdr)    //Finished to search.
		{
			break;
		}
		if(lpFirst->Dev_Res.IOPort.wEndPort + 1 == lpSecond->Dev_Res.IOPort.wStartPort)  //Statisfy the continues
			                                                             //condition.
		{
			DELETE_FROM_LIST(lpSecond);    //Delete the second port region.
			lpFirst->Dev_Res.IOPort.wEndPort = lpSecond->Dev_Res.IOPort.wEndPort;  //Modify the first port region.
			KMemFree((LPVOID)lpSecond,KMEM_SIZE_TYPE_ANY,0);
			//free((LPVOID)lpSecond);
			continue;
		}
		lpFirst = lpFirst->lpNext;
	}
}

//
//ReservePortRegion routine.
//The second parameter,lpRes,indicates desired start address of port region,
//and port region's length can be calculated by minus start value from end value
//of the desired port region.
//
static BOOL ReservePortRegion(__DEVICE_MANAGER* lpDevMgr,__RESOURCE* lpRes)
{
	BOOL                      bResult                    = FALSE;
	WORD                      wSize                      = 0;
	__RESOURCE*               lpStatisfy                 = NULL;
	__RESOURCE*               lpPotential                = NULL;
	BOOL                      bFind                      = FALSE;
	DWORD                     dwFlags                    = 0;
	__RESOURCE*               lpFirstRegion              = NULL;
	__RESOURCE*               lpSecondRegion             = NULL;
	__RESOURCE*               lpRes1                     = NULL;

	/* Parameter validation and basic checking. */
	if ((NULL == lpDevMgr) || (NULL == lpRes))
	{
		return bResult;
	}
	if (lpRes->dwResType != RESOURCE_TYPE_IO)
	{
		return bResult;
	}
	if (lpRes->Dev_Res.IOPort.wEndPort - lpRes->Dev_Res.IOPort.wStartPort >= MAX_IO_PORT)
	{
		return bResult;
	}

	/* Create resource descriptor object. */
	lpRes1 = (__RESOURCE*)KMemAlloc(sizeof(__RESOURCE),KMEM_SIZE_TYPE_ANY);
	if (NULL == lpRes1)
	{
		return bResult;
	}

	//
	//First,we look for free port list of DeviceManager object,to find
	//a block of port region statisify the original desired(lpRes).
	//
	wSize = lpRes->Dev_Res.IOPort.wEndPort - lpRes->Dev_Res.IOPort.wStartPort + 1;
	__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
	lpStatisfy = lpDevMgr->FreePortResource.lpNext;
	/* Trave the whole free port list. */
	while(lpStatisfy != &lpDevMgr->FreePortResource)
	{
		if((lpStatisfy->Dev_Res.IOPort.wStartPort <= lpRes->Dev_Res.IOPort.wStartPort) && 
			(lpStatisfy->Dev_Res.IOPort.wEndPort   >= lpRes->Dev_Res.IOPort.wEndPort))
		{
			/* Find one. */
			bFind = TRUE;
			break;
		}
		if((lpStatisfy->Dev_Res.IOPort.wEndPort - lpStatisfy->Dev_Res.IOPort.wStartPort) >= wSize)
		{
			lpPotential = lpStatisfy;    //The current block can statisfy the size of original
			                             //request,so it is a potential statisfying region.
		}
		lpStatisfy = lpStatisfy->lpNext;
	}

	if(bFind)    //Found a IO port region statisfy the original request.
	{
		DELETE_FROM_LIST(lpStatisfy);    //Delete the region from free list.
		if(lpStatisfy->Dev_Res.IOPort.wStartPort < lpRes->Dev_Res.IOPort.wStartPort) //Exceed the request.
		{
			//lpFirstRegion = (__RESOURCE*)malloc(sizeof(__RESOURCE));
			lpFirstRegion = (__RESOURCE*)KMemAlloc(sizeof(__RESOURCE),KMEM_SIZE_TYPE_ANY);
			if(NULL != lpFirstRegion)  //Allocate successfully.
			{
				lpFirstRegion->dwResType = RESOURCE_TYPE_IO;
				lpFirstRegion->Dev_Res.IOPort.wStartPort = lpStatisfy->Dev_Res.IOPort.wStartPort;
				lpFirstRegion->Dev_Res.IOPort.wEndPort   = lpRes->Dev_Res.IOPort.wStartPort - 1;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpFirstRegion);  //Insert into
			                                                                  //free list.
			}
		}
		if(lpStatisfy->Dev_Res.IOPort.wEndPort > lpRes->Dev_Res.IOPort.wEndPort) //Exceed the request.
		{
			lpSecondRegion = (__RESOURCE*)KMemAlloc(sizeof(__RESOURCE),KMEM_SIZE_TYPE_ANY);
			if(NULL != lpSecondRegion)  //Allocate successfully.
			{
				lpSecondRegion->dwResType = RESOURCE_TYPE_IO;
				lpSecondRegion->Dev_Res.IOPort.wStartPort = lpRes->Dev_Res.IOPort.wEndPort + 1;
				lpSecondRegion->Dev_Res.IOPort.wEndPort   = lpStatisfy->Dev_Res.IOPort.wEndPort;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpSecondRegion);
			}
		}
		lpRes1->dwResType = RESOURCE_TYPE_IO;
		lpRes1->Dev_Res.IOPort.wStartPort = lpRes->Dev_Res.IOPort.wStartPort;
		lpRes1->Dev_Res.IOPort.wEndPort   = lpRes->Dev_Res.IOPort.wEndPort;
		/* Insert it into used port list. */
		INSERT_INTO_LIST(&lpDevMgr->UsedPortResource,lpRes1);
		__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		bResult = TRUE;
		KMemFree((LPVOID)lpStatisfy,KMEM_SIZE_TYPE_ANY,0);
		goto __TERMINAL;
	}

	/* 
	 * Though we can not find a IO port region statisfy the original request,
	 * but there is at least one region statisfy requesting size,so reserve
	 * this region.
	 */
	if(lpPotential)    
	{
		DELETE_FROM_LIST(lpPotential);    //Delete from free list.
		if((WORD)(lpPotential->Dev_Res.IOPort.wEndPort - lpPotential->Dev_Res.IOPort.wStartPort) > wSize)
		{
			lpFirstRegion = (__RESOURCE*)KMemAlloc(sizeof(__RESOURCE),KMEM_SIZE_TYPE_ANY);
			if(lpFirstRegion)  //Allocate memory successfully.
			{
				lpFirstRegion->dwResType = RESOURCE_TYPE_IO;
				lpFirstRegion->Dev_Res.IOPort.wStartPort = lpPotential->Dev_Res.IOPort.wStartPort + wSize;
				lpFirstRegion->Dev_Res.IOPort.wEndPort   = lpPotential->Dev_Res.IOPort.wEndPort;
				INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpFirstRegion);
			}
		}
		lpRes1->dwResType = RESOURCE_TYPE_IO;
		lpRes1->Dev_Res.IOPort.wStartPort = lpPotential->Dev_Res.IOPort.wStartPort;
		lpRes1->Dev_Res.IOPort.wEndPort   = lpPotential->Dev_Res.IOPort.wStartPort + wSize - 1;
		INSERT_INTO_LIST(&lpDevMgr->UsedPortResource,lpRes1);
		lpRes->Dev_Res.IOPort.wStartPort  = lpRes1->Dev_Res.IOPort.wStartPort;
		lpRes->Dev_Res.IOPort.wEndPort    = lpRes1->Dev_Res.IOPort.wEndPort;
		__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		bResult = TRUE;
		KMemFree((LPVOID)lpPotential,KMEM_SIZE_TYPE_ANY,0);
		goto __TERMINAL;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);

__TERMINAL:
	if(!bResult)
	{
		/* Failed to reserve port,release allocated resources. */
		KMemFree((LPVOID)lpRes1,KMEM_SIZE_TYPE_ANY,0);
		return bResult;
	}
	return bResult;
}

//
//The implementation of ReleasePortRegion routine.
//
static VOID ReleasePortRegion(__DEVICE_MANAGER* lpDevMgr,__RESOURCE* lpRes)
{
	__RESOURCE* lpUsed = NULL;
	unsigned long dwFlags;

	/* Basic checking. */
	if ((NULL == lpDevMgr) || (NULL == lpRes))
	{
		return;
	}
	if (RESOURCE_TYPE_IO != lpRes->dwResType)
	{
		return;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
	lpUsed = lpDevMgr->UsedPortResource.lpNext;
	/* Travel whole list. */
	while(lpUsed != &lpDevMgr->UsedPortResource)
	{
		if((lpUsed->Dev_Res.IOPort.wStartPort == lpRes->Dev_Res.IOPort.wStartPort) &&
		   (lpUsed->Dev_Res.IOPort.wEndPort   == lpRes->Dev_Res.IOPort.wEndPort))
		{
			/* Find the same region. */
			break;
		}
		lpUsed = lpUsed->lpNext;
	}
	if(lpUsed == &lpDevMgr->UsedPortResource)
	{
		/* Can not find the original request one. */
		__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		return;
	}
	/*
	 * Now,we have found the IO port region that equals to lpRes,so delete it from the
	 * used list,and insert it into free list.
	 */
	/* Delete from used list. */
	DELETE_FROM_LIST(lpUsed);
	/* Insert into free list. */
	INSERT_INTO_LIST(&lpDevMgr->FreePortResource,lpUsed);
	/* Do a merge operation */
	MergeRegion(&lpDevMgr->FreePortResource);
	__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
	return;
}

//
//The implement of CheckPortRegion.
//This routine is used to check if a block of port region is used,if used,then returns FALSE,
//else,returns TRUE.
//
static BOOL CheckPortRegion(__DEVICE_MANAGER* lpDevMgr,__RESOURCE* lpRes)
{
	__RESOURCE* lpTmp = NULL;
	unsigned long dwFlags;
	
	/* Basic validation. */
	if ((NULL == lpDevMgr) || (NULL == lpRes))
	{
		return FALSE;
	}
	if (RESOURCE_TYPE_IO != lpRes->dwResType)
	{
		return FALSE;
	}

	__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
	lpTmp = lpDevMgr->FreePortResource.lpNext;
	/* Travel the whole free list. */
	while(lpTmp != &lpDevMgr->FreePortResource)
	{
		if((lpTmp->Dev_Res.IOPort.wStartPort <= lpRes->Dev_Res.IOPort.wStartPort) && 
		   (lpTmp->Dev_Res.IOPort.wEndPort   >= lpRes->Dev_Res.IOPort.wEndPort))
		{
			__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
			return TRUE;
		}
		lpTmp = lpTmp->lpNext;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
	return FALSE;
}

//
//Check if 2 device IDs are match(lpSecond includes lpFirst).If the inter-set 
//of ID members of these 2 IDs equal to the first one,then return TRUE,otherwise
//return FALSE.
//
BOOL DeviceIdMatch(__IDENTIFIER* lpFirst,__IDENTIFIER* lpSecond)
{
	UCHAR ucMask = 0;

	/* Basic checking. */
	if ((NULL == lpFirst) || (NULL == lpSecond))
	{
		return FALSE;
	}
	if (lpFirst->dwBusType != lpSecond->dwBusType)
	{
		return FALSE;
	}
	if (lpFirst->dwBusType == BUS_TYPE_NULL)
	{
		return FALSE;
	}

	switch(lpFirst->dwBusType)
	{
	case BUS_TYPE_PCI:    //PCI Identifier match.
#define PCI_ID_MEMBER(id,mem) ((id)->Bus_ID.PCI_Identifier.mem)
		if(PCI_ID_MEMBER(lpFirst,wVendor) == PCI_ID_MEMBER(lpSecond,wVendor))
			ucMask |= PCI_IDENTIFIER_MASK_VENDOR;
		if(PCI_ID_MEMBER(lpFirst,wDevice) == PCI_ID_MEMBER(lpSecond,wDevice))
			ucMask |= PCI_IDENTIFIER_MASK_DEVICE;
		if((lpFirst->Bus_ID.PCI_Identifier.dwClass >> 8) == 
		   (lpSecond->Bus_ID.PCI_Identifier.dwClass >> 8))
		    ucMask |= PCI_IDENTIFIER_MASK_CLASS;
		if(PCI_ID_MEMBER(lpFirst,ucHdrType) == PCI_ID_MEMBER(lpSecond,ucHdrType))
			ucMask |= PCI_IDENTIFIER_MASK_HDRTYPE;
		return ((lpFirst->Bus_ID.PCI_Identifier.ucMask & ucMask) == lpFirst->Bus_ID.PCI_Identifier.ucMask);
#undef PCI_ID_MEMBER

	case BUS_TYPE_ISA:    //ISA Identifier match.
		return (lpFirst->Bus_ID.ISA_Identifier.dwDevice == lpSecond->Bus_ID.ISA_Identifier.dwDevice);

	case BUS_TYPE_USB:
#define USB_ID_MEMBER(id,mem) (id->Bus_ID.USB_Identifier.mem)
		if (USB_ID_MEMBER(lpFirst, bDeviceClass) == USB_ID_MEMBER(lpSecond, bDeviceClass))
			ucMask |= USB_IDENTIFIER_MASK_DEVICECLASS;
		if (USB_ID_MEMBER(lpFirst, bDeviceSubClass) == USB_ID_MEMBER(lpSecond, bDeviceSubClass))
			ucMask |= USB_IDENTIFIER_MASK_DEVICESUBCLASS;
		if (USB_ID_MEMBER(lpFirst, bDeviceProtocol) == USB_ID_MEMBER(lpSecond, bDeviceProtocol))
			ucMask |= USB_IDENTIFIER_MASK_DEVICEPROTOCOL;
		if (USB_ID_MEMBER(lpFirst, bInterfaceClass) == USB_ID_MEMBER(lpSecond, bInterfaceClass))
			ucMask |= USB_IDENTIFIER_MASK_INTERFACECLASS;
		if (USB_ID_MEMBER(lpFirst, bInterfaceSubClass) == USB_ID_MEMBER(lpSecond, bInterfaceSubClass))
			ucMask |= USB_IDENTIFIER_MASK_INTERFACESUBCLASS;
		if (USB_ID_MEMBER(lpFirst, bInterfaceProtocol) == USB_ID_MEMBER(lpSecond, bInterfaceProtocol))
			ucMask |= USB_IDENTIFIER_MASK_INTERFACEPROTOCOL;
		if (USB_ID_MEMBER(lpFirst, wVendorID) == USB_ID_MEMBER(lpSecond, wVendorID))
			ucMask |= USB_IDENTIFIER_MASK_VENDORID;
		if (USB_ID_MEMBER(lpFirst, wProductID) == USB_ID_MEMBER(lpSecond, wProductID))
			ucMask |= USB_IDENTIFIER_MASK_PRODUCTID;
		return ((USB_ID_MEMBER(lpFirst, ucMask) & ucMask) == USB_ID_MEMBER(lpFirst,ucMask));
#undef USB_ID_MEMBER

	default:
		break;
	}
	return FALSE;
}

/*
 * GetDevice routine.
 * This routine returns the appropriate physical device object according to identifier.
 */
static __PHYSICAL_DEVICE* GetDevice(__DEVICE_MANAGER* lpDevMgr,
	DWORD dwBusType,
	__IDENTIFIER* lpId,
	__PHYSICAL_DEVICE* lpStart)
{
	DWORD dwIndex = 0;
	unsigned long dwFlags = 0;
	__PHYSICAL_DEVICE* lpPhyDev = NULL;

	if((NULL == lpDevMgr) || (NULL == lpId))
	{
		return NULL;
	}

	/* Call this routine for the first time. */
	if(NULL == lpStart)
	{
		__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		for(dwIndex = 0;dwIndex < MAX_BUS_NUM;dwIndex ++)
		{
			if (lpDevMgr->SystemBus[dwIndex].dwBusType != dwBusType)
			{
				continue;
			}
			lpPhyDev = lpDevMgr->SystemBus[dwIndex].lpDevListHdr;
			while(lpPhyDev)
			{
#define DEVICE_ID_MATCH(id1,id2) (DeviceIdMatch((id1),(id2)))
				if(DEVICE_ID_MATCH(lpId,&lpPhyDev->DevId))  //ID match.
				{
					__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
					return lpPhyDev;    //Find a device object statisfying the request.
				}
				lpPhyDev = lpPhyDev->lpNext;
			}
		}
		__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		return lpPhyDev;    //If reach here,the requested device must have not been found.
	}
	else
	{
		/* This routine isn't called for the first time. */
		BUG_ON(NULL == lpStart);
		dwIndex = 
			(DWORD)((DWORD)(lpStart->lpHomeBus) - 
			(DWORD)(&lpDevMgr->SystemBus[0])) / sizeof(__SYSTEM_BUS);
		lpPhyDev = lpStart->lpNext;

		__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		while((dwIndex < MAX_BUS_NUM) || lpPhyDev)
		{
			while(lpPhyDev)
			{
				if(DEVICE_ID_MATCH(lpId,&lpPhyDev->DevId))
				{
					__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
					return lpPhyDev;
				}
				lpPhyDev = lpPhyDev->lpNext;
			}
			dwIndex += 1;
			while(dwIndex < MAX_BUS_NUM)
			{
				/* Try to search next BUS. */
				if(lpDevMgr->SystemBus[dwIndex].dwBusType != dwBusType)
				{
					dwIndex += 1;
					continue;
				}
				lpPhyDev = lpDevMgr->SystemBus[dwIndex].lpDevListHdr;
				break;
			}
		}
		__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, dwFlags);
		/* No device found if reach here. */
		return lpPhyDev;
	}
}

/* Append a physical device object into a BUS. */
static BOOL AppendDevice(__DEVICE_MANAGER* lpDevMgr,__PHYSICAL_DEVICE* lpDev)
{
	__SYSTEM_BUS* pBus = NULL;
	unsigned long ulFlags = 0;
	BOOL bResult = FALSE;

	/* Basic checking of parameters. */
	if ((NULL == lpDevMgr) || (NULL == lpDev))
	{
		goto __TERMINAL;
	}
	if (NULL == lpDev->lpHomeBus)
	{
		goto __TERMINAL;
	}
	pBus = lpDev->lpHomeBus;
	/* Just add the physical device into device list. */
	__ENTER_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, ulFlags);
	lpDev->lpNext = pBus->lpDevListHdr;
	pBus->lpDevListHdr = lpDev;
	__LEAVE_CRITICAL_SECTION_SMP(lpDevMgr->spin_lock, ulFlags);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* This routine deletes one physical device from system bus. */
static VOID DeleteDevice(__DEVICE_MANAGER* lpDevMgr,__PHYSICAL_DEVICE* lpDev)
{
	return;
}

/****************************************************************************************
*****************************************************************************************
*****************************************************************************************
*****************************************************************************************
****************************************************************************************/
/*
 * DeviceManager object,all physical devices in system are
 * managed by this object.
 */
__DEVICE_MANAGER DeviceManager = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,              //spin_lock.
#endif
	{0},                               //SystemBus array.
	{0},                               //FreePortResource.
	{0},                               //UsedPortResource.
	DevMgrInitialize,                  //Initialize.
	GetDevice,                         //GetDevice.
	AppendDevice,                      //AppendDevice.
	DeleteDevice,                      //DeleteDevice.
	CheckPortRegion,                   //CheckPortRegion.
	ReservePortRegion,                 //ReservePortRegion.
	ReleasePortRegion                  //ReleasePortRegion.
};

#endif
