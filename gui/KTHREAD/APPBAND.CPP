//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 10,2009
//    Module Name               : APPBAND.CPP
//    Module Funciton           : 
//                                Application band window's procedure is implemented
//                                in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\stdio.h"
#include "..\INCLUDE\string.h"

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

//Header files for applications.
//#include "..\APP\CLENDAR.H"
#include "..\APP\HELLOW.H"
#include "clock.h"
#include "clend.h"

struct __APPLICATION_BUTTON{
	TCHAR*    pszAppName;        //Application name.
	DWORD     dwButtonId;        //Button ID for this application.
	DWORD     (*AppThreadEntry)(LPVOID);  //Entry point of application.
}Applications[] = {
	{"电话本(T)",    ID_TELEPHONEBOOK,  NULL},
	{"计算器(C)",    ID_CALCULATOR,     NULL},
	{"编辑器(E)",    ID_EDIT,           NULL},
	//{"日历(L)",      ID_CALENDAR,       ClendarEntry},
	{"文件(F)",      ID_FILE,           NULL},
	{"照相机(S)",    ID_CAMERA,         NULL},
	{"短信息(M)",    ID_SHORTMESSAGE,   NULL},
	{"游戏(G)",      ID_GAME,           NULL},
	{"浏览器(I)",    ID_EXPLORER,       NULL},
	{"Java(J)",      ID_JAVE,           NULL},
	{"其它...(O)",   ID_OTHER,          HelloWorld},
	{NULL,0},
};

//Local helper routine to launch appropriate application thread according to
//the application button ID.
static VOID LaunchApplication(DWORD dwButtonId)
{
	int i = 0;
	HANDLE hApplication = NULL;
	
	while(Applications[i].pszAppName)
	{
		if(dwButtonId == Applications[i].dwButtonId)  //Find.
		{
			if(NULL == Applications[i].AppThreadEntry)  //No entry point specified.
			{
				MessageBox(GuiGlobal.hMainFrame,"不能找到对应的程序。请确认对应的程序已安装。",
					"错误",MB_OK);
				return;
			}
			else  //Should start the application.
			{
				hApplication = CreateKernelThread(0,KERNEL_THREAD_STATUS_READY,PRIORITY_LEVEL_NORMAL,
					Applications[i].AppThreadEntry,(LPVOID)GuiGlobal.hMainFrame,
					NULL,Applications[i].pszAppName);
				if(NULL == hApplication)  //Can not create thread.
				{
					MessageBox(GuiGlobal.hMainFrame,"应用程序启动失败！",
						"错误",MB_OK);
				}
				return;
			}
		}
		i ++;
	}
	//If reach here,it means the application entry is not existing.
	MessageBox(GuiGlobal.hMainFrame,"不能找到对应的程序。请确认对应的程序已安装。","错误",MB_OK);
	return;
}

//WM_CREATE message handler of app band window.
static VOID OnCreate(HANDLE hWnd,HANDLE hDC)
{
	int i = 0;
	int x,y,height;  //Left-top coordinate of button and it's height.

	x = 5;
	y = 10;
	height = 32;
	while(Applications[i].pszAppName)
	{
		CreateButton(hWnd,Applications[i].pszAppName,
			Applications[i].dwButtonId,x,y,APPLICATIONBAND_WIDTH - 10,height);
		y += height;
		y += 10;    //Reserve space between application buttons.
		i += 1;
	}
}

//A helper routine to create the clock component window and clendar window,will
//return these 2 windows' handle if successful.
static BOOL CreateChildWnd(HANDLE hWnd,HANDLE* pClockWnd,HANDLE* pClendarWnd)
{
	__RECT rect;   //Application band's window rectangle.
	int cx,cy;     //Child window's width and height.
	int cxScreen = Video.dwScreenWidth;
	HANDLE hClockWnd = NULL;
	HANDLE hClendWnd = NULL;

	if((NULL == hWnd) || (NULL == pClockWnd) || (NULL == pClendarWnd))
	{
		return FALSE;
	}

	if(!GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT))
	{
		return FALSE;
	}
	cx = rect.right - rect.left;
	cy = rect.bottom - rect.top;
	cy /= 2;  //2 children windows.

	hClockWnd = CreateWindow(
		0,
		NULL,
		rect.left, //cxScreen - APPLICATIONBAND_WIDTH,
		rect.top,  //INDICATEBAND_HEIGHT + 1,
		cx,
		cy,
		ClockWndProc,  //This routine will lead GUI crash,since it uses floating point
		//operation,which additional instructions will be inserted into it by compiler VS 2010.
		hWnd,
		NULL,
		COLOR_APPLICATIONBAND,
		NULL);
	if(NULL == hClockWnd)
	{
		return FALSE;
	}
	*pClockWnd = hClockWnd;

	hClendWnd = CreateWindow(WS_WITHBORDER, //WS_WITHCAPTION | WS_WITHBORDER,
		NULL, //"日历(Clendar)-V0.5",
		rect.left,
		rect.top + cy + 5,  //Keep 5 pixels between 2 children.
		cx,  //CLENDAR_WND_WIDTH + GlobalParams.dwWndBorderWidth * 2,  //Consider the window frame's width.
		cy,  //CLENDAR_WND_HEIGHT,
		ClendarWndProc,
		hWnd,
		NULL,
		GlobalParams.COLOR_WINDOW,
		//COLOR_LIGHTORANGE,
		NULL);
	if(NULL == hClendWnd)
	{
		return FALSE;
	}
	*pClendarWnd = hClendWnd;

	return TRUE;
}

//Window procedure of application band window.
DWORD AppBandWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	HANDLE hDC = GetClientDC(hWnd);
	__WINDOW_MESSAGE msg;
	static HANDLE hClockWnd = NULL;  //Clock window's handle.
	static HANDLE hClendWnd = NULL;  //Clendar window's handle.

	switch(message)
	{
	case WM_CREATE:
		if(!CreateChildWnd(hWnd,&hClockWnd,&hClendWnd))
		{
			return 0;
		}
		break;
	case WM_DRAW:
		msg.hWnd = hWnd;
		msg.message = WM_DRAW;
		msg.wParam  = 0;
		msg.lParam  = 0;
		SendWindowChildMessage(hWnd,&msg);  //Notify all child window to re-draw.
		break;
	case WM_CLOSE:
		break;
	case WM_COMMAND:  //Button notice.
		//LaunchApplication(wParam);  //Try to start up the application thread.
		break;
	case WM_DESTROY:
		if(hClockWnd)
		{
			DestroyWindow(hClockWnd);
		}
		if(hClendWnd)
		{
			DestroyWindow(hClendWnd);
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}
