//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 12, 2017
//    Module Name               : netmgr.h
//    Module Funciton           : 
//                                Network manager object's implementation.
//                                Network manager is the core object of network
//                                subsystem in HelloX,it supplies network common
//                                service,such as timer,common task,...
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>

#include "netcfg.h"
#include "proto.h"
#include "netmgr.h"
#include "netglob.h"

/* Initializer of network manager. */
static BOOL NetMgrInitialize(__NETWORK_MANAGER* pMgr)
{
	BOOL bResult = FALSE;
	int i = 0;

	BUG_ON(NULL == pMgr);

	/* Initialize network global object. */
	NetworkGlobal.Initialize(&NetworkGlobal);

	/* Initialize all network protocol object(s) in system. */
	i = 0;
	while ((NetworkProtocolArray[i].szProtocolName != NULL) && 
		(NetworkProtocolArray[i].ucProtocolType))
	{
		if (!NetworkProtocolArray[i].Initialize(&NetworkProtocolArray[i]))
		{
			/* Fail of any protocol object's initialization will lead whole failure. */
			goto __TERMINAL;
		}
		_hx_printf("Initialize protocol[%s] OK.\r\n",NetworkProtocolArray[i].szProtocolName);
		i++;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Network manager object. */
__NETWORK_MANAGER NetworkManager = {
	NetMgrInitialize,       //Initialize.
};
