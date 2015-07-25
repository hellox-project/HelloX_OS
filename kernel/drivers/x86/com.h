//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 16, 2013
//    Module Name               : MOUSE.H
//    Module Funciton           : 
//                                This module countains the pre-definitions or
//                                structures for COM device driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __COM_H__
#define __COM_H__ 

//Data buffer's length of each COM interface.
#define COM_BUFF_LENGTH 1600

//COM interface names.
#define COM1_NAME "\\\\.\\COM1"
#define COM2_NAME "\\\\.\\COM2"

//Base address/port_number for COM1 and COM2 devices.
#define COM1_BASE 0x3F8
#define COM2_BASE 0x2F8

//Default interrupt vectors for COM1 and COM2
#define COM1_INT_VECTOR 0x24  //COM1's interrupt vector.
#define COM2_INT_VECTOR 0x23  //COM2's interrupt vector.

//Structures to manage COM interface,one for each and it will
//be the device extension part of COM device object.
typedef struct{
         CHAR*             pComName;   //COM interface's name.
//#ifdef __I386__
         WORD              wBasePort;  //IO port base address.
         BYTE              IntVector;  //Interrupt vector number.
//#else
//#endif
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

         //COM interface's data buffer and it's pointer.
         CHAR              Buffer[COM_BUFF_LENGTH];
         volatile int      nBuffHeader;
         volatile int      nBuffTail;
}__COM_CONTROL_BLOCK;

//A macro used to add extra COM/USART/UART device into COM device array.
#define ADD_COM_DEVICE(name,base,intvec) \
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
		 {0},                                 \
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
#define COMRecvByte(base) base
#endif
 
//Entry point of COM device driver,all COM interfaces,include
//COM1 and COM2 and others,will use the only driver entry routine.
BOOL COMDrvEntry(__DRIVER_OBJECT* lpDriverObject);

#endif //__COM_H__

