//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 2004-07-05
//    Module Name               : ktmsg.h
//    Module Funciton           : 
//                                All kernel messages are defined in this file.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KTMSG_H__
#define __KTMSG_H__

#ifdef __cplusplus
extern "C" {
#endif

//Messages used in kernel,please do not modify the value of each
//message.
//The message value from 0x0001 to 0xFF are reserved by system,
//user thread should not use them.

//Terminal message and timer message.
#define KERNEL_MESSAGE_TERMINAL     0x05
#define KERNEL_MESSAGE_TIMER        0x06

//Keyboard messages.
#define KERNEL_MESSAGE_AKEYDOWN       0x01     //ASCII key down.
#define KERNEL_MESSAGE_AKEYUP         0x02     //ASCII key up.
#define KERNEL_MESSAGE_VKEYDOWN       0x10     //Virtual key down.
#define KERNEL_MESSAGE_VKEYUP         0x11     //Virtual key up.

//Mouse messages.
#define KERNEL_MESSAGE_LBUTTONDOWN    0x20     //Left button down.
#define KERNEL_MESSAGE_LBUTTONUP      0x21     //Left button released.
#define KERNEL_MESSAGE_RBUTTONDOWN    0x22     //Right button down.
#define KERNEL_MESSAGE_RBUTTONUP      0x23     //Right button released.
#define KERNEL_MESSAGE_LBUTTONDBCLK   0x24     //Left button double clicked.
#define KERNEL_MESSAGE_RBUTTONDBCLK   0x25     //Right button double clicked.
#define KERNEL_MESSAGE_MOUSEMOVE      0x26     //Mouse is moving.
#define KERNEL_MESSAGE_MBUTTONDOWN    0x27     //Middle button down.
#define KERNEL_MESSAGE_MBUTTONUP      0x28     //Middle button up.
#define KERNEL_MESSAGE_MBUTTONDBCLK   0x29     //Middle button double clicked.

//User defined messages can be append here,please note
//the message value should start from 0x0100.
#define MSG_USER_START          0x0100

#ifdef __cplusplus
}
#endif

#endif //__KTMSG_H__
