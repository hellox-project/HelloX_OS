//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 23, 2020
//    Module Name               : ahci.h
//    Module Funciton           : 
//                                Driver header for AHCI(Advanced Host Controller
//                                Interface) driver.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __AHCI_H__
#define __AHCI_H__

/* General ATA header. */
#include <ata.h>
/* SATA spec definitions. */
#include "ahcidef.h"

/* 
 * PCI class code(0x01), sub class code(0x06), and
 * programming interface(0x01) of AHCI devices. 
 * Shift 8 bits to left to skip the revision field.
 */
#define PCI_CLASS_AHCI (0x010601 << 8)

/*
 * AHCI background thread's name.
 * One dedicated kernel thread running in background to
 * serve AHCI controllers.
 */
#define AHCI_BG_THREAD_NAME "ahci_bg"

/* partition name base. */
#define ATA_PARTITION_NAME_BASE "\\\\.\\ATAPART0"

/* Messages that background thread could handle. */
#define AHCI_MSG_INIT_PORT (MSG_USER_START + 1)

/* Constants of SATA drvers. */
/* PRDT entries for one command. */
#define SATA_PRDT_PER_COMMAND 8
/* Maximal command header number. */
#define AHCI_MAX_CMDLIST_ENTRY 32
/* Maximal ports on one ahci controller. */
#define AHCI_MAX_PORT_NUM 32

/* Port interrupt status. */
#define AHCI_PxINT_DHRS    (1 << 0)
#define AHCI_PxINT_PSS     (1 << 1)
#define AHCI_PxINT_DSS     (1 << 2)
#define AHCI_PxINT_SDBS    (1 << 3)
#define AHCI_PxINT_UFS     (1 << 4)
#define AHCI_PxINT_DPS     (1 << 5)
#define AHCI_PxINT_PCS     (1 << 6)
#define AHCI_PxINT_PRCS    (1 << 22)

/* Port interrupt enable bits. */
#define AHCI_PxINT_DHRE    (1 << 0)
#define AHCI_PxINT_PSE     (1 << 1)
#define AHCI_PxINT_DSE     (1 << 2)
#define AHCI_PxINT_SDBE    (1 << 3)
#define AHCI_PxINT_UFE     (1 << 4)
#define AHCI_PxINT_DPE     (1 << 5)
#define AHCI_PxINT_PCE     (1 << 6)
#define AHCI_PxINT_PRCE    (1 << 22)

/*
 * SATA port object,to manage one port on SATA
 * controller.
 */
typedef struct tag__AHCI_PORT_OBJECT {
	/* Port index value. */
	int port_index;

	/* Reference counter. */
	__atomic_t ref_count;

	/* 
	 * Link pointer pointing to next port 
	 * object belong to same controller. 
	 */
	struct tag__AHCI_PORT_OBJECT* pNext;

	/* I/O requests pending on this port. */
	__DRCB* request_list;
	__DRCB* request_list_tail;
	volatile int request_num;

	/* Statistics counters of this port. */
	unsigned long int_raised;
	unsigned long int_dhrs_raised;
	unsigned long int_pss_raised;
	unsigned long int_dss_raised;
	unsigned long int_sdbs_raised;
	unsigned long int_ufs_raised;
	unsigned long int_dps_raised;
	unsigned long int_pcs_raised;
	unsigned long int_prcs_raised;
	unsigned long sectors_in;
	unsigned long sectors_out;
	unsigned long xfer_errors;

	/* Configure registers. */
	__HBA_PORT* pConfigRegister;

	/* The device type attaching on this port. */
	unsigned long device_type;

	/* Port status. */
	uint32_t port_status;

	/* AHCI controller this port attaching on. */
	struct tag__AHCI_CONTROLLER* pCtrl;

#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/* Port operations,read count sectors from port into buf. */
	BOOL (*port_device_read)(struct tag__AHCI_PORT_OBJECT* pPort, uint32_t startl,
		uint32_t starth, const uint32_t count, uint8_t *buf, __DRCB* drcb_req);
	/* write count sectors into port in buf. */
	BOOL (*port_device_write)(struct tag__AHCI_PORT_OBJECT* pPort, uint32_t startl,
		uint32_t starth, const uint32_t count, uint8_t *buf, __DRCB* drcb_req);
	/* Identify the disk attaching on port. */
	BOOL (*port_device_identify)(struct tag__AHCI_PORT_OBJECT *port_obj, uint8_t *buf);
}__AHCI_PORT_OBJECT;

/* I/O directions. */
typedef enum {
	/* device to host. */
	__IN = 0,
	/* host to device. */
	__OUT = 1,
}__AHCI_IO_DIRECT;

/* Record one port request. */
typedef struct tag__AHCI_PORT_REQUEST {
	__AHCI_PORT_OBJECT* pPort;
	__AHCI_IO_DIRECT inout;
	uint32_t startl;
	uint32_t starth;
	uint32_t count;
	uint8_t* buffer;
	int cmd_slot;
	BOOL bReqResult;

	/* The FIS received from device. */
	FIS_REG_D2H fis_d2h;
}__AHCI_PORT_REQUEST;

/* Set port operation routines give port object. */
void SetPortOperations(__AHCI_PORT_OBJECT* pPort);
/* Set driver object's operation routine. */
void SetDrvOperations(__DRIVER_OBJECT* pDriver);
/* Init storage on port. */
BOOL InitPortStorage(__AHCI_PORT_OBJECT* pPort);

/*
 * AHCI controller object, describe one AHCI controller
 * in system.
 */
typedef struct tag__AHCI_CONTROLLER{
	/* Object list pointer. */
	struct tag__AHCI_CONTROLLER* pNext;

	/* spin lock. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/* Controller's name. */
	char ctrl_name[16];
	/* Device object's handle. */
	HANDLE device_handle;
	/* Handle of interrupt. */
	HANDLE interrupt_handle;

	/* System resources. */
	unsigned char interrupt;
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned long mem_size;

	/* How many port attached. */
	int port_num;
	/* Port object list root. */
	__AHCI_PORT_OBJECT* pPortList;

	/* Config registers of this controller. */
	__HBA_MEM* pConfigRegister;
}__AHCI_CONTROLLER;

/* Entry point of AHCI driver. */
BOOL AHCIDriverEntry(__DRIVER_OBJECT* pDriverObject);

/* SATA signature. */
#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101		// Port multiplier

/* AHCI device types. */
#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

/* Port status. */
#define HBA_PORT_NOT_START 0
#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

/* Flags value for port command. */
#define HBA_PxCMD_CR    (1 << 15) /* CR - Command list Running */
#define HBA_PxCMD_FR    (1 << 14) /* FR - FIS receive Running */
#define HBA_PxCMD_FRE   (1 <<  4) /* FRE - FIS Receive Enable */
#define HBA_PxCMD_SUD   (1 <<  1) /* SUD - Spin-Up Device */
#define HBA_PxCMD_ST    (1 <<  0) /* ST - Start (command processing) */
#define HBA_PxTFD_BSY   (1 <<  7) /* BSY of TFD. */
#define HBA_PxTFD_DRQ   (1 <<  3) /* DRQ of TFD. */
#define HBA_PxTFD_ERR   (1 <<  0) /* ERR of TFD. */
#define ATA_DEV_BUSY    0x80
#define ATA_DEV_DRQ     0x08

#define HBA_PxIS_TFES   (1 << 30) /* TFES - Task File Error Status */

#endif //__AHCI_H__
