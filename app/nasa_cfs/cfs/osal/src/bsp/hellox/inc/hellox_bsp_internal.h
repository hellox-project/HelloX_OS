/*
 * hellox_bsp_internal.h - internal definitions for the HelloX OSAL BSP.
 */

#ifndef HELLOX_BSP_INTERNAL_H
#define HELLOX_BSP_INTERNAL_H

#include <pthread.h>

typedef struct
{
    pthread_mutex_t AccessMutex;
} OS_BSP_HelloxGlobalData_t;

extern OS_BSP_HelloxGlobalData_t OS_BSP_HelloxGlobal;

#endif /* HELLOX_BSP_INTERNAL_H */
