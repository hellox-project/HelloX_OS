//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 10, 2009
//    Module Name               : MSGBOX.H
//    Module Funciton           : 
//                                The definition code of MessageBox.
//                                This control also be the most important and useful
//                                control in GUI module.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MSGBOX_H__
#define __MSGBOX_H__
#endif

//Message box types.
#define MB_OK        0x00000001
#define MB_CANCEL    0x00000002
#define MB_YES       0x00000004
#define MB_NO        0x00000008

//Alias of available combination,other value is illegal.
#define MB_OKCANCEL     (MB_OK | MB_CANCEL)
#define MB_YESNO        (MB_YES | MB_NO)
#define MB_YESNOCANCEL  (MB_YES | MB_NO | MB_CANCEL)
//#define MB_OK           MB_OK  //MB_OK itself is a valid combination.

//Button ID for message box.
#define BUTTON_ID_OK      0x00000001
#define BUTTON_ID_CANCEL  0x00000002
#define BUTTON_ID_YES     0x00000004
#define BUTTON_ID_NO      0x00000008

//MessageBox routine pro-type.
DWORD MessageBox(HANDLE hWnd,TCHAR* pszText,TCHAR* pszTitle,UINT uType);

//Message box related specific data.
#define MSGBOX_TEXT_LENGTH 64

struct __MESSAGE_BOX{
	TCHAR        MsgBoxText[MSGBOX_TEXT_LENGTH];
	DWORD        dwMsgBoxType;
	DWORD        dwRetVal;
	DWORD        dwMsgBoxWidth;  //Message box's width in pixel.
	DWORD        dwMsgBoxHeight; //Message box's height in pixel.
};

//Return values for MessageBox routine.
#define ID_OK      0x00000001
#define ID_CANCEL  0x00000002
#define ID_YES     0x00000003
#define ID_NO      0x00000004

//The space occupied by one button.
#define MSGBOX_BUTTON_WIDTH 80
//The space between 2 buttons.
#define MSGBOX_BUTTON_SPACE 40
//The height of a button in message box.
#define MSGBOX_BUTTON_HEIGHT 25

