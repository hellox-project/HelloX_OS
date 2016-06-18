//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 12, 2016
//    Module Name               : fmt_ucmp.c
//    Module Funciton           : 
//                                Implementation file of USB video uncompressed
//                                streaming format.
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

//GUIDs for different uncompressed format.

//Helper routine used to show out a GUID.
static void ShowGUID(char* tmp)
{
	int i = 0;
	unsigned char* guid = (unsigned char*)tmp;

	_hx_printf("%08X", *(__u32*)guid);
	guid += 4;

	_hx_printf("-");
	_hx_printf("%04X", *(__u16*)guid);
	guid += 2;

	_hx_printf("-");
	_hx_printf("%04X", *(__u16*)guid);
	guid += 2;

	_hx_printf("-");
	_hx_printf("%04X", *(__u16*)guid);
	guid += 2;

	_hx_printf("-");
	for (i = 0; i < 6; i++)
	{
		_hx_printf("%02X", *guid);
		guid++;
	}
}

//Get uncompressed format's name according to it's GUID.
static char* GetFormatName(char* guid)
{
	return VS_FORMAT_UNCOMPRESSED_NAME_YUY2;
}

//Decode operation of this format.
static BOOL uvcUncomStartStreaming(__USB_VIDEO_DEVICE* pUvcDev, struct usb_video_format_header* format,
	char* frameBuff, int size, int* sizeReturn)
{
	struct uvc_uncompressed_stream_hdr* pStreamHdr = NULL;
	int xfer_size = 0;
	int accumu_size = 0;       //Accumulates all xfer size,will be returned as total size of a frame.
	char* xfer_buff = NULL;
	char* currPos = frameBuff;
	int rest_length = size;
	int content_size = 0;
	int transact = 0;

	while (TRUE)
	{
		if (usbStartISOXfer(pUvcDev->isoDesc))
		{
			_hx_printf("%s:video content len = %d,first_4oct = %X.\r\n",
				__func__,
				pUvcDev->isoDesc->xfersize,
				*(int*)pUvcDev->streamBuff);
			for (transact = 0; transact < 8; transact++)
			{
				if (0 == pUvcDev->isoDesc->FrameBuffer[transact].xfer_length)
				{
					break;  //Assume all rest xfers are not used.
				}
				//xfer_size = pUvcDev->isoDesc->xfersize;
				xfer_size = pUvcDev->isoDesc->FrameBuffer[transact].xfer_length;
				//xfer_buff = pUvcDev->streamBuff;
				xfer_buff = pUvcDev->isoDesc->FrameBuffer[transact].pBuffer;

				pStreamHdr = (struct uvc_uncompressed_stream_hdr*)xfer_buff;
				//Validate the xfer size.
				if (xfer_size < pStreamHdr->hdr_length)
				{
					_hx_printf("%s:xfer_sz < hdr_len.\r\n", __func__);
					return FALSE;
				}
				if (pStreamHdr->bit_field_hdr & UVC_UNCOM_BFH_ERR)  //Something error.
				{
					_hx_printf("Stream content error[#],BFH = %X,accu_sz = %d.\r\n",
						//uvcGetStreamError(pUvcDev,pUvcDev->streamIntNum),
						pStreamHdr->bit_field_hdr,
						accumu_size);
					return FALSE;
				}
				//Accumulate the data into frame buffer.
				xfer_buff += pStreamHdr->hdr_length;
				content_size = xfer_size - pStreamHdr->hdr_length;
				if (rest_length < content_size)
				{
					//Frame buffer is overflow,error.
					_hx_printf("%s:revc buff overflow,rest_len = %d.\r\n", __func__, rest_length);
					return FALSE;
				}
				rest_length -= content_size;
				accumu_size += content_size;
				memcpy(currPos, xfer_buff, content_size);
				currPos += content_size;
				if (pStreamHdr->bit_field_hdr & UVC_UNCOM_BFH_EOF)  //End of frame.
				{
					if (sizeReturn)
					{
						*sizeReturn = accumu_size;
					}
					_hx_printf("%s:accumulated a complete frame.\r\n", __func__);
					return TRUE;
				}
			}
		}
		else
		{
			_hx_printf("%s:get video content failed.\r\n", __func__);
			break;
		}
	}
	return FALSE;
}

//Show out the format information.
static VOID uvcUncomShowFormatInfo(struct usb_video_format_header* pFormatHdr)
{
	__UVC_UNCOMPRESSED_FORMAT* pFormat = NULL;
	__UVC_UNCOMPRESSED_FRAME* pFrame = NULL;
	int i = 0;

	if (NULL == pFormatHdr)
	{
		return;
	}
	pFormat = pFormatHdr->SpecFormatData;
	pFrame = pFormat->pFrameArray;
	//Show format information.
	_hx_printf("  Format GUID:");
	ShowGUID(pFormat->guidFormat);
	_hx_printf("\r\n");
	_hx_printf("  Format_Index: %d, Demension: %d:%d\r\n", pFormatHdr->FormatIndex,
		pFormat->aspectRatioX, pFormat->aspectRatioY);
	_hx_printf("  BitsPerPixel: %d, default_frame: %d\r\n", pFormat->bitsPerPixel,
		pFormat->defaultFrameIndex);
	_hx_printf("  Frame(s) belong to this format:\r\n");
	//Show all frames information.
	for (i = 0; i < pFormatHdr->NumOfFrames; i++)
	{
		_hx_printf("    Idx[%d]: width = %d,height = %d,min_r = %d,max_r = %d.\r\n",
			pFrame[i].frameIndex, pFrame[i].width, pFrame[i].height,
			pFrame[i].minBitRate, pFrame[i].maxBitRate);
		_hx_printf("           : default_frame_interval = %d.\r\n",
			pFrame[i].frameIndex, pFrame[i].defaultFrameInterval);
	}
	return;
}

//Get the max and min bit rate of a given frame,in the current video format.
static BOOL uvcUncomGetFrameBitRate(struct usb_video_format_header* pFormatHdr, int frmIndex,
	int* pMinBitRate, int* pMaxBitRate)
{
	__UVC_UNCOMPRESSED_FORMAT* pFormat = NULL;
	__UVC_UNCOMPRESSED_FRAME* pFrame = NULL;
	
	if ((NULL == pFormatHdr) || (frmIndex < 1))
	{
		return FALSE;
	}
	if (frmIndex > pFormatHdr->NumOfFrames)
	{
		return FALSE;
	}
	pFormat = pFormatHdr->SpecFormatData;
	if (NULL == pFormat)
	{
		BUG();
	}
	pFrame = &pFormat->pFrameArray[frmIndex - 1];
	if (pMaxBitRate)
	{
		*pMaxBitRate = pFrame->maxBitRate;
	}
	if (pMinBitRate)
	{
		*pMinBitRate = pFrame->minBitRate;
	}
	return TRUE;
}

//Parse uncompressed format's specific data.It creates a video format object
//and link it into USB video device's format list for each supported uncompressed
//video format.
char* uvcUncompressedParseFormat(char* pCSInterface, __USB_VIDEO_DEVICE* pUvcDev)
{
	struct usb_cs_interface_header* head = (struct usb_cs_interface_header*)pCSInterface;
	char* ptr = pCSInterface;
	char numOfFrames = 0;
	char fmtIndex = 0;
	int parsedLength = 0;
	struct usb_video_format_header* fmtHdr = NULL;
	__UVC_UNCOMPRESSED_FORMAT* pFormat = NULL;
	__UVC_UNCOMPRESSED_FRAME* pFrame = NULL;
	int allocSize = 0, i = 0;
	BOOL bResult = FALSE;

	if ((NULL == pCSInterface) || (NULL == pUvcDev))
	{
		goto __TERMINAL;
	}
	if (head->bSubtype != VS_FORMAT_UNCOMPRESSED)
	{
		goto __TERMINAL;
	}
	parsedLength = head->bLength;

	ptr += 3;
	fmtIndex = *ptr;
	//_hx_printf("FmtIndex = %d.", *ptr);
	ptr += 1;
	//_hx_printf("NumOfFrms = %d.", *ptr);
	numOfFrames = *ptr;
	ptr += 1;  //Now ptr points to GUID of the format.
	if (0 == numOfFrames)
	{
		_hx_printf("%s:format with 0 frame desc.\r\n", __func__);
		goto __TERMINAL;
	}
	//Now allocate video format header object and initialize it.
	fmtHdr = _hx_malloc(sizeof(struct usb_video_format_header));
	if (NULL == fmtHdr)
	{
		goto __TERMINAL;
	}
	memset(fmtHdr, 0, sizeof(struct usb_video_format_header));
	fmtHdr->FormatIndex = fmtIndex;
	fmtHdr->NumOfFrames = numOfFrames;
	fmtHdr->currFrameIndex = 1;
	fmtHdr->FormatType = VS_FORMAT_UNCOMPRESSED;
	fmtHdr->szFormatName = GetFormatName(ptr);
	fmtHdr->StartStreaming = uvcUncomStartStreaming;
	fmtHdr->ShowFormatInfo = uvcUncomShowFormatInfo;
	fmtHdr->GetFrameBitRate = uvcUncomGetFrameBitRate;
	
	//Allocate the format specific data,include all frames information.
	allocSize = sizeof(__UVC_UNCOMPRESSED_FORMAT) + sizeof(__UVC_UNCOMPRESSED_FRAME) * numOfFrames;
	pFormat = _hx_malloc(allocSize);
	if (NULL == pFormat)
	{
		goto __TERMINAL;
	}
	memset(pFormat, 0, allocSize);
	pFrame = (__UVC_UNCOMPRESSED_FRAME*)((char*)pFormat + sizeof(__UVC_UNCOMPRESSED_FORMAT));
	pFormat->pFrameArray = pFrame;

	//Link the specific data to format header.
	fmtHdr->SpecFormatData = pFormat;

	//Initialize format and frame data according CS interface descriptors.
	memcpy(pFormat->guidFormat, ptr, 16);
	ptr += 16;
	pFormat->bitsPerPixel = *ptr;
	ptr += 1;
	pFormat->defaultFrameIndex = *ptr;
	ptr += 1;
	pFormat->aspectRatioX = *ptr;
	ptr += 1;
	pFormat->aspectRatioY = *ptr;
	ptr += 1;
	pFormat->interlaceFlags = *ptr;
	ptr += 1;
	pFormat->bCopyProtect = *ptr;
	ptr += 1;  //Now ptr points to the start of all frame descriptors belong to this format.

	//Process frames information.
	for (i = 0; i < numOfFrames; i++)
	{
		head = (struct usb_cs_interface_header*)ptr;
		if (head->bDescriptorType != USB_DT_CS_INTERFACE)
		{
			goto __TERMINAL;
		}
		if (head->bSubtype != VS_FRAME_UNCOMPRESSED)
		{
			goto __TERMINAL;
		}
		ptr += sizeof(struct usb_cs_interface_header);  //Now points to bFrameIndex.
		pFrame[i].frameIndex = *ptr;
		ptr++;
		pFrame[i].capabilities = *ptr;
		ptr++;
		pFrame[i].width = get_unaligned((__u16*)ptr);
		ptr += 2;
		pFrame[i].height = get_unaligned((__u16*)ptr);
		ptr += 2;
		pFrame[i].minBitRate = get_unaligned((__u32*)ptr);
		ptr += 4;
		pFrame[i].maxBitRate = get_unaligned((__u32*)ptr);
		ptr += 4;
		pFrame[i].maxFrameBufferSize = get_unaligned((__u32*)ptr);
		ptr += 4;
		pFrame[i].defaultFrameInterval = get_unaligned((__u32*)ptr);
		ptr += 4;
		pFrame[i].frameIntervalType = *ptr;
		ptr += 1;

		//Update parsed length.
		parsedLength += head->bLength;
		ptr = pCSInterface + parsedLength;
	}
	//Everything should OK,now link the format into USB video device.
	fmtHdr->pNext = pUvcDev->pFormatList;
	pUvcDev->pFormatList = fmtHdr;

	//Everything is in place if reach here.
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		//Release all allocated resources.
		if (fmtHdr)
		{
			_hx_free(fmtHdr);
		}
		if (pFormat)
		{
			_hx_free(pFormat);
		}
		_hx_printf("%s:failed to parse uncompressed data.\r\n", __func__);
	}
	return (pCSInterface + parsedLength);
}
