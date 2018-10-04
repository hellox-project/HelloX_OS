//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,06 2004
//    Module Name               : system.h
//    Module Funciton           : 
//                                This module countains system mechanism releated objects's
//                                definition.
//                                Including the following aspect:
//                                1. Interrupt object and interrupt management code;
//                                2. Timer object and timer management code;
//                                3. System level parameters management coee,such as
//                                   physical memory,system time,etc;
//                                4. Other system mechanism releated objects.
//
//                                ************
//                                This file is one of the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : 2009.03.14
//    Last modified Content     : 
//                                1.Add one member routine into system object,to handle exception 
//                                  and system call.
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "perf.h"
#include "ktmgr2.h"
#include "kmemmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Interrupt handler's pro-type. */
typedef BOOL (*__INTERRUPT_HANDLER)(LPVOID lpEsp,LPVOID);

//Maximal interrupt vector supported now.
//#define MAX_INTERRUPT_VECTOR  256  //Moved to config.h file.

//Interrupt vector value's definition.
#ifdef __I386__
#define INTERRUPT_VECTOR_TIMER         0x20
#elif defined(__STM32__)
#define INTERRUPT_VECTOR_TIMER         0x0F
#endif

#define INTERRUPT_VECTOR_BASE          0x20    //Base of all hardware interrupt.
#define INTERRUPT_VECTOR_KEYBOARD      0x21
#define INTERRUPT_VECTOR_MOUSE         0x2C
#define INTERRUPT_VECTOR_COM1          0x23
#define INTERRUPT_VECTOR_COM2          0x24
#define INTERRUPT_VECTOR_CLOCK         0x25
#define INTERRUPT_VECTOR_IDE           0x26
#define EXCEPTION_VECTOR_SYSCALL       0x7F     //For system call.

//Interrupt object.
BEGIN_DEFINE_OBJECT(__INTERRUPT_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	struct tag__INTERRUPT_OBJECT*           lpPrevInterruptObject;
    struct tag__INTERRUPT_OBJECT*           lpNextInterruptObject;
	UCHAR                         ucVector;
	BOOL                          (*InterruptHandler)(LPVOID lpParam,LPVOID lpEsp);
	LPVOID                        lpHandlerParam;
END_DEFINE_OBJECT(__INTERRUPT_OBJECT)

/* Interrupt object's initializing routine. */
BOOL InterruptInitialize(__COMMON_OBJECT* lpThis);
/* Uninitializing routine of interrupt object. */
VOID InterruptUninitialize(__COMMON_OBJECT* lpThis);

//
//Timer object's definition.
//The timer object is used to manage the system timer.
//
typedef DWORD (*__DIRECT_TIMER_HANDLER)(LPVOID);

BEGIN_DEFINE_OBJECT(__TIMER_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	DWORD dwObjectSignature;
	/* 
	 * Timer ID,one kernel thread may set several timer objects
	 * concurrently,use ID to distinguish each one.
	 */
	DWORD                       dwTimerID;
	/* Timer span in millisecond. */
	DWORD                       dwTimeSpan;
	/* The kernel thread who set the timer. */
	__KERNEL_THREAD_OBJECT*     lpKernelThread;
	LPVOID                      lpHandlerParam;
	DWORD                       (*DirectTimerHandler)(LPVOID);
	/* Flags to control the timer's action. */
	DWORD                       dwTimerFlags;
END_DEFINE_OBJECT(__TIMER_OBJECT)

BOOL  TimerInitialize(__COMMON_OBJECT* lpThis);    //Initializing routine of timer object.
VOID  TimerUninitialize(__COMMON_OBJECT* lpThis);  //Uninitializing routine of timer object.

//Determine interrupt types:interrupt or exception.
//In some platform,only exception exists,so we must adjust the target platform
//to adjust these 2 macros.
#if defined(__I386__)
#define IS_INTERRUPT(vector) (((vector) <= 0x2F) && ((vector) >= 0x20))
#define IS_EXCEPTION(vector) (!IS_INTERRUPT(vector))
#elif defined(__STM32__)
#define IS_INTERRUPT(vecotr) (TRUE)  //Treat anything as interrupt.
#define IS_EXCEPTION(vector) (FALSE)
#endif

//Interrupt slot object,corresponding a general interrupt vector.Many interrupt
//objects may belong to one interrupt slot,and they are linked together by a 
//link list.
BEGIN_DEFINE_OBJECT(__INTERRUPT_SLOT)
    volatile DWORD dwTotalInt;            //Total interrupt times since boot.
    volatile DWORD dwSuccHandledInt;      //Successfully handled interrupt number.
	__INTERRUPT_OBJECT* lpFirstIntObject; //First interrupt object belong to this slot.
END_DEFINE_OBJECT(__INTERRUPT_SLOT)

//Contains interrupt related statistics information for a specified interrupt vector.
BEGIN_DEFINE_OBJECT(__INTERRUPT_VECTOR_STAT)
    DWORD dwTotalInt;
	DWORD dwSuccHandledInt;
	DWORD dwTotalIntObject;
END_DEFINE_OBJECT(__INTERRUPT_VECTOR_STAT)

//
//The following is the definition of system object.
//
BEGIN_DEFINE_OBJECT(__SYSTEM)
    __PRIORITY_QUEUE* lpTimerQueue;
	__INTERRUPT_SLOT InterruptSlotArray[MAX_INTERRUPT_VECTOR];

	/* Current interrupt or exception vector. */
#if defined(__CFG_SYS_SMP)
	volatile UCHAR ucCurrInt[MAX_CPU_NUM];
#else
	volatile UCHAR ucCurrInt[1];
#endif

#if defined(__CFG_SYS_SMP)
	/* Spin lock to protect the system object. */
	__SPIN_LOCK spin_lock;
	/* 
	 * Set to 1 when BSP finished the initialization phase. 
	 * Other AP(s) initialization can not start unless this flag
	 * is set to 1.
	 */
	volatile unsigned char bspInitialized;
#endif

	/* Records how many clock tickes has ellapsed since system start.*/
	volatile DWORD dwClockTickCounter;
	/* 
	 * When dwClockTickCounter reaches this number, one or more timer events 
	 * set by kernel thread should be processed. 
	 */
	volatile DWORD dwNextTimerTick;
	
	/* Interrupt nesting level,for each CPU. */
#if defined(__CFG_SYS_SMP)
	volatile UCHAR ucIntNestLevel[MAX_CPU_NUM];
#else
	volatile UCHAR ucIntNestLevel[1];
#endif

	/* Current context is in interrupt. */
#define IN_INTERRUPT()  (System.ucIntNestLevel[__CURRENT_PROCESSOR_ID])
	/* Current execution context is in process(kernel thread). */
#define IN_KERNELTHREAD() (System.ucIntNestLevel[__CURRENT_PROCESSOR_ID] == 0)

	/* Indicate if the system is initialized successfully. */
	volatile UCHAR bSysInitialized;
	/* To check if the system is under initialization phase. */
#define IN_SYSINITIALIZATION() (FALSE == System.bSysInitialized)

	/* Align to 4 bytes border. */
	UCHAR ucReserved1;
	UCHAR ucReserved2;

	/* How many physical memory in system. */
	DWORD dwPhysicalMemorySize;

	/* Called before the operating system enter into initialization phase. */
	BOOL (*BeginInitialize)(__COMMON_OBJECT* lpThis);
	/* Hook to initialize hardware,should be implemented in arch specific. */
	BOOL (*HardwareInitialize)(__COMMON_OBJECT* lpThis);
	/* Called after the OS finish the initialization phase. */
	BOOL (*EndInitialize)(__COMMON_OBJECT* lpThis);
	/* Initialize system object itself. */
	BOOL (*Initialize)(__COMMON_OBJECT* lpThis);
	DWORD (*GetClockTickCounter)(__COMMON_OBJECT* lpThis);
	DWORD (*GetSysTick)(DWORD* pdwHigh32);
	DWORD (*GetPhysicalMemorySize)(__COMMON_OBJECT* lpThis);
	/* Dispatch interrupt. */
	VOID  (*DispatchInterrupt)(__COMMON_OBJECT* lpThis,
		LPVOID lpEsp,
		UCHAR ucVector);
	/* Dispatch exception. */
	VOID (*DispatchException)(__COMMON_OBJECT* lpThis,
		LPVOID lpEsp,
		UCHAR ucVector);
	/* Connect a interrupt vector to it's handler. */
	__COMMON_OBJECT* (*ConnectInterrupt)(__COMMON_OBJECT* lpThis,
		__INTERRUPT_HANDLER InterruptHandler,
		LPVOID lpHandlerParam,
		UCHAR ucVector,
		UCHAR ucReserved1,
		UCHAR ucReserved2,
		UCHAR ucInterruptMode,
		BOOL bIfShared,
		DWORD dwCPUMask);
	/* Disconnect interrupt. */
	VOID (*DisconnectInterrupt)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpIntObj);

	/* Set a timer. */
	__COMMON_OBJECT* (*SetTimer)(__COMMON_OBJECT* lpThis,
		__KERNEL_THREAD_OBJECT* lpKernelThread,
		DWORD dwTimerID,
		DWORD dwTimeSpan,
		__DIRECT_TIMER_HANDLER DirectTimerHandler,
		LPVOID lpHandlerParam,
		DWORD dwTimerFlags);
	/* Cancel a timer. */
	BOOL (*CancelTimer)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpTimer);

	/* Get interrupt statistics information. */
	BOOL(*GetInterruptStat)(__COMMON_OBJECT* lpThis, UCHAR ucVector,
		__INTERRUPT_VECTOR_STAT* pStat);
END_DEFINE_OBJECT(__SYSTEM)

/* 
 * Set a timer with this flags,the timer only
 * apply once,i.e,the kernel thread who set
 * the timer can receive timer message only
 * once.
 */
#define TIMER_FLAGS_ONCE        0x00000001
/* 
 * Set a timer with this flags,the timer will
 * availiable always,only if the kernel thread
 * cancel the timer by calling CancelTimer.
 */
#define TIMER_FLAGS_ALWAYS      0x00000002
/*
 * The timer will not be deleted automatically after
 * fired,only available for one shot timer.
 */
#define TIMER_FLAGS_NOAUTODEL   0x00000004
/* The one shot timer is fired. */
#define TIMER_FLAGS_FIRED       0x00000008

/**************************************************************************************
***************************************************************************************
***************************************************************************************
***************************************************************************************
**************************************************************************************/

/* Global system object,only one in system. */
extern __SYSTEM System;

/* 
 * Performance recorder object used to mesure
 * the performance of timer interrup
 */
extern __PERF_RECORDER  TimerIntPr;

/* 
 * General interrupt handler,the unified interrupt entry point 
 * for all interrupt(s) and exception(s).
 * It is mainly invoked by mini kernel.
 */
VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp);

#ifdef __cplusplus
}
#endif

#endif  //__SYSTEM_H__
