//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 10, 2009
//    Module Name               : MSGBOX.CPP
//    Module Funciton           : 
//                                The implementation code of MessageBox.
//                                This control also be the most important and useful
//                                control in GUI module.
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

//Message loop routine for dialog.
static DWORD DialogLoop(HANDLE hDialog)
{
	MSG Msg;
	__WINDOW_MESSAGE* pwmsg;

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			switch(Msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				break;
			case KERNEL_MESSAGE_WINDOW:
				pwmsg = (__WINDOW_MESSAGE*)Msg.dwParam;
				switch(pwmsg->message)
				{
					//All mouse message of the current thread should be processed
					//by the model dialog.
					//We modify the own window's handle to achieve this.
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MOUSEMOVE:
				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
					{
						if(!IsChild(hDialog,pwmsg->hWnd))
						{
							pwmsg->hWnd = hDialog;
						}
					}
					break;
				default:
					break;
				}
				DispatchWindowMessage(pwmsg);
				break;
			case KERNEL_MESSAGE_DIALOG:
				return Msg.dwParam;  //Return the dialog result.
				break;
			default:
				break;
			}
		}
	}
}

//Local helper routine to draw a message box.
static VOID DrawMessageBox(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{
	HANDLE hDC = GetClientDC(hWnd);
	//Text out the message box text.
	TextOut(hDC,10,5,pMsgBox->MsgBoxText);
}

//Create only one button message box,MB_OK.
static VOID CreateOk(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{
	int x,y;  //Start position of button.
	x = pMsgBox->dwMsgBoxWidth - MSGBOX_BUTTON_WIDTH;  //Only one button.
	x /= 2;
	y = pMsgBox->dwMsgBoxHeight - 10 - GlobalParams.dwWndCaptionHeight;
	y -= MSGBOX_BUTTON_HEIGHT;
	if(y < 0)
	{
		y = 0;
	}
	CreateButton(hWnd,"确定(O)",ID_OK,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	return;
}

//MB_OKCANCEL.
static VOID CreateOkCancel(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{
	int x,y;
	x = pMsgBox->dwMsgBoxWidth - MSGBOX_BUTTON_WIDTH * 2;  //2 buttons.
	x -= MSGBOX_BUTTON_SPACE;  //Skip the space between these 2 buttons.
	x /= 2;
	y = pMsgBox->dwMsgBoxHeight - 10 - GlobalParams.dwWndCaptionHeight;
	y -= MSGBOX_BUTTON_HEIGHT;
	if(y < 0)
	{
		y = 0;
	}
	//Create first button.
	CreateButton(hWnd,"确定(O)",ID_OK,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	x += MSGBOX_BUTTON_WIDTH;
	x += MSGBOX_BUTTON_SPACE;
	//Create the second button.
	CreateButton(hWnd,"取消(C)",ID_CANCEL,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	return;
}

//MB_YESNO
static VOID CreateYesNo(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{	
	int x,y;
	x = pMsgBox->dwMsgBoxWidth - MSGBOX_BUTTON_WIDTH * 2;  //2 buttons.
	x -= MSGBOX_BUTTON_SPACE;  //Skip the space between these 2 buttons.
	x /= 2;
	y = pMsgBox->dwMsgBoxHeight - 10 - GlobalParams.dwWndCaptionHeight;
	y -= MSGBOX_BUTTON_HEIGHT;
	if(y < 0)
	{
		y = 0;
	}
	//Create first button.
	CreateButton(hWnd,"是(Y)",ID_YES,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	x += MSGBOX_BUTTON_WIDTH;
	x += MSGBOX_BUTTON_SPACE;
	//Create the second button.
	CreateButton(hWnd,"否(N)",ID_NO,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	return;
}

//MB_YESNOCANCEL
static VOID CreateYesNoCancel(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{
	int x,y;
	x = pMsgBox->dwMsgBoxWidth - MSGBOX_BUTTON_WIDTH * 3;  //3 buttons.
	x -= MSGBOX_BUTTON_SPACE * 2;  //Skip the space between these 3 buttons.
	x /= 2;
	y = pMsgBox->dwMsgBoxHeight - 10 - GlobalParams.dwWndCaptionHeight;
	y -= MSGBOX_BUTTON_HEIGHT;
	if(y < 0)
	{
		y = 0;
	}
	//Create first button.
	CreateButton(hWnd,"是(Y)",ID_YES,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	x += MSGBOX_BUTTON_WIDTH;
	x += MSGBOX_BUTTON_SPACE;
	//Create the second button.
	CreateButton(hWnd,"否(N)",ID_NO,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);
	x += MSGBOX_BUTTON_WIDTH;
	x += MSGBOX_BUTTON_SPACE;
	//Create the third button.
	CreateButton(hWnd,"取消(C)",ID_CANCEL,x,y,MSGBOX_BUTTON_WIDTH,MSGBOX_BUTTON_HEIGHT);

	return;
}

//Local helper routine to create a message box and show it.
static VOID CreateMessageBox(HANDLE hWnd,__MESSAGE_BOX* pMsgBox)
{
	switch(pMsgBox->dwMsgBoxType)
	{
	case MB_OK:
		CreateOk(hWnd,pMsgBox);
		break;
	case MB_OKCANCEL:
		CreateOkCancel(hWnd,pMsgBox);
		break;
	case MB_YESNO:
		CreateYesNo(hWnd,pMsgBox);
		break;
	case MB_YESNOCANCEL:
		CreateYesNoCancel(hWnd,pMsgBox);
		break;
	default:
		break;
	}
	DrawMessageBox(hWnd,pMsgBox);
	return;
}

//Message box's window procedure.
static DWORD MsgBoxWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
	__MESSAGE_BOX*  pMsgBox = (__MESSAGE_BOX*)GetWindowExtension(hWnd);
	__WINDOW_MESSAGE msg;
	MSG kmsg;

	switch(message)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		if(pMsgBox)
		{
			//Send a dialog message to thread's message queue.
			kmsg.wCommand = KERNEL_MESSAGE_DIALOG;
			kmsg.dwParam  = ID_CANCEL;  //If user close this window,assume it cancels this dialog.
			SendMessage(GetCurrentThread(),&kmsg);
			KMemFree(pMsgBox,KMEM_SIZE_TYPE_ANY,0);
			SetWindowExtension(hWnd,NULL);
		}
		CloseWindow(hWnd);  //Close this window.
		break;
	case WM_COMMAND:
		if(pMsgBox)
		{
			//Send a dialog message to thread's message queue.
			kmsg.wCommand = KERNEL_MESSAGE_DIALOG;
			kmsg.dwParam  = wParam;  //wParam indicates the ID of button.
			SendMessage(GetCurrentThread(),&kmsg);
			KMemFree(pMsgBox,KMEM_SIZE_TYPE_ANY,0);
			SetWindowExtension(hWnd,NULL);
		}
		CloseWindow(hWnd);  //Close this window.
		break;
	case WM_PAINT:
		break;
	case WM_DRAW:
		if(pMsgBox)
		{
			DrawMessageBox(hWnd,pMsgBox);
			msg.hWnd = hWnd;
			msg.message = WM_DRAW;
			msg.wParam  = 0;
			msg.lParam  = 0;
			SendWindowChildMessage(hWnd,&msg);  //Tell all child(ren) to redraw.
		}
		break;
	case WM_INITDIALOG:    //This is the only case to create all dialog controls.
		if(pMsgBox)
		{
			CreateMessageBox(hWnd,pMsgBox);
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}

//MessageBox routine,to show a message box and get user's input as return.
DWORD MessageBox(HANDLE hWnd,TCHAR* pszText,TCHAR* pszTitle,UINT uType)
{
	HANDLE  hMsgBoxWnd = NULL;
	__MESSAGE_BOX* pMsgBox = NULL;
	int length = 0;
	int width = 0;
	BOOL bResult = FALSE;
	DWORD dwRetVal = 0;
	__WINDOW_MESSAGE msg;
	int x,y;  //Start position of message box window.
	__RECT rect;  //Parent window's rectangle.

	if((length = strlen(pszText)) >= MSGBOX_TEXT_LENGTH)  //Text too long.
	{
		return 0;
	}

	pMsgBox = (__MESSAGE_BOX*)KMemAlloc(sizeof(__MESSAGE_BOX),KMEM_SIZE_TYPE_ANY);
	if(NULL == pMsgBox)
	{
		goto __TERMINAL;
	}
	strcpy(pMsgBox->MsgBoxText,pszText);
	pMsgBox->dwMsgBoxType = uType;
	//Calculate message box's width.
	if(uType & MB_OK)
	{
		width += MSGBOX_BUTTON_SPACE;
		width += MSGBOX_BUTTON_WIDTH;
	}
	if(uType & MB_CANCEL)
	{
		width += MSGBOX_BUTTON_SPACE;
		width += MSGBOX_BUTTON_WIDTH;
	}
	if(uType & MB_YES)
	{
		width += MSGBOX_BUTTON_SPACE;
		width += MSGBOX_BUTTON_WIDTH;
	}
	if(uType & MB_NO)
	{
		width += MSGBOX_BUTTON_SPACE;
		width += MSGBOX_BUTTON_WIDTH;
	}
	width += MSGBOX_BUTTON_SPACE;  //Add the left margin between button and window frame.
	length *= 8;  //Calculate the text's space.
	length += 20;  //Reserve 10 pixel between text and window frame.
	pMsgBox->dwMsgBoxWidth = (length > width) ? length : width;
	pMsgBox->dwMsgBoxHeight = 100;  //Use default height.

	//Calculate message window's position in screen,default is in it's parent's center.
	GetWindowRect(hWnd,&rect,GWR_INDICATOR_WINDOW);
	x = rect.right - rect.left - pMsgBox->dwMsgBoxWidth;
	y = rect.bottom - rect.top - pMsgBox->dwMsgBoxHeight;
	x = rect.left + x / 2;
	y = rect.top + y / 2;
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	//Create message box window.
	hMsgBoxWnd = CreateWindow(WS_WITHCAPTION | WS_WITHBORDER,
		pszTitle,
		x,
		y,
		pMsgBox->dwMsgBoxWidth,
		pMsgBox->dwMsgBoxHeight,
		MsgBoxWndProc,
		hWnd,
		NULL,
		GlobalParams.COLOR_BTNFACE,
		NULL);
	if(NULL == hMsgBoxWnd)
	{
		goto __TERMINAL;
	}
	SetWindowExtension(hMsgBoxWnd,pMsgBox);

	msg.hWnd = hMsgBoxWnd;
	msg.message = WM_INITDIALOG;
	msg.lParam  = 0;
	msg.wParam  = 0;
	SendWindowMessage(hMsgBoxWnd,&msg);  //Refresh the message box.
	//Enter the message loop now.
	dwRetVal = DialogLoop(hMsgBoxWnd);

	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(pMsgBox)
		{
			KMemFree(pMsgBox,KMEM_SIZE_TYPE_ANY,0);
		}
		if(hMsgBoxWnd)
		{
			DestroyWindow(hMsgBoxWnd);
		}
	}
	return dwRetVal;
}

