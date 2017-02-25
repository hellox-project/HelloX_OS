/**
 * Adaptation file of JerryScript under HelloX operating system.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "jerry-port.h"
#include "config.h"  /* For __DEFINE_DISPATCH_XXXX_ROUTINE macro.*/
#include "ecma-globals.h" /* For ecma_value_t type. */
#include "ecma-exceptions.h"
#include "ecma-helpers.h"

/**
* Signal the port that jerry experienced a fatal failure from which it cannot
* recover.
*
* @param code gives the cause of the error.
*
* Note:
*      Jerry expects the function not to return.
*
* Example: a libc-based port may implement this with exit() or abort(), or both.
*/
void jerry_port_fatal(jerry_fatal_code_t code)
{
	_hx_printf("Fatal error with code = %d.\r\n", code);
	exit(0);
}

/*
*  I/O Port API
*/

/**
* Print a string to the console. The function should implement a printf-like
* interface, where the first argument specifies a format string on how to
* stringify the rest of the parameter list.
*
* This function is only called with strings coming from the executed ECMAScript
* wanting to print something as the result of its normal operation.
*
* It should be the port that decides what a "console" is.
*
* Example: a libc-based port may implement this with vprintf().
*/
void jerry_port_console(const char *format, ...)
{
	va_list args;
	char buff[512];
	va_start(args, format);
	_hx_vsprintf(buff, format, args);
	_hx_printf(buff);
	va_end(args);
}

/**
* Display or log a debug/error message. The function should implement a printf-like
* interface, where the first argument specifies the log level
* and the second argument specifies a format string on how to stringify the rest
* of the parameter list.
*
* This function is only called with messages coming from the jerry engine as
* the result of some abnormal operation or describing its internal operations
* (e.g., data structure dumps or tracing info).
*
* It should be the port that decides whether error and debug messages are logged to
* the console, or saved to a database or to a file.
*
* Example: a libc-based port may implement this with vfprintf(stderr) or
* vfprintf(logfile), or both, depending on log level.
*/
void jerry_port_log(jerry_log_level_t level, const char *format, ...)
{
	va_list args;
	char buff[512];
	va_start(args, format);
	_hx_vsprintf(buff, format, args);
	_hx_printf("JERRY_LOG[level = %d]:%s\r\n", level, buff);
	va_end(args);
}

/*
* Date Port API
*/

/**
* Get timezone and daylight saving data
*
* @return true  - if success
*         false - otherwise
*/
bool jerry_port_get_time_zone(jerry_time_zone_t *tz_p)
{
	tz_p->daylight_saving_time = 0;
	tz_p->offset = 0;
	return true;
}

/**
* Get system time
*
* @return milliseconds since Unix epoch
*/
double jerry_port_get_current_time(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) != 0)
	{
		return 0.0;
	}
	return ((double)tv.tv_sec) * 1000.0 + ((double)tv.tv_usec) / 1000.0;
}

/**
 * Some missed routines when compiling under VS2013.
 * I don't know why these routines are missed in source release,
 * just supply them here.
 */
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(arraybuffer_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(arraybuffer_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(array_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(array_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(boolean_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(boolean_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(date_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(date_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(eval_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(eval_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(number_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(number_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(object_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(object_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(range_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(range_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(reference_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(reference_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(regexp_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(regexp_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(string_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(string_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(syntax_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(syntax_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(type_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(type_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(uri_error_prototype);
__DEFINE_DISPATCH_CALL_ROUTINE(uri_error_prototype);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(math);
__DEFINE_DISPATCH_CALL_ROUTINE(math);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(json);
__DEFINE_DISPATCH_CALL_ROUTINE(json);
__DEFINE_DISPATCH_CONSTRUCT_ROUTINE(global);
__DEFINE_DISPATCH_CALL_ROUTINE(global);
