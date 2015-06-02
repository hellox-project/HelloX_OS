//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 21,2009
//    Module Name               : KERMOD.CPP
//    Module Funciton           : 
//                                This module contains all kernel module(s)
//                                implemented in static mode.
//                                All these modules are initialized in process
//                                of system loading.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "modmgr.h"

#define ADD_STATIC_MODULE(baseaddr) {baseaddr,(__MODULE_INIT)baseaddr},

__KERNEL_MODULE KernelModule[] = {
	//ADD_STATIC_MODULE(0x00130000)    //For GUI module.
	//ADD_STATIC_MODULE(0x00150000)    //For NETWORK module.
	{0,NULL},
};

