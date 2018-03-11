//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2009
//    Module Name               : SYSCALL.CPP
//    Module Funciton           : 
//                                System call implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "syscall.h"
#include "kapi.h"
#include "modmgr.h"
#include "stdio.h"
#include "devmgr.h"
#include "iomgr.h"

#ifdef __I386__
#include "../arch/x86/bios.h"
#endif

#define PARAM(i) (pspb->lpParams[i])

//A static array to contain system call range.
static __SYSCALL_RANGE SyscallRange[SYSCALL_RANGE_NUM] = {0};
static SYSCALL_ENTRY  s_szScCallArray[SYSCALL_MAX_COUNT] = {0};

//Register all call 
void RegisterKernelEntry(SYSCALL_ENTRY* pSysCallEntry);
void RegisterIoEntry(SYSCALL_ENTRY* pSysCallEntry);
void RegisterSocketEntry(SYSCALL_ENTRY* pSysCallEntry);

//System call used by other kernel module to register their system
//call range.
BOOL RegisterSystemCall(DWORD dwStartNum,DWORD dwEndNum,__SYSCALL_DISPATCH_ENTRY sde)
{
	int i = 0;

	if((dwStartNum > dwEndNum) || (NULL == sde))
	{
		return FALSE;
	}
	while(i < SYSCALL_RANGE_NUM)
	{
		if((0 == SyscallRange[i].dwStartSyscallNum) && ( 0 == SyscallRange[i].dwEndSyscallNum))
		{
			break;
		}
		i += 1;
	}

	if(SYSCALL_RANGE_NUM == i)  //Can not find a empty slot.
	{
		return FALSE;
	}

	//Add the requested range to system call range array.
	SyscallRange[i].dwStartSyscallNum = dwStartNum;
	SyscallRange[i].dwEndSyscallNum   = dwEndNum;
	SyscallRange[i].sde               = sde;

	return TRUE;
}

//If can not find a proper service routine in master kernel,then try to dispatch
//the system call to other module(s) by looking system call range array.
static BOOL DispatchToModule(LPVOID lpEsp,LPVOID lpParam)
{
	__SYSCALL_PARAM_BLOCK*  pspb = (__SYSCALL_PARAM_BLOCK*)lpEsp;
	int i = 0;

	for(i = 0;i < SYSCALL_RANGE_NUM;i ++)
	{
		if((SyscallRange[i].dwStartSyscallNum <= pspb->dwSyscallNum) &&
		   (SyscallRange[i].dwEndSyscallNum   >= pspb->dwSyscallNum))
		{
			SyscallRange[i].sde(lpEsp,NULL);
			return TRUE;
		}
	}
	return FALSE;  //Can not find a system call range to handle it.
}

//Register All System call entry point.
void  RegisterSysCallEntry()
{	
	//Register Kernel routines.
	RegisterKernelEntry(s_szScCallArray);
	RegisterIoEntry(s_szScCallArray);
	RegisterSocketEntry(s_szScCallArray);
}

//System call entry point.
BOOL SyscallHandler(LPVOID lpEsp,LPVOID lpParam)
{
	__SYSCALL_PARAM_BLOCK*  pspb     = (__SYSCALL_PARAM_BLOCK*)lpEsp;
	SYSCALL_ENTRY           pScEntry = NULL; 

	if (NULL == lpEsp)
	{
		return FALSE;
	}
	if(pspb->dwSyscallNum >= SYSCALL_MAX_COUNT)
	{
		__LOG("Invalid system call ID[call_id = %d].\r\n",
			pspb->dwSyscallNum);
		return FALSE;
	}
	
	//_hx_printf("call num=%X\r\n",pspb->dwSyscallNum);
	pScEntry = s_szScCallArray[pspb->dwSyscallNum];
	if(pScEntry)
	{
		pScEntry(pspb);
	}
	else if(!DispatchToModule(lpEsp,NULL))
	{
		_hx_printf("SyscallHandler: Unknown system call[num = 0x%0X] raised.\r\n",
			pspb->dwSyscallNum);
	}
	
	return TRUE;
}
