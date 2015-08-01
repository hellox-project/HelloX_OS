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

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

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
typedef struct SYSCALL_RANGE{
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
BOOL RegisterSystemCall(DWORD dwStartNum,DWORD dwEndNum,
						__SYSCALL_DISPATCH_ENTRY sde);

//System call numbers.The dispatch routine(SyscallHandler) will dispatch
//each system call to specific service routine by using this value.
#define SYSCALL_CREATEKERNELTHREAD    0x01     //CreateKernelThread.
#define SYSCALL_DESTROYKERNELTHREAD   0x02     //DestroyKernelThread.
#define SYSCALL_SETLASTERROR          0x03     //SetLastError.
#define SYSCALL_GETLASTERROR          0x04     //GetLastError.
#define SYSCALL_GETTHREADID           0x05     //GetThreadID.
#define SYSCALL_SETTHREADPRIORITY     0x06     //SetThreadPriority.
#define SYSCALL_GETMESSAGE            0x07     //GetMessage.
#define SYSCALL_SENDMESSAGE           0x08     //SendMessage.
#define SYSCALL_SLEEP                 0x09     //Sleep.
#define SYSCALL_SETTIMER              0x0A     //SetTimer.
#define SYSCALL_CANCELTIMER           0x0B     //CancelTimer.
#define SYSCALL_CREATEEVENT           0x0C     //CreateEvent.
#define SYSCALL_DESTROYEVENT          0x0D     //DestroyEvent.
#define SYSCALL_SETEVENT              0x0E     //SetEvent.
#define SYSCALL_RESETEVENT            0x0F     //ResetEvent.
#define SYSCALL_CREATEMUTEX           0x10     //CreateMutex.
#define SYSCALL_DESTROYMUTEX          0x11     //DestroyMutex.
#define SYSCALL_RELEASEMUTEX          0x12     //ReleaseMutex.
#define SYSCALL_WAITFORTHISOBJECT     0x13     //WaitForThisObject.
#define SYSCALL_WAITFORTHISOBJECTEX   0x14     //WaitForThisObjectEx.
#define SYSCALL_CONNECTINTERRUPT      0x15     //ConnectInterrupt.
#define SYSCALL_DISCONNECTINTERRUPT   0x16     //DisconnectInterrupt.
#define SYSCALL_VIRTUALALLOC          0x17     //VirtualAlloc.
#define SYSCALL_VIRTUALFREE           0x18     //VirtualFree.
#define SYSCALL_CREATEFILE            0x19     //CreateFile.
#define SYSCALL_READFILE              0x1A     //ReadFile.
#define SYSCALL_WRITEFILE             0x1B     //WriteFile.
#define SYSCALL_CLOSEFILE             0x1C     //CloseFile.
#define SYSCALL_CREATEDIRECTORY       0x1D     //CreateDirectory.
#define SYSCALL_DELETEFILE            0x1E     //DeleteFile.
#define SYSCALL_FINDFIRSTFILE         0x1F     //FindFirstFile.
#define SYSCALL_FINDNEXTFILE          0x20     //FindNextFile.
#define SYSCALL_FINDCLOSE             0x21     //FindClose.
#define SYSCALL_GETFILEATTRIBUTES     0x22     //GetFileAttributes.
#define SYSCALL_GETFILESIZE           0x23     //GetFileSize.
#define SYSCALL_REMOVEDIRECTORY       0x24     //RemoveDirectory.
#define SYSCALL_SETENDOFFILE          0x25     //SetEndOfFile.
#define SYSCALL_IOCONTROL             0x26     //IOControl.
#define SYSCALL_SETFILEPOINTER        0x27     //SetFilePointer.
#define SYSCALL_FLUSHFILEBUFFERS      0x28     //FlushFileBuffers.
#define SYSCALL_CREATEDEVICE          0x29     //CreateDevice.
#define SYSCALL_DESTROYDEVICE         0x2A     //DestroyDevice.
#define SYSCALL_KMEMALLOC             0x2B     //KMemAlloc.
#define SYSCALL_KMEMFREE              0x2C     //KMemFree.
#define SYSCALL_PRINTLINE             0x2D     //PrintLine.
#define SYSCALL_PRINTCHAR             0x2E     //PrintChar.
#define SYSCALL_REGISTERSYSTEMCALL    0x2F     //RegisterSystemCall.
#define SYSCALL_REPLACESHELL          0x30     //ReplaceShell.
#define SYSCALL_LOADDRIVER            0x31     //LoadDriver.
#define SYSCALL_GETCURRENTTHREAD      0x32     //GetCurrentThread.
#define SYSCALL_GETDEVICE             0x33     //GetDevice.
#define SYSCALL_SWITCHTOGRAPHIC       0x34     //SwitchToGraphic.
#define SYSCALL_SWITCHTOTEXT          0x35     //SwitchToText.
#define SYSCALL_SETFOCUSTHREAD        0x36     //SetFocusThread.

//Every system call should be handled by this routine.
BOOL SyscallHandler(LPVOID lpEsp,LPVOID);

#ifdef __cplusplus
}
#endif

#endif  //__SYSCALL_H__
