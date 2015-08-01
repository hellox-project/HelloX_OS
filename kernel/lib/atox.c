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

