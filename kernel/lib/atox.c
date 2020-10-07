//***********************************************************************
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : atox.c
//    Module Funciton           : 
//                                Stdand C library simulation code.It only implements a subset
//                                of C library,even much more simple.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************

/************************************************************************/
//
//    THE FOLLOWING CODE IS COPIED FROM MICROSOFT'S IMPLEMENTATION,PLEASE
//    REPLACE IT WHEN YOUR PURPUSE IS MERCHANT.
//
/************************************************************************/

/************************************************************************
*atox.c - atoi and atol conversion 
* 
* Copyright (c) 1989-1997, Microsoft Corporation. All rights reserved. 
* 
*Purpose: 
* Converts a character string into an int or long. 
* 
*************************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#include "stddef.h"
#include "stdlib.h"
  
/*** 
*long atol(char *nptr) - Convert string to long 
* 
*Purpose: 
* Converts ASCII string pointed to by nptr to binary. 
* Overflow is not detected. 
* 
*Entry: 
* nptr = ptr to string to convert 
* 
*Exit: 
* return long int value of the string 
* 
*Exceptions: 
* None - overflow is not detected. 
* 
*******************************************************************************/  

static int isspace(int x)    
{    
    if(x==' '||x=='\t'||x=='\n'||x=='\f'||x=='\b'||x=='\r')    
        return 1;    
    else     
        return 0;    
}    
    
static int isdigit(int x)    
{    
    if(x<='9'&&x>='0')             
        return 1;     
    else     
        return 0;    
}
  
long atol(const char *nptr)
{
	int c; /* current char */  
	long total; /* current total */  
	int sign; /* if ''-'', then negative, otherwise positive */  

	/* skip whitespace */
	while (isspace((int)(unsigned char)*nptr))
	{
		++nptr;
	}
	c = (int)(unsigned char)*nptr++;  
	sign = c; /* save sign indication */  
	if (c == '-' || c == '+')  
	{
		c = (int)(unsigned char)*nptr++; /* skip sign */  
	}
	total = 0;  
	
	while (isdigit(c)) {
		total = 10 * total + (c - '0'); /* accumulate digit */  
		c = (int)(unsigned char)*nptr++; /* get next char */  
	}
	
	if (sign == '-')
		return -total;  
	else  
		return total; /* return result, negated if necessary */  
}  

/***
*int atoi(char *nptr) - Convert string to long
*
*Purpose:
* Converts ASCII string pointed to by nptr to binary.
* Overflow is not detected. Because of this, we can just use
* atol().
*
*Entry:
* nptr = ptr to string to convert
*
*Exit:
* return int value of the string
*
*Exceptions:
* None - overflow is not detected.
*
*******************************************************************************/

int atoi(const char *nptr)
{
	return (int)atol(nptr);
}

char* itoa(int value, char* string, int radix)
{
	char tmp[33];
	char* tp = tmp;
	int i;
	unsigned v;
	int sign;
	char* sp;

	if (radix > 36 || radix <= 1)
	{
		//__set_errno(EDOM);
		return 0;
	}
	sign = (radix == 10 && value < 0);
	if (sign)
		v = -value;
	else
		v = (unsigned)value;
	while (v || tp == tmp)
	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
			*tp++ = i+'0';
		else
			*tp++ = i + 'a' - 10;
	}

	if (string == 0)
		string = (char*)_hx_malloc((tp-tmp)+sign+1);
	sp = string;

	if (sign)
		*sp++ = '-';
	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;
	return string;
}

static char * _getbase(char *, int *);
#ifdef HAVE_QUAD
static int _atob(unsigned long long *, char *p, int);
#else
static int _atob(unsigned long  *, char *, int);
#endif

static char* _getbase(char *p, int *basep)
{
	if (p[0] == '0') {
		switch (p[1]) {
		case 'x':
			*basep = 16;
			break;
		case 't': case 'n':
			*basep = 10;
			break;
		case 'o':
			*basep = 8;
			break;
		default:
			*basep = 10;
			return (p);
		}
		return (p + 2);
	}
	*basep = 10;
	return (p);
}


/*
 *  _atob(vp,p,base)
 */
static int
#ifdef HAVE_QUAD
_atob(u_quad_t *vp, char *p, int base)
{
	u_quad_t value, v1, v2;
#else
_atob(unsigned long *vp, char *p, int base)
{
	unsigned long value, v1, v2;
#endif
	char *q, tmp[20];
	int digit;

	if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
		base = 16;
		p += 2;
	}

	if (base == 16 && (q = strchr(p, '.')) != 0) {
		if (q - p > sizeof(tmp) - 1)
			return (0);

		strncpy(tmp, p, q - p);
		tmp[q - p] = '\0';
		if (!_atob(&v1, tmp, 16))
			return (0);

		q++;
		if (strchr(q, '.'))
			return (0);

		if (!_atob(&v2, q, 16))
			return (0);
		*vp = (v1 << 16) + v2;
		return (1);
	}

	value = *vp = 0;
	for (; *p; p++) {
		if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else if (*p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else
			return (0);

		if (digit >= base)
			return (0);
		value *= base;
		value += digit;
	}
	*vp = value;
	return (1);
}

/*
 *  atob(vp,p,base)
 *      converts p to binary result in vp, rtn 1 on success
 */
int
atob(uint32_t *vp, char *p, int base)
{
#ifdef HAVE_QUAD
	u_quad_t v;
#else
	unsigned long  v;
#endif

	if (base == 0)
		p = _getbase(p, &base);
	if (_atob(&v, p, base)) {
		*vp = v;
		return (1);
	}
	return (0);
}


#ifdef HAVE_QUAD
/*
 *  llatob(vp,p,base)
 *      converts p to binary result in vp, rtn 1 on success
 */
int
llatob(u_quad_t *vp, char *p, int base)
{
	if (base == 0)
		p = _getbase(p, &base);
	return _atob(vp, p, base);
}
#endif


/*
 *  char *btoa(dst,value,base)
 *      converts value to ascii, result in dst
 */
char *
btoa(char *dst, unsigned int value, int base)
{
	char buf[34], digit;
	int i, j, rem, neg;

	if (value == 0) {
		dst[0] = '0';
		dst[1] = 0;
		return (dst);
	}

	neg = 0;
	if (base == -10) {
		base = 10;
		if (value & (1L << 31)) {
			value = (~value) + 1;
			neg = 1;
		}
	}

	for (i = 0; value != 0; i++) {
		rem = value % base;
		value /= base;
		if (rem >= 0 && rem <= 9)
			digit = rem + '0';
		else if (rem >= 10 && rem <= 36)
			digit = (rem - 10) + 'a';
		buf[i] = digit;
	}

	buf[i] = 0;
	if (neg)
		strcat(buf, "-");

	/* reverse the string */
	for (i = 0, j = strlen(buf) - 1; j >= 0; i++, j--)
		dst[i] = buf[j];
	dst[i] = 0;
	return (dst);
}

#ifdef HAVE_QUAD
/*
 *  char *btoa(dst,value,base)
 *      converts value to ascii, result in dst
 */
char *
llbtoa(char *dst, u_quad_t value, int base)
{
	char buf[66], digit;
	int i, j, rem, neg;

	if (value == 0) {
		dst[0] = '0';
		dst[1] = 0;
		return (dst);
	}

	neg = 0;
	if (base == -10) {
		base = 10;
		if (value & (1LL << 63)) {
			value = (~value) + 1;
			neg = 1;
		}
	}

	for (i = 0; value != 0; i++) {
		rem = value % base;
		value /= base;
		if (rem >= 0 && rem <= 9)
			digit = rem + '0';
		else if (rem >= 10 && rem <= 36)
			digit = (rem - 10) + 'a';
		buf[i] = digit;
	}

	buf[i] = 0;
	if (neg)
		strcat(buf, "-");

	/* reverse the string */
	for (i = 0, j = strlen(buf) - 1; j >= 0; i++, j--)
		dst[i] = buf[j];
	dst[i] = 0;
	return (dst);
}
#endif

/*
 *  gethex(vp,p,n)
 *      convert n hex digits from p to binary, result in vp,
 *      rtn 1 on success
 */
int
gethex(int32_t *vp, char *p, int n)
{
	unsigned long v;
	int digit;

	for (v = 0; n > 0; n--) {
		if (*p == 0)
			return (0);
		if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else if (*p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else
			return (0);

		v <<= 4;
		v |= digit;
		p++;
	}
	*vp = v;
	return (1);
}
