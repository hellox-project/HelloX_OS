//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29, 2024
//    Module Name               : loader.c
//    Module Funciton           : 
//                                HelloX's loader is implemented in this
//                                file. The loader is a common purpuse that
//                                can load many types of module, such as PE,
//                                COFF, ELF, ...
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "../include/mlayout.h"
#include "pteb.h"
#include "loader.h"
#include "fmt_pe.h"
#include "stdio.h"
#include "string.h"

/* Validate PE32's optional header. */
static BOOL ValidatePe32Hdr(IMAGE_NT_HEADERS* pNtHdr)
{
	BOOL bResult = FALSE;

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
		_hx_printf("[%s]Invalid size value: sectalign=%d, filealign=%d.\r\n",
			__func__, pNtHdr->OptionalHeader.SectionAlignment,
			pNtHdr->OptionalHeader.FileAlignment);
		goto __TERMINAL;
	}
	if ((pNtHdr->OptionalHeader.SectionAlignment == 0) || 
		(pNtHdr->OptionalHeader.FileAlignment == 0))
	{
		_hx_printf("[%s]Invalid size value: sectalign=%d, filealign=%d.\r\n",
			__func__, pNtHdr->OptionalHeader.SectionAlignment,
			pNtHdr->OptionalHeader.FileAlignment);
		goto __TERMINAL;
	}

#if 1 /* Proposed by AI, may not correct. */
	/* Total size of image must be section aligned. */
	if (pNtHdr->OptionalHeader.SizeOfImage % pNtHdr->OptionalHeader.SectionAlignment)
	{
		_hx_printf("[%s]Invalid size value: image=%d, sectalign=%d\r\n",
			__func__, pNtHdr->OptionalHeader.SizeOfImage,
			pNtHdr->OptionalHeader.SectionAlignment);
		goto __TERMINAL;
	}
	/* Header size must be file alignment aligned. */
	if (pNtHdr->OptionalHeader.SizeOfHeaders % pNtHdr->OptionalHeader.FileAlignment)
	{
		_hx_printf("[%s]Invalid size value: header_sz=%d, filealign=%d\r\n",
			__func__, pNtHdr->OptionalHeader.SizeOfHeaders,
			pNtHdr->OptionalHeader.FileAlignment);
		goto __TERMINAL;
	}
#endif 

	/* Pass validation. */
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/* Validate PE32PLUS's optional header. */
static BOOL ValidatePe32plusHdr(IMAGE_NT_HEADERS64* pNtHdr)
{
	BOOL bResult = FALSE;

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
	if (pNtHdr->OptionalHeader.SectionAlignment == 0)
	{
		PrintLine("Invalid alignment in optional header.");
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

/* Helper routine to validate the PE file's header. */
static BOOL ValidatePeHeader(char* pFileHdr, __MODULE_DESCRIPTOR* pModule)
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

	/* Validate DOS stub's length. */
	pDosHdr = (IMAGE_DOS_HEADER*)pFileHeader;
	if (pDosHdr->e_lfanew < (LONG)sizeof(IMAGE_DOS_HEADER)) {
		PrintLine("Invalid e_lfanew (too small).");
		goto __TERMINAL;
	}
	if (pDosHdr->e_lfanew >= MAX_PE_HEADER_SIZE) {
		PrintLine("Invalid e_lfanew (too large).");
		goto __TERMINAL;
	}
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
	if ((IMAGE_FILE_MACHINE_I386 != pNtHdr->FileHeader.Machine) &&
		(IMAGE_FILE_MACHINE_AMD64 != pNtHdr->FileHeader.Machine))
	{
		/* Only x86 or x86-64 are supported now. */
		printf("Unsupported machine type[0x%X].\r\n",
			pNtHdr->FileHeader.Machine);
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

	/* 
	 * Validate optional header, according file's 
	 * format(PE32 or PE32plus. 
	 */
	if (pNtHdr->OptionalHeader.Magic == PE_OPTIONAL_HEADER_MAGIC_PE32)
	{
		/* PE32, set flag and clear PE32+ flag. */
		pModule->module_flags |= MODULE_FLAG_PE32;
		pModule->module_flags &= (~MODULE_FLAG_PE32PLUS);
		bResult = ValidatePe32Hdr(pNtHdr);
	}
	else {
		if (pNtHdr->OptionalHeader.Magic == PE_OPTIONAL_HEADER_MAGIC_PE32PLUS)
		{
			/* PE32+. */
			pModule->module_flags |= MODULE_FLAG_PE32PLUS;
			pModule->module_flags &= (~MODULE_FLAG_PE32);
			bResult = ValidatePe32plusHdr((IMAGE_NT_HEADERS64*)pNtHdr);
		}
		else
		{
			/* Only PE format is supported. */
			_hx_printf("Not a valid PE format, magic: [%d].\r\n",
				pNtHdr->OptionalHeader.Magic);
			goto __TERMINAL;
		}
	}

	/* Save NT header to module descriptor. */
	pModule->nt_header = (DWORD)pNtHdr;

__TERMINAL:
	return bResult;
}

/* Construct full DLL file name. */
static BOOL __construct_full_name(char* pszName, const char* path, const char* name)
{
	strncpy(pszName, path, MAX_FILE_PATH_LEN);
	strcat(pszName, "\\");
	strcat(pszName, name);
	return TRUE;
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
static LPVOID LoadFileHeader(__PEB* peb, __MODULE_DESCRIPTOR* module, HANDLE* pPeHandle)
{
	char pszFileName[MAX_FILE_PATH_LEN + MAX_MODULE_NAME_LEN + 1];
	HANDLE hFile = NULL;
	LPVOID pFileHeader = NULL;
	BOOL bResult = FALSE;
	unsigned long read_size = 0;

	/* Construct full module's name, current dir as path. */
	__construct_full_name(pszFileName, peb->current_dir, module->module_name);
	/* Open the file. */
	hFile = CreateFile(pszFileName,
		FILE_ACCESS_READWRITE,
		0, NULL);
	if (NULL == hFile)
	{
		/* 
		 * Try to search the system directory,
		 * if the module is a DLL.
		 */
		if (module->module_flags & MODULE_FLAG_DLL)
		{
			if (peb->process_flags & PROCESS_FLAGS_BIT64)
			{
				/* 64 bit. */
				__construct_full_name(pszFileName, HELLOX_SYSTEM_DIR64, module->module_name);
			}
			else {
				/* 32 bit. */
				__construct_full_name(pszFileName, HELLOX_SYSTEM_DIR32, module->module_name);
			}
			hFile = CreateFile(pszFileName,
				FILE_ACCESS_READWRITE,
				0, NULL);
		}
	}
	if (NULL == hFile)
	{
		_hx_printf("Open module [%s] error [0x%d].\r\n", 
			pszFileName, GetLastError());
		goto __TERMINAL;
	}

	/* Allocate temporary memory from heap. */
	pFileHeader = (LPVOID)KMEM_USERHEAP_START;
	pFileHeader = VirtualAlloc(pFileHeader,
		MAX_PE_HEADER_SIZE,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"pe_hdr");
	if (NULL == pFileHeader)
	{
		_hx_printf("[%s/%d]alloc memory failed.\r\n",
			__FUNCTION__, __LINE__);
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
	if (!ValidatePeHeader(pFileHeader, module))
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
 * Apply base relocation of the code if necessary.
 * Only in scenario that the image's base address in file
 * is not same as the base address when loaded into
 * memory that should be relocated.
 * This routine could be used to relocate exe or dll
 * files.
 * DIR64 may not work since no thoroghly testing.
 */
static BOOL CodeRelocate(DWORD dwNewImageBase, DWORD dwRelocBlockVa,
	DWORD dwOldImageBase)
{
	PIMAGE_BASE_RELOCATION pRelocBlock = NULL;
	char* pRunBuffer = (char*)dwNewImageBase;
	WORD* pRelocData = NULL;
	int nRelocNum = 0, i = 0;
	BOOL bResult = FALSE;

	pRelocBlock = (PIMAGE_BASE_RELOCATION)(pRunBuffer + dwRelocBlockVa);
	while (TRUE)
	{
		nRelocNum = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;
		pRelocData = (WORD*)((char*)pRelocBlock + sizeof(IMAGE_BASE_RELOCATION));
		for (i = 0; i < nRelocNum; i++)
		{
			DWORD* pRelocAddress = NULL;
			ULONGLONG* pRelocAddress64 = NULL;
			WORD rel_entry = *pRelocData;
			int rel_type = (rel_entry >> 12);
			WORD nPageOff = 0;

			switch (rel_type)
			{
			case IMAGE_REL_BASED_ABSOLUTE:
				break;
			case IMAGE_REL_BASED_HIGHLOW:
				nPageOff = (*pRelocData) & 0xFFF;
				pRelocAddress = (DWORD*)(dwNewImageBase + pRelocBlock->VirtualAddress + nPageOff);
				*pRelocAddress = *pRelocAddress - dwOldImageBase + dwNewImageBase;
				break;
			case IMAGE_REL_BASED_DIR64:
				nPageOff = (*pRelocData) & 0xFFF;
				pRelocAddress64 = (ULONGLONG*)(dwNewImageBase + pRelocBlock->VirtualAddress + nPageOff);
				*pRelocAddress64 = *pRelocAddress64 - dwOldImageBase + dwNewImageBase;
				break;
			case IMAGE_REL_BASED_HIGHADJ:
			case IMAGE_REL_BASED_HIGH:
			case IMAGE_REL_BASED_LOW:
			default:
				/*
				 * Show a warning and give up since we cann't process
				 * other relocation type,but the type of 0 is skiped
				 * since it's only the pad of relacation table,
				 * to make sure the whole size of the table is align
				 * with double word boundary.
				 */
				_hx_printf("%s:Unknown relocate type[%d].\r\n",
					__FUNCTION__, (*pRelocData) >> 12);
				goto __TERMINAL;
			}
			pRelocData++;
		}
		/* Process next relocation block. */
		pRelocBlock = (PIMAGE_BASE_RELOCATION)((char*)pRelocBlock +
			pRelocBlock->SizeOfBlock);
		if (NULL == (VOID*)pRelocBlock->VirtualAddress)
		{
			break;
		}
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Helper routine to check if a section is 
 * regular section. Loader will skip no-regular
 * sections.
 */
const char* no_reg_sections[] = {
	".comment",
	".debug",
	".drectve",
	NULL
};

static BOOL NoRegularSection(IMAGE_SECTION_HEADER* pSectHdr)
{
	BOOL bResult = FALSE;
	const char* sec_name = NULL;
	char SecName[IMAGE_SIZEOF_SHORT_NAME + 1];
	int name_idx = 0;

	/* Retrieve the section name from header. */
	SecName[IMAGE_SIZEOF_SHORT_NAME] = '\0';
	memcpy(SecName, pSectHdr->Name, IMAGE_SIZEOF_SHORT_NAME);

	/* Check if the section is no-regular. */
	while (TRUE)
	{
		sec_name = no_reg_sections[name_idx];
		if (NULL == sec_name)
		{
			break;
		}
		if (0 == strncmp(sec_name, SecName, (IMAGE_SIZEOF_SHORT_NAME + 1)))
		{
			bResult = TRUE;
			break;
		}
		name_idx++;
	}

	return bResult;
}

/*
* Check if the image supports base relocation.
* This function only checks relocation directory existence
* from optional header(PE32/PE32+).
* Generated by AI(cursor).
*/
static BOOL IsRelocCapable(void* pOptionalHeader, DWORD characteristics)
{
	IMAGE_OPTIONAL_HEADER* pOpt32 = NULL;
	IMAGE_OPTIONAL_HEADER64* pOpt64 = NULL;
	if (NULL == pOptionalHeader)
	{
		return FALSE;
	}
	/* File header explicitly says relocations are stripped. */
	if (characteristics & IMAGE_FILE_RELOCS_STRIPPED)
	{
		return FALSE;
	}
	pOpt32 = (IMAGE_OPTIONAL_HEADER*)pOptionalHeader;
	if (pOpt32->Magic == PE_OPTIONAL_HEADER_MAGIC_PE32)
	{
		if (pOpt32->NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_BASERELOC)
		{
			return FALSE;
		}
		if (pOpt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0)
		{
			return FALSE;
		}
		if (pOpt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	pOpt64 = (IMAGE_OPTIONAL_HEADER64*)pOptionalHeader;
	if (pOpt64->Magic == PE_OPTIONAL_HEADER_MAGIC_PE32PLUS)
	{
		if (pOpt64->NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_BASERELOC)
		{
			return FALSE;
		}
		if (pOpt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0)
		{
			return FALSE;
		}
		if (pOpt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
		{
			return FALSE;
		}
		return TRUE;
	}
	/* Unknown optional header format */
	return FALSE;
}

/* Load PE32 image. */
static DWORD __load_image_32(__MODULE_DESCRIPTOR* module, __PEB* peb,
	LPVOID pFileHdr, HANDLE hFile)
{
	char* pFileHeader = (char*)pFileHdr;
	unsigned int num_sections = 0;
	unsigned int total_mem = 0;
	IMAGE_DOS_HEADER* pDosHdr = NULL;
	IMAGE_NT_HEADERS* pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER* pOptHdr = NULL;
	IMAGE_SECTION_HEADER* pSectHdr = NULL;
	LPVOID pBaseAddr = NULL;
	DWORD peEntry = 0;
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
	size_t sec_tbl_off = (size_t)((char*)pSectHdr - (char*)pFileHdr);
	size_t sec_tbl_len = (size_t)num_sections * sizeof(IMAGE_SECTION_HEADER);
	if (num_sections == 0 || sec_tbl_len / sizeof(IMAGE_SECTION_HEADER) != num_sections) {
		PrintLine("Invalid section count.");
		goto __TERMINAL;
	}
	if (sec_tbl_off > MAX_PE_HEADER_SIZE || sec_tbl_len > (MAX_PE_HEADER_SIZE - sec_tbl_off)) {
		PrintLine("Section table out of header bounds.");
		goto __TERMINAL;
	}

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

	/*
	 * Allocate memory from lineary space to hold the
	 * PE image. The start address is determined by the
	 * image's type, i.e, exe or dll.
	 */
	DWORD default_start = 0, alloc_flags = 0;
	if (pNtHdr->FileHeader.Characteristics & IMAGE_FILE_DLL)
	{
		default_start = KMEM_USERLIB_START;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_USERLIB;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_ALL;
	}
	else {
		default_start = KMEM_IMAGE_START;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_USERAPP;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_ALL;
	}

	/* 
	 * Check if the image is relocable. If not, the
	 * image base must be same as default_start.
	 */
	if (!IsRelocCapable(pOptHdr, pNtHdr->FileHeader.Characteristics))
	{
		if (pNtHdr->OptionalHeader.ImageBase != default_start)
		{
			PrintLine("No reloc table with different base specified.");
			goto __TERMINAL;
		}
	}

	pBaseAddr = VirtualAlloc((LPVOID)default_start,
		total_mem,
		alloc_flags,
		VIRTUAL_AREA_ACCESS_RW,
		"bin_image");
	if (NULL == pBaseAddr)
	{
		_hx_printf("Can not alloc image memory[0x%X].\r\n", default_start);
		goto __TERMINAL;
	}

	/*
	 * VERY IMPORTANT: Clear the allocated memory block.
	 * .bss section must be initialized to all 0,we just
	 * do on the whole memory block here.
	 */
	memset(pBaseAddr, 0, total_mem);

	/* Load all sections into memory one by one. */
	char* pSectStart = NULL;
	unsigned long ulRead = 0, toRead = 0;
	unsigned long fileOffset = 0;
	for (unsigned int i = 0; i < num_sections; i++)
	{
		/* Skip all no-regular sections. */
		if (NoRegularSection(pSectHdr))
		{
			pSectHdr++;
			continue;
		}

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
			_hx_printf("Invalid file offset[0x%X, fa = 0x%X.\r\n",
				fileOffset, pNtHdr->OptionalHeader.FileAlignment);
			goto __TERMINAL;
		}
		
		SetFilePointer(hFile, &fileOffset, NULL, FILE_FROM_BEGIN);
		//_hx_printf("[%s]read section data: addr=0x%0X, size=%d\r\n", __func__,
		//	pSectStart, toRead);
		if (!ReadFile(hFile, toRead, pSectStart, &ulRead))
		{
			PrintLine("Failed to read section data.");
			goto __TERMINAL;
		}
		if (ulRead != toRead)
		{
			/* Exception case. */
			_hx_printf("[%s]load file failure: request[%d], read[%d]\r\n",
				__func__, toRead, ulRead);
			goto __TERMINAL;
		}
		/* Zere out the gap space. */
		if (toRead < pSectHdr->Misc.VirtualSize)
		{
			pSectStart += toRead;
		}

		pSectHdr++;
	}

	/* Apply base relocation if necessary. */
	if (pNtHdr->OptionalHeader.ImageBase != (unsigned long)pBaseAddr)
	{
		/* Check if relocation directory exist. */
		if ((pNtHdr->OptionalHeader.NumberOfRvaAndSizes < (IMAGE_DIRECTORY_ENTRY_BASERELOC + 1)) ||
			(pNtHdr->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED))
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
		if (!CodeRelocate((unsigned long)pBaseAddr, rba,
			pNtHdr->OptionalHeader.ImageBase))
		{
			_hx_printf("Reloc failed.\r\n");
			goto __TERMINAL;
		}
	}
	/* Process data directories. */
	bResult = ProcessDataDirectory(module, peb, pNtHdr, (DWORD)pBaseAddr);
	if (!bResult)
	{
		_hx_printf("Process data dir failed.\r\n");
		goto __TERMINAL;
	}

	/* Save base and entry. */
	peEntry = (pOptHdr->AddressOfEntryPoint + (DWORD)pBaseAddr);
	module->loaded_addr = (DWORD)pBaseAddr;

	/* Everything is in place. */
	bResult = TRUE;

__TERMINAL:
	if (bResult)
	{
		return peEntry;
	}
	else
	{
		return 0;
	}
}

/* 64 bits version. */
static DWORD __load_image_64(__MODULE_DESCRIPTOR* module, __PEB* peb,
	LPVOID pFileHdr, HANDLE hFile)
{
	char* pFileHeader = (char*)pFileHdr;
	unsigned int num_sections = 0;
	unsigned int total_mem = 0;
	IMAGE_DOS_HEADER* pDosHdr = NULL;
	IMAGE_NT_HEADERS64* pNtHdr = NULL;
	IMAGE_OPTIONAL_HEADER64* pOptHdr = NULL;
	IMAGE_SECTION_HEADER* pSectHdr = NULL;
	LPVOID pBaseAddr = NULL;
	DWORD peEntry = 0;
	BOOL bResult = FALSE;

	/* Locate the optional header first. */
	pDosHdr = (IMAGE_DOS_HEADER*)pFileHeader;
	pFileHeader += pDosHdr->e_lfanew;
	pNtHdr = (IMAGE_NT_HEADERS64*)pFileHeader;
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

	/*
	 * Allocate memory from lineary space to hold the
	 * PE image. The start address is determined by the
	 * image's type, i.e, exe or dll.
	 */
	DWORD default_start = 0;
	unsigned long alloc_flags = 0;
	if (pNtHdr->FileHeader.Characteristics & IMAGE_FILE_DLL)
	{
		default_start = KMEM_USERLIB_START;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_ALL;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_USERLIB;
	}
	else {
		default_start = KMEM_IMAGE_START;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_ALL;
		alloc_flags |= VIRTUAL_AREA_ALLOCATE_USERAPP;
	}
	pBaseAddr = VirtualAlloc((LPVOID)default_start,
		total_mem,
		alloc_flags,
		VIRTUAL_AREA_ACCESS_RW,
		"usr_image");
	if (NULL == pBaseAddr)
	{
		_hx_printf("Can not alloc image memory[0x%X].\r\n", default_start);
		goto __TERMINAL;
	}

	/*
	 * VERY IMPORTANT: Clear the allocated memory block.
	 * .bss section must be initialized to all 0,we just
	 * do on the whole memory block here.
	 */
	memset(pBaseAddr, 0, total_mem);

	/* Load all sections into memory one by one. */
	char* pSectStart = NULL;
	unsigned long ulRead = 0, toRead = 0;
	unsigned long fileOffset = 0;
	for (unsigned int i = 0; i < num_sections; i++)
	{
		/* Skip all no-regular sections. */
		if (NoRegularSection(pSectHdr))
		{
			pSectHdr++;
			continue;
		}

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
			_hx_printf("Invalid file offset[0x%X, fa = 0x%X.\r\n",
				fileOffset, pNtHdr->OptionalHeader.FileAlignment);
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
			_hx_printf("[%s]load file failure: request[%d], read[%d]\r\n",
				__func__, toRead, ulRead);
			goto __TERMINAL;
		}
		/* Zere out the gap space. */
		if (toRead < pSectHdr->Misc.VirtualSize)
		{
			pSectStart += toRead;
		}

		pSectHdr++;
	}

	/* Apply base relocation if necessary. */
	if (pNtHdr->OptionalHeader.ImageBase != (unsigned long)pBaseAddr)
	{
		/* Check if relocation directory exist. */
		if ((pNtHdr->OptionalHeader.NumberOfRvaAndSizes < (IMAGE_DIRECTORY_ENTRY_BASERELOC + 1)) ||
			(pNtHdr->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED))
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
		if (!CodeRelocate((unsigned long)pBaseAddr, rba,
			(DWORD)pNtHdr->OptionalHeader.ImageBase))
		{
			_hx_printf("Reloc failed.\r\n");
			goto __TERMINAL;
		}
	}
	/* Process data directories. */
	bResult = ProcessDataDirectory64(module, peb, pNtHdr, (DWORD)pBaseAddr);
	if (!bResult)
	{
		_hx_printf("Process data dir failed.\r\n");
		goto __TERMINAL;
	}

	/* Save base and entry. */
	peEntry = (pOptHdr->AddressOfEntryPoint + (DWORD)pBaseAddr);
	module->loaded_addr = (DWORD)pBaseAddr;

	/* Everything is in place. */
	bResult = TRUE;

__TERMINAL:
	if (bResult)
	{
		return peEntry;
	}
	else
	{
		return 0;
	}
}

/*
 * A helper routine to load the PE image into memory.
 * It allocates memory from process's space starts from
 * KMEM_USERAPP_START,copy image file's sections into
 * memory,does relocation if necessary,then return the
 * entry point of the image.
 * It invokes the __load_image_xx sub-routine according
 * PE file's format, i.e, PE32 or PE32PLUS.
 */
static DWORD LoadImage(__MODULE_DESCRIPTOR* module, __PEB* peb,
	LPVOID pFileHdr, HANDLE hFile)
{
	DWORD entry = 0;
	if (module->module_flags & MODULE_FLAG_PE32)
	{
		entry = __load_image_32(module, peb, pFileHdr, hFile);
		return entry;
	}
	if (module->module_flags & MODULE_FLAG_PE32PLUS)
	{
		entry = __load_image_64(module, peb, pFileHdr, hFile);
		return entry;
	}
	return entry;
}

/* 
 * Load a module give it's descriptor, 
 * and the corresponding PEB also updated
 * accordingly.
 */
static unsigned long LoadModule(__MODULE_DESCRIPTOR* module, __PEB* peb)
{
	LPVOID pFileHeader = NULL;
	HANDLE hFile = NULL;
	unsigned long errors = 0;
	DWORD peEntry = 0;

	/* Load the PE's header into memory. */
	pFileHeader = LoadFileHeader(peb, module, &hFile);
	if (NULL == pFileHeader)
	{
		PrintLine("Failed to load image header.");
		errors = LOADER_ERROR_IMAGEHEADER;
		goto __TERMINAL;
	}

	/* Load image into memory after header is loaded. */
	peEntry = LoadImage(module, peb, pFileHeader, hFile);
	if (0 == peEntry)
	{
		PrintLine("Failed to load PE image.");
		errors = LOADER_ERROR_LOAD;
		goto __TERMINAL;
	}

	/* Load OK, update module and PEB. */
	module->entry_point = peEntry;
	module->image_header = (DWORD)pFileHeader;
	module->module_flags |= MODULE_FLAG_LOADED;
	module->ref_count++;
	module->hFile = hFile;

	//_hx_printf("[%s]module[%s] is loaded at[0x%X]\r\n",
	//	__func__, module->module_name, module->loaded_addr);

	/* Bind symbols that depend on this module. */
	//BindDependSymbol(module, peb);

	errors = LOADER_ERROR_OK;

__TERMINAL:
	if (errors != LOADER_ERROR_OK)
	{
		/* Something error. */
		if (hFile)
		{
			CloseFile(hFile);
		}
		/* Clear loaded flag. */
		module->module_flags &= ~MODULE_FLAG_LOADED;
	}
	return errors;
}

/*
 * Load all modules of an application.
 * It invokes the LoadModule routine one by one,
 * until all modules in queue is loaded or fail.
 */
unsigned long LoadAllModules(__PEB* peb)
{
	__MODULE_DESCRIPTOR* current_mod = NULL;
	unsigned long errors = LOADER_ERROR_OK;

	while (peb->mq_head < peb->mq_tail)
	{
		current_mod = &peb->mq[peb->mq_head];
		peb->mq_head++;
		//_hx_printf("[%s]Load module: %s\r\n", __func__, current_mod->module_name);
		errors = LoadModule(current_mod, peb);
		if (errors)
		{
			/* Error occurs, give up loading. */
			goto __TERMINAL;
		}
		if (peb->mq_head == 1)
		{
			/* 
			 * First module, i.e, the application 
			 * image itself. Use it initializes PEB
			 * farther.
			 */
			if (current_mod->module_flags & MODULE_FLAG_PE32PLUS)
			{
				peb->process_flags |= PROCESS_FLAGS_BIT64;
			}
			else {
				peb->process_flags &= (~PROCESS_FLAGS_BIT64);
			}
		}
	}
	_hx_printf("[%s]total %d module(s) loaded.\r\n", __func__, peb->mq_tail);

__TERMINAL:
	return errors;
}

/* Unload all loaded modules. */
unsigned long UnloadModules(__PEB* peb)
{
	unsigned long error = LOADER_ERROR_OK;
	
	/* Close the opend module files. */
	for (int i = 0; i < peb->mq_head; i++)
	{
		CloseFile(peb->mq[i].hFile);
	}

	return error;
}