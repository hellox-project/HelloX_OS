//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Nov,02 2014
//    Module Name               : CharDisplay
//    Module Funciton           : 
//                                
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "console.h"
#include "chardisplay.h"
#include "dbm.h"

#include "../arch/x86/biosvga.h"

void PrintStr(const char* pszMsg)
{
	CD_PrintString((LPSTR)pszMsg,FALSE);
}

void ClearScreen()
{
	CD_Clear();
}

void PrintCh(unsigned short ch)
{
	CD_PrintChar((CHAR)ch);
}

VOID CD_GotoHome()
{
#ifdef __I386__
	WORD CursorX = 0;
	WORD CursorY = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	CD_SetCursorPos(0,CursorY);
#endif 

#ifdef __CFG_SYS_CONSOLE
	Console.GotoHome();
#endif
}

VOID CD_GotoPrev()
{
#ifdef __I386__

	WORD CursorX = 0;
	WORD CursorY = 0;

	CD_GetCursorPos(&CursorX,&CursorY);
	if(CursorX > 0 )
		{
		CD_SetCursorPos(CursorX-1,CursorY);
		}

#endif

#ifdef __CFG_SYS_CONSOLE
	Console.GotoPrev();
#endif
}

void GotoHome()
{
	CD_GotoHome();
}
void ChangeLine()
{
	CD_ChangeLine();
}

VOID GotoPrev()
{
	CD_GotoPrev();
}

/* Initializes the display devices. */
VOID CD_InitDisplay(INT nDisplayMode)
{	
	return ;
}

/* Set display mode. */
VOID CD_SetDisplayMode(INT nMode)
{
	return ;
}

/* Get current dimension of display. */
VOID CD_GetDisPlayRang(WORD* pLines,WORD* pColums)
{
#ifdef __I386__
	VGA_GetDisplayRange((INT*)pLines,(INT*)pColums);
#endif	
}

/* Get cursor's current position. */
VOID  CD_GetCursorPos(WORD* pCursorX,WORD* pCursorY)
{
#ifdef __I386__
	VGA_GetCursorPos(pCursorX,pCursorY);
#endif 
}

/* Change to a new line. */
VOID CD_ChangeLine()
{
#ifdef __I386__
	VGA_ChangeLine();
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.GotoHome();
	Console.ChangeLine();
#endif
	
	DispBufferManager.PutChar('\r');
	DispBufferManager.PutChar('\n');
}

/* Set cursor's current position. */
VOID  CD_SetCursorPos(WORD nCursorX,WORD nCursorY)
{
	#ifdef __I386__
		VGA_SetCursorPos(nCursorX,nCursorY);
	#endif
}

/* Show out a string, change line if cl is TRUE. */
VOID CD_PrintString(LPSTR pStr,BOOL cl)
{
#ifdef __I386__
	VGA_PrintString(pStr,FALSE);
	if(cl)
	{
		CD_GotoHome();
		CD_ChangeLine();
	}
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.PrintStr(pStr);
	if(cl == TRUE)
	{
		CD_ChangeLine();
	}
#endif	

	DispBufferManager.PutString(pStr, strlen(pStr));
	if (cl)
	{
		DispBufferManager.PutChar('\r');
		DispBufferManager.PutChar('\n');
	}
}

/* Show a char on screen or terminal. */
VOID CD_PrintChar(CHAR ch)
{
#ifdef __I386__
		VGA_PrintChar(ch);
#endif

#ifdef __CFG_SYS_CONSOLE
		Console.PrintCh(ch);
#endif
		DispBufferManager.PutChar(ch);
}

/* Get string from the given position. */
VOID  CD_GetString(WORD nCursorX,WORD nCursorY,LPSTR pString,INT nBufLen)
{
#ifdef __I386__
	VGA_GetString(nCursorX,nCursorY,pString,nBufLen);	
#endif
}

/* Get the whole screen's content. */
VOID  CD_GetScreen(LPSTR pBuf,INT nBufLen)
{
#ifdef __I386__
	VGA_GetScreen(pBuf,nBufLen);	
#endif
}

/* Delete string from given position. */
VOID  CD_DelString(WORD nCursorX,WORD nCursorY,INT nDelLen)
{
#ifdef __I386__
	VGA_DelString(nCursorX,nCursorY,nDelLen);
#endif
}

/* Delete char from current position. */
VOID  CD_DelChar(INT nDelMode)
{
#ifdef __I386__
	VGA_DelChar(nDelMode);
#endif
}

/* Clear the whole screen. */
VOID  CD_Clear()
{
#ifdef __I386__
	VGA_Clear();
#endif

#ifdef __CFG_SYS_CONSOLE
	Console.ClearScreen();
#endif
}

/* Set current displaying attributes. */
VOID  CD_SetCharAttr(BYTE cAttr)
{
	#ifdef __I386__
		VGA_SetCharAttr(cAttr);
	#endif
}
