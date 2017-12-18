//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Aug 25,2015
//    Module Name               : apploader_pe.c
//    Module Funciton           : 
//                                This module countains Win32 PE APP exec .
//    Last modified Author      : tywind
//    Last modified Date        : 2017.2.7
//    Last modified Content     : 
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include "kapi.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

// Directory Entries
#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor


#define IMAGE_REL_BASED_HIGHLOW               3

typedef struct __tagFILE_HEADER
{
	unsigned char ucNop[4];
	DWORD         dwJmpAddr;
}__FILL_HEADER;


typedef struct _IMAGE_DOS_HEADER
{
	// DOS .EXE header
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	WORD   e_res2[10];                  // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;


typedef struct _IMAGE_FILE_HEADER
{
	WORD    Machine;
	WORD    NumberOfSections;
	DWORD   TimeDateStamp;
	DWORD   PointerToSymbolTable;
	DWORD   NumberOfSymbols;
	WORD    SizeOfOptionalHeader;
	WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;


typedef struct _IMAGE_DATA_DIRECTORY
{
	DWORD   VirtualAddress;
	DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_BASE_RELOCATION {
	DWORD   VirtualAddress;
	DWORD   SizeOfBlock;
	//  WORD    TypeOffset[1];
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;
//
// Optional header format.
//

typedef struct _IMAGE_OPTIONAL_HEADER
{
	//
	// Standard fields.
	//

	WORD    Magic;
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;
	DWORD   SizeOfInitializedData;
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;
	DWORD   BaseOfCode;
	DWORD   BaseOfData;

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;


typedef struct _IMAGE_NT_HEADERS
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef IMAGE_NT_HEADERS32       IMAGE_NT_HEADERS;
typedef IMAGE_OPTIONAL_HEADER32  IMAGE_OPTIONAL_HEADER;

#define IMAGE_SIZEOF_SHORT_NAME 8
typedef struct _IMAGE_SECTION_HEADER
{
	BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
	union
	{
		DWORD   PhysicalAddress;
		DWORD   VirtualSize;
	} Misc;
	DWORD   VirtualAddress;
	DWORD   SizeOfRawData;
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;


// code data  relocate
static BOOL CodeReLocate(DWORD dwNewImageBase, DWORD dwRelocBlockVa, DWORD dwOldImageBase)
{
	PIMAGE_BASE_RELOCATION  pRelocBlock = NULL;
	LPBYTE                  pRunBuffer = (LPBYTE)dwNewImageBase;
	WORD*                   pRelocData = NULL;
	INT                     nRelocNum = 0;
	INT                     i = 0;

	//_hx_printf("ImageBase = 0x%X,NewImageBase= 0x%X RelocBlock VA = 0x%X\r\n",dwOldImageBase,dwNewImageBase,dwRelocBlockVa);

	pRelocBlock = (PIMAGE_BASE_RELOCATION)(pRunBuffer + dwRelocBlockVa);

	while (TRUE)
	{
		nRelocNum = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

		//_hx_printf("Blockaddr=%X  SizeOfBlock= 0x%X,num=%d\r\n",pRelocBlock,pRelocBlock->SizeOfBlock, nRelocNum);

		pRelocData = (WORD*)((LPBYTE)pRelocBlock + sizeof(IMAGE_BASE_RELOCATION));
		for (i = 0; i<nRelocNum; i++)
		{
			DWORD*   pRelocAddress = NULL;
			WORD     nPageOff = 0;


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

			//next data 
			pRelocData++;
		}

		// next block
		pRelocBlock = (PIMAGE_BASE_RELOCATION)((LPBYTE)pRelocBlock + pRelocBlock->SizeOfBlock);
		//_hx_printf("Block= %X\r\n",(LPBYTE)pRelocBlock);

		if (NULL == (VOID*)pRelocBlock->VirtualAddress)
		{
			break;
		}

	}

	return TRUE;
}

LPBYTE LoadAppToMemory_PE(HANDLE hFileObj)
{
	IMAGE_DOS_HEADER        ImageDosHeader = { 0 };
	IMAGE_NT_HEADERS        ImageNtHeader = { 0 };
	IMAGE_OPTIONAL_HEADER*  ImageOptionalHeader = NULL;
	IMAGE_SECTION_HEADER*   pSectionHdr = NULL;
	IMAGE_SECTION_HEADER*   pSectionHdrPos = NULL;

	// app jmp command and new app addres 
	__FILL_HEADER           szFillHeader = { 0x90, 0x90, 0x90, 0xe9, 0x00000000 };
	__FILL_HEADER           szFillHeader2 = { 0x90, 0x90, 0x90, 0xe9, 0x00000000 };
	LPBYTE                  pRunBuffer = NULL;//(LPBYTE)dwStartAddress;
	LPBYTE                  pJmpBuffer = NULL;//(LPBYTE)dwStartAddress;
	BOOL                    bResult = FALSE;
	DWORD                   dwHxHeadSize = sizeof(__FILL_HEADER);
	DWORD                   dwSecTableSize = 0;
	DWORD                   dwReadSize = 0;
	DWORD                   dwOffset = 0;
	DWORD                   dwSectNum = 0;
	DWORD                   dwIndex = 0;

	//reset file start pos
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);

	//read Pe Dos head info 
	if (!ReadFile(hFileObj, sizeof(ImageDosHeader), &ImageDosHeader, &dwReadSize))
	{
		goto __TERMINAL;
	}
	dwOffset = ImageDosHeader.e_lfanew;

	//read Pe nt head info 
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);
	ReadFile(hFileObj, sizeof(ImageNtHeader), &ImageNtHeader, &dwReadSize);
	ImageOptionalHeader = &(ImageNtHeader.OptionalHeader);

	//Calculate the target address will
	//szFillHeader.dwJmpAddr    = ImageOptionalHeader->BaseOfCode;
	szFillHeader.dwJmpAddr = ImageOptionalHeader->AddressOfEntryPoint;
	szFillHeader.dwJmpAddr -= dwHxHeadSize;

	/**
	 * Aligned allocation must be used here,and the align value should be fetched
	 * from image file.But we assume it as 64 for simplicity,it will work in must case.
	 * A little extra memory(8 bytes) is allocated in the end of run buffer,since
	 * some IDE will put some sections in the end of file,this extra space is used
	 * to determine the end of image.So set the whole memory block to zero.
	 */
	//pRunBuffer = (LPBYTE)_hx_malloc(ImageOptionalHeader->SizeOfImage);
	pRunBuffer = (LPBYTE)_hx_aligned_malloc(ImageOptionalHeader->SizeOfImage + 8, 64);
	if (NULL == pRunBuffer)
	{
		goto __TERMINAL;
	}
	memset(pRunBuffer, 0, ImageOptionalHeader->SizeOfImage + 8);

	//copy jmp command and address to app buf      
	memcpy(pRunBuffer, &szFillHeader, dwHxHeadSize);

	/*_hx_printf("SA=0x%X,FA=0x%X,ImageBase=0x%X,AddressOfEntryPoint=0x%X,BaseOfCode=0x%X,BaseOfData=0x%X\r\n",
	ImageOptionalHeader->SectionAlignment,
	ImageOptionalHeader->FileAlignment,
	ImageOptionalHeader->ImageBase,
	ImageOptionalHeader->AddressOfEntryPoint,
	ImageOptionalHeader->BaseOfCode,
	ImageOptionalHeader->BaseOfData);*/

	//read section table	
	dwOffset = dwOffset + sizeof(IMAGE_NT_HEADERS);
	SetFilePointer(hFileObj, &dwOffset, 0, FILE_FROM_BEGIN);
	dwSectNum = ImageNtHeader.FileHeader.NumberOfSections;
	dwSecTableSize = dwSectNum*sizeof(IMAGE_SECTION_HEADER);

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

	/*_hx_printf("dwSectNum=0x%X,dwSecTableSize=0x%X\r\n",
	dwSectNum,
	dwSecTableSize
	);*/

	for (dwIndex = 0; dwIndex < dwSectNum; dwIndex++)
	{
		DWORD   dwFileOffset = pSectionHdrPos->PointerToRawData;
		DWORD   dwMemOffset = pSectionHdrPos->VirtualAddress;
		DWORD   dwSecSize = pSectionHdrPos->SizeOfRawData;
		LPBYTE  pMemAddr = pRunBuffer + dwMemOffset;

		SetFilePointer(hFileObj, &dwFileOffset, 0, FILE_FROM_BEGIN);

		/*for(i=0;i<dwSecSize/512;i++)
		{
		ReadFile(hFileObj,512,pMemAddr,&dwReadSize);
		pMemAddr += 512;
		}*/
		if (!ReadFile(hFileObj, dwSecSize, pMemAddr, &dwReadSize))
		{
			goto __TERMINAL;
		}

		/*_hx_printf("sec name=%s,FO=0x%X,MO=0x%X,Size=%d\r\n",
		pSectionHdr->Name,
		dwFileOffset,
		dwMemOffset,
		dwSecSize
		);*/

		pSectionHdrPos++;
	}

	// is need relocate
	if ((DWORD)pRunBuffer != ImageOptionalHeader->ImageBase)
	{
		DWORD dwRelocBlockVa = ImageOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		if (dwRelocBlockVa != 0)
		{
			/*_hx_printf("dwRelocBlockVa=0x%X,ImageBase=0x%X,pRunBuffer=%X\r\n",
			dwRelocBlockVa,
			ImageOptionalHeader->ImageBase,
			(DWORD)pRunBuffer);*/
			CodeReLocate((DWORD)pRunBuffer, dwRelocBlockVa, ImageOptionalHeader->ImageBase);
		}
	}
	
	_hx_printf("Load PE module OK,size = %d,begin to run.\r\n", ImageOptionalHeader->SizeOfImage);
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
