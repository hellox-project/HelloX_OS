//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct,13 2014
//    Module Name               : chardisplay.h
//    Module Funciton           : char display api
//                                
//    Last modified Author      : tywind
//    Last modified Date        : oct,11 2014
//***********************************************************************/

#ifndef __CHARDISPLAY_H__
#define __CHARDISPLAY_H__

#ifndef __TYPES_H__
#include "TYPES.H"
#endif

#ifndef __CONFIG_H__
#include "..\config\config.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

//删除模式
#define  DISPLAY_DELCHAR_PREV        0      //删除光标前面一个字符
#define  DISPLAY_DELCHAR_CURR        1      //删除光标当前位置一个字符

//初始化显示设备
VOID CD_InitDisplay(INT nDisplayMode);

//设置显示模式
VOID CD_SetDisplayMode(INT nMode);

//得到行和列
VOID CD_GetDisPlayRang(WORD* pLines,WORD* pColums);

//换行
VOID CD_ChangeLine();

//得到当前光标位置
VOID  CD_GetCursorPos(WORD* pCursorX,WORD* pCursorY);

//设置当前光标位置
VOID  CD_SetCursorPos(WORD nCursorX,WORD nCursorY);

//打印字符串,cl表示是否换行
VOID CD_PrintString(LPSTR pStr,BOOL cl);

//打印一个字符
VOID CD_PrintChar(CHAR ch);

//从指定位置得到字符串
VOID  CD_GetString(WORD nCursorX,WORD nCursorY,LPSTR pString,INT nBufLen);

//删除字符串
VOID  CD_DelString(WORD nCursorX,WORD nCursorY,INT nDelLen);	

//删除字符
VOID  CD_DelChar(INT nDelMode);	

//清屏
VOID  CD_Clear();

#ifdef __cplusplus
}
#endif

#endif    //cd.h
