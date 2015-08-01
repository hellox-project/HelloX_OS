//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,06 2004
//    Module Name               : system.cpp
//    Module Funciton           : 
//                                This module countains system mechanism releated objects's
//                                implementation..
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
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 2011/12/16
//    Last modified Content     : Deleted some commented code from the file,and optimized the
//                                default exception and interrupt's handler.
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include "system.h"
#include "types.h"
#include "syscall.h"
#include "stdio.h"
#include "ktmsg.h"

#include "hellocn.h"
#include "kapi.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif




__PERF_RECORDER  TimerIntPr = {
	U64_ZERO,
	U64_ZERO,
	U64_ZERO,
	U64_ZERO
};                  //Performance recorder object used to mesure
                                //the performance of timer interrupt.

//
//TimerInterruptHandler routine.
//The following routine is the most CRITICAL routine of kernel of Hello China.
//The routine does the following:
// 1. Schedule timer object;
// 2. Update the system level variables,such as dwClockTickCounter;
// 3. Schedule kernel thread(s).
//
static BOOL TimerInterruptHandler(LPVOID lpEsp,LPVOID lpParam)
{
	DWORD                     dwPriority        = 0;
	__TIMER_OBJECT*           lpTimerObject     = 0;
	__KERNEL_THREAD_MESSAGE   Msg                   ;
	__PRIORITY_QUEUE*         lpTimerQueue      = NULL;
	__PRIORITY_QUEUE*         lpSleepingQueue   = NULL;
	__KERNEL_THREAD_OBJECT*   lpKernelThread    = NULL;
	DWORD                     dwFlags           = 0;
	
	if(NULL == lpEsp)    //Parameter check.
	{
		return TRUE;
	}

	if(System.dwClockTickCounter == System.dwNextTimerTick)     //Should schedule timer.
	{
		lpTimerQueue = System.lpTimerQueue;
		lpTimerObject = (__TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpTimerQueue,
			&dwPriority);
		if(NULL == lpTimerObject)
		{
			goto __CONTINUE_1;
		}
		dwPriority = MAX_DWORD_VALUE - dwPriority;
		while(dwPriority <= System.dwNextTimerTick)    //Strictly speaking,the dwPriority
			                                           //variable must EQUAL System.dw-
													   //NextTimerTick,but in the implement-
													   //ing of the current version,there
													   //may be some error exists,so we assume
													   //dwPriority equal or less than dwNext-
													   //TimerTic.
		{
			if(NULL == lpTimerObject->DirectTimerHandler)  //Send a message to the kernel thread.
			{
				Msg.wCommand = KERNEL_MESSAGE_TIMER;
				Msg.dwParam  = lpTimerObject->dwTimerID;
				KernelThreadManager.SendMessage(
					(__COMMON_OBJECT*)lpTimerObject->lpKernelThread,
					&Msg);
				//PrintLine("Send a timer message to kernel thread.");
			}
			else
			{
				lpTimerObject->DirectTimerHandler(lpTimerObject->lpHandlerParam); //Call the associated handler.
			}

			switch(lpTimerObject->dwTimerFlags)
			{
			case TIMER_FLAGS_ONCE:        //Delete the timer object processed just now.
				ObjectManager.DestroyObject(&ObjectManager,
					(__COMMON_OBJECT*)lpTimerObject);
				break;
			case TIMER_FLAGS_ALWAYS:    //Re-insert the timer object into timer queue.
				dwPriority  = lpTimerObject->dwTimeSpan;
				dwPriority /= SYSTEM_TIME_SLICE;
				dwPriority += System.dwClockTickCounter;
				dwPriority  = MAX_DWORD_VALUE - dwPriority;
				lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpTimerQueue,
					(__COMMON_OBJECT*)lpTimerObject,
					dwPriority);
				break;
			default:
				break;
			}

			lpTimerObject = (__TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpTimerQueue,
				&dwPriority);    //Check another timer object.
			if(NULL == lpTimerObject)
			{
				break;
			}
			dwPriority = MAX_DWORD_VALUE - dwPriority;
		}

		if(NULL == lpTimerObject)  //There is no timer object in queue.
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			System.dwNextTimerTick = 0;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		}
		else
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			System.dwNextTimerTick = dwPriority;    //Update the next timer tick counter.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpTimerQueue,
				(__COMMON_OBJECT*)lpTimerObject,
				dwPriority);
		}
	}

__CONTINUE_1:

	//
	//The following code wakes up all kernel thread(s) whose status is SLEEPING and
	//the time it(then) set is out.
	//
	if(System.dwClockTickCounter == KernelThreadManager.dwNextWakeupTick)  //There must existes
		                                                                   //kernel thread(s) to
																		   //be wake up.
	{
		lpSleepingQueue = KernelThreadManager.lpSleepingQueue;
		lpKernelThread  = (__KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpSleepingQueue,
			&dwPriority);
		while(lpKernelThread)
		{
			dwPriority = MAX_DWORD_VALUE - dwPriority;  //Now,dwPriority countains the tick
			                                            //counter value.
			if(dwPriority > System.dwClockTickCounter)
			{
				break;    //This kernel thread should not be wake up.
			}
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);  //Insert the waked up kernel thread into ready queue.

			lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpSleepingQueue,
				&dwPriority);  //Check next kernel thread in sleeping queue.
		}
		if(NULL == lpKernelThread)
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.dwNextWakeupTick = 0;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		}
		else
		{
			__ENTER_CRITICAL_SECTION(NULL,dwFlags);
			KernelThreadManager.dwNextWakeupTick = dwPriority;
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpSleepingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpSleepingQueue,
				(__COMMON_OBJECT*)lpKernelThread,
				dwPriority);
		}
	}

	goto __TERMINAL;

__TERMINAL:
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	System.dwClockTickCounter ++;    //Update the system clock interrupt counter.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return TRUE;
}


//
//The implementation of kConnectInterrupt routine of Interrupt Object.
//The routine do the following:
// 1. Insert the current object into interrupt object array(maintenanced by system object);
// 2. Set the object's data members correctly.
//

static __COMMON_OBJECT* kConnectInterrupt(__COMMON_OBJECT*     lpThis,
							 __INTERRUPT_HANDLER  lpInterruptHandler,
							 LPVOID               lpHandlerParam,
							 UCHAR                ucVector,
							 UCHAR                ucReserved1,
							 UCHAR                ucReserved2,
							 UCHAR                ucInterruptMode,
							 BOOL                 bIfShared,
							 DWORD                dwCPUMask)
{
	__INTERRUPT_OBJECT*      lpInterrupt          = NULL;
	__INTERRUPT_OBJECT*      lpObjectRoot         = NULL;
	__SYSTEM*                lpSystem             = &System;  //Had as a BUG here!!!
	DWORD                    dwFlags              = 0;

	if((NULL == lpThis) || (NULL == lpInterruptHandler))    //Parameters valid check.
	{
		return NULL;
	}

	if(ucVector >= MAX_INTERRUPT_VECTOR)                    //Impossible!!!
	{
		return NULL;
	}

	lpInterrupt = (__INTERRUPT_OBJECT*)
		ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_INTERRUPT);
	if(NULL == lpInterrupt)    //Failed to create interrupt object.
	{
		return FALSE;
	}
	if(!lpInterrupt->Initialize((__COMMON_OBJECT*)lpInterrupt))  //Failed to initialize.
	{
		return FALSE;
	}

	lpInterrupt->lpPrevInterruptObject = NULL;
	lpInterrupt->lpNextInterruptObject = NULL;
	lpInterrupt->InterruptHandler      = lpInterruptHandler;
	lpInterrupt->lpHandlerParam        = lpHandlerParam;
	lpInterrupt->ucVector              = ucVector;

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpObjectRoot = lpSystem->lpInterruptVector[ucVector];
	if(NULL == lpObjectRoot)    //If this is the first interrupt object of the vector.
	{
		System.lpInterruptVector[ucVector]  = lpInterrupt;
	}
	else
	{
		lpInterrupt->lpNextInterruptObject  = lpObjectRoot;
		lpObjectRoot->lpPrevInterruptObject = lpInterrupt;
		System.lpInterruptVector[ucVector]  = lpInterrupt;
	}
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return (__COMMON_OBJECT*)lpInterrupt;
}

//
//The implementation of kDiskConnectInterrupt.
//

static VOID kDiskConnectInterrupt(__COMMON_OBJECT* lpThis,
								__COMMON_OBJECT* lpInterrupt)
{
	__INTERRUPT_OBJECT*   lpIntObject    = NULL;
	__SYSTEM*             lpSystem       = NULL;
	UCHAR                 ucVector       = 0;
	DWORD                 dwFlags        = 0;

	if((NULL == lpThis) || (NULL == lpInterrupt)) //Parameters check.
	{
		return;
	}

	lpSystem = (__SYSTEM*)lpThis;
	lpIntObject = (__INTERRUPT_OBJECT*)lpInterrupt;
	ucVector    = lpIntObject->ucVector;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpIntObject->lpPrevInterruptObject)  //This is the first interrupt object.
	{
		lpSystem->lpInterruptVector[ucVector] = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject) //Is not the last object.
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = NULL;
		}
	}
	else    //This is not the first object.
	{
		lpIntObject->lpPrevInterruptObject->lpNextInterruptObject = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject)
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = lpIntObject->lpPrevInterruptObject;
		}
	}
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return;
}

//
//The implementation of Initialize routine of interrupt object.
//
BOOL InterruptInitialize(__COMMON_OBJECT* lpThis)
{
	__INTERRUPT_OBJECT*    lpInterrupt = NULL;

	if(NULL == lpThis)
	{
		return FALSE;
	}

	lpInterrupt = (__INTERRUPT_OBJECT*)lpThis;
	lpInterrupt->lpPrevInterruptObject = NULL;
	lpInterrupt->lpNextInterruptObject = NULL;
	lpInterrupt->InterruptHandler      = NULL;
	lpInterrupt->lpHandlerParam        = NULL;
	lpInterrupt->ucVector              = 0;
	return TRUE;
}

//
//The implementation of Uninitialize of interrupt object.
//This routine does nothing.
//

VOID InterruptUninitialize(__COMMON_OBJECT* lpThis)
{
	return;
}


//
//The implementation of timer object.
//
BOOL TimerInitialize(__COMMON_OBJECT* lpThis)    //Initializing routine of timer object.
{
	__TIMER_OBJECT*     lpTimer  = NULL;
	
	if(NULL == lpThis)
	{
		return FALSE;
	}

	lpTimer = (__TIMER_OBJECT*)lpThis;
	lpTimer->dwTimerID    = 0;
	lpTimer->dwTimeSpan   = 0;
	lpTimer->lpKernelThread      = NULL;
	lpTimer->lpHandlerParam      = NULL;
	lpTimer->DirectTimerHandler  = NULL;

	return TRUE;
}

//
//Uninitializing routine of timer object.
//

VOID TimerUninitialize(__COMMON_OBJECT* lpThis)
{
	return;
}

//-----------------------------------------------------------------------------------
//
//              The implementation of system object.
//
//------------------------------------------------------------------------------------

//
//Initializing routine of system object.
//The routine do the following:
// 1. Create a priority queue,to be used as lpTimerQueue,countains the timer object;
// 2. Create an interrupt object,as TIMER interrupt object;
// 3. Initialize system level variables,such as dwPhysicalMemorySize,etc.
//

static BOOL SystemInitialize(__COMMON_OBJECT* lpThis)
{
	__SYSTEM*            lpSystem         = (__SYSTEM*)lpThis;
	__PRIORITY_QUEUE*    lpPriorityQueue  = NULL;
	__INTERRUPT_OBJECT*  lpIntObject      = NULL;
	__INTERRUPT_OBJECT*  lpExpObject      = NULL;
	BOOL                 bResult          = FALSE;
	DWORD                dwFlags          = 0;

	if(NULL == lpSystem)
	{
		return FALSE;
	}

	lpPriorityQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);

	if(NULL == lpPriorityQueue)  //Failed to create priority queue.
	{
		return FALSE;
	}

	if(!lpPriorityQueue->Initialize((__COMMON_OBJECT*)lpPriorityQueue))  //Failed to initialize
		                                                                 //priority queue.
	{
		goto __TERMINAL;
	}
	lpSystem->lpTimerQueue = lpPriorityQueue;

	//Create and initialize timer interrupt object.
	lpIntObject = (__INTERRUPT_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_INTERRUPT);
	if(NULL == lpIntObject)
	{
		goto __TERMINAL;
	}
	bResult = lpIntObject->Initialize((__COMMON_OBJECT*)lpIntObject);
	if(!bResult)
	{
		goto __TERMINAL;
	}
	lpIntObject->ucVector = INTERRUPT_VECTOR_TIMER;
	lpIntObject->lpHandlerParam = NULL;
	lpIntObject->InterruptHandler = TimerInterruptHandler;

	//Create and initialize system call exception interrupt object.
	lpExpObject = (__INTERRUPT_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_INTERRUPT);
	if(NULL == lpExpObject)
	{
		goto __TERMINAL;
	}
	bResult = lpExpObject->Initialize((__COMMON_OBJECT*)lpExpObject);
	if(!bResult)
	{
		goto __TERMINAL;
	}
	lpExpObject->ucVector = EXCEPTION_VECTOR_SYSCALL;
	lpExpObject->lpHandlerParam = NULL;
	lpExpObject->InterruptHandler = SyscallHandler;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpSystem->lpInterruptVector[INTERRUPT_VECTOR_TIMER]   = lpIntObject;
	lpSystem->lpInterruptVector[EXCEPTION_VECTOR_SYSCALL] = lpExpObject;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(lpPriorityQueue != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpPriorityQueue);
		}
		if(lpIntObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpIntObject);
		}
		if(lpExpObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpExpObject);
		}
	}
	return bResult;
}

//
//GetClockTickCounter routine.
//
static DWORD GetClockTickCounter(__COMMON_OBJECT* lpThis)
{
	__SYSTEM*    lpSystem = (__SYSTEM*)lpThis;

	if(NULL == lpSystem)
	{
		return 0;
	}
	return lpSystem->dwClockTickCounter;
}

//Return the system tick counter,the pdwHigh32 contains the uper 32 bits
//of system tick counter.
static DWORD GetSysTick(DWORD* pdwHigh32)
{
	return System.dwClockTickCounter;
}

//
//GetPhysicalMemorySize.
//
static DWORD GetPhysicalMemorySize(__COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)
	{
		return 0;
	}
	return ((__SYSTEM*)lpThis)->dwPhysicalMemorySize;
}


//
//This routine is the default interrupt handler.
//If no entity(such as device driver) install an interrupt handler,this handler
//will be called to handle the appropriate interrupt.
//It will only print out a message indicating the interrupt's number and return.
//
static VOID DefaultIntHandler(LPVOID lpEsp,UCHAR ucVector)
{
	CHAR          strBuffer[64];
	static DWORD  dwTotalNum    = 0;

	dwTotalNum ++;    //Record this unhandled exception or interrupt.

	PrintLine("  Unhandled interrupt.");  //Print out this message.
	_hx_sprintf(strBuffer,"  Interrupt number = %d.",ucVector);
	PrintLine(strBuffer);
	_hx_sprintf(strBuffer,"  Total unhandled interrupt times = %d.",dwTotalNum);
	PrintLine(strBuffer);
	return;
}


static VOID DispatchInterrupt(__COMMON_OBJECT* lpThis,
							  LPVOID           lpEsp,
							  UCHAR ucVector)
{
	__INTERRUPT_OBJECT*    lpIntObject  = NULL;
	__SYSTEM*              lpSystem = (__SYSTEM*)lpThis;
	CHAR                   strError[64];    //To print out the BUG information.

	if((NULL == lpThis) || (NULL == lpEsp))
	{
		return;
	}

	lpSystem->ucIntNestLevel += 1;    //Increment nesting level.
	if(lpSystem->ucIntNestLevel <= 1)
	{
		//Call thread hook here,because current kernel thread is
		//interrupted.
		//If interrupt occurs before any kernel thread is scheduled,
		//lpCurrentKernelThread is NULL.
		if(KernelThreadManager.lpCurrentKernelThread)
		{
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE,
				KernelThreadManager.lpCurrentKernelThread,
				NULL);
		}
	}
	//For debugging.
	else
	{
#ifdef __CFG_SYS_INTNEST  //Interupt nest is enabled.
		//Do nothing.
#else
		_hx_printf("Fatal error,interrupt nested(enter-int,vector = %d,nestlevel = %d)\r\n",
			ucVector,
			lpSystem->ucIntNestLevel);
		BUG();
#endif
	}

	lpIntObject = lpSystem->lpInterruptVector[ucVector];
	//_hx_printf("lpIntObject=%p\n", lpIntObject);

	if(NULL == lpIntObject)  //The current interrupt vector has not handler object.
	{
		DefaultIntHandler(lpEsp,ucVector);
		goto __RETFROMINT;
	}

	while(lpIntObject)    //Travel the whole interrupt list of this vector.
	{
		if(lpIntObject->InterruptHandler(lpEsp,
			lpIntObject->lpHandlerParam))    //If an interrupt object handles the interrupt,then returns.
		{
			break;
		}
		lpIntObject = lpIntObject->lpNextInterruptObject;
	}

__RETFROMINT:
	lpSystem->ucIntNestLevel -= 1;    //Decrement interrupt nesting level.
	if(0 == lpSystem->ucIntNestLevel)  //The outmost interrupt.
	{
		if (IN_SYSINITIALIZATION())  //It's a abnormal case.
		{

			_hx_sprintf(strError, "Warning: Interrupt[%d] raised in sys initialization.", ucVector);
			PrintStr(strError);
		}
		else
		{
			KernelThreadManager.ScheduleFromInt((__COMMON_OBJECT*)&KernelThreadManager,
				lpEsp);  //Re-schedule kernel thread.
		}
	}
	else
	{
#ifdef __CFG_SYS_INTNEST  //Interrupt nest is enabled.
		//Do nothing.
#else
		_hx_printf("Fatal error,interrupt nested(leave-int,vector = %d,nestlevel = %d)\r\n",
			ucVector,
			lpSystem->ucIntNestLevel);
		BUG();
#endif  //__CFG_SYS_INTNEST
	}
	return;
}


//Default handler of Exception.
static VOID DefaultExcepHandler(LPVOID pESP,UCHAR ucVector)
{
         __KERNEL_THREAD_OBJECT* pKernelThread = KernelThreadManager.lpCurrentKernelThread;
         DWORD dwFlags;
         static DWORD totalExcepNum = 0;

         //Switch to text mode,because the exception maybe caused in GUI mode.
//#ifdef __I386__
//         SwitchToText();
//#endif
         _hx_printf("Exception occured: #%d.\r\n",ucVector);
         totalExcepNum ++;  //Increase total exception number.

         //Show kernel thread information which lead the exception.
         if(pKernelThread)
         {
                   _hx_printf("\tCurrent kthread ID: %d.\r\n",pKernelThread->dwThreadID);
                   _hx_printf("\tCurrent kthread name: %s.\r\n",pKernelThread->KernelThreadName);
         }
         else //In process of system initialization.
         {
                   _hx_printf("\tException occured in process of initialization.\r\n");
         }

         //Call processor specific exception handler.
         PSExcepHandler(pESP,ucVector);

         if(totalExcepNum >= 1)  //Too many exception,maybe in deadlock,so halt the system.
         {
                   _hx_printf("Fatal error: Total exception number reached maximal value(%d).\r\n",totalExcepNum);
                   _hx_printf("Please power off the system and reboot it.\r\n");
                   __ENTER_CRITICAL_SECTION(NULL,dwFlags);
                   while(1); //Make a dead loop.
                   __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
         }
         return;
}



//DispatchException,called by GeneralIntHandler to handle exception,include
//system call.
static VOID DispatchException(__COMMON_OBJECT* lpThis,
							  LPVOID           lpEsp,
							  UCHAR            ucVector)
{
	__SYSTEM*                 lpSystem  = (__SYSTEM*)lpThis;
	__INTERRUPT_OBJECT*       lpIntObj  = NULL;

	if(NULL == lpSystem)
	{
		return;
	}
	if(!IS_EXCEPTION(ucVector))  //Not a exception.
	{
		return;
	}
	lpIntObj = lpSystem->lpInterruptVector[ucVector];
	if(NULL == lpIntObj)  //Null exception,call default exception handler.
	{
		DefaultExcepHandler(lpEsp,ucVector);
	}
	//Call the exception handler now.For each exception,only one handler present.
	lpIntObj->InterruptHandler(lpEsp,lpIntObj->lpHandlerParam);
	return;
}

//
//kSetTimer.
//The routine do the following:
// 1. Create a timer object;
// 2. Initialize the timer object;
// 3. Insert into the timer object into timer queue of system object;
// 4. Return the timer object's base address if all successfully.
//
static __COMMON_OBJECT* kSetTimer(__COMMON_OBJECT* lpThis,
								 __KERNEL_THREAD_OBJECT* lpKernelThread,
					             DWORD  dwTimerID,
								 DWORD  dwTimeSpan,
								 __DIRECT_TIMER_HANDLER lpHandler,
					             LPVOID lpHandlerParam,
								 DWORD  dwTimerFlags)
{
	//__PRIORITY_QUEUE*            lpPriorityQueue    = NULL;
	__SYSTEM*                    lpSystem           = NULL;
	__TIMER_OBJECT*              lpTimerObject      = NULL;
	BOOL                         bResult            = FALSE;
	DWORD                        dwPriority         = 0;
	DWORD                        dwFlags            = 0;

	if((NULL == lpThis) || (NULL == lpKernelThread))    //Parameters check.
	{
		return NULL;
	}

	//At least one time slice is required for timer object.
	if(dwTimeSpan <= SYSTEM_TIME_SLICE)
	{
		dwTimeSpan = SYSTEM_TIME_SLICE;
	}

	lpSystem    = (__SYSTEM*)lpThis;
	lpTimerObject = (__TIMER_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_TIMER);
	if(NULL == lpTimerObject)    //Can not create timer object.
	{
		goto __TERMINAL;
	}
	bResult = lpTimerObject->Initialize((__COMMON_OBJECT*)lpTimerObject);  //Initialize.
	if(!bResult)
	{
		goto __TERMINAL;
	}
	lpTimerObject->dwTimerID           = dwTimerID;
	lpTimerObject->dwTimeSpan          = dwTimeSpan;
	lpTimerObject->lpKernelThread      = lpKernelThread;
	lpTimerObject->DirectTimerHandler  = lpHandler;
	lpTimerObject->lpHandlerParam      = lpHandlerParam;
	lpTimerObject->dwTimerFlags        = dwTimerFlags;

	//
	//The following code calculates the priority value of the timer object.
	//
	dwPriority     = dwTimeSpan;
	dwPriority    /= SYSTEM_TIME_SLICE;
	dwPriority    += lpSystem->dwClockTickCounter;    //Now,the dwPriority countains the
	                                                  //tick counter this timer must be
	                                                  //processed.
	dwPriority     = MAX_DWORD_VALUE - dwPriority;    //Final priority value.

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	bResult = lpSystem->lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(__COMMON_OBJECT*)lpTimerObject,
		dwPriority);
	if(!bResult)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	dwPriority = MAX_DWORD_VALUE - dwPriority;    //Now,dwPriority countains the next timer
	                                              //tick value.
	if((System.dwNextTimerTick > dwPriority) || (System.dwNextTimerTick == 0))
	{
		System.dwNextTimerTick = dwPriority;    //Update the next timer tick counter.
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	if(!bResult)
	{
		if(lpTimerObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpTimerObject);
			lpTimerObject = NULL;
		}
	}
	return (__COMMON_OBJECT*)lpTimerObject;
}

//
//kCancelTimer implementation.
//This routine is used to cancel timer.
//
static VOID kCancelTimer(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpTimer)
{
	__SYSTEM*                  lpSystem       = NULL;
	DWORD                      dwPriority     = 0;
	DWORD                      dwFlags;
	__TIMER_OBJECT*            lpTimerObject  = NULL;

	if((NULL == lpThis) || (NULL == lpTimer))
	{
		return;
	}

	lpSystem = (__SYSTEM*)lpThis;
	//if(((__TIMER_OBJECT*)lpTimer)->dwTimerFlags != TIMER_FLAGS_ALWAYS)
	//	return;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpSystem->lpTimerQueue->DeleteFromQueue((__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		lpTimer);
	lpTimerObject = (__TIMER_OBJECT*)
		lpSystem->lpTimerQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		&dwPriority);
	if(NULL == lpTimerObject)    //There is not any timer object to be processed.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __DESTROY_TIMER;
	}

	//
	//The following code updates the tick counter that timer object should be processed.
	//
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	if(dwPriority > lpSystem->dwNextTimerTick)
		lpSystem->dwNextTimerTick = dwPriority;
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	lpSystem->lpTimerQueue->InsertIntoQueue(
		(__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(__COMMON_OBJECT*)lpTimerObject,
		dwPriority);    //Insert into timer object queue.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__DESTROY_TIMER:  //Destroy the timer object.
	ObjectManager.DestroyObject(&ObjectManager,
		lpTimer);

	return;
}

//Hardware platform initialization routine,implemented in arch_xxx.c file and will be called
//in BeginInitialize routine.
extern BOOL HardwareInitialize(void);

//Called before the OS enter initialization phase.It prepares the initialization evnironment
//to run initializing code.Also hardware initialization routine will be called in this routine,
//which is BeginSysInitialize implemented in arch_xxx.c file.
static BOOL BeginInitialize(__COMMON_OBJECT* lpThis)
{
	System.bSysInitialized = FALSE;
	//Call hardware initialization routine.
	if(!HardwareInitialize())
	{
		return FALSE;
	}
	//Interrupt must be disabled in OS initialization process.
	__DISABLE_INTERRUPT();
	return TRUE;
}

//Called after the OS finish initialization.
static BOOL EndInitialize(__COMMON_OBJECT* lpThis)
{
	System.bSysInitialized = TRUE;
	__ENABLE_INTERRUPT();

	//In process of PC's loading,there may one or several interrupt(s) occurs,
	//which is not handled by OS since it's core data structure is not established
	//yet,the interrupt(s) may pending on interrupt controller to acknowledge,
	//and this may lead system halt.
	//So we add more of enabling interrupt operations to dismiss the pending
	//interrupt(s) here.
#ifdef __I386__
	__ENABLE_INTERRUPT();
	__ENABLE_INTERRUPT();
	__ENABLE_INTERRUPT();
#endif
	return TRUE;
}

/***************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/

//The definition of system object.

__SYSTEM System = {
	{0},                      //lpInterruptVector[MAX_INTERRUPT_VECTOR].
	NULL,                     //lpTimerQueue.
	0,                        //dwClockTickCounter,
	0,                        //dwNextTimerTick,
	0,                        //ucIntNestLeve;
	0,                        //bSysInitialized;
	0,
	0,                        //ucReserved3;
	0,                        //dwPhysicalMemorySize,
	BeginInitialize,          //BeginInitialize,
	EndInitialize,            //EndInitialize,
    SystemInitialize,         //Initialize routine.
	GetClockTickCounter,      //GetClockTickCounter routine.
	GetSysTick,               //GetSysTick routine.
	GetPhysicalMemorySize,    //GetPhysicalMemorySize routine.
	DispatchInterrupt,        //DispatchInterrupt routine.
	DispatchException,        //DispatchException routine.
	kConnectInterrupt,         //kConnectInterrupt.
	kDiskConnectInterrupt,      //kDiskConnectInterrupt.
	kSetTimer,                 //kSetTimerRoutine.
	kCancelTimer
};

//***************************************************************************************
//
//             General Interrupt Handler
//
//***************************************************************************************

//
//GeneralIntHandler.
//This routine is the general handler of all interrupts.
//Once an interrupt occurs,the low layer code (resides in Mini-Kernel) calls this routine,
//this routine then calls DispatchInterrupt of system object.
//

VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp)
{
	//PrintLine("GeneralIntHandler");
	UCHAR    ucVector = (BYTE)(dwVector);

	if(IS_EXCEPTION(ucVector))  //Exception.
	{
		System.DispatchException((__COMMON_OBJECT*)&System,
			lpEsp,
			ucVector);
		return;
	}
	//Interrupt,dispatch it by DispatchInterrupt routine.
	System.DispatchInterrupt((__COMMON_OBJECT*)&System,
		lpEsp,
		ucVector);
}
