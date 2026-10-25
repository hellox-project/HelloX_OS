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
	 * Ignore inline keyword if in standard C and Visual Studio IDE
	 * since it can not support the keyword.
	 */
#ifndef __cplusplus
#if defined(_MSC_VER)
#define inline
#endif //_MSC_VER
#endif //__cplusplus.

	/**
	 * Define a alias for __FUNCTION__ macro
	 */
#ifdef _MSC_VER
#define __func__ __FUNCTION__
#elif (defined(__GNUC__) || defined(__clang__))
#define __func__ __FUNCTION__
#endif //_MSC_VER

	/**
	 * Alignment attribute for variables or values.
	 */
#if  defined(_MSC_VER)
#define __HXCL_ALIGN(x) __declspec(align(x))
#elif (defined(__GNUC__) || defined(__clang__))
#define __HXCL_ALIGN(x) __attribute__ ((aligned(x)))
	//Append other IDE's corresponding premitives here.

#endif

	/*
	 * Naked function declaration both for MSVC and
	 * for clang and GNUC.
	 */
#if defined(_MSC_VER)
#define HX_NAKED __declspec(naked)
#elif defined(__clang__) || defined(__GNUC__)
#define HX_NAKED __attribute__((naked))
#else
#define HX_NAKED
#endif

	/*
	 * Function protype definition for cdecl.
	 */
#if defined(_MSC_VER)
#define __CDECL __cdecl
#elif defined(__clang__) || defined(__GNUC__)
#define __CDECL
#else
#define __CDECL
#endif

	/**
	 * Declare a memory aligned variable or object.
	 * The premitive position of different IDEs in declaration is
	 * different,we define this marco to make them same.
	 */
#if defined(_MSC_VER)
#define __HXCL_DEFINE_ALIGNED_OBJECT(type,name,x) \
	__declspec(align(x)) type name;
#elif (defined(__GNUC__) || defined(__clang__))
#define __HXCL_DEFINE_ALIGNED_OBJECT(type,name,x) \
	type name __attribute__ ((aligned(x)))
#endif //__HXCL_DEFINE_ALIGNED_OBJECT

#ifdef __cplusplus
	}
#endif

#endif //__IDE_H__
