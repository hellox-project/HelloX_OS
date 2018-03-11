//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 21, 2017
//    Module Name               : usbasync.h
//    Module Funciton           : 
//                                Header file of USB asynchronous transfer related
//                                functions.The USB asynchronous xfer include control
//                                transfer and bulk transfer.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __USBASYNC_H__
#define __USBASYNC_H__

//Switch of USB aysnchronous xfer debugging.
//#define __DEBUG_USB_ASYNC

/*
 * Maximal data length of asynchronous xfer,in bytes.
 */
#define USB_ASYNC_MAX_LENGTH (1024 * 64)  //16K bytes.

//Queue head struct,defined in ehci.h file.
struct QH;
//Queue tx descriptor,defined in ehci.h file.
struct qTD;

//Object used to describe one USB asynchronous transfer.
typedef struct tag__USB_ASYNC_DESCRIPTOR{
	struct tag__USB_ASYNC_DESCRIPTOR* pNext;    //Next element in link.
	HANDLE hEvent;  //Event object the query thread pending on.
	//__PHYSICAL_DEVICE* pPhyDev;      //USB device object bearing the xfer.
	struct usb_device* pUsbDev;      //Actual USB device that can be used directly.
	__COMMON_USB_CONTROLLER* pCtrl;  //The controller handling this req.
	HANDLE hOwnerThread;             //Thread handle of the descriptor owner.
	char* buffer;     //The data buffer.
	int bufflength;   //Length of the data buffer,must not exceed USB_ASYNC_MAX_LENGTH.
	int reqlength;    //Request length of xfer,can not exceed bufflength.
	volatile int xfersize;     //Actually transfered size.
	//__U8 endpoint;    //Endpoint.
	//__U8 multi;       //How many transactions per (micro)frame.
	//__U16 maxPacketSize; //Maximal packet size of the endpoint.
	__U32 pipe;
	//int direction;    //Transfer direction.
#ifndef USB_TRANSFER_DIR_IN
#define USB_TRANSFER_DIR_IN  0
#endif
#ifndef USB_TRANSFER_DIR_OUT
#define USB_TRANSFER_DIR_OUT 1
#endif
	volatile __U32 status; //Current status of the descriptor.
#define USB_ASYNCDESC_STATUS_INITIALIZED 0x01
#define USB_ASYNCDESC_STATUS_TIMEOUT     0x02
#define USB_ASYNCDESC_STATUS_ERROR       0x04
#define USB_ASYNCDESC_STATUS_INPROCESS   0x08
#define USB_ASYNCDESC_STATUS_CANCELED    0x10
#define USB_ASYNCDESC_STATUS_COMPLETED   0x20

	__U32 err_code;       //Error code if status = ERROR.

	struct QH* QueueHead;    //Queue head struct.
	struct devrequest* req;        //Request struct for control req.
	struct qTD* pFirstTD;          //Points to first qTD in qTD array.
	volatile struct qTD* pLastTD;  //Points to last qTD in qTD array.
	int num_qtd;                   //qTD number in this descriptor.
	int qtd_counter;               //qTD counter actually used.
	BOOL(*AsyncXferIntHandler)(struct tag__USB_ASYNC_DESCRIPTOR* pAsyncDesc);  //Interrupt handler.
}__USB_ASYNC_DESCRIPTOR;

//Create a USB asychronous transfer descriptor and install it into system.
__USB_ASYNC_DESCRIPTOR* usbCreateAsyncDescriptor(struct usb_device* pUsbDev, unsigned int pipe, 
	char* buffer, int bufflength, struct devrequest* req);

//Start USB asynchronous transfer.
BOOL usbStartAsyncXfer(__USB_ASYNC_DESCRIPTOR* pAsyncDesc,int new_size);

//Stop USB asynchronous transfer.
BOOL usbStopAsyncXfer(__USB_ASYNC_DESCRIPTOR* pAsyncDesc);

//Destroy USB asynchronous transfer descriptor.
BOOL usbDestroyAsyncDescriptor(__USB_ASYNC_DESCRIPTOR* pAsyncDesc);

#endif //__USBASYNC_H__
