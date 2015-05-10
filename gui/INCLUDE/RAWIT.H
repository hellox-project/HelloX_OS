//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : RAWIT.H
//    Module Funciton           : 
//                                RAW input thread is one of the kernel threads
//                                in GUI module,it responses to process sytem level
//                                messages and dispatch message to proper user
//                                thread.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __RAWIT_H__
#define __RAWIT_H__
#endif

//RAW input thread entry point.
DWORD RAWIT(LPVOID);

//Mouse manager object.
struct __MOUSE_MANAGER{
	int     MouseMap[16][16];    //Current mouse map.
	__COLOR MouseRect[16][16];   //Static buffer used to contain the screen content behind
	                             //mouse pointer.
	VOID (*SaveMouseRect)(__MOUSE_MANAGER* pMouseMgr,
		                  __VIDEO*         pVideo,
						  int x,
						  int y);
	VOID (*DrawMouse)(__MOUSE_MANAGER* pMouseMgr,
		              __VIDEO*         pVideo,
					  int x,
					  int y);
	VOID (*RestoreMouseRect)(__MOUSE_MANAGER* pMouseMgr,
		                     __VIDEO*         pVideo,
							 int x,
							 int y);
	VOID (*ReplaceMouseMap)(__MOUSE_MANAGER* pMouseMgr,
		                    int NewMouseMap);
};

//There is one global mouse manager object in GUI module.
extern __MOUSE_MANAGER MouseManager;


