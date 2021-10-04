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

#include "syslog.h"

/* Only available when enabled explicitly. */
#if defined(__CFG_APP_SYSLOG)

/* 
 * Helper routine to process log entry list, 
 * invoked by background thread. 
 */
static BOOL __process_log_list()
{
	__LOG_ENTRY* pLogEntry = NULL;
	unsigned long ulFlags;

	/* Log Manager must be inited properly. */
	BUG_ON(NULL == LogManager.log_file);
	BUG_ON(NULL == LogManager.wake_event);

	/* 
	 * Travel the whole list to get one from it if there is, 
	 * and write into file. 
	 */
	do {
		__ENTER_CRITICAL_SECTION_SMP(LogManager.spin_lock, ulFlags);
		pLogEntry = NULL;
		if (LogManager.pLogListHeader)
		{
			/* Unlink it from list. */
			pLogEntry = LogManager.pLogListHeader;
			LogManager.pLogListHeader = pLogEntry->pNext;
			LogManager.log_list_length--;
			if (NULL == LogManager.pLogListHeader)
			{
				/* No entry in list, reset tail. */
				LogManager.pLogListTail = NULL;
			}
		}
		__LEAVE_CRITICAL_SECTION_SMP(LogManager.spin_lock, ulFlags);

		if (pLogEntry)
		{
			/* Write it to file and destroy it. */
			if (LogManager.bLogOn)
			{
				if (!WriteFile(LogManager.log_file, pLogEntry->log_info_length,
					pLogEntry->log_info, NULL))
				{
					_hx_printf("[%s]write file failed.\r\n", __func__);
				}
			}
			if (LogManager.bShowLogging)
			{
				/* Show it out on screen. */
				_hx_printf(pLogEntry->log_info);
			}
			_hx_free(pLogEntry);
		}
	} while (pLogEntry);

	return TRUE;
}

/*
 * Background thread of logging service. The thread is 
 * created in process of system initialization, it's response is
 * write log into log file according request.
 */
static unsigned long __log_background_thread(LPVOID pData)
{
	HANDLE log_file = NULL;
	HANDLE wait_event = NULL;
	uint32_t file_sz = 0;

	/* Event object must be created. */
	BUG_ON(NULL == LogManager.wake_event);

	/* Open the logging file. */
	log_file = CreateFile(LOG_FILE_PATHNAME, 
		FILE_ACCESS_READWRITE | FILE_OPEN_ALWAYS, 0, NULL);
	if (NULL == log_file)
	{
		/* Could not open log file, exit thread. */
		_hx_printf("[%s]could not open logging file[%s]\r\n", __func__,
			LOG_FILE_PATHNAME);
		goto __TERMINAL;
	}
	/* Set the file pointer to file's end. */
	file_sz = GetFileSize(log_file, NULL);
	SetFilePointer(log_file, &file_sz, NULL, FILE_FROM_BEGIN);
	LogManager.log_file = log_file;

	/* Main loop. */
	while (TRUE)
	{
		unsigned long wait_result = WaitForThisObject(LogManager.wake_event);
		switch (wait_result)
		{
		case OBJECT_WAIT_RESOURCE:
			/* Should check log entry list and process it. */
			__process_log_list();
			ResetEvent(LogManager.wake_event);
			break;
		case OBJECT_WAIT_DELETED:
			_hx_printf("[%s]event deleted, thread exit.\r\n", __func__);
			goto __TERMINAL;
		default:
			goto __TERMINAL;
		}
	}

__TERMINAL:
	if (log_file)
	{
		CloseFile(log_file);
	}
	if (wait_event)
	{
		DestroyEvent(wait_event);
	}
	return 0;
}

/* 
 * Issue a log. 
 * The routine create a new log entry object, init it
 * using the parameters caller specified, put it into
 * log list, trigger the background thread to process 
 * it.
 */
static BOOL __WriteLog(const char* pLogString, size_t log_length)
{
	BOOL bResult = FALSE, bShouldTrigger = FALSE;
	__LOG_ENTRY* pLogEntry = NULL;
	char* pLogInfo = NULL;
	int offset = 0;
	unsigned long ulFlags;

	if ((NULL == pLogString) || (0 == log_length))
	{
		goto __TERMINAL;
	}
	if (log_length > MAX_LOG_INFO_LENGTH)
	{
		goto __TERMINAL;
	}
	/* Set the terminator of log string. */
	((char*)pLogString)[log_length] = 0;

	/* Create a new log entry and initialized it. */
	pLogEntry = _hx_malloc(sizeof(__LOG_ENTRY));
	if (NULL == pLogEntry)
	{
		_hx_printf("[%s]out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	/* Init it. */
	memset(pLogEntry, 0, sizeof(__LOG_ENTRY));
	/* Set log's owner's name accordingly. */
	if (IN_SYSINITIALIZATION())
	{
		/* In process of system initialization. */
		strcpy(pLogEntry->owner_name, "sys_init");
	}
	else
	{
		if (IN_INTERRUPT())
		{
			strcpy(pLogEntry->owner_name, "sys_interrupt");
		}
		else {
			/* In process context. */
			strcpy(pLogEntry->owner_name, __CURRENT_KERNEL_THREAD->KernelThreadName);
		}
	}
	/* Set the logging time. */
	__GetTime(pLogEntry->log_time);

	/* Construct the log content. */
	pLogInfo = &pLogEntry->log_info[0];
	offset = _hx_sprintf(pLogInfo, "[%s: ", pLogEntry->owner_name);
	pLogInfo += offset;
	offset = _hx_sprintf(pLogInfo, "%d-%d-%d %d:%d:%d]",
		pLogEntry->log_time[0] + 2000,
		pLogEntry->log_time[1],
		pLogEntry->log_time[2],
		pLogEntry->log_time[3],
		pLogEntry->log_time[4],
		pLogEntry->log_time[5]);
	pLogInfo += offset;
	offset = _hx_sprintf(pLogInfo, "%s", pLogString);
	pLogInfo += offset;
	pLogEntry->log_info_length = pLogInfo - &pLogEntry->log_info[0];

	/* Link the log entry into list. */
	__ENTER_CRITICAL_SECTION_SMP(LogManager.spin_lock, ulFlags);
	if (NULL == LogManager.pLogListHeader)
	{
		/* No log entry in list yet. */
		LogManager.pLogListHeader = pLogEntry;
		LogManager.pLogListTail = pLogEntry;
		LogManager.log_list_length = 1;
		/* Should trigger the back thread process it. */
		bShouldTrigger = TRUE;
	}
	else {
		/* Not the fist, just link to tail. */
		BUG_ON(NULL == LogManager.pLogListTail);
		LogManager.pLogListTail->pNext = pLogEntry;
		LogManager.pLogListTail = pLogEntry;
		LogManager.log_list_length++;
	}
	__LEAVE_CRITICAL_SECTION_SMP(LogManager.spin_lock, ulFlags);

	/* Trigger the back ground thread to process it. */
	if (bShouldTrigger)
	{
		BUG_ON(NULL == LogManager.wake_event);
		SetEvent(LogManager.wake_event);
	}

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Release the allocated resource in case of no success. */
		if (pLogEntry)
		{
			_hx_free(pLogEntry);
		}
	}
	return bResult;
}

/* Initializer of the log manager. */
static BOOL __Initialize(__LOGGING_MANAGER* pLogManager)
{
	BOOL bResult = FALSE;
	HANDLE log_thread = NULL;
	HANDLE wait_event = NULL;

	BUG_ON(NULL == pLogManager);

	/* Create the event object. */
	wait_event = CreateEvent(FALSE);
	if (NULL == wait_event)
	{
		_hx_printf("[%s]failed to create event.\r\n", __func__);
		goto __TERMINAL;
	}
	pLogManager->wake_event = wait_event;

	/* Create the back ground thread. */
	log_thread = CreateKernelThread(0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_LOW,
		__log_background_thread,
		NULL, NULL,
		"LogMgR");
	if (NULL == log_thread)
	{
		_hx_printf("[%s]can not create log thread.\r\n", __func__);
		goto __TERMINAL;
	}
	pLogManager->pLogThread = log_thread;

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Global object log manager. */
__LOGGING_MANAGER LogManager = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,     //spin lock.
#endif
	TRUE,                     //bLogOn.
	FALSE,                    //bShowLogging.
	NULL,                     //log_file.
	NULL,                     //wake_event.

	NULL,                     //Log list header.
	NULL,                     //Log list tail.
	0,                        //Log list length.

	NULL,                     //Back ground thread handle.

	/* Operations. */
	__Initialize,            //Initialized.
	__WriteLog,              //WriteLog.
};

#endif //__CFG_APP_SYSLOG
