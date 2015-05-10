//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : RAWIT.H
//    Module Funciton           : 
//                                RAW input thread is one of the kernel threads
//                                in GUI module,it responses to process sytem level
//                                messages and dispatch message to proper user
//                                thread.
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
#include "..\INCLUDE\WNDMGR.H"
#include "..\INCLUDE\GUISHELL.H"

//A local helper routine,to get the specified window where the given
//point falls in.
static HANDLE GetFallWindowFromTree(HANDLE hWnd,int x,int y)
{
	__WINDOW* pWindow = (__WINDOW*)hWnd;
	__WINDOW* pChild  = NULL;
	HANDLE    hResult = NULL;

	if(NULL == pWindow)
	{
		return NULL;
	}
	//Check children first.
	pChild = pWindow->pChild;
	do{
		if(NULL == pChild)
		{
			break;
		}
		hResult = GetFallWindowFromTree((HANDLE)pChild,x,y);
		if(hResult){
			return hResult;
		}
		pChild = pChild->pNextSibling;
	}while(pChild != pWindow->pChild);
	//If reach here,it means not any child contain the specified point.
	if(PtInRegion(pWindow->pRegion,x,y))
	{
		return (HANDLE)pWindow;
	}
	return NULL;
}

//A helper routine,get the window that the specified point falls in.
static HANDLE GetFallWindow(int x,int y)
{
	/*
	__WINDOW* pWindow = WindowManager.pCurrWindow;
	if(pWindow)
	{
		if(PtInRegion(WindowManager.pCurrWindow->pRegion,x,y))  //Falls into current focus window.
		{
			return (HANDLE)WindowManager.pCurrWindow;
		}
	}*/
	//Does not fall current window or current window does not specified,check the whole window tree.
	return GetFallWindowFromTree((HANDLE)WindowManager.pWndAncestor,x,y);
}

//MOUSE MOVE event handler.
static VOID DoMouseMove(int x,int y)  //x and y is the coordinate of mouse.
{
	static int xppos = 0;  //Previous position of x.
	static int yppos = 0;  //Previous position of y.
	int xpos,ypos;
	__WINDOW_MESSAGE* pWmsg = NULL;
	MSG               msg;
	HANDLE            hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	//Send mouse move message to target window.
	pWmsg->message = WM_MOUSEMOVE;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
	
	MouseManager.RestoreMouseRect(&MouseManager,&Video,xppos,yppos);  //Restore previous screen rectangle.
	MouseManager.DrawMouse(&MouseManager,&Video,xpos,ypos);  //Draw mouse in the new location.
	xppos = xpos;
	yppos = ypos;
}

//LEFT MOUSE BUTTON DOWN event handler.
static VOID DoLButtonDown(int x,int y)
{
	__WINDOW_MESSAGE*  pWmsg = NULL;
	MSG                msg;
	int xpos,ypos;
	HANDLE hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	if((HANDLE)WindowManager.pCurrWindow != hTargetWnd)
	{
		UnfocusWindow((HANDLE)WindowManager.pCurrWindow);  //Notice the losing of focus.
		FocusWindow(hTargetWnd);
	}
	//Set current focus window and current focus thread.
	pWmsg->message = WM_LBUTTONDOWN;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
}

//LEFT MOUSE BUTTON double clicked event handler.
static VOID DoLButtonDbClk(int x,int y)
{
	__WINDOW_MESSAGE*  pWmsg = NULL;
	MSG                msg;
	int xpos,ypos;
	HANDLE hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	if((HANDLE)WindowManager.pCurrWindow != hTargetWnd)
	{
		UnfocusWindow((HANDLE)WindowManager.pCurrWindow);  //Notice the losing of focus.
		FocusWindow(hTargetWnd);
	}
	//Set current focus window and current focus thread.
	pWmsg->message = WM_LBUTTONDBLCLK;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
}

//RIGHT MOUSE BUTTON double clicked event handler.
static VOID DoRButtonDbClk(int x,int y)
{
	__WINDOW_MESSAGE*  pWmsg = NULL;
	MSG                msg;
	int xpos,ypos;
	HANDLE hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	if((HANDLE)WindowManager.pCurrWindow != hTargetWnd)
	{
		UnfocusWindow((HANDLE)WindowManager.pCurrWindow);  //Notice the losing of focus.
		FocusWindow(hTargetWnd);
	}
	//Set current focus window and current focus thread.
	pWmsg->message = WM_RBUTTONDBLCLK;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
}

//RIGHT MOUSE BUTTON DOWN event handler.
static VOID DoRButtonDown(int x,int y)
{	
	__WINDOW_MESSAGE*  pWmsg = NULL;
	MSG                msg;
	int xpos,ypos;
	HANDLE hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	if((HANDLE)WindowManager.pCurrWindow != hTargetWnd)
	{
		UnfocusWindow((HANDLE)WindowManager.pCurrWindow);  //Notice the losing of focus.
		FocusWindow(hTargetWnd);
	}
	//Set current focus window and current focus thread.
	pWmsg->message = WM_RBUTTONDOWN;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
}

//LEFT MOUSE BUTTON UP event handler.
static VOID DoLButtonUp(int x,int y)
{
	int xpos,ypos;
	__WINDOW_MESSAGE* pWmsg = NULL;
	MSG               msg;
	HANDLE            hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	//Send mouse move message to target window.
	pWmsg->message = WM_LBUTTONUP;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
}

//RIGHT MOUSE BUTTON UP event handler.
static VOID DoRButtonUp(int x,int y)
{
	int xpos,ypos;
	__WINDOW_MESSAGE* pWmsg = NULL;
	MSG               msg;
	HANDLE            hTargetWnd = NULL;

	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	MouseToScreen(&Video,x,y,&xpos,&ypos);
	hTargetWnd = GetFallWindow(xpos,ypos);
	//Send mouse move message to target window.
	pWmsg->message = WM_RBUTTONUP;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = xpos;
	pWmsg->lParam <<= 16;
	pWmsg->lParam += ypos;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
}

//ASCII key down handler.
static VOID OnAkDown(MSG* pMsg)
{
	__WINDOW_MESSAGE* pWmsg = NULL;
	HANDLE            hTargetWnd = (HANDLE)WindowManager.pCurrWindow;
	MSG               msg;

	if(NULL == hTargetWnd)  //Without focus window.
	{
		return;
	}
	//Send key down message to target window.
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	pWmsg->message = WM_KEYDOWN;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = pMsg->dwParam;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
	//Send a WM_CHAR message to target thread(window).
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	pWmsg->message = WM_CHAR;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = pMsg->dwParam;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
}

//ASCII key up handler.
static VOID OnAkUp(MSG* pMsg)
{
	__WINDOW_MESSAGE* pWmsg = NULL;
	HANDLE            hTargetWnd = (HANDLE)WindowManager.pCurrWindow;
	MSG               msg;

	if(NULL == hTargetWnd)  //Without focus window.
	{
		return;
	}
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	//Send key up message to target window.
	pWmsg->message = WM_KEYUP;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = pMsg->dwParam;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
}

//Virtual key down handler.
static VOID OnVkDown(MSG* pMsg)
{
	__WINDOW_MESSAGE* pWmsg = NULL;
	HANDLE            hTargetWnd = (HANDLE)WindowManager.pCurrWindow;
	MSG               msg;

	if(NULL == hTargetWnd)  //Without focus window.
	{
		return;
	}
	pWmsg = (__WINDOW_MESSAGE*)KMemAlloc(sizeof(__WINDOW_MESSAGE),KMEM_SIZE_TYPE_ANY);
	if(NULL == pWmsg)
	{
		return;
	}
	//Send key up message to target window.
	pWmsg->message = WM_KEYDOWN;
	pWmsg->hWnd    = hTargetWnd;
	pWmsg->wParam  = 0;
	pWmsg->lParam  = pMsg->dwParam;
	msg.dwParam = (DWORD)pWmsg;
	msg.wCommand = KERNEL_MESSAGE_WINDOW;
	//SendMessage((HANDLE)WindowManager.pCurrWindow->hOwnThread,&msg);
	SendMessage(((__WINDOW*)hTargetWnd)->hOwnThread,&msg);
}

//Virtual key up handler.
static VOID OnVkUp(MSG* pMsg)
{
	//Process is same as ASCII key up message.
	OnAkUp(pMsg);
}

//Clear screen,set the background color to button color.
//The window created in this routine as the ancestor of all windows
//in system.
static VOID InitScreen()
{
	CreateWindow(0,
		NULL,
		0,
		0,
		Video.dwScreenWidth,
		Video.dwScreenHeight,
		NULL,
		NULL,
		NULL,
		GlobalParams.COLOR_BTNFACE,
		NULL);
}

//Initialize GUI shell thread,this is the first GUI shell thread runing
//in Hello China.
static BOOL InitGuiShell()
{
	HANDLE hGuiShell = NULL;
	hGuiShell = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH,
		GuiShellEntry,
		NULL,
		NULL,
		"GUISHELL");
	if(NULL == hGuiShell)  //Can not create GUI shell.
	{
		return FALSE;
	}
	return TRUE;
}

//Entry routine of RAWIT.
DWORD RAWIT(LPVOID)
{
	MSG Msg;
	WORD x = 0;     //Mouse x scale.
	WORD y = 0;     //Mouse y scale.
	MSG  msg;
	
	//Clear screen first.
	InitScreen();
	//InitGuiShell();

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			switch(Msg.wCommand)
			{
			case KERNEL_MESSAGE_LBUTTONDOWN:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonDown(x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONUP:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonUp(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDOWN:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonDown(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONUP:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonUp(x,y);
				break;
			case KERNEL_MESSAGE_MOUSEMOVE:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoMouseMove(x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONDBCLK:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonDbClk(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDBCLK:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonDbClk(x,y);
				break;
			case KERNEL_MESSAGE_TIMER:
				break;
			case KERNEL_MESSAGE_AKUP:    //ASCII key up.
				OnAkUp(&Msg);
				break;
			case KERNEL_MESSAGE_AKDOWN:  //ASCII key down.
				OnAkDown(&Msg);
				break;
			case KERNEL_MESSAGE_VKUP:    //Virtual key up.
				OnVkUp(&Msg);
				break;
			case KERNEL_MESSAGE_VKDOWN:  //Virtual key down.
				OnVkDown(&Msg);
				break;
			case KERNEL_MESSAGE_TERMINAL: //System terminal message.
				if(GlobalParams.hGUIShell)
				{
					msg.wCommand = KERNEL_MESSAGE_TERMINAL;
					SendMessage(GlobalParams.hGUIShell,
						&msg);  //Send terminate message to GUI shell thread.
				}
				goto __TERMINAL; //End this thread.
				break;
			default:
				break;
			}
		}
	}
__TERMINAL:
	return 0;
}
