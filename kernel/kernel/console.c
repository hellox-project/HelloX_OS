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

#include "StdAfx.h"

#ifdef __STM32__
#include <stm32f10x.h>                       /* STM32F10x definitions         */
#endif

#include "types.h"
#include "console.h"
#include "hellocn.h"

#include "iomgr.h"
#include "ktmgr.h"
#include <chardisplay.h>

/* 
 * Available when and only when the 
 * __CFG_SYS_CONSOLE macro is defined. 
 */
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

/*
 * Low level output routine,which is used in 
 * interrupt context or OS initialization phase.
 */
static void __LL_Output(UCHAR bt)
{
	unsigned long ulFlags;
#ifdef __I386__
	DWORD dwCount1 = 1024;
	DWORD dwCount2 = 3;
	UCHAR ctrlReg  = 0;
#endif //__I386__

	/* Can not be interrupted. */
	__ENTER_CRITICAL_SECTION_SMP(Console.spin_lock, ulFlags);

	/*
	 * Check if the low level output function is 
	 * and initialized,initialize it if not.
	 */
	if (!Console.bLLInitialized)
	{
#ifdef __I386__
		/* Disable all interrupts. */
		__outb(0x00, CON_DEF_COMBASE + 1);
		/* Set DLAB bit to 1,thus the baud rate divisor can be set. */
		__outb(0x80, CON_DEF_COMBASE + 3);
		/* Set low byte of baud rate divisor. */
		__outb(0x0C, CON_DEF_COMBASE);
		/* Set high byte of baud rate divisor. */
		__outb(0x00, CON_DEF_COMBASE + 1);
		/* Reset DLAB bit,set data bit to 8,1 stop bit,no parity check. */
		__outb(0x03, CON_DEF_COMBASE + 3);
#elif defined(__STM32__)
		//Initialize STM32's default USART port,
		//which is defined in console.h file,default is USART1.
		__Init_Default_Usart();
#endif
		Console.bLLInitialized = TRUE;
	}

#ifdef __I386__
	/*
	 * Try to output one byte.Please note that the
	 * output operation may fail,if the hardware is 
	 * not ready for sending for too long time.
	 * Control register of COM interface should be 
	 * saved before sending,and restore it after sending.
	 */
	ctrlReg = __inb(CON_DEF_COMBASE + 1);
	/* Disable all interrupts. */
	__outb(0x00,CON_DEF_COMBASE + 1);
	while((dwCount2 --) > 0)
	{
		while((dwCount1 --) > 0)
		{
			if(__inb(CON_DEF_COMBASE + 5) & 32)
			{
				__outb(bt,CON_DEF_COMBASE);
				__outb(ctrlReg,CON_DEF_COMBASE + 1);
				__LEAVE_CRITICAL_SECTION_SMP(Console.spin_lock, ulFlags);
				return;
			}
		}
		dwCount1 = 1024;
	}
	/* Restore previous control register. */
	__outb(ctrlReg,CON_DEF_COMBASE + 1);
#elif defined(__STM32__)
	//Call USART's low level output function.
	__SER_PutChar(bt);
#endif  //__I386__
	__LEAVE_CRITICAL_SECTION_SMP(Console.spin_lock, ulFlags);
}

/* 
 * Console reading thread.
 * It reads console input message and deliver
 * it to DIM object by simulating a key board down message.
 */
static DWORD ConReadThread(LPVOID pData)
{
	__DEVICE_MESSAGE msg;
	int ch;

	if(!Console.bInitialized)
	{
		return 0;
	}

	while(TRUE)
	{
		ch = Console.getch();
		if(-1 != ch)
		{
			msg.wDevMsgType = KERNEL_MESSAGE_AKEYDOWN;
			msg.dwDevMsgParam = (DWORD)ch;
			DeviceInputManager.SendDeviceMessage(
				(__COMMON_OBJECT*)&DeviceInputManager,
				&msg,
				NULL); 
		}
	}
	return 0;
}

/* Initializer of console object. */
static BOOL ConInitialize(__CONSOLE* pConsole)
{
	__COMMON_OBJECT* hCom1  = NULL;

	BUG_ON(NULL == pConsole);
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pConsole->spin_lock, "con");
#endif

	/* Open the console port file. */
	hCom1 = IOManager.CreateFile((__COMMON_OBJECT*)&IOManager,
		CON_DEF_COMNAME, 0, 0, NULL);
	if(NULL == hCom1)
	{
		return FALSE;
	}

	pConsole->hComInt      = hCom1;
	pConsole->bInitialized = TRUE;
	pConsole->nRowNum      = CON_MAX_ROWNUM;
	pConsole->nColNum      = CON_MAX_COLNUM;

	/* 
	 * Create console read thread to process
	 * COM interface input message,console works
	 * in polling mode.
	 */
	pConsole->hConThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		ConReadThread,
		NULL,
		NULL,
		CON_THREAD_NAME);

	if(NULL == pConsole->hConThread)
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,pConsole->hComInt);
		pConsole->hComInt      = NULL;
		pConsole->bInitialized = FALSE;
		return FALSE;
	}
	return TRUE;
}

/* Destructor of the console object. */
static VOID ConUninitialize(__CONSOLE* pConsole)
{

	BUG_ON(NULL == pConsole);

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

	if (IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		dwWriteSize = strlen(pszStr);
		for (i = 0; i < dwWriteSize; i++)
		{
			__LL_Output(pszStr[i]);
		}
		return;
	}

	if (!Console.bInitialized)
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
	if (IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		return;
	}
	if (!Console.bInitialized)
	{
		return;
	}
	return;
}

static VOID ConPrintCh(unsigned short ch)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg = (CHAR)ch;

	/* 
	 * Low level output operation should be used when 
	 * under interrupt context or in process of OS initialization.
	 */
	if (IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if (!Console.bInitialized)
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
	CHAR    chTarg = '\r';

	if (IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if (!Console.bInitialized)
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
	CHAR    chTarg = '\n';

	if (IN_INTERRUPT() || IN_SYSINITIALIZATION())
	{
		__LL_Output(chTarg);
		return;
	}

	if (!Console.bInitialized)
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
	CHAR    chTarg = VK_BACKSPACE;

	//Interrupt context or OS initialization phase.
	if (IN_INTERRUPT())
	{
		__LL_Output(chTarg);
		return;
	}

	if (!Console.bInitialized)
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

//Operations of Console object.
static VOID ConPrintStr_k(const char* pszStr)
{
	DWORD   dwWriteSize = 0;
	DWORD   i;

	dwWriteSize   = strlen(pszStr);
	for(i = 0;i < dwWriteSize;i ++)
	{
		__LL_Output(pszStr[i]);
	}
	return;
}

static VOID ConClearScreen_k(void)
{
	return;
}

static VOID ConPrintCh_k(unsigned short ch)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = (CHAR)ch;

	__LL_Output(chTarg);
	return;
}

static VOID ConGotoHome_k(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = '\r';

	__LL_Output(chTarg);
	return;
}

static VOID ConChangeLine_k(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = '\n';

	__LL_Output(chTarg);
	return;
}

static VOID ConGotoPrev_k(void)
{
	DWORD   dwWriteSize = 0;
	CHAR    chTarg      = VK_BACKSPACE;

	__LL_Output(chTarg);
	return;
}

static VOID ConPrintLine_k(const char* pszStr)
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

/* Global console object. */
__CONSOLE Console = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,
#endif
	FALSE,            //bInitialized;
	FALSE,            //bLLInitialized;
	0,                //nRowNum;
	0,                //nColNum;
	NULL,             //hComInt;
	NULL,             //hConThread;

	ConInitialize,    //Initialize;
	ConUninitialize,  //Uninitialize;

#if 0
	//Operations of console's output.
	ConPrintStr,
	ConClearScreen,
	ConPrintCh,
	ConGotoHome,
	ConChangeLine,
	ConGotoPrev,
	ConPrintLine,
#endif

	ConPrintStr_k,
	ConClearScreen_k,
	ConPrintCh_k,
	ConGotoHome_k,
	ConChangeLine_k,
	ConGotoPrev_k,
	ConPrintLine_k,

	//Console's input operations.
	getch,
	getchar
};

#endif  //__CFG_SYS_CONSOLE.
