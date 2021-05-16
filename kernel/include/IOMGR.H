//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May,01 2005
//    Module Name               : iomgr.h
//    Module Funciton           : 
//                                This module countains the pre-definition of Input and
//                                Output(I/O) manager.
//    Last modified Author      : Garry.Xin
//    Last modified Date        : DEC 17,2011
//    Last modified Content     :
//                                1. All definitions and types are moved to this file from
//                                   fs.h;
//                                2. Optimized some comments and syntax format of this file.
//    Lines number              :
//***********************************************************************/

#ifndef __IOMGR_H__
#define __IOMGR_H__

#include "commobj.h"
#include "ktmgr.h"
#include "ata.h" /* for harddisk, partition. */

/* Partition types. */
#define PARTITION_TYPE_FAT32     0x0B
#define PARTITION_TYPE_NTFS      0x07
#define PARTITION_TYPE_FAT16     0x06
#define PARTITION_TYPE_EXTENSION 0x0F
/* The physical hard disk. */
#define PARTITION_TYPE_RAW       0x0E

#define FILE_SYSTEM_TYPE_UNKNOWN 0x00
#define FILE_SYSTEM_TYPE_FAT32   PARTITION_TYPE_FAT32
#define FILE_SYSTEM_TYPE_FAT16   PARTITION_TYPE_FAT16
#define FILE_SYSTEM_TYPE_NTFS    PARTITION_TYPE_NTFS

//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20
/* Combination of READONLY/HIDDEN/SYSTEM/VOLUMEID */
#define FILE_ATTR_LONGNAME    0x0F

/* Max length of file name. */
#define MAX_FILE_NAME_LEN 256

//FILETIME struct,used to store file related date and time.
BEGIN_DEFINE_OBJECT(__FILE_TIME)    //-------- CAUTION!!! --------
    DWORD dwHighDateTime;
    DWORD dwLowDateTime;
END_DEFINE_OBJECT(__FILE_TIME)

//A struct to operate manipulate find operations.
BEGIN_DEFINE_OBJECT(FS_FIND_DATA)
    DWORD              dwFileAttribute;
    __FILE_TIME        ftCreationTime;    //-------- CAUTION!!! --------
	__FILE_TIME        ftLastAccessTime;  //-------- CAUTION!!! --------
	__FILE_TIME        ftLastWriteTime;   //-------- CAUTION!!! --------
	DWORD              nFileSizeHigh;
	DWORD              nFileSizeLow;
	DWORD              dwReserved0;
	DWORD              dwReserved1;
	BOOL               bGetLongName;
	CHAR               cFileName[MAX_FILE_NAME_LEN];
	CHAR               cAlternateFileName[13];
END_DEFINE_OBJECT(FS_FIND_DATA)

/* Helper macros. */
#ifndef CREATE_OBJECT
#define CREATE_OBJECT(type) _hx_calloc(1,sizeof(type))
#endif

#ifndef RELEASE_OBJECT
#define RELEASE_OBJECT(ptr) _hx_free(ptr)
#endif

/* Maximal length of device's name. */
#define MAX_DEV_NAME_LEN 256

/* routine protype for drcb. */
typedef unsigned long (*DRCB_WAITING_ROUTINE)(__COMMON_OBJECT*);
typedef unsigned long (*DRCB_COMPLETION_ROUTINE)(__COMMON_OBJECT*);
typedef unsigned long (*DRCB_CANCEL_ROUTINE)(__COMMON_OBJECT*);
typedef void (*DRCB_RESET_ROUTINE)(__COMMON_OBJECT*);

/*
 * DRCB(Device Request Control Block) object.
 * It's used to trace the device request operations,
 * and is the core object of i/o system.
 */
typedef struct tag__DRCB{
    INHERIT_FROM_COMMON_OBJECT

	/* Wake up the drcb. */
	__EVENT* lpSynObject;
	/* thread this drcb belonging. */
    __KERNEL_THREAD_OBJECT* lpKernelThread;
	/* drcb status. */
	unsigned long dwStatus;
	/* request mode,r/w... */
	unsigned long dwRequestMode;
	/* command for iocontrol. */
	unsigned long dwCtrlCommand;

	/* Output buffer and it's length. */
	unsigned long dwOutputLen;
	LPVOID lpOutputBuffer;

	/* Input buffer and it's length. */
	unsigned long dwInputLen;
	LPVOID lpInputBuffer;

	/* Link list pointers. */
	struct tag__DRCB* lpNext;
	struct tag__DRCB* lpPrev;

	/* Wait the drcb to finish. */
	DRCB_WAITING_ROUTINE WaitForCompletion;
	/* Invoked when request finish, must in device driver. */
	DRCB_COMPLETION_ROUTINE OnCompletion;
	/* Invoked when drcb canceled. */
	DRCB_CANCEL_ROUTINE OnCancel;
	/* Must invoke when reuse. */
	DRCB_RESET_ROUTINE Reset;

	/* Extra parameters used in rare case. */
	unsigned long dwExtraParam1;
	unsigned long dwExtraParam2;

	/* Extension,contains specific object. */
	LPVOID lpDrcbExtension;
}__DRCB;

/* 
 * Default wait time for DRCB request in ms.
 * The request will giveup if not finish in this
 * period of time.
 */
#define DRCB_DEFAULT_WAIT_TIME   10000

/* Init and deinit of drcb. */
BOOL DrcbInitialize(__COMMON_OBJECT*);
BOOL DrcbUninitialize(__COMMON_OBJECT*);

/* drcb status. */
#define DRCB_STATUS_INITIALIZED       0x00000000
#define DRCB_STATUS_FAIL              0x00000001  //The DRCB has not been handled.  
#define DRCB_STATUS_SUCCESS           0x00000002  //The DRCB has been handled successfully.
#define DRCB_STATUS_PENDING           0x00000004  //The DRCB is pended to handle.
#define DRCB_STATUS_CANCELED          0x00000008  //The DRCB has been canceled.

/* drcb request type. */
#define DRCB_REQUEST_MODE_READ        0x00000001        //Read operations.
#define DRCB_REQUEST_MODE_WRITE       0x00000002        //Write operations.
#define DRCB_REQUEST_MODE_OPEN        0x00000004        //Open operations.
#define DRCB_REQUEST_MODE_CLOSE       0x00000008        //Close operations.
#define DRCB_REQUEST_MODE_SEEK        0x00000010        //Seek(or SetFilePointer) operations.
#define DRCB_REQUEST_MODE_FLUSH       0x00000020        //Flush operations.
#define DRCB_REQUEST_MODE_CREATE      0x00000040        //Create operations.
#define DRCB_REQUEST_MODE_DESTROY     0x00000080        //Destroy operations.
#define DRCB_REQUEST_MODE_IOCTRL      0x00000100        //IOControl operations.
#define DRCB_REQUEST_MODE_SIZE        0x00000200        //GetFileSize

/* command for iocontrol routine. */
#define IOCONTROL_GET_READ_BLOCK_SIZE    0x00000001  //Get read block size.
#define IOCONTROL_GET_WRITE_BLOCK_SIZE   0x00000002  //Get write block size.
#define IOCONTROL_READ_SECTOR            0x00000004  //Read one or several sectors from
                                                     //device.
#define IOCONTROL_WRITE_SECTOR           0x00000008  //Write one or several sectors to
                                                     //device.
#define IOCONTROL_FS_CHECKPARTITION      0x00000010  //Issue check partition command to
                                                     //file system drivers.
#define IOCONTROL_FS_CREATEDIR           0x00000020  //Issue create directory command to file system.
#define IOCONTROL_FS_DELETEFILE          0x00000040  //Delete directory.
#define IOCONTROL_FS_GETFILEATTR         0x00000080  //Get file attributes.
#define IOCONTROL_FS_GETFILESIZE         0x00000100  //Get file size.
#define IOCONTROL_FS_SETENDFILE          0x00000200  //Set current position as one file's end.
#define IOCONTROL_FS_FINDCLOSE           0x00000400  //Close file finding handle.
#define IOCONTROL_FS_FINDFIRSTFILE       0x00000800  //Find first file given one directory.
#define IOCONTROL_FS_FINDNEXTFILE        0x00001000  //Find next file.
#define IOCONTROL_FS_REMOVEDIR           0x00002000  //Remove directory.

/* Transfer parameters of write sector command. */
BEGIN_DEFINE_OBJECT(__SECTOR_INPUT_INFO)
    unsigned long dwStartSector;
    unsigned long dwBufferLen;
	LPVOID   lpBuffer;
END_DEFINE_OBJECT(__SECTOR_INPUT_INFO)

/* 
 * This object is used to describe hardware 
 * resources, such as input/output port,DMA channel,
 * interrupt vector,etc.
 */
typedef struct tag__RESOURCE_DESCRIPTOR{
	/* List pointers. */
    struct tag__RESOURCE_DESCRIPTOR* lpNext;
    struct tag__RESOURCE_DESCRIPTOR* lpPrev;

    unsigned long dwStartPort;
    unsigned long dwEndPort;
	unsigned long dwDmaChannel;
	unsigned long dwInterrupt;
	LPVOID lpMemoryStartAddr;
	unsigned long dwMemoryLen;
}__RESOURCE_DESCRIPTOR;

/* Device driver object. */
typedef struct tag__DRIVER_OBJECT {
	INHERIT_FROM_COMMON_OBJECT
	
	/* Bidirectional link list pointers. */
	struct tag__DRIVER_OBJECT* lpPrev;
	struct tag__DRIVER_OBJECT* lpNext;

	/* Read from device. */
	DWORD (*DeviceRead)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Write into device. */
	DWORD (*DeviceWrite)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Get Device's size. */
	DWORD (*DeviceSize)(__COMMON_OBJECT*   lpDrv,
		__COMMON_OBJECT*   lpDev,
		__DRCB*            lpDrcb);

	/* Device specific control. */
	DWORD (*DeviceCtrl)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Flush in memory content into device. */
	DWORD (*DeviceFlush)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Move device pointer. */
	DWORD (*DeviceSeek)(__COMMON_OBJECT*   lpDrv,
		__COMMON_OBJECT*   lpDev,
		__DRCB*            lpDrcb);

	/* Open a device. */
	__COMMON_OBJECT* (*DeviceOpen)(__COMMON_OBJECT*   lpDrv,
		__COMMON_OBJECT*   lpDev,
		__DRCB*            lpDrcb);

	/* Close the opened device. */
	DWORD (*DeviceClose)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Create a new device. */
	DWORD (*DeviceCreate)(__COMMON_OBJECT*  lpDrv,
		__COMMON_OBJECT*  lpDev,
		__DRCB*           lpDrcb);

	/* Destroy the created device. */
	DWORD (*DeviceDestroy)(__COMMON_OBJECT* lpDrv,
		__COMMON_OBJECT* lpDev,
		__DRCB*          lpDrcb);

	/* Show out device specific information. */
	unsigned long (*DeviceSpecificShow)(__COMMON_OBJECT* lpDrv,
		__COMMON_OBJECT* lpDev,
		__DRCB*          lpDrcb);
}__DRIVER_OBJECT;

/* Init and deinit routines of drv object. */
BOOL DrvObjInitialize(__COMMON_OBJECT*);
BOOL DrvObjUninitialize(__COMMON_OBJECT*);

/* Device object signature. */
#define DEVICE_OBJECT_SIGNATURE 0xAA55AA55

/* Device object. */
typedef struct tag__DEVICE_OBJECT {
	INHERIT_FROM_COMMON_OBJECT

	/* Device signature,valid the device. */
	unsigned long dwSignature;

	/* Bidirectional link list. */
	struct tag__DEVICE_OBJECT* lpPrev;
	struct tag__DEVICE_OBJECT* lpNext;

	/* device name. */
	char DevName[MAX_DEV_NAME_LEN + 1];

	/* Device attributes, block size. */
	unsigned long dwAttribute;
	unsigned long dwBlockSize;
	unsigned long dwMaxReadSize;
	unsigned long dwMaxWriteSize;

	/*
	 * Current position pointer of the device.
	 * It will be moved forward when reading operation performed,
	 * and can be changed by calling SetFilePointer routine
	 * of IOManager.
	 */
	unsigned long dwCurrentPos;

	/* Point back to this device's driver. */
	__DRIVER_OBJECT* lpDriverObject;
	/* How many sectors has been r/w. */
	unsigned long dwTotalReadSector;
	unsigned long dwTotalWrittenSector;

	/* How many interrupt raised by this device. */
	unsigned long dwInterrupt;
	
	/* Device extension. */
	LPVOID lpDevExtension;
}__DEVICE_OBJECT;

/* Init and deinit routines. */
BOOL DevObjInitialize(__COMMON_OBJECT*);
BOOL DevObjUninitialize(__COMMON_OBJECT*);

/* Device types. */
#define DEVICE_TYPE_NORMAL             0x00000000         //Normal devices.
#define DEVICE_TYPE_FILESYSTEM         0x00000001         //File system object.
#define DEVICE_TYPE_STORAGE            0x00000002         //Storage devices.
#define DEVICE_TYPE_FILE               0x00000004         //File.
#define DEVICE_TYPE_PARTITION          0x00000008         //Partition object.
#define DEVICE_TYPE_FAT32              0x00000010         //A FAT32 partition.
#define DEVICE_TYPE_NTFS               0x00000020         //A NTFS partition.
#define DEVICE_TYPE_FSDRIVER           0x00000040         //File system driver object.
#define DEVICE_TYPE_HARDDISK           0x00000080         //Device is a hard disk.
#define DEVICE_TYPE_REMOVABLE          0x00000100         //Removable disk,such as DVDROM.
#define DEVICE_TYPE_NIC                0x00000200         //Network interface card.

//Special block size definition.
#define DEVICE_BLOCK_SIZE_ANY          0xFFFFFFFF         //When a device specifies this
                                                          //value as it's dwBlockSize or
                                                          //dwMaxReadSize or dwMaxWriteSize,
                                                          //it means application can access
                                                          //this device by using any block size.

#define DEVICE_BLOCK_SIZE_INVALID      0x00000000         //If one device does not want to
                                                          //be accessed,it can specifiy this
                                                          //value.

/*
 * Driver entry routine.
 * All device drivers must implement this routine,
 * it's the entry point of a device driver.
 */
typedef BOOL (*__DRIVER_ENTRY)(__DRIVER_OBJECT*);

/* Built in driver's array element. */
typedef struct{
	__DRIVER_ENTRY Entry;
	char* pszDriverName;
}__DRIVER_ENTRY_ARRAY;

//
//The following is the definition of __IO_MANAGER.
//This object is one of the core object in Hello China,it is used to manage all device(s) 
//and device driver(s) in the system,and it also supplies interface to user kernel thread 
//to access system device.
//
#define VOLUME_LBL_LEN  13  //Volume label length.
#define FILE_SYSTEM_NUM 16  //NEWLY ADDED.
typedef struct{             //NEWLY ADDED.
	BYTE              FileSystemIdentifier;        //Such as C:,D:,etc.
	__COMMON_OBJECT*  pFileSystemObject;
	DWORD             dwAttribute;                 //File attribute.
	BYTE              VolumeLbl[VOLUME_LBL_LEN];   //Volumne ID.
}__FS_ARRAY_ELEMENT;

/* How many file system,such as NTFS,FAT,etc. */
#define FS_CTRL_NUM 4

/* 
 * IO Manager object,all IO functions are offered by 
 * this global object. 
 */
typedef struct tag__IO_MANAGER{
	/* Lock to protect this object. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Device object list. */
    __DEVICE_OBJECT*               lpDeviceRoot;
	/* Device driver object list. */
    __DRIVER_OBJECT*               lpDriverRoot;
	/* File system object. */
	__FS_ARRAY_ELEMENT             FsArray[FILE_SYSTEM_NUM];
	__COMMON_OBJECT*               FsCtrlArray[FS_CTRL_NUM];
	/* Partition sequence number. */
	volatile DWORD                 dwPartitionNumber;
	/* Resource descriptor list. */
	__RESOURCE_DESCRIPTOR*         lpResDescriptor;

	/* Initialization routine. */
	BOOL (*Initialize)(__COMMON_OBJECT* lpThis);

	/* Routines to manipulate file systems. */
	/* Create a new file,or open a existed file or device. */
	__COMMON_OBJECT* (*CreateFile)(__COMMON_OBJECT* lpThis,
		LPSTR lpszFileName,
		DWORD dwAccessMode,
		DWORD dwShareMode,
		LPVOID lpReserved);

	/* Read data from file or device. */
	BOOL (*ReadFile)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject,
		DWORD dwByteSize,
		LPVOID lpBuffer,
		DWORD* lpReadSize);

	/* Write data into file or device. */
	BOOL (*WriteFile)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject,
		DWORD dwWriteSize,
		LPVOID lpBuffer,
		DWORD* lpWrittenSize);

	/* Close an opened file or device. */
	VOID (*CloseFile)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject);

	/* Create a directory under a file system. */
	BOOL (*CreateDirectory)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName,
		LPVOID  lpReserved);

	/* Delete a specified file. */
	BOOL (*DeleteFile)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName);

	/* Close a find transaction. */
	BOOL (*FindClose)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName,
		__COMMON_OBJECT* FindHandle);

	/* Return the first file of a finding transaction. */
	__COMMON_OBJECT* (*FindFirstFile)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName,
		FS_FIND_DATA* pFindData);

	/* Return the next file from current position in a finding transaction. */
	BOOL (*FindNextFile)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName,
		__COMMON_OBJECT* FindHandle,
		FS_FIND_DATA* pFindData);

	/* Get a specified file's attributes. */
	DWORD (*GetFileAttributes)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName);

	/* Return a specified file's size. */
	DWORD (*GetFileSize)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* FileHandle,
		DWORD* lpdwSizeHigh);

	/* Remove a specified directory. */
	BOOL (*RemoveDirectory)(__COMMON_OBJECT* lpThis,
		LPCTSTR lpszFileName);

	/* Set current position as file's end. */
	BOOL (*SetEndOfFile)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* FileHandle);

	/* Device specific control routine. */
	BOOL (*IOControl)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject,
		DWORD dwCommand,
		DWORD dwInputLen,
		LPVOID lpInputBuffer,
		DWORD dwOutputLen,
		LPVOID lpOutputBuffer,
		DWORD* lpdwOutFilled);

	/* Set current pointer's location in a specified file. */
	DWORD (*SetFilePointer)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject,
		DWORD* pdwDistLow,
		DWORD* pdwDistHigh,
		DWORD dwWhereBegin);

	/* Flush file content into storage. */
	BOOL (*FlushFileBuffers)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileObject);

	/* The following routines are used to manipulate device. */
	/* Create a new device object. */
	__DEVICE_OBJECT* (*CreateDevice)(__COMMON_OBJECT* lpThis,
		LPSTR lpszDevName,
		DWORD dwAttribute,
		DWORD dwBlockSize,
		DWORD dwMaxReadSize,
		DWORD dwMaxWriteSize,
		LPVOID lpDevExtension,
		__DRIVER_OBJECT* lpDrvObject);

	/* Return a device object given it's name. */
	__DEVICE_OBJECT* (*GetDevice)(__COMMON_OBJECT* lpThis,
		char* device_name);

	/* Release the device got by GetDevice. */
	BOOL (*ReleaseDevice)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* pDeviceObject);

	/* Destroy a device object. */
	VOID (*DestroyDevice)(__COMMON_OBJECT* lpThis,
		__DEVICE_OBJECT* lpDevObj);

	/* Reserve resource for a device. */
	BOOL (*ReserveResource)(__COMMON_OBJECT* lpThis,
		__RESOURCE_DESCRIPTOR* lpResDesc);

	/* Load a device driver by specifying it's entry point. */
	BOOL (*LoadDriver)(__DRIVER_ENTRY DrvEntry);

	/* The following routines are used by file system driver. */
	/* Add a new file system into IO Manager. */
	BOOL (*AddFileSystem)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileSystem,
		DWORD dwAttribute,
		CHAR* pVolumeLbl);
	/* Register a new file system object. */
	BOOL (*RegisterFileSystem)(__COMMON_OBJECT* lpThis,
		__COMMON_OBJECT* lpFileSystem);
}__IO_MANAGER;

/* Flags to control file accessing. */
/* read only access. */
#define FILE_ACCESS_READ         0x00000001
/* Write only access. */
#define FILE_ACCESS_WRITE        0x00000002
/* Both read and write. */
#define FILE_ACCESS_READWRITE    0x00000003
/* Create a new file. */
#define FILE_ACCESS_CREATE       0x00000004
/* Create a new file if the specified isn't exist. */
#define FILE_OPEN_ALWAYS         0x80000000
/* Create a new file,overwrite existing one. */
#define FILE_OPEN_NEW            0x40000000
/* Open a existing file. */
#define FILE_OPEN_EXISTING       0x20000000

/*
 * Used by SetFilePointer routine to control 
 * moving direction.
 */
#define FILE_FROM_BEGIN        0x00000001
#define FILE_FROM_CURRENT      0x00000002
#define FILE_FROM_END          0x00000003

/* Global I/O manager. */
extern __IO_MANAGER IOManager;

#endif    //__IOMGR_H__
