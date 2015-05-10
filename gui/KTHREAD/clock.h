//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 04,2012
//    Module Name               : clock.h
//    Module Funciton           : 
//                                Clock component's definition code.Clock component
//                                is a control residing in shell's control band,in screen's
//                                right side.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __CLOCK_H__
#define __CLOCK_H__

//Clock face's color.
#define CLK_FACE_COLOR   0x00FFFFFF  //White
//Clock scale and pointers color.
#define CLK_SCALE_COLOR  0x0000C0FF  //Same as task band.

//Clock window's window procedure.
DWORD ClockWndProc(HANDLE hWnd,UINT message,WORD wParam,DWORD lParam);

#endif
