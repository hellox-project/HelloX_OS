//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 29, 2015
//    Module Name               : ehciint.c
//    Module Funciton           : 
//                                USB EHCI Controller's interrupt handler and
//                                related functions are put into this file.
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

#include "errno.h"
#include "usb_defs.h"
#include "usbdescriptors.h"
#include "ch9.h"
#include "usb.h"
#include "ehci.h"
#include "usbiso.h"
#include "usbasync.h"

//Handlers to handle transfer competion interrupt.
static VOID OnXferCompletion(struct ehci_ctrl* pUsbCtrl)
{
	struct int_queue* pIntQueue = pUsbCtrl->pIntQueueFirst;
	__USB_ISO_DESCRIPTOR* pIsoDesc = pUsbCtrl->pIsoDescFirst;
	__USB_ASYNC_DESCRIPTOR* pAsyncDesc = pUsbCtrl->pAsyncDescFirst;

	if ((NULL == pIntQueue) && (NULL == pIsoDesc) && (NULL == pAsyncDesc))
	{
		debug("Warning: No pending transaction but interrupt raised.\r\n");
		return;
	}
	/*
	 * Check if the interrupt is raised by Isochronous xfer.
	 */
	if (pIsoDesc)
	{
		if (NULL == pIsoDesc->ISOXferIntHandler)
		{
			BUG();
		}
		pIsoDesc->ISOXferIntHandler(pIsoDesc);
	}

	/*
	 * Check if the interrupt is raised by asynchronous xfer.
	 */
	while (pAsyncDesc)
	{
		BUG_ON(NULL == pAsyncDesc->AsyncXferIntHandler);
		/*
		 * Invoke the interrupt hander of asynchronous xfer,returning TRUE means
		 * the interrupt is raised by this descriptor and processed successfully.
		 */
		pAsyncDesc->AsyncXferIntHandler(pAsyncDesc);
		/* 
		 * Check the next descriptor. 
		 */
		pAsyncDesc = pAsyncDesc->pNext;
	}

	/*
	 * Check if the interrupt is raised by interrupt xfer.
	 */
	if (pIntQueue)
	{
		if (pIntQueue->QueueIntHandler)
		{
			pIntQueue->QueueIntHandler(pIntQueue);
		}
		switch (pIntQueue->dwStatus)
		{
		case INT_QUEUE_STATUS_COMPLETED:
		case INT_QUEUE_STATUS_ERROR:
		case INT_QUEUE_STATUS_TIMEOUT:
		case INT_QUEUE_STATUS_CANCELED:
			//Remove the queue element from pending list.
			pUsbCtrl->pIntQueueFirst = pIntQueue->pNext;
			if (NULL == pUsbCtrl->pIntQueueFirst)
			{
				pUsbCtrl->pIntQueueLast = NULL;
			}
			pIntQueue->pNext = NULL;
			//Wakeup the pending kernel thread.
			SetEvent(pIntQueue->hEvent);
			break;
		default:
			break;
		}
	}
	debug("Exit %s with int queue status = 0x%X.\r\n", __func__,
		pIntQueue->dwStatus);
}

//Interrupt handler of EHCI Controller,it will be called indirectly by USB Manager.
unsigned long EHCIIntHandler(LPVOID pParam)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = (__COMMON_USB_CONTROLLER*)pParam;
	struct ehci_ctrl* pUsbCtrl = NULL;
	unsigned long ulResult = 0;
	unsigned long status = 0, int_bit = 0;
	unsigned long pciStatus = 0;
	unsigned long ulOwn = 0; /* Raised by this controller if = 1. */
	static int show_warn = 1;

	if (NULL == pCommCtrl)
	{
		goto __TERMINAL;
	}
	pUsbCtrl = (struct ehci_ctrl*)pCommCtrl->pUsbCtrl;
	if (NULL == pUsbCtrl)
	{
		BUG();
	}
	/*
	 * Try to handle all interrupt(s) in a loop.
	 */
	while (TRUE)
	{
		/*
		 * Read and acknowledge status register immediately,
		 * according EHCI spec.
		 */
		status = ehci_readl(&pUsbCtrl->hcor->or_usbsts);
		//ehci_writel(&pUsbCtrl->hcor->or_usbsts, status);
		if (status & INTR_AAE)
		{
			//ulResult++;
			/*
			 * Do not clear the AAE bit since it will be
			 * refered by door bell mechanism in asynchronous
			 * xfer.
			 */
		}
		if (status & INTR_PCE)
		{
			ulResult++;
			//Handler of PCE.
			/*
			 * Clear the status bit.
			 */
			int_bit = INTR_PCE;
			ehci_writel(&pUsbCtrl->hcor->or_usbsts, int_bit);
		}
		if (status & INTR_SEE)
		{
			pciStatus = pCommCtrl->pPhysicalDev->ReadDeviceConfig(pCommCtrl->pPhysicalDev,
				PCI_CONFIG_OFFSET_COMMAND, 4);
			_hx_printf("USB Controller [%d] system error,PCI_status/Ctrl_status = %X/%X.\r\n",
				pCommCtrl->pPhysicalDev->dwNumber, 
				pciStatus,
				ehci_readl(&pUsbCtrl->hcor->or_usbsts));
			//Should reset the USB controller according USB EHCI specification.But now we let 
			//it empty...
			ulResult++;
			/*
			* Clear the status bit.
			*/
			int_bit = INTR_SEE;
			ehci_writel(&pUsbCtrl->hcor->or_usbsts, int_bit);
		}
		if (status & INTR_UE)
		{
			/*
			* Clear the status bit.
			*/
			int_bit = INTR_UE;
			ehci_writel(&pUsbCtrl->hcor->or_usbsts, int_bit);
			ulResult++;
			OnXferCompletion(pUsbCtrl);
			pUsbCtrl->nXferIntNum++;
			//_hx_printf("%s: USB transfer complete interrupt,status = %X.\r\n", __func__, status);
		}
		if (status & INTR_UEE)
		{
			if (show_warn)
			{
				_hx_printf("USB Controller [Vendor = %X,Device = %X] encounters transfer error.\r\n",
					pCommCtrl->pPhysicalDev->DevId.Bus_ID.PCI_Identifier.wVendor,
					pCommCtrl->pPhysicalDev->DevId.Bus_ID.PCI_Identifier.wDevice);
				/* No more warning will be showed out next time. */
				show_warn = 0;
			}
			ulResult++;
			pUsbCtrl->nXferErrNum++;
			//Handler of UEE.
			/*
			* Clear the status bit.
			*/
			int_bit = INTR_UEE;
			ehci_writel(&pUsbCtrl->hcor->or_usbsts, int_bit);
		}
		if (status & INTR_FLR)
		{
			ulResult++;
			//_hx_printf("%s:periodic list frame flip over.\r\n", __func__);
			//Handler of FLR.
			/*
			* Clear the status bit.
			*/
			int_bit = INTR_FLR;
			ehci_writel(&pUsbCtrl->hcor->or_usbsts, int_bit);
		}
		//Acknowledge the interrupt.
		if (ulResult)
		{
			//debug("%s: Interrupt handling over,result = %d,status = 0x%X.\r\n",
			//	__func__, ulResult, status);
			//ehci_writel(&pUsbCtrl->hcor->or_usbsts, status);
			ulOwn = 1;
			ulResult = 0;
		}
		else
		{
			/*
			 * No more pending interrupt.
			 */
			break;
		}
	}

__TERMINAL:
	return ulOwn;
}

//Update int queue status in interrupt,it is revised according the routine
//_ehci_poll_int_queue.
static BOOL _ehciQueueIntHandler(struct int_queue* queue)
{
	struct QH *cur = queue->current;
	struct qTD *cur_td;
	uint32_t token, toggle;
	unsigned long pipe = queue->pipe;
	struct usb_device* dev = queue->pUsbDev;

	//Queue is canceled by user,maybe triggered by wait time out.
	if (INT_QUEUE_STATUS_CANCELED == queue->dwStatus)
	{
		return FALSE;
	}

	/* depleted queue */
	if (cur == NULL) {
		queue->dwStatus = INT_QUEUE_STATUS_COMPLETED;
		return TRUE;
	}
	/* still active */
	cur_td = &queue->tds[queue->current - queue->first];
	invalidate_dcache_range((unsigned long)cur_td,
		ALIGN_END_ADDR(struct qTD, cur_td, 1));
	token = hc32_to_cpu(cur_td->qt_token);
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE) {
		queue->dwStatus = INT_QUEUE_STATUS_INPROCESS;
		debug("Exit %s with no completed intr transfer. token is %x\r\n", __func__, token);
		return FALSE;
	}
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_HALTED){
		queue->dwStatus = INT_QUEUE_STATUS_ERROR;
		debug("Exit %s with halted intr transfer,token is %X.\r\n", __func__, token);
		return FALSE;
	}

	toggle = QT_TOKEN_GET_DT(token);
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), toggle);

	if (!(cur->qh_link & QH_LINK_TERMINATE))
	{
		queue->current++;
	}
	else
	{
		queue->current = NULL;
	}

	invalidate_dcache_range((unsigned long)cur->buffer,
		ALIGN_END_ADDR(char, cur->buffer,
		queue->elementsize));

	//Last qTD is transfer over.
	if (NULL == queue->current)
	{
		queue->dwStatus = INT_QUEUE_STATUS_COMPLETED;
		debug("Exit %s with completed intr transfer. token is %x at %p (first at %p)\r\n",
			__func__, token, cur, queue->first);
		return TRUE;
	}
	return FALSE;
}

//Create and return an interrupt queue object.
struct int_queue* EHCICreateIntQueue(struct usb_device *dev,
	unsigned long pipe, int queuesize, int elementsize,
	void *buffer, int interval)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);
	struct int_queue *result = NULL;
	uint32_t i, toggle;
	struct QH *list = NULL;
	int cmd = 0;
	DWORD dwFlags;

	/*
	* Interrupt transfers requiring several transactions are not supported
	* because bInterval is ignored.
	*
	* Also, ehci_submit_async() relies on wMaxPacketSize being a power of 2
	* <= PKT_ALIGN if several qTDs are required, while the USB
	* specification does not constrain this for interrupt transfers. That
	* means that ehci_submit_async() would support interrupt transfers
	* requiring several transactions only as long as the transfer size does
	* not require more than a single qTD.
	*/
	if (elementsize > usb_maxpacket(dev, pipe)) {
		printf("%s: xfers requiring several transactions are not supported.\r\n",
			"_ehci_create_int_queue");
		return NULL;
	}

	if (usb_pipetype(pipe) != PIPE_INTERRUPT) {
		debug("non-interrupt pipe (type=%lu)", usb_pipetype(pipe));
		return NULL;
	}

	/* limit to 4 full pages worth of data -
	* we can safely fit them in a single TD,
	* no matter the alignment
	*/
	if (elementsize >= 16384) {
		debug("too large elements for interrupt transfers\r\n");
		return NULL;
	}

	result = _hx_malloc(sizeof(*result));
	if (!result) {
		debug("ehci intr queue: out of memory\r\n");
		goto fail1;
	}

	//Create EVENT object to synchronizing the access.
	result->hEvent = CreateEvent(FALSE);
	if (NULL == result->hEvent)
	{
		goto fail1;
	}
	result->dwTimeOut = 0;
	result->pNext = NULL;
	result->pOwnerThread = __CURRENT_KERNEL_THREAD;
	result->QueueIntHandler = _ehciQueueIntHandler;
	result->pUsbDev = dev;
	result->dwStatus = INT_QUEUE_STATUS_INITIALIZED;

	result->elementsize = elementsize;
	result->pipe = pipe;
	result->first = memalign(USB_DMA_MINALIGN,
		sizeof(struct QH) * queuesize);
	if (!result->first) {
		debug("ehci intr queue: out of memory\r\n");
		goto fail2;
	}
	debug("%s: Allocate %d QH(s) at %X.\r\n", __func__,queuesize,result->first);

	result->current = result->first;
	result->last = result->first + queuesize - 1;
	result->tds = memalign(USB_DMA_MINALIGN,
		sizeof(struct qTD) * queuesize);
	if (!result->tds) {
		debug("ehci intr queue: out of memory\r\n");
		goto fail3;
	}
	debug("%s: Allocate %d qTD(s) at %X.\r\n", __func__,queuesize, result->tds);

	memset(result->first, 0, sizeof(struct QH) * queuesize);
	memset(result->tds, 0, sizeof(struct qTD) * queuesize);

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	for (i = 0; i < (uint32_t)queuesize; i++) {
		struct QH *qh = result->first + i;
		struct qTD *td = result->tds + i;
		void **buf = &qh->buffer;

		qh->qh_link = cpu_to_hc32((unsigned long)(qh + 1) | QH_LINK_TYPE_QH);
		if (i == queuesize - 1)
			qh->qh_link = cpu_to_hc32(QH_LINK_TERMINATE);

		qh->qh_overlay.qt_next = cpu_to_hc32((unsigned long)td);
		qh->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		qh->qh_endpt1 =
			cpu_to_hc32((0 << 28) | /* No NAK reload (ehci 4.9) */
			(usb_maxpacket(dev, pipe) << 16) | /* MPS */
			(1 << 14) |
			QH_ENDPT1_EPS(ehci_encode_speed(dev->speed)) |
			(usb_pipeendpoint(pipe) << 8) | /* Endpoint Number */
			(usb_pipedevice(pipe) << 0));
		qh->qh_endpt2 = cpu_to_hc32((1 << 30) | /* 1 Tx per mframe */
			(1 << 0)); /* S-mask: microframe 0 */
		if (dev->speed == USB_SPEED_LOW ||
			dev->speed == USB_SPEED_FULL) {
			/* C-mask: microframes 2-4 */
			qh->qh_endpt2 |= cpu_to_hc32((0x1c << 8));
		}
		ehci_update_endpt2_dev_n_port(dev, qh);

		td->qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		td->qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		debug("%s: communication direction is '%s'\r\n",
			__func__,
			usb_pipein(pipe) ? "in" : "out");

		if (i == queuesize - 1)  //Last one,set IoC bit.
		{
			td->qt_token = cpu_to_hc32(
				QT_TOKEN_DT(toggle) |
				(elementsize << 16) |
				(1 << 15) |   //Interrupt On Completion.
				//(3 << 10) |   //CERR bits.
				((usb_pipein(pipe) ? 1 : 0) << 8) | /* IN/OUT token */
				0x80); /* active */
		}
		else
		{
			td->qt_token = cpu_to_hc32(
				QT_TOKEN_DT(toggle) |
				(elementsize << 16) |
				//(3 << 10)           |   //CERR bits.
				((usb_pipein(pipe) ? 1 : 0) << 8) | /* IN/OUT token */
				0x80); /* active */
		}
		debug("%s: construct TD token = %X.\r\n", __func__, td->qt_token);
		td->qt_buffer[0] =
			cpu_to_hc32((unsigned long)buffer + i * elementsize);
		td->qt_buffer[1] =
			cpu_to_hc32((td->qt_buffer[0] + 0x1000) & ~0xfff);
		td->qt_buffer[2] =
			cpu_to_hc32((td->qt_buffer[0] + 0x2000) & ~0xfff);
		td->qt_buffer[3] =
			cpu_to_hc32((td->qt_buffer[0] + 0x3000) & ~0xfff);
		td->qt_buffer[4] =
			cpu_to_hc32((td->qt_buffer[0] + 0x4000) & ~0xfff);

#ifdef __MS_VC__
		//MS VC can not support sizeof(void) operation,we should
		//convert the buffer type to char*.
		*buf = (void*)((char*)buffer + i * elementsize);
#else
		//sizeof(void) is 1 under GCC or other environment,so the
		//following sentence is same as above one.
		*buf = buffer + i * elementsize;
#endif
		toggle ^= 1;
	}

	flush_dcache_range((unsigned long)buffer,
		ALIGN_END_ADDR(char, buffer,
		queuesize * elementsize));
	flush_dcache_range((unsigned long)result->first,
		ALIGN_END_ADDR(struct QH, result->first,
		queuesize));
	flush_dcache_range((unsigned long)result->tds,
		ALIGN_END_ADDR(struct qTD, result->tds,
		queuesize));

	//Acquire exclusively accessing of the controller.
	WaitForThisObject(ctrl->hMutex);

	if (ctrl->periodic_schedules > 0) {
		if (ehci_disable_periodic(ctrl) < 0) {
			ReleaseMutex(ctrl->hMutex);
			_hx_printf("FATAL %s: periodic should never fail, but did.\r\n",__func__);
			goto fail3;
		}
	}

	__ENTER_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);
	/* hook up to periodic list */
	list = &ctrl->periodic_queue;
	result->last->qh_link = list->qh_link;
	list->qh_link = cpu_to_hc32((unsigned long)result->first | QH_LINK_TYPE_QH);

	//Link interrupt queue to Controller's pending queue.
	if (NULL == ctrl->pIntQueueFirst)
	{
		ctrl->pIntQueueFirst = result;
		ctrl->pIntQueueLast = result;
	}
	else
	{
		result->pNext = ctrl->pIntQueueFirst;
		ctrl->pIntQueueFirst = result;
	}
	__LEAVE_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);

	flush_dcache_range((unsigned long)result->last,
		ALIGN_END_ADDR(struct QH, result->last, 1));
	flush_dcache_range((unsigned long)list,
		ALIGN_END_ADDR(struct QH, list, 1));

	if (ehci_enable_periodic(ctrl) < 0) {
		ReleaseMutex(ctrl->hMutex);
		_hx_printf("FATAL %s: periodic should never fail, but did.\r\n", __func__);;
		goto fail3;
	}
	ctrl->periodic_schedules++;
	ReleaseMutex(ctrl->hMutex);

	debug("Exit create_int_queue\r\n");
	return result;
fail3:
	if (result->tds)
		free(result->tds);
fail2:
	if (result->first)
		free(result->first);
	//if (result)
	//	free(result);
fail1:
	if (result)
	{
		if (NULL != result->hEvent)
		{
			DestroyEvent(result->hEvent);
		}
		free(result);
	}
	return NULL;
}

#ifndef USB_EHCI_DISABLE_INTERRUPT
//Poll a interrupt queue,check if the queue has been processed.
//Please note it's a blocking poll,since HelloX's event mechanism is adopted.
void* EHCIPollIntQueue(struct usb_device *dev,struct int_queue *queue)
{
	DWORD dwResult = 0;
	void* pRet = NULL;

	dwResult = WaitForThisObjectEx(queue->hEvent, USB_DEFAULT_XFER_TIMEOUT);
	switch (dwResult)
	{
	case OBJECT_WAIT_RESOURCE:
		if (INT_QUEUE_STATUS_COMPLETED == queue->dwStatus)
		{
			pRet = queue->first->buffer;
		}
		break;
	case OBJECT_WAIT_TIMEOUT:
	case OBJECT_WAIT_DELETED:
	case OBJECT_WAIT_FAILED:
		//Just for debugging.
		if (_ehciQueueIntHandler(queue))
		{
			pRet = queue->first->buffer;
			debug("%s: Transimit success,int queue status = %X,overlay token = %X,qTD token = %X.\r\n", 
				__func__,
				queue->dwStatus,
				queue->first->qh_overlay.qt_token,
				queue->tds->qt_token);
			return pRet;
		}
		else
		{
			debug("%s: Transmit failed,int queue status = %X,overlay_tok = %X,qTD_tok = %X.\r\n",
				__func__,
				queue->dwStatus,
				queue->current->qh_overlay.qt_token,
				queue->tds[queue->current - queue->first].qt_token);
			return pRet;
		}
		queue->dwStatus = INT_QUEUE_STATUS_CANCELED;
		break;
	default:
		BUG();
		break;
	}
	return pRet;
}

#else  //USB_EHCI_DISABLE_INTERRUPT.

//Use polling mode instead of interrupt mode.
void* EHCIPollIntQueue(struct usb_device *dev, struct int_queue *queue)
{
	DWORD dwResult = 0;
	void* pRet = NULL;
	int timeout = 0;

	timeout = get_timer(0);
	while (TRUE)
	{
		if (_ehciQueueIntHandler(queue))
		{
			pRet = queue->first->buffer;
			_hx_printf("%s: Transmit success with token = %x.\r\n",
				__func__,
				queue->first->qh_overlay.qt_token);
			break;
		}
		if (get_timer(timeout) >= 2000 / SYSTEM_TIME_SLICE)  //Wait 2 seconds.
		{
			_hx_printf("%s: Transmit failed with timeout = %d,token = %X.\r\n", 
				__func__,get_timer(timeout),
				queue->first->qh_overlay.qt_token);
			break;
		}
	}
	return pRet;
}

#endif //USB_EHCI_DISABLE_INTERRUPT.

/* Do not free buffers associated with QHs, they're owned by someone else */
int EHCIDestroyIntQueue(struct usb_device *dev,struct int_queue *queue)
{
	struct ehci_ctrl *ctrl = ehci_get_ctrl(dev);
	int result = -1;
	unsigned long timeout;
	struct QH *cur = NULL;
	DWORD dwFlags;
	struct int_queue* before = NULL, *current = NULL;

	//Remove it from the controller's pending list if it is in.It's a bit complicated since
	//the pending list is a one direction link list,and we also are not sure if the queue is
	//in list.
	__ENTER_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);
	if (NULL != ctrl->pIntQueueFirst)
	{
		if (queue == ctrl->pIntQueueFirst)
		{
			if (queue == ctrl->pIntQueueLast)
			{
				ctrl->pIntQueueFirst = NULL;
				ctrl->pIntQueueLast = NULL;
			}
			else
			{
				ctrl->pIntQueueFirst = queue->pNext;
				queue->pNext = NULL;
			}
		}
		else
		{
			before = ctrl->pIntQueueFirst;
			current = before->pNext;
			while (current && (current != queue))
			{
				before = current;
				current = current->pNext;
			}
			if (queue == current)  //Find the queue in list.
			{
				before->pNext = current->pNext;
				queue->pNext = NULL;
				if (NULL == current->pNext)  //Last one.
				{
					ctrl->pIntQueueLast = before;
				}
			}
		}
	}
	__LEAVE_CRITICAL_SECTION_SMP(ctrl->spin_lock, dwFlags);

	if (NULL != queue->pNext)
	{
		BUG();
	}

	WaitForThisObject(ctrl->hMutex);
	if (ehci_disable_periodic(ctrl) < 0) {
		ReleaseMutex(ctrl->hMutex);
		debug("FATAL: periodic should never fail, but did");
		goto out;
	}
	ctrl->periodic_schedules--;

	cur = &ctrl->periodic_queue;
	timeout = get_timer(0) + (500 / SYSTEM_TIME_SLICE); /* abort after 500ms */
	while (!(cur->qh_link & cpu_to_hc32(QH_LINK_TERMINATE))) {
		debug("considering %p, with qh_link %x\r\n", cur, cur->qh_link);
		if (NEXT_QH(cur) == queue->first) {
			debug("found candidate. removing from chain\r\n");
			cur->qh_link = queue->last->qh_link;
			flush_dcache_range((unsigned long)cur,
				ALIGN_END_ADDR(struct QH, cur, 1));
			result = 0;
			break;
		}
		cur = NEXT_QH(cur);
		if (get_timer(0) > timeout) {
			ReleaseMutex(ctrl->hMutex);
			_hx_printf("Timeout destroying interrupt endpoint queue\r\n");
			result = -1;
			goto out;
		}
	}

	if (ctrl->periodic_schedules > 0) {
		result = ehci_enable_periodic(ctrl);
		if (result < 0)
			debug("FATAL: periodic should never fail, but did");
	}
	ReleaseMutex(ctrl->hMutex);

out:
	free(queue->tds);
	free(queue->first);
	if (queue->hEvent)
	{
		DestroyEvent(queue->hEvent);
	}
	free(queue);

	return result;
}
