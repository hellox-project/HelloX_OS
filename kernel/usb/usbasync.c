//***********************************************************************/
//    Author                    : Garry
//    Original Date             : April 21, 2017
//    Module Name               : usbasync.c
//    Module Funciton           : 
//                                Source code of USB asynchronous transfer functions.
//                                The USB asynchronous xfer include control
//                                transfer and bulk transfer.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <pci_drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <align.h>

#include "usbdescriptors.h"
#include "usb_defs.h"
#include "ch9.h"
#include "usb.h"
#include "ehci.h"
#include "usbasync.h"
#include "errno.h"

/*
* Mark a asynchronous xfer inactive(i.e,pause to advance by controller),or
* activate it,according the bInactive parameter.
*/
static void MarkDescriptorInactive(__USB_ASYNC_DESCRIPTOR* pAsyncDesc, BOOL bInactive)
{
	int nqtd = 0;
	struct qTD* ptd = NULL;
	int i = 0;
	uint32_t token;

	BUG_ON(NULL == pAsyncDesc);
	ptd = pAsyncDesc->pFirstTD;
	BUG_ON(NULL == ptd);

	/* Total qTD number associated to this descriptor. */
	nqtd = pAsyncDesc->qtd_counter;

	if (bInactive)  //Should mark the descriptor inactive.
	{
		/*
		* Halt the QH to let HC skip this one,so as to modify the qTD(s)
		* linked to this QH.
		*/
		pAsyncDesc->QueueHead->qh_overlay.qt_token |= QT_TOKEN_STATUS(QT_TOKEN_STATUS_HALTED);
		__BARRIER();
		flush_dcache_range(pAsyncDesc->QueueHead, pAsyncDesc->QueueHead + 1);
		for (i = 0; i < nqtd; i++)
		{
			token = hc32_to_cpu(ptd[i].qt_token);
			token &= ~0xFF; //QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);  //Clear the active bit.
			ptd[i].qt_token = cpu_to_hc32(token);
			flush_dcache_range(&ptd[i], &ptd[i] + 1);
		}
	}
	else //Restart the descriptor.
	{
		for (i = 0; i < nqtd; i++)
		{
			token = hc32_to_cpu(ptd[i].qt_token);
			token &= ~0xFF;  //Clear all status bits.
			token |= QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);  //Set the active bit.
			ptd[i].qt_token = cpu_to_hc32(token);
			flush_dcache_range(&ptd[i], &ptd[i] + 1);
		}
		/* Update QH overlay to active. */
		pAsyncDesc->QueueHead->qh_overlay.qt_next = cpu_to_hc32((unsigned long)ptd);
		pAsyncDesc->QueueHead->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		__BARRIER();
		pAsyncDesc->QueueHead->qh_overlay.qt_token = 0;
		flush_dcache_range(pAsyncDesc->QueueHead, pAsyncDesc->QueueHead + 1);
	}
}

/*
* Helper routine to insert a Queue Head into EHCI queue head list.
*/
static BOOL AddQueueHead(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	struct ehci_ctrl* ctrl = NULL;
	struct QH* pQueueHead = NULL;
	struct QH* pCurrQH = NULL;
	DWORD dwFlags;

	BUG_ON(NULL == pAsyncDesc);
	BUG_ON(NULL == pAsyncDesc->pCtrl);

	ctrl = (struct ehci_ctrl*)pAsyncDesc->pCtrl->pUsbCtrl;
	BUG_ON(NULL == ctrl);
	pQueueHead = pAsyncDesc->QueueHead;

	/*
	* Use critical section to prevent this operation,since it maybe
	* invoked in interrupt context.
	*/
	__ENTER_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);
	pCurrQH = &ctrl->qh_list;
	pQueueHead->qh_link = cpu_to_hc32((unsigned long)pCurrQH->qh_link | QH_LINK_TYPE_QH);
	pCurrQH->qh_link = cpu_to_hc32((unsigned long)pQueueHead | QH_LINK_TYPE_QH);
	flush_dcache_range((unsigned long)pQueueHead, ALIGN_END_ADDR(struct QH, pQueueHead, 1));
	flush_dcache_range((unsigned long)pCurrQH, ALIGN_END_ADDR(struct QH, pCurrQH, 1));
	__LEAVE_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);

	//Start asynchronous schedule if not yet.
	WaitForThisObject(ctrl->hMutex);
	if (ctrl->async_schedules == 0)
	{
		if (ehci_enable_async(ctrl, /*pAsyncDesc->QueueHead*/ &ctrl->qh_list) < 0)
		{
			_hx_printf("%s: enable async_xfer failed.\r\n", __func__);
			ReleaseMutex(ctrl->hMutex);
			return FALSE;
		}
#ifdef __DEBUG_USB_ASYNC
		_hx_printf("%s: async_xfer enabled.\r\n", __func__);
#endif
	}
	ctrl->async_schedules++;
	ReleaseMutex(ctrl->hMutex);

	return TRUE;
}

/*
* EHCI controller asynchronous schedule door bell mechanism.
* This mechanism should be applied after one queue head element removed
* from controller's qh list.
* This mechanism makes sure that once one QH element is removed from list,
* the internal state of controller can reach a consistant state,since it
* may keep local copy of queue element.
*/
static int _async_door_bell(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	uint32_t usb_cmd = 0;
	uint32_t usb_sts = 0;
	struct ehci_ctrl* pCtrl = NULL;
	int ret = 0;

	if (NULL == pAsyncDesc)
	{
		BUG();
	}
	pCtrl = (struct ehci_ctrl*)pAsyncDesc->pCtrl->pUsbCtrl;

	/* Access controller excusively. */
	WaitForThisObject(pCtrl->hMutex);
	if (pCtrl->async_schedules == 0)  /* Async schedule disabled. */
	{
		ReleaseMutex(pCtrl->hMutex);
		return 0;
	}

	/*
	* Clear IAA bit in status register.
	* Write 1(not 0) to clear the status bit.
	*/
	usb_sts = STS_IAA;
	ehci_writel(&pCtrl->hcor->or_usbsts, usb_sts);

	/* Set IAAD bit. */
	usb_cmd = ehci_readl(&pCtrl->hcor->or_usbcmd);
	usb_cmd |= CMD_IAAD;
	ehci_writel(&pCtrl->hcor->or_usbcmd, usb_cmd);

	/* Hand shake with IAA bit in status register. */
	ret = handshake((uint32_t *)&pCtrl->hcor->or_usbsts, STS_IAA, STS_IAA,
		100 * 1000);
	if (ret < 0) {
		_hx_printf("%s: EHCI handshake time out of door bell[err = %d].\r\n",
			__func__,
			ret);
		ReleaseMutex(pCtrl->hMutex);
		return ret;
	}
	/* Handshake OK. */
	ReleaseMutex(pCtrl->hMutex);
	return 0;
}

/*
* Helper routine to remove a queue head from EHCI queue head list.
*/
static BOOL RemoveQueueHead(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	struct ehci_ctrl* ctrl = NULL;
	struct QH* pCurrQH = NULL;
	struct QH* pPrevQH = NULL;
	DWORD dwFlags;

	/*
	* Set to TRUE if can not find the QH to be delete in controller's
	* QH list.
	*/
	BOOL bNoFind = FALSE;

	if (NULL == pAsyncDesc)
	{
		BUG();
	}
	if (NULL == pAsyncDesc->pCtrl)
	{
		BUG();
	}
	ctrl = (struct ehci_ctrl*)pAsyncDesc->pCtrl->pUsbCtrl;
	if (NULL == ctrl)
	{
		BUG();
	}
	pPrevQH = &ctrl->qh_list;
	pCurrQH = pAsyncDesc->QueueHead;
	/*
	* Critical section protect the operation since it maybe invoked in
	* interrupt context.
	*/
	__ENTER_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);
	while (((unsigned long)pPrevQH->qh_link & ~QH_LINK_TYPEMASK) != ((unsigned long)pCurrQH & ~QH_LINK_TYPEMASK))
	{
		pPrevQH = (struct QH*)((unsigned long)pPrevQH->qh_link & ~QH_LINK_TYPEMASK);
		if (((unsigned long)pPrevQH & ~QH_LINK_TYPEMASK) == ((unsigned long)&ctrl->qh_list & ~QH_LINK_TYPEMASK))
		{
			/*
			* Can not find the queue head to be remove.
			*/
			bNoFind = TRUE;
			break;
		}
	}
	if (!bNoFind)
	{
		/* Remove the queue head. */
		pPrevQH->qh_link = pCurrQH->qh_link;
		flush_dcache_range((unsigned long)pPrevQH, ALIGN_END_ADDR(struct QH, pPrevQH, 1));
	}
	__LEAVE_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);

	if (!bNoFind)
	{
		/* Apply door bell mechanism according EHCI spec. */
		_async_door_bell(pAsyncDesc);

		//Disable asynchronous schedule if no more xfer requirement.
		WaitForThisObject(ctrl->hMutex);
		if (ctrl->async_schedules > 0)
		{
			ctrl->async_schedules--;
			if (0 == ctrl->async_schedules)
			{
				ehci_disable_async(ctrl);
#ifdef __DEBUG_USB_ASYNC
				_hx_printf("EHCI async_xfer stoped.\r\n");
#endif
			}
		}
		ReleaseMutex(ctrl->hMutex);
#ifdef __DEBUG_USB_ASYNC
		_hx_printf("%s: stop async_xfer success.\r\n", __func__);
#endif
	}
	if (bNoFind)
	{
		_hx_printf("%s: can not find the qTD to remove.\r\n", __func__);
	}
	return (!bNoFind);
}

/*
* This routine is in usb_ehci.c file and will be refered
* in this file.
*/
extern int ehci_td_buffer(struct qTD *td, void *buf, size_t sz);

/*
* Helper routine to construct asynchronous xfer data structures,such as
* qTD,queue head,by giving the asynchronous xfer descriptor.
*/
static BOOL ConstructAsyncXferData(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	struct usb_device *dev = pAsyncDesc->pUsbDev;
	unsigned long pipe = pAsyncDesc->pipe;
	void *buffer = pAsyncDesc->buffer;
	int length = pAsyncDesc->bufflength;
	struct devrequest *req = pAsyncDesc->req;
	struct QH* qh = pAsyncDesc->QueueHead;
	BOOL bLast_qTD = FALSE;

	struct qTD *qtd = NULL;
	int qtd_count = 0;
	int qtd_counter = 0;
	volatile struct qTD *vtd = NULL;
	uint32_t *tdp;
	uint32_t endpt, maxpacket, token;
	uint32_t c, toggle;
	int ret = 0;
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);
	BOOL bResult = FALSE;

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, qh=%X, req=%p\r\n", dev, pipe,
		buffer, length, qh, req);
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
		_hx_printf("%s: unable to allocate TDs.\r\n", __func__);
		goto __TERMINAL;
	}

	memset(qh, 0, sizeof(struct QH));
	memset(qtd, 0, qtd_count * sizeof(*qtd));

	/*
	* Save the qTD information into asynchronous descriptor.
	*/
	pAsyncDesc->num_qtd = qtd_count;
	pAsyncDesc->pFirstTD = qtd;
	pAsyncDesc->pLastTD = &qtd[qtd_count - 1];

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
	//endpt = QH_ENDPT1_RL(8) | QH_ENDPT1_C(c) |
	endpt = QH_ENDPT1_RL(0x0F) | QH_ENDPT1_C(c) |
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
			QT_TOKEN_PID(QT_TOKEN_PID_SETUP);
		// | QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], req, sizeof(*req))) {
			_hx_printf("unable to construct SETUP TD.\r\n");
			goto __TERMINAL;
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
			* Check if the qTD is last one in a list.
			* The IOC bit in token of this qTD should be set if
			* this is the last qTD.
			*/
			if (xfr_bytes >= left_length)
			{
				bLast_qTD = TRUE;
			}
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
				//QT_TOKEN_IOC(req == NULL) | QT_TOKEN_CPAGE(0) |
				QT_TOKEN_IOC(bLast_qTD && (req == NULL) ? 1 : 0) | QT_TOKEN_CPAGE(0) |
				QT_TOKEN_CERR(3) |
				QT_TOKEN_PID(usb_pipein(pipe) ?
			QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT);
			// | QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
			qtd[qtd_counter].qt_token = cpu_to_hc32(token);
			if (ehci_td_buffer(&qtd[qtd_counter], buf_ptr,
				xfr_bytes)) {
				_hx_printf("%s: unable to construct DATA TD.\r\n", __func__);
				goto __TERMINAL;
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
		QT_TOKEN_PID_OUT : QT_TOKEN_PID_IN);
		// | QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((unsigned long)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
	}

	/* Save the qTD counter actually used. */
	pAsyncDesc->qtd_counter = qtd_counter;
	/*
	* Link the queue head struct into controller's queue head list.
	* Mark it inactive before do it.
	*/
	MarkDescriptorInactive(pAsyncDesc, TRUE);
	AddQueueHead(pAsyncDesc);

	/* Flush dcache */
	flush_dcache_range((unsigned long)&ctrl->qh_list,
		ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
	flush_dcache_range((unsigned long)qh, ALIGN_END_ADDR(struct QH, qh, 1));
	flush_dcache_range((unsigned long)qtd,
		ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

	/* Mark the operation successful. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (qtd) /* Should release it. */
		{
			_hx_free(qtd);
		}
		/* Reset qTD related information of descriptor. */
		pAsyncDesc->num_qtd = 0;
		pAsyncDesc->pFirstTD = NULL;
		pAsyncDesc->pLastTD = NULL;
	}
	return bResult;
}

/*
* Interrupt handler of asynchronous xfer.
* Please be note that the interrupt may not raised by asynchronous xfer,
* and may not by the current xfer descriptor even if raised by asynchronous xfer,
* so the handler must handle these scenarios.
*/
static BOOL AsyncXferIntHandler(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	uint32_t token = 0, toggle = 0;
	int qtd_counter = 0;
	volatile struct qTD* vtd = NULL;

	if (NULL == pAsyncDesc)
	{
		BUG();
	}
	/*
	* The descriptor already complete.
	*/
	if (USB_ASYNCDESC_STATUS_COMPLETED == pAsyncDesc->status)
	{
		return FALSE;
	}

	/*
	* The descriptor is just linked into EHCI controller's global list but
	* not enabled.
	*/
	if (USB_ASYNCDESC_STATUS_INITIALIZED == pAsyncDesc->status)
	{
		return FALSE;
	}

	/*
	* If the descriptor already timeout before the interrupt raised.
	*/
	if (pAsyncDesc->status == USB_ASYNCDESC_STATUS_TIMEOUT)
	{
		return FALSE;
	}

	/*
	* The xfer descriptor is canceled.
	*/
	if (USB_ASYNCDESC_STATUS_CANCELED == pAsyncDesc->status)
	{
		return FALSE;
	}

	/*
	* Current descriptor is in process of waiting...
	*/
	if (pAsyncDesc->status == USB_ASYNCDESC_STATUS_INPROCESS)
	{
		qtd_counter = pAsyncDesc->qtd_counter;
		/* Just check the last qTD in list. */
		vtd = &pAsyncDesc->pFirstTD[qtd_counter - 1];
		invalidate_dcache_range(vtd, vtd + 1);
		token = hc32_to_cpu(vtd->qt_token);
		/*
		* If the status bit of the last qTD was cleared,it means
		* the queue head is processed over,then quit.
		*/
		if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE))
		{
			SetEvent(pAsyncDesc->hEvent);  //Wake up the pending thread.
			return TRUE;
		}
		else {  /* The interrupt is not raised by this desc. */
			return FALSE;
		}
	}
	/*
	* All other value of status may caused by internal error.
	*/
	//BUG();
	return FALSE;
}

//Create a USB asychronous transfer descriptor and install it into system.
__USB_ASYNC_DESCRIPTOR* usbCreateAsyncDescriptor(struct usb_device* pUsbDev, unsigned int pipe,
	char* buffer, int bufflength, struct devrequest* req)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	DWORD dwFlags;
	BOOL bResult = FALSE;

	if (NULL == pUsbDev)
	{
		goto __TERMINAL;
	}
	pCommCtrl = (__COMMON_USB_CONTROLLER*)pUsbDev->controller;
	BUG_ON(NULL == pCommCtrl);
	pEhciCtrl = (struct ehci_ctrl*)pCommCtrl->pUsbCtrl;
	BUG_ON(NULL == pEhciCtrl);

	/* Data buffer too long. */
	if (bufflength > USB_ASYNC_MAX_LENGTH)
	{
		_hx_printf("%s:buffer too long[%d],cann't exceed[%d].\r\n",
			__func__,
			bufflength,
			USB_ASYNC_MAX_LENGTH);
		goto __TERMINAL;
	}

	/*
	* Allocate a new descriptor object and initialize it.
	*/
	pAsyncDesc = (__USB_ASYNC_DESCRIPTOR*)_hx_malloc(sizeof(__USB_ASYNC_DESCRIPTOR));
	if (NULL == pAsyncDesc)
	{
		goto __TERMINAL;
	}
	memset(pAsyncDesc, 0, sizeof(__USB_ASYNC_DESCRIPTOR));

	pAsyncDesc->AsyncXferIntHandler = AsyncXferIntHandler;
	pAsyncDesc->buffer = buffer;
	pAsyncDesc->bufflength = bufflength;
	pAsyncDesc->reqlength = bufflength;
	pAsyncDesc->hEvent = CreateEvent(FALSE);
	if (NULL == pAsyncDesc->hEvent)
	{
		goto __TERMINAL;
	}
	pAsyncDesc->pCtrl = (__COMMON_USB_CONTROLLER*)pUsbDev->controller;
	pAsyncDesc->pipe = pipe;
	//pAsyncDesc->pPhyDev = pPhyDev;
	pAsyncDesc->pUsbDev = pUsbDev;
	pAsyncDesc->req = req;
	pAsyncDesc->QueueHead = (struct QH*)_hx_aligned_malloc(sizeof(struct QH), USB_DMA_MINALIGN);
	if (NULL == pAsyncDesc->QueueHead)
	{
		goto __TERMINAL;
	}
	pAsyncDesc->status = USB_ASYNCDESC_STATUS_INITIALIZED;

	/*
	* Construct the async xfer data structure between host and controller.
	*/
	if (!ConstructAsyncXferData(pAsyncDesc))
	{
		ReleaseMutex(pEhciCtrl->hMutex);
		goto __TERMINAL;
	}
	//MarkDescriptorInactive(pAsyncDesc, TRUE);  //Disable it initially.

	/*
	* Link the async descriptor into global list.
	*/
	__ENTER_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);
	if (NULL == pEhciCtrl->pAsyncDescFirst)  //First element.
	{
		pEhciCtrl->pAsyncDescFirst = pAsyncDesc;
		pEhciCtrl->pAsyncDescLast = pAsyncDesc;
		pAsyncDesc->pNext = NULL;
	}
	else  //Put it at last.
	{
		if (NULL == pEhciCtrl->pAsyncDescLast)
		{
			BUG();
		}
		pEhciCtrl->pAsyncDescLast->pNext = pAsyncDesc;
		pEhciCtrl->pAsyncDescLast = pAsyncDesc;
		pAsyncDesc->pNext = NULL;
	}
	__LEAVE_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);

	/* Mark the creation success. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		//Should release all kind of resource.
		if (pAsyncDesc)
		{
			if (pAsyncDesc->hEvent)
			{
				DestroyEvent(pAsyncDesc->hEvent);
			}
			if (pAsyncDesc->QueueHead)
			{
				_hx_free(pAsyncDesc->QueueHead);
			}
			_hx_free(pAsyncDesc);
			pAsyncDesc = NULL;
		}
#ifdef __DEBUG_USB_ASYNC
		_hx_printf("%s: create async_desc failed.\r\n", __func__);
#endif
	}
#ifdef __DEBUG_USB_ASYNC
	else
	{
		_hx_printf("%s: create async_desc OK[buff_len = %d,n_qtd = %d].\r\n",
			__func__,
			pAsyncDesc->bufflength,
			pAsyncDesc->num_qtd);
	}
#endif
	return pAsyncDesc;
}

/*
* Local helper routine to update a qTD according given buffer length.
* User can change xfer data buffer's length before start a async xfer,
* the new buffer length can not exceed the original buffer length
* that is designated when create asynchronous descriptor.
* Xfer buffer's base address can not be changed.
*/
static BOOL Update_qTD(__USB_ASYNC_DESCRIPTOR* pAsyncDesc, int new_size)
{
	uint32_t token = 0;
	uint32_t toggle = 0;
	struct qTD* ptd = NULL;
	int xfr_bytes = new_size;
	BOOL bResult = FALSE;
	struct ehci_ctrl* ctrl = (struct ehci_ctrl*)pAsyncDesc->pCtrl->pUsbCtrl;

	ptd = &pAsyncDesc->pFirstTD[pAsyncDesc->qtd_counter - 1];
	toggle = usb_gettoggle(pAsyncDesc->pUsbDev, usb_pipeendpoint(pAsyncDesc->pipe),
		usb_pipeout(pAsyncDesc->pipe));

	/* Set the halt bit in overlay of QH to stop the QH. */
	//pAsyncDesc->QueueHead->qh_overlay.qt_token = QT_TOKEN_STATUS(QT_TOKEN_STATUS_HALTED);

	token = QT_TOKEN_DT(toggle) |
		QT_TOKEN_TOTALBYTES(xfr_bytes) |
		//QT_TOKEN_IOC(req == NULL) | QT_TOKEN_CPAGE(0) |
		QT_TOKEN_IOC(1) | QT_TOKEN_CPAGE(0) |
		QT_TOKEN_CERR(3) |
		QT_TOKEN_PID(usb_pipein(pAsyncDesc->pipe) ?
	QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT);
	// | QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE

	ptd->qt_token = cpu_to_hc32(token);
	if (ehci_td_buffer(ptd, pAsyncDesc->buffer, xfr_bytes)) {
		_hx_printf("%s: unable to construct DATA TD.\r\n", __func__);
		goto __TERMINAL;
	}
	flush_dcache_range(ptd, ptd + 1);

	/* Update queue head overlay area accordingly. */
	//memset(&pAsyncDesc->QueueHead->qh_overlay, 0, sizeof(struct qTD));
	//pAsyncDesc->QueueHead->qh_overlay.qt_next = cpu_to_hc32((unsigned long)ptd);
	//pAsyncDesc->QueueHead->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	//flush_dcache_range(pAsyncDesc->QueueHead, pAsyncDesc->QueueHead + 1);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
* Start asynchronous xfer request,the caller
* can change the xfer size with new_size,but the
* data buffer can not be changed.
*/
BOOL usbStartAsyncXfer(__USB_ASYNC_DESCRIPTOR* pAsyncDesc, int new_size)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct usb_device* pUsbDev = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	DWORD dwResult = 0;
	BOOL bResult = FALSE;
	DWORD dwFlags;
	uint32_t token = 0, status = 0, toggle = 0;
	volatile struct qTD* vtd = NULL;
	int qtd_counter = 0;

	if (NULL == pAsyncDesc)
	{
		goto __TERMINAL;
	}

	pCommCtrl = pAsyncDesc->pCtrl;
	pUsbDev = pAsyncDesc->pUsbDev;
	BUG_ON((NULL == pCommCtrl) || (NULL == pUsbDev));
	pEhciCtrl = pCommCtrl->pUsbCtrl;
	BUG_ON(NULL == pEhciCtrl);
	BUG_ON(NULL == pAsyncDesc->hEvent);

	/* Xfer buffer length changed if new_size is not 0. */
	if (new_size)
	{
		/* Only bulk xfer's buffer length can be changed. */
		if (pAsyncDesc->req != NULL)
		{
			_hx_printf("%s:buff length can not be changed[req != NULL].\r\n"
				__func__);
			goto __TERMINAL;
		}
		if (new_size > pAsyncDesc->bufflength)
		{
			_hx_printf("%s:buff length too long[%d],can not exceed[%d].\r\n",
				__func__,
				new_size,
				pAsyncDesc->bufflength);
			goto __TERMINAL;
		}
		if (!Update_qTD(pAsyncDesc, new_size))
		{
			_hx_printf("%s:update qTD failed.\r\n", __func__);
			goto __TERMINAL;
		}
		pAsyncDesc->reqlength = new_size;
	}
	else
	{
		/*
		* Should update qTD also,since toggle bit changed after
		* previous xfer.
		*/
		if (NULL == pAsyncDesc->req)
		{
			Update_qTD(pAsyncDesc, pAsyncDesc->bufflength);
		}
	}

	/*
	* Set event to unsignal,it will be released in
	* IOC interrupt.
	*/
	ResetEvent(pAsyncDesc->hEvent);

	/*
	* Set descriptor's status to INPROCESS,and mark the
	* qh active.
	* Should be in atomic.
	*/
	__ENTER_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);
	pAsyncDesc->status = USB_ASYNCDESC_STATUS_INPROCESS;
	MarkDescriptorInactive(pAsyncDesc, FALSE);
	pEhciCtrl->nXferReqNum++;
	__LEAVE_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);

#ifdef __DEBUG_USB_ASYNC
	_hx_printf("%s: start async_xfer...\r\n", __func__);
#endif

	/*
	* Wait the transaction to be finished.
	*/
	dwResult = WaitForThisObjectEx(pAsyncDesc->hEvent, 25000, NULL);
	switch (dwResult)
	{
	case OBJECT_WAIT_RESOURCE:
		invalidate_dcache_range(pAsyncDesc->QueueHead, pAsyncDesc->QueueHead + 1);
		token = cpu_to_le32(pAsyncDesc->QueueHead->qh_overlay.qt_token);
		debug("%s: TOKEN=%#x\r\n", __func__, token);
		switch (QT_TOKEN_GET_STATUS(token) &
			~(QT_TOKEN_STATUS_SPLITXSTATE | QT_TOKEN_STATUS_PERR)) {
		case 0:
			toggle = QT_TOKEN_GET_DT(token);
			usb_settoggle(pAsyncDesc->pUsbDev, usb_pipeendpoint(pAsyncDesc->pipe),
				usb_pipeout(pAsyncDesc->pipe), toggle);
			pAsyncDesc->status = USB_ASYNCDESC_STATUS_COMPLETED;
			pAsyncDesc->err_code = 0;
			break;
		case QT_TOKEN_STATUS_HALTED:
		case QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_STATUS_DATBUFERR:
		case QT_TOKEN_STATUS_DATBUFERR:
		case QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET:
		case QT_TOKEN_STATUS_BABBLEDET:
		default:
			pAsyncDesc->status = USB_ASYNCDESC_STATUS_ERROR;
			pAsyncDesc->err_code = QT_TOKEN_GET_STATUS(token);
			break;
		}
		/*
		* Accumulate the total xfer bytes.
		*/
		pAsyncDesc->xfersize = pAsyncDesc->reqlength;
		for (int i = 0; i < pAsyncDesc->qtd_counter; i++)
		{
			token = pAsyncDesc->pFirstTD[i].qt_token;
			pAsyncDesc->xfersize -= QT_TOKEN_GET_TOTALBYTES(token);
		}
		return
			((USB_ASYNCDESC_STATUS_COMPLETED == pAsyncDesc->status) ? TRUE : FALSE);
	case OBJECT_WAIT_TIMEOUT:
		/*
		* Check qTD's status again,since the transaction may processed
		* over after the current kernel thread was woken up but not scheduled
		* yet.
		*/
		qtd_counter = pAsyncDesc->qtd_counter;
		vtd = &pAsyncDesc->pFirstTD[qtd_counter - 1];
		invalidate_dcache_range(vtd, vtd + 1);
		token = cpu_to_le32(vtd->qt_token);
#ifdef __DEBUG_USB_ASYNC
		_hx_printf("Timeout: owner/tk/pip = %s/0x%X/0x%X\r\n",
			((__KERNEL_THREAD_OBJECT*)(pAsyncDesc->hOwnerThread))->KernelThreadName,
			token, pAsyncDesc->pipe);
		_hx_printf("Overlay:%X/%X/%X/%X/%X/%X/%X\r\n",
			pAsyncDesc->QueueHead->qh_link,
			pAsyncDesc->QueueHead->qh_endpt1,
			pAsyncDesc->QueueHead->qh_endpt2,
			pAsyncDesc->QueueHead->qh_curtd,
			pAsyncDesc->QueueHead->qh_overlay.qt_next,
			pAsyncDesc->QueueHead->qh_overlay.qt_altnext,
			pAsyncDesc->QueueHead->qh_overlay.qt_token);
#endif
		if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE))
		{
			switch (QT_TOKEN_GET_STATUS(token))
			{
			case 0:
				toggle = QT_TOKEN_GET_DT(token);
				usb_settoggle(pAsyncDesc->pUsbDev, usb_pipeendpoint(pAsyncDesc->pipe),
					usb_pipeout(pAsyncDesc->pipe), toggle);
				pAsyncDesc->status = USB_ASYNCDESC_STATUS_COMPLETED;
				pAsyncDesc->err_code = 0;
				break;
			case QT_TOKEN_STATUS_HALTED:
			case QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_STATUS_DATBUFERR:
			case QT_TOKEN_STATUS_DATBUFERR:
			case QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET:
			case QT_TOKEN_STATUS_BABBLEDET:
			default:
				pAsyncDesc->status = USB_ASYNCDESC_STATUS_ERROR;
				pAsyncDesc->err_code = QT_TOKEN_GET_STATUS(token);
				break;
			}
			/*
			* Accumulate the total xfer bytes.
			*/
			pAsyncDesc->xfersize = pAsyncDesc->reqlength;
			for (int i = 0; i < pAsyncDesc->qtd_counter; i++)
			{
				token = pAsyncDesc->pFirstTD[i].qt_token;
				pAsyncDesc->xfersize -= QT_TOKEN_GET_TOTALBYTES(token);
			}
			return
				((USB_ASYNCDESC_STATUS_COMPLETED == pAsyncDesc->status) ? TRUE : FALSE);
		}
		/*
		* The last qTD still alive and wait to be processed.
		* No need to wait more.
		*/
		MarkDescriptorInactive(pAsyncDesc, TRUE);
		pAsyncDesc->status = USB_ASYNCDESC_STATUS_TIMEOUT;
		pAsyncDesc->err_code = QT_TOKEN_GET_STATUS(token);
		return FALSE;
	case OBJECT_WAIT_FAILED:
		MarkDescriptorInactive(pAsyncDesc, TRUE);  //Disable the xfer.
		pAsyncDesc->status = USB_ASYNCDESC_STATUS_CANCELED;
		return FALSE;
	case OBJECT_WAIT_DELETED:
		BUG(); /* Should no code destroy the desc in process of waiting... */
		break;
	default:
		_hx_printf("%s:unexcepted timeout waiting's return value[%d].\r\n",
			__func__, dwResult);
		BUG();
		break;
	}

__TERMINAL:
	return bResult;
}

/*
* Stop USB asynchronous transfer.
* It's just try to stop the async schedule,and mark the given
* descriptor to inactive.
*/
BOOL usbStopAsyncXfer(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct usb_device* pUsbDev = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	DWORD dwResult = 0;
	BOOL bResult = FALSE;

#ifdef __DEBUG_USB_ASYNC
	_hx_printf("%s: begin to stop async_xfer.\r\n", __func__);
#endif

	if (NULL == pAsyncDesc)
	{
		return FALSE;
	}

	pCommCtrl = pAsyncDesc->pCtrl;
	pUsbDev = pAsyncDesc->pUsbDev;
	if ((NULL == pCommCtrl) || (NULL == pUsbDev))
	{
		BUG();
	}
	pEhciCtrl = pCommCtrl->pUsbCtrl;
	if (NULL == pEhciCtrl)
	{
		BUG();
	}
	//Set descriptor's status accordingly.
	pAsyncDesc->status = USB_ASYNCDESC_STATUS_INITIALIZED;
	if (NULL == pAsyncDesc->hEvent)
	{
		BUG();
	}
	ResetEvent(pAsyncDesc->hEvent);

	/*
	* Mark the descriptor inactive.
	*/
	MarkDescriptorInactive(pAsyncDesc, TRUE);
	return TRUE;
}

//Destroy USB asynchronous transfer descriptor.
BOOL usbDestroyAsyncDescriptor(__USB_ASYNC_DESCRIPTOR* pAsyncDesc)
{
	BOOL bResult = FALSE;
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	DWORD dwFlags = 0;
	__USB_ASYNC_DESCRIPTOR* pDescNext = NULL;


	if (NULL == pAsyncDesc)
	{
		goto __TERMINAL;
	}

	if (USB_ASYNCDESC_STATUS_INITIALIZED != pAsyncDesc->status)
	{
		_hx_printf("%s:please stop the async_xfer before destroy it.\r\n", __func__);
		goto __TERMINAL;
	}
	pCommCtrl = pAsyncDesc->pCtrl;
	pEhciCtrl = pCommCtrl->pUsbCtrl;

#ifdef __DEBUG_USB_ASYNC
	_hx_printf("%s: begin to destroy async_desc.\r\n", __func__);
#endif

	/*
	* Remove queue head from controller's schduling
	* list.
	*/
	RemoveQueueHead(pAsyncDesc);

	//Detach the descriptor from global list.
	__ENTER_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);
	pDescNext = pEhciCtrl->pAsyncDescFirst;
	if (pAsyncDesc == pDescNext)  //First one is the delete target.
	{
		pEhciCtrl->pAsyncDescFirst = pDescNext->pNext;
		if (NULL == pEhciCtrl->pAsyncDescFirst)
		{
			pEhciCtrl->pAsyncDescLast = NULL;
		}
	}
	else
	{
		while (pAsyncDesc != pDescNext->pNext)
		{
			if (NULL == pDescNext->pNext)
			{
				BUG();
			}
			pDescNext = pDescNext->pNext;
		}
		pDescNext->pNext = pAsyncDesc->pNext;
		if (NULL == pDescNext->pNext)  //The last one.
		{
			pEhciCtrl->pAsyncDescLast = pDescNext;
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(pEhciCtrl->spin_lock, dwFlags);

	/*
	* Release all resources that allocated for the descriptor.
	*/
	DestroyEvent(pAsyncDesc->hEvent);
	_hx_free(pAsyncDesc->pFirstTD);
	_hx_free(pAsyncDesc->QueueHead);
	_hx_free(pAsyncDesc);
	bResult = TRUE;
#ifdef __DEBUG_USB_ASYNC
	_hx_printf("%s: destroy async_desc successful.\r\n", __func__);
#endif

__TERMINAL:
	return bResult;
}
