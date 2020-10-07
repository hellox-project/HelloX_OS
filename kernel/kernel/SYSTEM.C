//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,06 2004
//    Module Name               : system.cpp
//    Module Funciton           : 
//                                OS kernel system mechanism.
//    Last modified Author      : Garry.Xin
//    Last modified Date        : 2011/12/16
//    Last modified Content     : 1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "system.h"
#include "types.h"
#include "../syscall/syscall.h"
#include "stdio.h"
#include "ktmsg.h"
#include "hellocn.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#include "../arch/x86/pic.h"
#endif

/* Appended part of system source file. */
#include "system2.c"

/* Performance recorder object used to mesure the performance of timer interrupt. */
__PERF_RECORDER  TimerIntPr = {
	U64_ZERO,
	U64_ZERO,
	U64_ZERO,
	U64_ZERO
};

/* Local helper routine to process a timer object accordingly. */
static void __ProcessTimerObject(__TIMER_OBJECT* lpTimerObject)
{
	__KERNEL_THREAD_MESSAGE Msg;

	/* 
	 * Just send a message to the kernel thread 
	 * who set this timer object
	 * if no direct handler specified.
	 */
	if (NULL == lpTimerObject->DirectTimerHandler)
	{
		Msg.wCommand = KERNEL_MESSAGE_TIMER;
		Msg.dwParam = lpTimerObject->dwTimerID;
		KernelThreadManager.SendMessage(
			(__COMMON_OBJECT*)lpTimerObject->lpKernelThread,
			&Msg);
	}
	else
	{
		/*
		* Call the associated handler,the handler must be small enough
		* and can not block or trigger kernel thread switching.It should
		* be used as less as possible.
		*/
		lpTimerObject->DirectTimerHandler(lpTimerObject->lpHandlerParam);
	}
}

/*
 * TimerInterruptHandler routine.
 * The following routine is the most CRITICAL routine of kernel of Hello China.
 * The routine does the following:
 *  1. Schedule timer object;
 *  2. Update the system level variables,such as dwClockTickCounter;
 *  3. Schedule kernel thread(s).
 */
static BOOL __TimerInterruptHandler(LPVOID lpEsp,LPVOID lpParam)
{
	DWORD dwPriority = 0;
	__TIMER_OBJECT* lpTimerObject = 0;
	__PRIORITY_QUEUE* lpTimerQueue = NULL;
	__PRIORITY_QUEUE* lpSleepingQueue = NULL;
	__KERNEL_THREAD_OBJECT* lpKernelThread = NULL;
	DWORD dwFlags = 0;
	
	BUG_ON(NULL == lpEsp);

	/* 
	 * Schedule timers.
	 * Use system object's lock to protect this process since
	 * the timer list is shared by all CPUs in SMP environment.
	 */
	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	if(System.dwClockTickCounter == System.dwNextTimerTick)
	{
		lpTimerQueue = System.lpTimerQueue;
		lpTimerObject = (__TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpTimerQueue,
			&dwPriority);
		if(NULL == lpTimerObject)
		{
			__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
			goto __WAKEUP_KERNEL_THREAD;
		}
		dwPriority = MAX_DWORD_VALUE - dwPriority;
		/* 
		 * Strictly speaking,the dwPriority variable must
		 * EQUAL System.dwNextTimerTick,
		 * but in the implementing of the current 
		 * version,there may be some error exists,
		 * so we assume dwPriority equal or less 
		 * than dwNextTimerTic.
		 */
		while(dwPriority <= System.dwNextTimerTick)
		{
#if 0
			/* Just send a message to the kernel thread if no direct handler specified. */
			if(NULL == lpTimerObject->DirectTimerHandler)
			{
				Msg.wCommand = KERNEL_MESSAGE_TIMER;
				Msg.dwParam  = lpTimerObject->dwTimerID;
				KernelThreadManager.SendMessage(
					(__COMMON_OBJECT*)lpTimerObject->lpKernelThread,
					&Msg);
			}
			else
			{
				/* 
				 * Call the associated handler,the handler must be small enough
				 * and can not block or trigger kernel thread switching.It should
				 * be used as less as possible.
				 */
				lpTimerObject->DirectTimerHandler(lpTimerObject->lpHandlerParam);
			}
#endif

			if (lpTimerObject->dwTimerFlags & TIMER_FLAGS_ONCE)
			{
				/* One shot and always mode are exclusive. */
				BUG_ON(lpTimerObject->dwTimerFlags & TIMER_FLAGS_ALWAYS);
				if (lpTimerObject->dwTimerFlags & TIMER_FLAGS_NOAUTODEL)
				{
					if (0 == (lpTimerObject->dwTimerFlags & TIMER_FLAGS_FIRED))
					{
						__ProcessTimerObject(lpTimerObject);
						/* Set the already processed flag. */
						lpTimerObject->dwTimerFlags |= TIMER_FLAGS_FIRED;
					}
					/* 
					 * Save it into queue so as CancelTimer can delete it,use 0 as 
					 * priority so as the object is inserted in end of queue.
					 */
					lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpTimerQueue,
						(__COMMON_OBJECT*)lpTimerObject,
						0);
				}
				else
				{
					__ProcessTimerObject(lpTimerObject);
					/* Delete the timer object processed if no auto delete flag is set. */
					ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpTimerObject);
				}
			}
			else if (lpTimerObject->dwTimerFlags & TIMER_FLAGS_ALWAYS)
			{
				/* Process the timer object. */
				__ProcessTimerObject(lpTimerObject);
				/* Re-insert the timer object into timer queue. */
				dwPriority = lpTimerObject->dwTimeSpan;
				dwPriority /= SYSTEM_TIME_SLICE;
				dwPriority += System.dwClockTickCounter;
				dwPriority = MAX_DWORD_VALUE - dwPriority;
				lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpTimerQueue,
					(__COMMON_OBJECT*)lpTimerObject,
					dwPriority);
			}

			/* Check another timer object. */
			lpTimerObject = (__TIMER_OBJECT*)lpTimerQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpTimerQueue,
				&dwPriority);
			if(NULL == lpTimerObject)
			{
				break;
			}
			dwPriority = MAX_DWORD_VALUE - dwPriority;
		}

		/* There is no timer object in queue. */
		if(NULL == lpTimerObject)
		{
			System.dwNextTimerTick = 0;
		}
		else
		{
			/* Update the next timer tick counter. */
			System.dwNextTimerTick = dwPriority;
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpTimerQueue,
				(__COMMON_OBJECT*)lpTimerObject,
				dwPriority);
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

__WAKEUP_KERNEL_THREAD:
	/*
	 * Wakes up all kernel thread(s) whose status is SLEEPING and
	 * the time it(then) set is out.
	 */
	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	/* 
	 * Acquire KernelThreadManager's spin lock since it's 
	 * members will also be updated. 
	 */
	__ACQUIRE_SPIN_LOCK(KernelThreadManager.spin_lock);
	if(System.dwClockTickCounter == KernelThreadManager.dwNextWakeupTick)
	{
		lpSleepingQueue = KernelThreadManager.lpSleepingQueue;
		lpKernelThread  = (__KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
			(__COMMON_OBJECT*)lpSleepingQueue,
			&dwPriority);
		while(lpKernelThread)
		{
			/* Calculate the desired wake up tick counter. */
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			if(dwPriority > System.dwClockTickCounter)
			{
				/* No kernel thread need to wake up. */
				break;
			}
			/* Insert the waked up kernel thread into ready queue. */
			__ACQUIRE_SPIN_LOCK(lpKernelThread->spin_lock);
			lpKernelThread->dwThreadStatus = KERNEL_THREAD_STATUS_READY;
			KernelThreadManager.AddReadyKernelThread(
				(__COMMON_OBJECT*)&KernelThreadManager,
				lpKernelThread);
			__RELEASE_SPIN_LOCK(lpKernelThread->spin_lock);
			/* Check next kernel thread in sleeping queue. */
			lpKernelThread = (__KERNEL_THREAD_OBJECT*)lpSleepingQueue->GetHeaderElement(
				(__COMMON_OBJECT*)lpSleepingQueue,
				&dwPriority);
		}
		if(NULL == lpKernelThread)
		{
			KernelThreadManager.dwNextWakeupTick = 0;
		}
		else
		{
			KernelThreadManager.dwNextWakeupTick = dwPriority;
			dwPriority = MAX_DWORD_VALUE - dwPriority;
			lpSleepingQueue->InsertIntoQueue((__COMMON_OBJECT*)lpSleepingQueue,
				(__COMMON_OBJECT*)lpKernelThread,
				dwPriority);
		}
	}

	/* Update the system clock interrupt counter. */
	System.dwClockTickCounter++;
	__RELEASE_SPIN_LOCK(KernelThreadManager.spin_lock);
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

	return TRUE;
}

/*
 * ConnectInterrupt routine of Interrupt Object.
 * The routine do the following:
 *  1. Insert the current object into interrupt object 
 *     array(maintenanced by system object);
 *  2. Set the object's data members correctly.
 */
static __COMMON_OBJECT* __ConnectInterrupt(__COMMON_OBJECT* lpThis,
	__INTERRUPT_HANDLER lpInterruptHandler,
	LPVOID lpHandlerParam,
	UCHAR ucVector,
	UCHAR ucReserved1,
	UCHAR ucReserved2,
	UCHAR ucInterruptMode,
	BOOL bIfShared,
	DWORD dwCPUMask)
{
	__INTERRUPT_OBJECT*      lpInterrupt          = NULL;
	__INTERRUPT_OBJECT*      lpObjectRoot         = NULL;
	__SYSTEM*                lpSystem             = &System;
	DWORD                    dwFlags              = 0;

	/* Validate mandatory parameters. */
	if((NULL == lpThis) || (NULL == lpInterruptHandler))
	{
		return NULL;
	}

	/* Interrupt vector should not out of bound. */
	if(ucVector >= MAX_INTERRUPT_VECTOR)
	{
		return NULL;
	}

	/* Create and initialize an interrupt object. */
	lpInterrupt = (__INTERRUPT_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_INTERRUPT);
	if(NULL == lpInterrupt)
	{
		return FALSE;
	}
	if(!lpInterrupt->Initialize((__COMMON_OBJECT*)lpInterrupt))
	{
		return FALSE;
	}

	/* Set the mandatory member of interrupt object. */
	lpInterrupt->lpPrevInterruptObject = NULL;
	lpInterrupt->lpNextInterruptObject = NULL;
	lpInterrupt->ucVector = ucVector;
	lpInterrupt->InterruptHandler = lpInterruptHandler;
	lpInterrupt->lpHandlerParam = lpHandlerParam;

	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	lpObjectRoot = lpSystem->InterruptSlotArray[ucVector].lpFirstIntObject;
	if(NULL == lpObjectRoot)
	{
		/* this is the first interrupt object of the vector. */
		System.InterruptSlotArray[ucVector].lpFirstIntObject = lpInterrupt;
	}
	else
	{
		lpInterrupt->lpNextInterruptObject  = lpObjectRoot;
		lpObjectRoot->lpPrevInterruptObject = lpInterrupt;
		System.InterruptSlotArray[ucVector].lpFirstIntObject = lpInterrupt;
	}
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

	/* Clear the corresponding mask bit. */
	__Clear_Interrupt_Mask(ucVector);

	return (__COMMON_OBJECT*)lpInterrupt;
}

/* DisconnectInterrupt,detach a specified interrupt from system and destroy it. */
static VOID __DisconnectInterrupt(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpInterrupt)
{
	__INTERRUPT_OBJECT* lpIntObject = NULL;
	__SYSTEM* lpSystem = NULL;
	UCHAR ucVector = 0;
	DWORD dwFlags = 0;

	/* Parameters checking. */
	if((NULL == lpThis) || (NULL == lpInterrupt))
	{
		return;
	}

	lpSystem = (__SYSTEM*)lpThis;
	lpIntObject = (__INTERRUPT_OBJECT*)lpInterrupt;
	ucVector  = lpIntObject->ucVector;

	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	if(NULL == lpIntObject->lpPrevInterruptObject)
	{
		/* This is the first interrupt object on the vector list. */
		lpSystem->InterruptSlotArray[ucVector].lpFirstIntObject = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject)
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = NULL;
		}
	}
	else
	{
		lpIntObject->lpPrevInterruptObject->lpNextInterruptObject = lpIntObject->lpNextInterruptObject;
		if(NULL != lpIntObject->lpNextInterruptObject)
		{
			lpIntObject->lpNextInterruptObject->lpPrevInterruptObject = lpIntObject->lpPrevInterruptObject;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	return;
}

/* Initializer of interrupt object. */
BOOL InterruptInitialize(__COMMON_OBJECT* lpThis)
{
	__INTERRUPT_OBJECT* lpInterrupt = NULL;

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

/* Destructor of interrupt object. */
BOOL InterruptUninitialize(__COMMON_OBJECT* lpThis)
{
	return TRUE;
}

/* Initializer of timer object. */
BOOL TimerInitialize(__COMMON_OBJECT* lpThis)
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
	/* Set object signature. */
	lpTimer->dwObjectSignature = KERNEL_OBJECT_SIGNATURE;

	return TRUE;
}

/* Destructor of timer object. */
BOOL TimerUninitialize(__COMMON_OBJECT* lpThis)
{
	__TIMER_OBJECT* pTimer = (__TIMER_OBJECT*)lpThis;
	BUG_ON(NULL == pTimer);
	/* Reset object signature. */
	pTimer->dwObjectSignature = 0;
	return TRUE;
}

/*
 * Initializing routine of system object.
 * The routine do the following:
 *  1. Create a priority queue,to be used as lpTimerQueue,countains the timer object;
 *  2. Create an interrupt object,as TIMER interrupt object;
 *  3. Initialize system level variables,such as dwPhysicalMemorySize,etc.
 */
static BOOL SystemInitialize(__COMMON_OBJECT* lpThis)
{
	__SYSTEM* lpSystem = (__SYSTEM*)lpThis;
	__PRIORITY_QUEUE* lpPriorityQueue = NULL;
	__INTERRUPT_OBJECT* lpIntObject = NULL;
#ifdef __CFG_SYS_SYSCALL
	__INTERRUPT_OBJECT* lpExpObject = NULL;
#endif
	BOOL bResult = FALSE;
	DWORD dwFlags = 0;

	BUG_ON(NULL == lpSystem);

	/* Create timer list(queue) and initialize it. */
	lpPriorityQueue = (__PRIORITY_QUEUE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_PRIORITY_QUEUE);
	if(NULL == lpPriorityQueue)
	{
		return FALSE;
	}
	if(!lpPriorityQueue->Initialize((__COMMON_OBJECT*)lpPriorityQueue))
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
	lpIntObject->InterruptHandler = __TimerInterruptHandler;

#ifdef __CFG_SYS_SYSCALL
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

	//Register all system calls into call array.
	RegisterSysCallEntry();
#endif

	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock,dwFlags);
	lpSystem->InterruptSlotArray[INTERRUPT_VECTOR_TIMER].lpFirstIntObject = lpIntObject;
#ifdef __CFG_SYS_SYSCALL
	lpSystem->InterruptSlotArray[EXCEPTION_VECTOR_SYSCALL].lpFirstIntObject = lpExpObject;
#endif
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

	/* Clear timer interrupt's mask. */
	__Clear_Interrupt_Mask(INTERRUPT_VECTOR_TIMER);
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
#ifdef __CFG_SYS_SYSCALL
		if(lpExpObject != NULL)
		{
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpExpObject);
		}
#endif
	}
	return bResult;
}

/* Return current tick counter. */
static DWORD _GetClockTickCounter(__COMMON_OBJECT* lpThis)
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

/* Get the physical memory's size. */
static DWORD GetPhysicalMemorySize(__COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)
	{
		return 0;
	}
	return ((__SYSTEM*)lpThis)->dwPhysicalMemorySize;
}

/*
 * This routine is the default interrupt handler.
 * If no entity(such as device driver) install an interrupt handler,this handler
 * will be called to handle the appropriate interrupt.
 * It will only print out a message indicating the interrupt's number and return.
 */
static VOID DefaultIntHandler(LPVOID lpEsp,UCHAR ucVector)
{
	CHAR strBuffer[64];
	static DWORD dwTotalNum    = 0;

	/* Record this unhandled exception or interrupt. */
	dwTotalNum ++;

	_hx_printk("  Unhandled interrupt.");
	_hx_sprintf(strBuffer,"  Interrupt number = %d.",ucVector);
	_hx_printk(strBuffer);
	_hx_sprintf(strBuffer,"  Total unhandled interrupt times = %d.",dwTotalNum);
	_hx_printk(strBuffer);

	return;
}

/* 
 * Dispatch interrupt to it's handler,according to the 
 * interrupt's vector value.
 * lpEsp points to the stack top of current kernel thread.
 */
static VOID DispatchInterrupt(__COMMON_OBJECT* lpThis,
	LPVOID lpEsp,
	UCHAR ucVector)
{
	__INTERRUPT_OBJECT* lpIntObject  = NULL;
	__SYSTEM* lpSystem = (__SYSTEM*)lpThis;
	BOOL bServiced = FALSE;
#if defined(__CFG_SYS_SMP)
	__INTERRUPT_CONTROLLER* pIntCtrl = NULL;
	__LOGICALCPU_SPECIFIC* pSpec = NULL;
#endif
	unsigned int processor_id = __CURRENT_PROCESSOR_ID;

	BUG_ON((NULL == lpThis) || (NULL == lpEsp));

#if defined(__CFG_SYS_SMP)
	/* 
	 * Get the corresponding interrupt controller object from current 
	 * processor's specific information.
	 */
	pSpec = ProcessorManager.GetCurrentProcessorSpecific();
	BUG_ON(NULL == pSpec);
	pIntCtrl = pSpec->pIntCtrl;
	BUG_ON(NULL == pIntCtrl);
#endif

#if 0
	/* Just for debugging. */
	if (!__CURRENT_PROCESSOR_IS_BSP())
	{
		pIntCtrl->AckInterrupt(pIntCtrl, ucVector);
		return;
	}
#endif

	lpSystem->ucIntNestLevel[processor_id] += 1;    //Increment nesting level.
	if(lpSystem->ucIntNestLevel[processor_id] <= 1)
	{
		/* 
		 * Call the corresponding thread hook here,since current kernel thread is
		 * interrupted.
		 * If interrupt occurs before any kernel thread is scheduled,
		 * CurrentKernelThread is NULL.
		 */
		if(__CURRENT_KERNEL_THREAD)
		{
			KernelThreadManager.CallThreadHook(
				THREAD_HOOK_TYPE_ENDSCHEDULE,
				__CURRENT_KERNEL_THREAD,
				NULL);
		}
	}
	else
	{
#ifdef __CFG_SYS_INTNEST  //Interupt nest is enabled.
		//Do nothing.
#else
		/* It's a BUG if interrupt nest is disabled. */
		_hx_printk("Fatal error,interrupt nested(enter-int,vector = %d,nestlevel = %d)\r\n",
			ucVector,
			lpSystem->ucIntNestLevel[processor_id]);
		BUG();
#endif
	}

	/* 
	 * Get the corresponding interrupt object to handle current interrupt. 
	 * Just call default interrupt handler if no interrupt object installed.
	 */
	lpIntObject = lpSystem->InterruptSlotArray[ucVector].lpFirstIntObject;
	if(NULL == lpIntObject)
	{
		DefaultIntHandler(lpEsp,ucVector);
		//Increment the total interrupt counter.
		lpSystem->InterruptSlotArray[ucVector].dwTotalInt ++;
		goto __RETFROMINT;
	}

	/* Travel the whole interrupt list of this vector. */
	while(lpIntObject)
	{
		/* Call the corresponding handler. */
		if(lpIntObject->InterruptHandler(lpEsp,lpIntObject->lpHandlerParam))
		{
			/* The interrupt is serviced. */
			bServiced = TRUE;
			lpSystem->InterruptSlotArray[ucVector].dwSuccHandledInt ++;
		}
		lpSystem->InterruptSlotArray[ucVector].dwTotalInt++;
		/* 
		 * Call next interrupt object,make sure any interrupt 
		 * object has same chance to be called.
		 */
		lpIntObject = lpIntObject->lpNextInterruptObject;
	}
	if (!bServiced)
	{
		/* 
		 * No interrupt object service the interrupt,
		 * just bounce to default handler. 
		 */
		DefaultIntHandler(lpEsp, ucVector);
	}

__RETFROMINT:
	lpSystem->ucIntNestLevel[processor_id] -= 1;
	/* Acknowledge the interrupt. */
#if defined(__CFG_SYS_SMP)
	if (!pIntCtrl->AckInterrupt(pIntCtrl, ucVector))
	{
		/* Interrupt raise from default controller. */
		__AckDefaultInterrupt();
	}
#else
	/* Just use the default acknowledge routine. */
	__AckDefaultInterrupt();
#endif
	if(0 == lpSystem->ucIntNestLevel[processor_id])
	{
		if (IN_SYSINITIALIZATION())
		{
			/* 
			 * Just show out a warning message when interrupts raise in 
			 * procedure of system initialization. 
			 */
			_hx_printk("Warning: Interrupt[%d] raised in sys initialization.", ucVector);
		}
		else
		{
			if (NULL != __CURRENT_KERNEL_THREAD)
			{
				/* Reschedule kernel thread. */
				KernelThreadManager.ScheduleFromInt((__COMMON_OBJECT*)&KernelThreadManager,
					lpEsp);
			}
		}
	}
	else
	{
#ifdef __CFG_SYS_INTNEST  //Interrupt nest is enabled.
		//Do nothing.
#else
		_hx_printf("Fatal error,interrupt nested(leave-int,vector = %d,nestlevel = %d)\r\n",
			ucVector,
			lpSystem->ucIntNestLevel[processor_id]);
		BUG();
#endif  //__CFG_SYS_INTNEST
	}
	return;
}

/*
 * Default exception handler.
 * Show out some debugging information and dive to dead loop,since
 * the unknown exception will lead system crash in most case.
 */
static VOID DefaultExcepHandler(LPVOID pESP, UCHAR ucVector)
{
	__KERNEL_THREAD_OBJECT* pKernelThread = __CURRENT_KERNEL_THREAD;
	DWORD dwFlags;
	static unsigned long totalExcepNum = 0;
	unsigned int processor_id = __CURRENT_PROCESSOR_ID;

	/* 
	 * Switch to text mode,because the exception maybe 
	 * raise in GUI mode. 
	 */
#ifdef __I386__
	SwitchToText();
#endif
	_hx_printf("  Exception : [#%d] on processor [%d]\r\n", ucVector, processor_id);
	totalExcepNum++;

	/*
	 * Show kernel thread information which lead the exception,if
	 * the context when exception raise is in kernel thread.
	 */
	if (pKernelThread)
	{
		_hx_printf("  Current kthread ID: %d.\r\n", pKernelThread->dwThreadID);
		_hx_printf("  Current kthread name: %s.\r\n", pKernelThread->KernelThreadName);
	}
	else
	{
		/* In process of system initialization. */
		if (IN_SYSINITIALIZATION())
		{
			_hx_printf("  Exception raise in sysinit.\r\n");
		}
		else
		{
			_hx_printf("  Current thread is NULL.\r\n");
		}
	}

	/* Call processor specific exception handler. */
	PSExcepHandler(pESP, ucVector);

	/* If exception is caused by user thread. */
	if (pKernelThread && THREAD_IN_USER_MODE(pKernelThread) && !IN_INTERRUPT())
	{
		/* Kill the user thread. */
		pKernelThread->ulThreadFlags |= KERNEL_THREAD_FLAGS_KILLED;
		KernelThreadManager.ScheduleFromInt((__COMMON_OBJECT*)&KernelThreadManager,
			pESP);
	}
	else
	{
		/* Kernel error, just halt system. */
		if (totalExcepNum >= 1)
		{
			_hx_printf("  *** *** ***\r\n");
			_hx_printf("  Fatal error:exception in kernel.\r\n");
			_hx_printf("  No other choice but reboot system.\r\n");
			__DISABLE_LOCAL_INTERRUPT(dwFlags);
			while (1)
			{
				HaltSystem();
			}
		}
	}
	return;
}

/* 
 * DispatchException,called by GeneralIntHandler to handle exception,
 * include system calls.
 */
static VOID DispatchException(__COMMON_OBJECT* lpThis,
	LPVOID lpEsp,
	UCHAR ucVector)
{
	__SYSTEM* lpSystem  = (__SYSTEM*)lpThis;
	__INTERRUPT_OBJECT* lpIntObj  = NULL;

	BUG_ON(NULL == lpSystem);
	/* Not an exception. */
	BUG_ON(!IS_EXCEPTION(ucVector));

	/* Locate the handler to handle the exception. */
	lpIntObj = lpSystem->InterruptSlotArray[ucVector].lpFirstIntObject;
	if(NULL == lpIntObj)
	{
		/* Call default exception handler if no one installed. */
		DefaultExcepHandler(lpEsp,ucVector);
		//Update exception counter.
		lpSystem->InterruptSlotArray[ucVector].dwTotalInt ++;
		return;
	}
	/* Call the exception handler now.
	 * For each exception,only one handler present,
	 * it's different from interrupt since there maybe
	 * many interrupt handlers exist for one interrupt vector.
	 */
	lpIntObj->InterruptHandler(lpEsp,lpIntObj->lpHandlerParam);
	lpSystem->InterruptSlotArray[ucVector].dwTotalInt ++;
	lpSystem->InterruptSlotArray[ucVector].dwSuccHandledInt ++;
	return;
}

/*
 * Setup a timer.
 * The routine do the following:
 *  1. Create a timer object;
 *  2. Initialize the timer object;
 *  3. Insert into the timer object into timer queue of system object;
 *  4. Return the timer object's base address if all successfully.
 */
static __COMMON_OBJECT* __SetTimer(__COMMON_OBJECT* lpThis,
	__KERNEL_THREAD_OBJECT* lpKernelThread,
	DWORD dwTimerID,
	DWORD dwTimeSpan,
	__DIRECT_TIMER_HANDLER lpHandler,
	LPVOID lpHandlerParam,
	DWORD dwTimerFlags)
{
	__SYSTEM* lpSystem = NULL;
	__TIMER_OBJECT* lpTimerObject = NULL;
	BOOL bResult = FALSE;
	DWORD dwPriority = 0;
	DWORD dwFlags = 0;

	/* Check mandatory parameters. */
	if((NULL == lpThis) || (NULL == lpKernelThread))
	{
		return NULL;
	}

	/* Check flags value. */
	if (!((dwTimerFlags & TIMER_FLAGS_ONCE) || (dwTimerFlags & TIMER_FLAGS_ALWAYS)))
	{
		return NULL;
	}
	if ((dwTimerFlags & TIMER_FLAGS_ONCE) && (dwTimerFlags & TIMER_FLAGS_ALWAYS))
	{
		return NULL;
	}

	/* At least one time slice is required for timer object. */
	if (dwTimeSpan == 0)
	{
		return NULL;
	}
	if(dwTimeSpan <= SYSTEM_TIME_SLICE)
	{
		dwTimeSpan = SYSTEM_TIME_SLICE;
	}

	/* Create and initialize a timer object. */
	lpSystem = (__SYSTEM*)lpThis;
	lpTimerObject = (__TIMER_OBJECT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_TIMER);
	if(NULL == lpTimerObject)
	{
		goto __TERMINAL;
	}
	bResult = lpTimerObject->Initialize((__COMMON_OBJECT*)lpTimerObject);
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

	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	/* Calculates the priority value of the timer object in timer queue(list). */
	dwPriority = dwTimeSpan;
	dwPriority /= SYSTEM_TIME_SLICE;
	dwPriority += lpSystem->dwClockTickCounter;
	/* 
	 * Now dwPriority countains the tick counter on which this
	 * timer must be processed. 
	 * But the object queue's sort is descending,so we use the 
	 * complement value of dwPriority as priority when put into queue.
	 */
	dwPriority = MAX_DWORD_VALUE - dwPriority;

	/* Insert into timer list(queue). */
	bResult = lpSystem->lpTimerQueue->InsertIntoQueue((__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(__COMMON_OBJECT*)lpTimerObject,
		dwPriority);
	if(!bResult)
	{
		__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Update the next timer tick counter,if necessary. */
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	if((System.dwNextTimerTick > dwPriority) || (System.dwNextTimerTick == 0))
	{
		System.dwNextTimerTick = dwPriority;
	}
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

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

/*
 * Cancel a kernel timer.The return value indicates if
 * the timer is cancelled successfully,since the timer
 * maybe triggered and destroyed before this routine is
 * called.
 */
static BOOL __CancelTimer(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpTimer)
{
	__SYSTEM* lpSystem = NULL;
	DWORD dwPriority = 0;
	DWORD dwFlags;
	__TIMER_OBJECT* lpTimerObject = NULL;
	BOOL bDestroyed = FALSE;

	BUG_ON((NULL == lpThis) || (NULL == lpTimer));
	/* Validate the timer object. */
	if (KERNEL_OBJECT_SIGNATURE != ((__TIMER_OBJECT*)lpTimer)->dwObjectSignature)
	{
		return FALSE;
	}
	
	lpSystem = (__SYSTEM*)lpThis;
	__ENTER_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
	if (!lpSystem->lpTimerQueue->DeleteFromQueue(
		(__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		lpTimer))
	{
		/* 
		 * Can not find the timer object in queue,it maybe 
		 * destroyed before this routine is called,maybe deleted in
		 * timer interrupt handler.
		 */
		bDestroyed = TRUE;
	}
	lpTimerObject = (__TIMER_OBJECT*)
		lpSystem->lpTimerQueue->GetHeaderElement(
		(__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		&dwPriority);
	if(NULL == lpTimerObject)
	{
		/* Reset next timer tick counter since no timer in queue. */
		lpSystem->dwNextTimerTick = 0;
		__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);
		goto __DESTROY_TIMER;
	}

	/* Updates the tick counter that timer object should be processed. */
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	if(dwPriority > lpSystem->dwNextTimerTick)
		lpSystem->dwNextTimerTick = dwPriority;
	dwPriority = MAX_DWORD_VALUE - dwPriority;
	lpSystem->lpTimerQueue->InsertIntoQueue(
		(__COMMON_OBJECT*)lpSystem->lpTimerQueue,
		(__COMMON_OBJECT*)lpTimerObject,
		dwPriority);
	__LEAVE_CRITICAL_SECTION_SMP(System.spin_lock, dwFlags);

	/* Destroy the timer object. */
__DESTROY_TIMER:
	if (!bDestroyed)
	{
		ObjectManager.DestroyObject(&ObjectManager, lpTimer);
	}
	return (!bDestroyed);
}

/*
 * Called before the OS enter initialization phase.
 * It prepares the initialization evnironment
 * to run initializing code,such as set the 
 * initialized flags to FALSE,disable interrupt,and
 * other essential preparation.
 * This routine must be called at the begining 
 * of OS_Entry routine.
 */
static BOOL BeginInitialize(__COMMON_OBJECT* lpThis)
{
	System.bSysInitialized = FALSE;

	/* 
	 * Local interrupt must be disabled in 
	 * process of system initialization.
	 */
	__DISABLE_INTERRUPT();
	return TRUE;
}

/* 
 * Architecture specific initialization routine,
 * implemented in arch_xxx.c file and will be called
 * in HardwareInitialize routine.
 * We make it invisble anywhere except here,
 * by declaring it as external and 
 * do not put it into any header file,to guarantee 
 * the routine can not be invoked anywhere else.
 */
extern BOOL __HardwareInitialize(void);

/* Will be called after the OS's initialization. */
extern BOOL __EndHardwareInitialize();

/* 
 * Hardware initialization phase.
 * The arch specific hardware initialization routine,
 * __HardwareInitialize,will be called in this function.
 * HardwareInitialize routine must be implemented in 
 * arch specific source code,most case
 * resides in arch_xxx.c file.
 */
static BOOL HardwareInitialize(__COMMON_OBJECT* lpThis)
{
	if (!__HardwareInitialize())
	{
		return FALSE;
	}
	return TRUE;
}

//Called after the OS finish initialization.
static BOOL EndInitialize(__COMMON_OBJECT* lpThis)
{
	__SYSTEM* pSystem = (__SYSTEM*)lpThis;

	BUG_ON(NULL == pSystem);

#if defined(__CFG_SYS_SMP)
	/* 
	 * Mark the BSP initialized flag to 1,so other AP(s) 
	 * in system can start their initialization.
	 */
	pSystem->bspInitialized = 1;
#endif

	/* 
	 * Invoke the end initialization routine of arch. 
	 * AP(s) in system in case of SMP environment can
	 * start their initialization in this routine.
	 */
	if (!__EndHardwareInitialize())
	{
		return FALSE;
	}
	//System.bSysInitialized = TRUE;

#if 0
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
#endif
	return TRUE;
}

/* 
 * Return the interrupt vector's statistics 
 * information by giving a interrupt vector. 
 */
static BOOL _GetInterruptStat(__COMMON_OBJECT* lpThis, 
	UCHAR ucVector, 
	__INTERRUPT_VECTOR_STAT* pStat)
{
	__SYSTEM*           lpSystem = (__SYSTEM*)lpThis;
	__INTERRUPT_SLOT*   pIntSlot = NULL;
	__INTERRUPT_OBJECT* pIntObject = NULL;

	if (NULL == lpSystem)
	{
		return FALSE;
	}
	if (ucVector >= MAX_INTERRUPT_VECTOR)
	{
		return FALSE;
	}
	if (NULL == pStat)
	{
		return FALSE;
	}
	pIntSlot = &lpSystem->InterruptSlotArray[ucVector];
	pIntObject = pIntSlot->lpFirstIntObject;
	//Copy interrupt statistics information.
	pStat->dwSuccHandledInt = pIntSlot->dwSuccHandledInt;
	pStat->dwTotalInt       = pIntSlot->dwTotalInt;
	pStat->dwTotalIntObject = 0;
	while (pIntObject)
	{
		pStat->dwTotalIntObject ++;
		pIntObject = pIntObject->lpNextInterruptObject;
	}
	return TRUE;
}

/***************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/

/* Global system object. */
__SYSTEM System = {
	NULL,                     //lpTimerQueue.
	{0},                      //InterruptSlotArray[MAX_INTERRUPT_VECTOR].
	{0},                      //ucCurrInt[] array.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,     //spin_lock.
	0,                        //bspInitialized.
#endif 
	0,                        //dwClockTickCounter,
	0,                        //dwNextTimerTick,
	{0},                      //ucIntNestLeve[] array;
	0,                        //bSysInitialized;
	0,                        //ucReserved1;
	0,                        //ucReserved2;
	0,                        //dwPhysicalMemorySize,
	BeginInitialize,          //BeginInitialize,
	HardwareInitialize,       //HardwareInitialize,
	EndInitialize,            //EndInitialize,
    SystemInitialize,         //Initialize routine.
	_GetClockTickCounter,     //GetClockTickCounter routine.
	GetSysTick,               //GetSysTick routine.
	GetPhysicalMemorySize,    //GetPhysicalMemorySize routine.
	DispatchInterrupt,        //DispatchInterrupt routine.
	DispatchException,        //DispatchException routine.
	__ConnectInterrupt,       //ConnectInterrupt.
	__DisconnectInterrupt,   //DiskConnectInterrupt.
	__SetTimer,               //SetTimerRoutine.
	__CancelTimer,            //CancelTimer.
	_GetInterruptStat,        //GetInterruptStat.
	__GetSystemInfo           //GetSystemInfo.
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
	UCHAR ucVector = (BYTE)(dwVector);
	unsigned int processor_id = __CURRENT_PROCESSOR_ID;

	/* Save current interrupt or exception vector value. */
	System.ucCurrInt[processor_id] = ucVector;

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
