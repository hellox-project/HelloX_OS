//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 10, 2009
//    Module Name               : BUTTON.CPP
//    Module Funciton           : 
//                                The implementation code related to button,
//                                one of the most useful common control in GUI.
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

//Local helper routine,to draw a normal status button in a given DC.
//The contract status of normal status is pressed status.
static VOID DrawButtonNormal(HANDLE hDC,__BUTTON* pButton)
{
	HANDLE hOldPen   = NULL;
	HANDLE hWhitePen = NULL;
	HANDLE hBlackPen = NULL;

	hWhitePen = CreatePen(0,1,COLOR_WHITE);
	if(NULL == hWhitePen)
	{
		goto __TERMINAL;
	}
	hBlackPen = CreatePen(0,1,COLOR_BLACK);
	if(NULL == hBlackPen)
	{
		goto __TERMINAL;
	}
	//Draw the black lines of button's border.
	hOldPen = SelectPen(hDC,hBlackPen);
	DrawLine(hDC,0,pButton->cy,pButton->cx,pButton->cy);
	DrawLine(hDC,pButton->cx,1,pButton->cx,pButton->cy);
	//Draw the white lines of button border.
	SelectPen(hDC,hWhitePen);
	DrawLine(hDC,0,0,pButton->cx,0);
	DrawLine(hDC,0,0,0,pButton->cy);
	SelectPen(hDC,hOldPen);  //Restore the old pen object.
	//Draw button text.
	TextOut(hDC,pButton->xtxt,pButton->ytxt,pButton->ButtonText);
__TERMINAL:
	if(hWhitePen)
	{
		DestroyPen(hWhitePen);
	}
	if(hBlackPen)
	{
		DestroyPen(hBlackPen);
	}
	return;
}

//Local helper routine,to draw a normal status button in a given DC.
//The contract status of normal status is pressed status.
static VOID DrawButtonPressed(HANDLE hDC,__BUTTON* pButton)
{
	HANDLE hOldPen   = NULL;
	HANDLE hWhitePen = NULL;
	HANDLE hBlackPen = NULL;

	hWhitePen = CreatePen(0,1,COLOR_WHITE);
	if(NULL == hWhitePen)
	{
		goto __TERMINAL;
	}
	hBlackPen = CreatePen(0,1,COLOR_BLACK);
	if(NULL == hBlackPen)
	{
		goto __TERMINAL;
	}
	//Draw the white lines of button's border.
	hOldPen = SelectPen(hDC,hWhitePen);
	DrawLine(hDC,0,pButton->cy,pButton->cx,pButton->cy);
	DrawLine(hDC,pButton->cx,1,pButton->cx,pButton->cy);
	//Draw the black lines of button border.
	SelectPen(hDC,hBlackPen);
	DrawLine(hDC,0,0,pButton->cx,0);
	DrawLine(hDC,0,0,0,pButton->cy);
	SelectPen(hDC,hOldPen);  //Restore the old pen object.
	//Draw button text.
	TextOut(hDC,pButton->xtxt,pButton->ytxt,pButton->ButtonText);
__TERMINAL:
	if(hWhitePen)
	{
		DestroyPen(hWhitePen);
	}
	if(hBlackPen)
	{
		DestroyPen(hBlackPen);
	}
	return;
}

//Window procedure of button control.
static DWORD ButtonWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	HANDLE hDC = GetClientDC(hWnd);
	__BUTTON*  pButton = NULL;
	__WINDOW_MESSAGE msg;

	if(NULL == hWnd)
	{
		return 0;
	}
	pButton = (__BUTTON*)GetWindowExtension(hWnd);
	if(NULL == pButton)
	{
		return DefWindowProc(hWnd,message,wParam,lParam);
	}
	
	switch(message)
	{
	case WM_CREATE:
		break;
	case WM_DRAW:
		if(BUTTON_STATUS_NORMAL == pButton->dwBtnStatus)
		{
			DrawButtonNormal(hDC,pButton);  //Draw the button.
		}
		if(BUTTON_STATUS_PRESSED == pButton->dwBtnStatus)
		{
			DrawButtonPressed(hDC,pButton);
		}
		break;
	case WM_LBUTTONDOWN:
		if(pButton->dwBtnStatus == BUTTON_STATUS_PRESSED) //Already in pressed status.
		{
			break;
		}
		pButton->dwBtnStatus = BUTTON_STATUS_PRESSED;
		DrawButtonPressed(hDC,pButton);
		break;
	case WM_MOUSEMOVE:
		if(pButton->dwBtnStatus == BUTTON_STATUS_PRESSED) //Button is hold.
		{
			pButton->dwBtnStatus = BUTTON_STATUS_NORMAL;
			DrawButtonNormal(hDC,pButton);
		}
		break;
	case WM_LBUTTONUP:
		if(pButton->dwBtnStatus == BUTTON_STATUS_NORMAL) //Already in normal status.
		{
			break;
		}
		pButton->dwBtnStatus = BUTTON_STATUS_NORMAL;
		DrawButtonNormal(hDC,pButton);
		//Send button pressed message to it's parent.
		msg.hWnd = GetParentWindow(hWnd);
		msg.message = WM_COMMAND;
		msg.wParam  = pButton->dwButtonId;
		msg.lParam  = 0;
		SendWindowMessage(msg.hWnd,&msg);
		break;
	case WM_CLOSE:
		//Release the common control specific resource,the system level resource,
		//such as window resource,will be released by DefWindowProc routine.
		KMemFree(pButton,KMEM_SIZE_TYPE_ANY,0);
		SetWindowExtension(hWnd,NULL);
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//The implementation code of CreateButton.
HANDLE CreateButton(HANDLE hParent,TCHAR* pszText,DWORD dwButtonId,int x,int y,int cx,int cy)
{
	HANDLE    hButton  = NULL;
	__WINDOW* pBtnWnd  = NULL;
	__BUTTON* pButton  = NULL;
	BOOL      bResult  = FALSE;
	__WINDOW* pParent  = (__WINDOW*)hParent;
	__WINDOW_MESSAGE msg;
	int       height   = cy;  //Text width of button.
	int       width    = cx;  //Text height of button.
	int       ntxtlen  = 0;
	HANDLE    hDC      = NULL;

	if(NULL == hParent)  //Invalid.
	{
		return NULL;
	}
	hDC = GetWindowDC(hParent);
	pButton = (__BUTTON*)KMemAlloc(sizeof(__BUTTON),KMEM_SIZE_TYPE_ANY);
	if(NULL == pButton)
	{
		goto __TERMINAL;
	}
	//Initialize button.
	ntxtlen = strlen(pszText);
	if(ntxtlen >= BUTTON_TEXT_LEN)  //Text too long.
	{
		goto __TERMINAL;
	}
	strcpy(pButton->ButtonText,pszText);
	pButton->x = x + pParent->xclient;
	pButton->y = y + pParent->yclient;
	height = GetTextMetric(hDC,pszText,TM_HEIGHT);
	pButton->cy = (cy > (height + 2)) ? cy : height;  //Use the maximal one.
	width = GetTextMetric(hDC,pszText,TM_WIDTH);
	//width += 4;  //Keep 2 pixel space in left and right margin.
	pButton->cx = (cx > (width + 4)) ? cx : width; //Keep 2 pixel space in left and right margin.
	pButton->dwButtonId = dwButtonId;
	pButton->xtxt = (pButton->cx - width) / 2;
	pButton->ytxt = (pButton->cy - height) / 2;
	pButton->txtwidth = width;
	pButton->dwBtnStatus = BUTTON_STATUS_NORMAL;

	//Create the button window.
	hButton = CreateWindow(0,  //Without any caption and border.
		NULL,  //Without title.
		pButton->x,
		pButton->y,
		pButton->cx,
		pButton->cy,
		ButtonWndProc,
		hParent,
		NULL,
		GlobalParams.COLOR_BTNFACE,
		NULL);
	if(NULL == hButton)
	{
		goto __TERMINAL;
	}
	pBtnWnd = (__WINDOW*)hButton;
	pBtnWnd->lpWndExtension = (LPVOID)pButton;  //Save button information to window's ext.

	//Send WM_PAINT message to button to re-draw itself.
	msg.hWnd = hButton;
	msg.message = WM_PAINT;
	msg.wParam  = 0;
	msg.lParam  = 0;
	SendWindowMessage(hButton,&msg);
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(pButton)
		{
			KMemFree(pButton,KMEM_SIZE_TYPE_ANY,0);
		}
		if(hButton)
		{
			CloseWindow(hButton);
		}
		hButton = NULL;
	}
	return hButton;
}

//A simple version of CreateButton,no need to specify button size members.
HANDLE CreateButtonSimple(HANDLE hParent,TCHAR* pszText,DWORD dwButtonId,int x,int y)
{
	return CreateButton(hParent,pszText,dwButtonId,x,y,0,0);
}

