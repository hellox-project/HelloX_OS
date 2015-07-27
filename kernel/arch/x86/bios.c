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
#include "StdAfx.h"
#endif

#ifndef __BIOS_H__
#include "bios.h"
#endif

#include "kapi.h"
#include "stdio.h"

#ifdef __I386__  //Only available in x86 based PC platform.

//Read HD sector's entry point.
BOOL BIOSReadSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer)
{
	DWORD i;

	if((NULL == pBuffer) || (0 == nSectorNum))
	{
		return FALSE;
	}

#ifdef __GCC__
	__asm__ (
			".code32		\n\t"
			"pushl %%ecx                        \n\t"
			"pushl %%edx                        \n\t"
			"pushl %%esi                        \n\t"
			"pushl %%ebx                        \n\t"
			"movl %0,	%%eax    				\n\t"
			"movl %1,	%%ecx           		\n\t"
			"movl %2,	%%edx           		\n\t"
			"movl %3,	%%esi           		\n\t"
			"movl %4,	%%ebx           		\n\t"
			"calll %%ebx				        \n\t"
			"popl %%ebx                         \n\t"
			"popl %%esi                         \n\t"
			"popl %%edx                         \n\t"
			"popl %%ecx                         \n\t"
			:
			:"r"(BIOS_SERVICE_READSECTOR),"r"(nHdNum),
			 "r"(nStartSector),"r"(nSectorNum),"r"(BIOS_ENTRY_ADDR)
			);

	__asm__ goto (
	    "cmpl $0x00,	%%eax   				\n\t"
	    "jz  %l[__BIOS_FAILED]                 	\n\t"
	    "jmp %l[__BIOS_SUCCESS] 				\n\t"
	    :
		:
		:
		:__BIOS_FAILED, __BIOS_SUCCESS
	);

#else
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
#endif

__BIOS_FAILED:    //BIOS call failed.
	return FALSE;

__BIOS_SUCCESS:   //BIOS call successed.
	for(i = 0;i < 512 * nSectorNum;i ++) //Copy the sector data.
	{
		pBuffer[i] = ((BYTE*)BIOS_HD_BUFFER)[i];
	}
	return TRUE;
}

//Write HD sector's entry point.
BOOL BIOSWriteSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer)
{
	DWORD i;

	if((NULL == pBuffer) || (0 == nSectorNum))
	{
		return FALSE;
	}

	//Copy the buf data to  sector buf
	for(i = 0;i < 512 * nSectorNum;i ++) 
	{
		((BYTE*)BIOS_HD_BUFFER)[i] = pBuffer[i];
	}
#ifdef __GCC__
	__asm__ (
	"pushl %%ecx              \n\t"
	"pushl %%edx              \n\t"
	"pushl %%esi              \n\t"
	"pushl %%ebx              \n\t"
	"movl %0, 	%%eax     	  \n\t"
	"movl %1,	%%ecx         \n\t"
	"movl %2,	%%edx         \n\t"
	"movl %3,	%%esi         \n\t"
	"movl %4,	%%ebx         \n\t"
	"call %%ebx          	  \n\t"
	"popl %%ebx               \n\t"
	"popl %%esi               \n\t"
	"popl %%edx               \n\t"
	"popl %%ecx               \n\t"
	:
	:"r"(BIOS_SERVICE_WRITESECTOR), "r"(nHdNum), "r"(nStartSector),
	 "r"(nSectorNum),"r"(BIOS_ENTRY_ADDR)
	);


	__asm__ goto (
			"cmpl 	$0x00, %%eax		\n\t"
			"jz		%l[__BIOS_FAILED]	\n\t"
			"jmp	%l[__BIOS_SUCCESS]	\n\t"
			:::
			:__BIOS_FAILED,__BIOS_SUCCESS
			);

#else
	__asm{
			push ecx
			push edx
			push esi
			push ebx
			mov eax,BIOS_SERVICE_WRITESECTOR
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
#endif

__BIOS_FAILED:    //BIOS call failed.
	return FALSE;

__BIOS_SUCCESS:   //BIOS call successed.

	return TRUE;
}

//Reboot the system.
VOID BIOSReboot()
{
#ifdef __GCC__
	__asm__(
		"pushl	%%ebx	;"
		"movl	%0,	%%eax	;"
		"movl	%1,	%%ebx	;"
		"calll	%%ebx		;"
		"popl	%%ebx		;"
		:
		:"r"(BIOS_SERVICE_REBOOT), "r"(BIOS_ENTRY_ADDR)
	);

#else
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_REBOOT
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
#endif
}

//Power off the system.
VOID BIOSPoweroff()
{
#ifdef __GCC__
	__asm__(
		"pushl	%%ebx		\n\t"
		"movl	%0,	%%eax	\n\t"
		"movl	%1,	%%ebx	\n\t"
		"calll	%%ebx		\n\t"
		"popl	%%ebx		\n\t"
			:
			:"r"(BIOS_SERVICE_POWEROFF),"r"(BIOS_ENTRY_ADDR)
	);

#else
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_POWEROFF
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
#endif
}

//Switch current display mode to graphic.
BOOL SwitchToGraphic()
{
#ifdef __GCC__
	__asm__(
		"pushl %%eax	\n\t"
		"movl	%0,	%%eax	\n\t"
		"movl	%1,	%%ebx	\n\t"
		"calll	%%ebx		\n\t"
		"popl	%%ebx		\n\t"
			::"r"(BIOS_SERVICE_TOGRAPHIC),"r"(BIOS_ENTRY_ADDR)
	);

#else
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_TOGRAPHIC
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
#endif
}

//Switch display mode to text.
VOID SwitchToText()
{
#ifdef __GCC__
	__asm__(
			"pushl	%%ebx	\n\t"
			"movl	%0, %%eax	\n\t"
			"movl	%1,	%%ebx	\n\t"
			"calll	%%ebx		\n\t"
			"popl	%%ebx		\n\t"
			:
			:"r"(BIOS_SERVICE_TOTEXT),"r"(BIOS_ENTRY_ADDR)
	);

#else
	__asm{
		push ebx
		mov eax,BIOS_SERVICE_TOTEXT
		mov ebx,BIOS_ENTRY_ADDR
		call ebx  //For debug...
		pop ebx
	}
#endif
}

#endif  //__I386__
