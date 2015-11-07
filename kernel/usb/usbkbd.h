//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 28,2015
//    Module Name               : usbkeyboard.h
//    Module Funciton           : 
//    Description               : USB Keyboard driver definitions are put into this file.
//    Last modified Author      :
//    Last modified Date        : 26 JAN,2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//
//***********************************************************************/

#ifndef __USBKBD_H__
#define __USBKBD_H__

//Class IDs of USB keyboard.
#define UK_INTERFACE_CLASS_ID         0x03
#define UK_INTERFACE_SUBCLASS_ID      0x01
#define UK_INTERFACE_PROTOCOL_ID      0x01

//USB data buffer length.
#define MAX_USBKEYBOARD_BUFF_LEN       8

//Entry point of USB keyboard driver.
BOOL USBKeyboard_DriverEntry(__DRIVER_OBJECT* lpDrvObj);

#endif  //__USBKBD_H__
