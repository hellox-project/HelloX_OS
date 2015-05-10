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

#ifndef __KAPI_H__
#define __KAPI_H__

#define __GUI_SUPPORT__ //Enable GUI functions.

//Types.
typedef unsigned char UINT_8;
typedef unsigned short UINT_16;
typedef unsigned long UINT_32;
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
#define MAX_FILE_NAME_LEN  256

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

struct __FILE_TIME{
    DWORD dwHighDateTime;
    DWORD dwLowDateTime;
};

struct FS_FIND_DATA{
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
};

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

HANDLE GetCurrentThread();
HANDLE SetFocusThread(HANDLE hNewThread);

BOOL SwitchToGraphic();
VOID SwitchToText();

#ifdef __GUI_SUPPORT__  //Support GUI functions.

//Syscall numbers.
//The following syscall's proto-type is in wndmgr.h.
#define SYSCALL_CREATEWINDOW          256          //CreateWindow.
#define SYSCALL_DESTROYWINDOW         257          //DestroyWindow.
#define SYSCALL_CLOSEWINDOW           258          //CloseWindow.
#define SYSCALL_FOCUSWINDOW           259          //FocusWindow.
#define SYSCALL_UNFOCUSWINDOW         260          //UnFocusWindow.
#define SYSCALL_GETWINDOWDC           261          //GetWindowDC.
#define SYSCALL_GETCLIENTDC           262          //GetClientDC.
#define SYSCALL_PAINTWINDOW           263          //PaintWindow.
#define SYSCALL_GETWINDOWSTATUS       264          //GetWindowStatus.
#define SYSCALL_GETWINDOWEXTENSION    265          //GetWindowExtension.
#define SYSCALL_SETWINDOWEXTENSION    266          //SetWindowExtension.
#define SYSCALL_GETPARENTWINDOW       267          //GetParentWindow.
#define SYSCALL_GETWINDOWRECT         268          //GetWindowRect.
#define SYSCALL_ISCHILD               269          //IsChild.
#define SYSCALL_HITTEST               270          //HitTest.
#define SYSCALL_SENDWINDOWMESSAGE     271          //SendWindowMessage.
#define SYSCALL_DISPATCHWINDOWMESSAGE 272          //DispatchWindowMessage.
//#define SYSCALL_SENDWINDOWTREEMESSAGE 273          //SendWindowTreeMessage.
#define SYSCALL_DEFWINDOWPROC         273          //DefWindowProc.
#define SYSCALL_POSTQUITMESSAGE       274          //PostQuitMessage.

//The following syscall's proto-type is in gdi.h.
#define SYSCALL_CREATEFONT            275          //CreateFont.
#define SYSCALL_DESTROYFONT           276          //DestroyFont.
#define SYSCALL_GETTEXTMETRIC         277          //GetTextMetric.
#define SYSCALL_CREATEBRUSH           278          //CreateBrush.
#define SYSCALL_DESTROYBRUSH          279          //DestroyBrush.
#define SYSCALL_CREATEPEN             280          //CreatePen.
#define SYSCALL_DESTROYPEN            281          //DestroyPen.
#define SYSCALL_PTINRECT              282          //PtInRect.
#define SYSCALL_CREATEDEVICECONTEXT   283          //CreateDeviceContext.
#define SYSCALL_DESTROYDEVICECONTEXT  284          //DestroyDeviceContext.
#define SYSCALL_TEXTOUT               285          //TextOut.
#define SYSCALL_DRAWPIXEL             286          //DrawPixel.
#define SYSCALL_MOVETO                287          //MoveTo.
#define SYSCALL_LINETO                288          //LineTo.
#define SYSCALL_DRAWLINE              289          //DrawLine.
#define SYSCALL_DRAWRECTANGLE         290          //DrawRectangle.
#define SYSCALL_DRAWCIRCLE            291          //DrawCircle.
#define SYSCALL_SELECTPEN             292          //SelectPen.
#define SYSCALL_SELECTBRUSH           293          //SelectBrush.
#define SYSCALL_SELECTFONT            294          //SelectFont.
#define SYSCALL_GETDCWINDOW           295          //GetDCWindow.

//The following syscall's proto-type is in button.h.
#define SYSCALL_CREATEBUTTON          296          //CreateButton.
#define SYSCALL_SETBUTTONTEXT         297          //SetButtonText.
#define SYSCALL_SETBUTTONCOLOR        298          //SetButtonColor.

//The following syscall's proto-type is in bmpbtn.h.
#define SYSCALL_CREATEBITMAPBUTTON    299          //CreateBitmapButton.
#define SYSCALL_SETBITMAPBUTTONTEXT   300          //SetBitmapButtonText.
#define SYSCALL_SETBITMAPBUTTONBITMAP 301          //SetBitmapButtonBitmap.

//The following syscal's proto-type is in msgbox.h.
#define SYSCALL_MESSAGEBOX            302          //MessageBox.

//COLOR structure is used to manage all colors.It is a 32 bits
//unsigned integer,which layout as following:
// bit0   -- bit7   : Blue dimension;
// bit8   -- bit15  : Gree dimension;
// bit16  -- bit23  : Red dimension.
typedef unsigned long __COLOR;

//A Macro to convert three dimensions to a color value.
#define RGB(r,g,b) ((((DWORD)r) << 16) + (((DWORD)g) << 8) + ((DWORD)b))

//Usable color constants.
#define COLOR_RED    0x00FF0000
#define COLOR_GREEN  0x0000FF00
#define COLOR_BLUE   0x000000FF
#define COLOR_WHITE  0x00FFFFFF
#define COLOR_BLACK  0x00000000

//Video object,one for each video device.
struct __VIDEO{
	DWORD          dwScreenWidth;    //Screen width.
	DWORD          dwScreenHeight;   //Screen height.
	DWORD          BitsPerPixel;     //Color bits for one pixel.
	LPVOID         pBaseAddress;     //Base address of display memory.
	DWORD          dwMemLength;      //Length of display memory.
	BOOL           (*Initialize)(__VIDEO* pVideo);
	VOID           (*Uninitialize)(__VIDEO* pVideo);

	VOID (*DrawPixel)(__VIDEO* pVideo,int x,int y,__COLOR color);
	__COLOR (*GetPixel)(__VIDEO* pVideo,int x,int y);
	VOID (*DrawLine)(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR color);
	VOID (*DrawRectangle)(__VIDEO* pVideo,int x1,int y1,int x2,int y2,
				   __COLOR lineclr,BOOL bFill,__COLOR fillclr);
	VOID (*DrawEllipse)(__VIDEO* pVide,int x1,int y1,int x2,int y2,__COLOR color,
				 BOOL bFill,__COLOR fillclr);
	VOID (*DrawCircle)(__VIDEO* pVideo,int xc,int yc,int r,__COLOR color,BOOL bFill);
	VOID (*MouseToScreen)(__VIDEO* pVideo,int x,int y,int* px,int* py);
};

//The definition of CLIPZONE object.
struct __CLIPZONE{
	int x;
	int y;
	int height;
	int width;
	__CLIPZONE* pNext;
	__CLIPZONE* pPrev;
};

//RECT object,to describe one rectangle object.
struct __RECT{
	int left;
	int top;
	int right;
	int bottom;
};

//Definition of region object.This object is used to represent window's clipzone.
struct __REGION{
	__CLIPZONE ClipZoneHdr;
};

//The following content is in wndmgr.h.
//Window procedure definition.
typedef DWORD (*__WINDOW_PROC)(HANDLE hWnd,UINT msg,WORD wParam,DWORD lParam);

#define WND_TITLE_LEN 128   //Window title's length,in WCHAR.

//Window object.
struct __WINDOW{
	TCHAR      WndTitle[WND_TITLE_LEN];
	DWORD      dwWndStyle;
	DWORD      dwWndStatus;
	int        x;              //Start position of window.
	int        y;
	int        cx;             //Width of this window.
	int        cy;
	int        xclient;        //Start position of client area.
	int        yclient;
	int        cxclient;       //Client area's width.
	int        cyclient;
	int        xcb;            //x coordinate of close button.
	int        ycb;            //y coordinate of close button.
	int        cxcb;           //width of close button.
	int        cycb;           //height of close button.
	HANDLE     hFocusChild;    //Child window in focus status.
	HANDLE     hCursor;        //Window cursor.
	HANDLE     hIcon;          //ICON of this window.
	HANDLE     hWindowDC;      //Device context of this window.
	HANDLE     hClientDC;      //Device context of this window's client area.
	__COLOR    clrBackground;  //Background color.
	LPVOID     lpWndExtension; //Window extension pointer.
	__WINDOW_PROC WndProc;     //Base address of window procedure.
	HANDLE     hOwnThread;     //The thread handle owns this window.
	__REGION*  pRegion;        //Clip zone of this window.

	__WINDOW*  pPrevSibling;   //Previous sibling of this window.
	__WINDOW*  pNextSibling;   //Next sibling.
	__WINDOW*  pParent;        //Parent;
	__WINDOW*  pChild;         //Child list header.
	DWORD      dwSignature;    //Signature of window object.
};

//Window signature,the window only contain this signature is to be
//treat as a TRUE window object.This rule is used to verify window 
//object's validity.
#define WINDOW_SIGNATURE 0xAA5555AA

//Window status.
#define WST_ONFOCUS        0x00000001
#define WST_UNFOCUS        0x00000002
#define WST_MINIMIZED      0x00000004
#define WST_MAXIMIZED      0x00000008
#define WST_NORMAL         0x00000010

//Window styles.
#define WS_WITHCAPTION    0x00000001    //With caption.
#define WS_3DFRAME        0x00000002    //3D frame effective.
#define WS_LINEFRAME      0x00000004    //Line frame.
#define WS_MAINFRAME      0x00000008    //Main frame window.
#define WS_HSCROLLBAR     0x00000010    //with HScroll bar.
#define WS_VSCROLLBAR     0x00000020    //With VScroll bar.
#define WS_WITHBORDER     0x00000040    //Window with border.
#define WS_HIDE           0x00000080    //Window will be hide after created.
#define WS_NORMAL         (WS_WITHCAPTION | WS_WITHBORDER | WS_LINEFRAME | WS_MAINFRAME)
#define WS_MODEL          0x00000100    //Model dialog.
#define WS_CANNOTFOCUS    0x00000200    //Can not focus on this window.
#define WS_UNFOCUSDRAW    0x00000400    //Can draw in un-focus mode.

//Windows messages can be sent to a window.
#define WM_CREATE         1
#define WM_DESTROY        2
#define WM_CLOSE          3
#define WM_LBUTTONDOWN    4
#define WM_RBUTTONDOWN    5
#define WM_LBUTTONDBLCLK  6
#define WM_RBUTTONDBLCLK  7
#define WM_SHOW           8
#define WM_HIDE           9
#define WM_SIZE           10
#define WM_VIRTUALKEY     11
#define WM_CHAR           12
#define WM_KEYDOWN        13
#define WM_KEYUP          14
#define WM_MOUSEMOVE      15
#define WM_LBUTTONUP      16
#define WM_RBUTTONUP      17
#define WM_TIMER          18
#define WM_COMMAND        19
#define WM_DRAGIN         20
#define WM_DRAGOUT        21
#define WM_MINIMAL        22
#define WM_MAXIMAL        23
#define WM_NORMAL         24
#define WM_NOTIFY         25
#define WM_DRAW           26
#define WM_UPDATE         27
#define WM_SCROLLUP       28
#define WM_SCROLLDOWN     29
#define WM_SCROLLLEFT     30
#define WM_SCROLLRIGHT    31
#define WM_SCROLLHOLD     32
#define WM_PAINT          33
#define WM_LOSTFOCUS      34
#define WM_ONFOCUS        35
#define WM_CHILDCLOSE     36
#define WM_CHILDSHOW      37
#define WM_INITDIALOG     38    //Initialize dialog box.

#define WM_USER           1024  //User defined messages start from this value.

//Default window procedure,this procedure should be called after
//your custom process for every message.
DWORD DefWindowProc(HANDLE hWnd,UINT msg,DWORD wParam,DWORD lParam);

//Definition of windows manager object.
struct __WINDOW_MANAGER{
	__WINDOW*   pWndAncestor;      //The first window of the system,it is
	                               //all other windows' ancestor.
	HANDLE      hCurrThread;       //Current focus thread.
	__WINDOW*   pCurrWindow;       //Current focus window.
	BOOL        (*Initialize)(__WINDOW_MANAGER* pWndManager);
	VOID        (*Uninitialize)(__WINDOW_MANAGER* pWndManager);
};

HANDLE CreateWindow(DWORD dwWndStyle,TCHAR* pszWndTitle,int x,
					int y,int cx,int cy,__WINDOW_PROC WndProc,
					HANDLE hParent,HANDLE hMenu,__COLOR clrbackground,
					LPVOID lpReserved); //Create one window.

VOID DestroyWindow(HANDLE hWnd);  //Destroy one window.
VOID CloseWindow(HANDLE hWnd);    //Close one window.
VOID FocusWindow(HANDLE hWnd);    //Set window in focus status.
VOID UnfocusWindow(HANDLE hWnd);  //Force a window loss focus.
HANDLE GetWindowDC(HANDLE hWnd);  //Returns window's DC.
HANDLE GetClientDC(HANDLE hWnd);  //Returns window client area's DC.
VOID PaintWindow(HANDLE hWnd);    //Paint a window.
DWORD GetWindowStatus(HANDLE hWnd);  //Get a given window's status.
LPVOID GetWindowExtension(HANDLE hWnd);  //Return window's extension pointer.
LPVOID SetWindowExtension(HANDLE hWnd,LPVOID lpNewExt);  //Set the ext to new and return old.
HANDLE GetParentWindow(HANDLE hWnd); //Returns parent window.
BOOL GetWindowRect(HANDLE hWnd,__RECT* pRect,DWORD dwIndicator);  //Returns window's rect according indicator.
BOOL IsChild(HANDLE hParent,HANDLE hChild);  //If the hChild is child of hParent.

//Indicators of GetWindowRect.
#define GWR_INDICATOR_WINDOW     0x00000001
#define GWR_INDICATOR_CLIENT     0x00000002

//Hit test results.
#define HT_CLOSEBUTTON    0x00000001
#define HT_CAPTION        0x00000002
#define HT_CLIENT         0x00000003
#define HT_LEFTFRAME      0x00000004
#define HT_RIGHTFRAME     0x00000005
#define HT_TOPFRAME       0x00000006
#define HT_BOTTOMFRAME    0x00000007
#define HT_SYSMENU        0x00000008
#define HT_RESTBUTTON     0x00000009
#define HT_MINBUTTON      0x0000000A

//HitTest routine,check the click position of a window.
DWORD HitTest(HANDLE hWnd,int x,int y);

//Window message.
struct __WINDOW_MESSAGE{
	HANDLE hWnd;
	UINT   message;
	WORD   wParam;
	WORD   wReserved;
	DWORD  lParam;
};

BOOL SendWindowMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg);  //Send a message to window.
BOOL DispatchWindowMessage(__WINDOW_MESSAGE* pWndMsg);  //Dispatch a window message.
VOID SendWindowTreeMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg); //Send a message to window and all
                                                                   //it's children.
VOID SendWindowChildMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg);
VOID PostQuitMessage(int nExitCode);  //Send a KERNEL_MESSAGE_TERMINAL to current thread.

//The following content is from gdi.h.
//FONT object,to describe a type of font.
struct __FONT{
	int width;
	int height;
	int chspace;  //Space between 2 characters in one line.
	int lnspace;  //Space between 2 lines in one page.
	int ascwidth; //ASCII character's width.
};

#define DEFAULT_FONT_WIDTH    16
#define DEFAULT_FONT_HEIGHT   16
#define DEFAULT_FONT_CHSPACE  0
#define DEFAULT_FONT_LNSPACE  5

HANDLE CreateFont(int width,int height,int chspace,int lnspace);
VOID DestroyFont(HANDLE hFont);

//Text metric information.
#define TM_WIDTH        0x0000001     //Get a text string's width in a specified DC.
#define TM_HEIGHT       0x0000002     //Get a text string's height in a spec DC.
#define TM_MAXWIDTH     0x0000003     //Get the maximal character width in a string.

//Get text string display information.
DWORD GetTextMetric(HANDLE hDC,TCHAR* pszText,DWORD dwInfoClass);

//BRUSH object,to describe a kind of brush,which is used to
//fill other GUI objects.
struct __BRUSH{
	BOOL bTransparent;   //If the brush is transparent.
	__COLOR color;
};

#define DEFAULT_BRUSH_COLOR COLOR_WHITE

HANDLE CreateBrush(BOOL bTransparent,__COLOR color);
VOID DestroyBrush(HANDLE hBrush);

//PEN object,used to draw in target graphic device.
struct __PEN{
	DWORD type;
	int width;
	__COLOR color;
};

#define DEFAULT_PEN_WIDTH 1

HANDLE CreatePen(DWORD type,int width,__COLOR color);
VOID DestroyPen(HANDLE hPen);

//Check if a point fall in the given rectangle.
BOOL PtInRect(__RECT* pRect,int x,int y);

//POINT object,to describe one point in graphic device.
struct __POINT{
	int x;
	int y;
};

//Device Context type.
#define DC_TYPE_SCREEN    0x00000001
#define DC_TYPE_PRINT     0x00000002
#define DC_TYPE_MEMORY    0x00000004
#define DC_TYPE_WINDOW    0x00000008  //Window DC.
#define DC_TYPE_CLIENT    0x00000010  //Window client DC.

//DC(Device Context) object,this is the most important object
//in GUI module,all application level drawing operations are
//depend on this object.
struct __DC{
	DWORD       dwDCType;      //DC type.
	__PEN*      pPen;          //Current drawing pen.
	__BRUSH*    pBrush;        //Current drawing brush.
	__FONT*     pFont;         //Current drawing font.
	__POINT     Point;         //Current position to draw.

	__VIDEO*    pVideo;       //Video device this DC based on.
	HANDLE      hOther;       //Other device this DC based on,such as PRINTER.
	HANDLE      hWindow;      //Window object of this DC.

	__REGION*   pRegion;      //Clip zone object,this object is defined
	                          //in CLIPZONE.H file.
};

//The following routine are used to operate DC.
//Create one DC object.
HANDLE CreateDeviceContext(DWORD dwDCType,HANDLE hDevice,HANDLE pWindow,__REGION* pRegion);

//Destroy one DC object.
VOID DestroyDeviceContext(HANDLE hDC);

//Draw a text string in specified DC.
VOID TextOut(HANDLE hDC,int x,int y,TCHAR* pszString);

//Draw a pixel on specified DC.
VOID DrawPixel(HANDLE hDC,int x,int y);

//Set current start drawing postion.
VOID MoveTo(HANDLE hDC,int x,int y);

//Draw a line from current position to target position.
VOID LineTo(HANDLE hDC,int x,int y);

//Draw a line from the given start and end position.
VOID DrawLine(HANDLE hDC,int x1,int y1,int x2,int y2);

//Draw a rectangle.
VOID DrawRectangle(HANDLE hDC,__RECT* rect);

//Draw a circle.
VOID DrawCircle(HANDLE hDC,int xc,int yc,int r,BOOL bFill);

//Select one pen as DC's drawing pen,old one will be returned.
HANDLE SelectPen(HANDLE hDC,HANDLE hNewPen);

//Select one brush as DC's drawing brush.
HANDLE SelectBrush(HANDLE hDC,HANDLE hNewBrush);

//Select one FONT as DC's drawing font.
HANDLE SelectFont(HANDLE hDC,HANDLE hFont);

//Get the homing window of one DC.
HANDLE GetDCWindow(HANDLE hDC);

//The following content is from button.h.
//Button control's property information.
#define BUTTON_TEXT_LEN 64  //Button text's maximal length.

struct __BUTTON{
	TCHAR    ButtonText[BUTTON_TEXT_LEN];
	DWORD    dwBtnStyle;
	DWORD    dwBtnStatus;    //Button status.
	DWORD    dwButtonId;     //Button identifier.
	int      x;
	int      y;
	int      cx;
	int      cy;
	//The following members are used to draw button's text.
	int      txtwidth;       //Button text's width,in pixel.
	int      xtxt;           //x coordinate of button text.
	int      ytxt;           //y coordinate of button text.
};

//Button status.
#define BUTTON_STATUS_NORMAL   0x0000001    //Normal status.
#define BUTTON_STATUS_PRESSED  0x0000002    //Pressed status.

//Create one button in a given window.
HANDLE CreateButton(HANDLE hParent,        //Parent window of the button.
					TCHAR* pszButtonText,  //Text displayed in button face.
					DWORD  dwButtonId,     //Button ID.
					int x,
					int y,
					int cx,
					int cy);

//Change button face text.
VOID SetButtonText(HANDLE hButton,TCHAR* pszButtonText);

//Change button face color.
VOID SetButtonColor(HANDLE hButton,__COLOR clr);

//The following content is from bmpbtn.h.
//Bitmap button's text's maximal length,in character.
#define BMPBTN_TEXT_LENGTH 64

//Default face color,if user does not specify.A bitmap will be put
//on this face.
#define DEFAULT_BMPBTN_FACECOLOR 0x00C0FF

//Default bitmap buttons' text background.
#define DEFAULT_BMPBTN_TXTBACKGROUND 0x800000

//Default bitmap button's text color.
#define DEFAULT_BMPBTN_TXTCOLOR 0x00FFFFFF

//Several button face dimensions,button text rectangle is not
//included,more accurate,it's bitmap's dimension.
#define BMPBTN_DIM_16   0x00000001        //16 * 16 bitmap.
#define BMPBTN_DIM_32   0x00000002        //32 * 32 bitmap
#define BMPBTN_DIM_64   0x00000003        //64 * 64 bitmap.
#define BMPBTN_DIM_78   0x00000004        //78 * 78 bitmap.
#define BMPBTN_DIM_96   0x00000005        //96 * 96 bitmap.
#define BMPBTN_DIM_128  0x00000006        //128 * 128 bitmap.
#define BMPBTN_DIM_256  0x00000007        //256 * 256 bitmap.

//Margin between text and text rectangle,so text banner will
//occpy 6 + TXT_HEIGHT pixels,which include upper margin and 
//below margin,text font height.
#define TXT_MARGIN      3

//Button status.
#define BUTTON_STATUS_NORMAL   0x0000001    //Normal status.
#define BUTTON_STATUS_PRESSED  0x0000002    //Pressed status.

//Bitmap buttons' definition.
struct __BITMAP_BUTTON{
	TCHAR    ButtonText[BMPBTN_TEXT_LENGTH];
	DWORD    dwBmpBtnStyle;
	DWORD    dwBmpBtnStatus;    //Button status.
	DWORD    dwBmpButtonId;     //Button identifier.

	int      x;                 //Coordinate relative to it's parent window.
	int      y;
	int      cx;                //Total width and height,include text part.
	int      cy;

	//The following members are used to draw bitmap button's text.
	int      txtwidth;       //Button text rectangle's width,in pixel.
	int      txtheight;      //Button text rectangle's height,in pixel.
	int      xtxt;           //x coordinate of button text.
	int      ytxt;           //y coordinate of button text.

	//Button's appearence colors.
	__COLOR  FaceClr;         //Face color.
	__COLOR  TxtBackground;   //Text background color.
	__COLOR  TxtColor;        //Button text's color.
	__COLOR  FaceColor;       //Button's face color.

	//Bitmap button's bitmap data.
	LPVOID   pBmpData;

	//Bitmap button extension,user specific data can be put here.
	LPVOID   pButtonExtension;
};

//Create one button in a given window.
HANDLE CreateBitmapButton(HANDLE hParent,        //Parent window of the button.
						  TCHAR* pszButtonText,  //Text displayed in button face.
						  DWORD  dwButtonId,     //Button ID.
						  int x,             //Coordinate of the bitmap button in screen.
						  int y,
						  int cxbmp,            //Button's bitmap dimension.
						  int cybmp,
						  LPVOID pBitmap,    //Button's bitmap data.
						  LPVOID pExtension  //Button's extension.
						  );

//Change button face text.
VOID SetBitmapButtonText(HANDLE hBmpButton,TCHAR* pszButtonText);

//Change bitmap button's bitmap.Bitmap's dimension can not be changed,only it's content
//can be changed using this routine.
VOID SetBitmapButtonBitmap(HANDLE hBmpButton,LPVOID pBitmap);

//Change bitmap button's text background and text color.
VOID SetBitmapButtonTextColor(HANDLE hBmpButton,__COLOR txtClr,__COLOR bkgroundClr);

//Set button's extension,the old one will be returned.
LPVOID SetBitmapButtonExtension(HANDLE hBmpButton,LPVOID pNewExtension);

//Get bitmap button's extension.
LPVOID GetBitmapButtonExtension(HANDLE hBmpButton);

//The following content is from msgbox.h.
//Message box types.
#define MB_OK        0x00000001
#define MB_CANCEL    0x00000002
#define MB_YES       0x00000004
#define MB_NO        0x00000008

//Alias of available combination,other value is illegal.
#define MB_OKCANCEL     (MB_OK | MB_CANCEL)
#define MB_YESNO        (MB_YES | MB_NO)
#define MB_YESNOCANCEL  (MB_YES | MB_NO | MB_CANCEL)
//#define MB_OK           MB_OK  //MB_OK itself is a valid combination.

//Button ID for message box.
#define BUTTON_ID_OK      0x00000001
#define BUTTON_ID_CANCEL  0x00000002
#define BUTTON_ID_YES     0x00000004
#define BUTTON_ID_NO      0x00000008

//MessageBox routine pro-type.
DWORD MessageBox(HANDLE hWnd,TCHAR* pszText,TCHAR* pszTitle,UINT uType);

//Message box related specific data.
#define MSGBOX_TEXT_LENGTH 64

struct __MESSAGE_BOX{
	TCHAR        MsgBoxText[MSGBOX_TEXT_LENGTH];
	DWORD        dwMsgBoxType;
	DWORD        dwRetVal;
	DWORD        dwMsgBoxWidth;  //Message box's width in pixel.
	DWORD        dwMsgBoxHeight; //Message box's height in pixel.
};

//Return values for MessageBox routine.
#define ID_OK      0x00000001
#define ID_CANCEL  0x00000002
#define ID_YES     0x00000003
#define ID_NO      0x00000004

//The space occupied by one button.
#define MSGBOX_BUTTON_WIDTH 80
//The space between 2 buttons.
#define MSGBOX_BUTTON_SPACE 40
//The height of a button in message box.
#define MSGBOX_BUTTON_HEIGHT 25

#endif //__GUI_SUPPORT__

//The following definitions and pro-types are from standard C library.
//Maximal and minimal value of double.
#define DBL_MAX	1.7976931348623158e+308
#define DBL_MIN	2.2250738585072014e-308

//Pi's value.
#define PI 3.14159265

//Calculate a floating point number's consine value.
double cos(double x);

//Calculate a floating point number's sinine value.
double sin(double x);

//Floating point mod operation.
double modf(double x,double* y);

//From stdio.h.
typedef char *  va_list;

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#define MAX_BUFFER_SIZE 512

int sprintf(char* buf,const char* fmt,...);
int printf(const char* fmt,...);

void* memcpy(void* pdst,const void* psrc,int size);

//From string.h.
char* strcat(char* dst,const char* src);
char* strcpy(char* dst,const char* src);
char* strchr(const char* string,int ch);
int strcmp(const char* src,const char* dst);
int strlen(const char* s);

long atol(const char* nptr);
int atoi(const char* nptr);

//Critical section operations.
#define __ENTER_CRITICAL_SECTION(obj,flag) {flag = 0;}
#define __LEAVE_CRITICAL_SECTION(obj,flag)

#endif //__KAPI_H__
