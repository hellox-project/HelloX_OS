//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 18 DEC, 2011
//    Module Name               : idle.cpp
//    Module Funciton           : 
//                                This module contains the IDLE thread implementation code.IDLE thread
//                                is one of the kernel level threads and will be scheduled when no any
//                                other thread need CPU.
//                                Auxiliary functions such as battery management will also be placed in
//                                this thread.
//                                These code lines are moved from os_entry.cpp file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include "idle.h"

#ifdef __I386__
//A keep alive function,to indicate user that the system is running well,but no
//active input is detected.
//The keyboard or mouse will not work in some rare case,the system seems halt from
//the user's side,this routine will show user that the system is OK.

//Detect if there is effective human input.
static BOOL DetectInput()
{
	__INTERRUPT_VECTOR_STAT ivs;

	//Check if there is key board interrupt,i.e,key board input.
	if (!System.GetInterruptStat((__COMMON_OBJECT*)&System,
		INTERRUPT_VECTOR_KEYBOARD, &ivs))
	{
		return TRUE;
	}
	if (ivs.dwSuccHandledInt >= 2)  //At lease 1 key strokes detected.
	{
		return TRUE;
	}
	//Check if there is mouse input.
	if (!System.GetInterruptStat((__COMMON_OBJECT*)&System,
		INTERRUPT_VECTOR_MOUSE, &ivs))
	{
		return TRUE;
	}
	if (ivs.dwSuccHandledInt >= 6)
	{
		return TRUE;
	}
	return FALSE;
}

//Print out alive message routinely.
static void ShowAlive()
{
	DWORD dwMillionSecond = 0;

	dwMillionSecond =  System.GetClockTickCounter((__COMMON_OBJECT*)&System);
	dwMillionSecond += 1;  //Skip the first 0 clock tick.
	dwMillionSecond *= SYSTEM_TIME_SLICE;
	if (0 == dwMillionSecond % SHOW_ALIVE_TIMESPAN)
	{
		if (DetectInput())  //User input detected.
		{
			return;
		}
		/* Show alive message if no input. */
		//_hx_printk("  CPU[%d]: System is alive,but no human input yet.\r\n",
		//	__CURRENT_PROCESSOR_ID);
	}
}
#endif

/* Intense computing algorithm to drive CPU busy. */
static int IntenseComputing(unsigned long loopCount)
{
	int i, j, l, k, m, jj;
	jj = 2342;
	k = 31455;
	l = 16452;
	m = 9823;
	i = 1000000;
	j = 9999;
#define RET_VALUE (i + j + k + m + l + jj)

	while (loopCount)
	{
		if (0 == m)
		{
			m = 9823;
		}
		m = m ^ l;
		k = (k / m * jj) % i;
		l = j * m * k;
		i = (j * k) ^ m;
		k = (k / m * jj) % i;
		m = m ^ l;
		m = m ^ l;
		i = (j * k) ^ m;
		k = (k / m * jj) % i;
		m = i * i * i * i * i * i * i; // m=k*l*jj*l;
		m = m ^ l;
		k = (k / m * jj) % i;
		l = j * m * k;
		i = (j * k) ^ m;
		l = (k / m * jj) % i;
		m = m ^ l;
		m = m ^ l;
		i = (j * k) ^ m;
		k = (k / m * jj) % i;
		m = k * k * k * k * k - m / i;

		loopCount--;
	}
	return RET_VALUE;
#undef RET_VALUE
}

/* Test kernel thread. */
#if 0
static DWORD __cdecl lazyPigThread(LPVOID pData)
{
	unsigned long ulCounter = 0;
	int accum = 0;
	while (TRUE)
	{
		if (ulCounter % 1000 == 0)
		{
			_hx_printk("Lazy pig on CPU[%d],counter[%d],accumulate[%d]\r\n",
				__CURRENT_PROCESSOR_ID,
				ulCounter,
				accum);
			Sleep(9000);
		}
		ulCounter++;
		accum += IntenseComputing(0xFFFF);
	}
	return 0;
}
#endif

#define __DEBUG_EVENT 1

/* Mailbox object used to communication each other. */
static __MAIL_BOX* globalMailbox = NULL;

/* Event object used to communicate with each other. */
#if __DEBUG_EVENT
static __EVENT* eventObject = NULL;
#endif

/* Local message. */
typedef struct tag__LAZYPIG_MESSAGE{
	unsigned long processorID;
	unsigned long times;
}__LAZYPIG_MESSAGE;

#if 0
static DWORD __cdecl lazyPigThread(LPVOID pData)
{
	unsigned long ulCounter = 0;
	MSG msg;
	int accum = 0;
	HANDLE hTimer = NULL;
	__LAZYPIG_MESSAGE* pMsg = NULL;

	/* Set a timer. */
	hTimer = SetTimer(2048, 5, NULL, NULL, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer)
	{
		_hx_printk("Failed to set timer on CPU[%d]\r\n",
			__CURRENT_PROCESSOR_ID);
	}

	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				accum += IntenseComputing(0xFFF);
				if (ulCounter % 9000 == 0)
				{
					_hx_printk("Lazy pig on CPU[%d],counter[%d],accumulate[%d]\r\n",
						__CURRENT_PROCESSOR_ID,
						ulCounter,
						accum);
				}
				ulCounter++;
				/* Reset timer. */
				CancelTimer(hTimer);
				hTimer = SetTimer(2048, 5, NULL, NULL, TIMER_FLAGS_ALWAYS);
				if (NULL == hTimer)
				{
					_hx_printk("Failed to set timer on CPU[%d],quit.\r\n",
						__CURRENT_PROCESSOR_ID);
					goto __EXIT;
				}
				/* Report to supervisor. */
				if (globalMailbox)
				{
					pMsg = (__LAZYPIG_MESSAGE*)_hx_malloc(sizeof(__LAZYPIG_MESSAGE));
					if (NULL == pMsg)
					{
						goto __EXIT;
					}
					pMsg->processorID = __CURRENT_PROCESSOR_ID;
					pMsg->times = 1;
					if (OBJECT_WAIT_RESOURCE != globalMailbox->SendMail((__COMMON_OBJECT*)globalMailbox,
						pMsg, 0, 10, NULL))
					{
						_hx_free(pMsg);
					}
				}
				break;
			default:
				break;
			}
		}
	}
	//CancelTimer(hTimer);
__EXIT:
	return 0;
}
#endif

#if __DEBUG_EVENT
static DWORD __cdecl lazyPigThread(LPVOID pData)
{
	unsigned long ulCounter = 0;
	int accum = 0;
	__LAZYPIG_MESSAGE* pMsg = NULL;

	while (TRUE)
	{
		accum += IntenseComputing(0xFFF);
		if (ulCounter % 1000 == 0)
		{
#if 1
			_hx_printf("Lazy pig on CPU[%d],counter[%d],accumulate[%u]\r\n",
				__CURRENT_PROCESSOR_ID,
				ulCounter,
				accum);
#endif
		}
		ulCounter++;
		/* Report to supervisor. */
		if (globalMailbox)
		{
			pMsg = (__LAZYPIG_MESSAGE*)_hx_malloc(sizeof(__LAZYPIG_MESSAGE));
			if (NULL == pMsg)
			{
				goto __EXIT;
			}
			pMsg->processorID = __CURRENT_PROCESSOR_ID;
			pMsg->times = 1;
			if (OBJECT_WAIT_RESOURCE != globalMailbox->SendMail((__COMMON_OBJECT*)globalMailbox,
				pMsg, 0, 10, NULL))
			{
				_hx_free(pMsg);
			}
		}
		/* Wait signaling from supervisor. */
		if (eventObject)
		{
			eventObject->WaitForThisObjectEx((__COMMON_OBJECT*)eventObject,10);
		}
	}
	//CancelTimer(hTimer);
__EXIT:
	return accum;
}
#endif

/* Supervisor of lazypig threads.:-) */
static DWORD __cdecl Supervisor(LPVOID pData)
{
	__COMMON_OBJECT* pMailbox = NULL;
	__COMMON_OBJECT* pEvent = NULL;
	__LAZYPIG_MESSAGE* pMsg = NULL;
	unsigned long ulCounter = 0;
	unsigned long times[MAX_CPU_NUM] = { 0 };

	/* Create mailbox first. */
	pMailbox = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_MAILBOX);
	if (NULL == pMailbox)
	{
		goto __TERMINAL;
	}
	if (!pMailbox->Initialize(pMailbox))
	{
		ObjectManager.DestroyObject(&ObjectManager, pMailbox);  //Destroy it.
		goto __TERMINAL;
	}

	//Set mailbox size accordingly.
	if (!((__MAIL_BOX*)pMailbox)->SetMailboxSize(pMailbox, 32))
	{
		ObjectManager.DestroyObject(&ObjectManager, pMailbox);
		goto __TERMINAL;
	}
	globalMailbox = (__MAIL_BOX*)pMailbox;

	/* Create event object if necessary. */
#if __DEBUG_EVENT
	pEvent = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);
	if (NULL == pEvent)
	{
		_hx_printk("Failed to create event object.\r\n");
		goto __TERMINAL;
	}
	if (!pEvent->Initialize(pEvent))
	{
		_hx_printk("Failed to initialize event object.\r\n");
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pEvent);
		goto __TERMINAL;
	}
	eventObject = (__EVENT*)pEvent;
#endif

	/* Main loop. */
	while (TRUE)
	{
#if __DEBUG_EVENT
		eventObject->PulseEvent((__COMMON_OBJECT*)eventObject);
#endif
		if (OBJECT_WAIT_RESOURCE == globalMailbox->GetMail(pMailbox, &pMsg, 5, NULL))
		{
			/* Process the message. */
			times[pMsg->processorID] += pMsg->times;
			/* Release the message. */
			_hx_free(pMsg);
			ulCounter++;
			if (0 == ulCounter % 3000)
			{
				/* Show result. */
				for (int i = 1; i < 4; i++)
				{
					_hx_printf("Lazypig[%d] has computing [%d] times.\r\n",
						i, times[i]);
				}
#if 1
				/* Debugging mailbox. */
				_hx_printf("Mailbox msg num:[%d],sending queue size:[%d]\r\n",
					globalMailbox->dwCurrMessageNum,
					globalMailbox->lpSendingQueue->dwCurrElementNum);
#endif
			}
		}
	}

__TERMINAL:
	if (pMailbox)
	{
		/* Destroy it. */
		ObjectManager.DestroyObject(&ObjectManager, pMailbox);
		pMailbox = NULL;
	}
#if __DEBUG_EVENT
	if (eventObject)
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)eventObject);
		eventObject = NULL;
	}
#endif
	return 0;
}

/*
 * System idle thread.
 * When there is no kernel thread to schedule,the system idle thread will run.
 * Some system level tasks can be processed in this thread,unless response time is not 
 * senstive,since this thread will not be scheduled in case of busying.
 * This kernel thread will never exit until the system is down.
 */
DWORD SystemIdle(LPVOID lpData)
{
	static DWORD dwIdleCounter = 0;

	/* Following code is used to test CPU synchronizing functions in SMP. */
#if 0
	char lazypigName[16];
	__KERNEL_THREAD_OBJECT* pKernelThread = NULL;

	/* Create testing kernel thread. */
	if (__CURRENT_PROCESSOR_IS_BSP())
	{
		/* Should create supervisor. */
		pKernelThread = (__KERNEL_THREAD_OBJECT*)CreateKernelThread(
			0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_NORMAL,
			Supervisor,
			NULL,
			NULL,
			"Supervisor");
		if (NULL == pKernelThread)
		{
			_hx_printk("Failed create supervisor.\r\n");
		}
	}
	else
	{
		/* Create lazypig thread on AP. */
		_hx_sprintf(lazypigName, "lazypig_%d", __CURRENT_PROCESSOR_ID);
		pKernelThread = (__KERNEL_THREAD_OBJECT*)CreateKernelThread(
			0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_NORMAL,
			lazyPigThread,
			NULL,
			NULL,
			lazypigName);
		if (NULL == pKernelThread)
		{
			_hx_printk("Failed to create testing thread on CPU[%d].\r\n",
				__CURRENT_PROCESSOR_ID);
		}
	}
#endif

	while(TRUE)
	{
#ifdef __I386__
		ShowAlive();
#endif
		/* Scan killed queue and clean up all killed thread(s). */
		KernelThreadManager.ProcessKilledQueue((__COMMON_OBJECT*)&KernelThreadManager);

		dwIdleCounter ++;
		if(0xFFFFFFFF == dwIdleCounter)
		{
			dwIdleCounter = 0;
		}
		//Halt the current CPU.
		HaltSystem();
	}
}
