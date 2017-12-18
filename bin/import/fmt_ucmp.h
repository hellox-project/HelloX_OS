//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 12, 2016
//    Module Name               : fmt_ucmp.h
//    Module Funciton           : 
//                                Header file of USB Video format uncompressed.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __FMT_UCMP_H__
#define __FMT_UCMP_H__

#ifndef VS_FORMAT_UNCOMPRESSED
#define VS_FORMAT_UNCOMPRESSED   0x04
#endif

#ifndef VS_FRAME_UNCOMPRESSED
#define VS_FRAME_UNCOMPRESSED    0x05
#endif

#define VS_FORMAT_UNCOMPRESSED_NAME_YUY2 "UNCOM-YUY2"
#define VS_FORMAT_UNCOMPRESSED_NAME_I420 "UNCOM-I420"
#define VS_FORMAT_UNCOMPRESSED_NAME_NV12 "UNCOM-NV12"
#define VS_FORMAT_UNCOMPRESSED_NAME_M420 "UNCOM-M420"

//Parse format related data of uncompressed.
char* uvcUncompressedParseFormat(char* pCSInterface, __USB_VIDEO_DEVICE* pUvcDev);

//Structure to describes the uncompressed video frame.
typedef struct tag__UVC_UNCOMPRESSED_FRAME{
	__u8 frameIndex;         //Index of this frame in the frame array specified to a format.
	__u8 capabilities;       //Capabilities bits of the frame.
	__u16 width;
	__u16 height;
	__u32 minBitRate;        //Min bit rate in bps.
	__u32 maxBitRate;        //Max bit rate in bps.
	__u32 maxFrameBufferSize;
	__u32 defaultFrameInterval;
	__u8 frameIntervalType;  //Type of the frame interval,continuous or discrete.
}__UVC_UNCOMPRESSED_FRAME;

//Structure to describes the uncompressed video format.
typedef struct tag__UVC_UNCOMPRESSED_FORMAT{
	__u8 guidFormat[16];     //Format's GUID.
	__u8 bitsPerPixel;       //Bits per pixel in decoded video frame.
	__u8 defaultFrameIndex;  //Default frame index.
	__u8 aspectRatioX;       //X dimension of the picture aspect ratio.
	__u8 aspectRatioY;       //Y dimension of the picture aspect ratio.
	__u8 interlaceFlags;     //A bitmap of flags to indicate the interlace attributes.
	__u8 bCopyProtect;       //Specifiy weather copying is restricted.
	__UVC_UNCOMPRESSED_FRAME*   pFrameArray;
}__UVC_UNCOMPRESSED_FORMAT;

/* Stream header for uncompressed format streaming data flow. */
#ifdef __MS_VC__
#pragma pack(push,1)
struct uvc_uncompressed_stream_hdr {
	__u8 hdr_length;
	__u8 bit_field_hdr;
	__u8 pts[4];
	__u8 scr[6];
};
#pragma pack(pop)
#else
struct uvc_uncompressed_stream_hdr {
	__u8 hdr_length;
	__u8 bit_field_hdr;
	__u8 pts[4];
	__u8 scr[6];
} __attribute__((packed));
#endif

//Bits meaning of bit field header.
#define UVC_UNCOM_BFH_PID (1 << 0)
#define UVC_UNCOM_BFH_EOF (1 << 1)
#define UVC_UNCOM_BFH_PTS (1 << 2)
#define UVC_UNCOM_BFH_SCR (1 << 3)
#define UVC_UNCOM_BFH_RES (1 << 4)
#define UVC_UNCOM_BFH_STI (1 << 5)
#define UVC_UNCOM_BFH_ERR (1 << 6)
#define UVC_UNCOM_BFH_EOH (1 << 7)

#endif  //__FMT_UCMP_H__
