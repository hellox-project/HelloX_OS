//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 24 FEB, 2009
//    Module Name               : IOMGR2.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of
//                                I/O Manager.
//                                This is the second part of IOManager's implementation.
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

#include "stdio.h"
#include "kapi.h"

//Only Device Driver Framework is enabled the following code will be included
//in OS kernel.
#ifdef __CFG_SYS_DDF

//
//In front of this file,we implement three call back routines first,these three
//call back routines are called by device driver(s) to report some events to IOManager.
//These three routines are members of DRCB object,i.e,their base addresses are countained 
//in DRCB object.
//The first routine is WaitForCompletion,this routine is called when device driver(s) 
//submit a device operation transaction,such as READ or WRITE,and to wait the operation
//over,in this situation,device driver(s) calls this this routine,put the current kernel
//thread to BLOCKED queue.
//The second routine is OnCompletion,this routine is called when device request operation
//over,to indicate the IOManager this event,and wakeup the kernel thread which is blocked
//in WaitForCompletion routine.
//The third routine is OnCancel,which is called when an IO operation is canceled.
//

//
//The implementation of WaitForCompletion.
//This routine does the following:
// 1. Check the validation of parameter(s);
// 2. Block the current kernel thread.
//
DWORD WaitForCompletion(__COMMON_OBJECT* lpThis)
{
	__DRCB*               lpDrcb           = NULL;
	__EVENT*              lpEvent          = NULL;
	DWORD                 dwWaitResult     = OBJECT_WAIT_TIMEOUT;

	if(NULL == lpThis) //Invalid parameter.
	{
		return 0;
	}
	lpDrcb   = (__DRCB*)lpThis;
	lpEvent  = lpDrcb->lpSynObject;
	
	dwWaitResult = lpEvent->WaitForThisObjectEx((__COMMON_OBJECT*)lpEvent,DRCB_DEFAULT_WAIT_TIME);  //Block the current kernel thread.
	if(OBJECT_WAIT_RESOURCE != dwWaitResult)  //Can not wait operation result before time out or other case.
	{
		lpDrcb->dwStatus = DRCB_STATUS_CANCELED;
	}
	return dwWaitResult;
}

//
//The implementation of OnCompletion.
//This routine does the following:
// 1. Check the parameter's validation;
// 2. Wakeup the kernel thread who waiting for the current device operation.
//
DWORD OnCompletion(__COMMON_OBJECT* lpThis)
{
	__EVENT*              lpEvent          = NULL;

	if(NULL == lpThis)
	{
		return 0;
	}

	lpEvent = ((__DRCB*)lpThis)->lpSynObject;
	lpEvent->SetEvent((__COMMON_OBJECT*)lpEvent);  //Wakeup kernel thread.
	return 1;
}

//
//The implementation of OnCancel.
//This routine does the following:
// 1. 
//
DWORD OnCancel(__COMMON_OBJECT* lpThis)
{
	if(NULL == lpThis)    //Parameter check.
	{
		return 0;
	}
	return 1;
}

//
//The Initialize routine and UnInitialize routine of DRCB.
//
BOOL DrcbInitialize(__COMMON_OBJECT*  lpThis)
{
	__EVENT*          lpSynObject     = NULL;
	__DRCB*           lpDrcb          = NULL;
	DWORD             dwFlags         = 0;

	if(NULL == lpThis)
	{
		return FALSE;
	}

	lpDrcb = (__DRCB*)lpThis;

	lpSynObject = (__EVENT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);
	if(NULL == lpSynObject)    //Failed to create event object.
	{
		return FALSE;
	}

	if(!lpSynObject->Initialize((__COMMON_OBJECT*)lpSynObject)) //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)lpSynObject);
		return FALSE;
	}

	lpDrcb->lpSynObject        = lpSynObject;

	//ENTER_CRITICAL_SECTION();
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpDrcb->lpKernelThread     = KernelThreadManager.lpCurrentKernelThread;
	//LEAVE_CRITICAL_SECTION();
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	lpDrcb->dwStatus           = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwRequestMode      = 0;
	lpDrcb->dwCtrlCommand      = 0;

	lpDrcb->dwOutputLen        = 0;
	lpDrcb->lpOutputBuffer     = NULL;
	lpDrcb->dwInputLen         = 0;
	lpDrcb->lpInputBuffer      = NULL;

	lpDrcb->lpNext             = NULL;
	lpDrcb->lpPrev             = NULL;

	//lpDrcb->WaitForCompletion  = NULL;
	//lpDrcb->OnCompletion       = NULL;
	//lpDrcb->OnCancel           = NULL;
	lpDrcb->WaitForCompletion  = WaitForCompletion;
	lpDrcb->OnCompletion       = OnCompletion;
	lpDrcb->OnCancel           = OnCancel;

	lpDrcb->lpDrcbExtension    = NULL;
	return TRUE;
}

//
//The Uninitialize of DRCB.
//
VOID DrcbUninitialize(__COMMON_OBJECT*  lpThis)
{
	__DRCB*           lpDrcb            = NULL;

	if(NULL == lpThis)
	{
		return;
	}

	lpDrcb = (__DRCB*)lpThis;

	if(lpDrcb->lpSynObject != NULL)
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)(lpDrcb->lpSynObject));
	}
	return;
}

//
//The implementation of driver object's initialize routine.
//
BOOL DrvObjInitialize(__COMMON_OBJECT*  lpThis)
{
	__DRIVER_OBJECT*       lpDrvObj     = NULL;

	if(NULL == lpThis)
	{
		return FALSE;
	}

	lpDrvObj = (__DRIVER_OBJECT*)lpThis;
	lpDrvObj->lpPrev            = NULL;
	lpDrvObj->lpNext            = NULL;
	return TRUE;
}

//
//The implementation of driver object's Uninitialize routine.
//
VOID DrvObjUninitialize(__COMMON_OBJECT* lpThis)
{
	return;
}

//
//The implementation of device object's initialize routine.
//

BOOL DevObjInitialize(__COMMON_OBJECT* lpThis)
{
	__DEVICE_OBJECT*          lpDevObject = NULL;

	if(NULL == lpThis)
	{
		return FALSE;
	}

	lpDevObject = (__DEVICE_OBJECT*)lpThis;
	lpDevObject->lpPrev           = NULL;
	lpDevObject->lpNext           = NULL;

	lpDevObject->DevName[0]       = 0;
	//lpDevObject->dwDevType        = DEVICE_TYPE_NORMAL;
	lpDevObject->lpDriverObject   = NULL;
	lpDevObject->lpDevExtension    = NULL;

	return TRUE;
}

//
//Device object's Uninitialize routine.
//

VOID DevObjUninitialize(__COMMON_OBJECT* lpThis)
{
	return;
}

//A helper macro to check if the specified value is a letter.
#define IS_LETTER(l) (((l) >= 'A') && ((l) <= 'Z'))
//Convert the lowcase character into capital one if it is.
#define TO_CAPITAL(l) \
    (((l) >= 'a') && ((l) <= 'z')) ? ((l) - 'a' + 'A') : (l);

//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
/**
 *
static VOID ToCapital(LPSTR lpszString)
{
	int nIndex = 0;

	if(NULL == lpszString)
	{
		return;
	}
	while(lpszString[nIndex])
	{
		if((lpszString[nIndex] >= 'a') && (lpszString[nIndex] <= 'z'))
		{
			lpszString[nIndex] += 'A' - 'a';
		}
		nIndex ++;
	}
}
 */

//Create a new file in the given file system.
BOOL CreateNewFile(__COMMON_OBJECT* lpThis,
				   LPSTR            lpszFileName)
{
	__DRIVER_OBJECT*              pFsDriver      = NULL;
	__DEVICE_OBJECT*              pFsObject      = NULL;
	__IO_MANAGER*                 pIoManager     = (__IO_MANAGER*)lpThis;
	DWORD                         dwFlags;
	BYTE                          FsIdentifier   = 0;  //File system identifier.
	__DRCB*                       pDrcb          = NULL;
	DWORD                         dwResult       = 0;
	int                           i;

	FsIdentifier   = TO_CAPITAL(lpszFileName[0]);
	//Create DRCB object first.
    pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(FsIdentifier == pIoManager->FsArray[i].FileSystemIdentifier)  //Located the file system.
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject; //Get the file system object.
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	if(NULL == pFsObject)  //Can not locate the desired file system.
	{
		goto __TERMINAL;
	}
	//Check the validity of file system object.
	if(DEVICE_OBJECT_SIGNATURE != pFsObject->dwSignature)
	{
		goto __TERMINAL;
	}
	pFsDriver = pFsObject->lpDriverObject;

	//Initialie the DRCB object,then call DeviceCreate routine of file system object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_CREATE;
	pDrcb->dwInputLen      = StrLen(lpszFileName);
	pDrcb->lpInputBuffer   = (LPVOID)lpszFileName;

	dwResult = pFsDriver->DeviceCreate((__COMMON_OBJECT*)pFsDriver,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if(NULL != pDrcb)  //DRCB object created yet,release it.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwResult ? TRUE : FALSE;
}

//
//The WriteFile's implementation.
//The routine does the following:
// 1. Create a DRCB object,and initialize it;
// 2. Commit the write transaction by calling DeviceWrite routine;
// 3. According to the result,set appropriate return value(s).
//
BOOL _WriteFile(__COMMON_OBJECT*  lpThis,
					  __COMMON_OBJECT*  lpFileObj,
					  DWORD             dwWriteSize,
					  LPVOID            lpBuffer,
					  DWORD*            lpWrittenSize)
{
	BOOL              bResult           = FALSE;
	__DRCB*           lpDrcb            = NULL;
	__DEVICE_OBJECT*  lpDevObject       = NULL;
	__DRIVER_OBJECT*  lpDrvObject       = NULL;
	DWORD             dwWriteBlockSize  = 0;
	DWORD             dwTotalSize       = 0;
	DWORD             dwWrittenSize     = 0;
	DWORD             dwTotalWritten    = 0;

	if((NULL == lpThis) || (NULL == lpFileObj) || (0 == dwWriteSize) ||
	  (NULL == lpBuffer))    //Parameters check.
	{
	  return bResult;
	}

	lpDevObject = (__DEVICE_OBJECT*)lpFileObj;
	lpDrvObject = lpDevObject->lpDriverObject;

	lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == lpDrcb)  //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}
	if(!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb)) //Failed to initialize.
	{
		goto __TERMINAL;
	}

	dwTotalSize           = dwWriteSize;
	lpDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_WRITE;
	dwWriteBlockSize      = lpDevObject->dwMaxWriteSize;

	while(TRUE)
	{
		lpDrcb->dwInputLen = dwTotalSize > dwWriteBlockSize ? dwWriteBlockSize : dwTotalSize;
		lpDrcb->lpInputBuffer  = lpBuffer;
		dwWrittenSize = lpDrvObject->DeviceWrite((__COMMON_OBJECT*)lpDrvObject,
			(__COMMON_OBJECT*)lpDevObject,
			lpDrcb);  //Commit the write transaction.

		if(0 == dwWrittenSize)  //Failed to write.
		{
			goto __TERMINAL;
		}
		dwTotalWritten += dwWrittenSize;
		if(dwTotalSize <= dwWriteBlockSize)    //This indicates the write transaction is
			                                   //over.
		{
			*lpWrittenSize = dwTotalWritten;
			bResult = TRUE;
			goto __TERMINAL;
		}
		dwTotalSize -= dwWriteBlockSize;
		lpBuffer = (LPVOID)((DWORD)lpBuffer + dwWriteBlockSize);  //Adjust the buffer.
	}

__TERMINAL:
	if(lpDrcb != NULL)    //Destroy the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDrcb);
	}
	return bResult;
}

//The implementation of ReadFile routine.
BOOL _ReadFile(__COMMON_OBJECT* lpThis,
					  __COMMON_OBJECT* lpFileObject,
					  DWORD            dwByteSize,
					  LPVOID           lpBuffer,
					  DWORD*           lpReadSize)
{
	BOOL              bResult          = FALSE;
	__DEVICE_OBJECT*  lpFile           = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  lpDriver         = NULL;
	__DRCB*           lpDrcb           = NULL;
	LPVOID            lpTmpBuff        = NULL;
	DWORD             dwToRead         = 0;
	DWORD             dwRead           = 0;
	DWORD             dwTotalRead      = 0;
	DWORD             dwPartRead       = 0;
	BYTE*             pPartBuff        = NULL;

	//Parameters validity checking.
	if((NULL == lpFileObject) || (0 == dwByteSize) || (NULL == lpBuffer))
	{
		goto __TERMINAL;
	}
	//Check if the file object is a valid file object.
	if(DEVICE_OBJECT_SIGNATURE != lpFile->dwSignature)
	{
		goto __TERMINAL;
	}
	//If block size is set to zero,then the target device object can not be accessed
	//directly by using this routine.
	if(DEVICE_BLOCK_SIZE_INVALID == lpFile->dwBlockSize)
	{
		goto __TERMINAL;
	}
	//Create DRCB object.
	lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == lpDrcb)        //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}

	if(!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}
	//Now read data from device by calling the DeviceRead routine.
	lpDriver  = lpFile->lpDriverObject;
	lpTmpBuff = lpBuffer;
	do{
		lpDrcb->dwRequestMode    = DRCB_REQUEST_MODE_READ;
		lpDrcb->dwStatus         = DRCB_STATUS_INITIALIZED;
		//lpDrcb->lpOutputBuffer   = lpTmpBuff;
		if(dwByteSize >= lpFile->dwMaxReadSize)
		{
			dwToRead    = lpFile->dwMaxReadSize;
			lpDrcb->lpOutputBuffer = lpTmpBuff;    //CAUTION: Modified but did not tested yet.
			lpTmpBuff   = (BYTE*)lpTmpBuff + lpFile->dwMaxReadSize;
			dwByteSize  = dwByteSize - dwToRead;
			lpDrcb->dwOutputLen = dwToRead;  //Set the request data size.
		}
		else
		{
			if(0 == dwByteSize)  //Read over.
			{
				break;
			}
			dwToRead   = dwByteSize;
			dwPartRead = dwToRead;    //It indicates the actual read size.
			if(dwToRead % lpFile->dwBlockSize)  //Should round to block size.
			{
				dwToRead += (lpFile->dwBlockSize - (dwToRead % lpFile->dwBlockSize));
			}
			pPartBuff = (BYTE*)KMemAlloc(dwToRead,KMEM_SIZE_TYPE_ANY);  //---- CAUTION!!! ----
			if(NULL == pPartBuff)  //Can not allocate buffer,giveup.
			{
				break;
			}
			//Use pPartBuff to read the remainder data.
			lpDrcb->lpOutputBuffer = pPartBuff;
			lpDrcb->dwOutputLen = dwToRead;  //Set the request data size.
			dwByteSize = 0;         //Read over.
			//dwToRead = dwByteSize;  //Set to initial value so as to jump out the loop.
		}
		//Issue read command to device.
		dwRead = lpDriver->DeviceRead(
			(__COMMON_OBJECT*)lpDriver,
			(__COMMON_OBJECT*)lpFile,
			lpDrcb);
		dwTotalRead += dwRead;
		if(dwRead < dwToRead)  //Only partition of the request has been read,this may caused by the end of file.
		{
			dwPartRead = dwRead; //dwPartRead indicates the actual read size.
			break;
		}
		if(0 == dwByteSize)    //Read over.
		{
			break;
		}
	}while(dwToRead >= lpFile->dwMaxReadSize);
	if(pPartBuff)  //It means partition(relate to dwMaxReadSize) data has been read.
	{
		memcpy(lpTmpBuff,pPartBuff,dwPartRead); //Append the partition data to buffer.
		dwTotalRead -= dwRead;
		dwTotalRead += dwPartRead;
	}
	//Set the total read size if necessary.
	if(NULL != lpReadSize)
	{
		*lpReadSize = dwTotalRead;
	}
	if(DRCB_STATUS_FAIL == lpDrcb->dwStatus)
	{
		bResult = FALSE;
	}
	else
	{
		bResult = TRUE;
	}
__TERMINAL:
	if(lpDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDrcb);
	}
	if(pPartBuff)
	{
		KMemFree(pPartBuff,KMEM_SIZE_TYPE_ANY,0);
	}
	return bResult;
}

//
//The implementation of CloseFile.
//This routine does the following:
// 1. Check the validation of the parameters;
// 2. Check the device type of the target device object;
// 3. If the device to closed is a normal file,then destroy the object;
// 4. If the target deivce is not a normal file,then reduce the
//    reference counter,and return.
//

//Implementation of close file.
VOID _CloseFile(__COMMON_OBJECT* lpThis,
			   __COMMON_OBJECT* lpFileObject)
{
	__DRCB*           pDrcb    = NULL;
	__DEVICE_OBJECT*  pFileObj = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  pFileDrv = NULL;

	if((NULL == lpThis) || (NULL == lpFileObject))  //Invalid parameters.
	{
		return;
	}
	//Validate the file system object.
	if(DEVICE_OBJECT_SIGNATURE != pFileObj->dwSignature)
	{
		return;
	}
	pFileDrv = pFileObj->lpDriverObject;
	//Create DRCB object and issue the close file command.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}
	pDrcb->dwStatus       = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode  = DRCB_REQUEST_MODE_CLOSE;
	pDrcb->dwInputLen     = sizeof(__COMMON_OBJECT*);
	pDrcb->lpInputBuffer  = (LPVOID)pFileObj;
	//Issue the command.
	pFileDrv->DeviceClose((__COMMON_OBJECT*)pFileDrv,
		(__COMMON_OBJECT*)pFileObj,
		pDrcb);

__TERMINAL:
	if(pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return;
}

//Implementation of DeleteFile.
BOOL _DeleteFile(__COMMON_OBJECT* lpThis,
				LPCTSTR lpszFileName)
{
	__DEVICE_OBJECT*         pFsObject     = NULL;
	__DRCB*                  pDrcb         = NULL;
	__IO_MANAGER*            pIoManager    = (__IO_MANAGER*)lpThis;
	BYTE                     FsIdentifier  = 0;
	DWORD                    dwFlags;
	CHAR                     FileName[512];
	DWORD                    dwResult;
	int                      i;

	if((NULL == lpszFileName) || (NULL == pIoManager))
	{
		return FALSE;
	}
	StrCpy((CHAR*)lpszFileName,FileName);
	ToCapital(FileName);  //Convert to capital.
	//Get file system identifier.
	FsIdentifier = FileName[0];
	//Get the file system object.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == pFsObject)  //Can not allocate the specified file system object.
	{
		return FALSE;
	}
	//Validate file system object's validity.
	if(DEVICE_OBJECT_SIGNATURE != pFsObject->dwSignature)
	{
		return FALSE;
	}
	//Create DRCB object and issue Create Directory command.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_FS_DELETEFILE;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(LPCTSTR);
	pDrcb->lpInputBuffer   = (LPVOID)&FileName[0];

	dwResult = pFsObject->lpDriverObject->DeviceCtrl((__COMMON_OBJECT*)pFsObject->lpDriverObject,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if(pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwResult ? TRUE : FALSE;
}

//Implementation of RemoveDirectory.
BOOL _RemoveDirectory(__COMMON_OBJECT* lpThis,
					 LPCTSTR lpszFileName)
{
	__DEVICE_OBJECT*         pFsObject     = NULL;
	__DRCB*                  pDrcb         = NULL;
	__IO_MANAGER*            pIoManager    = (__IO_MANAGER*)lpThis;
	BYTE                     FsIdentifier  = 0;
	DWORD                    dwFlags;
	CHAR                     FileName[512];
	DWORD                    dwResult;
	int                      i;

	if((NULL == lpszFileName) || (NULL == pIoManager))
	{
		return FALSE;
	}
	StrCpy((CHAR*)lpszFileName,FileName);
	ToCapital(FileName);  //Convert to capital.
	//Get file system identifier.
	FsIdentifier = FileName[0];
	//Get the file system driver object.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == pFsObject)
	{
		return FALSE;
	}
	//Validate file system object's validity.
	if(DEVICE_OBJECT_SIGNATURE != pFsObject->dwSignature)
	{
		return FALSE;
	}
	//Create DRCB object and issue Create Directory command.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_FS_REMOVEDIR;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(LPCTSTR);
	pDrcb->lpInputBuffer   = (LPVOID)&FileName[0];

	dwResult = pFsObject->lpDriverObject->DeviceCtrl((__COMMON_OBJECT*)pFsObject->lpDriverObject,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if(pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwResult ? TRUE : FALSE;
}

//Implementation of SetEndOfFile.
BOOL _SetEndOfFile(__COMMON_OBJECT* lpThis,
				   __COMMON_OBJECT* lpFileObject)
{
	__DRCB*                  pDrcb         = NULL;
	__IO_MANAGER*            pIoManager    = (__IO_MANAGER*)lpThis;
	__DEVICE_OBJECT*         pFileDevice   = (__DEVICE_OBJECT*)lpFileObject;
	DWORD                    dwResult      = 0;

	if((NULL == pFileDevice) || (NULL == pIoManager))
	{
		return FALSE;
	}
	//Validate file system object's validity.
	if(DEVICE_OBJECT_SIGNATURE != pFileDevice->dwSignature)
	{
		return FALSE;
	}
	//Create DRCB object and issue Create Directory command.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		goto __TERMINAL;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_FS_SETENDFILE;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(__DEVICE_OBJECT);
	pDrcb->lpInputBuffer   = (LPVOID)pFileDevice;

	dwResult = pFileDevice->lpDriverObject->DeviceCtrl((__COMMON_OBJECT*)pFileDevice->lpDriverObject,
		(__COMMON_OBJECT*)pFileDevice,
		pDrcb);

__TERMINAL:
	if(pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwResult ? TRUE : FALSE;
}

#endif
