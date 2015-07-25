//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,15 2007
//    Module Name               : SYSD_S.H
//    Module Funciton           : 
//                                This module countains HYPERTRM application's definitions.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HYPERTRM_H__
#define __HYPERTRM_H__

#define COM1_BASE 0x3F8
#define COM2_BASE 0x2F8

#define COM1_INT_VECTOR 0x24  //COM1's interrupt vector.
#define COM2_INT_VECTOR 0x23  //COM2's interrupt vector.

#define QUIT_CHARACTER 'z'  //If you press this key,the hypertrm application will
                            //terminate.
#define MAX_SEND_TIMES 4    //Try to call ComSendByte routine for these times,if all
                            //failed,then give up to send.
#define MAX_RECV_TIMES 4   //Try to call ComRecvByte routine to receive byte,if failed
                            //for these times,then check the event to see if
                            //the current thread need terminate.
#define MAX_SEND_WAIT 2000  //Wait for this time(in millionsecond) if the COM interface
                            //is not ready for transmit in interrupt mode.
#define MAX_RECV_WAIT 100   //Wait for this time(in millionsecond) if the COM interface
                            //is not ready for receive in interrupt mode.

DWORD Hypertrm(LPVOID);     //Main entry for the HYPERTRM application.
DWORD Hyptrm2(LPVOID);      //Main entry for Hyptrm2 application.

#endif
