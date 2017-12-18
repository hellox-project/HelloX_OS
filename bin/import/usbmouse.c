//***********************************************************************/
//    Author                    : Tywind Huang
//    Original Date             : Oct 26,2015
//    Module Name               : usbmouse.c
//    Module Funciton           : 
//    Description               : USB mouse driver source code are put into this file.
//    Last modified Author      :
//    Last modified Date        : 
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

//mouse button 
#define  MOUSE_BUTTON_LEFT     0x1
#define  MOUSE_BUTTON_RIGHT    0x2
#define  MOUSE_BUTTON_MIDDLE   0x4

//mouse pos symbol 
#define  MOUSE_X_SYMBOL       0x10
#define  MOUSE_Y_SYMBOL       0x20

// over flage ()
#define  MOUSE_X_OVER         0x40
#define  MOUSE_Y_OVER         0x80

//button pos 
#define  BUTTON_LEFT          0
#define  BUTTON_RIGHT         1
#define  BUTTON_MIDDLE        2

// x,y pos  SCALE
#define  POS_SCALE           16

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
	id.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_INTERFACECLASS;
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

typedef struct tag__BOTTON_INFO
{
	BOOL    bButtonDown;
	DWORD   dwTickCount;
	WORD    dwDownMsg;
	WORD    dwUpMsg;
	WORD    dwDBClickMsg;

}BOTTON_INFO;


static BOTTON_INFO s_szButtons[3] =
{
	{ 0, 0, KERNEL_MESSAGE_LBUTTONDOWN, KERNEL_MESSAGE_LBUTTONUP, KERNEL_MESSAGE_LBUTTONDBCLK },
	{ 0, 0, KERNEL_MESSAGE_RBUTTONDOWN, KERNEL_MESSAGE_RBUTTONUP, KERNEL_MESSAGE_RBUTTONDBCLK },
	{ 0, 0, KERNEL_MESSAGE_MBUTTONDOWN, KERNEL_MESSAGE_MBUTTONUP, KERNEL_MESSAGE_MBUTTONDBCLK }
};


static BOOL HandleMouseBottonMsg(BOTTON_INFO*  pButtonInfo, BOOL bIsDwon, WORD MouseX, WORD MouseY)
{
	__DEVICE_MESSAGE MouseMsg = { 0 };

	if (bIsDwon)
	{
		if (!pButtonInfo->bButtonDown)
		{
			if (System.dwClockTickCounter - pButtonInfo->dwTickCount <= (200 / SYSTEM_TIME_SLICE))
			{
				//dobule click
				MouseMsg.wDevMsgType = pButtonInfo->dwDBClickMsg;
				pButtonInfo->dwTickCount = 0;
			}
			else
			{
				//down
				MouseMsg.wDevMsgType = pButtonInfo->dwDownMsg;
				pButtonInfo->dwTickCount = System.dwClockTickCounter;
			}

			pButtonInfo->bButtonDown = TRUE;
		}
	}
	else if (pButtonInfo->bButtonDown == TRUE)
	{
		//release 
		MouseMsg.wDevMsgType = pButtonInfo->dwUpMsg;
		pButtonInfo->bButtonDown = FALSE;
		pButtonInfo->dwTickCount = 0;
	}
	else
	{
		// no operate
		return FALSE;
	}

	MouseMsg.dwDevMsgParam = (DWORD)MouseY;
	MouseMsg.dwDevMsgParam <<= 16;
	MouseMsg.dwDevMsgParam += (DWORD)MouseX;

	DeviceInputManager.SendDeviceMessage((__COMMON_OBJECT*)&DeviceInputManager, &MouseMsg, NULL);

	return TRUE;
}

//Parse USB input data,translate it into kernel message,and delivery it to HelloX kernel.
static BOOL DoMouseMessage(__USB_MOUSE_DATA* pMouseData)
{
	static unsigned short s_nMousePosX = 0;
	static unsigned short s_nMousePosY = 0;

#if (ENHANCED_WHEEL_SUPPORT == 1)
	//Just skip the first byte,which is used as report ID.
	//It's a simplified implementation of USB HID device.
	BYTE   bFlags   = pMouseData->MouseBuffer[1];
	UCHAR  cOffsetX = pMouseData->MouseBuffer[2];
	UCHAR  cOffsetY = pMouseData->MouseBuffer[3];
	BYTE   bMouseOffsetS = pMouseData->MouseBuffer[4];
#else
	BYTE   bFlags  = pMouseData->MouseBuffer[0];
	UCHAR  cOffsetX = pMouseData->MouseBuffer[1];
	UCHAR  cOffsetY = pMouseData->MouseBuffer[2];
	BYTE   bMouseOffsetS = pMouseData->MouseBuffer[3];
#endif
	BOOL   bButtonMsgSned = FALSE;

	//Debugging.
	debug("%s: usb data = %X %X %X %X %X %X.\r\n", __func__,
		pMouseData->MouseBuffer[0],
		pMouseData->MouseBuffer[1],
		pMouseData->MouseBuffer[2],
		pMouseData->MouseBuffer[3],
		pMouseData->MouseBuffer[4],
		pMouseData->MouseBuffer[5]);

	//Calculate the real coordinate of x according relative move position.
	//if (bFlags & MOUSE_X_SYMBOL)
	if (cOffsetX > 128)  //Negitive.
	{
		cOffsetX = 255 - cOffsetX;
		if (s_nMousePosX > cOffsetX)
		{
			s_nMousePosX -= cOffsetX;
		}
		else
		{
			s_nMousePosX = 0;
		}
	}
	else  //Postive.
	{
		s_nMousePosX += cOffsetX;
		if (s_nMousePosX > MAX_X_SCALE)
		{
			s_nMousePosX = MAX_X_SCALE;
		}
	}
	//Calculate the real coordinate of y according relative move position.
	//if (bFlags & MOUSE_Y_SYMBOL)
	if (cOffsetY > 128)  //Negitive.
	{
		cOffsetY = 255 - cOffsetY;
		s_nMousePosY += cOffsetY;
		if (s_nMousePosY > MAX_Y_SCALE)
		{
			s_nMousePosY = MAX_Y_SCALE;
		}
	}
	else  //Postive.
	{
		if (s_nMousePosY > cOffsetY)
		{
			s_nMousePosY -= cOffsetY;
		}
		else
		{
			s_nMousePosY = 0;
		}
	}

	//check buttons state
	bButtonMsgSned |= HandleMouseBottonMsg(&s_szButtons[BUTTON_LEFT], bFlags & MOUSE_BUTTON_LEFT, s_nMousePosX, s_nMousePosY);
	bButtonMsgSned |= HandleMouseBottonMsg(&s_szButtons[BUTTON_RIGHT], bFlags & MOUSE_BUTTON_RIGHT, s_nMousePosX, s_nMousePosY);
	bButtonMsgSned |= HandleMouseBottonMsg(&s_szButtons[BUTTON_MIDDLE], bFlags & MOUSE_BUTTON_MIDDLE, s_nMousePosX, s_nMousePosY);

	// not button msg,send mouse move msg
	if (!bButtonMsgSned)
	{
		__DEVICE_MESSAGE MouseMsg = { 0 };

		MouseMsg.dwDevMsgParam = (DWORD)s_nMousePosY;
		MouseMsg.dwDevMsgParam <<= 16;
		MouseMsg.dwDevMsgParam += (DWORD)s_nMousePosX;
		MouseMsg.wDevMsgType = KERNEL_MESSAGE_MOUSEMOVE;

		DeviceInputManager.SendDeviceMessage(
			(__COMMON_OBJECT*)&DeviceInputManager,
			&MouseMsg,
			NULL);
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
	int err = 0;

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
		err = USBManager.InterruptMessage(pUsbMouseDev, UsbMouseData.ulIntPipe,
			&UsbMouseData.MouseBuffer[0], UsbMouseData.inputPktSize, UsbMouseData.intInterval);
		if (!err)
		{
			//Interpret the USB input data,translate it to kernel message and delivery to kernel.
			DoMouseMessage(&UsbMouseData);
		}
		else
		{
			debug("%s: InterruptMessage failed,err = %d.\r\n", __func__,err);
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
		"USB_Mous_Poll");
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
