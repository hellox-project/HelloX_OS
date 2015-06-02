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

//
//Interrupt object's definition.
//The interrupt object is used to manage system interrupt.
//

typedef BOOL (*__INTERRUPT_HANDLER)(LPVOID lpEsp,LPVOID);    //Interrupt handler's pro-type.

//Maximal interrupt vector supported now.
//#define MAX_INTERRUPT_VECTOR  256  //Moved to config.h file.

//Interrupt vector value's definition.
#ifdef __I386__
#define INTERRUPT_VECTOR_TIMER         0x20
#elif defined(__STM32__)
#define INTERRUPT_VECTOR_TIMER         0x0F
#endif

#define INTERRUPT_VECTOR_KEYBOARD      0x21
#define INTERRUPT_VECTOR_MOUSE         0x22
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

BOOL InterruptInitialize(__COMMON_OBJECT* lpThis);    //Interrupt object's initializing routine.
VOID InterruptUninitialize(__COMMON_OBJECT* lpThis);  //Uninitializing routine.

//
//Timer object's definition.
//The timer object is used to manage the system timer.
//

typedef DWORD    (*__DIRECT_TIMER_HANDLER)(LPVOID);    //Timer handler's protype.

BEGIN_DEFINE_OBJECT(__TIMER_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	//__TIMER_OBJECT*             lpPrevTimerObject;
    //__TIMER_OBJECT*             lpNextTimerObject;
	DWORD                       dwTimerID;            //Timer ID,one kernel thread may set
	                                                  //several timers,this is it's ID.
	DWORD                       dwTimeSpan;           //Timer span in millisecond.
	__KERNEL_THREAD_OBJECT*     lpKernelThread;       //The kernel thread who set the timer.
	LPVOID                      lpHandlerParam;
	DWORD                       (*DirectTimerHandler)(LPVOID);       //lpHandlerParam is it's parameter.
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

//
//The following is the definition of system object.
//
BEGIN_DEFINE_OBJECT(__SYSTEM)
    __INTERRUPT_OBJECT*                   lpInterruptVector[MAX_INTERRUPT_VECTOR];
    __PRIORITY_QUEUE*                     lpTimerQueue;

	DWORD                                 dwClockTickCounter;    //Records how many clock
	                                                             //tickes have occured since
	                                                             //system start.
	DWORD                                 dwNextTimerTick;       //When dwClockTickCounter
	                                                             //reaches this number,
	                                                             //one or many timer event
	                                                             //set by kernel thread
	                                                             //should be processed.
	volatile UCHAR                        ucIntNestLevel;        //Interrupt nesting level.
#define IN_INTERRUPT()  (System.ucIntNestLevel)                  //Current context is interrupt.
#define IN_KERNELTHREAD() (System.ucIntNestLevel == 0)           //Current context is process.

	volatile UCHAR                        bSysInitialized;       //Indicate if the system is initialized successfully.
#define IN_SYSINITIALIZATION() (FALSE == System.bSysInitialized) //To check if the system is under initialization phase.

	UCHAR                                 ucReserved1;           //Align to 4 bytes border.
	UCHAR                                 ucReserved2;

	DWORD                                 dwPhysicalMemorySize;

	BOOL                                  (*BeginInitialize)(__COMMON_OBJECT* lpThis);  //Called before the operating system
	                                                                                    //enter into initialization phase.
	BOOL                                  (*EndInitialize)(__COMMON_OBJECT* lpThis);    //Called after the OS finish the initialization
	                                                                                    //phase.
	BOOL                                  (*Initialize)(__COMMON_OBJECT* lpThis);
	DWORD                                 (*GetClockTickCounter)(__COMMON_OBJECT* lpThis);
	DWORD                                 (*GetSysTick)(DWORD* pdwHigh32);
	DWORD                                 (*GetPhysicalMemorySize)(__COMMON_OBJECT* lpThis);
	VOID                                  (*DispatchInterrupt)(__COMMON_OBJECT* lpThis,
		                                                       LPVOID           lpEsp,
		                                                       UCHAR            ucVector);
	VOID                                  (*DispatchException)(__COMMON_OBJECT* lpThis,
		                                                       LPVOID           lpEsp,
															   UCHAR            ucVector);

	__COMMON_OBJECT*                      (*ConnectInterrupt)(__COMMON_OBJECT* lpThis,
		                                                      __INTERRUPT_HANDLER InterruptHandler,
															  LPVOID           lpHandlerParam,
									        	   			  UCHAR            ucVector,
											        		  UCHAR            ucReserved1,
													          UCHAR            ucReserved2,
											         		  UCHAR            ucInterruptMode,
												        	  BOOL             bIfShared,
										         			  DWORD            dwCPUMask
												        	  );
	VOID                                  (*DisconnectInterrupt)(__COMMON_OBJECT* lpThis,
		                                                         __COMMON_OBJECT* lpIntObj);


	__COMMON_OBJECT*                      (*SetTimer)(__COMMON_OBJECT*         lpThis,
		                                              __KERNEL_THREAD_OBJECT*  lpKernelThread,
											          DWORD                    dwTimerID,
											          DWORD                    dwTimeSpan,
											          __DIRECT_TIMER_HANDLER   DirectTimerHandler,
											          LPVOID                   lpHandlerParam,
													  DWORD                    dwTimerFlags
											          );
	VOID                                  (*CancelTimer)(__COMMON_OBJECT* lpThis,
		                                                 __COMMON_OBJECT* lpTimer);

END_DEFINE_OBJECT(__SYSTEM)

#define TIMER_FLAGS_ONCE        0x00000001    //Set a timer with this flags,the timer only
                                              //apply once,i.e,the kernel thread who set
											  //the timer can receive timer message only
											  //once.
#define TIMER_FLAGS_ALWAYS      0x00000002    //Set a timer with this flags,the timer will
											  //availiable always,only if the kernel thread
											  //cancel the timer by calling CancelTimer.


/**************************************************************************************
***************************************************************************************
***************************************************************************************
***************************************************************************************
**************************************************************************************/

extern __SYSTEM System;    //Declares a global object--System.

extern __PERF_RECORDER  TimerIntPr;    //Performance recorder object used to mesure
                                       //the performance of timer interrupt.

//-------------------------------------------------------------------------------------
//
//        General Interrupt Handler's declaration.
//
//-------------------------------------------------------------------------------------

//typedef VOID (*__GENERAL_INTERRUPT_HANDLER)(DWORD,LPVOID);    //General interrupt handler's
                                                              //protype.

VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp);

#ifdef __cplusplus
}
#endif

#endif  //__SYSTEM_H__
