//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 05,2012
//    Module Name               : bmpbtn.h
//    Module Funciton           : 
//                                The definitions related to bitmap button,which
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

#ifndef __BMPBTN_H__
#define __BMPBTN_H__

//Bitmap button's text's maximal length,in character.
#define BMPBTN_TEXT_LENGTH 64

//Default face color,if user does not specify.A bitmap will be put
//on this face.
#define DEFAULT_BMPBTN_FACECOLOR 0x00C0FF

//Default bitmap buttons' text background.
#define DEFAULT_BMPBTN_TXTBACKGROUND 0x800000

//Default bitmap button's text color.
#define DEFAULT_BMPBTN_TXTCOLOR 0x00FFFFFF

//Several button face dimensions,button text rectangle is not
//included,more accurate,it's bitmap's dimension.
#define BMPBTN_DIM_16   0x00000001        //16 * 16 bitmap.
#define BMPBTN_DIM_32   0x00000002        //32 * 32 bitmap
#define BMPBTN_DIM_64   0x00000003        //64 * 64 bitmap.
#define BMPBTN_DIM_78   0x00000004        //78 * 78 bitmap.
#define BMPBTN_DIM_96   0x00000005        //96 * 96 bitmap.
#define BMPBTN_DIM_128  0x00000006        //128 * 128 bitmap.
#define BMPBTN_DIM_256  0x00000007        //256 * 256 bitmap.

//Margin between text and text rectangle,so text banner will
//occpy 6 + TXT_HEIGHT pixels,which include upper margin and 
//below margin,text font height.
#define TXT_MARGIN      3

//Button status.
#define BUTTON_STATUS_NORMAL   0x0000001    //Normal status.
#define BUTTON_STATUS_PRESSED  0x0000002    //Pressed status.

//Bitmap buttons' definition.
struct __BITMAP_BUTTON{
	TCHAR    ButtonText[BMPBTN_TEXT_LENGTH];
	DWORD    dwBmpBtnStyle;
	DWORD    dwBmpBtnStatus;    //Button status.
	DWORD    dwBmpButtonId;     //Button identifier.

	int      x;                 //Coordinate relative to it's parent window.
	int      y;
	int      cx;                //Total width and height,include text part.
	int      cy;

	//The following members are used to draw bitmap button's text.
	int      txtwidth;       //Button text rectangle's width,in pixel.
	int      txtheight;      //Button text rectangle's height,in pixel.
	int      xtxt;           //x coordinate of button text.
	int      ytxt;           //y coordinate of button text.

	//Button's appearence colors.
	__COLOR  FaceClr;         //Face color.
	__COLOR  TxtBackground;   //Text background color.
	__COLOR  TxtColor;        //Button text's color.
	__COLOR  FaceColor;       //Button's face color.

	//Bitmap button's bitmap data.
	LPVOID   pBmpData;

	//Bitmap button extension,user specific data can be put here.
	LPVOID   pButtonExtension;
};

//Create one button in a given window.
HANDLE CreateBitmapButton(HANDLE hParent,        //Parent window of the button.
						  TCHAR* pszButtonText,  //Text displayed in button face.
						  DWORD  dwButtonId,     //Button ID.
						  int x,             //Coordinate of the bitmap button in screen.
						  int y,
						  int cxbmp,            //Button's bitmap dimension.
						  int cybmp,
						  LPVOID pBitmap,    //Button's bitmap data.
						  LPVOID pExtension  //Button's extension.
						  );

//Change button face text.
VOID SetBitmapButtonText(HANDLE hBmpButton,TCHAR* pszButtonText);

//Change bitmap button's bitmap.Bitmap's dimension can not be changed,only it's content
//can be changed using this routine.
VOID SetBitmapButtonBitmap(HANDLE hBmpButton,LPVOID pBitmap);

//Change bitmap button's text background and text color.
VOID SetBitmapButtonTextColor(HANDLE hBmpButton,__COLOR txtClr,__COLOR bkgroundClr);

//Set button's extension,the old one will be returned.
LPVOID SetBitmapButtonExtension(HANDLE hBmpButton,LPVOID pNewExtension);

//Get bitmap button's extension.
LPVOID GetBitmapButtonExtension(HANDLE hBmpButton);

#endif