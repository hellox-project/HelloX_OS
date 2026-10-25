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
 *   Specification for the sample library functions.
 */
#ifndef SAMPLE_LIB_H
#define SAMPLE_LIB_H

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*************************************************************************
** Exported Functions
*************************************************************************/

/************************************************************************/
/** \brief Library Initialization Function
**
**  \par Description
**        This function is required by CFE to initialize the library
**        It should be specified in the cfe_es_startup.scr file as part
**        of loading this library.  It is not directly invoked by
**        applications.
**
**  \par Assumptions, External Events, and Notes:
**        None
**
**  \return Execution status, see \ref CFEReturnCodes
**
**
*************************************************************************/
int32 SAMPLE_LIB_Init(void);

/************************************************************************/
/** \brief Sample Lib Function
**
**  \par Description
**        This is a sample function
**
**  \par Assumptions, External Events, and Notes:
**        None
**
**  \return Execution status, see \ref CFEReturnCodes
**
**
*************************************************************************/
int32 SAMPLE_LIB_Function(void);

#endif
