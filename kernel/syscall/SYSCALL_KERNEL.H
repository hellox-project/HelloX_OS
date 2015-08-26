//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2009
//    Module Name               : SYSCALL.H
//    Module Funciton           : 
//                                System call definition code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/


#ifndef __SYSCALL_KERNEL_H__
#define __SYSCALL_KERNEL_H__

#include "K_API_DEF.H"
#include "types.h"
#include "iomgr.h"
#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif

//System call parameter block,used to transfer parameters between user mode
//and kernel mode.
//This structure reflects the stack content of current kernel thread.
typedef struct SYSCALL_PARAM_BLOCK{
	DWORD                         ebp;
	DWORD                         edi;
	DWORD                         esi;
	DWORD                         edx;
	DWORD                         ecx;
	DWORD                         ebx;
	DWORD                         eax;
	DWORD                         eip;
	DWORD                         cs;
	DWORD                         eflags;
	DWORD                         dwSyscallNum;
	LPVOID                        lpRetValue;
	LPVOID                        lpParams[1];
}__SYSCALL_PARAM_BLOCK;

//Dispatch entry in other kernel module,used to dispatch a system call
//to it's service routine.
typedef BOOL (*__SYSCALL_DISPATCH_ENTRY)(LPVOID,LPVOID);

//System call range object,to manage system call(s) reside in other
//kernel module.
typedef struct SYSCALL_RANGE
{
	DWORD      dwStartSyscallNum;
	DWORD      dwEndSyscallNum;
	__SYSCALL_DISPATCH_ENTRY sde;

}__SYSCALL_RANGE;

//How many syscal range object in the global array.Each syscall range
//will occupy one element of this array,so if there are many syscall range,
//please enlarge this value.
#define SYSCALL_RANGE_NUM 16

//A system call implemented in master module,which is used by other
//kernel module to register their system call range to master module.
BOOL RegisterSystemCall(DWORD dwStartNum,DWORD dwEndNum,__SYSCALL_DISPATCH_ENTRY sde);

typedef VOID (*SYSCALL_ENTRY)(__SYSCALL_PARAM_BLOCK*);


//Every system call should be handled by this routine.
BOOL SyscallHandler(LPVOID lpEsp,LPVOID);

VOID RegisterSysCallEntry();

#ifdef __cplusplus
}
#endif

#endif  //__SYSCALL_KERNEL_H__
