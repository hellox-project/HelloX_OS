//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 01, 2009
//    Module Name               : GDI.H
//    Module Funciton           : 
//                                Graphic Device Interface pro-types and 
//                                structures.
//                                DC object,which is the most important
//                                object in GUI module,also defined in this
//                                file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __GDI_H__
#define __GDI_H__

//FONT object,to describe a type of font.
struct __FONT{
	int width;
	int height;
	int chspace;  //Space between 2 characters in one line.
	int lnspace;  //Space between 2 lines in one page.
	int ascwidth; //ASCII character's width.
};

#define DEFAULT_FONT_WIDTH    16
#define DEFAULT_FONT_HEIGHT   16
#define DEFAULT_FONT_CHSPACE  0
#define DEFAULT_FONT_LNSPACE  5

HANDLE CreateFont(int width,int height,int chspace,int lnspace);
VOID DestroyFont(HANDLE hFont);

//Text metric information.
#define TM_WIDTH        0x0000001     //Get a text string's width in a specified DC.
#define TM_HEIGHT       0x0000002     //Get a text string's height in a spec DC.
#define TM_MAXWIDTH     0x0000003     //Get the maximal character width in a string.

//Get text string display information.
DWORD GetTextMetric(HANDLE hDC,TCHAR* pszText,DWORD dwInfoClass);

//BRUSH object,to describe a kind of brush,which is used to
//fill other GUI objects.
struct __BRUSH{
	BOOL bTransparent;   //If the brush is transparent.
	__COLOR color;
};

#define DEFAULT_BRUSH_COLOR COLOR_WHITE

HANDLE CreateBrush(BOOL bTransparent,__COLOR color);
VOID DestroyBrush(HANDLE hBrush);

//PEN object,used to draw in target graphic device.
struct __PEN{
	DWORD type;
	int width;
	__COLOR color;
};

#define DEFAULT_PEN_WIDTH 1

HANDLE CreatePen(DWORD type,int width,__COLOR color);
VOID DestroyPen(HANDLE hPen);

//RECT object,to describe one rectangle object.
struct __RECT{
	int left;
	int top;
	int right;
	int bottom;
};

//Check if a point fall in the given rectangle.
BOOL PtInRect(__RECT* pRect,int x,int y);

//POINT object,to describe one point in graphic device.
struct __POINT{
	int x;
	int y;
};

//Device Context type.
#define DC_TYPE_SCREEN    0x00000001
#define DC_TYPE_PRINT     0x00000002
#define DC_TYPE_MEMORY    0x00000004
#define DC_TYPE_WINDOW    0x00000008  //Window DC.
#define DC_TYPE_CLIENT    0x00000010  //Window client DC.

//DC(Device Context) object,this is the most important object
//in GUI module,all application level drawing operations are
//depend on this object.
struct __DC{
	DWORD       dwDCType;      //DC type.
	__PEN*      pPen;          //Current drawing pen.
	__BRUSH*    pBrush;        //Current drawing brush.
	__FONT*     pFont;         //Current drawing font.
	__POINT     Point;         //Current position to draw.

	__VIDEO*    pVideo;       //Video device this DC based on.
	HANDLE      hOther;       //Other device this DC based on,such as PRINTER.
	HANDLE      hWindow;      //Window object of this DC.

	__REGION*   pRegion;      //Clip zone object,this object is defined
	                          //in CLIPZONE.H file.
};

//The following routine are used to operate DC.
//Create one DC object.
HANDLE CreateDeviceContext(DWORD dwDCType,HANDLE hDevice,HANDLE pWindow,__REGION* pRegion);

//Destroy one DC object.
VOID DestroyDeviceContext(HANDLE hDC);

//Draw a text string in specified DC.
VOID TextOut(HANDLE hDC,int x,int y,TCHAR* pszString);

//Draw a pixel on specified DC.
VOID DrawPixel(HANDLE hDC,int x,int y);
VOID DrawPixel(HANDLE hDC,int x,int y,__COLOR clr);

//Set current start drawing postion.
VOID MoveTo(HANDLE hDC,int x,int y);

//Draw a line from current position to target position.
VOID LineTo(HANDLE hDC,int x,int y);

//Draw a line from the given start and end position.
VOID DrawLine(HANDLE hDC,int x1,int y1,int x2,int y2);

//Draw a rectangle.
VOID DrawRectangle(HANDLE hDC,__RECT rect);
VOID DrawRectangle(HANDLE hDC,__RECT* pRect);

//Draw a circle.
VOID DrawCircle(HANDLE hDC,int xc,int yc,int r,BOOL bFill);

//Select one pen as DC's drawing pen,old one will be returned.
HANDLE SelectPen(HANDLE hDC,HANDLE hNewPen);

//Select one brush as DC's drawing brush.
HANDLE SelectBrush(HANDLE hDC,HANDLE hNewBrush);

//Select one FONT as DC's drawing font.
HANDLE SelectFont(HANDLE hDC,HANDLE hFont);

//Get the homing window of one DC.
HANDLE GetDCWindow(HANDLE hDC);

#endif