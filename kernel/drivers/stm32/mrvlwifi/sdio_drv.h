//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 2,2014
//    Module Name               : sdio_drv.h
//    Module Funciton           : 
//                                Header file for SDIO driver. It's a regular scheme to put
//                                driver entry routine's definition and device vector's definition
//                                into header file,and then include this header file in
//                                HelloX kernel's drventry.c file,where driver initialization code
//                                resides.
//
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __SDIO_DRV_H__
#define __SDIO_DRV_H__

//Entry point of SDIO driver.
BOOL SDIODriverEntry(__DRIVER_OBJECT* lpDriverObject);

//Interrupt vector number of SDIO.
#define SDIO_INT_VECTOR   65

//Default device name of SDIO.
#define SDIO_DEV_NAME     "\\\\.\\SDIO_DEV"  //Prefixed with '\\.\',as required by HelloX OS.

#endif //__SDIO_DRV_H__
