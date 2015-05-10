//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 13, 2009
//    Module Name               : CLENDAR.H
//    Module Funciton           : 
//                                The definitions and constants for clendar,a
//                                simple clendar application for Hello China.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __CLENDAR_H__
#define __CLENDAR_H__
#endif

//The entry point of Clendar application.
DWORD ClendarEntry(LPVOID);

//The default width and height of day rectangle.
#define DAY_RECT_WIDTH     60
#define DAY_RECT_HEIGHT    32

//The default clendar window's width and height.
#define CLENDAR_WND_WIDTH  (DAY_RECT_WIDTH * 7) //One week will be displayed in one row.
#define CLENDAR_WND_HEIGHT (DAY_RECT_HEIGHT * 9 + GlobalParams.dwWndCaptionHeight)

//The default width and height for year window and month window.
#define YEAR_WND_WIDTH     (CLENDAR_WND_WIDTH / 3)
#define YEAR_WND_HEIGHT    DAY_RECT_HEIGHT
#define MONTH_WND_WIDTH    (CLENDAR_WND_WIDTH / 3)
#define MONTH_WND_HEIGHT   DAY_RECT_HEIGHT
#define TIME_WND_WIDTH     (CLENDAR_WND_WIDTH / 3)
#define TIME_WND_HEIGHT    DAY_RECT_HEIGHT

//Colors for clendar.
#define COLOR_LIGHTORANGE     0xFF9900  //Light orange.
#define COLOR_VIOLET          0x800080  //Violet.
#define COLOR_CYAN            0x008080  //Cyan
//#ifdef COLOR_APPLICATIONBAND
//#undef COLOR_APPLICATIONBAND
//#endif
//#define COLOR_APPLICATIONBAND 0xC0C0C0

//Time structure,to manage time.
struct __TIME{
	int    hour;          //Hour of time.
	int    minute;        //Minute of time.
	int    second;        //Second of time.
};

//A struct to manager a day rectangle.
struct __DAY_RECT{
	int    day;          //Day part of date.
	int    x;            //x coordinate of this rectange.
	int    y;
	int    cx;
	int    cy;
	BOOL   bIsFestival;  //If this day is festival.
	BOOL   bIsToday;     //If this day is today.
	BOOL   bIfFocus;     //If this day is selected as focus day.
	//__MEMO_INFO* pMemoList;  //Memo list of today.
};

//Structure to manage one month.
struct __CLENDAR_MONTH{
	int    month;        //Month part of date.
	int    year;         //Year of this month.
	int    daynum;       //How many days in this month.
	int    firstweek;    //First day's week of this month,from 0(sunday) to 6(saturday).
	__DAY_RECT  DayArray[35];  //All days of this month.
};

//Messages used to initialize initial value for year/month/time window.
#define WM_SETYEAR       (WM_USER + 1)
#define WM_SETMONTH      (WM_USER + 2)
#define WM_SETTIME       (WM_USER + 3)

//Notify messages used by child window to tell parent window some value changed.
#define NM_YEARCHANGED   0x00000001
#define NM_MONTHCHANGED  0x00000002
#define NM_TIMECHANGED   0x00000003

//ID of buttons to operate year and month.
#define ID_NEXT          1  //Button of next year(month).
#define ID_PREV          2  //Button of previous year(month).

