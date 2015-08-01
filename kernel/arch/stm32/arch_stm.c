//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec 1,2013(It's 7 years from the first ARCH file was created for x86...)
//    Module Name               : ARCH_STM.CPP
//    Module Funciton           : 
//                                This module countains CPU and hardware platfrom specific code,this
//                                file is for STM32 series chipset and edited under MDK.
//                                This is a flagship for Hello China to migrate to other platforms than
//                                x86.
//
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <tdAfx.h>
#endif

//Include STM32 chip headers if necessary.
#ifdef __STM32__
#include "stm32f10x.h"
#endif

#include "ARCH.H"
#include "stdio.h"

#ifdef __STM32__  //Only available under STM32 chipset.

//Hardware initialization code,low level hardware should be initialized in
//this routine.It will be called before OS initialization process.
BOOL HardwareInitialize()
{
	int hz = 1000 / SYSTEM_TIME_SLICE;
	//Initialize systick.
	SysTick_Config(72000000 / hz);
	return TRUE;
}

//
//Enable Virtual Memory Management mechanism.This routine will be called in
//process of OS initialization if __CFG_SYS_VMM flag is defined.
//
VOID EnableVMM()
{
}

//
//This routine initializes a kernel thread's context.
//This routine's action depends on different platform.
//
VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
							 __KERNEL_THREAD_WRAPPER lpStartAddr)
{
	DWORD*        lpStackPtr = NULL;

	if((NULL == lpKernelThread) || (NULL == lpStartAddr))  //Invalid parameters.
	{
		return;
	}

//Define a macro to make the code readable.
#define __PUSH(stackptr,val) \
	do{  \
	(DWORD*)(stackptr) --; \
	*((DWORD*)stackptr) = (DWORD)(val); \
	}while(0)

	lpStackPtr = (DWORD*)lpKernelThread->lpInitStackPointer;

  //Push registers of the thread into stack,to simulate a interrupt stack frame.
	__PUSH(lpStackPtr,0x01000000);          //xPSR.
	__PUSH(lpStackPtr,(DWORD)lpStartAddr);  //PC.
	__PUSH(lpStackPtr,0xFFFFFFFF);          //LR.
	__PUSH(lpStackPtr,0x12121212);          //R12.
	__PUSH(lpStackPtr,0x03030303);          //R3.
	__PUSH(lpStackPtr,0x02020202);          //R2.
	__PUSH(lpStackPtr,0x01010101);          //R1.
	__PUSH(lpStackPtr,lpKernelThread);      //R0,should be the thread's handle,very important.
	
	__PUSH(lpStackPtr,0x11111111);          //R11.
	__PUSH(lpStackPtr,0x10101010);          //R10.
	__PUSH(lpStackPtr,0x09090909);          //R9.
	__PUSH(lpStackPtr,0x08080808);          //R8.
	__PUSH(lpStackPtr,0x07070707);          //R7.
	__PUSH(lpStackPtr,0x06060606);          //R6.
	__PUSH(lpStackPtr,0x05050505);          //R5.
	__PUSH(lpStackPtr,0x04040404);          //R4.

	//Save context.
	lpKernelThread->lpKernelThreadContext = (__KERNEL_THREAD_CONTEXT*)lpStackPtr;
	return;
}

//Get system TimeStamp counter.
VOID __GetTsc(__U64* lpResult)
{
#ifdef __I386__
	__asm{
		push eax
		push edx
		push ebx
		rdtsc    //Read time stamp counter.
		mov ebx,dword ptr [ebp + 0x08]
		mov dword ptr [ebx],eax
		mov dword ptr [ebx + 0x04],edx
		pop ebx
		pop edx
		pop eax
	}
#else
#endif
}


#define CLOCK_PER_MICROSECOND 1024  //Assume the CPU's clock is 1G Hz.

VOID __MicroDelay(DWORD dwmSeconds)
{
	return;
}

VOID __outd(WORD wPort,DWORD dwVal)  //Write one double word to a port.
{
}

DWORD __ind(WORD wPort)    //Read one double word from a port.
{
	return 0;
}

VOID __outb(UCHAR _bt,WORD _port)  //Send bt to port.
{
}

UCHAR __inb(WORD _port)  //Receive a byte from port.
{
	return 0;
}

WORD __inw(WORD wPort)
{
	return 0;
}

VOID __outw(WORD wVal,WORD wPort)
{
}

VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
}

VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort)
{
}

//External low level output routine.
extern void SER_PutString(char*);

//Hardware fault handler.
void HardFault_Handler()
{
	DWORD* msp = (DWORD*)__get_MSP();
	//int    i = 16;
	char   buff[32];
	DWORD* esp = (DWORD*)KernelThreadManager.lpCurrentKernelThread->lpKernelThreadContext;
	
	SER_PutString("\r\nHardFault_Handler\r\n");
		
	SER_PutString("  ---- Thread Info ---- \r\n");
	SER_PutString("  Thread name: ");
	SER_PutString((char*)KernelThreadManager.lpCurrentKernelThread->KernelThreadName);
	SER_PutString("\r\n");
	SER_PutString("  Thread Status: ");
	if(KERNEL_THREAD_STATUS_RUNNING == KernelThreadManager.lpCurrentKernelThread->dwThreadStatus)
	{
		SER_PutString(" RUNNING.\r\n");
	}
	if(KERNEL_THREAD_STATUS_READY == KernelThreadManager.lpCurrentKernelThread->dwThreadStatus)
	{
		SER_PutString(" READY.\r\n");
	}
	if(KERNEL_THREAD_STATUS_BLOCKED == KernelThreadManager.lpCurrentKernelThread->dwThreadStatus)
	{
		SER_PutString(" BLOCKED.\r\n");
	}
	
	_hx_sprintf(buff,"ESP = %X\r\n",KernelThreadManager.lpCurrentKernelThread->lpKernelThreadContext);
	SER_PutString(buff);
	_hx_sprintf(buff,"MSP = %X\r\n",msp);
	SER_PutString(buff);
	_hx_sprintf(buff,"xPSR = %X\r\n",*esp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"PC = %X\r\n",*esp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"LR = %X\r\n",*esp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"R12 = %X\r\n",*esp ++);
	SER_PutString(buff);
	
	SER_PutString("  ---- Exception Info ---- \r\n");
	_hx_sprintf(buff,"MSP = %X\r\n",msp);
	SER_PutString(buff);
	_hx_sprintf(buff,"D1 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D2 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D3 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D4 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D5 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D6 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D7 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D8 = %X\r\n",*msp ++);
	SER_PutString(buff);
	_hx_sprintf(buff,"D9 = %X\r\n",*msp ++);
	SER_PutString(buff);
	
	while(TRUE);
}

void MemManage_Handler()
{
	while(TRUE)
	{
		SER_PutString("\r\nMemManage_Handler\r\n");
	}
}

void BusFault_Handler()
{
	while(TRUE)
	{
		SER_PutString("\r\nBusFault_Handler\r\n");
	}
}

void UsageFault_Handler()
{
	while(TRUE)
	{
		SER_PutString("\r\nUsageFault_Handler\r\n");
	}
}

void CheckESP(__KERNEL_THREAD_OBJECT* pKernelThread,LPVOID pESP)
{
	char buff[64];
	if((DWORD)pKernelThread->lpInitStackPointer <= (DWORD)pESP)
	{
		_hx_sprintf(buff,"Fault error: InitStackPtr = %X,ESP = %X.\r\n",(DWORD)pKernelThread->lpInitStackPointer,
		    (DWORD)pESP);
		 SER_PutString(buff);
		 while(TRUE);
	}
	if((DWORD)pKernelThread->lpInitStackPointer - pKernelThread->dwStackSize >= (DWORD)pESP)
	{
		SER_PutString("  CHECK ESP ERROR,bottom overflow.\r\n");
		while(TRUE);
	}
	if((DWORD)pKernelThread->lpInitStackPointer - (DWORD)pESP >= (3 * DEFAULT_STACK_SIZE) / 4)
	{
		SER_PutString("  CHECK ESP WARNING:thread has less 1/4 bytes stack.\r\n");
	}
}

#endif //__STM32__
