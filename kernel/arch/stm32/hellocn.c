//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,27 2004
//    Module Name               : hellocn.cpp
//    Module Funciton           : 
//                                This module countains the source code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "..\..\INCLUDE\StdAfx.h"
#endif

#include "..\..\include\console.h"

#ifdef __I386__  //Only available in x86 based PC platform.

INT_HANDLER SetGeneralIntHandler(__GENERAL_INTERRUPT_HANDLER TimerHandler)
{
	__asm{
		push ebx
		push ecx
		mov ebx,__TIMERHANDLER_BASE
		mov eax,TimerHandler
		mov ecx,dword ptr [ebx]
		mov dword ptr [ebx],eax
		mov eax,ecx
		pop ecx
		pop ebx
	}
}


__declspec(naked) VOID WriteByteToPort(UCHAR byte,WORD wPort)
{
	__asm{
		push ebp
		mov ebp,esp
		push edx
		mov al,byte ptr [ebp + 8]
		mov dx,word ptr [ebp + 12]
		out dx,al
		pop edx
		leave
		retn
	}
}

__declspec(naked) VOID ReadByteStringFromPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{
	__asm{
		push ebp
		mov ebp,esp
		//push ebx
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 8]
		//mov esi,dword ptr [ebx]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep insb
		pop edi
		pop edx
		pop ecx
		//pop ebx
		leave
		retn
	}
}

__declspec(naked) VOID WriteByteStringToPort(LPVOID lpBuffer,DWORD dwBufLen,WORD wPort)
{
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 8]
		mov ecx,dword ptr [ebp + 12]
		mov dx,word ptr [ebp + 16]
		rep outsb
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
}

__declspec(naked) VOID ReadWordFromPort(WORD* pWord,WORD wPort)
{
	__asm{
		push ebp
		mov ebp,esp
		push ebx
		push edx
		mov dx,word ptr [ebp + 0x0c]
		mov ebx,dword ptr [ebp + 0x08]
		in ax,dx
		mov word ptr [ebx],ax
		pop edx
		pop ebx
		leave
		retn
	}
}

__declspec(naked) VOID WriteWordToPort(WORD w1,WORD w2)
{
	__asm{
		push ebp
		mov ebp,esp
		push dx
		mov dx,word ptr [ebp + 0x0c]
		mov ax,word ptr [ebp + 0x08]
		out dx,ax
		pop dx
		leave
		retn
	}
}

__declspec(naked) VOID ReadWordStringFromPort(LPVOID p1,DWORD d1,WORD w1)
{
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push edi
		mov edi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x01
		mov dx,  word ptr [ebp + 0x10]
		cld
		rep insw
		pop edi
		pop edx
		pop ecx
		leave
		retn
	}
}

__declspec(naked) VOID WriteWordStringToPort(LPVOID p1,DWORD d1,WORD w1)
{
	__asm{
		push ebp
		mov ebp,esp
		push ecx
		push edx
		push esi
		mov esi,dword ptr [ebp + 0x08]
		mov ecx,dword ptr [ebp + 0x0c]
		shr ecx,0x02
		mov dx,  word ptr [ebp + 0x10]
		rep outsw
		pop esi
		pop edx
		pop ecx
		leave
		retn
	}
}

#endif //__I386__

//
//Error handling routines implementing.
//
static VOID FatalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : FATAL(1)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID CriticalErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : CRITICAL(2)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID ImportantErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : IMPORTANT(3)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID AlarmErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : ALARM(4)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

static VOID InformErrorHandler(DWORD dwReason,LPSTR lpszMsg)
{
	PrintLine("Error Level : INFORM(5)");
	if(lpszMsg != NULL)
		PrintLine(lpszMsg);
	return;
}

VOID ErrorHandler(DWORD dwLevel,DWORD dwReason,LPSTR lpszMsg)
{
	switch(dwLevel)
	{
	case ERROR_LEVEL_FATAL:
		FatalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_CRITICAL:
		CriticalErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_IMPORTANT:
		ImportantErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_ALARM:
		AlarmErrorHandler(dwReason,lpszMsg);
		break;
	case ERROR_LEVEL_INFORM:
		InformErrorHandler(dwReason,lpszMsg);
		break;
	default:
		break;
	}
}

//
//The following routine prints out bug's information.
//
VOID __BUG(LPSTR lpszFileName,DWORD dwLineNum)
{
	CHAR    strBuff[12];
	DWORD   dwFlags;

	PrintLine("BUG oencountered.");
	PrintStr("File name: ");
	PrintStr(lpszFileName);
	Hex2Str(dwLineNum,strBuff);
	PrintLine("Lines: ");
	PrintStr(strBuff);

	//Enter infinite loop.
	__ENTER_CRITICAL_SECTION(NULL,dwFlags);
	while(TRUE);
	//__LEAVE_CRITICAL_SECTION(NULL,dwFlags);
}

