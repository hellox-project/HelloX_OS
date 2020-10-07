//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apr 14, 2019
//    Module Name               : hellox.h
//    Module Funciton           : 
//                                Header file for HelloX operationg system's
//                                API,applications for HelloX should include
//                                this file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __HELLOX_H__
#define __HELLOX_H__

/* Basic data types. */
typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef short WCHAR;
typedef char* LPSTR;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef signed short SHORT;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long LONG;
typedef long* LPLONG;
typedef void* LPVOID;
typedef void VOID;

/* uintxx_t. */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

/* intxx_t. */
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

/* __ux types. */
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned long __u32;
typedef unsigned long long __u64;

/* atomic int type. */
typedef volatile unsigned int __atomic_t;

/* All kernel objects are refered by handle. */
typedef LPVOID HANDLE;

/* Definition of standard size_t type. */
#ifndef __SIZE_T_DEFINED__
#define __SIZE_T_DEFINED__
typedef unsigned int size_t;
#endif

/* BOOL type. */
typedef enum {
	FALSE = 0x00,
	TRUE = 0x01
}BOOL;

/* NULL pointer or handle. */
#define NULL ((void*)0x00)

/* Constants. */
#define MAX_DWORD_VALUE    0xFFFFFFFF
#define MAX_FILE_NAME_LEN  512

/* Maximal X and Y dimension of mouse. */
#define MOUSE_DIM_X   511
#define MOUSE_DIM_Y   511

/* Virtual key values. */
#define ASCII_NUT     0
#define ASCII_SOH     1
#define ASCII_STX     2
#define ASCII_ETX     3
#define ASCII_EOT     4
#define ASCII_ENQ     5
#define ASCII_ACK     6
#define ASCII_BEL     7
#define ASCII_BS      8
#define ASCII_HT      9
#define ASCII_LF      10
#define ASCII_VT      11
#define ASCII_FF      12
#define ASCII_CR      13
#define ASCII_SO      14
#define ASCII_SI      15
#define ASCII_DLE     16
#define ASCII_DC1     17
#define ASCII_DC2     18
#define ASCII_DC3     19
#define ASCII_DC4     20
#define ASCII_NAK     21
#define ASCII_SYN     22
#define ASCII_TB      23
#define ASCII_CAN     24
#define ASCII_EM      25
#define ASCII_SUB     26
#define ASCII_ESC     27
#define ASCII_FS      28
#define ASCII_GS      29
#define ASCII_RS      30
#define ASCII_US      31

#define ASCII_SPACE           32
#define ASCII_EXCALMARK       33   //!
#define ASCII_QUOTATIONMARK   34   //"
#define ASCII_POUND           35   //#
#define ASCII_DOLLAR          36   //$
#define ASCII_PERCENT         37   //%
#define ASCII_QUOTA           38   //&
#define ASCII_MINUTE          39   //'
#define ASCII_LEFT_BRACKET    40   //(
#define ASCII_RIGHT_BRACKET   41   //)
#define ASCII_STAR            42   //*
#define ASCII_ADD             43   //+
#define ASCII_COMMA           44   //,
#define ASCII_SUBSTRACT       45   //-
#define ASCII_DOT             46   //.
#define ASCII_REVERSE_SLASH   47   //'/'

#define ASCII_0               48
#define ASCII_1               49
#define ASCII_2               50
#define ASCII_3               51
#define ASCII_4               52
#define ASCII_5               53
#define ASCII_6               54
#define ASCII_7               55
#define ASCII_8               56
#define ASCII_9               57
#define ASCII_COLON           58   //:
#define ASCII_SEMICOLON       59   //;
#define ASCII_LESS            60   //<
#define ASCII_EQUAL           61   //=
#define ASCII_GREATER         62   //>
#define ASCII_ASK             63   //?
#define ASCII_AT              64   //@

/*
 * Virtual keys defined by OS kernel to handle different 
 * physical key boards.
 */
#define VK_ESC             27
#define VK_RETURN          13
#define VK_TAB             9
#define VK_CAPS_LOCK       20
#define VK_SHIFT           10
#define VK_CONTROL         17
#define VK_MENU            18
#define VK_BACKSPACE       8
#define VK_LWIN            91
#define VK_RWIN            92
#define VK_INSERT          45
#define VK_HOME            36
#define VK_PAGEUP          33
#define VK_PAGEDOWN        34
#define VK_END             35
#define VK_DELETE          46
#define VK_LEFTARROW       37
#define VK_UPARROW         38
#define VK_RIGHTARROW      39
#define VK_DOWNARROW       40
/* Alt key,defined by HelloX. */
#define VK_ALT             5

/* Functions keys. */
#define VK_F1              112
#define VK_F2              113
#define VK_F3              114
#define VK_F4              115
#define VK_F5              116
#define VK_F6              117
#define VK_F7              118
#define VK_F8              119
#define VK_F9              120
#define VK_F10             121
#define VK_F11             122
#define VK_F12             123

#define VK_NUMLOCK         144
#define VK_NUMPAD0         96
#define VK_NUMPAD1         97
#define VK_NUMPAD2         98
#define VK_NUMPAD3         99
#define VK_NUMPAD4         100
#define VK_NUMPAD5         101
#define VK_NUMPAD6         102
#define VK_NUMPAD7         103
#define VK_NUMPAD8         104
#define VK_NUMPAD9         105
#define VK_DECIMAL         110
#define VK_NUMPADMULTIPLY  106
#define VK_NUMPADADD       107
#define VK_NUMPADSUBSTRACT 109
#define VK_NUMPADDIVIDE    111

#define VK_PAUSE           19
#define VK_SCROLL          145
#define VK_PRINTSC         6

/*
 * Extension parameter block of system call to contain input
 * parameters, handles the scenario that has more than 5
 * parameters.
 * At most 16 extension parameters can be supported in current
 * version,it shoud be enough.
 */
typedef struct SYSCALL_PARAM_EXTENSION_BLOCK {
	uint32_t ext_param0;
	uint32_t ext_param1;
	uint32_t ext_param2;
	uint32_t ext_param3;
	uint32_t ext_param4;
	uint32_t ext_param5;
	uint32_t ext_param6;
	uint32_t ext_param7;
}__SYSCALL_PARAM_EXTENSION_BLOCK;

/*
 * System call parameter block,used to transfer parameters
 * between user mode and kernel mode.
 * It's a memory block just same as the stack frame established
 * when interrupt raise, ret_ptr contains a pointer to a variable
 * in user mode thread's stack that used to contain return value.
 * The stack frame after int(x) instruction as follow:
 * 	uint32_t ebp (return value's ptr)
 *  uint32_t edi (param0)
 *  uint32_t esi (param1)
 *  uint32_t edx (param2)
 *  uint32_t ecx (param3)
 *  uint32_t ebx (param4,extblock's ptr)
 *  uint32_t eax (syscall num)
 *  uint32_t eip (ret addr)
 *  uint32_t cs  (cs)
 *  uint32_t eflags
 *  ......
 *
 * At most 5 parameters under x86 can be transfered from user
 * to kernel directly,a dedicated parameter extension block
 * is allocated in caller(syscall's agent) stack to be used
 * contain ALL parameters if the syscall accepts more than
 * 5 input parameters,the parameter extension block is pointed
 * by param_4(ebx) in directly block.
 * Usage examples:
 * 1. If the syscall only has 5 or less parameters,just put
 *    all parameters into general registers except eax/ebp,
 *    and raise the system call;
 * 2. If the syscall has 6 or more parameters,then:
 *    2.1 Define a syscall parameter extension block in
 *        user thread's stack(local variable),set each
 *        parameter properly;
 *    2.2 Move the parameter extension block's base addr
 *        into ebx(directly param4),and raise the syscall;
 *    2.3 The handler of system call retrieve extension
 *        block from param_0,and get all input parameters
 *        from the block and invoke actual kernel mode routine.
 */
typedef struct SYSCALL_PARAM_BLOCK {
	union {
		uint32_t* param;
		uint32_t ebp;
	}ret_ptr;
	union {
		uint32_t param;
		uint32_t edi;
	}param_0;
	union {
		uint32_t param;
		uint32_t esi;
	}param_1;
	union {
		uint32_t param;
		uint32_t edx;
	}param_2;
	union {
		uint32_t param;
		uint32_t ecx;
	}param_3;
	union {
		uint32_t param;
		uint32_t ebx;
		__SYSCALL_PARAM_EXTENSION_BLOCK* pExtBlock;
	}param_4;
	uint32_t syscall_num;
}__SYSCALL_PARAM_BLOCK;

/* The vector number of system call under x86. */
#define SYSCALL_VECTOR 0x7F

/* System call IDs. */
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
#define SYSCALL_GETETHERNETINTERFACESTATE 0x37 //GetEthernetInterfaceState.
#define SYSCALL_GETSYSTEMTIME         0x38
#define SYSCALL_GOTOHOME              0x39
#define SYSCALL_CHANGELINE            0x3A
#define SYSCALL_GETCURSORPOS          0x3B
#define SYSCALL_SETCURSORPOS          0x3C
#define SYSCALL_TERMINATEKERNELTHREAD 0x3D
#define SYSCALL_GOTOPREV              0x3E
#define SYSCALL_PEEKMESSAGE           0x3F     //Peek the message queue.
#define SYSCALL_PROCESSHALFBOTTOM     0x40
#define SYSCALL_CREATEUSERTHREAD      0x41
#define SYSCALL_DESTROYUSERTHREAD     0x42
#define SYSCALL_TERMINATEUSERTHREAD   0x43
#define SYSCALL_GETSYSTEMINFO         0x46
#define SYSCALL_VIRTUALQUERY          0x47
#define SYSCALL_GETCURRENTTHREADID    0x48

#define SYSCALL_TESTPARAMXFER         0x49

 /* Syscall IDs for I/O subsystem. */
#define SYSCALL_CREATEFILE            0x101     //CreateFile.
#define SYSCALL_READFILE              0x102     //ReadFile.
#define SYSCALL_WRITEFILE             0x103     //WriteFile.
#define SYSCALL_CLOSEFILE             0x104     //CloseFile.
#define SYSCALL_CREATEDIRECTORY       0x105     //CreateDirectory.
#define SYSCALL_DELETEFILE            0x106     //DeleteFile.
#define SYSCALL_FINDFIRSTFILE         0x107     //FindFirstFile.
#define SYSCALL_FINDNEXTFILE          0x108     //FindNextFile.
#define SYSCALL_FINDCLOSE             0x109     //FindClose.
#define SYSCALL_GETFILEATTRIBUTES     0x10A     //GetFileAttributes.
#define SYSCALL_GETFILESIZE           0x10B     //GetFileSize.
#define SYSCALL_REMOVEDIRECTORY       0x10C     //RemoveDirectory.
#define SYSCALL_SETENDOFFILE          0x10D     //SetEndOfFile.
#define SYSCALL_IOCONTROL             0x10E     //IOControl.
#define SYSCALL_SETFILEPOINTER        0x10F     //SetFilePointer.
#define SYSCALL_FLUSHFILEBUFFERS      0x110     //FlushFileBuffers.
#define SYSCALL_CREATEDEVICE          0x111     //CreateDevice.
#define SYSCALL_DESTROYDEVICE         0x112     //DestroyDevice.

/* System call IDs for network subsystem. */
#define SYSCALL_SOCKET                0x200     //Socket
#define SYSCALL_BIND                  0x201     //bind
#define SYSCALL_CONNECT               0x202     //connect
#define SYSCALL_RECV                  0x203     //recv
#define SYSCALL_SEND                  0x204     //send
#define SYSCALL_WRITE                 0x205     //write
#define SYSCALL_SELECT                0x206     //select
#define SYSCALL_GETSOCKET             0x207     //getsocktopt
#define SYSCALL_SETSOCKET             0x208     //setsocketopt
#define SYSCALL_GETHOSTBYNAME         0x209     //gethostbyname
#define SYSCALL_ACCEPT                0x20A     //accept
#define SYSCALL_CLOSESOCKET           0x20B     //close socket
#define SYSCALL_LISTEN                0x20C     //listen
#define SYSCALL_RECVFROM              0x20D     //recv from
#define SYSCALL_SENDTO                0x20E     //send to
#define SYSCALL_GETGENIFINFO          0x20F     //GetGenifInfo
#define SYSCALL_ADDGENIFADDRESS       0x210     //AddGenifAddress
#define SYSCALL_GETNETWORKINFO        0x211     //GetNetworkInfo

/* Atomic operations to support user mode spinlock. */
long __InterlockedCompareExchange(long volatile* destination,
	long exchange,
	long comparand);
long __InterlockedExchange(long volatile* destination, long exchange);

/* Message between thread/kernel or thread/thread. */
typedef struct _MSG {
	unsigned short command;
	unsigned short wParam;
	unsigned long lParam;
}MSG;

/* File time. */
typedef struct {
	DWORD dwHighDateTime;
	DWORD dwLowDateTime;
} __FILE_TIME;

/* File's attributes that returned from kernel. */
typedef struct {
	DWORD dwFileAttribute;
	__FILE_TIME ftCreationTime;
	__FILE_TIME ftLastAccessTime;
	__FILE_TIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	CHAR cFileName[MAX_FILE_NAME_LEN];
	CHAR cAlternateFileName[13];
} FS_FIND_DATA;

/* Proto-type for thread's entry routine. */
typedef DWORD(*__THREAD_START_ROUTINE)(LPVOID);
/* Proto-type for interrupt handler. */
typedef BOOL(*__INTERRUPT_HANDLER)(LPVOID lpEsp, LPVOID);

/* Thread's status. */
#define THREAD_STATUS_RUNNING    0x00000001
#define THREAD_STATUS_READY      0x00000002
#define THREAD_STATUS_SUSPENDED  0x00000003
#define THREAD_STATUS_SLEEPING   0x00000004
#define THREAD_STATUS_TERMINAL   0x00000005
#define THREAD_STATUS_BLOCKED    0x00000006

/* Thread's priority level. */
#define PRIORITY_LEVEL_CRITICAL         0x00000010
#define PRIORITY_LEVEL_HIGH             0x0000000C
#define PRIORITY_LEVEL_NORMAL           0x00000008
#define PRIORITY_LEVEL_LOW              0x00000004
#define PRIORITY_LEVEL_IDLE             0x00000000

/* 
 * Create a new user thread. 
 * Note:
 *   The work routine specified by caller with lpStartRoutine,
 *   must call ExitThread at end of routine,to return to kernel
 *   explicity.
 */
HANDLE CreateUserThread(
	DWORD dwStatus,
	DWORD dwPriority,
	__THREAD_START_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	char* pszName);

VOID DestroyUserThread(HANDLE hThread);
DWORD GetThreadID(HANDLE hThread);
DWORD SetThreadPriority(HANDLE hThread, DWORD dwPriority);
HANDLE GetCurrentThread(void);
unsigned long GetCurrentThreadID();
unsigned long TerminateKernelThread(HANDLE hTarget, int status);

/* Must be called after a thread,to return to OS kernel. */
void ExitThread(int errCode);

/* Message types. */
/* ASCII key pressed down. */
#define KERNEL_MESSAGE_AKDOWN         1
/* ASCII key up. */
#define KERNEL_MESSAGE_AKUP           2
/* Virtual key pressed down. */
#define KERNEL_MESSAGE_VKDOWN         203
/* Virtual key up. */
#define KERNEL_MESSAGE_VKUP           204
/* Terminate message. */
#define KERNEL_MESSAGE_TERMINAL       5
/* Timer message. */
#define KERNEL_MESSAGE_TIMER          6
/* Left button down of mouse. */
#define KERNEL_MESSAGE_LBUTTONDOWN    301
/* Left button up of mouse. */
#define KERNEL_MESSAGE_LBUTTONUP      302
/* Right button down of mouse. */
#define KERNEL_MESSAGE_RBUTTONDOWN    303
/* Right button up of mouse. */
#define KERNEL_MESSAGE_RBUTTONUP      304
/* Left button double clicked. */
#define KERNEL_MESSAGE_LBUTTONDBCLK   305
/* Right button double clicked. */
#define KERNEL_MESSAGE_RBUTTONDBCLK   306
/* Mouse is moving. */
#define KERNEL_MESSAGE_MOUSEMOVE      307

/* Messages for GUI. */
#define KERNEL_MESSAGE_WINDOW         308
#define KERNEL_MESSAGE_DIALOG         309

/* Message operation. */
BOOL GetMessage(MSG* pMsg);
BOOL SendMessage(HANDLE hThread, MSG* lpMsg);
BOOL PeekMessage(MSG* pMsg);

/* Sleep a while. */
BOOL Sleep(DWORD dwMillionSecond);

/* Flags to control the SetTimer's action. */
#define TIMER_FLAGS_ONCE        0x00000001    //Set a timer with this flags,the timer only
											  //apply once,i.e,the kernel thread who set
											  //the timer can receive timer message only
											  //once.
#define TIMER_FLAGS_ALWAYS      0x00000002    //Set a timer with this flags,the timer will
											  //availiable always,only if the kernel thread
											  //cancel the timer by calling CancelTimer.
HANDLE SetTimer(DWORD dwTimerID,
	DWORD dwMillionSecond,
	LPVOID lpHandlerParam,
	DWORD dwTimerFlags);
VOID CancelTimer(HANDLE hTimer);
HANDLE CreateEvent(BOOL bInitialStatus);
VOID DestroyEvent(HANDLE hEvent);
DWORD SetEvent(HANDLE hEvent);
DWORD ResetEvent(HANDLE hEvent);

/* Mutex object operations. */
HANDLE CreateMutex(void);
VOID DestroyMutex(HANDLE hMutex);
DWORD ReleaseMutex(HANDLE hEvent);

/* Wait for kernel objects. */
DWORD WaitForThisObject(HANDLE hObject);
DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond);

/* Return values of WaitForThisObject/Ex. */
#define OBJECT_WAIT_FAILED           0x00000000
#define OBJECT_WAIT_RESOURCE         0x00000001
#define OBJECT_WAIT_TIMEOUT          0x00000002
#define OBJECT_WAIT_DELETED          0x00000004

/* Virtual area attributes. */
#define VIRTUAL_AREA_ACCESS_READ        0x00000001
#define VIRTUAL_AREA_ACCESS_WRITE       0x00000002
#define VIRTUAL_AREA_ACCESS_RW          (VIRTUAL_AREA_ACCESS_READ | VIRTUAL_AREA_ACCESS_WRITE)
#define VIRTUAL_AREA_ACCESS_EXEC        0x00000008
#define VIRTUAL_AREA_ACCESS_NOACCESS    0x00000010

/* Cache policy flags of virtual area. */
#define VIRTUAL_AREA_CACHE_NORMAL       0x00000001
#define VIRTUAL_AREA_CACHE_IO           0x00000002
#define VIRTUAL_AREA_CACHE_VIDEO        0x00000004

/*
 * Virtual area's allocate flags.
 * The meaning of these flags and their difference
 * could be illustrated as following table:
 *
 *             Reserve VAD  |  Reserve Page Tables  |  Reserve RAM pages
 * ---------------------------------------------------------------------
 * RESERVE    |    *        |         N/A           |        N/A
 * ---------------------------------------------------------------------
 * COMMIT     |    N/A      |         *             |         *
 * ---------------------------------------------------------------------
 * I/O        |    *        |         *             |        N/A
 * ---------------------------------------------------------------------
 * ALL        |    *        |         *             |         *
 * ---------------------------------------------------------------------
 * IOCOMMIT   |    *        |         *             |         *
 * ---------------------------------------------------------------------
 * MAP        |    *        |         *             |        N/A
 * ---------------------------------------------------------------------
 *
 * Notes:
 * 1. The combining effectiveness of RESERVE and COMMIT,is same as ALL flag;
 * 2. The difference between ALL and IOCOMMIT is,cache flag is disabled
 *    when set page tables for IOCOMMIT,but cache flag is keep enabled when
 *    allocates page tables for ALL;
 * 3. DEFAULT as alias of ALL flag.
 */
#define VIRTUAL_AREA_ALLOCATE_RESERVE   0x00000001
#define VIRTUAL_AREA_ALLOCATE_COMMIT    0x00000002
#define VIRTUAL_AREA_ALLOCATE_IO        0x00000004
#define VIRTUAL_AREA_ALLOCATE_ALL       0x00000008
#define VIRTUAL_AREA_ALLOCATE_IOCOMMIT  0x00000010
#define VIRTUAL_AREA_ALLOCATE_MAP       0x00000020
#define VIRTUAL_AREA_ALLOCATE_DEFAULT   VIRTUAL_AREA_ALLOCATE_ALL

/* Allocate or free a virtual area from memory space. */
LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
	DWORD  dwSize,
	DWORD  dwAllocateFlags,
	DWORD  dwAccessFlags,
	CHAR*  lpszRegName);
BOOL VirtualFree(LPVOID lpVirtualAddr);

/* Access flags for a file. */
#define FILE_ACCESS_READ         0x00000001
#define FILE_ACCESS_WRITE        0x00000002
#define FILE_ACCESS_READWRITE    0x00000003
#define FILE_ACCESS_CREATE       0x00000004

/* If can not open one,create a new one then open it. */
#define FILE_OPEN_ALWAYS         0x80000000
/* Create a new file,overwrite existing if. */
#define FILE_OPEN_NEW            0x40000000
/* Open a existing file,return fail if does not exist. */
#define FILE_OPEN_EXISTING       0x20000000

/* Routines to manipulate file or device. */
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

/* File attributes. */
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20

DWORD GetFileAttributes(LPSTR lpszFileName);
DWORD GetFileSize(HANDLE hFile, DWORD* lpdwSizeHigh);
BOOL RemoveDirectory(LPSTR lpszDirName);
BOOL SetEndOfFile(HANDLE hFile);
BOOL IOControl(HANDLE hFile,
	DWORD dwCommand,
	DWORD dwInputLen,
	LPVOID lpInputBuffer,
	DWORD dwOutputLen,
	LPVOID lpOutputBuffer,
	DWORD* lpdwFilled);

/* Base of file's current position when seek. */
#define FILE_FROM_BEGIN 0x00000001
#define FILE_FROM_CURRENT 0x00000002

/* Seek file's current pointer position. */
BOOL SetFilePointer(HANDLE hFile,
	DWORD* lpdwDistLow,
	DWORD* lpdwDistHigh,
	DWORD dwMoveFlags);
BOOL FlushFileBuffers(HANDLE hFile);

/* Text mode output. */
void PrintLine(LPSTR lpszInfo);
void PrintChar(char ch);
void GotoHome();
void ChangeLine();

/*
 * System info structure,all members are
 * self-interpretible.
 */
typedef struct _SYSTEM_INFO {
	union {
		DWORD dwOemId;          // Obsolete field...do not use
		struct {
			WORD wProcessorArchitecture;
			WORD wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	DWORD dwPageSize;
	LPVOID lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress;
	DWORD dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO, __SYSTEM_INFO;

/* Get system information. */
BOOL GetSystemInfo(SYSTEM_INFO* pSysInfo);

/*
 * Structure to contain a block of virtual memory's
 * attributes,it is used by VirtualQuery routine in
 * virtual memory manager.
 */
typedef struct _MEMORY_BASIC_INFORMATION {
	LPVOID BaseAddress;
	LPVOID AllocationBase;
	unsigned long AllocationProtect;
	size_t RegionSize;
	unsigned long State;
	unsigned long Protect;
	unsigned long Type;
} MEMORY_BASIC_INFORMATION, __MEMORY_BASIC_INFORMATION;

/* Query a block of virtual memory's basic information. */
size_t VirtualQuery(LPVOID pStartAddr,
	MEMORY_BASIC_INFORMATION* pMemInfo,
	size_t info_sz);

/* Test the parameter xfering mechanism of system call. */
BOOL XferMoreParam(char* pszInfo0, char* pszInfo1, char* pszInfo2,
	int nInfo3, unsigned int nInfo4, char* pszInfo5, char* pszInfo6,
	char* pszInfo7);

#endif //__HELLOX_H__
