#include <kapi.h>

//A static array to store the CPI change percentage of every month.In order to void
//floating point operation,we use multiply 10 for each month.
static int CPIChange[12] = {15,27,24,28,31,29,33,35,36,44,51,45};
static char* months[]  = {"JAN","FEB","MAR","APRI","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
static char* figTitle  = "2010年各月CPI环比增幅统计";

//A new entry required by libc.
void main()
{
}

//A local helper routine to draw the statistics figure's frame.
static void DrawFrame(HANDLE hDC,HANDLE hWnd)
{
         int     lineNum   = 0;
         int     colNum    = 25;
         int     colWidth  = 0;
         int     yCoord    = 0;;    //Y coordinate od coordinate label.
         int     xCoord    = 0;     //X coordinate of coordinate label.
         int     lineGap   = 0;     //Height of each row.
         int     figWidth  = 0;     //The picture's width.
         int     txtHeight = 0;     //Title's height.
         int     txtWidth  = 0;     //Title's width.
         int     leftGap   = 60;    //Gap between picture and window left frame.
         int     topGap    = 80;    //Gap between picture and window top frame.
         HANDLE  hOldPen   = NULL;
         HANDLE  hOldBrush = NULL;
         HANDLE  hNewPen   = NULL;
         HANDLE  hNewBrush = NULL;
         __RECT  wndRect;
         __RECT  pctRect;
         char    buff[16];
         int     i;

         //Get the window client's dimension.
         GetWindowRect(hWnd,&wndRect,GWR_INDICATOR_CLIENT);
         //Calcluate the figure title's height.
         txtHeight = GetTextMetric(hDC,figTitle,TM_HEIGHT);
         txtWidth  = GetTextMetric(hDC,figTitle,TM_WIDTH);
         //Calculate how many rows should be drawn.
         for(i = 0;i < 12;i ++)
         {
                   if(lineNum < CPIChange[i] / 10)
                   {
                            lineNum = CPIChange[i] / 10;
                   }
         }
         lineNum += 2;

         //Calculate the picture's width.
         figWidth = wndRect.right - wndRect.left - leftGap - 20;

         //Calculate each colume's width.
         colWidth = figWidth / colNum;

         //Calculate the gap between each line.
         lineGap = wndRect.bottom - wndRect.top;
         lineGap -= txtHeight;
         lineGap -= topGap;  //Keep 40 pixels between picturen and up/bottom boundary.
         lineGap -= 40;      //Keep 10 pixels between title and figure.
         lineGap -= txtHeight;  //Reserve space for corridnate title.
         lineGap -= 10;      //10 pixels between picture and coordinate title.
         lineGap /= lineNum;
         if(lineGap > 40)  //At most 50 pixels.
         {
                   lineGap = 40;
         }

         //OK,write out the figure's title,preserve 40 pixels between
         //up and left margin.
         TextOut(hDC,(wndRect.right - wndRect.left - txtWidth) / 2,topGap,figTitle);

         //Draw the figure's frame.
         hNewPen = CreatePen(0,1,RGB(0x80,0x80,0x80));  //Gray color.
         if(NULL == hNewPen)
         {
                   goto __TERMINAL;
         }
         hOldPen = SelectPen(hDC,hNewPen);
         for(i = 0;i < lineNum;i ++)
         {
                   sprintf(buff,"%d%%",lineNum - i);
                   TextOut(hDC,leftGap - 28,topGap + 12 + txtHeight + i * lineGap,buff);
                   DrawLine(hDC,leftGap,
                            topGap + 20 + txtHeight + i * lineGap,
                            leftGap + figWidth,
                            topGap + 20 + txtHeight + i * lineGap);
                   DrawLine(hDC,leftGap,
                            topGap + 21 + txtHeight + i * lineGap,
                            leftGap + figWidth,
                            topGap + 21 + txtHeight + i * lineGap);
                   //DrawLine(hDC,20,42 + txtHeight + i * lineGap,20 + figWidth,42 + txtHeight + i * lineGap);
         }

         //Draw the coordinate label.
         //SelectPen(hDC,hOldPen); //Use original pen.
         xCoord  = leftGap + colWidth;
         yCoord  = topGap + 20 + txtHeight + (lineNum - 1) * lineGap;
         yCoord += 10;
         for(i = 0;i < 12;i ++)
         {
                   TextOut(hDC,xCoord,yCoord,months[i]);
                   xCoord += colWidth;
                   xCoord += colWidth;
         }

         //Draw percentage rectangle.
         SelectPen(hDC,hNewPen);
         hNewBrush = CreateBrush(FALSE,RGB(0,192,240));
         if(NULL == hNewBrush)
         {
                   goto __TERMINAL;
         }
         hOldBrush = SelectBrush(hDC,hNewBrush);
         pctRect.left    = leftGap + colWidth;
         pctRect.right   = pctRect.left + colWidth;
         pctRect.bottom  = topGap + 20 + txtHeight + (lineNum - 1) * lineGap;
         for(i = 0;i < 12;i ++)
         {
                   pctRect.top   = pctRect.bottom - (CPIChange[i] * lineGap) / 10;
                   DrawRectangle(hDC,&pctRect);
                   pctRect.left += colWidth * 2;
                   pctRect.right = pctRect.left + colWidth;
         }

__TERMINAL:
         //Restore the original pen and brush.
         if(hOldPen)
         {
                   SelectPen(hDC,hOldPen);
         }
         if(hOldBrush)
         {
                   SelectBrush(hDC,hOldBrush);
         }
         if(NULL != hNewPen)
         {
                   DestroyPen(hNewPen);
         }
         if(NULL != hNewBrush)
         {
                   DestroyBrush(hNewBrush);
         }
         return;
}

//Window procedure of Hello World.
static DWORD StatWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         static HANDLE hDC = GetClientDC(hWnd);

         switch(message)
         {
         case WM_CREATE:    //Will receive this message when the window is created.
                   break;
         case WM_TIMER:     //Only one timer can be set for one window in current version.
                   break;
         case WM_DRAW:
                   DrawFrame(hDC,hWnd);
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
                   "CPI change in percent statistics for year 2010",
                   150,
                   150,
                   600,
                   400,
                   StatWndProc,
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
