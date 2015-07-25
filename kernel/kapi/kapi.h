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
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#define __KAPI_H__
#endif

//Types.
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
typedef unsigned long BOOL;
typedef void* LPVOID;
typedef void VOID;
typedef LPVOID HANDLE;                             //Use handle to refer any kernel object.

//If WIDE char is used.
#ifdef __WIDE_CHAR
#define TCHAR WCHAR
#else
#define TCHAR CHAR
#endif

//Declare as wide character or wide char string.
#define WIDE_CHAR L
#define WIDE_STR  L

//Constants.
#define MAX_DWORD_VALUE    0xFFFFFFFF
#define TRUE               0xFFFFFFFF
#define FALSE              0x00000000
#define NULL               0
#define MAX_FILE_NAME_LEN  512

#define MOUSE_DIM_X   511  //Maximal x dimension of mouse.
#define MOUSE_DIM_Y   511  //y dimension.

//Virtual key maps.
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
#define ASCII_REVERSE_SLASH   47   ///

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

//
//Virtual keys defined by OS kernel to handle different 
//physical key boards.
//
#define VK_ESC             27
#define VK_RETURN          13
#define VK_TAB             9
#define VK_CAPS_LOCK       20
#define VK_SHIFT           10
#define VK_CONTROL         17
#define VK_MENU            18    //Menu key in windows complying key board.
#define VK_BACKSPACE       8
#define VK_LWIN            91    //Left windows key in windows complying kb.
#define VK_RWIN            92    //Right windows key.
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
#define VK_ALT             5     //Alt key,defined by Hello China.

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
#define VK_PRINTSC         6    //This value is defined by Hello China.

//System call numbers.
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
#define SYSCALL_SETFOCUSTHREAD        0x36

//Structures.
typedef struct _MSG{
	WORD    wCommand;
	WORD    wParam;
	DWORD   dwParam;
}MSG;

typedef struct  {
    DWORD dwHighDateTime;
    DWORD dwLowDateTime;
} __FILE_TIME;

typedef struct {
    DWORD           dwFileAttribute;
    __FILE_TIME     ftCreationTime;    //-------- CAUTION!!! --------
	__FILE_TIME     ftLastAccessTime;  //-------- CAUTION!!! --------
	__FILE_TIME     ftLastWriteTime;   //-------- CAUTION!!! --------
	DWORD           nFileSizeHigh;
	DWORD           nFileSizeLow;
	DWORD           dwReserved0;
	DWORD           dwReserved1;
	CHAR            cFileName[MAX_FILE_NAME_LEN];
	CHAR            cAlternateFileName[13];
} FS_FIND_DATA;

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
DWORD GetLastError(void);
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
HANDLE CreateMutex(void);
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

//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20

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

//Flags to control SetFilePointer.
#define FILE_FROM_BEGIN        0x00000001
#define FILE_FROM_CURRENT      0x00000002

BOOL SetFilePointer(HANDLE hFile,
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
					HANDLE hDrvObject);
VOID DestroyDevice(HANDLE hDevice);

//Kernel memory allocation flags.
#define KMEM_SIZE_TYPE_ANY       0x00000001
#define KMEM_SIZE_TYPE_1K        0x00000002
#define KMEM_SIZE_TYPE_4K        0x00000003

LPVOID KMemAlloc(DWORD dwSize,DWORD dwSizeType);
VOID KMemFree(LPVOID lpMemAddr,DWORD dwSizeType,DWORD dwMemLength);

VOID PrintLine(LPSTR lpszInfo);
VOID PrintChar(WORD ch);

typedef BOOL (*__SYSCALL_DISPATCH_ENTRY)(LPVOID,LPVOID);
BOOL RegisterSystemCall(DWORD dwStartSyscallNum,DWORD dwEndSyscallNum,
						__SYSCALL_DISPATCH_ENTRY sde);
BOOL ReplaceShell(__KERNEL_THREAD_ROUTINE shell);

typedef BOOL (*__DRIVER_ENTRY)(HANDLE hDriverObject);
BOOL LoadDriver(__DRIVER_ENTRY de);

HANDLE GetCurrentThread(void);
HANDLE SetFocusThread(HANDLE hNewThread);

BOOL SwitchToGraphic(void);
VOID SwitchToText(void);

//Critical section operations.
#define __ENTER_CRITICAL_SECTION(obj,flag) {flag = 0;}
#define __LEAVE_CRITICAL_SECTION(obj,flag)
