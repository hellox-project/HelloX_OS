//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : PCI_DRV.H
//    Module Funciton           : 
//                                This module countains PCI local system bus driver's definition
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PCI_DRV_H__
#define __PCI_DRV_H__

#include "types.h"
#include "devmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//The implementation of PciBusDriver routine.
//This is the entry routine for all system bus drivers,it is called by DeviceManager when
//this object is initializing.
//This routine checks if there is(are) PCI bus(es) in system,if not,returns FALSE,else,scan
//all PCI buses,configure all devices reside on the PCI buses,and returns TRUE.
//
BOOL PciBusDriver(__DEVICE_MANAGER* lpDevMgr);

//
//Constants used by PCI driver.
//
#define CONFIG_REGISTER          0xCF8    //Configure register used to read configure space.
#define DATA_REGISTER            0xCFC    //Data register.

//
//Common  part for all kinds of pre-defined headers.
//
#define PCI_CONFIG_OFFSET_VENDOR        0x00
#define PCI_CONFIG_OFFSET_DEVICE        0x02
#define PCI_CONFIG_OFFSET_COMMAND       0x04
#define PCI_CONFIG_OFFSET_STATUS        0x06
#define PCI_CONFIG_OFFSET_REVISION      0x08
#define PCI_CONFIG_OFFSET_CLASS         0x09
#define PCI_CONFIG_OFFSET_CACHELINESZ   0x0C
#define PCI_CONFIG_OFFSET_LATENCY       0x0D
#define PCI_CONFIG_OFFSET_HDRTYPE       0x0E
#define PCI_CONFIG_OFFSET_BIST          0x0F

//PCI command values.
#define PCI_COMMAND_IO 0x1 /* Enable response in I/O space */
#define PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define PCI_COMMAND_SPECIAL 0x8 /* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE 0x10 /* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE 0x20 /* Enable palette snooping */
#define PCI_COMMAND_PARITY 0x40 /* Enable parity checking */
#define PCI_COMMAND_WAIT 0x80 /* Enable address/data stepping */
#define PCI_COMMAND_SERR 0x100 /* Enable SERR */
#define PCI_COMMAND_FAST_BACK 0x200 /* Enable back-to-back writes */
#define PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

//
//Configure space content for normal PCI device(Header type 0).
//
#define PCI_CONFIG_OFFSET_BASE1         0x10
#define PCI_CONFIG_OFFSET_BASE2         0x14
#define PCI_CONFIG_OFFSET_BASE3         0x18
#define PCI_CONFIG_OFFSET_BASE4         0x1C
#define PCI_CONFIG_OFFSET_BASE5         0x20
#define PCI_CONFIG_OFFSET_BASE6         0x24

#define PCI_CONFIG_OFFSET_CIS           0x28
#define PCI_CONFIG_OFFSET_SUBSYSID      0x2C
#define PCI_CONFIG_OFFSET_SYSID         0x2E
#define PCI_CONFIG_OFFSET_ROMBASE       0x30
#define PCI_CONFIG_OFFSET_CAP           0x34

#define PCI_CONFIG_OFFSET_INTLINE       0x3C
#define PCI_CONFIG_OFFSET_INTPIN        0x3D
#define PCI_CONFIG_OFFSET_MINGNT        0x3E
#define PCI_CONFIG_OFFSET_MAXLAT        0x3F

//
//Configure space content for PCI-PCI bridge(Header type 1).
//
#define PCI_CONFIG_OFFSET_PRIMARY       0x18
#define PCI_CONFIG_OFFSET_SECONDARY     0x19
#define PCI_CONFIG_OFFSET_SUBORDINAGE   0x1A
//#define PCI_CONFIG_OFFSET_LATENCY       0x1B

#define PCI_CONFIG_OFFSET_IOBASE        0x1C
#define PCI_CONFIG_OFFSET_IOLIMIT       0x1D
#define PCI_CONFIG_OFFSET_SECONDSTATUS  0x1E
#define PCI_CONFIG_OFFSET_MEMBASE       0x20
#define PCI_CONFIG_OFFSET_MEMLIMIT      0x22
#define PCI_CONFIG_OFFSET_PREMEMBASE    0x24
#define PCI_CONFIG_OFFSET_PREMEMLIMIT   0x26
#define PCI_CONFIG_OFFSET_PREMEMBASEUP  0X28
#define PCI_CONFIG_OFFSET_PREMEMLIMITUP 0x2C
#define PCI_CONFIG_OFFSET_IOBASEUP      0x30
#define PCI_CONFIG_OFFSET_IOLIMITUP     0x32
#define PCI_CONFIG_OFFSET_BCONTROL      0x3E

/* Alias of PCI command registers. */
#define PCI_VENDOR_ID PCI_CONFIG_OFFSET_VENDOR
#define PCI_DEVICE_ID PCI_CONFIG_OFFSET_DEVICE
#define PCI_COMMAND PCI_CONFIG_OFFSET_COMMAND
#define PCI_STATUS PCI_CONFIG_OFFSET_STATUS
#define PCI_REVISION_ID PCI_CONFIG_OFFSET_REVISION
#define PCI_SUBSYSTEM_VENDOR_ID PCI_CONFIG_OFFSET_SUBSYSID
#define PCI_SUBSYSTEM_ID PCI_CONFIG_OFFSET_SYSID

//
//The following defines used to indicate which part of __IDENTIFIER structure is
//set.Please refer to __IDENTIFIER's definition.
//
#define PCI_IDENTIFIER_MASK_VENDOR      0x01
#define PCI_IDENTIFIER_MASK_DEVICE      0x02
#define PCI_IDENTIFIER_MASK_CLASS       0x04
#define PCI_IDENTIFIER_MASK_HDRTYPE     0x08
#define PCI_IDENTIFIER_MASK_ALL         (0x01 | 0x02 | 0x04 | 0x08)

//
//Define a structure to contain PCI device specific information.
//
typedef struct{
	DWORD             dwDeviceType;
	DWORD             DeviceNum   : 5;
	DWORD             FunctionNum : 3;
	DWORD             dwClassCode;
	UCHAR             ucPrimary;     //For PCI-PCI bridge only.
	UCHAR             ucSecondary;   //For PCI-PCI bridge only.
	UCHAR             ucSubordinate; //For PCI-PCI bridge only.
} __PCI_DEVICE_INFO;

//
//Device type's definition.
//
#define PCI_DEVICE_TYPE_BRIDGE       0x00000001
#define PCI_DEVICE_TYPE_NORMAL       0x00000002
#define PCI_DEVICE_TYPE_CARDBUS      0x00000004
#define PCI_DEVICE_TYPE_UNSUPPORTED  0x00000008
#define PCI_DEVICE_TYPE_EMPTY        0x00000000

#ifdef __cplusplus
}
#endif

#endif //__PCI_DRV_H__
