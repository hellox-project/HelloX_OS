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

#include "ethmgr.h"
#include "proto.h"

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
static void netifConfig(__ETHERNET_INTERFACE* pif,UCHAR proto, __ETH_INTERFACE_STATE* pifState, __ETH_IP_CONFIG* pifConfig)
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
		_hx_printf("  The interface [%d] is not bound to protocol [%].\r\n",
			pif->ethName, proto);
		return;
	}

#ifdef __ETH_DEBUG
	_hx_printf("  %s: Begin to compare flags and locate proper routine.\r\n",__func__);
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
		dhcpRestart(pif,pProtocol, pifState);
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
			//_hx_printf("  %s: Received ethernet frame,type = %X,length = %d.\r\n",
			//	__func__,p->frame_type, p->act_length);
			//Update interface statistics.
			pEthInt->ifState.dwFrameRecv++;
			pEthInt->ifState.dwTotalRecvSize += p->act_length;
			//Delivery the frame to layer 3.
			for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
			{
				if (pEthInt->Proto_Interface[index].frame_type_mask & p->frame_type)
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
				_hx_printf("Config IP address on ethernet interface [%s] OK.\r\n", pEthInt->ethName);
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
	if (pEthInt != pBuffer->pEthernetInterface)
	{
		return FALSE;
	}
	if (pBuffer->pNext != NULL)
	{
		return FALSE;
	}
	//Link the ethernet buffer object to list,and send a message to ethernet core
	//thread if is the fist buffer object.
	__ENTER_CRITICAL_SECTION(NULL, dwFlags);
	if (NULL == EthernetManager.pBufferFirst)
	{
		//We send the post frame message to ethernet core thread first,
		//since it may fail,we will give up in this case...
		msg.wCommand = ETH_MSG_POSTFRAME;
		msg.wParam = 0;
		msg.dwParam = (DWORD)pBuffer;
		if (!SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg))
		{
			__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
			return FALSE;
		}
		//Link the buffer object to list.
		EthernetManager.pBufferFirst = pBuffer;
		EthernetManager.pBufferLast = pBuffer;
		EthernetManager.nBuffListSize += 1;
	}
	else
	{
		if (NULL == EthernetManager.pBufferLast)
		{
			__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
			BUG();
		}
		//Exceed the maximal buffer list size.
		if (EthernetManager.nBuffListSize > MAX_ETH_RXBUFFLISTSZ)
		{
			__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
			return FALSE;
		}
		EthernetManager.pBufferLast->pNext = pBuffer;
		EthernetManager.pBufferLast = pBuffer;
		EthernetManager.nBuffListSize += 1;
	}
	__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
	return TRUE;
}

//Handler the Post Frame message.
static BOOL _PostFrameHandler()
{
	__ETHERNET_BUFFER* pBuffer = NULL;
	__ETHERNET_INTERFACE* pEthInt = NULL;
	__NETWORK_PROTOCOL* pProtocol = NULL;
	BOOL bDeliveryResult = FALSE;
	int index = 0;
	DWORD dwFlags;

	while (TRUE)
	{
		//Get one ethernet buffer from list.
		__ENTER_CRITICAL_SECTION(NULL, dwFlags);
		if (NULL == EthernetManager.pBufferFirst)  //No frame pending.
		{
			if (EthernetManager.pBufferLast)  //Check again.
			{
				__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
				BUG();
				return FALSE;
			}
			__LEAVE_CRITICAL_SECTION(NULL, dwFlags);
			return FALSE;
		}
		pBuffer = EthernetManager.pBufferFirst;
		EthernetManager.pBufferFirst = pBuffer->pNext;
		if (NULL == pBuffer->pNext)
		{
			EthernetManager.pBufferLast = NULL;
		}
		EthernetManager.nBuffListSize -= 1;
		if (EthernetManager.nBuffListSize < 0)
		{
			BUG();
		}
		__LEAVE_CRITICAL_SECTION(NULL, dwFlags);

		//Update interface statistics.
		pEthInt = pBuffer->pEthernetInterface;
		if (NULL == pEthInt)  //Should not occur.
		{
			BUG();
			return FALSE;
		}
		pEthInt->ifState.dwFrameRecv++;
		pEthInt->ifState.dwTotalRecvSize += pBuffer->act_length;
		//Delivery the frame to layer 3.
		for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
		{
			if (pEthInt->Proto_Interface[index].frame_type_mask & pBuffer->frame_type)
			{
				pProtocol = pEthInt->Proto_Interface[index].pProtocol;
				if (NULL == pProtocol)
				{
					BUG();
				}
				bDeliveryResult = pProtocol->DeliveryFrame(pBuffer, pEthInt->Proto_Interface[index].pL3Interface);
				if (bDeliveryResult)
				{
					pEthInt->ifState.dwFrameRecvSuccess++;
					break;
				}
			}
		}
		//Release the Ethernet Buffer object.
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
				netifConfig(pEthInt,pifConfig->protoType,pifState, pifConfig);
				KMemFree(pifConfig, KMEM_SIZE_TYPE_ANY, 0);
				break;

			case ETH_MSG_SEND:                //Send a link level frame.
				pEthBuffer = (__ETHERNET_BUFFER*)msg.dwParam;
				if (NULL == pEthBuffer)  //Use the default send buffer.
				{
					BUG();
					break;
				}
				pEthInt = (__ETHERNET_INTERFACE*)pEthBuffer->pEthernetInterface;
				if (NULL == pEthInt)
				{
					BUG();
					break;
				}
				if (NULL == pEthInt->SendFrame)
				{
					BUG();
					break;
				}
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
				if (pEthInt->SendFrame(pEthInt))
				{
					//Update statistics info.
					pEthInt->ifState.dwFrameSendSuccess += 1;
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

	//Create the ethernet core thread.
	EthernetManager.EthernetCoreThread = KernelThreadManager.CreateKernelThread(
		(__COMMON_OBJECT*)&KernelThreadManager,
		0,                           //Use default stack size.
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,       //Normal priority.
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
		if (pEthBuff->act_length > pEthBuff->buff_length)
		{
			BUG();
		}
		pEthBuff->pEthernetInterface = pEthInt;
		msg.dwParam = (DWORD)pEthBuff;
		pSendBuff = pEthBuff;
	}
	else  //Use the embedded Ethernet Buffer object in Ethernet Interface.
	{
		//Validate the default Ethernet Buffer object.
		if (pEthInt->SendBuffer.act_length > pEthInt->SendBuffer.buff_length)
		{
			BUG();
		}
		pEthInt->SendBuffer.pEthernetInterface = pEthInt;
		msg.dwParam = (DWORD)(&pEthInt->SendBuffer);
		pSendBuff = &pEthInt->SendBuffer;
	}
	//Do some checks before send...
	if (pSendBuff->act_length >= ETH_DEFAULT_MTU + ETH_HEADER_LEN)
	{
		_hx_printf("%s:frame length[%d] may exceed.\r\n", pSendBuff->act_length);
	}
	//_hx_printf("%s:send out a frame with length = %d.\r\n", __func__,
	//	pSendBuff->act_length);
	SendMessage((HANDLE)EthernetManager.EthernetCoreThread, &msg);
	return TRUE;
}

//Implementation of AddEthernetInterface,which is called by Ethernet Driver to register an interface
//object.
static __ETHERNET_INTERFACE* AddEthernetInterface(char* ethName,char* mac,LPVOID pIntExtension,
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
	pEthInt->SendBuffer.pEthernetInterface = NULL;
	pEthInt->SendBuffer.pNext = NULL;

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

	//Restart DHCP process on the ethernet interface for each L3 protocol.
	for (index = 0; index < MAX_BIND_PROTOCOL_NUM; index++)
	{
		dhcpRestart(pEthInt, pEthInt->Proto_Interface[index].pProtocol, &pEthInt->ifState);
	}

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
	_hx_printf("  %s: Try to send message to core thread.\r\n",__func__);
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
			_hx_printf("  \r\n");
			_hx_printf("  Statistics information for interface '%s':\r\n",
				EthernetManager.EthInterfaces[index].ethName);
			_hx_printf("    Send frame #       : %d\r\n", pState->dwFrameSend);
			_hx_printf("    Success send #     : %d\r\n", pState->dwFrameSendSuccess);
			_hx_printf("    Send bytes size    : %d\r\n", pState->dwTotalSendSize);
			_hx_printf("    Receive frame #    : %d\r\n", pState->dwFrameRecv);
			_hx_printf("    Success recv #     : %d\r\n", pState->dwFrameRecvSuccess);
			_hx_printf("    Receive bytes size : %d\r\n", pState->dwTotalRecvSize);
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
	pEthBuff->act_length = buff_length;
	pEthBuff->buff_length = ETH_DEFAULT_MTU + ETH_HEADER_LEN;
	pEthBuff->frame_type = 0;
	pEthBuff->buff_status = ETHERNET_BUFFER_STATUS_FREE;
	pEthBuff->pEthernetInterface = NULL;
	memset(pEthBuff->srcMAC, 0, sizeof(pEthBuff->srcMAC));
	memset(pEthBuff->dstMAC, 0, sizeof(pEthBuff->dstMAC));

__TERMINAL:
	return pEthBuff;
}

//Destroy a specified Ethernet Buffer object.
static VOID _DestroyEthernetBuffer(__ETHERNET_BUFFER* pEthBuff)
{
	if (NULL == pEthBuff)
	{
		return;
	}
	//Buffer length should be fixed as ETH_DEFAULT_MTU + ETH_HEADER_LEN currently.
	if ((ETH_DEFAULT_MTU + ETH_HEADER_LEN)!= pEthBuff->buff_length)
	{
		BUG();
		return;
	}
	//Release it.
	_hx_free(pEthBuff);
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

	Initialize,             //Initialize.
	AddEthernetInterface,   //AddEthernetInterface.
	ConfigInterface,        //ConfigInterface.
	Rescan,                 //Rescan.
	Assoc,                  //Assoc.
	Delivery,               //Delivery.
	SendFrame,              //SendFrame.
	_TriggerReceive,        //TriggerReceive.
	_PostFrame,             //PostFrame.
	ShowInt,                //ShowInt.
	ShutdownInterface,      //ShutdownInterface.
	UnshutInterface,        //UnshutInterface.
	_GetEthernetInterfaceState,    //GetEthernetInterfaceState.
	_CreateEthernetBuffer,         //CreateEthernetBuffer.
	_DestroyEthernetBuffer         //DestroyEthernetBuffer.
};
