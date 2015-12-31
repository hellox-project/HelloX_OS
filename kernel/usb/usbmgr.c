//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 16, 2015
//    Module Name               : usbmgr.c
//    Module Funciton           : 
//                                USB Manager object's implementation.
//                                All global level USB operations,such as
//                                USB device's creation and destroy,are
//                                encapsulated in this object.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <align.h>

#include "errno.h"
#include "usb_defs.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"

//Global array of all devices and it's number.
extern struct usb_device usb_dev[USB_MAX_DEVICE];

//USB driver entry point array.
extern __DRIVER_ENTRY_ARRAY UsbDriverEntryArray[];

//Common interrupt handler of a USB controller,it will call controller
//specific routine accordingly.
static BOOL CommonInterruptHandler(LPVOID lpESP, LPVOID lpParam)
{
	__COMMON_USB_CONTROLLER* pUsbCtrl = (__COMMON_USB_CONTROLLER*)lpParam;
	if (NULL == pUsbCtrl)
	{
		BUG();
	}
	if (NULL == pUsbCtrl->ctrlOps.InterruptHandler)
	{
		_hx_printf("Warning: Interrupt is enabled for USB Controller 0x%X but without HANDLER set.\r\n", 
			pUsbCtrl->pPhysicalDev->dwNumber);
		return FALSE;
	}
	//Process the interrupt by calling controller specific handler.
	if (pUsbCtrl->ctrlOps.InterruptHandler((LPVOID)pUsbCtrl))
	{
		return TRUE;
	}
	return FALSE;
}

//A dedicated kernel thread is used to service all USB controllers
//or devices in system.It's the core of USB sub-system.
static DWORD USBCoreThread(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	DWORD dwIndex = 0;

	//Initialize the USB controllers and load device drivers before enter
	//message loop.
	usb_init();

	//Load all embedded USB drivers.
	while (UsbDriverEntryArray[dwIndex].Entry)
	{
		if (!IOManager.LoadDriver(UsbDriverEntryArray[dwIndex].Entry)) //Failed to load.
		{
			//Show an error.
			_hx_printf("Warning: Failed to load USB driver [%s].\r\n",
				UsbDriverEntryArray[dwIndex].pszDriverName);
		}
		else
		{
			//Show the correct loaded driver.
			_hx_printf("Load USB driver [%s] OK.\r\n", UsbDriverEntryArray[dwIndex].pszDriverName);
		}
		dwIndex++;  //Continue to load.
	}

	//Main message loop.
	while (TRUE)
	{
		if (KernelThreadManager.GetMessage(NULL, &msg))
		{
			//Process the message.
		}
	}
	return 1;
}

//Initialization routine of USBManager.
static BOOL UsbMgrInit(__USB_MANAGER* pUsbMgr)
{
	int i = 0;

	if (NULL == pUsbMgr)
	{
		BUG();
	}

	//Assign global arrays to USB Manager.
	for (i = 0; i < USB_MAX_DEVICE; i++)
	{
		pUsbMgr->UsbDevArray[i] = &usb_dev[i];
	}
	pUsbMgr->dev_index = 0;

	//Create the background service kernel thread.
	pUsbMgr->UsbCoreThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		USBCoreThread,
		NULL,
		NULL,
		"USB_Core");
	if (NULL == pUsbMgr->UsbCoreThread)
	{
		_hx_printf("USB: Can not create service thread.\r\n");
		return FALSE;
	}

	//usb_init();
	return TRUE;
}

//Create and return a common USB controller object.
static __COMMON_USB_CONTROLLER* CreateUsbCtrl(__USB_CONTROLLER_OPERATIONS* ops,DWORD dwCtrlType,
	__PHYSICAL_DEVICE* pPhyDev,void* priv)
{
	__COMMON_USB_CONTROLLER* pUsbCtrl = NULL;
	int i = 0;
	unsigned char ucInt = 0;
	
	//Try to find a free USB controller slot in array.
	for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_NUM; i++)
	{
		if (NULL == USBManager.CtrlArray[i])
		{
			break;
		}
	}
	if (CONFIG_USB_MAX_CONTROLLER_NUM == i)
	{
		goto __TERMINAL;
	}

	//Create a common USB controller object,and save it to global array.
	pUsbCtrl = (__COMMON_USB_CONTROLLER*)_hx_malloc(sizeof(__COMMON_USB_CONTROLLER));
	if (NULL == pUsbCtrl)
	{
		goto __TERMINAL;
	}
	USBManager.CtrlArray[i] = pUsbCtrl;

	//Initialize it.
	pUsbCtrl->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;
	pUsbCtrl->dwCtrlType = dwCtrlType;
	pUsbCtrl->pPhysicalDev = pPhyDev;
	pUsbCtrl->IntObject = NULL;
	pUsbCtrl->ctrlOps.submit_bulk_msg = ops->submit_bulk_msg;
	pUsbCtrl->ctrlOps.submit_control_msg = ops->submit_control_msg;
	pUsbCtrl->ctrlOps.submit_int_msg = ops->submit_int_msg;
	pUsbCtrl->ctrlOps.create_int_queue = ops->create_int_queue;
	pUsbCtrl->ctrlOps.destroy_int_queue = ops->destroy_int_queue;
	pUsbCtrl->ctrlOps.poll_int_queue = ops->poll_int_queue;
	pUsbCtrl->ctrlOps.usb_reset_root_port = ops->usb_reset_root_port;
	pUsbCtrl->ctrlOps.get_ctrl_status = ops->get_ctrl_status;
	pUsbCtrl->ctrlOps.InterruptHandler = ops->InterruptHandler;

	//Save private data of the user specified.
	pUsbCtrl->pUsbCtrl = priv;

	//If physical device is specified,then should establish interrupt mechanism.
	if (pPhyDev)
	{
		for (i = 0; i < MAX_RESOURCE_NUM; i++)
		{
			if (RESOURCE_TYPE_INTERRUPT == pPhyDev->Resource[i].dwResType)
			{
				ucInt = pPhyDev->Resource[i].Dev_Res.ucVector;
				break;
			}
		}
		if (0 == ucInt)  //No interrupt resource is found.
		{
			debug("%s: Can not find interrupt vector for the USB controller.\r\n", __func__);
		}
	}
	if (ucInt)
	{
#ifdef __I386__
		ucInt += 0x20;  //Offset to CPU's device interrupt vector region.
#endif
		//Create interrupt object now.
		pUsbCtrl->IntObject = ConnectInterrupt(CommonInterruptHandler,
			(LPVOID)pUsbCtrl,
			ucInt);
		if (NULL == pUsbCtrl->IntObject)  //Failed to connect interrupt.
		{
			_hx_printf("%s: Failed to connect interrupt object,vector = %d.\r\n",__func__, ucInt);
			//Destroy the common USB controller object.
			_hx_free(pUsbCtrl);
			pUsbCtrl = NULL;
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return pUsbCtrl;
}

//A local helper routine to retrieve usb descriptors from device.
static int usb_get_descriptor(struct usb_device *dev, unsigned char type,unsigned char index, void *buf, int size)
{
	int res;
	res = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
		(type << 8) + index, 0,
		buf, size, USB_CNTL_TIMEOUT);
	return res;
}

//Retrieve a specified USB device's configuration descriptor,NULL will be returned if fail.
//The caller should response to destroy the returned descriptor object.
static struct usb_configuration_descriptor* _GetConfigDescriptor(struct usb_device* dev, int cfgno)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buff, 16);
	int result = 0, length = 0;
	struct usb_configuration_descriptor* pCfgTmp = NULL;
	struct usb_configuration_descriptor* pCfgReturn = NULL;

	if (NULL == dev)
	{
		goto __TERMINAL;
	}

	//Retrieve configuration descriptor's total length.
	pCfgTmp = (struct usb_configuration_descriptor*)&buff[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, pCfgTmp, 9);
	if (result < 9)
	{
		if (result < 0)
		{
			_hx_printf("USB: %s failed with err = [%d].\r\n", __func__, result);
			goto __TERMINAL;
		}
		else
		{
			_hx_printf("USB: %s got too short config,expected = %d,got = %d.\r\n",
				__func__,
				9,
				result);
			goto __TERMINAL;
		}
	}
	length = le16_to_cpu(pCfgTmp->wTotalLength);

	//Allocate a memory block to hold the descriptor,should be released by the caller.
	pCfgReturn = aligned_malloc(length,ARCH_DMA_MINALIGN);
	if (NULL == pCfgReturn)
	{
		_hx_printf("USB: %s alloc memory failed.\r\n",__func__);
		goto __TERMINAL;
	}
	//Get the total configuration descriptor of the device.
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, pCfgReturn, length);
	if (result < 0)
	{
		_hx_printf("USB: %s get config descriptor failed,err = %d.\r\n",
			__func__,
			result);
		aligned_free(pCfgReturn);
		pCfgReturn = NULL;
		goto __TERMINAL;
	}

__TERMINAL:
	return pCfgReturn;
}

//BulkMessage.
static int _BulkMessage(__PHYSICAL_DEVICE* pUsbDev, unsigned long pipe,
	void *buffer, int transfer_len)
{
	__COMMON_USB_CONTROLLER* pUsbCtrl = NULL;
	struct usb_device* dev = NULL;

	if (NULL == pUsbDev)
	{
		return -1;
	}
	dev = (struct usb_device*)pUsbDev->lpPrivateInfo;
	if (NULL == dev)
	{
		BUG();
	}
	pUsbCtrl = (__COMMON_USB_CONTROLLER*)dev->controller;
	return pUsbCtrl->ctrlOps.submit_bulk_msg(dev, pipe, buffer, transfer_len);
}

//ControlMessage.
static int _ControlMessage(__PHYSICAL_DEVICE* pUsbDev, unsigned long pipe, void *buffer,
	int transfer_len, struct devrequest *setup)
{
	__COMMON_USB_CONTROLLER* pUsbCtrl = NULL;
	struct usb_device* dev = NULL;

	if (NULL == pUsbDev)
	{
		return -1;
	}
	dev = (struct usb_device*)pUsbDev->lpPrivateInfo;
	if (NULL == dev)
	{
		BUG();
	}
	pUsbCtrl = (__COMMON_USB_CONTROLLER*)dev->controller;
	return pUsbCtrl->ctrlOps.submit_control_msg(dev, pipe, buffer, transfer_len, setup);
}

//InterruptMessage.
static int _InterruptMessage(__PHYSICAL_DEVICE* pUsbDev, unsigned long pipe, void *buffer,
	int transfer_len, int interval)
{
	__COMMON_USB_CONTROLLER* pUsbCtrl = NULL;
	struct usb_device* dev = NULL;

	if (NULL == pUsbDev)
	{
		return -1;
	}
	dev = (struct usb_device*)pUsbDev->lpPrivateInfo;
	if (NULL == dev)
	{
		BUG();
	}
	pUsbCtrl = (__COMMON_USB_CONTROLLER*)dev->controller;
	return pUsbCtrl->ctrlOps.submit_int_msg(dev, pipe, buffer, transfer_len, interval);
}

//Add a physical USB device into system,for each scaned USB device.
static BOOL _AddUsbDevice(struct usb_device* pDevice)
{
	__PHYSICAL_DEVICE* pPhysicalDevice = NULL;
	struct usb_device_descriptor* pDevDesc = NULL;
	struct usb_config* pConfDesc = NULL;
	struct usb_interface_descriptor* pIntDesc = NULL;
	BOOL bResult = FALSE;
	DWORD dwFlags;
	int usb_int_num = 0, i = 0, j = 0;

	if (NULL == pDevice)
	{
		goto __TERMINAL;
	}
	pDevDesc = &pDevice->descriptor;
	pConfDesc = &pDevice->config;
	usb_int_num = pConfDesc->desc.bNumInterfaces;
	if (0 == usb_int_num)
	{
		goto __TERMINAL;
	}

	//Add one physical device for each USB interface.
	for (i = 0; i < usb_int_num; i++)
	{
		pIntDesc = &pConfDesc->if_desc[i].desc;
		//Alocate physical device.
		pPhysicalDevice = (__PHYSICAL_DEVICE*)_hx_malloc(sizeof(__PHYSICAL_DEVICE));
		if (NULL == pPhysicalDevice)
		{
			break;
		}
		memzero(pPhysicalDevice, sizeof(__PHYSICAL_DEVICE));

		//The high part(16 bits) of dwNumber contains the USB device address,and
		//the low part contains the USB interface ID.
		pPhysicalDevice->dwNumber = pDevice->devnum;
		pPhysicalDevice->dwNumber <<= 16;
		pPhysicalDevice->dwNumber += i;

		pPhysicalDevice->DevId.dwBusType = BUS_TYPE_USB;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_ALL;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bDeviceClass = pDevDesc->bDeviceClass;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bDeviceSubClass = pDevDesc->bDeviceSubClass;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bDeviceProtocol = pDevDesc->bDeviceProtocol;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bInterfaceClass = pIntDesc->bInterfaceClass;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bInterfaceSubClass = pIntDesc->bInterfaceSubClass;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.bInterfaceProtocol = pIntDesc->bInterfaceProtocol;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.wVendorID = pDevDesc->idVendor;
		pPhysicalDevice->DevId.Bus_ID.USB_Identifier.wProductID = pDevDesc->idProduct;

		pPhysicalDevice->lpPrivateInfo = (LPVOID)pDevice;
		//Copy device name.
		for (j = 0; j < MAX_DEV_NAME - 1; j++)
		{
			if (pDevice->mf[j])
			{
				pPhysicalDevice->strName[j] = pDevice->mf[j];
			}
			else
			{
				break;
			}
		}
		pPhysicalDevice->strName[j] = 0;

		//Now add the physical device into USB Manager's global list.
		__ENTER_CRITICAL_SECTION(NULL, dwFlags);
		pPhysicalDevice->lpNext = USBManager.pUsbDeviceRoot;
		USBManager.pUsbDeviceRoot = pPhysicalDevice;
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
		debug("%s: Add one USB physical device [name = %s] into system.\r\n", __func__,
			pPhysicalDevice->strName);
	}

	if (i > 0)  //At lease one physical device are added successfully.
	{
		bResult = TRUE;
	}
	
__TERMINAL:
	if (!bResult)
	{
		debug("%s failed.\r\n", __func__);
		if (pPhysicalDevice)
		{
			_hx_free(pPhysicalDevice);
		}
	}
	return bResult;
}

//Get a physical device corresponding a specified USB device identified by id.
static __PHYSICAL_DEVICE* _GetUsbDevice(__IDENTIFIER* id, __PHYSICAL_DEVICE* pStart)
{
	__PHYSICAL_DEVICE* pBegin = NULL;
	__PHYSICAL_DEVICE* pReturn = NULL;

	if (NULL == id)
	{
		goto __TERMINAL;
	}
	if (id->dwBusType != BUS_TYPE_USB)
	{
		goto __TERMINAL;
	}

	if (pStart)
	{
		pBegin = pStart;
	}
	else
	{
		pBegin = USBManager.pUsbDeviceRoot;
	}

	//Try to find the desired device.
	while (pBegin)
	{
		if (DeviceIdMatch(id, &pBegin->DevId))
		{
			pReturn = pBegin;
			break;
		}
		pBegin = pBegin->lpNext;
	}

__TERMINAL:
	return pReturn;
}

//Defination of USB Manager object.
__USB_MANAGER USBManager = {
	{ 0 },    //CtrlArray.
	{ 0 },    //UsbDevArray.
	0,        //dev_index.
	NULL,     //pUsbDeviceRoot.
	NULL,     //UsbCoreThread.

	//Operations.
	CreateUsbCtrl,              //CreateUsbCtrl.
	NULL,                       //CreateUsbDevice.
	_AddUsbDevice,              //AddUsbDevice.
	_GetUsbDevice,              //GetUsbDevice.
	_GetConfigDescriptor,       //GetConfigDescriptor.
	_BulkMessage,               //BulkMessage.
	_ControlMessage,            //ControlMessage.
	_InterruptMessage,          //InterruptMessage.
	UsbMgrInit                  //Initialize.
};
