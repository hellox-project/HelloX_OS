//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : cc.h
//    Module Funciton           : 
//                                cc.h for lwIP stack,internal used data types and
//                                some macros are defined in this file.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

//Little endian.
#define BYTE_ORDER LITTLE_ENDIAN

//Data types for lwIP.
typedef unsigned char  u8_t;
typedef signed   char  s8_t;
typedef unsigned short u16_t;
typedef signed   short s16_t;
typedef unsigned long  u32_t;
typedef signed   long  s32_t;

typedef unsigned long mem_ptr_t;

//Structure packing method,use including.
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_USE_INCLUDES

//For diagnostic output.
#define LWIP_PLATFORM_ASSERT(x) { \
    PrintLine(x); \
	BUG(); \
	}while(0);

//Structure pack definition.
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

//Use lwIP provided error number.
#define LWIP_PROVIDE_ERRNO
