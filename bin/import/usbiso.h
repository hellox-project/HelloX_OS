//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 26, 2016
//    Module Name               : usbiso.h
//    Module Funciton           : 
//                                Header file of USB isochronous transfer related
//                                functions.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __USBISO_H__
#define __USBISO_H__

//Switch of isochronous xfer debugging.
//#define __DEBUG_USB_ISO

//Maximal bandwidth of EHCI ISO transfer for one pipe,in bps.
//One iTD can bearer 24K maximally,there are 1024 slots,and 8 bits for
//one octect,so totally the following.
#define EHCI_ISO_MAX_BANDWIDTH (24 * 1024 * 1024 * 8)

//USB isochronous xfer buffer corresponding to one USB (micro)frame,describes
//the frame or microframe's data buffer.
typedef struct tag__USB_ISO_XFER_BUFFER_DESC{
	char* pBuffer;    //Start address of the data buffer.
	int buffLength;   //Buffer's length.
	int xfer_length;  //Actual data size contained in the buffer.
}__USB_ISO_XFER_BUFFER_DESC;

//Object used to describe one USB isochronous transfer.
typedef struct tag__USB_ISO_DESCRIPTOR{
	struct tag__USB_ISO_DESCRIPTOR* pNext;    //Next element in link.
	HANDLE hEvent;  //Event object the query thread pending on.
	__PHYSICAL_DEVICE* pPhyDev;  //USB device object bearing the xfer.
	__COMMON_USB_CONTROLLER* pCtrl;  //The controller handling this req.
	int bandwidth;    //Bandwidth of the xfer,in bps.
	char* buffer;     //The data buffer.
	int bufflength;   //Length of the data buffer.
	volatile int xfersize;     //Actually transfered size.
	__USB_ISO_XFER_BUFFER_DESC FrameBuffer[8];
	__U8 endpoint;    //Endpoint.
	__U8 multi;       //How many transactions per (micro)frame.
	__U16 maxPacketSize; //Maximal packet size of the endpoint.
	int direction;    //Transfer direction.
#define USB_TRANSFER_DIR_IN  0
#define USB_TRANSFER_DIR_OUT 1
	volatile __U32 status; //Current status of the descriptor.
#define USB_ISODESC_STATUS_INITIALIZED 0x01
#define USB_ISODESC_STATUS_TIMEOUT     0x02
#define USB_ISODESC_STATUS_ERROR       0x04
#define USB_ISODESC_STATUS_INPROCESS   0x08
#define USB_ISODESC_STATUS_CANCELED    0x10
#define USB_ISODESC_STATUS_COMPLETED   0x20
	struct iTD* itdArray;  //Array that contains the iTD descriptor's base address.
	int itdnumber;         //How many iTD in iTD array.
	int slot_num;          //How many periodic list slot the iTD occupied.
	BOOL(*ISOXferIntHandler)(struct tag__USB_ISO_DESCRIPTOR* pIsoDesc);  //Interrupt handler.
}__USB_ISO_DESCRIPTOR;

//Create a USB isochronous transfer descriptor and install it into system.
__USB_ISO_DESCRIPTOR* usbCreateISODescriptor(__PHYSICAL_DEVICE* pPhyDev, int direction, int bandwidth,
	char* buffer, int bufflength, __U8 endpoint,__U16 maxPacketSize,__U8 multi);

//Start USB isochronous transfer.
BOOL usbStartISOXfer(__USB_ISO_DESCRIPTOR* pIsoDesc);

//Stop USB isochronous transfer.
BOOL usbStopISOXfer(__USB_ISO_DESCRIPTOR* pIsoDesc);

//Destroy USB isochronous transfer descriptor.
BOOL usbDestroyISODescriptor(__USB_ISO_DESCRIPTOR* pIsoDesc);

#endif //__USBISO_H__
