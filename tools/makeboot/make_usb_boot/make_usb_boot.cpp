// make_usb_boot.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "make_usb_boot.h"



VHD_INFO g_szVhdInfo = 
{
	0x78697463656E6F63,
	0x02000000,
	0x00000100,
	0xFFFFFFFFFFFFFFFF,
	0x0AF89717,
	0x20637076,
	0x00000100,
	0x6B326957,
	0x0000800c00000000,
	0x0000800c00000000,
	0x110CEB03,
	0x02000000,
	0x5DE8FFFF,
	{0x10,0x4b,0x2e,0x1e,0x73,0x6c,0x4a,0xd4,0x82,0x86,0x5e,0x89,0x6b,0x3d,0xcd,0x32},
	{0}	
};

BOOL       g_bUsbFound             = FALSE;
TCHAR      g_szUSBDeviceName[256]  = {0};
TCHAR      g_szUsbDirName[256]     = {0};

TCHAR      g_szProcessInfo[256]    = {0};
DISK_INFO  g_szDiskInfo            = {0};



//设置主分区信息
BOOL APIENTRY SetMainPationInfo(LPBYTE pBuf,UINT  nReserved,UINT  nTotalSector)
{
	MAIN_PATION_INFO* pPationInfo            = NULL;

	UINT              nSectors               = 0;
	UINT              nClinderSecots         = 0; 
	UINT              nTrackSecots           = 0;


	pPationInfo = (MAIN_PATION_INFO*)(pBuf+HX_BOOTSEC_MPS);
	pPationInfo->bActiveFlage = 0x80;
	pPationInfo->bPationType  = 0x0c;

	//根据fat32分区LBA地址计算出CHS地址
	nSectors     = nReserved;
	if(nReserved == HX_HDD_RESERVED)
	{
		pPationInfo->bStartClinder = (BYTE)(nSectors/(g_szDiskInfo.nSectorsPerTrack*g_szDiskInfo.nTracksPerCylinder));
		nClinderSecots             = (pPationInfo->bStartClinder*g_szDiskInfo.nTracksPerCylinder*g_szDiskInfo.nSectorsPerTrack);

		nSectors                  -= nClinderSecots;
		pPationInfo->bStartTrack   = (BYTE)(nSectors/g_szDiskInfo.nSectorsPerTrack); 
		nTrackSecots               = pPationInfo->bStartTrack*g_szDiskInfo.nSectorsPerTrack; 
		nSectors                  -= nTrackSecots;

		pPationInfo->bStartSector   = nSectors+1;
		pPationInfo->nBeforSectors   = nReserved;

		pPationInfo->nPationSectors  = nTotalSector - HX_HDD_RESERVED-HX_FAT32_BLANK;

	}
	else 
	{  
		UINT  nClinderPerTrack     = HX_VDISK_CLINDER_PER_TRACK;
		UINT  nTrackPerSector      = HX_VDISK_TRACK_PER_SECTOR;

		pPationInfo->bStartClinder = (BYTE)(nSectors/(nTrackPerSector*nClinderPerTrack));
		nClinderSecots             = (pPationInfo->bStartClinder*nClinderPerTrack*nTrackPerSector);

		nSectors                  -= nClinderSecots;
		pPationInfo->bStartTrack   = (BYTE)(nSectors/nTrackPerSector); 
		nTrackSecots               = pPationInfo->bStartTrack*nTrackPerSector; 
		nSectors                  -= nTrackSecots;

		pPationInfo->bStartSector    = nSectors+1;
		pPationInfo->nBeforSectors   = nReserved;

		pPationInfo->nPationSectors  = nTotalSector - HX_VDISK_RESERVED-HX_FAT32_BLANK;
	}

	return TRUE;
}

//写引导扇区
BOOL  APIENTRY WriteBootSec(HANDLE hDiskDrive,LPCTSTR pAppPath,UINT  nReserved,UINT nTotalSector)
{
	TCHAR    szBootFile[512]             = {0};
	BYTE     szBuf[HX_SECTOR_SIZE]       = {0};	
	HANDLE   hDiskFile                   = NULL;	
	DWORD    dwRead                      = 0;
	DWORD    dwWrite                     = 0;

	
	wsprintf(szBootFile,L"%sbootsect",pAppPath);

	hDiskFile = CreateFile(szBootFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
	if(hDiskFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	ReadFile(hDiskFile,szBuf,sizeof(szBuf),&dwRead,NULL);
	if(dwRead < sizeof(szBuf))
	{
		return FALSE;

	}

	SetMainPationInfo(szBuf,nReserved,nTotalSector);

	//将引导区信息写入扇区
	WriteFile(hDiskDrive,szBuf,sizeof(szBuf),&dwWrite,NULL);
	WriteFile(hDiskDrive,szBuf,sizeof(szBuf),&dwWrite,NULL);

	CloseHandle(hDiskFile);

	return TRUE;
}

//写HX内核
BOOL  APIENTRY WriteHxKernel(HANDLE hDiskDrive,LPCTSTR pAppPath)
{
	TCHAR    szKernelFile[512]           = {0};
	BYTE     szBuf[HX_SECTOR_SIZE]       = {0};	
	HANDLE   hDiskFile                   = NULL;	
	DWORD    dwRead                      = 0;
	DWORD    dwWrite                     = 0;

	wsprintf(szKernelFile,L"%shcnimge.bin",pAppPath);	
	hDiskFile = CreateFile(szKernelFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
	if(hDiskFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	//写实际内核数据
	SetFilePointer(hDiskDrive,HX_KERNEL_START*HX_SECTOR_SIZE,NULL,FILE_BEGIN);
	while(TRUE)
	{				
		ReadFile(hDiskFile,szBuf,sizeof(szBuf),&dwRead,NULL);
		WriteFile(hDiskDrive,szBuf,sizeof(szBuf),&dwWrite,NULL);

		ZeroMemory(szBuf,sizeof(szBuf));
		if(dwRead < sizeof(szBuf) )
		{
			break;
		}
	}

	CloseHandle(hDiskFile);

	return TRUE;
}

//得到设备信息
UINT64 APIENTRY GetDiriveInfo(LPCTSTR pDriveName)
{
	CHANGER_PRODUCT_DATA szInfo = {0};
	HANDLE         hDiskDrive    = NULL;
	DISK_GEOMETRY  pg            = {0};
	UINT64         nDiriveSize   = 0;
	DWORD          dwRetSize     = 0;


	hDiskDrive = CreateFile(pDriveName,0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,0);//GENERIC_READ|GENERIC_WRITE
	if(hDiskDrive == INVALID_HANDLE_VALUE)
	{
		return nDiriveSize;
	}


	DeviceIoControl(hDiskDrive,    // device to be queried
		IOCTL_CHANGER_GET_PRODUCT_DATA,    // operation to perform
		NULL, 0,              // no input buffer
		&szInfo, sizeof(szInfo),    // output buffer
		&dwRetSize,                // # bytes returned
		(LPOVERLAPPED) NULL);  // synchronous I/O

	DeviceIoControl(hDiskDrive,    // device to be queried
		IOCTL_DISK_GET_DRIVE_GEOMETRY,    // operation to perform
		NULL, 0,              // no input buffer
		&pg, sizeof(pg),    // output buffer
		&dwRetSize,                // # bytes returned
		(LPOVERLAPPED) NULL);  // synchronous I/O

	g_szDiskInfo.nCylinders           = pg.Cylinders.LowPart;
	g_szDiskInfo.nTracksPerCylinder   = pg.TracksPerCylinder;
	g_szDiskInfo.nSectorsPerTrack     = pg.SectorsPerTrack;
	g_szDiskInfo.nTotalSector         = (UINT64)pg.Cylinders.LowPart*(UINT64)pg.TracksPerCylinder*(UINT64)pg.SectorsPerTrack;
	g_szDiskInfo.nTotalSize           = g_szDiskInfo.nTotalSector*HX_SECTOR_SIZE;


	CloseHandle(hDiskDrive);

	return nDiriveSize;
}

BOOL APIENTRY  Detect_UsbDevice(HWND hDlg,TCHAR chDriveIndex)
{
	TCHAR szDriveName[256] = {0};
	UINT  nDriveType       = 0;
	

	wsprintf(szDriveName,L"%c:\\",chDriveIndex);
	wsprintf(g_szUsbDirName,L"\\\\.\\%c:",chDriveIndex);

	nDriveType = GetDriveType(szDriveName);
	if(nDriveType == DRIVE_REMOVABLE)
		{
			
		for(DWORD i = 26;i >= 0;i--)
			{				
			
			wsprintf(g_szUSBDeviceName,L"\\\\.\\PHYSICALDRIVE%d",i);
			GetDiriveInfo(g_szUSBDeviceName);
			if(g_szDiskInfo.nTotalSize != 0)
				{
					g_bUsbFound = TRUE;
					break;
				}
			}

		if(g_bUsbFound )			
			{
			TCHAR  szUsbInfo[256] = {0};
			
			wsprintf(szUsbInfo,L"当前USB设备 (%c:)  容量:%dGB",chDriveIndex,(UINT)(g_szDiskInfo.nTotalSize/1024/1024/1024));
			SetDlgItemText(hDlg,IDC_USB_INFO,szUsbInfo);

			return g_bUsbFound;
			}
		}

	SetDlgItemText(hDlg,IDC_USB_INFO,L"没有发现USB设备，请插入您的USB设备");
	g_szDiskInfo.nTotalSize  = 0;
	g_bUsbFound              = FALSE;

	return g_bUsbFound;
}

//查询USB设备
BOOL APIENTRY  Query_UsbDevice(HWND hDlg)
{
	for(TCHAR nDriveIndex = 'Z';nDriveIndex >= 'A'; nDriveIndex--)
	{
		if(Detect_UsbDevice(hDlg,nDriveIndex) == TRUE)
		{
			return TRUE;			
		}
	}

	return FALSE;
}

VOID  APIENTRY SetProcessInfo(HWND hWnd,UINT nPID)
{
	switch(nPID)
	{
		case  PROCESS_MAKE_BOOT:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，请不要拔出U盘\r\n正在写入引导扇区......");
			}
			break;
		case  PROCESS_MAKE_KERNEL:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，请不要拔出U盘\r\n正在写入HX内核......");
			}
			break;
		case  PROCESS_MAKE_FAT32:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，请不要拔出U盘\r\n正在格式化文件系统......");
			}
			break;
		case  PROCESS_INIT_DEIVCE:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，请不要拔出U盘\r\n正在初始化设备......");
			}
			break;
		case PROCESS_INIT_VHD:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，正在初始化虚拟硬盘文件.....");
			}
			break;
		case PROCESS_IMPORT_VHD:
			{
			wsprintf(g_szProcessInfo,L"正在生成BOOT信息，正在导入初始化文件.....");
			}
			break;
		default:
			return;
	}

	PostMessage(hWnd,WM_MAKE_PROCESS,0,0);
}

VOID  APIENTRY SetCompleteInfo(HWND hWnd,UINT nId)
{
	switch(nId)
	{
	case  RET_COMPLETE:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成成功!");
		}
		break;
	case  RET_USB_ERR:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成失败:USB无法访问");
		}
		break;
	case  RET_BOOTSEC_ERR:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成失败:引导文件读取失败");
		}
		break;
	case  RET_KERNEL_ERR:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成失败:HX核心文件读取失败");
		}
		break;
	case  RET_FAT32_ERR:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成失败:文件系统格式化错误");
		}
		break;
	case  RET_IMPORT_ERR:	
		{
			wsprintf(g_szProcessInfo,L"BOOT生成失败:导入文件过大");
		}
		break;
	}

	PostMessage(hWnd,WM_MAKE_PROCESS,0,0);
}


DWORD APIENTRY Cmd_MakeUsbBoot(LPCTSTR pCmdLine)
{
	TCHAR    szAppPath[512]  = {0};
	TCHAR    szBootFile[512] = {0};
	TCHAR    szImportPath[512] = {0};	
	HANDLE   hDiskDrive      = NULL;
	HANDLE   hDiskFile       = NULL;
	DWORD    nStartSector    = 0;
	DWORD    dwFlage         = 0;
	DWORD    dwRet           = S_OK;

	
	if(Query_UsbDevice(NULL) == FALSE)
	{
		return MessageBox(NULL,L"没有发现USB设备，请插入USB设备重试!",L"提示",MB_OK|MB_ICONERROR);
	}

	GetModelPath(NULL,szAppPath,sizeof(szAppPath));	
	wsprintf(szImportPath,L"%simport\\",szAppPath);
	nStartSector = HX_KERNEL_START;
	
	while(dwRet == S_OK)
	{
		BYTE     szBuf[HX_SECTOR_SIZE]  = {0};	
		DWORD    dwRead                 = 0;
		DWORD    dwWrite                = 0;

		//打开U盘物理设备
		hDiskDrive = CreateFile(g_szUSBDeviceName,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
		if(hDiskDrive == INVALID_HANDLE_VALUE)
		{
			dwRet = RET_USB_ERR;
			break;
		}

		//先写BOOT扇区
		if(WriteBootSec(hDiskDrive,szAppPath,HX_HDD_RESERVED,g_szDiskInfo.nTotalSector) == FALSE)
		{
			dwRet = RET_BOOTSEC_ERR;
			break;
		}
		

		//再写HX核心				
		if(WriteHxKernel(hDiskDrive,szAppPath) == FALSE)
		{
			dwRet = RET_KERNEL_ERR;
			break;
		}
		
		//格式化文件系统
		if(StrStr(pCmdLine,L"-format") && Fat32Format(hDiskDrive,g_szDiskInfo.nTotalSector-HX_HDD_RESERVED-HX_FAT32_BLANK,HX_HDD_RESERVED) == FALSE)
		{
			dwRet = RET_FAT32_ERR; 
			break;
		}
		
		//导入文件
		CloseHandle(hDiskDrive);	
		hDiskDrive = NULL;		
		ImportFilesToVhd(g_szUSBDeviceName,szImportPath,HX_HDD_RESERVED);

		dwRet = RET_COMPLETE;
	}
	
	if(hDiskDrive)
	{
		CloseHandle(hDiskDrive);
	}

	CloseHandle(hDiskFile);
	

	SetCompleteInfo(NULL,dwRet);	
	dwFlage = (dwRet == RET_COMPLETE)?MB_OK:MB_OK|MB_ICONERROR;
	MessageBox(NULL,g_szProcessInfo,L"提示",dwFlage);

	return S_OK;
}

UINT APIENTRY MakeVhdCheckSum(LPBYTE pVhdFootBuf)
{
	UINT    nSum        = 0;
	UINT    nRetSum     = 0;
	LPBYTE  pTemBuf1     = (LPBYTE)&nSum;
	LPBYTE  pTemBuf2     = (LPBYTE)&nRetSum;

	for(INT i=0;i<512;i++)
	{
		nSum += pVhdFootBuf[i];
	}
	nSum = ~nSum;

	pTemBuf2[0] = pTemBuf1[3];
	pTemBuf2[1] = pTemBuf1[2];
	pTemBuf2[2] = pTemBuf1[1];
	pTemBuf2[3] = pTemBuf1[0];

	return nRetSum;
}
//计算虚拟硬盘字节尺寸
UINT64 APIENTRY ComputeVDiskSize(UINT nSectorCount)
{
	LPBYTE   pIntBuf      = NULL;
	BYTE     bTemp        = 0;
	UINT64   nTotalSize   = 0;
	UINT     nSizeLow     = 0;
	UINT     nSizeHi      = 0;

	nTotalSize   = nSectorCount*HX_SECTOR_SIZE;

	pIntBuf = (LPBYTE)&nTotalSize;
	CopyMemory(&nSizeLow,pIntBuf,4);
	CopyMemory(&nSizeHi, pIntBuf+4,4);
	
	pIntBuf    = (LPBYTE)&nSizeLow;
	bTemp      = pIntBuf[0];
	pIntBuf[0] = pIntBuf[3];
	pIntBuf[3] = bTemp;

	bTemp      = pIntBuf[1];
	pIntBuf[1] = pIntBuf[2];
	pIntBuf[2] = bTemp;

	pIntBuf    = (LPBYTE)&nSizeHi;
	bTemp      = pIntBuf[0];
	pIntBuf[0] = pIntBuf[3];
	pIntBuf[3] = bTemp;

	bTemp      = pIntBuf[1];
	pIntBuf[1] = pIntBuf[2];
	pIntBuf[2] = bTemp;

	pIntBuf = (LPBYTE)&nTotalSize;
	CopyMemory(pIntBuf,&nSizeHi,4);
	CopyMemory(pIntBuf+4,&nSizeLow,4);

	return nTotalSize;
}

//生成虚拟硬盘引导线程
DWORD APIENTRY Thread_MakeVDiskBoot(LPVOID p)
{
	TCHAR    szAppPath[512]     = {0};
	TCHAR    szVdiskPath[512]   = {0};
	TCHAR    szImportPath[512]  = {0};
	DWORD    dwVolumeCount      = 0;
	UINT     nTotalSectors      = HX_VDISK_SECTOR;
	UINT64   nImportSize        = 0;
	HWND	 hWnd               = (HWND)p;
	HANDLE   hDiskVolume        = NULL;	
	HANDLE   hVhdFile           = NULL;			
	HANDLE   hDiskFile          = NULL;		
	BOOL     dwRet              = S_OK;


	GetModelPath(NULL,szAppPath,sizeof(szAppPath));		
	wsprintf(szVdiskPath,L"%svdisk.vhd",szAppPath);
	wsprintf(szImportPath,L"%simport\\",szAppPath);

	nImportSize = GetDirSize(szImportPath);
	if(nImportSize  >= HX_VDISK_REDUN_SIZE)
	{
		UINT64   nNewVhdSize    = 0;
		UINT64   nDiskFreeSize  = 0;

		GetDiskFreeSpaceEx(szImportPath,(PULARGE_INTEGER)&nDiskFreeSize,NULL,NULL);
		if(nImportSize > nDiskFreeSize/2)
		{
			dwRet = RET_IMPORT_ERR;
			goto _END; 
		}

		nImportSize   += HX_SECTOR_SIZE;
		nNewVhdSize   = nImportSize +HX_VDISK_SECTOR*HX_SECTOR_SIZE;
		nTotalSectors = (UINT)(nNewVhdSize / HX_SECTOR_SIZE);
	}
	

	while(dwRet == S_OK)
	{
		BYTE     szBuf[HX_SECTOR_SIZE]   = {0};			
		DWORD    dwWrite                 = 0;
		

		SetProcessInfo(hWnd,PROCESS_INIT_VHD);

		//创建打开虚拟硬盘文件
		hVhdFile = CreateFile(szVdiskPath,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,0);
		if(hVhdFile == INVALID_HANDLE_VALUE)
		{
			dwRet = RET_USB_ERR;
			break;
		}
		SetEndOfFile(hVhdFile);
		//填充空白区域
		for(INT i=0;i<nTotalSectors;i++)
		{
			WriteFile(hVhdFile,szBuf,sizeof(szBuf),&dwWrite,NULL);
		}
		SetFilePointer(hVhdFile,0,NULL,FILE_BEGIN);

		//先写BOOT扇区
		if(WriteBootSec(hVhdFile,szAppPath,HX_VDISK_RESERVED,nTotalSectors) == FALSE)
		{
			dwRet = RET_BOOTSEC_ERR;
			break;
		}
		
		//再写HX核心	
		SetProcessInfo(hWnd,PROCESS_MAKE_KERNEL);	
		if(WriteHxKernel(hVhdFile,szAppPath) == FALSE)
		{
			dwRet = RET_KERNEL_ERR;
			break;
		}
		
		//最后对剩余空间进行FAT32格式化
		SetProcessInfo(hWnd,PROCESS_MAKE_FAT32);	
		if(Fat32Format(hVhdFile,nTotalSectors-HX_VDISK_RESERVED-HX_FAT32_BLANK,HX_VDISK_RESERVED) == FALSE)
		{
			dwRet = RET_FAT32_ERR; 
			break;
		}

		//写VHD信息标志
		g_szVhdInfo.orgin_size    = ComputeVDiskSize(nTotalSectors);
		g_szVhdInfo.current_size  = g_szVhdInfo.orgin_size;
		g_szVhdInfo.checksum      = 0;
		g_szVhdInfo.checksum      = MakeVhdCheckSum((LPBYTE)&g_szVhdInfo);

		SetFilePointer(hVhdFile,nTotalSectors*HX_SECTOR_SIZE,NULL,FILE_BEGIN);		
		WriteFile(hVhdFile,&g_szVhdInfo,sizeof(g_szVhdInfo),&dwWrite,NULL);

		CloseHandle(hVhdFile);
		hVhdFile = NULL;

		//导入文件
		SetProcessInfo(hWnd,PROCESS_IMPORT_VHD);	
		ImportFilesToVhd(szVdiskPath,szImportPath,HX_VDISK_RESERVED);

		dwRet = RET_COMPLETE;		
	}

	if(hVhdFile)
	{
		CloseHandle(hVhdFile);	
	}

_END:
	SetCompleteInfo(hWnd,dwRet);	

	EnableWindow(GetDlgItem(hWnd,IDC_MAKE_BOOT),TRUE);

	if(hWnd == NULL)	
	{		
		MessageBox(NULL,g_szProcessInfo,L"提示",(dwRet == RET_COMPLETE)?MB_OK:MB_OK|MB_ICONERROR);
	}

	return S_OK;
}

//生成usb  boot线程
DWORD APIENTRY Thread_MakeUsbBoot(LPVOID p)
{
	TCHAR    szAppPath[512]     = {0};
	TCHAR    szImportPath[512]  = {0};
	DWORD    dwVolumeCount      = 0;
	UINT64   nImportSize        = 0;
	HWND	 hWnd               = (HWND)p;
	HANDLE   hDiskVolume     = NULL;	
	HANDLE   hDiskDrive      = NULL;			
	HANDLE   hDiskFile       = NULL;		
	BOOL     dwRet           = S_OK;
		

	GetModelPath(NULL,szAppPath,sizeof(szAppPath));	
	wsprintf(szImportPath,L"%simport\\",szAppPath);

	nImportSize = GetDirSize(szImportPath);
	if(nImportSize > g_szDiskInfo.nTotalSize - HX_HDD_RESERVED*HX_SECTOR_SIZE)
	{
		dwRet = RET_IMPORT_ERR;
		goto _END;
	}

	while(dwRet == S_OK)
	{
		CHAR     szFatFlage[HX_SECTOR_SIZE]  = {0}; 
		BYTE     szBuf[HX_SECTOR_SIZE]       = {0};	
		DWORD    dwRead                      = 0;
		DWORD    dwWrite                     = 0;


		//锁住U盘文件系统
		hDiskVolume = CreateFile(g_szUsbDirName,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
		if(hDiskVolume == INVALID_HANDLE_VALUE)
		{
			dwRet = RET_USB_ERR;
			break;
		}
		DeviceIoControl(hDiskVolume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwVolumeCount, NULL);

		//打开U盘物理设备
		hDiskDrive = CreateFile(g_szUSBDeviceName,GENERIC_WRITE|GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,OPEN_EXISTING,0,0);
		if(hDiskDrive == INVALID_HANDLE_VALUE)
		{
			dwRet = RET_USB_ERR;
			break;
		}
				
		//写BOOT扇区
		if(WriteBootSec(hDiskDrive,szAppPath,HX_HDD_RESERVED,g_szDiskInfo.nTotalSector) == FALSE)
		{
			dwRet = RET_BOOTSEC_ERR;
			break;
		}
				
		//再写HX核心	
		SetProcessInfo(hWnd,PROCESS_MAKE_KERNEL);	
		if(WriteHxKernel(hDiskDrive,szAppPath) == FALSE)
		{
			dwRet = RET_KERNEL_ERR;
			break;
		}
		
				
		//再对剩余空间进行FAT32格式化
		SetProcessInfo(hWnd,PROCESS_MAKE_FAT32);	
		if(Fat32Format(hDiskDrive,g_szDiskInfo.nTotalSector-HX_HDD_RESERVED-HX_FAT32_BLANK,HX_HDD_RESERVED) == FALSE)
		{
			dwRet = RET_FAT32_ERR; 
			break;
		}
	
		//导入文件
		CloseHandle(hDiskDrive);	
		hDiskDrive = NULL;
		SetProcessInfo(hWnd,PROCESS_IMPORT_VHD);	
		ImportFilesToVhd(g_szUSBDeviceName,szImportPath,HX_HDD_RESERVED);
			
		dwRet = RET_COMPLETE;		
	}
		
	if(hDiskDrive)
	{
		CloseHandle(hDiskDrive);
	}	

_END:
	SetCompleteInfo(hWnd,dwRet);	

	//解锁
	DeviceIoControl(hDiskVolume, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwVolumeCount, NULL);
	CloseHandle(hDiskVolume);
	
	EnableWindow(GetDlgItem(hWnd,IDC_MAKE_BOOT),TRUE);
	
	return dwRet;

}

INT_PTR APIENTRY OnCmdProc(HWND hDlg,WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case IDCANCEL:
	case IDCLOSE:	
		{
		EndDialog(hDlg,0);
		}
		break;
	case IDC_HDD_BOOT:
		{
		CheckDlgButton(hDlg,IDC_FLOPPY_BOOT,BST_UNCHECKED);	
		CheckDlgButton(hDlg,IDC_HDD_BOOT,BST_CHECKED);	
		EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),g_bUsbFound);
		}
		break;
	case IDC_FLOPPY_BOOT:
		{
		CheckDlgButton(hDlg,IDC_HDD_BOOT,BST_UNCHECKED);	
		CheckDlgButton(hDlg,IDC_FLOPPY_BOOT,BST_CHECKED);	
		EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),TRUE);
		}
		break;
	case IDC_MAKE_BOOT:
		{
		BOOL bMake  = FALSE;

		if(IsDlgButtonChecked(hDlg,IDC_FLOPPY_BOOT) == BST_CHECKED)
			{
			EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),FALSE);
			g_szProcessInfo[0] = 0;
			CreateThread(NULL,NULL,Thread_MakeVDiskBoot,(LPVOID)hDlg,NULL,NULL);		
			break;
			}

		if(MessageBox(hDlg,L"生成BOOT将会清除U盘设备所有的文件数据，是否继续？",L"警告",MB_YESNO|MB_ICONWARNING) == IDYES)
			{
			EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),FALSE);
			g_szProcessInfo[0] = 0;
			CreateThread(NULL,NULL,Thread_MakeUsbBoot,(LPVOID)hDlg,NULL,NULL);		
			}
		}
		break;	
	}

	return S_OK;
}

// 消息处理程序。
INT_PTR CALLBACK Main_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
	case WM_INITDIALOG:
		{
		SetDlgItemText(hDlg,IDC_USB_INFO,USB_NO_FOUND);

		Query_UsbDevice(hDlg);

		CheckDlgButton(hDlg,IDC_FLOPPY_BOOT,BST_CHECKED);
		SetDlgItemInt(hDlg,IDC_DISK_ID,128,FALSE);		
				
		return TRUE;
		}
		break;
	case WM_DEVICECHANGE:
		{
			switch(wParam)
			{
				case DBT_DEVICEARRIVAL:
					{
					PDEV_BROADCAST_HDR lpdb     = (PDEV_BROADCAST_HDR)lParam;  
					PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
					
					Detect_UsbDevice(hDlg,GetDriveFromMask(lpdbv->dbcv_unitmask));
					
					if(IsDlgButtonChecked(hDlg,IDC_HDD_BOOT) == BST_CHECKED)
						{
						EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),g_bUsbFound);
						}
					}
					break;
				case DBT_DEVNODES_CHANGED:
					{
					GetDiriveInfo(g_szUSBDeviceName);
					if(g_szDiskInfo.nTotalSize  == 0)
						{
							SetDlgItemText(hDlg,IDC_USB_INFO,USB_NO_FOUND);				
							g_bUsbFound = FALSE;
						}

					if(IsDlgButtonChecked(hDlg,IDC_HDD_BOOT) == BST_CHECKED)
						{
						EnableWindow(GetDlgItem(hDlg,IDC_MAKE_BOOT),g_bUsbFound);
						}
					}
					break;
			}
		}
		break;
	case WM_MAKE_PROCESS:
		{
		SetDlgItemText(hDlg,IDC_PROCESS,g_szProcessInfo);	
		}
		break;
	case WM_COMMAND:
		{
		OnCmdProc(hDlg,wParam,lParam);
		}		
		break;
	}

	return (INT_PTR)FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	
	if(StrStrI(lpCmdLine,L"-make_vhd"))
	{
		return Thread_MakeVDiskBoot(NULL);
	}
	else if(StrStrI(lpCmdLine,L"-make_usb"))
	{
		return Cmd_MakeUsbBoot(lpCmdLine);
	}
	else 
	{
		return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, Main_DlgProc);
	}
	
}



