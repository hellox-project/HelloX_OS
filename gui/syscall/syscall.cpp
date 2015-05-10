//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 08,2012
//    Module Name               : syscall.h
//    Module Funciton           : 
//                                System call stub's implementation code of GUI module.
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
#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\VESA.H"
#include "..\INCLUDE\VIDEO.H"
#include "..\INCLUDE\GLOBAL.H"
#include "..\INCLUDE\CLIPZONE.H"
#include "..\INCLUDE\GDI.H"
#include "..\INCLUDE\RAWIT.H"
#include "..\INCLUDE\GUISHELL.H"
#include "..\INCLUDE\WNDMGR.H"
#include "..\INCLUDE\BMPAPI.H"
#include "..\include\WordLib.H"
#include "..\INCLUDE\BUTTON.H"
#include "..\INCLUDE\MSGBOX.H"
#include "..\include\bmpbtn.h"

#include "..\kapi\math.h"
#include "..\INCLUDE\stdio.h"
#include "..\INCLUDE\string.h"

#include "syscall.h"

//System call entry point of GUI module.All system calls in GUI module,
//will be lead to this entry point by kernel,then the appropriate routine
//is located by the syscall number.
BOOL SyscallHandler(LPVOID lpEsp,LPVOID)
{
	__SYSCALL_PARAM_BLOCK*  pspb = (__SYSCALL_PARAM_BLOCK*)lpEsp;
	DWORD* pTmpDw = NULL;

	if(NULL == lpEsp)
	{
		return FALSE;
	}
	//Call the proper service routine according to system call number.
	//We use switch and case clauses to avoid the complaint problem in
	//assemble language.
	//But don't worry about the performance,clever compiler will generate
	//jumping table which can locate one service routine only by ONE array
	//locating.
#define PARAM(i) (pspb->lpParams[i])  //To simplify the programming.
	switch(pspb->dwSyscallNum)
	{
	case SYSCALL_CREATEWINDOW:
		if(NULL == PARAM(7))  //Parent is not specified,use main frame as parent.
		{
			pspb->lpRetValue = (LPVOID)CreateWindow(
				(DWORD)PARAM(0),
				(TCHAR*)PARAM(1),
				GuiGlobal.xmf,
				GuiGlobal.ymf,
				GuiGlobal.cxmf,
				GuiGlobal.cymf,
				(__WINDOW_PROC)PARAM(6),
				(HANDLE)PARAM(7),
				(HANDLE)PARAM(8),
				(__COLOR)PARAM(9),PARAM(10));
			break;
		}
		//If the application specified a parent window,then just use it.
		pspb->lpRetValue = (LPVOID)CreateWindow(
			(DWORD)PARAM(0),
			(TCHAR*)PARAM(1),
			(int)PARAM(2),
			(int)PARAM(3),
			(int)PARAM(4),
			(int)PARAM(5),
			(__WINDOW_PROC)PARAM(6),
			(HANDLE)PARAM(7),
			(HANDLE)PARAM(8),
			(__COLOR)PARAM(9),
			PARAM(10));
		break;
	case SYSCALL_DESTROYWINDOW:
		DestroyWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CLOSEWINDOW:
		CloseWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_FOCUSWINDOW:
		FocusWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_UNFOCUSWINDOW:
		UnfocusWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETWINDOWDC:
		pspb->lpRetValue = (LPVOID)GetWindowDC(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETCLIENTDC:
		pspb->lpRetValue = (LPVOID)GetClientDC(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_PAINTWINDOW:
		PaintWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETWINDOWSTATUS:
		pspb->lpRetValue = (LPVOID)GetWindowStatus(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETWINDOWEXTENSION:
		pspb->lpRetValue = (LPVOID)GetWindowExtension(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_SETWINDOWEXTENSION:
		pspb->lpRetValue = (LPVOID)SetWindowExtension(
			(HANDLE)PARAM(0),
			(LPVOID)PARAM(1));
		break;
	case SYSCALL_GETPARENTWINDOW:
		pspb->lpRetValue = (LPVOID)GetParentWindow(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETWINDOWRECT:
		pspb->lpRetValue = (LPVOID)GetWindowRect(
			(HANDLE)PARAM(0),
			(__RECT*)PARAM(1),
			(DWORD)PARAM(2));
		break;
	case SYSCALL_ISCHILD:
		pspb->lpRetValue = (LPVOID)IsChild(
			(HANDLE)PARAM(0),
			(HANDLE)PARAM(1));
		break;
	case SYSCALL_HITTEST:
		pspb->lpRetValue = (LPVOID)HitTest(
			(HANDLE)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2));
		break;
	case SYSCALL_SENDWINDOWMESSAGE:
		pspb->lpRetValue = (LPVOID)SendWindowMessage(
			(HANDLE)PARAM(0),
			(__WINDOW_MESSAGE*)PARAM(1));
		break;
	case SYSCALL_DISPATCHWINDOWMESSAGE:
		pspb->lpRetValue = (LPVOID)DispatchWindowMessage(
			(__WINDOW_MESSAGE*)PARAM(0));
		break;
	case SYSCALL_DEFWINDOWPROC:
		pspb->lpRetValue = (LPVOID)DefWindowProc(
			(HANDLE)PARAM(0),
			(UINT)PARAM(1),
			(WORD)PARAM(2),
			(DWORD)PARAM(3));
		break;
	case SYSCALL_POSTQUITMESSAGE:
		PostQuitMessage(
			(int)PARAM(0));
		break;
	case SYSCALL_CREATEFONT:
		pspb->lpRetValue = (LPVOID)CreateFont(
			(int)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2),
			(int)PARAM(3));
		break;
	case SYSCALL_DESTROYFONT:
		DestroyFont(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_GETTEXTMETRIC:
		pspb->lpRetValue = (LPVOID)GetTextMetric(
			(HANDLE)PARAM(0),
			(TCHAR*)PARAM(1),
			(DWORD)PARAM(2));
		break;
	case SYSCALL_CREATEBRUSH:
		pspb->lpRetValue = (LPVOID)CreateBrush(
			(BOOL)PARAM(0),
			(__COLOR)PARAM(1));
		break;
	case SYSCALL_DESTROYBRUSH:
		DestroyBrush(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_CREATEPEN:
		pspb->lpRetValue = (LPVOID)CreatePen(
			(DWORD)PARAM(0),
			(int)PARAM(1),
			(__COLOR)PARAM(2));
		break;
	case SYSCALL_DESTROYPEN:
		DestroyPen(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_PTINRECT:
		pspb->lpRetValue = (LPVOID)PtInRect(
			(__RECT*)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2));
		break;
	case SYSCALL_CREATEDEVICECONTEXT:
		pspb->lpRetValue = (LPVOID)CreateDeviceContext(
			(DWORD)PARAM(0),
			(HANDLE)PARAM(1),
			(HANDLE)PARAM(2),
			(__REGION*)PARAM(3));
		break;
	case SYSCALL_DESTROYDEVICECONTEXT:
		DestroyDeviceContext(
			(HANDLE)PARAM(0));
		break;
	case SYSCALL_TEXTOUT:
		TextOut(
			(HANDLE)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2),
			(TCHAR*)PARAM(3));
		break;
	case SYSCALL_DRAWPIXEL:
		DrawPixel(
			(HANDLE)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2));
		break;
	case SYSCALL_MOVETO:    //Not implemented yet.
		break;
	case SYSCALL_LINETO:    //Not implemented yet.
		break;
	case SYSCALL_DRAWLINE:
		DrawLine(
			(HANDLE)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2),
			(int)PARAM(3),
			(int)PARAM(4));
		break;
	case SYSCALL_DRAWRECTANGLE:  //Will be optimized.
		DrawRectangle(
			(HANDLE)PARAM(0),
			(__RECT*)PARAM(1));
		break;
	case SYSCALL_DRAWCIRCLE:
		DrawCircle(
			(HANDLE)PARAM(0),
			(int)PARAM(1),
			(int)PARAM(2),
			(int)PARAM(3),
			(BOOL)PARAM(4));
		break;
	case SYSCALL_SELECTPEN:
		pspb->lpRetValue = (LPVOID)SelectPen(
			(HANDLE)PARAM(0),
			(HANDLE)PARAM(1));
		break;
	case SYSCALL_SELECTBRUSH:
		pspb->lpRetValue = (LPVOID)SelectBrush(
			(HANDLE)PARAM(0),
			(HANDLE)PARAM(1));
		break;
	case SYSCALL_SELECTFONT:
		pspb->lpRetValue = (LPVOID)SelectFont(
			(HANDLE)PARAM(0),
			(HANDLE)PARAM(1));
		break;
	case SYSCALL_GETDCWINDOW:  //Not implemented yet.
		break;
	case SYSCALL_CREATEBUTTON:
		pspb->lpRetValue = (LPVOID)CreateButton(
			(HANDLE)PARAM(0),
			(TCHAR*)PARAM(1),
			(DWORD)PARAM(2),
			(int)PARAM(3),
			(int)PARAM(4),
			(int)PARAM(5),
			(int)PARAM(6));
		break;
	case SYSCALL_SETBUTTONTEXT:  //Not implemented yet.
		break;
	case SYSCALL_SETBUTTONCOLOR: //Not implemented yet.
		break;
	case SYSCALL_CREATEBITMAPBUTTON:
		pspb->lpRetValue = (LPVOID)CreateBitmapButton(
			(HANDLE)PARAM(0),
			(TCHAR*)PARAM(1),
			(DWORD)PARAM(2),
			(int)PARAM(3),
			(int)PARAM(4),
			(int)PARAM(5),
			(int)PARAM(6),
			PARAM(7),
			PARAM(8));
		break;
	case SYSCALL_SETBITMAPBUTTONTEXT:    //Not implemented yet.
		break;
	case SYSCALL_SETBITMAPBUTTONBITMAP:  //Not implemented yet.
		break;
	case SYSCALL_MESSAGEBOX:
		pspb->lpRetValue = (LPVOID)MessageBox(
			(HANDLE)PARAM(0),
			(TCHAR*)PARAM(1),
			(TCHAR*)PARAM(2),
			(UINT)PARAM(3));
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
