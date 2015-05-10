//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 3,2011
//    Module Name               : math.cpp
//    Module Funciton           : 
//                                Implementation files for standard C library's mathematic operations.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "math.h"

//Calculate x's sinine value.
double sin(double x)
{
	__asm{
		fld x
		fsin
	}
}

//Calculate x's cosine value.
double cos(double x)
{
	__asm{
		fld x
		fcos
	}
}

//Two helper routines used by modf,a structure contains
//2 long intergers is adopted here to simulate 64 bits
//integer.
static int isnan(double d)
{
	int result = 0;
    union 
    {
		struct{
			unsigned long l1;
			unsigned long l2;
		} l;
       double d;
    } u;
    u.d=d;
	if((u.l.l1 == 0) && (u.l.l2 == 0x7FF80000))
	{
		result = 1;
	}
	if((u.l.l1 == 0) && (u.l.l2 == 0x7FF00000))
	{
		result = 1;
	}
	if((u.l.l1 == 0) && (u.l.l2 == 0xFFF80000))
	{
		result = 1;
	}
	return result;
}

static int isinf(double d) 
{
    union 
    {
		struct{
			unsigned long l1;
			unsigned long l2;
		}l;
       double d;
    } u;
    u.d=d;
	if((u.l.l1 == 0) && (u.l.l2 == 0x7FF00000))
	{
		return 1;
	}
	if((u.l.l1 == 0) && (u.l.l2 == 0xFFF00000))
	{
		return -1;
	}
	return 0;
}

typedef struct 
{
    unsigned int mantissal:32;
    unsigned int mantissah:20;
    unsigned int exponent:11;
    unsigned int sign:1;
}double_t; //This structure definied in IEEEE.H.

//Floating point mod operation,integer part of x will
//be returned in *y and mantissa part will be returned
//directly.
double modf(double x, double *y)
{
    double_t * z = (double_t *)&x;
    double_t * iptr = (double_t *)y;

    int j0;
    unsigned int i;
    j0 = z->exponent - 0x3ff;   /* exponent of x */  
    if(j0<20)
    {/* integer part in high x */
       if(j0<0) 
       {                   /* |x|<1 */
         *y = 0.0;
         iptr->sign = z->sign;
         return x;
       } 
       else 
       {
         if ( z->mantissah == 0 && z->mantissal == 0 ) 
         {
            *y = x;
            return 0.0;
         }
         i = (0x000fffff)>>j0;
         iptr->sign = z->sign;
         iptr->exponent = z->exponent;
         iptr->mantissah = z->mantissah&(~i);
         iptr->mantissal = 0;
         if ( x == *y ) 
         {
            x = 0.0;
            z->sign = iptr->sign;
            return x;
         }          
               return x - *y;        
       }
    } 
    else if (j0>51) 
    {              /* no fraction part */
       *y = x; 
       if ( isnan(x) || isinf(x) )
         return x;
       x = 0.0;
       z->sign = iptr->sign;
       return x;
    } 
    else 
    {                         /* fraction part in low x */
       i = ((unsigned)(0xffffffff))>>(j0-20);
       iptr->sign = z->sign;
       iptr->exponent = z->exponent;
       iptr->mantissah = z->mantissah;
       iptr->mantissal = z->mantissal&(~i);
       if ( x == *y ) 
       {
         x = 0.0;
         z->sign = iptr->sign;
         return x;
       }
       return x - *y;        
    }
}
