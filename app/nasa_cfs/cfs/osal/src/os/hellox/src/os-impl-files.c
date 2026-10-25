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
 * \ingroup  posix
 * \author   joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "os-hellox.h"
#include "os-impl-files.h"
#include "os-shared-file.h"

/****************************************************************************************
                                     GLOBALS
 ***************************************************************************************/

/*
 * The global file handle table.
 *
 * This is shared by all OSAL entities that perform low-level I/O.
 */
OS_impl_file_internal_record_t OS_impl_filehandle_table[OS_MAX_NUM_OPEN_FILES];

/*
 * These two constants (EUID and EGID) are local cache of the
 * euid and egid of the user running the OSAL application.  They
 * assist the "stat" implementation in determination of permissions.
 *
 * For an OS that does not have multiple users, these could be
 * defined as 0.  Otherwise they should be populated via the system
 * geteuid/getegid calls.
 */
uid_t OS_IMPL_SELF_EUID = 0;
gid_t OS_IMPL_SELF_EGID = 0;

/*
 * Flag(s) to set on file handles for regular files.
 *
 * HelloX's select() (lwip_select) only supports sockets, NOT regular
 * files.  If O_NONBLOCK is set here, every regular file handle gets
 * marked "selectable", and then reads/writes are routed through
 * OS_SelectSingle_Impl -> select(), which returns OS_ERR_OPERATION_NOT_SUPPORTED
 * (-38) for a regular file descriptor.  That breaks reading the ES startup
 * script (/cf/cfe_es_startup.scr) and any other file I/O.
 *
 * Leave this as 0 (blocking) so regular files bypass select() entirely
 * and go straight to read()/write().
 */
const int OS_IMPL_REGULAR_FILE_FLAGS = 0;

/****************************************************************************************
                         IMPLEMENTATION-SPECIFIC ROUTINES
             These are specific to this particular operating system
 ****************************************************************************************/

/* --------------------------------------------------------------------------------------
    Name: OS_Hellox_StreamAPI_Impl_Init

    Purpose: File/Stream subsystem global initialization

    Returns: OS_SUCCESS if success
 ---------------------------------------------------------------------------------------*/
int32 OS_Hellox_StreamAPI_Impl_Init(void)
{
    osal_index_t local_id;

    /*
     * init all filehandles to -1, which is always invalid.
     * this isn't strictly necessary but helps when debugging.
     */
    for (local_id = 0; local_id < OS_MAX_NUM_OPEN_FILES; ++local_id)
    {
        OS_impl_filehandle_table[local_id].fd = -1;
    }

    OS_IMPL_SELF_EUID = geteuid();
    OS_IMPL_SELF_EGID = getegid();

    return OS_SUCCESS;
}
