//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : marvel.c
//    Module Funciton           : 
//                                This module countains Marvell WiFi ethernet
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
#include "marvell_ops.h"

#ifndef __MARVELIF_H__
#include "marvelif.h"
#endif

/*
*
*  Implementation of Marvell WiFi ethernet drivers,under the framework
*  of HelloX's Ethernet Manager.
*
*/

//A helper local variable to record the lbs_private.
static struct lbs_private* priv = NULL;

//Assist functions to handle the associate request.
static void DoAssoc(__WIFI_ASSOC_INFO* pAssocInfo)
{
	if(0 == pAssocInfo->mode)  //Infrastructure mode.
	{
		marvel_assoc_network(priv,pAssocInfo->ssid,pAssocInfo->key,WIFI_MODE_INFRA,pAssocInfo->channel);
  }
	else  //Adhoc mode.
	{
		//Radio channel must be specified.
		if(0 == pAssocInfo->channel)
		{
			pAssocInfo->channel = 6;
		}
		marvel_assoc_network(priv,pAssocInfo->ssid,pAssocInfo->key,WIFI_MODE_ADHOC,pAssocInfo->channel);
	}
}

//Handle the scan request.
static void DoScan()
{
	struct bss_descriptor* iter = NULL;
	int i = 0;
	
	lbs_scan_worker(priv);				
	//Dump out the scan result.
#define list_for_each_entry_bssdes(pos, head, member)                 \
	for (pos = list_entry((head)->next,struct bss_descriptor, member);	\
	&pos->member != (head);                                             \
	pos = list_entry(pos->member.next,struct bss_descriptor, member))

	_hx_printf("  Available WiFi list:\r\n");
	_hx_printf("  ----------------------------- \r\n");
	list_for_each_entry_bssdes(iter, &priv->network_list, list)
	{
		_hx_printf("  %02d: BSSID = %06X, RSSI = %d, SSID = '%s', channel = %d\r\n", \
						  i++, iter->bssid, iter->rssi, \
						  iter->ssid,iter->channel);
  }
}

//Initializer of the ethernet interface,it will be called by HelloX's ethernet framework.
//No need to specify it if no customized requirement.
static BOOL Marvel_Int_Init(__ETHERNET_INTERFACE* pInt)
{
	return TRUE;
}

//Control functions of the ethernet interface.Some special operations,such as
//scaninn or association in WLAN,should be implemented in this routine,since they
//are not common operations for Ethernet.
static BOOL Marvel_Ctrl(__ETHERNET_INTERFACE* pInt,DWORD dwOperation,LPVOID pData)
{
	__WIFI_ASSOC_INFO*    pAssocInfo = (__WIFI_ASSOC_INFO*)pData;
	
	switch(dwOperation)
	{
		case ETH_MSG_SCAN:
			DoScan();
			break;
		case ETH_MSG_ASSOC:
			if(NULL == pAssocInfo)
			{
				break;
			}
			DoAssoc(pAssocInfo);
			break;
		default:
#ifdef __ETH_DEBUG
		_hx_printf("  Marvell Driver: Invalid control operations[opcode = %d].\r\n",dwOperation);
#endif
			break;
	}
	return TRUE;
}

//Send a ethernet frame out through Marvell wifi interface.The frame's content is in pInt's
//send buffer.
static BOOL Marvel_SendFrame(__ETHERNET_INTERFACE* pInt)
{
	BOOL          bResult       = FALSE;
	struct txpd   *txpd         = NULL;
	char          *p802x_hdr    = NULL;
	char          *buffer       = NULL;
	uint16_t      pkt_len       = 0;
	int           ret           = 0;
	__ETH_INTERFACE_STATE*      pIfState = NULL;
	
	if(NULL == pInt)
	{
		goto __TERMINAL;
	}
	if((0 == pInt->buffSize) || (pInt->buffSize > ETH_DEFAULT_MTU))  //No data to send or exceed the MTU.
	{
		goto __TERMINAL;
	}
	
	pIfState = &pInt->ifState;
	
	sdio_sys_wait = 0;
	
#ifdef __ETH_DEBUG
	_hx_printf("  Marvel Driver: SendFrame routine is called.\r\n");
#endif

	txpd=(void *)&priv->resp_buf[0][4];     //Why start from 4?
	memset(txpd, 0, sizeof(struct txpd));
	p802x_hdr = (char *)&pInt->SendBuff[0];         //802.3 mac.
	pkt_len = (uint16_t)pInt->buffSize;
	
	memcpy(txpd->tx_dest_addr_high, p802x_hdr, ETH_ALEN);
	txpd->tx_packet_length = cpu_to_le16(pkt_len);
	txpd->tx_packet_location = cpu_to_le32(sizeof(struct txpd));
	
	//Copy the frame to be sent into buffer.
	buffer=(char *)&txpd[1];
	memcpy(buffer,pInt->SendBuff,pkt_len);
	priv->resp_len[0] = pkt_len + sizeof(struct txpd);//Total sending length,include txpd.
	
	if (priv->resp_len[0] > 0)
	{
		ret = if_sdio_send_data(priv,priv->resp_buf[0],priv->resp_len[0]);
		if (ret)
		{
#ifdef __ETH_DEBUG
			_hx_printf("   Marvel Driver: host_to_card failed %d\r\n", ret);
#endif
			priv->dnld_sent = DNLD_RES_RECEIVED;
			bResult         = FALSE;
	  }
		else
		{
			//Update interface statistics info.
			pIfState->dwFrameSendSuccess += 1;
			bResult                       = TRUE;
#ifdef __ETH_DEBUG
			_hx_printf("  Marvel Driver: host_to_card successfully.\r\n");
#endif
		}
		priv->resp_len[0] = 0;
	}
	sdio_sys_wait=1;

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
static struct pbuf* Marvel_RecvFrame(__ETHERNET_INTERFACE* pInt)
{
	struct eth_packet   *rx_pkt    = &pgmarvel_priv->rx_pkt;
  struct pbuf         *p, *q;
  u16                 len        = 0;
  int                 l          = 0;
  char                *buffer    = NULL;
  
	p = NULL;
  /* Obtain the size of the packet and put it into the "len"
     variable. */ 
  len = lbs_rev_pkt();
	
  if(len > 0){
		buffer = rx_pkt->data;
		/* We allocate a pbuf chain of pbufs from the pool. */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
		if (p != NULL){
			for (q = p; q != NULL; q = q->next){
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
BOOL Marvel_Initialize(LPVOID pData)
{
	__ETHERNET_INTERFACE* pMarvelInt = NULL;
	char                  mac[ETH_MAC_LEN];
	
	//Initialize Ethernet Interface Driver.
#ifdef __ETH_DEBUG
	_hx_printf("  Marvel Driver: Begin to initialize SDIO and Marvel WiFi device...\r\n");
#endif
	priv = init_marvell_driver();
	lbs_scan_worker(priv);
#ifdef __ETH_DEBUG
	_hx_printf("  Marvel Driver: End of SDIO and WiFi initialization.\r\n");
#endif
	//Try to associate to the default SSID,use INFRASTRUCTURE mode.
	marvel_assoc_network(priv,WIFI_DEFAULT_SSID,WIFI_DEFAULT_KEY,WIFI_MODE_ADHOC,6);
	
	//Copy the MAC address.
	memcpy(mac,pgmarvel_priv->current_addr,ETH_MAC_LEN);
	
	//Register the ethernet interface.
	pMarvelInt = EthernetManager.AddEthernetInterface(
	  MARVEL_ETH_NAME,
	  &mac[0],
	  (LPVOID)priv,
	  Marvel_Int_Init,
	  Marvel_SendFrame,
		Marvel_RecvFrame,
		Marvel_Ctrl);
	if(NULL == pMarvelInt)
	{
		return FALSE;
	}
	return TRUE;
}
