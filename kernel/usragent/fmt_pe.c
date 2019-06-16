//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 12, 2019
//    Module Name               : fmt_pe.c
//    Module Funciton           : 
//                                Source code of user agent to process PE
//                                module format.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "fmt_pe.h"
/* 
 * Constants describe the layout of process's
 * memory space is in this file.
 */
#include "../include/mlayout.h"

/* Debugging. */
#define _hx_malloc(x) NULL
#define _hx_aligned_malloc(x, y) NULL
#define _hx_free(x)

/* Relocate code. */
static BOOL CodeReLocate(DWORD dwNewImageBase, DWORD dwRelocBlockVa, DWORD dwOldImageBase)
{
	PIMAGE_BASE_RELOCATION pRelocBlock = NULL;
	BYTE* pRunBuffer = (BYTE*)dwNewImageBase;
	WORD* pRelocData = NULL;
	int nRelocNum = 0, i = 0;

	pRelocBlock = (PIMAGE_BASE_RELOCATION)(pRunBuffer + dwRelocBlockVa);
	while (TRUE)
	{
		nRelocNum = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		pRelocData = (WORD*)((BYTE*)pRelocBlock + sizeof(IMAGE_BASE_RELOCATION));
		for (i = 0; i < nRelocNum; i++)
		{
			DWORD* pRelocAddress = NULL;
			WORD nPageOff = 0;

			if (((*pRelocData) >> 12) == IMAGE_REL_BASED_HIGHLOW)
			{
				nPageOff = (*pRelocData) & 0xFFF;
				pRelocAddress = (DWORD*)(dwNewImageBase + pRelocBlock->VirtualAddress + nPageOff);
				//if(nRelocNum <= 2)
				//{
				//	_hx_printf("VirtualAddress=0x%X,pRelocAddress= 0x%X\r\n",pRelocBlock->VirtualAddress,*pRelocAddress);
				//}
				*pRelocAddress = *pRelocAddress - dwOldImageBase + dwNewImageBase;
			}
			else
			{
				/*
				 * Show a warning and give up since we cann't process other relocation type,but
				 * the type of 0 is skiped since it's only the pad of relacation table,
				 * to make sure the whole size of the table is align with double word
				 * boundary.
				 */
				if ((*pRelocData) >> 12)
				{
					//_hx_printf("%s:Unknown relocate type[%d].\r\n", __FUNCTION__, (*pRelocData) >> 12);
					PrintLine("Unknown relocation type value.");
					return FALSE;
				}
			}
			pRelocData++;
		}
		// next block
		pRelocBlock = (PIMAGE_BASE_RELOCATION)((BYTE*)pRelocBlock + pRelocBlock->SizeOfBlock);
		//_hx_printf("Block= %X\r\n",(LPBYTE)pRelocBlock);
		if (NULL == (VOID*)pRelocBlock->VirtualAddress)
		{
			break;
		}
	}

	return TRUE;
}

BYTE* LoadAppToMemory_PE(HANDLE hFileObj)
{
	IMAGE_DOS_HEADER        ImageDosHeader = { 0 };
	IMAGE_NT_HEADERS        ImageNtHeader = { 0 };
	IMAGE_OPTIONAL_HEADER*  ImageOptionalHeader = NULL;
	IMAGE_SECTION_HEADER*   pSectionHdr = NULL;
	IMAGE_SECTION_HEADER*   pSectionHdrPos = NULL;
	/*
	 * The first several bytes of executable image will
	 * be replaced by the following header,which composed
	 * by three nop instructions and a jump instruct,the
	 * target of the jumping is the entry point of the
	 * image,wich is obtained from file header and is used
	 * to replace the all zero in the following structure.
	 */
	__FILL_HEADER           szFillHeader = { 0x90, 0x90, 0x90, 0xe9, 0x00000000 };
	__FILL_HEADER           szFillHeader2 = { 0x90, 0x90, 0x90, 0xe9, 0x00000000 };
	/*
	 * The buffer that contains the processed image,will be
	 * returned to the caller,if the loading and pre-processing
	 * are success.
	 * The caller just jump to the first byte of this buffer to
	 * launch the application.
	 */
	char*                   pRunBuffer = NULL;
	char*                   pJmpBuffer = NULL;
	BOOL                    bResult = FALSE;
	DWORD                   dwHxHeadSize = sizeof(__FILL_HEADER);
	DWORD                   dwSecTableSize = 0;
	DWORD                   dwReadSize = 0;
	DWORD                   dwOffset = 0;
	DWORD                   dwSectNum = 0;
	DWORD                   dwIndex = 0;
	/*
	 * A temporary buffer to hold the whole image file,to avoid
	 * multiple reading operations,since the SetFilePointer routine may has some
	 * issues to be solve.
	 * *********************************************************
	 * The issue is solved and the whole reading obsoleted.
	 * The issue is caused by FAT32 code,which leads a buffer overlapping
	 * by code mistake.
	 *
	 * char* pTmpBuff = NULL;
	 */

	DWORD dwImageSz = 0;

	/* Allocate a big buffer to hold the whole file. */
	dwImageSz = GetFileSize(hFileObj, NULL);
	if (0 == dwImageSz)
	{
		PrintLine("Can not get image file's size.");
		goto __TERMINAL;
	}

	/* Read DOS header from PE file. */
	dwOffset = 0;
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);
	if (!ReadFile(hFileObj, sizeof(ImageDosHeader), &ImageDosHeader, &dwReadSize))
	{
		goto __TERMINAL;
	}
	dwOffset = ImageDosHeader.e_lfanew;

	/* Load NT header. */
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);
	ReadFile(hFileObj, sizeof(ImageNtHeader), &ImageNtHeader, &dwReadSize);
	ImageOptionalHeader = &(ImageNtHeader.OptionalHeader);

	/* Image size must be equal or less than file size. */
	if (ImageOptionalHeader->SizeOfImage > dwImageSz)
	{
		PrintLine("Image size in header is too large.");
		goto __TERMINAL;
	}

	szFillHeader.dwJmpAddr = ImageOptionalHeader->AddressOfEntryPoint;
	szFillHeader.dwJmpAddr -= dwHxHeadSize;

	/**
	 * Aligned allocation must be used here,and the align value should be fetched
	 * from image file.But we assume it as 64 for simplicity,it will work in must case.
	 * A little extra memory(8 bytes) is allocated in the end of run buffer,since
	 * some IDE will put some sections in the end of file,this extra space is used
	 * to determine the end of image.So set the whole memory block to zero.
	 */
	pRunBuffer = (BYTE*)_hx_aligned_malloc(ImageOptionalHeader->SizeOfImage + 8, 64);
	if (NULL == pRunBuffer)
	{
		goto __TERMINAL;
	}
	memset(pRunBuffer, 0, ImageOptionalHeader->SizeOfImage + 8);

	/* Copy the jump instruction into header. */
	memcpy(pRunBuffer, &szFillHeader, dwHxHeadSize);

#if 0
	/* Show out module layout info,just for debugging. */
	_hx_printf("SA=0x%X,FA=0x%X,ImageBase=0x%X,AddressOfEntryPoint=0x%X,BaseOfCode=0x%X,BaseOfData=0x%X\r\n",
		ImageOptionalHeader->SectionAlignment,
		ImageOptionalHeader->FileAlignment,
		ImageOptionalHeader->ImageBase,
		ImageOptionalHeader->AddressOfEntryPoint,
		ImageOptionalHeader->BaseOfCode,
		ImageOptionalHeader->BaseOfData);
#endif

	/* Load the section table into memory. */
	dwOffset = dwOffset + sizeof(IMAGE_NT_HEADERS);
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);
	dwSectNum = ImageNtHeader.FileHeader.NumberOfSections;
	dwSecTableSize = dwSectNum * sizeof(IMAGE_SECTION_HEADER);

	pSectionHdr = (IMAGE_SECTION_HEADER*)_hx_malloc(dwSecTableSize);
	if (NULL == pSectionHdr)
	{
		goto __TERMINAL;
	}
	pSectionHdrPos = pSectionHdr;
	if (!ReadFile(hFileObj, dwSecTableSize, pSectionHdrPos, &dwReadSize))
	{
		goto __TERMINAL;
	}

#if 0
	/* Show out section information,just for debugging. */
	_hx_printf("dwSectNum=0x%X,dwSecTableSize=0x%X\r\n",
		dwSectNum,
		dwSecTableSize);
#endif
	for (dwIndex = 0; dwIndex < dwSectNum; dwIndex++)
	{
		DWORD   dwFileOffset = pSectionHdrPos->PointerToRawData;
		DWORD   dwMemOffset = pSectionHdrPos->VirtualAddress;
		DWORD   dwSecSize = pSectionHdrPos->SizeOfRawData;
		char*   pMemAddr = pRunBuffer + dwMemOffset;

		/* Load all sections into memory one by one. */
		SetFilePointer(hFileObj, &dwFileOffset, 0, FILE_FROM_BEGIN);
		if (!ReadFile(hFileObj, dwSecSize, pMemAddr, &dwReadSize))
		{
			goto __TERMINAL;
		}

#if 0
		_hx_printf("Section name=%s,FO=0x%X,MO=0x%X,Size=%d.\r\n",
			pSectionHdr->Name,
			dwFileOffset,
			dwMemOffset,
			dwSecSize);
#endif
		pSectionHdrPos++;
	}

	/* Relocate the code. */
	if ((DWORD)pRunBuffer != ImageOptionalHeader->ImageBase)
	{
		DWORD dwRelocBlockVa = ImageOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		if (dwRelocBlockVa != 0)
		{
			/*_hx_printf("dwRelocBlockVa=0x%X,ImageBase=0x%X,pRunBuffer=%X\r\n",
				dwRelocBlockVa,
				ImageOptionalHeader->ImageBase,
				(DWORD)pRunBuffer);*/
				/* Relocate the code. */
			if (!CodeReLocate((DWORD)pRunBuffer, dwRelocBlockVa, ImageOptionalHeader->ImageBase))
			{
				/* Failed to relocate the binary,just give up. */
				goto __TERMINAL;
			}
		}
	}

	//_hx_printf("Load PE module OK,size = %d.\r\n", ImageOptionalHeader->SizeOfImage);
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pRunBuffer)  //Should release it.
		{
			_hx_free(pRunBuffer);
			pRunBuffer = NULL;
		}
	}
	if (pSectionHdr)
	{
		_hx_free(pSectionHdr);
	}

	return pRunBuffer;
}

BOOL AppFormat_PE(HANDLE hFileObj)
{
	BYTE   szPeFlag[2] = { 0 };
	DWORD  dwOffset = 0;
	DWORD  dwRead = 0;

	//reset file start pos
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);

	if (!ReadFile(hFileObj, sizeof(szPeFlag), szPeFlag, &dwRead))
	{
		return FALSE;
	}
	if (dwRead != sizeof(szPeFlag))
	{
		return FALSE;
	}

	if (szPeFlag[0] == 'M' && szPeFlag[1] == 'Z')
	{
		return TRUE;
	}
	return FALSE;
}
