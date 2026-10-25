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
 *   This file is main hdr file for the Command Ingest lab application.
 */
#ifndef CI_LAB_APP_H
#define CI_LAB_APP_H

/*
** Required header files...
*/
#include "common_types.h"
#include "cfe.h"

#include "osapi.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/****************************************************************************/

#define CI_LAB_BASE_UDP_PORT 1234
#define CI_LAB_MAX_INGEST    768
#define CI_LAB_PIPE_DEPTH    32

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes...
**
** Note: Except for the entry point (CI_LAB_AppMain), these
**       functions are not called from any other source module.
*/
void CI_Lab_AppMain(void);
void CI_LAB_TaskInit(void);
void CI_LAB_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr);
void CI_LAB_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr);
void CI_LAB_ResetCounters_Internal(void);
void CI_LAB_ReadUpLink(void);

bool CI_LAB_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif
