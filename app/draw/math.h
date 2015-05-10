//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 3,2011
//    Module Name               : math.h
//    Module Funciton           : 
//                                Header files for standard C library's mathematic operations.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MATH_H__
#define __MATH_H__

//Maximal and minimal value of double.
#define DBL_MAX	1.7976931348623158e+308
#define DBL_MIN	2.2250738585072014e-308

//Pi's value.
#define PI 3.14159265

//Calculate a floating point number's consine value.
double cos(double x);

//Calculate a floating point number's sinine value.
double sin(double x);

//Floating point mod operation.
double modf(double x,double* y);

#endif