#include <kapi.h>

//Window procedure of the main window.
static DWORD HelloWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         static HANDLE hDC = GetClientDC(hWnd);
          __RECT rect;
 
         switch(message)
         {
         case WM_CREATE:    //Will receive this message when the window is created.
             break;
         case WM_TIMER:     //Only one timer can be set for one window in current version.
             break;
         case WM_DRAW:
               //GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
			   TextOut(hDC,0,0,"Hello,world!");
             break;
         case WM_CLOSE:
             PostQuitMessage(0);  //Exit the application.
             break;
         default:
             break;
         }
         return DefWindowProc(hWnd,message,wParam,lParam);
}
 
//Entry point of Hello World.
extern "C"
{
DWORD HCNMain(LPVOID pData)
{
         MSG msg;
         HANDLE hMainFrame = NULL;
         __WINDOW_MESSAGE wmsg;
 
         //Create hello world's window.
         hMainFrame = CreateWindow(WS_WITHBORDER | WS_WITHCAPTION,
                   "Main window of the demonstration application",
                   150,
                   150,
                   600,
                   400,
                   HelloWndProc,  //Window procedure.
                   NULL,
                   NULL,
                   0x00FFFFFF,    //Window's background color.
                   NULL);
         if(NULL == hMainFrame)
         {
                   MessageBox(NULL,"Can not create the main frame window.","Error",MB_OK);
                   goto __TERMINAL;
         }
 
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
         if(hMainFrame)
         {
                   DestroyWindow(hMainFrame);
         }
         return 0;
}
}
