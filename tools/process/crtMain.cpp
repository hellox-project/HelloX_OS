#include "stdio.h"
#include "windows.h"
#include "stdlib.h"

typedef struct __tagFILE_HEADER{
	unsigned char ucNop[4];
	DWORD         dwJmpAddr;
}__FILL_HEADER;

__FILL_HEADER g_FillHeader = {0x90,0x90,0x90,0xe9,0x00000000};    //This structure will be
                                                                  //written to target file.

//Work routine of process.
BOOL Process(HANDLE hSource,HANDLE hTarget)
{
	IMAGE_DOS_HEADER*       ImageDosHeader = NULL;
	IMAGE_NT_HEADERS*       ImageNtHeader = NULL;
	IMAGE_OPTIONAL_HEADER*  ImageOptionalHeader = NULL;
	DWORD                   dwFileSize  = 0;
	DWORD                   dwFileSizeH = 0;
	CHAR*                   pBuffer     = NULL;
	BOOL                    bResult = FALSE;
	DWORD                   dwActualBytes = 0L;
	DWORD                   dwOffset = 0L;
	UCHAR*                  lpucSource = NULL;
	UCHAR*                  lpucDes    = NULL;
	DWORD                   dwLoop     = 0;

	dwFileSize = GetFileSize(hSource,&dwFileSizeH);
	if(0 == dwFileSize)
	{
		goto __TERMINAL;
	}
	pBuffer = (CHAR*)malloc(dwFileSize);
	if(NULL == pBuffer)
	{
		printf("  Can not allocate enough memory.\r\n");
		goto __TERMINAL;
	}
	//Read the whole file.
	bResult = ReadFile(hSource,pBuffer,dwFileSize,&dwActualBytes,NULL);
	if(!bResult)
	{
		goto __TERMINAL;
	}

	//
	//The following code locates the entry point of the PE file,and modifies it.
	//
	ImageDosHeader = (IMAGE_DOS_HEADER*)pBuffer;
	dwOffset = ImageDosHeader->e_lfanew;

	ImageNtHeader = (IMAGE_NT_HEADERS*)(pBuffer + dwOffset);
	ImageOptionalHeader = &(ImageNtHeader->OptionalHeader);
	g_FillHeader.dwJmpAddr = ImageOptionalHeader->AddressOfEntryPoint;
	printf("  Entry Point      : 0x%08X\r\n",ImageOptionalHeader->AddressOfEntryPoint);
	printf("  Section Aligment : 0x%08X\r\n",ImageOptionalHeader->SectionAlignment);
	printf("  File Aligment    : 0x%08X\r\n",ImageOptionalHeader->FileAlignment);

	g_FillHeader.dwJmpAddr -= sizeof(__FILL_HEADER);    //Calculate the target address will
	                                                    //jump to.
	                                                    //Because we have added some nop instruc-
	                                                    //tions in front of the target file,so
	                                                    //we must adjust it.

	lpucSource = (UCHAR*)&g_FillHeader.ucNop[0];
	lpucDes    = (UCHAR*)pBuffer;

	for(dwLoop = 0;dwLoop < sizeof(__FILL_HEADER);dwLoop ++)  //Modify the target file's header.
	{
		*lpucDes = *lpucSource;
		lpucDes ++;
		lpucSource ++;
	}

	bResult = WriteFile(hTarget,pBuffer,dwFileSize,&dwActualBytes,
		NULL);

__TERMINAL:
	if(pBuffer)
	{
		free(pBuffer);
	}
	return bResult;
}

//Print out help information.
static void Usage()
{
	printf("    Usage: process -i input_file_name -o output_file_name\r\n");
	printf("    Where:\r\n");
	printf("      input_file_name  : The file name and path to be processed;\r\n");
	printf("      output_file_name : The output file's name and path.\r\n");
	exit(0);
}

//Main entry of process.
void main(int argc,char* argv[])
{
	char*   pszSourceFile  = "master.dll";
	char*   pszDestFile    = "master.bin";
	HANDLE  hSourceFile    = INVALID_HANDLE_VALUE;
	HANDLE  hDestFile      = INVALID_HANDLE_VALUE;

	//Process argument(s).
	for(int i = 1;i < argc;i ++)
	{
		if('-' == argv[i][0])
		{
			switch(argv[i][1])
			{
			case 'i':
				i += 1;
				if(argc < i + 1)
				{
					Usage();
				}
				else
				{
					pszSourceFile = argv[i];
				}
				break;
			case 'o':
				i += 1;
				if(argc < i + 1)
				{
					Usage();
				}
				else
				{
					pszDestFile = argv[i];
				}
				break;
			default:
				Usage();
				break;
			}
		}
	}
	hSourceFile = CreateFile(                //Open the input file.
		pszSourceFile,
		GENERIC_READ,
		0L,
		NULL,
		OPEN_EXISTING,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hSourceFile)
	{
		printf("  Can not open the target file to read.\r\n");
		goto __TERMINAL;
	}
	hDestFile = CreateFile(
		pszDestFile,
		GENERIC_WRITE,
		0L,
		NULL,
		OPEN_ALWAYS,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hDestFile)
	{
		printf("  Can not open the specified target file.\r\n");
		goto __TERMINAL;
	}
	//Now launch the process routine.
	if(!Process(hSourceFile,hDestFile))
	{
		printf("  Can not process the specified file.\r\n");
		goto __TERMINAL;
	}
	printf("  \r\nProcess the specified file successfully!\r\n");
__TERMINAL:
	if(hSourceFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSourceFile);
	}
	if(hDestFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDestFile);
	}
}
