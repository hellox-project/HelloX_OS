//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Aug 25,2015
//    Module Name               : k_API_DEF.H
//    Module Funciton           : 
//                                kernel api id define
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef _K_API_DEF_H_
#define _K_API_DEF_H_

//System call numbers.The dispatch routine(SyscallHandler) will dispatch
//each system call to specific service routine by using this value.
// kernel id start
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

// io id start
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

// socket id start
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

#define SYSCALL_MAX_COUNT             0x1000  // syscall  count         

#endif  //_K_API_DEF_H_
