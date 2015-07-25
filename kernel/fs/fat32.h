//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 18 DEC, 2008
//    Module Name               : FAT32.H
//    Module Funciton           : 
//                                This module countains the pre-definition of FAT32
//                                file system.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __FAT32_H__
#define __FAT32_H__

//Display name of FAT32 file system driver.
#define FAT32_DRIVER_DEVICE_NAME "\\\\.\\FS_FAT32"    //Name of FAT32 driver device object.
#define FAT32_DEVICE_NAME_BASE   "\\\\.\\FAT32_DEV0"  //Base name of FAT32 file system.
#define FAT32_FILE_NAME_BASE     "\\\\.\\FAT_FILE0"   //File device name base.

#define SECTOR_SIZE 512    //Only can handle partition with 512 bytes sector currently.

//Entry point for FAT32 driver.
BOOL FatDriverEntry(__DRIVER_OBJECT* lpDriverObject);

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


//Definition for FAT32 directory short entry.
typedef struct FAT32_SHORTENTRY{
	CHAR       FileName[11];
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

#define  FAT32_SHORTDIR_FILENAME_LEN   11     //Fat32 short entry file name len .        
#define  FAT32_SHORTDIR_PREFIX_LEN      6     //Fat32 short entry file name len no include ext  .       
#define  FAT32_LONGDIR_FILENAME_LEN    13     //Fat32 long  entry  file name len .        
#define  FAT32_STANDARDEXT_NAME_LEN    3      //Fat32 file Standard extension name len .  

       
//FAT32 file object.
struct FAT32_FS;
typedef struct FAT32_FILE{
	CHAR       FileName[13];     //Zero terminated file name.
	CHAR       FileExtension[4]; //Zero terminated file extension.
	BYTE       Attributes;       //File attributes.
	DWORD      dwStartClusNum;   //Start cluster number of this file.
	DWORD      dwCurrClusNum;    //Current cluster number.
	DWORD      dwCurrPos;        //Current file pointer position.
	DWORD      dwClusOffset;     //Current offset in current cluster.
	DWORD      dwFileSize;       //File size.
	DWORD      dwOpenMode;       //Opened mode,such as READ,WRITE,etc.
	DWORD      dwShareMode;      //Share mode.
	BOOL       bInRoot;          //If in root directory.
	DWORD      dwParentClus;     //Parent directory's cluster number.
	DWORD      dwParentOffset;   //File short entry's offset in directory.

	struct FAT32_FS*          pFileSystem;    //File system this file belong to.
	__COMMON_OBJECT*   pFileCache;     //File cache object.
	__COMMON_OBJECT*   pPartition;     //Partition this file belong to.
	struct FAT32_FILE*        pNext;          //Pointing to next one.
	struct FAT32_FILE*        pPrev;          //Pointing to previous one.
}__FAT32_FILE;

#define FAT_CACHE_LENGTH 1024                  //Fat entry cache,default is 1K.
#define IS_EOC(clus) ((clus) >= 0x0FFFFFF8)    //Check if the cluster value is EOC.
#define EOC 0x0FFFFFFFF                        //EOC for Hello China.
#define IS_EMPTY_CLUSTER_ENTRY(ce) (0 == (ce)) //Check if the cluster entry is empty.

//FAT32 file system object.
typedef struct FAT32_FS{
	__COMMON_OBJECT*    pPartition;      //Partition this file system based.
	DWORD               dwAttribute;     //File system attributes.
    BYTE                SectorPerClus;   //Sector per cluster.
	CHAR                VolumeLabel[13]; //Volume lable.
	BYTE                FatNum;          //How many FAT in this volume.
	BYTE                Reserved;        //Reserved to align DWORD.
	WORD                wReservedSector; //Reserved sector number.
	WORD                wFatInfoSector;  //FAT information sector number.
	DWORD               dwBytePerSector; //Byte number per sector.
	DWORD               dwPartitionSatrt; //Partition start sector
	DWORD               dwClusterSize;   //Byte number of one cluster.
	DWORD               dwDataSectorStart;          //Start sector number of data.
	DWORD               dwRootDirClusStart;         //Start cluster number of root dir.
	DWORD               dwFatBeginSector;           //Start sector number of FAT.
	DWORD               dwFatSectorNum;             //Sector number per FAT.
	DWORD               FatCache[FAT_CACHE_LENGTH]; //FAT entry cache.
	//FAT32_FS*           pPrev;                      //Pointing to previous one.
	//FAT32_FS*           pNext;                      //Pointing to next one.
	__FAT32_FILE*       pFileList;                  //File list header.
}__FAT32_FS;

//Types to maintain the directory searching context.
//When FindFirstFile is called,FAT32 code will create one of this object to
//track the searching of the directory,and returns to user.This object must be
//transfered to FAT32 code by the subsequent calling,such as FindNextFile and
//FindClose.
//A subordinate type is defined to main the directory clusters.
typedef struct tag__FAT32_DIR_CLUSTER{
	BYTE*         pCluster;     //Pointing to cluster buffer.
	struct tag__FAT32_DIR_CLUSTER* pNext; //Pointing to next one.
}__FAT32_DIR_CLUSTER;

typedef struct FAT32_FIND_HANDLE{
	DWORD                      dwClusterSize;     //Cluster size.
	DWORD                      dwClusterOffset;   //Current position in one cluster.
	__FAT32_DIR_CLUSTER*       pClusterRoot;      //Root of cluster list.
	__FAT32_DIR_CLUSTER*       pCurrCluster;      //Current cluster block.
}__FAT32_FIND_HANDLE;

//Globa routines used by FAT32 driver code.
BOOL ReadDeviceSector(__COMMON_OBJECT* pPartition,DWORD dwStartSectorNum,
					  DWORD dwSector,BYTE* pBuffer);     //Read one or several sectors.
BOOL WriteDeviceSector(__COMMON_OBJECT* pPartition,DWORD dwStartSectorNum,
					   DWORD dwSectorNum,BYTE* pBuffer); //Write one or several sectors.
BOOL Fat32Init(__FAT32_FS* pFatFs,BYTE* pSector0);       //Initialize FAT32 file system.

BOOL GetNextCluster(__FAT32_FS* pFat32Fs,DWORD* pdwCluster);  //Get next cluster given current 1.

VOID  CombinLongFileName(__FAT32_LONGENTRY** plongEntry,INT nLongEntryNum, CHAR* pFileFullName);//combin file long name to full name

//Get one free cluster and mark the cluster as used.
//If find,TRUE will be returned and the cluster number will be set in pdwFreeCluster,
//else FALSE will be returned without any changing to pdwFreeCluster.
BOOL GetFreeCluster(__FAT32_FS* pFat32Fs, DWORD dwStartToFind, DWORD* pdwFreeCluster);

DWORD GetClusterSector(__FAT32_FS* pFat32Fs,DWORD dwCluster); //Get cluster's start sector no.
BOOL  ConvertName(__FAT32_SHORTENTRY* pfse,BYTE* pResult);     //Convert to regular file name string.

VOID  AddSpace(CHAR* pStrBuf,INT nCount);                      // add string end space 
BOOL  GetShortEntry(__FAT32_FS* pFat32Fs,
				   DWORD dwStartCluster,
				   CHAR* pFileName,
				   __FAT32_SHORTENTRY* pShortEntry,
				   DWORD* pDirClus,
				   DWORD* pDirOffset);          //Get the short entry by name.
BOOL GetDirEntry(__FAT32_FS* pFat32Fs,
				 CHAR* pFullName,
				 __FAT32_SHORTENTRY* pShortEntry,
				 DWORD* pDirClus,
				 DWORD* pDirOffset); //Obtain the directory entry of a full name.
BOOL CreateFatDir(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszDirName,BYTE Attributes);
BOOL CreateFatFile(__FAT32_FS* pFat32Fs,DWORD dwStartCluster,CHAR* pszFileName,BYTE Attributes);
BOOL InitShortEntry(__FAT32_SHORTENTRY* pfse,CHAR* pszName,DWORD dwFirstClus,DWORD dwInitSize,
					BYTE FileAttr);
BOOL ConvertShortEntry(__FAT32_SHORTENTRY* pfse,FS_FIND_DATA* pffd);
BOOL AppendClusterToChain(__FAT32_FS* pFat32Fs,DWORD* pCurrCluster);

//Device dispatching routines implemented in FAT322.CPP.
DWORD FatDeviceCreate(__COMMON_OBJECT* lpDrv,
					  __COMMON_OBJECT* lpDev,
					  __DRCB*          lpDrcb);
DWORD FatDeviceWrite(__COMMON_OBJECT* lpDrv,
					 __COMMON_OBJECT* lpDev,
					 __DRCB*          lpDrcb);
DWORD FatDeviceRead(__COMMON_OBJECT* lpDrv,
					__COMMON_OBJECT* lpDev,
					__DRCB*          lpDrcb);
DWORD FatDeviceSize(__COMMON_OBJECT* lpDrv,
	__COMMON_OBJECT* lpDev,
	__DRCB*          lpDrcb);

BOOL DeleteFatFile(__FAT32_FS* pFat32Fs,CHAR* pszFileName);
BOOL DeleteFatDir(__FAT32_FS* pFat32Fs,CHAR* pszDirName);
BOOL ReleaseClusterChain(__FAT32_FS* pFat32Fs,DWORD dwStartCluster);

#define  FAT32_DATETIME_CREATE     0x1
#define  FAT32_DATETIME_WRITE      0x2
#define  FAT32_DATETIME_ACCEPT     0x4

VOID     SetFatFileDateTime(__FAT32_SHORTENTRY*  pDirEntry,DWORD dwTimeFlage);


VOID* FatMem_Alloc(INT nSize);
VOID  FatMem_Free(VOID* p);

#endif  //__FAT32_H__
