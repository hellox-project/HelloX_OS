//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : WNDMGR.CPP
//    Module Funciton           : 
//                                The implementation of windows manager object.
//                                This is the kernel object in GUI module of
//                                Hello China.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\STRING.H"
#include "..\INCLUDE\VESA.H"
#include "..\INCLUDE\VIDEO.H"
#include "..\INCLUDE\GLOBAL.H"
#include "..\INCLUDE\CLIPZONE.H"
#include "..\INCLUDE\GDI.H"
#include "..\INCLUDE\WNDMGR.H"

#include "..\INCLUDE\WORDLIB.H"

//A helper routine to draw a window's close button.
static VOID DrawCloseButton(__WINDOW* pWindow)
{
	int x = 0;  //x coordinate of close button.
	int y = 0;  //y coordinate of close button.
	int width = 0;  //Width of close button,height is same.

	if(NULL == pWindow)
	{
		return;
	}
	//Calculate the close button's position(x,y) and it's size.
	width = GlobalParams.dwWndCaptionHeight - 4;  //Keep 2 pixel away from caption's border.
	x = pWindow->x + pWindow->cx;
	x -= 2;  //Keep 2 pixel from the right side of window frame.
	x -= width;  //Now x is the left-top's x coordinate of close button.
	y = pWindow->y;
	y += 2;  //Keep 2 pixel from the top side of window frame.
	//Draw the close button.
	DrawRectangle(&Video,x,y,x + width,y + width,COLOR_WHITE,TRUE,
		GlobalParams.COLOR_CLOSEBUTTON);
	DrawLine(&Video,x,y,x + width,y + width,COLOR_WHITE);
	//Initialize the close button's position and size members of window.
	pWindow->xcb  = x;
	pWindow->ycb  = y;
	pWindow->cxcb = width;
	pWindow->cycb = width;
	DrawLine(&Video,x + width,y,x,y + width,COLOR_WHITE);
}

//A helper routine to draw window caption.
static VOID DrawCaption(int x,int y,int width,TCHAR* pszWndTitle,DWORD dwStyle,BOOL bInFocus)
{
	//Draw the window's caption.
	if(bInFocus)
	{
		DrawRectangle(&Video,x,y,x + width,y + GlobalParams.dwWndCaptionHeight,
			COLOR_BLACK,
			TRUE,
			GlobalParams.COLOR_ACTIVECAPTION);
	}
	else
	{
		DrawRectangle(&Video,x,y,x + width,y + GlobalParams.dwWndCaptionHeight,
			COLOR_BLACK,
			TRUE,
			GlobalParams.COLOR_INACTIVECAPTION);
	}
	if(pszWndTitle)
	{
		TextOut(x + 4,y + 3,(char*)pszWndTitle,COLOR_WHITE);
	}
}

//A helper routine to draw window's caption.
//This routine maybe called by other modules so we declare it as global routine.
VOID DrawWndCaption(__WINDOW* pWindow)
{
	if(NULL == pWindow)
	{
		return;
	}
	DrawCaption(pWindow->x,pWindow->y,pWindow->cx,pWindow->WndTitle,pWindow->dwWndStyle,
		(pWindow->dwWndStatus & WST_ONFOCUS) ? TRUE : FALSE);
	DrawCloseButton(pWindow);  //Draw window's CLOSE BUTTON.
}

//Draw one window's frame border.
static VOID DrawFrame(int x,int y,int width,int height)
{
	//Draw window border first.
	for(DWORD i = 0;i < GlobalParams.dwWndBorderWidth;i ++)
	{
		DrawRectangle(&Video,x + i,y + i,x + width -i,y + height -i,
			GlobalParams.COLOR_WINDOWFRAME,
			FALSE,
			GlobalParams.COLOR_WINDOW);
	}
}

//Draw window's child area.
static VOID DrawClient(int x,int y,int width,int height,__COLOR fillclr)
{
	DrawRectangle(&Video,x,y,x + width,y + height,fillclr,
		TRUE,fillclr);
}

//Draw a window given it's pointer.
static VOID DrawWindow(__WINDOW* pWindow)
{
	if(NULL == pWindow)
	{
		return;
	}
	if(pWindow->dwWndStyle & WS_WITHCAPTION)  //With caption,draw caption first.
	{
		DrawWndCaption(pWindow);
	}
	if(pWindow->dwWndStyle & WS_WITHBORDER)   //With border,draw it then.
	{
		DrawFrame(pWindow->x,pWindow->y,pWindow->cx,pWindow->cy);
	}
	DrawClient(pWindow->xclient,pWindow->yclient,pWindow->cxclient,pWindow->cyclient,pWindow->clrBackground);
}

//To paint a window on screen.
VOID PaintWindow(HANDLE hWnd)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	if(NULL == pWindow)
	{
		return;
	}
	if(pWindow->dwWndStyle & WS_WITHCAPTION)  //With caption,draw caption first.
	{
		DrawWndCaption(pWindow);
	}
	if(pWindow->dwWndStyle & WS_WITHBORDER)   //With border,draw it then.
	{
		DrawFrame(pWindow->x,pWindow->y,pWindow->cx,pWindow->cy);
	}
	DrawClient(pWindow->xclient,pWindow->yclient,pWindow->cxclient,pWindow->cyclient,pWindow->clrBackground);
}

//The CreateWindow routine.This routine is one of the most important routines in
//GUI module.
HANDLE CreateWindow(DWORD dwWndStyle,TCHAR* pszWndTitle,int x,
					int y,int cx,int cy,__WINDOW_PROC WndProc,
					HANDLE hParent,HANDLE hMenu,__COLOR clrbackground,
					LPVOID lpReserved)
{
	__WINDOW*           pWindow  = NULL;
	__WINDOW*           pParent  = NULL;
	__WINDOW*           pChild   = NULL;  //This is parent's child.
	BOOL                bResult  = FALSE;
	DWORD               dwFlags;
	HANDLE              hWindowDC = NULL;
	HANDLE              hClientDC = NULL;
	__REGION*           pRegion   = NULL;
	__CLIPZONE*         pClipZone = NULL; //Initial clip zone rectangle.
	__WINDOW_MESSAGE    wmsg;

	//Fist verity the parent object's validity.
	if(hParent)
	{
		if(((__WINDOW*)hParent)->dwSignature != WINDOW_SIGNATURE)
		{
			goto __TERMINAL;
		}
	}

	pWindow = (__WINDOW*)KMemAlloc(sizeof(__WINDOW),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWindow)  //Can not allocate memory.
	{
		goto __TERMINAL;
	}
	pWindow->clrBackground  = clrbackground;
	pWindow->dwWndStatus    = WST_NORMAL;
	pWindow->dwWndStyle     = dwWndStyle;
	pWindow->hCursor        = NULL;
	pWindow->hFocusChild    = NULL;
	pWindow->hIcon          = NULL;
	pWindow->hOwnThread     = GetCurrentThread();
	pWindow->WndProc        = (NULL == WndProc) ? DefWindowProc : WndProc;
	pWindow->lpWndExtension = NULL;  //Very important,this member maybe re-initialized by common ctrls.
	pWindow->pChild         = NULL;
	pWindow->pParent        = NULL;
	pWindow->pNextSibling   = NULL;
	pWindow->pPrevSibling   = NULL;
	pWindow->dwSignature    = WINDOW_SIGNATURE;

	pClipZone = (__CLIPZONE*)KMemAlloc(sizeof(__CLIPZONE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pClipZone)
	{
		goto __TERMINAL;
	}
	//Initialize the initial clip zone rectangle.
	pClipZone->x = x;
	pClipZone->y = y;
	pClipZone->width  = cx;
	pClipZone->height = cy;
	pClipZone->pNext = pClipZone->pPrev = pClipZone;

	pRegion = CreateRegion();
	if(NULL == pRegion)
	{
		goto __TERMINAL;
	}
	//Add the initial clip zone rectange into region.
	AddClipZone(pRegion,pClipZone);

	//Create window DC and it's client area DC.
	hWindowDC = CreateDeviceContext(DC_TYPE_SCREEN | DC_TYPE_WINDOW,
		&Video,
		(HANDLE)pWindow,
		pRegion);
	if(NULL == hWindowDC)
	{
		goto __TERMINAL;
	}
	hClientDC = CreateDeviceContext(DC_TYPE_SCREEN | DC_TYPE_CLIENT,
		&Video,
		(HANDLE)pWindow,
		pRegion);
	if(NULL == hClientDC)
	{
		goto __TERMINAL;
	}

	if(pszWndTitle)
	{
		strcpy(pWindow->WndTitle,pszWndTitle);
	}
	pWindow->hWindowDC      = hWindowDC;
	pWindow->hClientDC      = hClientDC;
	pWindow->pRegion        = pRegion;

	//Initialize window's position and size members.
	pWindow->x              = x;
	pWindow->y              = y;
	pWindow->cx             = (cx == 0) ? GlobalParams.dwWndDefCx : cx;
	pWindow->cy             = (cy == 0) ? GlobalParams.dwWndDefCy : cy;
	pWindow->cxclient       = pWindow->cx;
	pWindow->cyclient       = pWindow->cy;
	pWindow->xclient        = x;
	pWindow->yclient        = y;

	if(pWindow->dwWndStyle & WS_WITHBORDER)  //Window has border.
	{
		if((pWindow->cx <= GlobalParams.dwWndBorderWidth * 4) || //cx too small.
		   (pWindow->cy <= GlobalParams.dwWndBorderWidth * 4))   //cy too small.
		{
			goto __TERMINAL;
		}
		pWindow->xclient += GlobalParams.dwWndBorderWidth;
		pWindow->yclient += GlobalParams.dwWndBorderWidth;
		pWindow->cxclient -= GlobalParams.dwWndBorderWidth * 2;
		pWindow->cyclient -= GlobalParams.dwWndBorderWidth * 2;
	}
	if(pWindow->dwWndStyle & WS_WITHCAPTION)  //Window with caption.
	{
		if(pWindow->cyclient <= GlobalParams.dwWndCaptionHeight)  //Height too small.
		{
			goto __TERMINAL;
		}
		pWindow->yclient  += GlobalParams.dwWndCaptionHeight;
		pWindow->cyclient -= GlobalParams.dwWndCaptionHeight;
	}

	//Insert this window into window tree.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	if(NULL == WindowManager.pWndAncestor)  //This is the first window.
	{
		WindowManager.pWndAncestor = pWindow;
		pWindow->pChild            = NULL;
		pWindow->pParent           = NULL;
		pWindow->pPrevSibling      = pWindow;
		pWindow->pNextSibling      = pWindow;
	}
	else
	{
		pParent = (NULL == hParent) ? WindowManager.pWndAncestor : (__WINDOW*)hParent;
		pWindow->pParent = pParent;
		//Insert this window into it's parent child list.
		if(NULL == pParent->pChild)  //Has not child yet.
		{
			pParent->pChild = pWindow;
			pWindow->pPrevSibling = pWindow;
			pWindow->pNextSibling = pWindow;
		}
		else
		{
			pChild = pParent->pChild;
			pWindow->pNextSibling = pChild;
			pWindow->pPrevSibling = pChild->pPrevSibling;
			pChild->pPrevSibling->pNextSibling = pWindow;
			pChild->pPrevSibling  = pWindow;
			pParent->pChild = pWindow;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//If this new created window replace the previous focus window,
	//then send LOSSFOCUS message to this window and it's owner thread.
	if(pChild)
	{
		//UnfocusWindow((HANDLE)pChild);
	}

	if(!(dwWndStyle & WS_HIDE))  //Window will be show after created.
	{
		//Unfocus the previous focused window if existing.
		if(WindowManager.pCurrWindow)
		{
			UnfocusWindow((HANDLE)WindowManager.pCurrWindow);
		}
		FocusWindow((HANDLE)pWindow);  //Set current window as focus.
	}

	//Send the WM_CREATE and WM_PAINT message to this window.
	wmsg.hWnd     = (HANDLE)pWindow;
	wmsg.message  = WM_CREATE;
	wmsg.wParam   = 0;
	wmsg.lParam   = 0;
	SendWindowMessage(wmsg.hWnd,&wmsg);

	wmsg.message = WM_PAINT;
	SendWindowMessage(wmsg.hWnd,&wmsg);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(pWindow)
		{
			KMemFree(pWindow,KMEM_SIZE_TYPE_ANY,0);
			pWindow = NULL;
		}
		if(pClipZone)
		{
			if(!pRegion)  //Has not been added into region object yet.
			{
				KMemFree(pClipZone,KMEM_SIZE_TYPE_ANY,0);
			}
		}
		if(pRegion)
		{
			DestroyRegion(pRegion); //Clip zone rectangle object will be released too.
		}
		if(hWindowDC)
		{
			DestroyDeviceContext(hWindowDC);
		}
		if(hClientDC)
		{
			DestroyDeviceContext(hClientDC);
		}
	}
	return (HANDLE)pWindow;
}

//DestroyWindow.
VOID DestroyWindow(HANDLE hWindow)
{
	__WINDOW* pWindow = (__WINDOW*)hWindow;
	DWORD     dwFlags;
	__WINDOW_MESSAGE msg;

	if(NULL == pWindow)
	{
		return;
	}
	//Verify the window's validity.
	if(pWindow->dwSignature != WINDOW_SIGNATURE)  //Is not a window.
	{
		return;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pWindow->pNextSibling->pPrevSibling = pWindow->pPrevSibling;
	pWindow->pPrevSibling->pNextSibling = pWindow->pNextSibling;
	if(pWindow->pParent)  //Has parent.
	{
		if(pWindow == pWindow->pParent->pChild)  //This window is parent's first child.
		{
			pWindow->pParent->pChild = pWindow->pNextSibling;
		}
		if(pWindow == pWindow->pParent->pChild)  //Parent only has one(pWindow) child.
		{
			pWindow->pParent->pChild = NULL;
		}
	}
	//If current window in focus,then de-focus it.
	if(pWindow == WindowManager.pCurrWindow)
	{
		WindowManager.pCurrWindow = NULL;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//Destroy all child windows of this one.
	while(pWindow->pChild)
	{
		DestroyWindow(pWindow->pChild);  //This is a recursion routine.
	}
	//Send WM_DESTROY message to this window.
	msg.hWnd = hWindow;
	msg.message = WM_DESTROY;
	msg.wParam  = 0;
	msg.lParam  = 0;
	SendWindowMessage(hWindow,&msg);

	/*
	//Destroy the window itself.
	pWindow->dwSignature = 0;  //Clear the window signature.
	DestroyRegion(pWindow->pRegion);
	DestroyDeviceContext(pWindow->hWindowDC);
	DestroyDeviceContext(pWindow->hClientDC);
	KMemFree(pWindow,KMEM_SIZE_TYPE_ANY,0);
	*/
}

//Close one window.This routine calls DestroyWindow after send WM_CHILDCLOSE message to it's
//parent if has.
VOID CloseWindow(HANDLE hWnd)
{
	__WINDOW*        pWindow = (__WINDOW*)hWnd;
	__WINDOW_MESSAGE msg;
	HANDLE           hParent = NULL;
	DWORD            dwFlags;

	if(NULL == hWnd)
	{
		return;
	}
	if(pWindow->dwSignature != WINDOW_SIGNATURE)
	{
		return;
	}
	hParent = (HANDLE)pWindow->pParent;  //Save parent.
	//Send WM_CLOSE message to all it's child(ren).
	msg.hWnd = hWnd;
	msg.message = WM_CLOSE;
	msg.wParam  = 0;
	msg.lParam  = 0;
	SendWindowChildMessage(hWnd,&msg);

	//Dettach this window from window tree.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	pWindow->pNextSibling->pPrevSibling = pWindow->pPrevSibling;
	pWindow->pPrevSibling->pNextSibling = pWindow->pNextSibling;
	if(pWindow->pParent)  //Has parent.
	{
		if(pWindow == pWindow->pParent->pChild)  //This window is parent's first child.
		{
			pWindow->pParent->pChild = pWindow->pNextSibling;
		}
		if(pWindow == pWindow->pParent->pChild)  //Parent only has one(pWindow) child.
		{
			pWindow->pParent->pChild = NULL;
		}
	}
	//If current window in focus,then de-focus it.
	if(pWindow == WindowManager.pCurrWindow)
	{
		WindowManager.pCurrWindow = NULL;
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
	//DestroyWindow(hWnd);
	//Set the window's status as closed.
	pWindow->dwWndStatus = WST_CLOSED;

	if(hParent)  //Has parent.
	{
		msg.hWnd = hParent;
		msg.lParam = (DWORD)hWnd;
		msg.wParam = 0;
		msg.message = WM_CHILDCLOSE;
		SendWindowMessage(hParent,&msg);  //Send WM_CHILDCLOSE message to it's parent.
	}
}

//Get a specified window's status.
DWORD GetWindowStatus(HANDLE hWnd)
{
	__WINDOW* pWnd = (__WINDOW*)hWnd;
	if(NULL == pWnd)
	{
		return 0L;
	}
	return pWnd->dwWndStatus;
}

//Get a window's rectangle information.
BOOL GetWindowRect(HANDLE hWnd,__RECT* pRect,DWORD dwIndicator)
{
	__WINDOW*      pWindow = (__WINDOW*)hWnd;

	if(NULL == pRect)
	{
		return FALSE;
	}
	if(NULL == pWindow)  //Should return the window ancestor's size.
	{
		pWindow = WindowManager.pWndAncestor;
	}
	switch(dwIndicator)
	{
	case GWR_INDICATOR_WINDOW:
		pRect->left    = pWindow->x;
		pRect->right   = pWindow->x + pWindow->cx;
		pRect->top     = pWindow->y;
		pRect->bottom  = pWindow->y + pWindow->cy;
		return TRUE;
		break;
	case GWR_INDICATOR_CLIENT:
		pRect->left    = pWindow->xclient;
		pRect->right   = pWindow->xclient + pWindow->cxclient;
		pRect->top     = pWindow->yclient;
		pRect->bottom  = pWindow->yclient + pWindow->cyclient;
		return TRUE;
		break;
	default:
		break;
	}
	return FALSE;
}

//Check if hChild is one child of hParent.
BOOL IsChild(HANDLE hParent,HANDLE hChild)
{
	__WINDOW*   pParent = (__WINDOW*)hParent;
	__WINDOW*   pChild  = NULL;

	if((NULL == hParent) || (NULL == hChild))
	{
		return FALSE;
	}
	pChild = pParent->pChild;
	do{
		if(NULL == pChild)
		{
			return FALSE;
		}
		if((HANDLE)pChild == hChild)
		{
			return TRUE;
		}
		//Check if the hChild is child of pChild.
		if(IsChild((HANDLE)pChild,hChild))
		{
			return TRUE;
		}
		//Check next sibling.
		pChild = pChild->pNextSibling;
	}while(pChild != pParent->pChild);

	return FALSE;
}

//HitTest,get the window's position given a point,x and y.
//If the point has not fallen into hWnd,0 will be returned,
//else the actual position indicator will be returned.
DWORD HitTest(HANDLE hWnd,int x,int y)
{
	__WINDOW*        pWindow = (__WINDOW*)hWnd;
	__RECT           rect;

	if(NULL == pWindow)
	{
		return 0;
	}
	if(pWindow->dwWndStyle & WS_WITHCAPTION)
	{
		rect.left   = pWindow->xcb;
		rect.right  = pWindow->xcb + pWindow->cxcb;
		rect.top    = pWindow->ycb;
		rect.bottom = pWindow->ycb + pWindow->cycb;
		if(PtInRect(&rect,x,y))  //Falls into close button.
		{
			return HT_CLOSEBUTTON;
		}
		rect.left   = pWindow->x;
		rect.right  = pWindow->x + pWindow->cx;
		rect.top    = pWindow->y;
		rect.bottom = pWindow->y + GlobalParams.dwWndCaptionHeight;
		if(PtInRect(&rect,x,y))  //Falls into caption rectangle.
		{
			return HT_CAPTION;
		}
	}
	rect.left   = pWindow->xclient;
	rect.right  = pWindow->xclient + pWindow->cxclient;
	rect.top    = pWindow->yclient;
	rect.bottom = pWindow->yclient + pWindow->cyclient;
	if(PtInRect(&rect,x,y))  //Falls into client area.
	{
		return HT_CLIENT;
	}
	return 0;
}

//Set the specific window as current focus window.
VOID FocusWindow(HANDLE hWnd)
{
	__WINDOW_MESSAGE* pWmsg = NULL;
	MSG               msg;
	__WINDOW*         pWnd  = (__WINDOW*)hWnd;
	__WINDOW*         pParent = NULL;
	DWORD             dwFlags = 0;

	if(NULL == hWnd)
	{
		return;
	}
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)  //Can not allocate memory.
	{
		return;
	}
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	//Set current focus window and current thread as given window and it's own thread.
	WindowManager.pCurrWindow = pWnd;
	WindowManager.hCurrThread = pWnd->hOwnThread;
	pWnd->dwWndStatus |= WST_ONFOCUS;
	//Adjust the order of this window in sibling list to head.
	pParent = pWnd->pParent;
	if(pParent)  //Has parent.
	{
		if(pParent->pChild != pWnd)  //Is not the first one in sibling list.
		{
			//Delete itself from sibling list.
			pWnd->pPrevSibling->pNextSibling = pWnd->pNextSibling;
			pWnd->pNextSibling->pPrevSibling = pWnd->pPrevSibling;
			//Add as sibling list header.
			pWnd->pPrevSibling = pParent->pChild->pPrevSibling;
			pWnd->pNextSibling = pParent->pChild;
			pParent->pChild->pPrevSibling->pNextSibling = pWnd;
			pParent->pChild->pPrevSibling = pWnd;
			pParent->pChild = pWnd;
		}
		else //Already is the fist one in sibling list.
		{
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL,dwFlags);

	//Send On focus message to target thread.
	pWmsg->hWnd     = hWnd;
	pWmsg->message  = WM_ONFOCUS;
	pWmsg->wParam   = 0;
	pWmsg->lParam   = 0;

	msg.wCommand  = KERNEL_MESSAGE_WINDOW;
	msg.wParam    = 0;
	msg.dwParam   = (DWORD)pWmsg;
	SendMessage(((__WINDOW*)hWnd)->hOwnThread,&msg);
}

//Make the specific window un-focus.
//After this routine is called,the corresponding FocusWindow must be called
//to set current focus window.
VOID UnfocusWindow(HANDLE hWnd)
{
	__WINDOW_MESSAGE* pWmsg = NULL;
	__WINDOW*         pWnd  = (__WINDOW*)hWnd;
	MSG               msg;

	if(NULL == pWnd)
	{
		return;
	}
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	//Cancel current focus window and current thread.
	WindowManager.pCurrWindow = NULL;
	WindowManager.hCurrThread = NULL;
	pWnd->dwWndStatus &= ~WST_ONFOCUS;  //Clear the unfocus flag.

	pWmsg->hWnd    = hWnd;
	pWmsg->message = WM_LOSTFOCUS;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = 0;

	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	msg.wParam   = 0;
	msg.dwParam  = (DWORD)pWmsg;
	SendMessage(pWnd->hOwnThread,&msg);
}

//Send message to a window tree,i.e,not only send message to the specified window,but
//also send message to all it's children if existing.
VOID SendWindowTreeMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg)
{
	__WINDOW*  pWnd   = (__WINDOW*)hWnd;
	__WINDOW*  pChild = NULL;
	__WINDOW_MESSAGE msg;

	if((NULL == pWnd) || (NULL == pWndMsg))
	{
		return;
	}
	if(hWnd != pWndMsg->hWnd)
	{
		return;
	}
	//Send message to this window first.
	if(NULL == pWnd->WndProc)
	{
		return;
	}
	//pWnd->WndProc(hWnd,pWndMsg->message,pWndMsg->wParam,pWndMsg->lParam);
	SendWindowMessage(hWnd,pWndMsg);

	//Then send message to all children of this window.
	//CAUTION: In reverse order,that is,the last child is sent first.
	if(NULL == pWnd->pChild)  //Without any child.
	{
		return;
	}
	pChild = pWnd->pChild->pPrevSibling;
	do{
		if(NULL == pChild)
		{
			break;
		}
		msg.hWnd    = (HANDLE)pChild;
		msg.lParam  = pWndMsg->lParam;
		msg.message = pWndMsg->message;
		msg.wParam  = pWndMsg->wParam;
		SendWindowTreeMessage(msg.hWnd,&msg);  //Recursion call.
		pChild = pChild->pPrevSibling;
	}while(pChild != pWnd->pChild->pPrevSibling);
}

//Send a specific message to all it's child(ren).
VOID SendWindowChildMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg)
{
	__WINDOW*  pWnd   = (__WINDOW*)hWnd;
	__WINDOW*  pChild = NULL;
	__WINDOW_MESSAGE msg;

	if((NULL == pWnd) || (NULL == pWndMsg))
	{
		return;
	}
	if(hWnd != pWndMsg->hWnd)
	{
		return;
	}
	//Then send message to all children of this window.
	//CAUTION: In reverse order,that is,the last child is sent first.
	if(NULL == pWnd->pChild)  //Without any child.
	{
		return;
	}
	pChild = pWnd->pChild->pPrevSibling;
	do{
		if(NULL == pChild)
		{
			break;
		}
		msg.hWnd    = (HANDLE)pChild;
		msg.lParam  = pWndMsg->lParam;
		msg.message = pWndMsg->message;
		msg.wParam  = pWndMsg->wParam;
		SendWindowTreeMessage(msg.hWnd,&msg);  //Recursion call.
		pChild = pChild->pPrevSibling;
	}while(pChild != pWnd->pChild->pPrevSibling);
}

//Send a KERNEL_MESSAGE_TERMINAL to current kernel thread.
VOID PostQuitMessage(int nExitCode)
{
	HANDLE hThread = GetCurrentThread();
	MSG msg;

	msg.wCommand = KERNEL_MESSAGE_TERMINAL;
	msg.dwParam  = nExitCode;
	msg.wParam   = 0;
	SendMessage(hThread,&msg);
}

//Get window's DC.
HANDLE GetWindowDC(HANDLE hWnd)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	if(NULL == pWindow)
	{
		return NULL;
	}
	return (HANDLE)pWindow->hWindowDC;
}

//Get window client area's DC.
HANDLE GetClientDC(HANDLE hWnd)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	if(NULL == pWindow)
	{
		return NULL;
	}
	return (HANDLE)pWindow->hClientDC;
}

//Get window's extension pointer.
LPVOID GetWindowExtension(HANDLE hWnd)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	if(NULL == pWindow)
	{
		return NULL;
	}
	return pWindow->lpWndExtension;
}

//Set window's extension pointer and returns the old one.
LPVOID SetWindowExtension(HANDLE hWnd,LPVOID lpNewExt)
{
	__WINDOW*  pWindow = (__WINDOW*)hWnd;
	LPVOID     pOldExt = NULL;

	if(NULL == pWindow)
	{
		return NULL;
	}
	pOldExt = pWindow->lpWndExtension;
	pWindow->lpWndExtension = lpNewExt;
	return pOldExt;
}

//Get parent window.
HANDLE GetParentWindow(HANDLE hWnd)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	if(NULL == pWindow)
	{
		return NULL;
	}
	return (HANDLE)pWindow->pParent;
}

//Send a message to window.
BOOL SendWindowMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg)
{
	__WINDOW*  pWnd = (__WINDOW*)hWnd;

	if((NULL == hWnd) || (NULL == pWndMsg))
	{
		return FALSE;
	}
	if(hWnd != pWndMsg->hWnd)
	{
		return FALSE;
	}
	if(NULL == pWnd->WndProc)
	{
		return FALSE;
	}
	if(pWnd->dwSignature != WINDOW_SIGNATURE)
    {
        return FALSE;
    }
    //Some messages,such as WM_DRAW and WM_PAINT,should not send to
    //the window in CLOSED status.
    if(pWnd->dwWndStatus & WST_CLOSED)
    {
         if((pWndMsg->message == WM_DRAW) || (pWndMsg->message == WM_PAINT))
         {
             return FALSE;
         }
    }
	return pWnd->WndProc(hWnd,
		pWndMsg->message,
		pWndMsg->wParam,
		pWndMsg->lParam);
}

//Dispatch a window message to a appropriate window proc.
//After handle the window message,this routine will release the
//memory occupied by pWndMsg.Because this block of memory is
//allocated by the sender thread.
BOOL DispatchWindowMessage(__WINDOW_MESSAGE* pWndMsg)
{
	if(NULL == pWndMsg)
	{
		return FALSE;
	}
	BOOL bResult = SendWindowMessage(pWndMsg->hWnd,
		pWndMsg);
	KMemFree(pWndMsg,KMEM_SIZE_TYPE_ANY,0);
	return TRUE;
}

//Initialize routine of Window Manager object.
static BOOL Initialize(__WINDOW_MANAGER* pWndManager)
{
	return TRUE;
}

//Uninitialize routine of Window Manager object.
static VOID Uninitialize(__WINDOW_MANAGER* pWndManager)
{
	return;
}

//Global object declaration: WindowManager.
__WINDOW_MANAGER WindowManager = {
	NULL,          //pWndAncestor.
	NULL,          //Current thread handle.
	NULL,          //Current focus window.
	Initialize,    //Initialize routine.
	Uninitialize,  //Uninitialize routine.
};

