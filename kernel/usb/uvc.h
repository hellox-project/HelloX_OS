//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 12, 2016
//    Module Name               : uvc.h
//    Module Funciton           : 
//                                Header file of USB Video Class function.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __UVC_H__
#define __UVC_H__

//IDs of Video Class.
#define CC_VIDEO             0x0E
#define SC_VIDEOCONTROL      0x01
#define SC_VIDEOSTREAMING    0x02
#define SC_VIDEO_INTERFACE_COLLECTION 0x03

#define CS_UNDEFINED         0x20
#define CS_DEVICE            0x21
#define CS_CONFIGURATION     0x22
#define CS_STRING            0x23
#define CS_INTERFACE         0x24
#define CS_ENDPOINT          0x25

//Request codes,from UVC Spec 1.5 document.
#define RC_UNDEFINED 0x00
#define SET_CUR 0x01
#define SET_CUR_ALL 0x11
#define GET_CUR 0x81
#define GET_MIN 0x82
#define GET_MAX 0x83
#define GET_RES 0x84
#define GET_LEN 0x85
#define GET_INFO 0x86
#define GET_DEF 0x87
#define GET_CUR_ALL 0x91
#define GET_MIN_ALL 0x92
#define GET_MAX_ALL 0x93
#define GET_RES_ALL 0x94
#define GET_DEF_ALL 0x97

/* Control selectors for different units
   and terminals,from UVC spec 1.5. */

//VideoControl interface control selectors.
#define VC_CONTROL_UNDEFINED          0x00
#define VC_VIDEO_POWER_MODE_CONTROL   0x01
#define VC_REQUEST_ERROR_CODE_CONTROL 0x02

//Terminal control selector.
#define TE_CONTROL_UNDEFINED          0x00

//Selector unit control selectors.
#define SU_CONTROL_UNDEFINED          0x00
#define SU_INPUT_SELECT_CONTROL       0x01

//Camera terminal control selectors.
#define CT_CONTROL_UNDEFINED 0x00
#define CT_SCANNING_MODE_CONTROL 0x01
#define CT_AE_MODE_CONTROL 0x02
#define CT_AE_PRIORITY_CONTROL 0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL 0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL 0x05
#define CT_FOCUS_ABSOLUTE_CONTROL 0x06
#define CT_FOCUS_RELATIVE_CONTROL 0x07
#define CT_FOCUS_AUTO_CONTROL 0x08
#define CT_IRIS_ABSOLUTE_CONTROL 0x09
#define CT_IRIS_RELATIVE_CONTROL 0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL 0x0B
#define CT_ZOOM_RELATIVE_CONTROL 0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL 0x0D
#define CT_PANTILT_RELATIVE_CONTROL 0x0E
#define CT_ROLL_ABSOLUTE_CONTROL 0x0F
#define CT_ROLL_RELATIVE_CONTROL 0x10
#define CT_PRIVACY_CONTROL 0x11
#define CT_FOCUS_SIMPLE_CONTROL 0x12
#define CT_WINDOW_CONTROL 0x13
#define CT_REGION_OF_INTEREST_CONTROL 0x14

//Processing unit control selectors.
#define PU_CONTROL_UNDEFINED 0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL 0x01
#define PU_BRIGHTNESS_CONTROL 0x02
#define PU_CONTRAST_CONTROL 0x03
#define PU_GAIN_CONTROL 0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL 0x05
#define PU_HUE_CONTROL 0x06
#define PU_SATURATION_CONTROL 0x07
#define PU_SHARPNESS_CONTROL 0x08
#define PU_GAMMA_CONTROL 0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL 0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL 0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL 0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL 0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL 0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL 0x0F
#define PU_HUE_AUTO_CONTROL 0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL 0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL 0x12
#define PU_CONTRAST_AUTO_CONTROL 0x13

//Encoding unit control selectors.
#define EU_CONTROL_UNDEFINED 0x00
#define EU_SELECT_LAYER_CONTROL 0x01
#define EU_PROFILE_TOOLSET_CONTROL 0x02
#define EU_VIDEO_RESOLUTION_CONTROL 0x03
#define EU_MIN_FRAME_INTERVAL_CONTROL 0x04
#define EU_SLICE_MODE_CONTROL 0x05
#define EU_RATE_CONTROL_MODE_CONTROL 0x06
#define EU_AVERAGE_BITRATE_CONTROL 0x07
#define EU_CPB_SIZE_CONTROL 0x08
#define EU_PEAK_BIT_RATE_CONTROL 0x09
#define EU_QUANTIZATION_PARAMS_CONTROL 0x0A
#define EU_SYNC_REF_FRAME_CONTROL 0x0B
#define EU_LTR_BUFFER_ CONTROL 0x0C
#define EU_LTR_PICTURE_CONTROL 0x0D
#define EU_LTR_VALIDATION_CONTROL 0x0E
#define EU_LEVEL_IDC_LIMIT_CONTROL 0x0F
#define EU_SEI_PAYLOADTYPE_CONTROL 0x10
#define EU_QP_RANGE_CONTROL 0x11
#define EU_PRIORITY_CONTROL 0x12
#define EU_START_OR_STOP_LAYER_CONTROL 0x13
#define EU_ERROR_RESILIENCY_CONTROL 0x14

//Extension unit control selectors.
#define XU_CONTROL_UNDEFINED 0x00

//VideoStreaming interface control selectors.
#define VS_CONTROL_UNDEFINED 0x00
#define VS_PROBE_CONTROL 0x01
#define VS_COMMIT_CONTROL 0x02
#define VS_STILL_PROBE_CONTROL 0x03
#define VS_STILL_COMMIT_CONTROL 0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL 0x05
#define VS_STREAM_ERROR_CODE_CONTROL 0x06
#define VS_GENERATE_KEY_FRAME_CONTROL 0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL 0x08
#define VS_SYNCH_DELAY_CONTROL 0x09

//Video format and frame types.
#define VS_FORMAT_UNCOMPRESSED   0x04
#define VS_FRAME_UNCOMPRESSED    0x05
#define VS_FORMAT_MJPEG          0x06
#define VS_FRAME_MJPEG           0x07
#define VS_FORMAT_MPEG2TS        0x0a
#define VS_FORMAT_DV             0x0c
#define VS_COLORFORMAT           0x0d
#define VS_FORMAT_FRAME_BASED    0x10
#define VS_FRAME_FRAME_BASED     0x11
#define VS_FORMAT_STREAM_BASED   0x12
#define VS_FORMAT_H264           0x13
#define VS_FRAME_H264            0x14
#define VS_FORMAT_H264_SIMULCAST 0x15
#define VS_FORMAT_VP8            0x16
#define VS_FRAME_VP8             0x17
#define VS_FORMAT_VP8_SIMULCAST  0x18

//Request types.
#define USB_CTRLREQ_TYPE_CS  (1 << 5)  //Class specific req.
#define USB_CTRLREQ_TYPE_INT 1         //Direct to interface.
#define USB_CTRLREQ_TYPE_EP  2         //Direct to endpoint.

/* Video Probe and Commit controls. */
#ifdef __MS_VC__
#pragma pack(push,1)
struct uvc_video_probe_commit {
	__U16 bmHint;
	__U8 bFormatIndex;
	__U8 bFrameIndex;
	__U32 dwFrameInterval;
	__U16 wKeyFrameRate;
	__U16 wPFrameRate;
	__U16 wCompQuality;
	__U16 wCompWindowSize;
	__U16 wDelay;
	__U32 dwMaxVideoFrameSize;
	__U32 dwMaxPayloadTransferSize;
	__U32 dwClockFrequency;
	__U8 bmFramingInfo;
	__U8 bPreferedVersion;
	__U8 bMinVersion;
	__U8 bMaxVersion;
	__U8 bUsage;
	__U8 bBitDepthLuma;
	__U8 bmSettings;
	__U8 bMaxNumberOfRefFramesPlus1;
	__U16 bmRateControlModes;
	__U8 Number[8];
};
#pragma pack(pop)
#else
struct uvc_video_probe_commit {
	__U16 bmHint;
	__U8 bFormatIndex;
	__U8 bFrameIndex;
	__U32 dwFrameInterval;
	__U16 wKeyFrameRate;
	__U16 wPFrameRate;
	__U16 wCompQuality;
	__U16 wCompWindowSize;
	__U16 wDelay;
	__U32 dwMaxVideoFrameSize;
	__U32 dwMaxPayloadTransferSize;
	__U32 dwClockFrequency;
	__U8 bmFramingInfo;
	__U8 bPreferedVersion;
	__U8 bMinVersion;
	__U8 bMaxVersion;
	__U8 bUsage;
	__U8 bBitDepthLuma;
	__U8 bmSettings;
	__U8 bMaxNumberOfRefFramesPlus1;
	__U16 bmRateControlModes;
	__U8 Number[8];
} __attribute__((packed));
#endif

/* Assume the second interface in one interface association is for streaming,
   it's arbitary but in most case can work,just for simplicity... */
#define UVC_DEFAULT_STREAM_INT  1

/* Assume the first interface in one interface association is for control. */
#define UVC_DEFAULT_CONTROL_INT 0

//Pre-definitions.
struct usb_video_format_header;

/* USB Video device object,describes one UVC device in HelloX OS. */
typedef struct tag__USB_VIDEO_DEVICE{
	struct tag__USB_VIDEO_DEVICE*   pNext;
	__PHYSICAL_DEVICE*              pPhyDev;
	struct usb_device*              pUsbDev;
	__USB_INTERFACE_ASSOCIATION*    pIntAssoc;
	struct usb_video_format_header* pFormatList;       //List holds all supported formats.
	__U32                           recvCtrlPipe;      //Control pipe of UVC device,for receiving.
	__U32                           sendCtrlPipe;      //Send control pipe.
	__U32                           recvStreamPipe;    //Streaming pipe of UVC device,for receiving.
	__U32                           sendStreamPipe;    //Send streaming pipe.
	__S8                            ctrlIntNum;        //Interface number of control.
	__S8                            streamIntNum;      //Streaming interface number.
	int                             alternates;        //Alternate setting of current streaming.
	int                             streamBandwidth;   //Current streaming bandwidth.
	__U32                           maxXferSize;       //Current streaming's maximal transfer size.
	char*                           streamBuff;        //Streaming buffer.
	__USB_ISO_DESCRIPTOR*           isoDesc;           //Isochronous transfer descriptor for streaming.

	//State for current streaming setting.
	int                             frameSize;
	int                             sampleSize;
	__U8                            currFormat;       //Current format index.
	__U8                            currFrame;        //Current frame index belong to current format.
	struct usb_video_format_header* pCurrentFormat;   //Format for current streaming's setting.

	//Probe and commit structure to record negotiated parameters.
	struct uvc_video_probe_commit   uvpc;
}__USB_VIDEO_DEVICE;

//Generic video format object header that describes the common part of all USB
//video formats.
struct usb_video_format_header{
	struct usb_video_format_header* pNext;
	__u8 FormatType;
	__u8 FormatIndex;      //Index value in the whole format array of a UVC device.
	__u8 NumOfFrames;      //How many frame descriptors followed.
	__u8 currFrameIndex;   //Frame index of current streaming setting.
	char* szFormatName;    //Format name string.
	void* SpecFormatData;  //Private data for a specific format.

	//Operations specific to this format.
	BOOL (*StartStreaming)(__USB_VIDEO_DEVICE* pUvcDev, struct usb_video_format_header* format,
		char* frameBuff, int size, int* sizeReturn);
	VOID (*ShowFormatInfo)(struct usb_video_format_header* pFormatHdr);
	BOOL (*GetFrameBitRate)(struct usb_video_format_header* pFormatHdr, int frmIndex,
		int* pMinBitRate, int* pMaxBitRate);
};

/* Array element that corresponding to each supported video format. UVC subsystem will
   call the corresponding routines to parse format data according to format type. */
typedef struct tag__UVC_FORMAT_ARRAY_ELEMENT{
	__u8 formatType;
	char* (*ParseFormat)(char* pCSInterface,__USB_VIDEO_DEVICE* pUvcDev);
}__UVC_FORMAT_ARRAY_ELEMENT;

/* Pro-types of UVC low level operation routine. */
BOOL uvcQueryInterfaceControl(__USB_VIDEO_DEVICE* pUvcDev, __u8 request,
	__u8 ifnum, __u8 cs, void* data,__u16 size, int timeout);
BOOL uvcQueryUnitControl(__USB_VIDEO_DEVICE* pUvcDev, __u8 request,
	__u8 entity_id, __u8 ifnum, __u8 cs, void* data,__u16 size, int timeout);
__U8 uvcGetDeviceError(__USB_VIDEO_DEVICE* pUvcDev,__u8 ifnum);
__U8 uvcGetStreamError(__USB_VIDEO_DEVICE* pUvcDev, __u8 ifnum);

/* Streaming interface parameters negotiation. */
BOOL uvcStreamNegotiation(__USB_VIDEO_DEVICE* pUvcDev, __U16 bmHint,__U8 FormatIndex,__U8 FrameIndex,
	int bandwidth,struct uvc_video_probe_commit* pUVPC);

/* Obtain all supported UVC video formats and their frames by parsing class specific
   information. */
BOOL uvcDecodeFormat(__USB_VIDEO_DEVICE* pUvcDev);

/* Prepare video streaming on video stream interface,the pUVPC parameter is from
   uvcStreamNegotiation routine,please do not modify it manually. */
BOOL uvcPrepareStreaming(__USB_VIDEO_DEVICE* pUvcDev, int bandwidth, struct uvc_video_probe_commit* pUVPC);

/* Start streaming,prepare streaming should be applied before this routine. */
BOOL uvcStartStreaming(__USB_VIDEO_DEVICE* pUvcDev, char* frameBuff, int buffSize,int* sizeReturned);

/* Stop streaming on a video interface. */
BOOL uvcStopStreaming(__USB_VIDEO_DEVICE* pUvcDev);

/* Global manager object to manage all USB video related functions and structures. */
typedef struct tag__USB_VIDEO_MANAGER{
	__USB_VIDEO_DEVICE* pUsbVideoList;
	//Operations.
	BOOL (*RegisterVideoDevice)(__USB_VIDEO_DEVICE* pNewDevice);
}__USB_VIDEO_MANAGER;

extern __USB_VIDEO_MANAGER UsbVideoManager;

//Entry point of UVC driver.
#if defined(__CFG_DRV_UVC) && defined(__CFG_SYS_USB)
BOOL UVC_DriverEntry(__DRIVER_OBJECT*);
#endif //__CFG_DRV_UVC && __CFG_SYS_USB

#endif  //__UVC_H__
