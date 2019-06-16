//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,18 2004
//    Module Name               : ktmgr.h
//    Module Funciton           : 
//                                This module countains kernel thread and kernel thread 
//                                manager's definition.
//
//                                ************
//                                This file is the most important file of Hello China.
//                                ************
//    Last modified Author      : Garry
//    Last modified Date        : Mar 10,2012
//    Last modified Content     : 
//                                1. __KERNEL_THREAD_CONTEXT's definition is not right now,this is 
//                                   a bug,and will be corrected in the future.
//                                2. Multiple objects waiting mechanism is added into the kernel.
//    Lines number              :
//***********************************************************************/

#ifndef __KTMGR_H__
#define __KTMGR_H__

#include "config.h"
#include "types.h"
#include "commobj.h"
#include "objqueue.h"
#include "smp.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//***********************************************************************
//
#define MAX_THREAD_NAME    32    //Maximal kernel thread name's length.

DECLARE_PREDEFINED_OBJECT(__KERNEL_FILE)
DECLARE_PREDEFINED_OBJECT(__EVENT)

//Kernel thread's context.
//This structure is obsolete now,and it's definition is not right even.So please
//do not use it directly.
#if 0
BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_CONTEXT)
    DWORD           dwEFlags;
    //WORD            wCS;
	//WORD            wReserved;
	DWORD           dwCSnRsvd;
	DWORD           dwEIP;
	DWORD           dwEAX;
	DWORD           dwEBX;
	DWORD           dwECX;
	DWORD           dwEDX;
	DWORD           dwESI;
	DWORD           dwEDI;
	DWORD           dwEBP;
	DWORD           dwESP;
	END_DEFINE_OBJECT(__KERNEL_THREAD_CONTEXT)
#endif
BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_CONTEXT)
	DWORD dwEBP;
	DWORD dwEDI;
	DWORD dwESI;
	DWORD dwEDX;
	DWORD dwECX;
	DWORD dwEBX;
	DWORD dwEAX;
	DWORD dwEIP;
	DWORD dwCSnRsvd;
	DWORD dwEFlags;
END_DEFINE_OBJECT(__KERNEL_THREAD_CONTEXT)

//
//The following macro is used to initialize a kernel thread's executation
//context.
//
#define INIT_EFLAGS_VALUE 512
#define INIT_KERNEL_THREAD_CONTEXT_I(lpContext,initEip,initEsp) \
    (lpContext)->dwEFlags    = INIT_EFLAGS_VALUE;               \
    (lpContext)->wCS         = 0x08;                            \
	(lpContext)->wReserved   = 0x00;                            \
	(lpContext)->dwEIP       = initEip;                         \
	(lpContext)->dwEAX       = 0x00000000;                      \
	(lpContext)->dwEBX       = 0x00000000;                      \
	(lpContext)->dwECX       = 0x00000000;                      \
	(lpContext)->dwEDX       = 0x00000000;                      \
	(lpContext)->dwESI       = 0x00000000;                      \
	(lpContext)->dwEDI       = 0x00000000;                      \
	(lpContext)->dwEBP       = 0x00000000;                      \
	(lpContext)->dwESP       = initEsp;

//
//In order to access the members of a context giving it's base address,
//we define the following macros to make this job easy.
//
#define CONTEXT_OFFSET_EFLAGS        0x00
#define CONTEXT_OFFSET_CS            0x04
#define CONTEXT_OFFSET_EIP           0x08
#define CONTEXT_OFFSET_EAX           0x0C
#define CONTEXT_OFFSET_EBX           0x10
#define CONTEXT_OFFSET_ECX           0x14
#define CONTEXT_OFFSET_EDX           0x18
#define CONTEXT_OFFSET_ESI           0x1C
#define CONTEXT_OFFSET_EDI           0x20
#define CONTEXT_OFFSET_EBP           0x24
#define CONTEXT_OFFSET_ESP           0x28

//
//Common synchronization object's definition.
//The common synchronization object is a abstract object,all synchronization objects,such
//as event,mutex,etc,all inherited from this object.
//
BEGIN_DEFINE_OBJECT(__COMMON_SYNCHRONIZATION_OBJECT)
    DWORD                (*WaitForThisObject)(struct tag__COMMON_SYNCHRONIZATION_OBJECT*);
    DWORD                dwObjectSignature;  //Signature of the kernel object,to verify the object's validate.
END_DEFINE_OBJECT(__COMMON_SYNCHRONIZATION_OBJECT)

#define KERNEL_OBJECT_SIGNATURE     0xAA5555AA

//
//The following macro is defined to be used by other objects to inherit from
//common synchronization object.
//
#define INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT \
	DWORD                (*WaitForThisObject)(__COMMON_OBJECT*); \
	DWORD                dwObjectSignature;

//
//Kernel thread's message.
//
BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_MESSAGE)
    WORD             wCommand;
    WORD             wParam;
	DWORD            dwParam;
END_DEFINE_OBJECT(__KERNEL_THREAD_MESSAGE)

/* 
 * The maximal message number in the kernel thread's 
 * message queue. 
 */
#ifndef MAX_KTHREAD_MSG_NUM
#define MAX_KTHREAD_MSG_NUM 32
#endif

/* 
 * Status value of a kernel thread. 
 */
#define KERNEL_THREAD_STATUS_RUNNING    0x00000001
#define KERNEL_THREAD_STATUS_READY      0x00000002
#define KERNEL_THREAD_STATUS_SUSPENDED  0x00000003
#define KERNEL_THREAD_STATUS_SLEEPING   0x00000004
#define KERNEL_THREAD_STATUS_TERMINAL   0x00000005
#define KERNEL_THREAD_STATUS_BLOCKED    0x00000006
#define KERNEL_THREAD_STATUS_DESTROYED  0x00000007

/* 
 * Flags can be set on a thread.It will lead special 
 * operations on this thread object,such as kill it
 * forceful.
 */
#define KERNEL_THREAD_FLAGS_KILLED          0x00000001
#define KERNEL_THREAD_FLAGS_HUNG            0x00000002
#define KERNEL_THREAD_FLAGS_AFFINITYCHANGED 0x00000004

/* 
 * Waiting tags,indicates what kind of object the
 * current kernel thread is waiting for,when current
 * kernel thread's status is BLOCKED.
 */
#define KERNEL_THREAD_WAITTAG_MESSAGE   1
#define KERNEL_THREAD_WAITTAG_SEMAPHORE 2
#define KERNEL_THREAD_WAITTAG_MAILBOX   3
#define KERNEL_THREAD_WAITTAG_MUTEX     4
#define KERNEL_THREAD_WAITTAG_KTHREAD   5
#define KERNEL_THREAD_WAITTAG_EVENT     6
#define KERNEL_THREAD_WAITTAG_MULTIPLE  7
#define KERNEL_THREAD_WAITTAG_COND      8

//
//Definition of kernel thread's priority.
//In V1.5,there are 17 kernel thread priority levels,separated into
//five groups,that are,CRITICAL,HIGH,NORMAL,LOW,IDLE,and in each group
//expect IDLE,there are four levels.
//
#define MAX_KERNEL_THREAD_PRIORITY      0x00000010  //16

#define PRIORITY_LEVEL_CRITICAL         0x00000010
#define PRIORITY_LEVEL_CRITICAL_1       0x00000010
#define PRIORIRY_LEVEL_CRITICAL_2       0x0000000F
#define PRIORITY_LEVEL_CRITICAL_3       0x0000000E
#define PRIORITY_LEVEL_CRITICAL_4       0x0000000D

#define PRIORITY_LEVEL_HIGH             0x0000000C
#define PRIORITY_LEVEL_HIGH_1           0x0000000C
#define PRIORITY_LEVEL_HIGH_2           0x0000000B
#define PRIORITY_LEVEL_HIGH_3           0x0000000A
#define PRIORITY_LEVEL_HIGH_4           0x00000009

#define PRIORITY_LEVEL_NORMAL           0x00000008
#define PRIORITY_LEVEL_NORMAL_1         0x00000008
#define PRIORITY_LEVEL_NORMAL_2         0x00000007
#define PRIORITY_LEVEL_NORMAL_3         0x00000006
#define PRIORITY_LEVEL_NORMAL_4         0x00000005

#define PRIORITY_LEVEL_LOW              0x00000004
#define PRIORITY_LEVEL_LOW_1            0x00000004
#define PRIORITY_LEVEL_LOW_2            0x00000003
#define PRIORITY_LEVEL_LOW_3            0x00000002
#define PRIORITY_LEVEL_LOW_4            0x00000001

#define PRIORITY_LEVEL_IDLE             0x00000000

//
//The following macros alse can be used to define kernel
//thread's priority,they are aliases for above priority level.
//
#define PRIORITY_LEVEL_REALTIME         PRIORITY_LEVEL_CRITICAL_1
#define PRIORITY_LEVEL_IMPORTANT        PRIORITY_LEVEL_HIGH_1
#define PRIORITY_LEVEL_LOWEST           PRIORITY_LEVEL_IDLE

//The maximal kernel objects number one kernel thread can wait at the same time.
#define MAX_MULTIPLE_WAIT_NUM           0x08

//
//The kernel thread object's definition.
//
BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_OBJECT)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
    __KERNEL_THREAD_CONTEXT* lpKernelThreadContext;
    unsigned long dwThreadID;
	/* 
	 * The process owns the kernel thread. 
	 * This value must be NULL if is a kernel thread.
	 * otherwise it's value must be set.
	 */
	__COMMON_OBJECT* lpOwnProcess;
	/* Thread's status value. */
	volatile unsigned long dwThreadStatus;
	/* Flags that can be set on thread object. */
	volatile unsigned long ulThreadFlags;
	/* 
	 * Waiting queue of the kernel thread object.
	 * One kernel thread who want to wait the current
	 * kernel thread object will be put into this queue,
	 * unless the current thread's status is TERMINAL.
	 */
	__PRIORITY_QUEUE* lpWaitingQueue;
	/* Priority value of the thread. */
	unsigned long dwThreadPriority;
	/* Scheduling counter used to control scheduler. */
	unsigned long dwScheduleCounter;
	/* Return value after finish execution or terminated. */
	unsigned long dwReturnValue;
	unsigned long dwTotalRunTime;
	/* How many physical memory used. */
	unsigned long dwTotalMemSize;

	/* Local heap mechanism. */
	LPVOID lpHeapObject;
	LPVOID lpDefaultHeap;
	/* Whether FPU is used by this thread. */
	BOOL bUsedMath;

	/* Kernel stack's size. */
	unsigned long dwStackSize;
	/* END address of kernel stack. */
	LPVOID lpInitStackPointer;
#if defined(__CFG_SYS_PROCESS)
	/* Thread's user stack and it's size. */
	LPVOID pUserStack;
	unsigned long user_stk_size;
	/* If the thread is in user mode. */
	__atomic_t in_user;
#endif

	/* Start routine and it's parameter of the thread. */
	unsigned long (*KernelThreadRoutine)(LPVOID);
	LPVOID lpRoutineParam;

	/*
	 * Message queue of the kernel thread,include the variables,such as
	 * queue head and tail index,to manipulate the message queue.
	 */
	__KERNEL_THREAD_MESSAGE KernelThreadMsg[MAX_KTHREAD_MSG_NUM];
	volatile UCHAR ucMsgQueueHeader;
	volatile UCHAR ucMsgQueueTrial;
	volatile UCHAR ucCurrentMsgNum;
	UCHAR ucAligment;

	/* How many messages received from start. */
	volatile UINT nMsgReceived;
	/* Messages droped,caused by queue full or others. */
	volatile UINT nMsgDroped;
	/* Pending queue to wait the message's arrival or fetching. */
	__PRIORITY_QUEUE* lpMsgWaitingQueue;

	/* User private data. */
	unsigned long dwUserData;
	/* Last error number. */
	unsigned long dwLastError;

	/* Name of this thread,in ASCII format. */
	char KernelThreadName[MAX_THREAD_NAME];

	//Used by synchronous object to indicate the waiting result.
	volatile DWORD dwWaitingStatus;
	//Multiple object waiting flags,the object index which first wakeup the thread
	//also stored in this variable.
	volatile DWORD dwMultipleWaitFlags;
	//Kernel object number array the thread is waiting for.
	__COMMON_OBJECT* MultipleWaitObjectArray[MAX_MULTIPLE_WAIT_NUM];

	//TLS array.
	LPVOID TLS[MAX_TLS_NUM];

#if defined(__CFG_SYS_SMP)
	/*
	 * CPU affinity. 
	 * It's the CPU id on which the kernel thread is running,in SMP 
	 * environment,and only available when the SMP is enabled.
	 * It's default value when created is the current CPU id,and
	 * can be changed by ChangeAffinity routine of KernelThreadManager.
	 * When changed the kernel thread will be moved to target CPU's
	 * ready queue and will be scheduled in this CPU.
	 */
	__atomic_t cpuAffinity;
	/* spin lock to protect the object. */
	__SPIN_LOCK spin_lock;
#endif

	/* 
	 * Suspending flags and suspending counter,used to manage the 
	 * suspending operations for kernel thread.
	 * The meanings of each bit as follows:
	 *  bit0 - bit15:  Suspending counter,incremented by kSuspendKernelThread 
	 *                 routine and decremented by ResumeKernelThread routine;
	 *  bit16 - bit31: Suspending flags,only the 31 bit is used,to indicate 
	 *                 the suspending disable/enable status.
	 *  SUSPEND_FLAG_MASK is used to seperate these 2 parts.
	 */
	volatile unsigned long dwSuspendFlags;
END_DEFINE_OBJECT(__KERNEL_THREAD_OBJECT)

/* Check if the thread is running in user mode. */
#if defined(__CFG_SYS_PROCESS)
#define THREAD_IN_USER_MODE(pThread) (pThread->in_user)
#else
#define THREAD_IN_USER_MODE(pThread)
#endif

//Flags to control the suspending operation on kernel thread.
#define SUSPEND_FLAG_MASK                0xFFFF0000
#define SUSPEND_FLAG_DISABLE             0x80000000
#define MAX_SUSPEND_COUNTER              0x0000FFF0

//Flags to control the multiple objects waiting mechanism.
#define MULTIPLE_WAITING_STATUS          0x80000000  //If set then the kernel is in multiple waiting status.
#define MULTIPLE_WAITING_WAITALL         0x40000000  //If set then the kernel is waiting for all objects.

//A macro to check if the specified thread is in multiple waiting status.
#define IN_MULTIPLE_WAITING(p)           ((p)->dwMultipleWaitFlags & MULTIPLE_WAITING_STATUS)

//Macros to set and get the signaled object's index,which is the last 8 bits of dwMultipleWaitFlags.
#define MULTIPLE_WAITING_SETINDEX(p,index)  ((p)->dwMultipleWaitFlags &= 0xFFFFFF00); \
	((p)->dwMultipleWaitFlags += ((index) & 0x000000FF);
#define MULTIPLE_WAITING_GETINDEX(p)        ((p)->dwMultipleWaitFlags & 0x000000FF)

//Kernel thread object's initialize and uninitialize routine's definition.
BOOL KernelThreadInitialize(__COMMON_OBJECT* lpThis);
BOOL KernelThreadUninitialize(__COMMON_OBJECT* lpThis);


//Kernel thread's start routine.
typedef DWORD (*__KERNEL_THREAD_ROUTINE)(LPVOID);
//Thread hook routine's protype.
typedef DWORD (*__THREAD_HOOK_ROUTINE)(__KERNEL_THREAD_OBJECT*, DWORD*);

/* Hook types. */
#define THREAD_HOOK_TYPE_CREATE        0x00000001
#define THREAD_HOOK_TYPE_ENDSCHEDULE   0x00000002
#define THREAD_HOOK_TYPE_BEGINSCHEDULE 0x00000004
#define THREAD_HOOK_TYPE_TERMINAL      0x00000008

/* 
 * Ready queue object to hold kernel thread(s) with READY status. 
 * MAX_KERNEL_THREAD_PRIORITY object queues in each ready queue,
 * hold all ready kernel thread(s) with corresponding priority level.
 * There as many as processor number ready queue(s) in system,each
 * processor has it's own ready queue.
 * The ready queue object is created in process of KernelThreadManager's
 * initialization,according to the actual processor number detected
 * by ProcessorManager.
 * A ready queue pointer array is hold by KernelThreadManager object,
 * the array's size is same as MAX_CPU_NUM defined in config.h file,when
 * SMP is enabled.
 */
typedef struct tag__KERNEL_THREAD_READY_QUEUE {
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	__PRIORITY_QUEUE* ThreadQueue[MAX_KERNEL_THREAD_PRIORITY + 1];
}__KERNEL_THREAD_READY_QUEUE;

/* 
 * Object to manage all thread in HelloX. 
 * All threads,include user and kernel,are
 * managed by this object,even it's name is
 * KernelThreadManager.
 */
BEGIN_DEFINE_OBJECT(__KERNEL_THREAD_MANAGER)
    DWORD dwCurrentIRQL;
	/* Flags to control the enabling or disabling of kernel thread scheduling. */
	__atomic_t scheduling_enabled;
#if defined(__CFG_SYS_SMP)
	/* Current kernel thread's handle,each CPU has one. */
	__KERNEL_THREAD_OBJECT* CurrentKernelThread[MAX_CPU_NUM];
	/* IDLE kernel thread's handle,each CPU has one. */
	__KERNEL_THREAD_OBJECT* IdleKernelThread[MAX_CPU_NUM];
#else /* UP. */
	__KERNEL_THREAD_OBJECT* CurrentKernelThread[1];
#endif

	/* 
	 * CPU start status flags,the corresponding bit is set to 1 
	 * when the CPU start successful.
	 */
#if defined(__CFG_SYS_SMP)
#if MAX_CPU_NUM > 64
#error "MAX_CPU_NUM can not exceed 64."
#else
	volatile uint64_t cpuStartFlags;
	__SPIN_LOCK spin_lock;
#endif
#endif //__CFG_SYS_SMP

	/* 
	 * A macro to obtain current kernel thread,each CPU will get different 
	 * current kernel thread instance by using this macro.
	 */
#define __CURRENT_KERNEL_THREAD (KernelThreadManager.CurrentKernelThread[__CURRENT_PROCESSOR_ID])

	/* All suspended threads are put into this queue. */
	__PRIORITY_QUEUE* lpSuspendedQueue;
	/* All threads in sleeping status are put into this queue. */
	__PRIORITY_QUEUE* lpSleepingQueue;
	/* All threads whose status are TERMINAL. */
	__PRIORITY_QUEUE* lpTerminalQueue;
	/* All threads forcely killed are put into this queue. */
	__PRIORITY_QUEUE* lpKilledQueue;

	/* Ready queue object array. */
#if defined(__CFG_SYS_SMP)
	__KERNEL_THREAD_READY_QUEUE* KernelThreadReadyQueue[MAX_CPU_NUM];
#else
	__KERNEL_THREAD_READY_QUEUE* KernelThreadReadyQueue[1];
#endif

	DWORD dwNextWakeupTick;
	__THREAD_HOOK_ROUTINE lpCreateHook;
	__THREAD_HOOK_ROUTINE lpEndScheduleHook;
	__THREAD_HOOK_ROUTINE lpBeginScheduleHook;
	__THREAD_HOOK_ROUTINE lpTerminalHook;

	__THREAD_HOOK_ROUTINE (*SetThreadHook)(
		DWORD dwHookType,
		__THREAD_HOOK_ROUTINE lpNew);
	VOID (*CallThreadHook)(
		DWORD dwHookType,
		__KERNEL_THREAD_OBJECT* lpPrev,
		__KERNEL_THREAD_OBJECT* lpNext);

	//Get a schedulable kernel thread from ready queue.
	__KERNEL_THREAD_OBJECT* (*GetScheduleKernelThread)(
		__COMMON_OBJECT* lpThis,
		DWORD dwPriority);

	//Add a ready kernel thread to ready queue.
	VOID (*AddReadyKernelThread)(
		__COMMON_OBJECT* lpThis,
		__KERNEL_THREAD_OBJECT* lpThread);

	/* Initializer of this object. */
	BOOL (*Initialize)(__COMMON_OBJECT* lpThis);

	/* Create a new kernel thread,run in kernel space. */
	__KERNEL_THREAD_OBJECT* (*CreateKernelThread)(
		__COMMON_OBJECT* lpThis,
		DWORD dwStackSize,
		DWORD dwStatus,
		DWORD dwPriority,
		__KERNEL_THREAD_ROUTINE lpStartRoutine,
		LPVOID lpRoutineParam,
		LPVOID lpOwnerProcess,
		LPSTR lpszName);

#if defined(__CFG_SYS_PROCESS)
	/* Create a user thread,running in user space. */
	__KERNEL_THREAD_OBJECT* (*CreateUserThread)(
		__COMMON_OBJECT* lpThis,
		DWORD dwStackSize,
		DWORD dwStatus,
		DWORD dwPriority,
		__KERNEL_THREAD_ROUTINE lpStartRoutine,
		LPVOID lpRoutineParam,
		LPVOID lpOwnerProcess,
		LPSTR lpszName);
	/* Destroy the user thread created by above routine. */
	BOOL (*DestroyUserThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* pUserThread);
#endif

	BOOL (*DestroyKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread);

	/* Suspend or resume a thread. */
	BOOL (*EnableSuspend)(
		__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread,
		BOOL bSuspend);
	BOOL (*SuspendKernelThread)(
		__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread);
	BOOL (*ResumeKernelThread)(
		__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread);

	/* Trigger a new schedule in process or interrupt context. */
	VOID (*ScheduleFromProc)(__KERNEL_THREAD_CONTEXT* lpContext);
	VOID (*ScheduleFromInt)(__COMMON_OBJECT* lpThis, LPVOID lpESP);
	LPVOID(*UniSchedule)(__COMMON_OBJECT* lpThis,LPVOID lpESP);

	/* Change or get a thread's priority value. */
	DWORD (*SetThreadPriority)(__COMMON_OBJECT* lpKernelThread,
		DWORD dwNewPriority);
	DWORD (*GetThreadPriority)(__COMMON_OBJECT* lpKernelThread);

	/* Terminate a specified thread brutely. */
	DWORD (*TerminateKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread, DWORD dwExitCode);

	/* Sleep a while or cancel sleep. */
	BOOL (*Sleep)(__COMMON_OBJECT* lpThis, DWORD dwMilliSecond);
	BOOL (*CancelSleep)(__COMMON_OBJECT* lpThis, __COMMON_OBJECT* lpKernelThread);

	/* Operations applied to kernel thread internal attributes. */
	DWORD (*GetLastError)();
	DWORD (*SetLastError)(DWORD dwNewError);
	DWORD (*GetThreadID)(__COMMON_OBJECT* lpKernelThread);
	DWORD (*GetThreadStatus)(__COMMON_OBJECT* lpKernelThread);

	/* Kernel thread's message queue related operations. */
	BOOL (*SendMessage)(__COMMON_OBJECT* lpKernelThread,
		__KERNEL_THREAD_MESSAGE* lpMsg);
	BOOL (*GetMessage)(__COMMON_OBJECT* lpKernelThread,
		__KERNEL_THREAD_MESSAGE* lpMsg);
	BOOL (*PeekMessage)(__COMMON_OBJECT* lpKernelThread,
		__KERNEL_THREAD_MESSAGE* lpMsg);
	BOOL (*MsgQueueFull)(__COMMON_OBJECT* lpKernelThread);
	BOOL (*MsgQueueEmpty)(__COMMON_OBJECT* lpKernelThread);

	//Lock or Unlock kernel thread.
	BOOL (*LockKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread);
	VOID (*UnlockKernelThread)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpKernelThread);

	/* Return the current running thread in currrent CPU. */
	__COMMON_OBJECT* (*GetCurrentKernelThread)();
#if defined(__CFG_SYS_SMP)
	/* Change the specified kernel thread's cpu affinity value. */
	unsigned int (*ChangeAffinity)(__COMMON_OBJECT* lpTargetThread, unsigned int newAffinity);
#endif
	/* Start the scheduling process,it will be called after the initialization of OS. */
	BOOL (*StartScheduling)();
	/* Disable or enable the scheduling of kernel thread. */
	BOOL (*EnableScheduling)(BOOL bEnable);
	/* Scan killed queue and destroy threads in it. */
	void (*ProcessKilledQueue)(__COMMON_OBJECT* pThis);
END_DEFINE_OBJECT(__KERNEL_THREAD_MANAGER) /* End of the kernel thread manager's definition. */

/* 
 * Check if is a kernel thread. 
 * Kernel thread has not own process os it's lpOwnProcess
 * must be NULL.It's user stack also must be NULL.
 */
#define IS_KERNEL_THREAD(_thread) (NULL == _thread->lpOwnProcess)

/* Check if is a user thread. */
#define IS_USER_THREAD(_thread) (!IS_KERNEL_THREAD(_thread))

/* Flags to enable or disable kernel thread's scheduling. */
#define KERNEL_THREAD_SCHEDULING_ENABLE  0
#define KERNEL_THREAD_SCHEDULING_DISABLE 1

/* The protype of event handler. */
typedef DWORD (*__KERNEL_THREAD_MESSAGE_HANDLER)(WORD,WORD,DWORD);

/* Dispatch a message to it's handler. */
DWORD DispatchMessage(__KERNEL_THREAD_MESSAGE*,__KERNEL_THREAD_MESSAGE_HANDLER);

/***************************************************************************************
****************************************************************************************
****************************************************************************************
****************************************************************************************
***************************************************************************************/

extern __KERNEL_THREAD_MANAGER KernelThreadManager;

//--------------------------------------------------------------------------------------
//
//                          SYNCHRONIZATION OBJECTS
//
//--------------------------------------------------------------------------------------


//
//Event object's definition.
//The event object is inherited from common object and common synchronization object.
//
BEGIN_DEFINE_OBJECT(__EVENT)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	volatile DWORD dwEventStatus;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
    __PRIORITY_QUEUE* lpWaitingQueue;
	DWORD (*SetEvent)(__COMMON_OBJECT*);
	DWORD (*ResetEvent)(__COMMON_OBJECT*);
	DWORD (*PulseEvent)(__COMMON_OBJECT*);
	/* Time out waiting operation. */
	DWORD                 (*WaitForThisObjectEx)(__COMMON_OBJECT*, DWORD);
END_DEFINE_OBJECT(__EVENT)

#define EVENT_STATUS_FREE            0x00000001    //Event status.
#define EVENT_STATUS_OCCUPIED        0x00000002

//
//The following values are returned by WaitForThisObjectEx routine.
//
#define OBJECT_WAIT_MASK             0x0000FFFF
#define OBJECT_WAIT_WAITING          0x00000000
#define OBJECT_WAIT_FAILED           0x00000000
#define OBJECT_WAIT_RESOURCE         0x00000001
#define OBJECT_WAIT_TIMEOUT          0x00000002
#define OBJECT_WAIT_DELETED          0x00000004

BOOL EventInitialize(__COMMON_OBJECT*);            //The event object's initializing routine
BOOL EventUninitialize(__COMMON_OBJECT*);          //and uninitializing routine.

//--------------------------------------------------------------------------------------
//
//                                MUTEX
//
//---------------------------------------------------------------------------------------

//
//The definition of MUTEX object.
//
BEGIN_DEFINE_OBJECT(__MUTEX)
    INHERIT_FROM_COMMON_OBJECT
	INHERIT_FROM_COMMON_SYNCHRONIZATION_OBJECT
	volatile DWORD dwMutexStatus;
    volatile DWORD dwWaitingNum;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Current owner of the mutex object. */
	__KERNEL_THREAD_OBJECT* lpCurrentOwner;
	/* Current owner counter. */
	volatile int nCurrOwnCount;
	/* Waiting queue. */
    __PRIORITY_QUEUE* lpWaitingQueue;
    DWORD (*ReleaseMutex)(__COMMON_OBJECT* lpThis);
	/* Timeout waiting. */
	DWORD (*WaitForThisObjectEx)(__COMMON_OBJECT* lpThis,
		DWORD dwMillionSecond);
END_DEFINE_OBJECT(__MUTEX)

#define MUTEX_STATUS_FREE      0x00000001
#define MUTEX_STATUS_OCCUPIED  0x00000002

//
//The initializing routine of MUTEX object and uninitializing routine.
//

BOOL MutexInitialize(__COMMON_OBJECT* lpThis);
BOOL MutexUninitialize(__COMMON_OBJECT* lpThis);

/* WaitForMultipleObjects only available in UP(non SMP) environment. */
#if !defined(__CFG_SYS_SMP)
//-----------------------------------------------------------------------------------
//
//                            Multiple object waiting mechanism's definition.
//
//-----------------------------------------------------------------------------------

//WaitForMultipleObjects,bWaitAll indicates if all objects in array should be waited,if FALSE then the
//routine will return in case of any object is signaled,otherwise the routine will not return unless all
//objects in array are signaled.
//It's a timeout wait,dwMillionSeconds indicates the time to wait.OBJECT_WAIT_TIMEOUT will be returned if
//no object is signaled and the time is out.WAIT_TIME_INFINITE can be set to indicate a forever waiting.
//pnSignalObjectIndex returns the first signaled object's index value in pObjectArray,if bWaitAll is set to
//FALSE.
//OBJECT_WAIT_DELETED also maybe returned,no matter bWaitAll is TRUE or FALSE,if any one of the object in 
//array is destroyed.the pnSignalObjectIndex contains the destroyed object's index value in array.
//pnSignalObjectIndex can be NULL if you don't care about the object which wakeup the kernel thread,in case
//if bWaitAll is set to FALSE.
DWORD WaitForMultipleObjects(
							 __COMMON_OBJECT** pObjectArray, //Object handles to wait.
							 int nObjectNum,                 //Objects' number.
							 BOOL bWaitAll,                  //If wait all objects.
							 DWORD dwMillionSeconds,         //Time out value.
							 int* pnSignalObjectIndex        //Index of the first signaled object if not wait all.
							 );
#endif 

//Will wait forever,never return if no desired resource can be obtained.
#define WAIT_TIME_INFINITE   0xFFFFFFFF

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__KTMGR_H__
