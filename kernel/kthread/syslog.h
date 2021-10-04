//***********************************************************************/
//    Author                    : Garry
//    Original Date             : JUN 13, 2021
//    Module Name               : syslog.h
//    Module Funciton           : 
//                                Header file for system logging function.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                1. 
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYSLOG_H__
#define __SYSLOG_H__

/* 
 * Maximal length of a logging information and 
 * it's header. The header is appended by log manager,
 * it contains the thread name, time, and other
 * common information.
 */
#define MAX_LOG_INFO_LENGTH 512
#define MAX_LOG_HEADER_LENGTH 128

/* Log file's dir and name. */
#define LOG_FILE_PATHNAME "c:\\syslog\\logfile.txt"

/* 
 * Log entry, corresponding to each log originated by 
 * other applications or kernel threads. When __log routine is invoked,
 * a new log entry is created and linked into log entry list of
 * logging manager.
 */
typedef struct tag__LOG_ENTRY {
	/* Log infor string's length. */
	unsigned long log_info_length;
	char log_info[MAX_LOG_INFO_LENGTH + MAX_LOG_HEADER_LENGTH];
	/* Logging time. */
	BYTE log_time[6];
	/* Owner name of thread who originates this log entry. */
	char owner_name[MAX_THREAD_NAME];

	/* Pointing to next one in list. */
	struct tag__LOG_ENTRY* pNext;
}__LOG_ENTRY;

/* Log manager object. */
typedef struct tag__LOGGING_MANAGER {
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	/* Turn on/off logging. */
	BOOL bLogOn;
	/* If show logging on screen. */
	BOOL bShowLogging;
	/* File object to store log information. */
	HANDLE log_file;
	/* Event to wakeup the background thread. */
	HANDLE wake_event;

	/* Log entry list and it's current length. */
	__LOG_ENTRY* pLogListHeader;
	__LOG_ENTRY* pLogListTail;
	unsigned long log_list_length;

	/* Object of the logging thread, runs in background. */
	__COMMON_OBJECT* pLogThread;

	/* Operation routines. Initializer of this object. */
	BOOL (*Initialize)(struct tag__LOGGING_MANAGER* pLogManager);
	/* Issue a log. */
	BOOL (*WriteLog)(const char* pLogString, size_t log_length);
}__LOGGING_MANAGER;

#if defined(__CFG_APP_SYSLOG)
/* Global logging manager object. */
extern __LOGGING_MANAGER LogManager;
#endif

#endif //__SYSLOG_H__
