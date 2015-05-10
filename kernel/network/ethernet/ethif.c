//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : ethif.c
//    Module Funciton           : 
//                                This module countains HelloX ethernet skeleton's
//                                implementation code.
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

#include "ethif.h"
#include "netif/etharp.h"

//A helper routine to refresh DHCP configurations of the given interface.
static void dhcpRestart(struct netif* pif,__ETH_INTERFACE_STATE* pifState)
{
#ifdef __ETH_DEBUG
	_hx_printf("  dhcpRestart: Begin to entry.\r\n");
#endif
	if(!pifState->bDhcpCltEnabled)  //DHCP is not enabled.
	{
#ifdef __ETH_DEBUG
		_hx_printf("  dhcpRestart: Try to start DHCP on interface.\r\n");
#endif
		pifState->bDhcpCltEnabled = TRUE;
		pifState->bDhcpCltOK      = FALSE;
		//start DHCP on 
		dhcp_start(pif);
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
		dhcp_release(pif);
		dhcp_stop(pif);
		//Re-enable the DHCP client on the given interface.
		pifState->bDhcpCltOK = FALSE;
		dhcp_start(pif);
		return;
	}
#ifdef __ETH_DEBUG
	_hx_printf("  dhcpRestart: End of routine.\r\n");
#endif
}

//Configure a given interface by applying the given parameters.
static void netifConfig(struct netif* pif,__ETH_INTERFACE_STATE* pifState,__ETH_IP_CONFIG* pifConfig)
{
	if((NULL == pif) || (NULL == pifState) || (NULL == pifConfig))
	{
		return;
	}
#ifdef __ETH_DEBUG
	_hx_printf("  netifConfig: Begin to compare flags and locate proper routine.\r\n");
#endif
	//Configure the interface according different DHCP flags.
	if(pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_DISABLE)
	{
		//Disable DHCP on the given interface.
		if(pifState->bDhcpCltEnabled)
		{
			if(pifState->bDhcpCltOK)  //Release configurations first.
			{
				dhcp_release(pif);
				pifState->bDhcpCltOK = FALSE;
			}
			dhcp_stop(pif);
			pifState->bDhcpCltEnabled = FALSE;
		}
		//Configure static IP address on the interface.
		netif_set_down(pif);
		netif_set_addr(pif,&pifConfig->ipaddr,
		    &pifConfig->mask,&pifConfig->defgw);
		netif_set_up(pif);
		//Save new configurations to interface state.
		memcpy(&pifState->IpConfig,pifConfig,sizeof(__ETH_IP_CONFIG));
		_hx_printf("\r\n  DHCP on interface [%s] is disabled and IP address is set.\r\n",pifConfig->ethName);
	}
	if(pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_ENABLE)
	{
		pifState->bDhcpCltEnabled = TRUE;
	}
	if(pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RESTART)
	{
#ifdef __ETH_DEBUG
		_hx_printf("  netifConfig: DHCP Restart is required.\r\n");
#endif
		//pifState->bDhcpCltEnabled = TRUE;
		dhcpRestart(pif,pifState);
		_hx_printf("\r\n  DHCP restarted on interface [%s].\r\n",pifConfig->ethName);
	}
	if(pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RENEW)
	{
		if(pifState->bDhcpCltEnabled)
		{
			dhcp_renew(pif);
			pifState->bDhcpCltOK = FALSE;
		}
	}
	if(pifConfig->dwDHCPFlags & ETH_DHCPFLAGS_RELEASE)
	{
		if(pifState->bDhcpCltEnabled)
		{
			dhcp_release(pif);
			pifState->bDhcpCltOK = FALSE;
		}
	}
	return;
}

/* 
*
* Implementation of Ethernet Manager object,the core object of HelloX's ethernet framework.
*
*/

//A helper routine,to poll all ethernet interface(s) to check if there is frame availabe,
//and delivery it to layer 3 if so.
static void _ethernet_if_input()
{
	__ETHERNET_INTERFACE*  pEthInt  = NULL;
  struct pbuf*           p        = NULL;
	struct netif*          netif    = NULL;
	err_t                  err      = 0;
	int                    index    = 0;
	
	for(index = 0;index < EthernetManager.nIntIndex;index ++)
	{
		pEthInt = &EthernetManager.EthInterfaces[index];
		if(pEthInt->RecvFrame)
		{
			netif = (struct netif*)pEthInt->pL3Interface;
			if(NULL == netif)  //Should not occur.
			{
				BUG();
			}
			while(TRUE)
			{
				p = pEthInt->RecvFrame(pEthInt);
				if(NULL == p)  //No available frames.
				{
					break;
				}
				//Update interface statistics.
				pEthInt->ifState.dwFrameRecv ++;
				pEthInt->ifState.dwTotalRecvSize += p->tot_len;
				//Delivery the frame to layer 3.
				err = netif->input(p, netif);
				if(err != ERR_OK)
				{
#ifdef __ETH_DEBUG
					_hx_printf("  _ethernet_if_input: Can not delivery [%s]'s frame to IP,err = %d.\r\n",
					  pEthInt->ethName,err);
#endif
					pbuf_free(p);
					p = NULL;
				}
				else
				{
					pEthInt->ifState.dwFrameRecvSuccess ++;
				}
			}
		}
	}
}

//A helper routine to check assist the DHCP process.It checks if the DHCP
//process is successful,and do proper actions(such as set the offered IP
//address to interface) according DHCP status.
static void _dhcpAssist()
{
	__ETHERNET_INTERFACE*    pEthInt  = NULL;
	struct netif*            netif    = NULL;
	int                      index    = 0;
	
	for(index = 0;index < EthernetManager.nIntIndex;index ++)
	{
		pEthInt = &EthernetManager.EthInterfaces[index];
		netif   = (struct netif*)pEthInt->pL3Interface;
		if(NULL == netif)
		{
			BUG();
		}
		if(!pEthInt->ifState.bDhcpCltEnabled)  //DHCP client is disabled.
		{
			continue;
		}
		if(pEthInt->ifState.bDhcpCltOK)  //Already obtained configurations.
		{
			pEthInt->ifState.dwDhcpLeasedTime += WIFI_POLL_TIME;  //Just increment leased time.
			continue;
		}
		//Now try to check if we get the valid configuration from DHCP server,which
		//stored in netif->dhcp by DHCP client function,implemented in lwIP.
		if(netif->dhcp)
		{
			if((netif->dhcp->offered_ip_addr.addr != 0) &&
				(netif->dhcp->offered_gw_addr.addr != 0) &&
			  (netif->dhcp->offered_sn_mask.addr != 0))
			{
				pEthInt->ifState.bDhcpCltOK = TRUE;
				//Set DHCP server offered IP configurations to interface.
				netif_set_down(netif);
				netif_set_addr(netif,&netif->dhcp->offered_ip_addr,&netif->dhcp->offered_sn_mask,&netif->dhcp->offered_gw_addr);
				netif_set_up(netif);
#ifdef __ETH_DEBUG
				_hx_printf("  _dhcpAssist: Get interface [%s]'s configuration from DHCP server.\r\n",pEthInt->ethName);
#endif
				//Stop DHCP process in the interface.
				dhcp_stop(netif);
				pEthInt->ifState.dwDhcpLeasedTime = 0;  //Start to count DHCP time.
			}
		}
	}
	return;
}

//Dedicated Ethernet core thread,repeatly to poll all ethernet interfaces to receive frame,if
//interrupt mode is not supported by ethernet driver,and do some other functions.
static DWORD EthCoreThreadEntry(LPVOID pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	HANDLE                  hTimer = NULL;           //Handle of receiving poll timer.
	struct netif*           pif        = (struct netif*)pData;
	__ETHERNET_INTERFACE*   pEthInt    = NULL;
	__WIFI_ASSOC_INFO*      pAssocInfo = NULL;
	__IF_PBUF_ASSOC*        pIfPbuf    = NULL;
	__ETH_IP_CONFIG*        pifConfig  = NULL;
	__ETH_INTERFACE_STATE*  pifState   = NULL;
	int                     tot_len    = 0;
	int                     index      = 0;
	
	//Initialize all ethernet driver(s) registered in system.
#ifdef __ETH_DEBUG
	_hx_printf("  Ethernet Manager: Begin to load ethernet drivers...\r\n");
#endif
	while(EthernetDriverEntry[index].EthEntryPoint)
	{
		if(!EthernetDriverEntry[index].EthEntryPoint(EthernetDriverEntry[index].pData))
		{
#ifdef __ETH_DEBUG
			_hx_printf("  Ethernet Manager: Initialize ethernet driver failed[%d].\r\n",index);
#endif
		}
		index ++;
	}
	index = 0;

	//Set the receive polling timer.
	hTimer = SetTimer(WIFI_TIMER_ID,WIFI_POLL_TIME,NULL,NULL,TIMER_FLAGS_ALWAYS);
	if(NULL == hTimer)
	{
		goto __TERMINAL;
	}
	
	//Main message loop.
	while(TRUE)
	{
		if(GetMessage(&msg))
		{
			switch(msg.wCommand)
			{
				case KERNEL_MESSAGE_TERMINAL:
					goto __TERMINAL;
				
				case ETH_MSG_SETCONF:             //Configure a given interface.
					pifConfig = (__ETH_IP_CONFIG*)msg.dwParam;
          if(NULL == pifConfig)
          {
						break;
          }
					//Locate the ethernet interface object by it's name.
					pif = NULL;
					for(index = 0;index < EthernetManager.nIntIndex;index ++)
					{
						if(0 == strcmp(pifConfig->ethName,EthernetManager.EthInterfaces[index].ethName))
						{
							pif = (struct netif*)EthernetManager.EthInterfaces[index].pL3Interface;
							if(NULL == pif)  //Should not occur.
							{
								BUG();
							}
							pifState = &EthernetManager.EthInterfaces[index].ifState;
							break;
						}
					}
					if(NULL == pif)  //Not find.
					{
						KMemFree(pifConfig,KMEM_SIZE_TYPE_ANY,0);
						break;
					}
					//Commit the configurations to interface.
					netifConfig(pif,pifState,pifConfig);
					KMemFree(pifConfig,KMEM_SIZE_TYPE_ANY,0);
					break;
				
				case ETH_MSG_SEND:                //Send a link level frame.
					pIfPbuf = (__IF_PBUF_ASSOC*)msg.dwParam;
				  if(NULL == pIfPbuf)
					{
						break;
					}
					pEthInt = (__ETHERNET_INTERFACE*)pIfPbuf->pnetif->state;
					if(NULL == pEthInt)
					{
						BUG();
						break;
					}
					if(NULL == pEthInt->SendFrame)
					{
						BUG();
						break;
					}
					tot_len = pEthInt->buffSize;
					//Now try to send the frame.
					if(pEthInt->SendFrame(pEthInt))
					{
						//Update statistics info.
						pEthInt->ifState.dwFrameSend += 1;
						pEthInt->ifState.dwTotalSendSize += tot_len;
					}
					KMemFree(pIfPbuf,KMEM_SIZE_TYPE_ANY,0);
					break;
					
				case ETH_MSG_RECEIVE:              //Receive frame,may triggered by interrupt.
				case KERNEL_MESSAGE_TIMER:
					if(WIFI_TIMER_ID == msg.dwParam) //Must match the receiving timer ID.
					{
						_ethernet_if_input();
					}
					_dhcpAssist();        //Call DHCP assist function routinely.
					break;
					
				case ETH_MSG_DELIVER:  //Delivery a packet.
					break;
				case ETH_MSG_ASSOC:
					pAssocInfo = (__WIFI_ASSOC_INFO*)msg.dwParam;
				  if(NULL == pAssocInfo->pPrivate)
					{
						break;
					}
					pEthInt = (__ETHERNET_INTERFACE*)pAssocInfo->pPrivate;
					if(pEthInt->IntControl)
					{
						pEthInt->IntControl(pEthInt,ETH_MSG_ASSOC,pAssocInfo);
					}
					//Release the association object,which is allocated in Assoc
					//function.
					KMemFree(pAssocInfo,KMEM_SIZE_TYPE_ANY,0);
					break;
				default:
					break;
			}
		}
	}
	
__TERMINAL:
	if(hTimer)  //Should cancel it.
	{
		CancelTimer(hTimer);
	}
	return 0;
}

//Initializer of Ethernt Manager object.
static BOOL Initialize(struct __ETHERNET_MANAGER* pManager)
{
	BOOL      bResult           = FALSE;
	
	if(NULL == pManager)
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
	if(NULL == EthernetManager.EthernetCoreThread)  //Can not create the core thread.
	{
		goto __TERMINAL;
	}
	
	pManager->bInitialized = TRUE;
	bResult = TRUE;
	
__TERMINAL:
	return bResult;
}

//Send out a ethernet frame through the specified ethernet interface.
static BOOL SendFrame(__ETHERNET_INTERFACE* pEthInt,struct pbuf* p)
{
	__IF_PBUF_ASSOC*    pAssoc    = NULL;
	BOOL                bResult   = FALSE;
	struct pbuf*        q         = NULL;
	__KERNEL_THREAD_MESSAGE msg;
	int                 l         = 0;
	
	if((NULL == pEthInt) || (NULL == p))
	{
		goto __TERMINAL;
	}
	pAssoc = (__IF_PBUF_ASSOC*)KMemAlloc(sizeof(__IF_PBUF_ASSOC),KMEM_SIZE_TYPE_ANY);
	if(NULL == pAssoc)
	{
		goto __TERMINAL;
	}
	pAssoc->p      = p;
	pAssoc->pnetif = (struct netif*)pEthInt->pL3Interface;
	if(NULL == pAssoc->pnetif)
	{
		BUG();
	}
	
	//Copy the pbuf's content into inteface's sending buffer.
	if(p->tot_len > ETH_DEFAULT_MTU)
	{
		goto __TERMINAL;
	}
	for(q = p; q != NULL; q = q->next)
	{
		memcpy(pEthInt->SendBuff + l, q->payload, q->len);
		l+= (int)q->len;
	}
	pEthInt->buffSize = l;
	
	//Send a message to the ethernet core thread.
	msg.wCommand  = ETH_MSG_SEND;
	msg.wParam    = 0;
	msg.dwParam   = (DWORD)pAssoc;
	SendMessage((HANDLE)EthernetManager.EthernetCoreThread,&msg);
	bResult = TRUE;
	
__TERMINAL:
	return bResult;
}

//A helper routine called by lwIP,to send out an ethernet frame in a given interface.
static err_t eth_level_output(struct netif* netif,struct pbuf* p)
{
	__ETHERNET_INTERFACE*   pEthInt = NULL;
	
	if((NULL == netif) || (NULL == p))
	{
		return !ERR_OK;
	}
	pEthInt = (__ETHERNET_INTERFACE*)netif->state;
	if(NULL == pEthInt)  //Should not occur.
	{
		BUG();
	}
	
	if(EthernetManager.SendFrame(pEthInt,p))
	{
		return ERR_OK;
	}
	return !ERR_OK;
}

//A local helper routine,called by lwIP to initialize a netif object.
static err_t _ethernet_if_init(struct netif *netif)
{
	__ETHERNET_INTERFACE*   pEthInt     = NULL;
	
	if(NULL == netif)
	{
		return !ERR_OK;
	}
	pEthInt = netif->state;
	if(NULL == pEthInt)
	{
		return !ERR_OK;
	}
	
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
  netif->name[0] = pEthInt->ethName[0];
  netif->name[1] = pEthInt->ethName[1];
	
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output       = etharp_output;
	netif->hwaddr_len   = ETH_MAC_LEN;
  netif->linkoutput   = eth_level_output;
	
  /* maximum transfer unit */
  netif->mtu          = 1500;
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags        = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	//Set the MAC address of this interface.
	memcpy(netif->hwaddr,pEthInt->ethMac,ETH_MAC_LEN);
	
  return ERR_OK;
}

//Implementation of AddEthernetInterface,which is called by Ethernet Driver to register an interface
//object.
static __ETHERNET_INTERFACE*   AddEthernetInterface(char* ethName,
                                                  char* mac,
	                                                LPVOID pIntExtension,
	                                                __ETHOPS_INITIALIZE Init,
	                                                __ETHOPS_SEND_FRAME SendFrame,
	                                                __ETHOPS_RECV_FRAME RecvFrame,
	                                                __ETHOPS_INT_CONTROL IntCtrl)
{
	__ETHERNET_INTERFACE*        pEthInt            = NULL;
	BOOL                         bResult            = FALSE;
	int                          index              = 0;
	struct netif*                pIf                = NULL;
	BOOL                         bDefaultInt        = FALSE;       //If the added interface is default one.
	
	if((NULL == ethName) || (NULL == SendFrame) || (NULL == mac))  //Name and send operation are mandatory.
	{
		goto __TERMINAL;
	}
	if(!EthernetManager.bInitialized)  //Ethernet Manager is not initialized yet.
	{
		goto __TERMINAL;
	}
	if(EthernetManager.nIntIndex >= MAX_ETH_INTERFACE_NUM) //No available interface slot.
	{
		goto __TERMINAL;
	}
	
	//Get the free ethernet interface object from array.
	pEthInt = &EthernetManager.EthInterfaces[EthernetManager.nIntIndex];
	if(0 == EthernetManager.nIntIndex)  //First interface,set to default.
	{
		bDefaultInt = TRUE;
	}
	EthernetManager.nIntIndex ++;
	memset(pEthInt,0,sizeof(__ETHERNET_INTERFACE));
	
	//Copy ethernet name,do not use strxxx routine for safety.
	while(ethName[index] && (index < MAX_ETH_NAME_LEN))
	{
		pEthInt->ethName[index] = ethName[index];
		index ++;
	}
	pEthInt->ethName[index] = 0;
	
	//Copy MAC address,there may exist some risk,since we don't check the length
	//of MAC address,but the caller should gaurantee it's length.
	for(index = 0;index < ETH_MAC_LEN;index ++)
	{
		pEthInt->ethMac[index] = mac[index];
	}
	
	pEthInt->ifState.bDhcpCltEnabled   = FALSE;
	pEthInt->ifState.bDhcpCltOK        = FALSE;
	pEthInt->ifState.bDhcpSrvEnabled   = FALSE;
	
	pEthInt->pIntExtension             = pIntExtension;
	pEthInt->SendFrame                 = SendFrame;
	pEthInt->RecvFrame                 = RecvFrame;
	pEthInt->IntControl                = IntCtrl;
	
	//Allocate layer 3 interface for this ethernet interface.
	pIf = (struct netif*)KMemAlloc(sizeof(struct netif),KMEM_SIZE_TYPE_ANY);
	if(NULL == pIf)
	{
		goto __TERMINAL;
	}
	memset(pIf,0,sizeof(struct netif));
	pIf->state            = pEthInt;     //Point to the layer 2 interface.
	pEthInt->pL3Interface = pIf;         //Point to the layer 3 interface.
	
	//Initialize name of layer 3 interface,since it only occupies 2 bytes.
	pIf->name[0] = pEthInt->ethName[0];
	pIf->name[1] = pEthInt->ethName[1];
	
	//Add the netif to lwIP.
	netif_add(pIf,&pEthInt->ifState.IpConfig.ipaddr,&pEthInt->ifState.IpConfig.mask,
	          &pEthInt->ifState.IpConfig.defgw,pEthInt,_ethernet_if_init, &tcpip_input);
	if(bDefaultInt)
	{
		netif_set_default(pIf);	
	}
	
	//Call driver's initializer,if specified.
	if(Init)
	{
		if(!Init(pEthInt))
		{
			goto __TERMINAL;
		}
	}
	
	//Start DHCP on the interface.
	dhcpRestart(pIf,&pEthInt->ifState);
	
	bResult = TRUE;
	
__TERMINAL:
	if(!bResult)
	{
		//Release the occupied interface slot.
		if(pEthInt)
		{
			EthernetManager.nIntIndex --;
		}
		if(pIf)  //Delete from IP layer and release it.
		{
			netif_remove(pIf);
			KMemFree(pIf,KMEM_SIZE_TYPE_ANY,0);
		}
	}
	return pEthInt;
}

//Config a specified interface,such as it's IP address,DNS server,DHCP
//flags,etc.
static BOOL ConfigInterface(char* ethName,__ETH_IP_CONFIG* pConfig)
{
	__KERNEL_THREAD_MESSAGE   msg;
	__ETH_IP_CONFIG*          pifConfig = NULL;
	BOOL                      bResult   = FALSE;
	
	if(NULL == ethName)
	{
		goto __TERMINAL;
	}
	
	if(NULL == pConfig)
	{
		goto __TERMINAL;
	}
#ifdef __ETH_DEBUG
	_hx_printf("  ConfigInterface: Try to allocate memory and init it.\r\n");
#endif
	//Allocate a configuration object,it will be released by ethernet core thread.
	pifConfig = (__ETH_IP_CONFIG*)KMemAlloc(sizeof(__ETH_IP_CONFIG),KMEM_SIZE_TYPE_ANY);
	if(NULL == pifConfig)
	{
		goto __TERMINAL;
	}
	memcpy(pifConfig,pConfig,sizeof(__ETH_IP_CONFIG));
	strcpy(pifConfig->ethName,ethName);

#ifdef __ETH_DEBUG
  _hx_printf("  ConfigInterface: Try to send message to core thread.\r\n");
#endif	
	//Delivery a message to ethernet core thread.
	msg.wCommand = ETH_MSG_SETCONF;
	msg.wParam   = 0;
	msg.dwParam  = (DWORD)pifConfig;
	SendMessage((HANDLE)EthernetManager.EthernetCoreThread,&msg);
	
	bResult = TRUE;

__TERMINAL:
	if(!bResult)
	{
		if(!pifConfig)
		{
			KMemFree(pifConfig,KMEM_SIZE_TYPE_ANY,0);
		}
	}
	return bResult;
}

//Display all ethernet interface's statistics information.
static VOID ShowInt(char* ethName)
{
	int                    index  = 0;
	__ETH_INTERFACE_STATE* pState = NULL;
	
	if(NULL == ethName)  //Show all interface(s).
	{
		for(index = 0;index < EthernetManager.nIntIndex;index ++)
		{
			pState = &EthernetManager.EthInterfaces[index].ifState;
			_hx_printf("\r\n");
			_hx_printf("  Statistics information for interface '%s':\r\n",
			  EthernetManager.EthInterfaces[index].ethName);
			_hx_printf("    Send frame #       : %d\r\n",pState->dwFrameSend);
			_hx_printf("    Success send #     : %d\r\n",pState->dwFrameSendSuccess);
			_hx_printf("    Send bytes size    : %d\r\n",pState->dwTotalSendSize);
			_hx_printf("    Receive frame #    : %d\r\n",pState->dwFrameRecv);
			_hx_printf("    Success recv #     : %d\r\n",pState->dwFrameRecvSuccess);
			_hx_printf("    Receive bytes size : %d\r\n",pState->dwTotalRecvSize);
		}
	}
	else //Show a specified ethernet interface.
	{
	}
	return;
}

//Delivery a frame to layer 3 entity,usually it will be used by ethernet driver intertupt handler,
//when a frame is received,and call this routine to delivery the frame.
static BOOL Delivery(__ETHERNET_INTERFACE* pIf,struct pbuf* p)
{
	return FALSE;
}

//Scan all available APs in case of WiFi.
static BOOL Rescan(char* ethName)
{
	__ETHERNET_INTERFACE*  pEthInt    = NULL;
	int                    index      = 0;
	BOOL                   bResult    = FALSE;

	if(0 == EthernetManager.nIntIndex)  //Have not available interface.
	{
		goto __TERMINAL;
	}
	if(NULL == ethName)
	{
		pEthInt = &EthernetManager.EthInterfaces[0];
	}
	else
	{
		for(index = 0;index < EthernetManager.nIntIndex;index ++)
		{
			if(0 == strcmp(ethName,EthernetManager.EthInterfaces[index].ethName))
			{
				pEthInt = &EthernetManager.EthInterfaces[index];
				break;
			}
		}
		if(NULL == pEthInt)
		{
			goto __TERMINAL;
		}
	}
	//OK,we located the proper ethernet interface,now launch rescan command.
	if(pEthInt->IntControl)
	{
		bResult = pEthInt->IntControl(pEthInt,ETH_MSG_SCAN,NULL);
	}
	
__TERMINAL:
	return bResult;
}

//Associate to a specified WiFi hotspot in case of WLAN.
static BOOL Assoc(char* ethName,__WIFI_ASSOC_INFO* pAssoc)
{
	__ETHERNET_INTERFACE*    pEthInt    = NULL;
	BOOL                     bResult    = FALSE;
	int                      index      = 0;
	__WIFI_ASSOC_INFO*       pAssocInfo = NULL;
	__KERNEL_THREAD_MESSAGE  msg;
	
	if(NULL == pAssoc)
	{
		goto __TERMINAL;
	}
	//Locate the proper ethernet interface.
	if(NULL == ethName)
	{
		pEthInt = &EthernetManager.EthInterfaces[0];
	}
	else
	{
		for(index = 0;index < EthernetManager.nIntIndex;index ++)
		{
			if(0 == strcmp(ethName,EthernetManager.EthInterfaces[index].ethName))
			{
				pEthInt = &EthernetManager.EthInterfaces[index];
				break;
			}
		}
		if(NULL == pEthInt)
		{
			goto __TERMINAL;
		}
	}
	
	//Issue assoc command to ethernet interface.
	if(pEthInt->IntControl)
	{
		//Allocate a association object and sent to Ethernet core thread,it will be released
		//by ethernet core thread.
		pAssocInfo = (__WIFI_ASSOC_INFO*)KMemAlloc(sizeof(__WIFI_ASSOC_INFO),KMEM_SIZE_TYPE_ANY);
		if(NULL == pAssocInfo)
		{
			goto __TERMINAL;
		}
		memcpy(pAssocInfo,pAssoc,sizeof(__WIFI_ASSOC_INFO));
		pAssocInfo->pPrivate = (LPVOID)pEthInt;
		msg.wCommand  = ETH_MSG_ASSOC;
		msg.wParam    = 0;
		msg.dwParam   = (DWORD)pAssocInfo;
		SendMessage((HANDLE)EthernetManager.EthernetCoreThread,&msg);
		bResult       = TRUE;
		//bResult = pEthInt->IntControl(pEthInt,ETH_MSG_ASSOC,pAssoc);
	}
__TERMINAL:
	return bResult;
}

//Shut down a given ethernet interface.
static BOOL ShutdownInterface(char* ethName)
{
	return TRUE;
}

//Restart the shut interface.
static BOOL UnshutInterface(char* ethName)
{
	return TRUE;
}

/*
*
*  Definition of Ethernet Manager object.
*
*/

struct __ETHERNET_MANAGER EthernetManager = {
	{0},                    //Ethernet interface array.
  0,                      //Index of free slot.
  NULL,                   //Handle of ethernet core thread.
  FALSE,                  //Not initialized yet.

  Initialize,             //Initialize.
  AddEthernetInterface,   //AddEthernetInterface.
  ConfigInterface,        //ConfigInterface.
  Rescan,                 //Rescan.
  Assoc,                  //Assoc.
  Delivery,                   //Delivery.
  SendFrame,              //SendFrame.
  ShowInt,                //ShowInt.
  ShutdownInterface,      //ShutdownInterface.
  UnshutInterface         //UnshutInterface.
};
