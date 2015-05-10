#include "stdio.h"
#include "stm32f10x_lib.h"
#include "stm32sys.h"
#include "lwip/opt.h"

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

	/* ADCCLK = PCLK2/4 */
        RCC_ADCCLKConfig(RCC_PCLK2_Div4);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }

    /* Enable ADC1 and GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);


    //这里是模块时钟，需要使用的
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3|\
                           RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|\
                           RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|\
                           RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO,
                           ENABLE);
    /* Enable the SDIO AHB Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);
    /* Enable the DMA2 Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
}

void EnableSDIOClk()
{
	  /* Enable ADC1 and GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);


    //这里是模块时钟，需要使用的
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB1Periph_USART3|\
                           RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|\
                           RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|\
                           RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO,
                           ENABLE);
	
	/* Enable the SDIO AHB Clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);
  /* Enable the DMA2 Clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
}

/*******************************************************************************
* Function Name  : watch_dog_config
* Description    : Configures system watch dog 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void watch_dog_config(void)
{
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    /* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    /* Set counter reload value T=(fn(分频系数)/4)*0.1*RLR(重新装载值)  */
    IWDG_SetReload(0xffff);	  //500ms
    /* Reload IWDG counter喂狗 */
    IWDG_ReloadCounter();
    //IWDG_Enable();

}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

#ifdef  VECT_TAB_RAM  
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /*串口1，引脚功能*/
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP; //GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*串口2，引脚功能*/
    /* Configure USART1 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP; //GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure USART1 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*串口3，引脚功能*/
    /* Configure USART1 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF_PP;//GPIO_Mode_Out_PP; //GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* Configure USART1 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*SDIO 管脚配置*/
    /* Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* Configure PD.02 CMD line */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Power control */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* D1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* D2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* 继电器 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//default config pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//default config pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIOC->ODR&=~(1<<1);//PC14-0


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void SDIO_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*SDIO 管脚配置*/
    /* Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* Configure PD.02 CMD line */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Power control */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* D1 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* D2 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* 继电器 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//default config pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//default config pin
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIOC->ODR&=~(1<<1);//PC14-0


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

#define ADC1_DR_Address    ((u32)0x4001244C)
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
vu16 ADCConvertedValue;
ErrorStatus HSEStartUpStatus;
void adc_init()
{
    /* DMA1 channel1 configuration*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);

    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    /* ADC1 regular channel13 configuration */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_55Cycles5);

    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* Enable ADC1 reset calibaration register */
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

int get_adcval()
{
    return ADC_GetConversionValue(ADC1);
}

/*******************************************************************************
* Function Name  : USART_Configuration
* Description    : Configures the USART1.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART_Configuration(USART_TypeDef *USARTx)
{
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef  USART_ClockInitStructure;
    /* USART1 configuration -----------------------------------------------	-------*/
    /* USART1 configured as follow:
	        - BaudRate = 115200 baud  
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	        - USART Clock disabled
	        - USART CPOL: Clock is active low
	        - USART CPHA: Data is captured on the middle 
	        - USART LastBit: The clock pulse of the last data bit is not 

	output to 
	                         the SCLK pin
	  */
    USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
    USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
    USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
    /* Configure the USART1 synchronous paramters */
    USART_ClockInit(USARTx, &USART_ClockInitStructure);
    USART_InitStructure.USART_BaudRate = 115200;//115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* Configure USART1 basic and asynchronous paramters */
    USART_Init(USARTx, &USART_InitStructure);
    /* Enable USART1 Receive and Transmit interrupts */
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    /* Enable USART1 */
    USART_Cmd(USARTx, ENABLE);
}


/*********************定时器3供lwip tcp使用*****************************************************/
void enable_timer(void)
{
    TIM_Cmd(TIM2, ENABLE);
}
void disable_timer(void)
{
    TIM_Cmd(TIM2, DISABLE);
}

void timerbase_config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    /*timer2 for uart process*/
    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = TCP_TMR_INTERVAL*10;//250ms SYS TIMER
    TIM_TimeBaseStructure.TIM_Prescaler = 0;//10k
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    /* Prescaler configuration 0~65535(128M/(per+1))=128M/(127+1)=1M*/
    TIM_PrescalerConfig(TIM2,7199, TIM_PSCReloadMode_Immediate); //10k
    /* TIM IT enable */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //TIM_IT_CC1
    TIM_Cmd(TIM2, ENABLE);
}


/*******************************************************************************
* Function Name  : fputc
* Description    : Retargets the C library printf function to the USART.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//int fputc(int ch, FILE *f)
//{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
//    while(USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_TC) == RESET){
//    }
//    if(ch=='\n')
//        fputc('\r',f);
//    USART_SendData(CONSOLE_UART, (u8)ch);
    /* Loop until the end of transmission */
//    while(USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_TC) == RESET){
//    }
//    return ch;
//}

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
    /* User can add his own implementation to report the file name and line number */

    _hx_printf("\n\r Wrong parameter value detected on\r\n");
    _hx_printf("       file  %s\n", file);
    _hx_printf("       line  %d\n", line);
    while (1){
    }
}

void send_char(char ch)
{
    USART_SendData(CONSOLE_UART, (u8) ch);
    while(USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_TC) == RESET);
}

char get_char(void)
{
    while(!(USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_RXNE) == SET)){
    }
    return (USART_ReceiveData(CONSOLE_UART));
}


char Uart_GetKey(void)
{
    if((USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_RXNE) == SET))
        return (USART_ReceiveData(CONSOLE_UART));
    else
        return 0;
}

void Uart_SendByte(u8 ch)
{
    USART_SendData(CONSOLE_UART, (u8) ch);
    while(USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_TC) == RESET);
}


/*部分串口函数*/
//====================================================================
void Uart_GetString(char *string)
{
    char *string2 = string;
    char c;
    while(1)
    {
        c = get_char();
        if(c=='\r')
            break;
        else if(c=='\b')
        {
            if( (int)string2 < (int)string )
            {
                _hx_printf("\b \b");
                string--;
            }
        }
        else 
        {
            *string++ = c;
            send_char(c);
        }
    }
    *string='\0';
    _hx_printf("\n");
}
//*****************************************************************************
//get a number for the uart
//*****************************************************************************
int Uart_GetIntNum_GJ(void)
{
    char string[16] ;
    char *p_string=string ;
    int ret=1;
    int i = 0 ;
    int data = 0 ;
    Uart_GetString(p_string);
    i = 0 ;
    if(string[0]=='-' ){
        ret=-1;
        i++;
    }
    while( string[i] != '\0' )
    {
        data = data * 10 ;
        if( string[i]<'0'||string[i]>'9' ){
            _hx_printf("input error!\n");
            return -1 ;
        }
        data = data + ( string[i]-'0' ) ;
        i++ ;
    }
    return (data*ret );
}
//*****************************************************************************




/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
void dbg_void(void)
{
    return;
}
void print_void(void *fmt,...)
{
    return;
}
void debug_data_stream(char *info,char *pdata,u16 len)
{
    u16 i;
    _hx_printf("%s(len=%d):\n",info,len);
    for(i=0;i<len;i++){
        if((!(i%10))&&(i))
            _hx_printf("\n");
        _hx_printf("0x%2x     ",*pdata++);
    }
    _hx_printf("\n");
}


/****************读写默认配置flash***********************************************/
int write_flash(u32 StartAddr,u16 *buf,u16 len)
{
    volatile FLASH_Status FLASHStatus;
    u32 FlashAddr;
    len=(len+1)/2;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FlashAddr=StartAddr;
    FLASH_ErasePage(StartAddr);
    while(len--){
        FLASHStatus = FLASH_ProgramHalfWord(FlashAddr,*buf++);
        if (FLASHStatus != FLASH_COMPLETE){
            _hx_printf("FLSH :Error %08X\n\r",FLASHStatus);
            return -1;
        }
        FlashAddr += 2;
    }
    FLASH_Lock();
    return 0;
}
int read_flash(u32 StartAddr,u16 *buf,u16 len)
{
    u16 *p=(u16 *)StartAddr;
    len=(len+1)/2;
    while(len--){
        *buf++=*p++;
    }
    return 0;
}

