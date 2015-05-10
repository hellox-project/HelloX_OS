//LED driver souce code,written by garry.xin in May 10,2014.
#include <stm32f10x.h>
#ifndef __STDAFX_H__
#include "..\..\include\kapi.h"
#endif

#ifndef __LED_H__
#include "led.h"
#endif

//Initialize LEDs in STM32 EVL board.
static void InitLED()
{
	RCC->APB2ENR|=1<<3;    //??PORTB??           
  RCC->APB2ENR|=1<<6;    //??PORTE??             
  GPIOB->CRL&=0XFF0FFFFF;   
  GPIOB->CRL|=0X00300000;//PB.5  ????         
  GPIOB->ODR|=1<<5;      //PB.5  ???                      
  GPIOE->CRL&=0XFF0FFFFF; 
  GPIOE->CRL|=0X00300000;//PE.5 ???? 
  GPIOE->ODR|=1<<5;      //PE.5 ???   
	
	//Turn off all LEDs.
	LED0_SET(1);
	LED1_SET(1);
}

//LED write operation.
static DWORD LEDDeviceWrite(__COMMON_OBJECT* lpDriver,__COMMON_OBJECT* lpDevice,__DRCB* lpDrcb)
{
	char* pData = NULL;
	
	if(NULL == lpDrcb)
	{
		return 0;
	}
	pData = (char*)lpDrcb->lpInputBuffer;
	if(0 == *pData) //Write 0 to LED,implies turn LED off.
	{
		LED0_SET(1);
	}
	else  //Write non-zero value,implies turn on LED.
	{
		LED0_SET(0);
	}
	return 1;
}

//Driver entry point of LED device driver.
BOOL LEDDrvEntry(__DRIVER_OBJECT* lpDriverObject)
{
	HANDLE pLedDevice = NULL;
	
	//Initialize LED device first.
	//In some case the initialization process for some devices may fail,you should
	//return FALSE directly in this scenario,it will cause the OS kernel to stop loading
	//device driver.
	InitLED();
	
	pLedDevice = CreateDevice
	    ("\\\\.\\LED0",
	     0,1,1024,1024,
	     NULL,lpDriverObject);
	if(NULL == pLedDevice)  //Failed to create device object.
	{
		return FALSE;
	}
	
	//Initialize driver operations.
	lpDriverObject->DeviceWrite = LEDDeviceWrite;
	
	return TRUE;
}
