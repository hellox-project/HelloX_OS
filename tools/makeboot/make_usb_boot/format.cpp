#include "stdafx.h"
#include "make_usb_boot.h"


#define  FS_VOLUME   "HX_SYSTEM"

typedef struct BPB_FAT32BS
{  //BPB and fat32 boot sector.
	UCHAR      BS_jmpBoot1;
	UCHAR      BS_jmpBoot2;
	UCHAR      BS_jmpBoot3;

	BYTE       BS_OEMName[8];
	UCHAR      BPB_BytsPerSec[2];
	BYTE       BPB_SecPerClus;
	UCHAR      BPB_RsvdSecCnt[2];
	BYTE       BPB_NumFATs;
	UCHAR      BPB_RootEntCnt[2];
	UCHAR      BPB_TotSec16[2];
	UCHAR      BPB_Media;
	UCHAR      BPB_FATSz16[2];
	UCHAR      BPB_SecPerTrk[2];
	UCHAR      BPB_NumHeads[2];
	UCHAR      BPB_HiddSec[4];
	UCHAR      BPB_TotSec32[4];

	//From here is the BS for FAT32. FAT16/12 is skiped in current version.
	UCHAR      BPB_FATSz32[4];
	UCHAR      BPB_ExtFlags[2];
	UCHAR      BPB_FSVer[2];
	UCHAR      BPB_RootClus[4];
	UCHAR      BPB_FSInfo[2];
	UCHAR      BPB_BkBootSec[2];
	UCHAR      BPB_Reserved[12];
	UCHAR      BS_DrvNum;
	UCHAR      BS_Reserved1;
	UCHAR      BS_BootSig;
	UCHAR      BS_VolID[4];
	UCHAR      BS_VolLab[11];
	UCHAR      BS_FilSysType[8];
}__BPB_FAT32BS;


//Definition for FAT32 directory short entry.
typedef struct FAT32_SHORTENTRY{
	BYTE       FileName[11];
	BYTE       FileAttributes;
	BYTE       NTRsved;
	BYTE       CreateTimeTenth;
	WORD       CreateTime;
	WORD       CreateDate;
	WORD       LastAccessDate;
	WORD       wFirstClusHi;
	WORD       WriteTime;
	WORD       WriteDate;
	WORD       wFirstClusLow;
	DWORD      dwFileSize;
}__FAT32_SHORTENTRY;



//Typical instance of __BPB_FAT32BS,in most case of format,only modify several variables
//of this instance and write it into first sector of target partition is ok.
static __BPB_FAT32BS Fat32Sector0 = {
	0xEB,                               //BS_jmpBoot1,2,3
	0x00,
	0x90,

	{'M','S','W','I','N','4','.','1'},  //BS_OEMName,get these values from MS's FAT32 SPEC.
	{0x00,0x02},                        //Bytes per sector,512.
	0,                                  //Sector per cluster,should be determined when execute format.
	{0,0},                             //Reserved sector counter,32/1024 is get from MS's FAT32 SPEC. 
	2,                                  //Number of FAT.
	{0,0},                              //Root entry counter,0 for FAT32.
	{0,0},                              //Total sector 16,0 for FAT32.
	0xF8,                               //Media type,0xF8 for fixed.
	{0,0},                              //FAT size 16,0 for FAT32.
	{0,0},                              //Sector per track.
	{0,0},                              //Number of heads.
	{0,0,0,0},                          //Hidden sector counter.
	{0,0,0,0},                          //Total sector 32,should be modified when format.

	{0,0,0,0},                          //FAT size 32,should be modified when format.
	{0,0},                              //Extend flags,mirroring at run time.
	{0,0},                              //FS Version.
	{2,0,0,0},                          //Root directory cluster number,modified later.
	{1,0},                              //FSInfo sector number,default is 1.
	{6,0},                              //Backup sector number for boot sector,6 is default.
	{0},                                //Reserved for FAT32.
	0,                                  //Driver number for int 13h call in DOS.
	0,                                  //Reserved1.
	0x29,                               //Boot signature.
	{0,0,0,0},                          //Volume ID,modified late.
	{'N','O',' ','N','A','M','E',' ',' ',' ',' '},  //Volume label.
	{'F','A','T','3','2',' ',' ',' '}   //File system type,no meaning.
};



//Calculate sector number occupied by one FAT.This is a local helper routine,only suitable for FAT32.
//This algorithm is copied from MS's FAT32 SPEC,and only keep the FAT32 related part.
static DWORD FatSize32(DWORD dwDiskSect,DWORD dwReservedSect,DWORD dwSecPerClus,DWORD dwNumFats)
{
	DWORD    TmpVal1 = dwDiskSect - dwReservedSect;
	DWORD    TmpVal2 = 256 * dwSecPerClus + dwNumFats;

	//if(FATType == FAT32)
	TmpVal2 = TmpVal2 / 2;
	return (TmpVal1 + TmpVal2 - 1) / TmpVal2;
}

BOOL APIENTRY WritePationInfo(HANDLE hDiskDrive,DWORD dwSecPerClus,DWORD dwFatStart,DWORD dwFatSize,DWORD dwSectorCount,UINT nReserved)
{
	BYTE      Buffer[HX_SECTOR_SIZE] = {0};
	LPDWORD   pdwTmp              = NULL;
	LPWORD    pwTmp              = NULL;
	DWORD     dwWrite             = 0;


	//Initialize BPB_SecPerClus of Fat32Sector0.
	Fat32Sector0.BPB_SecPerClus = (BYTE)dwSecPerClus;

	pwTmp   = (LPWORD)&Fat32Sector0.BPB_RsvdSecCnt[0];
	*pwTmp  = (WORD)dwFatStart;

	//Initialize BPB_TotSec32.
	pdwTmp  = (DWORD*)&Fat32Sector0.BPB_TotSec32[0];
	*pdwTmp = dwSectorCount;

	//Initialize BPB_FATSz32.
	pdwTmp     = (DWORD*)&Fat32Sector0.BPB_FATSz32[0];
	*pdwTmp    = dwFatSize;	

	//Initialize BS_VolID.
	Fat32Sector0.BS_VolID[0]    = 0x0A;
	Fat32Sector0.BS_VolID[1]    = 0x0A;
	Fat32Sector0.BS_VolID[2]    = 0x0A;
	Fat32Sector0.BS_VolID[3]    = 0x0A;

	ZeroMemory(Buffer,512);
	memcpy((char*)&Buffer[0],(const char*)&Fat32Sector0,sizeof(Fat32Sector0));
	//Set the 0x55 and 0xAA flags.
	Buffer[510] = (UCHAR)0x55;
	Buffer[511] = (UCHAR)0xAA;

	SetFilePointer(hDiskDrive,(nReserved*HX_SECTOR_SIZE),NULL,FILE_BEGIN);
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
	if(dwWrite < sizeof(Buffer))
	{
		return FALSE;
	}

	//write back 
	SetFilePointer(hDiskDrive,(nReserved+6)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	dwWrite = 0;
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
		

	return (dwWrite == sizeof(Buffer));
}

BOOL APIENTRY WriteRootDirInfo(HANDLE hDiskDrive,DWORD dwSecPerClus,DWORD dwRootStart,UINT nReserved)
{
	__FAT32_SHORTENTRY*   pfse    = NULL; 
	BYTE      Buffer[HX_SECTOR_SIZE] = {0};
	DWORD     dwWrite             = 0;	
	DWORD     i                   = 0;

	pfse = (__FAT32_SHORTENTRY*)Buffer;
	pfse->CreateDate       = 0;
	pfse->CreateTime       = 0;
	pfse->CreateTimeTenth  = 0;
	pfse->dwFileSize       = 0;
	pfse->FileAttributes   = FILE_ATTR_VOLUMEID;
	pfse->LastAccessDate   = 0;
	pfse->wFirstClusHi     = 0xFFFF;
	pfse->wFirstClusLow    = 0xFFFF;

	CopyMemory(pfse->FileName,FS_VOLUME,11);

	SetFilePointer(hDiskDrive,(dwRootStart+nReserved)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
	if(dwWrite < sizeof(Buffer))
	{
		return FALSE;
	}

	ZeroMemory(Buffer,HX_SECTOR_SIZE);	
	for(i = 1;i < dwSecPerClus;i ++)
	{		
		WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);	
		if(dwWrite < sizeof(Buffer))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL APIENTRY WriteFatTab(HANDLE hDiskDrive ,DWORD dwFatStart,DWORD dwFatSize,UINT nReserved)
{
	BYTE      Buffer[HX_SECTOR_SIZE] = {0};
	DWORD     dwWrite             = 0;
	DWORD*    pFatEntry           = NULL;
	DWORD     i                   = 0;
	
	
	pFatEntry = (DWORD*)&Buffer[0];
	*pFatEntry = 0x0FFFFFF8;//0xFFFFFFFF;
	pFatEntry ++;
	*pFatEntry = 0xFFFFFFFF;
	pFatEntry ++;
	*pFatEntry = 0x0FFFFFFF;

	SetFilePointer(hDiskDrive,(dwFatStart+nReserved)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
	if(dwWrite < sizeof(Buffer))
	{
		return FALSE;
	}

	SetFilePointer(hDiskDrive,(dwFatStart+nReserved+dwFatSize)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
	if(dwWrite < sizeof(Buffer))
	{
		return FALSE;
	}


	ZeroMemory(Buffer,HX_SECTOR_SIZE);
	SetFilePointer(hDiskDrive,(dwFatStart+nReserved+1)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	for(i = 1;i < dwFatSize;i ++)
	{	
		WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
		if(dwWrite < sizeof(Buffer))
		{
			return FALSE;
		}
	}

	SetFilePointer(hDiskDrive,(dwFatStart+nReserved+dwFatSize+1)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	for(i = 1;i < dwFatSize;i ++)
	{
		WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
		if(dwWrite < sizeof(Buffer))
		{
			return FALSE;
		}
	}

	return (dwWrite == sizeof(Buffer));
}

BOOL APIENTRY WriteFsInfo(HANDLE hDiskDrive ,DWORD dwStartSector,UINT nReserved)
{
	BYTE      Buffer[HX_SECTOR_SIZE] = {0};
	DWORD     dwWrite             = 0;
	LPDWORD   pdwTmp              = NULL;


	//write fs info
	SetFilePointer(hDiskDrive,(dwStartSector+nReserved)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	ZeroMemory(Buffer,sizeof(Buffer));
	pdwTmp  = (LPDWORD)&Buffer[0];
	*pdwTmp = 0x41615252;    

	pdwTmp  = (LPDWORD)&Buffer[484];
	*pdwTmp = 0x61417272;

	pdwTmp  = (LPDWORD)&Buffer[488];
	*pdwTmp = 0xFFFFFFFF;
	Buffer[492] = 2;
	Buffer[510] = (UCHAR)0x55;
	Buffer[511] = (UCHAR)0xAA;
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);
	if(dwWrite < sizeof(Buffer))
	{
		return FALSE;
	}

	SetFilePointer(hDiskDrive,(dwStartSector+nReserved+6)*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	WriteFile(hDiskDrive,Buffer,sizeof(Buffer),&dwWrite,NULL);

	return (dwWrite == sizeof(Buffer));
}

BOOL APIENTRY Fat32Format(HANDLE  hDiskDrive,UINT nSectorCount,UINT nReserved)
{	
	DWORD    dwFatSize      = HX_FAT32_FATSIZE;
	DWORD    dwFatStart     = HX_FAT32_RESERVED;
	DWORD    dwSecPerClus   = HX_FAT32_SECPERCLUS;   	
	BOOL     bFormatOk      = FALSE;
		

	while(bFormatOk == FALSE)
	{
		//fat32 pation info
		if(WritePationInfo(hDiskDrive,dwSecPerClus,dwFatStart,dwFatSize,nSectorCount,nReserved) == FALSE)
		{
			break;
		}

		//write fs info
		if(WriteFsInfo(hDiskDrive,1,nReserved) == FALSE)
		{
			break;
		}

		//Write the FAT table into partition,the first three entry(cluster 0,1,2) are initialized to EOC.
		if(WriteFatTab(hDiskDrive,dwFatStart,dwFatSize,nReserved) == FALSE)
		{
			break;
		}

		//Now initialize the root directory,assume the start cluster number of root is 2.	
		if(WriteRootDirInfo(hDiskDrive,dwSecPerClus,dwFatStart + dwFatSize * 2,nReserved) == FALSE)
		{
			break;
		}

		bFormatOk = TRUE;
	}

		
	return bFormatOk;
}
