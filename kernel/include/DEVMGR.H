//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,15 2005
//    Module Name               : DEVMGR.H
//    Module Funciton           : 
//                                This module countains device manager object's definition
//                                code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DEVMGR_H__
#define __DEVMGR_H__

/* The maximal bus number supported by Hello China currently. */
#define MAX_BUS_NUM        16
/* Maximal resource records one device can have. */
#define MAX_RESOURCE_NUM    7
/* The maximal length of device's name. */
#define MAX_DEV_NAME       32

/* The definition of device identifier. */
typedef struct{
	DWORD             dwBusType;
	union{
		/* Identifier for a PCI device. */
		struct{
			UCHAR     ucMask;
			WORD      wVendor;
			WORD      wDevice;
			DWORD     dwClass;
			UCHAR     ucHdrType;
			WORD      wReserved;
		}PCI_Identifier;

		/* Identifier for ISA bus device. */
		struct{
			DWORD     dwDevice;
		}ISA_Identifier;

		/* USB device or interface identifer. */
		struct{
			UCHAR ucMask;
			UCHAR bDeviceClass;
			UCHAR bDeviceSubClass;
			UCHAR bDeviceProtocol;
			UCHAR bInterfaceClass;
			UCHAR bInterfaceSubClass;
			UCHAR bInterfaceProtocol;
			WORD  wVendorID;
			WORD  wProductID;
		}USB_Identifier;
	}Bus_ID;
}__IDENTIFIER;

/* Member masks of USB identifier. */
#define USB_IDENTIFIER_MASK_DEVICECLASS          0x01
#define USB_IDENTIFIER_MASK_DEVICESUBCLASS       0x02
#define USB_IDENTIFIER_MASK_DEVICEPROTOCOL       0x04
#define USB_IDENTIFIER_MASK_INTERFACECLASS       0x08
#define USB_IDENTIFIER_MASK_INTERFACESUBCLASS    0x10
#define USB_IDENTIFIER_MASK_INTERFACEPROTOCOL    0x20
#define USB_IDENTIFIER_MASK_VENDORID             0x40
#define USB_IDENTIFIER_MASK_PRODUCTID            0x80
#define USB_IDENTIFIER_MASK_ALL                  0xFF

//Check if 2 device IDs are match,i.e,the pIdFullClass includes the first one.
BOOL DeviceIdMatch(__IDENTIFIER* pIdSubclass, __IDENTIFIER* pIdFullClass);

/* Resource descriptor object. */
typedef struct tag__RESOURCE{
	struct tag__RESOURCE* lpNext;
	struct tag__RESOURCE* lpPrev;
	/* Resource type. */
	DWORD dwResType;
	union{
		/* IO port for x86. */
		struct{
			WORD        wStartPort;
			WORD        wEndPort;
		}IOPort;
		/* Memory region. */
		struct{
			LPVOID      lpStartAddr;
			LPVOID      lpEndAddr;
		}MemoryRegion;
		/* Interrupt vector. */
		UCHAR ucVector;
		/* Base address of USB interface association. */
		LPVOID usbIntAssocBase;
	}Dev_Res;
}__RESOURCE;

//Resource types.
#define RESOURCE_TYPE_EMPTY      0x00000000    //No resource in resource descriptor object.
#define RESOURCE_TYPE_IO         0x00000001    //IO Port region resides in resource descriptor.
#define RESOURCE_TYPE_MEMORY     0x00000002    //Memory region resides in descriptor.
#define RESOURCE_TYPE_INTERRUPT  0x00000004    //Interrupt vector resides in descriptor.

//IO Range under x86 platform.
#ifdef __I386__
#define MIN_IO_PORT              0x0000  //The minimal value of IO port in IA32.
#define MAX_IO_PORT              0xFFFF  //The maximal value of IO port in IA32.
#endif

struct __PHYSICAL_DEVICE;
struct __SYSTEM_BUS;

/*
 * Physical device object,used to describe a device object on system bus.
 * Each device that HelloX can identify will be managed by one of this
 * object.
 */
typedef struct tag__PHYSICAL_DEVICE{
	/* Device's identifier. */
	__IDENTIFIER DevId;
	/* Device number in a system bus. */
	DWORD dwNumber;
	/* Device's name. */
	UCHAR strName[MAX_DEV_NAME];
	/* Resource descripotrs describes the resources that the device occupying. */
	__RESOURCE Resource[MAX_RESOURCE_NUM];
	/* Physical device list. */
	struct tag__PHYSICAL_DEVICE* lpNext;
	/* Bus where the device resides. */
	struct tag__SYSTEM_BUS* lpHomeBus;
	/* Child bus,if the device is a bridge. */
	struct tag__SYSTEM_BUS* lpChildBus;
	/* Pointing to device specific information. */
	LPVOID lpPrivateInfo;
	/* Operations can apply on physical device. */
	DWORD (*ReadDeviceConfig)(struct tag__PHYSICAL_DEVICE* dev, DWORD dwConfig,int size);
	BOOL (*WriteDeviceConfig)(struct tag__PHYSICAL_DEVICE* dev, DWORD dwConfig, DWORD dwVal,int size);
}__PHYSICAL_DEVICE;

/*
 * The definition of system bus object.
 * This object is used to describe a common bus.
 */
typedef struct tag__SYSTEM_BUS{
	/* Parent bus. */
	struct tag__SYSTEM_BUS* lpParentBus;
	/* Physical device(s) on this bus. */
	struct tag__PHYSICAL_DEVICE* lpDevListHdr;
	/* The bridge this bus attachs on. */
	struct tag__PHYSICAL_DEVICE* lpHomeBridge;
	/* Resource list header in this bus. */
	__RESOURCE Resource;

	/* Bus number and bus type. */
	DWORD dwBusNum;
	DWORD dwBusType;
	//BUS operations.
	DWORD   (*ReadConfig)(struct tag__SYSTEM_BUS* bus, DWORD dwConfigReg,int size);
	BOOL    (*WriteConfig)(struct tag__SYSTEM_BUS* bus, DWORD dwConfigReg, DWORD dwVal,int size);
}__SYSTEM_BUS;

//BUS types.
#define BUS_TYPE_NULL            0x00000000
#define BUS_TYPE_PCI             0x00000001
#define BUS_TYPE_ISA             0x00000002
#define BUS_TYPE_EISA            0x00000004
#define BUS_TYPE_USB             0x00000008
#define BUS_TYPE_SPI             0x00000010
#define BUS_TYPE_SDIO            0x00000020
#define BUS_TYPE_I2C             0x00000040

/*
 * Address translation between PCI bus and memory.
 */
#define PCI_TO_VIRTUAL_MEMORY(dev,ptr) ((void*)ptr)
#define PCI_TO_PHYSICAL_MEMORY(dev,ptr) ((void*)ptr)
#define VIRTUAL_MEMORY_TO_PCI(dev,ptr) ((void*)ptr)
#define PHYSICAL_MEMORY_TO_PCI(dev,ptr) ((void*)ptr)

/*
 * The definition of device manager object.
 * This object is a global object,all physical device releated operations,are offered
 * by this object.
 */
typedef struct tag__DEVICE_MANAGER{
	/* Lock to protect this object. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Bus array. */
	__SYSTEM_BUS SystemBus[MAX_BUS_NUM];
	/* Used to link free port region together. */
	__RESOURCE FreePortResource;
	/* Links used port region together. */
	__RESOURCE UsedPortResource;

	/* Operations that device manager can offer. */
	/* Initializer of device manager. */
	BOOL (*Initialize)(struct tag__DEVICE_MANAGER*);
	/* Get a device from system given it's bus type and device ID. */
	__PHYSICAL_DEVICE* (*GetDevice)(struct tag__DEVICE_MANAGER*,
		DWORD dwBusType,
		__IDENTIFIER* lpIdentifier,
		__PHYSICAL_DEVICE* lpStart);
	/* Append a new physical device into device manager. */
	BOOL (*AppendDevice)(struct tag__DEVICE_MANAGER*,
		__PHYSICAL_DEVICE*);
	/* Delete a specified physical device. */
	VOID (*DeleteDevice)(struct tag__DEVICE_MANAGER*,
		__PHYSICAL_DEVICE*);
	/* Check if a port range is free. */
	BOOL (*CheckPortRegion)(struct tag__DEVICE_MANAGER*,
		__RESOURCE*);
	/* Reserve a range of port. */
	BOOL (*ReservePortRegion)(struct tag__DEVICE_MANAGER*,
		__RESOURCE*);
	/* Release port range. */
	VOID (*ReleasePortRegion)(struct tag__DEVICE_MANAGER*,
		__RESOURCE*);
}__DEVICE_MANAGER;

/****************************************************************************************
*****************************************************************************************
*****************************************************************************************
*****************************************************************************************
****************************************************************************************/
//
//Declaration of DeviceManager object,which is a global object and can be refered by any
//code in system scope.
//
extern __DEVICE_MANAGER DeviceManager;

#endif //__DEVMGR_H__
