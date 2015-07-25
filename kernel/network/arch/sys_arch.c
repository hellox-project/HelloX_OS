//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : sys_arch.c
//    Module Funciton           : 
//                                OS simulation layer for lwIP stack.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "ktmgr.h"
#include "heap.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "arch/cc.h"
#include "arch/sys_arch.h"

//Light protection mechanism used by lwIP,it's critical section actually
//in HelloX.
sys_prot_t sys_arch_protect(void)
{
	DWORD  dwFlags;
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	return dwFlags;
}

void sys_arch_unprotect(sys_prot_t val)
{
	__LEAVE_CRITICAL_SECTION(NULL,val);
}

//Create a new thread.
sys_thread_t sys_thread_new(const char* name,void (*thread)(void* arg),void* arg,int stacksize,int prio)
{
	return KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		prio,
		(__KERNEL_THREAD_ROUTINE)thread,  //Force convert to HelloX defined thread routine.
		arg,
		NULL,
		(char*)name);
}

//Check if mailbox is valid.
int sys_mbox_valid(sys_mbox_t* mbox)
{
	return (*mbox) ? 1 : 0;
}

//Set mailbox object to invalid.
void sys_mbox_set_invalid(sys_mbox_t* mbox)
{
	if(mbox)
	{
		*mbox = NULL;
	}
}

//Try to fetch one message from mailbox.
u32_t sys_arch_mbox_tryfetch(sys_mbox_t* mbox,void** msg)
{
	__MAIL_BOX*     pMailbox  = (__MAIL_BOX*)*mbox;
	DWORD           dwRetVal  = OBJECT_WAIT_FAILED;

	dwRetVal = pMailbox->GetMail((__COMMON_OBJECT*)pMailbox,msg,0,NULL);
	if(OBJECT_WAIT_RESOURCE == dwRetVal)  //Get mail successfully.
	{
		return 0;
	}
	return SYS_MBOX_EMPTY;
}

//Fetch a mail from mailbox,in a specific time.
u32_t sys_arch_mbox_fetch(sys_mbox_t* mbox,void** msg,u32_t timeout)
{
	__MAIL_BOX*    pMailbox  = (__MAIL_BOX*)*mbox;
	DWORD          dwWait    = 0;
	DWORD          dwRetVal  = OBJECT_WAIT_FAILED;
	DWORD          dwTimeOut = timeout ? timeout : WAIT_TIME_INFINITE;

	dwRetVal = pMailbox->GetMail((__COMMON_OBJECT*)pMailbox,msg,dwTimeOut,&dwWait);
	if(OBJECT_WAIT_RESOURCE == dwRetVal)
	{
		return dwWait;
	}
	if(OBJECT_WAIT_TIMEOUT == dwRetVal)
	{
		return SYS_ARCH_TIMEOUT;
	}
	return 0;
}

//Try to send a mail into mailbox.
err_t sys_mbox_trypost(sys_mbox_t* mbox,void* msg)
{
	__MAIL_BOX*    pMailbox    = (__MAIL_BOX*)*mbox;
	DWORD          dwRetVal    = OBJECT_WAIT_FAILED;

	dwRetVal = pMailbox->SendMail((__COMMON_OBJECT*)pMailbox,msg,0,0,NULL);
	if(OBJECT_WAIT_RESOURCE == dwRetVal)
	{
		return ERR_OK;
	}
	if(OBJECT_WAIT_TIMEOUT == dwRetVal)
	{
		return ERR_MEM;
	}
	return 0;
}

//Post a message into mailbox.
void sys_mbox_post(sys_mbox_t* mbox,void* msg)
{
	__MAIL_BOX* pMailbox = (__MAIL_BOX*)*mbox;

	pMailbox->SendMail((__COMMON_OBJECT*)pMailbox,msg,0,WAIT_TIME_INFINITE,NULL);
}

//Release a mailbox object.
void sys_mbox_free(sys_mbox_t* mbox)
{
	ObjectManager.DestroyObject(&ObjectManager,
		(__COMMON_OBJECT*)*mbox);
}

//Create a new mailbox object.
err_t sys_mbox_new(sys_mbox_t* mbox,int size)
{
	__COMMON_OBJECT*   pMailbox = NULL;

	pMailbox = ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_MAILBOX);
	if(NULL == pMailbox)
	{
		return 0;
	}
	if(!pMailbox->Initialize(pMailbox))
	{
		ObjectManager.DestroyObject(&ObjectManager,pMailbox);  //Destroy it.
		return 0;
	}

	//Set mailbox size accordingly.
	if(0 == size)
	{
		size = 4;
	}
	if(!((__MAIL_BOX*)pMailbox)->SetMailboxSize(pMailbox,size))
	{
		ObjectManager.DestroyObject(&ObjectManager,pMailbox);
		return 0;
	}

	//Return the new created mailbox object.
	*mbox = pMailbox;
	return ERR_OK;
}

//Check if a semaphore object is valid.
int sys_sem_valid(sys_sem_t* sem)
{
	return (*sem) ? 1 : 0;
}

//Set a semaphore object to invalid.
void sys_sem_set_invalid(sys_sem_t* sem)
{
	if(sem)
	{
		*sem = NULL;
	}
}

//Wait a semaphore object in a given period of time.
u32_t sys_arch_sem_wait(sys_sem_t* sem,u32_t timeout)
{
	__SEMAPHORE*   pSem     = (__SEMAPHORE*)*sem;
	DWORD          dwRetVal = OBJECT_WAIT_FAILED;
	DWORD          dwWait   = 0;

	if(0 == timeout)  //Wait forever until semaphore is signal.
	{
		dwRetVal = pSem->WaitForThisObject((__COMMON_OBJECT*)pSem);
	}
	else  //Wait for a period of time.
	{
		dwRetVal = pSem->WaitForThisObjectEx((__COMMON_OBJECT*)pSem,timeout,&dwWait);
	}
	switch(dwRetVal)
	{
	case OBJECT_WAIT_TIMEOUT:
		return SYS_ARCH_TIMEOUT;
	case OBJECT_WAIT_RESOURCE:
		return dwWait;
	default:
		return 0;
	}
}

//Set a semaphore object's status to signal.
void sys_sem_signal(sys_sem_t* sem)
{
	__SEMAPHORE* pSem = (__SEMAPHORE*)*sem;
	pSem->ReleaseSemaphore((__COMMON_OBJECT*)pSem,NULL);
}

//Destroy a semaphore object.
void sys_sem_free(sys_sem_t* sem)
{
	ObjectManager.DestroyObject(&ObjectManager,*sem);
}

//Create a new semaphore object and set it's initial status.
err_t sys_sem_new(sys_sem_t* sem,u8_t count)
{
	__SEMAPHORE*  pSem = (__SEMAPHORE*)ObjectManager.CreateObject(&ObjectManager,
		NULL,
		OBJECT_TYPE_SEMAPHORE);
	if(NULL == pSem)
	{
		return !ERR_OK;
	}
	if(!pSem->Initialize((__COMMON_OBJECT*)pSem))
	{
		ObjectManager.DestroyObject(&ObjectManager,(__COMMON_OBJECT*)pSem);
		return !ERR_OK;
	}
	//Change initial status according count.
	if(0 == count)
	{
		pSem->SetSemaphoreCount((__COMMON_OBJECT*)pSem,1,0);
	}
	*sem = (__COMMON_OBJECT*)pSem;  //Return semaphore object.
	return ERR_OK;
}

//Get system tick counter.
u32_t sys_now(void)
{
	return System.GetClockTickCounter((__COMMON_OBJECT*)&System);
}

//An empty sys_init routine to fit lwIP's requirement.
void sys_init()
{
	return;
}

//Net protocol intialize routine,it's called in process of OS initialization
//to initialize the network protocol.
//Each network protocol should offer this routine to OS.
BOOL IPv4_Entry(VOID* pArg)
{
	tcpip_init(NULL,NULL);
	return TRUE;
}
