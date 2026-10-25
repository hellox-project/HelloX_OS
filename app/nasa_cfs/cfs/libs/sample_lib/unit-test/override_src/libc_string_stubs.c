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
** File: libc_string_stubs.c
**
** Purpose:
** Demonstrates the concept of compile-time replacement
** for C library calls.
**
** This provides the implementation of the "OCS_strncpy"
** function, which is the stub for the C library strncpy call.
**
** Notes:
** For most Unit tests this is _NOT_ necessary.  Whenever
** possible, FSW code should only call CFE/OSAL/PSP code for
** which there are already stubs available and the
** replacements can be made transparently at link time.
*/

/*
 * Note - these are the _real_ system headers here
 * as the "overrides" dir is _not_ part of the include path here
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utstubs.h"

/*
 * This is for the prototype of the "OCS_strncpy()" function
 */
#include "OCS_string.h"

/* **********************************
 * Implementation of OCS_strncpy stub
 * **********************************/
char *OCS_strncpy(char *dst, const char *src, unsigned long size)
{
    int32  Status;
    uint32 CopySize;

    Status = UT_DEFAULT_IMPL(OCS_strncpy);

    if (Status != 0)
    {
        /*
         * a "failure" response -
         * the real C library would never do this.
         */
        return (char *)0;
    }
    /*
     * Demonstrate use of a test-provided local data buffer
     * this provides the "real" data that is written to the output
     */
    CopySize = UT_Stub_CopyToLocal(UT_KEY(OCS_strncpy), dst, size);
    if (CopySize < size)
    {
        /* clear the remained, as strncpy would */
        memset(&dst[CopySize], 0, size - CopySize);
    }

    /* normal response is to return a pointer to the source */
    return dst;
}
