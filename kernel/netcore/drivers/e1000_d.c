//***********************************************************************/
//    Author                    : Garry
//    Original Date             : May 21,2017
//    Module Name               : e1000_d.c
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

#include <kapi.h>
#include <pci_drv.h>
#include <pci_ids.h>
#include <align.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <io.h>

#include <hx_inet.h>
#include <ethmgr.h>
#include "e1000.h"
#include "e1000_d.h"

/* Device ID array that the device driver can support. */
static struct pci_device_id e1000_supported[] = {
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x15A2),
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
	/* Terminator of the ID array. */
	{ 0, 0 }
};

/* Global list to save all e1000 nics in system. */
static struct e1000_hw* e1000_hw_list = NULL;
#if defined(__CFG_SYS_SMP)
/* Spin lock to protect the hw list. */
static __SPIN_LOCK hw_spinlock = SPIN_LOCK_INIT_VALUE;
#endif

/*
* Simulation routines of PCI operation under Linux.
*/
void pci_read_config_byte(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned char* ret)
{
	*ret = (unsigned char)pdev->ReadDeviceConfig(pdev, dwOffset, sizeof(unsigned char));
}

void pci_read_config_word(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned short* ret)
{
	*ret = (unsigned short)pdev->ReadDeviceConfig(pdev, dwOffset, sizeof(unsigned short));
}

void pci_read_config_dword(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned long* ret)
{
	*ret = pdev->ReadDeviceConfig(pdev, dwOffset, sizeof(unsigned long));
}

void pci_write_config_byte(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned char val)
{
	pdev->WriteDeviceConfig(pdev, dwOffset, val, sizeof(unsigned char));
}

void pci_write_config_word(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned short val)
{
	pdev->WriteDeviceConfig(pdev, dwOffset, val, sizeof(unsigned short));
}

void pci_write_config_dword(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, unsigned long val)
{
	pdev->WriteDeviceConfig(pdev, dwOffset, val, sizeof(unsigned long));
}

/* Just return the base address of device configure space. */
void* pci_map_bar(__PHYSICAL_DEVICE* pdev, DWORD dwOffset, DWORD dwFlags)
{
	int index;
	void* memaddr_start = NULL;
	void* memaddr_end = NULL;
	void* vir_mem = NULL;
	
	BUG_ON(dwOffset != PCI_CONFIG_OFFSET_BASE1);

	for (index = 0; index < MAX_RESOURCE_NUM; index++)
	{
		if (pdev->Resource[index].dwResType == RESOURCE_TYPE_MEMORY)
		{
			memaddr_start = pdev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr;
			memaddr_end = pdev->Resource[index].Dev_Res.MemoryRegion.lpEndAddr;
			break;
		}
	}
	BUG_ON(NULL == memaddr_start);
	BUG_ON(NULL == memaddr_end);

	/*
	 * Map the PCI config address space to memory.
	 */
	vir_mem = memaddr_start;
#ifdef __CFG_SYS_VMM
	vir_mem = VirtualAlloc(memaddr_start,
		(char*)memaddr_end - (char*)memaddr_start,
		VIRTUAL_AREA_ALLOCATE_IO,
		VIRTUAL_AREA_ACCESS_RW,
		"E1000_VIR");
	if (NULL == vir_mem)
	{
		_hx_printf("%s:map PCI addr[start = 0x%X,end = 0x%X] failed.\r\n", __func__,
			memaddr_start, memaddr_end);
	}
	else
	{
		BUG_ON(vir_mem != memaddr_start);
	}
#endif
	return vir_mem;
}

/* 
 * Initializer of the ethernet interface,it will be called by 
 * HelloX's ethernet framework.
 * No need to implement it if no customized requirement.
 */
static BOOL Ethernet_Int_Init(__ETHERNET_INTERFACE* pInt)
{
	return TRUE;
}

/*
 * Control functions of the ethernet interface.Some special operations,such as
 * scaninn or association in WLAN,should be implemented in this routine,since they
 * are not common operations for Ethernet.
 */
static BOOL Ethernet_Ctrl(__ETHERNET_INTERFACE* pInt, DWORD dwOperation, LPVOID pData)
{
	return TRUE;
}

/* Transmition worker routine of NIC. */
extern int _e1000_transmit(struct e1000_hw *hw, void *txpacket, int length);
/* Receiving worker routine of NIC. */
extern int e1000_poll(__ETHERNET_INTERFACE* nic);

/* Process incoming packet. */
void net_process_received_packet(__ETHERNET_INTERFACE* pEthInt, unsigned char* packet, int length)
{
	__ETHERNET_BUFFER* pEthBuff = NULL;

	pEthBuff = EthernetManager.CreateEthernetBuffer(length);
	if (NULL == pEthBuff)
	{
		_hx_printf("  %s:failed to create eth_buff.\r\n", __func__);
		return;
	}
	else
	{
		/* Copy the received data to Ethernet Buffer object. */
		memcpy(pEthBuff->Buffer, packet, length);
		pEthBuff->act_length = length;
		//Fill the MAC addresses and packet types.
		memcpy(pEthBuff->dstMAC, packet, ETH_MAC_LEN);
		packet += ETH_MAC_LEN;
		memcpy(pEthBuff->srcMAC, packet, ETH_MAC_LEN);
		packet += ETH_MAC_LEN;
		pEthBuff->frame_type = _hx_ntohs(*(__u16*)packet);
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

/* Dedicated receiving thread for e1000 NIC. */
static DWORD e1000_recv_thread(void* pNic)
{
	__ETHERNET_INTERFACE* pInt = (__ETHERNET_INTERFACE*)pNic;

	while (TRUE)
	{
		e1000_poll(pInt);
	}
	return 0;
}

/*
 * Send a ethernet frame out through ethernet interface.
 * The frame's content is in pInt's sending buffer.
 */
static BOOL Ethernet_SendFrame(__ETHERNET_INTERFACE* pInt)
{
	BOOL          bResult = FALSE;
	struct e1000_hw* dev = NULL;
	__ETHERNET_BUFFER* pEthBuff = NULL;

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
	if (0 == _e1000_transmit(dev, pEthBuff->Buffer, pEthBuff->act_length))
	{
		goto __TERMINAL;
	}
	pInt->ifState.dwFrameSendSuccess += 1;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
 * Initialize a new found E1000 nic.
 */
static BOOL _Init_E1000(struct e1000_hw* hw)
{
	char macAddr[6];
	int ret = -1;
	static unsigned int if_idx = 0;
	char if_name[16];
	__ETHERNET_INTERFACE* pInt = NULL;
	BOOL bResult = FALSE;
	HANDLE pRecvThread = NULL;

	/* 
	 * First phase initialization. macAddr will contain the 
	 * NIC's hardware address after return with success.
	 */
	ret = e1000_init_one(hw, hw->cardnum, hw->pdev, macAddr);
	if (ret)
	{
		/* Failed,give up. */
		goto __TERMINAL;
	}
	/* More initial works. */
	ret = _e1000_init(hw, macAddr);
	if (ret < 0)
	{
		goto __TERMINAL;
	}

	/* Register the NIC into system. */
	_hx_sprintf(if_name, "e1000_if_%d", if_idx++);
	pInt = EthernetManager.AddEthernetInterface(
		if_name,
		macAddr,
		(void*)hw,
		Ethernet_Int_Init,
		Ethernet_SendFrame,
		NULL,
		Ethernet_Ctrl);
	if (NULL == pInt)
	{
		_hx_printf("Failed to register e1000 NIC.\r\n");
		goto __TERMINAL;
	}
	/* Show a message. */
	_hx_printf("Added eth-if[name:%s,MAC:%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] into system.\r\n",
		if_name,
		macAddr[0],
		macAddr[1],
		macAddr[2],
		macAddr[3],
		macAddr[4],
		macAddr[5]);
	/* Save to hw specific structure. */
	hw->nic = pInt;

	/* Create the corresponding receiving thread. */
	unsigned int nAffinity = 0;
	pRecvThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_SUSPENDED,
		PRIORITY_LEVEL_NORMAL,
		e1000_recv_thread,
		(void*)pInt,
		NULL,
		if_name);
	if (NULL == pRecvThread)
	{
		_hx_printk("Failed to create recv thread.\r\n");
		goto __TERMINAL;
	}
	/* Get a CPU to schedule DHCP server thread to. */
	nAffinity = GetScheduleCPU();
	ChangeAffinity(pRecvThread, nAffinity);
	/* Resume the kernel thread to ready to run. */
	ResumeKernelThread(pRecvThread);

	bResult = TRUE;
__TERMINAL:
	return bResult;
}

/*
 * Check if there is a PCI device in system that matches
 * the given ID,and initialize it if found.
 */
static BOOL _Check_E1000_Nic(struct pci_device_id* id)
{
	__PHYSICAL_DEVICE*    pDev = NULL;
	__IDENTIFIER          devId;
	struct e1000_hw*      priv = NULL;
	__U16                 iobase_s = 0, iobase_e = 0;
	LPVOID                memAddr_s = NULL;
	LPVOID                memAddr_e = NULL;
	int                   intVector = 0;
	int                   index = 0;
	BOOL                  bResult = FALSE;
	int                   cardnum = 0;
	unsigned long ulFlags;

	//Set device ID accordingly.
	devId.dwBusType = BUS_TYPE_PCI;
	devId.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_VENDOR | PCI_IDENTIFIER_MASK_DEVICE;
	devId.Bus_ID.PCI_Identifier.wVendor = id->vendor_id;
	devId.Bus_ID.PCI_Identifier.wDevice = id->device_id;

	//Try to fetch the physical device with the specified ID.
	pDev = GetDevice(&devId,NULL);
	if (NULL == pDev)
	{
		goto __TERMINAL;
	}

	/*
	 * Got a valid device,retrieve it's hardware resource and save them into 
	 * private structure.
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
				memAddr_s = pDev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr;
				memAddr_e = pDev->Resource[index].Dev_Res.MemoryRegion.lpEndAddr;
			}
		}
		//Check if the device with valid resource.
		if ((0 == intVector) && (0 == iobase_s) && (NULL == memAddr_s))
		{
			DEBUGOUT("%s: Find a device without valid resource.\r\n", __func__);
			goto __TERMINAL;
		}

		//Now allocate a rtl8111_priv_t structure and fill the resource.
		priv = (struct e1000_hw*)_hx_malloc(sizeof(struct e1000_hw));
		if (NULL == priv)
		{
			DEBUGOUT("%s:allocate e1000 private structure failed.\r\n", __func__);
			goto __TERMINAL;
		}
		memset(priv, 0, sizeof(struct e1000_hw));

		/* Create tx/rx descriptors and packet buffer. */
		priv->tx_base = (struct e1000_tx_desc*)_hx_aligned_malloc(
			TX_DESC_NUM * sizeof(struct e1000_tx_desc), E1000_BUFFER_ALIGN);
		if (NULL == priv->tx_base)
		{
			goto __TERMINAL;
		}
		priv->rx_base = (struct e1000_rx_desc*)_hx_aligned_malloc(
			RX_DESC_NUM * sizeof(struct e1000_rx_desc), E1000_BUFFER_ALIGN);
		if (NULL == priv->rx_base)
		{
			goto __TERMINAL;
		}
		priv->packet = (unsigned char*)_hx_aligned_malloc(PACKET_BUFFER_SIZE,
			E1000_BUFFER_ALIGN);
		if (NULL == priv->packet)
		{
			goto __TERMINAL;
		}

		priv->pdev = pDev;
		priv->ioport_s = iobase_s;
		priv->ioport_e = iobase_e;
		priv->memaddr_s = memAddr_s;
		priv->memaddr_e = memAddr_e;
		priv->int_vector = intVector;
		priv->cardnum = cardnum++;
		/* Save PCI id. */
		priv->device_id = id->device_id;
		priv->vendor_id = id->vendor_id;

		if (_Init_E1000(priv))
		{
			//Link the private object into global list.
			__ENTER_CRITICAL_SECTION_SMP(hw_spinlock, ulFlags);
			priv->pNext = e1000_hw_list;
			e1000_hw_list = priv;
			__LEAVE_CRITICAL_SECTION_SMP(hw_spinlock, ulFlags);
			bResult = TRUE;
		}
		else
		{
			goto __TERMINAL;
		}

		DEBUGOUT("%s: Get a E1000 NIC device[int = %d,iobase = 0x%X,mem = 0x%X].\r\n",
			__func__,
			intVector,
			iobase_s,
			memAddr_s);

		//Try to fetch another NIC device.
		pDev = GetDevice(&devId, pDev);
	}

__TERMINAL:
	if (!bResult)
	{
		if (priv)
		{
			if (priv->tx_base)
			{
				_hx_free(priv->tx_base);
			}
			if (priv->rx_base)
			{
				_hx_free(priv->rx_base);
			}
			if (priv->packet)
			{
				_hx_free(priv->packet);
			}
			_hx_free(priv);
		}
	}
	return bResult;
}

/* 
 * Enumerate all e1000 compatiable NICs in system.
 * It just calls _Check_E1000_Nic by supply a PCI id.
 */
static BOOL Enumerate_E1000()
{
	struct pci_device_id* id = &e1000_supported[0];
	int index = 0;
	BOOL bResult = FALSE;

	while (id->device_id && id->vendor_id)
	{
		if (_Check_E1000_Nic(id))
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
 * Entry point of E1000 NIC driver.
 * It will be called by ethernet manager in process of system
 * initialization.
 */
BOOL E1000_Drv_Initialize(LPVOID pData)
{
	/* Initialize spin lock first. */
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(hw_spinlock, "e1k_hw");
#endif
	/* Enumerate all NICs in system. */
	return Enumerate_E1000();
}
