//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 01, 2009
//    Module Name               : GDI.CPP
//    Module Funciton           : 
//                                The implementation code of GDI object.
//                                DC object,which is the most important
//                                object in GUI module,also implemented in this
//                                file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "..\INCLUDE\KAPI.H"
#include "..\INCLUDE\STRING.H"
#include "..\INCLUDE\VESA.H"
#include "..\INCLUDE\VIDEO.H"
#include "..\INCLUDE\CLIPZONE.H"
#include "..\INCLUDE\GDI.H"
#include "..\INCLUDE\WNDMGR.H"
#include "..\INCLUDE\WORDLIB.H"

//DrawPixel of DC object.
VOID DrawPixel(HANDLE hDC,int x,int y)
{
	__DC*     pDC     = (__DC*)hDC;
	__WINDOW* pWindow = NULL;
	int ax = x;
	int ay = y;
	
	if(NULL == pDC) //Invalid parameter.
	{
		return;
	}
	pWindow = (__WINDOW*)pDC->hWindow;
	if(pWindow)  //Window existing,calculate actual position of x and y.
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Window client's DC.
		{
			ax += pWindow->xclient;
			ay += pWindow->yclient;
		}
		if(pDC->dwDCType & DC_TYPE_WINDOW)  //Window's DC.
		{
			ax += pWindow->x;
			ay += pWindow->y;
		}
	}
	if(pDC->dwDCType & DC_TYPE_SCREEN)  //Text out to screen.
	{
		if(PtInRegion(pDC->pRegion,ax,ay))  //Should draw this pixel.
		{
			pDC->pVideo->DrawPixel(pDC->pVideo,ax,ay,pDC->pPen->color);
		}
	}
}

//DrawPixel of DC object,specify a color.
VOID DrawPixel(HANDLE hDC,int x,int y,__COLOR clr)
{
	__DC*     pDC     = (__DC*)hDC;
	__WINDOW* pWindow = NULL;
	int ax = x;
	int ay = y;
	
	if(NULL == pDC) //Invalid parameter.
	{
		return;
	}
	pWindow = (__WINDOW*)pDC->hWindow;
	if(pWindow)  //Window existing,calculate actual position of x and y.
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Window client's DC.
		{
			ax += pWindow->xclient;
			ay += pWindow->yclient;
		}
		if(pDC->dwDCType & DC_TYPE_WINDOW)  //Window's DC.
		{
			ax += pWindow->x;
			ay += pWindow->y;
		}
	}
	if(pDC->dwDCType & DC_TYPE_SCREEN)  //Text out to screen.
	{
		if(PtInRegion(pDC->pRegion,ax,ay))  //Should draw this pixel.
		{
			pDC->pVideo->DrawPixel(pDC->pVideo,ax,ay,clr);
		}
	}
}

//Local static helper routines to display characters.
static VOID __DispHZK16(HANDLE hDC,int x, int y, unsigned char *pHZ)
{   
	int QM=0;//区码
	int WM=0;//位码
	unsigned char * p;//汉字库的起始地址
	unsigned char * pm=0;//将要显示的字模的其实地址
	unsigned char mat[16][2];//字模数组
	int i,j,k=0;
	p=(unsigned char *)CHNCHAR_LIB_BASE;
	QM=*pHZ-0xA0;
	WM=*(pHZ+1)-0xA0;
	pm=p+(94*(QM-1)+(WM-1))*32;
	for(i=0;i<16;i++)
		for(j=0;j<2;j++)
			mat[i][j]=*(pm+i*2+j);
		
		for(i=0;i<16;i++)
			for(j=0;j<2;j++)
				for(k=0;k<8;k++)
					if(mat[i][j]&(0x80>>k))
						DrawPixel(hDC,x+j*8+k,y+i);					
}

static VOID __DispASC16(HANDLE hDC,int x, int y, unsigned char *pXZ)
{   
	
	unsigned char * p;//英文字库的起始地址
	unsigned char * pm=0;//将要显示的字模的起始地址
	unsigned char mat[16];//字模数组
	int i,k=0;
	p=(unsigned char *)ASCII_LIB_BASE;
	//if((*pXZ)>0 && (*pXZ)<0x80)
	{
		pm=(*pXZ)*16+p;
	}
	
	for(i=0;i<16;i++)
		mat[i]=*(pm+i);
	
	for(i=0;i<16;i++)
		for(k=0;k<8;k++)
			if(mat[i]&(0x80>>k))
				DrawPixel(hDC,x+k,y+i);
}

static VOID __TextOut(HANDLE hDC,int x,int y,char *pStr)
{
#define WITH_HZK16 16
#define WITH_ASC16 8
	unsigned char *p;
	int x0,y0;
	x0=x;
	y0=y;
	p=(unsigned char*)pStr;
	while(*p!='\0')
	{
		if((*p>=0x80)&&(*(p+1)>=0x80))
		{	
			__DispHZK16(hDC,x0, y0, (unsigned char *)p);
			p+=2;
			x0+=WITH_HZK16;
		}
		else if((*p<0x80)&&(*p>0))
		{
			__DispASC16(hDC,x0, y0, (unsigned char *)p);
			p++;
			x0+=WITH_ASC16;
		}
		else if(*p>=80 && *(p+1)<0x80)
		{
			p++;
		}
	}
}

//TextOut of DC.
VOID TextOut(HANDLE hDC,int x,int y,TCHAR* pszString)
{
	__DC* pDC = (__DC*)hDC;
	__WINDOW* pWindow = NULL;
	int strspace = 0;
	int ax = x;  //Actual x,should be adjusted later according to DC's base window.
	int ay = y;  //Actual y,should be adjusted later according to DC's base window.

	if((NULL == pDC) || (NULL == pszString))  //Invalid parameters.
	{
		return;
	}
	pWindow = (__WINDOW*)pDC->hWindow;
	if(pWindow)  //Window existing,adjust actual position of x and y.
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Window client's DC.
		{
			ax += pWindow->xclient;
			ay += pWindow->yclient;
		}
		if(pDC->dwDCType & DC_TYPE_WINDOW)  //Window's DC.
		{
			ax += pWindow->x;
			ay += pWindow->y;
		}
	}
	//Clear the background of the text.
	strspace = strlen(pszString);
	strspace *= 8;
	DrawRectangle(pDC->pVideo,ax,ay,ax + strspace,ay + 16,
		pDC->pBrush->color,TRUE,pDC->pBrush->color);
	//Call device level text out routine to draw text.
	__TextOut(hDC,x,y,pszString);
	return;
}

//Create a DC object.
HANDLE CreateDeviceContext(DWORD dwDCType,HANDLE hDevice,HANDLE hWnd,__REGION* pRegion)
{
	__DC* pDC         = NULL;
	HANDLE hPen       = NULL;
	HANDLE hBrush     = NULL;
	HANDLE hFont      = NULL;
	BOOL   bResult    = FALSE;
	__WINDOW* pWnd    = (__WINDOW*)hWnd;
	
	if(NULL == hDevice)
	{
		return NULL;
	}
	pDC = (__DC*)KMemAlloc(sizeof(__DC),KMEM_SIZE_TYPE_ANY);
	if(NULL == pDC) //Can not allocate memory.
	{
		goto __TERMINAL;
	}
	//Now create pen,font,brush objects.
	hPen = CreatePen(0,1,COLOR_BLACK);
	if(NULL == hPen)
	{
		goto __TERMINAL;
	}
	hFont = CreateFont(DEFAULT_FONT_WIDTH,DEFAULT_FONT_HEIGHT,
		DEFAULT_FONT_CHSPACE,DEFAULT_FONT_LNSPACE);
	if(NULL == hFont)
	{
		goto __TERMINAL;
	}
	hBrush = CreateBrush(FALSE,pWnd->clrBackground);
	if(NULL == hBrush)
	{
		goto __TERMINAL;
	}

	//Initialize DC object.
	pDC->dwDCType = dwDCType;
	pDC->pBrush   = (__BRUSH*)hBrush;
	pDC->pFont    = (__FONT*)hFont;
	pDC->hWindow  = hWnd;
	pDC->pPen     = (__PEN*)hPen;
	pDC->pRegion  = pRegion;
	if(dwDCType | DC_TYPE_SCREEN)
	{
		pDC->pVideo = (__VIDEO*)hDevice;
	}
	else
	{
		pDC->hOther = hDevice;
	}
	bResult = TRUE;
__TERMINAL:
	if(!bResult)
	{
		if(pDC)
		{
			KMemFree(pDC,KMEM_SIZE_TYPE_ANY,0);
		}
		if(hPen)
		{
			DestroyPen(hPen);
		}
		if(hBrush)
		{
			DestroyBrush(hBrush);
		}
		if(hFont)
		{
			DestroyFont(hFont);
		}
		pDC = NULL;
	}
	return (HANDLE)pDC;
}

//Destroy a DC object.
VOID DestroyDeviceContext(HANDLE hDC)
{
	__DC* pDC = (__DC*)hDC;

	if(pDC)
	{
		DestroyPen(pDC->pPen);
		DestroyBrush(pDC->pBrush);
		DestroyFont(pDC->pFont);
		KMemFree(pDC,KMEM_SIZE_TYPE_ANY,0);
	}
}

//Create a PEN object.
HANDLE CreatePen(DWORD type,int width,__COLOR color)
{
	__PEN* pPen = (__PEN*)KMemAlloc(sizeof(__PEN),KMEM_SIZE_TYPE_ANY);
	if(NULL == pPen)
	{
		return NULL;
	}
	pPen->type   = type;
	pPen->width  = width;
	pPen->color  = color;

	return (HANDLE)pPen;
}

//Destroy a pen object.
VOID DestroyPen(HANDLE hPen)
{
	if(hPen)
	{
		KMemFree(hPen,KMEM_SIZE_TYPE_ANY,0);
	}
}

//Create a BRUSH object.
HANDLE CreateBrush(BOOL bTransparent,__COLOR color)
{
	__BRUSH* pBrush = (__BRUSH*)KMemAlloc(sizeof(__BRUSH),KMEM_SIZE_TYPE_ANY);
	if(NULL == pBrush)
	{
		return NULL;
	}
	pBrush->bTransparent = bTransparent;
	pBrush->color        = color;
	return (HANDLE)pBrush;
}

//Destroy a brush object.
VOID DestroyBrush(HANDLE hBrush)
{
	if(hBrush)
	{
		KMemFree(hBrush,KMEM_SIZE_TYPE_ANY,0);
	}
}

//Create a FONT object.
HANDLE CreateFont(int width,int height,int chspace,int lnspace)
{
	__FONT* pFont = (__FONT*)KMemAlloc(sizeof(__FONT),KMEM_SIZE_TYPE_ANY);
	if(NULL == pFont)
	{
		return NULL;
	}
	pFont->width    = width;
	pFont->ascwidth = width / 2;
	pFont->height   = height;
	pFont->chspace  = chspace;
	pFont->lnspace  = lnspace;
	return (HANDLE)pFont;
}

//Destroy a FONT object.
VOID DestroyFont(HANDLE hFont)
{
	if(hFont)
	{
		KMemFree(hFont,KMEM_SIZE_TYPE_ANY,0);
	}
}

//Check if a point fall in the given rectange.
BOOL PtInRect(__RECT* pRect,int x,int y)
{
	if(NULL == pRect)
	{
		return FALSE;
	}
	return ((x >= pRect->left)    &&
		    (x <= pRect->right)   &&
			(y >= pRect->top)     &&
			(y <= pRect->bottom));
}

//Get text string's display information.
DWORD GetTextMetric(HANDLE hDC,TCHAR* pszText,DWORD dwInfoClass)
{
	__DC*    pDC = (__DC*)hDC;
	int      len = 0;
	DWORD    dwRetVal = 0;

	if((NULL == pDC) || (NULL == pszText))
	{
		return 0;
	}
	switch(dwInfoClass)
	{
	case TM_WIDTH:
		len = strlen(pszText);
		len -= 1;  //Skip the terminator,zero.
		dwRetVal = len * pDC->pFont->ascwidth;
		break;
	case TM_HEIGHT:
		dwRetVal = pDC->pFont->height;
		break;
	default:
		break;
	}
	return dwRetVal;
}

//Draw a rectangle in the given DC.
VOID DrawRectangle(HANDLE hDC,__RECT rect)
{
	__DC*          pDC    = (__DC*)hDC;
	__VIDEO*       pVideo = NULL;
	__COLOR        clrbrush  = COLOR_WHITE;
	__COLOR        clrpen    = COLOR_BLACK;
	__WINDOW*      pWnd      = NULL;
	int x1,y1,x2,y2;

	if(NULL == pDC)
	{
		return;
	}
	if(pDC->dwDCType & DC_TYPE_SCREEN)
	{
		pVideo = (__VIDEO*)pDC->pVideo;
	}
	else
	{
		pVideo = (__VIDEO*)pDC->hOther;
	}
	clrbrush = pDC->pBrush->color;
	clrpen   = pDC->pPen->color;
	pWnd     = (__WINDOW*)pDC->hWindow;

	x1 = rect.left;
	y1 = rect.top;
	x2 = rect.right;
	y2 = rect.bottom;
	if(pDC->dwDCType & DC_TYPE_WINDOW)  //Is a window frame DC.
	{
		if(pWnd)
		{
			x1 += pWnd->x;
			x2 += pWnd->x;
			y1 += pWnd->y;
			y2 += pWnd->y;
		}
	}
	else
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Client DC.
		{
			if(pWnd)
			{
				x1 += pWnd->xclient;
				x2 += pWnd->xclient;
				y1 += pWnd->yclient;
				y2 += pWnd->yclient;
			}
		}
	}
	pVideo->DrawRectangle(pVideo,x1,y1,x2,y2,clrpen,TRUE,clrbrush);
	return;
}

//Draw a rectangle given a rectangle's pointer.
VOID DrawRectangle(HANDLE hDC,__RECT* pRect)
{
	DrawRectangle(hDC,*pRect);
}

//Draw a circle by using the given context hDC.
VOID DrawCircle(HANDLE hDC,int xc,int yc,int r,BOOL bFill)
{
	__DC*          pDC    = (__DC*)hDC;
	__VIDEO*       pVideo = NULL;
	__COLOR        clrbrush  = COLOR_WHITE;
	__COLOR        clrpen    = COLOR_BLACK;
	__WINDOW*      pWnd      = NULL;
	int x1,y1;

	if(NULL == pDC)
	{
		return;
	}
	if(pDC->dwDCType & DC_TYPE_SCREEN)
	{
		pVideo = (__VIDEO*)pDC->pVideo;
	}
	else
	{
		pVideo = (__VIDEO*)pDC->hOther;
	}
	clrbrush = pDC->pBrush->color;
	clrpen   = pDC->pPen->color;
	pWnd     = (__WINDOW*)pDC->hWindow;

	x1 = xc;
	y1 = yc;
	if(pDC->dwDCType & DC_TYPE_WINDOW)  //Is a window frame DC.
	{
		if(pWnd)
		{
			x1 += pWnd->x;
			y1 += pWnd->y;
		}
	}
	else
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Client DC.
		{
			if(pWnd)
			{
				x1 += pWnd->xclient;
				y1 += pWnd->yclient;
			}
		}
	}
	pVideo->DrawCircle(pVideo,x1,y1,r,clrpen,FALSE);
	if(bFill)  //Should fill the circle.
	{
		pVideo->DrawCircle(pVideo,x1,y1,r - 1,clrbrush,TRUE);
	}
	return;
}

//Draw line in a given DC.
VOID DrawLine(HANDLE hDC,int x1,int y1,int x2,int y2)
{
	int startx = x1;
	int starty = y1;
	int endx   = x2;
	int endy   = y2;
	__WINDOW* pWnd = NULL;
	__DC*     pDC = (__DC*)hDC;
	__COLOR   clr = COLOR_BLACK;
	__VIDEO*  pVideo = NULL;

	if(NULL == pDC)
	{
		return;
	}
	if(pDC->dwDCType & DC_TYPE_SCREEN)
	{
		pVideo = (__VIDEO*)pDC->pVideo;
	}
	else
	{
		pVideo = (__VIDEO*)pDC->hOther;
	}
	clr   = pDC->pPen->color;
	pWnd     = (__WINDOW*)pDC->hWindow;

	if(pDC->dwDCType & DC_TYPE_WINDOW)  //Is a window frame DC.
	{
		if(pWnd)
		{
			startx += pWnd->x;
			endx   += pWnd->x;
			starty += pWnd->y;
			endy   += pWnd->y;
		}
	}
	else
	{
		if(pDC->dwDCType & DC_TYPE_CLIENT)  //Client DC.
		{
			if(pWnd)
			{
				startx += pWnd->xclient;
				endx   += pWnd->xclient;
				starty += pWnd->yclient;
				endy   += pWnd->yclient;
			}
		}
	}
	pVideo->DrawLine(pVideo,startx,starty,endx,endy,clr);
}

//Select a new pen object for given DC,returns the old one.
HANDLE SelectPen(HANDLE hDC,HANDLE hNewPen)
{
	__DC*        pDC     = (__DC*)hDC;
	HANDLE       hOldPen = NULL;

	if((NULL == pDC) || (NULL == hNewPen))
	{
		return NULL;
	}
	hOldPen = (HANDLE)pDC->pPen;
	pDC->pPen = (__PEN*)hNewPen;
	return hOldPen;
}

//Select a new brush object for given DC,returns the old one.
HANDLE SelectBrush(HANDLE hDC,HANDLE hNewBrush)
{
	__DC*        pDC       = (__DC*)hDC;
	HANDLE       hOldBrush = NULL;

	if((NULL == pDC) || (NULL == hNewBrush))
	{
		return NULL;
	}
	hOldBrush = (HANDLE)pDC->pBrush;
	pDC->pBrush = (__BRUSH*)hNewBrush;
	return hOldBrush;
}

//Select a new font object for given DC,returns the old one.
HANDLE SelectFont(HANDLE hDC,HANDLE hNewFont)
{
	__DC*        pDC      = (__DC*)hDC;
	HANDLE       hOldFont = NULL;

	if((NULL == pDC) || (NULL == hNewFont))
	{
		return NULL;
	}
	hOldFont = (HANDLE)pDC->pFont;
	pDC->pFont = (__FONT*)hNewFont;
	return hOldFont;
}

