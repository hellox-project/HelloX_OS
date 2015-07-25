//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 05,2013
//    Module Name               : console.c
//    Module Funciton           : 
//                                This module countains the implementation of console object.
//    Last modified Author      : Garry
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifdef __STM32__
#include <stm32f10x.h>                       /* STM32F10x definitions         */
#endif

#include "types.h"
#include "console.h"
#include "hellocn.h"

#include "iomgr.h"
#include "ktmgr.h"
#include <chardisplay.h>

//Available when and only when the __CFG_SYS_CONSOLE macro is defined.
#ifdef __CFG_SYS_CONSOLE

#ifdef __STM32__
//Low level output routine for STM32.
static int __SER_PutChar (int c) {
#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(DEFAULT_USART->SR & USART_SR_TXE));
  DEFAULT_USART->DR = (c & 0x1FF);
#endif
  return (c);
}

//Initialize STM32's default USART.
static void __Init_Default_Usart()
{
	  int i = 0;
	
	  RCC->APB2ENR |=  (   1UL <<  0);         /* enable clock Alternate Function */
		AFIO->MAPR   &= ~(   1UL <<  2);         /* clear USART1 remap              */
		
		RCC->APB2ENR |=  (   1UL <<  2);         /* enable GPIOA clock              */
		GPIOA->CRH   &= ~(0xFFUL <<  4);         /* clear PA9, PA10                 */
		GPIOA->CRH   |=  (0x0BUL <<  4);         /* USART1 Tx (PA9) output push-pull*/
		GPIOA->CRH   |=  (0x04UL <<  8);         /* USART1 Rx (PA10) input floating */
		
		RCC->APB2ENR |=  (   1UL << 14);         /* enable USART1 clock             */
		
		/* 115200 baud, 8 data bits, 1 stop bit, no flow control */
		DEFAULT_USART->CR1   = 0x002C;                  /* enable RX, TX                   */
		
		//Disable all interrupts one by one.
		DEFAULT_USART->CR1  &= (~256);                  //Disable PEIE
		DEFAULT_USART->CR1  &= (~128);                  //Disable TXEIE
		DEFAULT_USART->CR1  &= (~64);                   //Disable TC interrupt.
		DEFAULT_USART->CR1  &= (~32);                   //Disable RXNE interrupt.
		DEFAULT_USART->CR1  &= (~16);                   //Disable IDLEIE.
		
		DEFAULT_USART->CR2   = 0x0000;
		DEFAULT_USART->CR3   = 0x0000;                  /* no flow control                 */
		DEFAULT_USART->BRR   = 0x0271;
		for (i = 0; i < 0x1000; i++) __NOP();    /* avoid unwanted output           */
		DEFAULT_USART->CR1  |= 0x2000;                    /* enable USART                   */
}
#endif //__STM32__

//Low level output routine,which is used in interrupt context or OS initialization
//phase.
static void __LL_Output(UCHAR bt)
{
#ifdef __I386__
	DWORD dwCount1 = 1024;
	DWORD dwCount2 = 3;
	UCHAR ctrlReg  = 0;    //Used to save and restore temporary control register of COM interface.
#endif //__I386__

	//Check if the low level output function is initialized,initialize it if not.
	if(!Console.bLLInitialized)
	{
#ifdef __I386__
		 __outb(0x80,CON_DEF_COMBASE + 3);  //Set DLAB bit to 1,thus the baud rate divisor can be set.
         __outb(0x0C,CON_DEF_COMBASE);      //Set low byte of baud rate divisor.
         __outb(0x00,CON_DEF_COMBASE + 1);  //Set high byte of baud rate divisor.
         __outb(0x07,CON_DEF_COMBASE + 3);  //Reset DLAB bit,and set data bit to 8,one stop bit,without parity check.
         __outb(0x00,CON_DEF_COMBASE + 1);  //Disable all interrupts.
#elif defined(__STM32__)
		 //Initialize STM32's default USART port,which is defined in console.h file,default is USART1.
		 __Init_Default_Usart();
#endif  //__I386__
		 Console.bLLInitialized = TRUE;      //Indicate low level function is initialized.
	}

#ifdef __I386__
	//Try to output one byte.Please note that the output operation may fail,if the hardware is not ready for
	//sending for too long time.Control register of COM interface should be saved before sending,and restore
	//it after sending.
	ctrlReg = __inb(CON_DEF_COMBASE + 1);
	__outb(0x00,CON_DEF_COMBASE + 1);     //Disable all interrupts.
	while((dwCount2 --) > 0)
	{
		while((dwCount1 --) > 0)
		{
			if(__inb(CON_DEF_COMBASE + 5) & 32)  //Send register empty.
			{
				__outb(bt,CON_DEF_COMBASE);
				__outb(ctrlReg,CON_DEF_COMBASE + 1);
				return;
			}
		}
		dwCount1 = 1024;
	}
	__outb(ctrlReg,CON_DEF_COMBASE + 1);  //Restore previous control register.
#elif defined(__STM32__)
	//Call USART's low level output function.
	__SER_PutChar(bt);
#endif  //__I386__
}

//Console reading thread,it reads console input message and deliver
//it to DIM object by simulating a key board down message.
static DWORD ConReadThread(LPVOID pData)
{
	__DEVICE_MESSAGE  msg;
	int               ch;

	if(!Console.bInitialized)  //Check Console object's status.
	{
		return 0;
	}
	//The thread will never terminate.
	while(TRUE)
	{
		ch = Console.getch();
		if(-1 != ch)
		{
			msg.wDevMsgType = ASCII_KEY_DOWN;
			msg.dwDevMsgParam = (DWORD)ch;
			DeviceInputManager.SendDeviceMessage(
				(__COMMON_OBJECT*)&DeviceInputManager,
				&msg,
				NULL); 
		}
	}
	//return 0;
}

//Initialization routine of Console object.
static BOOL ConInitialize(__CONSOLE* pConsole)
{
	__COMMON_OBJECT* hCom1  = NULL;
	if(NULL == pConsole)
	{
		return FALSE;
	}

	//Try to open COM1 interface.
	hCom1 = IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		CON_DEF_COMNAME,
		0,
		0,
		NULL);

	//debug
	PrintLine("hCom1="); PrintLine(hCom1);

	if(NULL == hCom1)
	{
		return FALSE;
	}

	pConsole->hComInt      = hCom1;
	pConsole->bInitialized = TRUE;
	pConsole->nRowNum      = CON_MAX_ROWNUM;
	pConsole->nColNum      = CON_MAX_COLNUM;

	//Create console read thread to process COM interface input message.
	pConsole->hConThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		ConReadThread,
		NULL,
		NULL,
		CON_THREAD_NAME);

	PrintStr("pConsole=");
	PrintStr(pConsole);
	GotoHome();

	if(NULL == pConsole->hConThread)  //Failed to create thread.
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,pConsole->hComInt);
		pConsole->hComInt      = NULL;
		pConsole->bInitialized = FALSE;
		return FALSE;
	}
	return TRUE;
}

//Un-initialization routine of console object.
static VOID ConUninitialize(__CONSOLE* pConsole)
{
	if(NULL == pConsole)
	{
		return;
	}
	//Close COM interface.
	if(NULL != pConsole->hComInt)
	{
		IOManager.CloseFile(
			(__COMMON_OBJECT*)&IOManager,
			pConsole->hComInt);
	}
	//Destroy console read thread.
	if(NULL != pConsole->hConThread)
	{
		KernelThreadManager.DestroyKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)pConsole->hConThread);
	}
	pConsole->bInitialized = FALSE;	
	return;
}

//Operations of Console object.
static VOID ConPrintStr(const char* pszStr)
{
	DWORD   dwWriteSize = 0;
	DWORD   i;

	//Low level output should be used if in context or in system initialization phase.
	if(IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		dwWriteSize   = strlen(pszStr);
		for(i = 0;i < dwWriteSize;i ++)
		{
			__LL_Output(pszStr[i]);
		}
		return;
	}

	if(!Console.bInitialized)
	{
		return;
	}
	//Write string to COM interface.
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		strlen(pszStr),
		(LPVOID)pszStr,
		&dwWriteSize);
	return;
}

static VOID ConClearScreen(void)
{
	//Adopt low level output operation if in interrupt context or in process
	//of system initialization.
	if(IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		return;
	}
	if(!Console.bInitialized)
	{
		return;
	}
	return;
}

static VOID ConPrintCh(unsigned short ch)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = (CHAR)ch;

	//Low level output operation should be used when under interrupt context or in process
	//of OS initialization.
	if(IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if(!Console.bInitialized)
	{
		return;
	}
	//Write string to COM interface.
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		1,
		(LPVOID)&chTarg,
		&dwWriteSize);
	return;
}

static VOID ConGotoHome(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = '\r';

	//Low level output operation should be used when in interrupt context or in process
	//of OS initialization.
	if(IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if(!Console.bInitialized)
	{
		return;
	}
	//Write string to COM interface.
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		1,
		(LPVOID)&chTarg,
		&dwWriteSize);
	return;
}

static VOID ConChangeLine(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = '\n';

	//Interrupt context or OS initialization phase output.
	if(IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if(!Console.bInitialized)
	{
		return;
	}
	//Write string to COM interface.
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		1,
		(LPVOID)&chTarg,
		&dwWriteSize);
	return;
}

static VOID ConGotoPrev(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = VK_BACKSPACE;

	//Interrupt context or OS initialization phase.
	if(IN_INTERRUPT())
	{
		__LL_Output(chTarg);
		return;
	}

	if(!Console.bInitialized)
	{
		return;
	}
	//Write string to COM interface.
	IOManager.WriteFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		1,
		(LPVOID)&chTarg,
		&dwWriteSize);
	return;
}

static VOID ConPrintLine(const char* pszStr)
{
	ConGotoHome();
	ConChangeLine();
	ConPrintStr(pszStr);
	return;
}

//Input operation routines of console.
static int getch(void)
{
	char   ch    = 0;

	if(!Console.bInitialized)
	{
		return -1;
	}

	//Try to read user's input from COM interface.
	if(!IOManager.ReadFile((__COMMON_OBJECT*)&IOManager,
		Console.hComInt,
		1,
		(LPVOID)&ch,
		NULL))
	{
		return -1;
	}
	return ch;
}

static int getchar(void)
{
	return -1;
}

//CONSOLE object's definition.
__CONSOLE Console = {
	FALSE,                         //bInitialized;
	FALSE,                         //bLLInitialized;
	0,                             //nRowNum;
	0,                             //nColNum;
	NULL,                          //hComInt;
	NULL,                          //hConThread;

	//Initializer and un-initializer routines.
	ConInitialize,                          //Initialize;
	ConUninitialize,                        //Uninitialize;

	//Operations of console's output.
	ConPrintStr,
	ConClearScreen,
	ConPrintCh,
	ConGotoHome,
	ConChangeLine,
	ConGotoPrev,
	ConPrintLine,

	//Console's input operations.
	getch,
	getchar
};

#endif  //__CFG_SYS_CONSOLE.
