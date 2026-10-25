/************************************************************************
 * HelloX Platform Support Package configuration
 ************************************************************************/

#ifndef CFE_PSP_CONFIG_H
#define CFE_PSP_CONFIG_H

#include "common_types.h"

#include <time.h>
#include <hellox.h>

/*
 * Number of memory ranges in the memory range definition table.
 */
#define CFE_PSP_MEM_TABLE_SIZE 10

/*
 * Exception storage (HelloX has no signal/backtrace support, so these
 * are kept minimal; exception capture is degraded to abort()).
 */
#define CFE_PSP_MAX_EXCEPTION_ENTRIES        4
#define CFE_PSP_MAX_EXCEPTION_BACKTRACE_SIZE 16

#define CFE_PSP_BOOTRECORD_VALID   ((uint32)0x2aebe984)
#define CFE_PSP_BOOTRECORD_INVALID (~CFE_PSP_BOOTRECORD_VALID)

#define CFE_PSP_RESTART_DELAY 10000

/* The tick period of the simulated (soft) timebase, in microseconds.
 * 10ms / 100Hz, used to drive the SCH minor frame and the 1Hz clock. */
#define CFE_PSP_SOFT_TIMEBASE_PERIOD 10000

/*
 * Reserved memory boot record.
 */
typedef struct
{
    uint32 ValidityFlag;
    uint32 NextResetType;
} CFE_PSP_ReservedMemoryBootRecord_t;

/*
 * State of the PSP "idle task" (the initial thread).
 */
typedef struct
{
    HANDLE         ThreadID;
    volatile bool  ShutdownReq;
} CFE_PSP_IdleTaskState_t;

/*
 * Underlying OS thread ID type (HelloX uses a handle).
 */
typedef HANDLE CFE_PSP_Exception_SysTaskId_t;

/*
 * Exception context data (minimal: only the event time, no siginfo/backtrace).
 */
typedef struct
{
    struct timespec event_time;
} CFE_PSP_Exception_ContextDataEntry_t;

/*
 * Watchdog values (HelloX watchdog is NOT_IMPLEMENTED).
 */
#define CFE_PSP_WATCHDOG_MIN (0)
#define CFE_PSP_WATCHDOG_MAX (0xFFFFFFFF)

/*
 * Number of EEPROM banks.
 */
#define CFE_PSP_NUM_EEPROM_BANKS 1

extern CFE_PSP_IdleTaskState_t CFE_PSP_IdleTaskState;

#endif
