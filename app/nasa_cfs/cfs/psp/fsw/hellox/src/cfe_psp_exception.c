/*
 * cfe_psp_exception.c - HelloX PSP exception handling (degraded).
 *
 * HelloX has no POSIX signals/backtrace, so exceptions are not captured;
 * these functions are minimal and the exception log is never populated.
 */

#include <string.h>

#include "common_types.h"
#include "osapi.h"
#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "cfe_psp_exceptionstorage_types.h"
#include "cfe_psp_exceptionstorage_api.h"

void CFE_PSP_AttachExceptions(void)
{
    /* HelloX: no signal handlers to attach. */
}

void CFE_PSP_SetDefaultExceptionEnvironment(void)
{
    /* HelloX: no per-task exception environment. */
}

int32 CFE_PSP_ExceptionGetSummary_Impl(const CFE_PSP_Exception_LogData_t *Buffer, char *ReasonBuf, uint32 ReasonSize)
{
    const char *reason = "unknown";

    if (ReasonBuf != NULL && ReasonSize > 0)
    {
        strncpy(ReasonBuf, reason, ReasonSize);
        ReasonBuf[ReasonSize - 1] = 0;
    }

    return CFE_PSP_SUCCESS;
}
