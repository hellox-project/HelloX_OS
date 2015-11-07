//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28, 2009
//    Module Name               : MOUSE.H
//    Module Funciton           : 
//                                This module countains the pre-definition of 
//                                MOUSE device driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MOUSE_H__
#define __MOUSE_H__

#define MOUSE_INT_VECTOR INTERRUPT_VECTOR_MOUSE    //Vector of MOUSE device.

//Entry point of MOUSE driver.
BOOL MouseDrvEntry(__DRIVER_OBJECT* lpDriverObject);

//IO ports for i8042 chip.
#define MOUSE_DATA_PORT 0x60
#define MOUSE_CTRL_PORT 0x64

//Maximal X scale and Y scale.
#define MAX_X_SCALE  511
#define MAX_Y_SCALE  511

#endif  //__MOUSE_H__
