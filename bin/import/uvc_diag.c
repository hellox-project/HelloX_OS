//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13, 2016
//    Module Name               : uvc_diag.c
//    Module Funciton           : 
//                                Implementation file of USB video diagnostic
//                                application.All routines in this file will
//                                be refered by the shell code file 'usbvideo.c'.
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
#include "fmt_ucmp.h"

//A local helper routine to show a video format object.
static void ShowVideoFormat(struct usb_video_format_header* pFormat)
{
	_hx_printf("  Format index: %d.\r\n", pFormat->FormatIndex);
	_hx_printf("  Format name: %s.\r\n", pFormat->szFormatName);
	_hx_printf("  Number of frames: %d.\r\n", pFormat->NumOfFrames);
}

//Show out all supported video format information.
void uvcFormatList()
{
	__USB_VIDEO_DEVICE* pVideoDevice = UsbVideoManager.pUsbVideoList;
	struct usb_video_format_header* pFormat = NULL;

	while (pVideoDevice)
	{
		pFormat = pVideoDevice->pFormatList;
		_hx_printf("Format list about USB video device[%s]:\r\n",
			pVideoDevice->pUsbDev->mf);
		_hx_printf("----------------------------------\r\n");
		while (pFormat)
		{
			ShowVideoFormat(pFormat);
			pFormat = pFormat->pNext;
		}
		pVideoDevice = pVideoDevice->pNext;
	}
	return;
}

//Show verbose information of a specified format.
void uvcFormatInfo(int index)
{
	__USB_VIDEO_DEVICE* pVideoDevice = UsbVideoManager.pUsbVideoList;
	struct usb_video_format_header* pFormat = NULL;

	if (pVideoDevice)
	{
		pFormat = pVideoDevice->pFormatList;
		while (pFormat)
		{
			pFormat->ShowFormatInfo(pFormat);
			pFormat = pFormat->pNext;
		}
	}
	return;
}

//Set current streaming format and frame.
void uvcSetCurrent(int fmt, int frm)
{
	struct usb_video_format_header* pFormat = NULL;
	__USB_VIDEO_DEVICE* pVideoDevice = UsbVideoManager.pUsbVideoList;
	int minBitRate = 1024 * 1024 * 24;

	if ((fmt < 1) || (frm < 1))
	{
		_hx_printf("  Please speicify valid index value[>= 1].\r\n");
		return;
	}
	if (NULL == pVideoDevice)
	{
		_hx_printf("  No USB video device exists.\r\n");
		return;
	}
	pFormat = pVideoDevice->pFormatList;
	while (pFormat)
	{
		if (pFormat->FormatIndex == fmt)
		{
			break;
		}
		pFormat = pFormat->pNext;
	}
	if (NULL == pFormat)
	{
		_hx_printf("  Can not locate the format you specified.\r\n");
		return;
	}
	//Check if frame index is valid.
	if (frm > pFormat->NumOfFrames)
	{
		_hx_printf("  Please specify a valid frame index[from 1 to %d].\r\n",
			pFormat->NumOfFrames);
		return;
	}

	//Try to negotiate streaming parameters with USB video device.
	if (!uvcStreamNegotiation(pVideoDevice, 1, fmt, frm, 0, &pVideoDevice->uvpc))
	{
		_hx_printf("  Failed to negotiate streaming config,err = %d.\r\n",
			uvcGetDeviceError(pVideoDevice, pVideoDevice->streamIntNum));
		return;
	}

	//OK,save the current settings' information to video device object.
	pVideoDevice->currFormat = fmt;
	pVideoDevice->currFrame = frm;
	pVideoDevice->pCurrentFormat = pFormat;
	pFormat->currFrameIndex = frm;
	pFormat->GetFrameBitRate(pFormat, frm, &minBitRate, NULL);
	pVideoDevice->streamBandwidth = minBitRate;  //Use minimal bit rate as streaming bandwidth.

	_hx_printf("  Current setting is: fmt_index = %d,frm_index = %d,min_br = %d.\r\n", 
		fmt, frm,minBitRate);
	return;
}

//Display a video frame on screen or other device,return TRUE to indicate the caller that
//another frame is required,FALSE indicates that no more frame is need.
static BOOL ShowVideoFrame(char* frameBuff, int size)
{
	static int count = 6;
	//Just put your showing code here.
	while (count)
	{
		_hx_printf("  Show a frame with length = %d,first_4_bytes = %08X.\r\n",
			size,
			*(__U32*)frameBuff);
		count--;
		return TRUE;
	}
	_hx_printf("  Show a frame with length = %d,first_4_bytes = %08X.\r\n",
		size,
		*(__U32*)frameBuff);
	//Reset to default value.
	count = 6;
	return FALSE;
}

//Start streaming on the current setting.
void uvcStart()
{
	__USB_VIDEO_DEVICE* pVideoDevice = UsbVideoManager.pUsbVideoList;
	BOOL bResult = FALSE;
	char* frameBuff = NULL;
	int size_ret = 0;
	int try_times = 0;

	//Try to negotiate configuration with USB video device.
	if (!uvcStreamNegotiation(pVideoDevice, 1, pVideoDevice->currFormat,
		pVideoDevice->currFrame, pVideoDevice->streamBandwidth, &pVideoDevice->uvpc))
	{
		_hx_printf("  Failed to negotiate parameters,err = %d.\r\n",
			uvcGetDeviceError(pVideoDevice, pVideoDevice->streamIntNum));
		goto __TERMINAL;
	}
	//Prepare to streaming.
	if (!uvcPrepareStreaming(pVideoDevice, pVideoDevice->streamBandwidth, &pVideoDevice->uvpc))
	{
		_hx_printf("  Failed to prepare streaming,err = %d.\r\n",
			uvcGetDeviceError(pVideoDevice, pVideoDevice->streamIntNum));
		goto __TERMINAL;
	}
	//OK,try to capture frames from streaming and display it.
	if (0 == pVideoDevice->frameSize)
	{
		_hx_printf("  Frame size in UVC device is zero.\r\n");
		goto __TERMINAL;
	}
	frameBuff = _hx_aligned_malloc(pVideoDevice->frameSize, DEFAULT_CACHE_LINE_SIZE);
	if (NULL == frameBuff)
	{
		_hx_printf("%s:failed to allocate frame buffer.\r\n", __func__);
		goto __TERMINAL;
	}
	while (TRUE)
	{
		if (uvcStartStreaming(pVideoDevice, frameBuff, pVideoDevice->frameSize, &size_ret))
		{
			if (!ShowVideoFrame(frameBuff, size_ret))
			{
				//Just free the frame buffer.
				_hx_free(frameBuff);
				frameBuff = NULL;
				break;
			}
		}
		else    //Get streaming data error.
		{
			try_times++;
			if (try_times > 128)  //Too many errors,just break.
			{
				_hx_free(frameBuff);
				break;
			}
		}
	}
	//Stop streaming.
	uvcStopStreaming(pVideoDevice);
	bResult = TRUE;
__TERMINAL:
	if (!bResult)
	{
		if (frameBuff)
		{
			_hx_free(frameBuff);
		}
	}
	return;
}
