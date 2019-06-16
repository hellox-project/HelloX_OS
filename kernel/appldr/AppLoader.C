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

#include "StdAfx.h"
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

static BOOL StartRunApp(DWORD dwStartAddress,LPVOID pParamBlock,LPSTR pAppName)
{
	__KERNEL_THREAD_OBJECT*   hKernelThread  = NULL;
	BOOL                      bResult        = FALSE;

	if(dwStartAddress <= 0x00100000) //Low end 1M memory is reserved.
	{
		goto __TERMINAL;
	}

	_hx_printf("Run [%s] at 0x%X.\r\n", pAppName, dwStartAddress);
	s_pAppMain = (PAPP_MAIN)dwStartAddress;
	/* 
	 * Create the dedicated kernel thread to hold the application 
	 * module,it's the main thread of the application.
	 * More kernel thread(s) can be bring up by the application in
	 * this main kernel thread.
	 */
	hKernelThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		(__KERNEL_THREAD_ROUTINE)AppEntryPoint,
		pParamBlock,
		NULL,
		pAppName);

	if(NULL == hKernelThread)  //Can not create the thread.
	{
		goto __TERMINAL;
	}

	/* 
	 * Give current user input focus to the application,so application 
	 * can capture the user's input,include keyboard,mouse,and other interrupt
	 * driven device(s).
	 */
	DeviceInputManager.SetFocusThread(
		(__COMMON_OBJECT*)&DeviceInputManager,
		(__COMMON_OBJECT*)hKernelThread);
	/* Shell will pause and wait for the application to run over. */
	hKernelThread->WaitForThisObject((__COMMON_OBJECT*)hKernelThread);
	
	/* Destroy the application kernel thread afater run over. */
	KernelThreadManager.DestroyKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		(__COMMON_OBJECT*)hKernelThread);

	/* Obtain back the input focus,to shell again. */
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

/* 
 * Load the application file into memory,check it,and run it if everything
 * is OK.
 * 
 */
BOOL RunDynamicAppModule(LPSTR pAppFilePath,LPVOID pParamBlock)
{
	__APP_ENTRY* pAppEntry = AppEntryArray;	
	HANDLE hFileObj = NULL;
	char* thread_name = NULL;
	int i = 0;
	LPBYTE pAppBuf = NULL;
	BOOL bRunOk = FALSE;

	while(bRunOk == FALSE)
	{
		hFileObj = OpenAppFile(pAppFilePath);
		if(hFileObj == NULL)
		{
			_hx_printf("Can not open the specified module[%s].",pAppFilePath);
			break;
		}

		/* Load the app module according it's format. */
		while(pAppEntry->CheckFormat)
		{
			if(pAppEntry->CheckFormat(hFileObj))
			{
				pAppBuf = pAppEntry->LoadApp(hFileObj);
				break;
			}
			pAppEntry ++;			
		}

		/* Can not load the app module into memory. */
		if(pAppBuf == NULL)
		{
			_hx_printf("Load binary module error.");
			break;
		}
		/* Construct the corresponding kernel thread's name. */
		thread_name = strrchr(pAppFilePath, '\\');
		if (NULL == thread_name)
		{
			_hx_printf("Can not obtain a valid name from[%s].\r\n",
				pAppFilePath);
			break;
		}
		if (strlen(thread_name) < 2) /* Name is too short. */
		{
			_hx_printf("Obtained a too short name[%d].\r\n", thread_name);
			break;
		}
		/* Skip the first slash character. */
		thread_name += 1;
		/* Omit the last surfix(.exe). */
		while (thread_name[i])
		{
			if ('.' == thread_name[i])
			{
				thread_name[i] = 0;
				break;
			}
			i++;
		}
		/* Everything is in place,try to start the module now. */
		if(!StartRunApp((DWORD)pAppBuf,pParamBlock,thread_name))
		{
			_hx_printf("Failed to run binary module.");
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
