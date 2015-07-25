//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 01 Feb,2009
//    Module Name               : FDISK.H
//    Module Funciton           : 
//    Description               : Prototypes for fdisk thread.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __FDISK_H__
#define __FDISK_H__
#endif

//Main entry for fdisk application.
DWORD fdiskEntry(LPVOID lpData);

//Flags for command processing result.
#define FDISK_CMD_TERMINAL     0x00000001
#define FDISK_CMD_INVALID      0x00000002
#define FDISK_CMD_FAILED       0x00000004
#define FDISK_CMD_SUCCESS      0x00000008


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

//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20
#define FILE_ATTR_LONGNAME    0x0F //Combination of READONLY/HIDDEN/SYSTEM/VOLUMEID

