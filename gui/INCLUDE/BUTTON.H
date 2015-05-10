//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 10, 2009
//    Module Name               : BUTTON.H
//    Module Funciton           : 
//                                The definitions and constants related button,
//                                one of the most useful common control in GUI.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __BUTTON_H__
#define __BUTTON_H__
#endif

//Button control's property information.
#define BUTTON_TEXT_LEN 64  //Button text's maximal length.

struct __BUTTON{
	TCHAR    ButtonText[BUTTON_TEXT_LEN];
	DWORD    dwBtnStyle;
	DWORD    dwBtnStatus;    //Button status.
	DWORD    dwButtonId;     //Button identifier.
	int      x;
	int      y;
	int      cx;
	int      cy;
	//The following members are used to draw button's text.
	int      txtwidth;       //Button text's width,in pixel.
	int      xtxt;           //x coordinate of button text.
	int      ytxt;           //y coordinate of button text.
};

//Button status.
#define BUTTON_STATUS_NORMAL   0x0000001    //Normal status.
#define BUTTON_STATUS_PRESSED  0x0000002    //Pressed status.

//Create one button in a given window.
HANDLE CreateButton(HANDLE hParent,        //Parent window of the button.
					TCHAR* pszButtonText,  //Text displayed in button face.
					DWORD  dwButtonId,     //Button ID.
					int x,
					int y,
					int cx,
					int cy);

//Change button face text.
VOID SetButtonText(HANDLE hButton,TCHAR* pszButtonText);

//Change button face color.
VOID SetButtonColor(HANDLE hButton,__COLOR clr);
