//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 2,2014
//    Module Name               : sdio_drv.h
//    Module Funciton           : 
//                                SDIO driver's implementation.
//                                It's skeleton part of SDIO driver to adapt HelloX
//                                device driver framework.
//                                Actual function code is in some other files under
//                                the arch/stm32 directory.
//
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include <stm32f10x.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sdio_drv.h"

//Interupt handler of SDIO,it calls stm32_irq routine directly,which is a global routine.
extern stm32_irq(void);
static BOOL SDIOIntHandler(LPVOID lpESP,LPVOID lpParam)
{
	stm32_irq();
	return TRUE;
}

//External routine,used to enable SDIO and DMA clock.
extern void EnableSDIOClk(void);

//Configure SDIO related GPIO.
extern void SDIO_GPIO_Configuration(void);

//Driver entry routine.
BOOL SDIODriverEntry(__DRIVER_OBJECT* lpDriverObject)
{
         __DEVICE_OBJECT*      pDevObject  = NULL;
         BOOL                  bResult     = FALSE;
	       __COMMON_OBJECT*      hInterrupt  = NULL;
         
	       //Create device object for SDIO.
         pDevObject = IOManager.CreateDevice((__COMMON_OBJECT*)&IOManager,
                            SDIO_DEV_NAME,
                            0,
                            1,
                            1024,
                            1024,
                            NULL,
                            lpDriverObject);
         if(NULL == pDevObject)  //Failed to create device object.
         {
					 _hx_printf("SDIO driver: Can not create device object.\r\n");
					 bResult = FALSE;
         }
         
				 //Connect SDIO's interrupt.
         hInterrupt = System.ConnectInterrupt(
									          (__COMMON_OBJECT*)&System,
                            SDIOIntHandler,
                            NULL,
                            SDIO_INT_VECTOR,
									          0,0,0,FALSE,0);
         if(NULL == hInterrupt)  //Can not connect interrupt.
         {
					 _hx_printf("SDIO driver: Can not connect interrupt of SDIO.\r\n");
					 goto __TERMINAL;
         }
				 
				 //Enable SDIO and related DMA clock first.
				 EnableSDIOClk();
				 
				 //Configure SDIO related GPIO.
				 SDIO_GPIO_Configuration();
				 
				 //Enable interrupt of SDIO.
				 NVIC_SetPriority(SDIO_IRQn,0x0F);
				 NVIC_EnableIRQ(SDIO_IRQn);
				 
         bResult = TRUE; //Indicate the whole process is successful.
				 
__TERMINAL:
         if(!bResult)  //Should release all resource allocated above.
         {
					 IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
                                    pDevObject);
           if(hInterrupt)
					 {
						 //Disconnect interrupt.
             System.DisconnectInterrupt(
						   (__COMMON_OBJECT*)&System,
						   hInterrupt);
           }
         }
         return bResult;
}
