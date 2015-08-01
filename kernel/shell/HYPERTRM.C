//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,15 2007
//    Module Name               : HYPERTRM.CPP
//    Module Funciton           : 
//                                Countains HYPERTRM application's implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "../include/StdAfx.h"
#endif

#include "shell.h"
#include "hypertrm.h"

#include "kapi.h"
#include "stdio.h"

/*------------------------------------------------------------------------
    The poll mode COM IO application,named HYPERTRM.
	The following are the routines used by POLL mode hypertrm,including
	poll mode initialize routine,receiving byte routing,sending byte routine,
	and receiving kernel thread entry and sending kernel thread entry.
-------------------------------------------------------------------------*/

static void InitComPort(WORD base)  //Initialize the COM port for POLL mode.
{
	if((base != 0x3F8) && (base != 0x2F8))
	{
		return;
	}
	__outb(0x80,base + 3);  //Set DLAB bit to 1,thus the baud rate divisor can be set.
	__outb(0x0C,base);  //Set low byte of baud rate divisor.
	__outb(0x0,base + 1);  //Set high byte of baud rate divisor.
	__outb(0x07,base + 3); //Reset DLAB bit,and set data bit to 8,one stop bit,without parity check.
	__outb(0x0,base + 1); //Disable all interrupt enable bits.
	//__inb(base);  //Reset data register.
}

static BOOL ComSendByte(UCHAR bt,WORD port)  //Send a byte bt to port.
{
	DWORD dwCount1 = 1024;
	DWORD dwCount2 = 3;

	while((dwCount2 --) > 0)
	{
		while((dwCount1 --) > 0)
		{
			if(__inb(port + 5) & 32)  //Send register empty.
			{
				__outb(bt,port);
				return TRUE;
			}
		}
		dwCount1 = 1024;
		__MicroDelay(1024);  //Delay 1s and try again.
	}
	return FALSE;
}

static BOOL ComRecvByte(UCHAR* pbt,WORD port)  //Receive a byte from port,stored to *pbt.
{
	DWORD nCount1 = 1024;
	DWORD nCount2 = 3;

	while(nCount2 -- > 0)
	{
		while(nCount1 -- > 0)
		{
			if(__inb(port + 5) & 1)  //Data available.
			{
				*pbt = __inb(port);
				return TRUE;
			}
		}
		nCount1 = 1024;
		__MicroDelay(1024);  //Delay 1s and try again.
	}
	return FALSE;
}

//A local struct used to pass parameter for send and receive thread.
typedef struct{
	WORD wPortBase;
	__EVENT* lpEvent;
}__BASE_EVENT;

//Poll mode send thread.
static DWORD PollSend(LPVOID lpData)
{
	__BASE_EVENT* lpbe = (__BASE_EVENT*)lpData;  //Parameters passed from parent thread.
	__KERNEL_THREAD_MESSAGE msg;
	UCHAR bt;
	DWORD count;
	BOOL bSendResult = FALSE;

	if(NULL == lpbe)  //Invalid parameter.
	{
		return 0;
	}
	while(TRUE)
	{
		if(GetMessage(&msg))
		{
			if(MSG_KEY_DOWN == msg.wCommand)  //Key press event.
			{
				bt = (BYTE)msg.dwParam;
				if(QUIT_CHARACTER == bt)  //Should quit.
				{
					lpbe->lpEvent->SetEvent((__COMMON_OBJECT*)lpbe->lpEvent);
					PrintLine("Sending kernel thread exit now.");
					return 0;
				}
				bSendResult = FALSE;
				for(count = MAX_SEND_TIMES;count > 0;count --)
				{
					if(ComSendByte(bt,lpbe->wPortBase))  //Send successfully.
					{
						bSendResult = TRUE;
						break;
					}
				}
				if(!bSendResult)  //Failed to send out.
				{
					PrintLine("Failed to send out,connection may break.");
				}
			}
		}
	}
}

//Poll mode receive thread.
static DWORD PollRecv(LPVOID lpData)
{
	__BASE_EVENT*  lpbe = (__BASE_EVENT*)lpData;
	UCHAR bt;
	DWORD count;
	WORD wr = 0x0700;

	while(TRUE)
	{
		for(count = MAX_RECV_TIMES;count > 0;count --)
		{
			if(ComRecvByte(&bt,lpbe->wPortBase))  //Received a byte.
			{
				switch(bt)
				{
				case '\r':
					ChangeLine();
					break;
				case '\n':
					GotoHome();
					break;
				default:
					wr += bt;
					PrintCh(wr);
					wr = 0x0700;  //Reset the background color.
					break;
				}				
			}
		}
		if(OBJECT_WAIT_RESOURCE == lpbe->lpEvent->WaitForThisObjectEx(
			(__COMMON_OBJECT*)lpbe->lpEvent,0))  //Should terminate.
		{
			PrintLine("Receiving kernel thread exit now.");
			return 0;
		}
	}
}

//Main entry of the HYPERTRM application.
DWORD Hypertrm(LPVOID lpData)
{
	__BASE_EVENT be;
	__KERNEL_THREAD_OBJECT* lpSendThread = NULL;
	__KERNEL_THREAD_OBJECT* lpRecvThread = NULL;

	//Print application information.
	PrintLine(" -------- Hypertrm for Hello China is running -------- ");
	GotoHome();
	ChangeLine();
	
	//
	//Create a event object to sychronize the receive and send kernel thread.
	//Once user input the terminate character,the send kernel thread will
	//set this event object(to signal status),the receive kernel thread will
	//check this event and will terminate if the event object's status is set.
	//
	be.lpEvent = (__EVENT*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);  //Create event object.
	if(NULL == be.lpEvent)   //Can not create object.
	{
		PrintLine("Can not create event object.");
		goto __TERMINAL;
	}
	if(!be.lpEvent->Initialize((__COMMON_OBJECT*)be.lpEvent))
	{
		PrintLine("Can not initialize the event object.");
		goto __TERMINAL;
	}
	be.lpEvent->ResetEvent((__COMMON_OBJECT*)be.lpEvent);
	be.wPortBase = COM1_BASE;  //Use COM1 as default port.

	//Initialize the COM port.
	InitComPort(be.wPortBase);

	//Now create the receive and send kernel thread.
	lpSendThread = (__KERNEL_THREAD_OBJECT*)KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		PollSend,
		(LPVOID)&be,
		NULL,
		"COMSEND");
	if(NULL == lpSendThread)
	{
		PrintLine("Can not create send kernel thread.");
		goto __TERMINAL;
	}

	lpRecvThread = (__KERNEL_THREAD_OBJECT*)KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		PollRecv,
		(LPVOID)&be,
		NULL,
		"COMRECV");
	if(NULL == lpRecvThread)  //Can not create receive thread.
	{
		PrintLine("Can not create receive kernel thread.");
		goto __TERMINAL;
	}

	//Give the current focus to send thread.
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)lpSendThread);

	//Now wait the two kernel threads to finish.
	WaitForThisObject((HANDLE)lpSendThread);
	WaitForThisObject((HANDLE)lpRecvThread);

__TERMINAL:

	if(NULL != be.lpEvent)
	{
		DestroyEvent((HANDLE)be.lpEvent);
	}
	if(NULL != lpSendThread)
	{
		DestroyKernelThread((HANDLE)lpSendThread);
	}
	if(NULL != lpRecvThread)
	{
		DestroyKernelThread((HANDLE)lpRecvThread);
	}
	return 0;
}

/*------------------------------------------------------------------------
    The INTERRUPT mode COM IO application,named HYPTRM2.
	The following are the implementations of interrupt mode HYPTRM2.
	Including the following routines:
	    Interrupt mode Initialize routine;
		Interrupt mode Sending byte routine;
		Interrupt mode Receiving byte routine;
		Interrupt mode Sending kernel thread routine;
		Interrupt mode Receiving kernel thread routine;
		Interrupt mode main entry;
		Interrupt handler.
------------------------------------------------------------------------*/

static void InitComPort2(WORD base)  //Initialize COM port for Interrupt mode.
{
	if((base != 0x3F8) && (base != 0x2F8))
	{
		return;
	}
	__outb(0x80,base + 3);  //Set DLAB bit to 1,thus the baud rate divisor can be set.
	__outb(0x0C,base);  //Set low byte of baud rate divisor.
	__outb(0x0,base + 1);  //Set high byte of baud rate divisor.
	__outb(0x07,base + 3); //Reset DLAB bit,and set data bit to 8,one stop bit,without parity check.
	//__outb(0x0D,base + 1); //Set all interrupts but write buffer empty interrupt.
	__outb(0x01,base + 1);  //Enable data available interrupt.
	__outb(0x0B,base + 4); //Enable DTR,RTS and Interrupt.
	__inb(base);  //Reset data register.
}

static BOOL ComSendByte2(UCHAR bt,WORD port,HANDLE hEvent)  //Send bt to port,
                                                            //in interrupt mode.
															//The hEvent is the event
															//object to wait if send
															//failed.
{
	DWORD nCount = 16;
	DWORD dwResult = 0;
	DWORD dwFlags;

	ResetEvent(hEvent);  //Reset the event object.

	//The following code is critical section code.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
__REPEAT:
	while((nCount --) > 0)  //Try nCount times.
	{
		if(__inb(port + 5) & 32)  //Sending hold register is empty.
		{
			__outb(bt,port);  //Send out.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return TRUE;
		}
	}
	//If try nCount times even not successfull,then wait on the event
	//for a longer time.
	__outb(0x0F,port + 1);  //Set write hold register empty interrupt.
	dwResult = WaitForThisObjectEx(hEvent,MAX_SEND_WAIT);  //Wait for 2s.
	if(OBJECT_WAIT_RESOURCE == dwResult)  //Wait successfully.
	{
		nCount = 16;
		goto __REPEAT;  //Try again.
	}
	//Wait time out or other errors,give up.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return FALSE;
}

static BOOL ComRecvByte2(UCHAR* pbt,WORD port,HANDLE hEvent)
{
	DWORD nCount = 16;
	DWORD dwResult = 0;
	DWORD dwFlags;

	ResetEvent(hEvent);
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
__REPEAT:
	while((nCount --) > 0)
	{
		if(__inb(port + 5) & 1)  //Receive register full.
		{
			*pbt = __inb(port);  //Read the byte.
			__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
			return TRUE;
		}
	}
	//If even failed after try nCount times,then wait on the hEvent for a
	//longer time.
	dwResult = WaitForThisObjectEx(hEvent,MAX_RECV_WAIT);
	if(OBJECT_WAIT_RESOURCE == dwResult)  //COM interface has data for read.
	{
		nCount = 16;
		goto __REPEAT;  //Try to read again.
	}
	//If failed to wait,then give up.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return FALSE;
}

//Define a struct used to pass parameter.
typedef struct{
	WORD wBasePort;      //The COM port's base address(port).
	HANDLE hWaitEvent;   //The event object to synchronize interrupt.
	HANDLE hTerminateEvent;  //The event object to indicate terminate.
	HANDLE hSendRb;      //Sending ring buffer.
	HANDLE hRecvRb;      //Receiving ring buffer.
}__BASE_AND_EVENT2;

//COM interface's interrupt handler routine.
//Both send and receive interrupts are all handled in this routine.
static BOOL ComIntHandler(LPVOID lpEsp,LPVOID lpParam)
{
	__BASE_AND_EVENT2* pbe2 = (__BASE_AND_EVENT2*)lpParam;
	UCHAR              isr;
	UCHAR bt;

	if(NULL == pbe2)
	{
		BUG();
		return FALSE;
	}

	isr = __inb(pbe2->wBasePort + 2);  //Read interrupt status register.
	if(isr & 1)  //No interrupt to process.
	{
		return FALSE;
	}
	if(__inb(pbe2->wBasePort + 5) & 1)  //Data available.
	{
		bt = __inb(pbe2->wBasePort);  //Read the byte.
	    AddRingBuffElement(pbe2->hRecvRb,(DWORD)bt);  //Add to ring buffer.
	}
	/*
	//while(!(isr & 1))  //Has interrupt to process.
	//{
		if(isr & 2)  //Sending hold register empty interrupt occured.
		{
			__outb(0x0D,pbe2->wBasePort + 1); //Mask this interrupt.
			SetEvent(pbe2->hWaitEvent);  //Wake up the sending kernel thread.
		}
		else{
			if(isr & 4)  //Receiving data interrupt.
			{
				//SetEvent(pbe2->hTerminateEvent);  //Wake up the receiving kernel thread.
				bt = __inb(pbe2->wBasePort);  //Read the byte.
				AddRingBuffElement(pbe2->hRecvRb,(DWORD)bt);  //Add to ring buffer.
				PrintLine("Add one element to receiving ring buffer."); //-- ** debug ** --
			}
		}
		isr = __inb(pbe2->wBasePort + 2);  //Check again.
	//}
	*/
	return TRUE;
}

static DWORD IntSend(LPVOID lpData)  //Interrupt mode sending thread routine.
{
	__BASE_AND_EVENT2* pbe2 = (__BASE_AND_EVENT2*)lpData;
	__KERNEL_THREAD_MESSAGE msg;
	UCHAR bt;

	if(NULL == pbe2)  //Invalid parameter.
	{
		return 0;
	}
	while(TRUE)
	{
		if(GetMessage(&msg))
		{
			if(MSG_KEY_DOWN == msg.wCommand)  //Key press event.
			{
				bt = (BYTE)msg.dwParam;
				if(QUIT_CHARACTER == bt)  //Should quit.
				{
					SetEvent(pbe2->hTerminateEvent);  //Indicate the receiving thread
					                                  //to exit.
					PrintLine("Sending kernel thread exit now.");
					return 0;
				}
				//bSendResult = FALSE;
				__outb(bt,pbe2->wBasePort);
				/*for(count = MAX_SEND_TIMES;count > 0;count --)
				{
					if(ComSendByte2(bt,pbe2->wBasePort,pbe2->hWaitEvent))  //Send successfully.
					{
						bSendResult = TRUE;
						break;
					}
				}
				if(!bSendResult)  //Failed to send out.
				{
					PrintLine("Failed to send out,connection may break.");
				}*/
			}
		}
	}
}

static DWORD IntRecv(LPVOID lpData)  //Receive thread routine in interrupt mode.
{
	__BASE_AND_EVENT2*  pbe2 = (__BASE_AND_EVENT2*)lpData;
	WORD wr = 0x0700;
	DWORD element;

	while(TRUE)
	{
		if(GetRingBuffElement(pbe2->hRecvRb,&element,MAX_RECV_WAIT))
		{
			switch((UCHAR)element)
			{
			case '\r':
				ChangeLine();
				break;
			case '\n':
				GotoHome();
				break;
			default:
				wr += (UCHAR)element;
				PrintCh(wr);
				wr = 0x0700;  //Reset the background color.
				break;
			}
		}
		if(OBJECT_WAIT_RESOURCE == WaitForThisObjectEx(pbe2->hTerminateEvent,
			0))  //Should terminate.
		{
			PrintLine("Receiving kernel thread exit now.");
			return 0;
		}
	}
}

//Main entry for HYPTRM2 application.
DWORD Hyptrm2(LPVOID lpData)
{
	__BASE_AND_EVENT2 be21;    //Transmit parameters to interrupt handler.
	__BASE_AND_EVENT2 besend;  //Transmit parameters to send thread.
	__BASE_AND_EVENT2 berecv;  //Transmit parameters to receive thread.
	HANDLE hSendThread  = NULL;
	HANDLE hRecvThread  = NULL;
	HANDLE hSendEvent   = NULL;    //Event to synchronize sending thread and interrupt.
	HANDLE hRecvEvent   = NULL;    //Event to synchronize receiving thread and interrupt.
	HANDLE hTerminateEvent = NULL;
	HANDLE hIntHandler     = NULL;
	HANDLE hSendRb         = NULL;
	HANDLE hRecvRb         = NULL;

	//Print out application title information.
	PrintLine(" -------- Hyptrm2 for Hello China is running -------- ");
	GotoHome();
	ChangeLine();

	hSendEvent = CreateEvent(FALSE);
	if(NULL == hSendEvent)
	{
		PrintLine("Can not create hSendEvent.");
		goto __TERMINAL;
	}
	hRecvEvent = CreateEvent(FALSE);
	if(NULL == hRecvEvent)
	{
		PrintLine("Can not create hRecvEvent.");
		goto __TERMINAL;
	}
	hTerminateEvent = CreateEvent(FALSE);
	if(NULL == hTerminateEvent)
	{
		PrintLine("Can not create hTerminateEvent.");
		goto __TERMINAL;
	}
	
	hSendRb = CreateRingBuff(0);
	if(NULL == hSendRb)
	{
		PrintLine("Can not create sending ring buffer.");
		goto __TERMINAL;
	}
	hRecvRb = CreateRingBuff(0);
	if(NULL == hRecvRb)
	{
		PrintLine("Can not create receiving ring buffer.");
		goto __TERMINAL;
	}

	be21.hWaitEvent      = hSendEvent;
	be21.hTerminateEvent = hRecvEvent;
	be21.hSendRb         = hSendRb;
	be21.hRecvRb         = hRecvRb;
	be21.wBasePort       = COM1_BASE;

	//Connect interrupt handler.
	hIntHandler = ConnectInterrupt(ComIntHandler,(LPVOID)&be21,COM1_INT_VECTOR);
	if(NULL == hIntHandler)
	{
		PrintLine("Can not set COM's interrupt handler.");
		goto __TERMINAL;
	}

	//Initialize the COM interface.
	InitComPort2(COM1_BASE);

	//Create sending kernel thread now.
	besend.wBasePort =       COM1_BASE;
	besend.hWaitEvent =      hSendEvent;
	besend.hTerminateEvent = hTerminateEvent;
	besend.hSendRb    = hSendRb;
	hSendThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		IntSend,
		(LPVOID)&besend,
		NULL,
		"COMSEND_INT");
	if(NULL == hSendThread)  //Failed to create sending kernel thread.
	{
		PrintLine("Can not create sending thread.");
		goto __TERMINAL;
	}

	//Create receiving kernel thread.
	berecv.hTerminateEvent = hTerminateEvent;
	berecv.hWaitEvent      = hRecvEvent;
	berecv.wBasePort       = COM1_BASE;
	berecv.hRecvRb         = hRecvRb;
	hRecvThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		IntRecv,
		(LPVOID)&berecv,
		NULL,
		"COMRECV_INT");
	if(NULL == hRecvThread)  //Can not create receiving thread.
	{
		PrintLine("Can not create receiving kernel thread.");
		goto __TERMINAL;
	}

	//Set sending kernel thread as current focus thread.
	DeviceInputManager.SetFocusThread((__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)hSendThread);

	//Wait for receiving and sending kernel threads to terminate.
	WaitForThisObject(hSendThread);
	WaitForThisObject(hRecvThread);

__TERMINAL:
	if(NULL != hIntHandler)
	{
		DisconnectInterrupt(hIntHandler);  //Disconnect the interrupt handler.
	}
	if(NULL != hSendEvent)
	{
		DestroyEvent(hSendEvent);
	}
	if(NULL != hRecvEvent)
	{
		DestroyEvent(hRecvEvent);
	}
	if(NULL != hTerminateEvent)
	{
		DestroyEvent(hTerminateEvent);
	}

	if(NULL != hSendThread)
	{
		DestroyKernelThread(hSendThread);
	}
	if(NULL != hRecvThread)
	{
		DestroyKernelThread(hRecvThread);
	}

	if(NULL != hRecvRb)
	{
		DestroyRingBuff(hRecvRb);
	}
	if(NULL != hSendRb)
	{
		DestroyRingBuff(hSendRb);
	}
	return 0;
}

