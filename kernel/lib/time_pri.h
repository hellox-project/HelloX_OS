
#ifndef	_TIME_PRI_H_
#define	_TIME_PRI_H_


#define MB_LEN_MAX    2             /* max. # bytes in multibyte char */
#define SHRT_MIN    (-32768)        /* minimum (signed) short value */
#define SHRT_MAX      32767         /* maximum (signed) short value */
#define USHRT_MAX     0xffff        /* maximum unsigned short value */
#define INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
#define INT_MAX       2147483647    /* maximum (signed) int value */
#define UINT_MAX      0xffffffff    /* maximum unsigned int value */
#define LONG_MIN    (-2147483647L - 1) /* minimum (signed) long value */
#define LONG_MAX      2147483647L   /* maximum (signed) long value */
#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */


#define _ASCBUFSIZE 26
/*
 * ChkAdd evaluates to TRUE if dest = src1 + src2 has overflowed
 */
#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L)  && (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )

/*
 * ChkMul evaluates to TRUE if dest = src1 * src2 has overflowed
 */
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )


#define  _DAY_SEC          86400                  /* secs in a day */ //(24 * 60 * 60) 

#define _YEAR_SEC          31536000//(365 * _DAY_SEC)    /* secs in a year */

#define _FOUR_YEAR_SEC     126230400//(1461 * _DAY_SEC)   /* secs in a 4 year interval */

#define _DEC_SEC           315532800           /* secs in 1970-1979 */

#define _BASE_YEAR         70L                  /* 1970 is the base year */

#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */

#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */

#define _MAX_YEAR          138L                 /* 2038 is the max year */


int _lpdays[] = {
	-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

int _days[] = {
	-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

long _timezone   = 8 * 3600L; /* Pacific Time Zone */
int _daylight    = 1;          /* Daylight Saving Time (DST) in timezone */
long _dstbias    = -3600L;     /* DST offset in seconds */

static struct tm tb = { 0 };    /* time block */

const char __dnames[] = 
{
	"SunMonTueWedThuFriSat"
};

/*  Month names must be Three character abbreviations strung together */

const char __mnames[] = 
{
	"JanFebMarAprMayJunJulAugSepOctNovDec"
};

static char buf[_ASCBUFSIZE];

#endif	/* _TIME_PRI_H_*/
