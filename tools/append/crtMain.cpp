//------------------------------------------------------------------------
//  Append another binary module into one original one,to create a binary
//  OS kernel image.
//  Author             : Garry
//  Date               : Mar 21,2009
//  Module name        : crtMain.cpp
//  Last modified date :
//  Last modified list :
//
//------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <windows.h>

//Print help information.
static void Usage()
{
	printf("\r\n");
	printf("  Usage: append -s original_file -a appended_file -b offset -o output_file\r\n");
	printf("  Where:\r\n");
	printf("    original_file  : The original file name and path to append to;\r\n");
	printf("    appended_file  : The target file to be append;\r\n");
	printf("    offset         : Offset of the appended file relative to original file's beginning.\r\n");
	printf("    output_file    : The combined target file name and path.\r\n");
	exit(0);
}

//
//Convert the string's low character to uper character.
//Such as,the input string is "abcdefg",then,the output
//string would be "ABCDEFG".
//
static VOID ConvertToUper(LPSTR pszSource)
{
	BYTE     bt        = 'a' - 'A';
	DWORD    dwIndex   = 0x0000;
	DWORD    dwMaxLen  = 512;

	if(NULL == pszSource)
		return;

	while(*pszSource)
	{
		if((*pszSource >= 'a') && ( *pszSource <= 'z'))
		{
			*pszSource -= bt;
		}
		pszSource ++;
		dwMaxLen --;
		if(0 == dwMaxLen)
			break;
	}

	return;
}

//
//Convert the string to hex number.
//If success,it returns TRUE,else,returns FALSE.
//
static BOOL Str2Hex(LPSTR pszSrc,DWORD* pdwResult)
{
	BOOL     bResult  = FALSE;
	DWORD    dwResult = 0x00000000;
	if((NULL == pszSrc) || (NULL == pdwResult))  //Parameters check.
		return bResult;

	if(strlen(pszSrc) > 8)                      //If the string's length is longer
		                                        //than the max hex number length.
	{
		return bResult;
	}
	ConvertToUper(pszSrc);

	while(*pszSrc)
	{
		dwResult <<= 4;
		switch(*pszSrc)
		{
		case '0':
			dwResult += 0;
			break;
		case '1':
			dwResult += 1;
			break;
		case '2':
			dwResult += 2;
			break;
		case '3':
			dwResult += 3;
			break;
		case '4':
			dwResult += 4;
			break;
		case '5':
			dwResult += 5;
			break;
		case '6':
			dwResult += 6;
			break;
		case '7':
			dwResult += 7;
			break;
		case '8':
			dwResult += 8;
			break;
		case '9':
			dwResult += 9;
			break;
		case 'A':
			dwResult += 10;
			break;
		case 'B':
			dwResult += 11;
			break;
		case 'C':
			dwResult += 12;
			break;
		case 'D':
			dwResult += 13;
			break;
		case 'E':
			dwResult += 14;
			break;
		case 'F':
			dwResult += 15;
			break;
		default:
			bResult = FALSE;
			return bResult;
		}
		pszSrc ++;
	}

	bResult    = TRUE;
	*pdwResult = dwResult;
	return bResult;
}

#define MASTER_LOAD_ADDR 0x00110000  //The base address of master.bin module.
#define ROUND_TO_64K(addr) ((addr % 65536) ? (addr + (65536 - (addr % 65536))) : addr)

//Main process routine of append.
static BOOL Append(HANDLE hOriginalFile,
				   HANDLE hAppendFile,
				   HANDLE hOutputFile,
				   ULONG nBaseAddress)
{
	DWORD   dwOriginalSize  = 0;
	DWORD   dwAppendSize    = 0;
	DWORD   dwOutputSize    = 0;
	DWORD   dwSizeHigh      = 0;
	char*   pBuffer         = NULL;
	BOOL    bResult         = FALSE;
	DWORD   dwAppendOffset  = 0;     //The offset of appended file in pBuffer.

	dwOriginalSize = GetFileSize(hOriginalFile,&dwSizeHigh);
	dwAppendSize   = GetFileSize(hAppendFile,&dwSizeHigh);
	//Calculate offset of appended file.
	if(0 == nBaseAddress)
	{
		dwAppendOffset = ROUND_TO_64K(dwOriginalSize);
	}
	else
	{
		dwAppendOffset = nBaseAddress;
		//if(dwAppendOffset < MASTER_LOAD_ADDR)  //Modified in Jan 5,2012.
		//{
		//	printf("  Invalid base address.\r\n");
		//	goto __TERMINAL;
		//}
		//dwAppendOffset -= MASTER_LOAD_ADDR;
		if(dwAppendOffset < dwOriginalSize)
		{
			printf("  Invalid base address,less than original file.\r\n");
			goto __TERMINAL;
		}
		dwAppendOffset = ROUND_TO_64K(dwAppendOffset);
	}
	//Allocate temporary buffer.
	dwOutputSize = dwAppendOffset + dwAppendSize;
	pBuffer = (char*)malloc(dwOutputSize);
	if(NULL == pBuffer)
	{
		printf("  Can not allocate memory.\r\n");
		goto __TERMINAL;
	}
	memset(pBuffer,0,dwOutputSize);

	//Now read the original and append file,then write them into output file.
	bResult = ReadFile(hOriginalFile,
		pBuffer,
		dwOriginalSize,
		&dwSizeHigh,
		NULL);
	if(!bResult)
	{
		printf("  Can not read the original file.\r\n");
		goto __TERMINAL;
	}
	bResult = ReadFile(hAppendFile,
		pBuffer + dwAppendOffset,
		dwAppendSize,
		&dwSizeHigh,
		NULL);
	if(!bResult)
	{
		printf("  Can not read the append file.\r\n");
		goto __TERMINAL;
	}
	//Write to output file.
	SetFilePointer(hOutputFile,0,NULL,FILE_BEGIN);
	bResult = WriteFile(hOutputFile,
		pBuffer,
		dwAppendOffset + dwAppendSize,
		&dwSizeHigh,
		NULL);
	SetEndOfFile(hOutputFile);  //Truck the target file.
	printf("  Appended base address : 0x%08X\r\n",dwAppendOffset + MASTER_LOAD_ADDR);
	printf("  Output file's length  : %d\r\n",dwAppendOffset + dwAppendSize);

__TERMINAL:
	if(pBuffer)
	{
		free(pBuffer);
	}
	return bResult;
}

//Main entry of append.
void main(int argc,char* argv[])
{
	char*    pszOriginalFile   = "master.bin";
	char*    pszAppendFile     = NULL;
	char*    pszOutputFile     = NULL;
	HANDLE   hOriginalFile     = INVALID_HANDLE_VALUE;
	HANDLE   hAppendFile       = INVALID_HANDLE_VALUE;
	HANDLE   hOutputFile       = INVALID_HANDLE_VALUE;
	ULONG    nBaseAddress      = 0;
	char*    pszBaseAddress    = NULL;

	//Process the argument(s) user specified.
	for(int i = 1;i < argc;i ++)
	{
		if(argv[i][0] == '-')
		{
			switch(argv[i][1])
			{
			case 's':
				i += 1;
				if(argc < i + 1)  //Too few parameters.
				{
					Usage();
				}
				else
				{
					pszOriginalFile = argv[i];
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
					pszOutputFile = argv[i];
				}
				break;
			case 'a':
				i += 1;
				if(argc < i + 1)
				{
					Usage();
				}
				else
				{
					pszAppendFile = argv[i];
				}
				break;
			case 'b':
				i += 1;
				if(argc < i + 1)
				{
					Usage();
				}
				else
				{
					pszBaseAddress = argv[i];
				}
				break;
			default:
				break;
			}
		}
	}

	//Ensure the appended file is specified.
	if(NULL == pszAppendFile)
	{
		Usage();
	}
	if(pszBaseAddress)  //Base address is specified.
	{
		if(!Str2Hex(pszBaseAddress,&nBaseAddress))
		{
			printf("  Please specify a valid base address.\r\n");
			goto __TERMINAL;
		}
	}
	//Now try to open the files.
	hOriginalFile = CreateFile(
		pszOriginalFile,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE == hOriginalFile)  //Can not open file.
	{
		printf("  Can not open the original file.\r\n");
		goto __TERMINAL;
	}
	hAppendFile = CreateFile(
		pszAppendFile,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE == hAppendFile)
	{
		printf("  Can not open the append file.\r\n");
		goto __TERMINAL;
	}
	if(NULL == pszOutputFile)
	{
		hOutputFile = hOriginalFile;
	}
	else
	{
		hOutputFile = CreateFile(
			pszOutputFile,
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			0,
			NULL);
		if(INVALID_HANDLE_VALUE == hOutputFile)
		{
			printf("  Can not open the specified output file.\r\n");
			goto __TERMINAL;
		}
	}
	//All arguments are ok,call main process routine.
	if(Append(hOriginalFile,hAppendFile,hOutputFile,nBaseAddress))
	{
		printf("  Combine file successfully!\r\n");
	}
	else
	{
		printf("  Append file failed.\r\n");
	}
__TERMINAL:
	if(INVALID_HANDLE_VALUE != hOriginalFile)
	{
		CloseHandle(hOriginalFile);
	}
	if(INVALID_HANDLE_VALUE != hAppendFile)
	{
		CloseHandle(hAppendFile);
	}
	if(INVALID_HANDLE_VALUE != hOutputFile)
	{
		CloseHandle(hOutputFile);
	}
}
