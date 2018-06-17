//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Apri 8,2017
//    Module Name               : r8152_e.c
//    Module Funciton           : 
//                                Source of R8152 chipset,only the adaptation
//                                routines to HelloX are put into this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <align.h>
#include <byteord.h>
#include <hx_inet.h>
#include <mii.h>
#include <ethmgr.h>

#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "usbasync.h"
#include "usb_ether.h"
#include "r8152.h"
#include "r8152_e.h"

/* Macors used in this module. */
#define BIT(x) (1 << x)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
* Vendor ID and product ID.
*/
struct r8152_dongle {
	unsigned short vendor;
	unsigned short product;
};

static const struct r8152_dongle r8152_dongles[] = {
	/* Realtek */
	{ 0x0bda, 0x8050 },
	{ 0x0bda, 0x8152 },
	{ 0x0bda, 0x8153 },

	/* Samsung */
	{ 0x04e8, 0xa101 },

	/* Lenovo */
	{ 0x17ef, 0x304f },
	{ 0x17ef, 0x3052 },
	{ 0x17ef, 0x3054 },
	{ 0x17ef, 0x3057 },
	{ 0x17ef, 0x7205 },
	{ 0x17ef, 0x720a },
	{ 0x17ef, 0x720b },
	{ 0x17ef, 0x720c },

	/* TP-LINK */
	{ 0x2357, 0x0601 },

	/* Nvidia */
	{ 0x0955, 0x09ff },
};

/*
* USB Ethernet management data structure list,each element in this
* list corresponds one USB ethernet device.
* A new element will be allocated and added into this list in case
* of a new USB ethernet device is scaned.
*/
static struct ueth_data* global_list = NULL;

/* Helper macro to align sending buffer. */
#define __R8152_TX_ALIGN(base) __ALIGN((unsigned long)base, TX_ALIGN)

/*
* Local helper routine to pack several ethernet buffer objects into
* one buffer.
*/
static BOOL __R8152_Pack_Ethernet_Buffer(char* pSendingBuff, __ETHERNET_BUFFER* pEthBuff,
	int offset, int* data_len)
{
	struct tx_desc* pTxDesc = NULL;
	int remain = RTL8152_AGG_BUF_SZ;
	char* base = pSendingBuff + offset;
	char* data = (char*)__R8152_TX_ALIGN(base);
	BOOL bResult = FALSE;

	/* Validate the ethernet buffer object. */
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pEthBuff->dwSignature);

	int length = pEthBuff->act_length; /* Data length. */
	remain -= offset;
	remain -= sizeof(struct tx_desc);
	remain -= (data - base);
	if (remain < length) /* Insufficient buffer. */
	{
		goto __TERMINAL;
	}
	if (length > ETH_MAX_FRAME_LEN)
	{
		goto __TERMINAL;
	}

	/*
	* Construct tx_desc.
	*/
	pTxDesc = (struct tx_desc*)data;
	pTxDesc->opts1 = cpu_to_le32(length | TX_FS | TX_LS);
	pTxDesc->opts2 = 0;
	memcpy(data + sizeof(struct tx_desc), pEthBuff->Buffer, length);

	/*
	* Update used data buffer's length and return it.
	*/
	length += offset;
	length += sizeof(struct tx_desc);
	length += (data - base);
	*data_len = length;

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
* Commit USB sending request.
*/
static BOOL __Commit_Send(__ETHERNET_INTERFACE* pEthInt, char* pSendingBuff, int length, int buff_num)
{
	int actual_len = 0;
	struct ueth_data* ss = NULL;
	struct usb_device *udev = NULL;

	ss = (struct ueth_data*)pEthInt->pIntExtension;
	BUG_ON(NULL == ss);
	udev = ss->pusb_dev;
	BUG_ON(NULL == udev);

	char* msg = _hx_aligned_malloc(length, USB_DMA_MINALIGN);
	if (NULL == msg)
	{
		return FALSE;
	}
	memcpy(msg, pSendingBuff, length);

	int err = usb_bulk_msg(udev, usb_sndbulkpipe(udev, ss->ep_out),
		(void *)pSendingBuff, length,
		//(void*)msg,length,
		&actual_len, USB_BULK_SEND_TIMEOUT);

	/* debug */
	//_hx_printf("%s:len = %d,buff_num = %d,actual_len = %d.\r\n", 
	//	__func__, length, buff_num, actual_len);

	if (err < 0)
	{
		if (pEthInt->ifState.dwTxErrorNum < 6)
		{
			_hx_printf("%s: bulk msg fail[err = %d,buf_l = %d,buff_n = %d].\r\n",
				__func__,
				err,
				length,
				buff_num);
		}
		pEthInt->ifState.dwTxErrorNum += buff_num;
		_hx_free(msg);
		return FALSE;
	}
	/* Update interface level statistics counter. */
	pEthInt->ifState.dwFrameSendSuccess += buff_num;
	_hx_free(msg);
	return TRUE;
}

static BOOL __Commit_Nostop_Send(__ETHERNET_INTERFACE* pEthInt, char* pSendingBuff, int length, int buff_num)
{
	int actual_len = 0;
	struct ueth_data* ss = NULL;
	struct usb_device *udev = NULL;

	ss = (struct ueth_data*)pEthInt->pIntExtension;
	BUG_ON(NULL == ss);
	udev = ss->pusb_dev;
	BUG_ON(NULL == udev);

	actual_len = USBManager.StartXfer(ss->pTxDesc, length);
	USBManager.StopXfer(ss->pTxDesc);

	/* debug */
	//_hx_printf("%s:len = %d,buff_num = %d,actual_len = %d.\r\n", 
	//	__func__, length, buff_num, actual_len);

	if (actual_len < 0)
	{
		if (pEthInt->ifState.dwTxErrorNum < 6)
		{
			_hx_printf("%s: bulk msg fail[err = %d,buf_l = %d,buff_n = %d].\r\n",
				__func__,
				actual_len,
				length,
				buff_num);
		}
		pEthInt->ifState.dwTxErrorNum += buff_num;
		return FALSE;
	}
	else
	{
		/* Update interface level statistics counter. */
		pEthInt->ifState.dwFrameSendSuccess += buff_num;
	}
	return TRUE;
}

/*
* Kernel thread responses for USB ethernet frame transmition.
* It polls message queue in loop and delivery the received
* frame to Ethernet Manager object if has.
* A KERNEL_MESSAGE_TERMINAL message will cause the thread
* exit.
*/
static DWORD __R8152_Send(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	__ETHERNET_BUFFER* pEthBuff = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	struct ueth_data* pEthData = (struct ueth_data*)pData;
	DWORD dwFlags;
	BOOL bShouldBreak = FALSE;

	/* BIG BUG */
	/*
	* Static local variable was used before the big bug located,
	* this caused that more than one sending thread of R8152 chip
	* would use the same buffer(pSendingBuff) when sending but the
	* actually packet content was filled somewhere else(in async
	* xfer descriptor).
	* This lead the crash of R8152 without any response.
	* More than 1 month time was spent to shot this issue,so regard
	* it as a BIG BUG.
	*/
	//static char* pSendingBuff = NULL;
	char* pSendingBuff = NULL; /* Dedicated send buffer. */
	int buff_num = 0, offset = 0;

	if (NULL == pEthData)
	{
		goto __TERMINAL;
	}
	if (NULL == pEthData->thisIf)  //Shoud not occur.
	{
		BUG();
	}
	pEthInt = pEthData->thisIf;

	BUG_ON(NULL == pEthData->tx_buff);
	BUG_ON(NULL == pEthData->pTxDesc);
	pSendingBuff = pEthData->tx_buff;

	/*
	* Change the owner of tx xfer descriptor,it's default
	* owner is USB_Core thread since it is created in process
	* of initialization.
	* Set to current thead to facilite debugging.
	*/
	pEthData->pTxDesc->hOwnerThread = (HANDLE)KernelThreadManager.lpCurrentKernelThread;

	//Check the message queue.
	while (KernelThreadManager.GetMessage(NULL, &msg))
	{
		if (msg.wCommand == KERNEL_MESSAGE_TERMINAL)
		{
			/*
			* If the thread exit without process the sending queue,memory
			* leaking may lead since there maybe ethernet buffer(s) pending
			* in queue.
			* We will curb this issue later.(:-)
			*/
			goto __TERMINAL;
		}
		if (msg.wCommand == R8152_SEND_FRAME)
		{
			bShouldBreak = FALSE;
			while (TRUE)
			{
				__ENTER_CRITICAL_SECTION(NULL, dwFlags);
				/*
				* Fetch one ethernet frame from sending list.
				*/
				pEthBuff = pEthData->pSndListFirst;
				if (NULL == pEthBuff) /* No pending frame. */
				{
					__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
					break;
				}
				/*
				* Pack as many as possible ethernet buffers into
				* the sending buffer.
				*/
				while (TRUE)
				{
					if (!__R8152_Pack_Ethernet_Buffer(pSendingBuff, pEthBuff, offset, &offset))
					{
						/* Sending buffer is full. */
						break;
					}
					/*
					* Unlink the ethernet buffer from sending list,
					* destroy it,and update the sending list accordingly.
					*/
					pEthData->pSndListFirst = pEthBuff->pNext;
					EthernetManager.DestroyEthernetBuffer(pEthBuff);
					pEthBuff = pEthData->pSndListFirst;
					buff_num++; /* How many eth buffers have been packed. */
					/*
					* Decrease the general sending queue size of system.
					*/
					EthernetManager.nDrvSendingQueueSz--;
					/* Update the sending queue(list) size. */
					pEthInt->nSendingQueueSz--;

					if (NULL == pEthBuff)
					{
						pEthData->pSndListLast = NULL;
						/*
						* Reach the end of the sending list,the loop should
						* break,to fetch message again.
						* Otherwise may lead the message queue full,since
						* the sendor continue to drop message to this thread
						* in case of pSndListLast is NULL.
						*/
						bShouldBreak = TRUE;
						break;
					}
				}
				__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
				/* Send out the packed ethernet buffers. */
				if (buff_num)
				{
					__Commit_Nostop_Send(pEthInt, pSendingBuff, offset, buff_num);
				}
				/* Reset state info. */
				offset = 0;
				buff_num = 0;

				/* Avoid thread message queue full. */
				if (bShouldBreak)
				{
					break;
				}
			}
		}
	}

__TERMINAL:
	/*
	* Destroy the tx descriptor and tx buffer,they are created in process
	* of NIC initialization.
	*/
	if (pEthData->pTxDesc)
	{
		USBManager.StopXfer(pEthData->pTxDesc);
		USBManager.DestroyXferDescriptor(pEthData->pTxDesc);
		pEthData->pTxDesc = NULL;
	}
	if (NULL != pSendingBuff)
	{
		_hx_free(pSendingBuff);
	}
	_hx_printf("%s:exit tx thread of ethernet if[%s].\r\n",
		__func__, pEthInt->ethName);
	return 0;
}

/*
* Rx routine,it submit an USB bulk xfer request,
* process the incoming data in case of success.
*/
static __ETHERNET_BUFFER* __r8152_nostop_recv(__ETHERNET_INTERFACE *eth,
	__USB_XFER_DESCRIPTOR* pRxDesc)
{
	struct ueth_data *dev = (struct ueth_data *)eth->pIntExtension;
	__ETHERNET_BUFFER* pEthBuffer = NULL;
	unsigned char *pkt_ptr;
	int err = 0;
	int actual_len;
	int packet_len;
	uint8_t* recv_buf = NULL;
	static int err_c = 0;
	static int xfer_ct = 0;

	int bytes_process = 0;
	struct rx_desc *rx_desc;

	/*
	* Fetch USB incoming data from R8152,
	*/
	actual_len = USBManager.StartXfer(pRxDesc, 0);
	USBManager.StopXfer(pRxDesc);
	if (actual_len < 0) {
		debug("Rx: failed to receive.\r\n");
		err = actual_len;
		goto __TERMINAL;
	}
	recv_buf = pRxDesc->pBuffer;

	if (actual_len > RTL8152_AGG_BUF_SZ) {
		_hx_printf("%s: received too many bytes %d\n", __func__, actual_len);
		goto __TERMINAL;
	}

	while (bytes_process < actual_len) {
		rx_desc = (struct rx_desc *)(recv_buf + bytes_process);
		pkt_ptr = recv_buf + sizeof(struct rx_desc) + bytes_process;

		packet_len = le32_to_cpu(rx_desc->opts1) & RX_LEN_MASK;
		/*
		* Check if the remaind data is enough.
		*/
		if (bytes_process + packet_len + (int)sizeof(struct rx_desc) > actual_len)
		{
			goto __TERMINAL;
		}
		/* Minus the CRC bytes. */
		packet_len -= CRC_SIZE;

		if (packet_len > ETH_DEFAULT_MTU + ETH_HEADER_LEN)
		{
			_hx_printf("%s:huge(invalid) frame received,len = %d.\r\n",
				__func__,
				packet_len);
			goto __TERMINAL;
		}
		if (packet_len < 60) //Please note that the CRC is omited.
		{
			_hx_printf("%s:frame length too short[len = %d].\r\n",
				__func__,
				packet_len);
			goto __TERMINAL;
		}

		/* Allocate an ethernet buffer to hold the content. */
		pEthBuffer = EthernetManager.CreateEthernetBuffer(packet_len + 8);
		if (NULL == pEthBuffer)
		{
			goto __TERMINAL;
		}
		memcpy(pEthBuffer->Buffer, pkt_ptr, packet_len);
		pEthBuffer->act_length = packet_len;
		memcpy(pEthBuffer->dstMAC, pkt_ptr, ETH_MAC_LEN);
		//_hx_ntoh_mac(pEthBuffer->dstMAC);
		pkt_ptr += ETH_MAC_LEN;
		memcpy(pEthBuffer->srcMAC, pkt_ptr, ETH_MAC_LEN);
		//_hx_ntoh_mac(pEthBuffer->srcMAC);
		pkt_ptr += ETH_MAC_LEN;
		pEthBuffer->frame_type = _hx_ntohs(*(__u16*)pkt_ptr);
		//pEthBuffer->pEthernetInterface = eth;
		pEthBuffer->pInInterface = eth;
		pEthBuffer->buff_status = ETHERNET_BUFFER_STATUS_INITIALIZED;
		/*
		* Delivery the ethernet buffer to OS kernel.
		*/
		if (!EthernetManager.PostFrame(eth, pEthBuffer))
		{
			EthernetManager.DestroyEthernetBuffer(pEthBuffer);
			break;
		}
		/*
		* Process the next packet,if has,in the bulk received
		* data.
		*/
		bytes_process += (packet_len + sizeof(struct rx_desc) + CRC_SIZE);
		if (bytes_process % RX_ALIGN)
		{
			bytes_process = bytes_process + RX_ALIGN - (bytes_process % RX_ALIGN);
		}
	}

__TERMINAL:
	xfer_ct++;
	if ((err_c < 6) && (err != 0))
	{
		//_hx_printf("%s:recv err,code = %d,xfer_ct = %d.\r\n", __func__, 
		//	err,xfer_ct);
		err_c++;
	}
	return NULL;
}

/*
* Kernel thread responses for USB ethernet frame receiving.
* It polls USB ethernet device in loop and delivery the received
* frame to Ethernet Manager object if has.
* A KERNEL_MESSAGE_TERMINAL message will cause the thread
* exit.
*/
static DWORD __R8152_Recv(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	__ETHERNET_BUFFER* pEthBuff = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	struct ueth_data* pEthData = (struct ueth_data*)pData;
	struct usb_device* dev = NULL;
	char* rx_buff = NULL;

	if (NULL == pEthData)
	{
		goto __TERMINAL;
	}
	BUG_ON(NULL == pEthData->thisIf);
	pEthInt = pEthData->thisIf;
	dev = pEthData->pusb_dev;
	BUG_ON(NULL == dev);
	BUG_ON(NULL == pEthData->rx_buff);
	BUG_ON(NULL == pEthData->pRxDesc);
	rx_buff = pEthData->rx_buff;

	/*
	* Change the owner of tx xfer descriptor,it's default
	* owner is USB_Core thread since it is created in process
	* of initialization.
	* Set to current thead to facilite debugging.
	*/
	pEthData->pRxDesc->hOwnerThread = (HANDLE)KernelThreadManager.lpCurrentKernelThread;

	while (TRUE)
	{
		//Peek the message queue.
		while (KernelThreadManager.PeekMessage(NULL, &msg))
		{
			if (msg.wCommand == KERNEL_MESSAGE_TERMINAL)
			{
				/*
				* If the thread exit without process the sending queue,memory
				* leaking may lead since there maybe ethernet buffer(s) pending
				* in queue.
				* We will curb this issue later.(:-)
				*/
				goto __TERMINAL;
			}
		}
		//Poll the USB ethernet device to receive ethernet buffer.
		//__r8152_recv(pEthInt);
		__r8152_nostop_recv(pEthInt, pEthData->pRxDesc);
	}

__TERMINAL:
	/*
	* Destroy the rx descriptor and rx buffer,
	* they are created in process of NIC initialization.
	*/
	if (pEthData->pRxDesc)
	{
		USBManager.StopXfer(pEthData->pRxDesc);
		USBManager.DestroyXferDescriptor(pEthData->pRxDesc);
		/* Reset to NULL. */
		pEthData->pRxDesc = NULL;
	}
	if (rx_buff)
	{
		_hx_free(rx_buff);
	}
	_hx_printf("%s:exit rx thread of ethernet[%s].\r\n",
		__func__,
		pEthInt->ethName);
	return 0;
}

/*
* Handle NIC status changing.
*/
static void ProcessNicStatus(struct ueth_data* ss, unsigned short status)
{
	//_hx_printf("R8152 nic[idx = %d] status changed[status = %X].\r\n",
	//	ss->ef_idx, status);
	return;
}

/*
* Background kernel thread that polling the status
* of r8152 NIC,show warning message if NIC's status
* change.
*/
#if 0
static DWORD __R8152_Damon(LPVOID pData)
{
	struct ueth_data* ss = (struct ueth_data*)pData;
	struct usb_device* pUsbDev = NULL;
	unsigned short status = 0;
	unsigned long pipe = 0;
	int err = 0;

	BUG_ON(NULL == ss);
	pUsbDev = ss->pusb_dev;
	pipe = usb_rcvintpipe(pUsbDev, ss->ep_int);

	//Main polling loop.
	while (TRUE)
	{
		err = USBManager.InterruptMessage(pUsbDev->phy_dev, pipe,
			&status, sizeof(status), ss->irqinterval);
		if (!err)
		{
			//Interpret the USB input data,translate it to kernel message and delivery to kernel.
			ProcessNicStatus(ss, status);
		}
		else
		{
			//_hx_printf("R8152 nic[idx = %d] failed to fetch status[err = %d].\r\n",
			//	ss->ef_idx, err);
			usb_clear_halt(pUsbDev, pipe);
		}
		Sleep(60000);
	}
	return 0;
}
#endif

/*
* A helper routine to register a new initialized R8152 device into
* HelloX Kernel.
*/
static int Register_R8152(struct usb_device *dev, struct ueth_data *ss)
{
	__ETHERNET_INTERFACE* pEthInt = NULL;
	unsigned char mac_addr[ETH_MAC_LEN];
	char eth_name[MAX_ETH_NAME_LEN + 1];
	int err = 0;

	/* Retrieve hardware address of the device. */
	err = r8152_read_mac(ss->dev_priv, mac_addr);
	if (err < 0)
	{
		_hx_printf("%s: failed to retrieve MAC addr,err = %d.\r\n",
			__func__,
			err);
		return 0;
	}

	/* Construct the default ethernet interface name. */
	_hx_sprintf(eth_name, "%s%d", R8152_NAME_BASE, ss->ef_idx);

	/* Add R8152 corresponding Ethernet interface. */
	pEthInt = EthernetManager.AddEthernetInterface(
		eth_name,
		mac_addr,
		ss,
		__r8152_init,
		__r8152_send,
		NULL, //Receive operation is put into receiving thread,since it may lead long time blocking.
		__r8152_control);

	if (NULL == pEthInt)
	{
		_hx_printf("Failed to add [%s] to network.\r\n", eth_name);
		return 0;
	}

	//Save into USB ethernet private data.
	ss->thisIf = pEthInt;

	_hx_printf("Add eth_if[name = %s,mac = %.2X-%.2X-%.2X-%.2X-%.2X-%.2X] successfully.\r\n",
		eth_name,
		mac_addr[0],
		mac_addr[1],
		mac_addr[2],
		mac_addr[3],
		mac_addr[4],
		mac_addr[5]);

	return 1;
}

/* Unregister the USB ethernet interface from system. */
static int Unregister_R8152(struct usb_device* dev, struct ueth_data* ss)
{
	if (NULL == ss)
	{
		return 0;
	}
	if (NULL == ss->thisIf)
	{
		return 0;
	}
	EthernetManager.DeleteEthernetInterface(ss->thisIf);
	return 1;
}

/*
* A helper routine to initialize and register a new found R8152
* device. It will be invoked when a new device is scaned in Driver Entry
* routine.
*/
static int Init_R8152(__PHYSICAL_DEVICE* pPhyDev, unsigned int ifnum)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int ep_in_found = 0, ep_out_found = 0;
	struct ueth_data* ss = NULL;
	int i, ret = 0;
	struct r8152 *tp = NULL;
	char recv_name[64];
	static int ef_idx = 0;
	struct usb_device *dev = NULL;

	/* Parameter checking. */
	BUG_ON(NULL == pPhyDev);
	/* Get the corresponding usb device. */
	dev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	BUG_ON(NULL == dev);

	/* Construct a new management data struct of R8152. */
	ss = _hx_malloc(sizeof(struct ueth_data));
	if (NULL == ss)
	{
		goto __TERMINAL;
	}
	memset(ss, 0, sizeof(struct ueth_data));

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected: %#04x:%#04x\n",
		dev->descriptor.idVendor, dev->descriptor.idProduct);

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct r8152));
	if (!ss->dev_priv)
	{
		goto __TERMINAL;
	}
	memzero(ss->dev_priv, sizeof(struct r8152));

	/*
	* We are expecting a minimum of 3 endpoints - in, out (bulk), and
	* int. We will ignore any others.
	*/
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
			USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			u8 ep_addr = iface->ep_desc[i].bEndpointAddress;
			if ((ep_addr & USB_DIR_IN) && !ep_in_found) {
				ss->ep_in = ep_addr &
					USB_ENDPOINT_NUMBER_MASK;
				ep_in_found = 1;
			}
			else {
				if (!ep_out_found) {
					ss->ep_out = ep_addr &
						USB_ENDPOINT_NUMBER_MASK;
					ep_out_found = 1;
				}
			}
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
			USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}

	debug("Endpoints In %d Out %d Int %d\n", ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
		!ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("Problems with device\n");
		goto __TERMINAL;
	}

	/* Establish data structure's relationship. */
	dev->privptr = (void *)ss;
	tp = ss->dev_priv;
	tp->udev = dev;
	tp->intf = iface;

	r8152b_get_version(tp);

	if (rtl_ops_init(tp))
	{
		_hx_printf("%s: failed to init r8152 device.\r\n", __func__);
		goto __TERMINAL;
	}

	tp->rtl_ops.init(tp);
	tp->rtl_ops.up(tp);

	rtl8152_set_speed(tp, AUTONEG_ENABLE,
		tp->supports_gmii ? SPEED_1000 : SPEED_100,
		DUPLEX_FULL);

	/* Assign a index value to this USB ethernet interface. */
	ss->ef_idx = ef_idx++;

	/* Initialization seems OK,now register the R8152 device into
	* HelloX kernel.
	*/
	if (!Register_R8152(dev, ss))
	{
		_hx_printf("Failed to register R8152 into system.\r\n");
		goto __TERMINAL;
	}

	/*
	* Create the recv and send buffer.These 2 buffers
	* will be released when the corresponding rx/tx thread exit.
	*/
	ss->rx_buff = (char*)_hx_aligned_malloc(RTL8152_AGG_BUF_SZ, USB_DMA_MINALIGN);
	if (NULL == ss->rx_buff)
	{
		goto __TERMINAL;
	}
	ss->tx_buff = (char*)_hx_aligned_malloc(RTL8152_AGG_BUF_SZ, USB_DMA_MINALIGN);
	if (NULL == ss->tx_buff)
	{
		goto __TERMINAL;
	}

	/*
	* Create the rx and tx bulk transfer descriptor,which is used
	* by the corresponding rx/tx thread.
	* These 2 descriptors will be destroyed by the rx/tx thread before
	* exit.
	*/
	ss->pRxDesc = USBManager.CreateXferDescriptor(
		pPhyDev, usb_rcvbulkpipe(ss->pusb_dev, ss->ep_in),
		ss->rx_buff, RTL8152_AGG_BUF_SZ, NULL, 0);
	if (NULL == ss->pRxDesc)
	{
		_hx_printf("%s: create rx desc failed.\r\n", __func__);
		goto __TERMINAL;
	}

	ss->pTxDesc = USBManager.CreateXferDescriptor(
		pPhyDev, usb_sndbulkpipe(ss->pusb_dev, ss->ep_out),
		ss->tx_buff, RTL8152_AGG_BUF_SZ, NULL, 0);
	if (NULL == ss->pTxDesc)
	{
		goto __TERMINAL;
	}

	/*
	* Create the dedicated receiving kernel thread.
	*/
	_hx_sprintf(recv_name, "%s%d", R8152_RECV_NAME_BASE, ss->ef_idx);
	ss->pRxThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_SUSPENDED,
		PRIORITY_LEVEL_NORMAL,
		__R8152_Recv,
		(LPVOID)ss,
		NULL,
		recv_name);
	if (NULL == ss->pRxThread)
	{
		goto __TERMINAL;
	}

	/*
	* Create the dedicated sending kernel thread.
	*/
	_hx_sprintf(recv_name, "%s%d", R8152_SEND_NAME_BASE, ss->ef_idx);
	ss->pTxThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_SUSPENDED,
		PRIORITY_LEVEL_NORMAL,
		__R8152_Send,
		(LPVOID)ss,
		NULL,
		recv_name);
	if (NULL == ss->pTxThread)
	{
		goto __TERMINAL;
	}

#if 0
	/* Create the dedicated status polling thread. */
	_hx_sprintf(recv_name, "%s%d", R8152_DAMON_NAME_BASE, ss->ef_idx);
	ss->pDamonThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_SUSPENDED,
		PRIORITY_LEVEL_NORMAL,
		__R8152_Damon,
		(LPVOID)ss,
		NULL,
		recv_name);
	if (NULL == ss->pDamonThread)
	{
		goto __TERMINAL;
	}
#endif

	/* Link the management data structure into global list. */
	ss->pNext = global_list;
	global_list = ss;

	/* Mark all things in place. */
	ret = 1;

__TERMINAL:
	if (0 == ret)  /* Failed,should release all resources allocated. */
	{
		if (ss)
		{
			Unregister_R8152(dev, ss);
			if (ss->pRxThread)  //Shoud destroy it.
			{
				WaitForThisObject((HANDLE)ss->pRxThread);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)ss->pRxThread);
			}
			if (ss->pTxThread)
			{
				WaitForThisObject((HANDLE)ss->pTxThread);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)ss->pTxThread);
			}
			if (ss->pDamonThread)
			{
				WaitForThisObject((HANDLE)ss->pDamonThread);
				KernelThreadManager.DestroyKernelThread(
					(__COMMON_OBJECT*)&KernelThreadManager,
					(__COMMON_OBJECT*)ss->pDamonThread);
			}
			if (ss->dev_priv)
			{
				_hx_free(ss->dev_priv);
			}
			if (ss->rx_buff)
			{
				_hx_free(ss->rx_buff);
			}
			if (ss->tx_buff)
			{
				_hx_free(ss->tx_buff);
			}
			if (ss->pRxDesc)
			{
				USBManager.StopXfer(ss->pRxDesc);
				USBManager.DestroyXferDescriptor(ss->pRxDesc);
			}
			if (ss->pTxDesc)
			{
				USBManager.StopXfer(ss->pTxDesc);
				USBManager.DestroyXferDescriptor(ss->pTxDesc);
			}
			_hx_free(ss);
		}
		/* Reset USB device's private pointer since it maybe changed in
		* above process.
		*/
		dev->privptr = NULL;
	}
	return ret;
}

/*
* Entry point of R8152 chipset driver.
*/
BOOL R8152_DriverEntry(__DRIVER_OBJECT* lpDrvObj)
{
	__IDENTIFIER id;
	__PHYSICAL_DEVICE* pPhyDev = NULL;
	struct usb_device* usb_dev = NULL;
	int i = 0;
	BOOL bResult = FALSE;

	/* Initialize id accordingly. */
	id.dwBusType = BUS_TYPE_USB;
	for (i = 0; i < ARRAY_SIZE(r8152_dongles); i++)
	{
		id.Bus_ID.USB_Identifier.ucMask = USB_IDENTIFIER_MASK_PRODUCTID;
		id.Bus_ID.USB_Identifier.ucMask |= USB_IDENTIFIER_MASK_VENDORID;
		id.Bus_ID.USB_Identifier.wProductID = r8152_dongles[i].product;
		id.Bus_ID.USB_Identifier.wVendorID = r8152_dongles[i].vendor;
		while (pPhyDev = USBManager.GetUsbDevice(&id, pPhyDev))
		{
			usb_dev = (struct usb_device*)pPhyDev->lpPrivateInfo;
			BUG_ON(NULL == usb_dev);

			/* Initialize it,one successful initialization for a new found
			* R8152 device will lead the successful of the whole routine.
			*/
			if (Init_R8152(pPhyDev, 0))
			{
				bResult = TRUE;
			}
		}
	}

	if (!bResult)  //Can not find any R8152 device,or failed to init it.
	{
		_hx_printf("No R8152 device available in system.\r\n");
		goto __TERMINAL;
	}

	/* Pull up the rx/tx kernel thread(s) for each R8152 device. */
	struct ueth_data* ss = global_list;
	while (ss)
	{
		BUG_ON(ss->pRxThread == NULL);
		BUG_ON(ss->pTxThread == NULL);
		KernelThreadManager.ResumeKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)ss->pRxThread);
		KernelThreadManager.ResumeKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)ss->pTxThread);
		KernelThreadManager.ResumeKernelThread(
			(__COMMON_OBJECT*)&KernelThreadManager,
			(__COMMON_OBJECT*)ss->pDamonThread);
		ss = ss->pNext;
	}

	/* Mark everything OK. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

#undef ARRAY_SIZE
#undef BIT
