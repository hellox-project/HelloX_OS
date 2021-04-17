//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 23, 2020
//    Module Name               : ahci.c
//    Module Funciton           : 
//                                Source code for AHCI(Advanced Host Controller
//                                Interface) driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

/*
 * Procedures of AHCI controller's initialization:
 * 1. Enum the controller on PCI bus;
 * 2. For each device found:
 *    1) Create device object, connect interrupt, allocate
 *       memory maped space;
 *    2) Reset the controller;
 *    3) Init controller in low level, i.e, enable AE, controller
 *       level interrupt(hirarchy interrupt);
 *    4) Probe each port according port implementation register(pi);
 *    5) Send a init port message to ahci background thread;
 *
 * Port initialization procedure, running in ahci background thread:
 * 1. Allocate base memory for clb/clbu, fb/fbu;
 * 2. Create port object and init it;
 * 3. stop port(port should not start);
 * 4. set base address of command list, fis buffer, and others;
 * 5. init port in low level, i.e, enable port level interrupts;
 * 6. Spin up device attaching the port, if staggered spin up
 *    function is supported by controller;
 * 7. Wait driver ready(belong to staggered spin up);
 * 8. Get device type on port, save to port object;
 * 9. Start command engine(sets ST/FRE bits);
 * 10. Init storage device if it is.
 */

#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PCI_DRV.H>

#include "ahcidef.h"
#include "ahci.h"

/* How many ms for one wait try. */
#define AHCI_WAIT_TRY_TIME 10

/* 
 * AHCI controller list header. 
 * All controllers in system will be denoted by a
 * controller object and linked to this list.
 */
static __AHCI_CONTROLLER* ctrl_list_header = NULL;

/* Handle of AHCI background thread. */
static HANDLE ahci_back = NULL;

/* 
 * Int handler for DHRS(D2H register FIS recv). 
 * This routine is in ahciio.c file, since it will
 * invoke some local routines in that file.
 */
extern int __int_handler_dhrs(__AHCI_PORT_OBJECT* pPortObject);

/* Int handler for PSS(PIO Setup FIS recv). */
static int __int_handler_pss(__AHCI_PORT_OBJECT* pPortObject)
{
	return -1;
}

/* Int handler for DSS(DMA Setup FIS recv). */
static int __int_handler_dss(__AHCI_PORT_OBJECT* pPortObject)
{
	return -1;
}

/* Int handler for SDBS(Set Device Bits Int). */
static int __int_handler_sdbs(__AHCI_PORT_OBJECT* pPortObject)
{
	return -1;
}

/* Int handler for UFS(Unknown FIS recv). */
static int __int_handler_ufs(__AHCI_PORT_OBJECT* pPortObject)
{
	return -1;
}

/* Int handler for dps(Descriptor Processed recv). */
static int __int_handler_dps(__AHCI_PORT_OBJECT* pPortObject)
{
	return -1;
}

/* Int handler for PCS(Port Connect Change). */
static int __int_handler_pcs(__AHCI_PORT_OBJECT* pPortObject)
{
	/* Just clear the X bit of SERR. */
	pPortObject->pConfigRegister->serr &= (1 << 26);
	return -1;
}

/* Int handler for PRCS(PhyRdy Change). */
static int __int_handler_prcs(__AHCI_PORT_OBJECT* pPortObject)
{
	/* Just clear the N bit of SERR. */
	pPortObject->pConfigRegister->serr &= (1 << 16);
	return -1;
}

/* General interrupt handler of AHCI controller. */
static BOOL __ahci_int_handler(LPVOID lpESP, LPVOID lpParam)
{
	__AHCI_CONTROLLER* pController = (__AHCI_CONTROLLER*)lpParam;
	__HBA_MEM* pBase = NULL;
	__AHCI_PORT_OBJECT* pPortObject = NULL;
	__HBA_PORT* pPort = NULL;

	BUG_ON(NULL == pController);
	pBase = pController->pConfigRegister;

	if (0 == pBase->is)
	{
		/* Interrupt for other device. */
		return FALSE;
	}

	/* Travel the whole port list. */
	pPortObject = pController->pPortList;
	while (pPortObject)
	{
		pPort = pPortObject->pConfigRegister;
		if (pPort->is)
		{
			/* 
			 * Invoke corresponding handlers according 
			 * status, and clear the corresponding bit.
			 */
			if (pPort->is & AHCI_PxINT_DHRS)
			{
				__int_handler_dhrs(pPortObject);
				pPort->is &= AHCI_PxINT_DHRS;
				pPortObject->int_dhrs_raised++;
			}
			if (pPort->is & AHCI_PxINT_PSS)
			{
				__int_handler_pss(pPortObject);
				pPort->is &= AHCI_PxINT_PSS;
				pPortObject->int_pss_raised++;
			}
			if (pPort->is & AHCI_PxINT_DSS)
			{
				__int_handler_dss(pPortObject);
				pPort->is &= AHCI_PxINT_DSS;
				pPortObject->int_dss_raised++;
			}
			if (pPort->is & AHCI_PxINT_SDBS)
			{
				__int_handler_sdbs(pPortObject);
				pPort->is &= AHCI_PxINT_SDBS;
				pPortObject->int_sdbs_raised++;
			}
			if (pPort->is & AHCI_PxINT_UFS)
			{
				__int_handler_ufs(pPortObject);
				pPort->is &= AHCI_PxINT_UFS;
				pPortObject->int_ufs_raised++;
			}
			if (pPort->is & AHCI_PxINT_DPS)
			{
				__int_handler_dps(pPortObject);
				pPort->is &= AHCI_PxINT_DPS;
				pPortObject->int_dps_raised++;
			}
			if (pPort->is & AHCI_PxINT_PCS)
			{
				__int_handler_pcs(pPortObject);
				pPort->is &= AHCI_PxINT_PCS;
				pPortObject->int_pcs_raised++;
			}
			if (pPort->is & AHCI_PxINT_PRCS)
			{
				__int_handler_prcs(pPortObject);
				pPort->is &= AHCI_PxINT_PRCS;
				pPortObject->int_prcs_raised++;
			}
			
			/* Still has unhandled interrupt. */
			if (pPort->is)
			{
				_hx_printf("[%s]unknown interrupt[is = 0x%X]\r\n",
					__func__,
					pPort->is);
				pPort->is = (uint32_t)-1;
				/* IS = 0x400040*/
			}
			/* Increment interrupt counter. */
			pPortObject->int_raised++;
		}
		pPortObject = pPortObject->pNext;
	}

	/* Clear controller's is register, with WC. */
	pBase->is = (uint32_t)-1;

	return TRUE;
}

/*
 * Helper routine to check the type of a device 
 * which is attaching to one HBA port.
 */
static int __ahci_check_port(__AHCI_PORT_OBJECT* port_obj)
{
	__HBA_PORT* port = port_obj->pConfigRegister;
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	/* Check drive status. */
	if (det != HBA_PORT_DET_PRESENT)
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	/* Save status to port object. */
	port_obj->port_status = ssts;

	/* 
	 * Return device type according 
	 * port signature's value.
	 */
	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

/*
 * Spin up a device attaching to the HBA port designated
 * by port if staggered spin-up function is supported by
 * the HBA controller.
 */
static BOOL __spin_up_device(__HBA_PORT* pPort)
{
	BOOL bResult = FALSE;
	int count = 0;

	BUG_ON(NULL == pPort);
	/* Set FRE bit first. */
	pPort->cmd |= HBA_PxCMD_FRE;
	/* Spin up the device on this port. */
	pPort->cmd |= HBA_PxCMD_SUD;
	/* Polling of device presence, 50ms at most. */
	count = 5;
	while (count--)
	{
		/*
		 * ssts det value of port is 1 (presence but no communication)
		 * or 3(presence & communication) indicates device presence.
		 */
		if (3 == (pPort->ssts & 0x0F) ||
			1 == (pPort->ssts & 0xF))
		{
			break;
		}
		__MicroDelay(AHCI_WAIT_TRY_TIME);
	}
	if (0 == count)
	{
		/* No device present on this port. */
		goto __TERMINAL;
	}
	/* Device presence, clear serr */
	pPort->serr = -1;
	/* Wait for drive ready. */
	count = 5;
	while (((pPort->tfd & HBA_PxTFD_BSY) ||
		(pPort->tfd & HBA_PxTFD_DRQ) ||
		(pPort->tfd & HBA_PxTFD_ERR)) && count--)
	{
		__MicroDelay(AHCI_WAIT_TRY_TIME);
	}
	if (0 == count)
	{
		/* Drive is not ready after wait. */
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* If the controller supports staggered spin up. */
static BOOL __staggered_spin_up(__AHCI_CONTROLLER* pController)
{
	__HBA_MEM* pConfigReg = pController->pConfigRegister;

	/* The SSS bit(27) of cap register indicates this ability. */
	if (0 == (pConfigReg->cap & (1 << 27)))
	{
		return FALSE;
	}
	return TRUE;
}

/* Start command engine of one port. */
static BOOL __port_start_cmd(__AHCI_PORT_OBJECT* pPort)
{
	int wait_count = 5;
	BOOL bResult = FALSE;
	__HBA_PORT* port = pPort->pConfigRegister;
	__AHCI_CONTROLLER* pController = pPort->pCtrl;

	/* 
	 * Wait until CR (bit15), ST, FRE, FR
	 * are cleared.
	 */
	while (((port->cmd & HBA_PxCMD_CR) ||
		(port->cmd & HBA_PxCMD_ST) ||
		(port->cmd & HBA_PxCMD_FR) ||
		(port->cmd & HBA_PxCMD_FRE)) && wait_count--)
	{
		__MicroDelay(AHCI_WAIT_TRY_TIME);
	}
	if (0 == wait_count)
	{
		/* port engine is still running. */
		goto __TERMINAL;
	}

	/* 
	 * spin up device on this port if 
	 * staggered spin up is supported by controller. 
	 */
	if (__staggered_spin_up(pController))
	{
		if (!__spin_up_device(port))
		{
			goto __TERMINAL;
		}
	}

	/* Get device type attaching on this port. */
	pPort->device_type = __ahci_check_port(pPort);
	if (AHCI_DEV_NULL == pPort->device_type)
	{
		/* No device. */
		goto __TERMINAL;
	}

	/* Start engine. Set FRE (bit4) and ST (bit0). */
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Stop command engine of one port. */
static void __port_stop_cmd(__HBA_PORT *port)
{
	int wait_count = 5;

	/* Clear st and fre bits of cmd. */
	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;

	/* 
	 * Wait until FR (bit14), CR (bit15) are cleared,
	 * and wait 50 ms at most.
	 */
	while (wait_count--)
	{
		if (0 == (port->cmd & HBA_PxCMD_FR) &&
			0 == (port->cmd & HBA_PxCMD_CR))
		{
			break;
		}
		__MicroDelay(AHCI_WAIT_TRY_TIME);
	}
}

/* Local helper routine to init a ahci port in low level. */
static BOOL __lowlevel_init_port(__HBA_PORT* port)
{
	/* Enable all interrupts. */
	port->serr = (uint32_t)-1;
	port->is = (uint32_t)-1;
	port->ie = (AHCI_PxINT_DHRS |
		AHCI_PxINT_PSS |
		AHCI_PxINT_DSS |
		AHCI_PxINT_SDBS |
		AHCI_PxINT_UFS |
		AHCI_PxINT_DPS |
		AHCI_PxINT_PCS);

	return TRUE;
}

/* 
 * Initializes a ahci port. 
 * Allocates cache disabled memory and
 * initializes it as port's command list,
 * command table, FIS, ...
 * Stop and start the port before and after
 * that.
 */
static __AHCI_PORT_OBJECT* __init_port_device(__AHCI_CONTROLLER* pController, int port_index)
{
	char* pPortBase = NULL;
	char* pBaseOffset = NULL;
	BOOL bResult = FALSE;
	__HBA_PORT* port = NULL;
	__AHCI_PORT_OBJECT* pPortObject = NULL;
	unsigned long ulFlags;

	BUG_ON(NULL == pController);
	BUG_ON(port_index >= AHCI_MAX_PORT_NUM);
	port = &pController->pConfigRegister->ports[port_index];

	/* 
	 * Allocate a batch of memory for port. 
	 * About 12K memory is used for each AHCI port.
	 * Please be noted the memory region must be
	 * cache disabled since it will be used by device
	 * with DMA enable.
	 * ---- PENDING ISSUE.
	 */
	pPortBase = (char*)_hx_aligned_malloc(12 * 1024, 4096);
	if (NULL == pPortBase)
	{
		__LOG("[%s]no memory for port.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(pPortBase, 0, 12 * 1024);

	/* Create and init a port object. */
	pPortObject = (__AHCI_PORT_OBJECT*)_hx_malloc(sizeof(__AHCI_PORT_OBJECT));
	if (NULL == pPortObject)
	{
		__LOG("[%s]out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(pPortObject, 0, sizeof(__AHCI_PORT_OBJECT));
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pPortObject->spin_lock, "portobj");
#endif
	pPortObject->pConfigRegister = port;
	pPortObject->port_index = port_index;
	pPortObject->pCtrl = pController;
	pPortObject->device_type = AHCI_DEV_NULL;
	pPortObject->port_status = HBA_PORT_NOT_START;
	/* Init refcount as 1. */
	pPortObject->ref_count = 1;
	/* Set port operations. */
	SetPortOperations(pPortObject);

	/* Stop the port first. */
	__port_stop_cmd(port);

	/* 
	 * Initializes port space memory. 
	 * Allocations of the memory as following:
	 * 1. Command list,32 entries,32 bytes for each, total 1024;
	 * 2. Received FIS,256 bytes fixed;
	 * 3. 32 command tables,corresponding to 32 command list:
	 *   3.1 Command FIS, 64 bytes fixed;
	 *   3.2 ATAPI command,16 bytes;
	 *   3.3 48 bytes reserved;
	 *   3.4 SATA_PRDT_PER_COMMAND PRDT entries,16 bytes for each.
	 *   3.5 so each command table's size is:
	 *       64 + 16 + 48 + 16 * SATA_PRDT_PER_COMMAND
	 */
	pBaseOffset = pPortBase;
	port->clb = (uint32_t)pBaseOffset;
	port->clbu = 0;
	pBaseOffset += 1024;

	port->fb = (uint32_t)pBaseOffset;
	port->fbu = 0;
	pBaseOffset += 256;

	/* Init command tables,it's must be 128 bytes alignment. */
	__HBA_CMD_HEADER* pCmdHdr = (__HBA_CMD_HEADER*)(port->clb);
	for (int i = 0; i < 32; i++)
	{
		pCmdHdr[i].prdtl = SATA_PRDT_PER_COMMAND;
		pCmdHdr[i].ctba = (uint32_t)pBaseOffset;
		pCmdHdr[i].ctbau = 0;
		pBaseOffset += (64 + 16 + 48);
		pBaseOffset += sizeof(__HBA_PRDT_ENTRY) * SATA_PRDT_PER_COMMAND;
	}

	/* Link the port object into hosting controller. */
	__ENTER_CRITICAL_SECTION_SMP(pController->spin_lock, ulFlags);
	pController->port_num++;
	pPortObject->pNext = pController->pPortList;
	pController->pPortList = pPortObject;
	__LEAVE_CRITICAL_SECTION_SMP(pController->spin_lock, ulFlags);

	/* Low level init the port. */
	__lowlevel_init_port(port);
	/* Start port again. */
	if (!__port_start_cmd(pPortObject))
	{
		goto __TERMINAL;
	}

	bResult = TRUE;
	_hx_printf("[%s]init port[%d] @base[0x%X].\r\n", __func__, port_index, pPortBase);

__TERMINAL:
	if (!bResult)
	{
		/* 
		 * Should not release port object since it's 
		 * already linked into controller's port list.
		 */
#if 0
		/* Init port fail,release any allocated res. */
		if (pPortBase)
		{
			_hx_free(pPortBase);
		}
		if (pPortObject)
		{
			_hx_free(pPortObject);
			pPortObject = NULL;
		}
#endif
	}
	return pPortObject;
}

/* 
 * Initializes a SATA device that attaching
 * to one port of AHCI controller.
 * The port structure denotes the port
 * hosting the SATA device.
 * Implementation: in order to make the OS init
 * process fast,we postpone this work to working 
 * thread of AHCI controller.
 */
static BOOL __init_port_msg(__AHCI_CONTROLLER* pController, int port_index)
{
	__KERNEL_THREAD_MESSAGE msg;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pController);
	BUG_ON(port_index >= AHCI_MAX_PORT_NUM);

	/* Back ground thread must be created. */
	BUG_ON(NULL == ahci_back);

	/* 
	 * Send init message to bg. 
	 * Please be noted the rescheduling flag of
	 * send message must be disabled,since now is
	 * in system initialization process.
	 */
	msg.wCommand = AHCI_MSG_INIT_PORT;
	msg.dwParam = (unsigned long)pController;
	msg.wParam = (WORD)port_index;
	bResult = KernelThreadManager.SendMessageEx(ahci_back, 
		&msg, 
		FALSE);
	return bResult;
}

/*
 * Helper routine to probe each port of
 * AHCI controller.
 * Initializes the port according to it's
 * type in this process.
 */
static int __ahci_probe_port(__AHCI_CONTROLLER* pController)
{
	uint32_t pi = 0;
	int i = 0, valid_dev = 0;
	__HBA_MEM *abar = NULL;

	BUG_ON(NULL == pController);
	BUG_ON(NULL == pController->pConfigRegister);
	abar = pController->pConfigRegister;
	pi = abar->pi;

	while (i < 32)
	{
		if (pi & 1)
		{
			/* 
			 * port initialized in background work thread,
			 * just send message to triger it.
			 */
			if (__init_port_msg(pController, i))
			{
				valid_dev++;
			}
		}
		pi >>= 1;
		i++;
	}
	return valid_dev;
}

#if 0
/* Obsoleted. */
static int __ahci_probe_port_old(__AHCI_CONTROLLER* pController)
{
	uint32_t pi = 0;
	int i = 0, valid_dev = 0;
	__HBA_MEM *abar = NULL;

	BUG_ON(NULL == pController);
	BUG_ON(NULL == pController->pConfigRegister);
	abar = pController->pConfigRegister;
	pi = abar->pi;

	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = __ahci_check_port(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				//_hx_printf("[%s]SATA drive found at port %d\n", __func__, i);
				if (__init_port_msg(pController, i))
				{
					valid_dev++;
				}
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				_hx_printf("[%s]SATAPI drive found at port %d\n", __func__, i);
				valid_dev++;
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				_hx_printf("[%s]SEMB drive found at port %d\n", __func__, i);
				valid_dev++;
			}
			else if (dt == AHCI_DEV_PM)
			{
				_hx_printf("[%s]PM drive found at port %d\n", __func__, i);
				valid_dev++;
			}
			else
			{
				_hx_printf("[%s]No drive found at port %d\n", __func__, i);
			}
		}

		pi >>= 1;
		i++;
	}
	return valid_dev;
}
#endif

#define AHCI_IO_REQUEST_LENGTH (64 * 1024)
static char content_pad[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/* Temporary helper to fill a buffer. */
static void __fill_buff(char* buff)
{
	int round = AHCI_IO_REQUEST_LENGTH / sizeof(content_pad);
	
	for (int i = 0; i < round; i++)
	{
		memcpy(buff, (void*)&content_pad[0], sizeof(content_pad));
		buff += sizeof(content_pad);
	}
}

/* Main routine of AHCI's background thread. */
static unsigned long __ahci_back_ground(void* pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	__AHCI_PORT_OBJECT* pPort = NULL;
	char* buff = NULL;

	/* Allocate temporary buffer to hold device data. */
	buff = _hx_malloc(AHCI_IO_REQUEST_LENGTH);
	if (NULL == buff)
	{
		_hx_printf("[%s]out of memory\r\n", __func__);
		return 0;
	}

	/* Main loop. */
	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case AHCI_MSG_INIT_PORT:
				pPort = __init_port_device((__AHCI_CONTROLLER*)msg.dwParam, (int)msg.wParam);
				if (NULL == pPort)
				{
					__LOG("[%s]init port fail\r\n", __func__);
					break;
				}
				/* Init the storage attaching to this port. */
				if ((AHCI_DEV_SATA == pPort->device_type) ||
					(AHCI_DEV_SATAPI == pPort->device_type) ||
					(AHCI_DEV_SEMB == pPort->device_type))
				{
					if (!InitPortStorage(pPort))
					{
						__LOG("[%s]init port storage fail\r\n", __func__);
						break;
					}
				}
#if 0
				/* Try to fetch several sectors from port device. */
				int sector_count = AHCI_IO_REQUEST_LENGTH / 512;
				int sector_start = 16;
				if (pPort->port_device_read(pPort, 0, 0, sector_count, buff))
				{
					_hx_printf("Read sector from port[%d] OK\r\n", pPort->port_index);
				}
				else {
					_hx_printf("Read sector from port[%d] fail.\r\n", pPort->port_index);
				}
				/* Try to write several sectors into disk. */
				__fill_buff(buff);
				for (int i = 0; i < 16; i++)
				{
					if (!pPort->port_device_write(pPort, i * sector_count + sector_start,
						0, sector_count, buff))
					{
						_hx_printf("Failed to write port[%d]\r\n", pPort->port_index);
						break;
					}
				}
#endif
				break;
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;
			default:
				break;
			}
		}
	}

__TERMINAL:
	/* Resource should be cleared up here. */
	return 0;
}

/* Perform OS/BIOS ownership handover. */
static void __bios_os_ho(__HBA_MEM* ahci_base)
{
	int spin_count = 0;

	/* 
	 * OS/BIOS handover function is optional 
	 * so first check if the controller supports
	 * it, the 1st bit of cap2 indicates this.
	 */
	if (ahci_base->cap2 & (1 << 0))
	{
		/* Set the OOS bit. */
		ahci_base->bohc |= (1 << 1);
		/* Spin on BOS bit, for max 2 seconds. */
		while (ahci_base->bohc & (1 << 0))
		{
			__MicroDelay(AHCI_WAIT_TRY_TIME);
			spin_count++;
			if (spin_count > (2000 / AHCI_WAIT_TRY_TIME))
			{
				_hx_printf("[%s]os/bios handover fail.\r\n", __func__);
				break;
			}
		}
	}
}

/* Hardware reset of the ahci controller. */
static BOOL __controller_reset(__HBA_MEM* ahci_base)
{
	unsigned int loop_count = 5;

	/* Perform OS/BIOS handover. */
	__bios_os_ho(ahci_base);

	/* reset the whole controller. */
	ahci_base->ghc |= (1 << 0);
	while (TRUE)
	{
		/* Poll the reset process. */
		if (ahci_base->ghc & 0x01)
		{
			/* Wait 200ms. */
			__MicroDelay(200);
			loop_count--;
		}
		else
		{
			break;
		}
		/* reset timeout. */
		if (0 == loop_count)
		{
			_hx_printf("[%s]reset controller error!\r\n", __func__);
			return FALSE;
		}
	}
	return TRUE;
}

/* Low level init of ahci controller. */
static BOOL __lowlevel_init_controller(__HBA_MEM* ahci_base)
{	
	/* Enable ahci mode and interrupt. */
	ahci_base->ghc |= (1 << 31);
	ahci_base->ghc |= (1 << 1);
	return TRUE;
}

/* Helper routine to initializes one AHCI controller. */
static BOOL __init_ahci_controller(__AHCI_CONTROLLER* pController)
{
	__HBA_MEM* ahci_ghc = NULL;
	__HBA_PORT* ahci_port = NULL;
	int valid_dev = 0;

	BUG_ON(NULL == pController);

	ahci_ghc = (__HBA_MEM*)pController->mem_start;
	BUG_ON(NULL == ahci_ghc);

	/* Reset the controller. */
	if (!__controller_reset(ahci_ghc))
	{
		return FALSE;
	}

	/* Low level init the controller. */
	if (!__lowlevel_init_controller(ahci_ghc))
	{
		return FALSE;
	}

	/* Create the background thread if not yet. */
	if (NULL == ahci_back)
	{
		ahci_back = CreateKernelThread(0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_HIGH,
			__ahci_back_ground,
			NULL, NULL,
			AHCI_BG_THREAD_NAME);
		if (NULL == ahci_back)
		{
			__LOG("[%s]create bg thread fail.\r\n", __func__);
			return FALSE;
		}
	}

	/* Probe the controller. */
	valid_dev = __ahci_probe_port(pController);
	if (!valid_dev)
	{
		return FALSE;
	}

	return TRUE;
}

/* 
 * Local helper routine to enumerate all AHCI 
 * controllers in system.
 */
static BOOL __enum_ahci_controller(__DRIVER_OBJECT* pDriverObject)
{
	BOOL bResult = FALSE;
	__IDENTIFIER id;
	__PHYSICAL_DEVICE* pDev = NULL;
	int interrupt = 0;
	unsigned long mem_start = 0, mem_end = 0;
	unsigned long mem_size = 0;
	int index = 0, device_index = 0;
	__AHCI_CONTROLLER* pController = NULL;

	/* Use id as filter to find AHCI controller. */
	id.dwBusType = BUS_TYPE_PCI;
	id.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_CLASS;
	id.Bus_ID.PCI_Identifier.dwClass = PCI_CLASS_AHCI;
	pDev = GetDevice(&id, NULL);
	while (pDev)
	{
		/* 
		 * Find one AHCI controller device. 
		 * Get the interrupt line and memory maped I/O.
		 */
		for (index = 0; index < MAX_RESOURCE_NUM; index++)
		{
			if (RESOURCE_TYPE_INTERRUPT == pDev->Resource[index].dwResType)
			{
				interrupt = pDev->Resource[index].Dev_Res.ucVector;
			}
			if (RESOURCE_TYPE_MEMORY == pDev->Resource[index].dwResType)
			{
				/* Memory maped I/O space. */
				mem_start = (unsigned long)pDev->Resource[index].Dev_Res.MemoryRegion.lpStartAddr;
				mem_end = (unsigned long)pDev->Resource[index].Dev_Res.MemoryRegion.lpEndAddr;
				mem_size = mem_end - mem_start;
				mem_size += 1;
			}
		}

		/* Save the device configuration into ahci object. */
		if ((interrupt) && (mem_size))
		{
			/* Create a new AHCI controller object. */
			pController = (__AHCI_CONTROLLER*)_hx_malloc(sizeof(__AHCI_CONTROLLER));
			if (NULL == pController)
			{
				__LOG("[%s]out of memory\r\n", __func__);
				goto __TERMINAL;
			}
			memset(pController, 0, sizeof(__AHCI_CONTROLLER));

			/* Initializes the controller. */
			pController->interrupt = interrupt;
			pController->mem_start = mem_start;
			pController->mem_end = mem_end;
			pController->mem_size = mem_size;
			__INIT_SPIN_LOCK(pController->spin_lock, "ahci");
			_hx_sprintf(pController->ctrl_name, "\\\\.\\AHCI%d", device_index);
			pController->pConfigRegister = (__HBA_MEM*)mem_start;
			device_index++;

			/* 
			 * Create the corresponding device object so 
			 * as the object could register into system.
			 */
			pController->device_handle = CreateDevice(
				pController->ctrl_name,
				0, 512, 16384, 16384, (LPVOID)pController, pDriverObject);
			if (NULL == pController->device_handle)
			{
				__LOG("[%s]create device failed.\r\n", __func__);
				_hx_free(pController);
				goto __TERMINAL;
			}

			/* Connect interrupt. */
			pController->interrupt_handle = ConnectInterrupt(
				"int_ahci", __ahci_int_handler, pController, interrupt + INTERRUPT_VECTOR_BASE);
			if (NULL == pController->interrupt_handle)
			{
				__LOG("[%s]failed to connect interrupt.\r\n", __func__);
				DestroyDevice(pController->device_handle);
				_hx_free(pController);
				goto __TERMINAL;
			}

			/* Map the i/o space. */
			if (VirtualAlloc((LPVOID)mem_start, mem_size, VIRTUAL_AREA_ALLOCATE_IO,
				VIRTUAL_AREA_ACCESS_RW, "ahci") != (LPVOID)mem_start)
			{
				__LOG("[%s]failed to map i/o.\r\n", __func__);
				DestroyDevice(pController->device_handle);
				DisconnectInterrupt(pController->interrupt_handle);
				_hx_free(pController);
				goto __TERMINAL;
			}

			/* Now initializes the AHCI controller. */
			if (!__init_ahci_controller(pController))
			{
				__LOG("[%s]failed to init controller.\r\n", __func__);
				DestroyDevice(pController->device_handle);
				DisconnectInterrupt(pController->interrupt_handle);
				_hx_free(pController);
				goto __TERMINAL;
			}

			/* OK, link it to local list. */
			pController->pNext = ctrl_list_header;
			ctrl_list_header = pController;

			/* Mark success. */
			bResult = TRUE;
		}

		/* Reset local variables. */
		mem_start = mem_end = 0;
		interrupt = 0;

		/* Find next ahci controller. */
		pDev = GetDevice(&id, pDev);
	}

__TERMINAL:
	return bResult;
}

/* Entry point of AHCI driver. */
BOOL AHCIDriverEntry(__DRIVER_OBJECT* pDriverObject)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pDriverObject);

	/* Init driver operations. */
	SetDrvOperations(pDriverObject);

	/* Enumerate all AHCI controllers. */
	if (!__enum_ahci_controller(pDriverObject))
	{
		_hx_printf("[%s]No AHCI controller found.\r\n", __func__);
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}
