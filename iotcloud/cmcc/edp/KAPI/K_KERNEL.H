//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.H
//    Module Funciton           : 
//                                Declares all kernel service routines can be
//                                used by other modules in kernel.
//                                This file is used by user application,so all
//                                routines declared in it is system calls.
//
//    Last modified Author      : Garry
//    Last modified Date        : Jan 09,2012
//    Last modified Content     :
//                                1. System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "KAPI.H"

#ifndef __K_KERNEL_H__
#define __K_KERNEL_H__

#ifdef __cplusplus
extern "C"
{
#endif

//Structures.
typedef struct _MSG
{
	WORD    wCommand;
	WORD    wParam;
	DWORD   dwParam;
}MSG;


typedef DWORD (*__KERNEL_THREAD_ROUTINE)(LPVOID);            //Kernel thread's start routine.
typedef DWORD    (*__DIRECT_TIMER_HANDLER)(LPVOID);          //Timer handler's protype.
typedef BOOL (*__INTERRUPT_HANDLER)(LPVOID lpEsp,LPVOID);    //Interrupt handler's pro-type.

//Flags used to control CreateKernelThread's action.
#define KERNEL_THREAD_STATUS_RUNNING    0x00000001
#define KERNEL_THREAD_STATUS_READY      0x00000002
#define KERNEL_THREAD_STATUS_SUSPENDED  0x00000003
#define KERNEL_THREAD_STATUS_SLEEPING   0x00000004
#define KERNEL_THREAD_STATUS_TERMINAL   0x00000005
#define KERNEL_THREAD_STATUS_BLOCKED    0x00000006

#define PRIORITY_LEVEL_CRITICAL         0x00000010
#define PRIORITY_LEVEL_HIGH             0x0000000C
#define PRIORITY_LEVEL_NORMAL           0x00000008
#define PRIORITY_LEVEL_LOW              0x00000004
#define PRIORITY_LEVEL_IDLE             0x00000000

HANDLE CreateKernelThread(DWORD dwStackSize,
						  DWORD dwInitStatus,
						  DWORD dwPriority,
						  __KERNEL_THREAD_ROUTINE lpStartRoutine,
						  LPVOID lpRoutineParam,
						  LPVOID lpReserved,
						  LPSTR  lpszName);

VOID DestroyKernelThread(HANDLE hThread);
DWORD SetLastError(DWORD dwNewError);
DWORD GetLastError();
DWORD GetThreadID(HANDLE hThread);
DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority);

//Flags for GetMessage,especially used by wCommand member of
//MSG structure.
#define KERNEL_MESSAGE_AKDOWN         1       //ASCII key down.
#define KERNEL_MESSAGE_AKUP           2       //ASCII key up.
#define KERNEL_MESSAGE_VKDOWN         203
#define KERNEL_MESSAGE_VKUP           204
#define KERNEL_MESSAGE_TERMINAL       5
#define KERNEL_MESSAGE_TIMER          6
#define KERNEL_MESSAGE_LBUTTONDOWN    301      //Left button down.
#define KERNEL_MESSAGE_LBUTTONUP      302      //Left button released.
#define KERNEL_MESSAGE_RBUTTONDOWN    303      //Right button down.
#define KERNEL_MESSAGE_RBUTTONUP      304      //Right button released.
#define KERNEL_MESSAGE_LBUTTONDBCLK   305      //Left button double clicked.
#define KERNEL_MESSAGE_RBUTTONDBCLK   306      //Right button double clicked.
#define KERNEL_MESSAGE_MOUSEMOVE      307      //Mouse is moving.
#define KERNEL_MESSAGE_WINDOW         308      //Window message,defined for GUI module.
#define KERNEL_MESSAGE_DIALOG         309      //Kernel message for dialog processing.

//Message command defination,please note the message
//value from 0x0001 to 0x00FF are reserved by kernel,
//user customized message can start from 0x0100.
#define MSG_KEY_DOWN            0x0001
#define MSG_KEY_UP              0x0002
#define MSG_LEFT_MOUSE_UP       0x0003
#define MSG_MIDD_MOUSE_UP       0x0004
#define MSG_RIGHT_MOUSE_UP      0x0005
#define MSG_LEFT_MOUSE_DOWN     0x0006
#define MSG_MIDD_MOUSE_DOWN     0x0007
#define MSG_RIGHT_MOUSE_DOWN    0x0008

#define MSG_SYS_TERMINAL        0x0009  //If OS want to task terminal,
//it sends this message to the task.
#define MSG_TIMER               0x000a  //Time message.

#define MSG_VK_KEY_DOWN         0x0010  //
#define MSG_VK_KEY_UP           0x0011  //

BOOL GetMessage(MSG* lpMsg);
BOOL SendMessage(HANDLE hThread,MSG* lpMsg);
BOOL Sleep(DWORD dwMillionSecond);

//Flags to control the SetTimer's action.
#define TIMER_FLAGS_ONCE        0x00000001    //Set a timer with this flags,the timer only
                                              //apply once,i.e,the kernel thread who set
											  //the timer can receive timer message only
											  //once.
#define TIMER_FLAGS_ALWAYS      0x00000002    //Set a timer with this flags,the timer will
											  //availiable always,only if the kernel thread
											  //cancel the timer by calling CancelTimer.
HANDLE SetTimer(DWORD dwTimerID,
				DWORD dwMillionSecond,
				__DIRECT_TIMER_HANDLER lpHandler,
				LPVOID lpHandlerParam,
				DWORD dwTimerFlags);

VOID CancelTimer(HANDLE hTimer);
HANDLE CreateEvent(BOOL bInitialStatus);
VOID DestroyEvent(HANDLE hEvent);
DWORD SetEvent(HANDLE hEvent);
DWORD ResetEvent(HANDLE hEvent);
HANDLE CreateMutex();
VOID DestroyMutex(HANDLE hMutex);
DWORD ReleaseMutex(HANDLE hEvent);
DWORD WaitForThisObject(HANDLE hObject);
DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond);
HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
						LPVOID              lpHandlerParam,
						UCHAR               ucVector);
VOID DisconnectInterrupt(HANDLE hInterrupt);

//
//Flags to control the action of VirtualAlloc.
//
//Virtual area's access flags.
//
#define VIRTUAL_AREA_ACCESS_READ        0x00000001    //Read access.
#define VIRTUAL_AREA_ACCESS_WRITE       0x00000002    //Write access.
#define VIRTUAL_AREA_ACCESS_RW          (VIRTUAL_AREA_ACCESS_READ | VIRTUAL_AREA_ACCESS_WRITE)
#define VIRTUAL_AREA_ACCESS_EXEC        0x00000008    //Execute only.
#define VIRTUAL_AREA_ACCESS_NOACCESS    0x00000010    //Access is denied.

//
//Virtual area's cache flags.
//
#define VIRTUAL_AREA_CACHE_NORMAL       0x00000001    //Cachable.
#define VIRTUAL_AREA_CACHE_IO           0x00000002    //Not cached.
#define VIRTUAL_AREA_CACHE_VIDEO        0x00000004

//
//Virtual area's allocate flags.
//
#define VIRTUAL_AREA_ALLOCATE_RESERVE   0x00000001    //Reserved only.
#define VIRTUAL_AREA_ALLOCATE_COMMIT    0x00000002    //Has been committed.
#define VIRTUAL_AREA_ALLOCATE_IO        0x00000004    //Allocated as IO mapping area.
#define VIRTUAL_AREA_ALLOCATE_ALL       0x00000008    //Committed.Only be used by VirtualAlloc
                                                      //routine,the dwAllocFlags variable of
													  //virtual area descriptor never set this
													  //value.
#define VIRTUAL_AREA_ALLOCATE_DEFAULT   VIRTUAL_AREA_ALLOCATE_ALL

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
					DWORD  dwSize,
					DWORD  dwAllocateFlags,
					DWORD  dwAccessFlags,
					CHAR*  lpszRegName);

VOID VirtualFree(LPVOID lpVirtualAddr);

//Flags used to control CreateFile's action.
#define FILE_ACCESS_READ         0x00000001    //Read access.
#define FILE_ACCESS_WRITE        0x00000002    //Write access.
#define FILE_ACCESS_READWRITE    0x00000003    //Read and write access.
#define FILE_ACCESS_CREATE       0x00000004    //Create a new file.

#define FILE_OPEN_ALWAYS         0x80000000    //If can not open one,create a new one then open it.
#define FILE_OPEN_NEW            0x40000000    //Create a new file,overwrite existing if.
#define FILE_OPEN_EXISTING       0x20000000    //Open a existing file,return fail if does not exist.

//Kernel memory allocation flags.
#define KMEM_SIZE_TYPE_ANY       0x00000001
#define KMEM_SIZE_TYPE_1K        0x00000002
#define KMEM_SIZE_TYPE_4K        0x00000003

LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType);
VOID   KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength);
VOID   KMemCpy(VOID* pDst,VOID* pSrc,INT nSize);

#define  hx_malloc(s)     KMemAlloc(s,KMEM_SIZE_TYPE_ANY)
#define  hx_free(o)       KMemFree(o,KMEM_SIZE_TYPE_ANY,0)
#define  hx_memcpy(d,s,l) KMemCpy(d,s,l);

typedef BOOL (*__SYSCALL_DISPATCH_ENTRY)(LPVOID,LPVOID);

BOOL   RegisterSystemCall(DWORD dwStartSyscallNum,DWORD dwEndSyscallNum,	__SYSCALL_DISPATCH_ENTRY sde);
BOOL   ReplaceShell(__KERNEL_THREAD_ROUTINE shell);

typedef BOOL (*__DRIVER_ENTRY)(HANDLE hDriverObject);
BOOL   LoadDriver(__DRIVER_ENTRY de);

HANDLE GetCurrentThread();
HANDLE SetFocusThread(HANDLE hNewThread);

BOOL   SwitchToGraphic();
VOID   SwitchToText();

BOOL   GetEthernetInterfaceState(__ETH_INTERFACE_STATE* pState, int nIndex, int* pnNextInt);

//Critical section operations.
#define __ENTER_CRITICAL_SECTION(obj,flag) {flag = 0;}
#define __LEAVE_CRITICAL_SECTION(obj,flag)

#ifdef __cplusplus
}
#endif


#endif //__KAPI_H__
