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

#ifdef __cplusplus
extern "C" {
#endif

//Partition type,indicates which kind of file system is used
//for this partition.
#define PARTITION_TYPE_FAT32     0x0B
#define PARTITION_TYPE_NTFS      0x07
#define PARTITION_TYPE_FAT16     0x06
#define PARTITION_TYPE_EXTENSION 0x0F
#define PARTITION_TYPE_RAW       0x0E  //The physical hard disk.

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
#define FILE_ATTR_LONGNAME    0x0F //Combination of READONLY/HIDDEN/SYSTEM/VOLUMEID

//A extension object used to describe the partition device object.
BEGIN_DEFINE_OBJECT(__PARTITION_EXTENSION)
    BYTE            BootIndicator;  //Boot indicator.
    BYTE            PartitionType;  //The partition type.
    DWORD           dwStartSector;  //The start position of this partition.
	DWORD           dwSectorNum;    //How many sectors this partition occupied.
	DWORD           dwCurrPos;      //Current pointer position,read or write
	                                //operation will be launched from here.
	int             nDiskNum;       //Disk number where this partition resides.
END_DEFINE_OBJECT(__PARTITION_EXTENSION)

#define PARTITION_NAME_BASE "\\\\.\\PARTITION0"

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

//Helper macros to simply the developing process.
#ifndef CREATE_OBJECT
#define CREATE_OBJECT(type) KMemAlloc(sizeof(type),KMEM_SIZE_TYPE_ANY)
#endif

#ifndef RELEASE_OBJECT
#define RELEASE_OBJECT(ptr) KMemFree((LPVOID)(ptr),KMEM_SIZE_TYPE_ANY,0)
#endif

//
//Constants used by this module(IOManager).
//

#define MAX_DEV_NAME_LEN    256    //The maximal length of device object's name.

//
//The definition of DRCB(Device Request Control Block).
//This object is used to trace the device request operations,it is the core object
//used by IOManager and device drivers to communicate,and also the core object used
//between device drivers to communicate each other.
//

typedef DWORD (*DRCB_WAITING_ROUTINE)(__COMMON_OBJECT*);
typedef DWORD (*DRCB_COMPLETION_ROUTINE)(__COMMON_OBJECT*);
typedef DWORD (*DRCB_CANCEL_ROUTINE)(__COMMON_OBJECT*);

//BEGIN_DEFINE_OBJECT(__DRCB)
typedef struct tag__DRCB{
    INHERIT_FROM_COMMON_OBJECT
	__EVENT*                          lpSynObject;      //Used to synchronize the access.
    __KERNEL_THREAD_OBJECT*           lpKernelThread;   //The kernel thread who originates
	                                                    //the device access.
	//DWORD                             dwDrcbFlag;
	DWORD                             dwStatus;
	DWORD                             dwRequestMode;    //Read,write,io control,etc.
	DWORD                             dwCtrlCommand;    //If the request mode is io control,
	                                                    //then is variable is used to indicate
	                                                    //the control command.

	//DWORD                             dwOffset;
	DWORD                             dwOutputLen;      //Output buffer's length.
	LPVOID                            lpOutputBuffer;   //Output buffer.

	DWORD                             dwInputLen;
	LPVOID                            lpInputBuffer;

	struct tag__DRCB*                           lpNext;
	struct tag__DRCB*                           lpPrev;

	DRCB_WAITING_ROUTINE              WaitForCompletion;  //Called when waiting the current
	                                                      //device request to finish.
	DRCB_COMPLETION_ROUTINE           OnCompletion;       //Called when the current request
	                                                      //is over.
	DRCB_CANCEL_ROUTINE               OnCancel;

	//DWORD                             dwDrcbExtension[1];
	DWORD                             dwExtraParam1;      //Extra parameter,very useful in some cases.
	DWORD                             dwExtraParam2;      //Extra parameter.
	LPVOID                            lpDrcbExtension;    //Extension pointer,pointing to specific data.
}__DRCB;
//END_DEFINE_OBJECT(__DRCB)                                       //End of __DRCB's definition.

//Default wait time for DRCB request.If the physical operations can not be finished in
//this time,the DRCB will give up.
#define DRCB_DEFAULT_WAIT_TIME   10000 //10 seconds.

//
//Initialize routine and Uninitialize routine's definition.
//
BOOL DrcbInitialize(__COMMON_OBJECT*);
VOID DrcbUninitialize(__COMMON_OBJECT*);

//
//DRCB status definition.
//
#define DRCB_STATUS_INITIALIZED       0x00000000
#define DRCB_STATUS_FAIL              0x00000001  //The DRCB has not been handled.  
#define DRCB_STATUS_SUCCESS           0x00000002  //The DRCB has been handled successfully.
#define DRCB_STATUS_PENDING           0x00000004  //The DRCB is pended to handle.
#define DRCB_STATUS_CANCELED          0x00000008  //The DRCB has been canceled.

//
//DRCB request mode definition.
//
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

//
//The following definitions are used to indicate the appropriate I/O control command.
//
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

//This object is used to transfer parameter in case of IOCONTROL_WRITE_SECTOR.
BEGIN_DEFINE_OBJECT(__SECTOR_INPUT_INFO)
    DWORD    dwStartSector;       //Start position to write.
    DWORD    dwBufferLen;         //How many sector to write.
	LPVOID   lpBuffer;            //Data to write.
END_DEFINE_OBJECT(__SECTOR_INPUT_INFO)

//
//The definition of __RESOURCE_DESCRIPTOR.
//This object is used to describe system resource,such as input/output port,DMA channel,
//interrupt vector,etc.
//
//BEGIN_DEFINE_OBJECT(__RESOURCE_DESCRIPTOR)
typedef struct tag__RESOURCE_DESCRIPTOR{
    struct tag__RESOURCE_DESCRIPTOR*    lpNext;      //Pointes to next descriptor.
    struct tag__RESOURCE_DESCRIPTOR*    lpPrev;      //Pointes to previous descriptor.

    DWORD              dwStartPort;
    DWORD              dwEndPort;
	DWORD              dwDmaChannel;
	DWORD              dwInterrupt;
	LPVOID             lpMemoryStartAddr;
	DWORD              dwMemoryLen;
}__RESOURCE_DESCRIPTOR;
//END_DEFINE_OBJECT(__RESOURCE_DESCRIPTOR)  //End definition of __RESOURCE_DESCRIPTOR.

//
//The definition of __DRIVER_OBJECT.
//
//BEGIN_DEFINE_OBJECT(__DRIVER_OBJECT)
typedef struct tag__DRIVER_OBJECT{
    INHERIT_FROM_COMMON_OBJECT

	struct tag__DRIVER_OBJECT*            lpPrev;
    struct tag__DRIVER_OBJECT*            lpNext;

	DWORD          (*DeviceRead)(__COMMON_OBJECT*  lpDrv,    //Read routine.
	                             __COMMON_OBJECT*  lpDev,
								 __DRCB*           lpDrcb);

    DWORD          (*DeviceWrite)(__COMMON_OBJECT*  lpDrv,   //Write routine.
		                          __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);
	DWORD          (*DeviceSize)(__COMMON_OBJECT*   lpDrv,
		                         __COMMON_OBJECT*   lpDev,
		                         __DRCB*            lpDrcb);

	DWORD          (*DeviceCtrl)(__COMMON_OBJECT*  lpDrv,
		                         __COMMON_OBJECT*  lpDev,
								 __DRCB*           lpDrcb);

	DWORD          (*DeviceFlush)(__COMMON_OBJECT*  lpDrv,
		                          __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);

	DWORD          (*DeviceSeek)(__COMMON_OBJECT*   lpDrv,
		                         __COMMON_OBJECT*   lpDev,
								 __DRCB*            lpDrcb);

	__COMMON_OBJECT* (*DeviceOpen)(__COMMON_OBJECT*   lpDrv,
		                         __COMMON_OBJECT*   lpDev,
								 __DRCB*            lpDrcb);

	DWORD          (*DeviceClose)(__COMMON_OBJECT*  lpDrv,
		                          __COMMON_OBJECT*  lpDev,
								  __DRCB*           lpDrcb);

	DWORD          (*DeviceCreate)(__COMMON_OBJECT*  lpDrv,
		                           __COMMON_OBJECT*  lpDev,
								   __DRCB*           lpDrcb);

	DWORD          (*DeviceDestroy)(__COMMON_OBJECT* lpDrv,
		                            __COMMON_OBJECT* lpDev,
									__DRCB*          lpDrcb);
}__DRIVER_OBJECT;
//END_DEFINE_OBJECT(__DRIVER_OBJECT) //End definition of __DRIVER_OBJECT.

//
//Initialize routine and Uninitialize routine's definition.
//
BOOL DrvObjInitialize(__COMMON_OBJECT*);
VOID DrvObjUninitialize(__COMMON_OBJECT*);

//
//The definition of __DEVICE_OBJECT.
//
#define DEVICE_OBJECT_SIGNATURE 0xAA55AA55

//BEGIN_DEFINE_OBJECT(__DEVICE_OBJECT)
typedef struct tag__DEVICE_OBJECT{
    INHERIT_FROM_COMMON_OBJECT

	DWORD                       dwSignature;  //Used to validate the object's validation.
	struct tag__DEVICE_OBJECT*            lpPrev;
    struct tag__DEVICE_OBJECT*            lpNext;

	CHAR                        DevName[MAX_DEV_NAME_LEN + 1];
	DWORD                       dwAttribute;
	DWORD                       dwBlockSize;
	DWORD                       dwMaxReadSize;
	DWORD                       dwMaxWriteSize;

	__DRIVER_OBJECT*            lpDriverObject;    //Point back to this device's driver.
	DWORD                       dwTotalReadSector; //How many sectors has been read.
	DWORD                       dwTotalWrittenSector; //How many sectors has been written.
	DWORD                       dwInterrupt;       //How many interrupts raised by this
	                                               //device.
	LPVOID                      lpDevExtension;
}__DEVICE_OBJECT;
//END_DEFINE_OBJECT(__DEVICE_OBJECT)     //__DEVICE_OBJECT.

//
//Initialize routine and Uninitialize routine's definition.
//
BOOL DevObjInitialize(__COMMON_OBJECT*);
VOID DevObjUninitialize(__COMMON_OBJECT*);

//
//Device type definition.
//
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

//
//Driver entry routine.
//All device drivers must implement this routine,this routine is the entry point
//of a device driver.
//
typedef BOOL (*__DRIVER_ENTRY)(__DRIVER_OBJECT*);

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

#define FS_CTRL_NUM 4       //How many file system controller,such as NTFS,FAT,etc.

//BEGIN_DEFINE_OBJECT(__IO_MANAGER)
typedef struct tag__IO_MANAGER{
    __DEVICE_OBJECT*               lpDeviceRoot;      //Pointes to device object's list.
    __DRIVER_OBJECT*               lpDriverRoot;      //Pointes to driver object's list.
	__FS_ARRAY_ELEMENT             FsArray[FILE_SYSTEM_NUM];
	__COMMON_OBJECT*               FsCtrlArray[FS_CTRL_NUM];

	__RESOURCE_DESCRIPTOR*         lpResDescriptor;   //Pointes to resource descriptor's list.

	BOOL                 (*Initialize)(__COMMON_OBJECT*    lpThis);  //Initialize routine.

	//
	//Routines to manipulate file systems.
	//
	__COMMON_OBJECT*     (*CreateFile)(__COMMON_OBJECT*    lpThis,
		                               LPSTR               lpszFileName,
									   DWORD               dwAccessMode,
									   DWORD               dwShareMode,
									   LPVOID              lpReserved);  //CreateFile routine.

	BOOL                 (*ReadFile)(__COMMON_OBJECT*   lpThis,
		                             __COMMON_OBJECT*   lpFileObject,
									 DWORD              dwByteSize,
									 LPVOID             lpBuffer,
									 DWORD*             lpReadSize);

	BOOL                 (*WriteFile)(__COMMON_OBJECT*  lpThis,
		                              __COMMON_OBJECT*  lpFileObject,
									  DWORD             dwWriteSize,
									  LPVOID            lpBuffer,
									  DWORD*            lpWrittenSize);

	VOID                 (*CloseFile)(__COMMON_OBJECT*  lpThis,
		                              __COMMON_OBJECT*  lpFileObject);

	BOOL                 (*CreateDirectory)(__COMMON_OBJECT* lpThis,
		                                    LPCTSTR lpszFileName,
											LPVOID  lpReserved);

	BOOL                 (*DeleteFile)(__COMMON_OBJECT* lpThis,
		                               LPCTSTR lpszFileName);

	BOOL                 (*FindClose)(__COMMON_OBJECT* lpThis,
		                              LPCTSTR lpszFileName,
		                              __COMMON_OBJECT* FindHandle);

	__COMMON_OBJECT*     (*FindFirstFile)(__COMMON_OBJECT* lpThis,
		                                  LPCTSTR lpszFileName,
										  FS_FIND_DATA* pFindData);

	BOOL                 (*FindNextFile)(__COMMON_OBJECT* lpThis,
		                                 LPCTSTR lpszFileName,
		                                 __COMMON_OBJECT* FindHandle,
										 FS_FIND_DATA* pFindData);

	DWORD                (*GetFileAttributes)(__COMMON_OBJECT* lpThis,
		                                      LPCTSTR lpszFileName);

	DWORD                (*GetFileSize)(__COMMON_OBJECT* lpThis,
		                                __COMMON_OBJECT* FileHandle,
										DWORD* lpdwSizeHigh);

	BOOL                 (*RemoveDirectory)(__COMMON_OBJECT* lpThis,
		                                    LPCTSTR lpszFileName);

	BOOL                 (*SetEndOfFile)(__COMMON_OBJECT* lpThis,
		                                 __COMMON_OBJECT* FileHandle);

	BOOL                 (*IOControl)(__COMMON_OBJECT*  lpThis,
		                              __COMMON_OBJECT*  lpFileObject,
									  DWORD             dwCommand,
									  DWORD             dwInputLen,
									  LPVOID            lpInputBuffer,
									  DWORD             dwOutputLen,
									  LPVOID            lpOutputBuffer,
									  DWORD*            lpdwOutFilled);

	DWORD                 (*SetFilePointer)(__COMMON_OBJECT*  lpThis,
		                                   __COMMON_OBJECT*  lpFileObject,
										   DWORD*            pdwDistLow,
										   DWORD*            pdwDistHigh,
										   DWORD             dwWhereBegin);

	BOOL                 (*FlushFileBuffers)(__COMMON_OBJECT*  lpThis,
		                                     __COMMON_OBJECT*  lpFileObject);

	//The following routines are used to manipulate device.
	__DEVICE_OBJECT*     (*CreateDevice)(__COMMON_OBJECT*  lpThis,
		                                 LPSTR             lpszDevName,
										 DWORD             dwAttribute,
										 DWORD             dwBlockSize,
										 DWORD             dwMaxReadSize,
										 DWORD             dwMaxWriteSize,
										 LPVOID            lpDevExtension,
										 __DRIVER_OBJECT*  lpDrvObject);

	VOID                 (*DestroyDevice)(__COMMON_OBJECT* lpThis,
		                                  __DEVICE_OBJECT* lpDevObj);

	BOOL                 (*ReserveResource)(__COMMON_OBJECT*  lpThis,
		                                    __RESOURCE_DESCRIPTOR* lpResDesc);

	BOOL                 (*LoadDriver)(__DRIVER_ENTRY DrvEntry);

	//The following routines are used by file system driver.
	BOOL                 (*AddFileSystem)(__COMMON_OBJECT* lpThis,
		                                  __COMMON_OBJECT* lpFileSystem,
										  DWORD            dwAttribute,
										  CHAR*            pVolumeLbl);
	BOOL                 (*RegisterFileSystem)(__COMMON_OBJECT* lpThis,
		                                       __COMMON_OBJECT* lpFileSystem);
}__IO_MANAGER;
//END_DEFINE_OBJECT(__IO_MANAGER)    //End of __IO_MANAGER.

//
//The following macros are used by CreateFile.
//
#define FILE_ACCESS_READ         0x00000001    //Read access.
#define FILE_ACCESS_WRITE        0x00000002    //Write access.
#define FILE_ACCESS_READWRITE    0x00000003    //Read and write access.
#define FILE_ACCESS_CREATE       0x00000004    //Create a new file.

#define FILE_OPEN_ALWAYS         0x80000000    //If can not open one,create a new one then open it.
#define FILE_OPEN_NEW            0x40000000    //Create a new file,overwrite existing if.
#define FILE_OPEN_EXISTING       0x20000000    //Open a existing file,return fail if does not exist.

//
//The following macros are used by SetFilePointer routine.
//
#define FILE_FROM_BEGIN        0x00000001
#define FILE_FROM_CURRENT      0x00000002
#define FILE_FROM_END          0x00000003

/*************************************************************************
**************************************************************************
**************************************************************************
**************************************************************************
*************************************************************************/

//
//The declaration of IOManager object.
//
extern __IO_MANAGER IOManager;

#ifdef __cplusplus
}
#endif

#endif    //iomgr.h
