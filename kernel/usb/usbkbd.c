//***********************************************************************/
//    Author                    : Tywind Huang
//    Original Date             : Oct 28,2015
//    Module Name               : usbkbd.c
//    Module Funciton           : 
//    Description               : USB keyboard driver source code are put into this file.
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
#include "usbkbd.h"

//Only available when the USB KEYBOARD function is enabled.
#if defined(__CFG_DRV_USBKBD) && defined(__CFG_SYS_USB)

//ctrl keys bit flage
#define  KEY_CTRL_FLAGE     0x11
#define  KEY_SHIFT_FLAGE    0x22
#define  KEY_ALT_FLAGE      0x44
#define  KEY_GUI_FLAGE      0x88

#define  KEY_UP             0x0
#define  KEY_DWON           0x1

//scan code max num
#define  MAX_SCANECODE_NUM    6

//scan code start index
#define  SCANCODE_START       2

//Temporary structure used to contain USB mouse related data.
typedef struct tag__USB_KEYBOARD_DATA
{
	unsigned long ulIntPipe;  //Pipe number of USB mouse device.
	int inputPktSize;
	int intInterval;
	unsigned char KeycodeBuffer[MAX_USBKEYBOARD_BUFF_LEN];
}__USB_KEYBOARD_DATA;

typedef struct tag__SYSTEM_KEY
{
	BOOL  bAltDown;
	BOOL  bGuiDown;
	BOOL  bCtrlDown;
	BOOL  bShiftDown;

}SYSTEM_KEY;

typedef struct tag__KEY_INFO
{
	BYTE  bScanCode;
	BYTE  bKeyCode;

}KEY_INFO;

static BYTE usb_scancode_shiftdown[256] =
{
	0, 0, 0, 0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',

	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',

	'#', '$', '%', '^', '&', '*', '(', ')', VK_RETURN, VK_ESC, VK_BACKSPACE, VK_TAB, VK_SPACE, '_', '+', '{',

	'}', '|', 0, ':', '"', '~', '<', '>', '?', VK_CAPS_LOCK, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
	//
	VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_PRINT, VK_SCROLL, VK_BREAK, VK_INSERT, VK_HOME, VK_PAGEUP, VK_DELETE, VK_END, VK_PAGEDOWN, VK_RIGHTARROW,

	VK_LEFTARROW, VK_DOWNARROW, VK_UPARROW, VK_NUMLOCK, '/', '*', '-', '+', VK_RETURN, '1', '2', '3', '4', '5', '6', '7',

	'8', '9', '0', '.', 0, VK_APPS, 0/*power key */, '=', VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20,

	VK_F21, VK_F22, VK_F23, VK_F24, 0, 0, VK_MENU, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

};

static BYTE usb_scancode_shiftup[256] =
{
	0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',

	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',

	'3', '4', '5', '6', '7', '8', '9', '0', VK_RETURN, VK_ESC, VK_BACKSPACE, VK_TAB, VK_SPACE, '-', '=', '[',

	']', 0x5c/*\*/, 0, ';', 0x27, '`', ',', '.', '/', VK_CAPS_LOCK, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,

	VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_PRINT, VK_SCROLL, VK_BREAK, VK_INSERT, VK_HOME, VK_PAGEUP, VK_DELETE, VK_END, VK_DOWN, VK_RIGHTARROW,

	VK_LEFTARROW, VK_DOWNARROW, VK_UPARROW, VK_NUMLOCK, '/', '*', '-', '+', VK_RETURN, '1', '2', '3', '4', '5', '6', '7',

	'8', '9', '0', '.', 0, VK_APPS, 0/*power key */, '=', VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20,

	VK_F21, VK_F22, VK_F23, VK_F24, 0, 0, VK_MENU, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

};

//The USB keyboard device found in system,it will be setby ScanUsbMouse routine.
static __PHYSICAL_DEVICE* pUsbKeyboardDev = NULL;

//Interrupt handler for USBHD.
static BOOL _Keyboard_IntHandler(LPVOID pParam, LPVOID pEsp)
{
	return TRUE;
}

//Scan USB devices in system to find USB Keyboard device.
static BOOL ScanUsbKeyboard()
{
	__PHYSICAL_DEVICE*              pPhyDev = NULL;
	struct usb_device*              pUsbDev = NULL;
	struct usb_interface*           pUsbInt = NULL;
	struct usb_endpoint_descriptor* pED = NULL;
	__IDENTIFIER                    id = { 0 };
	BOOL bResult = FALSE;

	//Initialize id accordingly.
	id.dwBusType = BUS_TYPE_USB;
	id.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_INTERFACECLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACESUBCLASS;
	id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_INTERFACEPROTOCOL;
	id.Bus_ID.USB_Identifier.bInterfaceClass = UK_INTERFACE_CLASS_ID;
	id.Bus_ID.USB_Identifier.bInterfaceSubClass = UK_INTERFACE_SUBCLASS_ID;
	id.Bus_ID.USB_Identifier.bInterfaceProtocol = UK_INTERFACE_PROTOCOL_ID;

	//Try to find the USB mouse device from system.
	pPhyDev = USBManager.GetUsbDevice(&id, NULL);
	if (NULL == pPhyDev)
	{
		_hx_printf("No usb keyboard found.\r\n");
		goto __TERMINAL;
	}

	//Make farther check according HID spec.
	pUsbDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	BUG_ON(NULL == pUsbDev);

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
	_hx_printf("Found USB keyboard device:mf = [%s].\r\n", pUsbDev->mf);
	pUsbKeyboardDev = pPhyDev;
	bResult = TRUE;

__TERMINAL:

	return bResult;
}




//check is visable key by scan pos
static BOOL IsVisableKey(BYTE bScandCode)
{
	if (bScandCode >= 4 && bScandCode <= 40)
	{
		return TRUE;
	}

	if (bScandCode == 42)
	{
		return TRUE;
	}

	if (bScandCode >= 44 && bScandCode <= 56 && bScandCode != 50)
	{
		return TRUE;
	}

	if (bScandCode >= 84 && bScandCode <= 99)
	{
		return TRUE;
	}

	return FALSE;
}


static BOOL OnKeyHandler(BYTE bKeyCode, BOOL bVisableCode, DWORD dwKeyState, DWORD dwCtrlKeyState)
{
	__DEVICE_MESSAGE dmsg;

	if (dwKeyState == KEY_DWON)
	{
		//Control + Alt + Delete combined key pressed ;
		if (bKeyCode == VK_DELETE && (dwCtrlKeyState&KEY_ALT_FLAGE) && (dwCtrlKeyState&KEY_CTRL_FLAGE))
		{
			__DEVICE_MESSAGE msg;

			msg.wDevMsgType = KERNEL_MESSAGE_TERMINAL;
			DeviceInputManager.SendDeviceMessage(
				(__COMMON_OBJECT*)&DeviceInputManager,
				&msg,
				NULL);

			PrintLine("Control + Alt + Delete combined key pressed.");
		}

		//Key is hold(make).
		dmsg.wDevMsgType = (bVisableCode) ? KERNEL_MESSAGE_AKEYDOWN : KERNEL_MESSAGE_VKEYDOWN;

	}
	else  //Key is released.
	{
		dmsg.wDevMsgType = (bVisableCode) ? KERNEL_MESSAGE_AKEYUP : KERNEL_MESSAGE_VKEYUP;
	}

	dmsg.dwDevMsgParam = (DWORD)bKeyCode;

	DeviceInputManager.SendDeviceMessage((__COMMON_OBJECT*)&DeviceInputManager, &dmsg, NULL);

	return TRUE;
}

//Parse USB input data,translate it into kernel message,and delivery it to HelloX kernel.
static BOOL DoKeyboardMessage(__USB_KEYBOARD_DATA* pKeyboardData)
{
	static KEY_INFO  s_szCurKeyInfo[6] = { 0 };
	LPBYTE    pUsbScanCode = usb_scancode_shiftup;
	BYTE      bCtrlKeyState = pKeyboardData->KeycodeBuffer[0];
	INT       i = 0;

	//check shift is down
	if (bCtrlKeyState&KEY_SHIFT_FLAGE)
	{
		pUsbScanCode = usb_scancode_shiftdown;
	}

	//handle  all scan codes
	for (i = 0; i<MAX_SCANECODE_NUM; i++)
	{
		BYTE       bKeyScancode = pKeyboardData->KeycodeBuffer[i + SCANCODE_START];
		KEY_INFO*  pCurKeyInfo = &s_szCurKeyInfo[i];
		BOOL       bVisableCode = FALSE;
		BYTE       bKeyCode = 0;

		//Get real keycode
		bKeyCode = pUsbScanCode[bKeyScancode];

		if (bKeyCode != 0)
		{
			pCurKeyInfo->bKeyCode = bKeyCode;
			pCurKeyInfo->bScanCode = bKeyScancode;

			bVisableCode = IsVisableKey(bKeyScancode);

			OnKeyHandler(bKeyCode, bVisableCode, KEY_DWON, bCtrlKeyState);
		}
		else
		{
			//key release
			if (pCurKeyInfo->bKeyCode)
			{
				bVisableCode = IsVisableKey(pCurKeyInfo->bScanCode);

				OnKeyHandler(pCurKeyInfo->bKeyCode, bVisableCode, KEY_UP, bCtrlKeyState);
				pCurKeyInfo->bKeyCode = 0;
			}
		}
	}

	/*_hx_printf("USB input data: %X %X %X %X %X %X %X %X.\r\n",
	pKeyboardData->KeycodeBuffer[0],
	pKeyboardData->KeycodeBuffer[1],
	pKeyboardData->KeycodeBuffer[2],
	pKeyboardData->KeycodeBuffer[3],
	pKeyboardData->KeycodeBuffer[4],
	pKeyboardData->KeycodeBuffer[5],
	pKeyboardData->KeycodeBuffer[6],
	pKeyboardData->KeycodeBuffer[7]
	);*/

	return TRUE;
}

//A dedicated kernel thread is running to poll USB mouse input looply.
static DWORD USB_Poll_Thread(LPVOID pData)
{
	__USB_KEYBOARD_DATA  UsbKeyboardData;
	struct usb_device*              pUsbDev = NULL;
	struct usb_interface*           pUsbInt = NULL;
	struct usb_endpoint_descriptor* pED = NULL;

	//The pUsbKeyboardDev should be initialized before this thread is running.
	if (NULL == pUsbKeyboardDev)
	{
		BUG();
		return 0;
	}

	pUsbDev = (struct usb_device*)pUsbKeyboardDev->lpPrivateInfo;
	pUsbInt = &pUsbDev->config.if_desc[pUsbKeyboardDev->dwNumber & 0xFFFF];
	pED = &pUsbInt->ep_desc[0];

	//Initialize the USB mouse private data.
	UsbKeyboardData.ulIntPipe = usb_rcvintpipe(pUsbDev, pED->bEndpointAddress);
	UsbKeyboardData.intInterval = pED->bInterval;
	UsbKeyboardData.inputPktSize = min(usb_maxpacket(pUsbDev, UsbKeyboardData.ulIntPipe), MAX_USBKEYBOARD_BUFF_LEN);

	//Main polling loop.
	while (TRUE)
	{
		if (!USBManager.InterruptMessage(pUsbKeyboardDev, UsbKeyboardData.ulIntPipe,
			&UsbKeyboardData.KeycodeBuffer[0], UsbKeyboardData.inputPktSize, UsbKeyboardData.intInterval))
		{
			//Interpret the USB input data,translate it to kernel message and delivery to kernel.
			DoKeyboardMessage(&UsbKeyboardData);
		}
	}

	return 1;
}

//The main entry point of USB mouse driver.
BOOL USBKeyboard_DriverEntry(__DRIVER_OBJECT* lpDrvObj)
{

	__KERNEL_THREAD_OBJECT* PollThread = NULL;
	BOOL                    bResult = FALSE;

	//Try to scan USB mouse device in system.
	if (!ScanUsbKeyboard())
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
		"USB_Keybd");
	if (NULL == PollThread)
	{
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#endif  //__CFG_SYS_USB && __CFG_DRV_USBKBD

#endif  //__CFG_SYS_DDF
