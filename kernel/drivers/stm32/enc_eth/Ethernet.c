//***********************************************************************/
//    Author                    : tywind
//    Original Date             : Jan 6,2015
//    Module Name               : Ethernet.c
//    Module Funciton           : 
//                                This module countains ethernet 28j60
//                                driver's implementation code.
//                                The hardware driver only need implement several
//                                low level routines and link them into HelloX's
//                                ethernet skeleton.
//
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

#ifndef __KAPI_H__
#include "kapi.h"
#endif

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "string.h"
#include "stdio.h"

#include "ethernet/ethif.h"
#include "netif/etharp.h"
#include "enc28j60.h"

#ifndef __ETHERNET_H__
#include "Ethernet.h"
#endif

//自定义mac地址 
const char macaddr[ETH_MAC_LEN]={0x04,0x02,0x35,0x08,0x00,0x01}; 

//Initializer of the ethernet interface,it will be called by HelloX's ethernet framework.
//No need to specify it if no customized requirement.
static BOOL Ethernet_Int_Init(__ETHERNET_INTERFACE* pInt)
{
	return TRUE;
}

//Control functions of the ethernet interface.Some special operations,such as
//scaninn or association in WLAN,should be implemented in this routine,since they
//are not common operations for Ethernet.
static BOOL Ethernet_Ctrl(__ETHERNET_INTERFACE* pInt,DWORD dwOperation,LPVOID pData)
{
	return TRUE;
}

//Send a ethernet frame out through Marvell wifi interface.The frame's content is in pInt's
//send buffer.
static BOOL Ethernet_SendFrame(__ETHERNET_INTERFACE* pInt)
{
	BOOL          bResult    = FALSE;	

	if(NULL == pInt)
	{
		goto __TERMINAL;
	}
	if((0 == pInt->buffSize) || (pInt->buffSize > ETH_DEFAULT_MTU))  //No data to send or exceed the MTU.
	{
		goto __TERMINAL;
	}


	ENC28J60_Packet_Send(pInt->buffSize,(u8*)pInt->SendBuff);
	bResult = TRUE;

__TERMINAL:

	return bResult;
}

/**
*
* Receive a frame from ehternet link.
* Should allocate a pbuf and transfer the bytes of the incoming
* packet from the interface into the pbuf.
*
*/
static struct pbuf* Ethernet_RecvFrame(__ETHERNET_INTERFACE* pInt)
{

	struct pbuf  *p         = NULL;
	struct pbuf  *q         = NULL;
    u16          len        = 0;	
	u8           recvbuf[MAX_FRAMELEN];  
	
	/* Obtain the size of the packet and put it into the "len"
	variable. */ 
	
	len = ENC28J60_Packet_Receive(MAX_FRAMELEN,recvbuf);

	if(len > 0)
	{			
		u8       *buffer  = recvbuf;
		int      l        = 0;


		/* We allocate a pbuf chain of pbufs from the pool. */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
		if (p != NULL)
		{
			for (q = p; q != NULL; q = q->next)
			{
				memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
				l = l + q->len;
			}    
		}
		else	
		{
			#ifdef __ETH_DEBUG
				_hx_printf("  Marvel Driver: Allocate pbuf failed in RecvFrame.\r\n");
			#endif
		}
	}

	return p;
}

//Initializer or the Marvel Ethernet Driver,it's a global function and is called
//by Ethernet Manager in process of initialization.
BOOL Ethernet_Initialize(LPVOID pData)
{
	__ETHERNET_INTERFACE* pMarvelInt = NULL;

	//Initialize Ethernet Interface Driver.
	if(ENC28J60_Init((u8*)macaddr) == 1)
	{
		#ifdef __ETH_DEBUG
			_hx_printf("  Ethernet28j60 Driver: initialize failid...\r\n");
		#endif	

		return FALSE;
	}

	//PHLCON：PHY 模块LED 控制寄存器	    
	ENC28J60_PHY_Write(PHLCON,0x0476);
	
	//Register the ethernet interface.
	pMarvelInt = EthernetManager.AddEthernetInterface(
		ETHERNET_NAME,
		(char*)&macaddr[0],
		(LPVOID)NULL,
		Ethernet_Int_Init,
		Ethernet_SendFrame,
		Ethernet_RecvFrame,
		Ethernet_Ctrl);

	if(NULL == pMarvelInt)
	{
		#ifdef __ETH_DEBUG
			_hx_printf("  Ethernet28j60_Driver: AddEthernetInterface failid...\r\n");
		#endif	

		return FALSE;	
	}

	return TRUE;
}
