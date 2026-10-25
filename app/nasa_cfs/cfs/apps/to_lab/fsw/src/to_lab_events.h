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
 *   Define TO Lab Event messages
 */
#ifndef TO_LAB_EVENTS_H
#define TO_LAB_EVENTS_H

/*****************************************************************************/

/* Event message ID's */
#define TO_LAB_EVM_RESERVED 0

#define TO_LAB_INIT_INF_EID          1
#define TO_LAB_CRCMDPIPE_ERR_EID     2
#define TO_LAB_TLMOUTENA_INF_EID     3
#define TO_LAB_SUBSCRIBE_ERR_EID     4
#define TO_LAB_TLMPIPE_ERR_EID       5
#define TO_LAB_TLMOUTSOCKET_ERR_EID  6
#define TO_LAB_TLMOUTSTOP_ERR_EID    7
#define TO_LAB_MSGID_ERR_EID         8
#define TO_LAB_FNCODE_ERR_EID        9
#define TO_LAB_ADDPKT_ERR_EID        10
#define TO_LAB_REMOVEPKT_ERR_EID     11
#define TO_LAB_REMOVEALLPTKS_ERR_EID 12
#define TO_LAB_ADDPKT_INF_EID        15
#define TO_LAB_REMOVEPKT_INF_EID     16
#define TO_LAB_REMOVEALLPKTS_INF_EID 17
#define TO_LAB_NOOP_INF_EID          18
#define TO_LAB_TBL_ERR_EID           19

/******************************************************************************/

#endif
