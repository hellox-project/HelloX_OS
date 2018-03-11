//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 11,2017
//    Module Name               : ide.h
//    Module Funciton           : 
//                                IDE related macros and definitions are
//                                declared in this file.
//                                All codes in HelloX C Library will refer
//                                this file,it's purpose is to simplify the
//                                migration of source code in different IDE	,
//                                since different IDE uses personalized syntax,
//                                such as alignment,function parameter description
//                                and other aspect.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __IDE_H__
#define __IDE_H__

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * This macro is applied for Microsoft Visual studio,the
	 * default definition of all HelloX source code.
	 * Please comment the following macro and enable other definitions
	 * in case of port to other IDE accordingly.
	 */
#define __MS_VC__

	/**
	 * Enable this macro if you use GCC.
	 */
	//#define __GNU_GCC__

	/**
	 * Other IDE's can be defined here.
	 */

	/**
	 * Ignore inline keyword if in standard C and Visual Studio IDE
	 * since it can not support the keyword.
	 */
#ifndef __cplusplus
#if defined(__MS_VC__)
#define inline
#endif //__MS_VC__
#endif //__cplusplus.

	/**
	 * Define a alias for __FUNCTION__ macro
	 */
#ifdef __MS_VC__
#define __func__ __FUNCTION__
#endif //__MS_VC__

	/**
	 * Alignment attribute for variables or values.
	 */
#if  defined(__MS_VC__)
#define __HXCL_ALIGN(x) __declspec(align(x))
#elif defined(__GNU_GCC__)
#define __HXCL_ALIGN(x) __attribute__ ((aligned(x)))
	//Append other IDE's corresponding premitives here.

#endif

	/**
	 * Declare a memory aligned variable or object.
	 * The premitive position of different IDEs in declaration is
	 * different,we define this marco to make them same.
	 */
#if defined(__MS_VC__)
#define __HXCL_DEFINE_ALIGNED_OBJECT(type,name,x) \
	__declspec(align(x)) type name;
#elif defined(__GNU_GCC__)
	type name __attribute__ ((aligned(x)))
#endif //__HXCL_DEFINE_ALIGNED_OBJECT

#ifdef __cplusplus
	}
#endif

#endif //__IDE_H__
