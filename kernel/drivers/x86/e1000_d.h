//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 21,2017
//    Module Name               : e1000_d.h
//    Module Funciton           : 
//                                Entry point and other framework related routines
//                                complying to HelloX,of E1000 ethernet chip driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __E1000_H__
#define __E1000_H__

/* Intel i210 needs the DMA descriptor rings aligned to 128b */
#define E1000_BUFFER_ALIGN	128

/* Tx/Rx descriptor number. */
#define TX_DESC_NUM 16
#define RX_DESC_NUM 16

/* Packet buffer size. */
#define PACKET_BUFFER_SIZE 4096

/* PCI device ID. */
struct pci_device_id{
	unsigned short vendor_id;
	unsigned short device_id;
};
#define PCI_DEVICE(vendor,device) {vendor,device}

/* Manual add it since it is not updated in pci_ids.h file. */
#define PCI_DEVICE_ID_INTEL_I218 0x15a2

/* Device ID array that the driver can support. */
extern struct pci_device_id e1000_supported[];

/*
 * Simulation routines of PCI operation under uboot.
 */
void pci_read_config_byte(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned char* ret);
void pci_read_config_word(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned short* ret);
void pci_read_config_dword(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned long* ret);

void pci_write_config_byte(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned char val);
void pci_write_config_word(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned short val);
void pci_write_config_dword(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned long val);

void* pci_map_bar(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, DWORD dwFlags);

/* Entry point of E1000 driver. */
BOOL E1000_Drv_Initialize(LPVOID pData);

/* Process incoming packet. */
void net_process_received_packet(unsigned char* packet, int length);

#endif //__E1000_H__
