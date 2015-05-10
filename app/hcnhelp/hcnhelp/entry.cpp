#include <kapi.h>

//Window procedure of the main window.
static DWORD HCNHelpWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         static HANDLE hDC = GetClientDC(hWnd);
          __RECT rect;
		  int   ypos = 5;
 
         switch(message)
         {
         case WM_CREATE:    //Will receive this message when the window is created.
             break;
         case WM_TIMER:     //Only one timer can be set for one window in current version.
             break;
         case WM_DRAW:
               //GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
			   TextOut(hDC,10,ypos,"Hello China是一个智能嵌入式操作系统，应用在各种智能控制设备中。");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"当前版本是V1.75，下列是一些相关的操作帮助：");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"------------------------------------------");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"当前模式是图形模式，按下CTRL + ALT + DEL组合键，可返回字符界面；");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"在字符模式下，输入GUI并按回车，可进入图形模式；");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"在字符模式下，输入help并按回车，可输出字符模式的帮助信息；");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"任何技术问题，请加入QQ群：38467832 进行讨论；");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"官方blog链接:http://blog.csdn.net/hellochina15");
			   ypos += 20;
			   TextOut(hDC,10,ypos,"技术支持邮件地址:garryxin@yahoo.com.cn");
			   ypos += 20;
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
                   "Help information for Hello China V1.75",
                   150,
                   150,
                   600,
                   400,
                   HCNHelpWndProc,  //Window procedure.
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
