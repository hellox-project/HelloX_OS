//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : PERF.H
//    Module Funciton           : 
//                                This module countains the performance mesuring mechanism's 
//                                definition.
//                                The performance mesuring mechanism is used to mesure one
//                                segment of code's performance,in CPU clock circle.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PERF_H__
#define __PERF_H__

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

//
//The definition of __PERF_RECORDER object.
//This object is used to contain CPU clock circle.
//
typedef struct{
	__U64     u64Start;
	__U64     u64End;
	__U64     u64Result;
	__U64     u64Max;
} __PERF_RECORDER;

//
//The following routines are used to process the performance recording operations.
//

//
//This routine is ued to start record the current CPU clock circle.
//Before to execute the tested code segment,programmer should call this routine 
//to record the current CPU clock circle.
//
VOID PerfBeginRecord(__PERF_RECORDER* lpPr);

//
//This routine is used to stop record the CPU's clock circle counter.
//When the tested code executed over,this routine must be called to record the
//CPU's clock circle currently.
//
VOID PerfEndRecord(__PERF_RECORDER* lpPr);

//
//This routine is used to calculate the actually clock circle consumed by
//the tested code.
//It stores the result into u64Result.
//
VOID PerfCommit(__PERF_RECORDER* lpPr);

#ifdef __cplusplus
}
#endif

#endif  //End of PERF.H
