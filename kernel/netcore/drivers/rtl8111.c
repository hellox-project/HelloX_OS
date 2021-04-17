//***********************************************************************/
//    Author                    : Tywin.Huang
//    Original Date             : Dec 12, 2015
//    Module Name               : rtl8111.c
//    Module Funciton           : 
//                                This module countains the implementation code
//                                of Realtek 8111/8168 series NIC.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <pci_drv.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hx_inet.h"
#include "ethmgr.h"
#include "rtl8111.h"

//RxDescriptor and TxDescriptor array should align to 256 byte boundary,forced
//by hardware.
#define  RTL8111_ALIGN_SIZE     256

#define	 IRQ_NONE               0
#define	 IRQ_HANDLED            1
#define  ETH_ZLEN               60

//Chipset info array.
const static struct
{
	const char *name;
	unsigned char  mcfg;        /* depend on documents of Realtek */
	unsigned int RxConfigMask;  /* should clear the bits supported by this chip */
}
rtl_chip_info[] = {
	{ "RTL8169", MCFG_METHOD_1, 0xff7e1880 },
	{ "RTL8169S/8110S", MCFG_METHOD_2, 0xff7e1880 },
	{ "RTL8169S/8110S", MCFG_METHOD_3, 0xff7e1880 },
	{ "RTL8169SB/8110SB", MCFG_METHOD_4, 0xff7e1880 },
	{ "RTL8169SC/8110SC", MCFG_METHOD_5, 0xff7e1880 },
	{ "RTL8168B/8111B", MCFG_METHOD_11, 0xff7e1880 },
	{ "RTL8168B/8111B", MCFG_METHOD_12, 0xff7e1880 },
	{ "RTL8101E", MCFG_METHOD_13, 0xff7e1880 },
	{ "RTL8100E", MCFG_METHOD_14, 0xff7e1880 },
	{ "RTL8100E", MCFG_METHOD_15, 0xff7e1880 },
	{ "RTL8168C", MCFG_METHOD_16, 0xff7e1880 },
	{ "RTL8168CP", MCFG_METHOD_17, 0xff7e1880 },
	{ "RTL8168C", MCFG_METHOD_18, 0xff7e1880 },
	{ 0 }
};

//Receive config register and interrupt mask register.
static const unsigned int   rtl8111_rx_config = (RX_FIFO_THRESH << RxCfgFIFOShift) | (RX_DMA_BURST << RxCfgDMAShift) | 0x0000000E;
static const unsigned short rtl8111_intr_mask = LinkChg | RxOverflow | RxFIFOOver | TxErr | TxOK | RxErr | RxOK;

//Global variables for RTL8111 NIC.
static rtl8111_priv_t* global_nic = NULL;

static int max_interrupt_work = 20;

static void get_mac_addr(rtl8111_priv_t* prvi)
{
	unsigned long ioaddr = prvi->ioaddr;
	int i;

	// Get MAC address.
	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		prvi->macAddr[i] = RTL_R8(MAC0 + i);
	}

	_hx_printf("macAddr[%X,%X,%X,%X,%X,%X]\r\n",
		(BYTE)prvi->macAddr[0],
		(BYTE)prvi->macAddr[1],
		(BYTE)prvi->macAddr[2],
		(BYTE)prvi->macAddr[3],
		(BYTE)prvi->macAddr[4],
		(BYTE)prvi->macAddr[5]);
}

void RTL111_WRITE_GMII_REG(unsigned long ioaddr, int RegAddr, int value)
{
	int	i;

	RTL_W32(PHYAR, 0x80000000 | (RegAddr & 0xFF) << 16 | value);
	__MicroDelay(1000);

	for (i = 2000; i > 0; i--)
	{
		// Check if the RTL8169 has completed writing to the specified MII register
		if (!(RTL_R32(PHYAR) & 0x80000000))
		{
			break;
		}
		else
		{
			__MicroDelay(100);
		}
	}
}

//Enumerates all PCNet NICs in system,by searching the PCI bus device list.
//Create a rtl8111_priv_t structure for each PCNet NIC device.
static BOOL RTL8111_NIC_EUM()
{
	__PHYSICAL_DEVICE*    pDev = NULL;
	__IDENTIFIER          devId;
	rtl8111_priv_t*       priv = NULL;
	__U16                 iobase = 0;
	LPVOID                memAddr = NULL;
	int                   intVector = 0;
	int                   index = 0;
	BOOL                  bResult = FALSE;

	//Set device ID accordingly.
	devId.dwBusType = BUS_TYPE_PCI;
	devId.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_VENDOR | PCI_IDENTIFIER_MASK_DEVICE;
	devId.Bus_ID.PCI_Identifier.wVendor = RTL8111_VENDOR_ID;
	devId.Bus_ID.PCI_Identifier.wDevice = RTL8111_DEVICE_ID;

	//Try to fetch the physical device with the specified ID.
	pDev = DeviceManager.GetDevice(
		&DeviceManager,
		BUS_TYPE_PCI,
		&devId,
		NULL);
	if (NULL == pDev)  //PCNet may not exist.
	{
		_rtl8111_debug("%s:Can not get any RTL8111 NIC device in system.\r\n", __func__);
		goto __TERMINAL;
	}

	//Got a valid device,retrieve it's hardware resource and save them into 
	//rtl8111_priv_t structure.
	while (pDev)
	{
		for (index = 0; index < MAX_RESOURCE_NUM; index++)
		{
			if (pDev->Resource[index].dwResType == RESOURCE_TYPE_INTERRUPT)
			{
				intVector = pDev->Resource[index].Dev_Res.ucVector;
			}
			if (pDev->Resource[index].dwResType == RESOURCE_TYPE_IO)
			{
				iobase = pDev->Resource[index].Dev_Res.IOPort.wStartPort;
			}
			if (pDev->Resource[index].dwResType == RESOURCE_TYPE_MEMORY)
			{
				memAddr = pDev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr;
			}
		}
		//Check if the device with valid resource.
		if ((0 == intVector) && (0 == iobase))
		{
			_rtl8111_debug("%s: Find a device without valid resource.\r\n", __func__);
			goto __TERMINAL;
		}

		//Now allocate a rtl8111_priv_t structure and fill the resource.
		priv = (rtl8111_priv_t*)_hx_malloc(sizeof(rtl8111_priv_t));
		if (NULL == priv)
		{
			_rtl8111_debug("%s:allocate rtl8111 private structure failed.\r\n", __func__);
			goto __TERMINAL;
		}
		memset(priv, 0, sizeof(rtl8111_priv_t));

		priv->ioaddr = iobase;
		priv->memaddr = (unsigned long)memAddr;
		priv->intVector = (char)intVector;
		priv->available = 1;
		//Link the private object into global list.
		priv->next = global_nic;
		global_nic = priv;

		_rtl8111_debug("%s: Get a PCNet NIC device[int = %d,iobase = 0x%X].\r\n",
			__func__,
			intVector,
			iobase);

		//Try to fetch another PCNet device.
		pDev = DeviceManager.GetDevice(
			&DeviceManager,
			BUS_TYPE_PCI,
			&devId,
			pDev);
	}
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		_rtl8111_debug("%s: Enumerate PCNet NIC device failed.\r\n", __func__);
		//May release resource,such as rtl8111_priv_t structures here.
	}
	return bResult;
}

//Receive a frame from link into device private data structure's buffer,and return
//the buffer to caller.
static char* RTL8111_Recv(rtl8111_priv_t* priv, int* len)
{
	unsigned long ioaddr = priv->ioaddr;
	char*         rtlbuf = NULL;
	int           cur_rx;
	int           pkt_size = 0;
	int           rxdesc_cnt = 0;
	struct	RxDesc	*rxdesc;

	cur_rx = priv->cur_rx;
	rxdesc = &priv->RxDescArray[cur_rx];
	//Synchronizing cache content if necessary.
	__FLUSH_CACHE(rxdesc, sizeof(struct RxDesc), CACHE_FLUSH_INVALIDATE);

	while (((le32_to_cpu(rxdesc->status) & OWNbit) == 0) && (rxdesc_cnt < max_interrupt_work))
	{
		rxdesc_cnt++;
		if (le32_to_cpu(rxdesc->status) & RxRES)
		{
			//printk (KERN_INFO "%s: Rx ERROR!!!\n", netdev->name);			
		}
		else
		{
			pkt_size = (int)(le32_to_cpu(rxdesc->status) & 0x00001FFF) - 4;
			if (pkt_size > priv->rx_pkt_len)
			{
				_hx_printf("%s: Error -- Rx packet size(%d) > mtu(%d)+14\n",
					__func__,
					pkt_size,
					priv->rx_pkt_len);
				pkt_size = priv->rx_pkt_len;
			}

			rtlbuf = (char*)rxdesc->buf_addr;
			// Update rx descriptor
			if (cur_rx == (NUM_RX_DESC - 1))
			{
				priv->RxDescArray[cur_rx].status = 
					cpu_to_le32((OWNbit | EORbit) | (unsigned long)priv->hw_rx_pkt_len);
			}
			else
			{
				priv->RxDescArray[cur_rx].status = 
					cpu_to_le32(OWNbit | (unsigned long)priv->hw_rx_pkt_len);
			}
			__FLUSH_CACHE(rxdesc, sizeof(struct RxDesc), CACHE_FLUSH_WRITEBACK);
		}

		cur_rx = (cur_rx + 1) % NUM_RX_DESC;
		rxdesc = &priv->RxDescArray[cur_rx];
		__FLUSH_CACHE(rxdesc, sizeof(struct RxDesc), CACHE_FLUSH_INVALIDATE);

		//only get one rtl111 frame
		if (rtlbuf)
		{
			*len = pkt_size;
			break;
		}
	}

	priv->cur_rx = cur_rx;
	return rtlbuf;
}

//Send out a frame to net link,return 0 if fail,otherwise no-zero.
static int RTL8111_Send(rtl8111_priv_t* dev, char* buff, int len)
{
	rtl8111_priv_t*  priv = dev;
	unsigned long    ioaddr = priv->ioaddr;
	int              entry = priv->cur_tx % NUM_TX_DESC;
	int              buf_len = 60;
	dma_addr_t       txbuf_dma_addr = 0;

	//Synchronizing cache content.
	__FLUSH_CACHE(&priv->TxDescArray[entry], sizeof(struct TxDesc), CACHE_FLUSH_INVALIDATE);

	if ((le32_to_cpu(priv->TxDescArray[entry].status) & OWNbit) == 0)
	{
		if (len <= priv->tx_pkt_len)
		{
			buf_len = len;
		}
		else
		{
			//printk("%s: Error -- Tx packet size(%d) > mtu(%d)+14\n", netdev->name, skb->len, netdev->mtu);
			buf_len = priv->tx_pkt_len;
		}

		//memcpy((char*)priv->TxDescArray[entry].buf_addr, buff, buf_len);
		priv->TxDescArray[entry].buf_addr = (__u32)buff;
		//Write back the cache content if necessary.
		__FLUSH_CACHE((char*)priv->TxDescArray[entry].buf_addr, buf_len, CACHE_FLUSH_WRITEBACK);

		if (entry != (NUM_TX_DESC - 1))
		{
			priv->TxDescArray[entry].status = cpu_to_le32((OWNbit | FSbit | LSbit) | buf_len);
		}
		else
		{
			priv->TxDescArray[entry].status = cpu_to_le32((OWNbit | EORbit | FSbit | LSbit) | buf_len);
		}
		//Write back the TxDescriptor content from cache to main memory.
		__FLUSH_CACHE(&priv->TxDescArray[entry], sizeof(struct TxDesc), CACHE_FLUSH_WRITEBACK);

		//Start to poll.
		RTL_W8(TxPoll, 0x40);

		priv->cur_tx++;
	}
	__MicroDelay(1000);
	return len;
}

//Interrupt handler for transmition.
static BOOL RTL8111_TX_Interrupt(rtl8111_priv_t* priv)
{
	unsigned long  ioaddr = priv->ioaddr;
	unsigned long  dirty_tx, tx_left = 0;
	int txloop_cnt = 0;
	int entry = priv->cur_tx % NUM_TX_DESC;

	dirty_tx = priv->dirty_tx;
	tx_left = priv->cur_tx - dirty_tx;

	while ((tx_left > 0) && (txloop_cnt < max_interrupt_work))
	{
		if ((le32_to_cpu(priv->TxDescArray[entry].status) & OWNbit) == 0)
		{
			//frame already send
			dirty_tx++;
			tx_left--;
			entry++;
		}
		txloop_cnt++;
	}

	if (priv->dirty_tx != dirty_tx)
	{
		priv->dirty_tx = dirty_tx;
	}

	return TRUE;
}

//Interrupt handler for RxOK.
static BOOL RTL8111_RX_Interrupt(rtl8111_priv_t* priv)
{
	__ETHERNET_INTERFACE*  pEthInt = priv->pEthInt;
	__ETHERNET_BUFFER*  pEthBuff = NULL;
	unsigned char*      buf = NULL;
	int                 len = 0;

	if (NULL == pEthInt)
	{
		BUG();
	}

	while (TRUE)
	{
		pEthBuff = NULL;
		buf = NULL;
		len = 0;

		buf = (unsigned char*)RTL8111_Recv(priv, &len);
		if (buf)
		{
			//Received a pakcet,post to kernel.			
			pEthBuff = EthernetManager.CreateEthernetBuffer(len, 0);
			if (NULL == pEthBuff)
			{
				_rtl8111_debug("  %s: create ethernet buffer failed.\r\n", __func__);
				return FALSE;
			}
			else  //Copy the received data to Ethernet Buffer object.
			{
				memcpy(pEthBuff->Buffer, buf, len);
				pEthBuff->act_length = len;
				//Fill the MAC addresses and packet types.
				memcpy(pEthBuff->dstMAC, buf, ETH_MAC_LEN);
				buf += ETH_MAC_LEN;
				memcpy(pEthBuff->srcMAC, buf, ETH_MAC_LEN);
				buf += ETH_MAC_LEN;
				pEthBuff->frame_type = _hx_ntohs(*(__u16*)buf);
				//pEthBuff->pEthernetInterface = pEthInt;
				pEthBuff->pInInterface = pEthInt;
				pEthBuff->buff_status = ETHERNET_BUFFER_STATUS_INITIALIZED;
			}
			if (!EthernetManager.PostFrame(pEthInt, pEthBuff))
			{
				//Must destroy the ethernet buffer object,since it may lead memory leak.
				EthernetManager.DestroyEthernetBuffer(pEthBuff);
			}
		}
		else
		{
			//No packet received.
			break;
		}
	}

	return TRUE;
}

//Interrupt handler for RxFIFOOver and RxOverflow,we just print out a warning
//message currently,since it seldom occur in real network,RxOK or others instead.
static BOOL RxOverflowHandler(rtl8111_priv_t* priv,__u32 status)
{
	static unsigned int overflowCnt = 0;

	_hx_printf("RxOverflow in rtl8111,status = %X,overflow counter = %d.\r\n", 
		status,
		overflowCnt ++);
	return FALSE;
}

//Interrupt handler of ethernet link status change.we just keep it empty now.
static BOOL LinkChangeHandler(rtl8111_priv_t* priv, __u32 status)
{
	_hx_printf("Ethernet link connect or pull out.\r\n");
	return FALSE;
}

//Interrupt handler of RTL8111 NIC.
static BOOL RTL8111_Interrupt(LPVOID lpESP, LPVOID lpParam)
{
	rtl8111_priv_t* priv = (rtl8111_priv_t*)lpParam;
	int             boguscnt = max_interrupt_work;
	unsigned long   ioaddr = priv->ioaddr;
	unsigned int    status = 0;
	unsigned int    phy_status = 0;

	//Disable interrupt.
	RTL_W16(IntrMask, 0x0000);
	do {

		status = RTL_R16(IntrStatus);
		if (status == 0xFFFF)
		{
			break;
		}
		//Ackowledge interrupt by writting back the corresponding bit(s).
		RTL_W16(IntrStatus, status);

		if ((status & rtl8111_intr_mask) == 0)
		{
			//_hx_printf("%s:Interrupt masked but raised,status = %X.\r\n"__func__,status);
			break;
		}

		// Rx interrupt.
		if (status & (RxOK | RxErr))
		{
			RTL8111_RX_Interrupt(priv);
		}
		
		//Link change interrupt.
		if (status & LinkChg)
		{
			LinkChangeHandler(priv, status);
		}

		//RxFIFO overflow or RxDesc overflow.
		if (status & (RxOverflow | RxFIFOOver))
		{
			//RxOverflowHandler(priv, status);
		}

		// Tx interrupt
		if (status & (TxOK | TxErr))
		{
			RTL8111_TX_Interrupt(priv);
		}

		if ((status & TxOK) && (status & TxDescUnavail))
		{
			RTL_W8(TxPoll, 0x40);
		}

		phy_status = RTL_R8(PHYstatus);
		boguscnt--;
	} while (boguscnt > 0);

	//Enable interrupt again.
	RTL_W16(IntrMask, rtl8111_intr_mask);
	return TRUE;
}

static void RTL8111set_RX_mode(rtl8111_priv_t *priv)
{
	unsigned long ioaddr = priv->ioaddr;
	unsigned int mc_filter[2];	/* Multicast hash filter */
	int rx_mode;
	unsigned int tmp = 0;

	rx_mode = /*AcceptBroadcast |*/ AcceptMulticast | AcceptMyPhys | AcceptAllPhys;
	mc_filter[1] = mc_filter[0] = 0xffffffff;

	//rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
	//mc_filter[1] = mc_filter[0] = 0xffffffff;	
	//rx_mode = AcceptBroadcast | AcceptMulticast | AcceptMyPhys;
	//mc_filter[1] = mc_filter[0] = 0;

	tmp = rtl8111_rx_config | rx_mode | (RTL_R32(RxConfig) & rtl_chip_info[priv->chipset].RxConfigMask);
	RTL_W32(RxConfig, tmp);
	RTL_W32(MAR0 + 0, mc_filter[0]);
	RTL_W32(MAR0 + 4, mc_filter[1]);
}

static BOOL RTL8111_Start(rtl8111_priv_t* priv)
{
	unsigned long ioaddr = priv->ioaddr;
	int  i;
	BOOL bResult = FALSE;

	priv->cur_rx = 0;
	priv->cur_tx = 0;
	priv->dirty_tx = 0;

	//alloc txdesc/rxdesc buf.
	priv->sizeof_txdesc_space = NUM_TX_DESC * sizeof(struct TxDesc);
	priv->txdesc_space = _hx_aligned_malloc(priv->sizeof_txdesc_space, RTL8111_ALIGN_SIZE);
	if (NULL == priv->txdesc_space)
	{
		_hx_printf("%s:can not allocate txdesc for rtl8111.\r\n", __func__);
		goto __TERMINAL;
	}
	priv->txdesc_phy_dma_addr = (dma_addr_t)priv->txdesc_space;

	priv->sizeof_rxdesc_space = NUM_RX_DESC * sizeof(struct RxDesc);
	priv->rxdesc_space = _hx_aligned_malloc(priv->sizeof_rxdesc_space, RTL8111_ALIGN_SIZE);
	if (NULL == priv->rxdesc_space)
	{
		_hx_printf("%s:can not allocate rxdesc for rtl8111.\r\n", __func__);
		goto __TERMINAL;
	}
	priv->rxdesc_phy_dma_addr = (dma_addr_t)priv->rxdesc_space;

	// Set tx/rx descriptor space
	priv->TxDescArray = (struct TxDesc *)priv->txdesc_space;
	priv->RxDescArray = (struct RxDesc *)priv->rxdesc_space;

	memset(priv->TxDescArray, 0, priv->sizeof_txdesc_space);
	memset(priv->RxDescArray, 0, priv->sizeof_rxdesc_space);

	for (i = 0; i < NUM_RX_DESC; i++)
	{
		if (i == (NUM_RX_DESC - 1))
		{
			priv->RxDescArray[i].status = 
				cpu_to_le32((OWNbit | EORbit) | (unsigned long)priv->hw_rx_pkt_len);
		}
		else
		{
			priv->RxDescArray[i].status = 
				cpu_to_le32(OWNbit | (unsigned long)priv->hw_rx_pkt_len);
		}

		priv->rx_skbuff_dma_addr[i] = (dma_addr_t)_hx_malloc(MAX_RX_SKBDATA_SIZE);
		if (0 == priv->rx_skbuff_dma_addr[i])
		{
			_hx_printf("%s:can not allocate rx buffer.\r\n", __func__);
			goto __TERMINAL;
		}
		priv->RxDescArray[i].buf_addr = cpu_to_le32(priv->rx_skbuff_dma_addr[i]);
		priv->RxDescArray[i].buf_Haddr = 0;
		priv->rxdesc_array_dma_addr[i] = (dma_addr_t)&priv->RxDescArray[i];
	}

	//Synchronizing cache content if necessary.
	__FLUSH_CACHE(priv->TxDescArray, priv->sizeof_txdesc_space, CACHE_FLUSH_WRITEBACK);
	__FLUSH_CACHE(priv->RxDescArray, priv->sizeof_rxdesc_space, CACHE_FLUSH_WRITEBACK);

	if ((priv->mcfg == MCFG_METHOD_1) || 
		(priv->mcfg == MCFG_METHOD_2) || 
		(priv->mcfg == MCFG_METHOD_3) || 
		(priv->mcfg != MCFG_METHOD_4))
	{
		/* Soft reset the chip. */
		RTL_W8(ChipCmd, CmdReset);

		/* Check that the chip has finished the reset. */
		for (i = 1000; i > 0; i--)
		{
			if ((RTL_R8(ChipCmd) & CmdReset) == 0)
			{
				_rtl8111_debug("%s:soft reset OK.\r\n",__func__);
				break;
			}
			else
			{
				__MicroDelay(10);
			}
		}

		RTL_W8(Cfg9346, Cfg9346_Unlock);
		RTL_W8(ChipCmd, CmdTxEnb | CmdRxEnb);
		RTL_W8(ETThReg, ETTh);

		// For gigabit rtl8169
		RTL_W16(RxMaxSize, (unsigned short)priv->hw_rx_pkt_len);

		// Set Rx Config register
		i = rtl8111_rx_config | (RTL_R32(RxConfig) & rtl_chip_info[priv->chipset].RxConfigMask);
		RTL_W32(RxConfig, i);

		/* Set DMA burst size and Interframe Gap Time */
		RTL_W32(TxConfig, (TX_DMA_BURST << TxDMAShift) | (InterFrameGap << TxInterFrameGapShift));
		RTL_W16(CPlusCmd, RTL_R16(CPlusCmd));

		if ((priv->mcfg == MCFG_METHOD_2) || (priv->mcfg == MCFG_METHOD_3))
		{
			RTL_W16(CPlusCmd, (RTL_R16(CPlusCmd) | (1 << 14) | (1 << 3)));
			_rtl8111_debug("Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n");
		}
		else
		{
			RTL_W16(CPlusCmd, (RTL_R16(CPlusCmd) | (1 << 3)));
			_rtl8111_debug("Set MAC Reg C+CR Offset 0xE0: bit-3.\n");
		}
		RTL_W16(0xE2, 0x0000);

		priv->cur_rx = 0;

		RTL_W32(TxDescStartAddr, priv->txdesc_phy_dma_addr);
		RTL_W32(TxDescStartAddr + 4, 0x00);
		RTL_W32(RxDescStartAddr, priv->rxdesc_phy_dma_addr);
		RTL_W32(RxDescStartAddr + 4, 0x00);

		RTL_W8(Cfg9346, Cfg9346_Lock);
		__MicroDelay(10);

		RTL_W32(RxMissed, 0);

		RTL8111set_RX_mode(priv);
		RTL_W16(MultiIntr, RTL_R16(MultiIntr) & 0xF000);
		RTL_W16(IntrMask, rtl8111_intr_mask);
	}

	priv->hInterrupt = ConnectInterrupt(
		"int_rtl8111",
		RTL8111_Interrupt,
		global_nic,
		priv->intVector + INTERRUPT_VECTOR_BASE);
	if (NULL == priv->hInterrupt)
	{
		_hx_printf("%s:can not connect interrupt object[vector = %d].\r\n",
			__func__,
			priv->intVector + INTERRUPT_VECTOR_BASE);
		goto __TERMINAL;
	}

	//Mark everything success.
	bResult = TRUE;

__TERMINAL:
	if (!bResult)  //Should release all resource allocated.
	{
		_hx_printf("%s:failed to execute this routine.\r\n", __func__);
	}
	return bResult;
}

static BOOL RTL8111_NIC_Init()
{
	unsigned long ioaddr = global_nic->ioaddr;
	int           i = 0;

	//set mtu size
	global_nic->curr_mtu_size = ETH_DEFAULT_MTU;
	global_nic->tx_pkt_len = ETH_DEFAULT_MTU + ETH_HDR_LEN;
	global_nic->rx_pkt_len = ETH_DEFAULT_MTU + ETH_HDR_LEN;
	global_nic->hw_rx_pkt_len = global_nic->rx_pkt_len + 8;

	// Soft reset the chip.
	RTL_W8(ChipCmd, CmdReset);

	// Check that the chip has finished the reset.
	for (i = 1000; i > 0; i--)
	{
		if ((RTL_R8(ChipCmd) & CmdReset) == 0)
		{
			break;
		}
		else
		{
			__MicroDelay(10);
		}
	}

	//Set the config method manually,since it can not locate proper predifined values...
	//This is a risk but it works in MainnowBoard MAX environment.
	global_nic->mcfg = MCFG_METHOD_1;
	global_nic->pcfg = PCFG_METHOD_1;

	//Save MAC address of the NIC.
	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		global_nic->macAddr[i] = RTL_R8(MAC0 + i);
	}

	return TRUE;
}

//A helper routine to show all RTL8111 NIC devices in lp list.
static void ShowAllNICs()
{
	rtl8111_priv_t* dev = global_nic;
	int             index = 0;

	while (dev)
	{
		if (dev->available)  //Available device,show it.
		{
			_rtl8111_debug("RTL8111: NIC device[%d] information:\r\n", index++);
			_rtl8111_debug("    MAC address :    %02X-%02X-%02X-%02X-%02X-%02X.\r\n",
				dev->macAddr[0],
				dev->macAddr[1],
				dev->macAddr[2],
				dev->macAddr[3],
				dev->macAddr[4],
				dev->macAddr[5]);
			_rtl8111_debug("\r\n");
		}
		else
		{
			_rtl8111_debug("RTL8111: Encounter an unavailable NIC device.\r\n");
		}

		dev = dev->next;
	}
}

//Enable all RTL8111 NIC's interrupt,only rint,sint,idint are enavbled.This function
//should be invoked at the end of the PCNet driver's entry point,since it may lead to system crash
//when data structures are not established before the end of driver's entry point.
static void EnableAllInterrupt()
{
	return;
}

//Initializer of the ethernet interface,it will be called by HelloX's ethernet framework.
//No need to specify it if no customized requirement.
static BOOL Ethernet_Int_Init(__ETHERNET_INTERFACE* pInt)
{
	return TRUE;
}

//Control functions of the ethernet interface.Some special operations,such as
//scaninn or association in WLAN,should be implemented in this routine,since they
//are not common operations for Ethernet.
static BOOL Ethernet_Ctrl(__ETHERNET_INTERFACE* pInt, DWORD dwOperation, LPVOID pData)
{
	return TRUE;
}

//Send a ethernet frame out through Marvell wifi interface.The frame's content is in pInt's
//send buffer.
static BOOL Ethernet_SendFrame(__ETHERNET_INTERFACE* pInt, __ETHERNET_BUFFER* pOutFrame)
{
	rtl8111_priv_t*    dev = NULL;
	__ETHERNET_BUFFER* pEthBuff = NULL;
	BOOL               bResult = FALSE;

	if (NULL == pInt)
	{
		goto __TERMINAL;
	}
	pEthBuff = &pInt->SendBuffer;

	//No data to send or exceed the MTU(include ethernet frame header).
	if ((0 == pEthBuff->act_length) || (pEthBuff->act_length > (ETH_DEFAULT_MTU + ETH_HEADER_LEN)))
	{
		goto __TERMINAL;
	}

	//Invoke sending routine of NIC to do actual transmition.
	dev = pInt->pIntExtension;
	if (0 == RTL8111_Send(dev, pEthBuff->Buffer, pEthBuff->act_length))
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/**
*
* Receive a frame from ehternet link.
* Should allocate a ethernet buffer and transfer the bytes of the incoming
* packet from the interface into the ethernet buffer.
*
*/
static __ETHERNET_BUFFER* Ethernet_RecvFrame(__ETHERNET_INTERFACE* pInt)
{
	__ETHERNET_BUFFER* pEthBuff = NULL;
	rtl8111_priv_t*    dev = NULL;
	unsigned char*     buf = NULL;
	int                len = 0;

	if (NULL == pInt)
	{
		return NULL;
	}

	dev = (rtl8111_priv_t*)pInt->pIntExtension;
	if (NULL == dev)
	{
		return NULL;
	}

	buf = RTL8111_Recv(dev, &len);
	if (!buf)  //No packet received.
	{
		return NULL;
	}

	//Received a pakcet,delivery it to IP stack.
	if (len > 0)
	{
		pEthBuff = EthernetManager.CreateEthernetBuffer(len, 0);
		if (NULL == pEthBuff)
		{
			_rtl8111_debug("  %s: create ethernet buffer failed.\r\n", __func__);
			return NULL;
		}
		else  //Copy the received data to Ethernet Buffer object.
		{
			memcpy(pEthBuff->Buffer, buf, len);
			pEthBuff->act_length = len;
			//Fill the MAC addresses and packet types.
			memcpy(pEthBuff->dstMAC, buf, ETH_MAC_LEN);
			buf += ETH_MAC_LEN;
			memcpy(pEthBuff->srcMAC, buf, ETH_MAC_LEN);
			buf += ETH_MAC_LEN;
			pEthBuff->frame_type = _hx_ntohs(*(__u16*)buf);
			//pEthBuff->pEthernetInterface = pInt;
			pEthBuff->pInInterface = pInt;
			pEthBuff->buff_status = ETHERNET_BUFFER_STATUS_INITIALIZED;
		}
	}
	return pEthBuff;
}

//Initializer of the PCNet Ethernet Driver,it's a global function and is called
//by Ethernet Manager in process of initialization.
BOOL RTL8111_Drv_Initialize(LPVOID pData)
{
	__ETHERNET_INTERFACE* pNetInt = NULL;
	char                  strEthName[64] = { 0 };
	rtl8111_priv_t*       dev = NULL;
	BOOL                  bResult = FALSE;
	int                   index = 0;

	//Enumerate all RTL8111 NIC device(s) in system.
	if (!RTL8111_NIC_EUM())
	{
		_rtl8111_debug("%s:can not find any RTL8111 NIC in system.\r\n", __func__);
		goto __TERMINAL;
	}

	//Initialize all NICs in system.
	if (!RTL8111_NIC_Init())
	{
		_rtl8111_debug("%s:can not initialize RTL8111 NIC in system.\r\n", __func__);
		goto __TERMINAL;
	}

	//Show all NICs in system.
#ifdef __RTL8111_DEBUG__
	ShowAllNICs();
#endif

	//Register the ethernet interface to HelloX's network framework.	
	dev = global_nic;

	RTL8111_Start(dev);
	while (dev)
	{
		//Skip the NICs that is not available.
		if (!dev->available)
		{
			dev = dev->next;
			index++;
			continue;
		}

		//Construct interface's name.
		_hx_sprintf(strEthName, RTL8111_NIC_NAME, index);
		dev->pEthInt = EthernetManager.AddEthernetInterface(
			strEthName,
			dev->macAddr,
			(LPVOID)dev,
			Ethernet_Int_Init,
			Ethernet_SendFrame,
			Ethernet_RecvFrame,
			Ethernet_Ctrl);
		if (NULL == dev->pEthInt)
		{
			_rtl8111_debug("%s: Add Ethernet Interface failid[index = %d].\r\n", __func__, index);
			//Mark the structure as unavailable,it will be released later.
			dev->available = 0;
		}
		//Process next one.
		dev = dev->next;
		index++;
	}

	//Enable all interrupts of ALL NICs in system.
	EnableAllInterrupt();

	//Mark the driver loading process is successful.
	bResult = TRUE;

__TERMINAL:

	return bResult;
}
