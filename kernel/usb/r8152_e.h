//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apri 8,2017
//    Module Name               : r8152_e.h
//    Module Funciton           : 
//                                Header files for Realtek 8152 chipset.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __R8152_E_H__
#define __R8152_E_H__

//R8152 ethernet interface's name,common part,should not
//exceed 30 characters.
#define R8152_NAME_BASE "r8152_eth_if_"

//Dedicated rx/tx thread name base,should not exceed
//32 characters.
#define R8152_RECV_NAME_BASE "r8152_rx_"
#define R8152_SEND_NAME_BASE "r8152_tx_"
#define R8152_DAMON_NAME_BASE "r8152_ctrl_"

//Sending message to driven the txrx thread to send.
//Please be noted that this value should not conflict with system
//pre-defined message values.
#define R8152_SEND_FRAME 419

//Entry point of R8152 driver.
BOOL R8152_DriverEntry(__DRIVER_OBJECT* lpDrvObj);

#endif //__R8152_E_H__
