//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 12,2019
//    Module Name               : system2.c
//    Module Funciton           : 
//                                Appended part of system.c source file,included
//                                by system.c file.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "system.h"
#include "types.h"
#include "../syscall/syscall.h"
#include "stdio.h"
#include "ktmsg.h"
#include "hellocn.h"
#include "mlayout.h"

/* Return system level information about this machine. */
static BOOL __GetSystemInfo(__SYSTEM_INFO* pSysInfo)
{
	BUG_ON(NULL == pSysInfo);

	/* Could not invoked in process of system initialization. */
	if (IN_SYSINITIALIZATION())
	{
		return FALSE;
	}

	/* Clear the sysinfo object. */
	memset(pSysInfo, 0, sizeof(__SYSTEM_INFO));

	/* Set it one by one. */
	pSysInfo->dwActiveProcessorMask = 0;
	pSysInfo->dwAllocationGranularity = ARCH_PAGE_FRAME_SIZE;
	pSysInfo->dwNumberOfProcessors = ProcessorManager.nProcessorNum;
	pSysInfo->dwPageSize = ARCH_PAGE_FRAME_SIZE;
	pSysInfo->dwProcessorType = 0;
	pSysInfo->lpMaximumApplicationAddress = (LPVOID)KMEM_USERSPACE_END;
	pSysInfo->lpMinimumApplicationAddress = (LPVOID)KMEM_USERSPACE_START;
	pSysInfo->wProcessorLevel = 3;
	return TRUE;
}
