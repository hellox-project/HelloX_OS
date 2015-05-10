// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息

// Windows 头文件:
#include <windows.h>
#include <winioctl.h>
#include <shlwapi.h>
#include <dbt.h>
#include <shlwapi.h>

// C 运行时头文件
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#define  FILE_ATTR_VOLUMEID 0x08

#define  HX_SECTOR_SIZE       512           //扇区字节数
#define  HX_KERNEL_START      2             //HX内核存放起始扇区地址
#define  HX_KERNEL_SIZE       (8+128+1120)  //HX内核占用扇区空间
#define  HX_HDD_RESERVED      204800        //HX保留空间扇区数量(U盘使用）
#define  HX_BOOTSEC_MPS       0x1BE         //引导扇区第一个主分区起始地址

//虚拟硬盘设置
#define  HX_VDISK_RESERVED            8000           //虚拟硬盘保留空间扇区数量
#define  HX_VDISK_SECTOR             (204800*2)     //虚拟硬盘扇区默认总数(200MB)
#define  HX_VDISK_CLINDER_PER_TRACK  255            //
#define  HX_VDISK_TRACK_PER_SECTOR   63
#define  HX_VDISK_REDUN_SIZE         (1024*1024*64) //虚拟磁盘空白冗余大小
    

//fat32相关定义
#define  HX_FAT32_SECPERCLUS  16            //fat32文件系统每蔟扇区数量
#define  HX_FAT32_FATSIZE     16145         //fat表大小(单位扇区)
#define  HX_FAT32_RESERVED    32            //fat32保留扇区数量
#define  HX_FAT32_BLANK       100           //fat32剩余扇区数量


//操作过程定义
#define  PROCESS_MAKE_BOOT	  1
#define  PROCESS_MAKE_KERNEL  2
#define  PROCESS_MAKE_FAT32   3 
#define  PROCESS_INIT_DEIVCE  4
#define  PROCESS_INIT_VHD	  5
#define  PROCESS_IMPORT_VHD	  6

//操作返回值
#define  RET_COMPLETE		  100
#define  RET_USB_ERR         -100
#define  RET_BOOTSEC_ERR     -200
#define  RET_KERNEL_ERR      -300
#define  RET_FAT32_ERR       -400
#define  RET_IMPORT_ERR      -800



#define  FAT32_FMT_FLAGE       "FAT32 FORMAT OK" //自定义的Fat32格式化标志，防止重新格式化以便节省时间
#define  FAT32_FMT_SEC         2                 //Fat32格式化标志扇区位置


#define  WM_MAKE_PROCESS   (WM_USER+1001)
#define  USB_NO_FOUND      L"没有发现USB设备，请插入您的USB设备"


//g_nUsbSectors = (UINT64)pg.Cylinders.LowPart*(UINT64)pg.TracksPerCylinder*(UINT64)pg.SectorsPerTrack;
//设备信息
typedef struct DISK_INFO
{
		UINT    nCylinders;
		UINT    nTracksPerCylinder;
		UINT    nSectorsPerTrack;
		UINT64  nTotalSector;
		UINT64  nTotalSize;

}__DISK_INFO;

typedef struct MAIN_PATION_INFO
{
	BYTE    bActiveFlage;
	BYTE    bStartClinder;
	BYTE    bStartTrack;
	BYTE    bStartSector;

	BYTE    bPationType;
	BYTE    bEndClinder;
	BYTE    bEndTrack;
	BYTE    bEndSector;

	UINT    nBeforSectors;
	UINT    nPationSectors;
	
}__MAIN__INFO;

typedef struct VHD_INFO 
{
	UINT64   cookie;
	UINT     Features;
	UINT     ffv;
	UINT64   offset;
	UINT     time;
	UINT     creat_app;
	UINT     creat_ver;
	UINT     creat_os;
	UINT64   orgin_size;
	UINT64   current_size;
	UINT     disk_geo;
	UINT     disk_type;
	UINT     checksum;
	CHAR     unique_id[16];
	CHAR     reserved[428];

}_VHD_INFO;


//设备格式化函数
BOOL APIENTRY Fat32Format(HANDLE  hDiskDrive,UINT nSectorCount,UINT nReserved);


//得到目录总大小
UINT64 APIENTRY GetDirSize(LPCTSTR pSrcDir);

//导入文件到Vhd文件
DWORD APIENTRY ImportFilesToVhd(LPCTSTR pVhdFile,LPCTSTR pSrcDir,DWORD dwPartitionSatrt);


INT   APIENTRY GetModelPath(HMODULE hModul,LPTSTR pModelPath,INT nBufLen);
TCHAR APIENTRY GetDriveFromMask(UINT nMask) ; 