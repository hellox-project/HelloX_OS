//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,16 2005
//    Module Name               : PERF.CPP
//    Module Funciton           : 
//                                This module countains the performance mesuring mechanism's 
//                                implementation.
//                                The performance mesuring mechanism is used to mesure one
//                                segment of code's performance,in CPU clock circle.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif
#include "commobj.h"
#include "perf.h"
//
//The implementation of PerfBeginRecord routine.
//This routine records the current CPU clock circle counter into u64Start member of
//lpPr object.
//

VOID PerfBeginRecord(__PERF_RECORDER* lpPr)
{
	__U64*           lpStart    = NULL;

	if(NULL == lpPr)    //Parameter check.
	{
		return;
	}
	lpStart = &lpPr->u64Start;

#ifdef __I386__
#ifdef __GCC__
	__asm__ (
			"push %%eax \n\t"
			"push %%ebx \n\t"
			"push %%edx \n\t"
			"movl %0, %%ebx	\n\t"
			"rdtsc	\n\t"
			"movl %%eax, (%%ebx)	\n\t"
			"movl %%edx, 4(%%ebx)	\n\t"
			"popl %%edx	\n\t"
			"popl %%ebx	\n\t"
			"popl %%eax	\n\t"
			::"r"(lpStart));
#else
	__asm{
		push eax
        push ebx
        push edx
        mov ebx,lpStart
        rdtsc                       //Read time stamp counter.
        mov dword ptr [ebx],eax     //Save low part
        mov dword ptr [ebx + 4],edx //Save high part.
        pop edx
        pop ebx
        pop eax
	}
#endif
#else
	(void)lpStart;  //Avoid warning generation under some compiler.
#endif
}

//
//The implementation of PerfEndRecord routine.
//This routine record the current CPU clock circle counter into u64End member of
//lpPr object.
//

VOID PerfEndRecord(__PERF_RECORDER* lpPr)
{
	__U64*         lpEnd            = NULL;

	if(NULL == lpPr)  //Parameter check.
	{
		return;
	}
	lpEnd = &lpPr->u64End;

#ifdef __I386__
#ifdef __GCC__
	__asm__(
	"pushl %%eax  \n\t"
	"pushl %%ebx  \n\t"
	"pushl %%edx  \n\t"
	"movl %0, %%ebx	\n\t"
	"rdtsc	\n\t"
	"movl %%eax, (%%ebx) \n\t"
	"movl %%edx, 4(%%ebx)	\n\t"
	"popl %%edx \n\t"
	"popl %%ebx \n\t"
	"popl %%eax \n\t"
	::"r"(lpEnd));
#else
	__asm{
        push eax
        push ebx
        push edx
        mov ebx,lpEnd
        rdtsc                        //Read time stamp counter.
        mov dword ptr [ebx],eax      //Save low part
        mov dword ptr [ebx + 4],edx  //Save high part.
        pop edx
        pop ebx
        pop eax
	}
#endif
#else
	(void)lpEnd;  //Avoid warning generation under some compiler.
#endif
}

//
//The following routine calculates the clock circle counter of the current test.
//It subtract u64Start from u64End of lpPr object,and save the result into
//u64Result member.
//

VOID PerfCommit(__PERF_RECORDER* lpPr)
{
	if(NULL == lpPr)    //Parameter check.
		return;
	u64Sub(&lpPr->u64End,&lpPr->u64Start,&lpPr->u64Result);  //Performance a subtraction.
	if(MoreThan(&lpPr->u64Result,&lpPr->u64Max)) //The current max is less than this
		                                         //result.
	{
		lpPr->u64Max = lpPr->u64Result;          //Update the current max value.
	}
}

