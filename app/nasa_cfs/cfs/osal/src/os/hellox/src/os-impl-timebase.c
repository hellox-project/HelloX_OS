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
 * This file contains the OSAL Timebase API for POSIX systems.
 *
 * This implementation depends on the POSIX Timer API which may not be available
 * in older versions of the Linux kernel. It was developed and tested on
 * RHEL 5 ./ CentOS 5 with Linux kernel 2.6.18
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-hellox.h"
#include "os-impl-timebase.h"
#include "os-impl-tasks.h"

#include "os-shared-timebase.h"
#include "os-shared-idmap.h"
#include "os-shared-common.h"

/****************************************************************************************
                                EXTERNAL FUNCTION PROTOTYPES
 ***************************************************************************************/

/****************************************************************************************
                                INTERNAL FUNCTION PROTOTYPES
 ***************************************************************************************/

/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

/*
 * Prefer to use the MONOTONIC clock if available, as it will not get disrupted by setting
 * the time like the REALTIME clock will.
 */
#ifndef OS_PREFERRED_CLOCK
#ifdef _POSIX_MONOTONIC_CLOCK
#define OS_PREFERRED_CLOCK CLOCK_MONOTONIC
#else
#define OS_PREFERRED_CLOCK CLOCK_REALTIME
#endif
#endif

/****************************************************************************************
                                     GLOBALS
 ***************************************************************************************/

OS_impl_timebase_internal_record_t OS_impl_timebase_table[OS_MAX_TIMEBASES];

/****************************************************************************************
                                INTERNAL FUNCTIONS
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseLock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    pthread_mutex_lock(&impl->handler_mutex);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseUnlock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    pthread_mutex_unlock(&impl->handler_mutex);
}

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
static uint32 OS_TimeBase_SyncImpl(osal_id_t obj_id)
{
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *impl;
    OS_timebase_internal_record_t *     timebase;
    uint32                              interval_time;

    interval_time = 0;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, obj_id, &token) == OS_SUCCESS)
    {
        impl     = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);
        timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, token);

        /*
         * Determine how much time this tick represents.
         * First tick after a Set uses the start time; steady state uses the interval.
         */
        if (impl->reset_flag == 0)
        {
            interval_time = timebase->nominal_interval_time;
        }
        else
        {
            interval_time = timebase->nominal_start_time;
            impl->reset_flag = 0;
        }

        /*
         * Wait out the interval using HelloX Sleep() directly.  The kernel
         * rounds the sleep up to its 5 ms time slice, which is sufficient for
         * the cFS 10 ms timebase.  Using Sleep() (a proper kernel sleep)
         * instead of clock_gettime()+nanosleep() avoids the TSC monotonic
         * clock calibration path and keeps the handler thread blocked.
         */
        if (interval_time > 0)
        {
            uint32 sleep_ms = (interval_time + 999) / 1000;
            if (sleep_ms == 0)
            {
                sleep_ms = 1;
            }
            Sleep(sleep_ms);
        }
    }

    return interval_time;
}

/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/******************************************************************************
 *
 *  Purpose:  Initialize the timer implementation layer
 *
 *  Arguments:
 *
 *  Return:
 */
int32 OS_Hellox_TimeBaseAPI_Impl_Init(void)
{
    int                 status;
    osal_index_t        idx;
    pthread_mutexattr_t mutex_attr;
    struct timespec     clock_resolution;
    int32               return_code;

    return_code = OS_SUCCESS;

    do
    {
        /*
        ** Mark all timers as available
        */
        memset(OS_impl_timebase_table, 0, sizeof(OS_impl_timebase_table));

        /*
        ** get the resolution of the selected clock
        */
        status = clock_getres(OS_PREFERRED_CLOCK, &clock_resolution);
        if (status != 0)
        {
            OS_DEBUG("failed in clock_getres: %s\n", strerror(errno));
            return_code = OS_ERROR;
            break;
        }

        /*
        ** Convert to microseconds
        ** Note that the resolution MUST be in the sub-second range, if not then
        ** it looks like the POSIX timer API in the C library is broken.
        ** Note for any flavor of RTOS we would expect <= 1ms.  Even a "desktop"
        ** linux or development system should be <= 100ms absolute worst-case.
        */
        if (clock_resolution.tv_sec > 0)
        {
            return_code = OS_TIMER_ERR_INTERNAL;
            break;
        }

        /* Round to the nearest microsecond */
        HELLOX_GlobalVars.ClockAccuracyNsec = (uint32)(clock_resolution.tv_nsec);

        /*
        ** initialize the attribute with default values
        */
        status = pthread_mutexattr_init(&mutex_attr);
        if (status != 0)
        {
            OS_DEBUG("Error: pthread_mutexattr_init failed: %s\n", strerror(status));
            return_code = OS_ERROR;
            break;
        }

        /*
        ** Allow the mutex to use priority inheritance
        */
        status = pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT);
        if (status != 0)
        {
            OS_DEBUG("Error: pthread_mutexattr_setprotocol failed: %s\n", strerror(status));
            return_code = OS_ERROR;
            break;
        }

        for (idx = 0; idx < OS_MAX_TIMEBASES; ++idx)
        {
            /*
            ** create the timebase sync mutex
            ** This gives a mechanism to synchronize updates to the timer chain with the
            ** expiration of the timer and processing the chain.
            */
            status = pthread_mutex_init(&OS_impl_timebase_table[idx].handler_mutex, &mutex_attr);
            if (status != 0)
            {
                OS_DEBUG("Error: Mutex could not be created: %s\n", strerror(status));
                return_code = OS_ERROR;
                break;
            }
        }

        /*
         * Pre-calculate the clock tick to microsecond conversion factor.
         */
        OS_SharedGlobalVars.TicksPerSecond = sysconf(_SC_CLK_TCK);
        if (OS_SharedGlobalVars.TicksPerSecond <= 0)
        {
            OS_DEBUG("Error: Unable to determine OS ticks per second: %s\n", strerror(errno));
            return_code = OS_ERROR;
            break;
        }

        /*
         * Calculate microseconds per tick
         *  - If the ratio is not an integer, this will round to the nearest integer value
         *  - This is used internally for reporting accuracy,
         *  - TicksPerSecond values over 2M will return zero
         */
        OS_SharedGlobalVars.MicroSecPerTick =
            (1000000 + (OS_SharedGlobalVars.TicksPerSecond / 2)) / OS_SharedGlobalVars.TicksPerSecond;
    } while (0);

    return return_code;
}

/****************************************************************************************
                                   Time Base API
 ***************************************************************************************/

static void *OS_TimeBasePthreadEntry(void *arg)
{
    OS_VoidPtrValueWrapper_t local_arg;

    /* cppcheck-suppress unreadVariable // intentional use of other union member */
    local_arg.opaque_arg = arg;
    OS_TimeBase_CallbackThread(local_arg.id);

    return NULL;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseCreate_Impl(const OS_object_token_t *token)
{
    int32                               return_code;
    OS_impl_timebase_internal_record_t *local;
    OS_timebase_internal_record_t *     timebase;
    OS_VoidPtrValueWrapper_t            arg;

    local    = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    timebase = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

    /*
     * Spawn a dedicated time base handler thread.
     */
    memset(&arg, 0, sizeof(arg));

    arg.id      = OS_ObjectIdFromToken(token);
    return_code = OS_Hellox_InternalTaskCreate_Impl(&local->handler_thread, OSAL_PRIORITY_C(0), 0,
                                                    OS_TimeBasePthreadEntry, arg.opaque_arg,
                                                    timebase->timebase_name);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    /*
     * Use the internal sync function: the handler thread blocks for the
     * configured interval using HelloX Sleep().  HelloX has no POSIX
     * timer/signal, so the handler thread is itself the tick generator.
     */
    if (timebase->external_sync == NULL)
    {
        timebase->external_sync = OS_TimeBase_SyncImpl;
    }

    return return_code;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseSet_Impl(const OS_object_token_t *token, uint32 start_time, uint32 interval_time)
{
    OS_impl_timebase_internal_record_t *local;
    int32                               return_code;

    (void)start_time;
    (void)interval_time;

    local       = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    return_code = OS_SUCCESS;

    /*
     * HelloX: the handler thread uses Sleep() for a relative wait, so there
     * is no absolute deadline to program.  Just set the reset flag so the
     * sync function reports the start time on the first fire.
     */
    local->reset_flag = 1;
    return return_code;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *local;

    local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    pthread_cancel(local->handler_thread);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseGetInfo_Impl(const OS_object_token_t *token, OS_timebase_prop_t *timer_prop)
{
    return OS_SUCCESS;
}
