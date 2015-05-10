//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 11,2009
//    Module Name               : DEFWPROC.CPP
//    Module Funciton           : 
//                                Default window procedure for all windows.
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
#include "..\INCLUDE\CLIPZONE.H"
#include "..\INCLUDE\GDI.H"
#include "..\INCLUDE\WNDMGR.H"

//External routines used by this module.
//extern VOID DrawCaption(int x,int y,int width,TCHAR* pszWndTitle,DWORD dwStyle,BOOL bInFocus);
extern VOID DrawWndCaption(__WINDOW* pWindow);

//Default window procedure,this routine should be called by any window procedure
//after specific processing.
DWORD DefWindowProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	__WINDOW_MESSAGE msg;
	DWORD            dwHitResult = 0;
	int x,y;

	switch(message)
	{
	case WM_PAINT:
		PaintWindow(hWnd);
		msg.hWnd = hWnd;
		msg.lParam = 0;
		msg.message = WM_DRAW;
		msg.wParam = 0;
		SendWindowMessage(hWnd,&msg);
		break;
	case WM_LOSTFOCUS:
		if(pWindow->dwWndStyle & WS_WITHCAPTION)
		{
			DrawWndCaption((__WINDOW*)hWnd);
		}
		break;
	case WM_ONFOCUS:
		msg.hWnd = hWnd;
		msg.lParam = 0;
		msg.wParam = 0;
		msg.message = WM_PAINT;
		SendWindowTreeMessage(hWnd,&msg);
		break;
	case WM_CHILDCLOSE:
		msg.hWnd = hWnd;
		msg.lParam = 0;
		msg.wParam = 0;
		msg.message = WM_PAINT;
		SendWindowTreeMessage(hWnd,&msg);
		break;
	case WM_CLOSE:
		CloseWindow(hWnd);
		break;
	case WM_LBUTTONUP:
		y = lParam & 0x0000FFFF;
		x = (lParam >> 16) & 0x0000FFFF;
		dwHitResult = HitTest(hWnd,x,y);
		if(HT_CLOSEBUTTON == dwHitResult)  //Send WM_CLOSE message to this window.
		{
			msg.hWnd = hWnd;
			msg.lParam = 0;
			msg.message = WM_CLOSE;
			msg.wParam = 0;
			SendWindowMessage(hWnd,&msg);
		}
		break;
	case WM_DESTROY:
		//Destroy the window itself.
		pWindow->dwSignature = 0;  //Clear the window signature.
		DestroyRegion(pWindow->pRegion);
		DestroyDeviceContext(pWindow->hWindowDC);
		DestroyDeviceContext(pWindow->hClientDC);
		KMemFree(pWindow,KMEM_SIZE_TYPE_ANY,0);
		break;
	default:
		break;
	}
	return 0;
}

