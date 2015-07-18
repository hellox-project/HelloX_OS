# 1 "mem_scat.c"
# 1 "/media/gaojie/jdev/helloX/git/HelloX_OS/kernel/osentry//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "mem_scat.c"
# 19 "mem_scat.c"
# 1 "../../kernel/include/StdAfx.h" 1
# 38 "../../kernel/include/StdAfx.h"
# 1 "../../kernel/include/types.h" 1
# 32 "../../kernel/include/types.h"
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long BOOL;

typedef char CHAR;
typedef short SHORT;
typedef int INT;
typedef unsigned char UCHAR;
typedef short WCHAR;
typedef short TCHAR;

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

typedef double DOUBLE;
typedef float FLOAT;

typedef char* LPSTR;
typedef const char* LPCTSTR;
typedef const char* LPCSTR;

typedef void VOID;
typedef void* LPVOID;




typedef unsigned char __U8;
typedef unsigned short __U16;
typedef unsigned long __U32;


typedef unsigned int size_t;
# 95 "../../kernel/include/types.h"
typedef struct{
 unsigned long dwLowPart;
 unsigned long dwHighPart;
}__U64;






VOID u64Add(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);





VOID u64Sub(__U64* lpu64_1,__U64* lpu64_2,__U64* lpu64_result);




BOOL EqualTo(__U64* lpu64_1,__U64* lpu64_2);
BOOL LessThan(__U64* lpu64_1,__U64* lpu64_2);
BOOL MoreThan(__U64* lpu64_1,__U64* lpu64_2);
VOID u64Div(__U64*,__U64*,__U64*,__U64*);




VOID u64RotateLeft(__U64* lpu64_1,DWORD dwTimes);
VOID u64RotateRight(__U64* lpu64_1,DWORD dwTimes);
# 138 "../../kernel/include/types.h"
BOOL u64Hex2Str(__U64* lpu64,LPSTR lpszResult);
# 39 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/../lib/string.h" 1
# 22 "../../kernel/include/../lib/string.h"
BOOL StrCmp(LPSTR,LPSTR);


WORD StrLen(LPSTR);


BOOL Hex2Str(DWORD,LPSTR);

BOOL Str2Hex(LPSTR,DWORD*);

BOOL Str2Int(LPSTR,DWORD*);
BOOL Int2Str(DWORD,LPSTR);

VOID PrintLine(LPSTR);

VOID StrCpy(LPSTR,LPSTR);


VOID ConvertToUper(LPSTR);

INT FormString(LPSTR,LPSTR,LPVOID*);


void* memcpy(void* dst,const void* src,size_t count);
void* memset(void* dst,int val,size_t count);
void* memzero(void* dst,size_t count);
int memcmp(const void* p1,const void* p2,int count);
void* memchr (const void * buf,int chr,size_t cnt);
void *memmove(void *dst,const void *src,int n);


char* strcat(char* dst,const char* src);
char* strcpy(char* dst,const char* src);


char * strchr (const char *s, int c_in);
char * strrchr(const char * str,int ch);
char * strstr(const char *s1,const char *s2);

int strcmp(const char* src,const char* dst);
int strlen(const char* s);


char* strncpy(char *dest,char *src,unsigned int n);
int strncmp ( char * s1, char * s2, size_t n);






void strtrim(char * dst,int flag);

int strtol(const char *nptr, char **endptr, int base);

void ToCapital(LPSTR lpszString);


int ffs(int x);
# 43 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/perf.h" 1
# 31 "../../kernel/include/perf.h"
typedef struct{
 __U64 u64Start;
 __U64 u64End;
 __U64 u64Result;
 __U64 u64Max;
} __PERF_RECORDER;
# 47 "../../kernel/include/perf.h"
VOID PerfBeginRecord(__PERF_RECORDER* lpPr);






VOID PerfEndRecord(__PERF_RECORDER* lpPr);






VOID PerfCommit(__PERF_RECORDER* lpPr);
# 47 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/commobj.h" 1
# 26 "../../kernel/include/commobj.h"
typedef struct tag__COMMON_OBJECT{
 DWORD dwObjectType;
 DWORD dwObjectID;
 DWORD dwObjectSize;
 struct tag__COMMON_OBJECT* lpPrevObject;
 struct tag__COMMON_OBJECT* lpNextObject;
 struct tag__COMMON_OBJECT* lpObjectOwner;


 BOOL (*Initialize)(struct tag__COMMON_OBJECT*);
 VOID (*Uninitialize)(struct tag__COMMON_OBJECT*);
} __COMMON_OBJECT;
# 77 "../../kernel/include/commobj.h"
typedef struct tag__OBJECT_INIT_DATA{
 DWORD dwObjectType;
 DWORD dwObjectSize;
 BOOL (*Initialize)(__COMMON_OBJECT*);
 VOID (*Uninitialize)(__COMMON_OBJECT*);
}__OBJECT_INIT_DATA;
# 111 "../../kernel/include/commobj.h"
typedef struct tag__OBJECT_LIST_HEADER{
    DWORD dwObjectNum;
    DWORD dwMaxObjectID;
 __COMMON_OBJECT* lpFirstObject;
}__OBJECT_LIST_HEADER;
# 124 "../../kernel/include/commobj.h"
typedef struct tag__OBJECT_MANAGER{
    DWORD dwCurrentObjectID;
 __OBJECT_LIST_HEADER ObjectListHeader[64];
 __COMMON_OBJECT* (*CreateObject)(struct tag__OBJECT_MANAGER*,__COMMON_OBJECT*,DWORD);
 __COMMON_OBJECT* (*GetObjectByID)(struct tag__OBJECT_MANAGER*,DWORD);
 __COMMON_OBJECT* (*GetFirstObjectByType)(struct tag__OBJECT_MANAGER*,DWORD);
 VOID (*DestroyObject)(struct tag__OBJECT_MANAGER*,__COMMON_OBJECT*);
}__OBJECT_MANAGER;
# 140 "../../kernel/include/commobj.h"
extern __OBJECT_MANAGER ObjectManager;
# 51 "../../kernel/include/StdAfx.h" 2
# 66 "../../kernel/include/StdAfx.h"
# 1 "../../kernel/include/objqueue.h" 1
# 23 "../../kernel/include/objqueue.h"
typedef struct tag__PRIORITY_QUEUE_ELEMENT{
    __COMMON_OBJECT* lpObject;
    DWORD dwPriority;
 struct tag__PRIORITY_QUEUE_ELEMENT* lpNextElement;
 struct tag__PRIORITY_QUEUE_ELEMENT* lpPrevElement;
}__PRIORITY_QUEUE_ELEMENT;



typedef struct tag__PRIORITY_QUEUE{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
    __PRIORITY_QUEUE_ELEMENT ElementHeader;
    DWORD dwCurrElementNum;
 BOOL (*InsertIntoQueue)(
                           __COMMON_OBJECT* lpThis,
         __COMMON_OBJECT* lpObject,
         DWORD dwPriority
         );
 BOOL (*DeleteFromQueue)(
                           __COMMON_OBJECT* lpThis,
         __COMMON_OBJECT* lpObject
         );
 __COMMON_OBJECT* (*GetHeaderElement)(
                           __COMMON_OBJECT* lpThis,
         DWORD* lpPriority
         );
}__PRIORITY_QUEUE;



BOOL PriQueueInitialize(__COMMON_OBJECT* lpThis);
VOID PriQueueUninitialize(__COMMON_OBJECT* lpThis);
# 67 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/ktmgr.h" 1
# 31 "../../kernel/include/ktmgr.h"
# 1 "../../kernel/include/../config/config.h" 1
# 32 "../../kernel/include/ktmgr.h" 2
# 45 "../../kernel/include/ktmgr.h"
struct __KERNEL_FILE;
struct __EVENT;





typedef struct tag__KERNEL_THREAD_CONTEXT{
    DWORD dwEFlags;
    WORD wCS;
 WORD wReserved;
 DWORD dwEIP;
 DWORD dwEAX;
 DWORD dwEBX;
 DWORD dwECX;
 DWORD dwEDX;
 DWORD dwESI;
 DWORD dwEDI;
 DWORD dwEBP;
 DWORD dwESP;
}__KERNEL_THREAD_CONTEXT;
# 109 "../../kernel/include/ktmgr.h"
typedef struct tag__COMMON_SYNCHRONIZATION_OBJECT{
    DWORD (*WaitForThisObject)(struct tag__COMMON_SYNCHRONIZATION_OBJECT*);
    DWORD dwObjectSignature;
}__COMMON_SYNCHRONIZATION_OBJECT;
# 129 "../../kernel/include/ktmgr.h"
typedef struct tag__KERNEL_THREAD_MESSAGE{
    WORD wCommand;
    WORD wParam;
 DWORD dwParam;

}__KERNEL_THREAD_MESSAGE;
# 208 "../../kernel/include/ktmgr.h"
typedef struct tag__KERNEL_THREAD_OBJECT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 __KERNEL_THREAD_CONTEXT KernelThreadContext;
    __KERNEL_THREAD_CONTEXT* lpKernelThreadContext;
    DWORD dwThreadID;
 __COMMON_OBJECT* lpOwnProcess;
 DWORD dwThreadStatus;

 __PRIORITY_QUEUE* lpWaitingQueue;







 DWORD dwThreadPriority;
 DWORD dwScheduleCounter;

 DWORD dwReturnValue;
 DWORD dwTotalRunTime;
 DWORD dwTotalMemSize;
 LPVOID lpHeapObject;
 LPVOID lpDefaultHeap;
 BOOL bUsedMath;
 DWORD dwStackSize;
 LPVOID lpInitStackPointer;
 DWORD (*KernelThreadRoutine)(LPVOID);
 LPVOID lpRoutineParam;



 __KERNEL_THREAD_MESSAGE KernelThreadMsg[32];
 volatile UCHAR ucMsgQueueHeader;
 volatile UCHAR ucMsgQueueTrial;
 volatile UCHAR ucCurrentMsgNum;
 UCHAR ucAligment;
 __PRIORITY_QUEUE* lpMsgWaitingQueue;

 DWORD dwUserData;
 DWORD dwLastError;
 UCHAR KernelThreadName[32];


 volatile DWORD dwWaitingStatus;


 volatile DWORD dwMultipleWaitFlags;

 __COMMON_OBJECT* MultipleWaitObjectArray[0x08];


 LPVOID TLS[0x04];
# 270 "../../kernel/include/ktmgr.h"
 volatile DWORD dwSuspendFlags;

}__KERNEL_THREAD_OBJECT;
# 292 "../../kernel/include/ktmgr.h"
BOOL KernelThreadInitialize(__COMMON_OBJECT* lpThis);
VOID KernelThreadUninitialize(__COMMON_OBJECT* lpThis);


typedef DWORD (*__KERNEL_THREAD_ROUTINE)(LPVOID);
typedef DWORD (*__THREAD_HOOK_ROUTINE)(__KERNEL_THREAD_OBJECT*,
            DWORD*);
# 307 "../../kernel/include/ktmgr.h"
typedef struct tag__KERNEL_THREAD_MANAGER{
    DWORD dwCurrentIRQL;
    __KERNEL_THREAD_OBJECT* lpCurrentKernelThread;

    __PRIORITY_QUEUE* lpRunningQueue;
 __PRIORITY_QUEUE* lpSuspendedQueue;
 __PRIORITY_QUEUE* lpSleepingQueue;
 __PRIORITY_QUEUE* lpTerminalQueue;
 __PRIORITY_QUEUE* ReadyQueue[0x00000010 + 1];

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


 __KERNEL_THREAD_OBJECT* (*GetScheduleKernelThread)(
                                        __COMMON_OBJECT* lpThis,
             DWORD dwPriority);


 VOID (*AddReadyKernelThread)(
                                        __COMMON_OBJECT* lpThis,
             __KERNEL_THREAD_OBJECT* lpThread);

 BOOL (*Initialize)(__COMMON_OBJECT* lpThis);

 __KERNEL_THREAD_OBJECT* (*kCreateKernelThread)(
                                        __COMMON_OBJECT* lpThis,
             DWORD dwStackSize,
             DWORD dwStatus,
             DWORD dwPriority,
             __KERNEL_THREAD_ROUTINE lpStartRoutine,
             LPVOID lpRoutineParam,
             LPVOID lpOwnerProcess,
             LPSTR lpszName);

 VOID (*kDestroyKernelThread)(__COMMON_OBJECT* lpThis,
                                       __COMMON_OBJECT* lpKernelThread
            );

 BOOL (*kEnableSuspend)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread,
            BOOL bSuspend);

 BOOL (*kSuspendKernelThread)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread);

 BOOL (*kResumeKernelThread)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread);

 VOID (*ScheduleFromProc)(
                                       __KERNEL_THREAD_CONTEXT* lpContext
            );

 VOID (*ScheduleFromInt)(
                                       __COMMON_OBJECT* lpThis,
            LPVOID lpESP
            );

 LPVOID (*UniSchedule)(
                                       __COMMON_OBJECT* lpThis,
            LPVOID lpESP);

 DWORD (*kSetThreadPriority)(
            __COMMON_OBJECT* lpKernelThread,
            DWORD dwNewPriority
            );

 DWORD (*GetThreadPriority)(
                                       __COMMON_OBJECT* lpKernelThread
            );

 DWORD (*TerminateKernelThread)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread,
            DWORD dwExitCode
            );

 BOOL (*kSleep)(
                                       __COMMON_OBJECT* lpThis,

            DWORD dwMilliSecond
            );

 BOOL (*CancelSleep)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread
            );

 DWORD (*SetCurrentIRQL)(
                                       __COMMON_OBJECT* lpThis,
            DWORD dwNewIRQL
            );

 DWORD (*GetCurrentIRQL)(
                                       __COMMON_OBJECT* lpThis
            );

 DWORD (*GetLastError)(

            );

 DWORD (*SetLastError)(

            DWORD dwNewError
            );

 DWORD (*kGetThreadID)(
                                       __COMMON_OBJECT* lpKernelThread
            );

 DWORD (*GetThreadStatus)(
                                       __COMMON_OBJECT* lpKernelThread
            );

 DWORD (*SetThreadStatus)(
                                       __COMMON_OBJECT* lpKernelThread,
            DWORD dwStatus
            );


 BOOL (*SendMessage)(
                                       __COMMON_OBJECT* lpKernelThread,
            __KERNEL_THREAD_MESSAGE* lpMsg
            );

 BOOL (*GetMessage)(
                                       __COMMON_OBJECT* lpKernelThread,
            __KERNEL_THREAD_MESSAGE* lpMsg
            );

 BOOL (*PeekMessage)(
                                       __COMMON_OBJECT* lpKernelThread,
            __KERNEL_THREAD_MESSAGE* lpMsg);

 BOOL (*MsgQueueFull)(
                                       __COMMON_OBJECT* lpKernelThread
            );

 BOOL (*MsgQueueEmpty)(
                                       __COMMON_OBJECT* lpKernelThread
            );


 BOOL (*LockKernelThread)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread);

 VOID (*UnlockKernelThread)(
                                       __COMMON_OBJECT* lpThis,
            __COMMON_OBJECT* lpKernelThread);

}__KERNEL_THREAD_MANAGER;





typedef DWORD (*__KERNEL_THREAD_MESSAGE_HANDLER)(WORD,WORD,DWORD);

DWORD DispatchMessage(__KERNEL_THREAD_MESSAGE*,__KERNEL_THREAD_MESSAGE_HANDLER);
# 491 "../../kernel/include/ktmgr.h"
extern __KERNEL_THREAD_MANAGER KernelThreadManager;
# 505 "../../kernel/include/ktmgr.h"
typedef struct tag__EVENT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 volatile DWORD dwEventStatus;
    __PRIORITY_QUEUE* lpWaitingQueue;
 DWORD (*SetEvent)(__COMMON_OBJECT*);
 DWORD (*ResetEvent)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObjectEx)(__COMMON_OBJECT*,
                                           DWORD);
}__EVENT;
# 529 "../../kernel/include/ktmgr.h"
BOOL EventInitialize(__COMMON_OBJECT*);
VOID EventUninitialize(__COMMON_OBJECT*);
# 542 "../../kernel/include/ktmgr.h"
typedef struct tag__MUTEX{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 volatile DWORD dwMutexStatus;
    volatile DWORD dwWaitingNum;
    __PRIORITY_QUEUE* lpWaitingQueue;
    DWORD (*ReleaseMutex)(__COMMON_OBJECT* lpThis);
 DWORD (*WaitForThisObjectEx)(__COMMON_OBJECT* lpThis,
                                       DWORD dwMillionSecond);
}__MUTEX;
# 560 "../../kernel/include/ktmgr.h"
BOOL MutexInitialize(__COMMON_OBJECT* lpThis);
VOID MutexUninitialize(__COMMON_OBJECT* lpThis);
# 580 "../../kernel/include/ktmgr.h"
DWORD WaitForMultipleObjects(
        __COMMON_OBJECT** pObjectArray,
        int nObjectNum,
        BOOL bWaitAll,
        DWORD dwMillionSeconds,
        int* pnSignalObjectIndex
        );
# 71 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/ktmgr2.h" 1
# 29 "../../kernel/include/ktmgr2.h"
# 1 "../../kernel/include/comqueue.h" 1
# 22 "../../kernel/include/comqueue.h"
# 1 "../../kernel/include/comqueue.h" 1
# 23 "../../kernel/include/comqueue.h" 2

# 1 "../../kernel/include/kmemmgr.h" 1
# 127 "../../kernel/include/kmemmgr.h"
typedef struct{
 LPVOID pStartAddress;
 DWORD dwMaxBlockSize;

 DWORD dwOccupMap[8];



}__4KSIZE_BLOCK;





LPVOID KMemAlloc(DWORD,DWORD);




VOID KMemFree(LPVOID,DWORD,DWORD);




DWORD GetTotalMemorySize(void);


DWORD GetFreeMemorySize(void);
# 25 "../../kernel/include/comqueue.h" 2
# 33 "../../kernel/include/comqueue.h"
typedef struct tag__COMMON_QUEUE_ELEMENT{
    struct tag__COMMON_QUEUE_ELEMENT* lpNext;
    struct tag__COMMON_QUEUE_ELEMENT* lpPrev;
 LPVOID lpObject;
}__COMMON_QUEUE_ELEMENT;
# 62 "../../kernel/include/comqueue.h"
typedef struct tag__COMMON_QUEUE{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 __COMMON_QUEUE_ELEMENT QueueHdr;
 DWORD dwQueueLen;
 DWORD dwCurrentLen;

 BOOL (*InsertIntoQueue)(__COMMON_OBJECT* lpThis,
                                                    LPVOID lpObject);
 LPVOID (*GetFromQueue)(__COMMON_OBJECT* lpThis);
 BOOL (*QueueEmpty)(__COMMON_OBJECT* lpThis);
 BOOL (*QueueFull)(__COMMON_OBJECT* lpThis);
 DWORD (*SetQueueLength)(__COMMON_OBJECT* lpThis,
                                                DWORD dwNewLen);
 DWORD (*GetQueueLength)(__COMMON_OBJECT* lpThis);
 DWORD (*GetCurrLength)(__COMMON_OBJECT* lpThis);
}__COMMON_QUEUE;







BOOL CommQueueInit(__COMMON_OBJECT* lpThis);




VOID CommQueueUninit(__COMMON_OBJECT* lpThis);
# 30 "../../kernel/include/ktmgr2.h" 2





typedef struct{
 __COMMON_OBJECT* lpSynObject;
 __PRIORITY_QUEUE* lpWaitingQueue;
 __KERNEL_THREAD_OBJECT* lpKernelThread;
 VOID (*TimeOutCallback)(VOID*);
}__TIMER_HANDLER_PARAM;


typedef struct tag__SEMAPHORE{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 DWORD dwMaxSem;
    DWORD dwCurrSem;
 __PRIORITY_QUEUE* lpWaitingQueue;


 BOOL (*SetSemaphoreCount)(__COMMON_OBJECT*,DWORD,DWORD);
 BOOL (*ReleaseSemaphore)(__COMMON_OBJECT*,DWORD* pdwPrevCount);
 DWORD (*WaitForThisObjectEx)(__COMMON_OBJECT*,DWORD dwMillionSecond,DWORD* pdwWait);
}__SEMAPHORE;


BOOL SemInitialize(__COMMON_OBJECT* pSemaphore);
VOID SemUninitialize(__COMMON_OBJECT* pSemaphore);


typedef struct tag_MB_MESSAGE{
 LPVOID pMessage;
 DWORD dwPriority;
}__MB_MESSAGE;


typedef struct tag__MAIL_BOX{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 __MB_MESSAGE* pMessageArray;
    DWORD dwMaxMessageNum;
 DWORD dwCurrMessageNum;
 DWORD dwMessageHeader;
 DWORD dwMessageTail;
 __PRIORITY_QUEUE* lpSendingQueue;
 __PRIORITY_QUEUE* lpGettingQueue;



 BOOL (*SetMailboxSize)(__COMMON_OBJECT*,DWORD);
 DWORD (*SendMail)(__COMMON_OBJECT*,LPVOID pMessage,
                        DWORD dwPriority,DWORD dwMillionSecond,DWORD* pdwWait);
 DWORD (*GetMail)(__COMMON_OBJECT*,LPVOID* ppMessage,
                       DWORD dwMillionSecond,DWORD* pdwWait);
}__MAIL_BOX;


BOOL MailboxInitialize(__COMMON_OBJECT* pMailbox);
VOID MailboxUninitialize(__COMMON_OBJECT* pMailbox);



typedef struct tag__CONDITION{
 DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;

 volatile int nThreadNum;
    __PRIORITY_QUEUE* lpPendingQueue;

 DWORD (*CondWait)(__COMMON_OBJECT* pCond,__COMMON_OBJECT* pMutex);

 DWORD (*CondWaitTimeout)(__COMMON_OBJECT* pCond,__COMMON_OBJECT* pMutex,DWORD dwMillisonSecond);

 DWORD (*CondSignal)(__COMMON_OBJECT* pCond);

 DWORD (*CondBroadcast)(__COMMON_OBJECT* pCond);
}__CONDITION;


BOOL ConditionInitialize(__COMMON_OBJECT* pCondObj);
VOID ConditionUninitialize(__COMMON_OBJECT* pCondObj);
# 75 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/process.h" 1
# 38 "../../kernel/include/process.h"
# 1 "../../kernel/include/kapi.h" 1
# 30 "../../kernel/include/kapi.h"
# 1 "../../kernel/include/hellocn.h" 1
# 64 "../../kernel/include/hellocn.h"
typedef VOID (*KEY_HANDLER)(DWORD);
# 86 "../../kernel/include/hellocn.h"
typedef VOID(*INT_HANDLER)(DWORD);
# 182 "../../kernel/include/hellocn.h"
VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR pszMsg);

VOID __BUG(LPSTR,DWORD);
# 199 "../../kernel/include/hellocn.h"
void PrintStr(const char* pszMsg);

void ClearScreen(void);

void PrintCh(unsigned short ch);

void GotoHome(void);

void ChangeLine(void);

void GotoPrev(void);


typedef VOID (*__GENERAL_INTERRUPT_HANDLER)(DWORD,LPVOID);


INT_HANDLER SetGeneralIntHandler(__GENERAL_INTERRUPT_HANDLER);




VOID WriteByteToPort(UCHAR,
      WORD);




VOID ReadByteStringFromPort(LPVOID,
          DWORD,
          WORD);




VOID WriteByteStringToPort(LPVOID,
            DWORD,
            WORD);

VOID ReadWordFromPort(WORD* pWord,
       WORD wPort);

VOID WriteWordToPort(WORD,
      WORD);

VOID ReadWordStringFromPort(LPVOID,
       DWORD,
       WORD);

VOID WriteWordStringToPort(LPVOID,
         DWORD,
         WORD);
# 31 "../../kernel/include/kapi.h" 2
# 39 "../../kernel/include/kapi.h"
# 1 "../../kernel/include/../arch/x86/syn_mech.h" 1
# 158 "../../kernel/include/../arch/x86/syn_mech.h"
typedef unsigned long __ATOMIC_T;
# 40 "../../kernel/include/kapi.h" 2
# 66 "../../kernel/include/kapi.h"
# 1 "../../kernel/include/ringbuff.h" 1
# 23 "../../kernel/include/ringbuff.h"
typedef struct tag__RING_BUFFER{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD* lpBuffer;
    volatile DWORD dwCount;
 DWORD dwBuffLength;
 __EVENT* eventWait;
 volatile DWORD dwHeader;
 volatile DWORD dwTail;


 BOOL (*SetBuffLength)(__COMMON_OBJECT* lpThis,
                               DWORD dwNewLength);
 BOOL (*GetElement)(__COMMON_OBJECT* lpThis,
                            DWORD* lpdwElement,
          DWORD dwMillionSecond);
 BOOL (*AddElement)(__COMMON_OBJECT* lpThis,
                            DWORD dwElement);
}__RING_BUFFER;






BOOL RbInitialize(__COMMON_OBJECT* lpThis);
VOID RbUninitialize(__COMMON_OBJECT* lpThis);
# 67 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/system.h" 1
# 43 "../../kernel/include/system.h"
typedef BOOL (*__INTERRUPT_HANDLER)(LPVOID lpEsp,LPVOID);
# 64 "../../kernel/include/system.h"
typedef struct tag__INTERRUPT_OBJECT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 struct tag__INTERRUPT_OBJECT* lpPrevInterruptObject;
    struct tag__INTERRUPT_OBJECT* lpNextInterruptObject;
 UCHAR ucVector;
 BOOL (*InterruptHandler)(LPVOID lpParam,LPVOID lpEsp);
 LPVOID lpHandlerParam;
}__INTERRUPT_OBJECT;

BOOL InterruptInitialize(__COMMON_OBJECT* lpThis);
VOID InterruptUninitialize(__COMMON_OBJECT* lpThis);






typedef DWORD (*__DIRECT_TIMER_HANDLER)(LPVOID);

typedef struct tag__TIMER_OBJECT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);


 DWORD dwTimerID;

 DWORD dwTimeSpan;
 __KERNEL_THREAD_OBJECT* lpKernelThread;
 LPVOID lpHandlerParam;
 DWORD (*DirectTimerHandler)(LPVOID);
 DWORD dwTimerFlags;
}__TIMER_OBJECT;

BOOL TimerInitialize(__COMMON_OBJECT* lpThis);
VOID TimerUninitialize(__COMMON_OBJECT* lpThis);
# 113 "../../kernel/include/system.h"
typedef struct tag__SYSTEM{
    __INTERRUPT_OBJECT* lpInterruptVector[256];
    __PRIORITY_QUEUE* lpTimerQueue;

 DWORD dwClockTickCounter;


 DWORD dwNextTimerTick;




 volatile UCHAR ucIntNestLevel;



 volatile UCHAR bSysInitialized;


 UCHAR ucReserved1;
 UCHAR ucReserved2;

 DWORD dwPhysicalMemorySize;

 BOOL (*BeginInitialize)(__COMMON_OBJECT* lpThis);

 BOOL (*EndInitialize)(__COMMON_OBJECT* lpThis);

 BOOL (*Initialize)(__COMMON_OBJECT* lpThis);
 DWORD (*GetClockTickCounter)(__COMMON_OBJECT* lpThis);
 DWORD (*GetSysTick)(DWORD* pdwHigh32);
 DWORD (*GetPhysicalMemorySize)(__COMMON_OBJECT* lpThis);
 VOID (*DispatchInterrupt)(__COMMON_OBJECT* lpThis,
                                                         LPVOID lpEsp,
                                                         UCHAR ucVector);
 VOID (*DispatchException)(__COMMON_OBJECT* lpThis,
                                                         LPVOID lpEsp,
                  UCHAR ucVector);

 __COMMON_OBJECT* (*ConnectInterrupt)(__COMMON_OBJECT* lpThis,
                                                        __INTERRUPT_HANDLER InterruptHandler,
                 LPVOID lpHandlerParam,
                          UCHAR ucVector,
                       UCHAR ucReserved1,
                       UCHAR ucReserved2,
                        UCHAR ucInterruptMode,
                       BOOL bIfShared,
                        DWORD dwCPUMask
                       );
 VOID (*DisconnectInterrupt)(__COMMON_OBJECT* lpThis,
                                                           __COMMON_OBJECT* lpIntObj);


 __COMMON_OBJECT* (*SetTimer)(__COMMON_OBJECT* lpThis,
                                                __KERNEL_THREAD_OBJECT* lpKernelThread,
                     DWORD dwTimerID,
                     DWORD dwTimeSpan,
                     __DIRECT_TIMER_HANDLER DirectTimerHandler,
                     LPVOID lpHandlerParam,
               DWORD dwTimerFlags
                     );
 VOID (*CancelTimer)(__COMMON_OBJECT* lpThis,
                                                   __COMMON_OBJECT* lpTimer);

}__SYSTEM;
# 194 "../../kernel/include/system.h"
extern __SYSTEM System;

extern __PERF_RECORDER TimerIntPr;
# 208 "../../kernel/include/system.h"
VOID GeneralIntHandler(DWORD dwVector,LPVOID lpEsp);
# 71 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/dim.h" 1
# 31 "../../kernel/include/dim.h"
typedef struct tag__DEVICE_MESSAGE{
    WORD wDevMsgType;
    WORD wDevMsgParam;
 DWORD dwDevMsgParam;
}__DEVICE_MESSAGE;




typedef struct tag__DEVICE_INPUT_MANAGER{
    __KERNEL_THREAD_OBJECT* lpFocusKernelThread;
    __KERNEL_THREAD_OBJECT* lpShellKernelThread;

 DWORD (*SendDeviceMessage)(__COMMON_OBJECT* lpThis,
                                    __DEVICE_MESSAGE* lpDevMsg,
                     __COMMON_OBJECT* lpTarget);

 __COMMON_OBJECT* (*SetFocusThread)(__COMMON_OBJECT* lpThis,
                                 __COMMON_OBJECT* lpFocusThread);

 __COMMON_OBJECT* (*SetShellThread)(__COMMON_OBJECT* lpThis,
                                 __COMMON_OBJECT* lpShellThread);

 BOOL (*Initialize)(__COMMON_OBJECT* lpThis,
                             __COMMON_OBJECT* lpFocusThread,
           __COMMON_OBJECT* lpShellThread);
}__DEVICE_INPUT_MANAGER;
# 78 "../../kernel/include/dim.h"
extern __DEVICE_INPUT_MANAGER DeviceInputManager;
# 75 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/memmgr.h" 1
# 39 "../../kernel/include/memmgr.h"
typedef struct tag__PAGE_FRAME{
    struct tag__PAGE_FRAME* lpNextFrame;
    struct tag__PAGE_FRAME* lpPrevFrame;
 DWORD dwKernelThreadNum;
 DWORD dwFrameFlag;
 LPVOID lpReserved;
}__PAGE_FRAME;
# 88 "../../kernel/include/memmgr.h"
typedef struct tag__PAGE_FRAME_BLOCK{
    __PAGE_FRAME* lpNextBlock;
    __PAGE_FRAME* lpPrevBlock;
 DWORD* lpdwBitmap;
}__PAGE_FRAME_BLOCK;




typedef struct tag__PAGE_FRAME_MANAGER{
    __PAGE_FRAME* lpPageFrameArray;
    __PAGE_FRAME_BLOCK FrameBlockArray[12];
 DWORD dwTotalFrameNum;
 DWORD dwFreeFrameNum;
 LPVOID lpStartAddress;

 BOOL (*Initialize)(__COMMON_OBJECT* lpThis,
                                   LPVOID lpStartAddr,
           LPVOID lpEndAddr);

 LPVOID (*FrameAlloc)(__COMMON_OBJECT* lpThis,
                                   DWORD dwSize,
           DWORD dwFrameFlag);

 VOID (*FrameFree)(__COMMON_OBJECT* lpThis,
                                  LPVOID lpStartAddr,
          DWORD dwSize);

}__PAGE_FRAME_MANAGER;
# 79 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/pageidx.h" 1
# 127 "../../kernel/include/pageidx.h"
typedef struct tag__PAGE_INDEX_MANAGER{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 __U32* lpPdAddress;
 LPVOID (*GetPhysicalAddress)(__COMMON_OBJECT*,LPVOID);
 BOOL (*ReservePage)(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
 BOOL (*SetPageFlags)(__COMMON_OBJECT*,LPVOID,LPVOID,DWORD);
 VOID (*ReleasePage)(__COMMON_OBJECT*,LPVOID);
}__PAGE_INDEX_MANAGER;





BOOL PageInitialize(__COMMON_OBJECT*);





VOID PageUninitialize(__COMMON_OBJECT*);
# 83 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/vmm.h" 1
# 55 "../../kernel/include/vmm.h"
struct __VIRTUAL_MEMORY_MANAGER;



typedef struct tag__VIRTUAL_AREA_DESCRIPTOR{
    struct tag__VIRTUAL_MEMORY_MANAGER* lpManager;
    LPVOID lpStartAddr;
 LPVOID lpEndAddr;
 struct tag__VIRTUAL_AREA_DESCRIPTOR* lpNext;
 DWORD dwAccessFlags;
 DWORD dwCacheFlags;
 DWORD dwAllocFlags;
 __ATOMIC_T Reference;


 struct tag__VIRTUAL_AREA_DESCRIPTOR* lpLeft;
 struct tag__VIRTUAL_AREA_DESCRIPTOR* lpRight;
    UCHAR strName[32];



}__VIRTUAL_AREA_DESCRIPTOR;
# 116 "../../kernel/include/vmm.h"
typedef struct tag__VIRTUAL_MEMORY_MANAGER{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 __PAGE_INDEX_MANAGER* lpPageIndexMgr;
    __VIRTUAL_AREA_DESCRIPTOR* lpListHdr;
 __VIRTUAL_AREA_DESCRIPTOR* lpTreeRoot;

 DWORD dwVirtualAreaNum;


 LPVOID (*VirtualAlloc)(__COMMON_OBJECT*,
                                               LPVOID,
              DWORD,
              DWORD,
              DWORD,
              UCHAR*,
              LPVOID
              );
 VOID (*VirtualFree)(__COMMON_OBJECT*,
                                              LPVOID
             );
 LPVOID (*GetPdAddress)(__COMMON_OBJECT*);
}__VIRTUAL_MEMORY_MANAGER;





BOOL VmmInitialize(__COMMON_OBJECT*);




VOID VmmUninitialize(__COMMON_OBJECT*);
# 160 "../../kernel/include/vmm.h"
extern __VIRTUAL_MEMORY_MANAGER* lpVirtualMemoryMgr;

VOID PrintVirtualArea(__VIRTUAL_MEMORY_MANAGER*);
# 87 "../../kernel/include/kapi.h" 2
# 95 "../../kernel/include/kapi.h"
# 1 "../../kernel/include/../arch/x86/arch.h" 1
# 24 "../../kernel/include/../arch/x86/arch.h"
typedef VOID (*__KERNEL_THREAD_WRAPPER)(__COMMON_OBJECT*);






VOID InitKernelThreadContext(__KERNEL_THREAD_OBJECT* lpKernelThread,
        __KERNEL_THREAD_WRAPPER lpStartAddr);


VOID EnableVMM(void);


VOID HaltSystem();


VOID __SwitchTo(__KERNEL_THREAD_CONTEXT* lpContext);


VOID __SaveAndSwitch(__KERNEL_THREAD_CONTEXT** lppOldContext,
      __KERNEL_THREAD_CONTEXT** lppNewContext);


VOID __GetTsc(__U64*);


VOID __GetTime(BYTE*);


VOID __MicroDelay(DWORD dwmSeconds);
# 67 "../../kernel/include/../arch/x86/arch.h"
DWORD __ind(WORD wPort);
VOID __outd(WORD wPort,DWORD dwVal);
UCHAR __inb(WORD wPort);
VOID __outb(UCHAR ucVal,WORD wPort);
WORD __inw(WORD wPort);
VOID __outw(WORD wVal,WORD wPort);
VOID __inws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort);
VOID __outws(BYTE* pBuffer,DWORD dwBuffLen,WORD wPort);
# 96 "../../kernel/include/kapi.h" 2






# 1 "../../kernel/include/iomgr.h" 1
# 51 "../../kernel/include/iomgr.h"
typedef struct tag__PARTITION_EXTENSION{
    BYTE BootIndicator;
    BYTE PartitionType;
    DWORD dwStartSector;
 DWORD dwSectorNum;
 DWORD dwCurrPos;

 int nDiskNum;
}__PARTITION_EXTENSION;






typedef struct tag__FILE_TIME{
    DWORD dwHighDateTime;
    DWORD dwLowDateTime;
}__FILE_TIME;


typedef struct tagFS_FIND_DATA{
    DWORD dwFileAttribute;
    __FILE_TIME ftCreationTime;
 __FILE_TIME ftLastAccessTime;
 __FILE_TIME ftLastWriteTime;
 DWORD nFileSizeHigh;
 DWORD nFileSizeLow;
 DWORD dwReserved0;
 DWORD dwReserved1;
 CHAR cFileName[256];
 CHAR cAlternateFileName[13];
}FS_FIND_DATA;
# 107 "../../kernel/include/iomgr.h"
typedef DWORD (*DRCB_WAITING_ROUTINE)(__COMMON_OBJECT*);
typedef DWORD (*DRCB_COMPLETION_ROUTINE)(__COMMON_OBJECT*);
typedef DWORD (*DRCB_CANCEL_ROUTINE)(__COMMON_OBJECT*);


typedef struct tag__DRCB{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 __EVENT* lpSynObject;
    __KERNEL_THREAD_OBJECT* lpKernelThread;


 DWORD dwStatus;
 DWORD dwRequestMode;
 DWORD dwCtrlCommand;




 DWORD dwOutputLen;
 LPVOID lpOutputBuffer;

 DWORD dwInputLen;
 LPVOID lpInputBuffer;

 struct tag__DRCB* lpNext;
 struct tag__DRCB* lpPrev;

 DRCB_WAITING_ROUTINE WaitForCompletion;

 DRCB_COMPLETION_ROUTINE OnCompletion;

 DRCB_CANCEL_ROUTINE OnCancel;


 DWORD dwExtraParam1;
 DWORD dwExtraParam2;
 LPVOID lpDrcbExtension;
}__DRCB;
# 154 "../../kernel/include/iomgr.h"
BOOL DrcbInitialize(__COMMON_OBJECT*);
VOID DrcbUninitialize(__COMMON_OBJECT*);
# 202 "../../kernel/include/iomgr.h"
typedef struct tag__SECTOR_INPUT_INFO{
    DWORD dwStartSector;
    DWORD dwBufferLen;
 LPVOID lpBuffer;
}__SECTOR_INPUT_INFO;







typedef struct tag__RESOURCE_DESCRIPTOR{
    struct tag__RESOURCE_DESCRIPTOR* lpNext;
    struct tag__RESOURCE_DESCRIPTOR* lpPrev;

    DWORD dwStartPort;
    DWORD dwEndPort;
 DWORD dwDmaChannel;
 DWORD dwInterrupt;
 LPVOID lpMemoryStartAddr;
 DWORD dwMemoryLen;
}__RESOURCE_DESCRIPTOR;






typedef struct tag__DRIVER_OBJECT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);

 struct tag__DRIVER_OBJECT* lpPrev;
    struct tag__DRIVER_OBJECT* lpNext;

 DWORD (*DeviceRead)(__COMMON_OBJECT* lpDrv,
                              __COMMON_OBJECT* lpDev,
         __DRCB* lpDrcb);

    DWORD (*DeviceWrite)(__COMMON_OBJECT* lpDrv,
                            __COMMON_OBJECT* lpDev,
          __DRCB* lpDrcb);
 DWORD (*DeviceSize)(__COMMON_OBJECT* lpDrv,
                           __COMMON_OBJECT* lpDev,
                           __DRCB* lpDrcb);

 DWORD (*DeviceCtrl)(__COMMON_OBJECT* lpDrv,
                           __COMMON_OBJECT* lpDev,
         __DRCB* lpDrcb);

 DWORD (*DeviceFlush)(__COMMON_OBJECT* lpDrv,
                            __COMMON_OBJECT* lpDev,
          __DRCB* lpDrcb);

 DWORD (*DeviceSeek)(__COMMON_OBJECT* lpDrv,
                           __COMMON_OBJECT* lpDev,
         __DRCB* lpDrcb);

 __COMMON_OBJECT* (*DeviceOpen)(__COMMON_OBJECT* lpDrv,
                           __COMMON_OBJECT* lpDev,
         __DRCB* lpDrcb);

 DWORD (*DeviceClose)(__COMMON_OBJECT* lpDrv,
                            __COMMON_OBJECT* lpDev,
          __DRCB* lpDrcb);

 DWORD (*DeviceCreate)(__COMMON_OBJECT* lpDrv,
                             __COMMON_OBJECT* lpDev,
           __DRCB* lpDrcb);

 DWORD (*DeviceDestroy)(__COMMON_OBJECT* lpDrv,
                              __COMMON_OBJECT* lpDev,
         __DRCB* lpDrcb);
}__DRIVER_OBJECT;





BOOL DrvObjInitialize(__COMMON_OBJECT*);
VOID DrvObjUninitialize(__COMMON_OBJECT*);







typedef struct tag__DEVICE_OBJECT{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);

 DWORD dwSignature;
 struct tag__DEVICE_OBJECT* lpPrev;
    struct tag__DEVICE_OBJECT* lpNext;

 CHAR DevName[256 + 1];
 DWORD dwAttribute;
 DWORD dwBlockSize;
 DWORD dwMaxReadSize;
 DWORD dwMaxWriteSize;

 __DRIVER_OBJECT* lpDriverObject;
 DWORD dwTotalReadSector;
 DWORD dwTotalWrittenSector;
 DWORD dwInterrupt;

 LPVOID lpDevExtension;
}__DEVICE_OBJECT;





BOOL DevObjInitialize(__COMMON_OBJECT*);
VOID DevObjUninitialize(__COMMON_OBJECT*);
# 349 "../../kernel/include/iomgr.h"
typedef BOOL (*__DRIVER_ENTRY)(__DRIVER_OBJECT*);
# 359 "../../kernel/include/iomgr.h"
typedef struct{
 BYTE FileSystemIdentifier;
 __COMMON_OBJECT* pFileSystemObject;
 DWORD dwAttribute;
 BYTE VolumeLbl[13];
}__FS_ARRAY_ELEMENT;




typedef struct tag__IO_MANAGER{
    __DEVICE_OBJECT* lpDeviceRoot;
    __DRIVER_OBJECT* lpDriverRoot;
 __FS_ARRAY_ELEMENT FsArray[16];
 __COMMON_OBJECT* FsCtrlArray[4];

 __RESOURCE_DESCRIPTOR* lpResDescriptor;

 BOOL (*Initialize)(__COMMON_OBJECT* lpThis);




 __COMMON_OBJECT* (*CreateFile)(__COMMON_OBJECT* lpThis,
                                 LPSTR lpszFileName,
            DWORD dwAccessMode,
            DWORD dwShareMode,
            LPVOID lpReserved);

 BOOL (*ReadFile)(__COMMON_OBJECT* lpThis,
                               __COMMON_OBJECT* lpFileObject,
          DWORD dwByteSize,
          LPVOID lpBuffer,
          DWORD* lpReadSize);

 BOOL (*WriteFile)(__COMMON_OBJECT* lpThis,
                                __COMMON_OBJECT* lpFileObject,
           DWORD dwWriteSize,
           LPVOID lpBuffer,
           DWORD* lpWrittenSize);

 VOID (*CloseFile)(__COMMON_OBJECT* lpThis,
                                __COMMON_OBJECT* lpFileObject);

 BOOL (*CreateDirectory)(__COMMON_OBJECT* lpThis,
                                      LPCTSTR lpszFileName,
           LPVOID lpReserved);

 BOOL (*DeleteFile)(__COMMON_OBJECT* lpThis,
                                 LPCTSTR lpszFileName);

 BOOL (*FindClose)(__COMMON_OBJECT* lpThis,
                                LPCTSTR lpszFileName,
                                __COMMON_OBJECT* FindHandle);

 __COMMON_OBJECT* (*FindFirstFile)(__COMMON_OBJECT* lpThis,
                                    LPCTSTR lpszFileName,
            FS_FIND_DATA* pFindData);

 BOOL (*FindNextFile)(__COMMON_OBJECT* lpThis,
                                   LPCTSTR lpszFileName,
                                   __COMMON_OBJECT* FindHandle,
           FS_FIND_DATA* pFindData);

 DWORD (*GetFileAttributes)(__COMMON_OBJECT* lpThis,
                                        LPCTSTR lpszFileName);

 DWORD (*GetFileSize)(__COMMON_OBJECT* lpThis,
                                  __COMMON_OBJECT* FileHandle,
          DWORD* lpdwSizeHigh);

 BOOL (*RemoveDirectory)(__COMMON_OBJECT* lpThis,
                                      LPCTSTR lpszFileName);

 BOOL (*SetEndOfFile)(__COMMON_OBJECT* lpThis,
                                   __COMMON_OBJECT* FileHandle);

 BOOL (*IOControl)(__COMMON_OBJECT* lpThis,
                                __COMMON_OBJECT* lpFileObject,
           DWORD dwCommand,
           DWORD dwInputLen,
           LPVOID lpInputBuffer,
           DWORD dwOutputLen,
           LPVOID lpOutputBuffer,
           DWORD* lpdwOutFilled);

 DWORD (*SetFilePointer)(__COMMON_OBJECT* lpThis,
                                     __COMMON_OBJECT* lpFileObject,
             DWORD* pdwDistLow,
             DWORD* pdwDistHigh,
             DWORD dwWhereBegin);

 BOOL (*FlushFileBuffers)(__COMMON_OBJECT* lpThis,
                                       __COMMON_OBJECT* lpFileObject);


 __DEVICE_OBJECT* (*CreateDevice)(__COMMON_OBJECT* lpThis,
                                   LPSTR lpszDevName,
           DWORD dwAttribute,
           DWORD dwBlockSize,
           DWORD dwMaxReadSize,
           DWORD dwMaxWriteSize,
           LPVOID lpDevExtension,
           __DRIVER_OBJECT* lpDrvObject);

 VOID (*DestroyDevice)(__COMMON_OBJECT* lpThis,
                                    __DEVICE_OBJECT* lpDevObj);

 BOOL (*ReserveResource)(__COMMON_OBJECT* lpThis,
                                      __RESOURCE_DESCRIPTOR* lpResDesc);

 BOOL (*LoadDriver)(__DRIVER_ENTRY DrvEntry);


 BOOL (*AddFileSystem)(__COMMON_OBJECT* lpThis,
                                    __COMMON_OBJECT* lpFileSystem,
            DWORD dwAttribute,
            CHAR* pVolumeLbl);
 BOOL (*RegisterFileSystem)(__COMMON_OBJECT* lpThis,
                                         __COMMON_OBJECT* lpFileSystem);
}__IO_MANAGER;
# 510 "../../kernel/include/iomgr.h"
extern __IO_MANAGER IOManager;
# 103 "../../kernel/include/kapi.h" 2



# 1 "../../kernel/include/../include/debug.h" 1
# 22 "../../kernel/include/../include/debug.h"
typedef struct LOG_MESSAGE{
 int code;
 int len;
 int time;
 int pid;
 int tid;

 int format;



 char name[32];
 char tag[32];
 char msg[32];
}__LOG_MESSAGE;

typedef struct BUFFER_QUEUE
{
 __LOG_MESSAGE BufferQueue[256];
 int head;
 int tail;
 int len;

 void (*Enqueue)(struct BUFFER_QUEUE *pThis, __LOG_MESSAGE *pMsg);
 __LOG_MESSAGE *(*Dequeue)(struct BUFFER_QUEUE *pThis);

}__BUFFER_QUEUE;

typedef struct DEBUG_MANAGER{

 __MUTEX *pMutexForBufferQueue;

 __BUFFER_QUEUE *pBufferQueue;


 __MUTEX *pMutexForKRNLBufferQueue;
 __BUFFER_QUEUE *pKRNLBufferQueue;

 void (*Log)(struct DEBUG_MANAGER *pThis, char *tag, char *msg);
 void (*Logk)(struct DEBUG_MANAGER *pThis, char *tag, char *msg);
 void (*Logcat)(struct DEBUG_MANAGER *pThis, char *buf, int len);
 void (*Initialize)(struct DEBUG_MANAGER *pThis);
 void (*Unintialize)(struct DEBUG_MANAGER *pThis);
}__DEBUG_MANAGER;

extern __DEBUG_MANAGER DebugManager;
# 107 "../../kernel/include/kapi.h" 2







typedef __COMMON_OBJECT* HANDLE;


typedef __KERNEL_THREAD_MESSAGE MSG;


HANDLE CreateKernelThread(DWORD dwStackSize,
        DWORD dwInitStatus,
        DWORD dwPriority,
        __KERNEL_THREAD_ROUTINE lpStartRoutine,
        LPVOID lpRoutineParam,
        LPVOID lpReserved,
        LPSTR lpszName);


VOID DestroyKernelThread(HANDLE hThread);


DWORD SetLastError(DWORD dwNewError);


DWORD GetLastError(void);


DWORD GetThreadID(HANDLE hThread);


DWORD SetThreadPriority(HANDLE hThread,DWORD dwPriority);


BOOL GetMessage(MSG* lpMsg);


BOOL SendMessage(HANDLE hThread,MSG* lpMsg);




BOOL EnableSuspend(HANDLE hThread,BOOL bEnable);


BOOL SuspendKernelThread(HANDLE hThread);


BOOL ResumeKernelThread(HANDLE hThread);


BOOL Sleep(DWORD dwMillionSecond);


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


HANDLE CreateMutex(void);


VOID DestroyMutex(HANDLE hMutex);



DWORD ReleaseMutex(HANDLE hEvent);


DWORD WaitForThisObject(HANDLE hObject);


DWORD WaitForThisObjectEx(HANDLE hObject,DWORD dwMillionSecond);


HANDLE CreateCondition(DWORD condAttr);


DWORD WaitCondition(HANDLE hCond,HANDLE hMutex);


DWORD TimeoutWaitCondition(HANDLE hCond,HANDLE hMutex,DWORD dwMillionSecond);


DWORD SignalCondition(HANDLE hCond);


DWORD BroadcastCondition(HANDLE hCond);


VOID DestroyCondition(HANDLE hCond);



HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
      LPVOID lpHandlerParam,
      UCHAR ucVector);


VOID DisconnectInterrupt(HANDLE hInterrupt);


LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
     DWORD dwSize,
     DWORD dwAllocateFlags,
     DWORD dwAccessFlags,
     UCHAR* lpszRegName);


VOID VirtualFree(LPVOID lpVirtualAddr);


HANDLE CreateFile(LPSTR lpszFileName,
      DWORD dwAccessMode,
      DWORD dwShareMode,
      LPVOID lpReserved);


BOOL ReadFile(HANDLE hFile,
     DWORD dwReadSize,
     LPVOID lpBuffer,
     DWORD* lpdwReadSize);


BOOL WriteFile(HANDLE hFile,
      DWORD dwWriteSize,
      LPVOID lpBuffer,
      DWORD* lpdwWrittenSize);


VOID CloseFile(HANDLE hFile);


BOOL CreateDirectory(LPSTR lpszDirName);


BOOL DeleteFile(LPSTR lpszFileName);



HANDLE FindFirstFile(LPSTR lpszDirName,
      FS_FIND_DATA* pFindData);


BOOL FindNextFile(LPSTR lpszDirName,
      HANDLE hFindHandle,
      FS_FIND_DATA* pFindData);


VOID FindClose(LPSTR lpszDirName,
      HANDLE hFindHandle);


DWORD GetFileAttributes(LPSTR lpszFileName);



DWORD GetFileSize(HANDLE hFile,DWORD* lpdwSizeHigh);


BOOL RemoveDirectory(LPSTR lpszDirName);


BOOL SetEndOfFile(HANDLE hFile);


BOOL IOControl(HANDLE hFile,
      DWORD dwCommand,
      DWORD dwInputLen,
      LPVOID lpInputBuffer,
      DWORD dwOutputLen,
      LPVOID lpOutputBuffer,
      DWORD* lpdwFilled);


DWORD SetFilePointer(HANDLE hFile,
     DWORD* lpdwDistLow,
     DWORD* lpdwDistHigh,
     DWORD dwMoveFlags);


BOOL FlushFileBuffers(HANDLE hFile);


HANDLE CreateDevice(LPSTR lpszDevName,
     DWORD dwAttributes,
     DWORD dwBlockSize,
     DWORD dwMaxReadSize,
     DWORD dwMaxWriteSize,
     LPVOID lpDevExtension,
     __DRIVER_OBJECT* lpDrvObject);


VOID DestroyDevice(HANDLE hDevice);


LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType);


VOID KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength);


HANDLE CreateRingBuff(DWORD dwBuffLength);
BOOL GetRingBuffElement(HANDLE hRb,DWORD* lpdwElement,DWORD dwMillionSecond);
BOOL AddRingBuffElement(HANDLE hRb,DWORD dwElement);
BOOL SetRingBuffLength(HANDLE hRb,DWORD dwNewLength);
VOID DestroyRingBuff(HANDLE hRb);







void Log(char *tag, char *msg);
void Logk(char *tag, char *msg);
# 39 "../../kernel/include/process.h" 2
# 1 "../../kernel/include/syscall.h" 1
# 30 "../../kernel/include/syscall.h"
typedef struct SYSCALL_PARAM_BLOCK{
 DWORD ebp;
 DWORD edi;
 DWORD esi;
 DWORD edx;
 DWORD ecx;
 DWORD ebx;
 DWORD eax;
 DWORD eip;
 DWORD cs;
 DWORD eflags;
 DWORD dwSyscallNum;
 LPVOID lpRetValue;
 LPVOID lpParams[1];
}__SYSCALL_PARAM_BLOCK;



typedef BOOL (*__SYSCALL_DISPATCH_ENTRY)(LPVOID,LPVOID);



typedef struct SYSCALL_RANGE{
 DWORD dwStartSyscallNum;
 DWORD dwEndSyscallNum;
 __SYSCALL_DISPATCH_ENTRY sde;
}__SYSCALL_RANGE;
# 65 "../../kernel/include/syscall.h"
BOOL RegisterSystemCall(DWORD dwStartNum,DWORD dwEndNum,
      __SYSCALL_DISPATCH_ENTRY sde);
# 126 "../../kernel/include/syscall.h"
BOOL SyscallHandler(LPVOID lpEsp,LPVOID);
# 40 "../../kernel/include/process.h" 2


typedef struct tag__KOBJ_LIST_NODE{
 __COMMON_OBJECT* pKernelObject;
 struct tag__KOBJ_LIST_NODE* prev;
 struct tag__KOBJ_LIST_NODE* next;
} __KOBJ_LIST_NODE;



typedef struct tag__PROCESS_OBJECT{
 DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;


 UCHAR ProcessName[32 + 1];


 DWORD dwProcessStatus;


 __KERNEL_THREAD_ROUTINE lpMainStartRoutine;
 LPVOID lpMainRoutineParam;






    __KOBJ_LIST_NODE KernelThreadList;
 volatile int nKernelThreadNum;


 __KERNEL_THREAD_OBJECT* lpMainKernelThread;



 volatile DWORD dwTLSFlags;
}__PROCESS_OBJECT;







BOOL ProcessInitialize(__COMMON_OBJECT* lpThis);
VOID ProcessUninitialize(__COMMON_OBJECT* lpThis);



typedef struct tag__PROCESS_MANAGER{

 __KOBJ_LIST_NODE ProcessList;


 __PROCESS_OBJECT* (*CreateProcess)(
                                        __COMMON_OBJECT* lpThis,
             DWORD dwMainThreadStackSize,
             DWORD dwMainThreadPriority,
             __KERNEL_THREAD_ROUTINE lpMainStartRoutine,
             LPVOID lpMainRoutineParam,
             LPVOID lpReserved,
             LPSTR lpszName);
 VOID (*DestroyProcess)(
                                        __COMMON_OBJECT* lpThis,
             __COMMON_OBJECT* lpProcess);


 BOOL (*LinkKernelThread)(
                                        __COMMON_OBJECT* lpThis,
                                        __COMMON_OBJECT* lpOwnProcess,
             __COMMON_OBJECT* lpKernelThread
             );
 BOOL (*UnlinkKernelThread)(
                                        __COMMON_OBJECT* lpThis,
             __COMMON_OBJECT* lpOwnProcess,
             __COMMON_OBJECT* lpKernelThread);


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

    __PROCESS_OBJECT* (*GetCurrentProcess)(__COMMON_OBJECT* lpThis);


 BOOL (*Initialize)(__COMMON_OBJECT* lpThis);

}__PROCESS_MANAGER;


extern __PROCESS_MANAGER ProcessManager;


VOID DumpProcess();
# 79 "../../kernel/include/StdAfx.h" 2
# 90 "../../kernel/include/StdAfx.h"
# 1 "../../kernel/include/devmgr.h" 1
# 29 "../../kernel/include/devmgr.h"
typedef struct{
 DWORD dwBusType;
 union{
  struct{
   UCHAR ucMask;
   WORD wVendor;
   WORD wDevice;
   DWORD dwClass;
   UCHAR ucHdrType;
   WORD wReserved;
  }PCI_Identifier;

  struct{
   DWORD dwDevice;
  }ISA_Identifier;



 }Bus_ID;

}__IDENTIFIER;




typedef struct tag__RESOURCE{
 struct tag__RESOURCE* lpNext;
 struct tag__RESOURCE* lpPrev;
 DWORD dwResType;
 union{
  struct{
   WORD wStartPort;
   WORD wEndPort;
  }IOPort;

  struct{
   LPVOID lpStartAddr;
   LPVOID lpEndAddr;
  }MemoryRegion;

  UCHAR ucVector;
 }Dev_Res;
}__RESOURCE;
# 83 "../../kernel/include/devmgr.h"
struct __PHYSICAL_DEVICE;
struct __SYSTEM_BUS;







struct tag__PHYSICAL_DEVICE{
 __IDENTIFIER DevId;
 UCHAR strName[32];
 __RESOURCE Resource[7];
 struct tag__PHYSICAL_DEVICE* lpNext;

 struct tag__SYSTEM_BUS* lpHomeBus;
 struct tag__SYSTEM_BUS* lpChildBus;
 LPVOID lpPrivateInfo;


};







struct tag__SYSTEM_BUS{
 struct tag__SYSTEM_BUS* lpParentBus;
 struct tag__PHYSICAL_DEVICE* lpDevListHdr;
 struct tag__PHYSICAL_DEVICE* lpHomeBridge;
 __RESOURCE Resource;

 DWORD dwBusNum;
 DWORD dwBusType;

};

typedef struct tag__PHYSICAL_DEVICE __PHYSICAL_DEVICE;
typedef struct tag__SYSTEM_BUS __SYSTEM_BUS;
# 138 "../../kernel/include/devmgr.h"
typedef struct tag__DEVICE_MANAGER{
 __SYSTEM_BUS SystemBus[16];
 __RESOURCE FreePortResource;

 __RESOURCE UsedPortResource;

 BOOL (*Initialize)(struct tag__DEVICE_MANAGER*);
 __PHYSICAL_DEVICE* (*GetDevice)(struct tag__DEVICE_MANAGER*,
                                        DWORD dwBusType,
             __IDENTIFIER* lpIdentifier,
             __PHYSICAL_DEVICE* lpStart);

 BOOL (*AppendDevice)(struct tag__DEVICE_MANAGER*,
                                           __PHYSICAL_DEVICE*);

 VOID (*DeleteDevice)(struct tag__DEVICE_MANAGER*,
                                           __PHYSICAL_DEVICE*);

 BOOL (*CheckPortRegion)(struct tag__DEVICE_MANAGER*,
                                              __RESOURCE*);

 BOOL (*ReservePortRegion)(struct tag__DEVICE_MANAGER*,
                                                __RESOURCE*);

 VOID (*ReleasePortRegion)(struct tag__DEVICE_MANAGER*,
                                                __RESOURCE*);


}__DEVICE_MANAGER;
# 177 "../../kernel/include/devmgr.h"
extern __DEVICE_MANAGER DeviceManager;
# 91 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/mailbox.h" 1
# 9 "../../kernel/include/mailbox.h"
typedef struct tag__MAILBOX{
    DWORD dwObjectType; DWORD dwObjectID; DWORD dwObjectSize; __COMMON_OBJECT* lpPrevObject; __COMMON_OBJECT* lpNextObject; __COMMON_OBJECT* lpObjectOwner; BOOL (*Initialize)(__COMMON_OBJECT*); VOID (*Uninitialize)(__COMMON_OBJECT*);
 DWORD (*WaitForThisObject)(__COMMON_OBJECT*); DWORD dwObjectSignature;
 __PRIORITY_QUEUE* lpGettingQueue;
    __PRIORITY_QUEUE* lpSendingQueue;
    LPVOID lpMsg;
 DWORD (*GetMail)(__COMMON_OBJECT*,LPVOID*,DWORD);
 DWORD (*SendMail)(__COMMON_OBJECT*,LPVOID,DWORD);
}__MAILBOX;







BOOL MailBoxInitialize(__COMMON_OBJECT*);
VOID MailBoxUninitialize(__COMMON_OBJECT*);
# 95 "../../kernel/include/StdAfx.h" 2
# 114 "../../kernel/include/StdAfx.h"
# 1 "../../kernel/include/heap.h" 1
# 115 "../../kernel/include/StdAfx.h" 2







# 1 "../../kernel/include/buffmgr.h" 1
# 47 "../../kernel/include/buffmgr.h"
typedef struct tag__BUFFER_CONTROL_BLOCK{
 volatile DWORD dwFlags;
 volatile LPVOID lpPoolStartAddress;
 volatile DWORD dwPoolSize;
 volatile DWORD dwFreeSize;
 volatile DWORD dwFreeBlocks;
 volatile DWORD dwAllocTimesH;
 volatile DWORD dwAllocTimesL;
 volatile DWORD dwAllocTimesSuccH;
 volatile DWORD dwAllocTimesSuccL;
 volatile DWORD dwFreeTimesH;
 volatile DWORD dwFreeTimesL;
 volatile LPVOID lpBuffExtension;

 DWORD (*GetControlBlockFlag)(struct tag__BUFFER_CONTROL_BLOCK*);
 struct tag__FREE_BUFFER_HEADER* lpFreeBufferHeader;


 BOOL (*Initialize)(struct tag__BUFFER_CONTROL_BLOCK* pControlBlock);
 BOOL (*InitializeBuffer)(struct tag__BUFFER_CONTROL_BLOCK* pControlBlock);


 BOOL (*CreateBuffer1)(struct tag__BUFFER_CONTROL_BLOCK*,DWORD);
 BOOL (*CreateBuffer2)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
 VOID (*AppendBuffer)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
 LPVOID (*Allocate)(struct tag__BUFFER_CONTROL_BLOCK*,DWORD);
 VOID (*Free)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID);
 DWORD (*GetBufferFlag)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID);
 BOOL (*SetBufferFlag)(struct tag__BUFFER_CONTROL_BLOCK*,LPVOID,DWORD);
 VOID (*DestroyBuffer)(struct tag__BUFFER_CONTROL_BLOCK*);
} __BUFFER_CONTROL_BLOCK;
# 88 "../../kernel/include/buffmgr.h"
typedef struct tag__FREE_BUFFER_HEADER{
 DWORD dwFlags;
 DWORD dwBlockSize;
 struct tag__FREE_BUFFER_HEADER* lpNextBlock;
 struct tag__FREE_BUFFER_HEADER* lpPrevBlock;
}__FREE_BUFFER_HEADER;

typedef struct tag__USED_BUFFER_HEADER{
 DWORD dwFlags;
 DWORD dwBlockSize;
 DWORD dwReserved1;
 DWORD dwReserved2;
}__USED_BUFFER_HEADER;




typedef struct{
 LPVOID lpBuffer;
 DWORD dwBufferSize;
}__MEMORY_REGION;






extern __BUFFER_CONTROL_BLOCK AnySizeBuffer;
# 123 "../../kernel/include/StdAfx.h" 2







# 1 "../../kernel/include/ktmsg.h" 1
# 131 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/sysnet.h" 1
# 23 "../../kernel/include/sysnet.h"
typedef BOOL (*__NETWORK_PROTOCOL_ENTRY)(VOID* pArg);




BOOL IPv4_Entry(VOID* pArg);
# 135 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/globvar.h" 1
# 24 "../../kernel/include/globvar.h"
extern __KERNEL_THREAD_OBJECT* g_lpShellThread;


extern CHAR HostName[16];
# 139 "../../kernel/include/StdAfx.h" 2



# 1 "../../kernel/include/chardisplay.h" 1
# 32 "../../kernel/include/chardisplay.h"
VOID CD_InitDisplay(INT nDisplayMode);


VOID CD_SetDisplayMode(INT nMode);


VOID CD_GetDisPlayRang(WORD* pLines,WORD* pColums);


VOID CD_ChangeLine();


VOID CD_GetCursorPos(WORD* pCursorX,WORD* pCursorY);


VOID CD_SetCursorPos(WORD nCursorX,WORD nCursorY);


VOID CD_PrintString(LPSTR pStr,BOOL cl);


VOID CD_PrintChar(CHAR ch);


VOID CD_GetString(WORD nCursorX,WORD nCursorY,LPSTR pString,INT nBufLen);


VOID CD_DelString(WORD nCursorX,WORD nCursorY,INT nDelLen);


VOID CD_DelChar(INT nDelMode);


VOID CD_Clear();
# 143 "../../kernel/include/StdAfx.h" 2
# 154 "../../kernel/include/StdAfx.h"
# 1 "../../kernel/config.h" 1
# 155 "../../kernel/include/StdAfx.h" 2
# 20 "mem_scat.c" 2


__MEMORY_REGION SystemMemRegion[] = {

 {(LPVOID)0x00C00000,0x00100000},
 {(LPVOID)(0x00C00000 + 0x00100000),0x00100000},
 {(LPVOID)(0x00C00000 + 0x00200000),0x00100000},
 {(LPVOID)(0x00C00000 + 0x00300000),0x00100000},


 {((void*)0x00000000),0}
};
