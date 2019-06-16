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

#include <StdAfx.h>
#include <types.h>
#include <ktmgr.h>
#include <process.h>

/*
 * The following array is used by Object Manager to create object.
 * Once a new object type is defined,you must add one line in the
 * following array using OBJECT_INIT_DATA,it's parameters meaning,
 * please refer commobj.h.
 */
BEGIN_DECLARE_INIT_DATA(ObjectInitData)
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

#ifdef __CFG_SYS_DDF
	/* Define the following objects only DDF enabled. */
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
	/* Objects related to virtual memory mechanism. */
	OBJECT_INIT_DATA(OBJECT_TYPE_PAGE_INDEX_MANAGER,sizeof(__PAGE_INDEX_MANAGER),
	PageInitialize,PageUninitialize)
	OBJECT_INIT_DATA(OBJECT_TYPE_VIRTUAL_MEMORY_MANAGER,sizeof(__VIRTUAL_MEMORY_MANAGER),
	VmmInitialize,VmmUninitialize)
#endif

	OBJECT_INIT_DATA(OBJECT_TYPE_SEMAPHORE,sizeof(__SEMAPHORE),
	SemInitialize,SemUninitialize)
	OBJECT_INIT_DATA(OBJECT_TYPE_CONDITION,sizeof(__CONDITION),
	ConditionInitialize,ConditionUninitialize)
	OBJECT_INIT_DATA(OBJECT_TYPE_PROCESS,sizeof(__PROCESS_OBJECT),
	ProcessInitialize,ProcessUninitialize)
END_DECLARE_INIT_DATA()

/*
 *  Create a object by Object Manager.
 * 
 *  Input:
 *    @lpObjectManager    : The base address of Object Manager.
 *    @lpObjectOwner      : The parent of the object will be created.
 *    @dwType             : Which type of object will be created.
 * 
 *  Output:
 *    If successfully,returns the base address of new created object,
 *    otherwise,returns NULL.
 */
static __COMMON_OBJECT* CreateObject(__OBJECT_MANAGER* lpObjectManager,
	__COMMON_OBJECT* lpObjectOwner,
	DWORD dwType)
{
	__COMMON_OBJECT* pObject = NULL;
	DWORD dwLoop = 0;
	BOOL bFind = FALSE;
	DWORD dwObjectSize = 0;
	unsigned long dwFlags;

	if((NULL == lpObjectManager) || (dwType >= MAX_KERNEL_OBJECT_TYPE))
	{
		goto __TERMINAL;
	}

	/* Find the initialize data of this type object. */
	while(TRUE)
	{
		if(MAX_KERNEL_OBJECT_TYPE == dwLoop)
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
	
	if(FALSE == bFind)
	{
		/* Give up if can not find the initialize data. */
		goto __TERMINAL;
	}

	dwObjectSize = ObjectInitData[dwLoop].dwObjectSize;
	if(0 == dwObjectSize)
	{
		/* Invalid object size. */
		goto __TERMINAL;
	}

	pObject = (__COMMON_OBJECT*)KMemAlloc(dwObjectSize,KMEM_SIZE_TYPE_ANY);
	if(NULL == pObject)
	{
		goto __TERMINAL;
	}
	/* Initializes to all zero. */
	memset(pObject, 0, dwObjectSize);

	/* Initialize the new created object. */
	__ENTER_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);
	pObject->dwObjectID = lpObjectManager->dwCurrentObjectID;
	lpObjectManager->dwCurrentObjectID ++;     //Now,update the Object Manager's status.

	pObject->dwObjectSize = dwObjectSize;
	pObject->dwObjectType = dwType;
	pObject->Initialize = ObjectInitData[dwLoop].Initialize;
	pObject->Uninitialize = ObjectInitData[dwLoop].Uninitialize;
	pObject->lpObjectOwner = lpObjectOwner;
	/* Set the reference counter as 1. */
	pObject->refCount = 1;

	/* Insert the new created object into ObjectArrayList. */
	if(NULL == lpObjectManager->ObjectListHeader[dwType].lpFirstObject)
	{
		pObject->lpNextObject = NULL;
		pObject->lpPrevObject = NULL;
		lpObjectManager->ObjectListHeader[dwType].lpFirstObject = pObject;
		if(lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID < pObject->dwObjectID)
		{
			lpObjectManager->ObjectListHeader[dwType].dwMaxObjectID = pObject->dwObjectID;
		}
		lpObjectManager->ObjectListHeader[dwType].dwObjectNum ++;
		__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);
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
	__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock, dwFlags);

__TERMINAL:
	return pObject;
}

/*
* A helper routine to validate if a kernel object is valid,i.e,
* the object is in kernel object list and is not destroyed yet.
* This routine must be called in critical section since it does
* not apply spin lock protection.
*/
static BOOL __ValidateObject(__OBJECT_MANAGER* pObjectMgr, __COMMON_OBJECT* pObject)
{
	__OBJECT_LIST_HEADER* pListHdr = NULL;
	__COMMON_OBJECT* pResult = NULL;

	BUG_ON((NULL == pObjectMgr) || (NULL == pObject));
	BUG_ON(pObject->dwObjectType >= MAX_KERNEL_OBJECT_TYPE);
	pListHdr = &pObjectMgr->ObjectListHeader[pObject->dwObjectType];
	pResult = pListHdr->lpFirstObject;
	while (pResult)
	{
		if (pObject == pResult)
		{
			break;
		}
		pResult = pResult->lpNextObject;
	}
	return (NULL == pResult) ? FALSE : TRUE;
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
	DWORD dwObjectID)
{
	__COMMON_OBJECT*         lpObject     = NULL;
	DWORD                    dwLoop       = 0;
	__OBJECT_LIST_HEADER*    lpListHeader = NULL;
	BOOL                     bFind        = FALSE;

	if(NULL == lpObjectManager)
	{
		goto __TERMINAL;
	}

	/* For every object type. */
	for(dwLoop = 0;dwLoop < MAX_KERNEL_OBJECT_TYPE;dwLoop ++)
	{
		lpListHeader = &(lpObjectManager->ObjectListHeader[dwLoop]);
		/* If the maximal ID smaller than dwObjectID. */
		if(lpListHeader->dwMaxObjectID < dwObjectID)
		{
			continue;
		}

		lpObject = lpListHeader->lpFirstObject;
		/* For every object in this type list. */
		while(lpObject)
		{
			if(lpObject->dwObjectID == dwObjectID)
			{
				bFind = TRUE;
				break;
			}
			lpObject = lpObject->lpNextObject;
		}
		if(TRUE == bFind)
		{
			break;
		}
	}

	/* If can not find the correct object,set the return value to NULL. */
	if(FALSE == bFind)
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

	if((NULL == lpObjectManager) || (dwObjectType >= MAX_KERNEL_OBJECT_TYPE))
	{
		goto __TERMINAL;
	}

	lpObject = lpObjectManager->ObjectListHeader[dwObjectType].lpFirstObject;

__TERMINAL:
	return lpObject;
}

/*
 * Destroy a kernel object.The object must be created by
 * Object Manager.
 * It decrease the object's reference counter,delete it
 * from object list and destroy it,if the refer counter
 * reaches 0.Just return otherwise(refer counter is not
 * 0 after decrease).
 * Following steps will be applied to destroy one kernel object:
 *   1. Unlink the object from common object list;
 *   2. Call Uninitialize routine of the kernel object;
 *   3. If the Uninitialize routine returns TRUE,then release
 *      memory resources by KMemFree;
 *   4. If the Uninitialize routine returns FALSE,then the
 *      object is undestroyable,so relink it into common
 *      object list,so another try of DestroyObject can be
 *      invoked later.
 * Maximal object ID value will be updated in case of 3. 
 * TRUE will be returned if the kernel object is destroyed
 * and FALSE will return in other cases.
 */
static BOOL __DestroyObject(__OBJECT_MANAGER* lpObjectManager,
	__COMMON_OBJECT* lpObject)
{
	__OBJECT_LIST_HEADER* lpListHeader = NULL;
	__COMMON_OBJECT* lpTmpObject = NULL;
	DWORD dwMaxID = 0;
	unsigned long dwFlags;
	unsigned long dwType = 0;
	BOOL bResult = FALSE;

	/* Parameters check. */
	BUG_ON((NULL == lpObjectManager) || (NULL == lpObject));
	BUG_ON(lpObject->dwObjectType >= MAX_KERNEL_OBJECT_TYPE);

	__ENTER_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);
	/* If the object is not a valid kernel object. */
	BUG_ON(!__ValidateObject(lpObjectManager, lpObject));
	/* Decrease the refer counter of this object. */
	__ATOMIC_DECREASE(&lpObject->refCount);
	if (lpObject->refCount)
	{
		/* Reference counter is not 0. */
		__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock, dwFlags);
		goto __TERMINAL;
	}
	lpListHeader = &(lpObjectManager->ObjectListHeader[lpObject->dwObjectType]);
	if(NULL == lpListHeader->lpFirstObject)
	{
		__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);
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
	__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);

	/* 
	 * Call the Uninitialize routine to uninitialize the current object.
	 * If this routine returns FALSE,then it means the kernel object is
	 * unabled be destroyed,so we must relink it into common object list.
	 * Later try of DestroyObject invoking may destroy it.
	 * Reference counter must be reset to initial value(1) before relink.
	 */
	if (!lpObject->Uninitialize(lpObject))
	{
		__ENTER_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock, dwFlags);
		/* Set the reference counter as 1. */
		lpObject->refCount = 1;
		dwType = lpObject->dwObjectType;

		/* Insert the new created object into ObjectArrayList. */
		if (NULL == lpObjectManager->ObjectListHeader[dwType].lpFirstObject)
		{
			lpObject->lpNextObject = NULL;
			lpObject->lpPrevObject = NULL;
			lpObjectManager->ObjectListHeader[dwType].lpFirstObject = lpObject;
			lpObjectManager->ObjectListHeader[dwType].dwObjectNum++;
			__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock, dwFlags);
			goto __TERMINAL;
		}

		//If this object is not the first object of this type.
		lpObject->lpNextObject = lpObjectManager->ObjectListHeader[dwType].lpFirstObject;
		lpObject->lpNextObject->lpPrevObject = lpObject;
		lpObject->lpPrevObject = NULL;
		lpObjectManager->ObjectListHeader[dwType].lpFirstObject = lpObject;
		lpObjectManager->ObjectListHeader[dwType].dwObjectNum++;
		__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock, dwFlags);
		goto __TERMINAL;
	}

	/* Release the object. */
	KMemFree((LPVOID)lpObject,KMEM_SIZE_TYPE_ANY,lpObject->dwObjectSize);

	__ENTER_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);
	/* Update maximal object ID. */
	if(dwMaxID >= lpListHeader->dwMaxObjectID)
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
	/* Update the current list's maximal ID value. */
	lpListHeader->dwMaxObjectID = dwMaxID;
	__LEAVE_CRITICAL_SECTION_SMP(lpObjectManager->spin_lock,dwFlags);

	/* The specified object is destroyed. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Initializer of the Object Manager. 
 * This routine will be invoked in process of system
 * initialization.
 */
static BOOL __Initialize(__OBJECT_MANAGER* pObjectMgr)
{
	BUG_ON(NULL == pObjectMgr);

	pObjectMgr->dwCurrentObjectID = 1;
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pObjectMgr->spin_lock,"objmgr");
#endif

	return TRUE;
}

/* Get a kernel object by specifying it's type and ID value. */
static __COMMON_OBJECT* __GetObject(__OBJECT_MANAGER* pObjectMgr,
	unsigned long objtype,
	unsigned long objid)
{
	__OBJECT_LIST_HEADER* pListHdr = NULL;
	unsigned long ulFlags = 0;
	__COMMON_OBJECT* pResult = NULL;

	BUG_ON((NULL == pObjectMgr) || (objtype >= MAX_KERNEL_OBJECT_TYPE));

	/* Can not interrupted in searching. */
	__ENTER_CRITICAL_SECTION_SMP(pObjectMgr->spin_lock, ulFlags);
	pListHdr = &pObjectMgr->ObjectListHeader[objtype];
	pResult = pListHdr->lpFirstObject;
	while (pResult)
	{
		if (objid == pResult->dwObjectID)
		{
			/* Found. */
			break;
		}
		pResult = pResult->lpNextObject;
	}
	__LEAVE_CRITICAL_SECTION_SMP(pObjectMgr->spin_lock, ulFlags);

	return pResult;
}

/* 
 * Increase kernel object's reference counter. 
 * Return TRUE if the reference counter is increased
 * and the caller can use the object safety.
 */
static BOOL __AddRefCount(__OBJECT_MANAGER* pObjectMgr, __COMMON_OBJECT* pObject)
{
	unsigned long ulFlags;
	BOOL bResult = FALSE;

	BUG_ON((NULL == pObjectMgr) || (NULL == pObject));
	BUG_ON(pObject->dwObjectType >= MAX_KERNEL_OBJECT_TYPE);
	__ENTER_CRITICAL_SECTION_SMP(pObjectMgr->spin_lock, ulFlags);
	bResult = __ValidateObject(pObjectMgr, pObject);
	if (!bResult)
	{
		/* The object maybe destroyed already. */
		__LEAVE_CRITICAL_SECTION_SMP(pObjectMgr->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	__ATOMIC_INCREASE(&pObject->refCount);
	__LEAVE_CRITICAL_SECTION_SMP(pObjectMgr->spin_lock, ulFlags);
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Object Manager,all kernel objects are managed by this object. */
__OBJECT_MANAGER ObjectManager = {
	1,									//Initial object ID value.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,
#endif
	{ 0 },

	/* Member routines. */
	__Initialize,                       //Initialize routine.
	CreateObject,                       //CreateObject routine.
	__GetObject,                        //GetObject routine.
	GetObjectByID,                      //GetObjectByID routine.
	GetFirstObjectByType,               //GetFirstObjectByType routine.
	__DestroyObject,                    //DestroyObject routine.
	__AddRefCount,                      //AddRefCount routine.
};
