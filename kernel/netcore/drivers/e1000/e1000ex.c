//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 10,2018
//    Module Name               : e1000e.h
//    Module Funciton           : 
//                                Source code of E1000E ethernet controller's 
//                                driver, for HelloX OS.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "KAPI.H"
#include "stdlib.h"
#include "stdio.h"
#include "PCI_DRV.H"
#include "pci_ids.h"
#include "hx_inet.h"

#include "netmgr.h"
#include "e1000ex.h"

/* PCI device ID. */
struct pci_device_id {
	unsigned short vendor_id;
	unsigned short device_id;
};
#define PCI_DEVICE(vendor,device) {vendor,device}

/* Device ID array that the device driver can support. */
static struct pci_device_id e1000_supported[] = {
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82543GC_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544EI_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82544GC_LOM),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545GM_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82545EM_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546EB_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546GB_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82540EM_LOM),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82541ER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82541GI_LF),
	/* E1000 PCIe card */
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571PT_QUAD_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_QUAD_COPPER_LOWPROFILE),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES_DUAL),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82571EB_SERDES_QUAD),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_FIBER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI_SERDES),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82572EI),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573E),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573E_IAMT),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82573L),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82574L),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82546GB_QUAD_COPPER_KSP3),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_COPPER_DPT),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_SERDES_DPT),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_COPPER_SPT),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_80003ES2LAN_SERDES_SPT),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_UNPROGRAMMED),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I211_UNPROGRAMMED),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I211_COPPER),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_COPPER_FLASHLESS),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_SERDES),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_SERDES_FLASHLESS),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I210_1000BASEKX),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, E1000_I217),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, E1000_I218_LM),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x15A2), /* Ethernet connection(3) I218-LM. */
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, E1000_82577LM),
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, E1000_82579LM),
	/* Terminator of the ID array. */
	{ 0, 0 }
};


/* Global list of E1000 device in system. */
static i825xx_device_t* pNicList = NULL;

/* Spin lock to protect the global data of all NICs. */
#if defined(__CFG_SYS_SMP)
static __SPIN_LOCK hw_spinlock = SPIN_LOCK_INIT_VALUE;
#endif

/* Read data from EEPROM of NIC. */
static uint16_t i825xx_eeprom_read(i825xx_device_t *priv, uint8_t ADDR)
{
	uint16_t DATA = 0;
	uint32_t tmp = 0;

	mmio_write32(i825xx_REG_EERD, (1) | ((uint32_t)(ADDR) << 8));
	while (!((tmp = mmio_read32(i825xx_REG_EERD)) & (1 << 4)))
	{
		__MicroDelay(10);
	}
	DATA = (uint16_t)((tmp >> 16) & 0xFFFF);
	return DATA;
}

/* Set mac type according device id. */
static void set_mac_type(i825xx_device_t* hw)
{
	BUG_ON(NULL == hw);

	switch (hw->device_id) {
	case PCI_DEVICE_ID_INTEL_I210_UNPROGRAMMED:
	case PCI_DEVICE_ID_INTEL_I211_UNPROGRAMMED:
	case PCI_DEVICE_ID_INTEL_I210_COPPER:
	case PCI_DEVICE_ID_INTEL_I211_COPPER:
	case PCI_DEVICE_ID_INTEL_I210_COPPER_FLASHLESS:
	case PCI_DEVICE_ID_INTEL_I210_SERDES:
	case PCI_DEVICE_ID_INTEL_I210_SERDES_FLASHLESS:
	case PCI_DEVICE_ID_INTEL_I210_1000BASEKX:
		hw->mac_type = E1000_IGB;
		break;
	case PCI_DEVICE_ID_INTEL_82574L:
		hw->mac_type = E1000_82574;
		break;
	default:
		/* Should never have loaded on this device */
		hw->mac_type = E1000_UNKNOWN;
	}
}

/*
 * Get the NIC's MAC address.
 * The MAC address is obtained from EEPROM or register.
 */
static void __get_mac_addr(i825xx_device_t* priv, char* mac_address)
{
	uint16_t mac16 = i825xx_eeprom_read(priv, 0);
	mac_address[0] = (mac16 & 0xFF);
	mac_address[1] = (mac16 >> 8) & 0xFF;
	mac16 = i825xx_eeprom_read(priv, 1);
	mac_address[2] = (mac16 & 0xFF);
	mac_address[3] = (mac16 >> 8) & 0xFF;
	mac16 = i825xx_eeprom_read(priv, 2);
	mac_address[4] = (mac16 & 0xFF);
	mac_address[5] = (mac16 >> 8) & 0xFF;
}

/* Get MAC address from RA(receiving address register). */
static void _get_mac_addr(i825xx_device_t* priv, char* mac_address)
{
	uint8_t * mem_base_mac_8 = (uint8_t *)((uint8_t*)priv->mmio_address + 0x5400);
	uint32_t * mem_base_mac_32 = (uint32_t *)((uint8_t*)priv->mmio_address + 0x5400);
	if (mem_base_mac_32[0] != 0)
	{
		for (int i = 0; i < 6; i++)
		{
			mac_address[i] = mem_base_mac_8[i];
		}
	}
}

/****************************************************
** Management Data Input/Output (MDI/O) Interface **
** "This interface allows upper-layer devices to  **
**  monitor and control the state of the PHY."    **
****************************************************/
#define MDIC_PHYADD			(1 << 21)
#define MDIC_OP_WRITE		(1 << 26)
#define MDIC_OP_READ		(2 << 26)
#define MDIC_R				(1 << 28)
#define MDIC_I				(1 << 29)
#define MDIC_E				(1 << 30)

static uint16_t net_i825xx_phy_read(i825xx_device_t *priv, int MDIC_REGADD)
{
	uint16_t MDIC_DATA = 0;

	mmio_write32(i825xx_REG_MDIC, (((MDIC_REGADD & 0x1F) << 16) |
		MDIC_PHYADD | MDIC_I | MDIC_OP_READ));

	while (!(mmio_read32(i825xx_REG_MDIC) & (MDIC_R | MDIC_E)))
	{
		__MicroDelay(1);
	}

	if (mmio_read32(i825xx_REG_MDIC) & MDIC_E)
	{
		_hx_printf("i825xx: MDI READ ERROR\r\n");
		return -1;
	}

	MDIC_DATA = (uint16_t)(mmio_read32(i825xx_REG_MDIC) & 0xFFFF);
	return MDIC_DATA;
}

static void net_i825xx_phy_write(i825xx_device_t *priv, int MDIC_REGADD, uint16_t MDIC_DATA)
{
	mmio_write32(i825xx_REG_MDIC, ((MDIC_DATA & 0xFFFF) | ((MDIC_REGADD & 0x1F) << 16) |
		MDIC_PHYADD | MDIC_I | MDIC_OP_WRITE));

	while (!(mmio_read32(i825xx_REG_MDIC) & (MDIC_R | MDIC_E)))
	{
		__MicroDelay(1);
	}

	if (mmio_read32(i825xx_REG_MDIC) & MDIC_E)
	{
		_hx_printf("i825xx: MDI WRITE ERROR\r\n");
		return;
	}
}

/* Enable rx function of NIC. */
static void net_i825xx_rx_enable(i825xx_device_t* priv)
{
	/* Enable rx. */
	mmio_write32(i825xx_REG_RCTL, mmio_read32(i825xx_REG_RCTL) | (RCTL_EN));
}

/* Initializes the rx function of NIC. */
static int net_i825xx_rx_init(i825xx_device_t* priv)
{
	uint32_t rxd_ctl = 0;

	/* Setup the receive descriptor ring buffer. */
	mmio_write32(i825xx_REG_RDBAH, (uint32_t)((uint64_t)priv->rx_desc_base >> 32));
	mmio_write32(i825xx_REG_RDBAL, (uint32_t)((uint64_t)priv->rx_desc_base & 0xFFFFFFFF));

	/* Receive buffer length; NUM_RX_DESCRIPTORS 16-byte descriptors. */
	mmio_write32(i825xx_REG_RDLEN, (uint32_t)(NUM_RX_DESCRIPTORS * 16));

	/*
	 * Enable descriptor queue if igb.
	 * Some registers,such as RDT and RDH,will
	 * ignore the writting if descriptor queue
	 * is disabled.
	 */
	if (E1000_IGB == priv->mac_type)
	{
		rxd_ctl = mmio_read32(priv->mmio_address + REG_RXDCTL);
		rxd_ctl |= DCTL_ENABLE;
		mmio_write32(priv->mmio_address + REG_RXDCTL, rxd_ctl);
		__MicroDelay(50);
	}

	/* Setup head and tail pointers. */
	if (E1000_IGB == priv->mac_type)
	{
		/* setup head and tail pointers */
		mmio_write32(i825xx_REG_RDH, 0);
		mmio_write32(i825xx_REG_RDT, NUM_RX_DESCRIPTORS);
		priv->rx_tail = 0;
	}
	if (E1000_82574 == priv->mac_type)
	{
		/* setup head and tail pointers */
		mmio_write32(i825xx_REG_RDH, 0);
		mmio_write32(i825xx_REG_RDT, NUM_RX_DESCRIPTORS);
		priv->rx_tail = 0;
	}
	else
	{
		/* setup head and tail pointers */
		mmio_write32(i825xx_REG_RDH, 0);
		mmio_write32(i825xx_REG_RDT, NUM_RX_DESCRIPTORS);
		priv->rx_tail = 0;
	}

	/* Set the receieve control register (promisc ON, 8K pkt size). */
	mmio_write32(i825xx_REG_RCTL, (RCTL_SBP | RCTL_UPE | RCTL_MPE |
		RTCL_RDMTS_HALF | RCTL_SECRC |
		RCTL_LPE | RCTL_BAM | RCTL_BSIZE_2048));
	return 0;
}

/* Initializes the tx function. */
static int net_i825xx_tx_init(i825xx_device_t *priv)
{
	/* setup the transmit descriptor ring buffer. */
	mmio_write32(i825xx_REG_TDBAH, (uint32_t)((uint64_t)priv->tx_desc_base >> 32));
	mmio_write32(i825xx_REG_TDBAL, (uint32_t)((uint64_t)priv->tx_desc_base & 0xFFFFFFFF));

	/* transmit buffer length; NUM_TX_DESCRIPTORS 16-byte descriptors. */
	mmio_write32(i825xx_REG_TDLEN, (uint32_t)(NUM_TX_DESCRIPTORS * 16));

	/* setup head and tail pointers. */
	if (E1000_IGB == priv->mac_type)
	{
		mmio_write32(i825xx_REG_TDH, 0);
		mmio_write32(i825xx_REG_TDT, NUM_TX_DESCRIPTORS);
	}
	if(E1000_82574 == priv->mac_type)
	{
		mmio_write32(i825xx_REG_TDH, 0);
		mmio_write32(i825xx_REG_TDT, NUM_TX_DESCRIPTORS);
	}
	else
	{
		mmio_write32(i825xx_REG_TDH, 0);
		mmio_write32(i825xx_REG_TDT, NUM_TX_DESCRIPTORS);
	}
	priv->tx_tail = 0;
	priv->old_tail = 0;
	priv->txq_full_times = 0;
	priv->tx_drop = 0;
	/* TX queue is empty at very first. */
	priv->bTxQueueEmpty = TRUE;

	/* Programming tx descriptor control reg. */
	if (E1000_IGB == priv->mac_type)
	{
		uint32_t txd_ctl = mmio_read32(priv->mmio_address + REG_TXDCTL);
		txd_ctl |= DCTL_ENABLE;
		mmio_write32(priv->mmio_address + REG_TXDCTL, txd_ctl);
		__MicroDelay(50);
	}
	if (E1000_82574 == priv->mac_type)
	{
		/* According 4.6.6 of datasheet, tx initialization. */
		uint32_t txd_ctl = 0;
		txd_ctl |= (1 << 24);  /* GRAN */
		txd_ctl |= (1 << 16);  /* WTHRESH */
		mmio_write32(priv->mmio_address + REG_TXDCTL, txd_ctl);
		__MicroDelay(50);
	}

	/* set the transmit control register (padshortpackets). */
	if (E1000_82574 == priv->mac_type)
	{
		mmio_write32(i825xx_REG_TCTL, (TCTL_EN | TCTL_PSP));
		/* 
		 * Fuck this line of code! 
		 * It lead about 4 days of my time to trouble shooting,
		 * since it caused the whole chips do not work properly.
		 * The effect is tx not working.
		 */
		//mmio_write32(i825xx_REG_TCTL, 0b0110000000000111111000011111010);

		/* IPGT = 8, IPGR1 = 2, IPGR2 = 10. */
		uint32_t tipg = 0;
		tipg = 8;
		tipg |= (2 << 10);
		tipg |= (10 << 20);
		mmio_write32(i825xx_REG_TIPG, tipg);
	}
	else {
		mmio_write32(i825xx_REG_TCTL, (TCTL_EN | TCTL_PSP));
		mmio_write32(i825xx_REG_TCTL, 0b0110000000000111111000011111010);
		mmio_write32(i825xx_REG_TIPG, 0x0060200A);
	}
	return 0;
}

/* Xfer a packet through the NIC. */
static int net_i825xx_tx_poll(i825xx_device_t *priv, void *pkt, uint16_t length)
{
	int retry_times = 100;

	priv->tx_desc[priv->tx_tail]->addr = (uint64_t)pkt;
	priv->tx_desc[priv->tx_tail]->length = length;
	priv->tx_desc[priv->tx_tail]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS;
	priv->tx_desc[priv->tx_tail]->status = 0;

	/* update the tail so the hardware knows it's ready. */
	int oldtail = priv->tx_tail;
	priv->tx_tail = (priv->tx_tail + 1) % NUM_TX_DESCRIPTORS;
	mmio_write32(i825xx_REG_TDT, priv->tx_tail);
	/* Tx queue is not empty now. */
	priv->bTxQueueEmpty = FALSE;

	while (!(priv->tx_desc[oldtail]->status & 0xF))
	{
		__MicroDelay(1);
		retry_times--;
		if (0 == retry_times)
		{
			/* Give up. */
			_hx_printf("send frame failed.\r\n");
			break;
		}
	}
	return retry_times;
}

/* Check if the tx descriptors queue is empty. */
static BOOL txq_empty(i825xx_device_t* priv)
{
	if (priv->old_tail == priv->tx_tail)
	{
		return TRUE;
	}
	return FALSE;
}

/* Check if the tx descriptors queue is full. */
static BOOL txq_full(i825xx_device_t* priv)
{
	if ((priv->tx_tail + 1) % NUM_TX_DESCRIPTORS == priv->old_tail)
	{
		return TRUE;
	}
	return FALSE;
}

/*
* Submit a tx request to NIC.
* It try to find a free tx descriptor and link the frame
* into it,if found.Otherwise(all tx descriptors are not
* free),it just link the sending buffer into sending list,
* and increment the tx list size.It will be submited to
* transmit in tx interrupt handler,where tx descriptors
* will be released and available.
*/
static BOOL _submit_tx_request(i825xx_device_t* priv, __ETHERNET_BUFFER* pSendingPkt)
{
	BOOL bResult = FALSE;
	unsigned long ulFlags;

	BUG_ON((NULL == priv) || (NULL == pSendingPkt));

	/* Atomic operation. */
	__ENTER_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);
	if (!txq_full(priv))
	{
		/*
		 * There are free tx descriptors,so just use
		 * one and trigger the hardware to send.
		 */
		priv->tx_desc[priv->tx_tail]->addr = (uint64_t)&pSendingPkt->Buffer[0];
		priv->tx_desc[priv->tx_tail]->length = pSendingPkt->act_length;
		priv->tx_desc[priv->tx_tail]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS; // ((1 << 3) | (3));
		priv->tx_desc[priv->tx_tail]->status = 0;

		/* Link the frame into current tx list. */
		if (NULL == priv->pCurrTxHeader)
		{
			/* No frame in tx list yet. */
			priv->pCurrTxHeader = priv->pCurrTxTail = pSendingPkt;
		}
		else
		{
			priv->pCurrTxTail->pNext = pSendingPkt;
			priv->pCurrTxTail = pSendingPkt;
		}
		priv->currtxlist_size++;

		/* Update the tail so the hardware knows it's ready. */
		priv->tx_tail = (priv->tx_tail + 1) % NUM_TX_DESCRIPTORS;
		mmio_write32(i825xx_REG_TDT, priv->tx_tail);
		/* Tx queue is not empty now. */
		priv->bTxQueueEmpty = FALSE;
	}
	else
	{
		/*
		* No free tx descriptor,so just link into tx list.
		* It will be transmited in tx interrupt handler when tx
		* hardware resource is available.
		*/
		if (priv->txlist_size >= MAX_ETH_SENDINGQUEUESZ)
		{
			/* Sending list full,give up. */
			priv->txq_full_times++;
			priv->tx_drop++;
			__LEAVE_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);
			goto __TERMINAL;
		}
		if (NULL == priv->pTxHeader)
		{
			priv->pTxHeader = priv->pTxTail = pSendingPkt;
		}
		else
		{
			priv->pTxTail->pNext = pSendingPkt;
			priv->pTxTail = pSendingPkt;
		}
		priv->txlist_size++;
	}
	__LEAVE_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pSendingPkt)
		{
			/*
			 * May caused by pending tx list full, show a warning.
			 */
			uint32_t tdh = mmio_read32(i825xx_REG_TDH);
			uint32_t tdt = mmio_read32(i825xx_REG_TDT);
			__LOG("%s:Tx queue full[card = %d,tx_tail = %d,old_tail = %d,tdh = %d,tdt = %d].\r\n",
				__func__,
				priv->cardnum,
				priv->tx_tail,
				priv->old_tail,
				tdh, tdt);
		}
		else
		{
			/* Caused by cloning ethernet buffer failure. */
			__LOG("%s:Clone eth buff failed.\r\n", __func__);
		}
	}
	return bResult;
}

/* Handle tx over interrupt,will be invoked in NIC's interrupt handler. */
static void _tx_int_handler(i825xx_device_t* priv)
{
	__ETHERNET_BUFFER* pCurrentPkt = NULL;
	unsigned long ulFlags;
	BOOL bTxDescUpdated = FALSE;

	BUG_ON(NULL == priv);
	__ENTER_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);
	while (!txq_empty(priv))
	{
		if (0 == (priv->tx_desc[priv->old_tail]->status & 0x0F))
		{
			/* No wrote back desc. */
			break;
		}
		/* Current frame send over,unlink and destroy it. */
		pCurrentPkt = (__ETHERNET_BUFFER*)priv->pCurrTxHeader;
		if (NULL == pCurrentPkt)
		{
			/* Unexpected interrupt. */
			__LEAVE_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);
			__LOG("%s:tx interrupt raise but current tx list is empty.\r\n", __func__);
			goto __TERMINAL;
		}
		priv->pCurrTxHeader = pCurrentPkt->pNext;
		if (NULL == priv->pCurrTxHeader)
		{
			/* No sending frame in current list yet. */
			priv->pCurrTxTail = NULL;
		}
		priv->currtxlist_size--;
		/* Destroy the just sent frame. */
		EthernetManager.DestroyEthernetBuffer(pCurrentPkt);
		/* Clear the status bits of tx descriptor. */
		priv->tx_desc[priv->old_tail]->status = 0;
		/* Update the old tail. */
		priv->old_tail = (priv->old_tail + 1) % NUM_TX_DESCRIPTORS;
	}
	/* Trigger a new sending if there is pending frame. */
	while (priv->pTxHeader)
	{
		/* Give up when tx desc queue full. */
		if (txq_full(priv))
		{
			break;
		}
		/* Unlink it from pending tx list. */
		pCurrentPkt = (__ETHERNET_BUFFER*)priv->pTxHeader;
		priv->pTxHeader = pCurrentPkt->pNext;
		if (NULL == priv->pTxHeader)
		{
			priv->pTxTail = NULL;
		}
		priv->txlist_size--;
		/* Reset next pointer,very important. */
		pCurrentPkt->pNext = NULL;

		/* Link it into current sending list. */
		if (NULL == priv->pCurrTxHeader)
		{
			priv->pCurrTxHeader = priv->pCurrTxTail = pCurrentPkt;
		}
		else
		{
			priv->pCurrTxTail->pNext = pCurrentPkt;
			priv->pCurrTxTail = pCurrentPkt;
		}
		priv->currtxlist_size++;

		/* Commit to hardware to send. */
		priv->tx_desc[priv->tx_tail]->addr = (uint64_t)&pCurrentPkt->Buffer[0];
		priv->tx_desc[priv->tx_tail]->length = pCurrentPkt->act_length;
		priv->tx_desc[priv->tx_tail]->cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS; // ((1 << 3) | (3));
		priv->tx_desc[priv->tx_tail]->status = 0;
		/* Update the tail's value accordingly. */
		priv->tx_tail = (priv->tx_tail + 1) % NUM_TX_DESCRIPTORS;
		bTxDescUpdated = TRUE;
	}
	if (bTxDescUpdated)
	{
		/* update the tail so the hardware knows it's ready. */
		mmio_write32(i825xx_REG_TDT, priv->tx_tail);
		/* Tx queue is not empty now. */
		priv->bTxQueueEmpty = FALSE;
	}
	__LEAVE_CRITICAL_SECTION_SMP(priv->spin_lock, ulFlags);

__TERMINAL:
	return;
}

/*
 * Handler for tx queue empty interrupt.
 * Just set the tx queue empty flag to TRUE.
 */
static void _txqe_int_handler(i825xx_device_t* priv)
{
	BUG_ON(NULL == priv);
	priv->bTxQueueEmpty = TRUE;
}

/* Process incoming packet. */
static void __process_rx_pkt(__GENERIC_NETIF* pGenif, unsigned char* packet, int length)
{
	struct pbuf* p = NULL;

	/* Validate parameters. */
	BUG_ON((NULL == packet) || (0 == length));
	if (NULL == pGenif)
	{
		/* The NIC is not registered into system yet. */
		return;
	}
	BUG_ON(NULL == pGenif->genif_input);

	/* 
	 * Create a new pbuf object and save the incoming 
	 * frame into it, then just bounce the pbuf to input
	 * handler of genif.
	 */
	p = pbuf_alloc(PBUF_RAW, (u16_t)length, PBUF_RAM);
	if (NULL == p)
	{
		/* Show a warning. */
		__LOG("out of memory.\r\n");
		return;
	}
	/* Copy the content into pbuf. */
	u8_t* pos = packet;
	for (struct pbuf* q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, pos, q->len);
		pos += q->len;
	}
	if (ERR_OK != pGenif->genif_input(pGenif, p))
	{
		/* Uper could not accept the frame. */
		pbuf_free(p);
	}
	return;
}

/*
 * Poll packet from NIC.
 * This can be used stand-alone or from an interrupt handler.
 */
static void net_i825xx_rx_poll(i825xx_device_t* priv)
{
	while ((priv->rx_desc[priv->rx_tail]->status & (1 << 0)))
	{
		// raw packet and packet length (excluding CRC)
		uint8_t *pkt = (void *)priv->rx_desc[priv->rx_tail]->addr;
		uint16_t pktlen = priv->rx_desc[priv->rx_tail]->length;
		BOOL dropflag = FALSE;

		if (pktlen < 60)
		{
			_hx_printf("net[u]: short packet (%u bytes)\n", priv->cardnum, pktlen);
			dropflag = TRUE;
		}

		// while not technically an error, there is no support in this driver
		if (!(priv->rx_desc[priv->rx_tail]->status & (1 << 1)))
		{
			_hx_printf("net[%u]: no EOP set! (len=%u, 0x%x 0x%x 0x%x)\n",
				priv->cardnum, pktlen, pkt[0], pkt[1], pkt[2]);
			dropflag = TRUE;
		}

		if (priv->rx_desc[priv->rx_tail]->errors)
		{
			_hx_printf("net[%d]: rx errors (0x%x)\n", priv->cardnum, 
				priv->rx_desc[priv->rx_tail]->errors);
			dropflag = TRUE;
		}

		if (!dropflag)
		{
			/* Delivery the frame to OS kernel. */
			__process_rx_pkt(priv->pGenif, pkt, pktlen);
		}

		// update RX counts and the tail pointer
		priv->rx_desc[priv->rx_tail]->status = (uint16_t)(0);
		priv->rx_tail = (priv->rx_tail + 1) % NUM_RX_DESCRIPTORS;

		// write the tail to the device
		mmio_write32(i825xx_REG_RDT, priv->rx_tail);
	}
}

/* Interrupt handler for link status change. */
static void LinkStatusChange(i825xx_device_t* priv)
{
	uint32_t status = 0;
	char* duplex = NULL;
	char* speed = NULL;
	enum __LINK_STATUS _status = down;
	enum __ETHERNET_SPEED _speed = _10M;
	enum __DUPLEX _duplex = half;

	/* Obtain current status register. */
	status = mmio_read32(priv->mmio_address + REG_STATUS);
	if (status & (1 << 1))
	{
		/* Link status is UP. */
		_status = up;

		if (status & (1 << 0))
		{
			duplex = "FULL";
			_duplex = full;
		}
		else
		{
			duplex = "HALF";
			_duplex = half;
		}
		switch ((status >> 6) & 3)
		{
		case 0:
			speed = "10Mbps";
			_speed = _10M;
			break;
		case 1:
			speed = "100Mbps";
			_speed = _100M;
			break;
		case 2:
			speed = "1000Mbps";
			_speed = _1000M;
			break;
		case 3:
			speed = "10Gbps";
			_speed = _10G;
			break;
		}
		/* Link status UP,show physical layer information. */
		__LOG("[e1000_%d]Link status:UP,duplex:%s,speed:%s\r\n",
			priv->cardnum,
			duplex,
			speed);
		/* Notify the ethernet manager. */
		if (priv->pGenif)
		{
			NetworkManager.LinkStatusChange(priv->pGenif, _status, _duplex, _speed);
		}
	}
	else
	{
		/* Link status is down. */
		_status = down;
		if (priv->pGenif)
		{
			NetworkManager.LinkStatusChange(priv->pGenif, _status, _duplex, _speed);
		}
		__LOG("[e1000_%d]Link status down.\r\n", priv->cardnum);
	}
}

/* Interrupt handler of the NIC device. */
static BOOL i825xx_interrupt_handler(LPVOID *ctx, LPVOID pIntParam)
{
	i825xx_device_t* priv = (i825xx_device_t*)pIntParam;
	uint32_t icr = 0;

	BUG_ON(NULL == priv);
	if (NULL == priv->pGenif)
	{
		/* The NIC is not registered into system yet. */
		return FALSE;
	}

	/* Read the pending interrupt status. */
	icr = mmio_read32(priv->mmio_address + 0xC0);
	if (0 == (icr & 0xFFFF))
	{
		/*
		 * No interrupt of this NIC pending,maybe raised
		 * by other NIC or devices with same interrupt line.
		 */
		return FALSE;
	}

	while (icr)
	{
		/* Tx status. */
		if (icr & (1 << 0))
		{
			_tx_int_handler(priv);
			priv->tx_int_num++;
		}
		if (icr & (1 << 1))
		{
			/* Tx queue empty. */
			_txqe_int_handler(priv);
			priv->txqe_int_num++;
		}
		icr &= ~(3);

		/* Link status changed. */
		if (icr & (1 << 2))
		{
			icr &= ~(1 << 2);
			LinkStatusChange(priv);
			/* Write set link up bit. */
			mmio_write32(i825xx_REG_CTRL, (mmio_read32(i825xx_REG_CTRL) | CTRL_SLU));
#if 0
			// debugging info (probably not necessary in most scenarios)
			_hx_printf("i825xx: Link Status Change, STATUS = 0x%p8x\n", mmio_read32(i825xx_REG_STATUS));
			_hx_printf("i825xx: PHY CONTROL = 0x%p4x\n", net_i825xx_phy_read(priv, i825xx_PHYREG_PCTRL));
			_hx_printf("i825xx: PHY STATUS = 0x%p4x\n", net_i825xx_phy_read(priv, i825xx_PHYREG_PSTATUS));
			_hx_printf("i825xx: PHY PSSTAT = 0x%p4x\n", net_i825xx_phy_read(priv, i825xx_PHYREG_PSSTAT));
			_hx_printf("i825xx: PHY ANA = 0x%p4x\n", net_i825xx_phy_read(priv, 4));
			_hx_printf("i825xx: PHY ANE = 0x%p4x\n", net_i825xx_phy_read(priv, 6));
			_hx_printf("i825xx: PHY GCON = 0x%p4x\n", net_i825xx_phy_read(priv, 9));
			_hx_printf("i825xx: PHY GSTATUS = 0x%p4x\n", net_i825xx_phy_read(priv, 10));
			_hx_printf("i825xx: PHY EPSTATUS = 0x%p4x\n", net_i825xx_phy_read(priv, 15));
#endif
		}

		// RX underrun / min threshold
		if (icr & (1 << 6) || icr & (1 << 4))
		{
			icr &= ~((1 << 6) | (1 << 4));

			volatile int i;
			for (i = 0; i < NUM_RX_DESCRIPTORS; i++)
			{
				if (priv->rx_desc[i]->status)
				{
					//_hx_printf(" pending descriptor (i=%u, status=0x%p4x)\n", i, priv->rx_desc[i]->status);
				}
			}
			/* Poll the NIC. */
			net_i825xx_rx_poll(priv);
			priv->ou_int_num++;
		}

		/* Packet is pending. */
		if (icr & (1 << 7))
		{
			icr &= ~(1 << 7);
			net_i825xx_rx_poll(priv);
			priv->rx_int_num++;
		}

		if (icr)
		{
#if 0
			_hx_printf("i825xx[%u]: unhandled interrupt #%u received! (0x%p8x)\n",
				priv->cardnum,
				priv->int_vector,
				icr);
#endif
		}

		/* Check if there is new arrival interrupt,and loop again if so. */
		icr = mmio_read32(priv->mmio_address + 0xC0);
	}
	return TRUE;
}

/* Show e1000e specific information for debugging. */
static void _specific_show_genif(__GENERIC_NETIF* pGenif)
{
	i825xx_device_t* priv = NULL;
	char* mac_info = NULL;

	BUG_ON(NULL == pGenif);
	priv = (i825xx_device_t*)pGenif->pGenifExtension;
	BUG_ON(NULL == priv);
	if (priv->mac_type == E1000_IGB)
	{
		mac_info = "e1000_igb";
	}
	if (priv->mac_type == E1000_82574)
	{
		mac_info = "e1000_82574";
	}
	else
	{
		mac_info = "others";
	}

	//_hx_printf("  rx/tx desc num: %d/%d\r\n", NUM_RX_DESCRIPTORS, NUM_TX_DESCRIPTORS);
	_hx_printf("  rx_tail: %d\r\n", priv->rx_tail);
	_hx_printf("  tx_tail/old_tail: %d/%d\r\n", priv->tx_tail, priv->old_tail);
	_hx_printf("  currtxlist/txlist size: %d/%d\r\n", priv->currtxlist_size, priv->txlist_size);
	_hx_printf("  rx/tx/ou/txqe int num: %d/%d/%d/%d\r\n", priv->rx_int_num, priv->tx_int_num,
		priv->ou_int_num,
		priv->txqe_int_num);
	_hx_printf("  txq full times/empty flag: %d/%d\r\n", priv->txq_full_times, priv->bTxQueueEmpty);
	_hx_printf("  tx drops: %d\r\n", priv->tx_drop);
	_hx_printf("  mac_type: %s\r\n", mac_info);
}

/* Helper routine to write command into device's config space. */
static BOOL pci_write_cmd(__PHYSICAL_DEVICE* pDev, uint32_t cmd);

/* reset function dedicated for 82574. */
static void __82574_reset(i825xx_device_t* priv)
{
	uint32_t reg = 0;

	/* Disable bus master under PCIe. */
	if (!pci_write_cmd(priv->pDev, PCI_COMMAND_MEMORY))
	{
		_hx_printf("%s:failed to disable bus master\r\n", __func__);
	}
	/* Disable all interrupts. */
	mmio_write32(i825xx_REG_IMC, 0xFFFFFFFF);
	/* stop rx/tx. */
	mmio_write32(REG_RCTRL, 0);
	mmio_write32(REG_TCTRL, TCTL_PSP);
	/* Should flush data cache here. */
	/* Wait 10ms,it's enough. */
	__MicroDelay(100);

	/* Write reset bit into ctrl. */
	reg = mmio_read32(i825xx_REG_CTRL);
	reg |= CTRL_RST;
	mmio_write32(i825xx_REG_CTRL, reg);
	/* Wait a short while to complate the reset. */
	__MicroDelay(1000);

	/* Clear interrupt again. */
	mmio_write32(i825xx_REG_IMC, 0xFFFFFFFF);
	reg = mmio_read32(priv->mmio_address + 0x0C);
	/* Wait a short while,1ms. */
	__MicroDelay(1000);

	/* Enable bus master again. */
	if (!pci_write_cmd(priv->pDev, PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER))
	{
		_hx_printf("%s:failed to enable bus master\r\n", __func__);
	}
}

/* Reset the specified e1000 NIC. */
static void ResetNic(i825xx_device_t* priv)
{
	uint32_t reg = 0;

	/* Performance dedicated reset. */
	if (E1000_82574 == priv->mac_type)
	{
		__82574_reset(priv);
		return;
	}

	/* Generic procedure of reset. */
	/* Disable all interrupts. */
	mmio_write32(i825xx_REG_IMC, 0xFFFFFFFF);
	/* Write reset bit into ctrl. */
	reg = mmio_read32(i825xx_REG_CTRL);
	reg |= CTRL_RST;
	mmio_write32(i825xx_REG_CTRL, reg);
	/* Wait a short while to complate the reset. */
	__MicroDelay(1000);

	/* Clear interrupt again. */
	mmio_write32(i825xx_REG_IMC, 0xFFFFFFFF);
	reg = mmio_read32(priv->mmio_address + 0x0C);
	/* Wait a short while,1ms. */
	__MicroDelay(1000);
}

/* Send frame handler of genif. */
static int e1000ex_genif_output(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	i825xx_device_t* priv = NULL;
	__ETHERNET_BUFFER* pEthBuff = NULL;
	int ret = ERR_UNKNOWN;

	BUG_ON((NULL == pGenif) || (NULL == pkt));
	priv = (i825xx_device_t*)pGenif->pGenifExtension;
	BUG_ON(NULL == priv);

	/* Apply callback first. */
	genif_apply_callback(pGenif, pkt);

	/*
	 * Create a new ethernet buffer object, copy the pkt
	 * into it,and submit the tx request by invoking _submit_tx_request
	 * routine.
	 */
	pEthBuff = EthernetManager.CreateEthernetBuffer(pkt->tot_len, 0);
	if (NULL == pEthBuff)
	{
		ret = ERR_MEM;
		goto __TERMINAL;
	}

	/* Convert the pbuf to ethernet buffer. */
	if (!EthernetManager.pbuf_to_ebuf(pkt, pEthBuff))
	{
		ret = ERR_BUF;
		goto __TERMINAL;
	}
	
	/* Submit to transmit. */
	if (!_submit_tx_request(priv, pEthBuff))
	{
		/* May caused by the fulling of tx queue. */
		EthernetManager.DestroyEthernetBuffer(pEthBuff);
		ret = ERR_IF;
		goto __TERMINAL;
	}

	/* Send OK, update the stat counters accordingly. */
	pGenif->stat.tx_pkt += 1;
	pGenif->stat.tx_success += 1;
	pGenif->stat.tx_bytes += pkt->tot_len;

	ret = ERR_OK;

__TERMINAL:
	return ret;
}

/* Initializes hardware bits,according mac type. */
static void initializes_hw_bits(i825xx_device_t* priv)
{
	uint32_t ctrl_reg = 0;

	switch (priv->mac_type)
	{
	case E1000_82574:
		_hx_printf("[82574]:init hw bits\r\n");
		/* tx descriptor 0. */
		ctrl_reg = mmio_read32(REG_TXDCTL);
		ctrl_reg |= (1 << 22);
		mmio_write32(REG_TXDCTL, ctrl_reg);

		/* tx arbitration control 0. */
		ctrl_reg = mmio_read32(REG_TARC);
		ctrl_reg &= ~(0xF << 27);
		ctrl_reg |= (1 << 26);
		ctrl_reg |= (1 << 10); /* Enable txd?? */
		mmio_write32(REG_TARC, ctrl_reg);

		/* 
		 * tx byte count fix,did not found in datasheet but in 
		 * refer source code.
		 */
		ctrl_reg = mmio_read32(i825xx_REG_CTRL);
		ctrl_reg &= ~(1 << 29);
		mmio_write32(i825xx_REG_CTRL, ctrl_reg);
		
		/* Ext ctrl register. */
		ctrl_reg = mmio_read32(REG_CTRL_EXT);
		ctrl_reg &= ~(1 << 23);
		ctrl_reg |= (1 << 22);
		mmio_write32(REG_CTRL_EXT, ctrl_reg);

		/* GCR control. */
		ctrl_reg = mmio_read32(REG_GCR);
		ctrl_reg |= (1 << 22);
		ctrl_reg |= (1 << 27);
		mmio_write32(REG_GCR, ctrl_reg);

		/* GCR2 control,fixes some errors in tx. */
		ctrl_reg = mmio_read32(REG_GCR2);
		ctrl_reg |= 1;
		mmio_write32(REG_GCR2, ctrl_reg);
		break;
	default:
		break;
	}
}

/* Initializes a new found E1000 NIC device. */
static BOOL _Init_E1000E(i825xx_device_t* priv)
{
	unsigned char mac_address[10];
	char if_name[32];
	uint32_t ims = 0;
	__GENERIC_NETIF* pGenif = NULL;

	BUG_ON(NULL == priv);

	/* Reset NIC. */
	ResetNic(priv);

	/* Initializes hardware bits. */
	initializes_hw_bits(priv);

	/* Initialize the Multicase Table Array. */
	for (int i = 0; i < 128; i++)
	{
		mmio_write32(i825xx_REG_MTA + (i * 4), 0);
	}

	/* Initialize rx and tx. */
	net_i825xx_rx_init(priv);
	net_i825xx_tx_init(priv);

	/* Disable interrupt throttle mechanism. */
	mmio_write32(i825xx_REG_ITR, 0x00);

#if 0
	/* Enable all interrupts (and clear existing pending ones). */
	ims = IMS_TXDW | IMS_LSC | IMS_RXDMT0 | IMS_RXDW | IMS_RXO | IMS_RXSEQ;
	mmio_write32(i825xx_REG_IMS, ims);
	mmio_read32(priv->mmio_address + 0xC0);

	/* Set the link up. */
	mmio_write32(i825xx_REG_CTRL, (mmio_read32(i825xx_REG_CTRL) | CTRL_SLU));
	/* Enable rx. */
	net_i825xx_rx_enable(priv);
#endif

	/* Register the NIC to network subsystem. */
	_get_mac_addr(priv, mac_address);
	/* Create and register a corresponding genif. */
	pGenif = NetworkManager.CreateGenif(NULL, NULL);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]:No genif created.\r\n", __func__);
		return FALSE;
	}
	_hx_sprintf(if_name, "g1000_if_%d", priv->cardnum);
	strcpy(pGenif->genif_name, if_name);
	memcpy(pGenif->genif_ha, mac_address, ETH_MAC_LEN);
	pGenif->ha_len = ETH_MAC_LEN;
	//pGenif->genif_input = e1000ex_genif_input;
	pGenif->genif_input = general_ethernet_input;
	pGenif->genif_l2_output = e1000ex_genif_output;
	/* 
	 * Use default IP output routine, since this is 
	 * a common network interface.
	 */
	pGenif->genif_l3_output = general_ip_output;
	pGenif->link_type = lt_ethernet;
	pGenif->pGenifExtension = priv;
	pGenif->genif_mtu = ETH_DEFAULT_MTU;
	if (!NetworkManager.RegisterGenif(pGenif))
	{
		_hx_printf("[%s]:register genif fail.\r\n", __func__);
		/* 
		 * Just release the genif's memory,since 
		 * it's not registered into system. 
		 */
		_hx_free(pGenif);
		return FALSE;
	}
	/* Save the genif to device object. */
	priv->pGenif = pGenif;
	
	/* Show a message. */
	_hx_printf("Add interface[name:%s,mac:%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] into system.\r\n",
		if_name,
		mac_address[0],
		mac_address[1],
		mac_address[2],
		mac_address[3],
		mac_address[4],
		mac_address[5]);

	/* Enable all interrupts (and clear existing pending ones). */
	ims = IMS_TXDW | IMS_TXQE | IMS_LSC | IMS_RXDMT0 | IMS_RXDW | IMS_RXO | IMS_RXSEQ;
	mmio_write32(i825xx_REG_IMS, ims);
	mmio_read32(priv->mmio_address + 0xC0);

	/* Set the link up. */
	mmio_write32(i825xx_REG_CTRL, (mmio_read32(i825xx_REG_CTRL) | CTRL_SLU));
	/* Link status changed. */
	LinkStatusChange(priv);

	/* Enable rx. */
	net_i825xx_rx_enable(priv);

	if (priv->pGenif)
	{
		priv->pGenif->specific_show = _specific_show_genif;
	}

	return TRUE;
}

/* Helper routine to write command into device's config space. */
static BOOL pci_write_cmd(__PHYSICAL_DEVICE* pDev, uint32_t cmd)
{
	uint32_t val = 0;

	pDev->WriteDeviceConfig(pDev, PCI_COMMAND, cmd, sizeof(uint16_t));
	/* Check if writting success. */
	val = pDev->ReadDeviceConfig(pDev, PCI_COMMAND, sizeof(uint32_t));
	if (cmd == (val & cmd))
	{
		return TRUE;
	}
	return FALSE;
}

/*
* Check if there is PCI device in system that matches
* the given ID,and initialize it if found.
*/
static BOOL _Check_E1000E_Nic(struct pci_device_id* id)
{
	static int cardnum = 0;
	__PHYSICAL_DEVICE* pDev = NULL;
	__IDENTIFIER devId;
	i825xx_device_t* priv = NULL;
	__U16 iobase_s = 0, iobase_e = 0;
	LPVOID memAddr_s = NULL, memAddr_e = NULL;
	unsigned int mem_size = 0;
	int intVector = 0, index = 0;
	BOOL bResult = FALSE;
	unsigned long ulFlags;

	//Set device ID accordingly.
	devId.dwBusType = BUS_TYPE_PCI;
	devId.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_VENDOR | PCI_IDENTIFIER_MASK_DEVICE;
	devId.Bus_ID.PCI_Identifier.wVendor = id->vendor_id;
	devId.Bus_ID.PCI_Identifier.wDevice = id->device_id;

	//Try to fetch the physical device with the specified ID.
	pDev = GetDevice(&devId, NULL);
	if (NULL == pDev)
	{
		goto __TERMINAL;
	}

	/*
	 * Got a valid device,retrieve it's hardware configuration 
	 * resource and save them into private structure.
	 */
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
				iobase_s = pDev->Resource[index].Dev_Res.IOPort.wStartPort;
				iobase_e = pDev->Resource[index].Dev_Res.IOPort.wEndPort;
			}
			if (pDev->Resource[index].dwResType == RESOURCE_TYPE_MEMORY)
			{
				if (memAddr_s != NULL)
				{
					/*
					 * Already set,only support one memory maped
					 * I/O memory range currently,so just show out a
					 * warning and skip it.
					 */
					_hx_printf("e1000: I/O maped mem[0x%X-0x%X] skiped.\r\n",
						pDev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr,
						pDev->Resource[index].Dev_Res.MemoryRegion.lpEndAddr);
					continue;
				}
				memAddr_s = pDev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr;
				memAddr_e = pDev->Resource[index].Dev_Res.MemoryRegion.lpEndAddr;
				mem_size = (unsigned int)memAddr_e - (unsigned int)memAddr_s + 1;
			}
		}
		//Check if the device with valid resource.
		if ((0 == intVector) && (0 == iobase_s) && (NULL == memAddr_s))
		{
			_hx_printf("%s: e1000 NIC found but no valid resource.\r\n", __func__);
			goto __TERMINAL;
		}

#if 0
		/* Enable memory I/O and bus master. */
		uint32_t val = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
		pDev->WriteDeviceConfig(pDev, PCI_COMMAND, val, sizeof(uint16_t));
		/* Check if writting success. */
		val = pDev->ReadDeviceConfig(pDev, PCI_COMMAND, sizeof(uint32_t));
		if (!(val & PCI_COMMAND_MEMORY))
		{
			_hx_printf("Can not enable memory I/O.\r\n");
			goto __TERMINAL;
		}
		if (!(val & PCI_COMMAND_MASTER))
		{
			_hx_printf("Can not enable bus master.\r\n");
			goto __TERMINAL;
		}
#endif
		/* Enable bus master and memory I/O of the NIC. */
		if (!pci_write_cmd(pDev, PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER))
		{
			_hx_printf("[%s]:Enable master fail.\r\n", __func__);
			goto __TERMINAL;
		}

		/* Remap I/O memory to virtual space. */
		if (VirtualAlloc(memAddr_s, mem_size,
			VIRTUAL_AREA_ALLOCATE_IO,
			VIRTUAL_AREA_ACCESS_RW,
			"e1000") != memAddr_s)
		{
			_hx_printf("[%s]: remap I/O memory failed.\r\n", __func__);
			goto __TERMINAL;
		}

		//Now allocate a device specific structure and fill the resource.
		priv = (i825xx_device_t*)_hx_malloc(sizeof(i825xx_device_t));
		if (NULL == priv)
		{
			_hx_printf("[%s]:out of memory.\r\n", __func__);
			goto __TERMINAL;
		}
		memset(priv, 0, sizeof(i825xx_device_t));

		/* Initializes the spin lock under SMP configuration. */
#if defined(__CFG_SYS_SMP)
		__INIT_SPIN_LOCK(priv->spin_lock, "e1k");
#endif

		/* Create memory space for tx/rx descriptors. */
		priv->tx_desc_base = (uint8_t*)_hx_aligned_malloc(
			NUM_TX_DESCRIPTORS * sizeof(struct e1000_tx_desc), 
			DESCRIPTOR_BASE_ALIGN);
		if (NULL == priv->tx_desc_base)
		{
			goto __TERMINAL;
		}
		memset((void*)priv->tx_desc_base, 0, 
			NUM_TX_DESCRIPTORS * sizeof(struct e1000_tx_desc));
		priv->rx_desc_base = (uint8_t*)_hx_aligned_malloc(
			NUM_RX_DESCRIPTORS * sizeof(struct e1000_rx_desc), 
			DESCRIPTOR_BASE_ALIGN);
		if (NULL == priv->rx_desc_base)
		{
			goto __TERMINAL;
		}
		memset((void*)priv->rx_desc_base, 0, 
			NUM_RX_DESCRIPTORS * sizeof(struct e1000_rx_desc));
		/* Initialize tx/rx descriptor ptr arrays. */
		for (int i = 0; i < NUM_RX_DESCRIPTORS; i++)
		{
			priv->rx_desc[i] = (struct e1000_rx_desc *)(priv->rx_desc_base + (i * 16));
			priv->rx_desc[i]->addr = (uint64_t)_hx_aligned_malloc(8192, DESCRIPTOR_BASE_ALIGN);
			if (0 == priv->rx_desc[i]->addr)
			{
				goto __TERMINAL;
			}
			priv->rx_desc[i]->status = 0;
		}
		for (int i = 0; i < NUM_TX_DESCRIPTORS; i++)
		{
			priv->tx_desc[i] = (struct e1000_tx_desc *)(priv->tx_desc_base + (i * 16));
			priv->tx_desc[i]->addr = 0;
			priv->tx_desc[i]->cmd = 0;
			//priv->tx_desc[i]->status = TSTA_DD;
		}

		priv->pDev = pDev;
		priv->ioport_s = iobase_s;
		priv->ioport_e = iobase_e;
		priv->memaddr_s = memAddr_s;
		priv->memaddr_e = memAddr_e;
		priv->mmio_address = (uintptr_t)memAddr_s;
		priv->int_vector = intVector;
		priv->cardnum = cardnum++;
		/* Save PCI id. */
		priv->device_id = id->device_id;
		priv->vendor_id = id->vendor_id;

		/* Set MAC type according device id. */
		set_mac_type(priv);

		/* Install the corresponding interrupt handler. */
		priv->hInterrupt = ConnectInterrupt(
			"int_i825xx",
			i825xx_interrupt_handler,
			priv,
			intVector + INTERRUPT_VECTOR_BASE);
		if (NULL == priv->hInterrupt)
		{
			goto __TERMINAL;
		}

		/*
		 * Do low level initialization of the found NIC,add it into
		 * global list in case of success.
		 */
		if (_Init_E1000E(priv))
		{
			__ENTER_CRITICAL_SECTION_SMP(hw_spinlock, ulFlags);
			priv->pNext = pNicList;
			pNicList = priv;
			__LEAVE_CRITICAL_SECTION_SMP(hw_spinlock, ulFlags);
			bResult = TRUE;
		}
		else
		{
			goto __TERMINAL;
		}

#if 0
		_hx_printf("New E1000 device found[int = %d,iobase = 0x%X,i/o mem = 0x%X,size = %d].\r\n",
			intVector,
			iobase_s,
			memAddr_s,
			mem_size);
#endif

		/* Try to fetch another NIC device. */
		pDev = GetDevice(&devId, pDev);
		/*
		 * Local variables must be reset since
		 * they will be reused, otherwise will
		 * cause issue,if there are 2 or more
		 * NICs with same chip.
		 */
		memAddr_s = memAddr_e = NULL;
		intVector = 0;
		iobase_s = iobase_e = 0;
	}

__TERMINAL:
	if (!bResult)
	{
		/* Release maped I/O memory first. */
		if (memAddr_s)
		{
			VirtualFree(memAddr_s);
		}
		if (priv)
		{
			/* Should release all resource(memory) allocated yet. */
		}
	}
	return bResult;
}

/*
 * Enumerates all E1000E NIC in system,initialize and
 * start it for each one.
 */
static BOOL Enumerate_NICs()
{
	struct pci_device_id* id = &e1000_supported[0];
	int index = 0;
	BOOL bResult = FALSE;

	while (id->device_id && id->vendor_id)
	{
		if (_Check_E1000E_Nic(id))
		{
			bResult = TRUE;
		}
		/* Check next supported ID. */
		index++;
		id = &e1000_supported[index];
	}
	return bResult;
}

/*
* Entry point of E1000E NIC driver.
* It will be called by ethernet manager in process of system
* initialization.
*/
BOOL E1000EX_Drv_Initialize(LPVOID pData)
{
	/* Initialize spin lock first. */
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(hw_spinlock, "e1k_hw");
#endif
	/* Enumerate all NICs in system. */
	return Enumerate_NICs();
}
