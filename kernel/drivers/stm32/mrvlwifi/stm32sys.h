#ifndef __SYSTEM__H__
#define __SYSTEM__H__

#include "stm32f10x_lib.h"
#include "stdio.h"

//Definition of TCP_TMR_INTERVAL,which maybe defined in lwIP statck.
#ifndef TCP_TMR_INTERVAL
#define TCP_TMR_INTERVAL       250  /* The TCP timer interval in milliseconds. */
#endif

#define CONSOLE_UART  USART1  //默认控制台串口

#define LED1(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<6):(GPIOB->ODR&(~(1<<6)))
#define LED2(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<7):(GPIOB->ODR&(~(1<<7)))
#define LED3(tmp) GPIOA->ODR=tmp?(GPIOA->ODR|1<<4):(GPIOA->ODR&(~(1<<4))) //这个控制LED的可以设置为控制继电器

#define MAR_POW(tmp) GPIOB->ODR=tmp?(GPIOB->ODR|1<<12):(GPIOB->ODR&(~(1<<12)))

#define DEFAULT_PIN  (GPIOC->IDR&(1<<0))//PC0

void dbg_void(void);
void print_void(void *fmt,...);
/*debug */
#define dbg_enter() dbg_void()//printf("enter %s\n",__func__)//dbg_void()//
#define dbg_leave() dbg_void()//printf("leave %s\n",__func__)//dbg_void()//
#define dbg_malloc print_void//printf




void enable_timer(void);
void disable_timer(void);
void Delay(u32 nTime);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void USART_Configuration(USART_TypeDef *USARTx);
void timerbase_config(void);
void SPI1_Config(void);
void EXTI_Configuration(void);
void debug_data_stream(char *info,char *pdata,u16 len);
char Uart_GetKey(void);
void Uart_SendByte(u8 ch);
void Uart_GetString(char *string);
int Uart_GetIntNum(void);
int Uart_GetIntNum_GJ(void);
int write_flash(u32 StartAddr,u16 *buf,u16 len);
int read_flash(u32 StartAddr,u16 *buf,u16 len);


void RCC_Configuration(void);
void watch_dog_config(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void assert_failed(u8* file, u32 line);

//外部申明
extern void adc_init(void);
extern int get_adcval(void);
#endif
