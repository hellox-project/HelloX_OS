//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 11,2017
//    Module Name               : __hxcomm.h
//    Module Funciton           : 
//                                This is a common header file for HelloX
//                                C Library,all other lib headers should include
//                                this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

/*
 * Version features or main changes:
 * V0.5.5:
 *   Features:
 *     1) POSIX standard condition variable implemented, pthread_cond_xxx
 *        routines added;
 *     2) Binary semaphore mechanism implemented, mainly used to support
 *        cFS porting;
 *     3) Timer API(SetTimer, CancelTimer) implemented and tested;
 *     4) Semaphore API of kernel implemented, and wrote a POSIX wrapper
 *        of semaphore by AI, with Garry tested;
 *     5) New API named GetCPUInfo implemented to return current CPU's
 *        frequency and TSC, which used by nanosleep, usleep, clock_gettime
 *        and other clock/time related routines;
 *     6) nanosleep, usleep, secondsleep routines implemented by AI and tested
 *        carefully by Garry;
 *     7) clock_gettime implemented by AI and tested by Garry carefully;
 *     8) Implemented the thread_attr related functions, with thread priority
 *        support;
 *   Pending issues:
 *     1) Unknown bug in printf routine when using dynamic memory allocation,
 *        found when apply condition var testing. Now skip the issue by
 *        using static global buffer;
 *
 * V0.5.4£º
 *
 */

#ifndef ____HXCOMM_H__
#define ____HXCOMM_H__

#ifndef __CPU_H__
#include "sys/cpu.h"
#endif

#ifndef __IDE_H__
#include "sys/ide.h"
#endif

/* HelloX lib version string. */
#define HELLOX_LIB_VER "0.5.5"

/* Maximal print buffer size. */
#define MAX_PRINTF_BUFFER_SIZE (1024 * 128)

#endif //____HXCOMM_H__
