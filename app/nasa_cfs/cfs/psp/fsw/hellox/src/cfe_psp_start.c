/*
 * cfe_psp_start.c - HelloX PSP application entry point.
 *
 * Provides OS_Application_Startup()/OS_Application_Run() and the global
 * config data that links the cFE core to the PSP/OSAL.
 */

#include <string.h>
#include <stdio.h>
#include <hellox.h>

#include "common_types.h"
#include "osapi.h"
#include "cfe_psp.h"
#include "cfe_psp_memory.h"
#include "cfe_psp_module.h"
#include "target_config.h"

#define CFE_PSP_MAIN_FUNCTION       (*GLOBAL_CONFIGDATA.CfeConfig->SystemMain)
#define CFE_PSP_NONVOL_STARTUP_FILE (GLOBAL_CONFIGDATA.CfeConfig->NonvolStartupFile)
#define CFE_PSP_CPU_ID              (GLOBAL_CONFIGDATA.Default_CpuId)
#define CFE_PSP_CPU_NAME            (GLOBAL_CONFIGDATA.Default_CpuName)
#define CFE_PSP_SPACECRAFT_ID       (GLOBAL_CONFIGDATA.Default_SpacecraftId)

#define CFE_PSP_CPU_NAME_LENGTH 32

/* Global variables */
uint32 CFE_PSP_SpacecraftId;
uint32 CFE_PSP_CpuId;
char   CFE_PSP_CpuName[CFE_PSP_CPU_NAME_LENGTH];

CFE_PSP_IdleTaskState_t CFE_PSP_IdleTaskState;

/* ---------------------------------------------------------
 * OS_Application_Startup - initialize OSAL, map filesystems,
 * initialize PSP modules/reserved memory, then start cFE.
 * --------------------------------------------------------- */
void OS_Application_Startup(void)
{
    uint32    reset_type;
    uint32    reset_subtype;
    osal_id_t fs_id;
    int32     Status;

    reset_type    = CFE_PSP_RST_TYPE_POWERON; /* HelloX always power-on reset */
    reset_subtype = 1;

    /* Assign the Spacecraft/CPU IDs from the target config. */
    CFE_PSP_SpacecraftId = CFE_PSP_SPACECRAFT_ID;
    CFE_PSP_CpuId        = CFE_PSP_CPU_ID;
    strncpy(CFE_PSP_CpuName, CFE_PSP_CPU_NAME, sizeof(CFE_PSP_CpuName) - 1);
    CFE_PSP_CpuName[sizeof(CFE_PSP_CpuName) - 1] = 0;

    /* Initialize the OSAL. */
    _hx_printf("[PSP] calling OS_API_Init()\r\n");
    Status = OS_API_Init();
    _hx_printf("[PSP] OS_API_Init() returned %d\r\n", (int)Status);
    if (Status != OS_SUCCESS)
    {
        OS_printf("CFE_PSP: OS_API_Init() failure (%d)\n", (int)Status);
        CFE_PSP_Panic(Status);
    }

    /* Set up the reserved memory map (volatile RAM). */
    _hx_printf("[PSP] calling CFE_PSP_SetupReservedMemoryMap()\r\n");
    CFE_PSP_SetupReservedMemoryMap();
    _hx_printf("[PSP] reserved memory map done\r\n");

    memset(&CFE_PSP_IdleTaskState, 0, sizeof(CFE_PSP_IdleTaskState));

    /* Map /cf (non-volatile) to a HelloX directory.  The volatile /ram
     * disk is created and mounted by CFE_ES_InitializeFileSystems(). */
    Status = OS_FileSysAddFixedMap(&fs_id, "c:\\app\\cfs\\cf", "/cf");
    if (Status != OS_SUCCESS)
    {
        OS_printf("CFE_PSP: OS_FileSysAddFixedMap(/cf) failure: %d\n", (int)Status);
    }

    /* Initialize the statically linked PSP modules. */
    _hx_printf("[PSP] calling CFE_PSP_ModuleInit()\r\n");
    CFE_PSP_ModuleInit();
    _hx_printf("[PSP] module init done\r\n");

    OS_printf("CFE_PSP: Starting the cFE with a POWER ON reset.\n");

    /* Initialize the reserved memory. */
    _hx_printf("[PSP] calling CFE_PSP_InitProcessorReservedMemory()\r\n");
    Status = CFE_PSP_InitProcessorReservedMemory(reset_type);
    if (Status != CFE_PSP_SUCCESS)
    {
        OS_printf("CFE_PSP: CFE_PSP_InitProcessorReservedMemory() failure\n");
        CFE_PSP_Panic(Status);
    }

    /* Call the cFE entry point. */
    _hx_printf("[PSP] calling CFE_ES_Main()\r\n");
    CFE_PSP_MAIN_FUNCTION(reset_type, reset_subtype, 1, CFE_PSP_NONVOL_STARTUP_FILE);
    _hx_printf("[PSP] CFE_ES_Main() returned\r\n");
}

/* ---------------------------------------------------------
 * OS_Application_Run - the original thread becomes the idle
 * thread.  HelloX has no signals, so simply sleep forever
 * (until a processor reset, which aborts).
 * --------------------------------------------------------- */
void OS_Application_Run(void)
{
    while (!CFE_PSP_IdleTaskState.ShutdownReq)
    {
        OS_TaskDelay(1000);
    }

    OS_printf("\nCFE_PSP: Shutdown initiated - Exiting cFE\n");
    OS_TaskDelay(100);
    OS_DeleteAllObjects();
}
