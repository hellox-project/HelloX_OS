//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 16, 2016
//    Module Name               : uvc.c
//    Module Funciton           : 
//                                Implementation of USB Video Class(UVC)'s core
//                                functions.
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

//------------------------------------------------------------------------

/* USB video format array,each supported format in system should put one entry into
this array,the UVC subsystem can the corresponding routine to process format data.
*/
static __UVC_FORMAT_ARRAY_ELEMENT Supported_Format_Array[] = {
	{VS_FORMAT_UNCOMPRESSED,uvcUncompressedParseFormat},
	{0,NULL}
};

//------------------------------------------------------------------------

//UVC low level query operation,query control information from a specified interface.
BOOL uvcQueryInterfaceControl(__USB_VIDEO_DEVICE* pUvcDev, __u8 request, 
	__u8 ifnum, __u8 cs, void* data,__u16 size, int timeout)
{
	__u8 requesttype;
	void* new_data = NULL;
	BOOL bResult = FALSE;

	//Parameter check.
	if ((NULL == pUvcDev) || (NULL == data))
	{
		return FALSE;
	}
	//Use new allocated and aligned buffer instead the original one.
	new_data = _hx_aligned_malloc(size, USB_DMA_MINALIGN);
	if (NULL == new_data)
	{
		return FALSE;
	}
	memcpy(new_data, data, size);

	//Initialize the req structure.
	requesttype = USB_CTRLREQ_TYPE_CS | USB_CTRLREQ_TYPE_INT;
	requesttype |= (request & 0x80) ? USB_DIR_IN : USB_DIR_OUT;

	//Issue control message.
	if (request & 0x80)
	{
		bResult = usb_control_msg(pUvcDev->pUsbDev, pUvcDev->recvCtrlPipe, request, requesttype,
			cs << 8, ifnum, new_data, size, timeout) < 0 ? FALSE : TRUE;
	}
	else
	{
		bResult = usb_control_msg(pUvcDev->pUsbDev, pUvcDev->sendCtrlPipe, request, requesttype,
			cs << 8, ifnum, new_data, size, timeout) < 0 ? FALSE : TRUE;
	}
	if (bResult)
	{
		memcpy(data, new_data, size);
	}
	_hx_free(new_data);
	return bResult;
}

//UVC low level query operation,query control information from a specified unit or terminal.
BOOL uvcQueryUnitControl(__USB_VIDEO_DEVICE* pUvcDev, __u8 request,__u8 entity_id,
	__u8 ifnum, __u8 cs, void* data, __u16 size, int timeout)
{
	__u8 requesttype;
	void* new_data = NULL;
	BOOL bResult = FALSE;

	//Parameter check.
	if ((NULL == pUvcDev) || (NULL == data))
	{
		return FALSE;
	}
	//Use new allocated and aligned buffer instead the original one.
	new_data = _hx_aligned_malloc(size, USB_DMA_MINALIGN);
	if (NULL == new_data)
	{
		return FALSE;
	}
	memcpy(new_data, data, size);

	//Initialize the req structure.
	requesttype = USB_CTRLREQ_TYPE_CS | USB_CTRLREQ_TYPE_INT;
	requesttype |= (request & 0x80) ? USB_DIR_IN : USB_DIR_OUT;

	//Issue control message.
	if (request & 0x80)
	{
		bResult = usb_control_msg(pUvcDev->pUsbDev, pUvcDev->recvCtrlPipe, request, requesttype,
			cs << 8, entity_id << 8 | ifnum, new_data, size, timeout) < 0 ? FALSE : TRUE;
	}
	else
	{
		bResult = usb_control_msg(pUvcDev->pUsbDev, pUvcDev->sendCtrlPipe, request, requesttype,
			cs << 8, entity_id << 8 | ifnum, new_data, size, timeout) < 0 ? FALSE : TRUE;
	}
	if (bResult)
	{
		memcpy(data, new_data, size);
	}
	_hx_free(new_data);
	return bResult;
}

//Get UVC device error code.
__U8 uvcGetDeviceError(__USB_VIDEO_DEVICE* pUvcDev,__u8 ifnum)
{
	__U8 dec = 0;
	
	if (!uvcQueryInterfaceControl(pUvcDev, GET_CUR, ifnum, VC_REQUEST_ERROR_CODE_CONTROL, &dec, 1, 100))
	{
		debug("%s:get device error code failed.\r\n", __func__);
	}
	return dec;
}

//Get video streaming error code.
__U8 uvcGetStreamError(__USB_VIDEO_DEVICE* pUvcDev, __u8 ifnum)
{
	__U8 dec = 0;

	if (NULL == pUvcDev)
	{
		return 0;
	}
	if (ifnum != pUvcDev->streamIntNum)
	{
		return 0;
	}
	if (!uvcQueryInterfaceControl(pUvcDev, GET_INFO, pUvcDev->streamIntNum,
		VS_STREAM_ERROR_CODE_CONTROL, &dec, 1, 100))
	{
		debug("%s:get stream error code failed.\r\n", __func__);
		return 0;
	}
	return dec;
}

//Helper routine to show out the negotiation result of streaming interface.
static VOID ShowStreamConfig(struct uvc_video_probe_commit* pUVPC)
{
	_hx_printf("  Streaming parameters for current set:\r\n");
	_hx_printf("    FrameInterval = %d.\r\n", get_unaligned(&pUVPC->dwFrameInterval));
	_hx_printf("    MaxPayloadTxSz = %d.\r\n", get_unaligned(&pUVPC->dwMaxPayloadTransferSize));
	_hx_printf("    KeyFrameRate = %d.\r\n", get_unaligned(&pUVPC->wKeyFrameRate));
	_hx_printf("    MaxFrameSize = %d.\r\n", get_unaligned(&pUVPC->dwMaxVideoFrameSize));
	_hx_printf("    Delay = %d.\r\n", get_unaligned(&pUVPC->wDelay));
	_hx_printf("    Usage = %d.\r\n", get_unaligned(&pUVPC->bUsage));
}

//Streaming interface parameters negotiation.
BOOL uvcStreamNegotiation(__USB_VIDEO_DEVICE* pUvcDev, __U16 bmHint, __U8 FormatIndex, __U8 FrameIndex,
	int bandwidth,struct uvc_video_probe_commit* pUVPC)
{
	BOOL bResult = FALSE;
	int local_bw = 0;

	if (NULL == pUvcDev)
	{
		goto __TERMINAL;
	}
	if ((0 == FormatIndex) || (0 == FrameIndex))
	{
		goto __TERMINAL;
	}
	//Initialize the probe commit struct.
	memset(pUVPC, 0, sizeof(*pUVPC));
	put_unaligned(bmHint, &pUVPC->bmHint);
	put_unaligned(FormatIndex, &pUVPC->bFormatIndex);
	put_unaligned(FrameIndex, &pUVPC->bFrameIndex);

	//Issue SET_CUR request to streaming interface's probe control.
	bResult = uvcQueryInterfaceControl(pUvcDev, SET_CUR, pUvcDev->streamIntNum, 
		VS_COMMIT_CONTROL, pUVPC, sizeof(*pUVPC), 100);
	if (!bResult)
	{
		_hx_printf("%s:SET_CUR of commit control failed,err = %d.\r\n", __func__,
			uvcGetDeviceError(pUvcDev,pUvcDev->streamIntNum));
		goto __TERMINAL;
	}
	//GET_CUR request to obtain the configuration of current set.
	bResult = uvcQueryInterfaceControl(pUvcDev, GET_CUR, pUvcDev->streamIntNum, 
		VS_PROBE_CONTROL, pUVPC, sizeof(*pUVPC), 100);
	if (!bResult)
	{
		_hx_printf("%s:GET_CUR of probe control failed,err = %d.\r\n", __func__,
			uvcGetDeviceError(pUvcDev, pUvcDev->streamIntNum));
		goto __TERMINAL;
	}

	//SET_CUR request to set the current configuration of UVC device,according UVC spec.
	bResult = uvcQueryInterfaceControl(pUvcDev, SET_CUR, pUvcDev->streamIntNum,
		VS_PROBE_CONTROL, pUVPC, sizeof(*pUVPC), 100);
	if (!bResult)
	{
		_hx_printf("%s:SET_CUR of probe control failed,err = %d.\r\n", __func__,
			uvcGetDeviceError(pUvcDev, pUvcDev->streamIntNum));
		goto __TERMINAL;
	}

	//GET_CUR OK,parse it.
	pUvcDev->maxXferSize = get_unaligned(&pUVPC->dwMaxPayloadTransferSize);
	pUvcDev->frameSize = get_unaligned(&pUVPC->dwMaxVideoFrameSize);
	pUvcDev->streamBandwidth = bandwidth;

	//Show negotiation result.
	ShowStreamConfig(pUVPC);

__TERMINAL:
	return bResult;
}

/* Initializes streaming related variables,allocate streaming buffer,and configure the
   Video device into streaming state. */
BOOL uvcPrepareStreaming(__USB_VIDEO_DEVICE* pUvcDev, int bandwidth, struct uvc_video_probe_commit* pUVPC)
{
	BOOL bResult = FALSE;
	int alternate = 0;      //Alternate setting of the streaming interface.
	__U32 maxTxSize = 0;
	__U32 maxBulkSize = 0;  //Maximal ISO xfer bulk size.
	struct usb_interface* pStreamInt = NULL;
	struct usb_endpoint_descriptor* pStreamEP = NULL;
	unsigned int max_pk_sz = 0;    //Maximal packet size of the endpoint.
	unsigned int multi = 0;        //Transaction numbers per (micro)frame.
	int i = 0, j = 0;

	if ((NULL == pUvcDev) || (NULL == pUVPC))
	{
		goto __TERMINAL;
	}
	if ((pUvcDev->streamBuff != NULL) || (pUvcDev->isoDesc != NULL))  //Already initialized.
	{
		BUG();
		goto __TERMINAL;
	}
	//Issue SET_CUR request to streaming interface's commit control.
	if(!uvcQueryInterfaceControl(pUvcDev, SET_CUR, pUvcDev->streamIntNum,
		VS_COMMIT_CONTROL, pUVPC, sizeof(*pUVPC), 100))
	{
		_hx_printf("%s:SET_CUR of commit control failed,err = %d.\r\n", __func__,
			uvcGetDeviceError(pUvcDev, pUvcDev->streamIntNum));
		goto __TERMINAL;
	}

	//We use maximal transfer size as factor to select the proper alternat setting.
	maxTxSize = get_unaligned(&pUVPC->dwMaxPayloadTransferSize);
	if (maxTxSize != pUvcDev->maxXferSize)
	{
		BUG();
		goto __TERMINAL;
	}

	//Find the proper alternate setting according xfer size.
	for (i = 0; i < pUvcDev->pIntAssoc->Interfaces[UVC_DEFAULT_STREAM_INT].nAlternateNum; i++)
	{
		pStreamInt = pUvcDev->pIntAssoc->Interfaces[UVC_DEFAULT_STREAM_INT].pAltInterfaces[i];
		for (j = 0; j < pStreamInt->no_of_ep; j++)
		{
			pStreamEP = &pStreamInt->ep_desc[j];
			//Only ISO endpoint applies.
			if ((get_unaligned(&pStreamEP->bmAttributes) & 3) == 1)
			{
				max_pk_sz = get_unaligned(&pStreamEP->wMaxPacketSize);
				multi = (max_pk_sz >> 11) & 3;
				multi += 1;
				max_pk_sz &= 0x7FF;
				if (max_pk_sz * multi >= maxTxSize)  //Found.
				{
					alternate = i + 1;
					break;
				}
			}
		}
		if (alternate != 0)  //Found the alternate setting.
		{
			pUvcDev->alternates = alternate;
			break;
		}
	}
	if (0 == alternate)  //Can not find proper alternate setting.
	{
		_hx_printf("%s:can not find alt setting with max_xfer_size >= %d.\r\n",
			__func__, maxTxSize);
		goto __TERMINAL;
	}

	//Change streaming interface to alternate setting,this lead the streaming start.
	if (usb_set_interface(pUvcDev->pUsbDev, pUvcDev->streamIntNum, alternate) < 0)
	{
		_hx_printf("%s:can not change int [%d] to alt setting [%d].\r\n",
			__func__, pUvcDev->streamIntNum, alternate);
		goto __TERMINAL;
	}

	//Everything is OK.
	_hx_printf("%s:begin streaming at alt_setting [%d],ep [%d].\r\n", __func__,
		alternate,pStreamEP->bEndpointAddress & 0xF);

	//Allocate xfer buffer,assume high speed USB interface...
	maxBulkSize = pUvcDev->maxXferSize * 8;
	//pUvcDev->streamBuff = _hx_aligned_malloc(maxBulkSize, ARCH_DMA_MINALIGN);
	pUvcDev->streamBuff = _hx_aligned_malloc(maxBulkSize, 4096);
	if (NULL == pUvcDev->streamBuff)
	{
		_hx_printf("%s:failed to allocate video buff.\r\n", __func__);
		goto __TERMINAL;
	}

	//Create a USB isochronous xfer descriptor.
	pUvcDev->isoDesc = usbCreateISODescriptor(pUvcDev->pPhyDev, USB_TRANSFER_DIR_IN, 
		bandwidth,
		pUvcDev->streamBuff, maxBulkSize, pStreamEP->bEndpointAddress & 0x0F,
		max_pk_sz, multi);
	if (NULL == pUvcDev->isoDesc)
	{
		_hx_printf("%s:failed to create USB iso_desc.\r\n", __func__);
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if (!bResult)  //Failed,all things initialized or created in this routine should be released.
	{
		pUvcDev->alternates = 0;
		if (pUvcDev->streamBuff)
		{
			_hx_free(pUvcDev->streamBuff);
			pUvcDev->streamBuff = NULL;
		}
		if (pUvcDev->isoDesc)
		{
			usbDestroyISODescriptor(pUvcDev->isoDesc);
			pUvcDev->isoDesc = NULL;
		}
		//Stop streaming.
		usb_set_interface(pUvcDev->pUsbDev, pUvcDev->streamIntNum, 0);
	}
	return bResult;
}

//Old one,only for reference.
BOOL __uvcStartStreaming(__USB_VIDEO_DEVICE* pUvcDev, char* frameBuff,int buffSize,int* sizeReturned)
{
	BOOL bResult = FALSE;

	//if ((NULL == pUvcDev) || (NULL == frameBuff))
	//{
	//	goto __TERMINAL;
	//}
	if ((NULL == pUvcDev->isoDesc) || (NULL == pUvcDev->streamBuff))
	{
		BUG();
		goto __TERMINAL;
	}

	while (TRUE)
	{
		if (usbStartISOXfer(pUvcDev->isoDesc))
		{
			_hx_printf("%s:get video content with length = %d,first_4oct = %X.\r\n",
				__func__,
				pUvcDev->isoDesc->xfersize,
				*(int*)pUvcDev->streamBuff);
		}
		else
		{
			_hx_printf("%s:get video content failed.\r\n", __func__);
			//usbStopISOXfer(pUvcDev->isoDesc);
			//usbDestroyISODescriptor(pUvcDev->isoDesc);
			//goto __TERMINAL;
		}
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Get one video frame from the current streaming bit stream.
BOOL uvcStartStreaming(__USB_VIDEO_DEVICE* pUvcDev, char* frameBuff, int buffSize, int* sizeReturned)
{
	struct usb_video_format_header* pFormat = NULL;

	if ((NULL == pUvcDev) || (NULL == frameBuff))
	{
		return FALSE;
	}
	if ((NULL == pUvcDev->isoDesc) || (NULL == pUvcDev->streamBuff))
	{
		BUG();
		return FALSE;
	}
	if (NULL == pUvcDev->pCurrentFormat)
	{
		BUG();
		return FALSE;
	}
	pFormat = pUvcDev->pCurrentFormat;
	//Call the format specific streaming routine to fill frame buffer.
	return pFormat->StartStreaming(pUvcDev, pFormat, frameBuff, buffSize, sizeReturned);
}

/* Stop streaming on a specified interface. */
BOOL uvcStopStreaming(__USB_VIDEO_DEVICE* pUvcDev)
{
	if (NULL == pUvcDev)
	{
		return FALSE;
	}
	//Just switch to alternate setting 0,according UVC spec.
	if (usb_set_interface(pUvcDev->pUsbDev, pUvcDev->streamIntNum, 0) < 0)
	{
		_hx_printf("%s:stop streaming failed.\r\n", __func__);
		return FALSE;
	}
	//Stop isochronous transfering of the current UVC device.
	usbStopISOXfer(pUvcDev->isoDesc);
	//Destroy the isochronous xfer descriptor of UVC device. ---- BUGS HERE ----
	usbDestroyISODescriptor(pUvcDev->isoDesc);
	pUvcDev->isoDesc = NULL;
	//Release the streaming buffer of USB video device.
	_hx_free(pUvcDev->streamBuff);
	pUvcDev->streamBuff = NULL;
	return TRUE;
}

/* Obtain all supported UVC video formats and their frames by parsing class specific
   information. It just calls the appropriate routine in format array according the format
   type value,this makes sure that the implementation of this routine is independ with
   specific video formats. */
BOOL uvcDecodeFormat(__USB_VIDEO_DEVICE* pUvcDev)
{
	struct usb_device* pUsbDev = NULL;
	char* pCSInterface = NULL;
	BOOL bResult = FALSE;
	BOOL bParsed = FALSE;
	struct usb_cs_interface_header* head = NULL;
	int parsedLength = 0;
	__UVC_FORMAT_ARRAY_ELEMENT* pFormatElement = NULL;

	if (NULL == pUvcDev)
	{
		goto __TERMINAL;
	}
	pUsbDev = pUvcDev->pUsbDev;
	if (NULL == pUsbDev)
	{
		BUG();
	}
	pCSInterface = pUsbDev->config.pClassSpecificInterfaces;
	if (NULL == pCSInterface)
	{
		BUG();
	}
	head = (struct usb_cs_interface_header*)pCSInterface;
	while ((parsedLength < USB_MAX_CSINTERFACE_LEN) && (head->bLength != 0))
	{
		pFormatElement = &Supported_Format_Array[0];
		bParsed = FALSE;
		while (pFormatElement->formatType != 0)
		{
			if (head->bSubtype == pFormatElement->formatType)
			{
				if (NULL == pFormatElement->ParseFormat)
				{
					BUG();
				}
				//Call the parse routine specific to this format.
				pCSInterface = pFormatElement->ParseFormat((char*)head,
					pUvcDev);
				if (NULL == pCSInterface)  //Failed to process the format data.
				{
					goto __TERMINAL;
				}
				parsedLength += pCSInterface - (char*)head;
				head = (struct usb_cs_interface_header*)pCSInterface;
				bParsed = TRUE;
				break;
			}
			pFormatElement += 1;
		}
		//Can not find a proper format parser,just skip this CS interface.
		if (!bParsed)
		{
			pCSInterface += head->bLength;
			parsedLength += head->bLength;
			head = (struct usb_cs_interface_header*)pCSInterface;
		}
	}

	//Set current format.
	if (pUvcDev->pFormatList)
	{
		pUvcDev->pCurrentFormat = pUvcDev->pFormatList;
		pUvcDev->currFormat = pUvcDev->pCurrentFormat->FormatIndex;
		pUvcDev->currFrame = pUvcDev->pCurrentFormat->currFrameIndex;
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/* Register a new USB video device into system,it's called by UVC
   driver in most case. */
static BOOL uvcRegisterVideoDevice(__USB_VIDEO_DEVICE* pNewDevice)
{
	if (NULL == pNewDevice)
	{
		return FALSE;
	}
	pNewDevice->pNext = UsbVideoManager.pUsbVideoList;
	UsbVideoManager.pUsbVideoList = pNewDevice;
	return TRUE;
}

//Global USB video manager object.
__USB_VIDEO_MANAGER UsbVideoManager = {
	NULL,                    //pUsbVideoList.
	uvcRegisterVideoDevice   //RegisterVideoDevice.
};
