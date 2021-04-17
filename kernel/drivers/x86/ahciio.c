//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 04, 2021
//    Module Name               : ahciio.c
//    Module Funciton           : 
//                                Source code for AHCI(Advanced Host Controller
//                                Interface) driver, I/O part.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ahcidef.h"
#include "ahci.h"

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

/* Helper routine to show out a port status. */
static void __show_ahci_port(__HBA_PORT* pPort)
{
	_hx_printf("  port is: 0x%X\r\n", pPort->is);
	_hx_printf("  port cmd: 0x%X\r\n", pPort->cmd);
	_hx_printf("  port tfd: 0x%X\r\n", pPort->tfd);
	_hx_printf("  port pxssts: 0x%X\r\n", pPort->ssts);
	_hx_printf("  port pxserr: 0x%X\r\n", pPort->serr);
	_hx_printf("  port pxsact: 0x%X\r\n", pPort->sact);
}

/* Find a free command list slot. */
static int find_cmdslot(__HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = (port->sact | port->ci);
	for (int i = 0; i < AHCI_MAX_CMDLIST_ENTRY; i++)
	{
		if ((slots & 1) == 0)
			return i;
		slots >>= 1;
	}
	__LOG("[%s]no free slot.\r\n", __func__);
	return -1;
}

/* 
 * Find a free cmd slot and commit the request. 
 * A D2H FIS will be sent from devide to host, 
 * DHRS interrupt will raise in this case.
 */
static BOOL __commit_port_request(__AHCI_PORT_OBJECT* port_obj, __AHCI_PORT_REQUEST* req)
{
	__HBA_PORT* port = NULL;
	/* spin timeout counter. */
	int spin = 0;
	int slot = -1;
	__HBA_CMD_HEADER *cmdheader = NULL;
	__HBA_CMD_TBL *cmdtbl = NULL;
	uint32_t sector_count = req->count;

	BUG_ON(NULL == port_obj);
	port = port_obj->pConfigRegister;
	slot = find_cmdslot(port);
	if (slot == -1)
	{
		_hx_printf("[%s]no free slot\r\n", __func__);
		return FALSE;
	}
	/* Save slot to request. */
	req->cmd_slot = slot;
	/* Clear pending interrupt bits. */
	//port->is = (uint32_t)-1;

	cmdheader = (__HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	if (__IN == req->inout)
	{
		/* Read. */
		cmdheader->w = 0;
	}
	else {
		/* write. */
		cmdheader->w = 1;
	}
	/* PRDT entries count. */
	cmdheader->prdtl = (uint16_t)((req->count - 1) >> 4) + 1;

	cmdtbl = (__HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(__HBA_CMD_TBL) +
		(cmdheader->prdtl - 1) * sizeof(__HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	uint8_t* buf = req->buffer;
	for (i = 0; i < cmdheader->prdtl - 1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
		/*
		 * 8K bytes (this value should always be set
		 * to 1 less than the actual value).
		 */
		cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;
		//cmdtbl->prdt_entry[i].i = 1;
		buf += 8 * 1024;
		/* 16 sectors paded. */
		sector_count -= 16;
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
	/* 512 bytes per sector. */
	cmdtbl->prdt_entry[i].dbc = (sector_count << 9) - 1;
	/* Only the last xfer triggers interrupt. */
	//cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	if (__IN == req->inout)
	{
		cmdfis->command = ATA_CMD_READ_DMA_EX;
	}
	else {
		cmdfis->command = ATA_CMD_WRITE_DMA_EX;
	}

	cmdfis->lba0 = (uint8_t)req->startl;
	cmdfis->lba1 = (uint8_t)(req->startl >> 8);
	cmdfis->lba2 = (uint8_t)(req->startl >> 16);
	cmdfis->device = 1 << 6;	// LBA mode

	cmdfis->lba3 = (uint8_t)(req->startl >> 24);
	cmdfis->lba4 = (uint8_t)req->starth;
	cmdfis->lba5 = (uint8_t)(req->starth >> 8);

	/* Use count in req instead of sector_count. */
	cmdfis->countl = req->count & 0xFF;
	cmdfis->counth = (req->count >> 8) & 0xFF;

	/*
	 * The below loop waits until the port is
	 * no longer busy before issuing a new command.
	 */
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		__LOG("[%s]port[%d] is hung\r\n", __func__, port_obj->port_index);
		return FALSE;
	}

	/* Issue command. */
	port->ci = 1 << slot;

#if 0
	/* Wait for completion. */
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			__LOG("[%s]Read disk error\r\n", __func__);
			__show_ahci_port(port);
			return FALSE;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		__LOG("[%s]Read disk error\r\n", __func__);
		__show_ahci_port(port);
		return FALSE;
	}
#endif

	return TRUE;
}

/* Cancel a committed request so as to avoid memory corruption. */
static BOOL __cancel_port_request(__AHCI_PORT_OBJECT* port_obj, __AHCI_PORT_REQUEST* req)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return FALSE;
}

/*
 * Local helper routine to check if a request is
 * success. It checks the returned information
 * from device in D2H FIS. The D2H
 * fis is copied into port request object in interrupt
 * handler.
 */
static BOOL __request_success(__AHCI_PORT_REQUEST* req)
{
	FIS_REG_D2H* d2h_fis = NULL;
	BOOL bResult = FALSE;

	d2h_fis = &req->fis_d2h;
	/* Check status and error. */
	if (d2h_fis->status & (1 << 0))
	{
		/* error raise. */
		goto __TERMINAL;
	}

#if 0
	/* Campare LBA address. */
	if (d2h_fis->lba0 != (uint8_t)req->startl)
	{
		goto __TERMINAL;
	}
	if (d2h_fis->lba1 != (uint8_t)(req->startl >> 8))
	{
		goto __TERMINAL;
	}
	if (d2h_fis->lba2 != (uint8_t)(req->startl >> 16))
	{
		goto __TERMINAL;
	}
	if (d2h_fis->lba3 != (uint8_t)(req->startl >> 24))
	{
		goto __TERMINAL;
	}
	if (d2h_fis->lba4 != (uint8_t)req->starth)
	{
		goto __TERMINAL;
	}
	if (d2h_fis->lba5 != (uint8_t)(req->starth >> 8))
	{
		goto __TERMINAL;
	}

	/* Campare sector counter xfered. */
	if (d2h_fis->countl != (req->count & 0xFF))
	{
		goto __TERMINAL;
	}
	if (d2h_fis->counth != ((req->count >> 8) & 0xFF))
	{
		goto __TERMINAL;
	}
#endif

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/*
 * Submit an I/O request to AHCI port object.
 * The r/w operation in interrupt mode. The routine
 * find a free command slot and fill it with the
 * request information and wait on the drcb object,
 * timeout or interrupt from controller will wake up
 * current thread. If there is(are) pending request(s),
 * the routine just link it to request list of port,
 * and wait on the drcb. Interrupt handler will submit
 * the request.
 * This is the first half part of a I/O operation and
 * executes in thread context.
 */
static BOOL __submit_port_request(__AHCI_PORT_OBJECT* pPort,
	__AHCI_PORT_REQUEST* req, __DRCB* drcb)
{
	BOOL bResult = FALSE;
	unsigned long ulFlags = 0;
	unsigned long wait_result = 0;
	__AHCI_PORT_REQUEST* pPortReq = NULL;

	/* Lock the drcb object. */
	if (!ObjectManager.AddRefCount(&ObjectManager, (__COMMON_OBJECT*)drcb))
	{
		/* drcb maybe destroyed. */
		_hx_printf("[%s]drcb may destroyed.\r\n", __func__);
		goto __TERMINAL;
	}

	__ENTER_CRITICAL_SECTION_SMP(pPort->spin_lock, ulFlags);
	if (pPort->request_num)
	{
		/* Pending req exist, just link it into tail of request list. */
		BUG_ON(NULL == pPort->request_list_tail);
		pPort->request_list_tail->lpNext = drcb;
		drcb->lpNext = NULL;
		drcb->lpPrev = pPort->request_list_tail;
		pPort->request_list_tail = drcb;
		pPort->request_num++;
	}
	else {
		/* First request. */
		BUG_ON(pPort->request_list || pPort->request_list_tail);
		BUG_ON(NULL == drcb->lpDrcbExtension);
		pPort->request_num++;
		pPort->request_list = pPort->request_list_tail = drcb;
		drcb->lpPrev = drcb->lpNext = NULL;
		/* Find a free slot and commit it. */
		bResult = __commit_port_request(pPort, (__AHCI_PORT_REQUEST*)drcb->lpDrcbExtension);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pPort->spin_lock, ulFlags);

	if (!bResult)
	{
		/* Port maybe hang. */
		/* Issue pending: should unlink the drcb from req list. */
		ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)drcb);
		goto __TERMINAL;
	}

	/* Wait the operation to complete. */
	BUG_ON(NULL == drcb->WaitForCompletion);
	wait_result = drcb->WaitForCompletion((__COMMON_OBJECT*)drcb);
	switch (wait_result)
	{
	case OBJECT_WAIT_RESOURCE:
		/* 
		 * request processed, drcb object
		 * is unlinked from list by interrupt hander.
		 * The request maybe fail because of hardware,
		 * so the result must be checked.
		 */
		pPortReq = (__AHCI_PORT_REQUEST*)drcb->lpDrcbExtension;
		bResult = __request_success(pPortReq);
		if (!bResult)
		{
			pPort->xfer_errors++;
		}
		else {
			/* update I/O statistics. */
			if (__IN == pPortReq->inout)
			{
				pPort->sectors_in += pPortReq->count;
			}
			else {
				pPort->sectors_out += pPortReq->count;
			}
		}
		break;
	case OBJECT_WAIT_TIMEOUT:
	case OBJECT_WAIT_DELETED:
	case OBJECT_WAIT_FAILED:
		/* 
		 * Exception case, the drcb should be unlinked 
		 * from request list, the request if commited should
		 * be canceled.
		 */
		__ENTER_CRITICAL_SECTION_SMP(pPort->spin_lock, ulFlags);
		if (drcb == pPort->request_list)
		{
			/* First req in list. */
			pPort->request_list = drcb->lpNext;
			if (drcb == pPort->request_list_tail)
			{
				/* Last one. */
				pPort->request_list_tail = NULL;
			}
			pPort->request_num--;
			__cancel_port_request(pPort, (__AHCI_PORT_REQUEST*)drcb->lpDrcbExtension);
			
			/* Commit the next request if exist. */
			if (pPort->request_list)
			{
				__commit_port_request(pPort, pPort->request_list->lpDrcbExtension);
			}
		}
		else {
			/* Not is the first one. */
			if (pPort->request_list_tail)
			{
				/* Last one in list. */
				pPort->request_list_tail = drcb->lpPrev;
			}
			else
			{
				BUG_ON((NULL == drcb->lpNext) || (NULL == drcb->lpPrev));
				drcb->lpPrev->lpNext = drcb->lpNext;
				drcb->lpNext->lpPrev = drcb->lpPrev;
			}
			pPort->request_num--;
		}
		__LEAVE_CRITICAL_SECTION_SMP(pPort->spin_lock, ulFlags);
		bResult = FALSE;
		break;
	default:
		break;
	}

	/* Release the drcb object. */
	ObjectManager.DestroyObject(&ObjectManager, (__COMMON_OBJECT*)drcb);

__TERMINAL:
	return bResult;
}

/*
 * Next half part of I/O in interrupt mode, this
 * routine will be invoked in DHRS interrupt hander.
 * It just wakeup the pending __DRCB and commit next
 * one if there is, or just do nothing if no pending
 * __DRCB request in list.
 */
int __int_handler_dhrs(__AHCI_PORT_OBJECT* pPortObject)
{
	unsigned long ulFlags;
	__DRCB* pDrcb = NULL;
	__AHCI_PORT_REQUEST* pPortReq = NULL;
	__HBA_FIS* pFis = NULL;
	__HBA_PORT* port_base = NULL;
	int ret = -1;

	__ENTER_CRITICAL_SECTION_SMP(pPortObject->spin_lock, ulFlags);
	pDrcb = pPortObject->request_list;
	if (NULL == pDrcb)
	{
		/* No pending request. */
		__LEAVE_CRITICAL_SECTION_SMP(pPortObject->spin_lock, ulFlags);
		goto __TERMINAL;
	}

	/* Remove the request from list and wakeup it. */
	pPortObject->request_list = pDrcb->lpNext;
	if (NULL == pPortObject->request_list)
	{
		/* No more request. */
		pPortObject->request_list_tail = NULL;
	}
	pPortObject->request_num--;

	/* Set request result accordingly. */
	pPortReq = (__AHCI_PORT_REQUEST*)pDrcb->lpDrcbExtension;
	BUG_ON(NULL == pPortReq);
	pPortReq->bReqResult = TRUE;
	/* Return the D2H fis. */
	port_base = pPortObject->pConfigRegister;
	pFis = (__HBA_FIS*)port_base->fb;
	memcpy((void*)&pPortReq->fis_d2h, (void*)&pFis->rfis, sizeof(FIS_REG_D2H));

	BUG_ON(NULL == pDrcb->OnCompletion);
	pDrcb->OnCompletion((__COMMON_OBJECT*)pDrcb);

	/* Commit next one if exist. */
	if (pPortObject->request_list)
	{
		__commit_port_request(pPortObject, pPortObject->request_list->lpDrcbExtension);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pPortObject->spin_lock, ulFlags);

	ret = 0;

__TERMINAL:
	return ret;
}

/*
 * Identify a disk attaching the specified port.
 * This routine is invoked in initialization phase of
 * HBA port, to get disk information.
 */
static BOOL __hba_port_identify(__AHCI_PORT_OBJECT *port_obj, uint8_t *buf)
{
	__HBA_PORT* port = NULL;
	int spin = 0; // Spin lock timeout counter
	int slot = -1;
	__HBA_CMD_HEADER *cmdheader = NULL;
	__HBA_CMD_TBL *cmdtbl = NULL;

	BUG_ON(NULL == port_obj);
	port = port_obj->pConfigRegister;
	slot = find_cmdslot(port);
	if (slot == -1)
	{
		_hx_printf("[%s]no free slot\r\n", __func__);
		return FALSE;
	}
	/* Clear pending interrupt bits. */
	port->is = (uint32_t)-1;

	cmdheader = (__HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	/* Read. */
	cmdheader->w = 0;
	/* PRDT entries count. */
	cmdheader->prdtl = 1;

	cmdtbl = (__HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(__HBA_CMD_TBL) +
		(cmdheader->prdtl - 1) * sizeof(__HBA_PRDT_ENTRY));

	cmdtbl->prdt_entry[0].dba = (uint32_t)buf;
	/* 512 bytes per sector. */
	cmdtbl->prdt_entry[0].dbc = 511;
	/* Only the last xfer triggers interrupt. */
	cmdtbl->prdt_entry[0].i = 1;

	/* Setup identify command. */
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
	memset((void*)cmdfis, 0, sizeof(FIS_REG_H2D));
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;
	cmdfis->device = 0;
	cmdfis->command = ATA_CMD_IDENTIFY;

	/*
	 * The below loop waits until the port is
	 * no longer busy before issuing a new command.
	 */
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		__LOG("[%s]port[%d] is hung\r\n", __func__, port_obj->port_index);
		return FALSE;
	}

	/* Issue command. */
	port->ci = 1 << slot;
	/* Wait for completion. */
	while (1)
	{
		/*
		 * In some longer duration reads, it may be 
		 * helpful to spin on the DPS bit in the PxIS port 
		 * field as well (1 << 5).
		 */
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)
		{
			/* Task file error. */
			__LOG("[%s]Read disk error\r\n", __func__);
			__show_ahci_port(port);
			return FALSE;
		}
	}

	/* Check again. */
	if (port->is & HBA_PxIS_TFES)
	{
		__LOG("[%s]Read disk error\r\n", __func__);
		__show_ahci_port(port);
		return FALSE;
	}

	return TRUE;
}

/*
 * Read *count* sectors content from the specified **port_obj**
 * and save into buf, apply polling mode.
 * count should not be changed in this routine.
 */
static BOOL __hba_port_read_poll(__AHCI_PORT_OBJECT *port_obj, uint32_t startl, 
	uint32_t starth, const uint32_t count, uint8_t *buf)
{
	__HBA_PORT* port = NULL;
	int spin = 0; // Spin lock timeout counter
	int slot = -1;
	__HBA_CMD_HEADER *cmdheader = NULL;
	__HBA_CMD_TBL *cmdtbl = NULL;
	uint32_t sector_count = count;
	
	BUG_ON(NULL == port_obj);
	port = port_obj->pConfigRegister;
	slot = find_cmdslot(port);
	if (slot == -1)
	{
		_hx_printf("[%s]no free slot\r\n", __func__);
		return FALSE;
	}
	/* Clear pending interrupt bits. */
	port->is = (uint32_t)-1;

	cmdheader = (__HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	/* Read. */
	cmdheader->w = 0;
	/* PRDT entries count. */
	cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;

	cmdtbl = (__HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(__HBA_CMD_TBL) +
		(cmdheader->prdtl - 1) * sizeof(__HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	for (i = 0; i < cmdheader->prdtl - 1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
		/*
		 * 8K bytes (this value should always be set 
		 * to 1 less than the actual value).
		 */
		cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;
		//cmdtbl->prdt_entry[i].i = 1;
		buf += 8 * 1024;
		/* 16 sectors paded. */
		sector_count -= 16;
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
	/* 512 bytes per sector. */
	cmdtbl->prdt_entry[i].dbc = (sector_count << 9) - 1;
	/* Only the last xfer triggers interrupt. */
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6;	// LBA mode

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	/* Use count instead of sector_count. */
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	/*
	 * The below loop waits until the port is 
	 * no longer busy before issuing a new command.
	 */
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		__LOG("[%s]port[%d] is hung\r\n", __func__, port_obj->port_index);
		return FALSE;
	}

	/* Issue command. */
	port->ci = 1 << slot;
	//_hx_printf("issue command to port[%d] at slot[%d],sector[%d]\r\n", 
	//	port_obj->port_index, slot, count);
	/* Wait for completion. */
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			__LOG("[%s]Read disk error\r\n", __func__);
			__show_ahci_port(port);
			return FALSE;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		__LOG("[%s]Read disk error\r\n", __func__);
		__show_ahci_port(port);
		return FALSE;
	}

	return TRUE;
}

/*
 * Read *count* sectors content from the specified **port_obj**
 * and save into buf.
 * count should not be changed in this routine.
 * If drcb_req is specified, then interrupt mode I/O
 * will be used, otherwise polling mode will apply.
 */
static BOOL __hba_port_read(__AHCI_PORT_OBJECT *port_obj, uint32_t startl,
	uint32_t starth, const uint32_t count, uint8_t *buf, __DRCB* drcb_req)
{
	__AHCI_PORT_REQUEST request;

	if (NULL == drcb_req)
	{
		return __hba_port_read_poll(port_obj, startl, starth, count, buf);
	}
	else {
		memset(&request, 0, sizeof(__AHCI_PORT_REQUEST));
		request.buffer = buf;
		request.cmd_slot = -1;
		request.count = count;
		request.inout = __IN;
		request.pPort = port_obj;
		request.starth = starth;
		request.startl = startl;
		/* Save to drcb. */
		drcb_req->lpDrcbExtension = (LPVOID)&request;
		/* Submit interupt mode request. */
		return __submit_port_request(port_obj, &request, drcb_req);
	}
}

/* 
 * write count sectors data in buf to port. 
 * count should not be changed in this routine.
 * Use interrupt mode.
 */
static BOOL __hba_port_write_poll(__AHCI_PORT_OBJECT *port_obj, uint32_t startl,
	uint32_t starth, const uint32_t count, uint8_t *buf)
{
	__HBA_PORT* port = NULL;
	int spin = 0; // Spin lock timeout counter
	int slot = -1;
	__HBA_CMD_HEADER *cmdheader = NULL;
	__HBA_CMD_TBL *cmdtbl = NULL;
	uint32_t sector_count = (uint32_t)count;

	BUG_ON(NULL == port_obj);
	port = port_obj->pConfigRegister;
	slot = find_cmdslot(port);
	if (slot == -1)
	{
		_hx_printf("[%s]no free slot\r\n", __func__);
		return FALSE;
	}
	/* Clear pending interrupt bits. */
	port->is = (uint32_t)-1;

	cmdheader = (__HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
	/* Write */
	cmdheader->w = 1;
	/* PRDT entries count. */
	cmdheader->prdtl = (uint16_t)((sector_count - 1) >> 4) + 1;

	cmdtbl = (__HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(__HBA_CMD_TBL) +
		(cmdheader->prdtl - 1) * sizeof(__HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	int i = 0;
	for (i = 0; i < cmdheader->prdtl - 1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
		/*
		 * 8K bytes (this value should always be set
		 * to 1 less than the actual value).
		 */
		cmdtbl->prdt_entry[i].dbc = 8 * 1024 - 1;
		//cmdtbl->prdt_entry[i].i = 1;
		buf += 8 * 1024;
		/* 16 sectors paded. */
		sector_count -= 16;
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)buf;
	/* 512 bytes per sector. */
	cmdtbl->prdt_entry[i].dbc = (sector_count << 9) - 1;
	/* Only the last xfer triggers interrupt. */
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_WRITE_DMA_EX;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	cmdfis->lba2 = (uint8_t)(startl >> 16);
	cmdfis->device = 1 << 6;	// LBA mode

	cmdfis->lba3 = (uint8_t)(startl >> 24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth >> 8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	/*
	 * The below loop waits until the port is
	 * no longer busy before issuing a new command.
	 */
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		__LOG("[%s]port[%d] is hung\r\n", __func__, port_obj->port_index);
		return FALSE;
	}

	/* Issue command. */
	port->ci = 1 << slot;
	//_hx_printf("issue command to port[%d] at slot[%d],sector[%d]\r\n",
	//	port_obj->port_index, slot, count);
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			__LOG("[%s]Read disk error\r\n", __func__);
			__show_ahci_port(port);
			return FALSE;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		__LOG("[%s]Read disk error\r\n", __func__);
		__show_ahci_port(port);
		return FALSE;
	}

	return TRUE;
}

/*
 * write count sectors data in buf to port.
 * count should not be changed in this routine.
 * If drcb_req is specified, then interrupt mode I/O
 * will be used, otherwise polling mode will apply.
 */
static BOOL __hba_port_write(__AHCI_PORT_OBJECT *port_obj, uint32_t startl,
	uint32_t starth, const uint32_t count, uint8_t *buf, __DRCB* drcb_req)
{
	__AHCI_PORT_REQUEST request;

	if (NULL == drcb_req)
	{
		/* Invoke polling mode. */
		return __hba_port_write_poll(port_obj, startl, starth, count, buf);
	}
	else
	{
		memset(&request, 0, sizeof(__AHCI_PORT_REQUEST));
		request.buffer = buf;
		request.cmd_slot = -1;
		request.count = count;
		request.inout = __OUT;
		request.pPort = port_obj;
		request.starth = starth;
		request.startl = startl;
		/* Save to drcb. */
		drcb_req->lpDrcbExtension = (LPVOID)&request;
		/* Submit interupt mode request. */
		return __submit_port_request(port_obj, &request, drcb_req);
	}
}

/* Set port operations. */
void SetPortOperations(__AHCI_PORT_OBJECT* pPort)
{
	BUG_ON(NULL == pPort);
	pPort->port_device_read = __hba_port_read;
	pPort->port_device_write = __hba_port_write;
	pPort->port_device_identify = __hba_port_identify;
	return;
}
