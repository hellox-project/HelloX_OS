//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29,2009
//    Module Name               : GUIENTRY.CPP
//    Module Funciton           : 
//                                GUI shell main entry routine is implemented
//                                in this routine.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include ".\KAPI\KAPI.H"
#include ".\KAPI\stdio.h"
#include ".\KAPI\string.h"

#include "VESA.H"
#include "VIDEO.H"
#include "HCGUI.H"

//Mouse map,to indicate which point should be draw in a mouse rectangle.
static int MouseMap[16][16] = {
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
	{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

//A array of color to save the screen covered by mouse icon.
static __COLOR MouseRect[16][16] = {COLOR_WHITE};

//A helper routine to save the screen rectangle occupied by mouse icon.
static VOID SaveMouseRect(__VIDEO* pVideo,int x,int y)
{
	int i,j;
	for(i = 0;i < 16;i ++)
	{
		if(y + i >= (int)pVideo->dwScreenHeight)
		{
			break;
		}
		for(j = 0;j < 16;j ++)
		{
			if(x + j >= (int)pVideo->dwScreenWidth)
			{
				break;
			}
			if(MouseMap[i][j])
			{
				MouseRect[i][j] = GetPixel(pVideo,x + j,y + i);
			}
		}
	}
}

//A helper routine to draw mouse ICON on given position.
static VOID DrawMouse(__VIDEO* pVideo,int x,int y)
{
	int i,j;
	//Save the rect occupied by mouse first.
	SaveMouseRect(pVideo,x,y);
	for(i = 0;i < 16;i ++)
	{
		if(y + i >= (int)pVideo->dwScreenHeight)
		{
			break;
		}
		for(j = 0;j < 16;j ++)
		{
			if(MouseMap[i][j])  //Should draw.
			{
				if(x + j >= (int)pVideo->dwScreenWidth)
				{
					break;
				}
				DrawPixel(pVideo,x + j,y + i,COLOR_BLACK);
			}
		}
	}
}

//A helper routine to restore the screen rect occupied by mouse.
static VOID RestoreMouseRect(__VIDEO* pVideo,int x,int y)
{
	int i,j;
	for(i = 0;i < 16;i ++)
	{
		if(y + i >= (int)pVideo->dwScreenHeight)
		{
			break;
		}
		for(j = 0;j < 16;j ++)
		{
			if(MouseMap[i][j])  //Should draw.
			{
				if(x + j >= (int)pVideo->dwScreenWidth)
				{
					break;
				}
				DrawPixel(pVideo,x + j,y + i,MouseRect[i][j]);  //Caution here,j dimension is for x.
			}
		}
	}
}

//MOUSE MOVE event handler.
static VOID DoMouseMove(int x,int y)  //x and y is the coordinate of mouse.
{
	static int xppos = 0;  //Previous position of x.
	static int yppos = 0;  //Previous position of y.
	int xpos,ypos;

	MouseToScreen(&Video,x,y,&xpos,&ypos);
	//DrawPixel(&Video,xpos,ypos,COLOR_WHITE);
	RestoreMouseRect(&Video,xppos,yppos);  //Restore previous screen rectangle.
	DrawMouse(&Video,xpos,ypos);  //Draw mouse in the new location.
	xppos = xpos;
	yppos = ypos;
}

//LEFT MOUSE BUTTON DOWN event handler.
static VOID DoLButtonDown(int x,int y)
{
	int xpos,ypos;

	MouseToScreen(&Video,x,y,&xpos,&ypos);
	DrawPixel(&Video,xpos,ypos,COLOR_WHITE);
}

//LEFT MOUSE BUTTON double clicked event handler.
static VOID DoLButtonDbClk(int x,int y)
{
	int xpos,ypos;

	MouseToScreen(&Video,x,y,&xpos,&ypos);
	//DrawPixel(&Video,xpos,ypos,COLOR_RED);
	DrawRectangle(&Video,xpos,ypos,xpos + 10,ypos + 10,COLOR_BLACK,TRUE,COLOR_RED);
}

//RIGHT MOUSE BUTTON double clicked event handler.
static VOID DoRButtonDbClk(int x,int y)
{
	int xpos,ypos;

	MouseToScreen(&Video,x,y,&xpos,&ypos);
	//DrawPixel(&Video,xpos,ypos,COLOR_GREEN);
	DrawRectangle(&Video,xpos,ypos,xpos + 10,ypos + 10,COLOR_BLACK,TRUE,COLOR_GREEN);
}

//RIGHT MOUSE BUTTON DOWN event handler.
static VOID DoRButtonDown(int x,int y)
{
}

//LEFT MOUSE BUTTON UP event handler.
static VOID DoLButtonUp(int x,int y)
{
}

//RIGHT MOUSE BUTTON UP event handler.
static VOID DoRButtonUp(int x,int y)
{
}

#define WND_CAPTION_HEIGHT 25
//This routine draws a window in screen.
static VOID DrawWindow(int x,int y,int cx,int cy,__COLOR bgcolor)
{
	//Draw the window's caption.
	DrawRectangle(&Video,x,y,x + cx,y + WND_CAPTION_HEIGHT,COLOR_BLACK,
		TRUE,COLOR_BLUE);
	//Draw the window's client area.
	DrawRectangle(&Video,x,y + WND_CAPTION_HEIGHT,x + cx,y + cy,COLOR_BLACK,
		TRUE,bgcolor);
}

//Clear screen,set the background color to button color.
static VOID InitScreen()
{
	DWORD* pVideoBase = (DWORD*)Video.pBaseAddress;
	for(int i = 0;i < 1024 * 768;i ++)
	{
		*pVideoBase = 0x00C0C0C0;
		pVideoBase ++;
	}
	//Draw two windows.
	DrawWindow(100,80,300,200,COLOR_WHITE);
	DrawWindow(450,200,400,300,COLOR_GREEN);
	DrawMouse(&Video,0,0);
}

//Entry routine of GUI module.
DWORD GuiShellEntry(LPVOID)
{
	MSG Msg;
	WORD x = 0;     //Mouse x scale.
	WORD y = 0;     //Mouse y scale.
	
	//Clear screen first.
	InitScreen();

	while(TRUE)
	{
		if(GetMessage(&Msg))
		{
			switch(Msg.wCommand)
			{
			case KERNEL_MESSAGE_LBUTTONDOWN:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonDown(x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONUP:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonUp(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDOWN:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonDown(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONUP:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonUp(x,y);
				break;
			case KERNEL_MESSAGE_MOUSEMOVE:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoMouseMove(x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONDBCLK:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoLButtonDbClk(x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDBCLK:
				x = (WORD)Msg.dwParam;
				y = (WORD)(Msg.dwParam >> 16);
				DoRButtonDbClk(x,y);
				break;
			//Please add other message processing action here.
			//For example,to handle timer message as follows:
			//
			// case KERNEL_MESSAGE_TIMER:
			//     break;

			default:
				break;
			}
		}
	}
	return 0;
}
