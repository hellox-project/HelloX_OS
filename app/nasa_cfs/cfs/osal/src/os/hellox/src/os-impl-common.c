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
 * \ingroup  posix
 * \author   joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-hellox.h"
#include "bsp-impl.h"

#include "os-impl-tasks.h"
#include "os-impl-queues.h"
#include "os-impl-binsem.h"
#include "os-impl-countsem.h"
#include "os-impl-mutex.h"

#include "os-shared-common.h"
#include "os-shared-idmap.h"

HELLOX_GlobalVars_t HELLOX_GlobalVars = {0};

/*---------------------------------------------------------------------------------------
   Name: OS_API_Impl_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: OS_SUCCESS or OS_ERROR
---------------------------------------------------------------------------------------*/
int32 OS_API_Impl_Init(osal_objtype_t idtype)
{
    int32 return_code;

    return_code = OS_Hellox_TableMutex_Init(idtype);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    switch (idtype)
    {
        case OS_OBJECT_TYPE_OS_TASK:
            return_code = OS_Hellox_TaskAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_QUEUE:
            return_code = OS_Hellox_QueueAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_BINSEM:
            return_code = OS_Hellox_BinSemAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_COUNTSEM:
            return_code = OS_Hellox_CountSemAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_MUTEX:
            return_code = OS_Hellox_MutexAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_MODULE:
            return_code = OS_Hellox_ModuleAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_TIMEBASE:
            return_code = OS_Hellox_TimeBaseAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_STREAM:
            return_code = OS_Hellox_StreamAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_DIR:
            return_code = OS_Hellox_DirAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_FILESYS:
            return_code = OS_Hellox_FileSysAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_CONDVAR:
            return_code = OS_Hellox_CondVarAPI_Impl_Init();
            break;
        default:
            break;
    }

    return return_code;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_IdleLoop_Impl(void)
{
    /*
     * HelloX has no POSIX signal mechanism, so the idle loop simply
     * sleeps forever.  The main thread is re-used as the idle thread
     * once OS_Application_Run() is entered; it never needs to do work.
     */
    for (;;)
    {
        Sleep(1000);
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_ApplicationShutdown_Impl(void)
{
    /*
     * HelloX has no signal mechanism.  The PSP's CFE_PSP_Restart() is
     * responsible for orderly shutdown; here we simply stop the process.
     */
    exit(0);
}

/*----------------------------------------------------------------
 *
 * Purpose:  Local helper function
 *
 * This function accept time interval, msecs, as an input and
 * computes the absolute time at which this time interval will expire.
 * The absolute time is programmed into a struct.
 *
 *-----------------------------------------------------------------*/
void OS_Hellox_CompAbsDelayTime(uint32 msecs, struct timespec *tm)
{
    clock_gettime(CLOCK_REALTIME, tm);

    /* add the delay to the current time */
    tm->tv_sec += (time_t)(msecs / 1000);
    /* convert residue ( msecs )  to nanoseconds */
    tm->tv_nsec += (msecs % 1000) * 1000000L;

    if (tm->tv_nsec >= 1000000000L)
    {
        tm->tv_nsec -= 1000000000L;
        tm->tv_sec++;
    }
}
