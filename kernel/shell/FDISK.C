//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 01 FEB,2009
//    Module Name               : fdisk.c
//    Module Funciton           : 
//    Description               : Hard disk operation tools as DOS's fdisk.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include "StdAfx.h"
#include "kapi.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "shell.h"
#include "fdisk.h"

#define  FDISK_PROMPT_STR   "[fdisk_view]"

/* Routines invoked in this module. */
static DWORD CommandParser(LPSTR);
static DWORD disklist(__CMD_PARA_OBJ*);
static DWORD use(__CMD_PARA_OBJ*);
static DWORD partlist(__CMD_PARA_OBJ*);
static DWORD partadd(__CMD_PARA_OBJ*);
static DWORD partdel(__CMD_PARA_OBJ*);
static DWORD help(__CMD_PARA_OBJ*);
static DWORD _exit(__CMD_PARA_OBJ*);
static DWORD pdevlist(__CMD_PARA_OBJ*);
extern DWORD format(__CMD_PARA_OBJ*);

/* Command and handler's map. */
static struct __SHELL_CMD_PARSER_MAP{
	LPSTR lpszCommand;
	unsigned long (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"disklist",   disklist,  "  disklist : List all available disk(s)."},
	{"use",        use,       "  use      : Select current disk."},
	{"partlist",   partlist,  "  partlist : List all partition(s)."},
	{"pdevlist",   pdevlist,  "  pdevlist : List all partition object(s)."},
	{"format",     format,    "  format   : Format one partition."},
	{"partadd",    partadd,   "  partadd  : Add one partition."},
	{"partdel",    partdel,   "  partdel  : Delete one partition."},
	{"exit",       _exit,     "  exit     : Exit."},
	{"help",       help,      "  help     : Print out this screen."},
	{NULL,		   NULL,      NULL}
};

/* Partition table entry in MBR. */
#pragma pack(push, 1)
typedef struct{
	/* 80 is active,00 is inactive. */
	UCHAR      IfActive;
	/* Start cylinder,sector and track. */
	UCHAR      StartCHS1;
	UCHAR      StartCHS2;
	UCHAR      StartCHS3;
	/* Partition type. */
	UCHAR      PartitionType;
	/* End cylinder,sector and track. */
	UCHAR      EndCHS1;
	UCHAR      EndCHS2;
	UCHAR      EndCHS3;
	/* Start logical sector, LBA mode. */
	uint32_t dwStartSector;
	/* Total sector number. */
	uint32_t dwTotalSector;
}__PARTITION_TABLE_ENTRY;
#pragma pack(pop)

/* Helper structure to operate disk. */
static struct __MBR_CONTROL_BLOCK{
	/* Partition entry table. */
	__PARTITION_TABLE_ENTRY TableEntry[4];
	/* Current disk object. */
	__DEVICE_OBJECT* pCurrentDisk;
	/* Sector counter of current disk. */
	unsigned long dwDiskSector;
	UCHAR DiskName[MAX_DEV_NAME_LEN + 1];
	/* Buffer holds current disk's MBR. */
	unsigned char* pMbrBuffer;
}MbrControlBlock = {0};

/* Helper routine convert LBA address to CHS address. */
static VOID LBAtoCHS(DWORD dwLbaAddr, UCHAR* pchs1, UCHAR* pchs2, UCHAR* pchs3)
{
	DWORD  CS = 0;
	DWORD  HS = 0;
	DWORD  SS = 1;
	DWORD  PS = 63;
	DWORD  PH = 255;
	DWORD  c, h, s;

	c = dwLbaAddr / (PH * PS) + CS;
	h = dwLbaAddr / PS - (c - CS) * PS + HS;
	s = dwLbaAddr - (c - CS) * PH * PS - (h - HS) * PS + SS;

	*pchs1 = (UCHAR)h;           //Header number.
	*pchs2 = (UCHAR)(s & 0x3F);  //Sector number,occupy low 6 bits.
	*pchs2 &= (UCHAR)((c >> 2) & 0xC0);  //Cylinder number,high 2 bits.
	*pchs3 = (UCHAR)c;           //Cylinder number,low 8 bits.
}

/*
 * Two helper routines used to read or write sector(s) 
 * from or into a parttion or hard disk.
 * These 2 routines are used by the whole fdisk app,
 * include other code modules in other source file.
 */
BOOL __ReadDeviceSector(__COMMON_OBJECT* pPartition,
	DWORD dwStartSector,
	DWORD dwSectorNum,
	BYTE* pBuffer)
{
	BOOL bResult = FALSE;
	__DRIVER_OBJECT* pDrvObject = NULL;
	__DEVICE_OBJECT* pDevObject = (__DEVICE_OBJECT*)pPartition;
	__DRCB* pDrcb = NULL;

	BUG_ON((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer));
	pDrvObject = pDevObject->lpDriverObject;

	/* New a drcb object to carry request. */
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		goto __TERMINAL;
	}

	/* Initializes DRCB object. */
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_READ_SECTOR;
	pDrcb->dwInputLen = sizeof(DWORD);
	/* Use input buffer to store the start sector. */
	pDrcb->lpInputBuffer = (LPVOID)&dwStartSector;
	pDrcb->dwOutputLen = dwSectorNum * 512; //(pDevObject->dwBlockSize);
	pDrcb->lpOutputBuffer = pBuffer;
	/* Issue read sector command to device. */
	if (0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))
	{
		/* Read fail. */
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if (pDrcb)
	{
		/* Release DRCB. */
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* 
 * Helper routine that writes 
 * one or several sector(s) to a disk or
 * partition device. 
 */
BOOL __WriteDeviceSector(__COMMON_OBJECT* pPartition,
	DWORD dwStartSector,
	DWORD dwSectorNum,
	BYTE* pBuffer)
{
	BOOL bResult = FALSE;
	__DRIVER_OBJECT* pDrvObject = NULL;
	__DEVICE_OBJECT* pDevObject = (__DEVICE_OBJECT*)pPartition;
	__DRCB* pDrcb = NULL;
	__SECTOR_INPUT_INFO ssi;

	BUG_ON((NULL == pPartition) || (0 == dwSectorNum) || (NULL == pBuffer));
	pDrvObject = pDevObject->lpDriverObject;

	/* New a drcb object to carry request. */
	pDrcb = (__DRCB*)ObjectManager.CreateObject(&ObjectManager,
		NULL, OBJECT_TYPE_DRCB);
	if (NULL == pDrcb)
	{
		goto __TERMINAL;
	}
	if (!pDrcb->Initialize((__COMMON_OBJECT*)pDrcb))
	{
		goto __TERMINAL;
	}

	/* use SSI to xfer input information. */
	ssi.dwBufferLen = dwSectorNum * STORAGE_DEFAULT_SECTOR_SIZE;
	ssi.lpBuffer = pBuffer;
	ssi.dwStartSector = dwStartSector;
	/* Initializes DRCB object. */
	pDrcb->dwStatus = DRCB_STATUS_INITIALIZED;
	pDrcb->dwRequestMode = DRCB_REQUEST_MODE_IOCTRL;
	pDrcb->dwCtrlCommand = IOCONTROL_WRITE_SECTOR;
	pDrcb->dwInputLen = sizeof(__SECTOR_INPUT_INFO);
	pDrcb->lpInputBuffer = (LPVOID)&ssi;
	pDrcb->dwOutputLen = 0;
	pDrcb->lpOutputBuffer = NULL;

	/* Issue write sector command to device. */
	if (0 == pDrvObject->DeviceCtrl((__COMMON_OBJECT*)pDrvObject,
		(__COMMON_OBJECT*)pDevObject,
		pDrcb))
	{
		/* Write fail. */
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	if (pDrcb)
	{
		/* Release DRCB. */
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)pDrcb);
	}
	return bResult;
}

/* Process user's input string. */
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

/* Command line's parser routine. */
static DWORD CommandParser(LPSTR lpszCmdLine)
{
	DWORD dwRetVal = SHELL_CMD_PARSER_INVALID;
	DWORD dwIndex = 0;
	__CMD_PARA_OBJ* lpCmdParamObj = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))
		return SHELL_CMD_PARSER_INVALID;

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	if(NULL == lpCmdParamObj)
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	if(0 == lpCmdParamObj->byParameterNum)
	{
		/* No parameter specified. */
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
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))
		{
			/* Handler located. */
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

	if(NULL != lpCmdParamObj)
		ReleaseParameterObj(lpCmdParamObj);

	return dwRetVal;
}

/* Entry point of fdisk tool. */
unsigned long fdiskEntry(LPVOID pParam)
{
	/* Allocate MBR buffer. */
	MbrControlBlock.pMbrBuffer = (unsigned char*)_hx_malloc(512);
	if (NULL == MbrControlBlock.pMbrBuffer)
	{
		return SHELL_CMD_PARSER_TERMINAL;
	}
	memset(MbrControlBlock.pMbrBuffer, 0, 512);

	return Shell_Msg_Loop(FDISK_PROMPT_STR, CommandParser, QueryCmdName);	
}

/* exit command's handler. */
static DWORD _exit(__CMD_PARA_OBJ* lpCmdObj)
{
#ifdef __CFG_SYS_DDF
	if(MbrControlBlock.pCurrentDisk)
	{
		/* Close the opened current disk. */
		IOManager.CloseFile((__COMMON_OBJECT*)&IOManager,
			(__COMMON_OBJECT*)MbrControlBlock.pCurrentDisk);
	}
	if (MbrControlBlock.pMbrBuffer)
	{
		_hx_free(MbrControlBlock.pMbrBuffer);
	}
	memzero(&MbrControlBlock, sizeof(MbrControlBlock));
	return SHELL_CMD_PARSER_TERMINAL;
#else
	return SHELL_CMD_PARSER_TERMINAL;
#endif
}

/* Handler of help command. */
static unsigned long help(__CMD_PARA_OBJ* lpCmdObj)
{
	unsigned long dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;
		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Local helper to show out one disk object. */
static VOID DumpDisk(__DEVICE_OBJECT* pDevObj)
{
	char Buffer[256];
	__ATA_DISK_OBJECT* pDisk = (__ATA_DISK_OBJECT*)pDevObj->lpDevExtension;

	BUG_ON(NULL == pDisk);
	_hx_sprintf(Buffer,"    %20s    %9d    0x%X",
		pDevObj->DevName,
		pDisk->total_sectors,
		pDevObj->dwAttribute);
	PrintLine(Buffer);
}

/* Show out all disks in system. */
static DWORD disklist(__CMD_PARA_OBJ* pcpo)
{
#ifdef __CFG_SYS_DDF
	__DEVICE_OBJECT* pDevList = (__DEVICE_OBJECT*)IOManager.lpDeviceRoot;

	/* Table header. */
	PrintLine("    DiskName                 SectorNum    Attribute");
	/* Travel the whole device list. */
	while(pDevList)
	{
		if(pDevList->dwSignature != DEVICE_OBJECT_SIGNATURE)
		{
			break;
		}
		if(pDevList->dwAttribute & DEVICE_TYPE_HARDDISK)
		{
			/* Hard disk object,show it. */
			DumpDisk(pDevList);
			/* Save as current disk. */
			StrCpy((CHAR*)&(pDevList->DevName[0]), (CHAR*)&MbrControlBlock.DiskName[0]);
		}
		pDevList = pDevList->lpNext;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

/* Show out a partition object. */
static VOID DumpPartDev(__DEVICE_OBJECT* pPartDev)
{
	__PARTITION_EXTENSION* ppe = (__PARTITION_EXTENSION*)pPartDev->lpDevExtension;
	CHAR Buffer[128];

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
	__DEVICE_OBJECT* pDevList = (__DEVICE_OBJECT*)IOManager.lpDeviceRoot;
	
	PrintLine("       PartitionName     StartSector    SectorNumber    PartType");
	while(pDevList)
	{
		if(pDevList->dwSignature != DEVICE_OBJECT_SIGNATURE)
		{
			break;
		}
		if(pDevList->dwAttribute & DEVICE_TYPE_PARTITION)
		{
			/* Partition object, show it. */
			DumpPartDev(pDevList);
		}
		pDevList = pDevList->lpNext;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

/* Helper routine used to check partition table entry. */
static BOOL IsValidPte(__PARTITION_TABLE_ENTRY* ppte)
{
	if(ppte->dwTotalSector == 0)
	{
		return FALSE;
	}
	if(ppte->dwStartSector == 0)
	{
		return FALSE;
	}
	return TRUE;
}

/* Helper routine to show out MBR. */
static void __show_mbr(char* pMbr)
{
	_hx_printf("\r\n");
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			_hx_printf("%02X", pMbr[i * 32 + j]);
		}
		_hx_printf("\r\n");
	}
}

/* 
 * use sub-command's handler. 
 * Select current disk object that other
 * operation will apply on.
 */
static unsigned long use(__CMD_PARA_OBJ* pCmdObj)
{
#ifdef __CFG_SYS_DDF
	__PARTITION_EXTENSION* pPe = NULL;
	__PARTITION_TABLE_ENTRY* ppte = NULL;
	int i = 0;
	char disk_name[MAX_FILE_NAME_LEN];

	/* Init the disk name to be opened. */
	if (pCmdObj->byParameterNum < 2)
	{
		strncpy(disk_name, &MbrControlBlock.DiskName[0], MAX_FILE_NAME_LEN - 1);
	}
	else {
		strncpy(disk_name, pCmdObj->Parameter[1], MAX_FILE_NAME_LEN);
	}

	if(NULL != MbrControlBlock.pCurrentDisk)
	{
		/* Disk already opened, close it. */;
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			(__DEVICE_OBJECT*)MbrControlBlock.pCurrentDisk);
	}

	/* Open the disk object. */
	MbrControlBlock.pCurrentDisk = (__DEVICE_OBJECT*)IOManager.CreateFile(
		(__COMMON_OBJECT*)&IOManager,
		&disk_name[0],
		FILE_ACCESS_READWRITE,
		0, NULL);
	if(NULL == MbrControlBlock.pCurrentDisk)
	{
		_hx_printf("  Can not open disk.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Read MBR from the disk into MBR buffer. */
	if(!__ReadDeviceSector(
		(__COMMON_OBJECT*)MbrControlBlock.pCurrentDisk,
		0, 1,
		MbrControlBlock.pMbrBuffer))
	{
		_hx_printf("  Read MBR fail.\r\n");
		IOManager.DestroyDevice((__COMMON_OBJECT*)&IOManager,
			(__DEVICE_OBJECT*)MbrControlBlock.pCurrentDisk);
		MbrControlBlock.pCurrentDisk = NULL;
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if((0x55 != MbrControlBlock.pMbrBuffer[510]) || 
		(0xAA != MbrControlBlock.pMbrBuffer[511]))
	{
		_hx_printf("  Not formated yet.\r\n");
	}

	/* Get disk's size. */
	__ATA_DISK_OBJECT* pDisk = (__ATA_DISK_OBJECT*)MbrControlBlock.pCurrentDisk->lpDevExtension;
	BUG_ON(NULL == pDisk);
	MbrControlBlock.dwDiskSector = pDisk->total_sectors;

	/* Locate the partition table entry. */
	ppte = (__PARTITION_TABLE_ENTRY*)(MbrControlBlock.pMbrBuffer + 0x1be);
	for (i = 0; i < 4; i++)
	{
		/* Valid partition table entry. */
		MbrControlBlock.TableEntry[i] = *ppte;
		ppte += 1;
	}
	return SHELL_CMD_PARSER_SUCCESS;
#else
	return SHELL_CMD_PARSER_FAILED;
#endif
}

/* List all partitions on current disk. */
static DWORD partlist(__CMD_PARA_OBJ* pcpo)
{
	CHAR Buffer[256];
	int  i;

	if(NULL == MbrControlBlock.pCurrentDisk)
	{
		PrintLine("  No current disk specified.");
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
	PrintLine("    partnum     : Partition number,from 0 to 3;");
	PrintLine("    parttype    : File system type;");
	PrintLine("    startsector : Start sector number;");
	PrintLine("    sectornum   : Sector counter;");
	PrintLine("    primary     : Active partition.");
}

/*
 * Add one partition entry into partition table of
 * a harddisk, the corresponding old one will be
 * overwrited by the new one.
 */
static DWORD partadd(__CMD_PARA_OBJ* pCmdParam)
{
	unsigned long dwPartNum = 0;
	UCHAR fstype = PARTITION_TYPE_FAT32;
	unsigned long dwStartSect = 0, dwSectorNum = 0;
	BOOL bIsActive = FALSE;

	if (pCmdParam->byParameterNum < 5)
	{
		/* No enough command parameters. */
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if (NULL == MbrControlBlock.pCurrentDisk)
	{
		PrintLine("  No current disk.");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	__PARTITION_TABLE_ENTRY* ppte = (__PARTITION_TABLE_ENTRY*)(MbrControlBlock.pMbrBuffer + 0x1be);

	/* Partition number(index in partition table). */
	dwPartNum = atoi(pCmdParam->Parameter[1]);
	if (dwPartNum > 3)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}
	/* Partition type, only FAT32 is supported. */
	if ((0 != strcmp("fat32", pCmdParam->Parameter[2])) &&
		(0 != strcmp("FAT32", pCmdParam->Parameter[2])))
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Start sector of the partition on disk. */
	dwStartSect = atoi(pCmdParam->Parameter[3]);
	if (dwStartSect >= MbrControlBlock.dwDiskSector)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Total sector number of this partition. */
	dwSectorNum = atoi(pCmdParam->Parameter[4]);
	if (dwSectorNum > MbrControlBlock.dwDiskSector)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if (dwStartSect + dwSectorNum > MbrControlBlock.dwDiskSector)
	{
		partaddusage();
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if (pCmdParam->byParameterNum == 6)
	{
		/* Primary partition flag is specified. */
		if ('p' != pCmdParam->Parameter[5][0])
		{
			partaddusage();
			return SHELL_CMD_PARSER_SUCCESS;
		}
		bIsActive = TRUE;
	}

	/* Construct the partition table entry. */
	ppte += dwPartNum;
	memzero(ppte, sizeof(__PARTITION_TABLE_ENTRY));
	ppte->dwStartSector = dwStartSect;
	ppte->dwTotalSector = dwSectorNum;
	ppte->PartitionType = PARTITION_TYPE_FAT32;
	if (bIsActive)
	{
		ppte->IfActive = 0x80;
	}
	LBAtoCHS(dwStartSect, &ppte->StartCHS1, &ppte->StartCHS2, &ppte->StartCHS3);
	LBAtoCHS(dwStartSect + dwSectorNum, &ppte->EndCHS1, &ppte->EndCHS2, &ppte->EndCHS3);

	/* Set terminal flags of MBR. */
	MbrControlBlock.pMbrBuffer[510] = 0x55;
	MbrControlBlock.pMbrBuffer[511] = 0xAA;
	if (!__WriteDeviceSector((__COMMON_OBJECT*)MbrControlBlock.pCurrentDisk,
		0, 1, MbrControlBlock.pMbrBuffer))
	{
		_hx_printf("  Add partition failed.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	_hx_printf("  Done.\r\n");
	/* Save to global array. */
	MbrControlBlock.TableEntry[dwPartNum] = *ppte;
	return SHELL_CMD_PARSER_SUCCESS;
}

/*
 * Delete a partition from disk. It 
 * just delete the corresponding partition table
 * entry in current disk.
 */
static DWORD partdel(__CMD_PARA_OBJ* pcpo)
{
	unsigned long dwPartNum = 0;

	if (pcpo->byParameterNum < 2)
	{
		/* No enough command parameters. */
		_hx_printf("  No part number specified.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	if (NULL == MbrControlBlock.pCurrentDisk)
	{
		_hx_printf("  No current disk specified.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	__PARTITION_TABLE_ENTRY* ppte = (__PARTITION_TABLE_ENTRY*)(MbrControlBlock.pMbrBuffer + 0x1be);

	/* Partition number(index in partition table). */
	dwPartNum = atoi(pcpo->Parameter[1]);
	if (dwPartNum > 3)
	{
		_hx_printf("  Invalid part number.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	/* Clear the corresponding partition table entry. */
	ppte += dwPartNum;
	memzero(ppte, sizeof(__PARTITION_TABLE_ENTRY));

	/* Set terminal flags of MBR. */
	MbrControlBlock.pMbrBuffer[510] = 0x55;
	MbrControlBlock.pMbrBuffer[511] = 0xAA;
	if (!__WriteDeviceSector((__COMMON_OBJECT*)MbrControlBlock.pCurrentDisk,
		0, 1, MbrControlBlock.pMbrBuffer))
	{
		_hx_printf("  Del partition failed.\r\n");
		return SHELL_CMD_PARSER_SUCCESS;
	}

	_hx_printf("  Done.\r\n");
	/* Save to global array. */
	MbrControlBlock.TableEntry[dwPartNum] = *ppte;
	return SHELL_CMD_PARSER_SUCCESS;
}
