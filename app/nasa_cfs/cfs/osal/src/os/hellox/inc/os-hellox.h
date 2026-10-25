/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *
 * \ingroup  posix
 *
 * Purpose: This file contains definitions that are shared across the POSIX
 *          OSAL implementation.  This file is private to the POSIX port and it
 *          may contain POSIX-specific definitions.
 */

#ifndef OS_HELLOX_H
#define OS_HELLOX_H

/****************************************************************************************
                                    COMMON INCLUDE FILES
 ***************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <hellox.h>

/*
 * Use the global definitions from the shared layer
 */
#include "os-shared-globaldefs.h"

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

/****************************************************************************************
                                    TYPEDEFS
 ***************************************************************************************/

typedef void *(*PthreadFuncPtr_t)(void *arg);

typedef struct
{
    int PriorityMax;
    int PriorityMin;
} HELLOX_PriorityLimits_t;

typedef struct
{
    bool                   EnableTaskPriorities;
    osal_blockcount_t      TruncateQueueDepth;
    uint32                 ClockAccuracyNsec;
    pthread_key_t          ThreadKey;
    sigset_t               MaximumSigMask;
    sigset_t               NormalSigMask;
    size_t                 PageSize;
    HELLOX_PriorityLimits_t PriLimits;
    int                    SelectedRtScheduler;
} HELLOX_GlobalVars_t;

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

extern HELLOX_GlobalVars_t HELLOX_GlobalVars;

/****************************************************************************************
                       POSIX IMPLEMENTATION FUNCTION PROTOTYPES
 ***************************************************************************************/

int32 OS_Hellox_TaskAPI_Impl_Init(void);
int32 OS_Hellox_QueueAPI_Impl_Init(void);
int32 OS_Hellox_BinSemAPI_Impl_Init(void);
int32 OS_Hellox_CountSemAPI_Impl_Init(void);
int32 OS_Hellox_MutexAPI_Impl_Init(void);
int32 OS_Hellox_CondVarAPI_Impl_Init(void);
int32 OS_Hellox_ModuleAPI_Impl_Init(void);
int32 OS_Hellox_TimeBaseAPI_Impl_Init(void);
int32 OS_Hellox_StreamAPI_Impl_Init(void);
int32 OS_Hellox_DirAPI_Impl_Init(void);
int32 OS_Hellox_FileSysAPI_Impl_Init(void);

int32 OS_Hellox_TableMutex_Init(osal_objtype_t idtype);

void OS_Hellox_CompAbsDelayTime(uint32 msecs, struct timespec *tm);

#endif /* OS_HELLOX_H */
