//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct 11 2014
//    Module Name               : BIOSVGA.c
//    Module Funciton           : 
//    Lines number              :
//***********************************************************************/


#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "chardisplay.h"
#include "biosvga.h"
#include <stdio.h>

#define  BIOS_VGABUF_ADDR    0xB8000
#define  BIOS_VGABUF_LEN     4000

#define  BIOS_VGABUF_COLUMS  80
#define  BIOS_VGABUF_LINES   25

//This module will be available if and only if DDF is enabled.
#ifdef __CFG_SYS_DDF

typedef struct tag__VGA_DISPLAY_INFO
{
	BYTE*   pVideoAddr;

	WORD    Lines;	
	WORD    Colums;
	WORD    LastLine;

	WORD    CursorX;
	WORD    CursorY;
	
}VGA_DISPLAY_INFO;

static VGA_DISPLAY_INFO    s_szVgaInfo   = {0};

WORD* _VGA_GetDisplayAddr(WORD nX,WORD nY)
{
	INT    nAddrOffset = 0;

	if(nX >= s_szVgaInfo.Colums || nY >= s_szVgaInfo.Lines)
	{
		return NULL;
	}

	nAddrOffset = (nY*s_szVgaInfo.Colums+nX)*2;// һ���ַ�ռ���ֽ�

	return (WORD*)(s_szVgaInfo.pVideoAddr+nAddrOffset);
}

VOID  _VGA_ScrollLine(BOOL bScrollUp)
{
	WORD* pBaseAddr  = _VGA_GetDisplayAddr(0,0);
	WORD* pNextLine  = pBaseAddr+s_szVgaInfo.Colums;
	WORD* pEndLine   = pBaseAddr+(s_szVgaInfo.LastLine-1)*s_szVgaInfo.Colums;

	if(bScrollUp)
	{
		INT nMoveLen = ((s_szVgaInfo.LastLine-1)*s_szVgaInfo.Colums)*2;
		INT i        = 0;

		memcpy(pBaseAddr,pNextLine,nMoveLen);
		for(i=0;i<s_szVgaInfo.Colums;i++)
		{
			pEndLine[i] = 0x700;
		}		
	}
	else
	{
	}
}

DWORD VGA_GetDisplayID()
{
	return DISPLAY_BIOSVGA;
}

BOOL  VGA_SetCursorPos(WORD CursorX,WORD CursorY)
{
	#define DEF_INDEX_PORT 0x3d4 //;;The index port of the display card.
	#define DEF_VALUE_PORT 0x3d5 //;;The data port of the display card.
	
	WORD  cursor_index  = 0;
	BYTE  cursor_h      = 0;
	BYTE  cursor_l      = 0;

	if(CursorY >= s_szVgaInfo.LastLine)
	{		
		_VGA_ScrollLine(TRUE);

		s_szVgaInfo.CursorX = 0;
		s_szVgaInfo.CursorY = s_szVgaInfo.LastLine-1;				
	}
	else
	{
		s_szVgaInfo.CursorX = CursorX;
		s_szVgaInfo.CursorY = CursorY;
	}

	cursor_index   = s_szVgaInfo.CursorY*BIOS_VGABUF_COLUMS+s_szVgaInfo.CursorX;
	cursor_h       = (cursor_index>>8)&0xFF;
	cursor_l       = cursor_index&0xFF;

#ifdef __GCC__
	__asm__ volatile (
	"movw %0,	%%dx                              \n\t"
	"movb $14,	%%al                              \n\t"
	"outb %%al,	%%dx                              \n\t"
	"movw %1,	%%dx                              \n\t"
	"movb %2,	%%al                              \n\t"
	"outb %%al,	%%dx                              \n\t"
	"movw %3,	%%dx                              \n\t"
	"movb $15,	%%al                              \n\t"
	"outb %%al,	%%dx                              \n\t"
	"movw %4,	%%dx                              \n\t"
	"movb %5,	%%al                              \n\t"
	"outb %%al,	%%dx                              \n\t"
	:
	:"i"(DEF_INDEX_PORT),"i"(DEF_VALUE_PORT),"r"(cursor_h),
	 "i"(DEF_INDEX_PORT),"i"(DEF_VALUE_PORT),"i"(DEF_VALUE_PORT),
	 "r"(cursor_l)
	 :
	);
#else
	__asm{
			mov dx,DEF_INDEX_PORT
			mov al,14
			out dx,al

			mov dx,DEF_VALUE_PORT
			mov al,cursor_h
			out dx,al

			mov dx,DEF_INDEX_PORT
			mov al,15
			out dx,al

			mov dx,DEF_VALUE_PORT
			mov al,cursor_l
			out dx,al
	}
#endif
	
	return TRUE;
}

BOOL VGA_ChangeLine()
{

	if(s_szVgaInfo.CursorY < s_szVgaInfo.Lines)
	{
		VGA_SetCursorPos(0,s_szVgaInfo.CursorY+1);
	}

    return TRUE;
}

BOOL  VGA_GetDisplayRange(INT* pLines,INT* pColums)
{
	if(NULL != pLines)
	{
		*pLines  = s_szVgaInfo.Lines;
	}

	if(NULL != pColums)
	{
		*pColums = s_szVgaInfo.Colums;
	}

	return TRUE;
}

BOOL  VGA_PrintChar(CHAR ch)
{
	WORD   CursorX     = s_szVgaInfo.CursorX;
	WORD   CursorY     = s_szVgaInfo.CursorY;
	WORD*  pVideoBuf   = _VGA_GetDisplayAddr(CursorX,CursorY);

	if(NULL == pVideoBuf || (DWORD)pVideoBuf >= BIOS_VGABUF_ADDR+BIOS_VGABUF_LEN)
	{
		return FALSE;
	}

	*pVideoBuf =  ch|0x700;
	CursorX ++;

	if(CursorX >= s_szVgaInfo.Colums)
	{
		CursorX = 0;
		CursorY ++;
	}
	VGA_SetCursorPos(CursorX,CursorY);
	
	return TRUE;
}

BOOL VGA_PrintString(LPCSTR pString,BOOL cl)
{
	WORD*  pVideoBuf   = _VGA_GetDisplayAddr(s_szVgaInfo.CursorX,s_szVgaInfo.CursorY);
	LPSTR  pos         = (LPSTR)pString;
	WORD   CursorX     = s_szVgaInfo.CursorX;
    WORD   CursorY     = s_szVgaInfo.CursorY;
	

	if(NULL == pos || NULL == pVideoBuf)
	{
		return FALSE;
	}

	while(*pos != 0)
	{
		WORD  wch   = 0x700;
	
		wch += (BYTE)*pos;
		*pVideoBuf =  wch;
		
		pVideoBuf ++;
		pos       ++;

		//�Ƿ�����
		CursorX   ++;
		if(CursorX >= s_szVgaInfo.Colums)
		{
			CursorX = 0; 
			CursorY ++;

			if(CursorY >= s_szVgaInfo.LastLine)
			{
				CursorY = s_szVgaInfo.LastLine-1;				
				_VGA_ScrollLine(TRUE);

				pVideoBuf -= s_szVgaInfo.Colums;
			}
		}

		if((DWORD)pVideoBuf >= BIOS_VGABUF_ADDR+BIOS_VGABUF_LEN)
		{
			break;
		}

	}

	VGA_SetCursorPos(CursorX,CursorY);

	if(cl == TRUE)
	{
		VGA_ChangeLine();
	}

	return TRUE;
}

BOOL   VGA_GetCursorPos(WORD* pCursorX,WORD* pCursorY)
{
	if(NULL != pCursorX)
	{
		*pCursorX  = s_szVgaInfo.CursorX;
	}

	if(NULL != pCursorY)
	{
		*pCursorY = s_szVgaInfo.CursorY;
	}

	return TRUE;
}

BOOL VGA_GetString(WORD CursorX,WORD CursorY,LPSTR pString,INT nBufLen)
{	
	WORD*  pVideoBuf    = _VGA_GetDisplayAddr(CursorX,CursorY);
	WORD   InputEnd     = s_szVgaInfo.CursorY*BIOS_VGABUF_COLUMS+s_szVgaInfo.CursorX;
	WORD   StartIndex   = CursorY*BIOS_VGABUF_COLUMS+CursorX;
	INT    i            = 0;

	for(i=0;i<nBufLen;i++)
	{		
		BYTE  ch   = (BYTE)(*pVideoBuf&0x7F);

		if(ch < 0x20 || ch >  0x7E /*|| ch == 0x5F*/)
		{
			break;
		}
		pString[i] = (CHAR)ch;

		pVideoBuf    ++;
		StartIndex   ++ ;

		if(StartIndex >= InputEnd)
		{
			break;
		}

		if((DWORD)pVideoBuf >= BIOS_VGABUF_ADDR+BIOS_VGABUF_LEN)
		{
			break;
		}
	}

	return TRUE;
}

BOOL  VGA_DelString(WORD CursorX,WORD CursorY,INT nDelLen)
{
	WORD*  pVideoBuf   = _VGA_GetDisplayAddr(CursorX,CursorY);
	INT    i           = 0;

	if(NULL == pVideoBuf)
	{
		return FALSE;
	}

	for(i=0;i<nDelLen;i++)
	{
		*pVideoBuf =  0x700;

		pVideoBuf ++;
		if((DWORD)pVideoBuf >= BIOS_VGABUF_ADDR+BIOS_VGABUF_LEN)
		{
			break;
		}
	}

	return TRUE;
}

BOOL  VGA_DelChar(INT nDelMode)
{	
	WORD*  pVideoBuf   = NULL;
	INT    nCharDelX   = 0;
	INT    nCharDelY   = s_szVgaInfo.CursorY;


	if(nDelMode == DISPLAY_DELCHAR_PREV)
	{		
		if(s_szVgaInfo.CursorX == 0 && s_szVgaInfo.CursorY == 0)
		{
			return FALSE;
		}

		if(s_szVgaInfo.CursorX == 0)
		{
			nCharDelX = s_szVgaInfo.Colums-1;
			nCharDelY --;
		}
		else
		{
			nCharDelX = s_szVgaInfo.CursorX-1;
		}		
	}
	else if(nDelMode == DISPLAY_DELCHAR_CURR)
	{
		nCharDelX = s_szVgaInfo.CursorX;		
	}
	else
	{
		return FALSE;	
	}

	//�ַ�ǰ��
	pVideoBuf = _VGA_GetDisplayAddr(nCharDelX,nCharDelY);
	while(TRUE)
	{
		*pVideoBuf = *(pVideoBuf+1);
		pVideoBuf ++;

		if((DWORD)pVideoBuf >= BIOS_VGABUF_ADDR+BIOS_VGABUF_LEN)
		{
			break;
		}
	}

	//�������α�λ��
	VGA_SetCursorPos(nCharDelX,nCharDelY);	
		
	return TRUE;
}

BOOL  VGA_Clear()
{
	WORD*  pVideoBuf   = (WORD*)(s_szVgaInfo.pVideoAddr);
	INT    i           = 0;

	for(i=0;i<BIOS_VGABUF_LEN/2;i++)
	{
		pVideoBuf[i] = 0x700;
	}

	return TRUE;
}

BOOL InitializeVGA(void)
{
	s_szVgaInfo.pVideoAddr = (BYTE*)BIOS_VGABUF_ADDR;
	s_szVgaInfo.Lines      = BIOS_VGABUF_LINES;
	s_szVgaInfo.Colums     = BIOS_VGABUF_COLUMS;
	s_szVgaInfo.LastLine   = s_szVgaInfo.Lines;
	return TRUE;
}

#endif  //__CFG_SYS_DDF
