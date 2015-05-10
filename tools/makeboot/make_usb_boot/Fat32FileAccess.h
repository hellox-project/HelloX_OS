
#pragma once

#include "resource.h"


#define FILE_SYSTEM_TYPE_FAT32   0x0B

//Definitions for FAT32 file system.
// Boot Sector
#define BS_jmpBoot				0	// Length = 3
#define BS_OEMName				3	// Length = 8
#define BPB_BytsPerSec			11	// Length = 2
#define BPB_SecPerClus			13	// Length = 1
#define BPB_RsvdSecCnt			14	// Length = 2
#define BPB_NumFATs				16	// Length = 1
#define BPB_RootEntCnt			17	// Length = 2
#define BPB_TotSec16			19	// Length = 2
#define BPB_Media				21	// Length = 1
#define	BPB_FATSz16				22	// Length = 2
#define BPB_SecPerTrk			24	// Length = 2
#define BPB_NumHeads			26	// Length = 2
#define BPB_HiddSec				28	// Length = 4
#define BPB_TotSec32			32	// Length = 4

// FAT 12/16
#define BS_FAT_DrvNum			36	// Length = 1
#define BS_FAT_BootSig			38	// Length = 1
#define BS_FAT_VolID			39	// Length = 4
#define BS_FAT_VolLab			43	// Length = 11
#define BS_FAT_FilSysType		54	// Length = 8

// FAT 32
#define BPB_FAT32_FATSz32		36	// Length = 4
#define BPB_FAT32_ExtFlags		40	// Length = 2
#define BPB_FAT32_FSVer			42	// Length = 2
#define BPB_FAT32_RootClus		44	// Length = 4
#define BPB_FAT32_FSInfo		48	// Length = 2
#define BPB_FAT32_BkBootSec		50	// Length = 2
#define BS_FAT32_DrvNum			64	// Length = 1
#define BS_FAT32_BootSig		66	// Length = 1
#define BS_FAT32_VolID			67	// Length = 4
#define BS_FAT32_VolLab			71	// Length = 11
#define BS_FAT32_FilSysType		82	// Length = 8


//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20
#define FILE_ATTR_LONGNAME    0x0F //Combination of READONLY/HIDDEN/SYSTEM/VOLUMEID

#define FAT_CACHE_LENGTH 1024                  //Fat entry cache,default is 1K.
#define IS_EOC(clus) ((clus) >= 0x0FFFFFF8)    //Check if the cluster value is EOC.
#define EOC 0x0FFFFFFFF                        //EOC for Hello China.
#define IS_EMPTY_CLUSTER_ENTRY(ce) (0 == (ce)) //Check if the cluster entry is empty.

#define  FAT32_DATETIME_CREATE         0x1
#define  FAT32_DATETIME_WRITE          0x2
#define  FAT32_DATETIME_ACCEPT         0x4

#define  FAT32_SHORTDIR_FILENAME_LEN   11     //Fat32 short entry file name len .        
#define  FAT32_LONGDIR_FILENAME_LEN    13     //Fat32 long entry  file name len .        

//Definition for FAT32 directory short entry.
typedef struct FAT32_SHORTENTRY
{
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

//Definition for FAT32 directory long entry.
typedef struct FAT32_LONGENTRY
{
	BYTE       LongNum[1];
	BYTE       szName1[10];
	BYTE       LongFlage;
	BYTE       Rsved1;
	BYTE       Checksum;
	BYTE       szName2[12];
	WORD       Rsved2;
	BYTE       szName3[4];

}__FAT32_LONGENTRY;