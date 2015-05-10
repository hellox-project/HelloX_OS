#include <kapi.h>
 
//A new entry required by libc.
void main()
{
}
 
//Global variables.
extern __COLOR clrTable[];
 
//Constants used in this module.
#define LINE_GAP 5
#define WINDOW_GAP 100  //Gap between control and frame window's edge.
 
//A local helper routine to draw the color selector.
static void DrawColorSelector(HANDLE hDC,int x,int y,int width,int height)
{
     int     nRectWidth        = 0;
     int     nRectHeight       = 0;
     int     xStart,yStart;
     int     nGap              = LINE_GAP;
     int     i,j;
     BYTE    clrbyte           = 0;
     __COLOR FullClr           = 0x00FFFFFF;
     __COLOR bkgroundClr       = RGB(128,128,128);
     __COLOR rectClr           = 0;
     HANDLE  hOldPen           = NULL;
     HANDLE  hNewPen           = NULL;
     HANDLE  hOldBrush         = NULL;
     HANDLE  hNewBrush         = NULL;
     __RECT  rect;
 
     if(NULL == hDC)
     {
         return;
     }
     if((width < 150) || (height < 150))  //Area is too small that not hold 255 colors.
     {
         return;
     }
 
     //Draw the background rectangle.
     rect.left = x;
     rect.top  = y;
     rect.right = x + width;
     rect.bottom = y + height;
     hNewBrush = CreateBrush(FALSE,bkgroundClr);
     if(NULL == hNewBrush)
     {
         goto __TERMINAL;
     }
     hOldBrush = SelectBrush(hDC,hNewBrush);
     DrawRectangle(hDC,&rect);
 
     //Now draw the color selector.
     nRectWidth   = width - nGap;
     nRectWidth  /= 15;
     nRectWidth  -= nGap;
     nRectHeight  = height - nGap;
     nRectHeight /= 15;
     nRectHeight -= nGap;
 
     for(i = 0;i < 15;i ++)
     {
         for(j = 0;j < 15;j ++)
         {
              xStart = x + i * (nRectWidth   + nGap) + nGap;
              yStart = y + j * (nRectHeight  + nGap) + nGap;
              rectClr = clrTable[clrbyte ++];
              if(hNewBrush)
              {
                   DestroyBrush(hNewBrush);
              }
              hNewBrush = CreateBrush(FALSE,rectClr);
              if(NULL == hNewBrush)
              {
                   goto __TERMINAL;
              }
              SelectBrush(hDC,hNewBrush);
              rect.left = xStart;
              rect.top  = yStart;
              rect.right = xStart + nRectWidth;
              rect.bottom = yStart + nRectHeight;
              DrawRectangle(hDC,&rect);
         }
     }
     SelectBrush(hDC,hOldBrush);
 
__TERMINAL:
     if(hNewBrush)
     {
         DestroyBrush(hNewBrush);
     }
     return;
}
 
//Check if a mouse click event fall in the color selector,and return the clicked color
//index in case of fit.
static int ClickedColorIndex(int x,int y,int width,int height,int xpos,int ypos)
{
     int    index   = -1;
     int    nRectWidth = 0;
     int    nRectHeight = 0;
     int    relx        = 0;
     int    rely        = 0;
     int    i,j;
 
     //Check if the click fall outside of the color selector's rectangle.
     if((xpos <= x) || (ypos <= y))
     {
         return index;
     }
     if((xpos >= x + width) || (ypos >= y + height))
     {
         return index;
     }
 
     relx = xpos - x;
     rely = ypos - y;
     if((relx <= LINE_GAP) || (rely <= LINE_GAP))
     {
         return index;
     }
     relx -= LINE_GAP;
     rely -= LINE_GAP;
 
     nRectWidth    = width - LINE_GAP;
     nRectWidth   /= 15;
     nRectWidth   -= LINE_GAP;
     nRectHeight   = height - LINE_GAP;
     nRectHeight  /= 15;
     nRectHeight  -= LINE_GAP;
 
     i    = relx / (nRectWidth + LINE_GAP);
     relx = relx % (nRectWidth + LINE_GAP);
     j    = rely / (nRectHeight + LINE_GAP);
     rely = rely % (nRectHeight + LINE_GAP);
 
     if((relx < nRectWidth) && (rely < nRectHeight))
     {
         index = i * 15 + j;
     }
     return index;
}
 
//Window procedure of Scratch
static DWORD ScratchWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         static HANDLE hDC = GetClientDC(hWnd);
          __RECT rect;
          int x,y,wndx,wndy;
          int clrIndex = 0;
          HANDLE hNewBrush = NULL;
          HANDLE hOldBrush = NULL;
          __COLOR selectedClr;
          //char debugBuff[128];
 
         switch(message)
         {
         case WM_CREATE:    //Will receive this message when the window is created.
             break;
         case WM_TIMER:     //Only one timer can be set for one window in current version.
             break;
          case WM_LBUTTONDOWN:
               y = lParam & 0x0000FFFF;
               x = (lParam >> 16) & 0x0000FFFF;
               GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
               rect.bottom -= 100;  //Keep the space for selected color presenting rectangle.
               wndx = x - rect.left;
               wndy = y - rect.top;
               clrIndex = ClickedColorIndex(WINDOW_GAP,WINDOW_GAP,rect.right - rect.left - WINDOW_GAP * 2,rect.bottom - rect.top - WINDOW_GAP * 2,wndx,wndy);
               rect.bottom += 100; //Adjust to the actual window rectangle.
               if((-1 == clrIndex) || (clrIndex > 255))
               {
                    break;
               }
               selectedClr = clrTable[clrIndex];
               hNewBrush = CreateBrush(FALSE,selectedClr);
               if(NULL == hNewBrush)
               {
                    break;
               }
               hOldBrush = SelectBrush(hDC,hNewBrush);
               rect.right = rect.right - rect.left;
               rect.bottom = rect.bottom - rect.top;
               rect.left = WINDOW_GAP;
               rect.top  = rect.bottom - 100 - WINDOW_GAP / 2;
               rect.right = rect.right - WINDOW_GAP;
               rect.bottom -= WINDOW_GAP;
               DrawRectangle(hDC,&rect);
               SelectBrush(hDC,hOldBrush);
               DestroyBrush(hNewBrush);
               //sprintf(debugBuff,"wndx = %d,wndy = %d,clrIndex = %d",wndx,wndy,clrIndex);
               //TextOut(hDC,0,20,debugBuff);
               break;
         case WM_DRAW:
               GetWindowRect(hWnd,&rect,GWR_INDICATOR_CLIENT);
               rect.bottom -= 100;  //Reserve the band to show color.
               DrawColorSelector(hDC,WINDOW_GAP,WINDOW_GAP,rect.right - rect.left - WINDOW_GAP * 2,rect.bottom - rect.top - WINDOW_GAP * 2);
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
         //HANDLE hFrameWnd = (HANDLE)pData;  //pData is the handle of screen window,it is all application's parent window.
         MSG msg;
         HANDLE hMainFrame = NULL;
         __WINDOW_MESSAGE wmsg;
 
         //Create hello world's window.
         hMainFrame = CreateWindow(WS_WITHBORDER | WS_WITHCAPTION,
                   "Color selecting demostration for Hello China V1.75",
                   150,
                   150,
                   600,
                   400,
                   ScratchWndProc,
                   NULL,//hFrameWnd,
                   NULL,
                   0x00FFFFFF,
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
