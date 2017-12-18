//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 02,2017
//    Module Name               : netentry.c
//    Module Funciton           : 
//                                The unify entry point of network subsystem.
//                                OS entry routine will invoke the routine in this file,to
//                                initialize network subsystem,if it is enabled.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <sysnet.h>

#include "netcfg.h"
#include "ethmgr.h"
#include "netglob.h"
#include "netmgr.h"

#ifdef __CFG_NET_PPPOE
#include "pppox/oemgr.h"
#endif

/*
 * Unify entry point of network subsystem.
 * It will be called by OS_Entry routine in phase of system initialization,
 * then network modules will be initialized orderly in this routine.
 */
BOOL Net_Entry(VOID* pArg)
{
	BOOL bResult = FALSE;

	/* Initialize Network Manager object. */
	bResult = NetworkManager.Initialize(&NetworkManager);
	if (!bResult)
	{
		goto __TERMINAL;
	}

	bResult = TRUE;
__TERMINAL:
	return bResult;
}
