//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 4, 2013
//    Module Name               : config.h
//    Module Funciton           : 
//                                This file is used to configure the OS before compile it,
//                                it defines several switches that controls which module will or
//                                will not be included in the binary module.
//                                Developers can include or exclude one function module by turn on
//                                or turn off the corresponding switch.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#define __CONFIG_H__  //Include switch.

//Use Microsoft Visual Studio as compiler.Just use "define __GCC__" replacing this
//if you use GCC as compiler.
#define __MS_VC__

//************************************************************************
//
//  Displayable information,such as version
//
//************************************************************************

#define OS_VERSION   "    HelloX V1.82"
#define HELLOX_VERSION_INFO "HelloX [Version 1.820(Beta),build in 2018/05/29,by Garry.Xin]"
#define HELLOX_SLOGAN_INFO  "HelloX OS,through which you can talk to everything."
#define HELLOX_SPECIAL_INFO "Nickname:babygirl,to remark the birth of Sophia,my little girl,in 2016/04/19."

//************************************************************************
//
//  Hardware platform pre-defined switches.
//
//************************************************************************

//Little endian or big endian.
#define __CFG_CPU_LE

//CPU types,the following definitions are exclusive.
#define __I386__
//#define __ARM7__
//#define __ARM9__
//#define __ARM11__
//#define __STM32__

//************************************************************************
//
//  Hardware related definitions.
//
//************************************************************************

//System time slice,in ms.You can adjust it,but you must configure the
//hardware to keep same as this definition.
#define SYSTEM_TIME_SLICE  5   //5 million seconds per tick.

//CPU byte alignment,memory allocation will use this value to align block.
#define SYSTEM_BYTE_ALIGN  4   //4 will work under most platforms.

//The maximal TLS(Thread Local Storage) number.
#define MAX_TLS_NUM        0x04

//************************************************************************
//
//  Common system level parameters that can fit any hardware platform.
//  Developers can adjust system level performance by adjusting these
//  parameters,such as reduce the memory occupation.
//
//************************************************************************

//
//This value is the smallest stack size of a kernel thread in IA32 platform.
//
#define MIN_STACK_SIZE 128

//Maximal interrupt vector number.It controls the size of global interrupt
//object array,and can be modified according to hardware platform.For example,
//we can change it to 80 in STM32 platform to save memory.
#define MAX_INTERRUPT_VECTOR  256

//
//This value is the default stack size of a kernel thread in IA32 platform,
//if the user does not give a stack size in CreateKernelThread calling,then
//use this value as stack size.
//
#define DEFAULT_STACK_SIZE 0x00008000 //32k bytes.

//************************************************************************
//
//  System level pre-defined switches.
//
//************************************************************************

//Use Inline Schedule(IS) mode.The inline schedule means OS kernel can establish
//thread context without interrupt,only by normal instructions,and also can switch
//to a kernel thread without need in interrupt context.
//A type case for inline schedule is x86 platform,STM32 is contrast,which can not
//support inline schedule,i.e,kernel thread context must be restored in interrupt(handler)
//mode,please comment off this flag when migrate to this kind of CPU.
#define __CFG_SYS_IS

//Include virtual memory management functions in OS.
#define __CFG_SYS_VMM

//Enable or disable interrupt nest.It should be disabled under x86 platform,
//and maybe enabled on ARM platform.
//#define __CFG_SYS_INTNEST

//Include thread heap functions.
#define __CFG_SYS_HEAP

//Include bus management code.
#define __CFG_SYS_BM

//Use Free Block List(FBL) as default kernel memory management algorithm.It should
//be exclusive with __CFG_SYS_MMTFA switch.
#define __CFG_SYS_MMFBL

//Use Time Fixed Allocation(TFA) as default kernel memory management algorithm.
//It should be exclusive with __CFG_SYS_MMFBL switch.
//#define __CFG_SYS_MMTFA

//Device Driver Framework(DDF) function in OS.
#define __CFG_SYS_DDF

//Include CPU statistics functions in OS.
#define __CFG_SYS_CPUSTAT

//Include the default user shell thread in OS,only enable it when character
//output device is ready.
#define __CFG_SYS_SHELL

//Include system call support.
#define __CFG_SYS_SYSCALL

//Include USB host controller suppot.
#define __CFG_SYS_USB

//Include console object into kernel.COM input and output functions are implemented
//in console object.
//#define __CFG_SYS_CONSOLE

//************************************************************************
//
//  Pre-defined switches to control inline device drivers .
//
//************************************************************************

//Include IDE driver in OS.
#define __CFG_DRV_IDE

//Include COM driver in OS.
#define __CFG_DRV_COM

//Include USART driver in OS,specific for STM32 or ARM platform.
//#define __CFG_DRV_USART

//Include SDIO driver support in OS,only available for STM32.
//#define __CFG_DRV_SDIO

//Include MOUSE driver in OS.
//#define __CFG_DRV_MOUSE

//Include Key board driver in OS.
#define __CFG_DRV_KEYBOARD

//Include USB mass storage driver in OS.
#define __CFG_DRV_USBSTORAGE

//Include USB Network device driver in OS.
#define __CFG_DRV_USBNETWORK

//Include USB mouse driver in OS kernel.
//#define __CFG_DRV_USBMOUSE

//Include USB keyboard driver in OS kernel.
#define __CFG_DRV_USBKBD

//Include USB Video Class driver in OS kernel.
//#define __CFG_DRV_UVC

//************************************************************************
//
//  Pre-defined switches to control file system.
//
//************************************************************************

//Include FAT32 file system function in OS.
#define __CFG_FS_FAT32

//Include NTFS file system function in OS.
//#define __CFG_FS_NTFS

//Include RAM file system in OS.
//#define __CFG_FS_RAM

//Include FLASH file system in OS.
//#define __CFG_FS_FLASH

//************************************************************************
//
//  Pre-defined switches to control network functions.
//
//************************************************************************

//Enable network subsystem in OS.
#define __CFG_NET_ENABLE

//Include ethernet manager in OS.
#define __CFG_NET_ETHMGR

//Enable ethernet bridging.
//#define __CFG_NET_EBRG

//Enable IP dhcp function.
#define __CFG_NET_DHCP

//Include IPv4 network protocol in OS.
#define __CFG_NET_IPv4

//Include PCNet driver in OS.
#define __CFG_NET_PCNET

//Include RTL8111 driver in OS.
//#define __CFG_NET_RTL8111

//Include E1000 driver in OS.
//#define __CFG_NET_E1000

//Include IPv6 network protocol in OS.
//#define __CFG_NET_IPv6

//Include Marvel WLAN's ethernet driver.
//#define __CFG_NET_MARVELLAN

//Include ENC28J60 ethernet driver.
//#define __CFG_NET_ENC28J60

//************************************************************************
//
//  Pre-defined switches to control what kinds of application components
//  will be included in system binary.
//
//************************************************************************

//Enable or disable Java virtual machine in system.
//#define __CFG_APP_JVM

//Enable or disable SSH support in system.
//#define __CFG_APP_SSH

//Enable or disable telnet support in system.
//#define __CFG_APP_TELNET

//************************************************************************
//
//  Pre-defined switches to control how to use this OS,and the user entry
//  point's priority if it's used as embedded operating system.
//
//************************************************************************

//Used as embedded OS,so user entry point should be loaded.
//#define __CFG_USE_EOS

//User entry point thread's priority if used as EOS.
#define __HCNMAIN_PRIORITY  PRIORITY_LEVEL_NORMAL

//User entry point thread's name.
#define __HCNMAIN_NAME      "HCN_Main"

//************************************************************************
//
//  More compiling switches can be added here.
//
//************************************************************************

#define _TIME_T_DEFINED

//#define __GCC__
