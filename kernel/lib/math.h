//Only implements fmod routine,which is refered by interpreter of JVM.

#ifndef __MATH_H__
#define __MATH_H__

#include <config.h>

#if defined(__CFG_CPU_LE)
#define __LITTLE_ENDIAN
#endif

#ifdef __LITTLE_ENDIAN
#define __HI(x) *(1 + (int *) &x)
#define __LO(x) *(int *) &x
#else /* !__LITTLE_ENDIAN */
#define __HI(x) *(int *) &x
#define __LO(x) *(1 + (int *) &x)
#endif /* __LITTLE_ENDIAN */

//Calculate the remainder of x/y.
double fmod(double x,double y);

//Get the floor of x.
double floor(double x);

double fabs(double x);

#endif //__MATH_H__
