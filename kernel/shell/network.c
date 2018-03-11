//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 29,2014
//    Module Name               : network.h
//    Module Funciton           : 
//    Description               : 
//                                Network diagnostic application,common used
//                                network tools such as ping/tracert,are implemented
//                                in network.c file.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/stats.h"  /* For IP layer statistics counter. */
#include "ethmgr.h"
#include "proto.h"
#include "netcfg.h"

#include "kapi.h"
#include "shell.h"
#include "network.h"

#ifdef __CFG_NET_DHCP_SERVER
#include "dhcp_srv/dhcp_srv.h"
#endif
#ifdef __CFG_NET_PPPOE
#include "pppox/oemgr.h"
#endif
#ifdef __CFG_NET_NAT
#include "nat/nat.h"
#endif

#define  NETWORK_PROMPT_STR   "[network_view]"

//
//Pre-declare routines.
//
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);        //help sub-command's handler.
static DWORD _exit(__CMD_PARA_OBJ*);        //exit sub-command's handler.
static DWORD iflist(__CMD_PARA_OBJ*);
static DWORD ping(__CMD_PARA_OBJ*);
static DWORD route(__CMD_PARA_OBJ*);
static DWORD netstat(__CMD_PARA_OBJ*);     //Show out IP layer statistics counter.
static DWORD showint(__CMD_PARA_OBJ*);    //Display ethernet interface's statistics information.
static DWORD showdbg(__CMD_PARA_OBJ*);    //Display ethernet related debugging information.
static DWORD assoc(__CMD_PARA_OBJ*);      //Associate to a specified WiFi SSID.
static DWORD scan(__CMD_PARA_OBJ*);       //Rescan the WiFi networks.
static DWORD setif(__CMD_PARA_OBJ*);      //Set a given interface's configurations.
#ifdef __CFG_NET_DHCP_SERVER
static DWORD dhcpd(__CMD_PARA_OBJ*);      //DHCP Server control command.
#endif
static DWORD ylight(__CMD_PARA_OBJ*);     //Entry routine of yeelight controlling.

#ifdef __CFG_NET_PPPOE                    //PPPoE control command.
static DWORD pppoe(__CMD_PARA_OBJ*);
#endif

#ifdef __CFG_NET_NAT                      //NAT control command.
static DWORD nat(__CMD_PARA_OBJ*);
#endif

//
//The following is a map between command and it's handler.
//
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{ "iflist",     iflist,    "  iflist   : Show all network interface(s) in system."},
	{ "ping",       ping,      "  ping     : Check a specified host's reachbility."},
	{ "route",      route,     "  route    : List all route entry(ies) in system."},
	{ "showint",    showint,   "  showint  : Display ethernet interface's statistics information."},
	{ "showdbg",    showdbg,   "  showdbg  : Display ethernet related debugging info." },
	{ "netstat",    netstat,   "  netstat  : Show out network statistics counter." },
	{ "assoc",      assoc,     "  assoc    : Associate to a specified WiFi SSID."},
	{ "scan",       scan,      "  scan     : Scan WiFi networks and show result."},
	{ "setif",      setif,     "  setif    : Set IP configurations to a given interface."},
#ifdef __CFG_NET_DHCP_SERVER
	{ "dhcpd",      dhcpd,     "  dhcpd    : DHCP Server control commands." },
#endif
#ifdef __CFG_NET_PPPOE
	{ "pppoe",      pppoe,     "  pppoe    : PPPoE function control commands." },
#endif
#ifdef __CFG_NET_NAT
	{ "nat",        nat,       "  nat      : NAT control commands." },
#endif
	{ "light",      ylight,    "  light    : Control command of yeelight." },
	{ "help",       help,      "  help     : Print out this screen." },
	{ "exit",       _exit,     "  exit     : Exit the application." },
	{ NULL,		   NULL,      NULL}
};

static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,SysDiagCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}

//
//The following routine processes the input command string.
//
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	
	
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	//{
	//	return SHELL_CMD_PARSER_FAILED;
	//}
	//CD_PrintString(lpCmdParamObj->Parameter[0],TRUE);

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.Calls the corresponding command handler if found,otherwise SYS_DIAG_CMD_PARSER_INVALID
	//will be returned to indicate this case.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

	//Release parameter object.
	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

//
//This is the application's entry point.
//
DWORD networkEntry(LPVOID p)
{
	return Shell_Msg_Loop(NETWORK_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD _exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

//route command's implementation.
static DWORD route(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_SUCCESS;
}

//ping command's implementation.
static DWORD ping(__CMD_PARA_OBJ* lpCmdObj)
{	
	__PING_PARAM     PingParam;
	ip_addr_t        ipAddr;
	int              count      = 3;    //Ping counter.
	int              size       = 64;   //Ping packet size.
	BYTE             index      = 1;
	DWORD            dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__CMD_PARA_OBJ*  pCurCmdObj = lpCmdObj;
	

	if(pCurCmdObj->byParameterNum <= 1)
	{
		return dwRetVal;
	}

	while(index < lpCmdObj->byParameterNum)
	{
		if(strcmp(pCurCmdObj->Parameter[index],"/c") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			count    = atoi(pCurCmdObj->Parameter[index]);
		}
		else if(strcmp(pCurCmdObj->Parameter[index],"/l") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			size   = atoi(pCurCmdObj->Parameter[index]);
		}
		else
		{
			ipAddr.addr = inet_addr(pCurCmdObj->Parameter[index]);
		}

		index ++;
	}
	
	if(ipAddr.addr != 0)
	{
		dwRetVal    = SHELL_CMD_PARSER_SUCCESS;
	}

	PingParam.count      = count;
	PingParam.targetAddr = ipAddr;
	PingParam.size       = size;

	//Call ping entry routine.
	ping_Entry((void*)&PingParam);
	
	return dwRetVal;
}

//setif command's implementation.
static DWORD setif(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD                   dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__ETH_IP_CONFIG*        pifConfig  = NULL;
	BYTE                    index      = 1;
	char*                   errmsg     = "  Error: Invalid parameter(s).\r\n";
	BOOL                    bAddrOK    = FALSE;
	BOOL                    bMaskOK    = FALSE;

	//Allocate a association information object,to contain user specified associating info.
	//This object will be destroyed by ethernet thread.
	pifConfig = (__ETH_IP_CONFIG*)KMemAlloc(sizeof(__ETH_IP_CONFIG),KMEM_SIZE_TYPE_ANY);
	if(NULL == pifConfig)
	{
		goto __TERMINAL;
	}
	
	//Initialize to default value.
	memset(pifConfig,0,sizeof(__ETH_IP_CONFIG));
	
	if(lpCmdObj->byParameterNum <= 1)
	{
		goto __TERMINAL;
	}

	//Parse command line.
	while(index < lpCmdObj->byParameterNum)
	{		
		if(strcmp(lpCmdObj->Parameter[index],"/d") == 0) //Key of association.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			if(strcmp(lpCmdObj->Parameter[index],"enable") == 0)  //Enable DHCP functions.
			{
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_ENABLE;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"disable") == 0) //Disable DHCP functions.
			{
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_DISABLE;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"restart") == 0) //Restart DHCP.
			{
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_RESTART;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"release") == 0) //Release DHCP configurations.
			{
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_RELEASE;
			}
			else
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/a") == 0) //Set IP address.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}						
			pifConfig->ipaddr.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->ipaddr.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			bAddrOK = TRUE;
		}		
		else if(strcmp(lpCmdObj->Parameter[index],"/m") == 0) //Set IP subnet mask.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			pifConfig->mask.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->mask.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			bMaskOK = TRUE;
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/g") == 0) //Set default gateway.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			pifConfig->defgw.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->defgw.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
		}
		else  //Default parameter as interface's name.
		{
			if(strlen(lpCmdObj->Parameter[index]) < 2)  //Invalid interface name.
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			//pifConfig->ifName[0] = lpCmdObj->Parameter[index][0];
			//pifConfig->ifName[1] = lpCmdObj->Parameter[index][1];
			strcpy(pifConfig->ethName,lpCmdObj->Parameter[index]);
		}
		index ++;
	}
	
	//If IP address and mask are all specified correctly,then assume the DHCP
	//functions will be disabled.
	if(bAddrOK && bMaskOK)
	{
		pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_DISABLE;
	}
	
	//If only specify one parameter without another,it's an error.
	if((bAddrOK && !bMaskOK) || (!bAddrOK & bMaskOK))
	{
		_hx_printf(errmsg);
		goto __TERMINAL;
	}
	
	//Everything is OK,send a message to EthernetManager to launch the
	//modification.
	pifConfig->protoType = NETWORK_PROTOCOL_TYPE_IPV4;
	EthernetManager.ConfigInterface(pifConfig->ethName,pifConfig);
	
	dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	
__TERMINAL:
	if(pifConfig)  //Should release the config object.
	{
		KMemFree(pifConfig,KMEM_SIZE_TYPE_ANY,0);
	}
	return dwRetVal;
}

//A helper routine used to dumpout a network interface.
static void ShowIf(struct netif* pIf)
{
	char    buff[128];
	
	//Print out all information about this interface.
	PrintLine("  --------------------------------------");
	_hx_sprintf(buff,"  Inetface name : %c%c",pIf->name[0],pIf->name[1]);
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 address   : %s",inet_ntoa(pIf->ip_addr));
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 mask      : %s",inet_ntoa(pIf->netmask));
	PrintLine(buff);
	_hx_sprintf(buff,"      IPv4 gateway   : %s",inet_ntoa(pIf->gw));
	PrintLine(buff);
	_hx_sprintf(buff,"      Interface MTU  : %d",pIf->mtu);
	PrintLine(buff);
	_hx_sprintf(buff,"      If flags       : 0x%X", pIf->flags);
	PrintLine(buff);
}

//iflist command's implementation.
static DWORD iflist(__CMD_PARA_OBJ* lpCmdObj)
{
	struct netif* pIfList = netif_list;
	char if_name[MAX_ETH_NAME_LEN];

	if (lpCmdObj->byParameterNum < 2) /* No interface name specified. */
	{
		while (pIfList)  //Travel the whole list and dumpout everyone.
		{
			ShowIf(pIfList);
			pIfList = pIfList->next;
		}
	}
	else /* Interface name specified. */
	{
		strncpy(if_name, lpCmdObj->Parameter[1], sizeof(if_name));
		if (strlen(if_name) < 2)
		{
			_hx_printf("Invalid interface name.\r\n");
			goto __TERMINAL;
		}
		while (pIfList)
		{
			if ((pIfList->name[0] == if_name[0]) && (pIfList->name[1] == if_name[1]))
			{
				break;
			}
			pIfList = pIfList->next;
		}
		if (pIfList)
		{
			ShowIf(pIfList);
		}
		else
		{
			_hx_printf("No interface found.\r\n");
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}

//showint command,display statistics information of ethernet interface.
static DWORD showint(__CMD_PARA_OBJ* lpCmdObj)
{
	//Just send a message to ethernet main thread.
	EthernetManager.ShowInt(NULL);
	
	return NET_CMD_SUCCESS;
}

static DWORD showdbg(__CMD_PARA_OBJ* lpCmdObj)
{
	/*
	 * Dump out ethernet related statistics information
	 * directly.
	 */
	_hx_printf("Total ethernet buff number: %d.\r\n",
		EthernetManager.nTotalEthernetBuffs);
	_hx_printf("Pending queue size: %d.\r\n",
		EthernetManager.nBuffListSize);
	_hx_printf("Broadcast queue size: %d.\r\n",
		EthernetManager.nBroadcastSize);
	_hx_printf("Total driver send_queue sz: %d.\r\n",
		EthernetManager.nDrvSendingQueueSz);
	_hx_printf("Droped broadcast frame: %d.\r\n",
		EthernetManager.nDropedBcastSize);
	return NET_CMD_SUCCESS;
}

/* Show out IP layer statistics counter. */
static void ipstat()
{
	/* Just show out lwIP statistics counter. */
	_hx_printf("IP layer statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.ip.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.ip.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.ip.fw);
	_hx_printf("  droped         : %d\r\n", lwip_stats.ip.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.ip.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.ip.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.ip.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.ip.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.ip.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.ip.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.ip.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.ip.err);
}

/* Show out TCP statistics counter. */
static void tcpstat()
{
	/* Just show out lwIP statistics counter. */
	_hx_printf("TCP statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.tcp.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.tcp.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.tcp.fw);
	_hx_printf("  droped         : %d\r\n", lwip_stats.tcp.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.tcp.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.tcp.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.tcp.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.tcp.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.tcp.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.tcp.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.tcp.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.tcp.err);
}

/* Show out link layer statistics counter. */
static void linkstat()
{
	_hx_printf("Link layer statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.link.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.link.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.link.fw);
	_hx_printf("  droped         : %d\r\n", lwip_stats.link.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.link.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.link.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.link.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.link.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.link.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.link.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.link.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.link.err);
}

/* Show usage of netstat. */
static void netstatUsage(char* sub_cmd)
{
	_hx_printf("Usage:\r\n");
	_hx_printf("  netstat [ip] : Show IP layer statistics counter.\r\n");
	_hx_printf("  netstat [link] : Show link layer statistics counter.\r\n");
	_hx_printf("  netstat [tcp] : Show tcp statistics counter.\r\n");
	return;
}

/* sub command types. */
typedef enum{
	ip,
	link,
	tcp,
}__NETSTAT_SUB_CMD;

/* Entry point of netstat command. */
static DWORD netstat(__CMD_PARA_OBJ* lpCmdObj)
{
	__CMD_PARA_OBJ* pCurCmdObj = lpCmdObj;
	int index = 1;
	__NETSTAT_SUB_CMD sub_cmd;

	BUG_ON(NULL == pCurCmdObj);

	if (pCurCmdObj->byParameterNum <= 1)
	{
		netstatUsage(NULL);
		goto __TERMINAL;
	}

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "ip") == 0)
		{
			/* Obtain the number of NAT entry to show. */
			index++;
			sub_cmd = ip;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "link") == 0)
		{
			/* Obtain interface name. */
			index++;
			sub_cmd = link;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "tcp") == 0)
		{
			/* Obtain interface name. */
			index++;
			sub_cmd = tcp;
		}
		else
		{
			netstatUsage(NULL);
			goto __TERMINAL;
		}
		index++;
	}

	switch (sub_cmd)
	{
	case ip:
		ipstat();
		break;
	case link:
		linkstat();
		break;
	case tcp:
		tcpstat();
		break;
	default:
		break;
	}

__TERMINAL:
	return NET_CMD_SUCCESS;
}

//WiFi association operation,associate to a specified WiFi SSID.
static DWORD assoc(__CMD_PARA_OBJ* lpCmdObj)
{
	 DWORD                  dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__WIFI_ASSOC_INFO       AssocInfo;
	BYTE                    index       = 1;

	//Initialize to default value.
	AssocInfo.ssid[0] = 0;
	AssocInfo.key[0]  = 0;
	AssocInfo.mode    = 0;
	AssocInfo.channel = 0;
	
	if(lpCmdObj->byParameterNum <= 1)
	{
		goto __TERMINAL;
	}

	//lpCmdObj[0].Parameter
	while(index < lpCmdObj->byParameterNum)
	{		
		if(strcmp(lpCmdObj->Parameter[index],"/k") == 0) //Key of association.		
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}
			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //Key is too long.
			{
				_hx_printf("  Error: The key you specified is too long.\r\n");
				goto __TERMINAL;
			}
			//Copy the key into information object.
			strcpy(AssocInfo.key,lpCmdObj->Parameter[index]);
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/m") == 0) //Assoction mode.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}						
			if(strcmp(lpCmdObj->Parameter[index],"adhoc") == 0)  //AdHoc mode.
			{
				AssocInfo.mode = 1;
			}
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/c") == 0)  //Association channel.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}
			AssocInfo.channel = atoi(lpCmdObj->Parameter[index]);
		}
		else
		{
			//Default parameter as SSID.
			//index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}

			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //SSID too long.
			{
				_hx_printf("  Error: SSID you specified is too long.\r\n");
				goto __TERMINAL;
			}

			//Copy the SSID into information object.
			strcpy(AssocInfo.ssid,lpCmdObj->Parameter[index]);		
		}
		index ++;
	}
	
	//Re-check the parameters.
	if(0 == AssocInfo.ssid[0])
	{
		_hx_printf("  Error: Please specifiy the SSID to associate with.\r\n");
		goto __TERMINAL;
	}
	
	//Everything is OK.
	EthernetManager.Assoc(NULL,&AssocInfo);
	
	dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	
__TERMINAL:
	return dwRetVal;
}


//Scan WiFi networks and show the scanning result.
static DWORD scan(__CMD_PARA_OBJ* lpCmdObj)
{
	EthernetManager.Rescan(NULL);
	return NET_CMD_SUCCESS;
}

#ifdef __CFG_NET_DHCP_SERVER
/* Show out dhcpd's usage. */
static void dhcpdUsage()
{
	_hx_printf("  dhcpd: start or stop DHCP server on a given interface.\r\n");
	return;
}
/*
 * DHCP Server control command,used to start or stop DHCP server damon,change
 * configurations of DHCP server.
 */
static DWORD dhcpd(__CMD_PARA_OBJ* lpCmdObj)
{
	char* subcmd = NULL;

	if (lpCmdObj->byParameterNum < 2)
	{
		dhcpdUsage();
		return NET_CMD_SUCCESS;
	}
	subcmd = lpCmdObj->Parameter[1];
	if (strlen(subcmd) < 2)
	{
		_hx_printf("  Invalid interface name.\r\n");
		return NET_CMD_SUCCESS;
	}
	if (strcmp(subcmd, "list") == 0)
	{
		ShowDhcpAlloc();
		return NET_CMD_SUCCESS;
	}
	DHCPSrv_Start_Onif(subcmd);
	return NET_CMD_SUCCESS;
}
#endif

#ifdef __CFG_NET_PPPOE
/*
 * PPPoE function control command.
 * Used to start,stop,or configure PPPoE functions.
 * Command and it's options:
 * pppoe create [instance_name] [eth_if] /u [username] /p [passwd] /a [auth_type]
 * pppoe start instance_name
 * pppoe stop instance_name
 * pppoe delete instance_name
 */
static void pppoeShowUsage(char* subcmd)
{
	if (NULL == subcmd)
	{
	__BEGIN:
		_hx_printf("Usage:\r\n");
		_hx_printf("  pppoe create [inst_name] [eth_if] /u [username] /p [passwd] /a [auth_type]\r\n");
		_hx_printf("  pppoe start [inst_name]\r\n");
		_hx_printf("  pppoe stop [inst_name]\r\n");
		_hx_printf("  pppoe delete [inst_name]\r\n");
		_hx_printf("  pppoe list all\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "create"))
	{
		_hx_printf("  pppoe create [inst_name] [eth_if] /u [username] /p [passwd] /a [auth_type]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    eth_if: ethernet interface name.\r\n");
		_hx_printf("    username: username used to authenticate.\r\n");
		_hx_printf("    passwd: password used to authenticate.\r\n");
		_hx_printf("    auth_type: authentication type,none,pap,or chap.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "start"))
	{
		_hx_printf("  Please specify the PPPoE instance name.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "stop"))
	{
		_hx_printf("  Please specify the PPPoE instance to stop.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "delete"))
	{
		_hx_printf("  Please specify the PPPoE instance to delete.\r\n");
		return;
	}
	/* All other subcmd will lead the showing of all usage. */
	goto __BEGIN;
}

/* Local helpers to make command process procedure simple. */
static void __CreatePPPoEInstance(char* inst_name, char* eth_if, char* username, char* passwd,
	char* auth_type)
{
	__PPPOE_AUTH_TYPE auth = PPPOE_AUTH_NONE;

	if (0 == strcmp(auth_type, "pap"))
	{
		auth = PPPOE_AUTH_PAP;
	}
	if (0 == strcmp(auth_type, "chap"))
	{
		auth = PPPOE_AUTH_CHAP;
	}
	if (NULL != pppoeManager.CreatePPPoEInstance(eth_if, inst_name, username, passwd, auth))
	{
		_hx_printf("  Create PPPoE instance[%s] successfully.\r\n", inst_name);
	}
	else
	{
		_hx_printf("  Failed to create PPPoE instance[%s].\r\n",inst_name);
	}
	return;
}

/* Identifier of pppoe sub-command. */
typedef enum {
	create = 0,
	start,
	stop,
	delete,
	test
}__PPPOE_SUB_CMD;

/* Test PPPoE's performance and debugging it. */
static void testPPPoE(int times)
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	int count = 0;

	if (NULL == pInstance) /* No instance yet. */
	{
		return;
	}
	/* Only test the first PPPoE instance. */
	for (int i = 0; i < times; i++)
	{
		_hx_printf(">>>>>>>>> PPPoE test begin: iteration# %d\r\n", i);
		if (!pppoeManager.StartPPPoEByName(pInstance->session_name))
		{
			_hx_printf(">>>>>>>>> Start PPPoE failed.\r\n");
			return;
		}
		Sleep(1000);
		if (!pppoeManager.StopPPPoEByName(pInstance->session_name))
		{
			_hx_printf(">>>>>>>>> Stop PPPoE session failed.\r\n");
			return;
		}
		_hx_printf(">>>>>>>>> PPPoE test end: iteration# %d\r\n", i);
		Sleep(1000);

		/* Pause 5 minutes after 500 timers. */
		count++;
		if (count > 500)
		{
			count = 0;
			Sleep(1000 * 60 * 5);
		}
	}
}

/* Default password and user name,to simplify testing. */
#define PPPOE_DEFAULT_USERNAME "053202039989"
#define PPPOE_DEFAULT_PASSWORD "60767168"

static DWORD pppoe(__CMD_PARA_OBJ* lpCmdObj)
{
	char instance[32];
	char eth_if[32];
	char username[32];
	char passwd[32];
	char auth_type[8];
	int index = 1;
	__PPPOE_SUB_CMD sub_cmd;
	DWORD dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	__CMD_PARA_OBJ*  pCurCmdObj = lpCmdObj;
	int test_times = 0;
	char str_test_times[16];

	//pppoeCmdEntry();
	//goto __TERMINAL;

	if (pCurCmdObj->byParameterNum <= 1)
	{
		pppoeShowUsage(NULL);
		goto __TERMINAL;
	}
	if (pCurCmdObj->byParameterNum <= 2)
	{
		pppoeShowUsage(pCurCmdObj->Parameter[1]);
		goto __TERMINAL;
	}

	/* Set default value of all command options. */
	instance[0] = eth_if[0] = 0;
	strncpy(username, PPPOE_DEFAULT_USERNAME, sizeof(username));
	strncpy(passwd, PPPOE_DEFAULT_PASSWORD, sizeof(passwd));
	strncpy(auth_type, "none",sizeof(auth_type));

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "create") == 0)
		{
			/* Obtain PPPoE instance name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance,pCurCmdObj->Parameter[index],sizeof(instance));
			/* Obtain ethernet interface name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(eth_if, pCurCmdObj->Parameter[index], sizeof(eth_if));
			sub_cmd = create;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "start") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = start;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "stop") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = stop;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "delete") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = delete;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "list") == 0)
		{
			ListPPPoE();
			goto __TERMINAL;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "test") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(str_test_times, pCurCmdObj->Parameter[index], sizeof(str_test_times));
			test_times = atol(str_test_times);
			sub_cmd = test;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/u") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(username, pCurCmdObj->Parameter[index], sizeof(username));
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/p") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(passwd, pCurCmdObj->Parameter[index], sizeof(passwd));
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/a") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(auth_type, pCurCmdObj->Parameter[index], sizeof(auth_type));
		}
		else
		{
			pppoeShowUsage(pCurCmdObj->Parameter[1]);
			goto __TERMINAL;
		}
		index++;
	}

	/* Call the sub command handler accordingly. */
	switch (sub_cmd)
	{
	case create:
		__CreatePPPoEInstance(instance, eth_if, username, passwd, auth_type);
		break;
	case start:
		pppoeManager.StartPPPoEByName(instance);
		break;
	case stop:
		pppoeManager.StopPPPoEByName(instance);
		break;
	case delete:
		pppoeManager.DestroyPPPoEInstanceByName(instance);
		break;
	case test:
		testPPPoE(test_times);
		break;
	default:
		BUG();
	}

__TERMINAL:
	return dwRetVal;
}
#endif

/* NAT control command. */
#ifdef __CFG_NET_NAT

/* Usage of the command. */
static void natUsage(char* subcmd)
{
	if (NULL == subcmd)
	{
	__BEGIN:
		_hx_printf("Usage:\r\n");
		_hx_printf("  nat enable [int_name]\r\n");
		_hx_printf("  nat disable [int_name]\r\n");
		_hx_printf("  nat list [entry_num]\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "enable"))
	{
		_hx_printf("  nat enable [int_name]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    int_name: interface name to enable NAT.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "disable"))
	{
		_hx_printf("  nat disable [int_name]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    int_name: interface name to disable NAT.\r\n");
		return;
	}
	/* All other sub-command string will lead showing of general usage. */
	goto __BEGIN;
}

/* Local helper routine to show out one easy NAT entry. */
static void ShowNatEntry(__EASY_NAT_ENTRY* pEntry)
{
	/* Output as: [x.x.x.x : p1]->[y.y.y.y : p2], pro:17, ms:10, mt:1000 */
	_hx_printf("    [%s:%d]->", inet_ntoa(pEntry->srcAddr_bef), pEntry->srcPort_bef);
	_hx_printf("[%s:%d]\r\n", inet_ntoa(pEntry->srcAddr_aft), pEntry->srcPort_aft);
	_hx_printf("    dst[%s:%d], pro:%d, ms:%d, mt:%d\r\n",
		inet_ntoa(pEntry->dstAddr_bef),
		pEntry->dstPort_bef,
		pEntry->protocol,
		pEntry->ms,
		pEntry->match_times);
}

/* Sub command of NAT. */
typedef enum{
	enable = 0,
	disable,
	list,
	stat,
}__NAT_SUB_CMD;

static DWORD nat(__CMD_PARA_OBJ* lpCmdObj)
{
	__CMD_PARA_OBJ* pCurCmdObj = lpCmdObj;
	int index = 1;
	char if_name[MAX_ETH_NAME_LEN];
	__NAT_SUB_CMD sub_cmd;
	BOOL bEnable = FALSE;
	size_t ss_num = 0;

	BUG_ON(NULL == pCurCmdObj);

	if (pCurCmdObj->byParameterNum <= 1)
	{
		natUsage(NULL);
		goto __TERMINAL;
	}

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "list") == 0)
		{
			/* Obtain the number of NAT entry to show. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				ss_num = 0;
			}
			else
			{
				ss_num = atoi(pCurCmdObj->Parameter[index]);
			}
			sub_cmd = list;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "enable") == 0)
		{
			/* Obtain interface name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = enable;
			bEnable = TRUE;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "disable") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = disable;
			bEnable = FALSE;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "stat") == 0)
		{
			index++;
			sub_cmd = stat;
		}
		else
		{
			natUsage(NULL);
			goto __TERMINAL;
		}
		index++;
	}

	/* Enable or disable NAT on interface. */
	if ((sub_cmd == enable) || (sub_cmd == disable))
	{
		if (NatManager.enatEnable(if_name, bEnable, TRUE))
		{
			_hx_printf("Set NAT flag on interface[%s] OK.\r\n", if_name);
		}
		else
		{
			_hx_printf("Set NAT flag on interface[%s] failed.\r\n", if_name);
		}
	}
	else if (sub_cmd == list)
	{
		NatManager.ShowNatSession(&NatManager, ss_num);
	}
	else if (sub_cmd == stat)
	{
		_hx_printf("  Nat entry num: %d\r\n", NatManager.stat.entry_num);
		_hx_printf("  Hash deep: %d\r\n", NatManager.stat.hash_deep);
		_hx_printf("  Total match times: %d\r\n", NatManager.stat.match_times);
		_hx_printf("  Total trans times: %d\r\n", NatManager.stat.trans_times);
		_hx_printf("  NAT entry with the maximal hash-deep:\r\n");
		ShowNatEntry(&NatManager.stat.deepNat);
	}

__TERMINAL:
	return NET_CMD_SUCCESS;
}

/* yeelight control command. */
extern BOOL ylight_entry();
static DWORD ylight(__CMD_PARA_OBJ* lpCmdObj)
{
	__LOG("Enter yeelight program...\r\n");
	ylight_entry();
	return NET_CMD_SUCCESS;
}

#endif  //__CFG_NET_NAT.
