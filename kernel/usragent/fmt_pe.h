//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 1, 2019
//    Module Name               : fmt_pe.h
//    Module Funciton           : 
//                                Structures and definitions for PE format
//                                file.The user agent will load the user
//                                specified PE file and run it as process.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __FMT_PE__
#define __FMT_PE__

// Directory Entries
#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//define IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

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

/* Magic value of DOS header,must be 'MZ'. */
#define PE_DOS_HEADER_MAGIC 0x5A4D

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

/* Machine types. */
#define IMAGE_FILE_MACHINE_I386   0x14C
#define IMAGE_FILE_MACHINE_IA64   0x200
#define IMAGE_FILE_MACHINE_ARM    0x1C0
#define IMAGE_FILE_MACHINE_ARM64  0xAA64

/* File characteristics. */
#define IMAGE_FILE_EXECUTABLE_IMAGE  0x0002
#define IMAGE_FILE_32BIT_MACHINE     0x0100
#define IMAGE_FILE_SYSTEM            0x1000
#define IMAGE_FILE_DLL               0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY    0x4000

/* Data directories. */
typedef struct _IMAGE_DATA_DIRECTORY
{
	DWORD   VirtualAddress;
	DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _IMAGE_BASE_RELOCATION {
	DWORD   VirtualAddress;
	DWORD   SizeOfBlock;
	//  WORD    TypeOffset[1];
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

/*
 * Optional header format.
 * This header onlly present when the file is executable,
 * without present if the file is COFF,so it's named
 * optional header.
 */
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

/* Magic value,only PE32 is supported now. */
#define PE_OPTIONAL_HEADER_MAGIC_PE32PLUS 0x20b
#define PE_OPTIONAL_HEADER_MAGIC_PE32     0x10b

typedef struct _IMAGE_NT_HEADERS
{
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

/* PE signature,'PE\0\0'. */
#define PE_NT_HEADER_SIGNATURE 0x00004550

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

/* Section flags,i.e,Characteristics in section header. */
#define IMAGE_SCN_TYPE_NO_PAD             0x00000008
#define IMAGE_SCN_CNT_CODE                0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA    0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA  0x00000080
#define IMAGE_SCN_LINK_OTHER              0x00000100
#define IMAGE_SCN_LINK_INFO               0x00000200
#define IMAGE_SCN_MEM_NOT_CACHED          0x04000000
#define IMAGE_SCN_MEM_NOT_PAGED           0x08000000
#define IMAGE_SCN_MEM_SHARED              0x10000000
#define IMAGE_SCN_MEM_EXECUTE             0x20000000
#define IMAGE_SCN_MEM_READ                0x40000000
#define IMAGE_SCN_MEM_WRITE               0x80000000

/* Base relocation types. */
#define IMAGE_REL_BASED_ABSOLUTE    0x00
#define IMAGE_REL_BASED_HIGH        0x01
#define IMAGE_REL_BASED_LOW         0x02
#define IMAGE_REL_BASED_HIGHLOW     0x03
#define IMAGE_REL_BASED_HIGHADJ     0x04

/* 
 * Maximal PE file header's size. 
 * Header's size exceed this value will be
 * rejected to run by HelloX currently,
 * this sould be a extremely seldom scenario.
 */
#define MAX_PE_HEADER_SIZE 4096

/* Entry point prototype of PE image. */
typedef int (*__PE_ENTRY_POINT)(int argc, char* argv[]);

#endif //__FMT_PE__
