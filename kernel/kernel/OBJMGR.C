//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,03 2004
//    Module Name               : objmgr.cpp
//    Module Funciton           : 
//                                This module countains Object manager code.
//    Last modified Author      : Garry
//    Last modified Date        : Jun 11,2013
//    Last modified Content     :
//                                1. Condition compiling flags are added in
//                                   global object array's definition.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include "types.h"
#include "ktmgr.h"
#include <../config/config.h>
#include "process.h"
#include "kmemmgr.h"
#include "iomgr.h"
//
//The following array is used by Object Manager to create object.
//Once a new object type is defined,you must add one line in the
//following array using OBJECT_INIT_DATA,it's parameters meaning,
//please refer commobj.h.
//

BEGIN_DECLARE_INIT_DATA(ObjectInitData)
	// Please add your new defined object's initialization data here.
	OBJECT_INIT_DATA(OBJECT_TYPE_PRIORITY_QUEUE,sizeof(__PRIORITY_QUEUE),
	PriQueueInitialize,PriQueueUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_KERNEL_THREAD,sizeof(__KERNEL_THREAD_OBJECT),
	KernelThreadInitialize,KernelThreadUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_EVENT,sizeof(__EVENT),
	EventInitialize,EventUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_MUTEX,sizeof(__MUTEX),
	MutexInitialize,MutexUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_TIMER,sizeof(__TIMER_OBJECT),
	TimerInitialize,TimerUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_INTERRUPT,sizeof(__INTERRUPT_OBJECT),
	InterruptInitialize,InterruptUninitialize)

#ifdef __CFG_SYS_DDF  //Define the following objects only DDF enabled.
	OBJECT_INIT_DATA(OBJECT_TYPE_DRIVER,sizeof(__DRIVER_OBJECT),
	DrvObjInitialize,DrvObjUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_DEVICE,sizeof(__DEVICE_OBJECT),
	DevObjInitialize,DevObjUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_DRCB,sizeof(__DRCB),
	DrcbInitialize,DrcbUninitialize)
#endif

	OBJECT_INIT_DATA(OBJECT_TYPE_MAILBOX,sizeof(__MAIL_BOX),
	MailboxInitialize,MailboxUninitialize)

#ifdef __CFG_SYS_VMM
	OBJECT_INIT_DATA(OBJECT_TYPE_PAGE_INDEX_MANAGER,sizeof(__PAGE_INDEX_MANAGER),
	PageInitialize,PageUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER,sizeof(__VIRTUAL_MEMORY_MANAGER),
	VmmInitialize,VmmUninitialize)
#endif
	//OBJECT_INIT_DATA(OBJECT_TYPE_COMMON_QUEUE,sizeof(__COMMON_QUEUE),
	//CommQueueInit,CommQueueUninit)

	OBJECT_INIT_DATA(OBJECT_TYPE_SEMAPHORE,sizeof(__SEMAPHORE),
	SemInitialize,SemUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_CONDITION,sizeof(__CONDITION),
	ConditionInitialize,ConditionUninitialize)

	OBJECT_INIT_DATA(OBJECT_TYPE_PROCESS,sizeof(__PROCESS_OBJECT),
	ProcessInitialize,ProcessUninitialize)

END_DECLARE_INIT_DATA()

//
//The predefinition of ObjectManager's member functions.
//
static __COMMON_OBJECT* CreateObject(__OBJECT_MANAGER*,__COMMON_OBJECT*,DWORD);
static __COMMON_OBJECT* GetObjectByID(__OBJECT_MANAGER*,DWORD);
static __COMMON_OBJECT* GetFirstObjectByType(__OBJECT_MANAGER*,DWORD);
static VOID             DestroyObject(__OBJECT_MANAGER*,__COMMON_OBJECT*);

//
//The definition of the ObjectManager,the first object and the only object in Hello
//China.
//
__OBJECT_MANAGER    ObjectManager = {
	1,                                  //Current avaiable object ID.
	{0},
	CreateObject,                       //CreateObject routine.
	GetObjectByID,                      //GetObjectByID routine.
	GetFirstObjectByType,               //GetFirstObjectByType routine.
	DestroyObject,                      //DestroyObject routine.
};

//
// Create a object by Object Manager.
//
// Input:
//   @lpObjectManager    : The base address of Object Manager.
//   @lpObjectOwner      : The parent of the object will be created.
//   @dwType             : Which type of object will be created.
//
// Output:
//   If successfully,returns the base address of new created object,
//   otherwise,returns NULL.
//
static __COMMON_OBJECT* CreateObject(__OBJECT_MANAGER* lpObjectManager,    //Object Manager.
									 __COMMON_OBJECT*  lpObjectOwner,      //Object's owner.
									 DWORD             dwType)
{
	__COMMON_OBJECT* pObject         = NULL;
	DWORD            dwLoop          = 0;
	BOOL             bFind           = FALSE;
	DWORD            dwObjectSize    = 0;
	DWORD            dwFlags;

	if((NULL == lpObjectManager) || (dwType >= MAX_OBJECT_TYPE))  //Parameters valid check.
	{
		goto __TERMINAL;
	}

	while(TRUE)    //To find the initialize data of this type object.
	{
		if(MAX_OBJECT_TYPE == dwLoop)
		{
			break;
		}
		if(0 == ObjectInitData[dwLoop].dwObjectType)
		{
			break;
		}
		if(dwType == ObjectInitData[dwLoop].dwObjectType)
		{
			bFind = TRUE;
			break;
		}
		dwLoop ++;
	}
	
	if(FALSE == bFind)    //If can not find the corrent initialize data.
	{
		goto __TERMINAL;
	}

	dwObjectSize = ObjectInitData[dwLoop].dwObjectSize;
	if(0 == dwObjectSize)  //Invalid object size.
	{
		goto __TERMINAL;
	}

	pObject = (__COMMON_OBJECT*)KMemAlloc(dwObjectSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pObject)  //Can not allocate memory.
	{
		goto __TERMINAL;
	}

	//The following lines initialize the new created object.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pObject->dwObjectID = lpObjectManager->dwCurrentObjectID;
	lpObjectManager->dwCurrentObjectID ++;     //Now,update the Object Manager's status.

	pObject->dwObjectSize     = dwObjectSize;
	pObject->dwObjectType     = dwType;
	pObject->Initialize       = ObjectInitData[dwLoop].Initialize;
	pObject->Uninitialize     = ObjectInitData[dwLoop].Uninitialize;
	pObject->lpObjectOwner    = lpObjectOwner;

	//The following code insert the new created object into ObjectArrayList.
	if(NULL == lpObjectManager->ObjectListHeader[dwType].lpFirstObject)  //If this is the
		                                                                 //first object of
																		 //this type.
	{
		pObject->lpNextObject = NULL;
		pObject->lpPrevObject = NULL;
		lpObjectManager->ObjectListHeader[dwType].lpFirstObject = pObject;
		if(lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID < pObject->dwObjectID)
		{
			lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID = pObject->dwObjectID;
		}
		lpObjectManager->ObjectListHeader[dwType].dwObjectNum ++;
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}

	//If this object is not the first object of this type.
	pObject->lpNextObject = lpObjectManager->ObjectListHeader[dwType].lpFirstObject;
	pObject->lpNextObject->lpPrevObject = pObject;
	pObject->lpPrevObject = NULL;
	lpObjectManager->ObjectListHeader[dwType].lpFirstObject = pObject;
	if(lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID < pObject->dwObjectID)
	{
		lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID = pObject->dwObjectID;
	}
	lpObjectManager->ObjectListHeader[dwType].dwObjectNum ++;
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	return pObject;
}


//
// Get the base address of a object by it's ID.
// 
// Input:
//   @lpObjectManager    : The base address of Object Manager.
//   @dwObjectID         : The ID of the object whose base address will be got.
//
// Output:
//   The base address of the object,if failed,returns NULL.
//

static __COMMON_OBJECT* GetObjectByID(__OBJECT_MANAGER* lpObjectManager,
									  DWORD             dwObjectID)
{
	__COMMON_OBJECT*         lpObject     = NULL;
	DWORD                    dwLoop       = 0;
	__OBJECT_LIST_HEADER*    lpListHeader = NULL;
	BOOL                     bFind        = FALSE;

	if(NULL == lpObjectManager)  //Parameters check.
	{
		goto __TERMINAL;
	}

	for(dwLoop = 0;dwLoop < MAX_OBJECT_TYPE;dwLoop ++)    //For every object type.
	{
		lpListHeader = &(lpObjectManager->ObjectListHeader[dwLoop]);
		if(lpListHeader->dwMaxObjectID < dwObjectID)  //If the maximal ID smaller than dwObjectID.
		{
			continue;
		}

		lpObject = lpListHeader->lpFirstObject;
		while(lpObject)    //For every object in this type list.
		{
			if(lpObject->dwObjectID == dwObjectID)  //Now,find the correct object.
			{
				bFind = TRUE;
				break;
			}
			lpObject = lpObject->lpNextObject;      //Seek the next object.
		}
		if(TRUE == bFind)  //Find the correct object.
		{
			break;
		}
	}

	if(FALSE == bFind)    //If can not find the correct object,set the return value to NULL.
	{
		lpObject = NULL;
	}

__TERMINAL:
	return lpObject;
}


//
// Get the first object by type.Using this function,you can list all of the objects
// belongs to one type.
// 
// Input:
//   @lpObjectManager    : The base address of Object Manager.
//   @dwObjectType       : Object type.
//
// Output:
//   Returns the base address of the first object,if failed,returns NULL.
//
static __COMMON_OBJECT* GetFirstObjectByType(__OBJECT_MANAGER* lpObjectManager,
											 DWORD             dwObjectType)
{
	__COMMON_OBJECT* lpObject = NULL;

	if((NULL == lpObjectManager) || (dwObjectType >= MAX_OBJECT_TYPE))    //Parameter check.
	{
		goto __TERMINAL;
	}

	lpObject = lpObjectManager->ObjectListHeader[dwObjectType].lpFirstObject;

__TERMINAL:
	return lpObject;
}


//
// Destroy one object.
//
// Input:
//   @lpObjectManager    : The base address of Object Manager.
//   @lpObject           : The base address of the object will be destroyed.
//
// Output:
//   No.
//
static VOID DestroyObject(__OBJECT_MANAGER* lpObjectManager,
						  __COMMON_OBJECT*  lpObject)
{
	__OBJECT_LIST_HEADER*      lpListHeader      = NULL;
	__COMMON_OBJECT*           lpTmpObject       = NULL;
	DWORD                      dwMaxID           = 0;
	DWORD                      dwFlags;

	if((NULL == lpObjectManager) || (NULL == lpObject))  //Parameters check.
	{
		goto __TERMINAL;
	}

	if(lpObject->dwObjectType >= MAX_OBJECT_TYPE)  //If the type value exceed maximal value.
	{
		goto __TERMINAL;
	}

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	lpListHeader = &(lpObjectManager->ObjectListHeader[lpObject->dwObjectType]);
	if(NULL == lpListHeader->lpFirstObject)
	{
		__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
		goto __TERMINAL;
	}
	dwMaxID = lpObject->dwObjectID;  //Record the object's ID.

	if(NULL == lpObject->lpPrevObject)  //This is the first object of the current list.
	{
		if(NULL == lpObject->lpNextObject) //This is the last object.
		{
			lpListHeader->lpFirstObject = NULL;
			lpListHeader->dwObjectNum --;
		}
		else    //This is not the last object.
		{
			lpObject->lpNextObject->lpPrevObject = NULL;
			lpListHeader->lpFirstObject = lpObject->lpNextObject;
			lpListHeader->dwObjectNum --;
		}
	}
	else  //This is not the first object of the current list.
	{
		if(NULL == lpObject->lpNextObject)  //This is the last object.
		{
			lpObject->lpPrevObject->lpNextObject = NULL;
			lpListHeader->dwObjectNum --;
		}
		else  //This is not the last object.
		{
			lpObject->lpPrevObject->lpNextObject = lpObject->lpNextObject;
			lpObject->lpNextObject->lpPrevObject = lpObject->lpPrevObject;
			lpListHeader->dwObjectNum --;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	lpObject->Uninitialize(lpObject);    //Call the Uninitialize routine to de-initialize
	                                     //the current object.

	KMemFree((LPVOID)lpObject,KMEM_SIZE_TYPE_ANY,lpObject->dwObjectSize);  //Free the
	                                                                       //Object's memory.

	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(dwMaxID >= lpListHeader->dwMaxObjectID)  //Now,should update the current list's maximal
		                                        //object ID.
	{
		lpTmpObject = lpListHeader->lpFirstObject;
		dwMaxID = lpTmpObject ? lpTmpObject->dwObjectID : 0;
		while(lpTmpObject)
		{
			if(dwMaxID < lpTmpObject->dwObjectID)
			{
				dwMaxID = lpTmpObject->dwObjectID;
			}
			lpTmpObject = lpTmpObject->lpNextObject;
		}
	}
	lpListHeader->dwMaxObjectID = dwMaxID;  //Update the current list's maximal ID value.
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

__TERMINAL:
	return;
}
