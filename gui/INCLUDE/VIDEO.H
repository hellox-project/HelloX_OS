//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 28,2009
//    Module Name               : VIDEO.H
//    Module Funciton           : 
//                                Declares video object and related structures,
//                                constants and global routines.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __VIDEO_H__
#define __VIDEO_H__
#endif

//Starting position of VBE related information. 
#define VBE_INFO_START 0x0000E000  //Start from 56K.

//Assume the display memory's length is 16M.
//Because there is not enough material to explain how to obtain the actual length
//of the display memory from VBE's mode information,so make this assumption.
//It has not any influence to the GUI module because only one page is used.
#define DISPLAY_MEMORY_LENGTH 0x1000000

//This structure resides at VBE_INFO_START postion.
struct __VBE_INFO{
	WORD                  IfInGraphic;
	WORD                  GraphicNum;
	__VBE_MODE_INFO       ModeInfo;
	__VBE_CONTROLLER_INFO ControllerInfo;
};

//COLOR structure is used to manage all colors.It is a 32 bits
//unsigned integer,which layout as following:
// bit0   -- bit7   : Blue dimension;
// bit8   -- bit15  : Gree dimension;
// bit16  -- bit23  : Red dimension.
typedef unsigned long __COLOR;

//A Macro to convert three dimensions to a color value.
#define RGB(r,g,b) ((((DWORD)r) << 16) + (((DWORD)g) << 8) + ((DWORD)b))

//Usable color constants.
#define COLOR_RED    0x00FF0000
#define COLOR_GREEN  0x0000FF00
#define COLOR_BLUE   0x000000FF
#define COLOR_WHITE  0x00FFFFFF
#define COLOR_BLACK  0x00000000

//Video object,one for each video device.
struct __VIDEO{
	DWORD          dwScreenWidth;    //Screen width.
	DWORD          dwScreenHeight;   //Screen height.
	DWORD          BitsPerPixel;     //Color bits for one pixel.
	LPVOID         pBaseAddress;     //Base address of display memory.
	DWORD          dwMemLength;      //Length of display memory.
	BOOL           (*Initialize)(__VIDEO* pVideo);
	VOID           (*Uninitialize)(__VIDEO* pVideo);

	VOID (*DrawPixel)(__VIDEO* pVideo,int x,int y,__COLOR color);
	__COLOR (*GetPixel)(__VIDEO* pVideo,int x,int y);
	VOID (*DrawLine)(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR color);
	VOID (*DrawRectangle)(__VIDEO* pVideo,int x1,int y1,int x2,int y2,
				   __COLOR lineclr,BOOL bFill,__COLOR fillclr);
	VOID (*DrawEllipse)(__VIDEO* pVide,int x1,int y1,int x2,int y2,__COLOR color,
				 BOOL bFill,__COLOR fillclr);
	VOID (*DrawCircle)(__VIDEO* pVideo,int xc,int yc,int r,__COLOR color,BOOL bFill);
	VOID (*MouseToScreen)(__VIDEO* pVideo,int x,int y,int* px,int* py);
};

//Basic routines to operate the video object.
VOID DrawPixel(__VIDEO* pVideo,int x,int y,__COLOR color);
__COLOR GetPixel(__VIDEO* pVideo,int x,int y);
VOID DrawLine(__VIDEO* pVideo,int x1,int y1,int x2,int y2,__COLOR c);
VOID DrawRectangle(__VIDEO* pVideo,int x1,int y1,int x2,int y2,
				   __COLOR lineclr,BOOL bFill,__COLOR fillclr);
VOID DrawEllipse(__VIDEO* pVide,int x1,int y1,int x2,int y2,__COLOR color,
				 BOOL bFill,__COLOR fillclr);
VOID MouseToScreen(__VIDEO* pVideo,int x,int y,int* px,int* py);

//The default video object.
extern __VIDEO Video;
