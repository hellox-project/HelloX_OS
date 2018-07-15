//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 30,2018
//    Module Name               : acpi.h
//    Module Funciton           : 
//                                ACPI(Advanced Configuration and Power Interface)
//                                related structures and constants,functions.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ACPI_H__
#define __ACPI_H__

#include "config.h"
#include "TYPES.H" /* For basic types. */
#include "stdint.h"

/*
* Entry point of ACPI initialization process.
* It will be invoked in process of hardware initialization.
*/
BOOL ACPI_Init();

/* RSDP(Root System Description Pointer) structure,for 1.0. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct __RSDP_Descriptor {
	char Signature[8]; /* Must be "RSD PTR ". */
	__u8 Checksum;
	char OEMID[6];
	__u8 Revision;     /* Version,1.0 or 2.0. */
	__u32 RsdtAddress; /* Physical address of RSDT. */
};
#pragma pack(pop)
#else
struct __RSDP_Descriptor {
	char Signature[8]; /* Must be "RSD PTR ". */
	__u8 Checksum;
	char OEMID[6];
	__u8 Revision;     /* Version,1.0 or 2.0. */
	__u32 RsdtAddress; /* Physical address of RSDT. */
}__attribute__((packed));
#endif

/* RSDP(Root System Description Pointer) structure,for 2.0. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct __RSDP_Descriptor20 {
	char Signature[8]; /* Must be "RSD PTR ". */
	__u8 Checksum;
	char OEMID[6];
	__u8 Revision;     /* Version,1.0 or 2.0. */
	__u32 RsdtAddress; /* Physical address of RSDT. */

	/* Specific for 2.0. */
	__u32 Length;
	__u64 XsdtAddress;
	__u8 ExtendedChecksum;
	__u8 reserved[3];
};
#pragma pack(pop)
#else
struct __RSDP_Descriptor20 {
	char Signature[8]; /* Must be "RSD PTR ". */
	__u8 Checksum;
	char OEMID[6];
	__u8 Revision;     /* Version,1.0 or 2.0. */
	__u32 RsdtAddress; /* Physical address of RSDT. */

	/* Specific for 2.0. */
	__u32 Length;
	__u64 XsdtAddress;
	__u8 ExtendedChecksum;
	__u8 reserved[3];
}__attribute__((packed));
#endif

/* 
 * Start and end address of the RDSP descriptor range,OS will
 * search from this address to locate the descriptor by
 * trying to match the signature string,which is "RSD PTR ".
 */
#define ACPI_RSDP_RANGE_BEGIN 0x000E0000
#define ACPI_RSDP_RANGE_END   0x000FFFFF

 /* SDT(System Description Table)'s header structure,common for all SDT. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct ACPISDTHeader {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
}; 
#pragma pack(pop)
#else
struct ACPISDTHeader {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
}__attribute__((packed));
#endif

/*
* Structure for RSDT,which will be used when Revision's
* value,in RSD descriptor, is less than 2.
*/
#if defined(__MS_VC__)
#pragma pack(push,1)
struct RSDT {
	struct ACPISDTHeader h;
	uint32_t PointerToOtherSDT[0]; /* At least one. */
};
#pragma pack(pop)
#else
struct RSDT {
	struct ACPISDTHeader h;
	uint32_t PointerToOtherSDT[0]; /* At least one. */
}__attribute__((packed));
#endif

/* 
 * Structure for XSDT,which will be used when Revision's 
 * value,in RSD descriptor, is larger or equal to 2. 
 */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct XSDT {
	struct ACPISDTHeader h;
	uint64_t PointerToOtherSDT[0]; /* At least one. */
}; 
#pragma pack(pop)
#else
struct XSDT {
	struct ACPISDTHeader h;
	uint64_t PointerToOtherSDT[0]; /* At least one. */
}__attribute__((packed));
#endif

/*
* Structure for MADT(Multiple APIC Descriptor Table),which
* is used to obtain APIC's information.
*/
#if defined(__MS_VC__)
#pragma pack(push,1)
struct MADT {
	struct ACPISDTHeader h;
	uint32_t local_APIC_addr;
	uint32_t flags;
	/* First pair of entry type and it's length. */
	uint8_t entry_type;
	uint8_t entry_length;
};
#pragma pack(pop)
#else
struct MADT {
	struct ACPISDTHeader h;
	uint32_t local_APIC_addr;
	uint32_t flags;
	/* First pair of entry type and it's length. */
	uint8_t entry_type;
	uint8_t entry_length;
}__attribute__((packed));
#endif

/* MADT entry type. */
#define MADT_ENTRY_TYPE_LAPIC   0 /* Local APIC. */
#define MADT_ENTRY_TYPE_IOAPIC  1 /* IO APIC. */
#define MADT_ENTRY_TYPE_ISO     2 /* Interrupt Source Override. */
#define MADT_ENTRY_TYPE_NMI     4 /* No-maskable interrupts. */
#define MADT_ENTRY_TYPE_LAPICAO 5 /* Local APIC Address Override. */

/* Structures to describe Local APIC in MADT. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct __LOCAL_APIC {
	uint8_t entry_type;
	uint8_t entry_length;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
};
#pragma pack(pop)
#else
struct __LOCAL_APIC {
	uint8_t entry_type;
	uint8_t entry_length;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
}__attribute__((packed));
#endif

/* Structures to describe IO APIC in MADT. */
#if defined(__MS_VC__)
#pragma pack(push,1)
struct __IO_APIC {
	uint8_t entry_type;
	uint8_t entry_length;
	uint8_t apic_id;
	uint8_t reserved;
	uint32_t address;
	uint32_t global_intbase; /* Global System Interrupt Base. */
};
#pragma pack(pop)
#else
struct __IO_APIC {
	uint8_t entry_type;
	uint8_t entry_length;
	uint8_t apic_id;
	uint8_t reserved;
	uint32_t address;
	uint32_t global_intbase; /* Global System Interrupt Base. */
}__attribute__((packed));
#endif

#endif //__ACPI_H__
