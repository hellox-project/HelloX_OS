//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,24 2006
//    Module Name               : EXTCMD.CPP
//    Module Funciton           : 
//                                This module countains Hello China's External command's
//                                implementation.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include "extcmd.h"
#include "hypertrm.h"  //For hypertrm application.
#include "fdisk.h"
#include "fs.h"        //For fs application.
#include "hedit.h"     //For hedit application.
#include "fibonacci.h" //A test application to calculate Fibonacci sequence.
#include "lwip/ip_addr.h"
#include "network.h"   //Network diagnostic application.

__EXTERNAL_COMMAND ExtCmdArray[] = {
	{"fs",NULL,FALSE,fsEntry},
	{"fdisk",NULL,FALSE,fdiskEntry},
	{"hedit",NULL,FALSE,heditEntry},
	{"fibonacci",NULL,FALSE,Fibonacci},
	{"hypertrm",NULL,FALSE,Hypertrm},
	{"hyptrm2",NULL,FALSE,Hyptrm2},

#if defined(__CFG_NET_IPv4) || defined(__CFG_NET_IPv6)
	{"network",NULL,FALSE,networkEntry},
#endif
	//Add your external command/application entry here.
	//{"yourcmd",NULL,FALSE,cmdentry},
	//The last entry of this array must be the following one,
	//to indicate the terminator of this array.
	{NULL,NULL,FALSE,NULL}
};

