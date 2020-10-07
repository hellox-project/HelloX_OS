//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 1,2020
//    Module Name               : e1000ex.h
//    Module Funciton           : 
//                                Entry point and other framework related routines
//                                complying to HelloX,of E1000E ethernet chip driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __E1000EX_H__
#define __E1000EX_H__

/* System level definitions. */
#include "config.h"
#include "stdint.h"
#include "ethmgr.h"
#include "genif.h"

#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217     0x153A  // Device ID for Intel I217
#define E1000_I218_LM  0x155A  // Device ID for I218 LM
#define E1000_82577LM  0x10EA  // Device ID for Intel 82577LM
#define E1000_82579LM  0x1502  // Device ID for Intel 82579LM

/* Constant values of E1000E chip. */

#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818

#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818

#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_TXDCTL       0x3828 // TX Descriptor Control
#define REG_RXDCTL       0x2828 // RX Descriptor control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt
#define REG_TARC         0x3840 // TX arbitration control 0.
#define REG_GCR          0x5B00 // 3GIO control register.
#define REG_GCR2         0x5B64 // 3GIO Control register 2.

#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        //set link up

/* Enable descriptor(for tx/rx). */
#define DCTL_ENABLE         (1 << 25)

#define CTRL_FD				(1 << 0)
#define CTRL_ASDE			(1 << 5)
#define CTRL_SLU			(1 << 6)
#define CTRL_RST            (1 << 26)
#define CTRL_DEV_RST        (1 << 29)

#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC

// Rx buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))

// Transmit Command
#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable

//TCTL Register
#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision

#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun

/* IMS register. */
#define IMS_TXDW                        (1 << 0)
#define IMS_TXQE                        (1 << 1)
#define IMS_LSC                         (1 << 2)
#define IMS_RXSEQ                       (1 << 3)
#define IMS_RXDMT0                      (1 << 4)
#define IMS_RXO                         (1 << 6)
#define IMS_RXDW                        (1 << 7)

/* Alias of above definitions,make programming simplicity. */
#define i825xx_REG_CTRL			(priv->mmio_address + 0x0000)
#define i825xx_REG_STATUS		(priv->mmio_address + 0x0008)
#define i825xx_REG_EECD			(priv->mmio_address + 0x0010)
#define i825xx_REG_EERD			(priv->mmio_address + 0x0014)
#define i825xx_REG_MDIC			(priv->mmio_address + 0x0020)
#define i825xx_REG_ITR          (priv->mmio_address + 0x00C4)
#define i825xx_REG_IMS			(priv->mmio_address + 0x00D0)
#define i825xx_REG_IMC          (priv->mmio_address + 0x00D8)
#define i825xx_REG_RCTL			(priv->mmio_address + 0x0100)
#define i825xx_REG_TCTL			(priv->mmio_address + 0x0400)
#define i825xx_REG_TIPG         (priv->mmio_address + 0x0410)
#define i825xx_REG_RDBAL		(priv->mmio_address + 0x2800)
#define i825xx_REG_RDBAH		(priv->mmio_address + 0x2804)
#define i825xx_REG_RDLEN		(priv->mmio_address + 0x2808)
#define i825xx_REG_RDH			(priv->mmio_address + 0x2810)
#define i825xx_REG_RDT			(priv->mmio_address + 0x2818)
#define i825xx_REG_TDBAL		(priv->mmio_address + 0x3800)
#define i825xx_REG_TDBAH		(priv->mmio_address + 0x3804)
#define i825xx_REG_TDLEN		(priv->mmio_address + 0x3808)
#define i825xx_REG_TDH			(priv->mmio_address + 0x3810)
#define i825xx_REG_TDT			(priv->mmio_address + 0x3818)
#define i825xx_REG_MTA			(priv->mmio_address + 0x5200)
#define i825xx_REG_RAL          (priv->mmio_address + 0x5400)
#define i825xx_REG_RAH          (priv->mmio_address + 0x5404)

/* PHY REGISTERS (for use with the MDI/O Interface) */
#define i825xx_PHYREG_PCTRL		(0)
#define i825xx_PHYREG_PSTATUS	(1)
#define i825xx_PHYREG_PSSTAT	(17)

/* Specific memory I/O operations. */
#define mmio_read32(reg) __readl(reg)
#define mmio_write32(reg,val) __writel(val,reg)

/* Rx and Tx descriptors,and their number. */
#define NUM_RX_DESCRIPTORS 16
#define NUM_TX_DESCRIPTORS 16

/* Descriptor base address alignment,128 should be enough. */
#define DESCRIPTOR_BASE_ALIGN 128

/* Use different pack options according IDE. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct e1000_rx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint16_t checksum;
	volatile uint8_t status;
	volatile uint8_t errors;
	volatile uint16_t special;
};
#pragma pack(pop)
#else
struct e1000_rx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint16_t checksum;
	volatile uint8_t status;
	volatile uint8_t errors;
	volatile uint16_t special;
} __attribute__((packed));
#endif

#if defined(__MS_VC__)
#pragma pack(push,1)
struct e1000_tx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint8_t cso;
	volatile uint8_t cmd;
	volatile uint8_t status;
	volatile uint8_t css;
	volatile uint16_t special;
};
#pragma pack(pop)
#else
struct e1000_tx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint8_t cso;
	volatile uint8_t cmd;
	volatile uint8_t status;
	volatile uint8_t css;
	volatile uint16_t special;
} __attribute__((packed));
#endif

/* NIC mac types. */
typedef enum {
	E1000_UNKNOWN = 0,
	E1000_IGB = 1,
	E1000_82543 = 2,
	E1000_82544 = 3,
	E1000_82545 = 4,
	E1000_82574 = 5,
}__E1000_MAC_TYPE;

/* E1000 device sepcific structure. */
typedef struct i825xx_device_s
{
	/* MAC type. */
	__E1000_MAC_TYPE mac_type;
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Link all E1000 devices together. */
	struct i825xx_device_s* pNext;

	/* genif corresponding to this device. */
	__GENERIC_NETIF* pGenif;

	/* Sending list for tx,not commited to send yet. */
	volatile __ETHERNET_BUFFER* pTxHeader;
	volatile __ETHERNET_BUFFER* pTxTail;
	volatile unsigned long txlist_size;
	/*
	 * Current sending list,commited into hardware to
	 * send but not finished yet.
	 */
	volatile __ETHERNET_BUFFER* pCurrTxHeader;
	volatile __ETHERNET_BUFFER* pCurrTxTail;
	volatile unsigned long currtxlist_size;
	/* How many times the tx queue full. */
	volatile unsigned long txq_full_times;
	/* How many tx frames droped. */
	volatile unsigned long tx_drop;

	/* The corresponding physical device of this NIC. */
	__PHYSICAL_DEVICE* pDev;

	/* PCI id of this NIC. */
	uint16_t vendor_id;
	uint16_t device_id;

	/* Corresponding interrupt object. */
	HANDLE hInterrupt;

	/* I/O and memory resources this NIC occupied. */
	uint16_t ioport_s;
	uint16_t ioport_e;
	LPVOID memaddr_s;
	LPVOID memaddr_e;
	int int_vector;

	/* The index of current NIC in system of the same type. */
	int cardnum;

	/* Specific information. */
	uintptr_t mmio_address;
	uint32_t io_address;
	volatile uint8_t *rx_desc_base;
	/* Receive descriptor buffer. */
	volatile struct e1000_rx_desc *rx_desc[NUM_RX_DESCRIPTORS];
	volatile uint16_t rx_tail;
	volatile uint8_t *tx_desc_base;
	/* transmit descriptor buffer. */
	volatile struct e1000_tx_desc *tx_desc[NUM_TX_DESCRIPTORS];
	/* Pointers tracking the tx descriptor. */
	volatile uint16_t tx_tail;
	volatile uint16_t old_tail;

	/* tx queue empty. */
	volatile BOOL bTxQueueEmpty;

	/* Statistics counters for debugging. */
	unsigned long tx_int_num;
	unsigned long rx_int_num;
	unsigned long ou_int_num; /* overrun or under run interrupts. */
	unsigned long txqe_int_num;

	uint16_t(*eeprom_read)(struct i825xx_device_s *, uint8_t);
} i825xx_device_t;

/* Entry point of E1000E device driver. */
BOOL E1000EX_Drv_Initialize(LPVOID pData);

#endif //__E1000EX_H__
