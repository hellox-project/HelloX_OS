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

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "iomgr.h"
#include "commobj.h"
#include "string.h"

//Only Device Driver Framework is enabled the following code is included in the
//OS kernel.
#ifdef __CFG_SYS_DDF

//The following routines are implemented in IOMGR2.CPP.
extern BOOL _SetEndOfFile(__COMMON_OBJECT* lpThis,
						  __COMMON_OBJECT* lpFileObject);
extern BOOL CreateNewFile(__COMMON_OBJECT* lpIOManager,
				   LPSTR            lpszFileName);
extern DWORD WaitForCompletion(__COMMON_OBJECT* lpThis);
extern DWORD OnCompletion(__COMMON_OBJECT* lpThis);
extern DWORD OnCancel(__COMMON_OBJECT* lpThis);
extern BOOL _WriteFile(__COMMON_OBJECT* lpThis,
			   __COMMON_OBJECT* lpFileObj,
			   DWORD dwWriteSize,
			   LPVOID lpBuffer,
			   DWORD* lpdwWrittenSize);
extern BOOL _ReadFile(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpFileObject,
					  DWORD dwByteSize,LPVOID lpBuffer,DWORD* lpReadSize);
extern VOID _CloseFile(__COMMON_OBJECT* lpThis,__COMMON_OBJECT* lpFileObj);
extern BOOL _DeleteFile(__COMMON_OBJECT* lpThis,LPCTSTR lpszFileName);
extern BOOL _RemoveDirectory(__COMMON_OBJECT* lpThis,LPCTSTR lpszFileName);
//
//The implementation of IOManager.
//
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

	if((NULL == pFileSystem) || (NULL == lpThis)) //Invalid parameters.
	{
		return FALSE;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FS_CTRL_NUM;i ++)
	{
		if(NULL == pManager->FsCtrlArray[i]) //Find a empty slot.
		{
			break;
		}
	}
	if(FS_CTRL_NUM == i)  //Can not find a empty slot.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		return FALSE;
	}
	//Insert the file system object into this slot.
	pManager->FsCtrlArray[i] = pFileSystem;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return TRUE;
}

//
//The initialize routine of IOManager.
//This routine does the following:
// 1. 
//

static BOOL IOManagerInitialize(__COMMON_OBJECT* lpThis)
{
	BOOL                       bResult         = FALSE;
	//__IO_MANAGER*              lpIoManager     = NULL;

	if(NULL == lpThis)    //Parameter check.
	{
		return bResult;
	}

	bResult = TRUE;

	return bResult;
}

//Used to open regular file.
static __COMMON_OBJECT* __OpenFile(__COMMON_OBJECT* lpThis,  //IOManager object.
								  LPSTR            lpszFileName,
								  DWORD            dwAccessMode,
								  DWORD            dwShareMode)
{
	__COMMON_OBJECT*              pFileObject    = NULL;
	__DRIVER_OBJECT*              pFsDriver      = NULL;
	__DEVICE_OBJECT*              pFsObject      = NULL;
	__IO_MANAGER*                 pIoManager     = (__IO_MANAGER*)lpThis;
	DWORD                         dwFlags;
	BYTE                          FsIdentifier   = 0;  //File system identifier.
	__DRCB*                       pDrcb          = NULL;
	int                           i;

	FsIdentifier   = TO_CAPITAL(lpszFileName[0]);
	//Create DRCB object first.
    pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

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

	//Initialie the DRCB object,then call DeviceOpen routine of file system object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_OPEN;
	pDrcb->dwInputLen      = dwAccessMode;
	pDrcb->dwOutputLen     = dwShareMode;
	pDrcb->lpInputBuffer   = (LPVOID)lpszFileName;

	pFileObject = pFsDriver->DeviceOpen((__COMMON_OBJECT*)pFsDriver,
		(__COMMON_OBJECT*)pFsObject,
		pDrcb);

__TERMINAL:
	if(NULL != pDrcb)  //DRCB object created yet,release it.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return pFileObject;
}

//Used to open device object.
static __COMMON_OBJECT* __OpenDevice(__COMMON_OBJECT* lpThis,
									LPSTR            lpszFileName,
									DWORD            dwAccessMode,
									DWORD            dwShareMode)
{
	__DEVICE_OBJECT* pDevice = NULL;
	DWORD dwFlags;
	//Travel the whole device list to find the desired one.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pDevice = ((__IO_MANAGER*)lpThis)->lpDeviceRoot;
	while(pDevice)
	{
		//if(StrCmp(lpszFileName,(LPSTR)&pDevice->DevName[0]))  //Find.
		if(0 == strcmp(lpszFileName,&pDevice->DevName[0])) //Find the device.
		{
			break;
		}
		pDevice = pDevice->lpNext;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	return (__COMMON_OBJECT*)pDevice;
}

//
//The CreateFile routine's implementation of IOManager.
//Several tips about this routine:
// 1. In current version implementation of Hello China,all devices are treated as files,
//    so,if users want to access device,he or she can open the target device by calling
//    this routine;
// 2. One file or device can be opend as READ ONLY,WRITE ONLY,or READ WRITE,the dwAccessMode
//    parameter of this routine indicates the opening mode;
// 3. In current version,one file or device can be opend more than one time,so,the
//    dwShareMode indicates the re-open mode of the currently opend file,for example,if one
//    kernel thread opens a file as READ WRITE mode,and also indicates the OS that this
//    file can only be re-opend as READ mode(by seting the appropriate value of the
//    dwShareMode parameter),so,if there is another kernel thread want to open the file
//    as READ WRITE mode or WRITE mode,it will fail,by contraries,if the second kernel
//    thread want to open the file only in READ ONLY mode,it will success.
// 4. The last parameter,lpReserved,is a reserved parameter that may be used in the future.
//Once success,this routine returns the base address of the device object that opend,
//otherwise,it will return a NULL value to indicate the failure,user can determine the
//failing reason by calling GetLastError routine.
//
//The routine does the following:
// 1. 
//

//CreateFile,open a device or file given it's name.
static __COMMON_OBJECT* _CreateFile(__COMMON_OBJECT* lpThis,  //IOManager object.
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
	if(StrLen(lpszFileName) > 511)  //File name too long.
	{
		return NULL;
	}
	if((lpszFileName[0] == 0) || 
	   (lpszFileName[1] == 0) ||
	   (lpszFileName[2] == 0)) //Target file name should has
		                       //at lease 3 characters.
	{
		return NULL;
	}

	//strcpy(FileName,lpszFileName);
	StrCpy(lpszFileName,FileName);
	ToCapital(FileName);  //Convert to capital.
	if(IS_LETTER(FileName[0]))  //Maybe a file object.
	{
		if(FileName[1] != ':')  //Invalid file system name.
		{
			return NULL;
		}
		if(FileName[2] != '\\') //Third character should specify root directory.
		{
			return NULL;
		}
		//A valid file name specified,so try to open it.
		pFileHandle = __OpenFile(lpThis,FileName,dwAccessMode,dwShareMode);
		if(NULL == pFileHandle)
		{
			if(FILE_OPEN_ALWAYS & dwAccessMode)  //Try to create one.
			{
				if(CreateNewFile(lpThis,FileName))  //Can create it.
				{
					pFileHandle = __OpenFile(lpThis,FileName,dwAccessMode,dwShareMode); //Try to open again.
				}
			}
		}
		return pFileHandle;
	}
	//The target name is not a file name,check if a device name.
	if((FileName[0] == '\\') && //For device name,the first 2 character should be '\'.
	   (FileName[1] == '\\') &&
	   (FileName[2] == '.' ))   //The third character should be a dot.
	{
		if(FileName[3] != '\\') //The 4th character also must be '\'.
		{
			return NULL;
		}
		//The name is a device name,try to open ti.
		return __OpenDevice(lpThis,FileName,dwAccessMode,dwShareMode);
	}
	return NULL;
}

//
//The implementation of IOControl.
//This routine does the following:
// 1. 
//
//Implementation of IOControl.Through this routine user can issue some uncommon
//commands related to specific device.
static BOOL kIOControl(__COMMON_OBJECT* lpThis,          //IOManager itself.
					  __COMMON_OBJECT* lpFileObject,    //Target file object.
					  DWORD            dwCommand,       //Command to do.
					  DWORD            dwInputLen,      //Input buffer's length.
					  LPVOID           lpInputBuffer,   //Input buffer.
					  DWORD            dwOutputLen,     //Output buffer's length.
					  LPVOID           lpOutputBuffer,  //Output buffer.
					  DWORD*           lpOutputFilled)  //How many bytes returned.
{
	BOOL              bResult          = FALSE;
	__DEVICE_OBJECT*  pDevice          = (__DEVICE_OBJECT*)lpFileObject;
	__DRIVER_OBJECT*  pDriver          = NULL;
	__DRCB*           pDrcb            = NULL;
	DWORD             dwRetValue       = 0;
	DWORD             dwFlags;
 
	if((NULL == lpThis) || (NULL == lpFileObject))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	//Initialize DRCB object.
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;  //Request mode.
	pDrcb->dwCtrlCommand   = dwCommand;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = dwInputLen;
	pDrcb->lpInputBuffer   = lpInputBuffer;
	pDrcb->dwOutputLen     = dwOutputLen;
	pDrcb->lpOutputBuffer  = lpOutputBuffer;
	//Now issue the IOControl command to device.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(pDevice->dwSignature != DEVICE_OBJECT_SIGNATURE)  //Validation failed.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	pDriver = pDevice->lpDriverObject;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	dwRetValue = pDriver->DeviceCtrl((__COMMON_OBJECT*)pDriver,
		(__COMMON_OBJECT*)pDevice,
		pDrcb);
	if(lpOutputFilled)
	{
		*lpOutputFilled = dwRetValue;
	}
	if(DRCB_STATUS_FAIL == pDrcb->dwStatus)  //Failed to handle.
	{
		bResult = FALSE;
	}
	else  //Assume the handling is successfully.
	{
		bResult = TRUE;
	}

__TERMINAL:
	if(pDrcb)  //Should release it.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

//Implementation of CreateDirectory.
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

//Implementation of FindFirstFile.
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
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDFIRSTFILE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindData;       //Use extra parameter to pass data.

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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
	//File system processes successfully,then return the result.
	pResult = (__COMMON_OBJECT*)pDrcb->lpOutputBuffer;  //Output buffer contains
	                                                    //the find result.

__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return pResult;
}

//Implementation of FindNextFile.
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

	if((NULL == lpThis) || (NULL == lpszFileName) || (NULL == pFindData)
		|| (NULL == pFindHandle))
	{
		goto __TERMINAL;
	}
	//Try to create DRCB object and initialize it.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDNEXTFILE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindData;       //Use extra parameter to pass data.
	pDrcb->lpInputBuffer = (LPVOID)pFindHandle;

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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
	bResult = TRUE;

__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

//Implementation of FindClose routine.
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
	if(NULL == pDrcb)        //Failed to create DRCB object.
		goto __TERMINAL;

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
		goto __TERMINAL;

	pDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_FS_FINDCLOSE;
	pDrcb->dwExtraParam1 = (DWORD)lpszFileName;    //Use extra parameter to pass name.
	pDrcb->dwExtraParam2 = (DWORD)pFindHandle;     //Use extra parameter to pass data.

	//Find the appropriate file system object.
	FsIdentifier = TO_CAPITAL(lpszFileName[0]);
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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
	bResult = TRUE;

__TERMINAL:
	if(pDrcb)  //Should release the DRCB object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

//Implementation of GetFileAttributes.
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
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(pIoManager->FsArray[i].FileSystemIdentifier == FsIdentifier)
		{
			pFileDriver = (__DEVICE_OBJECT*)pIoManager->FsArray[i].pFileSystemObject;
			break;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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

//Implementation of GetFileSize.
static DWORD _GetFileSize(__COMMON_OBJECT* lpThis,
						  __COMMON_OBJECT* lpFileObject,
						  DWORD* lpdwSizeHigh)
{
	__DRIVER_OBJECT*        pDrvObject  = NULL;
	__DEVICE_OBJECT*        pFileObject = (__DEVICE_OBJECT*)lpFileObject;
	__DRCB*                 pDrcb       = NULL;
	DWORD                   dwResult    = 0;

	if((NULL == pFileObject)) 
	{
		return 0;
	}

	pDrvObject = pFileObject->lpDriverObject;
	//Create DRCB object and issue DeviceSeek command to file system driver.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,NULL,OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		return 0;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pDrcb);

		return 0;
	}

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_SIZE;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(DWORD);
	pDrcb->lpInputBuffer   = 0;      //Use input buffer to contain move scheme,from begin or current.
	
	dwResult = pDrvObject->DeviceSize((__COMMON_OBJECT*)pDrvObject,(__COMMON_OBJECT*)pFileObject,pDrcb);
	ObjectManager.DestroyObject(&ObjectManager,	(__COMMON_OBJECT*)pDrcb);

	return dwResult;
}

//Implementation of SetFilePointer.
static DWORD _SetFilePointer(__COMMON_OBJECT* lpThis,
							__COMMON_OBJECT* lpFileObject,
							DWORD* pdwDistLow,
							DWORD* pdwDistHigh,
							DWORD  dwWhereBegin)
{
	__DRIVER_OBJECT*        pDrvObject  = NULL;
	__DEVICE_OBJECT*        pFileObject = (__DEVICE_OBJECT*)lpFileObject;
	__DRCB*                 pDrcb       = NULL;
	DWORD                   dwResult    = 0;

	if((NULL == pFileObject) || (NULL == pdwDistLow))  //Low part of offset must be not null.
	{
		return -1;
	}
		
	if((FILE_FROM_BEGIN != dwWhereBegin) && (FILE_FROM_CURRENT != dwWhereBegin) && (FILE_FROM_END != dwWhereBegin))
	{
		return -1;
	}
	//Check if file object's validity.
	if(DEVICE_OBJECT_SIGNATURE != pFileObject->dwSignature)
	{
		return -1;
	}
	pDrvObject = pFileObject->lpDriverObject;
	//Create DRCB object and issue DeviceSeek command to file system driver.
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_DRCB);
	if(NULL == pDrcb)        //Failed to create DRCB object.
	{
		return -1;
	}

	if(!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))  //Failed to initialize.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)pDrcb);

		return -1;
	}

	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_SEEK;
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwInputLen      = sizeof(DWORD);
	pDrcb->lpInputBuffer   = (LPVOID)dwWhereBegin;      //Use input buffer to contain move scheme,from begin or current.
	pDrcb->dwExtraParam1   = (DWORD)pdwDistLow;
	pDrcb->dwExtraParam2   = (DWORD)pdwDistHigh;

	dwResult = pDrvObject->DeviceSeek((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pFileObject,
		pDrcb);
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)pDrcb);

	return dwResult;
}

//Implementation of FlushFileBuffers.
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

	dwResult = pFileDriver->DeviceFlush((__COMMON_OBJECT*)pFileDriver,
		(__COMMON_OBJECT*)pFileObject,
		pDrcb);

	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)pDrcb);
	return dwResult ? TRUE : FALSE;
}

//The implementation of CreateDevice routine.
//
//The implementation of CreateDevice,this routine is called by device
//driver(s) to create device object.Generally,this routine is called in
//DriverEntry of device driver(s).
//This routine does the following:
// 1. Creates a device object by calling ObjectManager's interface;
// 2. Initializes the device object;
// 3. Allocates a block of memory as device object's extension;
// 4. Inserts the device object into device object's list.
//
static __DEVICE_OBJECT* kCreateDevice(__COMMON_OBJECT*  lpThis,
									 LPSTR             lpszDevName,
									 DWORD             dwAttribute,
									 DWORD             dwBlockSize,
									 DWORD             dwMaxReadSize,
									 DWORD             dwMaxWriteSize,
									 LPVOID            lpDevExtension,
									 __DRIVER_OBJECT*  lpDrvObject)
{
	__DEVICE_OBJECT*                 lpDevObject       = NULL;
	__DEVICE_OBJECT*                 lpFsDriver        = NULL;
	__DRCB*                          lpDrcb            = NULL;
	__IO_MANAGER*                    lpIoManager       = (__IO_MANAGER*)lpThis;
	DWORD                            dwFlags           = 0;
	DWORD                            dwCtrlRet         = 0;   //Return value of DeviceCtrl routine.
	int                              i;

	//Check the parameters.
	if((NULL == lpThis) || (NULL == lpszDevName) || (NULL == lpDrvObject))
	{
		return NULL;
	}

	if(StrLen(lpszDevName) > MAX_DEV_NAME_LEN)  //The device's name is too long.
	{
		return NULL;
	}

	lpDevObject    = (__DEVICE_OBJECT*)ObjectManager.CreateObject(
		&ObjectManager,
		NULL,
		OBJECT_TYPE_DEVICE);
	if(NULL == lpDevObject)      //Failed to create device object.
	{
		return NULL;
	}

	if(!lpDevObject->Initialize((__COMMON_OBJECT*)lpDevObject)) //Failed to initialize the object.
	{
		ObjectManager.DestroyObject(&ObjectManager,
			(__COMMON_OBJECT*)lpDevObject);
		return NULL;
	}

	//Initialize the device object's members.
	lpDevObject->dwSignature    = DEVICE_OBJECT_SIGNATURE;  //Validate the device object.
	lpDevObject->lpDevExtension = lpDevExtension;  //Device extension of this object.
	lpDevObject->lpDriverObject = lpDrvObject;     //Driver object of this device.
	lpDevObject->dwAttribute    = dwAttribute;
	lpDevObject->dwBlockSize    = dwBlockSize;
	lpDevObject->dwMaxWriteSize = dwMaxWriteSize;
	lpDevObject->dwMaxReadSize  = dwMaxReadSize;
	lpDevObject->lpNext         = NULL;
	lpDevObject->lpPrev         = NULL;

	StrCpy(lpszDevName,(LPSTR)&lpDevObject->DevName[0]);
	//strcpy((LPSTR)&lpDevObject->DevName[0],lpszDevName);
	if(!(dwAttribute & DEVICE_TYPE_PARTITION))  //This is not a partition,skip follows code.
	{
		goto __CONTINUE;
	}

	//If this is a partition object,then give a chance to file system(s) to
	//check if this partition is file system's target.
	for(i = 0;i < FS_CTRL_NUM;i++)
	{
		if(lpIoManager->FsCtrlArray[i])  //This slot is occupied.
		{
			lpFsDriver = (__DEVICE_OBJECT*)lpIoManager->FsCtrlArray[i];
			if(lpFsDriver->dwSignature != DEVICE_OBJECT_SIGNATURE)  //Invalid object.
			{
				break;
			}
			lpDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
				NULL,
				OBJECT_TYPE_DRCB);
			if(NULL == lpDrcb)        //Failed to create DRCB object.
				goto __CONTINUE;

			if(!lpDrcb->Initialize((__COMMON_OBJECT*)lpDrcb))  //Failed to initialize.
			{
				ObjectManager.DestroyObject(&ObjectManager,
					(__COMMON_OBJECT*)lpDrcb);
				goto __CONTINUE;
			}

			lpDrcb->dwStatus      = DRCB_STATUS_INITIALIZED;
			lpDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
			lpDrcb->dwCtrlCommand = IOCONTROL_FS_CHECKPARTITION;
			lpDrcb->dwInputLen    = sizeof(__DEVICE_OBJECT*);
			lpDrcb->lpInputBuffer = (LPVOID)lpDevObject;
			//Now issue the check partition command.
			dwCtrlRet = lpFsDriver->lpDriverObject->DeviceCtrl(
				(__COMMON_OBJECT*)lpFsDriver->lpDriverObject,
				(__COMMON_OBJECT*)lpFsDriver,
				lpDrcb);
			//When checking partition finished,DRCB object should be released.
			ObjectManager.DestroyObject(&ObjectManager,
				(__COMMON_OBJECT*)lpDrcb);
			if(dwCtrlRet)  //The appropriate file system is found.
			{
				break;
			}
		}
	}

__CONTINUE:
	//
	//The following code add the device object into device object's list.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpIoManager->lpDeviceRoot)  //This is the first object.
	{
		lpIoManager->lpDeviceRoot = lpDevObject;
	}
	else    //This is not the first object.
	{
		lpDevObject->lpNext                  = lpIoManager->lpDeviceRoot;
		lpDevObject->lpPrev                  = NULL;
		lpIoManager->lpDeviceRoot->lpPrev = lpDevObject;
		lpIoManager->lpDeviceRoot         = lpDevObject;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	return lpDevObject;
}

//Implementation of DestroyDevice routine.
static VOID kDestroyDevice(__COMMON_OBJECT* lpThis,
						  __DEVICE_OBJECT* lpDeviceObject)
{
	//When destroy a device,do not forget to clear the signature of device.For
	//example,if pDeviceObject is to be released,then do the following code before
	//release the memory:
	//
	// pDeviceObject->dwSignature  = 0;
	//
	//Then can release the memory occupied by pDeviceObject:
	// KMemFree(pDeviceObject);
	//
	__IO_MANAGER*         lpIoManager    = (__IO_MANAGER*)lpThis;
	DWORD                 dwFlags        = 0;

	if((NULL == lpThis) || (NULL == lpDeviceObject)) //Parameters check.
	{
		//BUG();
		return;
	}
	if(DEVICE_OBJECT_SIGNATURE != lpDeviceObject->dwSignature)  //Not a valid device.
	{
		return;
	}
	//
	//The following code deletes the device object from system list.
	//
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == lpDeviceObject->lpPrev)    //This is the first object.
	{
		if(NULL == lpDeviceObject->lpNext)  //This is the last object.
		{
			lpIoManager->lpDeviceRoot = NULL;
		}
		else    //This is not the last object.
		{
			lpDeviceObject->lpNext->lpPrev = NULL;
			lpIoManager->lpDeviceRoot      = lpDeviceObject->lpNext;
		}
	}
	else    //This is not the first object.
	{
		if(NULL == lpDeviceObject->lpNext)  //This is the last object.
		{
			lpDeviceObject->lpPrev->lpNext = NULL;
		}
		else    //This is not the last object.
		{
			lpDeviceObject->lpPrev->lpNext = lpDeviceObject->lpNext;
			lpDeviceObject->lpNext->lpPrev = lpDeviceObject->lpPrev;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Clear the signature of this device object.
	lpDeviceObject->dwSignature = 0;
	//Destroy the device object.
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
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	for(i = 0;i < FILE_SYSTEM_NUM;i ++)
	{
		if(0 == pMgr->FsArray[i].FileSystemIdentifier)  //Empty slot.
		{
			break;
		}
	}
	if(FILE_SYSTEM_NUM == i)  //No slot is free.
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
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
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//
//The implementation of ReserveResource.
//This routine does the following:
// 1. 
//

static BOOL ReserveResource(__COMMON_OBJECT*    lpThis,
							__RESOURCE_DESCRIPTOR*
							                    lpResDesc)
{
	BOOL                    bResult             = FALSE;

	if((NULL == lpThis) || (NULL == lpResDesc)) //Parameters check.
	{
		return bResult;
	}
	return bResult;
}

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The following code defines one of the global objects in Hello China - IOManager.
//This object is a global object,and only one in the whole system life-cycle.
//

__IO_MANAGER IOManager = {
	NULL,                                   //lpDeviceRoot.
	NULL,                                   //lpDriverRoot.
	{0},                                    //FsArray.
	{0},                                    //FsCtrlArray.
	NULL,                                   //lpResDescriptor.
	IOManagerInitialize,                    //Initialize.
	_CreateFile, //CreateFile,
	_ReadFile,   //ReadFile,
	_WriteFile,  //WriteFile,
	_CloseFile,   //CloseFile,
	_CreateDirectory,        //CreateDirectory,
	_DeleteFile,        //DeleteFile,
	_FindClose,        //FindClose,
	_FindFirstFile,        //FindFirstFile,
	_FindNextFile,        //FindNextFile,
	_GetFileAttributes,        //GetFileAttributes,
	_GetFileSize,        //GetFileSize,
	_RemoveDirectory,        //RemoveDirectory,
	_SetEndOfFile,        //SetEndOfFile,
	kIOControl,   //IOControl,
	_SetFilePointer,    //SetFilePointer,
	_FlushFileBuffers,  //FlushFileBuffers,

	kCreateDevice,                           //CreateDevice.
	kDestroyDevice,        //kDestroyDevice,                          //DestroyDevice.
	ReserveResource,        //ReserveResource,                        //ReserveResource.
	LoadDriver,
	AddFileSystem,
	RegisterFileSystem
};

#endif
