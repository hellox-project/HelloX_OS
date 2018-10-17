//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : ethmgr.c
//    Module Funciton           : 
//                                This module countains HelloX ethernet skeleton's
//                                implementation code.
//                                The main object is Ethernet Manager,which is a global
//                                object and provides all Ethernet related functios,and
//                                is implemented in this file.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <kapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <lwip/inet.h>

#include "hx_eth.h"
#include "ethmgr.h"
#include "ebridge/ethbrg.h"
#include "proto.h"

/*
* A local helper routine to show an ethernet frame.
*/
static void ShowEthernetFrame(__ETHERNET_BUFFER* pBuffer)
{
	if (NULL == pBuffer)
	{
		return;
	}
	_hx_printf("Frame: d_mac[%X-%X-%X-%X-%X-%X], s_mac[%X-%X-%X-%X-%X-%X], type = %X,len = %d.\r\n",
		pBuffer->dstMAC[0],
		pBuffer->dstMAC[1],
		pBuffer->dstMAC[2],
		pBuffer->dstMAC[3],
		pBuffer->dstMAC[4],
		pBuffer->dstMAC[5],
		pBuffer->srcMAC[0],
		pBuffer->srcMAC[1],
		pBuffer->srcMAC[2],
		pBuffer->srcMAC[3],
		pBuffer->srcMAC[4],
		pBuffer->srcMAC[5],
		pBuffer->frame_type,
		pBuffer->act_length);
}

//A helper routine to refresh DHCP configurations of the given interface.
static void dhcpRestart(__ETHERNET_INTERFACE* pEthInt, __NETWORK_PROTOCOL* pProtocol, __ETH_INTERFACE_STATE* pifState)
{
	LPVOID* pL3Interface = NULL;
	int index = 0;

	//Locate the corresponding L3 interface object.
	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		if (pEthInt->Proto_Interface[index].pProtocol == (LPVOID)pProtocol)
		{
			pL3Interface = pEthInt->Proto_Interface[index].pL3Interface;
			break;
		}
	}
	if (NULL == pL3Interface)  //No protocol bind to this ethernet interface.
	{
		return;
	}

#ifdef __ETH_DEBUG
	_hx_printf("  dhcpRestart: Begin to entry.\r\n");
#endif
	if (!pifState->bDhcpCltEnabled)  //DHCP is not enabled.
	{
#ifdef __ETH_DEBUG
		_hx_printf("  dhcpRestart: Try to start DHCP on interface.\r\n");
#endif
		pifState->bDhcpCltEnabled = TRUE;
		pifState->bDhcpCltOK = FALSE;
		//start DHCP on 
		//dhcp_start(pif);
		pProtocol->StartDHCP(pL3Interface);
#ifdef __ETH_DEBUG
		_hx_printf("  dhcpRestart: DHCP started on interface.\r\n");
#endif
		return;
	}
	else  //DHCP already enabled.
	{
#ifdef __ETH_DEBUG
		_hx_printf("  dhcpRestart: DHCP already enabled,restart it.\r\n");
#endif
		//Release the previous DHCP configurations.
		//dhcp_release(pif);
		pProtocol->ReleaseDHCP(pL3Interface);
		//dhcp_stop(pif);
		pProtocol->StopDHCP(pL3Interface);
		//Re-enable the DHCP client on the given interface.
		pifState->bDhcpCltOK = FALSE;
		//dhcp_start(pif);
		pProtocol->StartDHCP(pL3Interface);
		return;
	}
#ifdef __ETH_DEBUG
	_hx_printf("  dhcpRestart: End of routine.\r\n");
#endif
}

//Configure a given interface by applying the given parameters.
static void netifConfig(__ETHERNET_INTERFACE* pif, UCHAR proto, __ETH_INTERFACE_STATE* pifState, __ETH_IP_CONFIG* pifConfig)
{
	int proto_num = 0;
	__NETWORK_PROTOCOL* pProtocol = NULL;
	LPVOID pL3Interface = NULL;

	if ((NULL == pif) || (NULL == pifState) || (NULL == pifConfig))
	{
		return;
	}
	//Locate the protocol object bound to the ethernet interface.
	for (proto_num = 0; proto_num < MAX_BIND_PROTOCOL_NUM; proto_num++)
	{
		pProtocol = (__NETWORK_PROTOCOL*)pif->Proto_Interface[proto_num].pProtocol;
		if (NULL == pProtocol)
		{
			continue;
		}
		if (pProtocol->ucProtocolType == proto)  //Find.
		{
			pL3Interface = pif->Proto_Interface[proto_num].pL3Interface;
			if (NULL == pL3Interface)  //Should not occur.
			{
				BUG();
			}
			break;
		}
	}
	if (NULL == pL3Interface)
	{
		_hx_printf("  The interface [%s] is not bound to protocol [%d].\r\n",
			pif->ethName, proto);
		return;
	}

#ifdef __ETH_DEBUG
	_hx_printf("  %s: Begin to compare flags and locate proper routine.\r\n", __func__);
#endif
	//Configure the interface according different DHCP flags.
	if (pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_DISABLE)
	{
		//Disable DHCP on the given interface.
		if (pifState->bDhcpCltEnabled)
		{
			if (pifState->bDhcpCltOK)  //Release configurations first.
			{
				//dhcp_release(pif);
				pProtocol->ReleaseDHCP(pL3Interface);
				pifState->bDhcpCltOK = FALSE;
			}
			//dhcp_stop(pif);
			pProtocol->StopDHCP(pL3Interface);
			pifState->bDhcpCltEnabled = FALSE;
		}
		//Configure static IP address on the interface.
		pProtocol->SetIPAddress(pL3Interface, pifConfig);
		//Save new configurations to interface state.
		memcpy(&pifState->IpConfig, pifConfig, sizeof(__ETH_IP_CONFIG));
		_hx_printf("\r\n  DHCP on interface [%s] is disabled and IP address is set.\r\n", pifConfig->ethName);
	}
	if (pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_ENABLE)
	{
		pifState->bDhcpCltEnabled = TRUE;
	}
	if (pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RESTART)
	{
#ifdef __ETH_DEBUG
		_hx_printf("  netifConfig: DHCP Restart is required.\r\n");
#endif
		//pifState->bDhcpCltEnabled = TRUE;
		dhcpRestart(pif, pProtocol, pifState);
		_hx_printf("\r\n  DHCP restarted on interface [%s].\r\n", pifConfig->ethName);
	}
	if (pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RENEW)
	{
		if (pifState->bDhcpCltEnabled)
		{
			//dhcp_renew(pif);
			pProtocol->RenewDHCP(pL3Interface);
			pifState->bDhcpCltOK = FALSE;
		}
	}
	if (pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RELEASE)
	{
		if (pifState->bDhcpCltEnabled)
		{
			//dhcp_release(pif);
			pProtocol->ReleaseDHCP(pL3Interface);
			pifState->bDhcpCltOK = FALSE;
		}
	}
	return;
}

//Try to receive a packet from a specified interface.This routine maybe called by the
//ethernet core thread when processing DELIVERY message.
static void _eth_if_input(__ETHERNET_INTERFACE* pEthInt)
{
	__ETHERNET_BUFFER*    p = NULL;
	__NETWORK_PROTOCOL*   pProtocol = NULL;
	BOOL                  bDeliveryResult = FALSE;
	int                   err = 0;
	int                   index = 0;

	if (NULL == pEthInt)
	{
		return;
	}

	if (pEthInt->RecvFrame)
	{
		while (TRUE)
		{
			p = pEthInt->RecvFrame(pEthInt);
			if (NULL == p)  //No available frames.
			{
				break;
			}
			//Update interface statistics.
			pEthInt->ifState.dwFrameRecv++;
			pEthInt->ifState.dwTotalRecvSize += p->act_length;
			//Delivery the frame to layer 3.
			for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
			{
				if (pEthInt->Proto_Interface[index].frame_type_mask == 0)
				{
					continue;
				}
				if ((pEthInt->Proto_Interface[index].frame_type_mask & p->frame_type) ==
					p->frame_type)
				{
					pProtocol = pEthInt->Proto_Interface[index].pProtocol;
					if (NULL == pProtocol)
					{
						BUG();
					}
					//Delivery the frame to L3 protocol.
					bDeliveryResult = pProtocol->DeliveryFrame(p, pEthInt->Proto_Interface[index].pL3Interface);
					if (bDeliveryResult)
					{
						pEthInt->ifState.dwFrameRecvSuccess++;
						break;
					}
				}
			}
			//Release the Ethernet Buffer object.
			EthernetManager.DestroyEthernetBuffer(p);
		}
	}
}

//A helper routine,to poll all ethernet interface(s) to check if there is frame availabe,
//and delivery it to layer 3 if so.
static void _ethernet_if_input()
{
	__ETHERNET_INTERFACE*  pEthInt = NULL;
	int                    index = 0;

	for (index = 0; index < EthernetManager.nIntIndex; index++)
	{
		pEthInt = &EthernetManager.EthInterfaces[index];
		_eth_if_input(pEthInt);
	}
}

//A helper routine to check assist the DHCP process.It checks if the DHCP
//process is successful,and do proper actions(such as set the offered IP
//address to interface) according DHCP status.
static void ifDHCPAssist(__ETHERNET_INTERFACE* pEthInt)
{
	__NETWORK_PROTOCOL* pProtocol = NULL;
	LPVOID pL3Interface = NULL;
	__DHCP_CONFIG dhcp;
	__ETH_IP_CONFIG ethConf;
	int index = 0;

	//Do nothing if interface is shutdown.
	if (pEthInt->ifState.dwInterfaceStatus & ETHERNET_INTERFACE_STATUS_DOWN)
	{
		return;
	}
	//DHCP client is disabled.
	if (!pEthInt->ifState.bDhcpCltEnabled)
	{
		return;
	}
	//Already obtained configurations through DHCP protocol.
	if (pEthInt->ifState.bDhcpCltOK)
	{
		//Just increment leased time.
		pEthInt->ifState.dwDhcpLeasedTime += WIFI_POLL_TIME;
		return;
	}

	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		pL3Interface = pEthInt->Proto_Interface[index].pL3Interface;
		if (NULL == pL3Interface)  //This slot is empty.
		{
			continue;
		}
		pProtocol = (__NETWORK_PROTOCOL*)pEthInt->Proto_Interface[index].pProtocol;
		if (NULL == pProtocol)  //Shoud not occur.
		{
			BUG();
		}
		//Now try to check if we get the valid configuration from DHCP server
		if (pProtocol->GetDHCPConfig(pL3Interface, &dhcp))
		{
			//Configure the interface with the offered DHCP configurations.
			memset(&ethConf, 0, sizeof(__ETH_IP_CONFIG));
			memcpy(&ethConf.ipaddr, &dhcp.network_addr, sizeof(__COMMON_NETWORK_ADDRESS));
			memcpy(&ethConf.mask, &dhcp.network_mask, sizeof(__COMMON_NETWORK_ADDRESS));
			memcpy(&ethConf.defgw, &dhcp.default_gw, sizeof(__COMMON_NETWORK_ADDRESS));
			memcpy(&ethConf.dnssrv_1, &dhcp.dns_server1, sizeof(__COMMON_NETWORK_ADDRESS));
			memcpy(&ethConf.dnssrv_2, &dhcp.dns_server2, sizeof(__COMMON_NETWORK_ADDRESS));
			if (pProtocol->SetIPAddress(pL3Interface, &ethConf))
			{
				/*
				* Show IP address information.
				* Separate the printing since inet_ntoa use static
				* buffer to hold the result,one print with 2 invokes
				* of inet_ntoa yeilds same string output of ip_addr
				* and ip_mask.
				*/
				_hx_printf("\r\nDHCP: Assigned ip_addr[%s/",
					inet_ntoa(ethConf.ipaddr.Address.ipv4_addr));
				_hx_printf("%s] to net_if[%s].\r\n",
					inet_ntoa(ethConf.mask.Address.ipv4_addr),
					pEthInt->ethName);
				pEthInt->ifState.bDhcpCltOK = TRUE;
				//Save the ethernet interface's configuration.
				memcpy(&pEthInt->ifState.IpConfig.ipaddr, &dhcp.network_addr,
					sizeof(__COMMON_NETWORK_ADDRESS));
				memcpy(&pEthInt->ifState.IpConfig.defgw, &dhcp.default_gw,
					sizeof(__COMMON_NETWORK_ADDRESS));
				memcpy(&pEthInt->ifState.IpConfig.mask, &dhcp.network_mask,
					sizeof(__COMMON_NETWORK_ADDRESS));
				//Stop DHCP process in the interface.
				pProtocol->StopDHCP(pL3Interface);
				pEthInt->ifState.dwDhcpLeasedTime = 0;  //Start to count DHCP time.
				return;
			}
		}
	}
	return;
}

static void _dhcpAssist()
{
	__ETHERNET_INTERFACE*    pEthInt = NULL;
	struct netif*            netif = NULL;
	int                      index = 0;

	for (index = 0; index < EthernetManager.nIntIndex; index++)
	{
		pEthInt = &EthernetManager.EthInterfaces[index];
		ifDHCPAssist(pEthInt);
	}
	return;
}

//Post a ethernet buffer object to Ethernet Manager object,which is mainly called
//in NIC driver.
static BOOL _PostFrame(__ETHERNET_INTERFACE* pEthInt, __ETHERNET_BUFFER* pBuffer)
{
	__KERNEL_THREAD_MESSAGE msg;
	DWORD dwFlags;

	if ((NULL == pEthInt) || (NULL == pBuffer))
	{
		return FALSE;
	}
	//if (pEthInt != pBuffer->pEthernetInterface)
	if (pEthInt != pBuffer->pInInterface)
	{
		return FALSE;
	}
	/*
	 * This routine is called by NIC driver,so any ethernent buffer
	 * object delivered to this routine should be newly constructed,
	 * thus it's next pointer must be NULL.
	 */
	BUG_ON(pBuffer->pNext != NULL);

	//Link the ethernet buffer object to list,and send a message to ethernet core
	//thread if is the fist buffer object.
	__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
	if (NULL == EthernetManager.pBufferFirst)
	{
		if (NULL != EthernetManager.pBufferLast)
		{
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			BUG();
		}
		//Link the buffer object to list before send POSTFRAME message.
		EthernetManager.pBufferFirst = pBuffer;
		EthernetManager.pBufferLast = pBuffer;
		EthernetManager.nBuffListSize += 1;
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);

		//Then send the post frame message to ethernet core thread,make it noticed to
		//process the incoming packet.
		//Since it may fail,we will give up in this case...
		msg.wCommand = ETH_MSG_POSTFRAME;
		msg.wParam = 0;
		msg.dwParam = (DWORD)pBuffer;
		if (!SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg))
		{
			__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			EthernetManager.pBufferFirst = NULL;
			EthernetManager.pBufferLast = NULL;
			EthernetManager.nBuffListSize -= 1;
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			return FALSE;
		}
	}
	else
	{
		if (NULL == EthernetManager.pBufferLast)
		{
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			BUG();
		}
		//Exceed the maximal buffer list size.
		if (EthernetManager.nBuffListSize > MAX_ETH_RXBUFFLISTSZ)
		{
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			return FALSE;
		}
		EthernetManager.pBufferLast->pNext = pBuffer;
		EthernetManager.pBufferLast = pBuffer;
		EthernetManager.nBuffListSize += 1;
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
	}
	return TRUE;
}

//A helper routine to delivery an ethernet frame to local layer 3 protocol,such
//as IP,...
static BOOL _Delivery2Local(__ETHERNET_BUFFER* pEthBuffer)
{
	BOOL bDeliveryResult = FALSE;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	__NETWORK_PROTOCOL* pProtocol = NULL;
	int index = 0;

	/* 
	 * Here is something wrong that should be reviewed further
	 * insight,since it leads memory leak.
	 * Just mark it.
	 */

	if (NULL == pEthBuffer)
	{
		goto __TERMINAL;
	}
	//pEthInt = pEthBuffer->pEthernetInterface;
	pEthInt = pEthBuffer->pInInterface;

	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		if (0 == pEthInt->Proto_Interface[index].frame_type_mask)
		{
			/* Protocol slot is empty,skip. */
			continue;
		}
		if ((pEthInt->Proto_Interface[index].frame_type_mask & pEthBuffer->frame_type) ==
			pEthBuffer->frame_type)
		{
			pProtocol = pEthInt->Proto_Interface[index].pProtocol;
			if (NULL == pProtocol)
			{
				BUG();
			}
			bDeliveryResult = pProtocol->DeliveryFrame(pEthBuffer, pEthInt->Proto_Interface[index].pL3Interface);
			if (bDeliveryResult)
			{
				pEthInt->ifState.dwFrameRecvSuccess++;
				break;
			}
		}
	}

__TERMINAL:
	return bDeliveryResult;
}

//Handler the Post Frame message.
static BOOL _PostFrameHandler()
{
	__ETHERNET_BUFFER* pBuffer = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	BOOL bDeliveryResult = FALSE;
	DWORD dwFlags;
	BOOL bShouldReturn = FALSE, bResult = FALSE;

	/* 
	 * Disable scheduling first,to improve the efficiency. 
	 * There maybe several frames in list,each frame will be posted
	 * to upper layer protocol.If scheduling is enabled,then
	 * each frame may lead thread rescheduling.
	 * Disable scheduling may make sure all frames are posted to
	 * upper layer protocol at same time,like a trunk carriering
	 * goods,batch all goods together is more effective than
	 * carriering one by one.
	 */
	//bScheduling = KernelThreadManager.EnableScheduling(FALSE);

	while (TRUE)
	{
		//Get one ethernet buffer from list.
		__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
		if (NULL == EthernetManager.pBufferFirst)  //No frame pending.
		{
			if (EthernetManager.pBufferLast)  //Double check.
			{
				__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
				BUG();
				goto __TERMINAL;
			}
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			goto __TERMINAL;
		}
		bShouldReturn = FALSE;
		pBuffer = EthernetManager.pBufferFirst;
		EthernetManager.pBufferFirst = pBuffer->pNext;
		if (NULL == pBuffer->pNext)
		{
			EthernetManager.pBufferLast = NULL;
			/* 
			 * Should return,since we encounter empty condition of
			 * buffer list. Otherwise may lead the message queue full
			 * netCore thread,cause the NIC driver will drop a msg
			 * to netCore when it makes the buffer list to no-empty
			 * from empty.Just check the implementation code of
			 * PostFrame routine of EthernetManager object.
			 */
			bShouldReturn = TRUE;
		}
		EthernetManager.nBuffListSize -= 1;
		BUG_ON(EthernetManager.nBuffListSize < 0);
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);

		//Update interface statistics.
		//pEthInt = pBuffer->pEthernetInterface;
		pEthInt = pBuffer->pInInterface;
		BUG_ON(NULL == pEthInt);
		pEthInt->ifState.dwFrameRecv++;
		pEthInt->ifState.dwTotalRecvSize += pBuffer->act_length;

		//Process the frame according to it's destination MAC address.
		if (Eth_MAC_Match(pEthInt->ethMac, pBuffer->dstMAC))
		{
			bDeliveryResult = _Delivery2Local(pBuffer);  //Delivery to local.
			EthernetManager.DestroyEthernetBuffer(pBuffer);
		}
		else
		{
			//Broadcast or multicast frame also should be delivered to local.
			if (Eth_MAC_BM(pBuffer->dstMAC))
			{
				if (Eth_MAC_Multicast(pBuffer->dstMAC))
				{
					pEthInt->ifState.dwRxMcastNum ++;
				}
				bDeliveryResult = _Delivery2Local(pBuffer);
			}
#ifdef __CFG_NET_EBRG
			/*
			 * Delivery the ethernet frame to bridging thread.
			 * The buffer object will be released in bridging
			 * function if delivering is successful.
			 */
			if (Do_Bridging(pBuffer))
			{
				continue; //Continue to process next frame.
			}
			else /* Should destroy the frame here. */
#endif
			{
				EthernetManager.DestroyEthernetBuffer(pBuffer);
			}
			/*
			* The old ethernet buffer should be released here.
			*/
		}
		if (bShouldReturn)
		{
			bResult = TRUE;
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return bResult;
}

/*
* Handler of the ETH_MSG_BROADCAST in eth_core thread.
*/
static BOOL __BroadcastHandler()
{
	__ETHERNET_BUFFER* pBuffer = NULL;
	__ETHERNET_BUFFER* pNewBuff = NULL;
	__ETHERNET_INTERFACE* pRecvInt = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	BOOL bDeliveryResult = FALSE;
	int index = 0;
	DWORD dwFlags;

	while (TRUE)
	{
		__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
		if (NULL == EthernetManager.pBroadcastFirst)  //No frame pending.
		{
			if (EthernetManager.pBroadcastLast)  //Check again.
			{
				__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
				BUG();
				return FALSE;
			}
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			return FALSE;
		}
		/*
		* Fetch one frame buffer from broadcast list and delete it from list.
		*/
		pBuffer = EthernetManager.pBroadcastFirst;
		EthernetManager.pBroadcastFirst = pBuffer->pNext;
		if (NULL == pBuffer->pNext)  //Last frame buffer.
		{
			EthernetManager.pBroadcastLast = NULL;
		}
		EthernetManager.nBroadcastSize -= 1;
		BUG_ON(EthernetManager.nBroadcastSize < 0);
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);

		//pRecvInt = pBuffer->pEthernetInterface;
		pRecvInt = pBuffer->pInInterface;
		BUG_ON(NULL == pRecvInt);
		BUG_ON(KERNEL_OBJECT_SIGNATURE != pBuffer->dwSignature);

		/*
		* Broadcast the ethernet frame to all port(s) except the receiving
		* one.
		*/
		for (index = 0; index < EthernetManager.nIntIndex; index++)
		{
			pEthInt = &EthernetManager.EthInterfaces[index];
			if (pEthInt->ifState.dwInterfaceStatus == ETHERNET_INTERFACE_STATUS_UNKNOWN)
			{
				continue;  //Maybe empty slot.
			}
			if (pEthInt->ifState.dwInterfaceStatus == ETHERNET_INTERFACE_STATUS_DOWN)
			{
				continue;  //Skip the down interface.
			}
			if (pEthInt == pRecvInt)
			{
				continue;  //Skip the receiving interface.
			}
			BUG_ON(NULL == pEthInt->SendFrame);

			//Now use the default ehernet send buffer to sendout the frame.
			memcpy(&pEthInt->SendBuffer, pBuffer, sizeof(__ETHERNET_BUFFER));
			//pEthInt->SendBuffer.pEthernetInterface = pEthInt; //Mark the out interface.
			pEthInt->SendBuffer.pOutInterface = pEthInt; //Mark the out interface.
			BUG_ON(KERNEL_OBJECT_SIGNATURE != pEthInt->SendBuffer.dwSignature);
			if (pEthInt->SendFrame(pEthInt))
			{
				//pEthInt->ifState.dwFrameSendSuccess += 1;
			}
			pEthInt->ifState.dwFrameSend += 1;
			pEthInt->ifState.dwTotalSendSize += pBuffer->act_length;
		}
		/*
		* Destroy the ethernet frame object.
		*/
		EthernetManager.DestroyEthernetBuffer(pBuffer);
	}
	return TRUE;
}

//Dedicated Ethernet core thread,repeatly to poll all ethernet interfaces to receive frame,if
//interrupt mode is not supported by ethernet driver,and do some other functions.
static DWORD EthCoreThreadEntry(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	HANDLE                  hTimer = NULL;           //Handle of receiving poll timer.
	struct netif*           pif = (struct netif*)pData;
	__ETHERNET_INTERFACE*   pEthInt = NULL;
	__WIFI_ASSOC_INFO*      pAssocInfo = NULL;
	__ETHERNET_BUFFER*      pEthBuffer = NULL;
	__ETH_IP_CONFIG*        pifConfig = NULL;
	__ETH_INTERFACE_STATE*  pifState = NULL;
	int                     tot_len = 0;
	int                     index = 0;

	//Initialize all ethernet driver(s) registered in system.
#ifdef __ETH_DEBUG
	_hx_printf("  Ethernet Manager: Begin to load ethernet drivers...\r\n");
#endif
	while (EthernetDriverEntry[index].EthEntryPoint)
	{
		if (!EthernetDriverEntry[index].EthEntryPoint(EthernetDriverEntry[index].pData))
		{
#ifdef __ETH_DEBUG
			_hx_printf("  Ethernet Manager: Initialize ethernet driver failed[%d].\r\n", index);
#endif
		}
		index++;
	}
	index = 0;

	//Set the receive polling timer.
	hTimer = SetTimer(WIFI_TIMER_ID, WIFI_POLL_TIME, NULL, NULL, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer)
	{
		goto __TERMINAL;
	}

	//Main message loop.
	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;

			case ETH_MSG_SETCONF:             //Configure a given interface.
				pifConfig = (__ETH_IP_CONFIG*)msg.dwParam;
				if (NULL == pifConfig)
				{
					break;
				}
				//Locate the ethernet interface object by it's name.
				pEthInt = NULL;
				for (index = 0; index < EthernetManager.nIntIndex; index++)
				{
					if (0 == strcmp(pifConfig->ethName, EthernetManager.EthInterfaces[index].ethName))
					{
						pEthInt = &EthernetManager.EthInterfaces[index];
						pifState = &EthernetManager.EthInterfaces[index].ifState;
						break;
					}
				}
				if (NULL == pEthInt)  //Not find.
				{
					KMemFree(pifConfig, KMEM_SIZE_TYPE_ANY, 0);
					break;
				}
				//Commit the configurations to interface.
				netifConfig(pEthInt, pifConfig->protoType, pifState, pifConfig);
				KMemFree(pifConfig, KMEM_SIZE_TYPE_ANY, 0);
				break;
			case ETH_MSG_BROADCAST:  //Broadcast an ethernet frame.
				__BroadcastHandler();
				break;
			case ETH_MSG_SEND: //Send a link level frame.
				pEthBuffer = (__ETHERNET_BUFFER*)msg.dwParam;
				BUG_ON(NULL == pEthBuffer);
				BUG_ON(KERNEL_OBJECT_SIGNATURE != pEthBuffer->dwSignature);

				//pEthInt = (__ETHERNET_INTERFACE*)pEthBuffer->pEthernetInterface;
				pEthInt = pEthBuffer->pOutInterface;
				BUG_ON(NULL == pEthInt);
				BUG_ON(NULL == pEthInt->SendFrame);
				if (pEthInt->ifState.dwInterfaceStatus & ETHERNET_INTERFACE_STATUS_DOWN)
				{
					break;
				}
				tot_len = pEthBuffer->act_length;
				if (&pEthInt->SendBuffer != pEthBuffer)  //Not the default Ethernet Buffer.
				{
					//Copy to ethernet interface's default buffer.
					memcpy(&pEthInt->SendBuffer, pEthBuffer, sizeof(__ETHERNET_BUFFER));
					//Release the Ethernet Buffer object.
					EthernetManager.DestroyEthernetBuffer(pEthBuffer);
				}
				//Now try to send the frame.
				if (pEthInt->SendBuffer.dwSignature != KERNEL_OBJECT_SIGNATURE)
				{
					BUG();
				}
				if (pEthInt->SendFrame(pEthInt))
				{
					//Update statistics info.
					//pEthInt->ifState.dwFrameSendSuccess += 1;
				}
				pEthInt->ifState.dwFrameSend += 1;
				pEthInt->ifState.dwTotalSendSize += tot_len;
				break;

			case ETH_MSG_RECEIVE:              //Receive frame,may triggered by interrupt.
				pEthInt = (__ETHERNET_INTERFACE*)msg.dwParam;
				_eth_if_input(pEthInt);
				break;
			case ETH_MSG_POSTFRAME:
				_PostFrameHandler();
				break;
			case KERNEL_MESSAGE_TIMER:
				if (WIFI_TIMER_ID == msg.dwParam) //Must match the receiving timer ID.
				{
					_ethernet_if_input();
				}
				_dhcpAssist();        //Call DHCP assist function routinely.
				break;

			case ETH_MSG_DELIVER:  //Delivery a packet.
				break;
			case ETH_MSG_ASSOC:
				pAssocInfo = (__WIFI_ASSOC_INFO*)msg.dwParam;
				if (NULL == pAssocInfo->pPrivate)
				{
					break;
				}
				pEthInt = (__ETHERNET_INTERFACE*)pAssocInfo->pPrivate;
				if (pEthInt->IntControl)
				{
					pEthInt->IntControl(pEthInt, ETH_MSG_ASSOC, pAssocInfo);
				}
				//Release the association object,which is allocated in Assoc
				//function.
				KMemFree(pAssocInfo, KMEM_SIZE_TYPE_ANY, 0);
				break;
			default:
				break;
			}
		}
	}

__TERMINAL:
	if (hTimer)  //Should cancel it.
	{
		CancelTimer(hTimer);
	}
	return 0;
}

//Initializer of Ethernt Manager object.
static BOOL Initialize(struct __ETHERNET_MANAGER* pManager)
{
	BOOL      bResult = FALSE;

	if (NULL == pManager)
	{
		return FALSE;
	}

	/* Init spin lock. */
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pManager->spin_lock,"ethmgr");
#endif

	/* 
	 * Main thread of ethernet functions. 
	 * All ethernet related functions are bounced to this kernel thread
	 * to process.
	 * It's priority is higher than normal priority level,but lower than
	 * the main thread of tcp/ip.
	 * The priority level of kernel threads in network subsystem in HelloX,
	 * is arranged as following rule:
	 *   Kernel thread's priority level in device driver <=
	 *   Kernel thread's priority level in datalink layer <=
	 *   Kernel thread's priority level in network layer <=
	 *   Kernel thread's priority level in transportation layer.
	 */
	EthernetManager.EthernetCoreThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_HIGH_3,       //High priority,but lower than TCP/IP thread.
		EthCoreThreadEntry,
		NULL,
		NULL,
		ETH_THREAD_NAME);
	if (NULL == EthernetManager.EthernetCoreThread)  //Can not create the core thread.
	{
		goto __TERMINAL;
	}

	pManager->bInitialized = TRUE;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

//Trigger the ethernet core thread to launch a receiving poll.Mainly used by
//device drivers.
static BOOL _TriggerReceive(__ETHERNET_INTERFACE* pEthInt)
{
	__KERNEL_THREAD_MESSAGE msg;

	msg.wCommand = ETH_MSG_RECEIVE;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pEthInt;
	SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg);
	return TRUE;
}

//Send out a ethernet frame through the specified ethernet interface.
static BOOL SendFrame(__ETHERNET_INTERFACE* pEthInt, __ETHERNET_BUFFER* pEthBuff)
{
	__KERNEL_THREAD_MESSAGE msg;
	__ETHERNET_BUFFER* pSendBuff = NULL;

	if (NULL == pEthInt)
	{
		return FALSE;
	}

	//Send a message to the ethernet core thread.
	msg.wCommand = ETH_MSG_SEND;
	msg.wParam = 0;
	if (pEthBuff)
	{
		//Validate the pEthBuff object.
		if (pEthBuff->dwSignature != KERNEL_OBJECT_SIGNATURE)
		{
			BUG();
		}
		if (pEthBuff->act_length > pEthBuff->buff_length)
		{
			BUG();
		}
		//pEthBuff->pEthernetInterface = pEthInt;
		pEthBuff->pOutInterface = pEthInt;
		msg.dwParam = (DWORD)pEthBuff;
		pSendBuff = pEthBuff;
	}
	else  //Use the embedded Ethernet Buffer object in Ethernet Interface.
	{
		//Validate the default Ethernet Buffer object.
		if (pEthInt->SendBuffer.dwSignature != KERNEL_OBJECT_SIGNATURE)
		{
			BUG();
		}
		if (pEthInt->SendBuffer.act_length > pEthInt->SendBuffer.buff_length)
		{
			BUG();
		}
		//pEthInt->SendBuffer.pEthernetInterface = pEthInt;
		pEthInt->SendBuffer.pOutInterface = pEthInt;
		msg.dwParam = (DWORD)(&pEthInt->SendBuffer);
		pSendBuff = &pEthInt->SendBuffer;
	}
	//Do some checks before send...
	if (pSendBuff->act_length >= ETH_DEFAULT_MTU + ETH_HEADER_LEN)
	{
		_hx_printf("%s:frame length[%d] may exceed.\r\n", pSendBuff->act_length);
	}
	/*
	* Send a message to ethernet core tread to trigger frame
	* sending.
	* Message queue full or other reason may lead to failure.
	*/
	if (!SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg))
	{
		return FALSE;
	}
	return TRUE;
}

//Implementation of AddEthernetInterface,which is called by Ethernet Driver to register an interface
//object.
static __ETHERNET_INTERFACE* AddEthernetInterface(char* ethName, char* mac, LPVOID pIntExtension,
	__ETHOPS_INITIALIZE Init,
	__ETHOPS_SEND_FRAME SendFrame,
	__ETHOPS_RECV_FRAME RecvFrame,
	__ETHOPS_INT_CONTROL IntCtrl)
{
	__ETHERNET_INTERFACE*        pEthInt = NULL;
	BOOL                         bResult = FALSE;
	int                          index = 0, i = 0;
	BOOL                         bDefaultInt = FALSE;       //If the added interface is default one.
	__NETWORK_PROTOCOL*          pProtocol = NULL;

	if ((NULL == ethName) || (NULL == SendFrame) || (NULL == mac))  //Name and send operation are mandatory.
	{
		goto __TERMINAL;
	}
	if (!EthernetManager.bInitialized)  //Ethernet Manager is not initialized yet.
	{
		goto __TERMINAL;
	}
	if (EthernetManager.nIntIndex >= MAX_ETH_INTERFACE_NUM) //No available interface slot.
	{
		goto __TERMINAL;
	}

	//Get the free ethernet interface object from array.
	pEthInt = &EthernetManager.EthInterfaces[EthernetManager.nIntIndex];
	if (0 == EthernetManager.nIntIndex)  //First interface,set to default.
	{
		bDefaultInt = TRUE;
	}
	EthernetManager.nIntIndex++;
	memset(pEthInt, 0, sizeof(__ETHERNET_INTERFACE));

	//Initialize the embedded Ethernet Buffer object.
	pEthInt->SendBuffer.buff_length = ETH_DEFAULT_MTU + ETH_HEADER_LEN;
	pEthInt->SendBuffer.buff_status = ETHERNET_BUFFER_STATUS_FREE;
	pEthInt->SendBuffer.frame_type = 0;
	//pEthInt->SendBuffer.pEthernetInterface = pEthInt;  //Point back.
	pEthInt->SendBuffer.pNext = NULL;
	pEthInt->SendBuffer.dwSignature = KERNEL_OBJECT_SIGNATURE;

	//Copy ethernet name,do not use strxxx routine for safety.
	while (ethName[index] && (index < MAX_ETH_NAME_LEN))
	{
		pEthInt->ethName[index] = ethName[index];
		index++;
	}
	pEthInt->ethName[index] = 0;

	//Copy MAC address,there may exist some risk,since we don't check the length
	//of MAC address,but the caller should gaurantee it's length.
	for (index = 0; index < ETH_MAC_LEN; index++)
	{
		pEthInt->ethMac[index] = mac[index];
	}

	pEthInt->ifState.bDhcpCltEnabled = FALSE;
	pEthInt->ifState.bDhcpCltOK = FALSE;
	pEthInt->ifState.bDhcpSrvEnabled = FALSE;

	//Interface's default status is UP,driver can change it in Init routine.
	pEthInt->ifState.dwInterfaceStatus = ETHERNET_INTERFACE_STATUS_UP;

	pEthInt->pIntExtension = pIntExtension;
	pEthInt->SendFrame = SendFrame;
	pEthInt->RecvFrame = RecvFrame;
	pEthInt->IntControl = IntCtrl;

	//Sending queue(list) size.
	pEthInt->nSendingQueueSz = 0;

	//Bind to protocols.
	i = 0;
	index = 0;
	while (NetworkProtocolArray[i].szProtocolName)
	{
		pProtocol = &NetworkProtocolArray[i];
		i++;
		if (pProtocol->CanBindInterface(pProtocol, pEthInt, &pEthInt->Proto_Interface[index].frame_type_mask))
		{
			//Add the Ethernet interface to protocol.
			pEthInt->Proto_Interface[index].pL3Interface = pProtocol->AddEthernetInterface(pEthInt);
			if (NULL == pEthInt->Proto_Interface[index].pL3Interface)  //Failed to add.
			{
				//Clear the protocol number,since it maybe initialized by CanBindInterface routine.
				pEthInt->Proto_Interface[index].frame_type_mask = 0;
				continue;  //Check another protocol.
			}
			pEthInt->Proto_Interface[index].pProtocol = pProtocol;
			//Reserve the protocol-interface binding object.
			index++;
			if (index >= MAX_BIND_PROTOCOL_NUM)
			{
				break;
			}
		}
	}

	if (0 == index)  //Failed to bind any protocol.
	{
		_hx_printf("  Warning: No protocol bind to Ethernet Interface [%s].\r\n",
			pEthInt->ethName);
	}

	//Call driver's initializer,if specified.
	if (Init)
	{
		if (!Init(pEthInt))
		{
			goto __TERMINAL;
		}
	}

#ifdef __CFG_NET_DHCP
	//Restart DHCP process on the ethernet interface for each L3 protocol.
	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		dhcpRestart(pEthInt, pEthInt->Proto_Interface[index].pProtocol, &pEthInt->ifState);
	}
#endif

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		//Release the occupied interface slot.
		if (pEthInt)
		{
			//Unbind L3 protocol if already bound.
			for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
			{
				pProtocol = pEthInt->Proto_Interface[index].pProtocol;
				if (pProtocol)
				{
					pProtocol->DeleteEthernetInterface(pEthInt,
						pEthInt->Proto_Interface[index].pL3Interface);
				}
			}
			//Clear the allocated ethernet slot.
			memset(pEthInt, 0, sizeof(__ETHERNET_INTERFACE));
			EthernetManager.nIntIndex--;
		}
	}
	return pEthInt;
}

/*
* Delete an interface from system,it's the reverse operation of
* AddEthernetInterface.
*/
static BOOL DeleteEthernetInterface(__ETHERNET_INTERFACE* pEthInt)
{
	_hx_printf("Warning: Routine[%s] is not implemented yet.\r\n", __func__);
	return FALSE;
}

//Config a specified interface,such as it's IP address,DNS server,DHCP
//flags,etc.
static BOOL ConfigInterface(char* ethName, __ETH_IP_CONFIG* pConfig)
{
	__KERNEL_THREAD_MESSAGE   msg;
	__ETH_IP_CONFIG*          pifConfig = NULL;
	BOOL                      bResult = FALSE;

	if ((NULL == ethName) || (NULL == pConfig))
	{
		goto __TERMINAL;
	}

#ifdef __ETH_DEBUG
	_hx_printf("  ConfigInterface: Try to allocate memory and init it.\r\n");
#endif
	//Allocate a configuration object,it will be released by ethernet core thread.
	pifConfig = (__ETH_IP_CONFIG*)KMemAlloc(sizeof(__ETH_IP_CONFIG), KMEM_SIZE_TYPE_ANY);
	if (NULL == pifConfig)
	{
		goto __TERMINAL;
	}
	memcpy(pifConfig, pConfig, sizeof(__ETH_IP_CONFIG));
	strcpy(pifConfig->ethName, ethName);

#ifdef __ETH_DEBUG
	_hx_printf("  %s: Try to send message to core thread.\r\n", __func__);
#endif	
	//Delivery a message to ethernet core thread.
	msg.wCommand = ETH_MSG_SETCONF;
	msg.wParam = 0;
	msg.dwParam = (DWORD)pifConfig;
	SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg);

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (!pifConfig)
		{
			KMemFree(pifConfig, KMEM_SIZE_TYPE_ANY, 0);
		}
	}
	return bResult;
}

/*
 * Local helper to show all protocol(s) that bound to
 * a specified ethernet interface.
 * This routine is called by ShowIf.
 */
static void ShowBinding(__ETHERNET_INTERFACE* pEthInt)
{
	__NETWORK_PROTOCOL* pProtocol = NULL;
	struct __PROTO_INTERFACE_BIND* pBinding = NULL;
	int i = 0, bind_count = 0;

	BUG_ON(NULL == pEthInt);

	_hx_printf("  Protocol(s) bound:\r\n");
	for (i = 0; i < MAX_BIND_PROTOCOL_NUM; i++)
	{
		pBinding = &pEthInt->Proto_Interface[i];
		if (NULL == pBinding->pProtocol)
		{
			continue;
		}
		BUG_ON(NULL == pBinding->pL3Interface);
		pProtocol = pBinding->pProtocol;
		_hx_printf("    %s\r\n", pProtocol->szProtocolName);
		bind_count++;
	}
	if (0 == bind_count) /* No protocol bound to this interface. */
	{
		_hx_printf("    [No protocol bound]\r\n");
	}
}

//Display all ethernet interface's statistics information.
static VOID ShowInt(char* ethName)
{
	int                    index = 0;
	__ETH_INTERFACE_STATE* pState = NULL;

	if (NULL == ethName)  //Show all interface(s).
	{
		for (index = 0; index < EthernetManager.nIntIndex; index++)
		{
			pState = &EthernetManager.EthInterfaces[index].ifState;
			ShowBinding(&EthernetManager.EthInterfaces[index]);
			_hx_printf("  Statistics information for interface '%s':\r\n",
				EthernetManager.EthInterfaces[index].ethName);
			_hx_printf("    Send frame #       : %u\r\n", pState->dwFrameSend);
			_hx_printf("    Success send #     : %u\r\n", pState->dwFrameSendSuccess);
			_hx_printf("    Send bytes size    : %u\r\n", pState->dwTotalSendSize);
			_hx_printf("    Receive frame #    : %u\r\n", pState->dwFrameRecv);
			_hx_printf("    Success recv #     : %u\r\n", pState->dwFrameRecvSuccess);
			_hx_printf("    Receive bytes size : %u\r\n", pState->dwTotalRecvSize);
			_hx_printf("    Bridged frame #    : %u\r\n", pState->dwFrameBridged);
			_hx_printf("    Tx error number    : %u\r\n", pState->dwTxErrorNum);
			_hx_printf("    Recv mcast frame # : %u\r\n", pState->dwRxMcastNum);
			_hx_printf("    Send mcast frame # : %u\r\n", pState->dwTxMcastNum);
			_hx_printf("    Sending Queue Sz   : %u\r\n",
				EthernetManager.EthInterfaces[index].nSendingQueueSz);
		}
	}
	else //Show a specified ethernet interface.
	{
	}
	return;
}

//Delivery a frame to layer 3 entity,usually it will be used by ethernet driver intertupt handler,
//when a frame is received,and call this routine to delivery the frame.
static BOOL Delivery(__ETHERNET_INTERFACE* pIf, __ETHERNET_BUFFER* pBuff)
{
	return FALSE;
}

//Scan all available APs in case of WiFi.
static BOOL Rescan(char* ethName)
{
	__ETHERNET_INTERFACE*  pEthInt = NULL;
	int                    index = 0;
	BOOL                   bResult = FALSE;

	if (0 == EthernetManager.nIntIndex)  //Have not available interface.
	{
		goto __TERMINAL;
	}
	if (NULL == ethName)
	{
		pEthInt = &EthernetManager.EthInterfaces[0];
	}
	else
	{
		for (index = 0; index < EthernetManager.nIntIndex; index++)
		{
			if (0 == strcmp(ethName, EthernetManager.EthInterfaces[index].ethName))
			{
				pEthInt = &EthernetManager.EthInterfaces[index];
				break;
			}
		}
		if (NULL == pEthInt)
		{
			goto __TERMINAL;
		}
	}
	//OK,we located the proper ethernet interface,now launch rescan command.
	if (pEthInt->IntControl)
	{
		bResult = pEthInt->IntControl(pEthInt, ETH_MSG_SCAN, NULL);
	}

__TERMINAL:
	return bResult;
}

//Associate to a specified WiFi hotspot in case of WLAN.
static BOOL Assoc(char* ethName, __WIFI_ASSOC_INFO* pAssoc)
{
	__ETHERNET_INTERFACE*    pEthInt = NULL;
	BOOL                     bResult = FALSE;
	int                      index = 0;
	__WIFI_ASSOC_INFO*       pAssocInfo = NULL;
	__KERNEL_THREAD_MESSAGE  msg;

	if (NULL == pAssoc)
	{
		goto __TERMINAL;
	}
	//Locate the proper ethernet interface.
	if (NULL == ethName)
	{
		pEthInt = &EthernetManager.EthInterfaces[0];
	}
	else
	{
		for (index = 0; index < EthernetManager.nIntIndex; index++)
		{
			if (0 == strcmp(ethName, EthernetManager.EthInterfaces[index].ethName))
			{
				pEthInt = &EthernetManager.EthInterfaces[index];
				break;
			}
		}
		if (NULL == pEthInt)
		{
			goto __TERMINAL;
		}
	}

	//Issue assoc command to ethernet interface.
	if (pEthInt->IntControl)
	{
		//Allocate a association object and sent to Ethernet core thread,it will be released
		//by ethernet core thread.
		pAssocInfo = (__WIFI_ASSOC_INFO*)KMemAlloc(sizeof(__WIFI_ASSOC_INFO), KMEM_SIZE_TYPE_ANY);
		if (NULL == pAssocInfo)
		{
			goto __TERMINAL;
		}
		memcpy(pAssocInfo, pAssoc, sizeof(__WIFI_ASSOC_INFO));
		pAssocInfo->pPrivate = (LPVOID)pEthInt;
		msg.wCommand = ETH_MSG_ASSOC;
		msg.wParam = 0;
		msg.dwParam = (DWORD)pAssocInfo;
		SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg);
		bResult = TRUE;
		//bResult = pEthInt->IntControl(pEthInt,ETH_MSG_ASSOC,pAssoc);
	}
__TERMINAL:
	return bResult;
}

//Shut down a given ethernet interface.
static BOOL ShutdownInterface(char* ethName)
{
	__ETHERNET_INTERFACE* pEthInt = NULL;
	int index = 0;

	if (NULL == ethName)
	{
		return FALSE;
	}
	if (strlen(ethName) > MAX_ETH_NAME_LEN)
	{
		_hx_printf("  Please specify a valid interface name.\r\n");
		return FALSE;
	}

	//Locate the interface object by it's name.
	for (index = 0; index < MAX_ETH_INTERFACE_NUM; index++)
	{
		if (0 == strcmp(ethName, EthernetManager.EthInterfaces[index].ethName))
		{
			pEthInt = &EthernetManager.EthInterfaces[index];
			break;
		}
	}
	if (NULL == pEthInt)  //Can not find the ethernet interface object.
	{
		_hx_printf("  Please specify a valid interface name.\r\n");
		return FALSE;
	}
	if (pEthInt->ifState.dwInterfaceStatus & ETHERNET_INTERFACE_STATUS_DOWN)  //Already down.
	{
		return FALSE;
	}
	//Try to shutdown the ethernet interface.
	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		if (pEthInt->Proto_Interface[index].pProtocol)
		{
			//Notify the L3 protocol of shutdown.
			((__NETWORK_PROTOCOL*)pEthInt->Proto_Interface[index].pProtocol)->ShutdownInterface(
				pEthInt->Proto_Interface[index].pL3Interface);
		}
	}
	pEthInt->ifState.dwInterfaceStatus = ETHERNET_INTERFACE_STATUS_DOWN;
	return TRUE;
}

//Restart the shut interface.
static BOOL UnshutInterface(char* ethName)
{
	__ETHERNET_INTERFACE* pEthInt = NULL;
	int index = 0;

	if (NULL == ethName)
	{
		return FALSE;
	}
	if (strlen(ethName) > MAX_ETH_NAME_LEN)
	{
		_hx_printf("  Please specify a valid interface name.\r\n");
		return FALSE;
	}

	//Locate the interface object by it's name.
	for (index = 0; index < MAX_ETH_INTERFACE_NUM; index++)
	{
		if (0 == strcmp(ethName, EthernetManager.EthInterfaces[index].ethName))
		{
			pEthInt = &EthernetManager.EthInterfaces[index];
			break;
		}
	}
	if (NULL == pEthInt)  //Can not find the ethernet interface object.
	{
		_hx_printf("  Please specify a valid interface name.\r\n");
		return FALSE;
	}
	if (pEthInt->ifState.dwInterfaceStatus & ETHERNET_INTERFACE_STATUS_UP)  //Already up.
	{
		return FALSE;
	}
	//Try to shutdown the ethernet interface.
	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		if (pEthInt->Proto_Interface[index].pProtocol)
		{
			//Notify the L3 protocol of shutdown.
			((__NETWORK_PROTOCOL*)pEthInt->Proto_Interface[index].pProtocol)->UnshutdownInterface(
				pEthInt->Proto_Interface[index].pL3Interface);
		}
	}
	pEthInt->ifState.dwInterfaceStatus = ETHERNET_INTERFACE_STATUS_UP;
	return TRUE;
}

//Return a specified ethernet interface's state.@nIndex parameter specifies the interface
//index which state will be returned,and @pnNextInt contains the next interface index which
//can be used as nIndex parameter when next call of this routine is invoked.
static BOOL _GetEthernetInterfaceState(__ETH_INTERFACE_STATE* pState, int nIndex, int* pnNextInt)
{
	BOOL bResult = FALSE;

	if ((NULL == pState) || (nIndex >= MAX_ETH_INTERFACE_NUM))
	{
		goto __TERMINAL;
	}

	memcpy(pState, &EthernetManager.EthInterfaces[nIndex].ifState, sizeof(*pState));
	//Return the next interface's index whose state can be got.
	if (pnNextInt)
	{
		if (nIndex + 1 >= MAX_ETH_INTERFACE_NUM)
		{
			*pnNextInt = MAX_ETH_INTERFACE_NUM;
		}
		else
		{
			*pnNextInt = nIndex + 1;
		}
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

//Create an Ethernet Buffer object and return it,NULL will be returned
//if failed to create.
//buff_length will be skiped in current implementation,since the Ethernet Buffer
//use the MTU as default buffer size.
static __ETHERNET_BUFFER* _CreateEthernetBuffer(int buff_length)
{
	__ETHERNET_BUFFER* pEthBuff = NULL;

	pEthBuff = (__ETHERNET_BUFFER*)_hx_malloc(sizeof(__ETHERNET_BUFFER));
	if (NULL == pEthBuff)
	{
		_hx_printf("  %s: can not allocate Ethernet Buffer object.\r\n", __func__);
		goto __TERMINAL;
	}
	//Create OK,initialize it.
	pEthBuff->pNext = NULL;
	pEthBuff->dwSignature = KERNEL_OBJECT_SIGNATURE;
	pEthBuff->act_length = buff_length;
	pEthBuff->buff_length = ETH_DEFAULT_MTU + ETH_HEADER_LEN;
	pEthBuff->frame_type = 0;
	pEthBuff->buff_status = ETHERNET_BUFFER_STATUS_FREE;
	//pEthBuff->pEthernetInterface = NULL;
	pEthBuff->pInInterface = NULL;
	pEthBuff->pOutInterface = NULL;
	memset(pEthBuff->srcMAC, 0, sizeof(pEthBuff->srcMAC));
	memset(pEthBuff->dstMAC, 0, sizeof(pEthBuff->dstMAC));
	__ATOMIC_INCREASE(&EthernetManager.nTotalEthernetBuffs);

__TERMINAL:
	return pEthBuff;
}

//Clone a new ethernet buffer by giving a existing one.
static __ETHERNET_BUFFER* _CloneEthernetBuffer(__ETHERNET_BUFFER* pEthBuff)
{
	__ETHERNET_BUFFER* pNewEthBuff = NULL;

	if (NULL == pEthBuff)
	{
		goto __TERMINAL;
	}
	//Buffer length should be fixed as ETH_DEFAULT_MTU + ETH_HEADER_LEN currently.
	if ((ETH_DEFAULT_MTU + ETH_HEADER_LEN) != pEthBuff->buff_length)
	{
		BUG();
	}
	if (KERNEL_OBJECT_SIGNATURE != pEthBuff->dwSignature)
	{
		BUG();
	}

	pNewEthBuff = (__ETHERNET_BUFFER*)_hx_malloc(sizeof(__ETHERNET_BUFFER));
	if (NULL == pNewEthBuff)
	{
		goto __TERMINAL;
	}
	memcpy(pNewEthBuff, pEthBuff, sizeof(__ETHERNET_BUFFER));
	/*
	* Reset pNext pointer.
	*/
	pNewEthBuff->pNext = NULL;

	/*
	* Update total ethernet buffer counter.
	*/
	__ATOMIC_INCREASE(&EthernetManager.nTotalEthernetBuffs);

__TERMINAL:
	return pNewEthBuff;
}

//Destroy a specified Ethernet Buffer object.
static VOID _DestroyEthernetBuffer(__ETHERNET_BUFFER* pEthBuff)
{
	if (NULL == pEthBuff)
	{
		return;
	}
	BUG_ON(KERNEL_OBJECT_SIGNATURE != pEthBuff->dwSignature);
	//Buffer length should be fixed as ETH_DEFAULT_MTU + ETH_HEADER_LEN currently.
	BUG_ON((ETH_DEFAULT_MTU + ETH_HEADER_LEN) != pEthBuff->buff_length);

	/*
	 * Clear all object key memeber's value before release,since it will
	 * trigger assert mechanism in case of bug,such as a destroyed ethernet buffer
	 * is miss used by invalid pointer.
	 */
	pEthBuff->act_length = 0;
	pEthBuff->buff_length = 0;
	pEthBuff->dwSignature = 0;
	pEthBuff->pNext = NULL;
	pEthBuff->pInInterface = pEthBuff->pOutInterface = NULL;

	_hx_free(pEthBuff);

	/*
	* Decrease total ethernet buffer number in system.
	*/
	__ATOMIC_DECREASE(&EthernetManager.nTotalEthernetBuffs);
}

static BOOL __BroadcastEthernetFrame(__ETHERNET_BUFFER* pBuffer)
{
	DWORD dwFlags;
	__KERNEL_THREAD_MESSAGE msg;

	if (NULL == pBuffer)
	{
		return FALSE;
	}
	if (KERNEL_OBJECT_SIGNATURE != pBuffer->dwSignature)
	{
		BUG();
	}

	/*
	* Reset the next pointer,since it maybe not NULL.
	*/
	pBuffer->pNext = NULL;

	__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
	if (NULL == EthernetManager.pBroadcastFirst) /* No thread frame in list yet. */
	{
		if (NULL != EthernetManager.pBroadcastLast)
		{
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			BUG();
		}
		//Link the buffer object to list before send POSTFRAME message.
		EthernetManager.pBroadcastFirst = pBuffer;
		EthernetManager.pBroadcastLast = pBuffer;
		EthernetManager.nBroadcastSize += 1;
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);

		//Then send the post frame message to ethernet core thread,make it noticed to
		//process the incoming packet.
		//Since it may fail,we will give up in this case...
		msg.wCommand = ETH_MSG_BROADCAST;
		msg.wParam = 0;
		msg.dwParam = (DWORD)pBuffer;
		if (!SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg))
		{
			__ENTER_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			EthernetManager.pBroadcastFirst = NULL;
			EthernetManager.pBroadcastLast = NULL;
			EthernetManager.nBroadcastSize -= 1;
			EthernetManager.nDropedBcastSize += 1;
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			return FALSE;
		}
	}
	else
	{
		if (NULL == EthernetManager.pBroadcastLast)
		{
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			BUG();
		}
		//Exceed the maximal buffer list size.
		if (EthernetManager.nBroadcastSize > MAX_ETH_BCASTQUEUESZ)
		{
			EthernetManager.nDropedBcastSize += 1;
			__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
			return FALSE;
		}
		EthernetManager.pBroadcastLast->pNext = pBuffer;
		EthernetManager.pBroadcastLast = pBuffer;
		EthernetManager.nBroadcastSize += 1;
		__LEAVE_CRITICAL_SECTION_SMP(EthernetManager.spin_lock, dwFlags);
	}
	return TRUE;
}

static __ETHERNET_INTERFACE* GetEthernetInterface(char* name)
{
	__ETHERNET_INTERFACE* pEthInt = NULL;
	int i = 0;

	BUG_ON(NULL == name);

	for (i = 0; i < MAX_ETH_INTERFACE_NUM; i++)
	{
		pEthInt = &EthernetManager.EthInterfaces[i];
		if (strcmp(pEthInt->ethName, name) == 0)
		{
			break;
		}
	}
	if (i == MAX_ETH_INTERFACE_NUM) /* Ethernet interface can not be found. */
	{
		return NULL;
	}
	return pEthInt;
}

static VOID ReleaseEthernetInterface(__ETHERNET_INTERFACE* pEthInt)
{
	return;
}

/*
*
*  Definition of Ethernet Manager object.
*
*/
struct __ETHERNET_MANAGER EthernetManager = {
	{ 0 },                  //Ethernet interface array.
	0,                      //Index of free slot.
	NULL,                   //Handle of ethernet core thread.
	FALSE,                  //Not initialized yet.
	NULL,                   //Buffer list header.
	NULL,                   //Buffer list tail.
	0,                      //nBuffListSize.
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,   //spin_lock.
#endif

	NULL,                   //Broadcast list header.
	NULL,                   //Broadcast list tail.
	0,                      //nBroadcastSize.
	0,                      //nDropedBcastSize.

	ATOMIC_INIT_VALUE,      //nTotalEthernetBuffs.
	ATOMIC_INIT_VALUE,      //nDrvSendingQueueSz.

	Initialize,               //Initialize.
	AddEthernetInterface,     //AddEthernetInterface.
	GetEthernetInterface,     //GetEthernetInterface.
	ReleaseEthernetInterface, //ReleaseEthernetInterface.
	DeleteEthernetInterface,  //DeleteEthernetInterface.
	ConfigInterface,          //ConfigInterface.
	Rescan,                   //Rescan.
	Assoc,                    //Assoc.
	Delivery,                 //Delivery.
	SendFrame,                //SendFrame.
	__BroadcastEthernetFrame, //BroadcastEthernetFrame.
	_TriggerReceive,          //TriggerReceive.
	_PostFrame,               //PostFrame.
	ShowInt,                  //ShowInt.
	ShutdownInterface,        //ShutdownInterface.
	UnshutInterface,          //UnshutInterface.
	_GetEthernetInterfaceState,    //GetEthernetInterfaceState.
	_CreateEthernetBuffer,         //CreateEthernetBuffer.
	_CloneEthernetBuffer,          //CloneEthernetBuffer.
	_DestroyEthernetBuffer         //DestroyEthernetBuffer.
};
