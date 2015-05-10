//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Jan 6,2015
//    Module Name               : Ethernet.h
//    Module Funciton           : 
//                                This module countains  ethernet 29j60
//                                driver's low level function definitions.
//                                The hardware driver only need implement several
//                                low level routines and link them into HelloX's
//                                ethernet skeleton.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ETHERNET_H__
#define __ETHERNET_H__

//Ethernet interface's name,please be aware that no space character is allowed in name
//string since HelloX's shell can not support string contains space.
#define ETHERNET_NAME "ETHERNET_28J60"

BOOL Ethernet_Initialize(LPVOID pData);

#endif //__ETHERNET_H__
