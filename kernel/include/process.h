//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 18,2015
//    Module Name               : process.h
//    Module Funciton           : 
//                                Process mechanism related data types and
//                                operations.
//                                As a embedded operating system,it's no
//                                strong need to implement process,since it
//                                should be supported by VMM mechanism,and
//                                VMM feature is not supported in most
//                                embedded OS.But a clear difference is exist
//                                between traditional embedded OS and HelloX
//                                that HelloX will support more powerful smart
//                                devices than legacy embedded OS,and JamVM
//                                also requires some process features,such as
//                                signaling and TLS,so we implements part of
//                                process mechanism here,to support current's
//                                application.
//                                It's worth noting that the architecture of
//                                HelloX's process framework is extensible and
//                                add new features in the future according to
//                                real application.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PROCESS_H__
#define __PROCESS_H__


#include "types.h"
#include "commobj.h"
#include "kapi.h"
#include "syscall.h"

//Link list node used to contain all kernel objects belong to on process.
typedef struct tag__KOBJ_LIST_NODE{
	__COMMON_OBJECT*            pKernelObject;
	struct tag__KOBJ_LIST_NODE* prev;
	struct tag__KOBJ_LIST_NODE* next;
} __KOBJ_LIST_NODE;

//Process object,the main object to manage a process in HelloX,like task
//control block in tradition OS.
BEGIN_DEFINE_OBJECT(__PROCESS_OBJECT)
	INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT

	//Process's name.
	UCHAR ProcessName[MAX_THREAD_NAME + 1];

	//Process status.
	DWORD dwProcessStatus;

	//Entry point information.
	__KERNEL_THREAD_ROUTINE lpMainStartRoutine;
	LPVOID                  lpMainRoutineParam;

	//Kernel object list,contains all kernel objects except kernel thread
	//belong to this process.
	//__KOBJ_LIST_NODE KernelObjectList;

    //Kernel thread list,contain all kernel thread(s) belong to this process.
    __KOBJ_LIST_NODE KernelThreadList;
	volatile int nKernelThreadNum;

	//Main kernel thread of the process.
	__KERNEL_THREAD_OBJECT* lpMainKernelThread;
	
	//Flags used to control TLS,each bits in this word corresponding to each
	//TLS in kernel thread object.
	volatile DWORD  dwTLSFlags;
END_DEFINE_OBJECT(__PROCESS_OBJECT)

//Process status.
#define PROCESS_STATUS_INITIALIZING 0x00000001
#define PROCESS_STATUS_READY        0x00000002
#define PROCESS_STATUS_TERMINAL     0x00000004

//Initializer and uninitializer of process object.
BOOL ProcessInitialize(__COMMON_OBJECT* lpThis);
VOID ProcessUninitialize(__COMMON_OBJECT* lpThis);

//Process manager object,this is a system level object and manages all process
//in system.
BEGIN_DEFINE_OBJECT(__PROCESS_MANAGER)
    //Process object list,contains all process(es) in system.
	__KOBJ_LIST_NODE ProcessList;
    
    //Create or destroy a process.
	__PROCESS_OBJECT*                  (*CreateProcess)(
		                                      __COMMON_OBJECT*          lpThis,
											  DWORD                     dwMainThreadStackSize,
											  DWORD                     dwMainThreadPriority,
											  __KERNEL_THREAD_ROUTINE   lpMainStartRoutine,
											  LPVOID                    lpMainRoutineParam,
											  LPVOID                    lpReserved,
											  LPSTR                     lpszName);
	VOID                               (*DestroyProcess)(
		                                      __COMMON_OBJECT*          lpThis,
											  __COMMON_OBJECT*          lpProcess);

	//Link and unlink a kernel thread to it's owner process.
	BOOL                               (*LinkKernelThread)(
		                                      __COMMON_OBJECT*          lpThis,
		                                      __COMMON_OBJECT*          lpOwnProcess,
											  __COMMON_OBJECT*          lpKernelThread
											  );
	BOOL                               (*UnlinkKernelThread)(
		                                      __COMMON_OBJECT*          lpThis,
											  __COMMON_OBJECT*          lpOwnProcess,
											  __COMMON_OBJECT*          lpKernelThread);

	//TLS operation.
	BOOL                               (*GetTLSKey)(__COMMON_OBJECT*    lpThis,
		                                            DWORD*              pTLSKey,
													LPVOID              pReserved);
	VOID                               (*ReleaseTLSKey)(__COMMON_OBJECT* lpThis,
		                                                DWORD            TLSKey,
														LPVOID           pReserved);
	BOOL                               (*GetTLSValue)(__COMMON_OBJECT*  lpThis,
		                                              DWORD             TLSKey,
													  LPVOID*           ppValue);
	BOOL                               (*SetTLSValue)(__COMMON_OBJECT*  lpThis,
		                                              DWORD             TLSKey,
													  LPVOID            pValue);
    //Obtain current process.
    __PROCESS_OBJECT*                  (*GetCurrentProcess)(__COMMON_OBJECT* lpThis);

	//Initialize of ProcessManager.
	BOOL                               (*Initialize)(__COMMON_OBJECT* lpThis);

END_DEFINE_OBJECT(__PROCESS_MANAGER)

//Process manager is a global object which can be asscessed anywhere.
extern __PROCESS_MANAGER ProcessManager;

//A helper routine used to debugging,dumpout all process information.
VOID DumpProcess();


#endif //__PROCESS_H__
