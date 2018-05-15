//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 26 JAN,2009
//    Module Name               : IDEBASE.CPP
//    Module Funciton           : 
//    Description               : Low level routines,such as read sector and write sector,
//                                are implemented in this file.
//    Last modified Author      :
//    Last modified Date        : 04 Jun,2011
//    Last modified Content     :
//                                1. It will fail in some occasion when try to read harddisk directly,maybe caused
//                                   by modern S-ATA architecture,so use BIOSReadSector and BIOSWriteSector to replace
//                                   the original ReadSector and WriteSector code;
//                                2.
//    Lines number              :
//    Extra comment             : Today is 26 JAN,2009.In china's traditional canlendar,
//                                today is spring festival,i.e,new year.
//                                I like spring festival,in fact,most chinese like spring 
//                                festival,today can lead me recalling the time I was a child.
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>

#include "idebase.h"
#include "bios.h"

#if 0
BOOL ReadHDSector(LPVOID lpBuffer,
				  UCHAR byStartSector,
				  UCHAR byCylinderLo,
				  UCHAR byCylinderHi,
				  UCHAR byCtrlDrvHdr)     //bit 0 - 3 : Header.
				                          //bit 4     : Driver.
										  //bit 5 - 7 : Controller.
{
	BOOL          bResult      = FALSE;
	DWORD         dwCounter    = 0x00000000;
	UCHAR         byFlags      = 0x00;
	UCHAR         byDrvHdr     = 0x00;

	if(NULL == lpBuffer)
		return bResult;
	if(byCtrlDrvHdr >= 32)               //Currently,the system can not support
		                                 //more than one controller.
        return bResult;

	//---------- ** debug ** -------------
	WriteByteToPort(0x00,IDE_CTRL0_PORT_CTRL);  //Build the controller method.

	while(dwCounter < 0xFFFFFFFF)       //Wait for controller and driver ready.
	{
		ReadByteFromPort(&byFlags,IDE_CTRL0_PORT_STATUS);
		if(!CONTROLLER_READY(byFlags) || !DRIVER_READY(byFlags))
		{
			dwCounter ++;
			continue;
		}
		break;
	}
	if(0xFFFFFFFF == dwCounter)        //Time out.
	{
		PrintLine("The controller or driver is not ready in waiting time.");
		return bResult;
	}

	switch(16 & byCtrlDrvHdr)         //Determine which driver is to be accessed.
	{
	case 0:
		byDrvHdr = FORM_DRIVER_HEADER(IDE_DRV0_LBA,byCtrlDrvHdr);
		break;
	case 16:
		byDrvHdr = FORM_DRIVER_HEADER(IDE_DRV1_LBA,byCtrlDrvHdr);
		break;
	default:
		return bResult;
		break;
	}

	WriteByteToPort(0x4b,IDE_CTRL0_PORT_PRECOMP);
	WriteByteToPort(0x01,IDE_CTRL0_PORT_SECTORNUM);
	WriteByteToPort(byStartSector,IDE_CTRL0_PORT_STARTSECTOR);
	WriteByteToPort(byCylinderLo,IDE_CTRL0_PORT_CYLINDLO);
	WriteByteToPort(byCylinderHi,IDE_CTRL0_PORT_CYLINDHI);
	WriteByteToPort(byDrvHdr,IDE_CTRL0_PORT_HEADER);
	WriteByteToPort(IDE_CMD_READ,IDE_CTRL0_PORT_CMD);

	while(TRUE)
	{
		ReadByteFromPort(&byFlags,IDE_CTRL0_PORT_STATUS);
		if(CONTROLLER_READY(byFlags))
			break;
	}

    ReadWordStringFromPort(lpBuffer,512,IDE_CTRL0_PORT_DATA);

	bResult = TRUE;
	return bResult;
}
#endif

//Several low level routines to test specified flags in hard disk driver registers.
static BOOL WaitForRdy(WORD wPort,DWORD dwMillionSecond)
{
	BYTE Status;
	BOOL bResult = FALSE;
	int cnt = 65536;

	while(TRUE)
	{
		Status = __inb(wPort);
		if(DRIVER_READY(Status))
		{
			bResult = TRUE;
			break;
		}
		cnt --;
		if(0 == cnt)
		{
			break;
		}
	}
	return TRUE;
}

static BOOL WaitForBsy(WORD wPort,DWORD dwMillionSecond)
{
	BYTE Status;
	int cnt = 65536;
	BOOL bResult = FALSE;
	
	while(TRUE)
	{
		Status = __inb(wPort);  //Read status register.
		if(CONTROLLER_READY(Status))
		{
			bResult = TRUE;
			break;
		}
		cnt --;
		if(0 == cnt)
		{
			break;
		}
	}
	return bResult;
}

static BOOL WaitForDrq(WORD wPort,DWORD dwMillionSecond)
{
	BYTE Status = 0;
	BOOL bResult = FALSE;
	int cnt = 65536;

	while(TRUE)
	{
		Status = __inb(wPort);
		if(Status & 8)
		{
			bResult = TRUE;
			break;
		}
		cnt --;
		if(0 == cnt)
		{
			break;
		}
	}
	return bResult;
}

static BOOL CmdSucc(WORD wPort)
{
	BYTE status = __inb(wPort);
	if(COMMAND_SUCC(status))
	{
		return TRUE;
	}
	return FALSE;
}

/* 
 * Read one or several sector(s) from a specified hard disk.
 * Please make sure the pBuffer is long enough to contain the 
 * sectors to read.
 */
BOOL ReadSector(int nHdNum,DWORD dwStartSector,DWORD dwSectorNum,BYTE* pBuffer)
{
	//BYTE        DrvHdr  = (BYTE)IDE_DRV0_LBA;
	//BYTE        LbaLow;
	//BYTE        LbaMid;
	//BYTE        LbaHigh;
	//UCHAR       tmp;
	//CHAR        Buffer[64];  //For debug.
	//BOOL        bResult = FALSE;

#ifdef __I386__
	/* Just use BIOS service to complete the sector reading. */
	return BIOSReadSector(nHdNum,dwStartSector,dwSectorNum,pBuffer);
#else
	return FALSE;
#endif

#if 0
	if((nHdNum > 1) || (NULL == pBuffer) || (0 == dwSectorNum))
	{
		return FALSE;
	}
	//Form the access mode and driver.
	if(1 == nHdNum)
	{
		DrvHdr = (BYTE)IDE_DRV1_LBA;
	}
	tmp = (UCHAR)(dwStartSector >> 24);  //Get the high byte.
	tmp &= 0x0F;                         //Only keep the last 4 bits.
	DrvHdr += tmp;                       //Driver and access mode(LBA).
	tmp = (UCHAR)dwStartSector;
	LbaLow = tmp;
	tmp = (UCHAR)(dwStartSector >> 8);
	LbaMid = tmp;
	tmp = (UCHAR)(dwStartSector >> 16);
	LbaHigh = tmp;
	tmp = (UCHAR)dwSectorNum;
	//Issue the read sector command.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	//WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	//WriteByteToPort(0x4b,IDE_CTRL0_PORT_PRECOMP);
	__outb(0x4B,IDE_CTRL0_PORT_PRECOMP);
	//WriteByteToPort(0x01,IDE_CTRL0_PORT_SECTORNUM);
	__outb(tmp,IDE_CTRL0_PORT_SECTORNUM);
	//WriteByteToPort(byStartSector,IDE_CTRL0_PORT_STARTSECTOR);
	__outb(LbaLow,IDE_CTRL0_PORT_STARTSECTOR);
	//WriteByteToPort(byCylinderLo,IDE_CTRL0_PORT_CYLINDLO);
	__outb(LbaMid,IDE_CTRL0_PORT_CYLINDLO);
	//WriteByteToPort(byCylinderHi,IDE_CTRL0_PORT_CYLINDHI);
	__outb(LbaHigh,IDE_CTRL0_PORT_CYLINDHI);
	//WriteByteToPort(byDrvHdr,IDE_CTRL0_PORT_HEADER);
	__outb(DrvHdr,IDE_CTRL0_PORT_HEADER);
	//WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	//WriteByteToPort(IDE_CMD_READ,IDE_CTRL0_PORT_CMD);
	__outb(IDE_CMD_READ,IDE_CTRL0_PORT_CMD);
	//Wait for completion.
	//WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForDrq(IDE_CTRL0_PORT_STATUS,0);
	//WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	//Read data from port.
	__inws(pBuffer,tmp * 512,IDE_CTRL0_PORT_DATA);
	//WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	//WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	//Appended by Garry in 2011.05.30
	if(CmdSucc(IDE_CTRL0_PORT_STATUS))
	{
		return TRUE;
	}
	tmp = __inb(0x1f2);
	sprintf(Buffer,"The error number is : %d",tmp);
	PrintLine(Buffer);
	return FALSE;
#endif
}

/* 
 * Write one or several sector(s) to a specified hard disk.
 * Please make sure the pBuffer is long enough to contain the 
 * sectors to write.
 */
BOOL WriteSector(int nHdNum,DWORD dwStartSector,DWORD dwSectorNum,BYTE* pBuffer)
{
#ifdef __I386__
	/* Just to call BIOS service to write. */
	return BIOSWriteSector(nHdNum,dwStartSector,dwSectorNum,pBuffer);
#else
	return FALSE;
#endif

#if 0
	BYTE        DrvHdr  = (BYTE)IDE_DRV0_LBA;
	BYTE        LbaLow;
	BYTE        LbaMid;
	BYTE        LbaHigh;
	UCHAR       tmp;
	
	if((nHdNum > 1) || (NULL == pBuffer) || (0 == dwSectorNum))
	{
		return FALSE;
	}
	//Form the access mode and driver.
	if(1 == nHdNum)
	{
		DrvHdr = (BYTE)IDE_DRV1_LBA;
	}
	tmp = (UCHAR)(dwStartSector >> 24);  //Get the high byte.
	tmp &= 0x0F;                         //Only keep the last 4 bits.
	DrvHdr += tmp;                       //Driver and access mode(LBA).
	tmp = (UCHAR)dwStartSector;
	LbaLow = tmp;
	tmp = (UCHAR)(dwStartSector >> 8);
	LbaMid = tmp;
	tmp = (UCHAR)(dwStartSector >> 16);
	LbaHigh = tmp;
	tmp = (UCHAR)dwSectorNum;
	//Issue the read sector command.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	//WriteByteToPort(0x4b,IDE_CTRL0_PORT_PRECOMP);
	__outb(0x4B,IDE_CTRL0_PORT_PRECOMP);
	//WriteByteToPort(0x01,IDE_CTRL0_PORT_SECTORNUM);
	__outb(tmp,IDE_CTRL0_PORT_SECTORNUM);
	//WriteByteToPort(byStartSector,IDE_CTRL0_PORT_STARTSECTOR);
	__outb(LbaLow,IDE_CTRL0_PORT_STARTSECTOR);
	//WriteByteToPort(byCylinderLo,IDE_CTRL0_PORT_CYLINDLO);
	__outb(LbaMid,IDE_CTRL0_PORT_CYLINDLO);
	//WriteByteToPort(byCylinderHi,IDE_CTRL0_PORT_CYLINDHI);
	__outb(LbaHigh,IDE_CTRL0_PORT_CYLINDHI);
	//WriteByteToPort(byDrvHdr,IDE_CTRL0_PORT_HEADER);
	__outb(DrvHdr,IDE_CTRL0_PORT_HEADER);
	//WriteByteToPort(IDE_CMD_READ,IDE_CTRL0_PORT_CMD);
	__outb(IDE_CMD_WRITE,IDE_CTRL0_PORT_CMD);
	//Write data from port.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);    //Should wait the driver to ready,an complicated problem has been caused
	                                        //by this issue(did not wait before).
	__outws(pBuffer,tmp * 512,IDE_CTRL0_PORT_DATA);
	//Wait for completion.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	return TRUE;
#endif
}

/* 
 * Issue the IDENTIFY command and return the result.
 * pBuffer will contain the content returned by IDENTIFY command.
 */
BOOL Identify(int nHdNum,BYTE* pBuffer)
{
	BYTE DeviceReg = 0x0;  //Device select register.
	BYTE Status    = 0;  //Status register.

	if(nHdNum >= 2)  //Not supported yet.
	{
		return FALSE;
	}
	if(nHdNum == 1)
	{
		DeviceReg += 16;  //Select the second hard disk.
	}
	//Issue the IDENTIFY DEVICE command.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);  //Wait for controller to free.
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	__outb(DeviceReg,IDE_CTRL0_PORT_HEADER);
	//WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	__outb(IDE_CMD_IDENTIFY,IDE_CTRL0_PORT_CMD);
	//Wait for command completion.
	//WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	//WaitForRdy(IDE_CTRL0_PORT_STATUS,0);  //-- DEBUG --
	WaitForDrq(IDE_CTRL0_PORT_STATUS,0);
	__inws(pBuffer,512,IDE_CTRL0_PORT_DATA);
	if(CmdSucc(IDE_CTRL0_PORT_STATUS))
	{
		return TRUE;
	}
	return FALSE;
}

/* 
 * Initialization routine for IDE controller 0.
 * In current implementation,this routine only disabled the interrupt of 
 * IDE controller,so only polling scheme is used now.
 */
BOOL IdeInitialize()
{
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	__outb(0x0E,IDE_CTRL0_PORT_CTRL);    //Reset controller,disable interrupt.
	WaitForBsy(IDE_CTRL0_PORT_STATUS,0);
	WaitForRdy(IDE_CTRL0_PORT_STATUS,0);
	return TRUE;
}
