/* 
 * POSIX standard timer API's implementation. 
 * There may have bugs since these routines only
 * used in several applications.
 */

#include <hellox.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <time_pri.h>

time_t _gmtotime_t (int yr,/* 0 based */int mo,/* 1 based */int dy,/* 1 based */ int hr,int mn,int sc)
{
 	int tmpdays;
 	long tmptim;
 	struct tm tb;
 
 	/*
 	* Do a quick range check on the year and convert it to a delta
 	* off of 1900.
 	*/
 	if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
 	{
 		return (time_t)(-1);
 	}
 
 	tmpdays = dy + _days[mo - 1];
 	if ( !(yr & 3) && (mo > 2) )
 	{
 		tmpdays++;
 	}
 
 	/*
 	* Compute the number of elapsed seconds since the Epoch. Note the
 	* computation of elapsed leap years would break down after 2100
 	* if such values were in range (fortunately, they aren't).
 	*/
 
 	tmptim = (long)yr - _BASE_YEAR;
 
 	tmptim = /* 365 days for each year */
 		( ( ( ( tmptim ) * 365L
 
 		/* one day for each elapsed leap year */
 		+ ((long)(yr - 1) >> 2) - (long)_LEAP_YEAR_ADJUST
 
 		/* number of elapsed days in yr */
 		+ (long)tmpdays )
 
 		/* convert to hours and add in hr */
 		* 24L + (long)hr )
 
 		/* convert to minutes and add in mn */
 		* 60L + (long)mn )
 
 		/* convert to seconds and add in sec */
 		* 60L + (long)sc;
 
 	//_tzset();
 	tmptim += _timezone;        //timezone adjustment
 
 	/*
 	* Fill in enough fields of tb struct for _isindst(), then call it to
 	* determine DST.
 	*/
 	tb.tm_yday = tmpdays;
 	tb.tm_year = yr;
 	tb.tm_mon = mo - 1;
 	tb.tm_hour = hr;
 
 	if (_daylight)// && _isindst(&tb)
 	{
 		tmptim -= 3600L;
 	}
 
 	return (tmptim >= 0) ? (time_t)tmptim : (time_t)(-1);
}

static time_t  _make_time_t (struct tm *tb, int ultflag)
{
 	long tmptm1, tmptm2, tmptm3;
 	struct tm *tbtemp;
 
 	/*
 	* First, make sure tm_year is reasonably close to being in range.
 	*/
 	if ( ((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR+ 1) )
 		goto err_mktime;
 
 	/*
 	* Adjust month value so it is in the range 0 - 11.  This is because
 	* we don't know how many days are in months 12, 13, 14, etc.
 	*/
 
 	if ( (tb->tm_mon < 0) || (tb->tm_mon > 11) ) 
 	{
  		/*
 		* no danger of overflow because the range check above.
 		*/
 		tmptm1 += (tb->tm_mon / 12);
 
 		if ( (tb->tm_mon %= 12) < 0 ) 
 		{
 			tb->tm_mon += 12;
 			tmptm1--;
 		}
 
 		/*
 		* Make sure year count is still in range.
 		*/
 		if ( (tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1) )
 			goto err_mktime;
 	}
 
 	/***** HERE: tmptm1 holds number of elapsed years *****/
 
 	/*
 	* Calculate days elapsed minus one, in the given year, to the given
 	* month. Check for leap year and adjust if necessary.
 	*/
 	tmptm2 = _days[tb->tm_mon];
 	if ( !(tmptm1 & 3) && (tb->tm_mon > 1) )
 		tmptm2++;
 
 	/*
 	* Calculate elapsed days since base date (midnight, 1/1/70, UTC)
 	*
 	*
 	* 365 days for each elapsed year since 1970, plus one more day for
 	* each elapsed leap year. no danger of overflow because of the range
 	* check (above) on tmptm1.
 	*/
 	tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2) - _LEAP_YEAR_ADJUST;
 
 	/*
 	* elapsed days to current month (still no possible overflow)
 	*/
 	tmptm3 += tmptm2; 
 	/*
 	* elapsed days to current date. overflow is now possible.
 	*/
 	tmptm1 = tmptm3 + (tmptm2 = (long)(tb->tm_mday));
 	if ( ChkAdd(tmptm1, tmptm3, tmptm2) )
 		goto err_mktime;
 
 	/*
 	* Calculate elapsed hours since base date
 	*/

 	tmptm2 = tmptm1 * 24L;
 	if ( ChkMul(tmptm2, tmptm1, 24L) )
 		goto err_mktime;
 
 	tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_hour);
 	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
 		goto err_mktime;
 
 	/*
 	* Calculate elapsed minutes since base date
 	*/
 
 	tmptm2 = tmptm1 * 60L;
 	if ( ChkMul(tmptm2, tmptm1, 60L) )
 		goto err_mktime;
 
 	tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_min);
 	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
 		goto err_mktime;
 
 	/***** HERE: tmptm1 holds number of elapsed minutes *****/
 
 	/*
 	* Calculate elapsed seconds since base date
 	*/
 
 	tmptm2 = tmptm1 * 60L;
 	if ( ChkMul(tmptm2, tmptm1, 60L) )
 		goto err_mktime;
 
 	tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_sec);
 	if ( ChkAdd(tmptm1, tmptm2, tmptm3) )
 		goto err_mktime;
 
 	/***** HERE: tmptm1 holds number of elapsed seconds *****/
 
 	if  ( ultflag ) 
 	{
 
 		/*
 		* Adjust for timezone. No need to check for overflow since
 		* localtime() will check its arg value
 		*/
 
 		tmptm1 += _timezone;
 
 		/*
 		* Convert this second count back into a time block structure.
 		* If localtime returns NULL, return an error.
 		*/
 		if ( (tbtemp = localtime(&tmptm1)) == NULL )
 			goto err_mktime;
 
 		/*
 		* Now must compensate for DST. The ANSI rules are to use the
 		* passed-in tm_isdst flag if it is non-negative. Otherwise,
 		* compute if DST applies. Recall that tbtemp has the time without
 		* DST compensation, but has set tm_isdst correctly.
 		*/
 		if ( (tb->tm_isdst > 0) || ((tb->tm_isdst < 0) && (tbtemp->tm_isdst > 0)) ) 
		{
 				tmptm1 += _dstbias;
 				tbtemp = localtime(&tmptm1);    /* reconvert, can't get NULL */
 		}
 
 	}
 	else 
 	{
 		if ( (tbtemp = gmtime(&tmptm1)) == NULL )
 			goto err_mktime;
 	}
 
 
 	*tb = *tbtemp;
 
 	return (time_t)tmptm1;
 
 err_mktime:
 	/*
 	* All errors come to here
 	*/
//return (time_t)(-1);

return 0;
}

time_t  time (time_t* tptr)
{
	BYTE  szDate[6];
	time_t tim;

 	GetSystemTime(szDate);
 	tim = _gmtotime_t(szDate[0] + 2000,szDate[1],szDate[2],szDate[3],szDate[4],szDate[5]);
 
 	if( tptr )
 	{
 		*tptr = tim;
 	}
 	return tim;
}

time_t mktime (struct tm* p)
{
 	return( _make_time_t(p, 1) );
}

struct tm* gmtime (const time_t* timp)
{

	long caltim = *timp;            /* calendar time to convert */
	int islpyr = 0;                 /* is-current-year-a-leap-year flag */
	int tmptim;
	int *mdays;                /* pointer to days or lpdays */

	struct tm *ptb = &tb;

	if ( caltim < 0 )
	{
		return NULL;
	}


	tmptim = (int)(caltim / _FOUR_YEAR_SEC);
	caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

	tmptim = (tmptim * 4) + 70;        // 

	if ( caltim >= _YEAR_SEC ) 
	{
		tmptim++;                       
		caltim -= _YEAR_SEC;

		if ( caltim >= _YEAR_SEC ) 
		{

			tmptim++;                   
			caltim -= _YEAR_SEC;

			if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) 
			{

				tmptim++;           
				caltim -= (_YEAR_SEC + _DAY_SEC);
			}
			else 
			{
				islpyr++;
			}
		}
	}

	ptb->tm_year = tmptim;

	ptb->tm_yday = (int)(caltim / _DAY_SEC);
	caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

	if ( islpyr )
		mdays = _lpdays;
	else
		mdays = _days;


	for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

	ptb->tm_mon = --tmptim;

	ptb->tm_mday = ptb->tm_yday - mdays[tmptim];
	ptb->tm_wday = ((int)(*timp / _DAY_SEC ) + _BASE_DOW) % 7;

	
	ptb->tm_hour = (int)(caltim / 3600);
	caltim -= (long)ptb->tm_hour * 3600L;

	ptb->tm_min = (int)(caltim / 60);
	ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

	ptb->tm_isdst = 0;

	return (struct tm *)ptb;
}

struct tm* localtime (const time_t* ptime)
{
	struct tm *ptm;
	long ltime;

	if ( (long)*ptime < 0 )
	{
		return  NULL;
	}


	if ( (*ptime > 3 * _DAY_SEC) && (*ptime < LONG_MAX - 3 * _DAY_SEC) ) 
	{
		ltime = (long)*ptime - _timezone;
		ptm = gmtime( (time_t *)&ltime );

		if ( _daylight ) 
		{
			ltime -= _dstbias;
			ptm = gmtime( (time_t *)&ltime );
			ptm->tm_isdst = 1;
		}
	}
	else {
		ptm = gmtime( ptime );            
		ltime = (long)ptm->tm_sec - (_timezone + _dstbias);

		ptm->tm_sec = (int)(ltime % 60);
		if ( ptm->tm_sec < 0 ) 
		{
			ptm->tm_sec += 60;
			ltime -= 60;
		}

		ltime = (long)ptm->tm_min + ltime/60;
		ptm->tm_min = (int)(ltime % 60);
		if ( ptm->tm_min < 0 ) 
		{
			ptm->tm_min += 60;
			ltime -= 60;
		}

		ltime = (long)ptm->tm_hour + ltime/60;
		ptm->tm_hour = (int)(ltime % 24);
		if ( ptm->tm_hour < 0 ) 
		{
			ptm->tm_hour += 24;
			ltime -=24;
		}

		ltime /= 24;

		if ( ltime > 0L ) 
		{
			ptm->tm_wday = (ptm->tm_wday + ltime) % 7;
			ptm->tm_mday += ltime;
			ptm->tm_yday += ltime;
		}
		else if ( ltime < 0L ) 
		{
			ptm->tm_wday = (ptm->tm_wday + 7 + ltime) % 7;
			if ( (ptm->tm_mday += ltime) <= 0 ) 
			{
				ptm->tm_mday += 31;
				ptm->tm_yday = 364;
				ptm->tm_mon = 11;
				ptm->tm_year--;
			}
			else 
			{
				ptm->tm_yday += ltime;
			}
		}
	}
	return (ptm);
}

/*
 * Thread safe version of localtime under Linux,
 * it's just used to compatible with Linux and
 * should be revised future to support thread safe.
 */
struct tm* localtime_r(const time_t *timer, struct tm *buf)
{
	struct tm* tm_ret = localtime(timer);
	
	if ((NULL == tm_ret) || (NULL == buf))
	{
		return NULL;
	}
	memcpy(buf, tm_ret, sizeof(struct tm));
	return buf;
}

time_t difftime(time_t b, time_t a)
{
	return((b - a));
}
   
static char *  store_dt(char *p, int val)
{
	*p++ = ('0' + val / 10);
	*p++ = ('0' + val % 10);

	return p;
}
 
char*  asctime (const struct tm* tb)
{
 //	char *p = buf;
  	//int day, mon;
  	//int i;
 
 
 // 	
 // 
 // 	day = tb->tm_wday * 3;          /* index to correct day string */
 // 	mon = tb->tm_mon * 3;           /* index to correct month string */
 // 	for (i=0; i < 3; i++,p++) 
 // 	{
 // 		*p = *(__dnames + day + i);
 // 		*(p+4) = *(__mnames + mon + i);
 // 	}
 // 
 // 	*p = ' ';                   /* blank between day and month */
 // 
 // 	p += 4;
 // 
 // 	*p++ = ' ';
 // 	p = store_dt(p, tb->tm_mday);   /* day of the month (1-31) */
 // 	*p++ = ' ';
 // 	p = store_dt(p, tb->tm_hour);   /* hours (0-23) */
 // 	*p++ = ':';
 // 	p = store_dt(p, tb->tm_min);    /* minutes (0-59) */
 // 	*p++ = ':';
 // 	p = store_dt(p, tb->tm_sec);    /* seconds (0-59) */
 // 	*p++ = ' ';
 // 	p = store_dt(p, 19 + (tb->tm_year/100)); /* year (after 1900) */
 // 	p = store_dt(p, tb->tm_year%100);
 // 	*p++ = '\n';
 // 	*p = '\0';
 
 	return NULL;
 }

 char* ctime (const time_t* timp)
 {
 	struct tm *tmtemp;
 
	tmtemp = localtime(timp); 
 	if( tmtemp != NULL)
 	{
 		return(asctime((const struct tm *)tmtemp));
 	}
 	else
 	{
 		return NULL;
	}
 }

 clock_t clock (void)
{
	 unsigned long long _clock = 0;
	 unsigned long high_32 = 0;
	 unsigned long low_32 = 0;

	 low_32 = GetClockCounter(&high_32);
	 _clock = high_32;
	 _clock <<= sizeof(unsigned long) * 8;
	 _clock += low_32;
	 /* Convert to million seconds. */
	 _clock *= SYSTEM_TIME_SLICE;
	 return _clock;
}

 /* /////// AI GENERATED CODE BEGIN ////////////////////////
  *
  * The following code is generated by 
  * AI(DeepSeek), but tested by Garry. 
  */

 /* ==================== Constants ==================== */

#define NS_PER_SEC       1000000000ULL
#define NS_PER_MS        1000000ULL
#define KERNEL_GRAN_NS   5000000ULL   /* 5 ms kernel sleep granularity */
#define SAFETY_NS        2000000ULL   /* Safety margin before busy-waiting */

/* ==================== Cached state ==================== */

 static uint64_t g_freq_hz = 0;   /* TSC frequency, in Hz */
 static uint64_t g_tsc_realtime = 0;   /* TSC at the CLOCK_REALTIME baseline */
 static time_t   g_sec_realtime = 0;   /* UNIX seconds at the same baseline */
 static uint64_t g_tsc_monotonic = 0;   /* TSC origin for CLOCK_MONOTONIC */

 /* ==================== Helper functions ==================== */

 /* Read the current TSC value. Returns 0 on failure. */
 static inline uint64_t read_tsc(void) {
	 CPU_INFO ci;
	 ci.cbSize = sizeof(ci);
	 if (!GetCPUInfo(&ci)) return 0;
	 return (uint64_t)ci.cpuTSC;
 }

 /*
  * Convert a TSC delta (in ticks) to nanoseconds.
  *
  * Splits into integer and fractional parts to avoid 128-bit math:
  *   frac < freq, so frac * 1e9 < freq * 1e9, safe for any freq < 18.4 GHz.
  */
 static inline uint64_t tsc_delta_to_ns(uint64_t delta) {
	 uint64_t sec = delta / g_freq_hz;
	 uint64_t frac = delta % g_freq_hz;
	 uint64_t nsec = (frac * 1000000000ULL) / g_freq_hz;
	 return sec * NS_PER_SEC + nsec;
 }

 /*
  * Convert nanoseconds to a TSC tick count.
  *
  * Same technique: nsec < 1e9, so nsec * freq < 1e9 * freq.
  */
 static inline uint64_t ns_to_ticks(uint64_t ns) {
	 uint64_t sec = ns / NS_PER_SEC;
	 uint64_t nsec = ns % NS_PER_SEC;
	 return sec * g_freq_hz + (nsec * g_freq_hz) / NS_PER_SEC;
 }

 /* Convert Y-M-D h:m:s to a UNIX timestamp (UTC), days_from_civil algorithm. */
 static time_t ymdhms_to_unix(int y, int m, int d, int hh, int mm, int ss) {
	 y -= (m <= 2);
	 int era = (y >= 0 ? y : y - 399) / 400;
	 unsigned yoe = (unsigned)(y - era * 400);
	 unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2u) / 5u + d - 1u;
	 unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
	 int64_t days = (int64_t)era * 146097 + (int64_t)doe - 719468;
	 return (time_t)(days * 86400LL + hh * 3600 + mm * 60 + ss);
 }

 /* Convert the 6-byte time_info array to a UNIX timestamp. */
 static time_t time_info_to_unix(const unsigned char *ti) {
	 return ymdhms_to_unix(2000 + ti[0], ti[1], ti[2], ti[3], ti[4], ti[5]);
 }

 /*
  * Busy-wait until the TSC delta from entry reaches target_ticks.
  * Uses the x86 PAUSE hint where available to reduce power and bus contention.
  */
 static void busy_wait_ticks(uint64_t target_ticks) {
	 uint64_t start = read_tsc();
	 if (start == 0) return;   /* API failure: give up rather than spin forever */

	 for (;;) {
		 uint64_t now = read_tsc();
		 if (now == 0) return;
		 if (now - start >= target_ticks) return;

#if defined(__i386__) || defined(__x86_64__)
		 __asm__ __volatile__("pause");
#endif
	 }
 }

 /* ==================== Initialization ==================== */

 /*
  * Lazily initialize the clock baseline on first use. Captures the second
  * boundary from GetSystemTime() and snapshots the TSC at that instant.
  */
 static int ensure_init(void) {
	 if (g_freq_hz != 0) return 0;

	 CPU_INFO ci;
	 ci.cbSize = sizeof(ci);
	 if (!GetCPUInfo(&ci))  return -1;
	 if (ci.cpuFreq <= 0)   return -1;
	 g_freq_hz = (uint64_t)ci.cpuFreq;

	 /* Poll until the second changes, to capture a second boundary. */
	 unsigned char t0[6], t1[6];
	 GetSystemTime(t0);
	 do {
		 GetSystemTime(t1);
	 } while (memcmp(t0, t1, 6) == 0);

	 /* The second just changed; snapshot the TSC as the baseline. */
	 ci.cbSize = sizeof(ci);
	 if (!GetCPUInfo(&ci))  return -1;

	 g_tsc_realtime = (uint64_t)ci.cpuTSC;
	 g_sec_realtime = time_info_to_unix(t1);
	 g_tsc_monotonic = g_tsc_realtime;   /* Monotonic origin is the same instant */

	 return 0;
 }

 /* ==================== clock_gettime ==================== */

 int clock_gettime(clockid_t clk_id, struct timespec *tp) {
	 if (!tp) { errno = EFAULT; return -1; }
	 if (ensure_init() != 0) { errno = EINVAL; return -1; }

	 uint64_t now = read_tsc();
	 if (now == 0) { errno = EFAULT; return -1; }

	 uint64_t sec, nsec;

	 switch (clk_id) {
	 case CLOCK_REALTIME: {
		 uint64_t delta = now - g_tsc_realtime;   /* Unsigned subtraction handles wrap-around */
		 uint64_t total_ns = tsc_delta_to_ns(delta);
		 sec = total_ns / NS_PER_SEC;
		 nsec = total_ns % NS_PER_SEC;
		 tp->tv_sec = g_sec_realtime + (time_t)sec;
		 tp->tv_nsec = (long)nsec;
		 return 0;
	 }
	 case CLOCK_MONOTONIC: {
		 uint64_t delta = now - g_tsc_monotonic;
		 uint64_t total_ns = tsc_delta_to_ns(delta);
		 sec = total_ns / NS_PER_SEC;
		 nsec = total_ns % NS_PER_SEC;
		 tp->tv_sec = (time_t)sec;
		 tp->tv_nsec = (long)nsec;
		 return 0;
	 }
	 default:
		 errno = EINVAL;
		 return -1;
	 }
 }

 /* ==================== nanosleep / usleep / sleep ==================== */

 int nanosleep(const struct timespec *req, struct timespec *rem) {
	 if (!req) { errno = EFAULT; return -1; }
	 if (req->tv_sec < 0 ||
		 req->tv_nsec < 0 ||
		 req->tv_nsec >= (long)NS_PER_SEC) {
		 errno = EINVAL;
		 return -1;
	 }

	 if (ensure_init() != 0) { errno = EINVAL; return -1; }

	 uint64_t total_ns = (uint64_t)req->tv_sec * NS_PER_SEC
		 + (uint64_t)req->tv_nsec;
	 if (total_ns == 0) {
		 if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
		 return 0;
	 }

	 uint64_t start_tsc = read_tsc();
	 if (start_tsc == 0) { errno = EFAULT; return -1; }

	 uint64_t target_ticks = ns_to_ticks(total_ns);
	 if (target_ticks == 0) {
		 /* Delay is shorter than one TSC tick; treat as a no-op. */
		 if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
		 return 0;
	 }

	 for (;;) {
		 uint64_t now = read_tsc();
		 if (now == 0) { errno = EFAULT; return -1; }

		 uint64_t elapsed_ticks = now - start_tsc;
		 if (elapsed_ticks >= target_ticks) break;

		 uint64_t remaining_ticks = target_ticks - elapsed_ticks;
		 uint64_t remaining_ns = tsc_delta_to_ns(remaining_ticks);

		 if (remaining_ns > KERNEL_GRAN_NS + SAFETY_NS) {
			 /*
			  * Coarse phase:
			  *   sleep_ns = remaining_ns - SAFETY_NS
			  *   sleep_ms = sleep_ns / 1 ms, rounded DOWN to a multiple of 5
			  * The safety margin keeps Sleep() from overshooting the deadline;
			  * the busy-wait phase then lands on the exact target.
			  */
			 uint64_t sleep_ns = remaining_ns - SAFETY_NS;
			 DWORD    sleep_ms = (DWORD)(sleep_ns / NS_PER_MS);
			 sleep_ms = (sleep_ms / 5u) * 5u;

			 if (sleep_ms >= 5u) {
				 Sleep(sleep_ms);
				 continue;   /* Re-evaluate the remaining time. */
			 }
		 }

		 /*
		  * Fine phase:
		  *   Less than (5 ms + safety margin) remains. Busy-wait the rest.
		  */
		 busy_wait_ticks(remaining_ticks);
		 break;
	 }

	 if (rem) { rem->tv_sec = 0; rem->tv_nsec = 0; }
	 return 0;
 }

 int usleep(unsigned int usec) {
	 struct timespec req;
	 req.tv_sec = (time_t)(usec / 1000000u);
	 req.tv_nsec = (long)(usec % 1000000u) * 1000L;
	 return nanosleep(&req, NULL);
 }

 unsigned int secondsleep(unsigned int seconds) {
	 struct timespec req;

	 req.tv_sec = (time_t)seconds;
	 req.tv_nsec = 0;

	 if (nanosleep(&req, NULL) != 0) {
		 /* Report the entire request as not slept on failure. */
		 return seconds;
	 }

	 return 0;
 }

 /* /////// AI GENERATED CODE END //////////////////////// */

#if 0
 /* 
  * clock_gettime routine. 
  * Just simulates Linux and wraps the
  * clock function, should be optimized
  * later to comply POSIX standard.
  */
 int clock_gettime(int clk_id, struct timespec *tp)
 {
	 clock_t curr_clock = 0;

	 if (NULL == tp)
	 {
		 return -EINVAL;
	 }
	 curr_clock = clock();

	 /* 
	  * curr_clock is in millionsecond, convert
	  * it into timespec.
	  */
	 tp->tv_sec = curr_clock / 1000;
	 tp->tv_nsec = (curr_clock % 1000) * 1000000;

	 return 0;
 }
#endif

 int gettimeofday(struct timeval *tp, struct timezone *tzp)
 {
	 //Just return the million seconds,it's a simplified implementation.
	 if (tp)
	 {
		 time_t t = time(NULL);
		 tp->tv_sec = (long)t;
		 tp->tv_usec = 0;
	 }
	 if (tzp)
	 {
		 tzp->tz_dsttime = 0;
		 tzp->tz_minuteswest = 0;
	 }
	 return 0;
 }
