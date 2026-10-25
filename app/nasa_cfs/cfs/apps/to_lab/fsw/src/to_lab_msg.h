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
 * @file
 *   Define TO Lab Messages and info
 */
#ifndef TO_LAB_MSG_H
#define TO_LAB_MSG_H

#define TO_LAB_NOOP_CC            0 /*  no-op command     */
#define TO_LAB_RESET_STATUS_CC    1 /*  reset status      */
#define TO_LAB_ADD_PKT_CC         2 /*  add packet        */
#define TO_LAB_SEND_DATA_TYPES_CC 3 /*  send data types   */
#define TO_LAB_REMOVE_PKT_CC      4 /*  remove packet     */
#define TO_LAB_REMOVE_ALL_PKT_CC  5 /*  remove all packet */
#define TO_LAB_OUTPUT_ENABLE_CC   6 /*  output enable     */

/******************************************************************************/

typedef struct
{
    uint8 CommandCounter;
    uint8 CommandErrorCounter;
    uint8 spareToAlign[2];
} TO_LAB_HkTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    TO_LAB_HkTlm_Payload_t    Payload;         /**< \brief Telemetry payload */
} TO_LAB_HkTlm_t;

/******************************************************************************/

typedef struct
{
    uint16 synch;
#if 0
    bit_field          bit1:1;
    bit_field          bit2:1;
    bit_field          bit34:2;
    bit_field          bit56:2;
    bit_field          bit78:2;

    bit_field          nibble1:4;
    bit_field          nibble2:4;
#endif
    uint8  bl1, bl2; /* boolean */
    int8   b1, b2, b3, b4;
    int16  w1, w2;
    int32  dw1, dw2;
    float  f1, f2;
    double df1, df2;
    char   str[10];
} TO_LAB_DataTypes_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader; /**< \brief Telemetry header */
    TO_LAB_DataTypes_Payload_t Payload;         /**< \brief Telemetry payload */
} TO_LAB_DataTypesTlm_t;

/******************************************************************************/

typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeade; /**< \brief Command header */
} TO_LAB_NoArgsCmd_t;

/*
 * The following commands do not have any payload,
 * but should still "reserve" a unique structure type to
 * employ a consistent handler pattern.
 *
 * This matches the pattern in CFE core and other modules.
 */
typedef TO_LAB_NoArgsCmd_t TO_LAB_NoopCmd_t;
typedef TO_LAB_NoArgsCmd_t TO_LAB_ResetCountersCmd_t;
typedef TO_LAB_NoArgsCmd_t TO_LAB_RemoveAllCmd_t;
typedef TO_LAB_NoArgsCmd_t TO_LAB_SendDataTypesCmd_t;

typedef struct
{
    CFE_SB_MsgId_t Stream;
    CFE_SB_Qos_t   Flags;
    uint8          BufLimit;
} TO_LAB_AddPacket_Payload_t;

typedef struct
{
    CFE_MSG_CommandHeader_t    CmdHeader; /**< \brief Command header */
    TO_LAB_AddPacket_Payload_t Payload;   /**< \brief Command payload */
} TO_LAB_AddPacketCmd_t;

/*****************************************************************************/

typedef struct
{
    CFE_SB_MsgId_t Stream;
    CFE_SB_Qos_t   Flags;
    uint16         BufLimit;
} TO_LAB_subscription_t;

/******************************************************************************/

typedef struct
{
    CFE_SB_MsgId_t Stream;
} TO_LAB_RemovePacket_Payload_t;

typedef struct
{
    CFE_MSG_CommandHeader_t       CmdHeader; /**< \brief Command header */
    TO_LAB_RemovePacket_Payload_t Payload;   /**< \brief Command payload */
} TO_LAB_RemovePacketCmd_t;

/******************************************************************************/

typedef struct
{
    char dest_IP[16];
} TO_LAB_EnableOutput_Payload_t;

typedef struct
{
    CFE_MSG_CommandHeader_t       CmdHeader; /**< \brief Command header */
    TO_LAB_EnableOutput_Payload_t Payload;   /**< \brief Command payload */
} TO_LAB_EnableOutputCmd_t;

/******************************************************************************/

#endif
