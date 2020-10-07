//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 12 Sep,2019
//    Module Name               : sysinfo.h
//    Module Funciton           : 
//                                System information structure is defined in this
//                                file.
//    Last modified Author      : Garry
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

/* For standard types. */
#include "TYPES.H"

/* 
 * System info structure,all members are 
 * self-interpretible.
 */
typedef struct _SYSTEM_INFO {
	union {
		DWORD dwOemId;          // Obsolete field...do not use
		struct {
			WORD wProcessorArchitecture;
			WORD wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	DWORD dwPageSize;
	LPVOID lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress;
	DWORD dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO, __SYSTEM_INFO;

#endif //__SYSINFO_H__
