//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct,22 2006
//    Module Name               : STATCPU.H
//    Module Funciton           : 
//                                Countains CPU overload ratio statistics related
//                                data structures and routines.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STATCPU_H__
#define __STATCPU_H__


#include "types.h"
#include "ktmgr.h"
#include "kmemmgr.h"



#ifdef __cplusplus
extern "C" {
#endif

#define GET_KERNEL_MEMORY(size)   KMemAlloc(size,KMEM_SIZE_TYPE_ANY)
#define FREE_KERNEL_MEMORY(p)     KMemFree(p,KMEM_SIZE_TYPE_ANY,0)

#define MAX_STAT_PERIOD        300
#define PERIOD_TIME            1000
#define ONE_MINUTE_PERIOD      60

typedef struct tag__THREAD_STAT_OBJECT{
	struct tag__THREAD_STAT_OBJECT*     lpPrev;
	struct tag__THREAD_STAT_OBJECT*     lpNext;

	__U64                     TotalCpuCycle;
	__U64                     CurrPeriodCycle;
	__U64                     PreviousTsc;

	__KERNEL_THREAD_OBJECT*   lpKernelThread;

	WORD                      wQueueHdr;
	WORD                      wQueueTail;
	WORD                      RatioQueue[MAX_STAT_PERIOD];
	WORD                      wOneMinuteRatio;  //CPU ratio in last 1 minute.
	WORD                      wMaxStatRatio;    //CPU ratio in last maximal statistics
	                                            //period,such as 5 minutes.
	WORD                      wCurrPeriodRatio;
	WORD                      wReserved;
}__THREAD_STAT_OBJECT;

typedef struct tag__STAT_CPU_OBJECT{
	//__KERNEL_THREAD_OBJECT*   lpStatKernelThread;
	__U64                     PreviousTsc;       //Previous time stamp counter.
	__U64                     CurrPeriodCycle;   //CPU cycle conter in this stat period.
	__U64                     TotalCpuCycle;     //Total CPU cycle counter since
	                                             //system startup.
	__THREAD_STAT_OBJECT      IdleThreadStatObj;

	BOOL                     (*Initialize)(struct tag__STAT_CPU_OBJECT*);    //Initialize routine.
	__THREAD_STAT_OBJECT*    (*GetFirstThreadStatObj)(void);
	__THREAD_STAT_OBJECT*    (*GetNextThreadSTatObj)(__THREAD_STAT_OBJECT*);
	VOID                     (*DoStat)(void);
	VOID                     (*ShowStat)(void);
}__STAT_CPU_OBJECT;

extern __THREAD_HOOK_ROUTINE lpCreateHook;
extern __THREAD_HOOK_ROUTINE lpBeginScheduleHook;
extern __THREAD_HOOK_ROUTINE lpEndScheduleHook;
extern __THREAD_HOOK_ROUTINE lpTerminalHook;

extern __STAT_CPU_OBJECT  StatCpuObject;

#ifdef __cplusplus
}
#endif

#endif  //__STATCPU_H__
