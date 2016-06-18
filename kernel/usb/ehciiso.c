//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 31, 2016
//    Module Name               : ehciiso.c
//    Module Funciton           : 
//                                USB EHCI Controller's isochronous transfer
//                                function's implementation code is put into
//                                this file.
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

//Interrupt handler of USB isochronous transfer.
static BOOL ISOXferIntHandler(__USB_ISO_DESCRIPTOR* pIsoDesc)
{
	struct iTD* pitd = NULL;
	uint32_t itdStatus = 0;
	uint32_t xfersize = 0;
	int transact = 0;

	if (NULL == pIsoDesc)
	{
		BUG();
	}
	if (USB_ISODESC_STATUS_CANCELED == pIsoDesc->status)  //May caused by timeout.
	{
		return FALSE;
	}
	pitd = pIsoDesc->itdArray;
	invalidate_dcache_range((unsigned long)pitd, ALIGN_END_ADDR(struct iTD, pitd, 1));
	//Check if the interrupt is lead by this iTD.
	for (transact = 0; transact < 8; transact++)
	{
		itdStatus = pitd->transaction[transact];
		itdStatus >>= 28;
		if (ITD_TRANS_STATUS_ACT == itdStatus)
		{
			//The interrupt is not caused by this iTD,since there is active transaction.
			return FALSE;
		}
		if ((0 == itdStatus) && (USB_ISODESC_STATUS_INPROCESS != pIsoDesc->status))
		{
			//Also is not caused by this iTD,since no isochronous xfer pending.
			return FALSE;
		}
		if (itdStatus)
		{
			_hx_printf("%s:iTD xfer error with status[0x%X],transact[%d].\r\n", __func__, 
				itdStatus,transact);
			pIsoDesc->status = USB_ISODESC_STATUS_ERROR;
			//Clear the err bits.
			pitd->transaction[0] &= 0x0FFFFFFF;
			pitd->transaction[1] &= 0x0FFFFFFF;
			pitd->transaction[2] &= 0x0FFFFFFF;
			pitd->transaction[3] &= 0x0FFFFFFF;
			pitd->transaction[4] &= 0x0FFFFFFF;
			pitd->transaction[5] &= 0x0FFFFFFF;
			pitd->transaction[6] &= 0x0FFFFFFF;
			pitd->transaction[7] &= 0x0FFFFFFF;
			flush_dcache_range((unsigned long)pitd, ALIGN_END_ADDR(struct iTD, pitd, 1));
			SetEvent(pIsoDesc->hEvent);
			return FALSE;
		}
	}
	//It means every transaction's status bits are 0 when reach here.
	pIsoDesc->xfersize = 0;
	for (transact = 0; transact < 8;transact ++)
	{
		xfersize = pitd->transaction[transact];
		xfersize >>= 16;
		xfersize &= 0x0FFF;  //12 bits.
		pIsoDesc->xfersize += xfersize;
		//Save actual xfered data length into ISO descriptor.
		pIsoDesc->FrameBuffer[transact].xfer_length = xfersize;
#ifdef __DEBUG_USB_ISO
		_hx_printf("%s:trans[%d],xfer_sz[%d].\r\n", __func__, transact, xfersize);
#endif
	}
	pIsoDesc->status = USB_ISODESC_STATUS_COMPLETED;
	SetEvent(pIsoDesc->hEvent);
	return TRUE;
#if 0
	_hx_printf("%s:iTD xfer error with status[0x%X].\r\n", __func__, itdStatus);
	pIsoDesc->status = USB_ISODESC_STATUS_ERROR;
	//Clear the err bits.
	pitd->transaction[0] &= 0x0FFFFFFF;
	pitd->transaction[1] &= 0x0FFFFFFF;
	pitd->transaction[2] &= 0x0FFFFFFF;
	pitd->transaction[3] &= 0x0FFFFFFF;
	pitd->transaction[4] &= 0x0FFFFFFF;
	pitd->transaction[5] &= 0x0FFFFFFF;
	pitd->transaction[6] &= 0x0FFFFFFF;
	pitd->transaction[7] &= 0x0FFFFFFF;
	flush_dcache_range((unsigned long)pitd, ALIGN_END_ADDR(struct iTD, pitd, 1));
	SetEvent(pIsoDesc->hEvent);
	return FALSE;
#endif
}

//Local helper routine to fill one iTD.
//We refers Microsoft's EHCI source code when implement this routine,since iTD structure's description in
//EHCI spec is very complicated and hard to implement,it maybe the poorest design I ever meet...
#define EHCI_ITD_BUFFER (~4095)
static BOOL __Fill_iTD(__USB_ISO_DESCRIPTOR* pIsoDesc, struct iTD* pitd, 
	int direction, char* buffer, int bufflength, __U8 endpoint, __u16 maxPacketSize, __U8 multi)
{
	int Transaction, LastPage;
	__U32 Value32, LastPagePointer;
	char* BufferPointer;
	int nLeft, Length;

	BufferPointer = buffer;
	nLeft = bufflength;

	LastPage = -1;
	LastPagePointer = 1;  //Make sure will not match at start.

	for (Transaction = 0; Transaction < 8; Transaction++)
	{
		//Stop when we have covered the whole buffer.
		if (nLeft)
		{
			//Set a new page pointer if we have to.
			Value32 = (__U32)BufferPointer;
			if ((Value32 & EHCI_ITD_BUFFER) != LastPagePointer)
			{
				LastPagePointer = pitd->pg_pointer[++LastPage];
				LastPagePointer &= ~EHCI_ITD_BUFFER;
				pitd->pg_pointer[LastPage] = (Value32 & EHCI_ITD_BUFFER) | LastPagePointer;
				LastPagePointer = Value32 & EHCI_ITD_BUFFER;
			}

			//Cap and adjust the length.
			Length = nLeft;
			if (Length > maxPacketSize * multi)
			{
				Length = maxPacketSize * multi;
			}
			if (Length > 1024 * 3)
			{
				Length = 1024 * 3;
			}

			//if (LastPagePointer != ((Value32 + Length) & EHCI_ITD_BUFFER))
			//{
			//	Length = (LastPagePointer + 4096) - Value32;
			//}

			//Save each frame's data buffer and length into ISO descriptor.
			pIsoDesc->FrameBuffer[Transaction].pBuffer = BufferPointer;
			pIsoDesc->FrameBuffer[Transaction].buffLength = Length;
			pIsoDesc->FrameBuffer[Transaction].xfer_length = 0;

			//Adjust pointers and counts for next round.
			nLeft -= Length;
			BufferPointer += Length;

			//Fill transaction.
			Value32 = (ITD_TRANS_STATUS_SET(ITD_TRANS_STATUS_ACT) |
				((Length & 0x0FFF) << 16) |
				((LastPage & 0x07) << 12) |
				(Value32 & 0xFFF));
			//Set IoC if is the last transaction.
			if ((0 == nLeft) || (7 == Transaction))
			{
				Value32 |= (1 << 15);  //Set IoC bit.
			}
			//Write to xact area of iTD.
			pitd->transaction[Transaction] = Value32;
#ifdef __DEBUG_USB_ISO
			_hx_printf("%s:trans[%d],length[%d],trans_val[0x%X].\r\n", __func__,
				Transaction, Length, Value32);
#endif
		}
		else
		{
			pitd->transaction[Transaction] = 0;
			pIsoDesc->FrameBuffer[Transaction].pBuffer = NULL;
			pIsoDesc->FrameBuffer[Transaction].buffLength = 0;
			pIsoDesc->FrameBuffer[Transaction].xfer_length = 0;
		}
	}
	return TRUE;
}
#undef EHCI_ITD_BUFFER

//Create a USB isochronous transfer descriptor and install it into system.
__USB_ISO_DESCRIPTOR* usbCreateISODescriptor(__PHYSICAL_DEVICE* pPhyDev, int direction, int bandwidth,
	char* buffer, int bufflength, __U8 endpoint,__U16 maxPacketSize,__U8 multi)
{
	__USB_ISO_DESCRIPTOR* pIsoDesc = NULL;
	BOOL bResult = FALSE;
	struct usb_device* pUsbDev = NULL;
	struct iTD* pitd = NULL;
	__COMMON_USB_CONTROLLER* pCtrl = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	int maxXferSize = 0, transact_leng = 0;
	int slot_num = 0;     //How many slot the iTD should link to.
	int slot_space = 0;   //The slot number between 2 iTDs in periodic list.
	int i = 0;
	char* buff_ptr = buffer;
	DWORD dwFlags;

	//Parameters check.
	if ((NULL == pPhyDev) || (NULL == buffer) || (0 == bufflength) || (0 == endpoint))
	{
		goto __TERMINAL;
	}
	if ((bandwidth > EHCI_ISO_MAX_BANDWIDTH) || (0 == multi))
	{
		goto __TERMINAL;
	}
	if (maxPacketSize > 1024)  //EHCI spec.
	{
		goto __TERMINAL;
	}
	if ((direction != USB_TRANSFER_DIR_IN) && (direction != USB_TRANSFER_DIR_OUT))
	{
		goto __TERMINAL;
	}
	pUsbDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	if (NULL == pUsbDev)
	{
		BUG();
	}
	pCtrl = (__COMMON_USB_CONTROLLER*)pUsbDev->controller;
	pEhciCtrl = (struct ehci_ctrl*)pCtrl->pUsbCtrl;

	//8 transactions in one iTD descriptor.
	maxXferSize = maxPacketSize * multi * 8;
	if (bufflength > (int)maxXferSize)
	{
		goto __TERMINAL;
	}

	//Allocate a ISOxfer descriptor and initialize it accordingly.
	pIsoDesc = _hx_malloc(sizeof(__USB_ISO_DESCRIPTOR));
	if (NULL == pIsoDesc)
	{
		goto __TERMINAL;
	}
	memset(pIsoDesc, 0, sizeof(__USB_ISO_DESCRIPTOR));
	pIsoDesc->bandwidth = bandwidth;
	pIsoDesc->buffer = buffer;
	pIsoDesc->bufflength = bufflength;
	pIsoDesc->direction = direction;
	pIsoDesc->ISOXferIntHandler = ISOXferIntHandler;
	pIsoDesc->itdArray = NULL; //Will be initialized later.
	pIsoDesc->itdnumber = 0;   //Will be initialized later.
	pIsoDesc->pCtrl = (__COMMON_USB_CONTROLLER*)pUsbDev->controller;
	pIsoDesc->hEvent = NULL;
	pIsoDesc->endpoint = endpoint;
	pIsoDesc->maxPacketSize = maxPacketSize;
	pIsoDesc->multi = multi;
	pIsoDesc->pNext = NULL;
	pIsoDesc->pPhyDev = pPhyDev;
	pIsoDesc->status = USB_ISODESC_STATUS_INITIALIZED;

	pIsoDesc->hEvent = CreateEvent(FALSE);
	if (NULL == pIsoDesc)
	{
		goto __TERMINAL;
	}

	//Create iTD and initialize it.
	pitd = (struct iTD*)_hx_aligned_malloc(sizeof(struct iTD), 32);
	if (NULL == pitd)
	{
		goto __TERMINAL;
	}
	memset(pitd, 0, sizeof(struct iTD));
	pitd->lp_next |= ITD_NEXT_TERMINATE;  //Terminate bit.
	pitd->lp_next |= ITD_NEXT_TYPE_SET(ITD_NEXT_TYPE_ITD);  //Type as iTD.

	//pitd->transaction[0] |= ITD_TRANS_XLEN_SET(bufflength); //xlength.
	//pitd->transaction[0] |= ITD_TRANS_IOC_SET(1);           //Set IOC bit.
	//pitd->transaction[0] |= ITD_TRANS_PG_SET(0);            //Page 0.
	//pitd->transaction[0] |= ITD_TRANS_XOFFSET_SET(buffer);  //Buffer offset.
	//pitd->pg_pointer[0] |= ITD_PGPTR_SET(buffer);           //Buffer page pointer.

	pitd->pg_pointer[0] |= ITD_ENDPOINT_SET(endpoint);      //Endpoint.
	pitd->pg_pointer[0] |= pUsbDev->devnum;                 //Device address.
	//Set transfer direction.
	if (USB_TRANSFER_DIR_IN == direction)
	{
		pitd->pg_pointer[1] |= ITD_XFERDIR_SET(1);
	}
	else
	{
		pitd->pg_pointer[1] |= ITD_XFERDIR_SET(0);
	}
	pitd->pg_pointer[1] |= ITD_MAX_PKTSZ_SET(maxPacketSize);  //Max packet size.
	pitd->pg_pointer[2] |= ITD_MULTI_SET(multi);              //multiple per (micro)frame.

	//Fill iTD's transaction(s) accordingly.
	__Fill_iTD(pIsoDesc,pitd,direction,buffer,bufflength,endpoint,maxPacketSize,multi);

#if 0
	//Initializes data buffer offset and pointer one by one.
	i = 0;
	buff_ptr = buffer;
	maxXferSize = bufflength;
	transact_leng = (maxPacketSize * multi > maxXferSize) ?
	maxXferSize : (maxPacketSize * multi);
	while (maxXferSize)
	{
		pitd->transaction[i] |= ITD_TRANS_XLEN_SET(bufflength); //xlength.
		pitd->transaction[i] |= ITD_TRANS_PG_SET(i);            //Page 0.
		pitd->transaction[i] |= ITD_TRANS_XOFFSET_SET(buffer);  //Buffer offset.
		pitd->pg_pointer[i] |= ITD_PGPTR_SET(buffer);           //Buffer page pointer.
	}
#endif

#ifdef __DEBUG_USB_ISO
	_hx_printf("iso_iTD:lp_next = 0x%X,trans[0] = 0x%X,pg_ptr[0] = 0x%X,pg_ptr[1] = 0x%X,pg_ptr[2] = 0x%X.\r\n",
		pitd->lp_next, pitd->transaction[0], pitd->pg_pointer[0], pitd->pg_pointer[1],pitd->pg_pointer[2]);
#endif

	flush_dcache_range((unsigned long)pitd, ALIGN_END_ADDR(struct iTD, pitd, 1));

	//Link the iTD to isochronous transfer descriptor.
	pIsoDesc->itdArray = pitd;
	pIsoDesc->itdnumber = 1;

	//Calculate how many periodic list slot shall we use.
	slot_num = bandwidth / 8;
	if (slot_num < bufflength)
	{
		slot_num = 1;
	}
	else
	{
		slot_num = (0 == slot_num % bufflength) ? (slot_num / bufflength) :
			(slot_num / bufflength + 1);
	}
	if (slot_num > USB_PERIODIC_LIST_LENGTH) //Paameter checking makes sure this can not happen.
	{
		_hx_printf("%s:too many slot number[bw = %d,buff_len = %d,slot_num = %d.\r\n",
			__func__,
			bandwidth,
			bufflength,
			slot_num);
		goto __TERMINAL;
	}
	//Save the slot_num to descriptor,since it will be used in usbDestroyISODescriptor routine.
	pIsoDesc->slot_num = slot_num;
	slot_space = USB_PERIODIC_LIST_LENGTH / slot_num;
	if (0 == slot_space)
	{
		slot_space = 1;
	}
#ifdef __DEBUG_USB_ISO
	_hx_printf("slot_num:%d,slot_space:%d.\r\n", slot_num, slot_space);
#endif

	//Stop periodic schedule if enabled already.
	WaitForThisObject(pEhciCtrl->hMutex);
	if (pEhciCtrl->periodic_schedules > 0)
	{
		if (ehci_disable_periodic(pEhciCtrl) < 0) {
			ReleaseMutex(pEhciCtrl->hMutex);
			_hx_printf("FATAL %s: periodic should never fail, but did.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	//Insert the iTD to periodic list,and insert the ISOXfer descriptor into EHCI controller's
	//list.
	i = 0;
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	while (slot_num)
	{
		pitd->lp_next = pEhciCtrl->periodic_list[i];
		pEhciCtrl->periodic_list[i] = ((__U32)pitd | QH_LINK_TYPE_ITD);
		flush_dcache_range((unsigned long)&pEhciCtrl->periodic_list[i],
			ALIGN_END_ADDR(uint32_t, &pEhciCtrl->periodic_list[i], 1));
		i += slot_space;
		//Re-calculate the space between slot to make sure the iTD can be linked into
		//periodic list as scatterly as possible.
		slot_space = (USB_PERIODIC_LIST_LENGTH - i) / slot_num;
		if (0 == slot_space)
		{
			slot_space = 1;
		}
		slot_num--;
	}

	//Insert it into global list.
	if (NULL == pEhciCtrl->pIsoDescFirst)  //First element.
	{
		pEhciCtrl->pIsoDescFirst = pIsoDesc;
		pEhciCtrl->pIsoDescLast = pIsoDesc;
	}
	else  //Put it at last.
	{
		//We only support one isochronous xfer at the sametime for simpicity.
		BUG();
		if (NULL == pEhciCtrl->pIsoDescLast)
		{
			BUG();
		}
		pEhciCtrl->pIsoDescLast->pNext = pIsoDesc;
		pEhciCtrl->pIsoDescLast = pIsoDesc;
	}
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

	//Restart the periodic schedule if already enabled.
	if (pEhciCtrl->periodic_schedules > 0)
	{
		ehci_enable_periodic(pEhciCtrl);
	}
	ReleaseMutex(pEhciCtrl->hMutex);
	
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pIsoDesc)  //Should release it.
		{
			if (pIsoDesc->hEvent)
			{
				DestroyEvent(pIsoDesc->hEvent);
			}
			_hx_free(pIsoDesc);
		}
		if (pitd)
		{
			_hx_free(pitd);
		}
		pIsoDesc = NULL;  //Mark as failed.
	}
	return pIsoDesc;
}

//A helper routine to assist error recoverying.
static BOOL IsoClearError(__USB_ISO_DESCRIPTOR* pIsoDesc)
{
	int pipe = 0;
	struct usb_device* pUsbDev = (struct usb_device*)pIsoDesc->pPhyDev->lpPrivateInfo;

	if (USB_TRANSFER_DIR_IN == pIsoDesc->direction)
	{
		pipe = usb_rcvisocpipe(pUsbDev, pIsoDesc->endpoint);
	}
	else
	{
		pipe = usb_sndisocpipe(pUsbDev, pIsoDesc->endpoint);
	}
	//Just reset the endpoint.
	return 0 == usb_clear_halt(pUsbDev, pipe) ? TRUE : FALSE;
}

//Start USB isochronous transfer.
BOOL usbStartISOXfer(__USB_ISO_DESCRIPTOR* pIsoDesc)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct iTD* pitd = NULL;
	struct usb_device* pUsbDev = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	DWORD dwResult = 0;
	BOOL bResult = FALSE;
	//int transact = 0;

	if (NULL == pIsoDesc)
	{
		goto __TERMINAL;
	}
	pCommCtrl = pIsoDesc->pCtrl;
	pUsbDev = pIsoDesc->pPhyDev->lpPrivateInfo;
	if ((NULL == pCommCtrl) || (NULL == pUsbDev))
	{
		BUG();
	}
	pitd = pIsoDesc->itdArray;
	pEhciCtrl = pCommCtrl->pUsbCtrl;
	if ((NULL == pitd) || (NULL == pEhciCtrl))
	{
		BUG();
	}
	//Set descriptor's status accordingly.
	pIsoDesc->status = USB_ISODESC_STATUS_INPROCESS;
	ResetEvent(pIsoDesc->hEvent);

#if 0
	//Set the act bit of each transaction(s) iTD descriptor,so controller will process it.
	for (int transact = 0; transact < 8; transact++)
	{
		if (0 == pitd->transaction[transact])  //Reach the end.
		{
			break;
		}
		pitd->transaction[transact] |= ITD_TRANS_STATUS_SET(ITD_TRANS_STATUS_ACT);
		//Just for debugging.__Fill_iTD should be called here...
		pitd->transaction[0] &= 0xF000FFFF;
		pitd->transaction[0] |= ITD_TRANS_XLEN_SET(pIsoDesc->bufflength);
	}
#endif
	//Fill iTD's all variable fields.
	__Fill_iTD(pIsoDesc, pitd, pIsoDesc->direction, pIsoDesc->buffer, pIsoDesc->bufflength, pIsoDesc->endpoint, 
		pIsoDesc->maxPacketSize, pIsoDesc->multi);
	flush_dcache_range((unsigned long)pitd,
		ALIGN_END_ADDR(struct iTD, pitd, 1));

	//Start periodic schedule if not yet.
	WaitForThisObject(pEhciCtrl->hMutex);
	if (pEhciCtrl->periodic_schedules == 0)
	{
		ehci_enable_periodic(pEhciCtrl);
		pEhciCtrl->periodic_schedules++;
	}
	ReleaseMutex(pEhciCtrl->hMutex);

	//Pending on the descriptor to wait the arrival of data.
	dwResult = WaitForThisObjectEx(pIsoDesc->hEvent, USB_DEFAULT_XFER_TIMEOUT);
	switch (dwResult)
	{
	case OBJECT_WAIT_RESOURCE:
		if (USB_ISODESC_STATUS_COMPLETED == pIsoDesc->status)
		{
			bResult = TRUE;
			break;
		}
		if (USB_ISODESC_STATUS_ERROR == pIsoDesc->status)
		{
			//Try to restore from error.
			//IsoClearError(pIsoDesc);

			//Clear the act bit.
			pitd->transaction[0] &= ~0x80000000;
			pitd->transaction[1] &= ~0x80000000;
			pitd->transaction[2] &= ~0x80000000;
			pitd->transaction[3] &= ~0x80000000;
			pitd->transaction[4] &= ~0x80000000;
			pitd->transaction[5] &= ~0x80000000;
			pitd->transaction[6] &= ~0x80000000;
			pitd->transaction[7] &= ~0x80000000;
			flush_dcache_range((unsigned long)pitd,
				ALIGN_END_ADDR(struct iTD, pitd, 1));
			break;
		}
		BUG();
		break;
	case OBJECT_WAIT_TIMEOUT:
	case OBJECT_WAIT_FAILED:
		//Set status as canceled.
		pIsoDesc->status = USB_ISODESC_STATUS_CANCELED;
		//Clear the act bit.
		pitd->transaction[0] &= ~0x80000000;
		pitd->transaction[1] &= ~0x80000000;
		pitd->transaction[2] &= ~0x80000000;
		pitd->transaction[3] &= ~0x80000000;
		pitd->transaction[4] &= ~0x80000000;
		pitd->transaction[5] &= ~0x80000000;
		pitd->transaction[6] &= ~0x80000000;
		pitd->transaction[7] &= ~0x80000000;
		flush_dcache_range((unsigned long)pitd,
			ALIGN_END_ADDR(struct iTD, pitd, 1));
		_hx_printf("%s:iTD timed out.\r\n", __func__);
		break;
	case OBJECT_WAIT_DELETED:
		BUG();  //Should not occur.
		break;
	default:
		_hx_printf("%s:unexcepted timeout waiting's return value[%d].\r\n",
			__func__, dwResult);
		BUG();
	}
	//Disable EHCI periodic schedule if necessary.
#if 0  //It's not necessary since EHCI periodic schedule will be disabled in STOP operation.
	WaitForThisObject(pEhciCtrl->hMutex);
	pEhciCtrl->periodic_schedules--;
	if (pEhciCtrl->periodic_schedules == 0)
	{
		ehci_disable_periodic(pEhciCtrl);
	}
	ReleaseMutex(pEhciCtrl->hMutex);
#endif

__TERMINAL:
	return bResult;
}

//Stop USB isochronous transfer.
BOOL usbStopISOXfer(__USB_ISO_DESCRIPTOR* pIsoDesc)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	BOOL bResult = FALSE;
	struct iTD* pitd = NULL;
	int i = 0;

	if (NULL == pIsoDesc)
	{
		goto __TERMINAL;
	}
	pCommCtrl = pIsoDesc->pCtrl;
	pEhciCtrl = pCommCtrl->pUsbCtrl;
	if ((NULL == pCommCtrl) || (NULL == pEhciCtrl))
	{
		BUG();
	}

	//Disable periodic scheduling first.
	WaitForThisObject(pEhciCtrl->hMutex);
	if (pEhciCtrl->periodic_schedules > 0)
	{
		pEhciCtrl->periodic_schedules--;
		if (pEhciCtrl->periodic_schedules == 0)
		{
			if (ehci_disable_periodic(pEhciCtrl) < 0) {
				ReleaseMutex(pEhciCtrl->hMutex);
				_hx_printf("FATAL %s: periodic should never fail, but did.\r\n", __func__);
				goto __TERMINAL;
			}
		}
	}
	ReleaseMutex(pEhciCtrl->hMutex);

	//Clear ACT bit of iTD descriptor.
	pitd = pIsoDesc->itdArray;
	for (i = 0; i < pIsoDesc->itdnumber; i++)
	{
		pitd[i].transaction[0] &= ~0xF0000000;
		pitd[i].transaction[1] &= ~0xF0000000;
		pitd[i].transaction[2] &= ~0xF0000000;
		pitd[i].transaction[3] &= ~0xF0000000;
		pitd[i].transaction[4] &= ~0xF0000000;
		pitd[i].transaction[5] &= ~0xF0000000;
		pitd[i].transaction[6] &= ~0xF0000000;
		pitd[i].transaction[7] &= ~0xF0000000;
	}
	pIsoDesc->status = USB_ISODESC_STATUS_INITIALIZED;

	//Done.
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Destroy USB isochronous transfer descriptor.
BOOL usbDestroyISODescriptor(__USB_ISO_DESCRIPTOR* pIsoDesc)
{
	__COMMON_USB_CONTROLLER* pCommCtrl = NULL;
	struct ehci_ctrl* pEhciCtrl = NULL;
	struct iTD* pitd = NULL;
	DWORD dwFlags = 0;
	BOOL bResult = FALSE;
	int slot_num = 0, slot_space = 0, i = 0;
	uint32_t* lp_next = NULL;
	__USB_ISO_DESCRIPTOR* pDescNext = NULL;

	if ((NULL == pIsoDesc))
	{
		goto __TERMINAL;
	}
	if (USB_ISODESC_STATUS_INITIALIZED != pIsoDesc->status)
	{
		_hx_printf("%s:please stop the iso_xfer before destroy it.\r\n", __func__);
		goto __TERMINAL;
	}
	pCommCtrl = pIsoDesc->pCtrl;
	pEhciCtrl = pCommCtrl->pUsbCtrl;

	//Stop isochronous transfer first.
	WaitForThisObject(pEhciCtrl->hMutex);
	if (pEhciCtrl->periodic_schedules > 0)
	{
		if (ehci_disable_periodic(pEhciCtrl) < 0) {
			ReleaseMutex(pEhciCtrl->hMutex);
			_hx_printf("FATAL %s: periodic should never fail, but did.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	//Detach the iTD from periodic list of the EHCI controller.
	pitd = pIsoDesc->itdArray;
	slot_num = pIsoDesc->slot_num;
	slot_space = USB_PERIODIC_LIST_LENGTH / slot_num;
	if (0 == slot_space)
	{
		slot_space = 1;
	}
	i = 0;

	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	while (slot_num)
	{
		pEhciCtrl->periodic_list[i] = pitd->lp_next;
		flush_dcache_range((unsigned long)&pEhciCtrl->periodic_list[i],
			ALIGN_END_ADDR(uint32_t, &pEhciCtrl->periodic_list[i], 1));
		i += slot_space;
		//Re-calculate the space between slot to make sure the iTD can be linked into
		//periodic list as scatterly as possible.
		slot_space = (USB_PERIODIC_LIST_LENGTH - i) / slot_num;
		if (0 == slot_space)
		{
			slot_space = 1;
		}
		slot_num--;
	}

	//Detach the descriptor from global list.
	pDescNext = pEhciCtrl->pIsoDescFirst;
	if (pIsoDesc == pDescNext)  //First one is the delete target.
	{
		pEhciCtrl->pIsoDescFirst = pDescNext->pNext;
		if (NULL == pEhciCtrl->pIsoDescFirst)
		{
			pEhciCtrl->pIsoDescLast = NULL;
		}
	}
	else
	{
		while (pIsoDesc != pDescNext->pNext)
		{
			if (NULL == pDescNext->pNext)
			{
				BUG();
			}
			pDescNext = pDescNext->pNext;
		}
		pDescNext->pNext = pIsoDesc->pNext;
		if (NULL == pDescNext->pNext)
		{
			pEhciCtrl->pIsoDescLast = NULL;
		}
	}
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

	//Restart the periodic schedule if already enabled.
	if (pEhciCtrl->periodic_schedules > 0)
	{
		ehci_enable_periodic(pEhciCtrl);
	}
	ReleaseMutex(pEhciCtrl->hMutex);

	//Release all memories.
	_hx_free(pIsoDesc->itdArray);
	DestroyEvent(pIsoDesc->hEvent);
	_hx_free(pIsoDesc);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}
