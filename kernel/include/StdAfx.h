//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : StaAfx.H
//    Module Funciton           : 
//                                This is a common header file can be included by any source file,
//                                almost any kernel header files are included in this file,so use
//                                it directly instead of including each standalone file is a simplified
//                                programming way,but this may lead some problems,compiling slow maybe
//                                one of these glitches.Logical or fatal problems has not been found up
//                                to date.
//    Last modified Author      : Garry
//    Last modified Date        : Jun 4,2013
//    Last modified Content     :
//                                1. Deleted some unused header files:kthread.h/timer.h.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifdef _MSC_VER
//Disable the packing warning in case of Microsoft C environment.
#pragma warning(disable:4103)
//Disable the frame pointer register modified in inline assembly code warning.
#pragma warning(disable:4731)

#endif

//Include configuration headers in advance any of other headers.
//#ifndef __CONFIG_H__
//#include "../config/config.h"
//#endif



#ifndef __TYPES_H__
#include "types.h"
#endif

#ifndef __STRING__
#include "../lib/string.h"    //Different folder.
#endif

#ifndef __PERF_H__
#include "perf.h"
#endif

#ifndef __COMMOBJ_H__
#include "commobj.h"
#endif

#ifndef __SYN_MECH_H__
#if defined(__I386__)
#include "../arch/x86/syn_mech.h"
#elif defined(__STM32__)
#include "../arch/stm32/syn_mech.h"
#endif
#endif

//#ifndef __COMQUEUE_H__
//#include "COMQUEUE.H"
//#endif

#ifndef __OBJQUEUE_H__
#include "objqueue.h"
#endif

#ifndef __KTMGR_H__
#include "ktmgr.h"
#endif

#ifndef __KTMGR2_H__
#include "ktmgr2.h"
#endif

#ifndef __PROCESS_H__
#include "process.h"
#endif

#ifndef __RINGBUFF_H__
#include "ringbuff.h"
#endif

#ifndef __SYSTEM_H__
#include "system.h"
#endif

#ifndef __DEVMGR_H__
#include "devmgr.h"
#endif

#ifndef __MAILBOX_H__
#include "mailbox.h"
#endif

#ifndef __DIM_H__
#include "dim.h"
#endif

#ifndef __MEMMGR_H__
#include "memmgr.h"
#endif

#ifndef __PAGEIDX_H__
#include "pageidx.h"
#endif

#ifndef __VMM_H__
#include "vmm.h"
#endif

#ifndef __HEAP_H__
#include "heap.h"
#endif

//#ifndef __IOMGR_H__
#include "iomgr.h"
//#endif

#ifndef __BUFFMGR_H__
#include "buffmgr.h"
#endif

#ifndef __KMEMMGR__
#include "kmemmgr.h"
#endif

#ifndef __KTMSG_H__
#include "ktmsg.h"
#endif

#ifndef __SYSNET_H__
#include "sysnet.h"
#endif

#ifndef __GLOBAL_VAR__
#include "globvar.h"
#endif

#ifndef __CHARDISPLAY_H__
#include "chardisplay.h"
#endif

#ifndef __ARCH_H__
#if defined(__I386__)
#include "../arch/x86/arch.h"
#elif defined(__STM32__)
#include "../arch/stm32/arch.h"
#endif
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



////////////////////////////////////////////////////////////////////////////////
//
//                   Debug mechanism supporting code.
//
////////////////////////////////////////////////////////////////////////////////

#define __ASSERT()
#define __DEBUG_ASSERT()

#endif  //StdAfx.h
