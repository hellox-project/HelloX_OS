/*
* Copyright (c) 2011 The Chromium OS Authors.
*
* SPDX-License-Identifier:	GPL-2.0+
*/

#ifndef __USB_ETHER_H__
#define __USB_ETHER_H__

/*
*	IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
*	and FCS/CRC (frame check sequence).
*/
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	PKTSIZE_ALIGN	/* Max. octets in frame sans FCS */

struct ueth_data {
	/* Point to next one. */
	struct ueth_data* pNext;
	/* eth info,specific to HelloX ethernet framework. */
	__ETHERNET_INTERFACE* thisIf;
	/* Sending buffer list,i.e,the sending queue. */
	__ETHERNET_BUFFER* pSndList;
	/* driver private */
	void *dev_priv;
	/* mii phy id */
	int phy_id;

	/* Kernel thread for frame receiving and transmition. */
	__KERNEL_THREAD_OBJECT* pRxThread;
	__KERNEL_THREAD_OBJECT* pTxThread;

	/*
	 * USB asynchronous xfer descriptor,one for tx and
	 * one for rx.
	 */
	__USB_ASYNC_DESCRIPTOR* pRxDesc;
	__USB_ASYNC_DESCRIPTOR* pTxDesc;

	/*
	 * Receiving and sending buffer,used by USB bulk
	 * xfer for incoming and out ethernet frame.
	 */
	char* tx_buff;
	char* rx_buff;

	/* Ethernet interface index,in the R8152 scope. */
	int ef_idx;

	/* usb info */
	struct usb_device *pusb_dev;	/* this usb_device */
	unsigned char	ifnum;		/* interface number */
	unsigned char	ep_in;		/* in endpoint */
	unsigned char	ep_out;		/* out ....... */
	unsigned char	ep_int;		/* interrupt . */
	unsigned char	subclass;	/* as in overview */
	unsigned char	protocol;	/* .............. */
	unsigned char	irqinterval;	/* Intervall for IRQ Pipe */
};

#endif /* __USB_ETHER_H__ */
