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
 *  Define TO Lab CPU specific subscription table
 */

#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */

#include "to_lab_sub_table.h"

/*
** Add the proper include file for the message IDs below
*/
#include "to_lab_msgids.h"

#ifdef HAVE_CI_LAB
#include "ci_lab_msgids.h"
#endif

#ifdef HAVE_SAMPLE_APP
#include "sample_app_msgids.h"
#endif

#ifdef HAVE_HS_APP
#include "hs_msgids.h"
#endif

#ifdef HAVE_FM_APP
#include "fm_msgids.h"
#endif

#ifdef HAVE_SC_APP
#include "sc_msgids.h"
#endif

#ifdef HAVE_DS_APP
#include "ds_msgids.h"
#endif

#ifdef HAVE_LC_APP
#include "lc_msgids.h"
#endif

TO_LAB_Subs_t TO_LAB_Subs = {.Subs = {/* CFS App Subscriptions */
                                      {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(TO_LAB_DATA_TYPES_MID), {0, 0}, 4},

                                      /* cFE Core subscriptions */
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_HK_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_TIME_DIAG_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_SB_STATS_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_REG_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_EVS_LONG_EVENT_MSG_MID), {0, 0}, 32},

                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_APP_TLM_MID), {0, 0}, 4},
                                      {CFE_SB_MSGID_WRAP_VALUE(CFE_ES_MEMSTATS_TLM_MID), {0, 0}, 4},

#ifdef HAVE_CI_LAB
                                      {CFE_SB_MSGID_WRAP_VALUE(CI_LAB_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_SAMPLE_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(SAMPLE_APP_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_HS_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(HS_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_HS_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(FM_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_HS_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(SC_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_HS_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(DS_HK_TLM_MID), {0, 0}, 4},
#endif
#ifdef HAVE_HS_APP
                                      {CFE_SB_MSGID_WRAP_VALUE(LC_HK_TLM_MID), {0, 0}, 4},
#endif

                                      /* CFE_SB_MSGID_RESERVED entry to mark the end of valid MsgIds */
                                      {CFE_SB_MSGID_RESERVED, {0, 0}, 0}}};

CFE_TBL_FILEDEF(TO_LAB_Subs, TO_LAB_APP.TO_LAB_Subs, TO Lab Sub Tbl, tolabsub.tbl)
