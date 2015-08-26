//***********************************************************************/
//    Author                    : tywind
//    Original Date             : aug 11 2015
//    Module Name               : ApploaderDef.H
//    Module Funciton           : 
//    Lines number              :
//***********************************************************************/


#ifndef __APPLOADER_DEF_H__
#define __APPLOADER_DEF_H__

//app module type

#define  APPTYPE_UNKNOW     0x0    // 

#define  APPTYPE_PE			0x1    //windows exe or dll

#define  APPTYPE_ELF		0x2    // linux 

#define  APPTYPE_STM32      0x3     // arm 

#define  APPTYPE_ERROR      0xFFFF  // error 


// app main entry define
typedef int (* PAPP_MAIN)(int argc, char* argv[]);

typedef BOOL (*__APPFOTMAT_ENTRY)(HANDLE hFileObj);

typedef LPBYTE (*__LOADAPP_ENTRY)(HANDLE hFileObj);

//A array object to manage the drivers in system.
typedef struct
{
	__APPFOTMAT_ENTRY   CheckFormat;
	__LOADAPP_ENTRY     LoadApp;
	
}__APP_ENTRY;


BOOL    AppFormat_PE(HANDLE hFileObj);
LPBYTE  LoadAppToMemory_PE(HANDLE hFileObj);

BOOL    AppFormat_ELF(HANDLE hFileObj);
LPBYTE  LoadAppToMemory_ELF(HANDLE hFileObj);

BOOL    AppFormat_STM32(HANDLE hFileObj);
LPBYTE  LoadAppToMemory_STM32(HANDLE hFileObj);


#endif  //__APPLOADER_DEF_H__
