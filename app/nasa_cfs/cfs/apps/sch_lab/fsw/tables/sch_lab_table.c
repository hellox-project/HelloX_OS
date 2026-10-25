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

#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */
#include "sch_lab_table.h"
#include "cfe_sb.h" /* Required to use the CFE_SB_MSGID_WRAP_VALUE macro */

#ifdef HAVE_CI_LAB
#include "ci_lab_msgids.h"
#endif

#ifdef HAVE_TO_LAB
#include "to_lab_msgids.h"
#endif

#ifdef HAVE_SAMPLE_APP
#include "sample_app_msgids.h"
#endif

#ifdef HAVE_HS
#include "hs_msgids.h"
#endif

#ifdef HAVE_FM
#include "fm_msgids.h"
#endif

#ifdef HAVE_SC
#include "sc_msgids.h"
#endif

#ifdef HAVE_DS
#include "ds_msgids.h"
#endif

#ifdef HAVE_LC
#include "lc_msgids.h"
#endif


/*
** Include headers for message IDs here
*/
#include "ci_lab_msgids.h"
#include "to_lab_msgids.h"

#include "sample_app_msgids.h"

/*
** SCH Lab schedule table
** When populating this table:
**  1. The entire table is processed (SCH_LAB_MAX_SCHEDULE_ENTRIES) but entries with a
**     packet rate of 0 are skipped
**  2. You can have commented out entries or entries with a packet rate of 0
**  3. If the table grows too big, increase SCH_LAB_MAX_SCHEDULE_ENTRIES
*/

SCH_LAB_ScheduleTable_t SCH_TBL_Structure = {.TickRate = 100,
                                             .Config   = {
                                                 {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_SEND_HK_MID), 100, 0},   /* Example of a 1hz packet */
                                                 {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_SEND_HK_MID), 50, 0},
                                                 {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_SEND_HK_MID), 98, 0},
                                                 {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_SEND_HK_MID), 97, 0},
                                                 {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_SEND_HK_MID), 96, 0},

                                            /* Example of including additional open source apps  */
                                            #ifdef HAVE_CI_LAB
                                                 {CFE_SB_MSGID_WRAP_VALUE(CI_LAB_SEND_HK_MID), 95, 0},
                                            #endif
                                            #ifdef HAVE_TO_LAB
                                                 {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_SEND_HK_MID), 94, 0},
                                            #endif
                                            #ifdef HAVE_SAMPLE_APP
                                                 {CFE_SB_MSGID_WRAP_VALUE(SAMPLE_APP_SEND_HK_MID), 93, 0},
                                            #endif
                                            #ifdef HAVE_SC
                                                 {CFE_SB_MSGID_WRAP_VALUE(SC_SEND_HK_MID), 92, 0},
                                                 {CFE_SB_MSGID_WRAP_VALUE(SC_1HZ_WAKEUP_MID), 91, 0},
                                            #endif
                                            #ifdef HAVE_HS
                                                 {CFE_SB_MSGID_WRAP_VALUE(HS_SEND_HK_MID), 90, 0}, /* Example of a message that wouldn't be sent */
                                            #endif
                                            #ifdef HAVE_FM
                                                 {CFE_SB_MSGID_WRAP_VALUE(FM_SEND_HK_MID), 101, 0},
                                            #endif
                                            #ifdef HAVE_DS
                                                 {CFE_SB_MSGID_WRAP_VALUE(DS_SEND_HK_MID), 102, 0},
                                            #endif
                                            #ifdef HAVE_LC
                                                {CFE_SB_MSGID_WRAP_VALUE(LC_SEND_HK_MID), 103, 0},
                                                {CFE_SB_MSGID_WRAP_VALUE(LC_SAMPLE_AP_MID),     500, 0, 8, { 0, 175, 1 }},
                                            #endif

                                             }};

/*
** The macro below identifies:
**    1) the data structure type to use as the table image format
**    2) the name of the table to be placed into the cFE Table File Header
**    3) a brief description of the contents of the file image
**    4) the desired name of the table image binary file that is cFE compatible
*/
CFE_TBL_FILEDEF(SCH_TBL_Structure, SCH_LAB_APP.SCH_LAB_SchTbl, Schedule Lab MsgID Table, schlabtb.tbl)
