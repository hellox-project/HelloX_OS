//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Aug 25,2015
//    Module Name               : Apploader.c
//    Module Funciton           : 
//                                
//                                
//                                
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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "../shell/shell.h"
#include "AppLoaderDef.h"

__APP_ENTRY AppEntryArray[] = 
{
	//PE
	{ AppFormat_PE, LoadAppToMemory_PE },
	//ELF
	{ AppFormat_ELF, LoadAppToMemory_ELF },
	//STM32
	{ AppFormat_STM32, LoadAppToMemory_STM32},
	//Terminator .
	{ NULL, NULL }
};

static  PAPP_MAIN   s_pAppMain = NULL;

static DWORD AppEntryPoint(LPVOID pData)
{
	__CMD_PARA_OBJ* pCmdParaObj = (__CMD_PARA_OBJ*)pData;
	

	if (NULL == pData)
	{
		return	 s_pAppMain(0, NULL);
	}
	else
	{
		return s_pAppMain(pCmdParaObj->byParameterNum, pCmdParaObj->Parameter);
	}
		
}

static BOOL StartRunApp(DWORD dwStartAddress,LPVOID p,LPSTR pAppName)
{
	__KERNEL_THREAD_OBJECT*   hKernelThread  = NULL;
	BOOL                      bResult        = FALSE;

	if(dwStartAddress <= 0x00100000) //Low end 1M memory is reserved.
	{
		goto __TERMINAL;
	}

	s_pAppMain = (PAPP_MAIN)dwStartAddress;
	//Create a kernel thread to run the binary module.
	hKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		(__KERNEL_THREAD_ROUTINE)AppEntryPoint,
		p,
		NULL,
		pAppName);

	if(NULL == hKernelThread)  //Can not create the thread.
	{
		goto __TERMINAL;
	}

	//Switch input focus to the thread.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)hKernelThread);
	hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);  //Block shell to wait module over.
	
	//Destroy the module's kernel thread.
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)hKernelThread);

	//Switch back input focus to shell.
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		NULL);
	bResult = TRUE;

__TERMINAL:

	return bResult;
}

static HANDLE OpenAppFile(LPSTR pAppFilePath)
{
	HANDLE	hBinFile      = NULL;
	
	hBinFile = CreateFile(pAppFilePath,FILE_ACCESS_READ,0,NULL);
	if(hBinFile == NULL)
	{		
		return NULL;
	}

	if(GetFileSize(hBinFile,NULL) <= 512)  
	{	
		//Too short,invalid format.
		CloseFile(hBinFile);
		hBinFile = NULL;		
	}
	
	return hBinFile;
}

// load and run app module 
BOOL RunDynamicAppModule(LPSTR pAppFilePath,LPVOID p)
{
	__APP_ENTRY*  pAppEntry    = AppEntryArray;	
	HANDLE        hFileObj     = NULL;	
	LPBYTE        pAppBuf      = NULL;	
	BOOL          bRunOk       = FALSE;	

	while(bRunOk == FALSE)
	{
		hFileObj = OpenAppFile(pAppFilePath);
		if(hFileObj == NULL)
		{
			PrintLine("Can not open the specified app file.");
			break;
		}
		
		// look up app format
		while(pAppEntry->CheckFormat)
		{
			if(pAppEntry->CheckFormat(hFileObj))
			{
				pAppBuf = pAppEntry->LoadApp(hFileObj); //(DWORD)pAppBuf);
				break;
			}
			pAppEntry ++;			
		}

		//Load model failed.
		if(pAppBuf == NULL)
		{
			PrintLine("Load binary model error.");
			break;
		}
				
		if(!StartRunApp((DWORD)pAppBuf,p,strrchr(pAppFilePath,'\\')))
		{
			PrintLine("Failed to run binary model.");
			break;
		}
		bRunOk = TRUE;
	}

	CloseFile(hFileObj);
	if(pAppBuf)
	{
		_hx_free(pAppBuf);
	}
	return bRunOk;
}
