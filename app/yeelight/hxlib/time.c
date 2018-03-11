
#include "hellox.h"
#include "stddef.h"
#include "time.h"
#include "time_pri.h"

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
 		if ( (tbtemp = _localtime(&tmptm1)) == NULL )
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
 				tbtemp = _localtime(&tmptm1);    /* reconvert, can't get NULL */
 		}
 
 	}
 	else 
 	{
 		if ( (tbtemp = _gmtime(&tmptm1)) == NULL )
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

time_t  _time (time_t* tptr)
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

time_t    _mktime (struct tm* p)
{
 	return( _make_time_t(p, 1) );
}

struct tm* _gmtime (const time_t* timp)
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

	// 64除法需要 用的crt库
	ptb->tm_wday = ((int)(*timp / _DAY_SEC ) + _BASE_DOW) % 7;

	
	ptb->tm_hour = (int)(caltim / 3600);
	caltim -= (long)ptb->tm_hour * 3600L;

	ptb->tm_min = (int)(caltim / 60);
	ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

	ptb->tm_isdst = 0;

	return (struct tm *)ptb;

	//return NULL;
}

struct tm* _localtime (const time_t* ptime)
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
		ptm = _gmtime( (time_t *)&ltime );


		if ( _daylight ) 
		{
			ltime -= _dstbias;
			ptm = _gmtime( (time_t *)&ltime );
			ptm->tm_isdst = 1;
		}
	}
	else {
		ptm = _gmtime( ptime );            

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

   time_t   _difftime (time_t b, time_t a)
   {
   	return( ( b - a ) ); 
   }
   
   static char *  store_dt (char *p, int val)
   {
   	*p++ = ('0' + val / 10);
   	*p++ = ('0' + val % 10);
   
   	return p;
   }
 
  
  char*  _asctime (const struct tm* tb)
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

 char* _ctime (const time_t* timp)
 {
 	struct tm *tmtemp;
 
	tmtemp =_localtime(timp); 
 	if( tmtemp != NULL)
 	{
 		return(_asctime((const struct tm *)tmtemp));
 	}
 	else
 	{
 		return NULL;
	}
 }

 size_t _strftime (char* s, size_t size , const char* fmt, const struct tm* tp)
 {
 	return 0;
 }

 clock_t clock (void)
{
	//return _time(NULL);
	return 0;
}

/*
char* _strdate(char*);
char*   _strtime(char*);
*/

 int gettimeofday(struct timeval *tp, struct timezone *tzp)
 {
	 //Just return the million seconds,it's a simplified implementation.
	 if (tp)
	 {
		 time_t t = _time(NULL);
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
