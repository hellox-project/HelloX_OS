//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 26 JAN,2009
//    Module Name               : IDEBASE.H
//    Module Funciton           : 
//    Description               : Low level routines,such as read sector and write sector,
//                                are defined in this file.
//    Last modified Author      :
//    Last modified Date        : 26 JAN,2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : Today is 26 JAN,2009.In china's traditional canlendar,
//                                today is spring festival,i.e,new year.
//                                I like spring festival,in fact,most chinese like spring 
//                                festival,today can lead me recalling the time I was a child.
//***********************************************************************/

#ifndef __IDEBASE_H__
#define __IDEBASE_H__
#endif

//
//IDE HD controller I/O port.
//There may be several IDE controllers(at most 4),the following
//is the first IDE controller's I/O port,compitable with
//IBM AT/PC.
//
#define IDE_CTRL0_PORT_DATA              0x1F0
#define IDE_CTRL0_PORT_0                 0x1F0
#define IDE_CTRL0_PORT_1                 0x1F1
#define IDE_CTRL0_PORT_2                 0x1F2
#define IDE_CTRL0_PORT_3                 0x1F3
#define IDE_CTRL0_PORT_4                 0x1F4
#define IDE_CTRL0_PORT_5                 0x1F5
#define IDE_CTRL0_PORT_6                 0x1F6
#define IDE_CTRL0_PORT_7                 0x1F7
#define IDE_CTRL0_PORT_CMD               0x1F7
#define IDE_CTRL0_PORT_STATUS            0x1F7

#define IDE_CTRL0_PORT_CTRL              0x3F6

//IDE HD controller command.
#define IDE_CMD_READ                     0x20
#define IDE_CMD_WRITE                    0x30
#define IDE_CMD_CHECK                    0x40
#define IDE_CMD_FORMAT                   0x50
#define IDE_CMD_INIT                     0x60
#define IDE_CMD_SEEK                     0x70
#define IDE_CMD_DIAG                     0x80
#define IDE_CMD_BUILD                    0x90
#define IDE_CMD_IDENTIFY                 0xEC

//Access mode and driver select part.
#define IDE_DRV0_LBA                     0xe0    //LBA mode,driver 0.
#define IDE_DRV0_CHS                     0xa0    //CHS mode,driver 0.
#define IDE_DRV1_LBA                     0xf0    //LBA mode,driver 1.
#define IDE_DRV1_CHS                     0xb0    //CHS mode,driver 1.

//The alias name of the controller 0's register.
#define IDE_CTRL0_PORT_PRECOMP                IDE_CTRL0_PORT_1
#define IDE_CTRL0_PORT_SECTORNUM              IDE_CTRL0_PORT_2
#define IDE_CTRL0_PORT_STARTSECTOR            IDE_CTRL0_PORT_3
#define IDE_CTRL0_PORT_CYLINDLO               IDE_CTRL0_PORT_4
#define IDE_CTRL0_PORT_CYLINDHI               IDE_CTRL0_PORT_5
#define IDE_CTRL0_PORT_HEADER                 IDE_CTRL0_PORT_6

//The following macro forms the port 6's value
//according to the driver(0 or 1),mode(LBA or CHS) and header.
#define FORM_DRIVER_HEADER(mode,header)  ((0xF0 & mode) + (0x0F & header))

//The following macro gets the sector part from double
//word,this is the start sector to operate.
#define GET_SECTOR_PART(dword)  (LOBYTE(LOWORD(dword)))

//Get the low cylinder part from double word.
#define GET_LO_CYLINDER(dword)  (HIBYTE(LOWORD(dword)))

//Get the high cylinder part from double word.
#define GET_HI_CYLINDER(dword)  (LOBYTE(HIWORD(dword)))

//Get the header number from double word.
#define GET_HEADER_PART(dword)  (15 & (HIBYTE(HIWORD(dword))))

//Test controller ready flag.
//If the 8th bit of status register is 0,the controller is ready,
//otherwise,the controller is busying.
#define CONTROLLER_READY(flags)     ((~flags) & 0x80)

//Test driver ready flag.
//The 7th bit of status register is 1,then the driver is ready,otherwise,
//the driver is busying.
#define DRIVER_READY(flags)         (flags & 0x40)

//Test the command executing result.
//The first bit of status is 1,then the result successfully,otherwise,
//failed.
#define COMMAND_SUCC(flags)         ((~flags) & 0x01)

//Partition table in MBR.
typedef struct PARTITION_TABLE_ENTRY{
	UCHAR      IfActive;            //80 is active,00 is inactive.
	UCHAR      StartCST1;           //Start cylinder,sector and track.
	UCHAR      StartCST2;
	UCHAR      StartCST3;
	UCHAR      PartitionType;       //Partition type,such as FAT32,NTFS.
	UCHAR      EndCST1;             //End cylinder,sector and track.
	UCHAR      EndCST2;
	UCHAR      EndCST3;
	DWORD      dwStartSector;       //Start logical sector.
	DWORD      dwTotalSector;       //Sector number occupied by this partition.
}__PARTITION_TABLE_ENTRY;

//Low level routines to operate a hard disk.
BOOL ReadSector(int nHdNum,DWORD dwStartSector,DWORD dwSectorNum,BYTE* pBuffer);
BOOL WriteSector(int nHdNum,DWORD dwStartSector,DWORD dwSectorNum,BYTE* pBuff);
BOOL Identify(int nHdNum,BYTE* pBuffer);  //Issue IDENTIFY command and returns the result.
BOOL IdeInitialize(void);

