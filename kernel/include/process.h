//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 18,2015
//    Module Name               : process.h
//    Module Funciton           : 
//                                Process mechanism related data types and
//                                operations.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

/*
 * Procedures to implement process mechanism:
 * 1. Create and install TSS for each AP; --OK
 * 2. Adjust virtual memory manager and page index manager to support
 *    allocate VM from different region(kernel/user...);
 * 3. Pending issue: Remove all kernel thread(s) belong to one process,
 *    in case of kill process brutally;
 * 4. Process can not create kernel thread,only user thread can be
 *    created by process; --OK
 * 5. Create user thread routine should be added into kenrel thread manager,
 *    to create user thread(given user stack,cpu affinity,start address,...),common
 *    code of them should be grasped into a common routine; --OK
 * 6. Use same facilities of kernel thread for user thread,use flags
 *    to distinguish kernel/user thread,flags also indicates what
 *    type of the thread(realtime/no-realtime,others...); ---- Macros IS_USER_THREAD
 *    and IS_KERNEL_THREAD are implemented to tell the thread type; --OK
 * 7. Adjust __SwitchTo routine,to take prev and next thread as parameters,
 *    consider implement a new routine(Switch(prev,next)?) to as wrapper,
 *    then call __SwitchTo routine inner.__SwitchTo switches to kernel thread
 *    from kernel thread,another routine should be implemented to switch
 *    to user thread; --OK
 * 8. Switch types: kernel to kernel(kernel thread to kernel thread),kernel to
 *    user,user to kernel is implemented by interrupt or system call; --OK
 * 9. System call's mechanism should be revised to support process,copy data
 *    from kernel to user should be implemented by the way;
 * 10. Agent code of the first user thread of a process should be implemented,
 *    maybe through a common library;
 * 12. All objects that created by a process should be recorded and thus can
 *    be cleaned when destroy the process;---- HANDLE array can manage all kernel
 *    objects one process created,and all of them can be destroyed when HANDLE
 *    array is destroyed,in termination of process;
 * 13. All pages will be commited to avoid page fault exception in first phase,
 *    page allocation on demand will be implemented later;
 * 14. Process use CPU's ring level 3,kernel use level 0; --OK
 * 15. Stack:interrupt and system call use kernel stack of current user thread,if in
 *    user space.Use kernel thread's stack if in kernel space;---- Kernel
 *    stack is stored in TSS of each AP.The TR register of each AP is loaded
 *    in process of system initialization.Kernel stack is copyed into TSS
 *    in procedure of context switching; --OK
 * 16. All memory page frames allocated to a process must be recorded by
 *    virtual memory manager or other objects,and could be released in case
 *    of exception;
 * 17. Reference counter mechanism should be added into Object Manager object,
 *    member functions,such as AddRefCounter,should be implemented,DestroyObject
 *    should check the specified object's refer counter and destroy it only
 *    the refer counter is 0; ---- OK,done.
 * 18. All kernel objects expose to user process should be encapsulated by
 *    handle,and handle array should be implemented in process object.All
 *    kernel objects and related resources could be destroyed when process
 *    object is terminated;
 * 19. Each user thread should has it's user stack and kernel stack.Kernel 
 *    stack should be saved into current processor's TSS when switches to
 *    this user thread; --OK
 * 20. Two new GDT entries should be added corresponding to user process's
 *    code segment and data segment; --OK
 * 21. Switch to user mode:establish the corresponding stack frame(EIP/CS,eFlags,
 *    ESP/SS) and invoke iretd instruction. A new macro/routine may should
 *    be implemented such as __SwitchToUser; --OK
 * 22. Interrupt flag must be set before return to user,this can be achieved
 *    by or eFlags and 0x20(?) after flags register is pushed into stack,or
 *    pop->or->push procedure; --OK
 * 23. User stack and it's size must be added into __KERNEL_THREAD_OBJECT,
 *    since the existing pointer is used for kernel stack.Two macros
 *    IS_USER_THREAD and IS_KERNEL_THREAD should be revised by checking the
 *    value of user stack pointer.If the user stack pointer is NULL then
 *    the corresponding thread is kernel thread,otherwise is user thread; --OK
 * 24. The shell runs in kernel space and is a kernel thread.Applications
 *    loaded from storage or network are treated as user process,run in
 *    user space; --OK
 * 25. A new wrapper routine __UserThreadWrapper,that's the counterpart of
 *    __KernelThreadWrapper,should be implemented and all user threads start
 *    from this routine; --OK,no need since user agent exists.
 * 26. __UserThreadWrapper then calls another new implemented routine,named
 *    __UserThreadAgent.The __UserThreadAgent is mapped into user space,so
 *    the __UserThreadWrapper should use iretd instructions to invoke it.
 *    __UserThreadWrapper will never return;
 * 27. Then the user specified entry point will be invoked by __UserThreadAgent
 *    routine directly;
 * 28. The bottom part of __UserThreadAgent routine,after execution of user
 *    specified routine,must invoke a system call and return to kernel space;
 * 29. The return to kernel system call then cleans all resources that the user
 *    thread occupies,such as wait up all waiting threads. And does process
 *    level cleaning when it's the last thread of the current process;
 * 30. All process owned resources must be destroyed before all user threads'
 *    destroying,since timer objects may lead messages sending to thread 
 *    object.If all threads are destroyed and the timer is not canceled,may lead
 *    system crash;
 * 31. Kernel object's destroy mechanism should be revised,all kernel object's
 *    destroying process should be put into uninitialization code.So DestroyKernelThread,
 *    CancelTimer,DisconnectInterrupt,VirtualMemoryManager's uninitialization,
 *    page index manager's uninitialization,and other related kernel object's
 *    uninitialization routine should be checked or revised accordingly;
 * 32. So the only approach of destroy a kernel thread,is through the ObjectManager's
 *    DestroyObject routine.Then reference counter mechanism will apply; --OK
 * 33. ReleasePage should be optimized to support release of page table,and kill
 *    command should be trouble shotted since there is memory leak after execution
 *    of this command;
 * 34. More comments on user thread's context switching:1)User thread invokes system
 *    calls to trape into kernel.Once jump into kernel,it's stack is switched to
 *    kernel stack by CPU,and it becomes a kernel thread from now.Any context switching
 *    in kernel mode is same with kernel threads.After finishing the system service,
 *    system call wrapper routines in kernel will switch the thread back into user
 *    mode,by calling __SwitchToUser routine.Updating TSS of current processor,accumulating
 *    the CPU usage of current user thread in kernel node,should be applied before
 *    switch to user mode; 2)User thread is interrupted by external
 *    interrupt.In this case the kernel stack will be used as current stack,and thread
 *    related registers such as CS/eFlags/SS/ESP will be pushed into kernel stack.
 *    After execution of interrupt handler,the kernel will check the next thread's type,
 *    if it's a kernel thread,then use traditional kernel thread switching mechanism,
 *    otherwise the kernel will invoke __SwitchToUser routine to switch back to
 *    user mode.This routine(__SwitchToUser),sets TSS with the user thread's kernel
 *    stack pointer,reloads the CR3 register,and then issue iretd instruction.This
 *    instruction will branch to kernel or user mode,detoured by the contents in
 *    stack;
 * 35. One user thread must have it's parent process,the page table's base address is
 *    obtained from process's page index manager object; --OK
 * 36. The user thread's kernel stack pointer must be set into current CPU's TSS structure,
 *    each CPU has it's own TSS structure; --OK
 * 37. Each process has it's own page index manager,and a global virtual memory manager
 *    is defined to service all processes and the kernel(??); --OK
 * 38. Each process has it's own virtual memory manager object,the whole user space is
 *    managed by this dedicated manager object; --OK
 * 39. Check flag mechanism should be revised to support more threads with flags set,
 *    in ScheduleFromInt routine.Currently only one thread's flag is processed;
 * 40. __SwitchTo routine should be updated that the target thread as it's parameter,
 *    then it should check the thread's type,i.e,user or kernel,then invoked the
 *    corresponding context switching routines.If the thread is a kernel one,then 
 *    just call the old __SwitchTo routine,otherwise call __SwitchToUser routine,
 *    current process's page directory address and it's context as input parameters
 *    of this routine;-- done. --OK
 * 41. The only difference that traditional __SwitchTo and __SwitchToUser is,the last
 *    one will update the CR3 register,but no need of the first.IRETD instruction
 *    will be used by both to launch CPU's context switching,hardware will check the
 *    CPL and RPL and do different actions; --OK, CS/DS/SS... all these registers are
 *    updated when switch to user thread; 
 * 
 */

/*
 * 1. Procedures of create a user thread:
 *   1) Create user thread's stack from user space,using VirtualAlloc;
 *   2) Get current process handle;
 *   3) Call the underlay __CreateThread routine to do it.
 *
 * TO DO:
 * 1) Create virtual memory manager and page index manager in CreateProcess,
 *    reload CR3 register in __SwitchTo routine when the next thread is a 
 *    user one; --OK
 * 2) Add CS/DS for user space,add TSS for each CPU in process of system
 *    initialization; --OK
 * 3) Consider seperating __SaveAndSwitch into 2 operations,one to save
 *    current kernel thread's context,another switches to next thread,that
 *    can be achieved by calling __local_SwitchTo routine.Thus the switch
 *    to routine could be unified no matter the target thread is user or
 *    kernel;-- OK
 * 4) Initializes kernel stack and user stack properly and carefully,
 *    InitKernelThreadContext routine in arch_x86.c file may need change; --OK
 * 5) Allocate user space memory and fill it with dead loop machine code,
 *    then jump to there.In this case no system call triggered but only
 *    interrupts raise,the shell should be OK,and CPU's usage should reach
 *    100%,no exception raise; --OK
 * 6) Launch system call's implementation and optimization......
 * 7) 334 line of pageidx.c file may triggered when PCNet NIC is enabled
 *    in virtual box,issue pending;
 *
 * TO BE REVISED:
 * 1) InitKernelThreadContext,just push KMEM_USERAPP_START into stack;
 * 2) User stack should be established before calling InitKernelThreadContext
 *    routine in __CreateThread,since it will use the user stack;--OK
 * 3) Should check the current executing mode before switch to,currently
 *    just switch to if it's a user thread,since no system call is
 *    implemented yet;-- OK
 * 4) The entry routine specified to CreateUserThread in CreateProcess
 *    should be considered carefully; --OK
 * 5) Just allocate user app space from CreateProcess routine and copy
 *    a deadloop into it,for debugging;--OK
 * 6) UserMemoryCopy routine should consider the scenario that the caller 
 *    is current process;
 * 7) Memory leak in case of create process fail,PrepareProcessCtx routine
 *    fail may lead this; --OK,solved.
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "types.h"
#include "commobj.h" /* For __COMMON_OBJECT. */

/* 
 * Link list node used to contain all kernel objects 
 * belong to one process.
 */
typedef struct tag__KOBJ_LIST_NODE{
	__COMMON_OBJECT*            pKernelObject;
	struct tag__KOBJ_LIST_NODE* prev;
	struct tag__KOBJ_LIST_NODE* next;
} __KOBJ_LIST_NODE;

/* Maximal handle number in one handle array element. */
#define MAX_HANDLE_OBJECT_PTR 16

/* Handle array element to contain handles. */
typedef struct tag__HANDLE_ARRAY_ELEMENT {
	__COMMON_OBJECT* objectPtr[MAX_HANDLE_OBJECT_PTR];
	struct tag__HANDLE_ARRAY_ELEMENT* pNext;
}__HANDLE_ARRAY_ELEMENT;

/* 
 * Handle array object. 
 * Each process has one handle array object,all kernel
 * objects this process own will be put into this array,
 * and the index of the object ptr will be returned to 
 * user,to avoid user space seeing kernel object directly.
 */
typedef struct tag__HANDLE_ARRAY {
	__HANDLE_ARRAY_ELEMENT* pRoot;
	unsigned long maxHandleValue;
	unsigned long elementNum;
	unsigned long handleNum;
}__HANDLE_ARRAY;

/* 
 * Process object,the main object to manage a process 
 * under HelloX,like task control block in tradition OS.
 */
BEGIN_DEFINE_OBJECT(__PROCESS_OBJECT)
	INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT

	//Process's name.
	char ProcessName[MAX_THREAD_NAME + 1];

	//Process status.
	DWORD dwProcessStatus;

	//Entry point information.
	__KERNEL_THREAD_ROUTINE lpMainStartRoutine;
	LPVOID lpMainRoutineParam;

	/* 
	 * Virtual memory manager object,manages the memory
	 * space for this process.
	 */
	__VIRTUAL_MEMORY_MANAGER* pMemMgr;

	//Kernel object list,contains all kernel objects except kernel thread
	//belong to this process.
	//__KOBJ_LIST_NODE KernelObjectList;

    /* 
	 * Thread object list,contain all kernel thread(s) 
	 * belong to this process.
	 */
    __KOBJ_LIST_NODE ThreadObjectList;
	volatile int nKernelThreadNum;

	/* The main thread of this process. */
	__KERNEL_THREAD_OBJECT* lpMainThread;
	
	/*
	 * Flags used to control TLS,each bits in this word corresponding to each
	 * TLS in kernel thread object.
	 */
	volatile DWORD  dwTLSFlags;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/* Handle array of this process. */
	__HANDLE_ARRAY handleArray;
END_DEFINE_OBJECT(__PROCESS_OBJECT)

//Process status.
#define PROCESS_STATUS_INITIALIZING 0x00000001
#define PROCESS_STATUS_READY        0x00000002
#define PROCESS_STATUS_TERMINAL     0x00000004

//Initializer and uninitializer of process object.
BOOL ProcessInitialize(__COMMON_OBJECT* lpThis);
BOOL ProcessUninitialize(__COMMON_OBJECT* lpThis);

/* 
 * Process manager object,this is a system 
 * level object and manages all process in system.
 */
BEGIN_DEFINE_OBJECT(__PROCESS_MANAGER)
    //Process object list,contains all process(es) in system.
	__KOBJ_LIST_NODE ProcessList;
#if defined(__CFG_SYS_SMP)
	/* Spin lock to protect the process manager object. */
	__SPIN_LOCK spin_lock;
#endif

	/* 
	 * Buffer for user agent. User agent is a common
	 * executable binary block for all processes and
	 * is mapped to the start address of user space,
	 * user thread will jump to there at very first.
	 * We load the user agent into a common memory 
	 * block and map it to every process.
	 */
	LPVOID pUserAgent;
    
    /* Create a new process. */
	__PROCESS_OBJECT* (*CreateProcess)(__COMMON_OBJECT* lpThis,
		DWORD dwMainThreadStackSize,
		DWORD dwMainThreadPriority,
		char* pszCmdLine,
		LPVOID pCmdObject,
		LPVOID lpReserved,
		LPSTR lpszName);
	/* Destroy a given process. */
	VOID (*DestroyProcess)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpProcess);

	/* Link a user thread to it's owner process. */
	BOOL (*LinkKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpOwnProcess,
		__COMMON_OBJECT* lpKernelThread);

	/* Unlink a user thread from it's own process. */
	BOOL (*UnlinkKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpOwnProcess,
		__COMMON_OBJECT* lpKernelThread);

	//TLS operation routines.
	BOOL (*GetTLSKey)(__COMMON_OBJECT* lpThis,
		DWORD* pTLSKey,
		LPVOID pReserved);
	VOID (*ReleaseTLSKey)(__COMMON_OBJECT* lpThis,
		DWORD TLSKey,
		LPVOID pReserved);
	BOOL (*GetTLSValue)(__COMMON_OBJECT* lpThis,
		DWORD TLSKey,
		LPVOID* ppValue);
	BOOL (*SetTLSValue)(__COMMON_OBJECT* lpThis,
		DWORD TLSKey,
		LPVOID pValue);
    
	//Get Current process object. 
    __PROCESS_OBJECT* (*GetCurrentProcess)(__COMMON_OBJECT* lpThis);

	/* Initializer of ProcessManager itself. */
	BOOL (*Initialize)(__COMMON_OBJECT* lpThis);

	/* 
	 * Intializes CPU specific task management functions. 
	 * Task(process) related mechanism should be established
	 * in this routine,a typical example is, create TSS for
	 * current processor and initializes the TR register using
	 * the TSS.
	 * It just calls architecture specific task initialization
	 * routine.
	 */
	BOOL (*InitializeCPUTask)(__COMMON_OBJECT* pThis);

	/* 
	 * Half bottom of a process.
	 * It should be called before a user thread exit,
	 * through the maner of system call. 
	 */
	void (*ProcessHalfBottom)();

	/* Routines to operate handle array of process. */
	/*
	 * Save the specified kernel object into handle
	 * array and returns the index(HANDLE) of the slot.
	 */
	__HANDLE (*GetHandle)(__PROCESS_OBJECT* pProcess, __COMMON_OBJECT* pObject);

	/* Close a handle value. */
	int (*CloseHandle)(__PROCESS_OBJECT* pProcess, __HANDLE handle);

	/* Return the kernel object given it's handle. */
	__COMMON_OBJECT* (*GetObjectByHandle)(__PROCESS_OBJECT* pProcess, __HANDLE handle);

END_DEFINE_OBJECT(__PROCESS_MANAGER)

/* A macro to get the current process object. */
#define __CURRENT_PROCESS (ProcessManager.GetCurrentProcess((__COMMON_OBJECT*)&ProcessManager))

/* User agent file's name. */
#define USER_AGENT_FILE_NAME "C:\\USRAGENT.BIN"

#if defined(__CFG_SYS_PROCESS)
/* 
 * Process manager is a global object which can be 
 * accessed anywhere from the kernel.
 */
extern __PROCESS_MANAGER ProcessManager;
#endif

/* A helper routine used to debugging,dumpout all process information. */
VOID DumpProcess();

#endif //__PROCESS_H__
