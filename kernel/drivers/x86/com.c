//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28, 2009
//    Module Name               : MOUSE.CPP
//    Module Funciton           :
//                                This module countains the implementation code
//                                of MOUSE driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
 
#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "kapi.h"
#include "stdio.h"
#include "string.h"
#include "com.h"

//This module will be available if and only if the DDF function is enabled.
#ifdef __CFG_SYS_DDF

//COM device array,each COM/USART/UART interface existing in systen corresponding
//to one entry of this array.
#define COM_DEVICE_NUM 2
static __COM_CONTROL_BLOCK ComCtrlBlock[COM_DEVICE_NUM] = {
         ADD_COM_DEVICE(COM1_NAME,COM1_BASE,COM1_INT_VECTOR),
         ADD_COM_DEVICE(COM2_NAME,COM2_BASE,COM2_INT_VECTOR),
         //Please add extra COM interface device here.
};

//Initialization routine of all COM interface devices.
static BOOL InitializeCOM(WORD base)
{
         BOOL    bResult       = FALSE;

#ifdef __I386__
         if((base != 0x3F8) && (base != 0x2F8))
         {
			 goto __TERMINAL;;
         }
         __outb(0x80,base + 3);  //Set DLAB bit to 1,thus the baud rate divisor can be set.
         __outb(0x0C,base);  //Set low byte of baud rate divisor.
         __outb(0x0,base + 1);  //Set high byte of baud rate divisor.
         __outb(0x07,base + 3); //Reset DLAB bit,and set data bit to 8,one stop bit,without parity check.
         __outb(0x0F,base + 1); //Set all interrupts include write buffer empty interrupt.
         //__inb(base);  //Clear data buffer to avoide interrupt raising.
         //__outb(0x01,base + 1);  //Enable data available interrupt.
         __outb(0x0B,base + 4); //Enable DTR,RTS and Interrupt.
         __inb(base);  //Reset data register.

		 //Try to check if the COM interface is exist.
		 if(0xFF == __inb(base + 1))  //Port is not exist.
		 {
			 goto __TERMINAL;
		 }
         bResult = TRUE;
#else
#endif

__TERMINAL:
         return bResult;
}

//Unload entry for COM driver.
static DWORD COMDestroy(__COMMON_OBJECT* lpDriver,
                        __COMMON_OBJECT* lpDevice,
                        __DRCB*          lpDrcb)

{
         __COM_CONTROL_BLOCK* pCtrlBlock = (__COM_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;

         if(NULL == pCtrlBlock)
         {
			 return 0;
         }
         //Disconnect interrupt.
         DisconnectInterrupt((HANDLE)pCtrlBlock->hInterrupt);
         //Destroy device object.
         IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
                   (__DEVICE_OBJECT*)lpDevice);
         return 0;
}

//Handler of writting buffer empty interrupt.
static VOID WBIntHandler(__COM_CONTROL_BLOCK* pCtrlBlock)
{
         __DRCB*      pWrittingHeader = NULL;

         pWrittingHeader = pCtrlBlock->pWrittingList;
         if(NULL == pWrittingHeader)  //No DRCB is pending to write.
         {
                   DisableWBInt(pCtrlBlock->wBasePort);  //Disable WBE interrupt.
                   return;
         }
         if(pCtrlBlock->pCurrWritting != pWrittingHeader)  //The current writting DRCB maybe deleted.
         {
                   pCtrlBlock->pCurrWritting = pWrittingHeader;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)pWrittingHeader->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = pWrittingHeader->dwInputLen;
         }

         while(COMRecvByte(pCtrlBlock->wBasePort + 5) & 0x20)
         {
                   if(pCtrlBlock->nWrittingPtr == pCtrlBlock->nWrittingSize)  //Send over.
                   {
                            //Mark the DRCB is processed successfully.
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
                            COMSendByte(*pCtrlBlock->pWrittingPtr,pCtrlBlock->wBasePort);
                            //Update pointers.
                            pCtrlBlock->pWrittingPtr ++;
                            pCtrlBlock->nWrittingPtr ++;
                            continue;  //Continue to write.
                   }
                   pWrittingHeader = pCtrlBlock->pWrittingList;
                   if(NULL == pWrittingHeader)
                   {
                            pCtrlBlock->pCurrWritting = NULL;
                            DisableWBInt(pCtrlBlock->wBasePort);
                            return;
                   }
                   //Should process next DRCB object in pending queue.
                   pCtrlBlock->pCurrWritting = pWrittingHeader;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)pWrittingHeader->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = pWrittingHeader->dwInputLen;
         }
}

//Handler to handle the data available interrupt.
static VOID DAIntHandler(__COM_CONTROL_BLOCK* pCtrlBlock)
{
         CHAR                bt;

         //Process data available interrupt.
         while(COMRecvByte(pCtrlBlock->wBasePort + 5) & 1)  //Data available.
         {
                   bt = COMRecvByte(pCtrlBlock->wBasePort);  //Read the byte.
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
         while(COMRecvByte(pCtrlBlock->wBasePort + 5) & 1);  //Drain out all data from COM interface,since the buffer is full.
         //Wake up all threads waiting for COM data.
         while(pCtrlBlock->pReadingList)
         {
                   pCtrlBlock->pReadingList->OnCompletion((__COMMON_OBJECT*)pCtrlBlock->pReadingList);
                   pCtrlBlock->pReadingList = pCtrlBlock->pReadingList->lpNext;
         }
         pCtrlBlock->pReadingListTail = NULL;
}

//Interrupt handler of COM.
static BOOL COMIntHandler(LPVOID lpESP,LPVOID lpParam)
{
         __COM_CONTROL_BLOCK* pCtrlBlock = (__COM_CONTROL_BLOCK*)lpParam;
         UCHAR isr;                        //Read interrupt status register.
 
         isr = COMRecvByte(pCtrlBlock->wBasePort + 2);
         if(isr & 1)  //No interrupt to process.
         {
                   return FALSE;
         }
         if(COMRecvByte(pCtrlBlock->wBasePort + 5) & 1)  //Data available.
         {
                   DAIntHandler(pCtrlBlock);
         }
         //Process sending register empty interrupt.
         if(COMRecvByte(pCtrlBlock->wBasePort + 5) & 0x20)
         {
                   WBIntHandler(pCtrlBlock);
         }
         return TRUE;
}
 
//Read operations for COM interface.This routine will be called
//by API ReadFile,the device object's handle as input.
static DWORD ComDeviceRead(__COMMON_OBJECT* lpDriver,__COMMON_OBJECT* lpDevice,__DRCB* lpDrcb)
{
         DWORD                  dwReadSize    = 0;
         __COM_CONTROL_BLOCK*   pCtrlBlock    = NULL;
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
         pCtrlBlock = (__COM_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;
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
static BOOL __LLSend(WORD port,CHAR bt)
{
         DWORD nCount = 16;
 
         while((nCount --) > 0)  //Try nCount times.
         {
                   if(COMRecvByte(port + 5) & 32)  //Sending hold register is empty.
                   {
                            COMSendByte(bt,port);
                            return TRUE;
                   }
         }
         //Can not send out after trying several times,then give up.
         return FALSE;
}
 
//A local helper routine to delete a DRCB object from writting list.
static VOID DeleteWriteDrcb(__COM_CONTROL_BLOCK* pCtrlBlock,__DRCB* pDrcb)
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
static DWORD ComDeviceWrite(__COMMON_OBJECT* lpDriver,__COMMON_OBJECT* lpDevice,__DRCB* lpDrcb)
{
         DWORD                   dwWriteSize    = 0;
         CHAR*                   lpToWrite      = NULL;
         __COM_CONTROL_BLOCK*    pCtrlBlock     = NULL;
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
         pCtrlBlock = (__COM_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDevice)->lpDevExtension;
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
                   EnableWBInt(pCtrlBlock->wBasePort);
                   //Initialize the sending status machine and send one byte to cause WBE interrupt.
                   pCtrlBlock->pCurrWritting = lpDrcb;
                   pCtrlBlock->pWrittingPtr  = (CHAR*)lpDrcb->lpInputBuffer;
                   pCtrlBlock->nWrittingPtr  = 0;
                   pCtrlBlock->nWrittingSize = lpDrcb->dwInputLen;
                   if(__LLSend(pCtrlBlock->wBasePort,*pCtrlBlock->pWrittingPtr))
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
static __COMMON_OBJECT* ComDeviceOpen(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,
                                                                                      __DRCB* lpDrcb)
{
         __COMMON_OBJECT*      pRet       = NULL;
         __COM_CONTROL_BLOCK*  pCtrlBlock = NULL; 
         DWORD                 dwFlags;
 
         if(NULL == lpDev)
         {
                   goto __TERMINAL;
         }
         //Get interface control block from device extension.
         pCtrlBlock = (__COM_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;
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
static DWORD ComDeviceClose(__COMMON_OBJECT* lpDrv,__COMMON_OBJECT* lpDev,
                                                                 __DRCB* lpDrcb)
{
         __COM_CONTROL_BLOCK*  pCtrlBlock = NULL; 
         DWORD                 dwFlags;
 
         if(NULL == lpDev)
         {
                   goto __TERMINAL;
         }
         //Get interface control block from device extension.
         pCtrlBlock = (__COM_CONTROL_BLOCK*)((__DEVICE_OBJECT*)lpDev)->lpDevExtension;
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
 
//Main entry point of COM driver.
BOOL COMDrvEntry(__DRIVER_OBJECT* lpDriverObject)
{
         __DEVICE_OBJECT*      pDevObject[COM_DEVICE_NUM];
         BOOL                  bResult     = FALSE;
         int                   i,j;
 
         //Initialize each COM interface device in ComCtrlBlock array.
         for(i = 0;i < COM_DEVICE_NUM;i ++)
         {
                   pDevObject[i] = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
                            ComCtrlBlock[i].pComName,
                            0,
                            1,
                            1024,
                            1024,
                            &ComCtrlBlock[i],
                            lpDriverObject);
                   if(NULL == pDevObject[i])  //Failed to create device object.
                   {
                            PrintLine("COM Driver: Failed to create device object for COM.");
                            goto __TERMINAL;
                   }
                   //Connect each COM interface's interrupt.
                   ComCtrlBlock[i].hInterrupt = ConnectInterrupt(
                            COMIntHandler,
                            (LPVOID)&ComCtrlBlock[i],
                            ComCtrlBlock[i].IntVector);
                   if(NULL == ComCtrlBlock[i].hInterrupt)  //Can not connect interrupt.
                   {
                            PrintLine("COM Driver: Failed to connect interrupt for a COM device.");
                            goto __TERMINAL;
                   }
                   //Initialize the COM interface controller.
                   if(!InitializeCOM(ComCtrlBlock[i].wBasePort))
                   {
                            goto __TERMINAL;
                   }
         }
 
         //Asign call back functions of driver object.
         lpDriverObject->DeviceDestroy = COMDestroy;
         lpDriverObject->DeviceWrite   = ComDeviceWrite;
         lpDriverObject->DeviceRead    = ComDeviceRead;
         lpDriverObject->DeviceOpen    = ComDeviceOpen;
         lpDriverObject->DeviceClose   = ComDeviceClose;
         
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
                            if(ComCtrlBlock[j].hInterrupt)
                            {
                                     //Disconnect interrupt.
                                     DisconnectInterrupt((HANDLE)ComCtrlBlock[j].hInterrupt);
                            }
                   }
         }
         return bResult;
}
 
#endif
