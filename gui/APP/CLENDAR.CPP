//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 13, 2009
//    Module Name               : CLENDAR.CPP
//    Module Funciton           : 
//                                The implementation code for clendar,a
//                                simple clendar application for Hello China.
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

#include "CLENDAR.H"

//Global variables.
static __CLENDAR_MONTH Month;

//Local helper routine to initialize the month.
static VOID InitMonth(__CLENDAR_MONTH* pMonth)
{
	int i,j;
	int today = 9;
	int day = 1;
	int firstweek = 3;

	if(NULL == pMonth)
	{
		return;
	}
	//Here should acquire current date and set it to month.
	pMonth->year      = 2009;
	pMonth->month     = 5;
	pMonth->daynum    = 31;
	pMonth->firstweek = firstweek;    //Start from thursday.

	for(i = 0;i < 5;i ++)
	{
		for(j = 0;j < 7;j ++)
		{
			if(i * 7 + j < pMonth->firstweek)
			{
				pMonth->DayArray[i * 7 + j].day = 35;  //Mark as invalid.
				continue;
			}
			pMonth->DayArray[i * 7 + j].day = day;
			pMonth->DayArray[i * 7 + j].bIsFestival = FALSE;
			pMonth->DayArray[i * 7 + j].bIsToday    = FALSE;
			pMonth->DayArray[i * 7 + j].bIfFocus    = FALSE;
			if(day == today)
			{
				pMonth->DayArray[i * 7 + j].bIsToday = TRUE;
			}
			day ++;
		}
	}
}

//Change the year,should re-calculate current month.
static VOID ChangeYear(__CLENDAR_MONTH* pMonth,int year)
{
}

//Change the month,should re-calculate current month.
static VOID ChangeMonth(__CLENDAR_MONTH* pMonth,int month)
{
}

//Local helper routine to draw all clendar days.
static VOID DrawDays(HANDLE hWnd,__CLENDAR_MONTH* pMonth)
{
	__DAY_RECT*     pDay = NULL;
	HANDLE          hBrush = NULL,hOldBrush;
	HANDLE          hPen = NULL,hOldPen;
	HANDLE          hTodayBrush;
	HANDLE          hDC = GetClientDC(hWnd);
	TCHAR           daystr[10];
	int             txtx,txty;
	int             i,j;
	__RECT          rect;

	if(NULL == pMonth)
	{
		return;
	}

	hBrush = CreateBrush(FALSE,COLOR_LIGHTORANGE);
	if(NULL == hBrush)
	{
		goto __TERMINAL;
	}
	hTodayBrush = CreateBrush(FALSE,COLOR_VIOLET);
	if(NULL == hTodayBrush)
	{
		goto __TERMINAL;
	}
	hPen = CreatePen(0,1,COLOR_WHITE);
	if(NULL == hPen)
	{
		goto __TERMINAL;
	}
	hOldBrush = SelectBrush(hDC,hBrush);
	hOldPen   = SelectPen(hDC,hPen);
	//Draw the days now.
	rect.top  = DAY_RECT_HEIGHT * 2;    //Reserve space for week title and year/month/time window.
	rect.left = 0;
	rect.right = DAY_RECT_WIDTH;
	rect.bottom = rect.top + DAY_RECT_HEIGHT;
	txtx = 8;
	txty = 9;
	for(i = 0;i < 5;i ++)
	{
		for(j = 0;j < 7;j ++)
		{
			pDay = &pMonth->DayArray[i * 7 + j];
			if(pDay->day >= 32) //Invalid value.
			{
				rect.left += DAY_RECT_WIDTH;
				rect.right += DAY_RECT_WIDTH;
				continue;
			}
			if(pDay->bIsToday)  //Draw today.
			{
				SelectBrush(hDC,hTodayBrush);
				DrawRectangle(hDC,rect);
				txtx = rect.left + 8;
				txty = rect.top + 9;
				_hx_sprintf(daystr,"%d",pDay->day);
				TextOut(hDC,txtx,txty,daystr);
				rect.left += DAY_RECT_WIDTH;
				rect.right += DAY_RECT_WIDTH;
				SelectBrush(hDC,hBrush);
			}
			else  //Draw normal days.
			{
				DrawRectangle(hDC,rect);
				txtx = rect.left + 8;
				txty = rect.top + 9;
				_hx_sprintf(daystr,"%d",pDay->day);
				TextOut(hDC,txtx,txty,daystr);
				rect.left += DAY_RECT_WIDTH;
				rect.right += DAY_RECT_WIDTH;
			}
		}
		rect.left = 0;
		rect.right = DAY_RECT_WIDTH;
		rect.top += DAY_RECT_HEIGHT;
		rect.bottom += DAY_RECT_HEIGHT;
	}
	SelectBrush(hDC,hOldBrush);
	SelectPen(hDC,hOldPen);
__TERMINAL:
	if(hBrush)
	{
		DestroyBrush(hBrush);
	}
	if(hPen)
	{
		DestroyPen(hPen);
	}
	if(hTodayBrush)
	{
		DestroyBrush(hTodayBrush);
	}
	return;
}

//A local helper routine,to draw weeks title.
static VOID DrawWeekTitle(HANDLE hWnd)
{
	static TCHAR* Weeks[7] = {
		"SUN",
		"MON",
		"TUES",
		"WED",
		"THUR",
		"FRI",
		"SAT"};
	HANDLE hDC = GetClientDC(hWnd);
	__RECT rect;
	HANDLE hBrush,hOldBrush;
	HANDLE hPen,hOldPen;
	int txtx,txty;  //Text start position.

	hBrush = CreateBrush(FALSE,COLOR_CYAN);
	if(NULL == hBrush)
	{
		return;
	}
	hPen = CreatePen(0,1,COLOR_WHITE);
	if(NULL == hPen)
	{
		DestroyBrush(hBrush);
		return;
	}
	hOldBrush = SelectBrush(hDC,hBrush);
	hOldPen   = SelectPen(hDC,hPen);

	for(int i = 0;i < 7;i ++)
	{
		rect.left = i * DAY_RECT_WIDTH;
		rect.right = rect.left + DAY_RECT_WIDTH;
		rect.top = DAY_RECT_HEIGHT;  //Reserve the space for year/month/time window.
		rect.bottom = rect.top + DAY_RECT_HEIGHT;
		DrawRectangle(hDC,rect);
		txtx = rect.left + 8;
		txty = rect.top + 9;
		TextOut(hDC,txtx,txty,Weeks[i]);
	}
	SelectBrush(hDC,hOldBrush);
	SelectPen(hDC,hOldPen);
	DestroyPen(hPen);
	DestroyBrush(hBrush);
	return;
}

//A local helper routine to draw buttons for prev and next year/month.
static VOID DrawButton(HANDLE hWnd,DWORD dwPrevId,DWORD dwNextId)
{
	__RECT rect;
	int x,y;

	GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
	x = 3;
	y = 3;
	CreateButton(hWnd,"<-",dwPrevId,x,y,28,20);
	x = rect.right - rect.left - (5 + 28);
	CreateButton(hWnd,"->",dwNextId,x,y,28,20);
}

//Local helper routine to draw the content of year window.
static VOID DrawYear(HANDLE hWnd,int year)
{
	__RECT rect;
	int x,y,length;
	HANDLE hDC = GetClientDC(hWnd);
	TCHAR yearstr[16];

	GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
	_hx_sprintf(yearstr,"%04d年",year);
	length = GetTextMetric(hDC,yearstr,TM_WIDTH);
	x = (rect.right - rect.left - length) / 2;
	y = 7;
	TextOut(hDC,x,y,yearstr);
}

//Window procedure for year/month window.
static DWORD YearWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	static int year   = 2012;

	switch(message)
	{
	case WM_CREATE:
		DrawButton(hWnd,ID_PREV,ID_NEXT);  //Create next and previous button.
		break;
	case WM_DRAW:
		DrawYear(hWnd,year);
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_PREV:
			year --;
			if(year <= 0)
			{
				year = 0;
			}
			break;
		case ID_NEXT:
			year ++;
			break;
		default:
			break;
		}
		DrawYear(hWnd,year);
		break;
	case WM_SETYEAR:
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//Local helper routine to draw the content of year window.
static VOID DrawMonth(HANDLE hWnd,int month)
{
	__RECT rect;
	int x,y,length;
	HANDLE hDC = GetClientDC(hWnd);
	TCHAR yearstr[16];

	GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
	_hx_sprintf(yearstr,"%02d月",month);
	length = GetTextMetric(hDC,yearstr,TM_WIDTH);
	x = (rect.right - rect.left - length) / 2;
	y = 7;
	TextOut(hDC,x,y,yearstr);
}

//Window procedure for year/month window.
static DWORD MonthWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	static int month   = 5;

	switch(message)
	{
	case WM_CREATE:
		DrawButton(hWnd,ID_PREV,ID_NEXT);
		break;
	case WM_DRAW:
		DrawMonth(hWnd,month);
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case ID_PREV:
			if(month == 1)
			{
				month = 12;
			}
			else
			{
				month --;
			}
			break;
		case ID_NEXT:
			if(month == 12)
			{
				month = 1;
			}
			else
			{
				month ++;
			}
			break;
		default:
			break;
		}
		DrawMonth(hWnd,month);
		break;
	case WM_SETMONTH:
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//Window procedure for TimeWindow.
static DWORD TimeWindowProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	static __TIME currtime;
	TCHAR timestr[16];
	__RECT rect;
	int x,y,length;
	HANDLE hDC = NULL;

	switch(message)
	{
	case WM_CREATE:
		currtime.hour    = 12;
		currtime.minute  = 30;
		currtime.second  = 0;
		break;
	case WM_DRAW:
		_hx_sprintf(timestr,"%02d:%02d:%02d",currtime.hour,currtime.minute,currtime.second);
		hDC = GetClientDC(hWnd);
		length = GetTextMetric(hDC,timestr,TM_WIDTH);
		GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);  //Get client rect.
		//Calculate the x and y of string.
		x = (rect.right - rect.left - length) / 2;
		if(x < 0)
		{
			x = 0;
		}
		y = 5;
		TextOut(hDC,x,y,timestr);
		break;
	case WM_SETTIME:  //Set current time.
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//Window procedure of Clendar window.
static DWORD ClendarWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	int x,y;
	__RECT rect;

	switch(message)
	{
	case WM_CREATE:
		//Create time window.
		GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
		x = rect.left;
		y = rect.top;
		CreateWindow(WS_WITHBORDER,NULL,x,y,YEAR_WND_WIDTH,YEAR_WND_HEIGHT,
			YearWndProc,hWnd,NULL,COLOR_LIGHTORANGE,NULL);
		x += YEAR_WND_WIDTH;
		CreateWindow(WS_WITHBORDER,NULL,x,y,MONTH_WND_WIDTH,MONTH_WND_HEIGHT,
			MonthWndProc,hWnd,NULL,COLOR_LIGHTORANGE,NULL);
		x += MONTH_WND_WIDTH;
		CreateWindow(WS_WITHBORDER,NULL,x,y,TIME_WND_WIDTH,TIME_WND_HEIGHT,
			TimeWindowProc,hWnd,NULL,COLOR_LIGHTORANGE,NULL);
		//Initialize month.
		InitMonth(&Month);
		break;
	case WM_DRAW:
		DrawWeekTitle(hWnd);
		DrawDays(hWnd,&Month);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);  //Terminate current thread.
		CloseWindow(hWnd);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//A helper routine to intialize the clendar thread.
static BOOL InitClendar(HANDLE hWnd)
{
	int x,y;
	__RECT rect;

	GetWindowRect(hWnd,&rect,GWR_INDICATOR_WINDOW);
	x = (rect.right - rect.left - CLENDAR_WND_WIDTH) / 2;
	y = (rect.bottom - rect.top - CLENDAR_WND_HEIGHT) / 2;

	CreateWindow(WS_WITHCAPTION | WS_WITHBORDER,
		"日历",
		x,
		y,
		CLENDAR_WND_WIDTH + GlobalParams.dwWndBorderWidth * 2,  //Consider the window frame's width.
		CLENDAR_WND_HEIGHT,
		ClendarWndProc,
		hWnd,
		NULL,
		GlobalParams.COLOR_WINDOW,
		//COLOR_LIGHTORANGE,
		NULL);
	return TRUE;
}

//The entry point of clendar application.
DWORD ClendarEntry(LPVOID pFrameWnd)
{
	HANDLE hMainFrame = (HANDLE)pFrameWnd;
	MSG    Msg;

	if(!InitClendar(hMainFrame))
	{
		return 0;
	}
	
	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			switch(Msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				break;
			case KERNEL_MESSAGE_WINDOW:
				DispatchWindowMessage((__WINDOW_MESSAGE*)Msg.dwParam);
				break;
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;
			default:
				break;
			}
		}
	}

__TERMINAL:
	return 0;
}
