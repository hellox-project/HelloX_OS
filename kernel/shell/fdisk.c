//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 01 FEB,2009
//    Module Name               : FDISK.CPP
//    Module Funciton           : 
//    Description               : Implementation code of fdisk application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif


#include "kapi.h"
#include "string.h"
#include "stdio.h"

#include "shell.h"
#include "fdisk.h"

#define  FDISK_PROMPT_STR   "[fdisk_view]"

//
//Pre-declare routines.
//
static DWORD CommandParser(LPSTR);
static DWORD disklist(__CMD_PARA_OBJ*);    //disklist sub-command's handler.
static DWORD use(__CMD_PARA_OBJ*);         //use sub-command's handler.
static DWORD partlist(__CMD_PARA_OBJ*);    //partlist sub-commnad's handler.
static DWORD partadd(__CMD_PARA_OBJ*);     //partadd sub-command's handler.
static DWORD partdel(__CMD_PARA_OBJ*);     //partdel sub-command's handler.
static DWORD help(__CMD_PARA_OBJ*);        //help sub-command's handler.
static DWORD exit(__CMD_PARA_OBJ*);        //exit sub-command's handler.
static DWORD pdevlist(__CMD_PARA_OBJ*);
extern DWORD format(__CMD_PARA_OBJ*);      //Implemented in FDISK2.CPP.

//
//The following is a map between command and it's handler.
//
static struct __SHELL_CMD_PARSER_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"disklist",   disklist,  "  disklist : List all disk(s) information in current system."},
	{"use",        use,       "  use      : Select the current operational disk."},
	{"partlist",   partlist,  "  partlist : List all partition(s) in current disk."},
	{"pdevlist",   pdevlist,  "  pdevlist : List all partition object(s) in system."},
	{"format",     format,    "  format   : Use a specified file system to format one partition."},
	{"partadd",    partadd,   "  partadd  : Add one partition to current disk."},
	{"partdel",    partdel,   "  partdel  : Delete one partition from current disk."},
	{"exit",       exit,      "  exit     : Exit the application."},
	{"help",       help,      "  help     : Print out this screen."},
	{NULL,		   NULL,      NULL}
};

//Partition table in MBR.
typedef struct{
	UCHAR      IfActive;            //80 is active,00 is inactive.
	UCHAR      StartCHS1;           //Start cylinder,sector and track.
	UCHAR      StartCHS2;
	UCHAR      StartCHS3;
	UCHAR      PartitionType;       //Partition type,such as FAT32,NTFS.
	UCHAR      EndCHS1;             //End cylinder,sector and track.
	UCHAR      EndCHS2;
	UCHAR      EndCHS3;
	DWORD      dwStartSector;       //Start logical sector.
	DWORD      dwTotalSector;       //Sector number occupied by this partition.
}__PARTITION_TABLE_ENTRY;

//Free gap in disk.
typedef struct{
	DWORD dwStartSectorNum;         //Start sector number of a free gap.
	DWORD dwEndSectorNum;           //End sector number of a free gap.
}__DISK_FREE_GAP;

//A block of global data used to operate current disk.
static struct __MBR_CONTROL_BLOCK{
	__PARTITION_TABLE_ENTRY TableEntry[4];  //One for each partition table entry.
	__COMMON_OBJECT*        pCurrentDisk;   //Current operational disk object.
	__DISK_FREE_GAP         FreeGap[5];     //At most 5 gaps in case of 4 partitions.
	DWORD                   dwDiskSector;   //Sector counter of current disk.
	UCHAR                   DiskName[MAX_DEV_NAME_LEN + 1]; //Current disk's name.
	UCHAR                   SectorBuffer[512]; //MBR buffer.
}MbrControlBlock = {0};

//A helper routine convert LBA address to CHS address.
static VOID LBAtoCHS(DWORD dwLbaAddr,UCHAR* pchs1,UCHAR* pchs2,UCHAR* pchs3)
{
	DWORD  CS = 0;
	DWORD  HS = 0;
	DWORD  SS = 1;
	DWORD  PS = 63;
	DWORD  PH = 255;
	DWORD  c,h,s;

	c = dwLbaAddr / (PH * PS) + CS;
	h = dwLbaAddr / PS - (c - CS) * PS + HS;
	s = dwLbaAddr - (c - CS) * PH * PS - (h - HS) * PS + SS;

	*pchs1 =  (UCHAR)h;           //Header number.
	*pchs2 =  (UCHAR)(s & 0x3F);  //Sector number,occupy low 6 bits.
	*pchs2 &= (UCHAR)((c >> 2) & 0xC0);  //Cylinder number,high 2 bits.
	*pchs3 =  (UCHAR)c;           //Cylinder number,low 8 bits.
}

//Two helper routines used to read or write sector(s) from or into disk.
//Read one or several sector(s) from device object.
//This is the lowest level routine used by all FAT32 driver code.
static BOOL ReadDeviceSector(__COMMON_OBJECT* pPartition,
					  DWORD            dwStartSector,
					  DWORD            dwSectorNum,   //How many sector to read.
					  BYTE*            pBuffer)       //Must equal or larger than request.
{
	BOOL              bResult        = FALSE;
	__DRIVER_OBJECT*  pDrvObject     = NULL;
	__DEVICE_OBJECT*  pDevObject     = (__DEVICE_OBJECT*)pPartition;
	__DRCB*           pDrcb          = NULL;

	if((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	pDrcb = (__DRCB*)CREATE_OBJECT(__DRCB);
	if(NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if(DEVICE_OBJECT_SIGNATURE != pDevObject->dwSignature)
	{
		PrintLine("Invalid device object encountered.");
		goto __TERMINAL;
	}
	//Initialize the DRCB object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_READ_SECTOR;
	pDrcb->dwInputLen      = sizeof(DWORD);
	pDrcb->lpInputBuffer   = (LPVOID)&dwStartSector;    //Input buffer stores the start position pointer.
	pDrcb->dwOutputLen     = dwSectorNum * (pDevObject->dwBlockSize);
	pDrcb->lpOutputBuffer  = pBuffer;
	//Issue the IO control command to read data.
	if(0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))  //Can not read.
	{
		goto __TERMINAL;
	}
	bResult = TRUE;  //Indicate read successfully.

__TERMINAL:
	if(pDrcb)  //Should release it.
	{
		RELEASE_OBJECT(pDrcb);
	}
	return bResult;
}

//Write one or several sector(s) to device.
static BOOL WriteDeviceSector(__COMMON_OBJECT* pPartition,
					  DWORD            dwStartSector,
					  DWORD            dwSectorNum,   //How many sector to write.
					  BYTE*            pBuffer)       //Must equal or larger than request.
{
	BOOL              bResult        = FALSE;
	__DRIVER_OBJECT*  pDrvObject     = NULL;
	__DEVICE_OBJECT*  pDevObject     = (__DEVICE_OBJECT*)pPartition;
	__DRCB*           pDrcb          = NULL;
	__SECTOR_INPUT_INFO ssi;

	if((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer))  //Invalid parameters.
	{
		goto __TERMINAL;
	}
	pDrvObject = pDevObject->lpDriverObject;

	pDrcb = (__DRCB*)CREATE_OBJECT(__DRCB);
	if(NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	ssi.dwBufferLen   = dwSectorNum * pDevObject->dwBlockSize;
	ssi.lpBuffer      = pBuffer;
	ssi.dwStartSector = dwStartSector;
	//Initialize the DRCB object.
	pDrcb->dwStatus        = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode   = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand   = IOCONTROL_WRITE_SECTOR;
	pDrcb->dwInputLen      = sizeof(__SECTOR_INPUT_INFO);
	pDrcb->lpInputBuffer   = (LPVOID)&ssi;
	pDrcb->dwOutputLen     = 0;
	pDrcb->lpOutputBuffer  = NULL;
	//Issue the IO control command to read data.
	if(0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))  //Can not read.
	{
		goto __TERMINAL;
	}
	bResult = TRUE;  //Indicate read successfully.

__TERMINAL:
	if(pDrcb)  //Should release it.
	{
		RELEASE_OBJECT(pDrcb);
	}
	return bResult;
}



//
//The following routine processes the input command string.
static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,SysDiagCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}

//
static DWORD CommandParser(LPSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
		return SHELL_CMD_PARSER_INVALID;

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.If find,then calls the handler,else,return SYS_DIAG_CMD_PARSER_INVALID
	//to indicate the failure.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

//__TERMINAL:
	if(NULL != lpCmdParamObj)
		ReleaseParameterObj(lpCmdParamObj);

	return dwRetVal;
}

//
//This is the application's entry point.
//
DWORD fdiskEntry(LPVOID pParam)
{
	return Shell_Msg_Loop(FDISK_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD exit(__CMD_PARA_OBJ* lpCmdObj)
{
#ifdef __CFG_SYS_DDF
	if(MbrControlBlock.pCurrentDisk)  //Should close it.
	{
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			MbrControlBlock.pCurrentDisk);
	}
	memzero(&MbrControlBlock,sizeof(MbrControlBlock));  //Clear all global information.
	return SHELL_CMD_PARSER_TERMINAL;
#else
	return SHELL_CMD_PARSER_TERMINAL;
#endif
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

//A helper routine to print out disk information.
static VOID DumpDisk(__DEVICE_OBJECT* pDevObj)
{
	CHAR Buffer[256];
	__PARTITION_EXTENSION* pPe = (__PARTITION_EXTENSION*)pDevObj->lpDevExtension;

	_hx_sprintf(Buffer,"    %20s    %9d    0x%X",
		pDevObj->DevName,
		pPe->dwSectorNum,
		pDevObj->dwAttribute);
	PrintLine(Buffer);
}

//disklist sub-command's handler.
static DWORD disklist(__CMD_PARA_OBJ* pcpo)
{
#ifdef __CFG_SYS_DDF
	__DEVICE_OBJECT*  pDevList = (__DEVICE_OBJECT*)IOManager.lpDeviceRoot;
	//UCHAR             DevName[MAX_DEV_NAME_LEN + 1];
	//BYTE              Buffer[512];         //Sector buffer.
	//BOOL              bFound = FALSE;

	PrintLine("    DiskName                 SectorNum    Attribute");
	while(pDevList)
	{
		if(pDevList->dwSignature != DEVICE_OBJECT_SIGNATURE)  //Invalid device object.
		{
			break;
		}
		if(pDevList->dwAttribute & DEVICE_TYPE_HARDDISK)  //This is a hard disk.
		{
			DumpDisk(pDevList);
			StrCpy((CHAR*)&(pDevList->DevName[0]),
				(CHAR*)&MbrControlBlock.DiskName[0]);  //Get the device name as current disk.
		}
		pDevList = pDevList->lpNext;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

//A helper routine to dumpout partition device object information.
static VOID DumpPartDev(__DEVICE_OBJECT* pPartDev)
{
	__PARTITION_EXTENSION*  ppe = (__PARTITION_EXTENSION*)pPartDev->lpDevExtension;
	CHAR    Buffer[128];

	_hx_sprintf(Buffer,"    %16s    %12d    %12d    %8X",
		pPartDev->DevName,
		ppe->dwStartSector,
		ppe->dwSectorNum,
		ppe->PartitionType);
	PrintLine(Buffer);
}

static DWORD pdevlist(__CMD_PARA_OBJ* pcpo)
{
#ifdef __CFG_SYS_DDF
	__DEVICE_OBJECT*  pDevList = (__DEVICE_OBJECT*)IOManager.lpDeviceRoot;
	
	PrintLine("       PartitionName     StartSector    SectorNumber    PartType");
	while(pDevList)
	{
		if(pDevList->dwSignature != DEVICE_OBJECT_SIGNATURE)  //Invalid device object.
		{
			break;
		}
		if(pDevList->dwAttribute & DEVICE_TYPE_PARTITION)  //This is a hard disk.
		{
			DumpPartDev(pDevList);
		}
		pDevList = pDevList->lpNext;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

//A helper routine to check if a partiton table entry is valid.
static BOOL IsValidPte(__PARTITION_TABLE_ENTRY* ppte)
{
	if(ppte->dwTotalSector == 0)
	{
		return FALSE;
	}
	if(ppte->dwStartSector == 0)  //First sector should not occupied by partition.
	{
		return FALSE;
	}
	return TRUE;
}

//use sub-command's handler.
static DWORD use(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	__PARTITION_EXTENSION*     pPe = NULL;
	__PARTITION_TABLE_ENTRY*   ppte = NULL;
	int                        i;

	if(NULL != MbrControlBlock.pCurrentDisk)  //Opened yet.
	{
		PrintLine("  The disk already opened yet.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	//Try to open the disk whose name is MbrControlBlock.DevName.
	MbrControlBlock.pCurrentDisk = IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		(LPSTR)&MbrControlBlock.DiskName[0],
		FILE_ACCESS_READWRITE,
		0,
		NULL);
	if(NULL == MbrControlBlock.pCurrentDisk)  //Failed to open the disk.
	{
		PrintLine("  Can not open the target disk object.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	//Try to read MBR of the disk.
	if(!ReadDeviceSector(
		MbrControlBlock.pCurrentDisk,
		0,
		1,
		(BYTE*)MbrControlBlock.SectorBuffer))
	{
		PrintLine("  Can not read the MBR of target disk.");
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			(__DEVICE_OBJECT*)MbrControlBlock.pCurrentDisk);
		MbrControlBlock.pCurrentDisk = NULL;
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if((0x55 != MbrControlBlock.SectorBuffer[510]) || (0xAA != MbrControlBlock.SectorBuffer[511]))
	{
		PrintLine("  Disk has not been formatted yet.");
	}
	pPe = (__PARTITION_EXTENSION*)((__DEVICE_OBJECT*)MbrControlBlock.pCurrentDisk)->lpDevExtension;
	MbrControlBlock.dwDiskSector = pPe->dwSectorNum;
	ppte = (__PARTITION_TABLE_ENTRY*)(MbrControlBlock.SectorBuffer + 0x1BE);
	for(i = 0;i < 4;i ++)
	{
		if(IsValidPte(ppte))  //Is a valid partition table entry.
		{
			MbrControlBlock.TableEntry[i] = *ppte;  //Copy to MBR control block.
		}
		ppte += 1;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

//partlist sub-commnad's handler.    
static DWORD partlist(__CMD_PARA_OBJ* pcpo)
{
	CHAR Buffer[256];   //Temporary buffer to print out information.
	int  i;

	if(NULL == MbrControlBlock.pCurrentDisk)
	{
		PrintLine("  Current disk is not specified yet.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	PrintLine("     partNum    startsector   totalsector   parttype     actflag");
	for(i = 0;i < 4;i ++)
	{
		_hx_sprintf(Buffer,"    %8d    %10d    %10d    %8X    %8X",
			i,
			MbrControlBlock.TableEntry[i].dwStartSector,
			MbrControlBlock.TableEntry[i].dwTotalSector,
			MbrControlBlock.TableEntry[i].PartitionType,
			MbrControlBlock.TableEntry[i].IfActive);
		PrintLine(Buffer);
	}
	PrintLine("    ");
	PrintLine("    Comment: ");
	PrintLine("      parttype: 0x0B/0x0C : FAT32, 0x07 : NTFS");
	PrintLine("      actflag : 0x80 : partition is active, 0x00 : not active.");
	return SHELL_CMD_PARSER_SUCCESS;
}

//Helper routine used by partadd command.
static void partaddusage()
{
	PrintLine("  usage: partadd partnum parttype startsector sectornum [primary]");
	PrintLine("  where:");
	PrintLine("    partnum     : Partition number,from 0 to 3.");
	PrintLine("    parttype    : File system type in this part,fat32 is available now.");
	PrintLine("    startsector : Start sector number of this partition.");
	PrintLine("    sectornum   : How many sector this partition occupies.");
	PrintLine("    primary     : Active partition if specified.");
}

//partadd sub-command's handler.
static DWORD partadd(__CMD_PARA_OBJ* pCmdParam)
{
	DWORD    dwPartNum    = 0;
	UCHAR    fstype       = PARTITION_TYPE_FAT32;  //Default is FAT32.
	DWORD    dwStartSect  = 0;
	DWORD    dwSectorNum  = 0;
	BOOL     bIsActive    = FALSE;
	BYTE     Buffer[512]  = {0};    //Used to fill the first sector of a partition.

	__PARTITION_TABLE_ENTRY* ppte = (__PARTITION_TABLE_ENTRY*)(MbrControlBlock.SectorBuffer + 0x1be);

	if(pCmdParam->byParameterNum < 5)  //At least 4 parameter.
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if(NULL == MbrControlBlock.pCurrentDisk)
	{
		PrintLine("  Current disk is not specified yet.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	//Now try to interpret the parameters.
	/*if(!Str2Hex(pCmdParam->Parameter[1],&dwPartNum))
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}*/
	dwPartNum = atoi(pCmdParam->Parameter[1]);
	if(dwPartNum > 3)  //Invalid parameter.
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(!StrCmp("fat32",pCmdParam->Parameter[2]))
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	/*if(!Str2Hex(pCmdParam->Parameter[3],&dwStartSect))
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}*/
	dwStartSect = atoi(pCmdParam->Parameter[3]);
	if(dwStartSect >= MbrControlBlock.dwDiskSector)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	/*
	if(!Str2Hex(pCmdParam->Parameter[4],&dwSectorNum))
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}*/
	dwSectorNum = atoi(pCmdParam->Parameter[4]);
	if(dwSectorNum > MbrControlBlock.dwDiskSector)  //Invalid sector number.
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(dwStartSect + dwSectorNum > MbrControlBlock.dwDiskSector)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	if(pCmdParam->byParameterNum == 6)  //primary maybe specified.
	{
		if('p' != pCmdParam->Parameter[5][0])
		{
			partaddusage();
			return SHELL_CMD_PARSER_SUCCESS;
		}
		bIsActive = TRUE;
	}
	//Now all parameters have been interpretted,carry out the partition function now.
	ppte += dwPartNum;
	memzero(ppte,sizeof(__PARTITION_TABLE_ENTRY));
	ppte->dwStartSector   = dwStartSect;
	ppte->dwTotalSector   = dwSectorNum;
	ppte->PartitionType   = PARTITION_TYPE_FAT32;
	if(bIsActive)
	{
		ppte->IfActive = 0x80;
	}
	LBAtoCHS(dwStartSect,&ppte->StartCHS1,&ppte->StartCHS2,&ppte->StartCHS3);
	LBAtoCHS(dwStartSect + dwSectorNum,&ppte->EndCHS1,&ppte->EndCHS2,&ppte->EndCHS3);

	//Set the terminate flags.
	MbrControlBlock.SectorBuffer[510] = 0x55;
	MbrControlBlock.SectorBuffer[511] = 0xAA;
	if(!WriteDeviceSector(MbrControlBlock.pCurrentDisk,
		0,
		1,
		(BYTE*)MbrControlBlock.SectorBuffer))
	{
		PrintLine("  Add partition failed,please try again.");
		return SHELL_CMD_PARSER_SUCCESS;
	}
	//Now overwrite the first sector of the partition created just now.
	if(!WriteDeviceSector(MbrControlBlock.pCurrentDisk,
		dwStartSect,
		1,
		Buffer))
	{
		PrintLine("  Can not overwrite the first sector of the new partition.");
	}

	PrintLine("  Add partition successfully.");
	//Save to global variable.
	MbrControlBlock.TableEntry[dwPartNum] = *ppte;
	return SHELL_CMD_PARSER_SUCCESS;
}

//partdel sub-command's handler.
static DWORD partdel(__CMD_PARA_OBJ* pcpo)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

