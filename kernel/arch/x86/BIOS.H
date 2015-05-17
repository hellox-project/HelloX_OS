//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun,01 2011
//    Module Name               : BIOS.H
//    Module Funciton           : 
//                                BIOS service entries definition are put here.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __BIOS_H__
#define __BIOS_H__

#define BIOS_ENTRY_ADDR 0x1800  //Begin address in realinit.bin module,the begin point of switch back to 
                                //real mode.

#define BIOS_SERVICE_REBOOT      0x0000  //Reboot the system.
#define BIOS_SERVICE_POWEROFF    0x0001  //Power off the system.
#define BIOS_SERVICE_READSECTOR  0x0002  //Read harddisk sector.
#define BIOS_SERVICE_WRITESECTOR 0x0003  //Write harddisk sector.
#define BIOS_SERVICE_TOGRAPHIC   0x0004  //Switch to graphic mode.
#define BIOS_SERVICE_TOTEXT      0x0005  //Switch back text mode from.

#define BIOS_HD_BUFFER           0xF000  //Hard disk reading buffer in BIOS.

//Service pro-type definition.
//Reboot the system.
VOID BIOSReboot(void);

//Shutdown(poweroff) the system.
VOID BIOSPoweroff(void);

//Read harddisk sectors from BIOS.
BOOL BIOSReadSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer);

//Write harddisk sectors from BIOS.
BOOL BIOSWriteSector(int nHdNum,DWORD nStartSector,DWORD nSectorNum,BYTE* pBuffer);
//Switch current display mode to graphic.
BOOL SwitchToGraphic(void);

//Switch current display mode to text.
VOID SwitchToText(void);

#endif  //__BIOS_H__
