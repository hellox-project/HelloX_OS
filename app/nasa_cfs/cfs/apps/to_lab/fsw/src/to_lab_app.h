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
 *   Define TO Lab Application header file
 */

#ifndef TO_LAB_APP_H
#define TO_LAB_APP_H

#include "cfe.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "common_types.h"
#include "osapi.h"

/*****************************************************************************/

#define TO_LAB_TASK_MSEC 500 /* run at 2 Hz */
#define TO_LAB_UNUSED    CFE_SB_MSGID_RESERVED

/**
 * Depth of pipe for commands to the TO_LAB application itself
 */
#define TO_LAB_CMD_PIPE_DEPTH 8

/**
 * Depth of pipe for telemetry forwarded through the TO_LAB application
 */
#define TO_LAB_TLM_PIPE_DEPTH OS_QUEUE_MAX_DEPTH

/*
 * Ground station (Yamcs) downlink target.
 * These are build-time configurable so that each satellite VM can be
 * compiled with a different destination IP and port.  Override them via
 * -DCFG_GROUND_IP=... -DCFG_GROUND_PORT=... on the compiler command line.
 */
#ifndef CFG_GROUND_IP
#define CFG_GROUND_IP "192.168.169.8"
#endif
#ifndef CFG_GROUND_PORT
#define CFG_GROUND_PORT 1235
#endif

#define cfgTLM_ADDR        CFG_GROUND_IP
#define cfgTLM_PORT        CFG_GROUND_PORT
#define TO_LAB_VERSION_NUM "5.1.0"

/******************************************************************************/

/*
** Prototypes Section
*/
void TO_LAB_AppMain(void);

/******************************************************************************/

#endif
