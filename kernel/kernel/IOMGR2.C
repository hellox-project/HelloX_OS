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

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * The following code will be included
 * in OS kernel only Device Driver Framework is 
 * enabled in config file.
 */
#ifdef __CFG_SYS_DDF

/*
 * In front of this file,we implement three call back routines first,these three
 * call back routines are called by device driver(s) to report some events to IOManager.
 * These three routines are members of DRCB object,i.e,their base addresses are countained 
 * in DRCB object.
 * The first routine is WaitForCompletion,this routine is called when device driver(s) 
 * submit a device operation transaction,such as READ or WRITE,and to wait the operation
 * over,in this situation,device driver(s) calls this this routine,put the current kernel
 * thread to BLOCKED queue.
 * The second routine is OnCompletion,this routine is called when device request operation
 * over,to indicate the IOManager this event,and wakeup the kernel thread which is blocked
 * in WaitForCompletion routine.
 * The third routine is OnCancel,which is called when an IO operation is canceled.
 */

/*
 * Wait for completion of drcb.
 * This routine does the following:
 *  1. Check the validation of parameter(s);
 *  2. Block the current kernel thread.
 */
unsigned long WaitForCompletion(__COMMON_OBJECT* lpThis)
{
	__DRCB* lpDrcb = NULL;
	__EVENT* lpEvent = NULL;
	unsigned long dwWaitResult = OBJECT_WAIT_TIMEOUT;

	BUG_ON(NULL == lpThis);
	lpDrcb = (__DRCB*)lpThis;
	lpEvent = lpDrcb->lpSynObject;

	/* Just block the current thread on event object. */
	dwWaitResult = lpEvent->WaitForThisObjectEx((__COMMON_OBJECT*)lpEvent, 
		DRCB_DEFAULT_WAIT_TIME);
	if (OBJECT_WAIT_RESOURCE != dwWaitResult)
	{
		/* timeout, or drcb destroyed. */
		lpDrcb->dwStatus = DRCB_STATUS_CANCELED;
	}
	return dwWaitResult;
}

/*
 * OnCompletion routine of drcb.
 * This routine does the following:
 *  1. Check the parameter's validation;
 *  2. Wakeup the kernel thread who waiting for the current device operation.
 */
unsigned long OnCompletion(__COMMON_OBJECT* lpThis)
{
	__EVENT* lpEvent = NULL;

	BUG_ON(NULL == lpThis);
	lpEvent = ((__DRCB*)lpThis)->lpSynObject;
	/* Wake up the pending thread. */
	lpEvent->SetEvent((__COMMON_OBJECT*)lpEvent);
	return 1;
}

/*
 * Default implementaion of OnCancel.
 * Device drivers could specify a new one
 * corresponding the device.
 */
unsigned long OnCancel(__COMMON_OBJECT* lpThis)
{
	BUG_ON(NULL == lpThis);
	return 1;
}

/*
 * Reset one DRCB object to initial state.
 * It must be called when a drcb is resued
 * again.
 */
static void DrcbReset(__COMMON_OBJECT* pThis)
{
	__DRCB* pDrcb = (__DRCB*)pThis;

	BUG_ON(NULL == pDrcb);
	/* Reset the event object. */
	pDrcb->lpSynObject->ResetEvent((__COMMON_OBJECT*)pDrcb->lpSynObject);
}

/* Initializer of drcb object. */
BOOL DrcbInitialize(__COMMON_OBJECT* lpThis)
{
	__EVENT* lpSynObject = NULL;
	__DRCB* lpDrcb = NULL;

	BUG_ON(NULL == lpThis);
	lpDrcb = (__DRCB*)lpThis;

	/* Create and init the binding event object. */
	lpSynObject = (__EVENT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_EVENT);
	if (NULL == lpSynObject)
	{
		return FALSE;
	}
	if (!lpSynObject->Initialize((__COMMON_OBJECT*)lpSynObject))
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)lpSynObject);
		return FALSE;
	}

	lpDrcb->lpSynObject = lpSynObject;
	lpDrcb->lpKernelThread = __CURRENT_KERNEL_THREAD;
	lpDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwRequestMode = 0;
	lpDrcb->dwCtrlCommand = 0;

	lpDrcb->dwOutputLen = 0;
	lpDrcb->lpOutputBuffer = NULL;
	lpDrcb->dwInputLen = 0;
	lpDrcb->lpInputBuffer = NULL;

	lpDrcb->lpNext = NULL;
	lpDrcb->lpPrev = NULL;

	/* Use default routines to init drcb. */
	lpDrcb->WaitForCompletion = WaitForCompletion;
	lpDrcb->OnCompletion = OnCompletion;
	lpDrcb->OnCancel = OnCancel;
	lpDrcb->Reset = DrcbReset;

	lpDrcb->lpDrcbExtension = NULL;
	return TRUE;
}

/* Uninitializer of drcb. */
unsigned long DrcbUninitialize(__COMMON_OBJECT*  lpThis)
{
	__DRCB*           lpDrcb = NULL;

	BUG_ON(NULL == lpThis);

	lpDrcb = (__DRCB*)lpThis;
	/* Destroy the binding event object. */
	if (lpDrcb->lpSynObject != NULL)
	{
		ObjectManager.DestroyObject(&ObjectManager, 
			(__COMMON_OBJECT*)(lpDrcb->lpSynObject));
	}
	return TRUE;
}

/* Initializer of driver object. */
BOOL DrvObjInitialize(__COMMON_OBJECT* lpThis)
{
	__DRIVER_OBJECT* lpDrvObj     = NULL;

	BUG_ON(NULL == lpThis);

	lpDrvObj = (__DRIVER_OBJECT*)lpThis;
	lpDrvObj->lpPrev = NULL;
	lpDrvObj->lpNext = NULL;
	return TRUE;
}

/* deinit of driver object. */
BOOL DrvObjUninitialize(__COMMON_OBJECT* lpThis)
{
	return TRUE;
}

/* Initialize a device object. */
BOOL DevObjInitialize(__COMMON_OBJECT* lpThis)
{
	__DEVICE_OBJECT* lpDevObject = NULL;

	BUG_ON(NULL == lpThis);

	lpDevObject = (__DEVICE_OBJECT*)lpThis;
	lpDevObject->lpPrev           = NULL;
	lpDevObject->lpNext           = NULL;

	lpDevObject->DevName[0]       = 0;
	//lpDevObject->dwDevType        = DEVICE_TYPE_NORMAL;
	lpDevObject->lpDriverObject   = NULL;
	lpDevObject->lpDevExtension    = NULL;

	/* Set current position pointer as 0. */
	lpDevObject->dwCurrentPos = 0;

	return TRUE;
}

/* 
 * Uninit routine of driver object. 
 * It removes the device object from IOManager's
 * device list.
 */
BOOL DevObjUninitialize(__COMMON_OBJECT* lpThis)
{
	unsigned long ulFlags;
	__DEVICE_OBJECT* pDeviceObject = (__DEVICE_OBJECT*)lpThis;
	__IO_MANAGER* lpIoManager = &IOManager;

	BUG_ON(NULL == pDeviceObject);

	/* Remove the device from list. */
	__ENTER_CRITICAL_SECTION_SMP(lpIoManager->spin_lock, ulFlags);
	if (NULL == pDeviceObject->lpPrev)
	{
		/* First device object. */
		if (NULL == pDeviceObject->lpNext)
		{
			/* Last device object. */
			lpIoManager->lpDeviceRoot = NULL;
		}
		else
		{
			pDeviceObject->lpNext->lpPrev = NULL;
			lpIoManager->lpDeviceRoot = pDeviceObject->lpNext;
		}
	}
	else {
		if (NULL == pDeviceObject->lpNext)
		{
			/* Last device object. */
			pDeviceObject->lpPrev->lpNext = NULL;
		}
		else
		{
			pDeviceObject->lpPrev->lpNext = pDeviceObject->lpNext;
			pDeviceObject->lpNext->lpPrev = pDeviceObject->lpPrev;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpIoManager->spin_lock, ulFlags);

	return TRUE;
}

//A helper macro to check if the specified value is a letter.
#define IS_LETTER(l) (((l) >= 'A') && ((l) <= 'Z'))
//Convert the lowcase character into capital one if it is.
#define TO_CAPITAL(l) \
    (((l) >= 'a') && ((l) <= 'z')) ? ((l) - 'a' + 'A') : (l);

#if 0
//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
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
#endif

/*
 * Create a new file in a given partition,the identifier
 * of the partition can be obtained from the first letter of
 * the lpszFileName parameter.
 */
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
	
	/* Create a DRCB object to track the whole creating process. */
    pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		goto __TERMINAL;
	}

	/* Locate the file system object. */
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(FsIdentifier == pIoManager->FsArray[i].FileSystemIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);

	/* Can not locate the desired file system. */
	if(NULL == pFsObject)
	{
		goto __TERMINAL;
	}
	/* Validate the file system object. */
	if(DEVICE_OBJECT_SIGNATURE != pFsObject->dwSignature)
	{
		goto __TERMINAL;
	}
	pFsDriver = pFsObject->lpDriverObject;

	/* 
	 * Initialie the DRCB object,then call DeviceCreate routine
	 * of file system object,it's this routine that created file
	 * on partition.
	 */
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_CREATE;
	pDrcb->dwInputLen      = StrLen(lpszFileName);
	pDrcb->lpInputBuffer   = (LPVOID)lpszFileName;

	dwResult = pFsDriver->DeviceCreate((__COMMON_OBJECT*)pFsDriver,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if(NULL != pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwResult ? TRUE : FALSE;
}

/*
* Local helper of SetFilePointer routine,it invokes the device
* level of this routine(DeviceSeek).
* Just put in front of this file since it is refered by several
* routine,such as SetFilePointer,ReadFile,WriteFile.
*/
static DWORD __SetFilePointer_Local(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFile,
	DWORD* pdwDistLow,
	DWORD* pdwDistHigh,
	DWORD  dwWhereBegin)
{
	__DRIVER_OBJECT*        pDrvObject = NULL;
	__DEVICE_OBJECT*        pFileObject = (__DEVICE_OBJECT*)lpFile;
	__DRCB*                 pDrcb = NULL;
	DWORD                   dwResult = 0;

	/* Validate parameters,low part of offset must be specified. */
	if ((NULL == pFileObject) || (NULL == pdwDistLow))
	{
		return -1;
	}
	/* Check the position flags where to begin. */
	if ((FILE_FROM_BEGIN != dwWhereBegin) && (FILE_FROM_CURRENT != dwWhereBegin) && (FILE_FROM_END != dwWhereBegin))
	{
		return -1;
	}
	/* Validate the file object. */
	if (DEVICE_OBJECT_SIGNATURE != pFileObject->dwSignature)
	{
		return -1;
	}
	pDrvObject = pFileObject->lpDriverObject;

	/*
	* Create a DRCB object and initialize it.
	* All device seek command related parameters,are contained in
	* this object and transfered to device driver.
	*/
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)        //Failed to create DRCB object.
	{
		return -1;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
		return -1;
	}

	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_SEEK;
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen = sizeof(DWORD);
	/* Use input buffer to contain move scheme,from begin or current. */
	pDrcb->lpInputBuffer = (LPVOID)dwWhereBegin;
	pDrcb->dwExtraParam1 = (DWORD)pdwDistLow;
	/* Not supported of high part of distance now. */
	pDrcb->dwExtraParam2 = (DWORD)pdwDistHigh;

	/* Issue device seek command to the driver object. */
	dwResult = pDrvObject->DeviceSeek((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pFileObject,
		pDrcb);
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)pDrcb);

	return dwResult;
}

/*
 * Write into file or device, the unified routine of
 * device or file's writting access.
 * If the caller specified write size is not align
 * with the block size of device, this routine will
 * pad zeros at the end of user specified data buffer,
 * and make sure the writting request is align with
 * device's block size.
 */
BOOL _WriteFile(__COMMON_OBJECT*  lpThis, __COMMON_OBJECT* lpFileObj,
	DWORD dwWriteSize, LPVOID input_buffer, DWORD* lpWrittenSize)
{
	BOOL bResult = FALSE;
	char* lpBuffer = (char*)input_buffer;
	__DRCB* lpDrcb = NULL;
	__DEVICE_OBJECT* lpDevObject = NULL;
	__DRIVER_OBJECT* lpDrvObject = NULL;
	unsigned long max_write_size = 0, total_size = 0, write_size = 0;
	unsigned long pad_size = 0, request_ret = 0;
	char* pad_buffer = NULL;

	if ((NULL == lpThis) || (NULL == lpFileObj) ||
		(0 == dwWriteSize) || (NULL == lpBuffer))
	{
		/* Loose verification. */
		goto __TERMINAL;
	}

	lpDevObject = (__DEVICE_OBJECT*)lpFileObj;
	lpDrvObject = lpDevObject->lpDriverObject;
	BUG_ON(NULL == lpDrvObject);

	lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == lpDrcb)
	{
		goto __TERMINAL;
	}
	if (!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb))
	{
		goto __TERMINAL;
	}

	total_size = dwWriteSize;
	lpDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_WRITE;
	max_write_size = lpDevObject->dwMaxWriteSize;

	while (TRUE)
	{
		/* Reset drcb since it maybe reused. */
		lpDrcb->Reset((__COMMON_OBJECT*)lpDrcb);

		/* Set request buffer and length accordingly. */
		if (total_size > max_write_size)
		{
			write_size = max_write_size;
			lpDrcb->lpInputBuffer = lpBuffer;
		}
		else
		{
			/* Could be written for one time. */
			if (total_size % lpDevObject->dwBlockSize)
			{
				/* Not block size aligned. */
				write_size = total_size + lpDevObject->dwBlockSize;
				write_size -= (total_size % lpDevObject->dwBlockSize);
				if (write_size > max_write_size)
				{
					/* Invalid case. */
					goto __TERMINAL;
				}
				pad_size = write_size - total_size;
				/* 
				 * Use a new buffer to hold the data, since it's  
				 * size may exceed the original buffer after padding.
				 */
				pad_buffer = (char*)_hx_malloc(write_size);
				if (NULL == pad_buffer)
				{
					goto __TERMINAL;
				}
				memcpy(pad_buffer, lpBuffer, total_size);
				memset(pad_buffer + total_size, 0, pad_size);
				lpDrcb->lpInputBuffer = pad_buffer;
			}
			else {
				/* block size aligned. */
				write_size = total_size;
				lpDrcb->lpInputBuffer = lpBuffer;
			}
		}
		lpDrcb->dwInputLen = write_size;

		/* Issue write command. */
		request_ret = lpDrvObject->DeviceWrite((__COMMON_OBJECT*)lpDrvObject,
			(__COMMON_OBJECT*)lpDevObject,
			lpDrcb);
		if ((DRCB_STATUS_FAIL == lpDrcb->dwStatus) ||
			(write_size != request_ret))
		{
			/* request fail. */
			goto __TERMINAL;
		}
		
		total_size -= (write_size - pad_size);
		/* Adjust input buffer. */
		lpBuffer = (LPVOID)((char*)lpBuffer + (write_size - pad_size));

		if (0 == total_size)
		{
			/* Write over. */
			*lpWrittenSize = dwWriteSize;
			bResult = TRUE;
			goto __TERMINAL;
		}
	}

__TERMINAL:
	if (lpDrcb != NULL)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDrcb);
	}
	if (pad_buffer)
	{
		_hx_free(pad_buffer);
	}
	return bResult;
}

/* 
 * Read data from device object. 
 * All device objects are treated as files,so this routine's name
 * is read FILE.
 */
BOOL _ReadFile(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFileObject,
	DWORD            dwByteSize,
	LPVOID           lpBuffer,
	DWORD*           lpReadSize)
{
	BOOL              bResult = FALSE;
	__DEVICE_OBJECT*  lpFile = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  lpDriver = NULL;
	__DRCB*           lpDrcb = NULL;
	LPVOID            lpTmpBuff = NULL;
	DWORD             dwToRead = 0;
	DWORD             dwRead = 0;
	DWORD             dwTotalRead = 0;
	DWORD             dwPartRead = 0;
	BYTE*             pPartBuff = NULL;

	/* Parameters validity checking. */
	if ((NULL == lpFileObject) || (0 == dwByteSize) || (NULL == lpBuffer))
	{
		goto __TERMINAL;
	}
	/* Validate object. */
	if (DEVICE_OBJECT_SIGNATURE != lpFile->dwSignature)
	{
		goto __TERMINAL;
	}
	if (DEVICE_BLOCK_SIZE_INVALID == lpFile->dwBlockSize)
	{
		goto __TERMINAL;
	}

	/* Move the file's current pointer to current value. */
	__SetFilePointer_Local(lpThis, lpFileObject, &lpFile->dwCurrentPos, NULL, FILE_FROM_BEGIN);

	/* Create DRCB object and initialize it. */
	lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == lpDrcb)
	{
		goto __TERMINAL;
	}
	if (!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb))
	{
		goto __TERMINAL;
	}

	/* Start xfer operation. */
	lpDriver = lpFile->lpDriverObject;
	BUG_ON(NULL == lpDriver);
	lpTmpBuff = lpBuffer;
	do {
		/* Reset the drcb object. */
		lpDrcb->Reset((__COMMON_OBJECT*)lpDrcb);
		lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_READ;
		lpDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
		if (dwByteSize > lpFile->dwMaxReadSize)
		{
			dwToRead = lpFile->dwMaxReadSize;
			/* Request size and buffer. */
			lpDrcb->dwOutputLen = dwToRead;
			lpDrcb->lpOutputBuffer = lpTmpBuff;
			lpTmpBuff = (BYTE*)lpTmpBuff + dwToRead;
			dwByteSize = dwByteSize - dwToRead;
		}
		else
		{
			if (0 == dwByteSize)
			{
				/* read over. */
				break;
			}
			dwToRead = dwByteSize;
			dwPartRead = dwToRead;
			/*
			 * Round to read size to the device's
			 * block size, if they are different.
			 * Only device block size or multiple times
			 * of block size is valid.
			 */
			if (dwToRead % lpFile->dwBlockSize)
			{
				dwToRead += (lpFile->dwBlockSize - (dwToRead % lpFile->dwBlockSize));
			}

			/*
			 * Allocation partition reading buffer.
			 * Use partition reading buffer to hold the data,to avoid
			 * overflow of original buffer(lpBuffer).
			 * At least one block of data is obtained in this level.
			 */
			pPartBuff = (BYTE*)KMemAlloc(dwToRead, KMEM_SIZE_TYPE_ANY);
			if (NULL == pPartBuff)
			{
				break;
			}
			lpDrcb->lpOutputBuffer = pPartBuff;
			lpDrcb->dwOutputLen = dwToRead;
			/* Mark as read over. */
			dwByteSize = 0;
		}

		/* Issue read command to device. */
		dwRead = lpDriver->DeviceRead(
			(__COMMON_OBJECT*)lpDriver,
			(__COMMON_OBJECT*)lpFile,
			lpDrcb);
		dwTotalRead += dwRead;
		if (dwRead < dwToRead)
		{
			/*
			 * Only partition of the request has been read,this may caused by
			 * the end of file.
			 * dwPartRead indicates the actual read size.
			 */
			dwPartRead = dwRead;
			break;
		}
		if (0 == dwByteSize)
		{
			/* Read over. */
			break;
		}
	} while (dwToRead >= lpFile->dwMaxReadSize);

	/*
	 * Copy the partition reading data into original buffer,and
	 * updates the total read size counter.
	 */
	if (pPartBuff)
	{
		memcpy(lpTmpBuff, pPartBuff, dwPartRead);
		dwTotalRead -= dwRead;
		dwTotalRead += dwPartRead;
	}

	if (DRCB_STATUS_FAIL == lpDrcb->dwStatus)
	{
		bResult = FALSE;
	}
	else
	{
		/*
		 * Return back the total read size.
		 */
		if (NULL != lpReadSize)
		{
			*lpReadSize = dwTotalRead;
		}
		/* Update file's current position pointer. */
		lpFile->dwCurrentPos += dwTotalRead;

		bResult = TRUE;
	}

__TERMINAL:
	if (lpDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDrcb);
	}
	if (pPartBuff)
	{
		KMemFree(pPartBuff, KMEM_SIZE_TYPE_ANY, 0);
	}
	return bResult;
}

//
//Close a file or device object, opened by CreateFile.
//This routine does the following:
// 1. Check the validation of the parameters;
// 2. Check the device type of the target device object;
// 3. If the device to closed is a normal file,then destroy the object;
// 4. If the target deivce is not a normal file,then reduce the
//    reference counter,and return.
//
VOID _CloseFile(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFileObject)
{
	__DRCB*           pDrcb = NULL;
	__DEVICE_OBJECT*  pFileObj = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  pFileDrv = NULL;

	BUG_ON((NULL == lpThis) || (NULL == lpFileObject));

	/* Validates object signature. */
	if (DEVICE_OBJECT_SIGNATURE != pFileObj->dwSignature)
	{
		return;
	}
	pFileDrv = pFileObj->lpDriverObject;

	/* Invoke close command of the device. */
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}

	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		goto __TERMINAL;
	}
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_CLOSE;
	pDrcb->dwInputLen = sizeof(__COMMON_OBJECT*);
	pDrcb->lpInputBuffer = (LPVOID)pFileObj;
	//Issue close command.
	pFileDrv->DeviceClose((__COMMON_OBJECT*)pFileDrv,
		(__COMMON_OBJECT*)pFileObj,
		pDrcb);

__TERMINAL:
	if (pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return;
}

/* Delete a file from system. */
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
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock,dwFlags);
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
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
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

/*
* Change file object's current position pointer.
* Any read or write operations on a file,are begin from
* the current pointer's position.
* It first calls the device level's implementation(DeviceSeek),
* and update the device object's current position pointer
* value if DeviceSeek success.
*/
DWORD _SetFilePointer(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFile,
	DWORD* pdwDistLow,
	DWORD* pdwDistHigh,
	DWORD  dwWhereBegin)
{
	__DEVICE_OBJECT*        pFileObject = (__DEVICE_OBJECT*)lpFile;
	DWORD                   dwResult = 0;

	/* Validate parameters,low part of offset must be specified. */
	if ((NULL == pFileObject) || (NULL == pdwDistLow))
	{
		return -1;
	}
	/* Check the position flags where to begin. */
	if ((FILE_FROM_BEGIN != dwWhereBegin) && (FILE_FROM_CURRENT != dwWhereBegin) && (FILE_FROM_END != dwWhereBegin))
	{
		return -1;
	}
	/* Validate the file object. */
	if (DEVICE_OBJECT_SIGNATURE != pFileObject->dwSignature)
	{
		return -1;
	}

	/* Call device level's corresponding routine(DeviceSeek) first. */
	dwResult = __SetFilePointer_Local(lpThis, lpFile, pdwDistLow, pdwDistHigh,
		dwWhereBegin);

	/* Update the device object's current pointer correspondingly. */
	switch (dwWhereBegin)
	{
	case FILE_FROM_BEGIN:
		pFileObject->dwCurrentPos = *pdwDistLow;
		break;
	case FILE_FROM_CURRENT:
		pFileObject->dwCurrentPos += *pdwDistLow;
		break;
	default:
		dwResult = 0;
		break;
	}

	return dwResult;
}

#endif
