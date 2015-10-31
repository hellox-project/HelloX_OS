//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2015
//    Module Name               : usbmouse.c
//    Module Funciton           : 
//    Description               : USB mouse driver source code are put into this file.
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
#include "usbmouse.h"

//Only available when the USB Storage function is enabled.
#if defined(__CFG_DRV_USBMOUSE) && defined(__CFG_SYS_USB)

//Interrupt handler for USBHD.
static BOOL _Mouse_IntHandler(LPVOID pParam, LPVOID pEsp)
{
	PrintLine("  USB Mouse interrupt occurs.");
	return TRUE;
}

//The USB mouse device found in system,it will be set
//by ScanUsbMouse routine.
static __PHYSICAL_DEVICE* pUsbMouseDev = NULL;

//Scan USB devices in system to find USB mouse device.
static BOOL ScanUsbMouse()
{
	__IDENTIFIER id;
	__PHYSICAL_DEVICE* pPhyDev = NULL;
	struct usb_device* pUsbDev = NULL;
	struct usb_interface* pUsbInt = NULL;
	struct usb_endpoint_descriptor* pED = NULL;
	BOOL bResult = FALSE;

	//Initialize id accordingly.
	id.dwBusType = BUS_TYPE_USB;
	id.Bus_ID.USB_Identifier.ucMask  = USB_IDENTIFIER_MASK_INTERFACECLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACESUBCLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACEPROTOCOL;
	id.Bus_ID.USB_Identifier.bInterfaceClass = UM_INTERFACE_CLASS_ID;
	id.Bus_ID.USB_Identifier.bInterfaceSubClass = UM_INTERFACE_SUBCLASS_ID;
	id.Bus_ID.USB_Identifier.bInterfaceProtocol = UM_INTERFACE_PROTOCOL_ID;

	//Try to find the USB mouse device from system.
	pPhyDev = USBManager.GetUsbDevice(&id, NULL);
	if (NULL == pPhyDev)
	{
		goto __TERMINAL;
	}

	//Make farther check according HID spec.
	pUsbDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	if (NULL == pUsbDev)
	{
		BUG();
		goto __TERMINAL;
	}
	pUsbInt = &pUsbDev->config.if_desc[pPhyDev->dwNumber & 0xFFFF];
	if (pUsbInt->desc.bNumEndpoints != 1)
	{
		goto __TERMINAL;
	}
	pED = &pUsbInt->ep_desc[0];
	if (!(pED->bEndpointAddress & 0x80))
	{
		goto __TERMINAL;
	}

	//Found the USB mouse device.
	_hx_printf("Found USB mouse device:mf = [%s].\r\n",
		pUsbDev->mf);
	pUsbMouseDev = pPhyDev;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Temporary structure used to contain USB mouse related data.
typedef struct tag__USB_MOUSE_DATA{
	unsigned long ulIntPipe;  //Pipe number of USB mouse device.
	int inputPktSize;
	int intInterval;
	unsigned char MouseBuffer[MAX_USBMOUSE_BUFF_LEN];
}__USB_MOUSE_DATA;

//Parse USB input data,translate it into kernel message,and delivery it to HelloX kernel.
static BOOL DoMouseMessage(__USB_MOUSE_DATA* pMouseData)
{
	//Just dumpout the message for debugging.
	if (pMouseData->MouseBuffer[0] & 0x01)  //Left button is pressed.
	{
		_hx_printf("USB input data: %X %X %X %X.\r\n",
			pMouseData->MouseBuffer[0],
			pMouseData->MouseBuffer[1],
			pMouseData->MouseBuffer[2],
			pMouseData->MouseBuffer[3]);
	}
	return TRUE;
}

//A dedicated kernel thread is running to poll USB mouse input looply.
static DWORD USB_Poll_Thread(LPVOID pData)
{
	__USB_MOUSE_DATA UsbMouseData;
	struct usb_device* pUsbDev = NULL;
	struct usb_interface* pUsbInt = NULL;
	struct usb_endpoint_descriptor* pED = NULL;

	//The pUsbMouseDev should be initialized before this thread is running.
	if (NULL == pUsbMouseDev)
	{
		BUG();
		return 0;
	}

	pUsbDev = (struct usb_device*)pUsbMouseDev->lpPrivateInfo;
	pUsbInt = &pUsbDev->config.if_desc[pUsbMouseDev->dwNumber & 0xFFFF];
	pED = &pUsbInt->ep_desc[0];

	//Initialize the USB mouse private data.
	UsbMouseData.ulIntPipe = usb_rcvintpipe(pUsbDev, pED->bEndpointAddress);
	UsbMouseData.intInterval = pED->bInterval;
	UsbMouseData.inputPktSize = min(usb_maxpacket(pUsbDev, UsbMouseData.ulIntPipe),
		MAX_USBMOUSE_BUFF_LEN);

	//Main polling loop.
	while (TRUE)
	{
		if (!USBManager.InterruptMessage(pUsbMouseDev, UsbMouseData.ulIntPipe,
			&UsbMouseData.MouseBuffer[0], UsbMouseData.inputPktSize, UsbMouseData.intInterval))
		{
			//Interpret the USB input data,translate it to kernel message and delivery to kernel.
			DoMouseMessage(&UsbMouseData);
		}
	}
	return 1;
}

//The main entry point of USB mouse driver.
BOOL USBMouse_DriverEntry(__DRIVER_OBJECT* lpDrvObj)
{
	BOOL bResult = FALSE;
	__KERNEL_THREAD_OBJECT* PollThread = NULL;

	//Try to scan USB mouse device in system.
	if (!ScanUsbMouse())
	{
		goto __TERMINAL;
	}

	//Scan OK,create the dedicate polling thread.
	PollThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		USB_Poll_Thread,
		NULL,
		NULL,
		"USB_Poll");
	if (NULL == PollThread)
	{
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#endif  //__CFG_SYS_USB && __CFG_DRV_USBMOUSE

#endif  //__CFG_SYS_DDF
