//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov,02 2006
//    Module Name               : KAPI.CPP
//    Module Funciton           : 
//                                All routines in kernel module are wrapped
//                                in this file.
//
//    Last modified Author      : Garry
//    Last modified Date        : Jan 09,2012
//    Last modified Content     : 
//                                1.System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#define __GUI_SUPPORT__  //Support GUI function.


#include "K_GDI.H"



//System calls offered by GUI module.
#ifndef __GUI_SUPPORT__

HANDLE CreateWindow(DWORD dwWndStyle,WCHAR* pszWndTitle,int x,int y,int cx,int cy,__WINDOW_PROC WndProc,
					HANDLE hParent,
					HANDLE hMenu,
					__COLOR clrbackground,
					LPVOID lpReserved)
{
	SYSCALL_PARAM_11(
		SYSCALL_CREATEWINDOW,
		dwWndStyle,
		pszWndTile,
		x,
		y,
		cx,
		cy,
		WndProc,
		hParent,
		hMenu,
		clrbackground,
		lpReserved);
}

VOID DestroyWindow(HANDLE hWindow)
{
	SYSCALL_PARAM_1(
		SYSCALL_DESTROYWINDOW,
		hWindow);
}

VOID CloseWindow(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_CLOSEWINDOW,
		hWnd);
}

VOID FocusWindow(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_FOCUSWINDOW,
		hWnd);
}

VOID UnfocusWindow(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_UNFOCUSWINDOW,
		hWnd);
}

HANDLE GetWindowDC(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_GETWINDOWDC,
		hWnd);
}

HANDLE GetClientDC(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_GETCLIENTDC,
		hWnd);
}

VOID PaintWindow(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_PAINTWINDOW,
		hWnd);
}

DWORD GetWindowStatus(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_GETWINDOWSTATUS,
		hWnd);
}

LPVOID GetWindowExtension(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_GETWINDOWEXTENSION,
		hWnd);
}

LPVOID SetWindowExtension(HANDLE hWnd,LPVOID lpNewExt)
{
	SYSCALL_PARAM_2(
		SYSCALL_SETWINDOWEXTENSION,
		hWnd,
		lpNewExt);
}

HANDLE GetParentWindow(HANDLE hWnd)
{
	SYSCALL_PARAM_1(
		SYSCALL_GETPARENTWINDOW,
		hWnd);
}

BOOL GetWindowRect(HANDLE hWnd,__RECT* pRect,DWORD dwIndicator)
{
	SYSCALL_PARAM_3(
		SYSCALL_GETWINDOWRECT,
		hWnd,
		pRect,
		dwIndicator);
}

BOOL IsChild(HANDLE hParent,HANDLE hChild)
{
	SYSCALL_PARAM_2(
		SYSCALL_ISCHILD,
		hParent,
		hChild);
}

DWORD HitTest(HANDLE hWnd,int x,int y)
{
	SYSCALL_PARAM_3(
		SYSCALL_HITTEST,
		hWnd,
		x,
		y);
}

BOOL SendWindowMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg)
{
	SYSCALL_PARAM_2(
		SYSCALL_SENDWINDOWMESSAGE,
		hWnd,
		pWndMsg);
}

BOOL DispatchWindowMessage(__WINDOW_MESSAGE* pWndMsg)
{
	SYSCALL_PARAM_1(
		SYSCALL_DISPATCHWINDOWMESSAGE,
		pWndMsg);
}

DWORD DefWindowProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	SYSCALL_PARAM_4(
		SYSCALL_DEFWINDOWPROC,
		hWnd,
		message,
		wParam,
		lParam);
}

VOID PostQuitMessage(int nExitCode)
{
	SYSCALL_PARAM_1(
		SYSCALL_POSTQUITMESSAGE,
		nExitCode);
}

//The following syscall's proto-type is in gdi.h.
HANDLE CreateFont(int width,int height,int chspace,int lnspace)
{
	SYSCALL_PARAM_4(
		SYSCALL_CREATEFONT,
		width,
		height,
		chspace,
		lnspace);
}

VOID DestroyFont(HANDLE hFont)
{
	SYSCALL_PARAM_1(
		SYSCALL_DESTROYFONT,
		hFont);
}

DWORD GetTextMetric(HANDLE hDC,TCHAR* pszText,DWORD dwInfoClass)
{
	SYSCALL_PARAM_3(
		SYSCALL_GETTEXTMETRIC,
		hDC,
		pszText,
		dwInfoClass);
}

HANDLE CreateBrush(BOOL bTransparent,__COLOR color)
{
	SYSCALL_PARAM_2(
		SYSCALL_CREATEBRUSH,
		bTransparent,
		color);
}

VOID DestroyBrush(HANDLE hBrush)
{
	SYSCALL_PARAM_1(
		SYSCALL_DESTROYBRUSH,
		hBrush);
}

HANDLE CreatePen(DWORD type,int width,__COLOR color)
{
	SYSCALL_PARAM_3(
		SYSCALL_CREATEPEN,
		type,
		width,
		color);
}

VOID DestroyPen(HANDLE hPen)
{
	SYSCALL_PARAM_1(
		SYSCALL_DESTROYPEN,
		hPen);
}

BOOL PtInRect(__RECT* pRect,int x,int y)
{
	SYSCALL_PARAM_3(
		SYSCALL_PTINRECT,
		pRect,
		x,
		y);
}

HANDLE CreateDeviceContext(DWORD dwDCType,HANDLE hDevice,HANDLE hWnd,__REGION* pRegion)
{
	SYSCALL_PARAM_4(
		SYSCALL_CREATEDEVICECONTEXT,
		dwDCType,
		hDevice,
		hWnd,
		pRegion);
}

VOID DestroyDeviceContext(HANDLE hDC)
{
	SYSCALL_PARAM_1(
		SYSCALL_DESTROYDEVICECONTEXT,
		hDC);
}

VOID TextOut(HANDLE hDC,int x,int y,TCHAR* pszString)
{
	SYSCALL_PARAM_4(
		SYSCALL_TEXTOUT,
		hDC,
		x,
		y,
		pszString);
}

VOID DrawPixel(HANDLE hDC,int x,int y)
{
	SYSCALL_PARAM_3(
		SYSCALL_DRAWPIXEL,
		hDC,
		x,
		y);
}

//#define SYSCALL_MOVETO                287          //MoveTo.
//#define SYSCALL_LINETO                288          //LineTo.

VOID DrawLine(HANDLE hDC,int x1,int y1,int x2,int y2)
{
	SYSCALL_PARAM_5(
		SYSCALL_DRAWLINE,
		hDC,
		x1,
		y1,
		x2,
		y2);
}

//#define SYSCALL_DRAWRECTANGLE         290          //DrawRectangle.

VOID DrawCircle(HANDLE hDC,int xc,int yc,int r,BOOL bFill)
{
	SYSCALL_PARAM_5(
		SYSCALL_DRAWCIRCLE,
		hDC,
		xc,
		yc,
		r,
		bFill);
}

HANDLE SelectPen(HANDLE hDC,HANDLE hNewPen)
{
	SYSCALL_PARAM_2(
		SYSCALL_SELECTPEN,
		hDC,
		hNewPen);
}

HANDLE SelectBrush(HANDLE hDC,HANDLE hNewBrush)
{
	SYSCALL_PARAM_2(
		SYSCALL_SELECTBRUSH,
		hDC,
		hNewBrush);
}

HANDLE SelectFont(HANDLE hDC,HANDLE hNewFont)
{
	SYSCALL_PARAM_2(
		SYSCALL_SELECTFONT,
		hDC,
		hNewFont);
}

//#define SYSCALL_GETDCWINDOW           295          //GetDCWindow.

//The following syscall's proto-type is in button.h.
HANDLE CreateButton(HANDLE hParent,TCHAR* pszText,DWORD dwButtonId,int x,int y,int cx,int cy)
{
	SYSCALL_PARAM_7(
		SYSCALL_CREATEBUTTON,
		hParent,
		pszText,
		dwButtonId,
		x,
		y,
		cx,
		cy);
}

//#define SYSCALL_SETBUTTONTEXT         297          //SetButtonText.
//#define SYSCALL_SETBUTTONCOLOR        298          //SetButtonColor.

//The following syscall's proto-type is in bmpbtn.h.
HANDLE CreateBitmapButton(HANDLE hParent,TCHAR* pszText,DWORD dwButtonId,int x,int y,int cxbmp,int cybmp,LPVOID pBitmap,
						  LPVOID pExtension)
{
	SYSCALL_PARAM_9(
		SYSCALL_CREATEBITMAPBUTTON,
		hParent,
		pszText,
		dwButtonId,
		x,
		y,
		cxbmp,
		cybmp,
		pBitmap,
		pExtension);
}
//#define SYSCALL_SETBITMAPBUTTONTEXT   300          //SetBitmapButtonText.
//#define SYSCALL_SETBITMAPBUTTONBITMAP 301          //SetBitmapButtonBitmap.

//The following syscal's proto-type is in msgbox.h.
DWORD MessageBox(HANDLE hWnd,TCHAR* pszText,TCHAR* pszTitle,UINT uType)
{
	SYSCALL_PARAM_4(
		SYSCALL_MESSAGEBOX,
		hWnd,
		pszText,
		pszTitle,
		uType);
}

#endif