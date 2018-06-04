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

/* Read data from HD,can not exceed 4k length. */
static BOOL __BIOSReadSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer)
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
		pBuffer[i] = ((BYTE*)BIOS_HD_BUFFER_START)[i];
	}
	return TRUE;
}

/* Read data from hard disk,can support any length. */
BOOL BIOSReadSector(int nHdNum, DWORD nStartSector, DWORD nSectorNum, BYTE* pBuffer)
{
	unsigned long total_length = nSectorNum * BIOS_HD_SECTOR_SIZE;
	unsigned long total_sector = nSectorNum;
	unsigned long start_sector = nStartSector;
	BOOL bResult = FALSE;
	char* local_buff = pBuffer;

	/* At least BIOS_HD_BUFFER_LENGTH's data can be retrived in one batch. */
	while (total_length > BIOS_HD_BUFFER_LENGTH)
	{
		bResult = __BIOSReadSector(nHdNum, start_sector, BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE, local_buff);
		if (!bResult)
		{
			goto __TERMINAL;
		}
		total_length -= BIOS_HD_BUFFER_LENGTH;
		start_sector += BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE;
		total_sector -= BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE;
		local_buff += BIOS_HD_BUFFER_LENGTH;
	}

	/* Read the remaining data. */
	bResult = __BIOSReadSector(nHdNum, start_sector, total_sector, local_buff);

__TERMINAL:
	return bResult;
}

//Write HD sector's entry point.
static BOOL __BIOSWriteSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer)
{
	DWORD i;

	if((NULL == pBuffer) || (0 == nSectorNum))
	{
		return FALSE;
	}

	//Copy the buf data to  sector buf
	for(i = 0;i < 512 * nSectorNum;i ++) 
	{
		((BYTE*)BIOS_HD_BUFFER_START)[i] = pBuffer[i];
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
	_hx_printf("%s:failed to write sector.\r\n", __func__);
	return FALSE;

__BIOS_SUCCESS:   //BIOS call successed.
	return TRUE;
}

/* Write data from hard disk,can support any length. */
BOOL BIOSWriteSector(int nHdNum, DWORD nStartSector, DWORD nSectorNum, BYTE* pBuffer)
{
	unsigned long total_length = nSectorNum * BIOS_HD_SECTOR_SIZE;
	unsigned long total_sector = nSectorNum;
	unsigned long start_sector = nStartSector;
	BOOL bResult = FALSE;
	char* local_buff = pBuffer;

	/* At least BIOS_HD_BUFFER_LENGTH's data can be written in one batch. */
	while (total_length > BIOS_HD_BUFFER_LENGTH)
	{
		bResult = __BIOSWriteSector(nHdNum, start_sector, BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE, local_buff);
		if (!bResult)
		{
			goto __TERMINAL;
		}
		total_length -= BIOS_HD_BUFFER_LENGTH;
		start_sector += BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE;
		total_sector -= BIOS_HD_BUFFER_LENGTH / BIOS_HD_SECTOR_SIZE;
		local_buff += BIOS_HD_BUFFER_LENGTH;
	}

	/* Write the remaining data. */
	bResult = __BIOSWriteSector(nHdNum, start_sector, total_sector, local_buff);

__TERMINAL:
	return bResult;
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
	//Check if current display mode is GRAPHIC mode.
	if(!IN_GRAPHIC_DISPLAY_MODE())
	{
		//Not in graphic mode,just return.
		return;
	}
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
