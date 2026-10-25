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
** File: sample_lib_stubs.c
**
** Purpose:
** Unit test stubs for the SAMPLE library
**
** Notes:
** A stub function file should be implemented for each
** FSW source code unit.  This defines the same functions
** as the public API of the Library.
**
** This is to support testing of OTHER applications which
** depend on / call into this library.  The unit test of
** the external application will be linked with this stub
** library, rather than the real implementation.
*/

/*
 * The stub functions always share the same API header file as the
 * main FSW code does.  This should define the same functions
 * with the same parameters.
 */
#include "sample_lib.h"

/*
 * The "utstubs.h" provides the generic stub framework from UT Assert
 */
#include "utstubs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Sample Init function stub                                       */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SAMPLE_LIB_Init(void)
{
    /*
     * The UT_DEFAULT_IMPL macro is generally sufficient
     * for any call that does not have output parameters
     * and returns an integer status code.
     *
     * The default return value is 0, unless the test
     * case configures something different.
     */
    return UT_DEFAULT_IMPL(SAMPLE_LIB_Init);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Sample Lib function stub                                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 SAMPLE_LIB_Function(void)
{
    return UT_DEFAULT_IMPL(SAMPLE_LIB_Function);
}
