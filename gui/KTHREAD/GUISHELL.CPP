//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29,2009
//    Module Name               : GUIENTRY.CPP
//    Module Funciton           : 
//                                GUI shell main entry routine is implemented
//                                in this routine.
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

//Global variables used by GUI shell thread.
__GUI_GLOBAL GuiGlobal = {0};

//Initialize the GUI shell thread.
static BOOL InitGuiShell()
{
	int cxScreen = Video.dwScreenWidth;
	int cyScreen = Video.dwScreenHeight;

	GuiGlobal.hApplicationBand = CreateWindow(
		0,  //Without any caption and border.
		NULL,
		cxScreen - APPLICATIONBAND_WIDTH,
		INDICATEBAND_HEIGHT + 1,
		APPLICATIONBAND_WIDTH,
		cyScreen - INDICATEBAND_HEIGHT - TASKBAND_HEIGHT - 2,
		AppBandWndProc,
		NULL,
		NULL,
		COLOR_APPLICATIONBAND,
		NULL);
	if(NULL == GuiGlobal.hApplicationBand)
	{
		return FALSE;
	}

	GuiGlobal.hIndicateBand = CreateWindow(
		0,
		NULL,
		0,
		0,
		cxScreen,
		INDICATEBAND_HEIGHT,
		NULL,
		NULL,
		NULL,
		COLOR_INDICATEBAND,
		NULL);
	if(NULL == GuiGlobal.hIndicateBand)
	{
		return FALSE;
	}

	GuiGlobal.hTaskBand = CreateWindow(
		0,
		NULL,
		0,
		cyScreen - TASKBAND_HEIGHT,
		cxScreen,
		TASKBAND_HEIGHT,
		NULL,
		NULL,
		NULL,
		COLOR_TASKBAND,
		NULL);
	if(NULL == GuiGlobal.hIndicateBand)
	{
		return FALSE;
	}

	//Create GUI related windows.
	GuiGlobal.hMainFrame = CreateWindow(
		0,
		NULL,
		0,
		INDICATEBAND_HEIGHT + 1,
		cxScreen - APPLICATIONBAND_WIDTH - 3,  //Reserve space for separate line.
		cyScreen - INDICATEBAND_HEIGHT - TASKBAND_HEIGHT -2,
		MainFrameProc,
		NULL,
		NULL,
		COLOR_APPLAUNCHER,
		NULL);
	if(NULL == GuiGlobal.hMainFrame)
	{
		return FALSE;
	}

	//Store the main frame window's layout.
	GuiGlobal.xmf   = 0;
	GuiGlobal.ymf   = INDICATEBAND_HEIGHT + 1;
	GuiGlobal.cxmf  = cxScreen - APPLICATIONBAND_WIDTH - 3;
	GuiGlobal.cymf  = cyScreen - INDICATEBAND_HEIGHT - TASKBAND_HEIGHT - 2;

	//Draw a separate line between main frame and application band.
	Video.DrawLine(&Video,cxScreen - APPLICATIONBAND_WIDTH - 2,
		INDICATEBAND_HEIGHT + 1,
		cxScreen - APPLICATIONBAND_WIDTH - 2,
		cyScreen - TASKBAND_HEIGHT - 2,
		COLOR_BLACK);
	Video.DrawLine(&Video,cxScreen - APPLICATIONBAND_WIDTH - 1,
		INDICATEBAND_HEIGHT + 1,
		cxScreen - APPLICATIONBAND_WIDTH - 1,
		cyScreen - TASKBAND_HEIGHT - 2,
		COLOR_WHITE);

	return TRUE;
}

//Entry routine of GUI module.
DWORD GuiShellEntry(LPVOID)
{
	MSG Msg;
	WORD x = 0;     //Mouse x scale.
	WORD y = 0;     //Mouse y scale.
	__WINDOW_MESSAGE wmsg;
	
	if(!InitGuiShell())
	{
		return 0;
	}
	MouseManager.DrawMouse(&MouseManager,
		&Video,0,0);  //Show mouse arrow in screen.
	//Added by garry in 2009.04.22.
	//BmpShowArray(&Video, 200, 200, 14, 14, DataBlue, DataGreen, DataRed);
	
	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			switch(Msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:  //Only one timer can be set for one window.
				wmsg.hWnd = (HANDLE)Msg.dwParam;
				wmsg.message = WM_TIMER;
				wmsg.wParam  = 0;
				wmsg.lParam  = 0;
				SendWindowMessage(wmsg.hWnd,&wmsg);
				break;
			case KERNEL_MESSAGE_WINDOW:
				DispatchWindowMessage((__WINDOW_MESSAGE*)Msg.dwParam);
				break;
			case KERNEL_MESSAGE_TERMINAL:  //System terminal message.
				goto __TERMINAL;
			default:
				break;
			}
		}
	}
__TERMINAL:
	return 0;
}
