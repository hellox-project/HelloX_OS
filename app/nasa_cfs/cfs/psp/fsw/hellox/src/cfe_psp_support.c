/*
 * cfe_psp_support.c - HelloX PSP support functions.
 */

#include <stdlib.h>
#include <hellox.h>

#include "common_types.h"
#include "osapi.h"
#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "target_config.h"

#define CFE_PSP_CPU_NAME_LENGTH 32

extern uint32 CFE_PSP_SpacecraftId;
extern uint32 CFE_PSP_CpuId;
extern char   CFE_PSP_CpuName[CFE_PSP_CPU_NAME_LENGTH];

/* ---------------------------------------------------------
 * CFE_PSP_Restart - orderly restart (HelloX: just abort).
 * --------------------------------------------------------- */
void CFE_PSP_Restart(uint32 reset_type)
{
    OS_printf("CFE_PSP: Restarting (reset_type=%lu)\n", (unsigned long)reset_type);

    /* Set the shutdown request so the idle loop exits. */
    CFE_PSP_IdleTaskState.ShutdownReq = true;

    /* HelloX has no real reset mechanism exposed; abort. */
    abort();
}

/* ---------------------------------------------------------
 * CFE_PSP_Panic - print a message and abort.
 * --------------------------------------------------------- */
void CFE_PSP_Panic(int32 error_code)
{
    OS_printf("CFE_PSP: PANIC, error code = %d\n", (int)error_code);
    abort();
}

/* ---------------------------------------------------------
 * CFE_PSP_FlushCaches - no-op on HelloX (no cache flush needed).
 * --------------------------------------------------------- */
void CFE_PSP_FlushCaches(uint32 type, void *address, uint32 size)
{
}

/* ---------------------------------------------------------
 * CFE_PSP_GetProcessorId / GetSpacecraftId / GetProcessorName
 * --------------------------------------------------------- */
uint32 CFE_PSP_GetProcessorId(void)
{
    return CFE_PSP_CpuId;
}

uint32 CFE_PSP_GetSpacecraftId(void)
{
    return CFE_PSP_SpacecraftId;
}

const char *CFE_PSP_GetProcessorName(void)
{
    return CFE_PSP_CpuName;
}
