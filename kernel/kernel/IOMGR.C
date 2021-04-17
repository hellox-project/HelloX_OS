//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,02 2005
//    Module Name               : iomgr.cpp
//    Module Funciton           : 
//                                This module countains the implementation code of
//                                I/O Manager.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <string.h>

#include "iomgr.h"
#include "commobj.h"

//Only Device Driver Framework is enabled the following code is included in the
//OS kernel.
#ifdef __CFG_SYS_DDF

/* 
 * External routines,mainly implemented in iomgr2.c file,or
 * other extersion source file of IOManager object.
 * Just for reducing the code line number of one src file.
 */
extern BOOL _SetEndOfFile(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpFileObject);
extern BOOL CreateNewFile(__COMMON_OBJECT* lpIOManager, LPSTR lpszFileName);
extern DWORD WaitForCompletion(__COMMON_OBJECT* lpThis);
extern DWORD OnCompletion(__COMMON_OBJECT* lpThis);
extern DWORD OnCancel(__COMMON_OBJECT* lpThis);
extern BOOL _WriteFile(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpFileObj,
	DWORD dwWriteSize,
	LPVOID lpBuffer,
	DWORD* lpdwWrittenSize);
extern BOOL _ReadFile(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpFileObject,
	DWORD dwByteSize,
	LPVOID lpBuffer,
	DWORD* lpReadSize);
extern VOID _CloseFile(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpFileObj);
extern BOOL _DeleteFile(__COMMON_OBJECT* lpThis,LPCTSTR lpszFileName);
extern BOOL _RemoveDirectory(__COMMON_OBJECT* lpThis,LPCTSTR lpszFileName);
extern DWORD _SetFilePointer(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpFile,
	DWORD* pdwDistLow, 
	DWORD* pdwDistHigh, 
	DWORD  dwWhereBegin);

//A helper macro to check if the specified value is a letter.
#define IS_LETTER(l) (((l) >= 'A') && ((l) <= 'Z'))
//Convert the lowcase character into capital one if it is.
#define TO_CAPITAL(l) \
    (((l) >= 'a') && ((l) <= 'z')) ? ((l) - 'a' + 'A') : (l);

//A helper routine used to convert a string from lowercase to capital.
//The string should be terminated by a zero,i.e,a C string.
/**
VOID ToCapital(LPSTR lpszString)
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
//RegisterFileSystem,this routine add one file system controller into system.
static BOOL RegisterFileSystem(__COMMON_OBJECT* lpThis,
							   __COMMON_OBJECT* pFileSystem)
{
	__IO_MANAGER*       pManager = (__IO_MANAGER*)lpThis;
	DWORD               dwFlags;
	int                 i = 0;

	if((NULL == pFileSystem) || (NULL == lpThis))
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION_SMP(pManager->spin_lock,dwFlags);
	for(i = 0;i < FS_CTRL_NUM;i ++)
	{
		if(NULL == pManager->FsCtrlArray[i]) //Find a empty slot.
		{
			break;
		}
	}
	if(FS_CTRL_NUM == i)  //Can not find a empty slot.
	{
		__LEAVE_CRITICAL_SECTION_SMP(pManager->spin_lock, dwFlags);
		return FALSE;
	}
	//Insert the file system object into this slot.
	pManager->FsCtrlArray[i] = pFileSystem;
	__LEAVE_CRITICAL_SECTION_SMP(pManager->spin_lock, dwFlags);
	return TRUE;
}

/* The initialize routine of IOManager. */
static BOOL IOManagerInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL  bResult = FALSE;

	if(NULL == lpThis)
	{
		return bResult;
	}
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(IOManager.spin_lock, "iomgr");
#endif
	bResult = TRUE;

	return bResult;
}

/* Open a regular file object. */
static __COMMON_OBJECT* __OpenFile(__COMMON_OBJECT* lpThis,
	LPSTR            lpszFileName,
	DWORD            dwAccessMode,
	DWORD            dwShareMode)
{
	__COMMON_OBJECT*              pFileObject = NULL;
	__DRIVER_OBJECT*              pFsDriver = NULL;
	__DEVICE_OBJECT*              pFsObject = NULL;
	__IO_MANAGER*                 pIoManager = (__IO_MANAGER*)lpThis;
	DWORD                         dwFlags;
	BYTE                          FsIdentifier = 0;  //File system identifier.
	__DRCB*                       pDrcb = NULL;
	int                           i;

	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	//Create DRCB object first.
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

	/* 
	 * Locate the file system manager object by the fs
	 * identifier in file's name.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for (i = 0; i < FILE_SYSTEM_NUM; i++)
	{
		if (FsIdentifier == pIoManager->FsArray[i].FileSystemIdentifier)
		{
			pFsObject = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);

	if (NULL == pFsObject)
	{
		goto __TERMINAL;
	}

	//Check the validity of file system object.
	if (DEVICE_OBJECT_SIGNATURE != pFsObject->dwSignature)
	{
		goto __TERMINAL;
	}
	pFsDriver = pFsObject->lpDriverObject;

	/*
	 * Initialie the DRCB object,then invoke
	 * DeviceOpen routine of file system object.
	 */
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_OPEN;
	pDrcb->dwInputLen = dwAccessMode;
	pDrcb->dwOutputLen = dwShareMode;
	pDrcb->lpInputBuffer = (LPVOID)lpszFileName;

	pFileObject = pFsDriver->DeviceOpen((__COMMON_OBJECT*)pFsDriver,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if (NULL != pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return pFileObject;
}

/* 
 * Open a device object. 
 * This routine will be invoked by CreateFile when
 * the file name is a device name(start with \\.\...).
 */
static __COMMON_OBJECT* __OpenDevice(__COMMON_OBJECT* lpThis,
	LPSTR lpszFileName,
	DWORD dwAccessMode,
	DWORD dwShareMode)
{
	__DEVICE_OBJECT* pDevice = NULL;
	__DRIVER_OBJECT* pDriver = NULL;
	unsigned long dwFlags;
	__DRCB* drcb_open = NULL;
	BOOL open_result = FALSE;

	BUG_ON((NULL == lpThis) || (NULL == lpszFileName));

	/* Travel the whole device list to locate the specified one. */
	__ENTER_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);
	pDevice = ((__IO_MANAGER*)lpThis)->lpDeviceRoot;
	while (pDevice)
	{
		if (0 == strcmp(lpszFileName, &pDevice->DevName[0]))
		{
			/* found. */
			break;
		}
		pDevice = pDevice->lpNext;
	}
	/* Increase the reference counter. */
	if (pDevice)
	{
		BUG_ON(NULL == pDevice->lpDriverObject);
		if (!ObjectManager.AddRefCount(&ObjectManager,
			(__COMMON_OBJECT*)pDevice))
		{
			/* Failed to lock the device. */
			pDevice = NULL;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(IOManager.spin_lock, dwFlags);

	/* Invoke the open routine of device. */
	if (pDevice)
	{
		drcb_open = (__DRCB*)ObjectManager.CreateObject(
			&ObjectManager,
			NULL,
			OBJECT_TYPE_DRCB);
		if (NULL == drcb_open)
		{
			/* Decrease the refcount. */
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDevice);
			pDevice = NULL;
			goto __TERMINAL;
		}
		if (!drcb_open->Initialize((__COMMON_OBJECT*)drcb_open))
		{
			ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDevice);
			pDevice = NULL;
			goto __TERMINAL;
		}
		/* Set appropriate drcb members. */
		drcb_open->dwStatus = DRCB_STATUS_INITIALIZED;
		drcb_open->dwRequestMode = DRCB_REQUEST_MODE_OPEN;
		pDriver = pDevice->lpDriverObject;

		/* Invoke open routine. */
		if (pDriver->DeviceOpen)
		{
			if (pDriver->DeviceOpen((__COMMON_OBJECT*)pDriver,
				(__COMMON_OBJECT*)pDevice, drcb_open))
			{
				open_result = TRUE;
			}
		}
	}

__TERMINAL:
	if (open_result)
	{
		return (__COMMON_OBJECT*)pDevice;
	}
	else {
		return NULL;
	}
}

/*
 * Open a device or file given it's name,create a new one if
 * the file is not existed and dwAccessMode specifies create a new
 * one or open always.
 * 
 * Several tips about this routine:
 *  1. In current version implementation of Hello China,all devices are treated as files,
 *     so,if users want to access device,he or she can open the target device by calling
 *     this routine;
 *  2. One file or device can be opend as READ ONLY,WRITE ONLY,or READ WRITE,the dwAccessMode
 *     parameter of this routine indicates the opening mode;
 *  3. In current version,one file or device can be opend more than one time,so,the
 *     dwShareMode indicates the re-open mode of the currently opend file,for example,if one
 *     kernel thread opens a file as READ WRITE mode,and also indicates the OS that this
 *     file can only be re-opend as READ mode(by seting the appropriate value of the
 *     dwShareMode parameter),so,if there is another kernel thread want to open the file
 *     as READ WRITE mode or WRITE mode,it will fail,by contraries,if the second kernel
 *     thread want to open the file only in READ ONLY mode,it will success.
 *  4. The last parameter,lpReserved,is a reserved parameter that may be used in the future.
 * Once success,this routine returns the base address of the device object that opend,
 * otherwise,it will return a NULL value to indicate the failure,user can determine the
 * failing reason by calling GetLastError routine.
 */
static __COMMON_OBJECT* _CreateFile(__COMMON_OBJECT* lpThis,
	LPSTR            lpszFileName,
	DWORD            dwAccessMode,
	DWORD            dwShareMode,
	LPVOID           lpReserved)
{
	CHAR FileName[512];
	__COMMON_OBJECT*    pFileHandle = NULL;

	if(NULL == lpszFileName)
	{
		return NULL;
	}
	/* Path and name too long. */
	if(StrLen(lpszFileName) > MAX_FILE_NAME_LEN)
	{
		return NULL;
	}

	/* Target file's name must have at least 3 characters. */
	if((lpszFileName[0] == 0) || (lpszFileName[1] == 0) || (lpszFileName[2] == 0)) 
	{
		return NULL;
	}

	StrCpy(lpszFileName,FileName);
	ToCapital(FileName);
	/*
	 * The first letter must be characters if the target is a file.
	 * The valid format of a file name as X:\xxxx, where 'X' is the
	 * identifier of file system(file partition),and 'xxxx' is the
	 * target file's path and name,include file name's extension.
	 */
	if(IS_LETTER(FileName[0]))
	{
		if(FileName[1] != ':')  //Invalid file system name.
		{
			return NULL;
		}
		if(FileName[2] != '\\') //Third character should specify root directory.
		{
			return NULL;
		}
		/* A valid file name,try to open it. */
		pFileHandle = __OpenFile(lpThis,FileName,dwAccessMode,dwShareMode);
		/* 
		 * Create a new one if not exist and user requested by specifying
		 * FILE_OPEN_ALWAYS flag.
		 */
		if(NULL == pFileHandle)
		{
			if(FILE_OPEN_ALWAYS & dwAccessMode)
			{
				if(CreateNewFile(lpThis,FileName))
				{
					pFileHandle = __OpenFile(lpThis,FileName,dwAccessMode,dwShareMode); //Try to open again.
				}
				else
				{
					_hx_printf("%s: failed to create new file[%s].\r\n", __func__,
						FileName);
				}
			}
		}
		return pFileHandle;
	}

	/* 
	 * The target name is not a file name,check if it's a device name.
	 * The device name's format must be as \\.\xxxx, where 'xxxx' is 
	 * a meaningful(or no meaningful,no matter) string indicates the function
	 * or attributes of the device.
	 * Just very the name before open it.
	 * Please be noted that the device can not be newly created,just return
	 * NULL if it isn't exist.
	 */
	if((FileName[0] == '\\') && (FileName[1] == '\\') && (FileName[2] == '.' ))
	{
		if(FileName[3] != '\\')
		{
			return NULL;
		}
		/* Validate device name,try to open it. */
		return __OpenDevice(lpThis,FileName,dwAccessMode,dwShareMode);
	}
	return NULL;
}

/*
 * IOControl routine, device specific command or operations 
 * could be carry out through this routine.
 */
static BOOL _IOControl(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFileObject,
	DWORD            dwCommand,
	DWORD            dwInputLen,
	LPVOID           lpInputBuffer,
	DWORD            dwOutputLen,
	LPVOID           lpOutputBuffer,
	DWORD*           lpOutputFilled)
{
	BOOL              bResult = FALSE;
	__DEVICE_OBJECT*  pDevice = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  pDriver = NULL;
	__DRCB*           pDrcb = NULL;
	DWORD             dwRetValue = 0;

	if ((NULL == lpThis) || (NULL == lpFileObject))
	{
		goto __TERMINAL;
	}
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

	//Initialize DRCB object.
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = dwCommand;
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen = dwInputLen;
	pDrcb->lpInputBuffer = lpInputBuffer;
	pDrcb->dwOutputLen = dwOutputLen;
	pDrcb->lpOutputBuffer = lpOutputBuffer;
	//Now issue the IOControl command to device.
	if (pDevice->dwSignature != DEVICE_OBJECT_SIGNATURE)
	{
		goto __TERMINAL;
	}
	pDriver = pDevice->lpDriverObject;

	dwRetValue = pDriver->DeviceCtrl((__COMMON_OBJECT*)pDriver,
		(__COMMON_OBJECT*)pDevice,
		pDrcb);
	if (lpOutputFilled)
	{
		*lpOutputFilled = dwRetValue;
	}
	if (DRCB_STATUS_FAIL == pDrcb->dwStatus)
	{
		bResult = FALSE;
	}
	else
	{
		bResult = TRUE;
	}

__TERMINAL:
	if (pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* 
 * CreateDirectory, create a new directory
 * on the specified location of a file system.
 */
static BOOL _CreateDirectory(__COMMON_OBJECT* lpThis,
							 LPCTSTR lpszFileName,
							 LPVOID  lpReserved)
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
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_FS_CREATEDIR;
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

/* FindFirstFile,it's the begin of finding file's iteration. */
static __COMMON_OBJECT* _FindFirstFile(__COMMON_OBJECT* lpThis,
	LPCTSTR lpszFileName,
	FS_FIND_DATA* pFindData)
{
	__DEVICE_OBJECT*         pFileDriver  = NULL;
	BYTE                     FsIdentifier = 0;
	__DRCB*                  pDrcb        = NULL;
	DWORD                    dwFlags;
	__COMMON_OBJECT*         pResult      = NULL;
	__IO_MANAGER*            pIoManager   = (__IO_MANAGER*)lpThis;
	int                      i;

	if((NULL == lpThis) || (NULL == lpszFileName) || (NULL == pFindData))
	{
		goto __TERMINAL;
	}
	//Try to create DRCB object and initialize it.
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

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDFIRSTFILE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindData;       //Use extra parameter to pass data.

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	if(NULL == pFileDriver)
	{
		/* Can not find the corresponding file system. */
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pFileDriver->dwSignature)
	{
		goto __TERMINAL;
	}
	//Issue the IO control command to file system driver object.
	if(0 == pFileDriver->lpDriverObject->DeviceCtrl(
		(__COMMON_OBJECT*)pFileDriver->lpDriverObject,
		(__COMMON_OBJECT*)pFileDriver,
		pDrcb))
	{
		goto __TERMINAL;
	}
	/* 
	 * File system processes successfully,then return the result.
	 * lpOutputBuffer contains the finding handle.
	 */
	pResult = (__COMMON_OBJECT*)pDrcb->lpOutputBuffer;

__TERMINAL:
	if(pDrcb)
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return pResult;
}

/* FindNextFile,use the handle that FindFirstFile returned. */
static BOOL _FindNextFile(__COMMON_OBJECT* lpThis,
	LPCTSTR lpszFileName,
	__COMMON_OBJECT* pFindHandle,
	FS_FIND_DATA* pFindData)
{
	__DEVICE_OBJECT*         pFileDriver  = NULL;
	BYTE                     FsIdentifier = 0;
	__DRCB*                  pDrcb        = NULL;
	DWORD                    dwFlags;
	BOOL                     bResult      = FALSE;
	__IO_MANAGER*            pIoManager   = (__IO_MANAGER*)lpThis;
	int                      i;

	if((NULL == lpThis) || (NULL == lpszFileName) || (NULL == pFindData) || (NULL == pFindHandle))
	{
		goto __TERMINAL;
	}
	//Try to create DRCB object and initialize it.
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

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDNEXTFILE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindData;       //Use extra parameter to pass data.
	pDrcb->lpInputBuffer = (LPVOID)pFindHandle;

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	if(NULL == pFileDriver)  //Can not find the appropriate file system.
	{
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pFileDriver->dwSignature)
	{
		goto __TERMINAL;
	}
	//Issue the IO control command to file system driver object.
	if(0 == pFileDriver->lpDriverObject->DeviceCtrl(
		(__COMMON_OBJECT*)pFileDriver->lpDriverObject,
		(__COMMON_OBJECT*)pFileDriver,
		pDrcb))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* 
 * FindClose routine,call this routine after finding iteration finished,
 * to release the finding resource.
 */
static BOOL _FindClose(__COMMON_OBJECT* lpThis,
	LPCTSTR lpszFileName,
	__COMMON_OBJECT* pFindHandle)
{
	__DEVICE_OBJECT*         pFileDriver  = NULL;
	BYTE                     FsIdentifier = 0;
	__DRCB*                  pDrcb        = NULL;
	DWORD                    dwFlags;
	BOOL                     bResult      = FALSE;
	__IO_MANAGER*            pIoManager   = (__IO_MANAGER*)lpThis;
	int                      i;

	if((NULL == lpThis) || (NULL == lpszFileName) || (NULL == pFindHandle))
	{
		goto __TERMINAL;
	}
	//Try to create DRCB object and initialize it.
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

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDCLOSE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindHandle;     //Use extra parameter to pass data.

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	if(NULL == pFileDriver)  //Can not find the appropriate file system.
	{
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pFileDriver->dwSignature)
	{
		goto __TERMINAL;
	}
	//Issue the IO control command to file system driver object.
	if(0 == pFileDriver->lpDriverObject->DeviceCtrl(
		(__COMMON_OBJECT*)pFileDriver->lpDriverObject,
		(__COMMON_OBJECT*)pFileDriver,
		pDrcb))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* GetFileAttributes routine,returns the file's attributes information. */
static DWORD _GetFileAttributes(__COMMON_OBJECT* lpThis,
							   LPCTSTR lpszFileName)
{
	__DEVICE_OBJECT*         pFileDriver  = NULL;
	BYTE                     FsIdentifier = 0;
	__DRCB*                  pDrcb        = NULL;
	DWORD                    dwFlags;
	DWORD                    dwAttributes = 0;
	__IO_MANAGER*            pIoManager   = (__IO_MANAGER*)lpThis;
	int                      i;

	if((NULL == lpThis) || (NULL == lpszFileName))
	{
		goto __TERMINAL;
	}
	//Try to create DRCB object and initialize it.
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

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_GETFILEATTR;  //Get file's attributes.
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;       //Use extra parameter to pass name.

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoManager->spin_lock, dwFlags);
	if(NULL == pFileDriver)  //Can not find the appropriate file system.
	{
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pFileDriver->dwSignature)
	{
		goto __TERMINAL;
	}
	//Issue the IO control command to file system driver object.
	if(0 == pFileDriver->lpDriverObject->DeviceCtrl(
		(__COMMON_OBJECT*)pFileDriver->lpDriverObject,
		(__COMMON_OBJECT*)pFileDriver,
		pDrcb))  //File system processes failed.
	{
		goto __TERMINAL;
	}
	//Control command successfully,dwExtraParam2 will contain the file's attributes.
	dwAttributes = pDrcb->dwExtraParam2;
__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return dwAttributes;
}

/*
 * Get a file object's size, or storage device
 * volume.
 */
static DWORD _GetFileSize(__COMMON_OBJECT* lpThis,
	__COMMON_OBJECT* lpFileObject,
	DWORD* lpdwSizeHigh)
{
	__DRIVER_OBJECT* pDrvObject = NULL;
	__DEVICE_OBJECT* pFileObject = (__DEVICE_OBJECT*)lpFileObject;
	__DRCB* pDrcb = NULL;
	DWORD dwResult = 0;

	BUG_ON(NULL == pFileObject);

	pDrvObject = pFileObject->lpDriverObject;
	BUG_ON(NULL == pDrvObject);
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		return 0;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);
		return 0;
	}

	/* Issue device size command to driver. */
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_SIZE;
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen = 0;
	pDrcb->lpInputBuffer = 0;
	/* 
	 * Use output buffer to contain high part of size. 
	 * High part size will be saved upon return of device
	 * size command.
	 */
	pDrcb->lpOutputBuffer = lpdwSizeHigh;
	if (lpdwSizeHigh)
	{
		pDrcb->dwOutputLen = sizeof(*lpdwSizeHigh);
	}
	dwResult = pDrvObject->DeviceSize((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pFileObject,
		pDrcb);

	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);

	return dwResult;
}

/* Write buffered file data into storage immediately. */
static BOOL _FlushFileBuffers(__COMMON_OBJECT* lpThis,
							  __COMMON_OBJECT* lpFileObject)
{
	__DRCB*                   pDrcb       = NULL;
	__DEVICE_OBJECT*          pFileObject = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*          pFileDriver = NULL;
	DWORD                     dwResult    = 0;

	if(NULL == pFileObject)
	{
		return FALSE;
	}
	//Validate file object's validity.
	if(DEVICE_OBJECT_SIGNATURE != pFileObject->dwSignature)
	{
		return FALSE;
	}
	pFileDriver = pFileObject->lpDriverObject;

	//Create DRCB object and issue device flush command.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		return FALSE;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
		return FALSE;
	}

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_FLUSH;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(__DEVICE_OBJECT*);
	pDrcb->lpInputBuffer   = (LPVOID)pFileObject;

	/* Just to call the driver's implementation of DeviceFlush routine. */
	dwResult = pFileDriver->DeviceFlush((__COMMON_OBJECT*)pFileDriver,
		(__COMMON_OBJECT*)pFileObject,
		pDrcb);

	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)pDrcb);
	return dwResult ? TRUE : FALSE;
}

/* 
 * CreateDevice,this routine is called by device
 * driver(s) to create device object.Generally,this routine is called in
 * DriverEntry of device driver(s).
 * This routine does the following:
 *  1. Creates a device object by calling ObjectManager's interface;
 *  2. Initializes the device object;
 *  3. Allocates a block of memory as device object's extension;
 *  4. Inserts the device object into device object's list.
*/
static __DEVICE_OBJECT* kCreateDevice(__COMMON_OBJECT*  lpThis,
	LPSTR             lpszDevName,
	DWORD             dwAttribute,
	DWORD             dwBlockSize,
	DWORD             dwMaxReadSize,
	DWORD             dwMaxWriteSize,
	LPVOID            lpDevExtension,
	__DRIVER_OBJECT*  lpDrvObject)
{
	__DEVICE_OBJECT*                 lpDevObject = NULL;
	__DEVICE_OBJECT*                 lpFsDriver = NULL;
	__DRCB*                          lpDrcb = NULL;
	__IO_MANAGER*                    lpIoManager = (__IO_MANAGER*)lpThis;
	DWORD                            dwFlags = 0;
	DWORD                            dwCtrlRet = 0;   //Return value of DeviceCtrl routine.
	int                              i;

	/* Validate parameters. */
	if ((NULL == lpThis) || (NULL == lpszDevName) || (NULL == lpDrvObject))
	{
		return NULL;
	}

	/* Device's name must not exceed the MAX_DEV_NAME_LEN. */
	if (StrLen(lpszDevName) > MAX_DEV_NAME_LEN)
	{
		return NULL;
	}

	lpDevObject = (__DEVICE_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_DEVICE);
	if (NULL == lpDevObject)
	{
		return NULL;
	}

	/* Initialize the device object. */
	if (!lpDevObject->Initialize((__COMMON_OBJECT*)lpDevObject))
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDevObject);
		return NULL;
	}

	//Initialize the device object's members.
	lpDevObject->dwSignature = DEVICE_OBJECT_SIGNATURE;  //Validate the device object.
	lpDevObject->lpDevExtension = lpDevExtension;  //Device extension of this object.
	lpDevObject->lpDriverObject = lpDrvObject;     //Driver object of this device.
	lpDevObject->dwAttribute = dwAttribute;
	lpDevObject->dwBlockSize = dwBlockSize;
	lpDevObject->dwMaxWriteSize = dwMaxWriteSize;
	lpDevObject->dwMaxReadSize = dwMaxReadSize;
	lpDevObject->lpNext = NULL;
	lpDevObject->lpPrev = NULL;

	StrCpy(lpszDevName, (LPSTR)&lpDevObject->DevName[0]);
	if (!(dwAttribute & DEVICE_TYPE_PARTITION))
	{
		goto __CONTINUE;
	}

	//If this is a partition object,then give a chance to file system(s) to
	//check if this partition is file system's target.
	for (i = 0; i < FS_CTRL_NUM; i++)
	{
		if (lpIoManager->FsCtrlArray[i])
		{
			lpFsDriver = (__DEVICE_OBJECT*)lpIoManager->FsCtrlArray[i];
			if (lpFsDriver->dwSignature != DEVICE_OBJECT_SIGNATURE)
			{
				break;
			}
			lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
				NULL,
				OBJECT_TYPE_DRCB);
			if (NULL == lpDrcb)
				goto __CONTINUE;

			if (!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb))
			{
				ObjectManager.DestroyObject(&ObjectManager,
					(__COMMON_OBJECT*)lpDrcb);
				goto __CONTINUE;
			}

			lpDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
			lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
			lpDrcb->dwCtrlCommand = IOCONTROL_FS_CHECKPARTITION;
			lpDrcb->dwInputLen = sizeof(__DEVICE_OBJECT*);
			lpDrcb->lpInputBuffer = (LPVOID)lpDevObject;
			//Now issue the check partition command.
			dwCtrlRet = lpFsDriver->lpDriverObject->DeviceCtrl(
				(__COMMON_OBJECT*)lpFsDriver->lpDriverObject,
				(__COMMON_OBJECT*)lpFsDriver,
				lpDrcb);
			//When checking partition finished,DRCB object should be released.
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpDrcb);
			if (dwCtrlRet)  //The appropriate file system is found.
			{
				break;
			}
		}
	}

__CONTINUE:

	/*
	 * Add the device object into device object's global list,
	 * can not be interrupted.
	 */
	__ENTER_CRITICAL_SECTION_SMP(lpIoManager->spin_lock, dwFlags);
	if (NULL == lpIoManager->lpDeviceRoot)  //This is the first object.
	{
		lpIoManager->lpDeviceRoot = lpDevObject;
	}
	else    //This is not the first object.
	{
		lpDevObject->lpNext = lpIoManager->lpDeviceRoot;
		lpDevObject->lpPrev = NULL;
		lpIoManager->lpDeviceRoot->lpPrev = lpDevObject;
		lpIoManager->lpDeviceRoot = lpDevObject;
	}
	__LEAVE_CRITICAL_SECTION_SMP(lpIoManager->spin_lock, dwFlags);

	return lpDevObject;
}

/* Destroy a specified device object. */
static VOID __DestroyDevice(__COMMON_OBJECT* lpThis,
	 __DEVICE_OBJECT* lpDeviceObject)
{
	/* Delegate to object manager to destroy it. */
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)lpDeviceObject);

	return;
}

//Default device driver dispatch routine.When a driver object is created,
//it's dispatch routines are initialized as this routine.This is a dumb routine,do
//nothing.
static DWORD DefaultDrvDispatch(__COMMON_OBJECT* lpDrv,
								__COMMON_OBJECT* lpDev,
								__DRCB* lpDrcb)
{
	return 0;
}

static __COMMON_OBJECT* DefaultOpen(__COMMON_OBJECT* lpDrv,
									__COMMON_OBJECT* lpDev,
									__DRCB* lpDrcb)
{
	return NULL;
}

//
//The implementation of LoadDriver.
//This routine is used by OS loader to load device drivers.
//
static BOOL LoadDriver(__DRIVER_ENTRY DrvEntry)
{
	__DRIVER_OBJECT*   lpDrvObject  = NULL;

	if(NULL == DrvEntry)  //Invalid parameter.
	{
		return FALSE;
	}

	lpDrvObject = (__DRIVER_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_DRIVER);
	if(NULL == lpDrvObject)  //Can not create driver object.
	{
		return FALSE;
	}
	if(!lpDrvObject->Initialize((__COMMON_OBJECT*)lpDrvObject)) //Initialize failed.
	{
		return FALSE;
	}
	//Initialize driver object.
	lpDrvObject->DeviceClose    = DefaultDrvDispatch;
	lpDrvObject->DeviceCreate   = DefaultDrvDispatch;
	lpDrvObject->DeviceCtrl     = DefaultDrvDispatch;
	lpDrvObject->DeviceDestroy  = DefaultDrvDispatch;
	lpDrvObject->DeviceFlush    = DefaultDrvDispatch;
	lpDrvObject->DeviceOpen     = DefaultOpen;
	lpDrvObject->DeviceRead     = DefaultDrvDispatch;
	lpDrvObject->DeviceSeek     = DefaultDrvDispatch;
	lpDrvObject->DeviceWrite    = DefaultDrvDispatch;
	lpDrvObject->lpNext         = NULL;
	lpDrvObject->lpPrev         = NULL;

	//Call the driver entry.
	if(DrvEntry(lpDrvObject))
	{
		return TRUE;
	}
	//Failed to call DrvEntry routine,so release the driver object.
	ObjectManager.DestroyObject(
		&ObjectManager,
		(__COMMON_OBJECT*)lpDrvObject);
	return FALSE;
}

//The implementation of RegisterFileSystem.
static BOOL AddFileSystem(__COMMON_OBJECT* lpThis,
						  __COMMON_OBJECT* lpDevObj,
						  DWORD            dwAttribute,
						  CHAR*            pVolumeLbl)
{
	BOOL                       bResult      = FALSE;
	__IO_MANAGER*              pMgr         = (__IO_MANAGER*)lpThis;
	__DEVICE_OBJECT*           pFileSystem  = (__DEVICE_OBJECT*)lpDevObj;
	BYTE                       FsIdentifier = 'C';  //First file system identifier.
	DWORD                      dwFlags;
	int                        i,j,k;

	if((NULL == pMgr) || (NULL == pFileSystem))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	//Seek the empty slot of file system array,if there is.
	__ENTER_CRITICAL_SECTION_SMP(pMgr->spin_lock, dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(0 == pMgr->FsArray[i].FileSystemIdentifier)  //Empty slot.
		{
			break;
		}
	}
	if(FILE_SYSTEM_NUM == i)  //No slot is free.
	{
		__LEAVE_CRITICAL_SECTION_SMP(pMgr->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	//Calculate the file system identifier,if the maximal file system identifier
	//in current array is x,then use x + 1 as new file system's identifier.
	for(j = 0;j < FILE_SYSTEM_NUM;j ++)
	{
		if(pMgr->FsArray[j].FileSystemIdentifier >= FsIdentifier)
		{
			FsIdentifier += 1;  //Use next one.
		}
	}
	pMgr->FsArray[i].FileSystemIdentifier = FsIdentifier;
	pMgr->FsArray[i].pFileSystemObject    = (__COMMON_OBJECT*)pFileSystem;
	pMgr->FsArray[i].dwAttribute          = dwAttribute;
	//Set volume lable.
	k = 0;
	for(j = 0;j < VOLUME_LBL_LEN - 1;j ++)
	{
		if(' ' == pVolumeLbl[j])  //Skip space.
		{
			continue;
		}
		pMgr->FsArray[i].VolumeLbl[k] = pVolumeLbl[j];
		k += 1;
	}
	pMgr->FsArray[i].VolumeLbl[k] = 0;   //Set terminator.
	__LEAVE_CRITICAL_SECTION_SMP(pMgr->spin_lock, dwFlags);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Reserve system level resources,such as IO port,mapped device memory. */
static BOOL ReserveResource(__COMMON_OBJECT*    lpThis,
							__RESOURCE_DESCRIPTOR* lpResDesc)
{
	BOOL                    bResult             = FALSE;

	if((NULL == lpThis) || (NULL == lpResDesc)) //Parameters check.
	{
		return bResult;
	}
	return bResult;
}

/* 
 * Return a device object by giving it's name. 
 * The refer counter is increated before return
 * it,so the device must be released by invoking
 * ReleaseDevice routine.
 */
static __DEVICE_OBJECT* __GetDevice(__COMMON_OBJECT* lpThis, char* device_name)
{
	__IO_MANAGER* pIoMgr = (__IO_MANAGER*)lpThis;
	__DEVICE_OBJECT* pDevice = NULL;
	unsigned long ulFlags;

	BUG_ON(NULL == pIoMgr);
	BUG_ON(NULL == device_name);

	/* Travel the whole device list to find device. */
	__ENTER_CRITICAL_SECTION_SMP(pIoMgr->spin_lock, ulFlags);
	pDevice = pIoMgr->lpDeviceRoot;
	while (pDevice)
	{
		if (0 == strcmp(pDevice->DevName, device_name))
		{
			break;
		}
		pDevice = pDevice->lpNext;
	}
	if (pDevice)
	{
		/* Found,increase the refer count. */
		if (!ObjectManager.AddRefCount(&ObjectManager,
			(__COMMON_OBJECT*)pDevice))
		{
			/* The device maybe destroyed. */
			pDevice = NULL;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pIoMgr->spin_lock, ulFlags);

	return pDevice;
}

/* Release the device. */
static BOOL __ReleaseDevice(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* pDeviceObject)
{
	__IO_MANAGER* pIoMgr = (__IO_MANAGER*)lpThis;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pIoMgr);
	BUG_ON(NULL == pDeviceObject);

	/* Decrease the ref counter. */
	bResult = ObjectManager.DestroyObject(&ObjectManager,
		pDeviceObject);

	return bResult;
}

/*
 * Defines one of the global objects in HelloX - IOManager.
 * This object is a global object,and only one in the whole system life-cycle,
 * any kernel level IO operations,are delegated by this object.
 */
__IO_MANAGER IOManager = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,                   //spin_lock.
#endif
	NULL,                                   //lpDeviceRoot.
	NULL,                                   //lpDriverRoot.
	{0},                                    //FsArray.
	{0},                                    //FsCtrlArray.
	0,                                      //dwPartitionNumber;
	NULL,                                   //lpResDescriptor.
	IOManagerInitialize,                    //Initialize.
	_CreateFile,                            //CreateFile,
	_ReadFile,                              //ReadFile,
	_WriteFile,                             //WriteFile,
	_CloseFile,                             //CloseFile,
	_CreateDirectory,                       //CreateDirectory,
	_DeleteFile,                            //DeleteFile,
	_FindClose,                             //FindClose,
	_FindFirstFile,                         //FindFirstFile,
	_FindNextFile,                          //FindNextFile,
	_GetFileAttributes,                     //GetFileAttributes,
	_GetFileSize,                           //GetFileSize,
	_RemoveDirectory,                       //RemoveDirectory,
	_SetEndOfFile,                          //SetEndOfFile,
	_IOControl,                             //IOControl,
	_SetFilePointer,                        //SetFilePointer,
	_FlushFileBuffers,                      //FlushFileBuffers,

	kCreateDevice,                           //CreateDevice.
	__GetDevice,                             //GetDevice.
	__ReleaseDevice,                         //ReleaseDevice.
	__DestroyDevice,                         //DestroyDevice.
	ReserveResource,                         //ReserveResource.
	LoadDriver,
	AddFileSystem,
	RegisterFileSystem
};

#endif
