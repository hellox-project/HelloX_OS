//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 05,2012
//    Module Name               : bmpbtn.cpp
//    Module Funciton           : 
//                                The implementation code of bitmap button,which
//                                contains one bitmap and one text message,it's behavior
//                                likes normal button.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/
 
#include "..\INCLUDE\KAPI.H"
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
 
#include "..\INCLUDE\stdio.h"
#include "..\INCLUDE\string.h"
 
#include "..\include\bmpbtn.h"
 
//A local helper routine to display bitmap on bitmap button.
static void DrawBitmap(HANDLE hDC,__BITMAP_BUTTON* pButton)
{
         __COLOR* pClrArray = (__COLOR*)pButton->pBmpData;
         //__COLOR  clr       = RGB(0x80,0x80,0x80);
         int width  = pButton->cx;
         int height = pButton->cx;  //Assume the bitmap has same height value as cx.
         int i,j;
 
         for(i = 0;i < height;i ++)
         {
                   for(j = 0;j < width;j ++)
                   {
                            DrawPixel(hDC,j,i,*pClrArray);
                            //DrawPixel(hDC,i * width,j,clr);
                            pClrArray ++;
                   }
         }
}

//A local helper routine to draw bitmap button's frame,in normal state.
//It actually draws 2 white lines on button's left and top edges,then draws
//2 whites lines on button's right and bottom edges,to render the button
//looks like a 3D effect.
static VOID DrawButtonFrame(HANDLE hDC,__BITMAP_BUTTON* pButton)
{
	HANDLE hOldPen    = NULL;
	HANDLE hBlackPen  = NULL;
	HANDLE hWhitePen  = NULL;

	//Creat 2 pens,one is black and another is white.
	hBlackPen = CreatePen(0,1,RGB(0,0,0));
	if(NULL == hBlackPen)
	{
		goto __TERMINAL;
	}
	hWhitePen = CreatePen(0,1,RGB(255,255,255));
	if(NULL == hWhitePen)
	{
		goto __TERMINAL;
	}
	if(pButton->dwBmpBtnStatus == BUTTON_STATUS_NORMAL)
	{
		hOldPen = SelectPen(hDC,hWhitePen);
		//Draw button's top and left edge.
		DrawLine(hDC,0,0,pButton->cx,0);
		//DrawLine(hDC,0,1,pButton->cx,1);
		DrawLine(hDC,0,0,0,pButton->cy);
		//DrawLine(hDC,1,0,1,pButton->cy);
		//Draw button's bottom and right edit.
		SelectPen(hDC,hBlackPen);
		DrawLine(hDC,pButton->cx,0,pButton->cx,pButton->cy);
		//DrawLine(hDC,pButton->cx - 1,2,pButton->cx - 1,pButton->cy);
		DrawLine(hDC,0,pButton->cy,pButton->cx,pButton->cy);
		//DrawLine(hDC,2,pButton->cy,pButton->cx,pButton->cy);
	}
	else
	{
		hOldPen = SelectPen(hDC,hBlackPen);
		//Draw button's top and left edge.
		DrawLine(hDC,0,0,pButton->cx,0);
		//DrawLine(hDC,0,1,pButton->cx,1);
		DrawLine(hDC,0,0,0,pButton->cy);
		//DrawLine(hDC,1,0,1,pButton->cy);
		//Draw button's bottom and right edit.
		SelectPen(hDC,hWhitePen);
		DrawLine(hDC,pButton->cx,0,pButton->cx,pButton->cy);
		//DrawLine(hDC,pButton->cx - 1,2,pButton->cx - 1,pButton->cy);
		DrawLine(hDC,0,pButton->cy,pButton->cx,pButton->cy);
		//DrawLine(hDC,2,pButton->cy,pButton->cx,pButton->cy);
	}
	SelectPen(hDC,hOldPen);

__TERMINAL:
	if(NULL != hBlackPen)
	{
		DestroyPen(hBlackPen);
	}
	if(NULL != hWhitePen)
	{
		DestroyPen(hWhitePen);
	}
}
 
//Local helper routine,to draw a bitmap button in a given window or screen.
static VOID DrawButtonNormal(HANDLE hDC,__BITMAP_BUTTON* pButton)
{
         HANDLE hOldPen     = NULL;
         HANDLE hOldBrush   = NULL;
         HANDLE hTxtPen     = NULL;
         HANDLE hTxtBkPen   = NULL;
         HANDLE hTxtBrush   = NULL;
         HANDLE hFacePen    = NULL;
         HANDLE hFaceBrush  = NULL;
         __RECT rect;
 
         if((NULL == hDC) || (NULL == pButton))
         {
                   goto __TERMINAL;
         }
 
         //Create the pen used to draw bitmap button.
         hTxtPen = CreatePen(0,1,pButton->TxtColor);
         if(NULL == hTxtPen)
         {
                   goto __TERMINAL;
         }
         hTxtBkPen = CreatePen(0,1,pButton->TxtBackground);
         if(NULL == hTxtBkPen)
         {
                   goto __TERMINAL;
         }
         hTxtBrush = CreateBrush(FALSE,pButton->TxtBackground);
         if(NULL == hTxtBrush)
         {
                   goto __TERMINAL;
         }
         hFacePen = CreatePen(0,1,pButton->FaceClr);
         if(NULL == hFacePen)
         {
                   goto __TERMINAL;
         }
         hFaceBrush = CreateBrush(FALSE,pButton->FaceClr);
         if(NULL == hFaceBrush)
         {
                   goto __TERMINAL;
         }
 
         //Draw the button's face.
         hOldPen   = SelectPen(hDC,hFacePen);
         hOldBrush = SelectBrush(hDC,hFaceBrush);
         rect.left = 0;
         rect.right = pButton->cx;
         rect.top = 0;
         rect.bottom = pButton->cy - pButton->txtheight;
         DrawRectangle(hDC,rect);
 
         //Draw bitmap here.
		 if(pButton->pBmpData)  //Bitmap has been specified.
		 {
			 DrawBitmap(hDC,pButton);
		 }
 
         //Draw text background rectangle.
         SelectPen(hDC,hTxtBkPen);
         SelectBrush(hDC,hTxtBrush);
         rect.left = 0;
         rect.right = pButton->cx;
         rect.top = pButton->cy - pButton->txtheight;
         rect.bottom = pButton->cy;
         DrawRectangle(hDC,rect);
 
         //Write button's text.The text's coordinate and length is set and verified
         //in CreateBitmapButton routine.
         SelectPen(hDC,hTxtPen);
         TextOut(hDC,pButton->xtxt,pButton->ytxt,pButton->ButtonText);
 
         //Restore the DC's old pen and brush.
         SelectPen(hDC,hOldPen);
         SelectBrush(hDC,hOldBrush);
 
__TERMINAL:
         if(hTxtPen)
         {
                   DestroyPen(hTxtPen);
         }
         if(hTxtBkPen)
         {
                   DestroyPen(hTxtBkPen);
         }
         if(hTxtBrush)
         {
                   DestroyBrush(hTxtBrush);
         }
         if(hFacePen)
         {
                   DestroyPen(hFacePen);
         }
         if(hFaceBrush)
         {
                   DestroyBrush(hFaceBrush);
         }
         return;
}
 
//Window procedure of bitmap button control.
static DWORD BmpButtonWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam)
{
         HANDLE hDC                = GetClientDC(hWnd);
         __BITMAP_BUTTON*  pButton = NULL;
         __WINDOW_MESSAGE msg;
 
         if(NULL == hWnd)
         {
                   return 0;
         }
         pButton = (__BITMAP_BUTTON*)GetWindowExtension(hWnd);
         if(NULL == pButton)
         {
                   return DefWindowProc(hWnd,message,wParam,lParam);
         }
         
         switch(message)
         {
         case WM_CREATE:
                   DrawButtonNormal(hDC,pButton);
				   DrawButtonFrame(hDC,pButton);
                   break;
         case WM_DRAW:
                   DrawButtonNormal(hDC,pButton);
				   DrawButtonFrame(hDC,pButton);
                   break;
         case WM_LBUTTONDOWN:
                   if(pButton->dwBmpBtnStatus == BUTTON_STATUS_PRESSED) //Already in pressed status.
                   {
                            break;
                   }
                   pButton->dwBmpBtnStatus = BUTTON_STATUS_PRESSED;
                   DrawButtonFrame(hDC,pButton);
                   break;
         case WM_MOUSEMOVE:
                   if(pButton->dwBmpBtnStatus == BUTTON_STATUS_PRESSED) //Button is hold.
                   {
                            pButton->dwBmpBtnStatus = BUTTON_STATUS_NORMAL;
                            DrawButtonFrame(hDC,pButton);
                   }
                   break;
         case WM_LBUTTONUP:
                   if(pButton->dwBmpBtnStatus == BUTTON_STATUS_NORMAL) //Already in normal status.
                   {
                            break;
                   }
                   pButton->dwBmpBtnStatus = BUTTON_STATUS_NORMAL;
                   DrawButtonFrame(hDC,pButton);
                   //Send button pressed message to it's parent.
                   msg.hWnd = GetParentWindow(hWnd);
                   msg.message = WM_COMMAND;
                   msg.wParam  = pButton->dwBmpButtonId;
                   msg.lParam  = 0;
                   SendWindowMessage(msg.hWnd,&msg);
                   break;
         case WM_CLOSE:
         case WM_DESTROY:
                   //Release the common control specific resource,the system level resource,
                  //such as window resource,will be released by DefWindowProc routine.
                   KMemFree(pButton,KMEM_SIZE_TYPE_ANY,0);
                   SetWindowExtension(hWnd,NULL);
                   break;
         }
         return DefWindowProc(hWnd,message,wParam,lParam);
}
 
//The implementation code of CreateButton.
HANDLE CreateBitmapButton(HANDLE hParent,TCHAR* pszText,
                                                          DWORD dwButtonId,int x,int y,
                                                          int cxbmp,int cybmp,
                                                          LPVOID pBitmap,
                                                          LPVOID pExtension)
{
         HANDLE                hButton  = NULL;
         __WINDOW*             pBtnWnd  = NULL;
         __BITMAP_BUTTON*      pButton  = NULL;
         BOOL                  bResult  = FALSE;
         __WINDOW*             pParent  = (__WINDOW*)hParent;
         __WINDOW_MESSAGE      msg;
         int                   height   = 0;  //Total width.
         int                   width    = 0;  //Total height.
         int                   txtheight = 0;
         int                   txtwidth = 0;
         int                   ntxtlen  = 0;
         HANDLE                hDC      = NULL;
 
         if(NULL == hParent)  //Invalid.
         {
                   return NULL;
         }
 
         hDC = GetWindowDC(hParent);
         pButton = (__BITMAP_BUTTON*)KMemAlloc(sizeof(__BITMAP_BUTTON),KMEM_SIZE_TYPE_ANY);
         if(NULL == pButton)
         {
                   goto __TERMINAL;
         }
         //Initialize button.
         pButton->dwBmpButtonId = dwButtonId;
 
         ntxtlen = strlen(pszText);
         if(ntxtlen >= BMPBTN_TEXT_LENGTH - 1)  //Text too long.
         {
                   goto __TERMINAL;
         }
         strcpy(pButton->ButtonText,pszText);
         pButton->x = x; //+ pParent->xclient;
         pButton->y = y; //+ pParent->yclient;
 
         txtheight  = GetTextMetric(hDC,pszText,TM_HEIGHT);
         txtheight  += TXT_MARGIN;  //Uper margin.
         txtheight  += TXT_MARGIN;  //Bottom margin.
         pButton->cy = cybmp + txtheight;
         pButton->txtheight = txtheight;
 
         width = GetTextMetric(hDC,pszText,TM_WIDTH);
         if(width > cxbmp - TXT_MARGIN * 2)  //Too long.
         {
                   goto __TERMINAL;
         }
         pButton->cx = cxbmp;
 
         pButton->xtxt = (pButton->cx - width) / 2;
         pButton->ytxt = cybmp + TXT_MARGIN;
         pButton->txtwidth = width;
 
         pButton->dwBmpBtnStatus = BUTTON_STATUS_NORMAL;
         pButton->pBmpData    = pBitmap;
         pButton->pButtonExtension = pExtension;
 
         //Set default button colors.
         pButton->FaceClr        = DEFAULT_BMPBTN_FACECOLOR;
         pButton->TxtBackground  = DEFAULT_BMPBTN_TXTBACKGROUND;
         pButton->TxtColor       = DEFAULT_BMPBTN_TXTCOLOR;
 
         //Allocate memory for bitmap data.
         if(pBitmap)
         {
                   pButton->pBmpData = KMemAlloc(cxbmp * cybmp * sizeof(__COLOR),KMEM_SIZE_TYPE_ANY);
                   if(NULL == pButton->pBmpData)
                   {
                            goto __TERMINAL;
                   }
                   _hx_memcpy(pButton->pBmpData,pBitmap,cxbmp * cybmp * sizeof(__COLOR));
         }
 
         //Create the button window.
         hButton = CreateWindow(0,  //Without any caption and border.
                   NULL,  //Without title.
                   pButton->x + pParent->x,
                   pButton->y + pParent->y,
                   pButton->cx,
                   pButton->cy,
                   BmpButtonWndProc,
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
                            if(pButton->pBmpData)
                            {
                                     KMemFree(pButton->pBmpData,KMEM_SIZE_TYPE_ANY,0);
                            }
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
