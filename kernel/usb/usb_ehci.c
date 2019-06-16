/*-
* Copyright (c) 2007-2008, Juniper Networks, Inc.
* Copyright (c) 2008, Excito Elektronik....
* Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
*
* All rights reserved.
*
* SPDX-License-Identifier:	GPL-2.0
*/

#include <StdAfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <byteord.h>
#include <align.h>
#include <stdio.h>

#include "usb_defs.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "errno.h"
#include "ehci.h"
#include "usbasync.h"

//The source code inside this file only works when the EHCI is enabled.
#ifdef CONFIG_USB_EHCI

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

/*
* EHCI spec page 20 says that the HC may take up to 16 uFrames (= 4ms) to halt.
* Let's time out after 8 to have a little safety margin on top of that.
*/
#define HCHALT_TIMEOUT (8 * 1000)

#ifndef CONFIG_DM_USB
static struct ehci_ctrl ehcic[CONFIG_USB_MAX_CONTROLLER_COUNT] = { 0 };
#endif

#define ALIGN_END_ADDR(type, ptr, size)			\
	((unsigned long)(ptr) + roundup((size) * sizeof(type), USB_DMA_MINALIGN))

#ifdef __MS_VC__
#pragma pack(push,1)
static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_linux_config_descriptor config;
	struct usb_linux_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
} descriptor = {
#pragma pack(pop)
#else
static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_linux_config_descriptor config;
	struct usb_linux_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
}  __attribute__((packed)) descriptor = {
#endif
	{
		0x8,		/* bDescLength */
		0x29,		/* bDescriptorType: hub descriptor */
		2,		/* bNrPorts -- runtime modified */
		0,		/* wHubCharacteristics */
		10,		/* bPwrOn2PwrGood */
		0,		/* bHubCntrCurrent */
#ifdef __MS_VC__
		{ 0 },
		{ 0 },
#else
		{},		/* Device removable */
		{}		/* at most 7 ports! XXX */
#endif
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		cpu_to_le16(0x0200), /* bcdUSB: v2.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
		64,		/* bMaxPacketSize: 64 bytes */
		0x0000,		/* idVendor */
		0x0000,		/* idProduct */
		cpu_to_le16(0x0100), /* bcdDevice */
		1,		/* iManufacturer */
		2,		/* iProduct */
		0,		/* iSerialNumber */
		1		/* bNumConfigurations: 1 */
	},
	{
		0x9,
		2,		/* bDescriptorType: UDESC_CONFIG */
		cpu_to_le16(0x19),
		1,		/* bNumInterface */
		1,		/* bConfigurationValue */
		0,		/* iConfiguration */
		0x40,		/* bmAttributes: UC_SELF_POWER */
		0		/* bMaxPower */
	},
	{
		0x9,		/* bLength */
		4,		/* bDescriptorType: UDESC_INTERFACE */
		0,		/* bInterfaceNumber */
		0,		/* bAlternateSetting */
		1,		/* bNumEndpoints */
		9,		/* bInterfaceClass: UICLASS_HUB */
		0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
		0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
		0		/* iInterface */
	},
	{
		0x7,		/* bLength */
		5,		/* bDescriptorType: UDESC_ENDPOINT */
		0x81,		/* bEndpointAddress:
					* UE_DIR_IN | EHCI_INTR_ENDPT
					*/
					3,		/* bmAttributes: UE_INTERRUPT */
					8,		/* wMaxPacketSize */
					255		/* bInterval */
	},
};

#if defined(CONFIG_EHCI_IS_TDI)
#define ehci_is_TDI()	(1)
#else
#define ehci_is_TDI()	(0)
#endif

struct ehci_ctrl *ehci_get_ctrl(struct usb_device *udev)
{
#ifdef CONFIG_DM_USB
	return dev_get_priv(usb_get_bus(udev->dev));
#else
	__COMMON_USB_CONTROLLER* pUsbCtrl = (__COMMON_USB_CONTROLLER*)udev->controller;
	
	//Validate the common usb controller object.
	if (KERNEL_OBJECT_SIGNATURE != pUsbCtrl->dwObjectSignature)
	{
		BUG();
	}
	//return udev->controller;
	return pUsbCtrl->pUsbCtrl;
#endif
}

static int ehci_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	return PORTSC_PSPD(reg);
}

static void ehci_set_usbmode(struct ehci_ctrl *ctrl)
{
	uint32_t tmp;
	uint32_t *reg_ptr;

	reg_ptr = (uint32_t *)((u8 *)&ctrl->hcor->or_usbcmd + USBMODE);
	tmp = ehci_readl(reg_ptr);
	tmp |= USBMODE_CM_HC;
#if defined(CONFIG_EHCI_MMIO_BIG_ENDIAN)
	tmp |= USBMODE_BE;
#endif
	ehci_writel(reg_ptr, tmp);
}

static void ehci_powerup_fixup(struct ehci_ctrl *ctrl, uint32_t *status_reg,
	uint32_t *reg)
{
	mdelay(50);
}

static uint32_t *ehci_get_portsc_register(struct ehci_ctrl *ctrl, int port)
{
	uint32_t* portsc = NULL;

	if (port < 0 || port >= CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS) {
		/* Printing the message would cause a scan failure! */
		debug("The request port(%u) is not configured\r\n", port);
		return NULL;
	}
	portsc = (uint32_t *)&ctrl->hcor->or_portsc[port];
	return portsc;
}

int handshake(uint32_t *ptr, uint32_t mask, uint32_t done, int usec)
{
	uint32_t result;
	do {
		result = ehci_readl(ptr);
		udelay(2);
		if (result == ~(uint32_t)0)
		{
			_hx_printf("%s: result = 0xFFFFFFFF.\r\n", __func__);
			return -1;
		}
		result &= mask;
		if (result == done)
			return 0;
		usec--;
	} while (usec > 0);
	return -1;
}

static int ehci_reset(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	int ret = 0;

	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd = (cmd & ~CMD_RUN) | CMD_RESET;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	ret = handshake((uint32_t *)&ctrl->hcor->or_usbcmd,
		CMD_RESET, 0, 250 * 1000);
	if (ret < 0) {
		_hx_printf("EHCI fail to reset.\r\n");
		goto out;
	}

	if (ehci_is_TDI())
		ctrl->ops.set_usb_mode(ctrl);

#ifdef CONFIG_USB_EHCI_TXFIFO_THRESH
	cmd = ehci_readl(&ctrl->hcor->or_txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(CONFIG_USB_EHCI_TXFIFO_THRESH);
	ehci_writel(&ctrl->hcor->or_txfilltuning, cmd);
#endif
out:
	return ret;
}

static int ehci_shutdown(struct ehci_ctrl *ctrl)
{
	int i, ret = 0;
	uint32_t cmd, reg;

	if (!ctrl || !ctrl->hcor)
		return -EINVAL;

	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~(CMD_PSE | CMD_ASE);
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	ret = handshake(&ctrl->hcor->or_usbsts, STS_ASS | STS_PSS, 0,
		100 * 1000);

	if (!ret) {
		for (i = 0; i < CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS; i++) {
			reg = ehci_readl(&ctrl->hcor->or_portsc[i]);
			reg |= EHCI_PS_SUSP;
			ehci_writel(&ctrl->hcor->or_portsc[i], reg);
		}

		cmd &= ~CMD_RUN;
		ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
		ret = handshake(&ctrl->hcor->or_usbsts, STS_HALT, STS_HALT,
			HCHALT_TIMEOUT);
	}

	if (ret)
		puts("EHCI failed to shut down host controller.\r\n");

	return ret;
}

int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t delta, next;
	uint32_t addr = (unsigned long)buf;
	int idx;

	if (addr != ALIGN(addr, ARCH_DMA_MINALIGN))
		debug("EHCI-HCD: Misaligned buffer address (%p)\r\n", buf);

	flush_dcache_range(addr, ALIGN(addr + sz, ARCH_DMA_MINALIGN));

	idx = 0;
	while (idx < QT_BUFFER_CNT) {
		td->qt_buffer[idx] = cpu_to_hc32(addr);
		td->qt_buffer_hi[idx] = 0;
		next = (addr + EHCI_PAGE_SIZE) & ~(EHCI_PAGE_SIZE - 1);
		delta = next - addr;
		if (delta >= sz)
			break;
		sz -= delta;
		addr = next;
		idx++;
	}

	if (idx == QT_BUFFER_CNT) {
		printf("out of buffer pointers (%zu bytes left)\r\n", sz);
		return -1;
	}

	return 0;
}

u8 ehci_encode_speed(enum usb_device_speed speed)
{
#define QH_HIGH_SPEED	2
#define QH_FULL_SPEED	0
#define QH_LOW_SPEED	1
	if (speed == USB_SPEED_HIGH)
		return QH_HIGH_SPEED;
	if (speed == USB_SPEED_LOW)
		return QH_LOW_SPEED;
	return QH_FULL_SPEED;
}

void ehci_update_endpt2_dev_n_port(struct usb_device *udev,struct QH *qh)
{
	struct usb_device *ttdev;
	int parent_devnum;

	if (udev->speed != USB_SPEED_LOW && udev->speed != USB_SPEED_FULL)
		return;

	/*
	* For full / low speed devices we need to get the devnum and portnr of
	* the tt, so of the first upstream usb-2 hub, there may be usb-1 hubs
	* in the tree before that one!
	*/
#ifdef CONFIG_DM_USB
	/*
	* When called from usb-uclass.c: usb_scan_device() udev->dev points
	* to the parent udevice, not the actual udevice belonging to the
	* udev as the device is not instantiated yet. So when searching
	* for the first usb-2 parent start with udev->dev not
	* udev->dev->parent .
	*/
	struct udevice *parent;
	struct usb_device *uparent;

	ttdev = udev;
	parent = udev->dev;
	uparent = dev_get_parentdata(parent);

	while (uparent->speed != USB_SPEED_HIGH) {
		struct udevice *dev = parent;

		if (device_get_uclass_id(dev->parent) != UCLASS_USB_HUB) {
			printf("ehci: Error cannot find high-speed parent of usb-1 device\r\n");
			return;
		}

		ttdev = dev_get_parentdata(dev);
		parent = dev->parent;
		uparent = dev_get_parentdata(parent);
	}
	parent_devnum = uparent->devnum;
#else
	ttdev = udev;
	while (ttdev->parent && ttdev->parent->speed != USB_SPEED_HIGH)
		ttdev = ttdev->parent;
	if (!ttdev->parent)
		return;
	parent_devnum = ttdev->parent->devnum;
#endif

	qh->qh_endpt2 |= cpu_to_hc32(QH_ENDPT2_PORTNUM(ttdev->portnr) |
		QH_ENDPT2_HUBADDR(parent_devnum));
}

static int ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
	int length, struct devrequest *req)
{
	int ret = 0;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = usbCreateAsyncDescriptor(dev, pipe, buffer, length, req);
	if (NULL == pAsyncDesc)
	{
		_hx_printf("%s:create async_desc failed.\r\n", __func__);
		return -ENOMEM;
	}
	if (!usbStartAsyncXfer(pAsyncDesc,0))
	{
		usbStopAsyncXfer(pAsyncDesc);
		usbDestroyAsyncDescriptor(pAsyncDesc);
		dev->status = USB_ST_STALLED;
#ifdef __DEBUG_USB_ASYNC
		_hx_printf("%s: start async_xfer failed.\r\n", __func__);
#endif
		ret = pAsyncDesc->err_code;
		return -ret;
	}
	int xfer_sz = pAsyncDesc->xfersize;
#ifdef __DEBUG_USB_ASYNC
	_hx_printf("%s: ctrl msg OK[len = %d].\r\n", __func__,
		xfer_sz);
#endif
	usbStopAsyncXfer(pAsyncDesc);
	usbDestroyAsyncDescriptor(pAsyncDesc);
	dev->status = 0;
	dev->act_len = xfer_sz;
	return 0;
}

static int
__ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
int length, struct devrequest *req)
{
	ALLOC_ALIGN_BUFFER(struct QH, qh, 1, USB_DMA_MINALIGN);
	struct qTD *qtd;
	int qtd_count = 0;
	int qtd_counter = 0;
	volatile struct qTD *vtd;
	unsigned long ts;
	uint32_t *tdp;
	uint32_t endpt, maxpacket, token, usbsts;
	uint32_t c, toggle;
	uint32_t cmd;
	int timeout;
	int ret = 0;
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p\r\n", dev, pipe,
		buffer, length, req);
	if (req != NULL)
		debug("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\r\n",
		req->request, req->request,
		req->requesttype, req->requesttype,
		le16_to_cpu(req->value), le16_to_cpu(req->value),
		le16_to_cpu(req->index));

#define PKT_ALIGN	512
	/*
	* The USB transfer is split into qTD transfers. Eeach qTD transfer is
	* described by a transfer descriptor (the qTD). The qTDs form a linked
	* list with a queue head (QH).
	*
	* Each qTD transfer starts with a new USB packet, i.e. a packet cannot
	* have its beginning in a qTD transfer and its end in the following
	* one, so the qTD transfer lengths have to be chosen accordingly.
	*
	* Each qTD transfer uses up to QT_BUFFER_CNT data buffers, mapped to
	* single pages. The first data buffer can start at any offset within a
	* page (not considering the cache-line alignment issues), while the
	* following buffers must be page-aligned. There is no alignment
	* constraint on the size of a qTD transfer.
	*/
	if (req != NULL)
		/* 1 qTD will be needed for SETUP, and 1 for ACK. */
		qtd_count += 1 + 1;
	if (length > 0 || req == NULL) {
		/*
		* Determine the qTD transfer size that will be used for the
		* data payload (not considering the first qTD transfer, which
		* may be longer or shorter, and the final one, which may be
		* shorter).
		*
		* In order to keep each packet within a qTD transfer, the qTD
		* transfer size is aligned to PKT_ALIGN, which is a multiple of
		* wMaxPacketSize (except in some cases for interrupt transfers,
		* see comment in submit_int_msg()).
		*
		* By default, i.e. if the input buffer is aligned to PKT_ALIGN,
		* QT_BUFFER_CNT full pages will be used.
		*/
		int xfr_sz = QT_BUFFER_CNT;
		/*
		* However, if the input buffer is not aligned to PKT_ALIGN, the
		* qTD transfer size will be one page shorter, and the first qTD
		* data buffer of each transfer will be page-unaligned.
		*/
		if ((unsigned long)buffer & (PKT_ALIGN - 1))
			xfr_sz--;
		/* Convert the qTD transfer size to bytes. */
		xfr_sz *= EHCI_PAGE_SIZE;
		/*
		* Approximate by excess the number of qTDs that will be
		* required for the data payload. The exact formula is way more
		* complicated and saves at most 2 qTDs, i.e. a total of 128
		* bytes.
		*/
		qtd_count += 2 + length / xfr_sz;
	}
	/*
	* Threshold value based on the worst-case total size of the allocated qTDs for
	* a mass-storage transfer of 65535 blocks of 512 bytes.
	*/
#ifdef CONFIG_SYS_MALLOC_LEN
#if CONFIG_SYS_MALLOC_LEN <= 64 + 128 * 1024
	#warning CONFIG_SYS_MALLOC_LEN may be too small for EHCI
#endif
#endif
	qtd = memalign(USB_DMA_MINALIGN, qtd_count * sizeof(struct qTD));
	if (qtd == NULL) {
		printf("unable to allocate TDs\r\n");
		return -1;
	}

	memset(qh, 0, sizeof(struct QH));
	memset(qtd, 0, qtd_count * sizeof(*qtd));

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/*
	* Setup QH (3.6 in ehci-r10.pdf)
	*
	*   qh_link ................. 03-00 H
	*   qh_endpt1 ............... 07-04 H
	*   qh_endpt2 ............... 0B-08 H
	* - qh_curtd
	*   qh_overlay.qt_next ...... 13-10 H
	* - qh_overlay.qt_altnext
	*/
	qh->qh_link = cpu_to_hc32((unsigned long)&ctrl->qh_list | QH_LINK_TYPE_QH);
	c = (dev->speed != USB_SPEED_HIGH) && !usb_pipeendpoint(pipe);
	maxpacket = usb_maxpacket(dev, pipe);
	endpt = QH_ENDPT1_RL(8) | QH_ENDPT1_C(c) |
		QH_ENDPT1_MAXPKTLEN(maxpacket) | QH_ENDPT1_H(0) |
		QH_ENDPT1_DTC(QH_ENDPT1_DTC_DT_FROM_QTD) |
		QH_ENDPT1_EPS(ehci_encode_speed(dev->speed)) |
		QH_ENDPT1_ENDPT(usb_pipeendpoint(pipe)) | QH_ENDPT1_I(0) |
		QH_ENDPT1_DEVADDR(usb_pipedevice(pipe));
	qh->qh_endpt1 = cpu_to_hc32(endpt);
	endpt = QH_ENDPT2_MULT(1) | QH_ENDPT2_UFCMASK(0) | QH_ENDPT2_UFSMASK(0);
	qh->qh_endpt2 = cpu_to_hc32(endpt);
	ehci_update_endpt2_dev_n_port(dev, qh);
	qh->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	tdp = &qh->qh_overlay.qt_next;

	if (req != NULL) {
		/*
		* Setup request qTD (3.5 in ehci-r10.pdf)
		*
		*   qt_next ................ 03-00 H
		*   qt_altnext ............. 07-04 H
		*   qt_token ............... 0B-08 H
		*
		*   [ buffer, buffer_hi ] loaded with "req".
		*/
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = QT_TOKEN_DT(0) | QT_TOKEN_TOTALBYTES(sizeof(*req)) |
			QT_TOKEN_IOC(0) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(3) |
			QT_TOKEN_PID(QT_TOKEN_PID_SETUP) |
			QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], req, sizeof(*req))) {
			printf("unable to construct SETUP TD\r\n");
			goto fail;
		}
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((unsigned long)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		uint8_t *buf_ptr = buffer;
		int left_length = length;

		do {
			/*
			* Determine the size of this qTD transfer. By default,
			* QT_BUFFER_CNT full pages can be used.
			*/
			int xfr_bytes = QT_BUFFER_CNT * EHCI_PAGE_SIZE;
			/*
			* However, if the input buffer is not page-aligned, the
			* portion of the first page before the buffer start
			* offset within that page is unusable.
			*/
			xfr_bytes -= (unsigned long)buf_ptr & (EHCI_PAGE_SIZE - 1);
			/*
			* In order to keep each packet within a qTD transfer,
			* align the qTD transfer size to PKT_ALIGN.
			*/
			xfr_bytes &= ~(PKT_ALIGN - 1);
			/*
			* This transfer may be shorter than the available qTD
			* transfer size that has just been computed.
			*/
			xfr_bytes = min(xfr_bytes, left_length);

			/*
			* Setup request qTD (3.5 in ehci-r10.pdf)
			*
			*   qt_next ................ 03-00 H
			*   qt_altnext ............. 07-04 H
			*   qt_token ............... 0B-08 H
			*
			*   [ buffer, buffer_hi ] loaded with "buffer".
			*/
			qtd[qtd_counter].qt_next =
				cpu_to_hc32(QT_NEXT_TERMINATE);
			qtd[qtd_counter].qt_altnext =
				cpu_to_hc32(QT_NEXT_TERMINATE);
			token = QT_TOKEN_DT(toggle) |
				QT_TOKEN_TOTALBYTES(xfr_bytes) |
				QT_TOKEN_IOC(req == NULL) | QT_TOKEN_CPAGE(0) |
				QT_TOKEN_CERR(3) |
				QT_TOKEN_PID(usb_pipein(pipe) ?
			QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT) |
							  QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
			qtd[qtd_counter].qt_token = cpu_to_hc32(token);
			if (ehci_td_buffer(&qtd[qtd_counter], buf_ptr,
				xfr_bytes)) {
				printf("unable to construct DATA TD\r\n");
				goto fail;
			}
			/* Update previous qTD! */
			*tdp = cpu_to_hc32((unsigned long)&qtd[qtd_counter]);
			tdp = &qtd[qtd_counter++].qt_next;
			/*
			* Data toggle has to be adjusted since the qTD transfer
			* size is not always an even multiple of
			* wMaxPacketSize.
			*/
			if ((xfr_bytes / maxpacket) & 1)
				toggle ^= 1;
			buf_ptr += xfr_bytes;
			left_length -= xfr_bytes;
		} while (left_length > 0);
	}

	if (req != NULL) {
		/*
		* Setup request qTD (3.5 in ehci-r10.pdf)
		*
		*   qt_next ................ 03-00 H
		*   qt_altnext ............. 07-04 H
		*   qt_token ............... 0B-08 H
		*/
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = QT_TOKEN_DT(1) | QT_TOKEN_TOTALBYTES(0) |
			QT_TOKEN_IOC(1) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(3) |
			QT_TOKEN_PID(usb_pipein(pipe) ?
		QT_TOKEN_PID_OUT : QT_TOKEN_PID_IN) |
						   QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((unsigned long)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
	}

	ctrl->qh_list.qh_link = cpu_to_hc32((unsigned long)qh | QH_LINK_TYPE_QH);

	/* Flush dcache */
	flush_dcache_range((unsigned long)&ctrl->qh_list,
		ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
	flush_dcache_range((unsigned long)qh, ALIGN_END_ADDR(struct QH, qh, 1));
	flush_dcache_range((unsigned long)qtd,
		ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, (unsigned long)&ctrl->qh_list);

	usbsts = ehci_readl(&ctrl->hcor->or_usbsts);
	ehci_writel(&ctrl->hcor->or_usbsts, (usbsts & 0x3f));

	/* Enable async. schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd |= CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, STS_ASS,
		100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STS_ASS set\r\n");
		goto fail;
	}

	debug("%s: begin to wait for TDs to be processed.\r\n",__func__);

	/* Wait for TDs to be processed. */
	ts = get_timer(0);
	vtd = &qtd[qtd_counter - 1];
	timeout = USB_TIMEOUT_MS(pipe);
	do {
		/* Invalidate dcache */
		invalidate_dcache_range((unsigned long)&ctrl->qh_list,
			ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
		invalidate_dcache_range((unsigned long)qh,
			ALIGN_END_ADDR(struct QH, qh, 1));
		invalidate_dcache_range((unsigned long)qtd,
			ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

		token = hc32_to_cpu(vtd->qt_token);
		if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE))
			break;
		WATCHDOG_RESET();
	} while (get_timer(ts) < (ulong)timeout);

	debug("%s: end to wait for TDs to be processed.\r\n", __func__);
	/*
	* Invalidate the memory area occupied by buffer
	* Don't try to fix the buffer alignment, if it isn't properly
	* aligned it's upper layer's fault so let invalidate_dcache_range()
	* vow about it. But we have to fix the length as it's actual
	* transfer length and can be unaligned. This is potentially
	* dangerous operation, it's responsibility of the calling
	* code to make sure enough space is reserved.
	*/
	invalidate_dcache_range((unsigned long)buffer,
		ALIGN((unsigned long)buffer + length, ARCH_DMA_MINALIGN));

	/* Check that the TD processing happened */
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE)
		debug("EHCI timed out on TD - token=%#x\r\n", token);

	/* Disable async schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, 0,
		100 * 1000);
	if (ret < 0) {
		_hx_printf("EHCI fail timeout STS_ASS reset\r\n");
		goto fail;
	}

	token = hc32_to_cpu(qh->qh_overlay.qt_token);
	if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE)) {
		debug("TOKEN=%#x\r\n", token);
		switch (QT_TOKEN_GET_STATUS(token) &
			~(QT_TOKEN_STATUS_SPLITXSTATE | QT_TOKEN_STATUS_PERR)) {
		case 0:
			toggle = QT_TOKEN_GET_DT(token);
			usb_settoggle(dev, usb_pipeendpoint(pipe),
				usb_pipeout(pipe), toggle);
			dev->status = 0;
			break;
		case QT_TOKEN_STATUS_HALTED:
			dev->status = USB_ST_STALLED;
			break;
		case QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_STATUS_DATBUFERR:
		case QT_TOKEN_STATUS_DATBUFERR:
			dev->status = USB_ST_BUF_ERR;
			break;
		case QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET:
		case QT_TOKEN_STATUS_BABBLEDET:
			dev->status = USB_ST_BABBLE_DET;
			break;
		default:
			dev->status = USB_ST_CRC_ERR;
			if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_HALTED)
				dev->status |= USB_ST_STALLED;
			break;
		}
		dev->act_len = length - QT_TOKEN_GET_TOTALBYTES(token);
	}
	else {
		dev->act_len = 0;
#ifndef CONFIG_USB_EHCI_FARADAY
		debug("dev=%u, usbsts=%#x, p[1]=%#x, p[2]=%#x\r\n",
			dev->devnum, ehci_readl(&ctrl->hcor->or_usbsts),
			ehci_readl(&ctrl->hcor->or_portsc[0]),
			ehci_readl(&ctrl->hcor->or_portsc[1]));
#endif
	}

	free(qtd);
	return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:
	free(qtd);
	return -1;
}

static int ehci_submit_root(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length, struct devrequest *req)
{
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr = NULL;
	int len, srclen;
	uint32_t reg;
	uint32_t *status_reg;
	int port = le16_to_cpu(req->index) & 0xff;
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	srclen = 0;

	//debug("req=%u (%#x), type=%u (%#x), value=%u, index=%u\r\n",
	//	req->request, req->request,
	//	req->requesttype, req->requesttype,
	//	le16_to_cpu(req->value), le16_to_cpu(req->index));

	typeReq = req->request | req->requesttype << 8;

	switch (typeReq) {
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8) :
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8) :
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8) :
		status_reg = ctrl->ops.get_portsc_register(ctrl, port - 1);
		if (!status_reg)
			return -1;
		break;
	default:
		status_reg = NULL;
		break;
	}

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			debug("USB_DT_DEVICE request\r\n");
			srcptr = &descriptor.device;
			srclen = descriptor.device.bLength;
			break;
		case USB_DT_CONFIG:
			debug("USB_DT_CONFIG config\r\n");
			srcptr = &descriptor.config;
			srclen = descriptor.config.bLength +
				descriptor.interface.bLength +
				descriptor.endpoint.bLength;
			break;
		case USB_DT_STRING:
			debug("USB_DT_STRING config\r\n");
			switch (le16_to_cpu(req->value) & 0xff) {
			case 0:	/* Language */
				srcptr = "\4\3\1\0";
				srclen = 4;
				break;
			case 1:	/* Vendor */
				srcptr = "\16\3u\0-\0b\0o\0o\0t\0";
				srclen = 14;
				break;
			case 2:	/* Product */
				srcptr = "\52\3E\0H\0C\0I\0 "
					"\0H\0o\0s\0t\0 "
					"\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
				srclen = 42;
				break;
			default:
				debug("unknown value DT_STRING %x\r\n",
					le16_to_cpu(req->value));
				goto unknown;
			}
			break;
		default:
			debug("unknown value %x\r\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8) :
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
			debug("USB_DT_HUB config\r\n");
			srcptr = &descriptor.hub;
			srclen = descriptor.hub.bLength;
			break;
		default:
			debug("unknown value %x\r\n", le16_to_cpu(req->value));
			goto unknown;
	}
																   break;
	case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8) :
		debug("USB_REQ_SET_ADDRESS\r\n");
		ctrl->rootdev = le16_to_cpu(req->value);
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		debug("USB_REQ_SET_CONFIGURATION\r\n");
		/* Nothing to do */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8) :
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8) :
		memset(tmpbuf, 0, 4);
		reg = ehci_readl(status_reg);
		//debug("%s:port [%d]'s status = [%X].\r\n", __func__, port, reg);
		if (reg & EHCI_PS_CS)
			tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
		if (reg & EHCI_PS_PE)
			tmpbuf[0] |= USB_PORT_STAT_ENABLE;
		if (reg & EHCI_PS_SUSP)
			tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
		if (reg & EHCI_PS_OCA)
			tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
		if (reg & EHCI_PS_PR)
			tmpbuf[0] |= USB_PORT_STAT_RESET;
		if (reg & EHCI_PS_PP)
			tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;

		if (ehci_is_TDI()) {
			switch (ctrl->ops.get_port_speed(ctrl, reg)) {
			case PORTSC_PSPD_FS:
				break;
			case PORTSC_PSPD_LS:
				tmpbuf[1] |= USB_PORT_STAT_LOW_SPEED >> 8;
				break;
			case PORTSC_PSPD_HS:
			default:
				tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
				break;
			}
		}
		else {
			tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
		}

		if (reg & EHCI_PS_CSC)
			tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
		if (reg & EHCI_PS_PEC)
			tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
		if (reg & EHCI_PS_OCC)
			tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
		if (ctrl->portreset & (1 << port))
			tmpbuf[2] |= USB_PORT_STAT_C_RESET;

		srcptr = tmpbuf;
		srclen = 4;
		break;
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8) :
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg |= EHCI_PS_PE;
			ehci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams))) {
				reg |= EHCI_PS_PP;
				ehci_writel(status_reg, reg);
			}
			break;
		case USB_PORT_FEAT_RESET:
			if ((reg & (EHCI_PS_PE | EHCI_PS_CS)) == EHCI_PS_CS &&
				!ehci_is_TDI() &&
				EHCI_PS_IS_LOWSPEED(reg)) {
				/* Low speed device, give up ownership. */
				debug("port %d low speed --> companion\r\n",
					port - 1);
				reg |= EHCI_PS_PO;
				ehci_writel(status_reg, reg);
				return -ENXIO;
			}
			else {
				int ret;

				reg |= EHCI_PS_PR;
				reg &= ~EHCI_PS_PE;
				ehci_writel(status_reg, reg);
				/*
				* caller must wait, then call GetPortStatus
				* usb 2.0 specification say 50 ms resets on
				* root
				*/
				ctrl->ops.powerup_fixup(ctrl, status_reg, &reg);

				ehci_writel(status_reg, reg & ~EHCI_PS_PR);
				/*
				* A host controller must terminate the reset
				* and stabilize the state of the port within
				* 2 milliseconds
				*/
				ret = handshake(status_reg, EHCI_PS_PR, 0,
					2 * 1000);
				if (!ret) {
					reg = ehci_readl(status_reg);
					if ((reg & (EHCI_PS_PE | EHCI_PS_CS))
						== EHCI_PS_CS && !ehci_is_TDI()) {
						debug("port %d full speed --> companion\r\n", port - 1);
						reg &= ~EHCI_PS_CLEAR;
						reg |= EHCI_PS_PO;
						ehci_writel(status_reg, reg);
						return -ENXIO;
					}
					else {
						ctrl->portreset |= 1 << port;
					}
				}
				else {
					printf("port(%d) reset error\r\n",
						port - 1);
				}
			}
			break;
		case USB_PORT_FEAT_TEST:
			ehci_shutdown(ctrl);
			reg &= ~(0xf << 16);
			reg |= ((le16_to_cpu(req->index) >> 8) & 0xf) << 16;
			ehci_writel(status_reg, reg);
			break;
		default:
			debug("unknown feature %x\r\n", le16_to_cpu(req->value));
			goto unknown;
		}
		/* unblock posted writes */
		(void)ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8) :
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg &= ~EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_C_ENABLE:
			reg |= EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams)))
				reg &= ~EHCI_PS_PP;
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			reg |= EHCI_PS_CSC;
			break;
		case USB_PORT_FEAT_OVER_CURRENT:
			reg |= EHCI_PS_OCC;
			break;
		case USB_PORT_FEAT_C_RESET:
			ctrl->portreset &= ~(1 << port);
			break;
		default:
			debug("unknown feature %x\r\n", le16_to_cpu(req->value));
			goto unknown;
		}
		ehci_writel(status_reg, reg);
		/* unblock posted write */
		(void)ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	default:
		debug("Unknown request\r\n");
		goto unknown;
	}

	mdelay(1);
	len = min3(srclen, (int)le16_to_cpu(req->length), length);
	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	else
		debug("Len is 0\r\n");

	dev->act_len = len;
	dev->status = 0;
	return 0;

unknown:
	debug("requesttype=%x, request=%x, value=%x, index=%x, length=%x\r\n",
		req->requesttype, req->request, le16_to_cpu(req->value),
		le16_to_cpu(req->index), le16_to_cpu(req->length));

	dev->act_len = 0;
	dev->status = USB_ST_STALLED;
	return -1;
}

#ifdef __MS_VC__
//Some early version of MS VC compiler can not support the .[name] = [value] format
//of initialization.
const struct ehci_ops default_ehci_ops = {
	ehci_set_usbmode,
	ehci_get_port_speed,
	ehci_powerup_fixup,
	ehci_get_portsc_register,
};
#else
const struct ehci_ops default_ehci_ops = {
	.set_usb_mode = ehci_set_usbmode,
	.get_port_speed = ehci_get_port_speed,
	.powerup_fixup = ehci_powerup_fixup,
	.get_portsc_register = ehci_get_portsc_register,
};
#endif

static void ehci_setup_ops(struct ehci_ctrl *ctrl, const struct ehci_ops *ops)
{
	if (!ops) {
		ctrl->ops = default_ehci_ops;
	}
	else {
		ctrl->ops = *ops;
		if (!ctrl->ops.set_usb_mode)
			ctrl->ops.set_usb_mode = ehci_set_usbmode;
		if (!ctrl->ops.get_port_speed)
			ctrl->ops.get_port_speed = ehci_get_port_speed;
		if (!ctrl->ops.powerup_fixup)
			ctrl->ops.powerup_fixup = ehci_powerup_fixup;
		if (!ctrl->ops.get_portsc_register)
			ctrl->ops.get_portsc_register =
			ehci_get_portsc_register;
	}
}

#ifndef CONFIG_DM_USB
void ehci_set_controller_priv(int index, void *priv, const struct ehci_ops *ops)
{
	struct ehci_ctrl *ctrl = &ehcic[index];

	ctrl->priv = priv;
	ehci_setup_ops(ctrl, ops);
}

void *ehci_get_controller_priv(int index)
{
	return ehcic[index].priv;
}
#endif

static int ehci_common_init(struct ehci_ctrl *ctrl, uint tweaks)
{
	struct QH *qh_list;
	struct QH *periodic;
	uint32_t reg;
	uint32_t cmd;
	int i;

	/* Set the high address word (aka segment) for 64-bit controller */
	if (ehci_readl(&ctrl->hccr->cr_hccparams) & 1)
		ehci_writel(&ctrl->hcor->or_ctrldssegment, 0);

	qh_list = &ctrl->qh_list;

	/* Set head of reclaim list */
	memset(qh_list, 0, sizeof(*qh_list));
	qh_list->qh_link = cpu_to_hc32((unsigned long)qh_list | QH_LINK_TYPE_QH);
	qh_list->qh_endpt1 = cpu_to_hc32(QH_ENDPT1_H(1) |
		QH_ENDPT1_EPS(USB_SPEED_HIGH));
	qh_list->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_token =
		cpu_to_hc32(QT_TOKEN_STATUS(QT_TOKEN_STATUS_HALTED));

	flush_dcache_range((unsigned long)qh_list,ALIGN_END_ADDR(struct QH, qh_list, 1));

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, (unsigned long)qh_list);

	/*
	* Set up periodic list
	* Step 1: Parent QH for all periodic transfers.
	*/
	ctrl->periodic_schedules = 0;
	periodic = &ctrl->periodic_queue;
	memset(periodic, 0, sizeof(*periodic));
	periodic->qh_link = cpu_to_hc32(QH_LINK_TERMINATE);
	periodic->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	periodic->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	flush_dcache_range((unsigned long)periodic,ALIGN_END_ADDR(struct QH, periodic, 1));

	/*
	* Step 2: Setup frame-list: Every microframe, USB tries the same list.
	*         In particular, device specifications on polling frequency
	*         are disregarded. Keyboards seem to send NAK/NYet reliably
	*         when polled with an empty buffer.
	*
	*         Split Transactions will be spread across microframes using
	*         S-mask and C-mask.
	*/
	if (ctrl->periodic_list == NULL)
	{
		ctrl->periodic_list = memalign(4096, USB_PERIODIC_LIST_LENGTH * 4);
	}

	if (!ctrl->periodic_list)
	{
		_hx_printf("%s: allocate periodic list failed.\r\n", __func__);
		return -ENOMEM;
	}
	debug("%s: periodic list base = %X.\r\n", __func__, ctrl->periodic_list);

	for (i = 0; i < USB_PERIODIC_LIST_LENGTH; i++) {
		ctrl->periodic_list[i] = cpu_to_hc32((unsigned long)periodic
			| QH_LINK_TYPE_QH);
	}

	flush_dcache_range((unsigned long)ctrl->periodic_list,
		ALIGN_END_ADDR(uint32_t, ctrl->periodic_list,USB_PERIODIC_LIST_LENGTH));

	/* Set periodic list base address */
	ehci_writel(&ctrl->hcor->or_periodiclistbase,
		(unsigned long)ctrl->periodic_list);

	reg = ehci_readl(&ctrl->hccr->cr_hcsparams);
	descriptor.hub.bNbrPorts = HCS_N_PORTS(reg);
	_hx_printf("Register %x NbrPorts %d\r\n", reg, descriptor.hub.bNbrPorts);

	/* Port Indicators */
	if (HCS_INDICATOR(reg))
	{
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
			| 0x80, &descriptor.hub.wHubCharacteristics);
	}
	/* Port Power Control */
	if (HCS_PPC(reg))
	{
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
			| 0x01, &descriptor.hub.wHubCharacteristics);
	}

	//Create MUTEX object of the controller.
	ctrl->hMutex = CreateMutex();
	if (NULL == ctrl->hMutex)
	{
		_hx_printf("%s:failed to create MUTEX object.\r\n", __func__);
		return -1;
	}

	/* Start the host controller. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	/*
	* Philips, Intel, and maybe others need CMD_RUN before the
	* root hub will detect new devices (why?); NEC doesn't
	*/
	cmd &= ~(CMD_LRESET | CMD_IAAD | CMD_PSE | CMD_ASE | CMD_RESET);
	cmd |= CMD_RUN;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);  //Unblock posted write.
	mdelay(50);

	if (!(tweaks & EHCI_TWEAK_NO_INIT_CF)) {
		/* take control over the ports */
		cmd = ehci_readl(&ctrl->hcor->or_configflag);
		cmd |= FLAG_CF;
		ehci_writel(&ctrl->hcor->or_configflag, cmd);
		debug("%s:set CF bit,cf_reg = %X,cmd = %X.\r\n", __func__,
			&ctrl->hcor->or_configflag,cmd);
	}
	mdelay(50);

	/* unblock posted write */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);

	/*
	 * When estimately running here,the system 
	 * will halt for several seconds(about 10~20) without
	 * any response,I don't know why,maybe caused 
	 * by USB controller's hardware...
	 */
	/* Issue is caused by BIOS setting of EHCI and solved. */
	mdelay(5);

#ifndef USB_EHCI_DISABLE_INTERRUPT
	//Enable interrupt of the controller.
	cmd = ehci_readl(&ctrl->hcor->or_usbintr);
	//cmd |= (INTR_UE | INTR_UEE | INTR_AAE | INTR_PCE | INTR_SEE | INTR_FLR);
	cmd |= (INTR_UE | INTR_UEE | INTR_PCE | INTR_SEE | INTR_FLR);
	ehci_writel(&ctrl->hcor->or_usbintr, cmd);
#endif

	reg = HC_VERSION(ehci_readl(&ctrl->hccr->cr_capbase));
	reg = ehci_readl(&ctrl->hccr->cr_hccparams);

	return 0;
}

#ifndef CONFIG_DM_USB
int _ehci_usb_lowlevel_stop(int index)
{
	ehci_shutdown(&ehcic[index]);
	return ehci_hcd_stop(index);
}

//Declaration of several EHCI controller manipulation routines.
static int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int transfer_len);
static int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
	int transfer_len, struct devrequest *setup);
static int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
	int transfer_len, int interval);
static struct int_queue *create_int_queue(struct usb_device *dev, unsigned long pipe,
	int queuesize, int elementsize, void *buffer, int interval);
static int destroy_int_queue(struct usb_device *dev, struct int_queue *queue);
static void *poll_int_queue(struct usb_device *dev, struct int_queue *queue);

//Return EHCI controller's status register.
static unsigned long _get_ctrl_status(void* common_ctrl,DWORD ctrlFlag)
{
	__COMMON_USB_CONTROLLER* pCtrl = (__COMMON_USB_CONTROLLER*)common_ctrl;
	struct ehci_ctrl* pEhciCtrl = NULL;

	if (NULL == pCtrl)
	{
		BUG();
	}
	if (KERNEL_OBJECT_SIGNATURE != pCtrl->dwObjectSignature)
	{
		BUG();
	}
	pEhciCtrl = (struct ehci_ctrl*)pCtrl->pUsbCtrl;
	if (NULL == pEhciCtrl)
	{
		BUG();
	}
	switch (ctrlFlag)
	{
	case USB_CTRL_FLAG_EHCI_STATUS:
		return __readl(&(pEhciCtrl->hcor->or_usbsts));
	case USB_CTRL_FLAG_EHCI_COMMAND:
		return __readl(&pEhciCtrl->hcor->or_usbcmd);
	case USB_CTRL_FLAG_EHCI_INTR:
		return __readl(&pEhciCtrl->hcor->or_usbintr);
	case USB_CTRL_FLAG_EHCI_CF:
		return __readl(&pEhciCtrl->hcor->or_configflag);
	case USB_CTRL_FLAG_EHCI_PLBASE:
		return __readl(&pEhciCtrl->hcor->or_periodiclistbase);
	case USB_CTRL_FLAG_EHCI_ALBASE:
		return __readl(&pEhciCtrl->hcor->or_asynclistaddr);
	case USB_CTRL_FLAG_EHCI_XFERERR:
		return pEhciCtrl->nXferErrNum;
	case USB_CTRL_FLAG_EHCI_XFERREQ:
		return pEhciCtrl->nXferReqNum;
	case USB_CTRL_FLAG_EHCI_XFERINT:
		return pEhciCtrl->nXferIntNum;
	case USB_CTRL_FLAG_EHCI_ASSN:
		return pEhciCtrl->async_schedules;
	default:
		return -1;
	}
	return -1;
}

/*
 * EHCI continuous transfer operations.
 * These routines are called by USB Manager object's continuous xfer routines.
 */
/* Create a EHCI specific xfer descriptor and return it. */
static void* _ehciCreateXferDescriptor(__PHYSICAL_DEVICE* dev, unsigned long pipe,
	void* buffer, int buff_len,
	struct devrequest* setup, /* For control xfer. */
	int interval) /* For interrupt xfer. */
{
	struct usb_device* pUsbDev = NULL;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = NULL;
	void* ret = NULL;

	/* Parameters checking. */
	if ((NULL == dev) || (NULL == buffer) || (0 == buff_len))
	{
		goto __TERMINAL;
	}
	/* Get the USB device object. */
	pUsbDev = (struct usb_device*)dev->lpPrivateInfo;
	BUG_ON(NULL == pUsbDev);

	/* Create the EHCI specific xfer descriptor,according the xfer type. */
	if (usb_pipetype(pipe) == PIPE_BULK)
	{
		pAsyncDesc = usbCreateAsyncDescriptor(pUsbDev,
			pipe,
			buffer,
			buff_len,
			NULL);
		ret = pAsyncDesc;
	}
	else if (usb_pipetype(pipe) == PIPE_CONTROL)
	{
		/* Setup packet must be specified. */
		BUG_ON(NULL == setup);
		pAsyncDesc = usbCreateAsyncDescriptor(pUsbDev,
			pipe,
			buffer,
			buff_len,
			setup);
		ret = pAsyncDesc;
	}
	else if (usb_pipetype(pipe) == PIPE_INTERRUPT)
	{
		/* Should create interrupt queue object. */
	}
	else if (usb_pipetype(pipe) == PIPE_ISOCHRONOUS)
	{
		/* Should create a isochronous descriptor., */
	}
	else
	{
		/* Should not occur. */
		BUG();
	}

__TERMINAL:
	return ret;
}

/* 
 * Start a EHCI specific xfer. 
 * The requested data length is specified in req_len,and
 * the actual transfered length is returned back.
 */
static int _ehciStartXfer(__USB_XFER_DESCRIPTOR* pXferDesc,int req_len)
{
	int xfer_sz = -1;
	int pipe = 0;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = NULL;
	struct usb_device* dev = NULL;
	__PHYSICAL_DEVICE* pPhyDev = NULL;

	/* Check parameters. */
	BUG_ON(NULL == pXferDesc);
	/* Requested data length must be less than buffer's length. */
	if (pXferDesc->buffLength < req_len)
	{
		_hx_printf("%s:invalid request data length[req = %d,buff_len = %d.\r\n",
			req_len,
			pXferDesc->buffLength);
		goto __TERMINAL;
	}

	/* Get the corresponding USB device object. */
	pPhyDev = pXferDesc->pPhyDev;
	BUG_ON(NULL == pPhyDev);
	dev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	BUG_ON(NULL == dev);

	/* Start xfer according pipe type. */
	pipe = pXferDesc->pipe;
	if (usb_pipetype(pipe) == PIPE_BULK)
	{
		/* Bulk xfer. */
		pAsyncDesc = (__USB_ASYNC_DESCRIPTOR*)pXferDesc->priv;
		BUG_ON(NULL == pAsyncDesc);
		if (usbStartAsyncXfer(pAsyncDesc, req_len))
		{
			xfer_sz = pAsyncDesc->xfersize;
			dev->status = 0;
			dev->act_len = xfer_sz;
			goto __TERMINAL;
		}
		else /* Failed to do xfer. */
		{
			dev->status = USB_ST_STALLED;
			xfer_sz = pAsyncDesc->err_code;
			xfer_sz = -xfer_sz;
			goto __TERMINAL;
		}
	}
	else if (usb_pipetype(pipe) == PIPE_CONTROL)
	{
		/* Control xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_INTERRUPT)
	{
		/* Interrupt xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_ISOCHRONOUS)
	{
		/* Isochronous xfer. */
	}
	else
	{
		BUG();
	}

__TERMINAL:
	return xfer_sz;
}

/*
* Stop a EHCI specific xfer.
* Other thread(s) can call this routine to cancel the pending
* transfers that not it's own,since the pending xfer's owner
* thread is in blocked status and can not do anything.
*/
static int _ehciStopXfer(__USB_XFER_DESCRIPTOR* pXferDesc)
{
	int pipe = 0;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = NULL;
	struct usb_device* dev = NULL;
	__PHYSICAL_DEVICE* pPhyDev = NULL;
	BOOL bResult = FALSE;

	/* Check parameters. */
	BUG_ON(NULL == pXferDesc);

	/* Get the corresponding USB device object. */
	pPhyDev = pXferDesc->pPhyDev;
	BUG_ON(NULL == pPhyDev);
	dev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	BUG_ON(NULL == dev);

	/* Stop xfer according pipe type. */
	pipe = pXferDesc->pipe;
	if (usb_pipetype(pipe) == PIPE_BULK)
	{
		/* Bulk xfer. */
		pAsyncDesc = (__USB_ASYNC_DESCRIPTOR*)pXferDesc->priv;
		BUG_ON(NULL == pAsyncDesc);
		bResult = usbStopAsyncXfer(pAsyncDesc);
		goto __TERMINAL;
	}
	else if (usb_pipetype(pipe) == PIPE_CONTROL)
	{
		/* Control xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_INTERRUPT)
	{
		/* Interrupt xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_ISOCHRONOUS)
	{
		/* Isochronous xfer. */
	}
	else
	{
		BUG();
	}

__TERMINAL:
	return bResult;
}

/*
* Destroy a EHCI specific xfer descriptor.
*/
static void _ehciDestroyXferDescriptor(__USB_XFER_DESCRIPTOR* pXferDesc)
{
	int pipe = 0;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = NULL;
	struct usb_device* dev = NULL;
	__PHYSICAL_DEVICE* pPhyDev = NULL;

	/* Check parameters. */
	BUG_ON(NULL == pXferDesc);

	/* Get the corresponding USB device object. */
	pPhyDev = pXferDesc->pPhyDev;
	BUG_ON(NULL == pPhyDev);
	dev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	BUG_ON(NULL == dev);

	/* Stop xfer according pipe type. */
	pipe = pXferDesc->pipe;
	if (usb_pipetype(pipe) == PIPE_BULK)
	{
		/* Bulk xfer. */
		pAsyncDesc = (__USB_ASYNC_DESCRIPTOR*)pXferDesc->priv;
		BUG_ON(NULL == pAsyncDesc);
		usbDestroyAsyncDescriptor(pAsyncDesc);
		goto __TERMINAL;
	}
	else if (usb_pipetype(pipe) == PIPE_CONTROL)
	{
		/* Control xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_INTERRUPT)
	{
		/* Interrupt xfer. */
	}
	else if (usb_pipetype(pipe) == PIPE_ISOCHRONOUS)
	{
		/* Isochronous xfer. */
	}
	else
	{
		BUG();
	}

__TERMINAL:
	return;
}

/* 
 * Create a common USB controller and initialize it according to EHCI.
 */
static __COMMON_USB_CONTROLLER* CreateUsbCtrl(__PHYSICAL_DEVICE* pPhyDev,LPVOID pCtrl)
{
	__USB_CONTROLLER_OPERATIONS ctrlOps;

	ctrlOps.submit_bulk_msg = submit_bulk_msg;
	ctrlOps.submit_control_msg = submit_control_msg;
	ctrlOps.submit_int_msg = submit_int_msg;
	ctrlOps.create_int_queue = NULL;
	ctrlOps.destroy_int_queue = NULL;
	ctrlOps.poll_int_queue = NULL;
	ctrlOps.usb_reset_root_port = NULL;
	ctrlOps.get_ctrl_status = _get_ctrl_status;
	ctrlOps.InterruptHandler = EHCIIntHandler;

	ctrlOps.CreateXferDescriptor = _ehciCreateXferDescriptor;
	ctrlOps.StartXfer = _ehciStartXfer;
	ctrlOps.StopXfer = _ehciStopXfer;
	ctrlOps.DestroyXferDescriptor = _ehciDestroyXferDescriptor;

	return USBManager.CreateUsbCtrl(&ctrlOps,USB_CONTROLLER_EHCI,pPhyDev,pCtrl);
}

//Controller initialization routine called by usb_init routine.
int _ehci_usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	struct ehci_ctrl *ctrl = &ehcic[index];
	uint tweaks = 0;
	__PHYSICAL_DEVICE* pUsbCtrl = NULL;
	int rc = -1;

	/**
	* Set ops to default_ehci_ops, ehci_hcd_init should call
	* ehci_set_controller_priv to change any of these function pointers.
	*/
	ctrl->ops = default_ehci_ops;
	/* Init spin lock in SMP. */
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(ctrl->spin_lock, "ehci");
#endif

	//rc = ehci_hcd_init(index, init, &ctrl->hccr, &ctrl->hcor);
	//if (rc)
	//	return rc;
	pUsbCtrl = ehci_hcd_init(index, init, &ctrl->hccr, &ctrl->hcor);
	if (NULL == pUsbCtrl)
	{
		//_hx_printf("%s: ehci_hcd_init failed.\r\n", __func__);
		goto done;
	}

	if (init == USB_INIT_DEVICE)
	{
		rc = 0;
		goto done;
	}

	/* EHCI spec section 4.1 */
	if (ehci_reset(ctrl))
	{
		_hx_printf("%s:can not reset EHCI controller [%d].\r\n", __func__, index);
		goto done;
	}

	*controller = CreateUsbCtrl(pUsbCtrl,&ehcic[index]);
	//*controller = &ehcic[index];
	if (NULL == *controller) //Create USB common controller failed.
	{
		_hx_printf("%s:can not create USB Controller object.\r\n", __func__);
		goto done;
	}

#if defined(CONFIG_EHCI_HCD_INIT_AFTER_RESET)
	//rc = ehci_hcd_init(index, init, &ctrl->hccr, &ctrl->hcor);
	//if (rc)
	//	return rc;
#endif

#ifdef CONFIG_USB_EHCI_FARADAY
	tweaks |= EHCI_TWEAK_NO_INIT_CF;
#endif
	rc = ehci_common_init(ctrl, tweaks);
	if (rc)
	{
		_hx_printf("%s:ehci_common_init failed with error [%d].\r\n", rc);
		goto done;
	}

	ctrl->rootdev = 0;
done:
	return rc;
}
#endif

static int _ehci_submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length)
{
	if (usb_pipetype(pipe) != PIPE_BULK) {
		debug("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}
	return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

static int _ehci_submit_control_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length,struct devrequest *setup)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);

	if (usb_pipetype(pipe) != PIPE_CONTROL) {
		debug("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}

	if (usb_pipedevice(pipe) == ctrl->rootdev) {
		if (!ctrl->rootdev)
			dev->speed = USB_SPEED_HIGH;
		return ehci_submit_root(dev, pipe, buffer, length, setup);
	}
	return ehci_submit_async(dev, pipe, buffer, length, setup);
}

/*
 * Enable asynchronous transfer,set the ASYNCLISTADDR register
 * before actually enable it.
 */
int ehci_enable_async(struct ehci_ctrl* ctrl,struct QH* qh)
{
	int ret = -EIO;
	uint32_t cmd, usbsts;

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, (unsigned long)qh);

	usbsts = ehci_readl(&ctrl->hcor->or_usbsts);
	ehci_writel(&ctrl->hcor->or_usbsts, (usbsts & 0x3f));

	/* Enable async. schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd |= CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, STS_ASS,
		100 * 1000);
	if (ret < 0) {
		_hx_printf("EHCI fail timeout STS_ASS set\r\n");
		goto __TERMINAL;
	}
	ret = 0;  //Mark operation success.

__TERMINAL:
	return ret;
}

int ehci_disable_async(struct ehci_ctrl* ctrl)
{
	uint32_t cmd;
	int ret = -EIO;

	/* Disable async schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, 0,
		100 * 1000);
	if (ret < 0) {
		_hx_printf("EHCI fail timeout STS_ASS reset\r\n");
		goto __TERMINAL;
	}
	/* Mark the operation success. */
	ret = 0;

__TERMINAL:
	return ret;
}

int ehci_enable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |= CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
		STS_PSS, STS_PSS, 100 * 1000);
	if (ret < 0) {
		_hx_printf("EHCI failed: timeout when enabling periodic list\r\n");
		return -ETIMEDOUT;
	}
	udelay(USB_DEFAULT_SETTLE_TIME);
	return 0;
}

int ehci_disable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd &= ~CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
		STS_PSS, 0, 100 * 1000);
	if (ret < 0) {
		printf("EHCI failed: timeout when disabling periodic list\r\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static int _ehci_submit_int_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length, int interval)
{
	void *backbuffer;
	struct int_queue *queue;
	int result = 0, ret;

	debug("  %s: dev=%p, pipe=%lu, buffer=%p, length=%d, interval=%d.\r\n",__func__,
		dev, pipe, buffer, length, interval);

	queue = EHCICreateIntQueue(dev, pipe, 1, length, buffer, interval);
	if (!queue)
	{
		_hx_printf("%s: create int queue failed.\r\n", __func__);
		return -1;
	}

	backbuffer = EHCIPollIntQueue(dev, queue);

	if (NULL == backbuffer) {
		debug("%s:EHCIPollIntQueue failed,queue status = %X.\r\n", __func__,
			queue->dwStatus);
		result = -1;
	}

	ret = EHCIDestroyIntQueue(dev, queue);
	if (ret < 0)
	{
		_hx_printf("%s: EHCIDestroyIntQueue failed,ret = %d.\r\n", __func__, ret);
		result = ret;
	}

	return result;
}

static int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length)
{
	struct ehci_ctrl* ctrl = ehci_get_ctrl(dev);
	int ret = 0;
	//WaitForThisObject(ctrl->hMutex);
	ret = _ehci_submit_bulk_msg(dev, pipe, buffer, length);
	//ReleaseMutex(ctrl->hMutex);
	return ret;
}

static int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
	int length, struct devrequest *setup)
{
	struct ehci_ctrl* ctrl = ehci_get_ctrl(dev);
	int ret = 0;
	//WaitForThisObject(ctrl->hMutex);
	ret = _ehci_submit_control_msg(dev, pipe, buffer, length, setup);
	//ReleaseMutex(ctrl->hMutex);
	return ret;
}

static int submit_int_msg(struct usb_device *dev, unsigned long pipe,
	void *buffer, int length, int interval)
{
	return _ehci_submit_int_msg(dev, pipe, buffer, length, interval);
}

static struct int_queue *create_int_queue(struct usb_device *dev,
	unsigned long pipe, int queuesize, int elementsize,
	void *buffer, int interval)
{
	return EHCICreateIntQueue(dev, pipe, queuesize, elementsize,
		buffer, interval);
}

static void *poll_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	return EHCIPollIntQueue(dev, queue);
}

static int destroy_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	return EHCIDestroyIntQueue(dev, queue);
}
#endif  //CONFIG_USB_EHCI
