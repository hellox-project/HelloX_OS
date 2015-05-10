//------------------------------------------------------------------------
//  NTFS file system related definitions,all these structures and objects
//  are from NTFS documentation.
//
//  Author                 : Garry.Xin
//  Initial date           : Nov 05,2011
//  Last updated           : Nov 05,2011
//  Last updated author    : Garry.Xin
//  Last udpated content   : 
//------------------------------------------------------------------------
 
#ifndef __NTFS_H__
#define __NTFS_H__
 
#define PARTITION_DEVICE_NAME "\\\\.\\C:"
 
//Basic data types definition,only for portable.
typedef unsigned long     UINT_32;
typedef unsigned short    UINT_16;
typedef unsigned char     UINT_8;
typedef signed long       INT_32;
typedef signed short      INT_16;
typedef signed char       INT_8;
//typedef unsigned char     UCHAR;
//typedef signed char       CHAR;
//typedef signed short      WCHAR;
//typedef unsigned short    UWCHAR;
//typedef signed char       BYTE;
 
//typedef void*             LPVOID;
//typedef unsigned long     BOOL;
 
//#define TRUE              0xFFFFFFFF
//#define FALSE             0x00000000
 
#define __MEM_ALLOC(size)   malloc(size)
#define __MEM_FREE(p)       free(p)
 
#define __ENTER_CRITICAL_SECTION(a,b)
#define __LEAVE_CRITICAL_SECTION(a,b)
 
//NTFS file name's length.
#define NTFS_FILENAME_SIZE       256
 
//Macros used to retrieve key information from BPB sector.
#define BPB_OEMNAME_START        0x03
#define BPB_BYTES_PER_SEC(base)  (*(UINT_16*)(base + 0x0B))
#define BPB_SEC_PER_CLUS(base)   (*(UINT_8*)(base + 0x0D))
#define BPB_HIDDEN_SEC(base)     (*(UINT_32*)(base + 0x1C))
#define BPB_TOTAL_SEC_LOW(base)  (*(UINT_32*)(base + 0x28))
#define BPB_TOTAL_SEC_HIGH(base) (*(UINT_32*)(base + 0x2C))
#define BPB_MFT_START_LOW(base)  (*(UINT_32*)(base + 0x30))
#define BPB_MFT_START_HIGH(base) (*(UINT_32*)(base + 0x34))
#define BPB_CLUS_PER_FR(base)    (*(UINT_32*)(base + 0x40))
#define BPB_CLUS_PER_DR(base)    (*(UINT_32*)(base + 0x44))
#define BPB_SerialNum            0x70
 
//Macros used to obtain key value from file record.
#define FR_USN_OFFSET(frbase)    (*(UINT_16*)(frbase + 0x04))
#define FR_USN_SIZE(frbase)      (*(UINT_16*)(frbase + 0x06) - 1)
#define FR_ATTR_OFFSET(frbase)   (*(UINT_16*)(frbase + 0x14))
#define FR_FILE_FLAGS(frbase)    (*(UINT_16*)(frbase + 0x16))
#define FR_SIZE(frbase)          (*(UINT_32*)(frbase + 0x18))
#define FR_SIZE_ALLOC(frbase)    (*(UINT_32*)(frbase + 0x1C))
#define FR_NEXT_ATTRID(frbase)   (*(UINT_16*)(frbase + 0x28))
 
//NTFS file attributes definition.
#define NTFS_ATTR_SI             0x10   //Standard information.
#define NTFS_ATTR_AS             0x20   //Attribute list.
#define NTFS_ATTR_FILENAME       0x30   //File name.
#define NTFS_ATTR_OI             0x40   //Object ID.
#define NTFS_ATTR_SD             0x50   //Security descriptor.
#define NTFS_ATTR_VOLUMENAME     0x60   //Volume name.
#define NTFS_ATTR_VI             0x70   //Volume information.
#define NTFS_ATTR_DATA           0x80   //File data.
#define NTFS_ATTR_INDEXROOT      0x90   //Index root.
#define NTFS_ATTR_INDEXALLOC     0xA0   //Index allocation.
#define NTFS_ATTR_BM             0xB0   //Bitmap.
#define NTFS_ATTR_RP             0xC0   //Reparse point.
#define NTFS_ATTR_EI             0xD0   //EA information.
#define NTFS_ATTR_EA             0xE0   //EA.
#define NTFS_ATTR_LUS            0x100  //Logged untility stream.
 
//Macros used to obtain specific values from residential attribute.
#define RA_TYPE(rabase)          (*(UINT_32*)(rabase))
#define RA_SIZE(rabase)          (*(UINT_32*)(rabase + 0x04))
#define RA_NO_RA_FLAG(rabase)    (*(UINT_8*)(rabase + 0x08))
#define RA_NAME_SIZE(rabase)     (*(UINT_8*)(rabase + 0x09))
#define RA_NAME_OFFSET(rabase)   (*(UINT_16*)(rabase + 0x0A))
#define RA_FLAGS(rabase)         (*(UINT_16*)(rabase + 0x0C))
#define RA_ID(rabase)            (*(UINT_16*)(rabase + 0x0E))
#define RA_ADATA_SIZE(rabase)    (*(UINT_32*)(rabase + 0x10))
#define RA_ADATA_OFFSET(rabase)  (*(UINT_16*)(rabase + 0x14))
 
//Macros used to obtain specific values from non-residential attribute.
#define NRA_TYPE(rabase)         (*(UINT_32*)(rabase))
#define NRA_SIZE(rabase)         (*(UINT_32*)(rabase + 0x04))
#define NRA_NO_RA_FLAG(rabase)   (*(UINT_8*)(rabase + 0x08))
#define NRA_NAME_SIZE(rabase)    (*(UINT_8*)(rabase + 0x09))
#define NRA_NAME_OFFSET(rabase)  (*(UINT_16*)(rabase + 0x0A))
#define NRA_FLAGS(rabase)        (*(UINT_16*)(rabase + 0x0C))
#define NRA_ID(rabase)           (*(UINT_16*)(rabase + 0x0E))
#define NRA_DRUN_OFFSET(rabase)  (*(UINT_16*)(rabase + 0x20))
#define NRA_ALLOC_SIZE(rabase)   (*(UINT_32*)(rabase + 0x28))
#define NRA_TRUE_SIZE(rabase)    (*(UINT_32*)(rabase + 0x30))
 
//Macros used to retrieve file's information from FILE NAME attribute.
#define FN_ALLOC_SIZE(base)      (*(UINT_32*)(base + 0x28))
#define FN_VALID_SIZE(base)      (*(UINT_32*)(base + 0x30))
#define FN_DOS_ATTR(base)        (*(UINT_32*)(base + 0x38))
#define FN_NAME_SIZE(base)       (*(UINT_8*)(base + 0x40))
#define FN_NAME_OFFSET           0x42
#define FN_CTIME_OFFSET          0x08   //Create time's offset.
#define FN_ATIME_OFFSET          0x10   //Alternate time's offset.
#define FN_LATIME_OFFSET         0x20   //Last access time.
 
//Marcos used to fetch relative data from Index Block.
#define IB_USN_OFFSET(base)      (*(UINT_16*)(base + 0x04))
#define IB_USN_SIZE(base)        (*(UINT_16*)(base + 0x06) - 1)
#define IB_ENTRY_OFFSET(base)    (*(UINT_32*)(base + 0x18) + 0x18)
#define IB_ENTRY_SIZE(base)      (*(UINT_32*)(base + 0x1C))  //All Index Entry's total size in this block.
#define IB_ENTRY_ALLOC_SZ(base)  (*(UINT_32*)(base + 0x20))  //Allocated size for all Index Entries.
 
//Macros used to fetch key element from Index Entry.
#define IE_FR_NUMBER(base)       (*(UINT_32*)(base))
#define IE_ENTRY_SIZE(base)      (*(UINT_16*)(base + 0x08))  //Index Entry's whole size.
#define IE_ENTRYDATA_SIZE(base)  (*(UINT_16*)(base + 0x0A))  //Index Entry Data's size.
#define IE_ENTRY_FLAGS(base)     (*(UINT_16*)(base + 0x0C))  //Index Entry flags.
#define IE_STREAM_OFFSET         0x10                        //Index data's offset.
 
#define IE_FLAGS_SUBNODE         0x01  //Index Entry with sub node.
#define IE_FLAGS_LAST            0x02  //The last Index Entry in one index block.
 
//Pre-definitions of key objects.
struct __NTFS_FILE_SYSTEM;
struct __NTFS_FILE_OBJECT;
struct __NTFS_DATA_RUN;
 
//Data run list.
struct __NTFS_DATA_RUN{
         //__NTFS_DATA_RUN*      pPrev;
         __NTFS_DATA_RUN*      pNext;
         UINT_32               clusNum;          //How many cluster in this run.
         UINT_32               startClusLow;     //Low 4 bytes of start cluster number.
         UINT_32               startClusHigh;    //High 4 bytes of start number.
};
 
//NTFS file object's definition.
struct __NTFS_FILE_OBJECT{
         __NTFS_FILE_OBJECT*   pPrev;
         __NTFS_FILE_OBJECT*   pNext;
         BYTE*                 pFileRecord;
         __NTFS_FILE_SYSTEM*   pFileSystem;
 
         //Specific for directory only.
         UINT_32               irIndexEntryOff;   //Index Entry offset in INDEX ROOT attribute.
         UINT_32               irIndexEntryLen;   //Index Entry's total length in INDEX ROOT attribute.
                                                  //No IE in ROOT INDEX if ifIndexEntryLen = 0.
 
         //File's name.
         WCHAR                 FileName[NTFS_FILENAME_SIZE];
         UINT_8                fileNameSize;
 
         //File's data run.
         __NTFS_DATA_RUN*      pRunList;
         UINT_32               totalCluster;
 
         //File data buffer.
         BYTE*                 pFileBuffer;
         UINT_32               fileBuffSize;
         UINT_32               buffStartVCN;    //Start VCN number of file buffer.
         UINT_32               fileBuffContentSize; //How many bytes that is valid in buffer.
 
         //Current file pointer.
         UINT_32               currPtrVCN;      //Current pointer's VCN number.
         UINT_32               currPtrLCN;      //Current pointer's LCN number.
         UINT_32               currPtrClusOff;  //Offset in cluster.
 
         //File attributes.
         UINT_32               fileAttr;
         UINT_32               fileSizeLow;
         UINT_32               fileSizeHigh;
         UINT_32               fileAllocSize;
};

#define MIN_FILE_BUFFER_SIZE   4096    //Maximal file buffer's size.
 
//Definition of NTFS file system object.
struct __NTFS_FILE_SYSTEM{
         UINT_16               bytesPerSector;    //How many bytes in one disk sector.
         UINT_8                sectorPerClus;     //How many sectors per cluster.
         UINT_32               hiddenSector;      //How many sectors before this partition.
         UINT_32               totalSectorLow;
         UINT_32               totalSectorHigh;
         UINT_32               mftStartClusLow;   //Start cluster number of MFT.
         UINT_32               mftStartClusHigh;
         CHAR                  serialNumber[8];   //Serial number of this partition.
 
         UINT_32               FileRecordSize;    //File record's size,may be 1024 in most case.
         UINT_32               DirBlockSize;      //Directory block's size,maybe 4K in most case.
 
         UINT_32               clusSize;          //Cluster size,it's equal to bytesPerSector * sectorPerClus.
 
         __NTFS_FILE_OBJECT*   pMFT;
         __NTFS_FILE_OBJECT*   pRootDir;
         __NTFS_FILE_OBJECT*   pLog;
         __NTFS_FILE_OBJECT*   pBitmap;
         __NTFS_FILE_OBJECT*   pBoot;
         __NTFS_FILE_OBJECT*   pAttrib;
 
         __NTFS_FILE_OBJECT    fileRoot;          //File object list's root.
};
 
//A internal object to track the FindFirstFile/FindNextFile operations.
struct __NTFS_FIND_HANDLE{
         __NTFS_FILE_OBJECT*   pDirectory;       //Directory's handle to find.
         UINT_32               nextIndexBlock;   //The next cluster will be checked by FindNextFile.
         UINT_32               ieOffset;         //Offset in nextIndexBlock.
};
 
//File time container.
struct __FILE_TIME{
         UINT_32 dwHighDateTime;    //High 4 bytes of file's time.
         UINT_32 dwLowDateTime;     //Low 4 bytes of file time.
};
 
//A struct used to return the FindFirstFile and FindNextFile's result.
#define MAX_FILE_NAME_LEN     256
 
struct FS_FIND_DATA{
         UINT_32       dwFileAttribute;
         __FILE_TIME   ftCreationTime;
         __FILE_TIME   ftLastAccessTime;
         __FILE_TIME   ftLastWriteTime;
         UINT_32       nFileSizeHigh;
         UINT_32       nFileSizeLow;
         UINT_32       dwReserved0;
         UINT_32       dwReserved1;
         CHAR          cFileName[MAX_FILE_NAME_LEN];
         CHAR          cAlternateFileName[13]; //DOS file name.
};
 
//Global routines in NTFS module.
__NTFS_FILE_OBJECT* NtfsCreateFileByFRN(__NTFS_FILE_SYSTEM* pFileSystem,UINT_32 frIndex);
VOID NtfsDestroyFile(__NTFS_FILE_OBJECT* pFileObject);
BOOL ReadSector(__NTFS_FILE_SYSTEM*,UINT_32,UINT_32,BYTE*);
BOOL ReadCluster(__NTFS_FILE_SYSTEM*,UINT_32,UINT_32,BYTE*);
UINT_32 VCN2LCN(__NTFS_FILE_OBJECT*,UINT_32);
BOOL NtfsReadFile(__NTFS_FILE_OBJECT* pFileObject,BYTE* pBuffer,UINT_32 toReadSize,
                                       UINT_32* pReadSize,void* pExt);
UINT_32 NtfsGetFileSize(__NTFS_FILE_OBJECT* pFileObject,UINT_32* pSizeHigh);
UINT_32 NtfsSetFilePointer(__NTFS_FILE_OBJECT* pFileObject,UINT_32 toMoveLow,UINT_32* ptoMoveHigh,
                                                           UINT_32 moveMethod);
BOOL FindFile(__NTFS_FILE_OBJECT* pDirectory,WCHAR* pwszFileName,UINT_32* pfrIndex);
__NTFS_FILE_OBJECT* NtfsCreateFile(__NTFS_FILE_SYSTEM* pFileSystem,const WCHAR* pFullName);
__NTFS_FIND_HANDLE* NtfsFindFirstFile(__NTFS_FILE_SYSTEM* pFileSystem,
                                                                                      const WCHAR* pDirName,FS_FIND_DATA* pFindData);
BOOL NtfsFindNextFile(__NTFS_FIND_HANDLE* pFindHandle,FS_FIND_DATA* pFindData);
void NtfsCloseFind(__NTFS_FIND_HANDLE* pFindHandle);
UINT_32 NtfsGetFileAttr(__NTFS_FILE_OBJECT* pFileObject);
 
void ListDirFile(__NTFS_FILE_SYSTEM* pFileSystem,const WCHAR* pDirName);
void DumpFileObject(__NTFS_FILE_OBJECT* pFileObject);
 
//NTFS support routines,implemented in NTFS3.CPP
int wstrcmp(const WCHAR* src,const WCHAR* dst);
WCHAR* wstrcpy (WCHAR* dest,const WCHAR* src);
int wstrlen(const WCHAR* src);
WCHAR* byte2unicode(WCHAR* dest,const char* src);
 
BOOL NameIsValid(WCHAR* pFullName);
BOOL GetFullNameLevel(WCHAR* pFullName,DWORD* pdwLevel);
BOOL GetSubDirectory(WCHAR* pFullName,DWORD dwLevel,WCHAR* pSubDir);
BOOL GetPathName(WCHAR* pFullName,WCHAR* pDir,WCHAR* pFileName);
 
#endif
