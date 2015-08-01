//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 05,2013
//    Module Name               : console.h
//    Module Funciton           : 
//                                This module countains the definations,
//                                data types,and procedures of console object.
//    Last modified Author      : Garry
//    Last modified Date        : 
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __CONSOLE_H__
#define __CONSOLE_H__



#include "commobj.h"
#include "dim.h"
#include "hellocn.h"

#include "ktmsg.h"

#include "process.h"


#ifdef __cplusplus
extern "C" {
#endif

//Available when and only when the __CFG_SYS_CONSOLE macro is defined.
#ifdef __CFG_SYS_CONSOLE
	
//Default output usert under STM32 platform.
#ifdef __STM32__
#define DEFAULT_USART USART1
#endif

//CONSOLE object's definition.
typedef struct tag__CONSOLE{
	BOOL           bInitialized;   //If the console object is initialized.
	BOOL           bLLInitialized; //If low level output function is initialized.
	int            nRowNum;        //Current row number.
	int            nColNum;        //Current column number.

	__COMMON_OBJECT* hComInt;      //Handle of COM interface.
	__KERNEL_THREAD_OBJECT* hConThread;   //Handle of console reading thread.

	//Initializer and un-initializer routines.
	BOOL           (*Initialize)(struct tag__CONSOLE*);
	VOID           (*Uninitialize)(struct tag__CONSOLE*);

	//Output operation routines.
	void (*PrintStr)(const char* pszMsg);    //Print out a string.
	void (*ClearScreen)(void);               //Clear the whole screen.
	void (*PrintCh)(unsigned short ch);      //Print out a character at current position.
	void (*GotoHome)(void);
	void (*ChangeLine)(void);
	void (*GotoPrev)(void);
	void (*PrintLine)(const char* pszString);  //Print out a whole string.

	//Input operation routines.
	int  (*getch)();                         //Get one characeter from console.
	int  (*getchar)();                       //Asynomy of getchar in DOS.
}__CONSOLE;

//Maximal column number.
#define CON_MAX_COLNUM    255
#define CON_MAX_ROWNUM    80

//Default COM interface's device name.
#if defined(__I386__)
#define CON_DEF_COMNAME   "\\\\.\\COM1"
#elif defined(__STM32__)
#define CON_DEF_COMNAME   "\\\\.\\USART1"
#endif

//Default COM interface's base address,which is used as low level output facility.
#define CON_DEF_COMBASE   0x03F8

//Console thread's name.
#define CON_THREAD_NAME "CON_RD"

//Declaration of global CONSOLE object.
extern __CONSOLE Console;

#ifdef __cplusplus
}
#endif

#endif  //__CFG_SYS_CONSOLE.

#endif  //__CONSOLE_H__
