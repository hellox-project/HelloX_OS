/*
 * bsp_start.c - HelloX OSAL BSP main entry point.
 *
 * hxlib's main.c provides _hx_main(), which calls main().  This file
 * provides main(), which initializes the OSAL and enters the cFE
 * application startup/run sequence.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <hellox.h>

#include "hellox_bsp_internal.h"
#include "bsp-impl.h"

OS_BSP_HelloxGlobalData_t OS_BSP_HelloxGlobal;

/* ---------------------------------------------------------
 * OS_BSP_Initialize - initialize the BSP low-level access mutex.
 * --------------------------------------------------------- */
void OS_BSP_Initialize(void)
{
    pthread_mutex_init(&OS_BSP_HelloxGlobal.AccessMutex, NULL);
}

/* ---------------------------------------------------------
 * OS_BSP_Lock_Impl - lock the BSP console access mutex.
 * --------------------------------------------------------- */
void OS_BSP_Lock_Impl(void)
{
    pthread_mutex_lock(&OS_BSP_HelloxGlobal.AccessMutex);
}

/* ---------------------------------------------------------
 * OS_BSP_Unlock_Impl - unlock the BSP console access mutex.
 * --------------------------------------------------------- */
void OS_BSP_Unlock_Impl(void)
{
    pthread_mutex_unlock(&OS_BSP_HelloxGlobal.AccessMutex);
}

/* ---------------------------------------------------------
 * OS_BSP_GetReturnStatus - convert OSAL status to a process exit code.
 * --------------------------------------------------------- */
int OS_BSP_GetReturnStatus(void)
{
    int retcode;

    switch (OS_BSP_Global.AppStatus)
    {
        case OS_SUCCESS:
            retcode = EXIT_SUCCESS;
            break;
        case OS_ERROR:
            retcode = EXIT_FAILURE;
            break;
        default:
            retcode = OS_BSP_Global.AppStatus & 0x7F;
            break;
    }

    return retcode;
}

/* ---------------------------------------------------------
 * OS_BSP_Shutdown_Impl - abort the process.
 * --------------------------------------------------------- */
void OS_BSP_Shutdown_Impl(void)
{
    abort();
}

/* ---------------------------------------------------------
 * main - application entry point (invoked by hxlib _hx_main()).
 * --------------------------------------------------------- */
int main(int argc, char *argv[])
{
    _hx_printf("[BSP] main() entered, argc=%d\r\n", argc);

    memset(&OS_BSP_Global, 0, sizeof(OS_BSP_Global));

    OS_BSP_Global.ArgC = argc;
    OS_BSP_Global.ArgV = argv;

    _hx_printf("[BSP] calling OS_BSP_Initialize()\r\n");
    OS_BSP_Initialize();
    _hx_printf("[BSP] OS_BSP_Initialize() done\r\n");

    _hx_printf("[BSP] calling OS_Application_Startup()\r\n");
    OS_Application_Startup();
    _hx_printf("[BSP] OS_Application_Startup() returned\r\n");

    _hx_printf("[BSP] calling OS_Application_Run()\r\n");
    OS_Application_Run();

    /* Should typically never get here. */
    return OS_BSP_GetReturnStatus();
}
