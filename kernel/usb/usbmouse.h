//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2015
//    Module Name               : usbmouse.h
//    Module Funciton           : 
//    Description               : USB mouse driver definitions are put into this file.
//    Last modified Author      :
//    Last modified Date        : 26 JAN,2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//
//***********************************************************************/

#ifndef __USBMOUSE_H__
#define __USBMOUSE_H__

//Class IDs of USB mouse.
#define UM_INTERFACE_CLASS_ID         0x03
#define UM_INTERFACE_SUBCLASS_ID      0x01
#define UM_INTERFACE_PROTOCOL_ID      0x02

//USB data buffer length.
#define MAX_USBMOUSE_BUFF_LEN 4

//Entry point of USB mouse driver.
BOOL USBMouse_DriverEntry(__DRIVER_OBJECT* lpDrvObj);

#endif  //__USBMOUSE_H__
