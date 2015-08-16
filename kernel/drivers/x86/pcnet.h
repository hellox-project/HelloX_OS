//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Aug 02, 2015
//    Module Name               : pcnet.h
//    Module Funciton           : 
//                                This module countains the pre-definitions or
//                                structures for PCNet NIC.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PCNET_H__
#define __PCNET_H__ 

//Macro to control the debugging functions of PCNet.
//#define __PCNET_DEBUG

//Some macros used to endian converting.
#define cpu_to_le16(v)  (v)
#define cpu_to_le32(v)  (v)

#ifdef __CFG_SYS_VMM
#define PCI_TO_MEM_LE(dev,m) \
	    lpVirtualMemoryMgr->GetPhysicalAddress((__COMMON_OBJECT*)lpVirtualMemoryMgr,m)
#else
#define PCI_TO_MEM_LE(dev,m) (m)
#endif

//Ethernet interface name of PCNet.
#define PCNET_INT_NAME "PCNet Ethernet Interface[%d]"

//Device name of the PCNet NIC,please be aware that the
//device name is different to ethernet interface name.
#define PCNET_DEV_NAME "\\\\.\\PCNet_NIC"

//Vendor ID and device ID of PCNet NIC.
#define PCNET_VENDOR_ID   0x1022
#define PCNET_DEVICE_ID   0x2000

//PCI command bits of 79C97x,from datasheet.
#define PCNET_PCI_COMMAND_IOEN   0x01  //IO operation enable.
#define PCNET_PCI_COMMAND_MEMEN  0x02  //Memory map operation enable.
#define PCNET_PCI_COMMAND_BMEN   0x04  //BUS master enable.
#define PCNET_PCI_COMMAND_SCYCEN 0x08  //Special cycle enable.

//Offset from base IO address for WIO modes.All these values are from
//datasheet of PCNet 79C973 or PCNet 79C975 NIC controller.
#define PCNET_RDP     0x10
#define PCNET_RAP     0x12
#define PCNET_RESET   0x14
#define PCNET_BDP     0x16

//Set the number of Tx and Rx buffers, using Log_2(# buffers).
//Reasonable default values are 4 Tx buffers, and 16 Rx buffers.
//That translates to 2 (4 == 2^^2) and 4 (16 == 2^^4).
#define PCNET_LOG_TX_BUFFERS    0
#define PCNET_LOG_RX_BUFFERS    2

#define TX_RING_SIZE (1 << (PCNET_LOG_TX_BUFFERS))
#define TX_RING_LEN_BITS ((PCNET_LOG_TX_BUFFERS) << 12)

#define RX_RING_SIZE (1 << (PCNET_LOG_RX_BUFFERS))
#define RX_RING_LEN_BITS ((PCNET_LOG_RX_BUFFERS) << 4)

//Default packet buffer's size,1500 plus ethernet level headers.
#define PKT_BUF_SZ              1544

/* The PCNET Rx and Tx ring descriptors. */
struct pcnet_rx_head {
	__U32 base;
	__S16 buf_length;
	__S16 status;
	__U32 msg_length;
	__U32 reserved;
};

struct pcnet_tx_head {
	__U32 base;
	__S16 length;
	__S16 status;
	__U32 misc;
	__U32 reserved;
};

/* The PCNET 32-Bit initialization block, described in databook. */
struct pcnet_init_block {
	__U16 mode;
	__U16 tlen_rlen;
	__U8  phys_addr[6];
	__U16 reserved;
	__U32 filter[2];
	/* Receive and transmit ring base, along with extra bits. */
	__U32 rx_ring;
	__U32 tx_ring;
	__U32 reserved2;
};

struct pcnet_uncached_priv {
	struct pcnet_rx_head rx_ring[RX_RING_SIZE];
	struct pcnet_tx_head tx_ring[TX_RING_SIZE];
	struct pcnet_init_block init_block;
};

typedef struct pcnet_priv {
	/* If the structure is available,only is a internal flag. */
	int    available;
	struct pcnet_uncached_priv *uc;
	struct pcnet_uncached_priv *uc_unalign;
	struct pcnet_priv* next;
	/* Receive Buffer space */
	unsigned char(*rx_buf)[RX_RING_SIZE][PKT_BUF_SZ + 4];
	unsigned char(*rx_buf_unalign)[RX_RING_SIZE][PKT_BUF_SZ + 4];
	int cur_rx;
	int cur_tx;
	/* Hardware resources of the NIC. */
	__U16 ioBase;
	int   intVector;
	__U8  macAddr[6];
	int   chip_ver;
	char* chip_name;
	/* The corresponding physical device. */
	__PHYSICAL_DEVICE* pPhyDev;
	/* The ethernet interface object of this interface. */
	__ETHERNET_INTERFACE* pEthInt;
	/* Interrupt object of this NIC. */
	LPVOID hInterrupt;
} pcnet_priv_t;

//Entry point of PCNet NIC device.
BOOL PCNet_Drv_Initialize(LPVOID pData);

#endif //__PCNET_H__
