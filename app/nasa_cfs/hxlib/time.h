
#ifndef	__TIME_H__
#define	__TIME_H__

#include "__hxcomm.h"

/*
 * Number of clock ticks per second. A clock tick is the unit by which
 * processor time is measured and is returned by 'clock'.
 */

#ifndef _TIME_T_DEFINED
#define _TIME_T_DEFINED
typedef long long time_t;
#endif

#ifndef _CLOCK_T_DEFINED
#define _CLOCK_T_DEFINED
typedef signed long long clock_t;
#endif

#ifndef _CLOCKID_T_DEFINED
#define _CLOCKID_T_DEFINED
typedef signed long clockid_t;
#endif

#ifndef _TIME64_T_DEFINED
#define _TIME64_T_DEFINED
typedef signed long long __time64_t;
#endif

#define	CLOCKS_PER_SEC	((clock_t)1000)
#define	CLK_TCK		CLOCKS_PER_SEC

#ifndef _TM_DEFINED
/*
 * A structure for storing all kinds of useful information about the
 * current (or another) time.
 */
struct tm
{
	int	tm_sec;		/* Seconds: 0-59 (K&R says 0-61?) */
	int	tm_min;		/* Minutes: 0-59 */
	int	tm_hour;	/* Hours since midnight: 0-23 */
	int	tm_mday;	/* Day of the month: 1-31 */
	int	tm_mon;		/* Months *since* january: 0-11 */
	int	tm_year;	/* Years since 1900 */
	int	tm_wday;	/* Days since Sunday (0-6) */
	int	tm_yday;	/* Days since Jan. 1: 0-365 */
	int	tm_isdst;	/* +1 Daylight Savings Time, 0 No DST,
				 * -1 don't know */
};
#define _TM_DEFINED
#endif  //_TM_DEFINED.

#ifdef	__cplusplus
extern "C" {
#endif

 clock_t  	    clock (void);
 time_t  	    time (time_t*);
 time_t  	    difftime (time_t, time_t);
 time_t  	    mktime (struct tm*);
 char*  		asctime (const struct tm*);
 char*  		ctime (const time_t*);
 struct tm*   	gmtime (const time_t*);
 struct tm*   	localtime (const time_t*);
 size_t  		strftime(char*, size_t, const char*, const struct tm*);
 char*  	    strdate(char*);
 char*  	    strtime(char*);

 struct timeval{
	 long tv_sec;
	 long tv_usec;
 };

 struct timezone{
	 int tz_minuteswest;
	 int tz_dsttime;
 };

int gettimeofday(struct timeval* tp,struct timezone* tzp);

/* timespec object. */
 struct timespec{
	 long long tv_sec;
	 long long tv_nsec;
 };

 /*
 * Thread safe version of localtime under Linux,
 * it's just used to compatible with Linux and
 * should be revised future to support thread safe.
 */
 struct tm* localtime_r(const time_t *timer, struct tm *buf);

 /* clock_gettime under Linux. */
 int clock_gettime(clockid_t clk_id, struct timespec *tp);

 /* Clock ID's of clock_gettime. */
#define CLOCK_REALTIME             0x01
#define CLOCK_MONOTONIC            0x02
#define CLOCK_PROCESS_CPUTIME_ID   0x03
#define CLOCK_THREAD_CPUTIME_ID    0x04

 /* Sleep functions */
 /* Sleep for the requested interval. *rem is zeroed on return. */
 int nanosleep(const struct timespec *req, struct timespec *rem);

 /* Sleep for the given number of microseconds. Returns 0 on success. */
 int usleep(unsigned int usec);

 /* 
  * Sleep for the given number of seconds. 
  * Returns remaining seconds (always 0). 
  */
 unsigned int secondsleep(unsigned int seconds);

#ifdef	__cplusplus
}
#endif

#endif	/* __TIME_H__ */
