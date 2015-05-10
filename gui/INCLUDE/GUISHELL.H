//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29,2009
//    Module Name               : GUISHELL.H
//    Module Funciton           : 
//                                Main header file of Hello China GUI module,
//                                some constants and structures are defined
//                                in this module,include the GUI shell thread
//                                entry.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GUISHELL_H__
#define __GUISHELL_H__
#endif

//Entry point of GUI shell thread.
DWORD GuiShellEntry(LPVOID);

//GUI shell's global control block,used to store global information of GUI shell thread.
struct __GUI_GLOBAL{
	HANDLE hIndicateBand;     //Indicate band window,in screen's uper position.
	HANDLE hTaskBand;         //Task band window,in bottom of screen.
	HANDLE hApplicationBand;  //Application band,in right of screen.
	HANDLE hMainFrame;        //Main frame of GUI shell thread,occupies the rest screen zone.
	//Layout of the main frame window,which is all application window's parent.
	int xmf;
	int ymf;
	int cxmf;
	int cymf;
};

//Global data used to contain GUI thread related data and used by all GUI module.
extern __GUI_GLOBAL GuiGlobal;

//Colors used by GUI shell.
#define COLOR_INDICATEBAND    0x0080C0
#define COLOR_TASKBAND        0x00C0FF
#define COLOR_MAINFRAME       0xFFFFFF
#define COLOR_APPLICATIONBAND 0x800000
#define COLOR_APPLAUNCHER     0xC0C0C0

#define INDICATEBAND_HEIGHT    (GlobalParams.dwWndCaptionHeight)      //Height of indicate band.
#define TASKBAND_HEIGHT        (GlobalParams.dwWndCaptionHeight * 2)  //Height of task band.
//#define APPLICATIONBAND_WIDTH  (GlobalParams.dwWndCaptionHeight * 8)  //Application band's width.
#define APPLICATIONBAND_WIDTH  (Video.dwScreenWidth / 3)

//Window procedure for Main Frame window.
DWORD MainFrameProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam);

//Window procedure for Application band window.
DWORD AppBandWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam);

//Window procedure for Task band window.
DWORD TaskBandProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam);

//Window procedure for Indicate band window.
DWORD IndicateBandProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam);

//ID for application launch buttons.
#define ID_TELEPHONEBOOK    1
#define ID_CALCULATOR       2
#define ID_EDIT             3
#define ID_CALENDAR         4
#define ID_FILE             5
#define ID_CAMERA           6
#define ID_SHORTMESSAGE     7
#define ID_GAME             8
#define ID_EXPLORER         9
#define ID_JAVE             10
#define ID_OTHER            11

