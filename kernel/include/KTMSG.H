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

//
//These 2 messages are defined in early version of 
//Hello China and will be obsoleted later.
//
#define KERNEL_MESSAGE_TERMINAL     0x0005
#define KERNEL_MESSAGE_TIMER        0x0006

//Message command defination,please note the message
//value from 0x0001 to 0x00FF are reserved by kernel,
//user customized message can start from 0x0100.
#define MSG_KEY_DOWN            0x0001
#define MSG_KEY_UP              0x0002
#define MSG_LEFT_MOUSE_UP       0x0003
#define MSG_MIDD_MOUSE_UP       0x0004
#define MSG_RIGHT_MOUSE_UP      0x0005
#define MSG_LEFT_MOUSE_DOWN     0x0006
#define MSG_MIDD_MOUSE_DOWN     0x0007
#define MSG_RIGHT_MOUSE_DOWN    0x0008

#define MSG_SYS_TERMINAL        0x0009  //If OS want to task terminal,
                                        //it sends this message to the task.
#define MSG_TIMER               0x000a  //Time message.

#define MSG_VK_KEY_DOWN         0x0010  //
#define MSG_VK_KEY_UP           0x0011  //

//User defined messages can be append here,please note
//the message value should begin from 0x0100.
#define MSG_USER_START          0x0100

#ifdef __cplusplus
}
#endif

#endif //ktmsg.h
