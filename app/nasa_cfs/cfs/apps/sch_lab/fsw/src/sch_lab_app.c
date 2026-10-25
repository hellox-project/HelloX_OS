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
 *  This file contains the source code for the SCH lab application
 */

/*
** Include Files
*/
#include <string.h>

#include "cfe.h"
#include "cfe_sb.h"
#include "osapi.h"
#include "cfe_es.h"
#include "cfe_error.h"

#include "sch_lab_perfids.h"
#include "sch_lab_version.h"

/*
** SCH Lab Schedule table from the platform inc directory
*/
#include "sch_lab_table.h"

/*
** Global Structure
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16                  MessageBuffer[SCH_MAX_MSG_WORDS];
    uint16                  PayloadLength;
    uint32                  PacketRate;
    uint32                  Counter;
} SCH_LAB_StateEntry_t;

typedef struct
{
    SCH_LAB_StateEntry_t State[SCH_LAB_MAX_SCHEDULE_ENTRIES];
    osal_id_t            TimerId;
    osal_id_t            TimingSem;
    CFE_TBL_Handle_t     TblHandle;
    CFE_SB_PipeId_t      CmdPipe;
} SCH_LAB_GlobalData_t;

/*
** Global Variables
*/
SCH_LAB_GlobalData_t SCH_LAB_Global;


/*
** Local Function Prototypes
*/
int32 SCH_LAB_AppInit(void);

/*
** AppMain
*/
void SCH_Lab_AppMain(void)
{
    int                   i;
    uint32                SCH_OneHzPktsRcvd = 0;
    int32                 OsStatus;
    CFE_Status_t          Status;
    uint32                RunStatus = CFE_ES_RunStatus_APP_RUN;
    SCH_LAB_StateEntry_t *LocalStateEntry;
    CFE_SB_Buffer_t *     SBBufPtr;

    CFE_ES_PerfLogEntry(SCH_MAIN_TASK_PERF_ID);

    Status = SCH_LAB_AppInit();
    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SCH_LAB: Error Initializing RC = 0x%08lX\n", (unsigned long)Status);
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /* Loop Forever */
    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        CFE_ES_PerfLogExit(SCH_MAIN_TASK_PERF_ID);

        /* Pend on timing sem */
        OsStatus = OS_CountSemTake(SCH_LAB_Global.TimingSem);
        if (OsStatus == OS_SUCCESS)
        {
            /* check for arrival of the 1Hz - this should sync counts (TBD) */
            Status = CFE_SB_ReceiveBuffer(&SBBufPtr, SCH_LAB_Global.CmdPipe, CFE_SB_POLL);
        }
        else
        {
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }

        CFE_ES_PerfLogEntry(SCH_MAIN_TASK_PERF_ID);

        if (Status == CFE_SUCCESS)
        {
            SCH_OneHzPktsRcvd++;
        }

        if (OsStatus == OS_SUCCESS && SCH_OneHzPktsRcvd > 0)
        {
            /*
            ** Process table every tick, sending packets that are ready
            */
            LocalStateEntry = SCH_LAB_Global.State;

            for (i = 0; i < SCH_LAB_MAX_SCHEDULE_ENTRIES; i++)
            {
                if (LocalStateEntry->PacketRate != 0)
                {
                    ++LocalStateEntry->Counter;
                    if (LocalStateEntry->Counter >= LocalStateEntry->PacketRate)
                    {
                        LocalStateEntry->Counter = 0;
                        CFE_SB_TransmitMsg(CFE_MSG_PTR(LocalStateEntry->CommandHeader), true);
                    }
                }
                ++LocalStateEntry;
            }
        }

    } /* end while */

    CFE_ES_ExitApp(Status);
}

void SCH_LAB_LocalTimerCallback(osal_id_t object_id, void *arg)
{
    OS_CountSemGive(SCH_LAB_Global.TimingSem);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialization                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SCH_LAB_AppInit(void)
{
    int                           i, x;
    int32                         Status;
    int32                         OsStatus;
    uint32                        TimerPeriod;
    osal_id_t                     TimeBaseId = OS_OBJECT_ID_UNDEFINED;
    SCH_LAB_ScheduleTable_t *     ConfigTable;
    SCH_LAB_ScheduleTableEntry_t *ConfigEntry;
    SCH_LAB_StateEntry_t *        LocalStateEntry;
    void *                        TableAddr;

    memset(&SCH_LAB_Global, 0, sizeof(SCH_LAB_Global));

    OsStatus = OS_CountSemCreate(&SCH_LAB_Global.TimingSem, "SCH_LAB", 0, 0);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s: OS_CountSemCreate failed:RC=%ld\n", __func__, (long)OsStatus);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* The underlying timebase object should have been created by the PSP */
    OsStatus = OS_TimeBaseGetIdByName(&TimeBaseId, "cFS-Master");
    if (OsStatus != OS_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s: OS_TimeBaseGetIdByName failed:RC=%ld\n", __func__, (long)OsStatus);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Create the timer callback (but not set yet, as that requires the config table) */
    OsStatus = OS_TimerAdd(&SCH_LAB_Global.TimerId, "SCH_LAB", TimeBaseId, SCH_LAB_LocalTimerCallback, NULL);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s: OS_TimerAdd failed:RC=%ld\n", __func__, (long)OsStatus);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /*
    ** Register tables with cFE and load default data
    */
    Status = CFE_TBL_Register(&SCH_LAB_Global.TblHandle, "SCH_LAB_SchTbl", sizeof(SCH_LAB_ScheduleTable_t),
                              CFE_TBL_OPT_DEFAULT, NULL);

    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SCH_LAB: Error Registering SCH_LAB_SchTbl, RC = 0x%08lX\n", (unsigned long)Status);

        return Status;
    }
    else
    {
        /*
        ** Loading Table
        */
        Status = CFE_TBL_Load(SCH_LAB_Global.TblHandle, CFE_TBL_SRC_FILE, SCH_TBL_DEFAULT_FILE);
        if (Status != CFE_SUCCESS)
        {
            CFE_ES_WriteToSysLog("SCH_LAB: Error Loading Table SCH_LAB_SchTbl, RC = 0x%08lX\n", (unsigned long)Status);
            CFE_TBL_ReleaseAddress(SCH_LAB_Global.TblHandle);

            return Status;
        }
    }

    /*
    ** Get Table Address
    */
    Status = CFE_TBL_GetAddress(&TableAddr, SCH_LAB_Global.TblHandle);
    if (Status != CFE_SUCCESS && Status != CFE_TBL_INFO_UPDATED)
    {
        CFE_ES_WriteToSysLog("SCH_LAB: Error Getting Table's Address SCH_LAB_SchTbl, RC = 0x%08lX\n",
                             (unsigned long)Status);

        return Status;
    }

    /*
    ** Initialize the command headers
    */
    ConfigTable     = TableAddr;
    ConfigEntry     = ConfigTable->Config;
    LocalStateEntry = SCH_LAB_Global.State;

    /* Populate the CCSDS message and move the message content into the proper user data space. */
    for (i = 0; i < SCH_LAB_MAX_SCHEDULE_ENTRIES; i++)
    {
        if (ConfigEntry->PacketRate != 0)
        {
            /* Initialize the message with the length of the header + payload */
            CFE_MSG_Init(CFE_MSG_PTR(LocalStateEntry->CommandHeader), ConfigEntry->MessageID,
                        sizeof(LocalStateEntry->CommandHeader) + ConfigEntry->PayloadLength);
            CFE_MSG_SetFcnCode(CFE_MSG_PTR(LocalStateEntry->CommandHeader), ConfigEntry->FcnCode);

            LocalStateEntry->PacketRate = ConfigEntry->PacketRate;
            LocalStateEntry->PayloadLength = ConfigEntry->PayloadLength;

            for (x =0; x < SCH_MAX_MSG_WORDS; x++)
            {
                LocalStateEntry->MessageBuffer[x]=ConfigEntry->MessageBuffer[x];
            }
        }
        ++ConfigEntry;
        ++LocalStateEntry;
    }

    if (ConfigTable->TickRate == 0)
    {
        /* use default of 1 second */
        CFE_ES_WriteToSysLog("%s: Using default tick rate of 1 second\n", __func__);
        TimerPeriod = 1000000;
    }
    else
    {
        TimerPeriod = 1000000 / ConfigTable->TickRate;
        if ((TimerPeriod * ConfigTable->TickRate) != 1000000)
        {
            CFE_ES_WriteToSysLog("%s: WARNING: tick rate of %lu is not an integer number of microseconds\n", __func__,
                                 (unsigned long)ConfigTable->TickRate);
        }
    }

    /*
    ** Release the table
    */
    Status = CFE_TBL_ReleaseAddress(SCH_LAB_Global.TblHandle);
    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SCH_LAB: Error Releasing Table SCH_LAB_SchTbl, RC = 0x%08lX\n", (unsigned long)Status);
    }

    /* Create pipe and subscribe to the 1Hz pkt */
    Status = CFE_SB_CreatePipe(&SCH_LAB_Global.CmdPipe, 8, "SCH_LAB_CMD_PIPE");
    if (Status != CFE_SUCCESS)
    {
        OS_printf("SCH Error creating pipe!\n");
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CFE_TIME_1HZ_CMD_MID), SCH_LAB_Global.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        OS_printf("SCH Error subscribing to 1hz!\n");
    }

    /* Set timer period */
    OsStatus = OS_TimerSet(SCH_LAB_Global.TimerId, 1000000, TimerPeriod);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s: OS_TimerSet failed:RC=%ld\n", __func__, (long)OsStatus);
    }

    OS_printf("SCH Lab Initialized.%s\n", SCH_LAB_VERSION_STRING);

    return CFE_SUCCESS;
}
