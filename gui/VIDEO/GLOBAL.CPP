//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April,11 2009
//    Module Name               : GLOBAL.CPP
//    Module Funciton           : 
//                                Global parameter management object is implemented
//                                in this module,it's operating interface also
//                                included in this module.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __KAPI_H__
#include "..\INCLUDE\KAPI.H"
#endif

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif

#ifndef __GLOBAL_H__
#include "..\INCLUDE\GLOBAL.H"
#endif

//The initialize routine of global parameter manager object.
static BOOL Initialize(__GUI_GLOBAL_PARAM* pGlobalParam,__VIDEO* pVideo)
{
	//pGlobalParam->dwWndBorderWidth   = pVideo->dwScreenHeight / 150;
	pGlobalParam->dwWndBorderWidth   = 1;
	pGlobalParam->dwWndCaptionHeight = pVideo->dwScreenHeight / 30;
	pGlobalParam->dwWndDefCx         = pVideo->dwScreenWidth  / 3;
	pGlobalParam->dwWndDefCy         = pVideo->dwScreenHeight / 3;
	pGlobalParam->dwWndMenuHeight    = pVideo->dwScreenHeight / 30;
	pGlobalParam->dwWndStatusBarHeight = pVideo->dwScreenHeight / 30;
	pGlobalParam->dwWndToolBarHeight   = pVideo->dwScreenHeight / 30;
	pGlobalParam->SCROLLBAR_WIDTH      = pVideo->dwScreenHeight / 30;
	pGlobalParam->BUTTON_HEIGHT        = pVideo->dwScreenHeight / 30;
	pGlobalParam->EDIT_HEIGHT          = pVideo->dwScreenHeight / 30;
	pGlobalParam->LABEL_HEIGHT         = pVideo->dwScreenHeight / 30;
	
	//Initialize GUI kernel thread handles.
	pGlobalParam->hGUIShell          = NULL;
	pGlobalParam->hRawInputThread    = NULL;

	return TRUE;
}

//The definition of Global Parameter Manager object.
__GUI_GLOBAL_PARAM GlobalParams = {
	0xC00000,
	0xC0C0C0,
	0x008080,
	0x000080,
	0x808080,
	0xC0C0C0,
	0xFFFFFF,
	0x000000,
	0xC0C0C0,
	0x000000,
	0xFFFFFF,
	0xC0C0C0,
	0xC0C0C0,
	0x808080,
	0x000080,
	0xFFFFFF,
	0xC0C0C0,
	0x808080,
	0x808080,
	0x000000,
	0xC0C0C0,
	0xFFFFFF,
	0x000000,
	0xC0C0C0,
	0x000000,
	0xFFFFFF,
	0x0000FF,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	NULL,         //hGUIShell.
	NULL,         //hRawInputThread.
	Initialize,
};
