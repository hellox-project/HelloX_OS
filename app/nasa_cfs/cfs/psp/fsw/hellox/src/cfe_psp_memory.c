/*
 * cfe_psp_memory.c - HelloX PSP reserved memory management.
 *
 * HelloX has no persistent NVRAM; the "reserved" memory regions (CDS, reset
 * area, user reserved, volatile disk) are simply held in volatile RAM,
 * allocated from the heap at startup.
 */

#include <string.h>
#include <stdlib.h>

#include "common_types.h"
#include "osapi.h"
#include "cfe_psp.h"
#include "cfe_psp_memory.h"
#include "cfe_psp_config.h"
#include "target_config.h"
#include "cfe_platform_cfg.h"

CFE_PSP_ReservedMemoryMap_t CFE_PSP_ReservedMemoryMap;

/* A single combined block that holds all reserved regions. */
static uint8 *CFE_PSP_ReservedBlock;
static size_t CFE_PSP_ReservedBlockSize;

/* ---------------------------------------------------------
 * CFE_PSP_SetupReservedMemoryMap - allocate and map the
 * reserved memory regions into one volatile block.
 * --------------------------------------------------------- */
void CFE_PSP_SetupReservedMemoryMap(void)
{
    size_t cds_size, reset_size, user_size, vol_size, exc_size;
    size_t offset;

    cds_size   = GLOBAL_CONFIGDATA.CfeConfig->CdsSize;
    reset_size = GLOBAL_CONFIGDATA.CfeConfig->ResetAreaSize;
    user_size  = GLOBAL_CONFIGDATA.CfeConfig->UserReservedSize;
    vol_size   = (size_t)CFE_PLATFORM_ES_RAM_DISK_NUM_SECTORS * CFE_PLATFORM_ES_RAM_DISK_SECTOR_SIZE;
    exc_size   = sizeof(CFE_PSP_ExceptionStorage_t);

    /*
     * Zero the whole map first: HelloX does not guarantee that the BSS is
     * cleared at load time, and several fields (BootPtr, ExceptionStoragePtr,
     * SysMemoryTable) must start out as NULL/zero.
     */
    memset(&CFE_PSP_ReservedMemoryMap, 0, sizeof(CFE_PSP_ReservedMemoryMap));

    CFE_PSP_ReservedBlockSize = cds_size + reset_size + user_size + vol_size + exc_size;

    CFE_PSP_ReservedBlock = malloc(CFE_PSP_ReservedBlockSize);
    if (CFE_PSP_ReservedBlock == NULL)
    {
        CFE_PSP_ReservedBlockSize = 0;
    }

    offset = 0;
    CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr  = CFE_PSP_ReservedBlock + offset;
    CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize = cds_size;
    offset += cds_size;

    CFE_PSP_ReservedMemoryMap.ResetMemory.BlockPtr  = CFE_PSP_ReservedBlock + offset;
    CFE_PSP_ReservedMemoryMap.ResetMemory.BlockSize = reset_size;
    offset += reset_size;

    CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockPtr  = CFE_PSP_ReservedBlock + offset;
    CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockSize = user_size;
    offset += user_size;

    CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockPtr  = CFE_PSP_ReservedBlock + offset;
    CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockSize = vol_size;
    offset += vol_size;

    /* Boot record is the first part of the reset area. */
    CFE_PSP_ReservedMemoryMap.BootPtr = (CFE_PSP_ReservedMemoryBootRecord_t *)CFE_PSP_ReservedMemoryMap.ResetMemory.BlockPtr;

    /* Exception storage: keep a zeroed buffer so the ES exception scan
     * finds NumWritten == NumRead == 0 (no pending exceptions). */
    CFE_PSP_ReservedMemoryMap.ExceptionStoragePtr = (CFE_PSP_ExceptionStorage_t *)(CFE_PSP_ReservedBlock + offset);
    memset(CFE_PSP_ReservedMemoryMap.ExceptionStoragePtr, 0, exc_size);
    offset += exc_size;

    /* Register the RAM region in the system memory table. */
    CFE_PSP_MemRangeSet(0, CFE_PSP_MEM_RAM, 0, (size_t)-1, CFE_PSP_MEM_SIZE_DWORD, CFE_PSP_MEM_ATTR_READWRITE);
}

/* ---------------------------------------------------------
 * CFE_PSP_InitProcessorReservedMemory - zero the reserved block.
 * --------------------------------------------------------- */
int32 CFE_PSP_InitProcessorReservedMemory(uint32 RestartType)
{
    if (CFE_PSP_ReservedBlock != NULL && CFE_PSP_ReservedBlockSize > 0)
    {
        memset(CFE_PSP_ReservedBlock, 0, CFE_PSP_ReservedBlockSize);
    }

    /* Mark the boot record as valid with the next reset type. */
    if (CFE_PSP_ReservedMemoryMap.BootPtr != NULL)
    {
        CFE_PSP_ReservedMemoryMap.BootPtr->ValidityFlag  = CFE_PSP_BOOTRECORD_VALID;
        CFE_PSP_ReservedMemoryMap.BootPtr->NextResetType = CFE_PSP_RST_TYPE_POWERON;
    }

    return CFE_PSP_SUCCESS;
}

/* ---------------------------------------------------------
 * CFE_PSP_DeleteProcessorReservedMemory - free the reserved block.
 * --------------------------------------------------------- */
void CFE_PSP_DeleteProcessorReservedMemory(void)
{
    if (CFE_PSP_ReservedBlock != NULL)
    {
        free(CFE_PSP_ReservedBlock);
        CFE_PSP_ReservedBlock     = NULL;
        CFE_PSP_ReservedBlockSize = 0;
    }
}

/* ---------------------------------------------------------
 * CDS accessors.
 * --------------------------------------------------------- */
int32 CFE_PSP_GetCDSSize(uint32 *SizeOfCDS)
{
    if (SizeOfCDS == NULL)
    {
        return CFE_PSP_INVALID_POINTER;
    }

    *SizeOfCDS = (uint32)CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize;
    return CFE_PSP_SUCCESS;
}

int32 CFE_PSP_WriteToCDS(const void *PtrToDataToWrite, uint32 CDSOffset, uint32 NumBytes)
{
    if (CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr == NULL)
    {
        return CFE_PSP_ERROR;
    }
    if (CDSOffset + NumBytes > CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize)
    {
        return CFE_PSP_ERROR;
    }

    memcpy(CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr + CDSOffset, PtrToDataToWrite, NumBytes);
    return CFE_PSP_SUCCESS;
}

int32 CFE_PSP_ReadFromCDS(void *PtrToDataToRead, uint32 CDSOffset, uint32 NumBytes)
{
    if (CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr == NULL)
    {
        return CFE_PSP_ERROR;
    }
    if (CDSOffset + NumBytes > CFE_PSP_ReservedMemoryMap.CDSMemory.BlockSize)
    {
        return CFE_PSP_ERROR;
    }

    memcpy(PtrToDataToRead, CFE_PSP_ReservedMemoryMap.CDSMemory.BlockPtr + CDSOffset, NumBytes);
    return CFE_PSP_SUCCESS;
}

/* ---------------------------------------------------------
 * Reset / user reserved / volatile disk area accessors.
 * --------------------------------------------------------- */
int32 CFE_PSP_GetResetArea(cpuaddr *PtrToResetArea, uint32 *SizeOfResetArea)
{
    *PtrToResetArea  = (cpuaddr)CFE_PSP_ReservedMemoryMap.ResetMemory.BlockPtr;
    *SizeOfResetArea = (uint32)CFE_PSP_ReservedMemoryMap.ResetMemory.BlockSize;
    return CFE_PSP_SUCCESS;
}

int32 CFE_PSP_GetUserReservedArea(cpuaddr *PtrToUserArea, uint32 *SizeOfUserArea)
{
    *PtrToUserArea  = (cpuaddr)CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockPtr;
    *SizeOfUserArea = (uint32)CFE_PSP_ReservedMemoryMap.UserReservedMemory.BlockSize;
    return CFE_PSP_SUCCESS;
}

int32 CFE_PSP_GetVolatileDiskMem(cpuaddr *PtrToVolDisk, uint32 *SizeOfVolDisk)
{
    *PtrToVolDisk  = (cpuaddr)CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockPtr;
    *SizeOfVolDisk = (uint32)CFE_PSP_ReservedMemoryMap.VolatileDiskMemory.BlockSize;
    return CFE_PSP_SUCCESS;
}

/* ---------------------------------------------------------
 * Text segment info (kernel: not available; cFE: whole image).
 * --------------------------------------------------------- */
int32 CFE_PSP_GetKernelTextSegmentInfo(cpuaddr *PtrToKernelSegment, uint32 *SizeOfKernelSegment)
{
    /* HelloX does not expose the kernel text segment. */
    return CFE_PSP_ERROR_NOT_IMPLEMENTED;
}

int32 CFE_PSP_GetCFETextSegmentInfo(cpuaddr *PtrToCFESegment, uint32 *SizeOfCFESegment)
{
    /*
     * The cFE core checksum would require reading the whole loaded image,
     * but the tail of the image (the .reloc section) is not mapped after
     * HelloX loads the module, so it cannot be safely scanned.  Return
     * not-implemented and let the caller fall back to a checksum of 0xFFFF.
     */
    return CFE_PSP_ERROR_NOT_IMPLEMENTED;
}
