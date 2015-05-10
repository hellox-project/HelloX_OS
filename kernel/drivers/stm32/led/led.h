//Header file for LED device driver.
#ifndef __LED_H__
#define __LED_H__

//Macros to operate LEDs.
#define  LED0 (1<<5)  //led0     PB5 
#define  LED1 (1<<5)  //led1     PE5 
#define LED0_SET(x) GPIOB->ODR=(GPIOB->ODR&~LED0)|(x ? LED0:0) 
#define LED1_SET(x) GPIOE->ODR=(GPIOE->ODR&~LED1)|(x ? LED1:0) 

//Driver entry point of LED device driver.
BOOL LEDDrvEntry(__DRIVER_OBJECT* lpDriverObject);

#endif
