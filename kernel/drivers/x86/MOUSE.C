//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28, 2009
//    Module Name               : MOUSE.CPP
//    Module Funciton           : 
//                                This module countains the implementation code
//                                of MOUSE driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#ifndef __KAPI_H__
#include <kapi.h>
#endif
#include <stdio.h>

#ifndef __MOUSE_H__
#include "mouse.h"
#endif



//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//Global variables used by this module.
static HANDLE g_hIntHandler = NULL;     //Interrupt object handle,to preserve 
                                        //the interrupt object.

//Initialization routine of MOUSE.
static BOOL InitMouse()
{
	//Enable MOUSE channel of i8042 chip.
	__outb(0xA8,MOUSE_CTRL_PORT);  //Enable MOUSE.
	__outb(0xD4,MOUSE_CTRL_PORT);
	
	__inb(MOUSE_DATA_PORT);        //Consume the pendding data.
	__inb(MOUSE_DATA_PORT);
	__inb(MOUSE_DATA_PORT);
	__inb(MOUSE_DATA_PORT);
	__outb(0x60,MOUSE_CTRL_PORT);
	__outb(0x47,MOUSE_DATA_PORT);  //Enable keyboard and mouse interrupt.

	__outb(0xF4,MOUSE_DATA_PORT);  //Enable mouse to send data to host.

	return TRUE;
}

//Unload entry for MOUSE driver.
static DWORD MouseDestroy(__COMMON_OBJECT* lpDriver,
					   __COMMON_OBJECT* lpDevice,
					   __DRCB*          lpDrcb)
{
	DisconnectInterrupt(g_hIntHandler);  //Release key board interrupt.
	return 0;
}

//Interrupt handler of MOUSE.
static BOOL MouseIntHandler(LPVOID pParam,LPVOID pEsp)
{
	static BYTE MsgCount    = 0;
	static WORD x           = 0;
	static BOOL xpostive    = TRUE;   //True if x scale is postive.
	static WORD y           = 0;
	static BOOL ypostive    = TRUE;   //True if y scale is postive.
	static BOOL bLDPrev     = FALSE;  //Left button previous status,TRUE if down.
	static BOOL bRDPrev     = FALSE;  //Right button previous status,TRUE if down.
	static BOOL bLDCurr     = FALSE;  //Current left button status,TRUE if down.
	static BOOL bRDCurr     = FALSE;  //Current Right button status,TRUE if down.
	static BOOL bHasLDown   = FALSE;  //Left button has down before this down status.
	static BOOL bHasRDown   = FALSE;
	static DWORD dwTickCount = 0;
	__DEVICE_MESSAGE dmsg;
	UCHAR  data;

	data = __inb(MOUSE_DATA_PORT);    //Read the input data.
	MsgCount ++;
	switch(MsgCount)
	{
	case 1:    //The first byte of one mouse event.
		bLDCurr    = data & 0x01;     //If left button pressed.
		bRDCurr    = data & 0x02;     //If right button pressed.
		xpostive   = !(data & 0x10);  //If x is postive.
		ypostive   = !(data & 0x20);  //If y is postive.
		break;
	case 2:    //The second byte of one mouse event.
		if(xpostive)
		{
			x += data;
			if(x >= MAX_X_SCALE)  //Exceed the max X range.
			{
				x = MAX_X_SCALE;
			}
		}
		else
		{
			data = 255 - data;    //CAUTION HERE!
			if(x >= (WORD)data)
			{
				x -= (WORD)data;
			}
			else
			{
				x = 0;
			}
		}
		break;
	case 3:    //The third byte of one mouse event.
		if(!ypostive)  //For Y scale,down as postive.
		{
			data = 255 - data;    //CAUTION HERE!
			y += data;
			if(y >= MAX_Y_SCALE)  //Exceed the max X range.
			{
				y = MAX_Y_SCALE;
			}
		}
		else
		{
			if(y >= (WORD)data)
			{
				y -= (WORD)data;
			}
			else
			{
				y = 0;
			}
		}
#define XOR(a,b) ((a && (!b)) || (b && (!a)))
		//Send the message to current focus thread.
		if(XOR(bLDPrev,bLDCurr))  //Left button status changed.
		{
			if(bLDPrev)
			{
				dmsg.wDevMsgType = KERNEL_MESSAGE_LBUTTONUP;
			}
			else
			{
				dmsg.wDevMsgType = KERNEL_MESSAGE_LBUTTONDOWN;
				if(bHasLDown)  //A left button event has occured before this one.
				{
					if(System.dwClockTickCounter - dwTickCount <= 3)
					{
						dmsg.wDevMsgType = KERNEL_MESSAGE_LBUTTONDBCLK;
						bHasLDown = FALSE;
					}
					else
					{
						dwTickCount = System.dwClockTickCounter;
					}
				}
				else
				{
					bHasLDown = TRUE;
					dwTickCount = System.dwClockTickCounter;
				}
			}
		}
		else
		{
			if(XOR(bRDPrev,bRDCurr))  //Right button status changed.
			{
				if(bRDPrev)
				{
					dmsg.wDevMsgType = KERNEL_MESSAGE_RBUTTONUP;
				}
				else
				{
					dmsg.wDevMsgType = KERNEL_MESSAGE_RBUTTONDOWN;
					if(bHasRDown)    //A right button down event has occured before this one.
					{
						if(System.dwClockTickCounter - dwTickCount <= 3)
						{
							dmsg.wDevMsgType = KERNEL_MESSAGE_RBUTTONDBCLK;
							bHasRDown = FALSE;
						}
						else
						{
							dwTickCount = System.dwClockTickCounter;
						}
					}
					else
					{
						bHasRDown = TRUE;
						dwTickCount = System.dwClockTickCounter;
					}
				}
			}
			else  //Now button status is changed.
			{
				dmsg.wDevMsgType = KERNEL_MESSAGE_MOUSEMOVE;
			}
		}
		dmsg.dwDevMsgParam = (DWORD)y;
		dmsg.dwDevMsgParam <<= 16;
		dmsg.dwDevMsgParam += (DWORD)x;
		DeviceInputManager.SendDeviceMessage(
			(__COMMON_OBJECT*)&DeviceInputManager,
			&dmsg,
			NULL);
		//Change status variables.
		bLDPrev = bLDCurr;
		bRDPrev = bRDCurr;
		MsgCount = 0;  //Reset.
		break;
	}
	return TRUE;
}

//Main entry point of MOUSE driver.
BOOL MouseDrvEntry(__DRIVER_OBJECT* lpDriverObject)
{
	__DEVICE_OBJECT*  lpDevObject = NULL;
	BOOL              bResult     = FALSE;

	g_hIntHandler = ConnectInterrupt(MouseIntHandler,
		NULL,
		MOUSE_INT_VECTOR);
	if(NULL == g_hIntHandler)  //Can not connect interrupt.
	{
		goto __TERMINAL;
	}

	//Initialize the mouse device.
	if(!InitMouse())
	{
		goto __TERMINAL;
	}

	//Create driver object for mouse.
	lpDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
		"\\\\.\\MOUSE",
		0,
		0,
		0,
		0,
		NULL,
		lpDriverObject);
	if(NULL == lpDevObject)  //Failed to create device object.
	{
		PrintLine("Mouse Driver: Failed to create device object for MOUSE.");
		goto __TERMINAL;
	}

	//Asign call back functions of driver object.
	lpDriverObject->DeviceDestroy = MouseDestroy;
	
	bResult = TRUE; //Indicate the whole process is successful.
__TERMINAL:
	if(!bResult)  //Should release all resource allocated above.
	{
		if(lpDevObject)
		{
			IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
				lpDevObject);
		}
		if(g_hIntHandler)
		{
			DisconnectInterrupt(g_hIntHandler);
			g_hIntHandler = NULL;  //Set to initial value.
		}
	}
	return bResult;
}

#endif
