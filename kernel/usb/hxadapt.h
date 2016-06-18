//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 28, 2015
//    Module Name               : hxadapt.h
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
//#define __MS_VC__

//#define DEBUG  //Enable debugging.

//Enable or disable interrupt mode of EHCI controller.
//#define USB_EHCI_DISABLE_INTERRUPT

#ifdef __CFG_CPU_LE
#define __LITTLE_ENDIAN
#else
#define __BIG_ENDIAN
#endif

//Default DMA alignment.
#ifndef ARCH_DMA_MINALIGN
#define ARCH_DMA_MINALIGN 64
#endif

#ifndef CONFIG_SYS_HZ
#define CONFIG_SYS_HZ (1000 / SYSTEM_TIME_SLICE)
#endif

typedef ULONG ulong;
typedef unsigned long lbaint_t;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef __U32 u32;
typedef __U16 u16;
typedef __U8  u8;
typedef uint64_t u64;
typedef __S32 s32;
typedef __S16 s16;
typedef __S8  s8;

/* Macros for printing `intptr_t' and `uintptr_t'.  */
# define PRIdPTR	__PRIPTR_PREFIX "d"
# define PRIiPTR	__PRIPTR_PREFIX "i"
# define PRIoPTR	__PRIPTR_PREFIX "o"
# define PRIuPTR	__PRIPTR_PREFIX "u"
# define PRIxPTR	__PRIPTR_PREFIX "x"
# define PRIXPTR	__PRIPTR_PREFIX "X"

#define LBAF "%lx"
#define LBAFU "%lu"

typedef __U16 __le16;
typedef __U32 __le32;
typedef uint64_t __le64;

//Get part of 64 bits integer.
#define lower_32_bits(val) ((u32)val)
#define upper_32_bits(val) ((u32)(val >> 32))

#define __le16_to_cpu(x) (x)
#define __le32_to_cpu(x) (x)

#define __cpu_to_le64s(x) do { (void)(x); } while (0)
#define __le64_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_le32s(x) do { (void)(x); } while (0)
#define __le32_to_cpus(x) do { (void)(x); } while (0)
#define __cpu_to_le16s(x) do { (void)(x); } while (0)
#define __le16_to_cpus(x) do { (void)(x); } while (0)

#define get_unaligned(x) (*x)
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
	__MicroDelay(1000 * msec);
}

static void inline udelay(int usec)
{
	__MicroDelay(usec);
}

//Extern routines,not public to all modules.
extern uint64_t __GetCPUFrequency();
extern uint64_t __GetCPUTSC();

//Simulation of get_timer routine.
static ulong inline get_timer(ulong base)
{
	ulong timer = 0;
	uint64_t cpuFreq = 0;
	uint64_t tsc = 0;

	//If in system initialization process,the tick counter is not updated since all
	//interrupts are disabled,we must simulate the tick counter by CPU freq and TSC.
	if (IN_SYSINITIALIZATION())
	{
		cpuFreq = __GetCPUFrequency();
		tsc = __GetCPUTSC();
		cpuFreq = (cpuFreq / 1000) * SYSTEM_TIME_SLICE; //How many clock cycle in one time slice.
		tsc = tsc / cpuFreq; //How many ticks since boot.
		timer = (ulong)tsc;
		return (timer - base);
	}
	//After initialization,just return the clock time counter.
	//A risk exist here: If the caller calls get_timer in system initialization phase,as the
	//timer base,and then call get_timer again after system initialization phase but use the
	//previous base,issue may raise.
	//But in current implementation we can avoid this scenario by make sure all initialization
	//tasks are finished in init phase,so just let it go...
	//Sleep a system tick to avoid busying await.It's a balanced mechanism in current phase,
	//since USB INTERRUPT mechanism is in process of implementing...
	Sleep(SYSTEM_TIME_SLICE);
	return System.GetClockTickCounter((__COMMON_OBJECT*)&System) - base;
}

//Watch dog operations.
#define WATCHDOG_RESET()

//Cache operations,do nothing for invalidate cache range in x86
//architecture since cache snooping works.
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

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define max3(x, y, z) max(max(x, y), z)
#endif

//isprint simulation
#define isprint(c) (((c) >= 0x20) && ((c) <= 0x7E))

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
		 ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
		 ((x & 0xffff0000) ? 16 : 0))

//Debugging control.
#define BUG_ON(cond) \
	do { \
	if(cond) \
	  BUG(); \
				}while(0)

//A dead loop debug to check if a routine is called.
#define __DEAD_DEBUG() do { \
	_hx_printf("Routine [%s] is called.\r\n",__func__); \
							while(TRUE); \
			}while(0)

//Round up.
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#endif  //__HXADAPT_H__
