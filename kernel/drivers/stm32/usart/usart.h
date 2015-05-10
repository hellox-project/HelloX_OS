//***********************************************************************/
//    Author                    : Garry
//    Original Date             : March 07, 2014
//    Module Name               : usart.h
//    Module Funciton           : 
//                                Header file for USART device driver under STM32 chip.
//                                ****** MEMO ******
//                                Today is 8th of March,got news from phone that
//                                a flight(MH370) from KL bound to Beijing operated by 
//                                Malaysia airlines was lost,the aircraft is 777,
//                                well known by it's big and comfort and it's safety...
//                                Pray for all passengers and all crew members on
//                                this flight...
//                                
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/


#ifndef __USART_H__
#define __USART_H__ 


//Data buffer's length of each USART interface.
#define COM_BUFF_LENGTH 1600


//COM interface names.
#define USART1_NAME "\\\\.\\USART1"
#define USART2_NAME "\\\\.\\USART2"
//You can add more USART device name follow above.


//Base address/port_number for USART1 and USART2 devices.
#define __USART1_BASE USART1_BASE
#define __USART2_BASE USART2_BASE


//Default interrupt vectors for USART1 and USART2.
#define USART1_INT_VECTOR 53
#define USART2_INT_VECTOR 54


//Structures to manage USART interface,one for each and it will
//be the device extension part of USART device object.
typedef struct{
         CHAR*             pUsartName;   //Usart interface's name.
         DWORD             dwUsartBase;  //Base address of USART device.
	       CHAR              ucVector;     //Interrupt vector number of USART device.
	
         LPVOID            hInterrupt; //Handle of this device's interrupt object.
         int               nOpenCount;     //How many times the interface is opened.
         __DRCB*           pReadingList;   //DRCB list for reading.
         __DRCB*           pReadingListTail; //Reading list's tail,to simply operation of DRCB list.
         __DRCB*           pWrittingList;  //DRCB list for writting.
         __DRCB*           pWrittingListTail; //Witting list's tail.
         __DRCB*           pCurrWritting;  //The DRCB that is in writting process.
         volatile int      nReadingPtr;    //Reading buffer's current position.
         CHAR*             pReadingPtr;    //Reading buffer's pointer.
         volatile int      nReadingSize;   //How many bytes to read.


         volatile int      nWrittingPtr;   //Writting buffer's current position.
         CHAR*             pWrittingPtr;   //Writting buffer's pointer.
         volatile int      nWrittingSize;  //How many bytes to write.


         //USART interface's data buffer and it's pointer.
         CHAR              Buffer[COM_BUFF_LENGTH];
         volatile int      nBuffHeader;
         volatile int      nBuffTail;
}__USART_CONTROL_BLOCK;


//A macro used to add extra COM/USART/UART device into COM device array.
#define ADD_USART_DEVICE(name,base,intvec) \
{                                        \
         name,                                \
         base,                                \
         intvec,                              \
         NULL,                                \
         0,                                   \
         NULL,                                \
         NULL,                                \
         NULL,                                \
         NULL,                                \
         NULL,                                \
         0,                                   \
         NULL,                                \
         0,                                   \
         0,                                   \
         NULL,                                \
         0,                                   \
{0},                                     \
         0,                                   \
         0                                    \
}


//Define some macros to simplify the porting of COM driver between different hardware
//platforms.
//A helper macro to enable writting buffer empty interrupt.
#ifdef __I386__
#define EnableWBInt(base)  __outb(0x0F,base + 1)
#else
#define EnableWBInt(base)
#endif


//Disable writting buffer empty interrupt.
#ifdef __I386__
#define DisableWBInt(base) __outb(0x01,base + 1)
#else
#define DisableWBInt(base)
#endif


//Send out a byte.
#ifdef __I386__
#define COMSendByte(bt,base) __outb(bt,base)
#else
#define COMSendByte(bt,base)
#endif


//Receive a byte from COM interface.
#ifdef __I386__
#define COMRecvByte(base) __inb(base)
#else
#define COMRecvByte(base) (base)
#endif
 
//Entry point of COM device driver,all COM interfaces,include
//COM1 and COM2 and others,will use the only driver entry routine.
BOOL UsartDrvEntry(__DRIVER_OBJECT* lpDriverObject);


#endif //__USART_H__
