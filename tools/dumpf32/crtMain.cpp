#include <stdio.h>
#include <windows.h>

static BOOL ReadSector(BYTE* pBuffer)
{
	BOOL bResult = FALSE;
	char* szDeviceName = "\\\\.\\C:";
	HANDLE hDevice = NULL;

	hDevice = CreateFile(
		szDeviceName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE == hDevice)  //Can not open device.
	{
		return bResult;
	}
	//Read device.
	DWORD dwRead = 0L;
	bResult = ReadFile(hDevice,pBuffer,512,&dwRead,NULL);
	CloseHandle(hDevice);
	return bResult;
}

#define BPB_SecPerClus 13  //Length = 1
#define BPB_RsvdSecCnt 14  //Length = 2
#define BPB_NumFATs    16  //Length = 1
#define BPB_HiddSec    28  //Length = 4
#define BPB_TotSec32   32  //Length = 4

#define BPB_FAT32_FATSz32 36 //Length = 4
#define BPB_FAT32_RootClus 44 //Length = 4

#define HDBS_SECTORSPERCLUSTER  0x03   //Offset of sectors per cluster in HDBS
#define HDBS_RESERVEDSECTORS    0x04   //Reserved sectors number.
#define HDBS_NUMBEROFFATS       0x06
#define HDBS_HIDDENSECTORS      0x07
#define HDBS_SECTORSPERFAT32    0x0B
#define HDBS_ROOTDIRSTART       0x0F

//A helper routine used to print out key information of sector 0.
static void DumpoutSector0(BYTE* pBuffer)
{
	printf("Sector per cluster : %d\r\n",*(BYTE*)(pBuffer + BPB_SecPerClus));
	printf("Reserved sector cnt: %d\r\n",*(WORD*)(pBuffer + BPB_RsvdSecCnt));
	printf("Number of FATs     : %d\r\n",*(BYTE*)(pBuffer + BPB_NumFATs));
	printf("Hidden sector cnt  : %d\r\n",*(DWORD*)(pBuffer + BPB_HiddSec));
	printf("Total sector number: %d\r\n",*(DWORD*)(pBuffer + BPB_TotSec32));
	printf("FAT size           : %d\r\n",*(DWORD*)(pBuffer + BPB_FAT32_FATSz32));
	printf("Root cluster num   : %d\r\n",*(DWORD*)(pBuffer + BPB_FAT32_RootClus));
}

//A helper routine used to modify hard disk boot sector according to actual configure.
static void Modifyhdbs(BYTE* pBuffer,BYTE* hdbs)
{
	*(BYTE*)(hdbs + HDBS_SECTORSPERCLUSTER) = *(BYTE*)(pBuffer + BPB_SecPerClus);
	*(WORD*)(hdbs + HDBS_RESERVEDSECTORS)   = *(WORD*)(pBuffer + BPB_RsvdSecCnt);
	*(BYTE*)(hdbs + HDBS_NUMBEROFFATS)      = *(BYTE*)(pBuffer + BPB_NumFATs);
	*(DWORD*)(hdbs + HDBS_HIDDENSECTORS)    = *(DWORD*)(pBuffer + BPB_HiddSec);
	*(DWORD*)(hdbs + HDBS_SECTORSPERFAT32)  = *(DWORD*)(pBuffer + BPB_FAT32_FATSz32);
	*(DWORD*)(hdbs + HDBS_ROOTDIRSTART)     = *(DWORD*)(pBuffer + BPB_FAT32_RootClus);
	return;
}

//A helper routine to replace sector_per_clusters,number_of_FATs and other parameters in BOOTSECT.DOS.
static void ReplaceParameters(BYTE* pBuffer)
{
	BYTE hdbs[512];
	DWORD dwRead = 0L;
	BOOL bResult = FALSE;

	HANDLE hBootsect = CreateFile(
		"BOOTSECT.DOS",
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if(INVALID_HANDLE_VALUE == hBootsect)
	{
		printf("Can not open bootsect.dos to write.\r\n");
		printf("Please make sure this file is in current directory.\r\n");
		goto __TERMINAL;
	}
	//Read the bootsect.dos file.
	bResult = ReadFile(hBootsect,hdbs,512,&dwRead,NULL);
	if(!bResult)
	{
		printf("Can not read bootsect.dos,please try this program again.\r\n");
		goto __TERMINAL;
	}
	//Modify BOOTSECT.DOS file.
	Modifyhdbs(pBuffer,hdbs);
	//Rewrite the modified bootsect.dos file back.
	SetFilePointer(hBootsect,0,NULL,FILE_BEGIN);  //Move to file's begining.
	bResult = WriteFile(
		hBootsect,
		hdbs,
		512,
		&dwRead,
		NULL);
	if(!bResult)
	{
		printf("Can not write to bootsect.dos,please try again.\r\n");
		goto __TERMINAL;
	}
	printf("Replace bootsect.dos parameters successfully.\r\n");
__TERMINAL:
	if(INVALID_HANDLE_VALUE != hBootsect)
	{
		CloseHandle(hBootsect);
	}
	return;
}

//Main entry.
void main()
{
	BYTE buffer[512];
	BOOL bResult = ReadSector(buffer);
	if(!bResult)
	{
		printf("Can not read sector0 of your partiton.\r\n");
		return;
	}
	DumpoutSector0(buffer);
	ReplaceParameters(buffer);
}