/* 
 * Main source code file of yeelight application. 
 * The application applies for yeelight smart bulb controlling,include
 * discovery,presentation,operation,change configuration and other
 * actions suit for yeelight,by offering command line interface.
 * This application is fited for HelloX operating system.
 */

#include <hellox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inet.h>
#include <socket/sockets.h>

#include "yeelight.h"

 /* First line of search request if OK(200 OK). */
const char* okmsg = "HTTP/1.1 200 OK";

/* Global list to save all discovered yeelight object in LAN. */
static struct yeelight_object* pLightObject = NULL;

/* Delete the specified char from a given string. */
static void strdc(char* src, char del)
{
	int j, k;
	if (NULL == src)
	{
		return;
	}
	for (j = k = 0; src[j] != '\0'; j++)
	{
		if (src[j] != del)
		{
			src[k++] = src[j];
		}
	}
	src[k] = 0;
	return;
}

/* Get value sub-string by giving the key. */
static char* getvalue(char* str, const char* key)
{
	char* ret = NULL;
	int i = 0;

	if ((NULL == str) || (NULL == key))
	{
		goto __TERMINAL;
	}
	for (i = 0; str[i] && key[i]; i++)
	{
		if (str[i] != key[i])
		{
			break;
		}
	}
	/* Key can not match the proceeding part of str? */
	if ((0 == str[i]) || (key[i]))
	{
		goto __TERMINAL;
	}
	/* Skip all spaces and colons. */
	while (str[i] && ((' ' == str[i]) || (':' == str[i])))
	{
		i++;
	}
	if (0 == str[i])
	{
		goto __TERMINAL;
	}
	ret = &str[i];
__TERMINAL:
	return ret;
}

/* Obtain light's IP address and port from location string,fill them into light object. */
static BOOL GetSocketFromLocation(char* location, struct sockaddr_in* pSock)
{
	char* sock_str = NULL;
	BOOL bResult = FALSE;
	char addr[32];
	int len = 0;
	unsigned short port = 0;

	BUG_ON(NULL == location);
	BUG_ON(NULL == pSock);

	/* Get socket string(IP and port) from string. */
	sock_str = getvalue(location, "yeelight");
	if (NULL == sock_str)
	{
		goto __TERMINAL;
	}
	/* skip all slash line. */
	while (*sock_str == '/')
	{
		sock_str++;
	}
	//_hx_printf("sock_str = %s.\r\n", sock_str);
	len = 0;
	while (sock_str[len] && (sock_str[len] != ':') && (len < 31))
	{
		addr[len] = sock_str[len];
		len++;
	}
	if (len >= 31) /* Can not obtain IP address from location string. */
	{
		goto __TERMINAL;
	}
	addr[len] = 0;
	sock_str += len;
	//_hx_printf("IP addr = %s\r\n", addr);
	pSock->sin_addr.s_addr = inet_addr(addr);
	if (0 == pSock->sin_addr.s_addr)
	{
		goto __TERMINAL;
	}
	//_hx_printf("IP addr: %s.\r\n", inet_ntoa(pSock->sin_addr));
	/* skip all space and colon. */
	while (*sock_str && ((*sock_str == ' ') || (*sock_str == ':')))
	{
		sock_str++;
	}
	len = 0;
	while (sock_str[len] && (sock_str[len] != '\r') && (sock_str[len] != '\n') && len < 31)
	{
		addr[len] = sock_str[len];
		len++;
	}
	if (len >= 31)
	{
		goto __TERMINAL;
	}
	addr[len] = 0;
	//_hx_printf("port = %s\r\n", addr);
	pSock->sin_port = _hx_htons((__u16)atol(addr));
	if (0 == pSock->sin_port)
	{
		goto __TERMINAL;
	}
	bResult = TRUE;
__TERMINAL:
	return bResult;
}

static void ShowLight(struct yeelight_object* pLight)
{
	while (pLight)
	{
		_hx_printf("--------------------\r\n");
		_hx_printf("id: %s\r\n", pLight->id);
		_hx_printf("ip: %s\r\n", inet_ntoa(pLight->socket.sin_addr));
		_hx_printf("port: %d\r\n", _hx_ntohs(pLight->socket.sin_port));
		pLight = pLight->pNext;
	}
}

static BOOL ParseResponse(const char* pRespMsg,int msglen)
{
	char* charbuff = NULL;
	struct yeelight_object* pLight = NULL;
	BOOL bResult = FALSE;

	charbuff = _hx_malloc(msglen + 1);
	if (NULL == charbuff)
	{
		goto __TERMINAL;
	}
	strcpy(charbuff, pRespMsg);
	char* line = strtok(charbuff, "\r\n");
	char* value = NULL;
	if (line && (0 == strcmp(line, okmsg)))
	{
		/*
		* Got a valid response,create a new light object
		* to hold values derived from this response.
		*/
		pLight = (struct yeelight_object*)_hx_malloc(sizeof(struct yeelight_object));
		memset(pLight, 0, sizeof(struct yeelight_object));
		pLight->sock = -1; /* No socket opened yet. */
		if (NULL == pLight)
		{
			goto __TERMINAL;
		}
		while (line)
		{
			value = getvalue(line, "Location");
			if (value)
			{
				/* Obtain the light's IP address and port,fill into light object. */
				GetSocketFromLocation(value, &pLight->socket);
			}
			value = getvalue(line, "id");
			if (value)
			{
				if (strlen(value) > LIGHT_ID_LENGTH) /* Invalid ID value. */
				{
					goto __TERMINAL;
				}
				/* Save ID to light object. */
				strcpy(pLight->id, value);
			}
			value = getvalue(line, "name");
			if (value)
			{
				/* Save bulb name. */
			}
			line = strtok(NULL, "\r\n");
		}
	}
	/* Check if the response is from a known light. */
	if (pLight)
	{
		struct yeelight_object* pTmpLight = pLightObject;
		while (pTmpLight)
		{
			/*
			* The light object is already in list of their
			* IDs are same.
			*/
			if (0 == strcmp(pLight->id, pTmpLight->id))
			{
				/*
				* Replace old light object's attributes by the
				* newly one.
				* May lead issue in multi-thread environment here,
				* since the global light list is reachable from
				* all threads,and no mutex is applied here for easy.
				*/
				pLight->pNext = pTmpLight->pNext;
				memcpy(pTmpLight, pLight, sizeof(*pLight));
				break;
			}
			pTmpLight = pTmpLight->pNext;
		}
		if (NULL == pTmpLight) /* A new light. */
		{
			/* Just link the new found light into global list. */
			pLight->pNext = pLightObject;
			pLightObject = pLight;
			_hx_printf("\r\nA new light found[id = %s].\r\n",
				pLight->id);
			bResult = TRUE;
		}
	}
__TERMINAL:
	if (!bResult)
	{
		if (pLight)
		{
			_hx_free(pLight);
		}
	}
	_hx_free(charbuff);
	return bResult;
}

/* Search all yeelight in local network. */
static DWORD ylight_search(LPVOID pData)
{
	int s = -1, ret = -1, len = 0;
	int timeout = SSDP_WAIT_TIMEOUT;
	char* buf = NULL;
	struct sockaddr_in gaddr;
	struct in_addr ifaddr;
	HANDLE hTimer = NULL;
	MSG msg;

	/* Create the periodic timer that trigger light searching. */
	hTimer = SetTimer(YLIGHT_PERIODIC_ID, SSDP_PERIODIC_TIME, NULL, NULL, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer)
	{
		__LOG("Failed to set periodic timer object.\r\n");
		goto __TERMINAL;
	}

	/* Create socket to receive multicast data. */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		__LOG("%s:failed to create socket.\r\n", __FUNCTION__);
		goto __TERMINAL;
	}

	/* Set timeout value for this socket. */
	ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	if (ret < 0)
	{
		__LOG("%s:failed to set sock's timeout.\r\n", __FUNCTION__);
		goto __TERMINAL;
	}

	/* Set out going interface of this multicast socket. */
	ifaddr.s_addr = inet_addr(SSDP_LAN_ADDR);
	ret = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &ifaddr, sizeof(ifaddr));
	if (ret < 0)
	{
		__LOG("%s:failed to set out going interface.\r\n", __FUNCTION__);
		goto __TERMINAL;
	}

	/* Join the multicase group. */
	struct ip_mreq mreq;
	mreq.imr_interface.s_addr = _hx_htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr = inet_addr(SSDP_DEST_ADDR);
	ret = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
	if (ret < 0)
	{
		__LOG("%s:failed to join multicast group,err = %d.\r\n", __FUNCTION__, ret);
		goto __TERMINAL;
	}

	/* Bind to multicast group address. */
	gaddr.sin_family = AF_INET;
	gaddr.sin_port = _hx_htons(SSDP_DEST_PORT);
	gaddr.sin_addr.s_addr = _hx_htonl(INADDR_ANY);
	memset(gaddr.sin_zero, 0, sizeof(gaddr.sin_zero));
	ret = bind(s, (struct sockaddr*)&gaddr, sizeof(gaddr));
	if (ret < 0)
	{
		__LOG("%s:failed to bind to local interface.\r\n", __FUNCTION__);
		goto __TERMINAL;
	}

	/* Allocate data buffer for this socket. */
	buf = _hx_malloc(1500);
	if (NULL == buf)
	{
		goto __TERMINAL;
	}
	/* Main message loop. */
	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case KERNEL_MESSAGE_TIMER:
				/*
				* Search all WiFi bulbs in local network,send out a request to
				* multicast address to trigger the bulbs to response our request.
				*/
				len = sizeof(gaddr);
				gaddr.sin_family = AF_INET;
				gaddr.sin_port = _hx_htons(SSDP_DEST_PORT);
				gaddr.sin_addr.s_addr = inet_addr(SSDP_DEST_ADDR);
				memset(gaddr.sin_zero, 0, sizeof(gaddr.sin_zero));
				strcpy(buf, SSDP_REQ_CMD);
				ret = sendto(s, buf, strlen(buf), 0, (struct sockaddr*)&gaddr, len);
				if (ret < 0)
				{
					__LOG("%s:failed to send request.\r\n", __FUNCTION__);
					goto __TERMINAL;
				}
				else
				{
					//_hx_printf("%s:send %d bytes out with multicast.\r\n", __FUNCTION__, ret);
				}
				ret = recvfrom(s, buf, 1500, 0, (struct sockaddr*)&gaddr, &len);
				if (ret < 0)
				{
					//_hx_printf("%s:failed to receive data from socket[ret=%d].\r\n", __FUNCTION__,
					//	ret);
				}
				else
				{
					//_hx_printf("Recived [%d] byte(s) data from[%s:%d].\r\n", ret,
					//	inet_ntoa(gaddr.sin_addr.s_addr),
					//	_hx_htons(gaddr.sin_port));
					buf[ret] = 0; /* Set terminator of the string. */
					ParseResponse(buf, ret);
				}
				break;
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;
			default:
				break;
			}
		}
	}

__TERMINAL:
	{
		if (hTimer)
		{
			CancelTimer(hTimer);
		}
		if (s > 0)
		{
			close(s);
		}
		if (buf)
		{
			_hx_free(buf);
		}
	}
	/* Show exit message. */
	__LOG("Bulb searcher exit.\r\n");
	return 0;
}

/* Main entry point of the application. */
int _hx_main(int argc,char* argv[])
{
	char* cmd_array[4];
	int cmd_num = 0, i = 0;
	struct yeelight_object* pLight = NULL;
	HANDLE hSearchThread = NULL;
	HANDLE hServerThread = NULL;
	HANDLE hCtrlThread = NULL;
	MSG msg;

	/* Show version info. */
	_hx_printf("Yeelight controller for HelloX[v%d.%d].\r\n", __MAJOR_VER, __MINNOR_VER);

	/* Create the controller kernel thread. */
	hCtrlThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ylight_controller,
		NULL,
		NULL,
		"YLIGHT_CTRL");
	if (NULL == hCtrlThread)
	{
		__LOG("Failed to create controller thread.\r\n");
		goto __TERMINAL;
	}

	/* 
	 * A dedicated kernel thread is running in background, to search and 
	 * manages all bulb(s) in LAN.
	 * Bring up this kernel thread first.
	 */
	hSearchThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ylight_search,
		NULL,
		NULL,
		"YEELIGHT");
	if (NULL == hSearchThread)
	{
		__LOG("Failed to create searching thread.\r\n");
		goto __TERMINAL;
	}

	/* Create the server thread of controller. */
	hServerThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ylight_server,
		hCtrlThread, /* Send message to controller thread. */
		NULL,
		"YLIGHT_SRV");
	if (NULL == hServerThread)
	{
		__LOG("Failed to create server thread.\r\n");
		goto __TERMINAL;
	}

	/* Interactive with user. */
	while (TRUE)
	{
		cmd_num = getcmd(cmd_array, 4);
		if (cmd_num > 0)
		{
			if (strcmp(cmd_array[0], "list") == 0)
			{
				/* Show all found bulb(s). */
				ShowLight(pLightObject);
			}
			else if (strcmp(cmd_array[0], "exit") == 0)
			{
				/* Exit the application. */
				goto __TERMINAL;
			}
			else if (strcmp(cmd_array[0], "turnon") == 0)
			{
				/* 
				 * Turn on the specified light,but we just turn on 
				 * the first light in list,for simplicity.
				 */
				if (pLightObject)
				{
					msg.wCommand = YLIGHT_MSG_TOGGLE;
					msg.dwParam = (DWORD)pLightObject;
					SendMessage(hCtrlThread, &msg);
				}
			}
			else if (strcmp(cmd_array[0], "turnoff") == 0)
			{
				/* Turn off the first light. */
				if (pLightObject)
				{
					msg.wCommand = YLIGHT_MSG_TOGGLE;
					msg.dwParam = (DWORD)pLightObject;
					SendMessage(hCtrlThread, &msg);
				}
			}
			else if (strcmp(cmd_array[0], "help") == 0)
			{
				/* Show help information. */
				_hx_printf("  list    : Show out all found bulb(s).\r\n");
				_hx_printf("  turnon  : Turn on a specified bulb.\r\n");
				_hx_printf("  turnoff : Turn off a specified bulb.\r\n");
				_hx_printf("  exit    : Exit the application.\r\n");
			}
			else
			{
				/* Unknown command. */
				_hx_printf("  Unknown command.\r\n");
			}
		}
	}
__TERMINAL:
	if (hSearchThread)
	{
		/* Notify the searching thread to exit. */
		msg.wCommand = KERNEL_MESSAGE_TERMINAL;
		SendMessage(hSearchThread, &msg);
		/* Wait for the searching thread run over. */
		WaitForThisObject(hSearchThread);
		/* Destroy it. */
		DestroyKernelThread(hSearchThread);
	}
	if (hCtrlThread)
	{
		/* Trigger the controller thread to exit. */
		msg.wCommand = KERNEL_MESSAGE_TERMINAL;
		SendMessage(hCtrlThread, &msg);
		/* Wait the controller to exit. */
		WaitForThisObject(hCtrlThread);
		DestroyKernelThread(hCtrlThread);
	}
	if (hServerThread)
	{
		msg.wCommand = KERNEL_MESSAGE_TERMINAL;
		SendMessage(hServerThread, &msg);
		WaitForThisObject(hServerThread);
		DestroyKernelThread(hServerThread);
	}

	/* Destroy all light object(s). */
	while (pLightObject)
	{
		pLight = pLightObject;
		pLightObject = pLightObject->pNext;
		/* Close socket if open. */
		if (pLight->sock >= 0)
		{
			close(pLight->sock);
		}
		_hx_free(pLight);
	}
	
	return 0;
}
