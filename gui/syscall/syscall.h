//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 08,2012
//    Module Name               : syscall.h
//    Module Funciton           : 
//                                System call related definitions and objects.
//                                User application can access GUI module functions
//                                through system calls.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

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
	LPVOID                        lpParams[12];
}__SYSCALL_PARAM_BLOCK;

//System call numbers for each GUI routines,start from 256.The system call number
//between 1 and 255 are reserved for kernel module.
#define SYSCALL_GUI_BEGIN             256
#define SYSCALL_GUI_END               511

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

//SyscallHandler,all GUI module system calls will be handled by this routine.
BOOL SyscallHandler(LPVOID,LPVOID);

#endif //__SYSCALL_H__
