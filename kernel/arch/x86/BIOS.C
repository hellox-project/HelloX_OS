//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,01 2011
//    Module Name               : BIOS.CPP
//    Module Funciton           : 
//                                BIOS service entries are implemented here.
//
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

#ifndef __BIOS_H__
#include "BIOS.H"
#endif

#include "..\..\INCLUDE\KAPI.H"
#include "..\..\lib\stdio.h"

#ifdef __I386__  //Only available in x86 based PC platform.

//Read HD sector's entry point.
BOOL BIOSReadSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer)
{
	DWORD i;

	if((NULL == pBuffer) || (0 == nSectorNum))
	{
		return FALSE;
	}

	__asm{
		push ecx
		push edx
		push esi
		push ebx
		mov eax,BIOS_SERVICE_READSECTOR
		mov ecx,nHdNum
		mov edx,nStartSector
		mov esi,nSectorNum
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
		pop esi
		pop edx
		pop ecx
	}

    __asm{
	    cmp eax,0x00
	    jz __BIOS_FAILED
	    jmp __BIOS_SUCCESS
	}

__BIOS_FAILED:    //BIOS call failed.
	return FALSE;

__BIOS_SUCCESS:   //BIOS call successed.
	for(i = 0;i < 512 * nSectorNum;i ++) //Copy the sector data.
	{
		pBuffer[i] = ((BYTE*)BIOS_HD_BUFFER)[i];
	}
	return TRUE;
}

//Reboot the system.
VOID BIOSReboot()
{
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_REBOOT
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
}

//Power off the system.
VOID BIOSPoweroff()
{
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_POWEROFF
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
}

//Switch current display mode to graphic.
BOOL SwitchToGraphic()
{
		__asm{
		push ebx
		mov eax,BIOS_SERVICE_TOGRAPHIC
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
}

//Switch display mode to text.
VOID SwitchToText()
{
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_TOTEXT
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
}

#endif  //__I386__
