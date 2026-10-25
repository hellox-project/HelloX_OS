//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 03, 2025
//    Module Name               : xhciint.c
//    Module Funciton           : 
//                                USB xHCI Controller's interrupt handler and
//                                related functions are put into this file.
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
#include "xhci.h"
#include "usbiso.h"
#include "usbasync.h"

/*
 * General entry of xHCI controller
 * interrupt handler.
 */
unsigned long xHCIIntHandler(LPVOID pParam)
{
	return 1;
}
