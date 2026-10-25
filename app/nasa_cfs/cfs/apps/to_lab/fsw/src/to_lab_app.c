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
 *  This file contains the source code for the TO lab application
 */

#include "to_lab_app.h"
#include "to_lab_msg.h"
#include "to_lab_events.h"
#include "to_lab_msgids.h"
#include "to_lab_perfids.h"
#include "to_lab_version.h"
#include "to_lab_sub_table.h"

/*
** Global Data Section
*/
typedef struct
{
    CFE_SB_PipeId_t Tlm_pipe;
    CFE_SB_PipeId_t Cmd_pipe;
    osal_id_t       TLMsockid;
    bool            downlink_on;
    char            tlm_dest_IP[17];
    bool            suppress_sendto;

    TO_LAB_HkTlm_t        HkTlm;
    TO_LAB_DataTypesTlm_t DataTypesTlm;
} TO_LAB_GlobalData_t;

TO_LAB_GlobalData_t TO_LAB_Global;

TO_LAB_Subs_t *  TO_LAB_Subs;
CFE_TBL_Handle_t TO_SubTblHandle;

/*
** Prototypes Section
*/
void  TO_LAB_openTLM(void);
int32 TO_LAB_init(void);
void  TO_LAB_exec_local_command(CFE_SB_Buffer_t *SBBufPtr);
void  TO_LAB_process_commands(void);
void  TO_LAB_forward_telemetry(void);

/*
 * Individual Command Handler prototypes
 */
int32 TO_LAB_AddPacket(const TO_LAB_AddPacketCmd_t *data);
int32 TO_LAB_Noop(const TO_LAB_NoopCmd_t *data);
int32 TO_LAB_EnableOutput(const TO_LAB_EnableOutputCmd_t *data);
int32 TO_LAB_RemoveAll(const TO_LAB_RemoveAllCmd_t *data);
int32 TO_LAB_RemovePacket(const TO_LAB_RemovePacketCmd_t *data);
int32 TO_LAB_ResetCounters(const TO_LAB_ResetCountersCmd_t *data);
int32 TO_LAB_SendDataTypes(const TO_LAB_SendDataTypesCmd_t *data);
int32 TO_LAB_SendHousekeeping(const CFE_MSG_CommandHeader_t *data);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                   */
/* TO_LAB_AppMain() -- Application entry point and main process loop */
/*                                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TO_LAB_AppMain(void)
{
    uint32 RunStatus = CFE_ES_RunStatus_APP_RUN;
    int32  status;

    CFE_ES_PerfLogEntry(TO_LAB_MAIN_TASK_PERF_ID);

    status = TO_LAB_init();

    if (status != CFE_SUCCESS)
    {
        return;
    }

    /*
    ** TO RunLoop
    */
    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        CFE_ES_PerfLogExit(TO_LAB_MAIN_TASK_PERF_ID);

        OS_TaskDelay(TO_LAB_TASK_MSEC); /*2 Hz*/

        CFE_ES_PerfLogEntry(TO_LAB_MAIN_TASK_PERF_ID);

        TO_LAB_forward_telemetry();

        TO_LAB_process_commands();
    }

    CFE_ES_ExitApp(RunStatus);
}

/*
** TO delete callback function.
** This function will be called in the event that the TO app is killed.
** It will close the network socket for TO
*/
void TO_LAB_delete_callback(void)
{
    OS_printf("TO delete callback -- Closing TO Network socket.\n");
    if (TO_LAB_Global.downlink_on)
    {
        OS_close(TO_LAB_Global.TLMsockid);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_init() -- TO initialization                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_init(void)
{
    int32  status;
    char   PipeName[16];
    uint16 PipeDepth;
    uint16 i;
    char   ToTlmPipeName[16];
    uint16 ToTlmPipeDepth;

    TO_LAB_Global.downlink_on = false;
    PipeDepth                 = TO_LAB_CMD_PIPE_DEPTH;
    strcpy(PipeName, "TO_LAB_CMD_PIPE");
    ToTlmPipeDepth = TO_LAB_TLM_PIPE_DEPTH;
    strcpy(ToTlmPipeName, "TO_LAB_TLM_PIPE");

    /*
    ** Register with EVS
    */
    CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    /*
    ** Initialize housekeeping packet (clear user data area)...
    */
    CFE_MSG_Init(CFE_MSG_PTR(TO_LAB_Global.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(TO_LAB_HK_TLM_MID),
                 sizeof(TO_LAB_Global.HkTlm));

    status = CFE_TBL_Register(&TO_SubTblHandle, "TO_LAB_Subs", sizeof(*TO_LAB_Subs), CFE_TBL_OPT_DEFAULT, NULL);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TO_LAB_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't register table status %i",
                          __LINE__, (int)status);
        return status;
    }

    status = CFE_TBL_Load(TO_SubTblHandle, CFE_TBL_SRC_FILE, "/cf/tolabsub.tbl");

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TO_LAB_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't load table status %i", __LINE__,
                          (int)status);
        return status;
    }

    status = CFE_TBL_GetAddress((void *)&TO_LAB_Subs, TO_SubTblHandle);

    if (status != CFE_SUCCESS && status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(TO_LAB_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't get table addr status %i",
                          __LINE__, (int)status);
        return status;
    }

    /* Subscribe to my commands */
    status = CFE_SB_CreatePipe(&TO_LAB_Global.Cmd_pipe, PipeDepth, PipeName);
    if (status == CFE_SUCCESS)
    {
        CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), TO_LAB_Global.Cmd_pipe);
        CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TO_LAB_SEND_HK_MID), TO_LAB_Global.Cmd_pipe);
    }
    else
        CFE_EVS_SendEvent(TO_LAB_CRCMDPIPE_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't create cmd pipe status %i",
                          __LINE__, (int)status);

    /* Create TO TLM pipe */
    status = CFE_SB_CreatePipe(&TO_LAB_Global.Tlm_pipe, ToTlmPipeDepth, ToTlmPipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(TO_LAB_TLMPIPE_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't create Tlm pipe status %i",
                          __LINE__, (int)status);
    }

    /* Subscriptions for TLM pipe*/
    for (i = 0; (i < (sizeof(TO_LAB_Subs->Subs) / sizeof(TO_LAB_Subs->Subs[0]))); i++)
    {
        if (!CFE_SB_IsValidMsgId(TO_LAB_Subs->Subs[i].Stream))
        {
            /* Only process until invalid MsgId (aka TO_LAB_UNUSED) is found*/
            break;
        }
        else if (CFE_SB_IsValidMsgId(TO_LAB_Subs->Subs[i].Stream))
        {
            status = CFE_SB_SubscribeEx(TO_LAB_Subs->Subs[i].Stream, TO_LAB_Global.Tlm_pipe, TO_LAB_Subs->Subs[i].Flags,
                                        TO_LAB_Subs->Subs[i].BufLimit);
        }

        if (status != CFE_SUCCESS)
            CFE_EVS_SendEvent(TO_LAB_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "L%d TO Can't subscribe to stream 0x%x status %i", __LINE__,
                              (unsigned int)CFE_SB_MsgIdToValue(TO_LAB_Subs->Subs[i].Stream), (int)status);
    }

    /*
    ** Install the delete handler
    */
    OS_TaskInstallDeleteHandler(&TO_LAB_delete_callback);

    CFE_EVS_SendEvent(TO_LAB_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TO Lab Initialized.%s, Awaiting enable command.", TO_LAB_VERSION_STRING);

    /*
     * HelloX/Yamcs integration: auto-enable the telemetry downlink at
     * startup so telemetry flows to the Yamcs ground station on the host
     * without requiring an explicit "Enable Output" command.
     */
    strcpy(TO_LAB_Global.tlm_dest_IP, CFG_GROUND_IP);
    TO_LAB_Global.suppress_sendto = false;
    TO_LAB_openTLM();
    TO_LAB_Global.downlink_on = true;

    CFE_EVS_SendEvent(TO_LAB_TLMOUTENA_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TO telemetry output auto-enabled for IP %s", TO_LAB_Global.tlm_dest_IP);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_EnableOutput() -- TLM output enabled                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_EnableOutput(const TO_LAB_EnableOutputCmd_t *data)
{
    const TO_LAB_EnableOutput_Payload_t *pCmd = &data->Payload;

    (void)CFE_SB_MessageStringGet(TO_LAB_Global.tlm_dest_IP, pCmd->dest_IP, "", sizeof(TO_LAB_Global.tlm_dest_IP),
                                  sizeof(pCmd->dest_IP));
    TO_LAB_Global.suppress_sendto = false;
    CFE_EVS_SendEvent(TO_LAB_TLMOUTENA_INF_EID, CFE_EVS_EventType_INFORMATION, "TO telemetry output enabled for IP %s",
                      TO_LAB_Global.tlm_dest_IP);

    if (!TO_LAB_Global.downlink_on) /* Then turn it on, otherwise we will just switch destination addresses*/
    {
        TO_LAB_openTLM();
        TO_LAB_Global.downlink_on = true;
    }

    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_process_commands() -- Process command pipe message       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TO_LAB_process_commands(void)
{
    CFE_SB_Buffer_t *SBBufPtr;
    CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;

    while (1)
    {
        switch (CFE_SB_ReceiveBuffer(&SBBufPtr, TO_LAB_Global.Cmd_pipe, CFE_SB_POLL))
        {
            case CFE_SUCCESS:

                CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

                /* For SB return statuses that imply a message: process it. */
                switch (CFE_SB_MsgIdToValue(MsgId))
                {
                    case TO_LAB_CMD_MID:
                        TO_LAB_exec_local_command(SBBufPtr);
                        break;

                    case TO_LAB_SEND_HK_MID:
                        TO_LAB_SendHousekeeping((const CFE_MSG_CommandHeader_t *)SBBufPtr);
                        break;

                    default:
                        CFE_EVS_SendEvent(TO_LAB_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "L%d TO: Invalid Msg ID Rcvd 0x%x", __LINE__,
                                          (unsigned int)CFE_SB_MsgIdToValue(MsgId));
                        break;
                }
                break;
            default:
                /* Exit command processing loop if no message received. */
                return;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/*  TO_LAB_exec_local_command() -- Process local message           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TO_LAB_exec_local_command(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case TO_LAB_NOOP_CC:
            TO_LAB_Noop((const TO_LAB_NoopCmd_t *)SBBufPtr);
            break;

        case TO_LAB_RESET_STATUS_CC:
            TO_LAB_ResetCounters((const TO_LAB_ResetCountersCmd_t *)SBBufPtr);
            break;

        case TO_LAB_SEND_DATA_TYPES_CC:
            TO_LAB_SendDataTypes((const TO_LAB_SendDataTypesCmd_t *)SBBufPtr);
            break;

        case TO_LAB_ADD_PKT_CC:
            TO_LAB_AddPacket((const TO_LAB_AddPacketCmd_t *)SBBufPtr);
            break;

        case TO_LAB_REMOVE_PKT_CC:
            TO_LAB_RemovePacket((const TO_LAB_RemovePacketCmd_t *)SBBufPtr);
            break;

        case TO_LAB_REMOVE_ALL_PKT_CC:
            TO_LAB_RemoveAll((const TO_LAB_RemoveAllCmd_t *)SBBufPtr);
            break;

        case TO_LAB_OUTPUT_ENABLE_CC:
            TO_LAB_EnableOutput((const TO_LAB_EnableOutputCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(TO_LAB_FNCODE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "L%d TO: Invalid Function Code Rcvd In Ground Command 0x%x", __LINE__,
                              (unsigned int)CommandCode);
            ++TO_LAB_Global.HkTlm.Payload.CommandErrorCounter;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_Noop() -- Noop Handler                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_Noop(const TO_LAB_NoopCmd_t *data)
{
    CFE_EVS_SendEvent(TO_LAB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command");
    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_ResetCounters() -- Reset counters                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_ResetCounters(const TO_LAB_ResetCountersCmd_t *data)
{
    TO_LAB_Global.HkTlm.Payload.CommandErrorCounter = 0;
    TO_LAB_Global.HkTlm.Payload.CommandCounter      = 0;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_SendDataTypes()  -- Output data types                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_SendDataTypes(const TO_LAB_SendDataTypesCmd_t *data)
{
    int16 i;
    char  string_variable[10] = "ABCDEFGHIJ";

    /* initialize data types packet */
    CFE_MSG_Init(CFE_MSG_PTR(TO_LAB_Global.DataTypesTlm.TelemetryHeader), CFE_SB_ValueToMsgId(TO_LAB_DATA_TYPES_MID),
                 sizeof(TO_LAB_Global.DataTypesTlm));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TO_LAB_Global.DataTypesTlm.TelemetryHeader));

    /* initialize the packet data */
    TO_LAB_Global.DataTypesTlm.Payload.synch = 0x6969;
#if 0
    TO_LAB_Global.DataTypesTlm.Payload.bit1 = 1;
    TO_LAB_Global.DataTypesTlm.Payload.bit2 = 0;
    TO_LAB_Global.DataTypesTlm.Payload.bit34 = 2;
    TO_LAB_Global.DataTypesTlm.Payload.bit56 = 3;
    TO_LAB_Global.DataTypesTlm.Payload.bit78 = 1;
    TO_LAB_Global.DataTypesTlm.Payload.nibble1 = 0xA;
    TO_LAB_Global.DataTypesTlm.Payload.nibble2 = 0x4;
#endif
    TO_LAB_Global.DataTypesTlm.Payload.bl1 = false;
    TO_LAB_Global.DataTypesTlm.Payload.bl2 = true;
    TO_LAB_Global.DataTypesTlm.Payload.b1  = 16;
    TO_LAB_Global.DataTypesTlm.Payload.b2  = 127;
    TO_LAB_Global.DataTypesTlm.Payload.b3  = 0x7F;
    TO_LAB_Global.DataTypesTlm.Payload.b4  = 0x45;
    TO_LAB_Global.DataTypesTlm.Payload.w1  = 0x2468;
    TO_LAB_Global.DataTypesTlm.Payload.w2  = 0x7FFF;
    TO_LAB_Global.DataTypesTlm.Payload.dw1 = 0x12345678;
    TO_LAB_Global.DataTypesTlm.Payload.dw2 = 0x87654321;
    TO_LAB_Global.DataTypesTlm.Payload.f1  = 90.01;
    TO_LAB_Global.DataTypesTlm.Payload.f2  = .0000045;
    TO_LAB_Global.DataTypesTlm.Payload.df1 = 99.9;
    TO_LAB_Global.DataTypesTlm.Payload.df2 = .4444;

    for (i = 0; i < 10; i++)
        TO_LAB_Global.DataTypesTlm.Payload.str[i] = string_variable[i];

    CFE_SB_TransmitMsg(CFE_MSG_PTR(TO_LAB_Global.DataTypesTlm.TelemetryHeader), true);

    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_SendHousekeeping() -- HK status                          */
/* Does not increment CommandCounter                               */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_SendHousekeeping(const CFE_MSG_CommandHeader_t *data)
{
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TO_LAB_Global.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(TO_LAB_Global.HkTlm.TelemetryHeader), true);
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_openTLM() -- Open TLM                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TO_LAB_openTLM(void)
{
    int32 status;

    status = OS_SocketOpen(&TO_LAB_Global.TLMsockid, OS_SocketDomain_INET, OS_SocketType_DATAGRAM);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(TO_LAB_TLMOUTSOCKET_ERR_EID, CFE_EVS_EventType_ERROR, "L%d, TO TLM socket error: %d",
                          __LINE__, (int)status);
    }

    /*---------------- Add static arp entries ----------------*/
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_AddPacket() -- Add packets                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_AddPacket(const TO_LAB_AddPacketCmd_t *data)
{
    const TO_LAB_AddPacket_Payload_t *pCmd = &data->Payload;
    int32                             status;

    status = CFE_SB_SubscribeEx(pCmd->Stream, TO_LAB_Global.Tlm_pipe, pCmd->Flags, pCmd->BufLimit);

    if (status != CFE_SUCCESS)
        CFE_EVS_SendEvent(TO_LAB_ADDPKT_ERR_EID, CFE_EVS_EventType_ERROR, "L%d TO Can't subscribe 0x%x status %i",
                          __LINE__, (unsigned int)CFE_SB_MsgIdToValue(pCmd->Stream), (int)status);
    else
        CFE_EVS_SendEvent(TO_LAB_ADDPKT_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "L%d TO AddPkt 0x%x, QoS %d.%d, limit %d", __LINE__,
                          (unsigned int)CFE_SB_MsgIdToValue(pCmd->Stream), pCmd->Flags.Priority,
                          pCmd->Flags.Reliability, pCmd->BufLimit);

    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_RemovePacket() -- Remove Packet                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_RemovePacket(const TO_LAB_RemovePacketCmd_t *data)
{
    const TO_LAB_RemovePacket_Payload_t *pCmd = &data->Payload;
    int32                                status;

    status = CFE_SB_Unsubscribe(pCmd->Stream, TO_LAB_Global.Tlm_pipe);
    if (status != CFE_SUCCESS)
        CFE_EVS_SendEvent(TO_LAB_REMOVEPKT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "L%d TO Can't Unsubscribe to Stream 0x%x, status %i", __LINE__,
                          (unsigned int)CFE_SB_MsgIdToValue(pCmd->Stream), (int)status);
    else
        CFE_EVS_SendEvent(TO_LAB_REMOVEPKT_INF_EID, CFE_EVS_EventType_INFORMATION, "L%d TO RemovePkt 0x%x", __LINE__,
                          (unsigned int)CFE_SB_MsgIdToValue(pCmd->Stream));
    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_RemoveAll() --  Remove All Packets                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 TO_LAB_RemoveAll(const TO_LAB_RemoveAllCmd_t *data)
{
    int32 status;
    int   i;

    for (i = 0; (i < (sizeof(TO_LAB_Subs->Subs) / sizeof(TO_LAB_Subs->Subs[0]))); i++)
    {
        if (CFE_SB_IsValidMsgId(TO_LAB_Subs->Subs[i].Stream))
        {
            status = CFE_SB_Unsubscribe(TO_LAB_Subs->Subs[i].Stream, TO_LAB_Global.Tlm_pipe);

            if (status != CFE_SUCCESS)
                CFE_EVS_SendEvent(TO_LAB_REMOVEALLPTKS_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "L%d TO Can't Unsubscribe to stream 0x%x status %i", __LINE__,
                                  (unsigned int)CFE_SB_MsgIdToValue(TO_LAB_Subs->Subs[i].Stream), (int)status);
        }
    }

    CFE_EVS_SendEvent(TO_LAB_REMOVEALLPKTS_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "L%d TO Unsubscribed to all Commands and Telemetry", __LINE__);

    ++TO_LAB_Global.HkTlm.Payload.CommandCounter;
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* TO_LAB_forward_telemetry() -- Forward telemetry                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void TO_LAB_forward_telemetry(void)
{
    OS_SockAddr_t    d_addr;
    int32            status;
    int32            CFE_SB_status;
    size_t           size;
    CFE_SB_Buffer_t *SBBufPtr;

    OS_SocketAddrInit(&d_addr, OS_SocketDomain_INET);
    OS_SocketAddrSetPort(&d_addr, cfgTLM_PORT);
    OS_SocketAddrFromString(&d_addr, TO_LAB_Global.tlm_dest_IP);
    status = 0;

    do
    {
        CFE_SB_status = CFE_SB_ReceiveBuffer(&SBBufPtr, TO_LAB_Global.Tlm_pipe, CFE_SB_POLL);

        if ((CFE_SB_status == CFE_SUCCESS) && (TO_LAB_Global.suppress_sendto == false))
        {
            CFE_MSG_GetSize(&SBBufPtr->Msg, &size);

            if (TO_LAB_Global.downlink_on == true)
            {
                CFE_ES_PerfLogEntry(TO_LAB_SOCKET_SEND_PERF_ID);

                status = OS_SocketSendTo(TO_LAB_Global.TLMsockid, SBBufPtr, size, &d_addr);

                CFE_ES_PerfLogExit(TO_LAB_SOCKET_SEND_PERF_ID);
            }
            else
            {
                status = 0;
            }
            if (status < 0)
            {
                CFE_EVS_SendEvent(TO_LAB_TLMOUTSTOP_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "L%d TO sendto error %d. Tlm output suppressed\n", __LINE__, (int)status);
                TO_LAB_Global.suppress_sendto = true;
            }
        }
        /* If CFE_SB_status != CFE_SUCCESS, then no packet was received from CFE_SB_ReceiveBuffer() */
    } while (CFE_SB_status == CFE_SUCCESS);
}

/************************/
/*  End of File Comment */
/************************/
