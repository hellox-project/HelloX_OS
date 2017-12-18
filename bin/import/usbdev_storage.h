//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 04,2015
//    Module Name               : usbdev_storge.h
//    Module Funciton           : 
//    Description               : USB storage device framework related structures and
//                                definitions are put into this file.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//
//***********************************************************************/

#ifndef __USBDEV_STORAGE_H__
#define __USBDEV_STORAGE_H__

//Only available when USB storage supporting is enabled.
//#ifdef CONFIG_USB_STORAGE

//Default sector size.
#ifndef USB_STORAGE_SECTOR_SIZE
#define USB_STORAGE_SECTOR_SIZE 512
#endif

//Main entry point of USB Storage device driver.
BOOL USBStorage_DriverEntry(__DRIVER_OBJECT* lpDrvObj);

//A structure to tracking the current state of a USB storage devices,it's the
//device extension of PHYSICAL DEVICE object.
typedef struct tag__RAW_USB_STORAGE{
	ULONG lBlockNum;      //How many blocks.
	ULONG lBlockSz;       //Block(or sector) size.
	ULONG lCurrentBlock;  //Current position of accessing pointer.
	DWORD dwFlags;        //Flags of the USB storage.
	int   nDeviceIndex;   //USB device index in global block_dev_desc_t array.
}__RAW_USB_STORAGE;

//#endif //CONFIG_USB_STORAGE

#endif  //__USBDEV_STORAGE_H__
