#include <stm32f10x.h>                       /* STM32F10x definitions         */
#ifndef __STDAFX_H__
#include "..\..\include\StdAfx.h"
#endif
#include <stdio.h>


#ifndef __USART_H__
#include "usart.h"
#endif


/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
 *----------------------------------------------------------------------------*/
//Enable USART's transfer complete interrupt.
static void EnableTransferCompleteInt(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	pUsart->CR1 |= 64;  //Set TC bit.
}


//Disable USART's transfer complete interrupt.
static void DisableTransferCompleteInt(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	pUsart->CR1 &= (~64);  //Clear TC bit in CR.
	pUsart->SR  &= (~64);  //Clear TC bit in SR.
}


//Check if a raised interrupt is transfer complete.
static BOOL IsTransferCompleteInt(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	if(pUsart->SR & 64)
	{
		return TRUE;
	}
	return FALSE;
}


//Check if USART device's transfer buffer is enable.
static BOOL IsTransferEnable(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	if(pUsart->SR & USART_SR_TXE)
	{
		return TRUE;
	}
	return FALSE;
}


//Send out one byte.
static void SendUsartByte(CHAR bt,DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	pUsart->DR = bt & 0x1FF;
}


//Check if a raised interrupt is triggered by RXNE.
static BOOL IsReceiveInt(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	if(pUsart->SR & 32)
	{
		return TRUE;
	}
	return FALSE;
}


//Check if data is available.
static BOOL IsDataAvailable(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	if(pUsart->SR & 32)  //RXNE bit set.
	{
		return TRUE;
	}
	return FALSE;
}


//Get one byte from USART's data buffer.
static CHAR GetUsartByte(DWORD usartBase)
{
	USART_TypeDef* pUsart = (USART_TypeDef*)usartBase;
	return (CHAR)(pUsart->DR & 0xFF);
}


/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int SER_PutChar (int c) {
#ifdef __DBG_ITM
    ITM_SendChar(c);
#else
  while (!(USART1->SR & USART_SR_TXE));
  USART1->DR = (c & 0x1FF);
#endif
  return (c);
}

//For debugging.
void SER_PutString(CHAR* pszString)
{
	while(*pszString)
	{
		SER_PutChar(*pszString);
		pszString ++;
	}
}


//------------------------------------------------------------------------------
// USART driver framework functions.
//------------------------------------------------------------------------------


//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF


//COM device array,each COM/USART/UART interface existing in systen corresponding
//to one entry of this array.
#define USART_DEVICE_NUM 2
static __USART_CONTROL_BLOCK UsartCtrlBlock[USART_DEVICE_NUM] = {
         ADD_USART_DEVICE(USART1_NAME,USART1_BASE,USART1_INT_VECTOR),
         ADD_USART_DEVICE(USART2_NAME,USART2_BASE,USART2_INT_VECTOR),
         //Please add extra COM interface device here.
};


//Initialization routine of all USART interface devices.
static BOOL InitializeUsart(DWORD base)
{
  BOOL           bResult       = FALSE;
	USART_TypeDef* pUsart        = (USART_TypeDef*)base;
	int i;


	//The following initialization code is copied from Hello example of MDK package
	//containing.Actually the code only initializes USART1,please add more code as
	//the following to initialize USART2 or others.
	if((DWORD)USART1 == base)
	{
		RCC->APB2ENR |=  (   1UL <<  0);         /* enable clock Alternate Function */
		AFIO->MAPR   &= ~(   1UL <<  2);         /* clear USART1 remap              */
		
		RCC->APB2ENR |=  (   1UL <<  2);         /* enable GPIOA clock              */
		GPIOA->CRH   &= ~(0xFFUL <<  4);         /* clear PA9, PA10                 */
		GPIOA->CRH   |=  (0x0BUL <<  4);         /* USART1 Tx (PA9) output push-pull*/
		GPIOA->CRH   |=  (0x04UL <<  8);         /* USART1 Rx (PA10) input floating */
		
		RCC->APB2ENR |=  (   1UL << 14);         /* enable USART1 clock             */
		
		/* 115200 baud, 8 data bits, 1 stop bit, no flow control */
		pUsart->CR1   = 0x002C;                  /* enable RX, TX                   */
		
		//Disable all interrupts one by one.
		pUsart->CR1  &= (~256);                  //Disable PEIE
		pUsart->CR1  &= (~128);                  //Disable TXEIE
		pUsart->CR1  &= (~64);                   //Disable TC interrupt.
		//pUsart->CR1  &= (~32);                   //Disable RXNE interrupt.
		pUsart->CR1  &= (~16);                   //Disable IDLEIE.
		
		//Enable RXNE interrupt.
		pUsart->CR1  |= 32;
		
		pUsart->CR2   = 0x0000;
		pUsart->CR3   = 0x0000;                  /* no flow control                 */
		pUsart->BRR   = 0x0271;
		for (i = 0; i < 0x1000; i++) __NOP();    /* avoid unwanted output           */
		pUsart->CR1  |= 0x2000;                    /* enable USART                   */
		
		//Set USART1's interrupt priority and enable the interrupt.
		NVIC_SetPriority(USART1_IRQn,0x0F);
		NVIC_EnableIRQ(USART1_IRQn);
		bResult = TRUE;
	}
	if((DWORD)USART2 == base)
	{
		//Shoud put initialization code for USART2 here.
		bResult = TRUE;
	}
	if((DWORD)USART3 == base)
	{
		//Shoud put your initialization code here for USART3.
		bResult = TRUE;
	}
  return bResult;
}


//Unload entry for COM driver.
static DWORD UsartDestroy(__COMMON_OBJECT* lpDriver,
                        __COMMON_OBJECT* lpDevice,
                        __DRCB*          lpDrcb)


{
         __USART_CONTROL_BLOCK* pCtrlBlock = (__USART_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;


         if(NULL == pCtrlBlock)
         {
					 return 0;
         }
         //Disconnect interrupt.
         System.DisconnectInterrupt((__COMMON_OBJECT*)&System,(__COMMON_OBJECT*)pCtrlBlock->hInterrupt);
         //Destroy device object.
         IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
                   (__DEVICE_OBJECT*)lpDevice);
         return 0;
}


//Handler of Transfer Complete interrupt.
static VOID TCIntHandler(__USART_CONTROL_BLOCK* pCtrlBlock)
{
         __DRCB*      pWrittingHeader = NULL;


         pWrittingHeader = pCtrlBlock->pWrittingList;
         if(NULL == pWrittingHeader)  //No DRCB is pending to write.
         {
                   DisableTransferCompleteInt(pCtrlBlock->dwUsartBase);  //Disable WBE interrupt.
                   return;
         }
         if(pCtrlBlock->pCurrWritting != pWrittingHeader)  //The current writting DRCB maybe deleted.
         {
                   pCtrlBlock->pCurrWritting = pWrittingHeader;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)pWrittingHeader->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = pWrittingHeader->dwInputLen;
         }


         while(IsTransferEnable(pCtrlBlock->dwUsartBase))  //Can set byte.
         {
                   if(pCtrlBlock->nWrittingPtr == pCtrlBlock->nWrittingSize)  //Send over.
                   {
                            //Mark the DRCB as processed successfully.
                            pCtrlBlock->pCurrWritting->dwStatus = DRCB_STATUS_SUCCESS;
                            //Delete this DRCB from pending writting queue.
                            pCtrlBlock->pWrittingList = pCtrlBlock->pWrittingList->lpNext;
                            if(NULL == pCtrlBlock->pWrittingList)  //No pending DRCB object.
                            {
                                     pCtrlBlock->pWrittingListTail = NULL;
                            }
                            //Wake up the kernel thread that is waiting for writting completion.
                            pCtrlBlock->pCurrWritting->OnCompletion((__COMMON_OBJECT*)(pCtrlBlock->pCurrWritting));
                   }
                   else
                   {
                            //Send one byte.
                            SendUsartByte(*pCtrlBlock->pWrittingPtr,pCtrlBlock->dwUsartBase);
                            //Update pointers.
                            pCtrlBlock->pWrittingPtr ++;
                            pCtrlBlock->nWrittingPtr ++;
                            continue;  //Continue to write.
                   }
                   pWrittingHeader = pCtrlBlock->pWrittingList;
                   if(NULL == pWrittingHeader)
                   {
                            pCtrlBlock->pCurrWritting = NULL;
                            DisableTransferCompleteInt(pCtrlBlock->dwUsartBase);
                            return;
                   }
                   //Should process next DRCB object in pending queue.
                   pCtrlBlock->pCurrWritting = pWrittingHeader;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)pWrittingHeader->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = pWrittingHeader->dwInputLen;
         }
}


//Handler to handle the receive(RXNE) interrupt,DA means Data Available here,the name
//inherites from COM driver implemented on X86 platform.
static VOID DAIntHandler(__USART_CONTROL_BLOCK* pCtrlBlock)
{
         CHAR                bt;
	       DWORD               dwFlags;


         //Process data available interrupt.
	       __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         while(IsDataAvailable(pCtrlBlock->dwUsartBase))  //Data available.
         {
                   bt = GetUsartByte(pCtrlBlock->dwUsartBase);  //Read the byte.
                   pCtrlBlock->Buffer[pCtrlBlock->nBuffTail] = bt;
                   pCtrlBlock->nBuffTail ++;
                   if(COM_BUFF_LENGTH == pCtrlBlock->nBuffTail)  //Reach end of buffer.
                   {
                            pCtrlBlock->nBuffTail = 0;
                   }
                   if(pCtrlBlock->nBuffHeader == pCtrlBlock->nBuffTail)  //Buffer full.
                   {
                            break;
                   }
         }
         while(IsDataAvailable(pCtrlBlock->dwUsartBase))  //Drain out all data from COM interface,since the buffer is full.
				 {
					 GetUsartByte(pCtrlBlock->dwUsartBase);
				 }
         //Wake up all threads waiting for COM data.
         while(pCtrlBlock->pReadingList)
         {
                   pCtrlBlock->pReadingList->OnCompletion((__COMMON_OBJECT*)pCtrlBlock->pReadingList);
                   pCtrlBlock->pReadingList = pCtrlBlock->pReadingList->lpNext;
         }
         pCtrlBlock->pReadingListTail = NULL;
				 __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
}


//Interrupt handler of COM.
static BOOL UsartIntHandler(LPVOID lpESP,LPVOID lpParam)
{
         __USART_CONTROL_BLOCK* pCtrlBlock = (__USART_CONTROL_BLOCK*)lpParam;
         //UCHAR isr;                        //Read interrupt status register.
 
         //isr = COMRecvByte(pCtrlBlock->dwUsartBase + 2);
         if(IsTransferCompleteInt(pCtrlBlock->dwUsartBase))  //No interrupt to process.
         {
                   TCIntHandler(pCtrlBlock);
					         return TRUE;
         }
				 
         if(IsReceiveInt(pCtrlBlock->dwUsartBase)) //RXNE interrupt.
         {
                   DAIntHandler(pCtrlBlock);
					         return TRUE;
         }
         //Other interrupt may be handled here.
         return FALSE;
}
 
//Read operations for COM interface.This routine will be called
//by API ReadFile,the device object's handle as input.
static DWORD UsartDeviceRead(__COMMON_OBJECT* lpDriver,__COMMON_OBJECT* lpDevice,__DRCB* lpDrcb)
{
         DWORD                  dwReadSize    = 0;
         __USART_CONTROL_BLOCK*   pCtrlBlock    = NULL;
         DWORD                  dwFlags;
         CHAR*                  pOutputBuffer = NULL;
		 __DRCB*                pListHdr      = NULL;  //Pointer to manipulate reading list.
 
         //Parameters check.
         if((lpDrcb->dwStatus != DRCB_STATUS_INITIALIZED) || (lpDrcb->dwRequestMode != DRCB_REQUEST_MODE_READ))
         {
                   goto __TERMINAL;
         }
         if(NULL == lpDevice)
         {
                   goto __TERMINAL;
         }
         //Get device extension.
         pCtrlBlock = (__USART_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;
         if(NULL == pCtrlBlock)
         {
                   goto __TERMINAL;
         }
         pOutputBuffer = (CHAR*)lpDrcb->lpOutputBuffer;
 
         //Begin to read,this is a loop operation,since one try maybe failed.
__TRY_AGAIN:
		 __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         //Try to read data from buffer.
         while(pCtrlBlock->nBuffHeader != pCtrlBlock->nBuffTail)  //Buffer is not empty.
         {
                   pOutputBuffer[dwReadSize] = pCtrlBlock->Buffer[pCtrlBlock->nBuffHeader];
                   dwReadSize ++;
                   pCtrlBlock->nBuffHeader ++;
                   if(COM_BUFF_LENGTH == pCtrlBlock->nBuffHeader)  //Reach the end of buffer.
                   {
                            pCtrlBlock->nBuffHeader = 0;
                   }
                   if(dwReadSize >= lpDrcb->dwOutputLen)  //Requested size is fit.
                   {
                            break;
                   }
         }
         if(dwReadSize)  //At least read one byte data,return.
         {
                   __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
                   goto __TERMINAL;
         }
         //Can not read any data from buffer,so enter waiting status,insert the DRCB
         //object into pending reading list first.
         if(NULL == pCtrlBlock->pReadingListTail)  //No DRCB yet.
         {
                   pCtrlBlock->pReadingList = lpDrcb;
                   pCtrlBlock->pReadingListTail = lpDrcb;
                   lpDrcb->lpNext = NULL;
         }
         else  //Not the first one,just queue to list's tail.
         {
                   pCtrlBlock->pReadingListTail->lpNext = lpDrcb;
                   lpDrcb->lpNext = NULL;
                   pCtrlBlock->pReadingListTail = lpDrcb;
                   //Should wait the request to finish here by calling WaitForCompletion().
         }
		 __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
         switch(lpDrcb->WaitForCompletion((__COMMON_OBJECT*)lpDrcb))
         {
         case OBJECT_WAIT_RESOURCE:  //The thread is waken up by interrupt,try to read data.
             goto __TRY_AGAIN;
         case OBJECT_WAIT_TIMEOUT:   //Time out,should delete the queuing DRCB from reading list.
			 __ENTER_CRITICAL_SECTION(NULL,dwFlags);
			 pListHdr = pCtrlBlock->pReadingList;
			 if(NULL == pListHdr)    //The DRCB maybe deleted by COM interface's interrupt handler.
			 {
				 goto __DELETED_IN_INT;
			 }
			 if(lpDrcb == pListHdr)  //The first one is the timeout DRCB.
			 {
				 pCtrlBlock->pReadingList = pListHdr->lpNext;
				 if(NULL == pListHdr->lpNext)  //Also the last one.
				 {
					 pCtrlBlock->pReadingListTail = NULL;
				 }
			 }
			 else  //Not the first one.
			 {
				 while(pListHdr->lpNext != NULL)
				 {
					 if(lpDrcb == pListHdr->lpNext)  //Found,delete it.
					 {
						 pListHdr->lpNext = lpDrcb->lpNext;
						 if(NULL == pListHdr->lpNext) //The DRCB is the last one.
						 {
							 pCtrlBlock->pReadingListTail = pListHdr;
						 }
						 break;
					 }
					 pListHdr = pListHdr->lpNext;
				 }
			 }
__DELETED_IN_INT:
			 //Mark the DRCB as failed.
			 lpDrcb->dwStatus = DRCB_STATUS_FAIL;
             dwReadSize = 0;
			 __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
             break;
         }
 
__TERMINAL:
         return dwReadSize;
}


//The low level sending routine for COM interface.
static BOOL __LLSend(DWORD usartBase,CHAR bt)
{
	if(IsTransferEnable(usartBase))
	{
		SendUsartByte(bt,usartBase);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
 
//A local helper routine to delete a DRCB object from writting list.
static VOID DeleteWriteDrcb(__USART_CONTROL_BLOCK* pCtrlBlock,__DRCB* pDrcb)
{
         __DRCB*  pPrev = NULL;
         __DRCB*  pNext = NULL;
 
         if((NULL == pCtrlBlock) || (NULL == pDrcb))
         {
                   return;
         }
         pPrev = pCtrlBlock->pWrittingList;
         if(NULL == pPrev)
         {
                   return;
         }
         if(pPrev == pDrcb)  //The DRCB to delete is the first one in writting list.
         {
                   pCtrlBlock->pWrittingList = pPrev->lpNext;
                   if(NULL == pPrev->lpNext)
                   {
                            pCtrlBlock->pWrittingListTail = NULL;
                   }
                   return;
         }
         //Not the fist one,should travel the whole list to locate it.
         pNext = pPrev->lpNext;
         while(pNext != pDrcb)
         {
                   if(NULL == pNext)  //Can not find,break.
                   {
                            return;
                   }
                   pPrev = pNext;
                   pNext = pNext->lpNext;
         }
         //Find the DRCB,delete it.
         pPrev->lpNext = pNext->lpNext;
         if(NULL == pNext->lpNext)  //Last one.
         {
                   pCtrlBlock->pWrittingListTail = pPrev;
         }
         return;
}
 
//Write operations for COM interface.This routine will be called
//by API WriteFile,the device object's handle as input.
static DWORD UsartDeviceWrite(__COMMON_OBJECT* lpDriver,__COMMON_OBJECT* lpDevice,__DRCB* lpDrcb)
{
         DWORD                   dwWriteSize    = 0;
         //CHAR*                   lpToWrite      = NULL;
         __USART_CONTROL_BLOCK*    pCtrlBlock     = NULL;
         DWORD                   dwFlags        = 0;
 
         //Parameters check.
         if((lpDrcb->dwStatus != DRCB_STATUS_INITIALIZED) || (lpDrcb->dwRequestMode != DRCB_REQUEST_MODE_WRITE))
         {
                   goto __TERMINAL;
         }
         if(NULL == lpDevice)
         {
                   goto __TERMINAL;
         }
         //Get COM control block.
         pCtrlBlock = (__USART_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;
         if(NULL == pCtrlBlock)
         {
                   goto __TERMINAL;
         }
         //Queue the request.
         __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         if(NULL == pCtrlBlock->pWrittingListTail)  //The first DRCB.
         {
                   pCtrlBlock->pWrittingList = lpDrcb;
                   pCtrlBlock->pWrittingListTail = lpDrcb;
                   lpDrcb->lpNext = NULL;
                   //Enable writting buffer empty interrupt.
                   EnableTransferCompleteInt(pCtrlBlock->dwUsartBase);
                   //Initialize the sending status machine and send one byte to cause WBE interrupt.
                   pCtrlBlock->pCurrWritting = lpDrcb;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)lpDrcb->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = lpDrcb->dwInputLen;
                   if(__LLSend(pCtrlBlock->dwUsartBase,*pCtrlBlock->pWrittingPtr))
                   {
                            pCtrlBlock->pWrittingPtr ++;
                            pCtrlBlock->nWrittingPtr ++;
                   }
         }
         else  //Not the first DRCB,just putting into writting queue and wait.
         {
                   pCtrlBlock->pWrittingListTail->lpNext = lpDrcb;
                   pCtrlBlock->pWrittingListTail = lpDrcb;
                   lpDrcb->lpNext = NULL;
         }
		 __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
         //Wait for the operation to finish.
         switch(lpDrcb->WaitForCompletion((__COMMON_OBJECT*)lpDrcb))
         {
                   case OBJECT_WAIT_RESOURCE:  //Write OK before time out.
                            dwWriteSize = lpDrcb->dwInputLen;
                            break;
                   case OBJECT_WAIT_TIMEOUT:   //Time out before the writting is over.
                            if(lpDrcb->dwStatus == DRCB_STATUS_SUCCESS)  //Writting over before this kernel thread is scheduled.
                            {
                                     dwWriteSize = lpDrcb->dwInputLen;
                            }
                            else  //The DRCB still pending in writting list of COM interface.
                            {
                                     DeleteWriteDrcb(pCtrlBlock,lpDrcb);  //Delete from pending list.
                                     lpDrcb->dwStatus = DRCB_STATUS_FAIL;
                                     dwWriteSize = 0;
                                     break;
                            }
                            break;
                   default:
                            break;
         }
 
__TERMINAL:
         return dwWriteSize;
}


//OpenDevice operations,it will be called indirectly by CreateFile.
static __COMMON_OBJECT* UsartDeviceOpen(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,
                                                                                      __DRCB* lpDrcb)
{
         __COMMON_OBJECT*      pRet       = NULL;
         __USART_CONTROL_BLOCK*  pCtrlBlock = NULL; 
         DWORD                 dwFlags;
 
         if(NULL == lpDev)
         {
                   goto __TERMINAL;
         }
         //Get interface control block from device extension.
         pCtrlBlock = (__USART_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;
         if(NULL == pCtrlBlock)
         {
                   goto __TERMINAL;
         }
         //The COM device can noly be opened once.
         __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         if(0 == pCtrlBlock->nOpenCount)  //Not opened yet.
         {
                   pCtrlBlock->nOpenCount ++;
                   pRet = lpDev;
         }
         else  //Opened yet.
         {
                   pRet = NULL;
         }
         __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
 
__TERMINAL:
         return pRet;
}
 
//CloseDevice operations,it will be called indirectly by CloseFile.
static DWORD UsartDeviceClose(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,
                                                                 __DRCB* lpDrcb)
{
         __USART_CONTROL_BLOCK*  pCtrlBlock = NULL; 
         DWORD                 dwFlags;
 
         if(NULL == lpDev)
         {
                   goto __TERMINAL;
         }
         //Get interface control block from device extension.
         pCtrlBlock = (__USART_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;
         if(NULL == pCtrlBlock)
         {
                   goto __TERMINAL;
         }
         __ENTER_CRITICAL_SECTION(NULL,dwFlags);
         if(1 == pCtrlBlock->nOpenCount)
         {
                   pCtrlBlock->nOpenCount --;
         }
         __LEAVE_CRITICAL_SECTION(NULL,dwFlags);
 
__TERMINAL:
         return 0;
}


//Driver entry routine.
BOOL UsartDrvEntry(__DRIVER_OBJECT* lpDriverObject)
{
         __DEVICE_OBJECT*      pDevObject[USART_DEVICE_NUM];
         BOOL                  bResult     = FALSE;
         int                   i,j;
         
	       //Initialize each COM interface device in ComCtrlBlock array.
         for(i = 0;i < USART_DEVICE_NUM;i ++)
         {
                   pDevObject[i] = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
                            UsartCtrlBlock[i].pUsartName,
                            0,
                            1,
                            1024,
                            1024,
                            &UsartCtrlBlock[i],
                            lpDriverObject);
                   if(NULL == pDevObject[i])  //Failed to create device object.
                   {
                            //PrintLine("COM Driver: Failed to create device object for COM.");
										        SER_PutString("USART Driver: Failed to create device object for USART.\r\n");
                            goto __TERMINAL;
                   }
                   //Connect each COM interface's interrupt.
                   UsartCtrlBlock[i].hInterrupt = System.ConnectInterrupt(
									          (__COMMON_OBJECT*)&System,
                            UsartIntHandler,
                            (LPVOID)&UsartCtrlBlock[i],
                            UsartCtrlBlock[i].ucVector,
									          0,0,0,FALSE,0);
                   if(NULL == UsartCtrlBlock[i].hInterrupt)  //Can not connect interrupt.
                   {
                            //PrintLine("COM Driver: Failed to connect interrupt for a COM device.");
										        SER_PutString("USART Driver: Failed to connect interrupt for a USART device.\r\n");
                            goto __TERMINAL;
                   }
                   //Initialize the COM interface controller.
                   if(!InitializeUsart(UsartCtrlBlock[i].dwUsartBase))
                   {
                            goto __TERMINAL;
                   }
         }
				 
         //Asign call back functions of driver object.
         lpDriverObject->DeviceDestroy = UsartDestroy;
         lpDriverObject->DeviceWrite   = UsartDeviceWrite;
         lpDriverObject->DeviceRead    = UsartDeviceRead;
         lpDriverObject->DeviceOpen    = UsartDeviceOpen;
         lpDriverObject->DeviceClose   = UsartDeviceClose;
			 
         bResult = TRUE; //Indicate the whole process is successful.
__TERMINAL:
         if(!bResult)  //Should release all resource allocated above.
         {
                   for(j = 0;j <= i;j ++)
                   {
                            if(pDevObject[j])
                            {
                                     //Destroy the device object.
                                     IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
                                               pDevObject[j]);
                            }
                            if(UsartCtrlBlock[j].hInterrupt)
                            {
                                     //Disconnect interrupt.
                                     System.DisconnectInterrupt(
															           (__COMMON_OBJECT*)&System,
															           (__COMMON_OBJECT*)UsartCtrlBlock[j].hInterrupt);
                            }
                   }
         }
         return bResult;
}
#endif //__CFG_SYS_DDF
