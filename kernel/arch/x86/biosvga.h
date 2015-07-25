//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct 11 2014
//    Module Name               : BIOSVGA.H
//    Module Funciton           : 
//    Lines number              :
//***********************************************************************/

#ifndef __BIOSVGA_H__
#define __BIOSVGA_H__

#define  DISPLAY_BIOSVGA             0x1001

//显示模式
#define  DISPLAY_MODE_DEFAULT        0      //默认显示模式 80*25

//Initialize VGA devices,this shoud be done before any screen output.
BOOL InitializeVGA(void);

//VGA operating functions.
DWORD VGA_GetDisplayID();
BOOL  VGA_SetCursorPos(WORD CursorX,WORD CursorY);
BOOL  VGA_ChangeLine();
BOOL  VGA_GetDisplayRange(INT* pLines,INT* pColums);
BOOL  VGA_PrintChar(CHAR ch);
BOOL  VGA_PrintString(LPCSTR pString,BOOL cl);
BOOL  VGA_GetCursorPos(WORD* pCursorX,WORD* pCursorY);
BOOL  VGA_GetString(WORD CursorX,WORD CursorY,LPSTR pString,INT nBufLen);
BOOL  VGA_DelString(WORD CursorX,WORD CursorY,INT nDelLen);
BOOL  VGA_DelChar(INT nDelMode);
BOOL  VGA_Clear(void);

#endif  //__BIOSVGA_H__
