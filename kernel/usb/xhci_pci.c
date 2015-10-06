/*
* Copyright (c) 2015, Google, Inc
* Written by Simon Glass <sjg@chromium.org>
* All rights reserved.
*
* SPDX-License-Identifier:	GPL-2.0
*/

#include "xhci.h"
#include <pci_drv.h>

//This file only available when xHCI is enabled.
#ifdef CONFIG_USB_XHCI

#define XHCI_PCI_CLASS_IDENTIFIER 0x0C0330

/*
* Create the appropriate control structures to manage a new XHCI host
* controller.
*/
int xhci_hcd_init(int index, struct xhci_hccr **ret_hccr,
struct xhci_hcor **ret_hcor)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	__PHYSICAL_DEVICE* pUsbCtrl = NULL;
	__IDENTIFIER id;
	uint32_t cmd;
	u16 vid, did;
	u32 base;
	LPVOID pRegBase = NULL;
	int len;
	int ret = -1;

	//Try to locate XHCI controller's physical device in system.
	id.dwBusType = BUS_TYPE_PCI;
	id.Bus_ID.PCI_Identifier.ucMask = PCI_IDENTIFIER_MASK_CLASS;
	id.Bus_ID.PCI_Identifier.dwClass = (XHCI_PCI_CLASS_IDENTIFIER << 8);
	pUsbCtrl = DeviceManager.GetDevice(&DeviceManager,
		BUS_TYPE_PCI,
		&id,
		NULL);
	if (NULL == pUsbCtrl)  //Can not find the device.
	{
		_hx_printf("XHCI: Can not find XHCI controller in system.\r\n");
		goto __TERMINAL;
	}

	//Get the vendor ID and device ID and the base address.
	vid = (u16)pUsbCtrl->ReadDeviceConfig(pUsbCtrl, PCI_CONFIG_OFFSET_VENDOR, sizeof(vid));
	did = (u16)pUsbCtrl->ReadDeviceConfig(pUsbCtrl, PCI_CONFIG_OFFSET_DEVICE, sizeof(did));
	printf("XHCI pci controller (Vendor: %04X, Device: %04X) found.\r\n", vid, did);
	base = pUsbCtrl->ReadDeviceConfig(pUsbCtrl, PCI_CONFIG_OFFSET_BASE1, sizeof(base));
	printf("XHCI regs address 0x%08x\r\n", base);
	//Reserve the config register space in Virtual Memory Space.
#ifdef __CFG_SYS_VMM
	pRegBase = VirtualAlloc((LPVOID)base, 0x1000, VIRTUAL_AREA_ALLOCATE_IO,
		VIRTUAL_AREA_ACCESS_RW,
		"xHCI Regs");
	if (NULL == pRegBase)
	{
		goto __TERMINAL;
	}
	if (base != (u32)pRegBase)
	{
		goto __TERMINAL;
	}
#endif

	hccr = (struct xhci_hccr *)pRegBase;
	len = HC_LENGTH(xhci_readl(&hccr->cr_capbase));
	hcor = (struct xhci_hcor *)((uint32_t)hccr + len);

	debug("XHCI-PCI init hccr 0x%x and hcor 0x%x hc_length %d\n",
		(uint32_t)hccr, (uint32_t)hcor, len);

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	/* enable busmaster */
	cmd = pUsbCtrl->ReadDeviceConfig(pUsbCtrl, PCI_CONFIG_OFFSET_COMMAND, 2);
	cmd |= 0x04;
	pUsbCtrl->WriteDeviceConfig(pUsbCtrl, PCI_CONFIG_OFFSET_COMMAND, cmd, 2);

	//Mark the return value as success.
	ret = 0;

__TERMINAL:
	if (ret)  //Failed of this routine.
	{
		if (pRegBase)
		{
			VirtualFree(pRegBase);
		}
	}
	return ret;
}

/*
* Destroy the appropriate control structures corresponding * to the XHCI host
* controller
*/
void xhci_hcd_stop(int index)
{
}

#endif //CONFIG_USB_XHCI
