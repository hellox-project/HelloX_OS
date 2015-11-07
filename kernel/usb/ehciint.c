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

//Handlers to handle transfer competion interrupt.
static VOID OnXferCompletion(struct ehci_ctrl* pUsbCtrl)
{
	struct int_queue* pIntQueue = pUsbCtrl->pIntQueueFirst;
	if (NULL == pIntQueue)
	{
		debug("Warning: No pending int queue but interrupt raised.\r\n");
		return;
	}
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
	debug("Exit %s with int queue status = 0x%X.\r\n", __func__,
		pIntQueue->dwStatus);
}

//Interrupt handler of EHCI Controller,it will be called indirectly by USB Manager.
unsigned long EHCIIntHandler(LPVOID pParam)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = (__COMMON_USB_CONTROLLER*)pParam;
	struct ehci_ctrl* pUsbCtrl = NULL;
	unsigned long ulResult = 0;
	unsigned long status = 0;

	if (NULL == pCommCtrl)
	{
		goto __TERMINAL;
	}
	pUsbCtrl = (struct ehci_ctrl*)pCommCtrl->pUsbCtrl;
	if (NULL == pUsbCtrl)
	{
		BUG();
	}
	//Check if the interrupt is raised by this controller.
	status = ehci_readl(&pUsbCtrl->hcor->or_usbsts);
	if (status & INTR_AAE)
	{
		ulResult++;
		//Handler of AAE.
	}
	if (status & INTR_PCE)
	{
		ulResult++;
		//Handler of PCE.
	}
	if (status & INTR_SEE)
	{
		ulResult++;
		//Handler of SEE.
	}
	if (status & INTR_UE)
	{
		ulResult++;
		OnXferCompletion(pUsbCtrl);
	}
	if (status & INTR_UEE)
	{
		ulResult++;
		//Handler of UEE.
	}
	//Acknowledge the interrupt.
	if (ulResult)
	{
		debug("%s: Interrupt handling over,result = %d,status = 0x%X.\r\n",
			__func__, ulResult, status);
		ehci_writel(&pUsbCtrl->hcor->or_usbsts, status);
	}

__TERMINAL:
	return ulResult;
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

	toggle = QT_TOKEN_GET_DT(token);
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), toggle);

	if (!(cur->qh_link & QH_LINK_TERMINATE))
		queue->current++;
	else
		queue->current = NULL;

	invalidate_dcache_range((unsigned long)cur->buffer,
		ALIGN_END_ADDR(char, cur->buffer,
		queue->elementsize));

	queue->dwStatus = INT_QUEUE_STATUS_COMPLETED;
	debug("Exit %s with completed intr transfer. token is %x at %p (first at %p)\r\n",
		__func__, token, cur, queue->first);
	return TRUE;
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

	result = malloc(sizeof(*result));
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
	result->pOwnerThread = KernelThreadManager.lpCurrentKernelThread;
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
	result->current = result->first;
	result->last = result->first + queuesize - 1;
	result->tds = memalign(USB_DMA_MINALIGN,
		sizeof(struct qTD) * queuesize);
	if (!result->tds) {
		debug("ehci intr queue: out of memory\r\n");
		goto fail3;
	}
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
		debug("communication direction is '%s'\r\n",
			usb_pipein(pipe) ? "in" : "out");

		if (i == queuesize - 1)  //Last one,set IoC bit.
		{
			td->qt_token = cpu_to_hc32(
				QT_TOKEN_DT(toggle) |
				(elementsize << 16) |
				(1 << 15) |   //Interrupt On Completion.
				((usb_pipein(pipe) ? 1 : 0) << 8) | /* IN/OUT token */
				0x80); /* active */
		}
		else
		{
			td->qt_token = cpu_to_hc32(
				QT_TOKEN_DT(toggle) |
				(elementsize << 16) |
				((usb_pipein(pipe) ? 1 : 0) << 8) | /* IN/OUT token */
				0x80); /* active */
		}
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

		//Should be carefully here.!!!
#ifdef __MS_VC__
		*buf = (void*)((char*)buffer + i * elementsize);
#else
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
			debug("FATAL: periodic should never fail, but did");
			goto fail3;
		}
	}

	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
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
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

	flush_dcache_range((unsigned long)result->last,
		ALIGN_END_ADDR(struct QH, result->last, 1));
	flush_dcache_range((unsigned long)list,
		ALIGN_END_ADDR(struct QH, list, 1));

	if (ehci_enable_periodic(ctrl) < 0) {
		ReleaseMutex(ctrl->hMutex);
		debug("FATAL: periodic should never fail, but did");
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

//Poll a interrupt queue,check if the queue has been processed.
//Please note it's a blocking poll,since HelloX's event mechanism is adopted.
void* EHCIPollIntQueue(struct usb_device *dev,struct int_queue *queue)
{
	DWORD dwResult = 0;
	void* pRet = NULL;

	dwResult = WaitForThisObjectEx(queue->hEvent, 2000);
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
		queue->dwStatus = INT_QUEUE_STATUS_CANCELED;
		break;
	default:
		break;
	}
	return pRet;
}

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
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
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
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

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
