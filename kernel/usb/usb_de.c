//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2015
//    Module Name               : usb_de.c
//    Module Funciton           : 
//                                Each USB device drivers should put one entry
//                                to the array UsbDriverEntryArray,so as to be
//                                loaded by system.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>

#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBSTORAGE)
#include "usbdev_storage.h"
#endif

#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBMOUSE)
#include "usbmouse.h"
#endif

#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBKBD)
#include "usbkbd.h"
#endif

//Each USB driver should put one entry routine into this array,
//the USB subsystem will load all these drivers by calling the
//entry point after initialization of USB controllers.

__DRIVER_ENTRY_ARRAY UsbDriverEntryArray[] = {
#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBSTORAGE)
	//USB block storage driver.
	{ USBStorage_DriverEntry, "USB_Storage" },
#endif

#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBMOUSE)
	//USB Mouse driver.
	{ USBMouse_DriverEntry, "USB_Mouse" },
#endif

#if defined(__CFG_SYS_USB) && defined(__CFG_DRV_USBKBD)
	//USB keyboard driver.
	{ USBKeyboard_DriverEntry, "USB_Kbd" },
#endif

	//Terminator of the driver entry array.
	{ NULL, NULL }
};
