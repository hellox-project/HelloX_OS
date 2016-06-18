//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 12, 2016
//    Module Name               : uvc_drv.c
//    Module Funciton           : 
//                                Implementation of UVC driver.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <pci_drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <align.h>

#include "errno.h"
#include "usb_defs.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "ehci.h"
#include "usbiso.h"
#include "uvc.h"

//List header of all USB video devices in system.One USB video device will be
//allocated by UVC_Probe and append to the list.
static __USB_VIDEO_DEVICE* pVideoDeviceList = NULL;

//USB Video Class function probing routine.
static BOOL UVC_Probe()
{
	__IDENTIFIER id;
	__PHYSICAL_DEVICE* pPhyDev = NULL;
	__USB_VIDEO_DEVICE* pVideoDevice = NULL;
	struct usb_device* pUsbDev = NULL;
	struct usb_interface* pUsbInt = NULL;
	struct usb_endpoint_descriptor* pED = NULL;
	__USB_INTERFACE_ASSOCIATION* pIntAssoc = NULL;
	unsigned long pipe = 0;
	__u8 dpm = 0;  //Device Power Mode.
	__u8 streamEP = 0;  //Endpoint number of streaming interface.
	int i = 0;
	BOOL ret = FALSE;
	BOOL bResult = FALSE;

	//Initialize ID accordingly.
	id.dwBusType = BUS_TYPE_USB;
	id.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_INTERFACECLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACESUBCLASS;
	id.Bus_ID.USB_Identifier.bInterfaceClass = CC_VIDEO;
	id.Bus_ID.USB_Identifier.bInterfaceSubClass = SC_VIDEO_INTERFACE_COLLECTION;

	//Try to find the UVC device from system.
	pPhyDev = USBManager.GetUsbDevice(&id, NULL);
	if (NULL == pPhyDev)
	{
		_hx_printf("%s:can not find UVC device.\r\n", __func__);
		goto __TERMINAL;
	}

	//Make farther check according HID spec.
	pUsbDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	if (NULL == pUsbDev)
	{
		BUG();
		goto __TERMINAL;
	}

	//Get interface association control block.
	pIntAssoc = pPhyDev->Resource[0].Dev_Res.usbIntAssocBase;
	
	//Found the USB video device.
	pVideoDevice = _hx_malloc(sizeof(__USB_VIDEO_DEVICE));
	if (NULL == pVideoDevice)
	{
		_hx_printf("%s:failed to allocate USB video device.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(pVideoDevice, 0, sizeof(__USB_VIDEO_DEVICE));
	//Initialize the USB video device object.
	pVideoDevice->pPhyDev = pPhyDev;
	pVideoDevice->pUsbDev = pUsbDev;
	pVideoDevice->pIntAssoc = pIntAssoc;
	pVideoDevice->pFormatList = NULL;
	pVideoDevice->recvCtrlPipe = usb_rcvctrlpipe(pUsbDev, 0);
	pVideoDevice->sendCtrlPipe = usb_sndctrlpipe(pUsbDev, 0);  //Shoud be revised later.
	pVideoDevice->ctrlIntNum = 
		get_unaligned(&pIntAssoc->Interfaces[UVC_DEFAULT_CONTROL_INT].pPrimaryInterface->desc.bInterfaceNumber);
	pVideoDevice->streamIntNum = 
		get_unaligned(&pIntAssoc->Interfaces[UVC_DEFAULT_STREAM_INT].pPrimaryInterface->desc.bInterfaceNumber);
	pVideoDevice->alternates = 0;
	pVideoDevice->streamBuff = NULL;
	pVideoDevice->isoDesc = NULL;
	pVideoDevice->frameSize = 0;
	pVideoDevice->sampleSize = 0;
	pVideoDevice->currFormat = 1;
	pVideoDevice->currFrame = 1;
	pVideoDevice->pCurrentFormat = NULL;

	//Initialize probe and commit structure.
	put_unaligned(1,&pVideoDevice->uvpc.bFormatIndex);
	put_unaligned(1,&pVideoDevice->uvpc.bFrameIndex);

#if 0
	//Just assume the second interface as streaming interface for simplicity,and
	//assume the first isochronous end point as streaming endpoint.
	for (i = 0; i < pIntAssoc->Interfaces[UVC_DEFAULT_STREAM_INT].pPrimaryInterface->no_of_ep; i++)
	{
		pED = &pIntAssoc->Interfaces[UVC_DEFAULT_STREAM_INT].pPrimaryInterface->ep_desc[i];
		if ((get_unaligned(&pED->bmAttributes) & 3) == 1)  //Isochronous end point.
		{
			streamEP = get_unaligned(&pED->bEndpointAddress) & 0xF;
			break;
		}
	}
	if (0 == streamEP)  //Can not find proper endpoint.
	{
		_hx_printf("%s:can not find valid endpoint for streaming.\r\n", __func__);
		goto __TERMINAL;
	}

	//Construct streaming pipe and save it.
	pVideoDevice->recvStreamPipe = usb_rcvisocpipe(pUsbDev, streamEP);
#endif

	//Decode all supported video formats.
	if (!uvcDecodeFormat(pVideoDevice))
	{
		_hx_printf("%s:failed to decode video format.\r\n", __func__);
		goto __TERMINAL;
	}
	
	//Insert it into global USB video device list.
	pVideoDevice->pNext = pVideoDeviceList;
	pVideoDeviceList = pVideoDevice;
	//Also register it to USB video manager object.
	if (!UsbVideoManager.RegisterVideoDevice(pVideoDevice))
	{
		_hx_printf("%s:failed to register video device into system.\r\n", __func__);
		goto __TERMINAL;
	}

	_hx_printf("Found UVC device:mf = [%s],ctrl_int = [%d],stream_int = [%d].\r\n", 
		pUsbDev->mf,pVideoDevice->ctrlIntNum,
		pVideoDevice->streamIntNum);

	/*
	if (!uvcStreamNegotiation(pVideoDevice, 1, 2, 1, 100, &uvpc))
	{
		_hx_printf("%s:failed to negotiate streaming configurations.\r\n", __func__);
		goto __TERMINAL;
	}
	if (!uvcPrepareStreaming(pVideoDevice, 1024 * 256 * 8, &uvpc))
	{
		_hx_printf("%s:failed to prepare streaming.\r\n", __func__);
		goto __TERMINAL;
	}
	if (!uvcStartStreaming(pVideoDevice, NULL, 0, NULL))
	{
		_hx_printf("%s:failed to start streaming.\r\n", __func__);
		goto __TERMINAL;
	}
	//Stop streaming.
	uvcStopStreaming(pVideoDevice);
	*/

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Driver entry of UVC.
BOOL UVC_DriverEntry(__DRIVER_OBJECT* pDriverObject)
{
	CHAR uvcDeviceName[MAX_DEV_NAME];
	BOOL bResult = FALSE;
	__DEVICE_OBJECT* pVideoDeviceObject = NULL;
	__USB_VIDEO_DEVICE* pVideoDevice = NULL;
	int i = 0;

	bResult = UVC_Probe();
	if (!bResult)
	{
		_hx_printf("%s:can not probe valid UVC device.\r\n", __func__);
		goto __TERMINAL;
	}
	
	//Register all USB video device(s) into system.
	pVideoDevice = pVideoDeviceList;
	while (pVideoDevice)
	{
		//Construct USB video device name.
		strcpy(&uvcDeviceName[0], "\\\\.\\");
		for (i = 4; i < 26; i++)  //skip the "\\.\" space.
		{
			if (0 == pVideoDevice->pPhyDev->strName[i - 4])
			{
				break;
			}
			uvcDeviceName[i] = pVideoDevice->pPhyDev->strName[i - 4];
		}
		uvcDeviceName[i] = 0;
		pVideoDeviceObject = IOManager.CreateDevice(
			(__COMMON_OBJECT*)&IOManager,
			&uvcDeviceName[0],
			0,
			0,
			0,
			0,
			pVideoDevice,
			pDriverObject);
		if (NULL == pVideoDeviceObject)
		{
			_hx_printf("%s:failed to create device object.\r\n", __func__);
			goto __TERMINAL;
		}
		pVideoDevice = pVideoDevice->pNext;
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

