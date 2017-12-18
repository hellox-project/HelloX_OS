//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 16, 2015
//    Module Name               : usb_diag.c
//    Module Funciton           : 
//                                Diagnostic code of USB functions are put
//                                into this file.
//    Last modified Author      : Garry
//    Last modified Date        : Feb 08,2016
//    Last modified Content     : Now it's the first day morning,1 o'clock of the
//                                monkey year,2016,according to China's traditional
//                                calendar.Wish HelloX OS can get take off in this
//                                year,and wish anything go peace...
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

//USB base class description array.
static struct UsbBaseDesc{
	unsigned char index;
	char* desc;
}__UsbBaseDesc[] = {
	{ 0, "Shoud check farther." },
	{ 1, "Audio functions." },
	{ 2, "Communications and CDC control." },
	{ 3, "HID(Human Interface Devices)." },
	{ 5, "Physical." },
	{ 6, "Image device." },
	{ 7, "USB Printer." },
	{ 8, "USB Mass storage." },
	{ 9, "USB Hub device." },
	{ 10, "CDC-Data." },
	{ 11, "Smart Card." },
	{ 13, "Content security." },
	{ 14, "Video devices." },
	{ 15, "Personal Healthcare devices." },
	{ 0xDC, "Diagnostic devices." },
	{ 0xE0, "Wireless Controller." },
	{ 0xEF, "Miscellaneous devices." },
	{ 0xFE, "Application specific devices." },
	{ 0xFF, "Vendor specific." },
	{0, NULL} //Terminator of this array,desc as NULL.
	};

//Several helper routines to assist the USB diagnostic routines.
static int usb_clear_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
		port, NULL, 0, USB_CNTL_TIMEOUT);
}

static int usb_set_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
		port, NULL, 0, USB_CNTL_TIMEOUT);
}

//Show out an end point.
static void ShowEndPoint(struct usb_endpoint_descriptor* pEndPoint)
{
	char* format = "    ep# = %d,direct = %s,attr = %s,max_pksz = %d,multi = %d,bInt = %d.\r\n";
	__u8 epnum = get_unaligned(&pEndPoint->bEndpointAddress) & 0xF;
	char* dir = (get_unaligned(&pEndPoint->bEndpointAddress) & 0x80) ? "in" : "out";
	char* attr = NULL;
	__u16 max_pk_size = get_unaligned(&pEndPoint->wMaxPacketSize);
	__u16 multi = 0;
	__u8 Interval = get_unaligned(&pEndPoint->bInterval);

	//Set attribute value.
	switch (get_unaligned(&pEndPoint->bmAttributes) & 3)
	{
	case 0:
		attr = "ctrl";
		break;
	case 1:
		attr = "iso";
		break;
	case 2:
		attr = "bulk";
		break;
	case 3:
		attr = "int";
		break;
	}

	//Just show it,but wMaxPacketSize should be splitted as multi and packet size.
	multi = (max_pk_size >> 11) & 0x03;
	multi += 1;
	max_pk_size &= 0x7FF;
	_hx_printf(format, epnum, dir, attr, max_pk_size,multi, Interval);
}

//Dump out one USB interface's information.
static void ShowUsbInt(__USB_INTERFACE_FUNCTION* pIntFunc)
{
	struct usb_endpoint_descriptor* pEndpoint = NULL;
	int i = 0, j = 0;

	_hx_printf("  Int_class/Int_subclass/Int_proto: 0x%X/0x%X/0x%X\r\n",
		pIntFunc->pPrimaryInterface->desc.bInterfaceClass,
		pIntFunc->pPrimaryInterface->desc.bInterfaceSubClass,
		pIntFunc->pPrimaryInterface->desc.bInterfaceProtocol);
	//Show all endpoints,include the primary interface and it's alternate settings.
	_hx_printf("  Endpoint list(Pri):\r\n");
	for (i = 0; i < pIntFunc->pPrimaryInterface->no_of_ep; i++)
	{
		pEndpoint = &pIntFunc->pPrimaryInterface->ep_desc[i];
		ShowEndPoint(pEndpoint);
	}
	for (i = 0; i < pIntFunc->nAlternateNum; i++)  //Show 6 alternate settings.
	{
		_hx_printf("  Endpoint list(alt_set %d):\r\n", i + 1);
		for (j = 0; j < pIntFunc->pAltInterfaces[i]->no_of_ep; j++)
		{
			pEndpoint = &pIntFunc->pAltInterfaces[i]->ep_desc[j];
			ShowEndPoint(pEndpoint);
		}
	}
	return;
}

//Dump out one USB interface association information.
static void ShowUsbIntAssoc(__USB_INTERFACE_ASSOCIATION* pIntAssoc)
{
	int i;
	for (i = 0; i < pIntAssoc->nIntAssocNum; i++)
	{
		ShowUsbInt(&pIntAssoc->Interfaces[i]);
	}
	return;
}

//Print out one USB device's information
void ShowUsbDevice(int index)
{
	struct usb_device* pDev = NULL;
	__PHYSICAL_DEVICE* pPhyDev = USBManager.pUsbDeviceRoot;
	//struct usb_interface* pUsbInt = NULL;
	__USB_INTERFACE_ASSOCIATION* pIntAssoc = NULL;
	__USB_INTERFACE_FUNCTION* pIntFunc = NULL;
	int i = 0;

	if (0 == USBManager.nPhysicalDevNum)  //No USB physical device.
	{
		return;
	}
	if (index >= USBManager.nPhysicalDevNum)
	{
		_hx_printf("  Please specify a valid address value,range from [0] to [%d].\r\n",
			USBManager.nPhysicalDevNum - 1);
		return;
	}
	while (index)
	{
		pPhyDev = pPhyDev->lpNext;
		if (NULL == pPhyDev)
		{
			BUG();
			return;
		}
		index--;
	}
	if (!pPhyDev)
	{
		BUG();
		return;
	}

	pDev = (struct usb_device*)pPhyDev->lpPrivateInfo;
	pIntAssoc = (__USB_INTERFACE_ASSOCIATION*)pPhyDev->Resource[0].Dev_Res.usbIntAssocBase;
	if (NULL == pIntAssoc)
	{
		BUG();
		return;
	}

	if (NULL == pDev)
	{
		BUG();
	}

	_hx_printf("  Index#: %u\r\n  Dev_Name: %s\r\n  Manufacturer: %s\r\n  Product: %s\r\n  Serial: %s\r\n",
		index,
		pPhyDev->strName,
		pDev->mf,
		pDev->prod,
		pDev->serial);
	_hx_printf("  Class/Subclass/Proto: %d/%d/%d\r\n", pDev->descriptor.bDeviceClass,
		pDev->descriptor.bDeviceSubClass,
		pDev->descriptor.bDeviceProtocol);
	_hx_printf("  Vendor/Product/Device: 0x%X/0x%X/0x%X\r\n", pDev->descriptor.idVendor,
		pDev->descriptor.idProduct,
		pDev->descriptor.bcdDevice);
	if (1 == pIntAssoc->nIntAssocNum)  //Not interface association.
	{
		_hx_printf("  Normal USB interface function.\r\n");
		ShowUsbInt(&pIntAssoc->Interfaces[0]);
	}
	else
	{
		_hx_printf("  Interface assoc with [%d] interfaces.\r\n", pIntAssoc->nIntAssocNum);
		ShowUsbIntAssoc(pIntAssoc);
	}
	return;
}

static void ShowUsbGeneral(__PHYSICAL_DEVICE* pDevice,int index)
{
	int i = 0;
	char* desc = NULL;
	unsigned char base_class = 0;

	base_class = pDevice->DevId.Bus_ID.USB_Identifier.bDeviceClass;
	if (0 == base_class)  //Should retrieve interface's base class info.
	{
		base_class = pDevice->DevId.Bus_ID.USB_Identifier.bInterfaceClass;
	}

	while (__UsbBaseDesc[i].desc){
		if (base_class == __UsbBaseDesc[i].index){
			desc = __UsbBaseDesc[i].desc;
			break;
		}
		i++;
	}

	//Print out the USB device's general information.
	if (NULL == desc)
	{
		_hx_printf("  %d\t\t%d\t\t%d\t\tDesc = [NULL],base class = [%d]\r\n",
			index,pDevice->dwNumber >> 16,pDevice->dwNumber & 0xFFFF,base_class);
	}
	else
	{
		_hx_printf("  %d\t\t%d\t\t%d\t\t%s\r\n", index,pDevice->dwNumber >> 16,
			pDevice->dwNumber & 0xFFFF,desc);
	}
}

//Show all USB device(s) information in system.
void ShowUsbDevices()
{
	int i = 0;
	__PHYSICAL_DEVICE* pDevice = USBManager.pUsbDeviceRoot;

	_hx_printf("  index\t\tUSB_Addr\tInt_Num\t\tDescription\r\n");
	_hx_printf("  --------\t--------\t--------\t--------\r\n");
	for (i = 0; pDevice; i++,pDevice = pDevice->lpNext)
	{
		ShowUsbGeneral(pDevice, i);
	}
}

//How many USB port status and port change status.
#define USB_PORT_STATUS_NUM 14

//Local helper routine to print out USB device port's status information.
static void PrintUsbPort(int index,struct usb_port_status* pStatus)
{
	uint16_t port_status, port_change;
	unsigned char sign_array[USB_PORT_STATUS_NUM + 2] = { 0 };
	int i = 0;

	port_status = le16_to_cpu(pStatus->wPortStatus);
	port_change = le16_to_cpu(pStatus->wPortChange);
	//Analyze port status,mark the corresponding byte as one if the bit is set.
	if (port_status & USB_PORT_STAT_CONNECTION)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_ENABLE)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_SUSPEND)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_OVERCURRENT)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_RESET)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_POWER)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_LOW_SPEED)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_HIGH_SPEED)
		sign_array[i] = 1;
	i++;
	if (port_status & USB_PORT_STAT_SUPER_SPEED)
		sign_array[i] = 1;
	i++;

	//Process status change flag bits.
	if (port_change & USB_PORT_STAT_C_CONNECTION)
		sign_array[i] = 1;
	i++;
	if (port_change & USB_PORT_STAT_C_ENABLE)
		sign_array[i] = 1;
	i++;
	if (port_change & USB_PORT_STAT_C_SUSPEND)
		sign_array[i] = 1;
	i++;
	if (port_change & USB_PORT_STAT_C_OVERCURRENT)
		sign_array[i] = 1;
	i++;
	if (port_change & USB_PORT_STAT_C_RESET)
		sign_array[i] = 1;
	i++;

	//Show out all this information.
	_hx_printf("     %02d", index);
	for (i = 0; i < USB_PORT_STATUS_NUM; i++)
	{
		_hx_printf(" %d   ", sign_array[i]);
	}
	_hx_printf("\r\n");
	return;
}

//Show a specific USB device's all port status.
void ShowUsbPort(int index)
{
	int i = 0, ret = -1;
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, port_status, 1);
	struct usb_device* pUsbDev = NULL;

	if (index >= USBManager.dev_index)
	{
		_hx_printf("  Error: Please specify a valid USB device index,range from 0 to [%d].\r\n", (USBManager.dev_index - 1));
		return;
	}

	//Check how many child port(s) attaching on this device.
	pUsbDev = USBManager.UsbDevArray[index];
	if (0 == pUsbDev->maxchild)
	{
		_hx_printf("  Error: The USB device you specified is not a USB hub.\r\n");
		return;
	}

	//Print out table header.
	_hx_printf("  Port# CONN ENAB SUSP OVER REST POWR LOSP HISP SUPS C_CO C_EN C_SU C_OV C_RE\r\n");

	//Now get port status and show out one by one...
	for (i = 0; i < pUsbDev->maxchild; i++)
	{
		ret = usb_get_port_status(pUsbDev, i + 1, port_status);
		if (!ret)
		{
			_hx_printf("  Error: Failed to get port [%d]'s status.\r\n", i);
			return;
		}
		PrintUsbPort(i,port_status);
	}
}

//Reset a port of the given USB HUB.
static int __hub_port_reset(struct usb_device *dev, int port,
	unsigned short *portstat)
{
	int err, tries;
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_port_status, portsts, 1);
	unsigned short portstatus, portchange;

#ifdef CONFIG_DM_USB
	debug("%s: resetting '%s' port %d...\r\n", __func__, dev->dev->name,
		port + 1);
#else
	debug("%s: resetting port %d...\r\n", __func__, port + 1);
#endif
	for (tries = 0; tries < 5; tries++) {
		err = usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		if (err < 0)
		{
			debug("%s: usb_set_port_feature [%d] failed with err = %d.\r\n", __func__, port, err);
			return err;
		}
		mdelay(200);

		if (usb_get_port_status(dev, port + 1, portsts) < 0) {
			debug("get_port_status failed status %lX\r\n",
				dev->status);
			return -1;
		}
		portstatus = le16_to_cpu(portsts->wPortStatus);
		portchange = le16_to_cpu(portsts->wPortChange);

		debug("portstatus %x, change %x\r\n", portstatus, portchange);

		//if (portstatus & USB_PORT_STAT_ENABLE)
		//	break;
		break;
	}

	if (tries == 5) {
		debug("Cannot enable port %i after %i retries, " \
			"disabling port.\r\n", port + 1, 5);
		debug("Maybe the USB cable is bad?\r\n");
		return -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	return 0;
}

//Reset a port of the specified USB HUB.
void ResetUsbPort(int index, int port)
{
	struct usb_device* pUsbDev = NULL;
	uint16_t port_status = 0;
	int err = -1;

	if (index >= USBManager.dev_index)
	{
		_hx_printf("  Please specify a valid USB device index,range from [0] to [%d].\r\n", (USBManager.dev_index - 1));
		return;
	}
	pUsbDev = USBManager.UsbDevArray[index];
	if (port >= pUsbDev->maxchild)
	{
		_hx_printf(" Please specify the correct port ID of USB HUB.\r\n");
		return;
	}
	//Reset the port.
	if ((err = __hub_port_reset(pUsbDev, port, &port_status)) < 0)
	{
		_hx_printf("  Can not reset the specified port with err = %d.\r\n", err);
		return;
	}
	_hx_printf("  Success to reset the specified port,status = %X.\r\n", port_status);
	return;
}

//Show all USB controller's status register.
void ShowUsbCtrlStatus()
{
	__COMMON_USB_CONTROLLER* pCtrl = NULL;

	int i = 0;
	for (i = 0; i < CONFIG_USB_MAX_CONTROLLER_NUM; i++)
	{
		if (USBManager.CtrlArray[i])
		{
			pCtrl = USBManager.CtrlArray[i];
			if (NULL == pCtrl->ctrlOps.get_ctrl_status)
			{
				continue;
			}
			_hx_printf("  USB Controller [%d] status information:\r\n", i);
			_hx_printf("    status:    %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl, 
				USB_CTRL_FLAG_EHCI_STATUS));
			_hx_printf("    command:   %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_COMMAND));
			_hx_printf("    intr:      %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_INTR));
			_hx_printf("    conf_flag: %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_CF));
			_hx_printf("    pl_base:   %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_PLBASE));
			_hx_printf("    al_base:   %X\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_ALBASE));
			_hx_printf("    xfer err:  %d\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_XFERERR));
			_hx_printf("    xfer req:  %d\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_XFERREQ));
			_hx_printf("    xfer int:  %d\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_XFERINT));
			_hx_printf("    axfer_n:   %d\r\n", pCtrl->ctrlOps.get_ctrl_status(pCtrl,
				USB_CTRL_FLAG_EHCI_ASSN));
		}
	}
}

//Test USB mouse function.
void DoUsbMouse()
{
	int x = 0, y = 0;
	MSG msg;

	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			x = (int)(msg.dwParam & 0xFFFF);  //Low 16 bits contains the x position.
			y = (int)((msg.dwParam >> 16) & 0xFFFF);  //High part is y.

			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_LBUTTONDOWN:
				_hx_printf("  Left button down,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONUP:
				_hx_printf("  Left button up,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDOWN:
				_hx_printf("  Right button down,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONUP:
				_hx_printf("  Right button up,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_MOUSEMOVE:
				_hx_printf("  Mouse is moving,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_LBUTTONDBCLK:
				_hx_printf("  Left button double clicked,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_RBUTTONDBCLK:
				_hx_printf("  Right button double clicked,x = %d,y = %d.\r\n",x,y);
				break;
			case KERNEL_MESSAGE_AKEYDOWN:
			case KERNEL_MESSAGE_VKEYDOWN:
				return;
			}
		}
	}
	return;
}
