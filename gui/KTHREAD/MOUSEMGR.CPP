//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : MOUSEMGR.CPP
//    Module Funciton           : 
//                                Mouse related operating routines are implemented
//                                in this file,it's definition is in RAWIT.H file.
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
#include "..\INCLUDE\RAWIT.H"

//A helper routine to save the screen rectangle occupied by mouse icon.
static VOID SaveMouseRect(__MOUSE_MANAGER* pMouseMgr,__VIDEO* pVideo,int x,int y)
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
			if(pMouseMgr->MouseMap[i][j])
			{
				pMouseMgr->MouseRect[i][j] = GetPixel(pVideo,x + j,y + i);
			}
		}
	}
}

//A helper routine to draw mouse ICON on given position.
static VOID DrawMouse(__MOUSE_MANAGER* pMouseMgr,__VIDEO* pVideo,int x,int y)
{
	int i,j;
	//Save the rect occupied by mouse first.
	pMouseMgr->SaveMouseRect(pMouseMgr,pVideo,x,y);
	for(i = 0;i < 16;i ++)
	{
		if(y + i >= (int)pVideo->dwScreenHeight)
		{
			break;
		}
		for(j = 0;j < 16;j ++)
		{
			if(pMouseMgr->MouseMap[i][j])  //Should draw.
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
static VOID RestoreMouseRect(__MOUSE_MANAGER* pMouseMgr,__VIDEO* pVideo,int x,int y)
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
			if(pMouseMgr->MouseMap[i][j])  //Should draw.
			{
				if(x + j >= (int)pVideo->dwScreenWidth)
				{
					break;
				}
				DrawPixel(pVideo,x + j,y + i,pMouseMgr->MouseRect[i][j]);  //Caution here,j dimension is for x.
			}
		}
	}
}

//Declaration of mouse manager in GUI module.
__MOUSE_MANAGER MouseManager = {
	{
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
	},                                 //Initial mouse map,is a black arrow.
	{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	},                                  //Initial mousre rect,all zero. 

	SaveMouseRect,                      //SaveMouseRect.
	DrawMouse,                          //DrawMouse.
	RestoreMouseRect,                   //RestoreMouseRect.
	NULL,                               //ReplaceMouseMap.
};
