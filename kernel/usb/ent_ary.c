//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 06, 2015
//    Module Name               : ent_ary.c
//    Module Funciton           : 
//                                Entry point array of each USB controller driver
//                                in system.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "usb_defs.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"

//Low level initialization and stop operations for each defined controller.
//If we put these routines declaration in it's corresponding header file,
//compiling warnings will appear since mutual macros are defined in different 
//controller drivere's header file .
#ifdef CONFIG_USB_OHCI
int _ohci_usb_lowlevel_init(int index, enum usb_init_type init, void **controller);
int _ohci_usb_lowlevel_stop(int index);
#endif

#ifdef CONFIG_USB_EHCI
int _ehci_usb_lowlevel_init(int index, enum usb_init_type init, void **controller);
int _ehci_usb_lowlevel_stop(int index);
#endif

#ifdef CONFIG_USB_XHCI
int _xhci_usb_lowlevel_init(int index, enum usb_init_type init, void **controller);
int _xhci_usb_lowlevel_stop(int index);
#endif

//Entry point array of each USB controller's driver.Please add one entry in this
//array for the newly added driver.
__USB_CTRL_DRIVER_ENTRY UsbDriverEntry[] = {
#ifdef CONFIG_USB_XHCI
	{ _xhci_usb_lowlevel_init, _xhci_usb_lowlevel_stop },
#endif

#ifdef CONFIG_USB_EHCI
	{ _ehci_usb_lowlevel_init, _ehci_usb_lowlevel_stop },
#endif

#ifdef CONFIG_USB_OHCI
	{ _ohci_usb_lowlevel_init, _ohci_usb_lowlevel_stop },
#endif
	//The last entry must keep NULL to be a sign of array's end.
	{ NULL, NULL }
};
