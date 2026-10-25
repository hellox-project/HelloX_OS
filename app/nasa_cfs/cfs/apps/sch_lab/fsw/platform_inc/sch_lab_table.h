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
 *  This file contains the schedule tables for the SCH Lab app.
 *  It is intended to go in the platform include directory so the SCH_LAB
 *  app source code does not have to be modified.
 */
#ifndef SCH_LAB_TABLE_H
#define SCH_LAB_TABLE_H

#include "cfe_sb_extern_typedefs.h" /* for CFE_SB_MsgId_t */
#include "cfe_msg_api_typedefs.h"   /* For CFE_MSG_FcnCode_t */
#include "cfe_msgids.h"

/*
** Defines
*/
#define SCH_LAB_END_OF_TABLE         0
#define SCH_LAB_MAX_SCHEDULE_ENTRIES 32
#define SCH_TBL_DEFAULT_FILE         "/cf/schlabtb.tbl"

#define SCH_MAX_MSG_WORDS 32

#ifdef SOFTWARE_BIG_BIT_ORDER 
#define SCH_PACK_32BIT(value) \
(uint16)((value & 0xFFFF0000) >> 16), (uint16)(value & 0x0000FFFF) 
#else 
#define SCH_PACK_32BIT(value) \
(uint16)(value & 0x0000FFFF), (uint16)((value & 0xFFFF0000) >> 16)
#endif

/*
** Typedefs
*/
typedef struct
{
    CFE_SB_MsgId_t    MessageID;                        /* Message ID for the table entry */
    uint32            PacketRate;                       /* Rate: Send packet every N ticks */
    CFE_MSG_FcnCode_t FcnCode;                          /* Command/Function code to set */
    uint16            PayloadLength;                    /* Length of additional command args */
    uint16            MessageBuffer[SCH_MAX_MSG_WORDS]; /* Command args in 16 bit words */
} SCH_LAB_ScheduleTableEntry_t;

typedef struct
{
    uint32                       TickRate; /* Ticks per second to configure for timer (0=default) */
    SCH_LAB_ScheduleTableEntry_t Config[SCH_LAB_MAX_SCHEDULE_ENTRIES];
} SCH_LAB_ScheduleTable_t;

#endif
