//This program makes a virtual floppy image for Hello China,which is used to load Hello China in virtual PC environment.
//It reads the four binary images,bootsect.bin,realinit.bin,miniker.bin and master.bin,into memory,and then write to a file
//named vfloppy.vfd according to specific format.
//
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

//Macros to specify the source and destination file name.
#define BOOTSECT_NAME TEXT("BOOTSECT.BIN")
#define REALINIT_NAME TEXT("REALINIT.BIN")
#define MINIKER_NAME  TEXT("MINIKER.BIN")
#define MASTER_NAME   TEXT("MASTER.BIN")
#define VFLOPPY_NAME  TEXT("VFLOPPY.VFD")

//Constants to establish relationship between virtual floppy and physical floppy.
#define FD_HEAD_NUM 2     //2 headers for one floppy.
#define FD_TRACK_NUM 80   //80 tracks per header.
#define FD_SECTOR_NUM 18  //18 sectors per track.
#define FD_SECTOR_SIZE 512  //512 bytes per sector.

//The 1.4M floppy's size.
#define FD_SIZE (FD_HEAD_NUM * FD_TRACK_NUM * FD_SECTOR_NUM * FD_SECTOR_SIZE)
#define FD_TRACK_SIZE (FD_SECTOR_NUM * FD_SECTOR_SIZE)  //How many bytes in one track.

//This routine writes the source file to target file,which is the virtual floppy file.
static bool WriteVf(HANDLE hBootsect,    //Handle of the bootsect file.
					HANDLE hRealinit,    //Handle of the realinit file.
					HANDLE hMiniker,     //Handle of the miniker file.
					HANDLE hMaster,      //Handle of the master file.
					HANDLE hFloppy)      //Handle of the virtual floppy file.
{
	bool            bResult              = false;
	int             nBootsectStart       = 0;       //The bootsect resides in floppy's first sector,track 0 and header 0.
	int             nBootsectSize        = FD_SECTOR_SIZE;
	int             nRealinitStart       = 2 * FD_SECTOR_SIZE;  //The realinit resides in floppy's third sector,track 0 and hd 0.
	int             nRealinitSize        = 4096;    //Realinit.bin's size is 4K.
	int             nMinikerStart        = 10 * FD_SECTOR_SIZE; //Miniker resides in 11th sector,track 0 and header 0.
	int             nMinikerSize         = 48 * 1024;  //Miniker.bin's size is 48K.
	int             nMasterStart         = (7 * FD_SECTOR_NUM + 12) * FD_SECTOR_SIZE;  //Master resides in
	                                       //track 7,sector 13,and header 0.
	int             nMasterSize          = 560 * 1024;  //Master.bin's size is 560K.
	char*           pBuffer              = NULL;
	char*           p2Buffer             = NULL;
	char*           pBuffPtr             = NULL;
	char*           p2BuffPtr            = NULL;
	unsigned long   ulRead               = 0;
	unsigned long   ulToRead             = 0;
	int             i;

	//Check the parameters.
	if((INVALID_HANDLE_VALUE == hBootsect)   ||
	   (INVALID_HANDLE_VALUE == hRealinit)   ||
	   (INVALID_HANDLE_VALUE == hMiniker)    ||
	   (INVALID_HANDLE_VALUE == hMaster)     ||
	   (INVALID_HANDLE_VALUE == hFloppy))
	{
		printf(TEXT("Invalid parameter(s) encounted.\r\n"));
		goto __TERMINAL;
	}
	//Allocate temporary buffer.
	p2Buffer = (char*)malloc(FD_SIZE); //In Windows system,should successful.But if you like,you can allocate a small
	                                   //buffer improve the success probality,and use a little more complicated algorithm
	                                   //to implemente this program.
	if(NULL == p2Buffer)
	{
		printf(TEXT("Can not allocate enough memory.\r\n"));
		goto __TERMINAL;
	}
	pBuffer = (char*)malloc(FD_SIZE);  //In Windows system,should successful.:-)
	if(NULL == pBuffer)
	{
		printf(TEXT("Can not allocate enough memory.\r\n"));
		goto __TERMINAL;
	}
	memset(p2Buffer,0,FD_SIZE);  //Clear content of the buffer.
	
	//Now,read bootsect.bin into buffer.
	pBuffPtr = pBuffer + nBootsectStart;
	ulToRead = nBootsectSize;
	if(!ReadFile(hBootsect,
		pBuffPtr,
		ulToRead,
		&ulRead,
		NULL))  //Can not read.
	{
		printf(TEXT("Can not read bootsect.bin.\r\n"));
		goto __TERMINAL;
	}
	if(ulToRead != ulRead)  //File's size may incorrect.
	{
		printf(TEXT("bootsect.bin size may incorrect.\r\n"));
		goto __TERMINAL;
	}

	//Now,read realinit.bin into buffer.
	pBuffPtr = pBuffer + nRealinitStart;
	ulToRead = nRealinitSize;
	if(!ReadFile(hRealinit,
		pBuffPtr,
		ulToRead,
		&ulRead,
		NULL))
	{
		printf(TEXT("Can not read realinit.bin.\r\n"));
		goto __TERMINAL;
	}
	if(ulToRead != ulRead)  //File's size may incorrect.
	{
		printf(TEXT("realinit.bin size may incorrect.\r\n"));
		goto __TERMINAL;
	}

	//Now,read miniker.bin into buffer.
	pBuffPtr = pBuffer + nMinikerStart;
	ulToRead = nMinikerSize;
	if(!ReadFile(hMiniker,
		pBuffPtr,
		ulToRead,
		&ulRead,
		NULL))
	{
		printf(TEXT("Can not read miniker.bin.\r\n"));
		goto __TERMINAL;
	}
	if(ulToRead != ulRead)  //File's size may incorrect.
	{
		printf(TEXT("miniker.bin's size may incorrect.\r\n"));
		goto __TERMINAL;
	}

	//Now,read master.bin into buffer.
	pBuffPtr = pBuffer + nMasterStart;
	ulToRead = nMasterSize;
	if(!ReadFile(hMaster,
		pBuffPtr,
		ulToRead,
		&ulRead,
		NULL))
	{
		printf(TEXT("Can not read from master.bin.\r\n"));
		goto __TERMINAL;
	}
	//Because the master.bin's size may change,so no need to judge it's actually size.
	//But in any case,master.bin's size must larger or equal to 64K,so we ensure it.
	if(ulRead < 64 * 1024)
	{
		printf(TEXT("master.bin size may incorrect.\r\n"));
		goto __TERMINAL;
	}

	//I made a mistake originally,assumed the mapping relationship between physical floppy and virtual floppy file
	//is first track,second track,...,then another header. After several failures,I checked the documents related 
	//to the mapping relationship,found that it is first header,first track,then second header,first track.
	//So I adjust the track sequence in pBuffer to p2Buffer,to statisfy the correct mapping relationship.If you want
	//to write a more flexible program to create a virtual floppy image,please mind this.
	//Thank windows,we can allocate any size memory block,this lets my work simple,hehe.But you also can allocate
	//a big block of memory in Hello China.:-)
	p2BuffPtr = p2Buffer;
	pBuffPtr  = pBuffer;
	for(i = 0;i < FD_TRACK_NUM * FD_HEAD_NUM;i ++)
	{
		memcpy(p2BuffPtr,pBuffPtr,FD_TRACK_SIZE);
		pBuffPtr  += FD_TRACK_SIZE;
		if(FD_TRACK_NUM - 1 == i)
		{
			p2BuffPtr = p2Buffer + FD_TRACK_SIZE;
		}
		else
		{
			p2BuffPtr += FD_TRACK_SIZE * 2;
		}
	}
	
	//Write it to virtual floppy file.
	SetFilePointer(hFloppy,0,0,FILE_BEGIN);  //Write from begin.
	if(!WriteFile(hFloppy,
		p2Buffer,
		FD_SIZE,
		&ulRead,
		NULL))
	{
		printf(TEXT("Write to Vritual Floppy File failed.\r\n"));
		printf(TEXT("Please ensure no other process is using the virtual floppy file.\r\n"));
		goto __TERMINAL;
	}
	if(FD_SIZE != ulRead)  //Can not write the whole buffer to file,may has not enough disk size.
	{
		printf(TEXT("The byte mount written to virtual floppy file may incorrect.\r\n"));
		printf(TEXT("Please check the physical storage where the virtual floppy file residing has enough space.\r\n"));
		goto __TERMINAL;
	}
	SetEndOfFile(hFloppy);
	//If reach here,the whole operation is successfully.
	bResult = true;

__TERMINAL:
	if(pBuffer)
	{
		free(pBuffer);
	}
	if(p2Buffer)
	{
		free(p2Buffer);
	}
	return bResult;
}

//Main entry.
void main()
{
	HANDLE hBootsect     = INVALID_HANDLE_VALUE;
	HANDLE hRealinit     = INVALID_HANDLE_VALUE;
	HANDLE hMiniker      = INVALID_HANDLE_VALUE;
	HANDLE hMaster       = INVALID_HANDLE_VALUE;
	HANDLE hFloppy       = INVALID_HANDLE_VALUE;
	bool   bResult       = false;

	//Open all source files.
	hBootsect = CreateFile(BOOTSECT_NAME,
		GENERIC_READ,
		FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hBootsect)
	{
		goto __TERMINAL;
	}

	hRealinit = CreateFile(REALINIT_NAME,
		GENERIC_READ,
		FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hRealinit)
	{
		goto __TERMINAL;
	}

	hMiniker = CreateFile(MINIKER_NAME,
		GENERIC_READ,
		FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hMiniker)
	{
		goto __TERMINAL;
	}

	hMaster = CreateFile(MASTER_NAME,
		GENERIC_READ,
		FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hMaster)
	{
		goto __TERMINAL;
	}

	hFloppy = CreateFile(VFLOPPY_NAME,
		GENERIC_WRITE,
		FILE_SHARE_DELETE,
		NULL,
		OPEN_ALWAYS,
		0L,
		NULL);
	if(INVALID_HANDLE_VALUE == hFloppy)
	{
		goto __TERMINAL;
	}

	//Create virtual floppy file now.
	printf(TEXT("Writing to virtual floppy now...\r\n"));
	if(!WriteVf(hBootsect,
		hRealinit,
		hMiniker,
		hMaster,
		hFloppy))
	{
		goto __TERMINAL;
	}

	bResult = true;  //Mark the successful flag.

__TERMINAL:
	if(hBootsect != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hBootsect);
	}
	if(hRealinit != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hRealinit);
	}
	if(hMiniker != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hMiniker);
	}
	if(hMaster != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hMaster);
	}
	if(hFloppy != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFloppy);
	}
	if(!bResult)
	{
		printf(TEXT("  Create virtual floppy image file failed.\r\n"));
		printf(TEXT("  Please ensure the following file are in current directory:\r\n"));
		printf(TEXT("      bootsect.bin\r\n"));
		printf(TEXT("      realinit.bin\r\n"));
		printf(TEXT("      master.bin\r\n"));
		printf(TEXT("      miniker.bin\r\n"));
		printf(TEXT("  And make sure the virtual floppy image file is not used by other process.\r\n"));
	}
	else
	{
		printf("Create virtual floppy image file (vfloppy.vfd) successfully!\r\n");
	}
}
