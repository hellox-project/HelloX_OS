//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,07 2014
//    Module Name               : ethbrg.c
//    Module Funciton           : 
//                                This file countains implementation code of 
//                                Ethernet bridging functions.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <lwip/inet.h>

#include "hx_eth.h"
#include "ethmgr.h"
#include "ethbrg.h"

/*
* DPI hook routine.
*/
extern __ETHERNET_BUFFER* Do_DPI(__ETHERNET_BUFFER*);

/* 
 * Delivery an ethernet frame to bridging thread.
 * This routine is called by ethernet manager object,when
 * process an ethernet frame,and if the frame's destination
 * MAC address is not belong to the local scope.
 */
BOOL Do_Bridging(__ETHERNET_BUFFER* pBuffer)
{
	__ETHERNET_BUFFER* pNewBuff = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;

	BUG_ON(NULL == pBuffer);
	pEthInt = pBuffer->pInInterface;

	pNewBuff = Do_DPI(pBuffer);
	if (pNewBuff)
	{
		/*
		* Bridging the DPI processed new ethernet frame.
		* Please be noted that the new ethernet buffer object
		* will be destroyed by BroadcastEthernetFrame if it returns
		* TRUE. Otherwise the pNewBuff should be destroyed
		* here.
		*/
		if (!EthernetManager.BroadcastEthernetFrame(pNewBuff))
		{
			EthernetManager.DestroyEthernetBuffer(pNewBuff);
		}
		else
		{
			pEthInt->ifState.dwFrameBridged += 1;
		}
	}
	/*
	 * Destroy the pBuffer object.
	 */
	EthernetManager.DestroyEthernetBuffer(pBuffer);
	return TRUE;
}
