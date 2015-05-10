//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : GLOBAL.H
//    Module Funciton           : 
//                                Global parameter management object is defined 
//                                in this module,it's operating interface also
//                                included in this module.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#endif

struct __GUI_GLOBAL_PARAM{
	DWORD COLOR_CLOSEBUTTON;
	DWORD COLOR_SCROLLBAR;
	DWORD COLOR_BACKGROUND;
	DWORD COLOR_ACTIVECAPTION;
	DWORD COLOR_INACTIVECAPTION;
	DWORD COLOR_MENU;
	DWORD COLOR_WINDOW;
	DWORD COLOR_WINDOWFRAME;
	DWORD COLOR_MENUTEXT;
	DWORD COLOR_WINDOWTEXT;
	DWORD COLOR_CAPTIONTEXT;
	DWORD COLOR_ACTIVEBORDER;
	DWORD COLOR_INACTIVEBORDER;
	DWORD COLOR_APPWORKSPACE;
	DWORD COLOR_HIGHLIGHT;
	DWORD COLOR_HIGHLIGHTTEXT;
	DWORD COLOR_BTNFACE;
	DWORD COLOR_BTNSHADOW;
	DWORD COLOR_GRAYTEXT;
	DWORD COLOR_BTNTEXT;
	DWORD COLOR_INACTIVECAPTIONTEXT;
	DWORD COLOR_BTNHIGHLIGHT;
	DWORD COLOR_3DDKSHADOW;
	DWORD COLOR_3DLIGHT;
	DWORD COLOR_INFOTEXT;
	DWORD COLOR_INFOBK;
	DWORD COLOR_HOTLIGHT;
	DWORD dwWndBorderWidth;
	DWORD dwWndCaptionHeight;
	DWORD dwWndMenuHeight;
	DWORD dwWndStatusBarHeight;
	DWORD dwWndToolBarHeight;
	DWORD dwWndDefCx;  //Default width if not specified when create window.
	DWORD dwWndDefCy;  //Default height if not specified when create window.

	DWORD SCROLLBAR_WIDTH;    //Default scroll bar's width for horizental.
	DWORD BUTTON_HEIGHT;      //Default button height.
	DWORD EDIT_HEIGHT;        //Default edit control height.
	DWORD LABEL_HEIGHT;       //Default height for static label control.

	HANDLE hGUIShell;         //GUI shell thread's handle.
	HANDLE hRawInputThread;   //Raw input thread's handle.

	BOOL (*Initialize)(__GUI_GLOBAL_PARAM* pGlobalParam,__VIDEO* pVideo);
};

//Declaration of global object: GlobalParams.
extern __GUI_GLOBAL_PARAM GlobalParams;

