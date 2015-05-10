//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : WNDMGR.H
//    Module Funciton           : 
//                                The definition of windows manager object.
//                                This is the kernel object in GUI module of
//                                Hello China.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __WNDMGR_H__
#define __WNDMGR_H__
#endif

//Window procedure definition.
typedef DWORD (*__WINDOW_PROC)(HANDLE hWnd,UINT msg,WORD wParam,DWORD lParam);

#define WND_TITLE_LEN 128   //Window title's length,in WCHAR.

//Window object.
struct __WINDOW{
	TCHAR      WndTitle[WND_TITLE_LEN];
	DWORD      dwWndStyle;
	DWORD      dwWndStatus;
	int        x;              //Start position of window.
	int        y;
	int        cx;             //Width of this window.
	int        cy;
	int        xclient;        //Start position of client area.
	int        yclient;
	int        cxclient;       //Client area's width.
	int        cyclient;
	int        xcb;            //x coordinate of close button.
	int        ycb;            //y coordinate of close button.
	int        cxcb;           //width of close button.
	int        cycb;           //height of close button.
	HANDLE     hFocusChild;    //Child window in focus status.
	HANDLE     hCursor;        //Window cursor.
	HANDLE     hIcon;          //ICON of this window.
	HANDLE     hWindowDC;      //Device context of this window.
	HANDLE     hClientDC;      //Device context of this window's client area.
	__COLOR    clrBackground;  //Background color.
	LPVOID     lpWndExtension; //Window extension pointer.
	__WINDOW_PROC WndProc;     //Base address of window procedure.
	HANDLE     hOwnThread;     //The thread handle owns this window.
	__REGION*  pRegion;        //Clip zone of this window.

	__WINDOW*  pPrevSibling;   //Previous sibling of this window.
	__WINDOW*  pNextSibling;   //Next sibling.
	__WINDOW*  pParent;        //Parent;
	__WINDOW*  pChild;         //Child list header.
	DWORD      dwSignature;    //Signature of window object.
};

//Window signature,the window only contain this signature is to be
//treat as a TRUE window object.This rule is used to verify window 
//object's validity.
#define WINDOW_SIGNATURE 0xAA5555AA

//Window status.
#define WST_ONFOCUS        0x00000001
#define WST_UNFOCUS        0x00000002
#define WST_MINIMIZED      0x00000004
#define WST_MAXIMIZED      0x00000008
#define WST_NORMAL         0x00000010
#define WST_CLOSED         0x00000020   //Window is closed but it's kernel object exists.
#define WST_HIDDEN         0x00000040

//Window styles.
#define WS_WITHCAPTION    0x00000001    //With caption.
#define WS_3DFRAME        0x00000002    //3D frame effective.
#define WS_LINEFRAME      0x00000004    //Line frame.
#define WS_MAINFRAME      0x00000008    //Main frame window.
#define WS_HSCROLLBAR     0x00000010    //with HScroll bar.
#define WS_VSCROLLBAR     0x00000020    //With VScroll bar.
#define WS_WITHBORDER     0x00000040    //Window with border.
#define WS_HIDE           0x00000080    //Window will be hide after created.
#define WS_NORMAL         (WS_WITHCAPTION | WS_WITHBORDER | WS_LINEFRAME | WS_MAINFRAME)
#define WS_MODEL          0x00000100    //Model dialog.
#define WS_CANNOTFOCUS    0x00000200    //Can not focus on this window.
#define WS_UNFOCUSDRAW    0x00000400    //Can draw in un-focus mode.

//Windows messages can be sent to a window.
#define WM_CREATE         1
#define WM_DESTROY        2
#define WM_CLOSE          3
#define WM_LBUTTONDOWN    4
#define WM_RBUTTONDOWN    5
#define WM_LBUTTONDBLCLK  6
#define WM_RBUTTONDBLCLK  7
#define WM_SHOW           8
#define WM_HIDE           9
#define WM_SIZE           10
#define WM_VIRTUALKEY     11
#define WM_CHAR           12
#define WM_KEYDOWN        13
#define WM_KEYUP          14
#define WM_MOUSEMOVE      15
#define WM_LBUTTONUP      16
#define WM_RBUTTONUP      17
#define WM_TIMER          18
#define WM_COMMAND        19
#define WM_DRAGIN         20
#define WM_DRAGOUT        21
#define WM_MINIMAL        22
#define WM_MAXIMAL        23
#define WM_NORMAL         24
#define WM_NOTIFY         25
#define WM_DRAW           26
#define WM_UPDATE         27
#define WM_SCROLLUP       28
#define WM_SCROLLDOWN     29
#define WM_SCROLLLEFT     30
#define WM_SCROLLRIGHT    31
#define WM_SCROLLHOLD     32
#define WM_PAINT          33
#define WM_LOSTFOCUS      34
#define WM_ONFOCUS        35
#define WM_CHILDCLOSE     36
#define WM_CHILDSHOW      37
#define WM_INITDIALOG     38    //Initialize dialog box.

#define WM_USER           1024  //User defined messages start from this value.

//Default window procedure,this procedure should be called after
//your custom process for every message.
DWORD DefWindowProc(HANDLE hWnd,UINT msg,WORD wParam,DWORD lParam);

//Definition of windows manager object.
struct __WINDOW_MANAGER{
	__WINDOW*   pWndAncestor;      //The first window of the system,it is
	                               //all other windows' ancestor.
	HANDLE      hCurrThread;       //Current focus thread.
	__WINDOW*   pCurrWindow;       //Current focus window.
	BOOL        (*Initialize)(__WINDOW_MANAGER* pWndManager);
	VOID        (*Uninitialize)(__WINDOW_MANAGER* pWndManager);
};

HANDLE CreateWindow(DWORD dwWndStyle,TCHAR* pszWndTitle,int x,
					int y,int cx,int cy,__WINDOW_PROC WndProc,
					HANDLE hParent,HANDLE hMenu,__COLOR clrbackground,
					LPVOID lpReserved); //Create one window.

VOID DestroyWindow(HANDLE hWnd);  //Destroy one window.
VOID CloseWindow(HANDLE hWnd);    //Close one window.
VOID FocusWindow(HANDLE hWnd);    //Set window in focus status.
VOID UnfocusWindow(HANDLE hWnd);  //Force a window loss focus.
HANDLE GetWindowDC(HANDLE hWnd);  //Returns window's DC.
HANDLE GetClientDC(HANDLE hWnd);  //Returns window client area's DC.
VOID PaintWindow(HANDLE hWnd);    //Paint a window.
DWORD GetWindowStatus(HANDLE hWnd);  //Get a given window's status.
LPVOID GetWindowExtension(HANDLE hWnd);  //Return window's extension pointer.
LPVOID SetWindowExtension(HANDLE hWnd,LPVOID lpNewExt);  //Set the ext to new and return old.
HANDLE GetParentWindow(HANDLE hWnd); //Returns parent window.
BOOL GetWindowRect(HANDLE hWnd,__RECT* pRect,DWORD dwIndicator);  //Returns window's rect according indicator.
BOOL IsChild(HANDLE hParent,HANDLE hChild);  //If the hChild is child of hParent.

//Indicators of GetWindowRect.
#define GWR_INDICATOR_WINDOW     0x00000001
#define GWR_INDICATOR_CLIENT     0x00000002

//Hit test results.
#define HT_CLOSEBUTTON    0x00000001
#define HT_CAPTION        0x00000002
#define HT_CLIENT         0x00000003
#define HT_LEFTFRAME      0x00000004
#define HT_RIGHTFRAME     0x00000005
#define HT_TOPFRAME       0x00000006
#define HT_BOTTOMFRAME    0x00000007
#define HT_SYSMENU        0x00000008
#define HT_RESTBUTTON     0x00000009
#define HT_MINBUTTON      0x0000000A

//HitTest routine,check the click position of a window.
DWORD HitTest(HANDLE hWnd,int x,int y);

//Window message.
struct __WINDOW_MESSAGE{
	HANDLE hWnd;
	UINT   message;
	WORD   wParam;
	WORD   wReserved;
	DWORD  lParam;
};

BOOL SendWindowMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg);  //Send a message to window.
BOOL DispatchWindowMessage(__WINDOW_MESSAGE* pWndMsg);  //Dispatch a window message.
VOID SendWindowTreeMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg); //Send a message to window and all
                                                                   //it's children.
VOID SendWindowChildMessage(HANDLE hWnd,__WINDOW_MESSAGE* pWndMsg);
VOID PostQuitMessage(int nExitCode);  //Send a KERNEL_MESSAGE_TERMINAL to current thread.

//Global object declaration: WindowManager.
extern __WINDOW_MANAGER WindowManager;
