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

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "../network/ethernet/ethif.h"

#include "kapi.h"
#include "shell.h"
#include "network.h"
#include "string.h"
#include "stdio.h"

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
static DWORD showint(__CMD_PARA_OBJ*);    //Display ethernet interface's statistics information.
static DWORD assoc(__CMD_PARA_OBJ*);      //Associate to a specified WiFi SSID.
static DWORD scan(__CMD_PARA_OBJ*);       //Rescan the WiFi networks.
static DWORD setif(__CMD_PARA_OBJ*);      //Set a given interface's configurations.

//
//The following is a map between command and it's handler.
//
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{"iflist",     iflist,    "  iflist   : Show all network interface(s) in system."},
	{"ping",       ping,      "  ping     : Check a specified host's reachbility."},
	{"route",      route,     "  route    : List all route entry(ies) in system."},
	{"exit",       _exit,      "  exit     : Exit the application."},
	{"help",       help,      "  help     : Print out this screen."},
  {"showint",    showint,   "  showint  : Display ethernet interface's statistics information."},
  {"assoc",      assoc,     "  assoc    : Associate to a specified WiFi SSID."},
  {"scan",       scan,      "  scan     : Scan WiFi networks and show result."},
  {"setif",      setif,     "  setif    : Set IP configurations to a given interface."},
	{NULL,		   NULL,      NULL}
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
			pifConfig->ipaddr.addr = inet_addr(lpCmdObj->Parameter[index]);
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
			pifConfig->mask.addr = inet_addr(lpCmdObj->Parameter[index]);
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
			pifConfig->defgw.addr = inet_addr(lpCmdObj->Parameter[index]);
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
}

//iflist command's implementation.
static DWORD iflist(__CMD_PARA_OBJ* lpCmdObj)
{
	struct netif* pIfList = netif_list;
	//char   buff[128];

	while(pIfList)  //Travel the whole list and dumpout everyone.
	{
		ShowIf(pIfList);
		pIfList = pIfList->next;
	}

	return SHELL_CMD_PARSER_SUCCESS;
}

//showint command,display statistics information of ethernet interface.
static DWORD showint(__CMD_PARA_OBJ* lpCmdObj)
{
	//Just send a message to ethernet main thread.
	EthernetManager.ShowInt(NULL);
	
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
