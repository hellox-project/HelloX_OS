//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 28, 2015
//    Module Name               : config.h
//    Module Funciton           : 
//                                Definitions or conversations to fit USB porting
//                                are put here.
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __HXADAPT_H__
#define __HXADAPT_H__

//Use Microsoft Visual Studio as compiler.Just use "define __GCC__" replacing this
//if you use GCC as compiler.
#define __MS_VC__

//#define DEBUG  //Enable debugging.

#ifdef __CFG_CPU_LE
#define __LITTLE_ENDIAN
#endif

//Default DMA alignment.
#ifndef ARCH_DMA_MINALIGN
#define ARCH_DMA_MINALIGN 32
#endif

#ifndef CONFIG_SYS_HZ
#define CONFIG_SYS_HZ 1000
#endif

typedef ULONG ulong;
typedef unsigned int uint;
typedef __U32 u32;
typedef __U16 u16;
typedef __U8  u8;
typedef __S32 s32;
typedef __S16 s16;
typedef __S8  s8;

typedef __U16 __le16;
typedef __U32 __le32;

#define __le16_to_cpu(x) (x)
#define __le32_to_cpu(x) (x)

#define __cpu_to_le64s(x) do { (void)(x); } while (0)
#define __le64_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_le32s(x) do { (void)(x); } while (0)
#define __le32_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_le16s(x) do { (void)(x); } while (0)
#define __le16_to_cpus(x) do { (void)(x); } while (0)

#define get_unaligned(x) (*x) //__GET_UNALIGNED(x)
#define put_unaligned(val,addr) (*addr = val)

#ifdef __MS_VC__
typedef int bool;
#define false 0
#define true  1
#endif

#ifdef __MS_VC__
#define inline __inline
#define __func__ __FUNCTION__
#endif

//Macros to control the debugging output.
#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#ifndef serial_printf
#define serial_printf _hx_printf
#endif

/*
* Output a debug text when condition "cond" is met. The "cond" should be
* computed by a preprocessor in the best case, allowing for the best
* optimization.
*/
#ifdef __MS_VC__
#define debug_cond(cond, fmt, ...)			\
	do {						\
		if (cond)				\
			printf(pr_fmt(fmt), __VA_ARGS__);	\
							} while (0)

#define debug(fmt, ...)			\
	do { \
	if(_DEBUG) \
		{  \
	_hx_printf("Function name = %s,line# = %d.\r\n",__func__,__LINE__); \
		} \
	debug_cond(_DEBUG, fmt, __VA_ARGS__); \
	}while(0)
#else
#define debug_cond(cond, fmt, args...)			\
	do {						\
		if (cond)				\
			printf(pr_fmt(fmt), ##args);	\
		} while (0)

#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)
#endif

#define puts _hx_printf

//mdely's simulation.
static void inline mdelay(int msec)
{
	__MicroDelay(msec);
}

static void inline udelay(int usec)
{
	__MicroDelay(usec);
}

//Simulation of get_timer routine.
static ulong inline get_timer(ulong base)
{
	return System.GetClockTickCounter((__COMMON_OBJECT*)&System) - base;
}

//Watch dog operations.
#define WATCHDOG_RESET()

//Cache operations.
#define invalidate_dcache_range(start,end) __FLUSH_CACHE(start,(end - start),CACHE_FLUSH_INVALIDATE)
#define flush_dcache_range(start,end)      __FLUSH_CACHE(start,(end - start),CACHE_FLUSH_WRITEBACK)

//Alignment and round.
#define ALIGN __ALIGN
#define roundup __ROUND_UP
#define rounddown __ROUND_DOWN

//Maximal and minimal.
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define min3(x, y, z) min(min(x, y), z)
#endif

//isprint simulation
#define isprint(c) (((c) >= 0x20) && ((c) <= 0x7E))

#endif  //__HXADAPT_H__
