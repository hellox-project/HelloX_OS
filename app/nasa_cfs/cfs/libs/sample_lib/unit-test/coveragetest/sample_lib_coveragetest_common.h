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

/*
** File: sample_lib_coveragetest_common.h
**
** Purpose:
** Common definitions for all sample_lib coverage tests
*/

#ifndef SAMPLE_LIB_COVERAGETEST_COMMON_H
#define SAMPLE_LIB_COVERAGETEST_COMMON_H

/*
 * Includes
 */

#include "utassert.h"
#include "uttest.h"
#include "utstubs.h"

/*
 * Use the public API/definitions from CFE and SAMPLE LIB
 */
#include "cfe.h"
#include "sample_lib_internal.h"

/*
 * Macro to call a function and check its int32 return code
 *
 * This just simplifies the use of UtAssert_True() for the
 * frequent case where a function needs to be called and
 * an int32 return code needs to be tested.
 */
#define UT_TEST_FUNCTION_RC(func, exp)                                                                \
    {                                                                                                 \
        int32 rcexp = exp;                                                                            \
        int32 rcact = func;                                                                           \
        UtAssert_True(rcact == rcexp, "%s (%ld) == %s (%ld)", #func, (long)rcact, #exp, (long)rcexp); \
    }

/*
 * Macro to add a test case to the list of tests to execute
 * This just simplifies the use of UtTest_Add()
 */
#define ADD_TEST(test) UtTest_Add((Test_##test), Sample_UT_Setup, Sample_UT_TearDown, #test)

/*
 * Setup function prior to every test
 */
void Sample_UT_Setup(void);

/*
 * Teardown function after every test
 */
void Sample_UT_TearDown(void);

#endif
