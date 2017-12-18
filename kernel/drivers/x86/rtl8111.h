//***********************************************************************/
//    Author                    : Tywind.Huang
//    Original Date             : Dec 12, 2015
//    Module Name               : rtl8111.h
//    Module Funciton           : 
//                                This module countains the pre-definitions or
//                                structures for Realtek 8111/8168 series NIC.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __RTL8111_H__
#define __RTL8111_H__

//Disable or enable debugging.
#define __RTL8111_DEBUG__ 1

//Debugging facility.
#define __func__  __FUNCTION__

#define debug_cond(cond, fmt, ...)			\
	do {						\
		if (cond)				\
			_hx_printf(fmt, __VA_ARGS__);	\
										} while (0)

#define _rtl8111_debug(fmt, ...)			\
	do { \
	debug_cond(__RTL8111_DEBUG__, fmt, __VA_ARGS__); \
		}while(0)

#if __RTL8111_DEBUG__
#define assert(expr) \
	if(!(expr)) { _hx_printf("Assertion failed! %s,%s,%s,line=%d\n", #expr,__FILE__,__FUNCTION__,__LINE__);}
#else
#define assert(expr) \
	do{}while(0);
#endif

#define DBG_PRINT _rtl8111_debug

//Vendor ID and device ID.
#define RTL8111_VENDOR_ID  0x10EC
#define RTL8111_DEVICE_ID  0x8168

//MACROs to control NIC operations.
#undef R1000_DEBUG
#undef R1000_JUMBO_FRAME_SUPPORT
//#undef	R1000_HW_FLOW_CONTROL_SUPPORT
#define	R1000_HW_FLOW_CONTROL_SUPPORT
#undef R1000_IOCTL_SUPPORT
#define R1000_USE_IO

/* media options */
#define MAX_UNITS 8

/* MAC address length*/
#define MAC_ADDR_LEN        6

#define RX_FIFO_THRESH      7       /* 7 means NO threshold, Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST        7       /* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST        7       /* Maximum PCI burst, '6' is 1024 */
#define ETTh                0x3F    /* 0x3F means NO threshold */

#define ETH_HDR_LEN         14
#define DEFAULT_MTU         1500
#define DEFAULT_RX_BUF_LEN  1536

#ifdef R1000_JUMBO_FRAME_SUPPORT
#define MAX_JUMBO_FRAME_MTU	( 10000 )
#define MAX_RX_SKBDATA_SIZE	( MAX_JUMBO_FRAME_MTU + ETH_HDR_LEN )
#else
#define MAX_RX_SKBDATA_SIZE 1608
#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT


#define InterFrameGap       0x03    /* 3 means InterFrameGap = the shortest one */

#define NUM_TX_DESC         4     /* Number of Tx descriptors*/
#define NUM_RX_DESC         4     /* Number of Rx descriptors*/

#define RTL_MIN_IO_SIZE     0x80
#define TX_TIMEOUT          (6*1000 / SYSTEM_TIME_SLICE)    //(6*HZ)


#ifdef R1000_USE_IO
#define RTL_W8(reg, val8)   __outb ((val8),(WORD)(ioaddr + (reg)))
#define RTL_W16(reg, val16) __outw ((val16),(WORD)(ioaddr + (reg)))
#define RTL_W32(reg, val32) __outd ((WORD)(ioaddr + (reg)),(val32))  //Please note the parameters sort.
#define RTL_R8(reg)         __inb ((WORD)(ioaddr + (reg)))
#define RTL_R16(reg)        __inw ((WORD)(ioaddr + (reg)))
#define RTL_R32(reg)        ((unsigned long) __ind ((WORD)(ioaddr + (reg))))

#else	//R1000_USE_IO
#define RTL_W8(reg, val8)   __writeb ((val8), ioaddr + (reg))
#define RTL_W16(reg, val16) __writew ((val16), ioaddr + (reg))
#define RTL_W32(reg, val32) __writel ((val32), ioaddr + (reg))
#define RTL_R8(reg)         __readb (ioaddr + (reg))
#define RTL_R16(reg)        __readw (ioaddr + (reg))
#define RTL_R32(reg)        ((unsigned long) __readl (ioaddr + (reg)))
#endif	//R1000_USE_IO

#define MCFG_METHOD_1		0x01
#define MCFG_METHOD_2		0x02
#define MCFG_METHOD_3		0x03
#define MCFG_METHOD_4		0x04
#define MCFG_METHOD_5		0x05
#define MCFG_METHOD_11		0x0B
#define MCFG_METHOD_12		0x0C
#define MCFG_METHOD_13		0x0D
#define MCFG_METHOD_14		0x0E
#define MCFG_METHOD_15		0x0F
#define MCFG_METHOD_16		0x10
#define MCFG_METHOD_17		0x11
#define MCFG_METHOD_18		0x12

#define PCFG_METHOD_1		0x01	//PHY Reg 0x03 bit0-3 == 0x0000
#define PCFG_METHOD_2		0x02	//PHY Reg 0x03 bit0-3 == 0x0001
#define PCFG_METHOD_3		0x03	//PHY Reg 0x03 bit0-3 == 0x0002

enum r1000_registers {
	MAC0 = 0x0,
	MAR0 = 0x8,
	TxDescStartAddr = 0x20,
	TxHDescStartAddr = 0x28,
	FLASH = 0x30,
	ERSR = 0x36,
	ChipCmd = 0x37,
	TxPoll = 0x38,
	IntrMask = 0x3C,
	IntrStatus = 0x3E,
	TxConfig = 0x40,
	RxConfig = 0x44,
	RxMissed = 0x4C,
	Cfg9346 = 0x50,
	Config0 = 0x51,
	Config1 = 0x52,
	Config2 = 0x53,
	Config3 = 0x54,
	Config4 = 0x55,
	Config5 = 0x56,
	MultiIntr = 0x5C,
	PHYAR = 0x60,
	TBICSR = 0x64,
	TBI_ANAR = 0x68,
	TBI_LPAR = 0x6A,
	PHYstatus = 0x6C,
	Off7Ch = 0x7C,
	EPHYAR = 0x80,
	RxMaxSize = 0xDA,
	CPlusCmd = 0xE0,
	RxDescStartAddr = 0xE4,
	ETThReg = 0xEC,
	FuncEvent = 0xF0,
	FuncEventMask = 0xF4,
	FuncPresetState = 0xF8,
	FuncForceEvent = 0xFC,
};

enum r1000_register_content {
	/*InterruptStatusBits*/
	SYSErr = 0x8000,
	PCSTimeout = 0x4000,
	SWInt = 0x0100,
	TxDescUnavail = 0x80,
	RxFIFOOver = 0x40,
	LinkChg = 0x20,
	RxOverflow = 0x10,
	TxErr = 0x08,
	TxOK = 0x04,
	RxErr = 0x02,
	RxOK = 0x01,

	/*RxStatusDesc*/
	RxRES = 0x00200000,
	RxCRC = 0x00080000,
	RxRUNT = 0x00100000,
	RxRWT = 0x00400000,

	/*ChipCmdBits*/
	CmdReset = 0x10,
	CmdRxEnb = 0x08,
	CmdTxEnb = 0x04,
	RxBufEmpty = 0x01,

	/*Cfg9346Bits*/
	Cfg9346_Lock = 0x00,
	Cfg9346_Unlock = 0xC0,

	/*rx_mode_bits*/
	AcceptErr = 0x20,
	AcceptRunt = 0x10,
	AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04,
	AcceptMyPhys = 0x02,
	AcceptAllPhys = 0x01,

	/*RxConfigBits*/
	RxCfgFIFOShift = 13,
	RxCfgDMAShift = 8,

	/*TxConfigBits*/
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,

	/*rtl8169_PHYstatus (MAC offset 0x6C)*/
	TBI_Enable = 0x80,
	TxFlowCtrl = 0x40,
	RxFlowCtrl = 0x20,
	_1000Mbps = 0x10,
	_100Mbps = 0x08,
	_10Mbps = 0x04,
	LinkStatus = 0x02,
	FullDup = 0x01,

	/*GIGABIT_PHY_registers*/
	PHY_CTRL_REG = 0,
	PHY_STAT_REG = 1,
	PHY_AUTO_NEGO_REG = 4,
	PHY_1000_CTRL_REG = 9,

	/*GIGABIT_PHY_REG_BIT*/
	PHY_Restart_Auto_Nego = 0x0200,
	PHY_Enable_Auto_Nego = 0x1000,
	PHY_Reset = 0x8000,

	//PHY_STAT_REG = 1;
	PHY_Auto_Neco_Comp = 0x0020,

	//PHY_AUTO_NEGO_REG = 4;
	PHY_Cap_10_Half = 0x0020,
	PHY_Cap_10_Full = 0x0040,
	PHY_Cap_100_Half = 0x0080,
	PHY_Cap_100_Full = 0x0100,

	//PHY_1000_CTRL_REG = 9;
	PHY_Cap_1000_Full = 0x0200,
	PHY_Cap_1000_Half = 0x0100,

	PHY_Cap_PAUSE = 0x0400,
	PHY_Cap_ASYM_PAUSE = 0x0800,

	PHY_Cap_Null = 0x0,

	/*_MediaType*/
	_10_Half = 0x01,
	_10_Full = 0x02,
	_100_Half = 0x04,
	_100_Full = 0x08,
	_1000_Full = 0x10,

	/*_TBICSRBit*/
	TBILinkOK = 0x02000000,

	BIT31 = (1 << 31),
	BIT30 = (1 << 30),
	BIT29 = (1 << 29),
	BIT28 = (1 << 28),
	BIT27 = (1 << 27),
	BIT26 = (1 << 26),
	BIT25 = (1 << 25),
	BIT24 = (1 << 24),
	BIT23 = (1 << 23),
	BIT22 = (1 << 22),
	BIT21 = (1 << 21),
	BIT20 = (1 << 20),
	BIT19 = (1 << 19),
	BIT18 = (1 << 18),
	BIT17 = (1 << 17),
	BIT16 = (1 << 16),
	BIT15 = (1 << 15),
	BIT14 = (1 << 14),
	BIT13 = (1 << 13),
	BIT12 = (1 << 12),
	BIT11 = (1 << 11),
	BIT10 = (1 << 10),
	BIT9 = (1 << 9),
	BIT8 = (1 << 8),
	BIT7 = (1 << 7),
	BIT6 = (1 << 6),
	BIT5 = (1 << 5),
	BIT4 = (1 << 4),
	BIT3 = (1 << 3),
	BIT2 = (1 << 2),
	BIT1 = (1 << 1),
	BIT0 = (1 << 0),
};



enum _DescStatusBit {
	OWNbit = 0x80000000,
	EORbit = 0x40000000,
	FSbit = 0x20000000,
	LSbit = 0x10000000,
};


struct TxDesc {
	__u32		status;
	__u32		vlan_tag;
	__u32		buf_addr;
	__u32		buf_Haddr;
};

struct RxDesc {
	__u32		status;
	__u32		vlan_tag;
	__u32		buf_addr;
	__u32		buf_Haddr;
};

//RTL8111 ethernet NIC name.Please be noted that the last character is the sort of a
//NIC in system,since multiple NIC may exist.
#define RTL8111_NIC_NAME "RTL8111_NIC_%d"

//Private data structure for RTL8111 NIC.
struct rtl8111_priv{
	struct rtl8111_priv* next;
	int available;                 //If the NIC is available.
	__ETHERNET_INTERFACE* pEthInt; //Ethernet Interface associated to RTL8111 NIC.
	unsigned char macAddr[6];      //MAC address of this interface.

	unsigned long ioaddr;          //IO port base address.
	unsigned long memaddr;         //Memory mapping base.
	char intVector;                //Interrupt vector.
	HANDLE hInterrupt;             //Interrupt object of this NIC.
	//struct pci_dev *pci_dev;           /* Index of PCI device  */
	__PHYSICAL_DEVICE*   pPhysicalDev;   //Instead of pci_dev.
	//struct net_device_stats stats;     /* statistics of net device */
	//spinlock_t lock;                   /* spin lock flag */
	int chipset;
	int mcfg;
	int pcfg;
	//rt_timer_t r1000_timer;
	unsigned long expire_time;

	unsigned long phy_link_down_cnt;
	unsigned long cur_rx;                   /* Index into the Rx descriptor buffer of next Rx pkt. */
	unsigned long cur_tx;                   /* Index into the Tx descriptor buffer of next Rx pkt. */
	unsigned long dirty_tx;
	struct	TxDesc	*TxDescArray;           /* Index of 256-alignment Tx Descriptor buffer */
	struct	RxDesc	*RxDescArray;           /* Index of 256-alignment Rx Descriptor buffer */
	//struct	sk_buff	*Tx_skbuff[NUM_TX_DESC]; /* Index of Transmit data buffer */
	//struct	sk_buff	*Rx_skbuff[NUM_RX_DESC]; /* Receive data buffer */
	unsigned char   drvinit_fail;

	dma_addr_t txdesc_array_dma_addr[NUM_TX_DESC];
	dma_addr_t rxdesc_array_dma_addr[NUM_RX_DESC];
	dma_addr_t rx_skbuff_dma_addr[NUM_RX_DESC];

	void *txdesc_space;
	dma_addr_t txdesc_phy_dma_addr;
	int sizeof_txdesc_space;

	void *rxdesc_space;
	dma_addr_t rxdesc_phy_dma_addr;
	int sizeof_rxdesc_space;

	int curr_mtu_size;
	int tx_pkt_len;
	int rx_pkt_len;

	int hw_rx_pkt_len;

	__u16	speed;
	__u8	duplex;
	__u8	autoneg;
};

typedef struct rtl8111_priv rtl8111_priv_t;

//Entry point of the RTL8111 driver.
BOOL RTL8111_Drv_Initialize(LPVOID pData);

#endif //__RTL8111_H__
