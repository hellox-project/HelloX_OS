//***********************************************************************/
//    Author                    : Garry
//    Original Date             : JUN 13, 2021
//    Module Name               : syslog.c
//    Module Funciton           : 
//                                Source file for system logging function.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>

#include "../kthread/syslog.h"

/* Only available when enabled explicitly. */
#if defined(__CFG_APP_SYSLOG)

/* Global logging routine, invoked by other modules directly. */
int __issue_log(const char* fmt, ...)
{
	char* buff = NULL;
	va_list args;
	int n = 0, ret = -1;

	/* fmt must not exceed max length. */
	if (strlen(fmt) > MAX_LOG_INFO_LENGTH)
	{
		goto __TERMINAL;
	}

	/*
	 * Create buffer to hold the logging content,
	 * it's size is too large that we don't alloc
	 * from stack, since it's may lead stack overflow.
	 */
	buff = _hx_malloc(MAX_LOG_INFO_LENGTH + 128);
	if (NULL == buff)
	{
		goto __TERMINAL;
	}

	/* Construct the logging information. */
	va_start(args, fmt);
	n = _hx_vsprintf(buff, fmt, args);
	va_end(args);

	/* Drop it to log manager. */
	LogManager.WriteLog(buff, strlen(buff));

__TERMINAL:
	if (buff)
	{
		_hx_free(buff);
	}
	return ret;
}

#endif //__CFG_APP_SYSLOG
