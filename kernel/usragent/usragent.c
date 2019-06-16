//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apr 14, 2019
//    Module Name               : usragent.c
//    Module Funciton           : 
//                                Source code for user agent module.This module
//                                is loaded into kernel and is maped into process's
//                                user space when a new process is created.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "fmt_pe.h"
#include "stdio.h"
#include "../include/mlayout.h"

/* Helper routine to get a string's length. */
size_t __strlen(const char * str) 
{
	size_t length = 0;
	while (*str++)
		++length;
	return  length;
}

/* Debug the system call function. */
static void ShowFile(char* pszFileName)
{
	HANDLE hFile = NULL;
	unsigned long read_sz = 0;
	char buff[128];
	MSG msg;

	/* Open the file. */
	hFile = CreateFile(pszFileName,
		FILE_ACCESS_READWRITE,
		0, NULL);
	if (NULL == hFile)
	{
		PrintLine("Failed to open the file.");
		goto __TERMINAL;
	}
	else
	{
		PrintLine("Open file OK.");
	}
	while (ReadFile(hFile, 128, buff, &read_sz))
	{
		if (0 == read_sz)
		{
			break;
		}
		if (PeekMessage(&msg))
		{
			/* Any key press will abort. */
			if ((msg.command == KERNEL_MESSAGE_AKDOWN) || 
				(msg.command == KERNEL_MESSAGE_VKDOWN))
			{
				break;
			}
		}
		/* Show out the whole file. */
		for (unsigned long i = 0; i < read_sz; i++)
		{
			if (buff[i] == '\r')
			{
				GotoHome();
				continue;
			}
			if (buff[i] == '\n')
			{
				ChangeLine();
				continue;
			}
			if (buff[i] == '\t')
			{
				PrintChar(' ');
				PrintChar(' ');
				continue;
			}
			PrintChar(buff[i]);
		}
	}

__TERMINAL:
	if (hFile)
	{
		CloseFile(hFile);
	}
	return;
}

/* Helper routine to validate the PE file's header. */
static BOOL ValidatePeHeader(char* pFileHdr)
{
	char* pFileHeader = pFileHdr;
	BOOL bResult = FALSE;
	IMAGE_DOS_HEADER* pDosHdr = NULL;
	IMAGE_NT_HEADERS* pNtHdr = NULL;
	long size_bound = 0;
	long e_lfanew = 0;

	/* Validate DOS header magic. */
	if (*(SHORT*)pFileHeader != PE_DOS_HEADER_MAGIC)
	{
		goto __TERMINAL;
	}
	/* DOS stub's length must not be too large. */
	pDosHdr = (IMAGE_DOS_HEADER*)pFileHeader;
	size_bound = sizeof(IMAGE_FILE_HEADER) + sizeof(DWORD);
	size_bound = MAX_PE_HEADER_SIZE - size_bound;
	if (pDosHdr->e_lfanew > size_bound)
	{
		PrintLine("DOS stub too long.");
		goto __TERMINAL;
	}
	e_lfanew = pDosHdr->e_lfanew;

	/* Locate and validate the NT headers. */
	pFileHeader += pDosHdr->e_lfanew;
	pNtHdr = (IMAGE_NT_HEADERS*)pFileHeader;
	if (PE_NT_HEADER_SIGNATURE != pNtHdr->Signature)
	{
		PrintLine("Invalid PE header signature.");
		goto __TERMINAL;
	}
	if (IMAGE_FILE_MACHINE_I386 != pNtHdr->FileHeader.Machine)
	{
		PrintLine("Could not support his machine type.");
		goto __TERMINAL;
	}
	/* Optional header must be present. */
	if (0 == pNtHdr->FileHeader.SizeOfOptionalHeader)
	{
		PrintLine("Not an executable file.");
		goto __TERMINAL;
	}
	/* Make sure the optional header is not exceed the boundary. */
	e_lfanew = MAX_PE_HEADER_SIZE - e_lfanew;
	e_lfanew -= sizeof(IMAGE_FILE_HEADER);
	e_lfanew -= sizeof(DWORD); /* Signature. */
	if (pNtHdr->FileHeader.SizeOfOptionalHeader > e_lfanew)
	{
		PrintLine("Optional header too long.");
		goto __TERMINAL;
	}
	/* Validate file characteristics. */
	if (!(pNtHdr->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
	{
		PrintLine("Image is not an executable file.");
		goto __TERMINAL;
	}

	/* Validate optional header. */
	if (pNtHdr->OptionalHeader.Magic != PE_OPTIONAL_HEADER_MAGIC_PE32)
	{
		/* Only PE32 format is supported. */
		PrintLine("Not a PE32 format.");
		goto __TERMINAL;
	}
	/* 
	 * Total header size should not exceed max value. 
	 * The headers include DOS stub,PE header,and section
	 * headers.
	 */
	if (pNtHdr->OptionalHeader.SizeOfHeaders > MAX_PE_HEADER_SIZE)
	{
		PrintLine("Header size too large.");
		goto __TERMINAL;
	}
	/* Section alignment must larger or equal the file alignment. */
	if (pNtHdr->OptionalHeader.SectionAlignment < pNtHdr->OptionalHeader.FileAlignment)
	{
		PrintLine("Invalid section alignment.");
		goto __TERMINAL;
	}
	/* Total size of image must be section aligned. */
	if (pNtHdr->OptionalHeader.SizeOfImage % pNtHdr->OptionalHeader.SectionAlignment)
	{
		PrintLine("Invalid image size value.");
		goto __TERMINAL;
	}
	/* Header size must be file alignment aligned. */
	if (pNtHdr->OptionalHeader.SizeOfHeaders % pNtHdr->OptionalHeader.FileAlignment)
	{
		PrintLine("Invalid total header size.");
		goto __TERMINAL;
	}

	/* Pass validation. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
 * A helper routine to open the specified PE file
 * and load the file's header into memory.
 * Basic validation will be applied in this routine,
 * to make sure it's a legal PE file.
 * The memory location that header is loaded into is
 * the first 4K of process's heap,and it's should be
 * released at the end of loading,by calling the
 * VirtualFree routine.
 */
static LPVOID LoadFileHeader(char* pszFileName, HANDLE* pPeHandle)
{
	HANDLE hFile = NULL;
	LPVOID pFileHeader = NULL;
	BOOL bResult = FALSE;
	unsigned long read_size = 0;

	/* Open the file. */
	hFile = CreateFile(pszFileName,
		FILE_ACCESS_READWRITE,
		0, NULL);
	if (NULL == hFile)
	{
		PrintLine("Failed to open the file.");
		goto __TERMINAL;
	}

	/* Allocate temporary memory from heap. */
	pFileHeader = (LPVOID)KMEM_USERHEAP_START;
	pFileHeader = VirtualAlloc(pFileHeader,
		MAX_PE_HEADER_SIZE,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"pe_hdr");
	if ((NULL == pFileHeader) || ((LPVOID)KMEM_USERHEAP_START != pFileHeader))
	{
		PrintLine("Failed to allocate temp memory to hold header.");
		goto __TERMINAL;
	}

	/* Load the file's header into memory. */
	if (!ReadFile(hFile, MAX_PE_HEADER_SIZE, pFileHeader, &read_size))
	{
		PrintLine("Failed to load PE header.");
		goto __TERMINAL;
	}
	if (read_size < MAX_PE_HEADER_SIZE)
	{
		/* File's length is less than PE header. */
		if (read_size < (sizeof(IMAGE_NT_HEADERS32) + sizeof(IMAGE_DOS_HEADER)))
		{
			PrintLine("Invalid PE format(size too short).");
			goto __TERMINAL;
		}
	}

	/* Validate the PE's header. */
	if (!ValidatePeHeader(pFileHeader))
	{
		PrintLine("Invalid PE format.");
		goto __TERMINAL;
	}

	/* Load PE file's header OK. */
	bResult = TRUE;

__TERMINAL:
	if (bResult)
	{
		*pPeHandle = hFile;
		return pFileHeader;
	}
	else
	{
		/* Close file if opened. */
		if (hFile)
		{
			CloseFile(hFile);
		}
		/* Release the temporary memory. */
		if (pFileHeader)
		{
			VirtualFree(pFileHeader);
		}
		return NULL;
	}
}

/* 
 * Commit base relocation of the code if necessary. 
 * Only in scenario that the image's base address in file
 * is not same as the base address when loaded into 
 * memory that should be relocated.
 */
static BOOL CodeRelocate(DWORD dwNewImageBase, DWORD dwRelocBlockVa, DWORD dwOldImageBase)
{
	PIMAGE_BASE_RELOCATION pRelocBlock = NULL;
	char* pRunBuffer = (char*)dwNewImageBase;
	WORD* pRelocData = NULL;
	int nRelocNum = 0, i = 0;

	pRelocBlock = (PIMAGE_BASE_RELOCATION)(pRunBuffer + dwRelocBlockVa);
	while (TRUE)
	{
		nRelocNum = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		//_hx_printf("Blockaddr = 0x%X, SizeOfBlock = 0x%X, num = %d\r\n.",
		//	pRelocBlock,pRelocBlock->SizeOfBlock, nRelocNum);
		pRelocData = (WORD*)((char*)pRelocBlock + sizeof(IMAGE_BASE_RELOCATION));
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
					_hx_printf("%s:Unknown relocate type[%d].\r\n", __FUNCTION__, (*pRelocData) >> 12);
					return FALSE;
				}
			}
			pRelocData++;
		}
		/* Process next relocation block. */
		pRelocBlock = (PIMAGE_BASE_RELOCATION)((char*)pRelocBlock + pRelocBlock->SizeOfBlock);
		//_hx_printf("Block= %X\r\n",(LPBYTE)pRelocBlock);
		if (NULL == (VOID*)pRelocBlock->VirtualAddress)
		{
			break;
		}
	}

	return TRUE;
}

/*
 * A helper routine to load the PE image into memory.
 * It allocates memory from process's space starts from
 * KMEM_USERAPP_START,copy image file's sections into
 * memory,does relocation if necessary,then return the
 * entry point of the image.
 */
static __PE_ENTRY_POINT LoadImage(LPVOID pFileHdr, HANDLE hFile)
{
	char* pFileHeader = (char*)pFileHdr;
	unsigned int num_sections = 0;
	unsigned int total_mem = 0;
	IMAGE_DOS_HEADER* pDosHdr = NULL;
	IMAGE_NT_HEADERS* pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER* pOptHdr = NULL;
	IMAGE_SECTION_HEADER* pSectHdr = NULL;
	LPVOID pBaseAddr = NULL;
	__PE_ENTRY_POINT peEntry = NULL;
	BOOL bResult = FALSE;

	/* Locate the optional header first. */
	pDosHdr = (IMAGE_DOS_HEADER*)pFileHeader;
	pFileHeader += pDosHdr->e_lfanew;
	pNtHdr = (IMAGE_NT_HEADERS*)pFileHeader;
	pOptHdr = &pNtHdr->OptionalHeader;
	pFileHeader = (char*)pOptHdr;
	pFileHeader += pNtHdr->FileHeader.SizeOfOptionalHeader;
	/* Obtain the section array and size. */
	num_sections = pNtHdr->FileHeader.NumberOfSections;
	if (0 == num_sections)
	{
		PrintLine("No content section in file.");
		goto __TERMINAL;
	}
	pSectHdr = (IMAGE_SECTION_HEADER*)pFileHeader;

	/* 
	 * Calculate how many memory the image will use. 
	 * There is a SizeOfImage in optional header,and 
	 * SizeOfCode/SizeOfInitializedData/SizeOfUninitializedData
	 * in optional header,we use the maximal of
	 * these 2 values.
	 */
	total_mem = pNtHdr->OptionalHeader.SizeOfCode;
	total_mem += pNtHdr->OptionalHeader.SizeOfInitializedData;
	total_mem += pNtHdr->OptionalHeader.SizeOfUninitializedData;
	if (total_mem < pNtHdr->OptionalHeader.SizeOfImage)
	{
		total_mem = pNtHdr->OptionalHeader.SizeOfImage;
	}

	/* Allocate memory from lineary space to hold the image. */
	pBaseAddr = VirtualAlloc((LPVOID)(KMEM_USERAPP_START + KMEM_USERAGENT_LENGTH),
		total_mem, 
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"usr_app");
	if (NULL == pBaseAddr)
	{
		PrintLine("Failed to alloc app memory.");
		goto __TERMINAL;
	}

	/* OK, now load sections into memory one by one. */
	char* pSectStart = NULL;
	unsigned long ulRead = 0, toRead = 0;
	unsigned long fileOffset = 0;
	for (unsigned int i = 0; i < num_sections; i++)
	{
		if (pSectHdr->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
		{
			/* .bss section,just reserve space. */
			if (pSectHdr->Misc.VirtualSize > total_mem)
			{
				PrintLine(".bss section too long.");
				goto __TERMINAL;
			}
			total_mem -= pSectHdr->Misc.VirtualSize;
			pSectStart = (char*)pBaseAddr;
			pSectStart += pSectHdr->VirtualAddress;
			//memset(pSectStart, 0, pSectHdr->Misc.VirtualSize);
			_hx_printf("Load section[.bss] OK,base = 0x%X,len = 0x%X\r\n",
				pSectStart, pSectHdr->Misc.VirtualSize);
			pSectHdr++;
			continue;
		}
		/* How many section data to read. */
		toRead = pSectHdr->SizeOfRawData;
		if (toRead > pSectHdr->Misc.VirtualSize)
		{
			/* 
			 * Section is file alignment aligned,so it's 
			 * size could larger than actual section size.
			 */
			toRead = pSectHdr->Misc.VirtualSize;
		}
		/* Memory location the data should be loaded into. */
		if (pSectHdr->Misc.VirtualSize > total_mem)
		{
			_hx_printf("Invalid section size[%d].\r\n",
				pSectHdr->Misc.VirtualSize);
			goto __TERMINAL;
		}
		total_mem -= pSectHdr->Misc.VirtualSize;
		pSectStart = (char*)pBaseAddr;
		/* Section's RVA must be aligned with section alignment. */
		if (pSectHdr->VirtualAddress % pNtHdr->OptionalHeader.SectionAlignment)
		{
			_hx_printf("Section is unaligned with section_alignment.\r\n");
			goto __TERMINAL;
		}
		pSectStart += pSectHdr->VirtualAddress;
		/* Read the section to memory. */
		fileOffset = pSectHdr->PointerToRawData;
		/* File offset must be aligned with file_alignment. */
		if (fileOffset % pNtHdr->OptionalHeader.FileAlignment)
		{
			_hx_printf("File offset is unaligned with file_alignment.\r\n");
			goto __TERMINAL;
		}
		SetFilePointer(hFile, &fileOffset, NULL, FILE_FROM_BEGIN);
		if (!ReadFile(hFile, toRead, pSectStart, &ulRead))
		{
			PrintLine("Failed to read section data.");
			goto __TERMINAL;
		}
		if (ulRead != toRead)
		{
			/* Exception case. */
			PrintLine("Data read is not same as required.");
			goto __TERMINAL;
		}
		/* Zere out the gap space. */
		if (toRead < pSectHdr->Misc.VirtualSize)
		{
			pSectStart += toRead;
			//memset(pSectStart, 0, (pSectHdr->Misc.VirtualSize - toRead));
		}
#if 0
		/* Show out for debugging. */
		pSectHdr->Name[IMAGE_SIZEOF_SHORT_NAME - 1] = 0;
		_hx_printf("Load section[%s] at:0x%X, len = 0x%X.\r\n", pSectHdr->Name,
			(unsigned long)pBaseAddr + pSectHdr->VirtualAddress,
			pSectHdr->Misc.VirtualSize);
#endif
		/* Process next section. */
		pSectHdr++;
	}

	/* Carry out base relocation if necessary. */
	if (pNtHdr->OptionalHeader.ImageBase != (unsigned long)pBaseAddr)
	{
		/* Check if base relocation directory exist. */
		if (pNtHdr->OptionalHeader.NumberOfRvaAndSizes < (IMAGE_DIRECTORY_ENTRY_BASERELOC + 1))
		{
			_hx_printf("No code relocation directory exist.\r\n");
			goto __TERMINAL;
		}
		unsigned long rba = 
			pNtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		if (0 == rba)
		{
			_hx_printf("No relocation block info since relocating required.\r\n");
			goto __TERMINAL;
		}
		/* Do actual relocation. */
		if (!CodeRelocate((unsigned long)pBaseAddr, rba, pNtHdr->OptionalHeader.ImageBase))
		{
			_hx_printf("Failed to do base relocation.\r\n");
			goto __TERMINAL;
		}
	}

	/* Get the entry point and return it. */
	peEntry = (__PE_ENTRY_POINT)(pOptHdr->AddressOfEntryPoint + (DWORD)pBaseAddr);

	/* Everything is in place. */
	bResult = TRUE;

__TERMINAL:
	if (bResult)
	{
		return peEntry;
	}
	else
	{
		return NULL;
	}
}

/* Entry point of user agent. */
int __UsrAgent_Entry(int argc, char* argv[])
{
	HANDLE hFile = NULL;
	LPVOID pFileHeader = NULL;
	__PE_ENTRY_POINT peEntry = NULL;
	int main_ret = 0;

	if (argc < 2)
	{
		/* No application specfied. */
		PrintLine("No application specified.");
		goto __TERMINAL;
	}

	/* Load the PE's header into memory. */
	pFileHeader = LoadFileHeader(argv[1], &hFile);
	if (NULL == pFileHeader)
	{
		PrintLine("Failed to load PE header.");
		goto __TERMINAL;
	}
	/* Load image into memory after header is loaded. */
	peEntry = LoadImage(pFileHeader, hFile);
	if (NULL == peEntry)
	{
		PrintLine("Failed to load PE image.");
		goto __TERMINAL;
	}

	/* 
	 * Begin to launch the application,just to call the 
	 * entry routine by using parameters that transfered
	 * from OS kernel.
	 */
	main_ret = peEntry(argc - 1, &argv[1]);

#if 0
	/* Reserve virtual space for heap. */
	pUserHeap = VirtualAlloc(pUserHeap,
		USERHEAP_INIT_LENGTH,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"usrheap");
	if ((NULL == pUserHeap) || ((LPVOID)KMEM_USERHEAP_START != pUserHeap))
	{
		PrintLine("Failed to allocate user heap.");
		goto __TERMINAL;
	}

	/* Initializes the user heap's header. */
	pUserHeapDesc = (__USER_HEAP_DESCRIPTOR*)pUserHeap;
	pUserHeapDesc->hMutex = NULL;
	pUserHeapDesc->total_length = USERHEAP_INIT_LENGTH;
#endif

__TERMINAL:
	/* Close the corresponding PE file. */
	if (hFile)
	{
		CloseFile(hFile);
	}
	ExitThread(0);
	return main_ret;
}
