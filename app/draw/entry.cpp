#include "kapi.h"
#include "stdio.h"
#include "math.h"

#define CLK_FACE_COLOR   0x00FFFFFF  //White
#define CLK_SCALE_COLOR  0x0000C0FF  //Same as task band.

//A null routine required by linker when floating point operation is enabled.
void main()
{
}

//A helper local routine used to draw clock scale.
static void DrawClockScale(HANDLE hDC,int cx,int cy,int r)
{
	double _minAngle    = PI / 30;   //The angle value between each minute scale.
	int startx,starty;               //Start coordinate of clock indicator line.
	int endx,endy;                   //End coordinate of clock indicator line.
	int innerR1         = r * 9 / 10; //Minute indicator line length only occupy 1/10 of radius.
	int innerR2         = r * 4 / 5;  //5 minutes indicator line is 1/5 of radius.

	//Draw 60 lines,one for each minutes.
	for(int i = 0;i < 60;i ++)
	{
		if(i % 5 == 0) //5 minutes line.
		{
			startx = cx + (int)(innerR2 * cos(i * _minAngle));
			starty = cy + (int)(innerR2 * sin(i * _minAngle));
		}
		else
		{
			startx = cx + (int)(innerR1 * cos(i * _minAngle));
			starty = cy + (int)(innerR1 * sin(i * _minAngle));
		}
		endx = cx + (int)(r * cos(i * _minAngle));
		endy = cy + (int)(r * sin(i * _minAngle));
		DrawLine(hDC,startx,starty,endx,endy);
	}
}

//A local helper routine used to draw clock pointer.
static void _DrawClockPointer(HANDLE hDC,int cx,int cy,int r,int hour,int minute,int second)
{
	double angMin,angHur,angSec;  //Angle of hour,minute and second pointer.
	int r1 = r * 3 / 4;           //Second pointer's length.
	int r2 = r * 2 / 3;           //Minute pointer's length.
	int r3 = r / 2;               //Hour pointer's length.
	int endx,endy;

	angSec = PI * second / 30 - PI / 2;
	angMin = PI * minute / 30 - PI / 2 + angSec / 60;
	angHur = PI * hour / 6 - PI / 2 + angMin / 60;

	//Draw second pointer.
	endx = cx + (int)(r1 * cos(angSec));
	endy = cy + (int)(r1 * sin(angSec));
	DrawLine(hDC,cx,cy,endx,endy);
	//Draw minute pointer.
	endx = cx + (int)(r2 * cos(angMin));
	endy = cy + (int)(r2 * sin(angMin));
	DrawLine(hDC,cx,cy,endx,endy);
	//Draw hour pointer.
	endx = cx + (int)(r3 * cos(angHur));
	endy = cy + (int)(r3 * sin(angHur));
	DrawLine(hDC,cx,cy,endx,endy);
}

//A helper routine to erase clock's pointers.
static void EraseClockPointer(HANDLE hDC,int cx,int cy,int r,int hour,int minute,int second)
{
	HANDLE hOldPen  = NULL;
	HANDLE hNewPen  = NULL;

	hNewPen = CreatePen(0,1,CLK_FACE_COLOR);
	if(NULL == hNewPen)
	{
		return;
	}
	hOldPen = SelectPen(hDC,hNewPen);
	_DrawClockPointer(hDC,cx,cy,r,hour,minute,second);
	//Restore the old pen.
	SelectPen(hDC,hOldPen);
	DestroyPen(hNewPen);
}

//Draw clock's pointer.
static void DrawClockPointer(HANDLE hDC,int cx,int cy,int r,int hour,int minute,int second)
{
	HANDLE hOldPen = NULL;
	HANDLE hNewPen = NULL;
	HANDLE hOldBrush = NULL;
	HANDLE hNewBrush = NULL;

	hNewPen = CreatePen(0,1,CLK_SCALE_COLOR);
	if(NULL == hNewPen)
	{
		return;
	}
	hOldPen = SelectPen(hDC,hNewPen);
	_DrawClockPointer(hDC,cx,cy,r,hour,minute,second);
	//Restore DC's old pen.
	SelectPen(hDC,hOldPen);
	DestroyPen(hNewPen);

	//Draw the circle in the center of clock face.
	hNewBrush = CreateBrush(FALSE,RGB(0x80,0,0));
	if(NULL == hNewBrush)
	{
		return;
	}
	hOldBrush = SelectBrush(hDC,hNewBrush);
	DrawCircle(hDC,cx,cy,10,TRUE);
	SelectPen(hDC,hOldPen);
}

//Draw clock face.
static void DrawClockFace(HANDLE hWnd)
{
	HANDLE hDC = NULL;
	__RECT rect;       //Window client area's rect.
	HANDLE hPen = NULL;
	HANDLE hBrush = NULL;
	HANDLE hOldBrush = NULL;
	HANDLE hOldPen = NULL;
	int cx,cy,r;

	hDC = GetClientDC(hWnd);
	if(!GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT))
	{
		goto __TERMINAL;
	}
	//Calculate the circle's center coordinate and radius.
	cx = (rect.right - rect.left) / 2;
	cy = (rect.bottom - rect.top) / 2;
	r  = cx > cy ? cy : cx;
	r -= 10;  //Keep 10 pixel space between circle and window frame.

	//Create the pen and brush object used to draw circle.
	hPen = CreatePen(0,1,CLK_SCALE_COLOR);
	if(NULL == hPen)
	{
		goto __TERMINAL;
	}
	hBrush = CreateBrush(FALSE,CLK_FACE_COLOR);
	if(NULL == hBrush)
	{
		goto __TERMINAL;
	}
	hOldPen = SelectPen(hDC,hPen);
	hOldBrush = SelectBrush(hDC,hBrush);
	//Draw the clock face circle now.
	DrawCircle(hDC,cx,cy,r,FALSE);
	DrawCircle(hDC,cx,cy,r - 1,FALSE);
	DrawCircle(hDC,cx,cy,r - 2,FALSE);
	DrawCircle(hDC,cx,cy,r - 3,TRUE);
	DrawClockScale(hDC,cx,cy,r);  //Draw clock's scale.
	//Restore original pen and brush for this window's DC.
	SelectPen(hDC,hOldPen);
	SelectBrush(hDC,hOldBrush);

__TERMINAL:
	if(hPen)
	{
		DestroyPen(hPen);
	}
	if(hBrush)
	{
		DestroyBrush(hBrush);
	}
	return;
}

//Window procedure of Hello World.
static DWORD HelloWorldProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	static HANDLE hDC = NULL;
	static int cx = 0,cy = 0,r = 0;       //Circle's center and radius.
	__RECT rect;       //Window's rectangle.
	static HANDLE hTimer = NULL;     //Timer handle.
	static int hour = 3;
	static int minu = 25;
	static int secd = 0;

	switch(message)
	{
	case WM_CREATE:    //Will receive this message when the window is created.
		hTimer = SetTimer(
			(DWORD)hWnd,
			250,       //Current version's timer is not accurate since clock chip is not 
			           //reinitialized.
			NULL,
			NULL,
			TIMER_FLAGS_ALWAYS);
		if(NULL == hTimer)
		{
			break;
		}
		GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
		cx = (rect.right - rect.left) / 2;
		cy = (rect.bottom - rect.top) / 2;
		r = cx > cy ? cy : cx;
		r -= 20;
		hDC = GetClientDC(hWnd);
		break;
	case WM_TIMER:     //Only one timer can be set for one window in current version.
		EraseClockPointer(hDC,cx,cy,r,hour,minu,secd);
		secd ++;
		if(secd == 60)
		{
			minu ++;
			secd = 0;
		}
		if(minu == 60)
		{
			hour ++;
			minu = 0;
		}
		if(hour == 12)
		{
			hour = 0;
		}
		DrawClockPointer(hDC,cx,cy,r,hour,minu,secd);
		break;
	case WM_DRAW:
		DrawClockFace(hWnd);
		DrawClockPointer(hDC,cx,cy,r,hour,minu,secd);
		break;
	case WM_LBUTTONUP:
		//MessageBox(hWnd,"Left button is up.","Notification",MB_OK);
		/*
		x = (lParam >> 16) & 0x0000FFFF;
		y = lParam & 0x0000FFFF;
		htResult = HitTest(hWnd,x,y);
		if(HT_CLOSEBUTTON == htResult)
		{
			MessageBox(hWnd,"Close button is clicked.","Notification",MB_OK);
			break;
		}
		if(HT_CAPTION == htResult)
		{
			MessageBox(hWnd,"Window caption is clicked.","Notification",MB_OK);
			break;
		}
		MessageBox(hWnd,"Neither caption nor close button is clicked.","Notification",MB_OK);*/
		break;
	case WM_CLOSE:
		CloseWindow(hWnd);  //Exit application.
		if(NULL != hTimer)
		{
			CancelTimer(hTimer);
		}
		PostQuitMessage(0);  //Terminate the kernel thread.
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//Entry point of Hello World.
DWORD __HCNMain(LPVOID pData)
{
	//HANDLE hFrameWnd = (HANDLE)pData;  //pData is the handle of screen window,it is all application's parent window.
	MSG msg;
	HANDLE hHello = NULL;
	__WINDOW_MESSAGE wmsg;

	//Create hello world's window.
	hHello = CreateWindow(WS_WITHBORDER | WS_WITHCAPTION,
		"Analog clock - Version 1.0",
		150,
		150,
		600,
		400,
		HelloWorldProc,
		NULL,//hFrameWnd,
		NULL,
		0x00800000,
		NULL);
	if(NULL == hHello)
	{
		MessageBox(NULL,"Can not create the hello world window.","Error",MB_OK);
		goto __TERMINAL;
	}
	//MessageBox(NULL,"Draw application is running now.","Info",MB_OK);
	//CreateButton(NULL,(TCHAR*)0xCCCCCCCC,0xEEEEEEEE,128,128,256,256);

	//Message loop of this application.
	while(TRUE)
	{
		if(GetMessage(&msg))
		{
			switch(msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				wmsg.hWnd = (HANDLE)msg.dwParam;
				wmsg.message = WM_TIMER;
				SendWindowMessage(wmsg.hWnd,&wmsg);
				break;
			case KERNEL_MESSAGE_WINDOW:
				DispatchWindowMessage((__WINDOW_MESSAGE*)msg.dwParam);
				break;
			case KERNEL_MESSAGE_TERMINAL:  //Post by PostQuitMessage.
				goto __TERMINAL;
			default:
				break;
			}
		}
	}

__TERMINAL:
	return 0;
}

extern "C"
{
void HCNMain()
{
	__HCNMain(NULL);
}
}