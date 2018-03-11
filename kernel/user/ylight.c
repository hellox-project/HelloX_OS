/* Source code of yeelight bulb's controlling program. */
#include <StdAfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lwip/sockets.h"

/* Constants. */
#define SSDP_DEST_ADDR "239.255.255.250"
#define SSDP_DEST_PORT 1982
#define SSDP_WAIT_TIMEOUT 2000 /* Wait at most for 2s for incoming data. */
#define SSDP_LAN_ADDR "192.168.169.1"

/* First line of search request if OK(200 OK). */
const char* okmsg = "HTTP/1.1 200 OK";

/* Search request message. */
#define SSDP_REQ_CMD "M-SEARCH * HTTP/1.1\r\n\
HOST: 239.255.255.250:1982\r\n\
MAN: \"ssdp:discover\"\r\n\
ST: wifi_bulb"

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

/* Handler to parse the searching response message. */
static void ParseResponse(char* msg, int msg_len)
{
	char* charbuff = NULL;

	if ((NULL == msg) || (0 == msg_len))
	{
		return;
	}

	_hx_printf("%s:process response data[len = %d].\r\n",
		__FUNCTION__,
		msg_len);

	/* 
	 * Allocate a dedicated buffer to hold the message,
	 * since it maybe changed in parse process.
	 */
	charbuff = (char*)_hx_malloc(msg_len + 1);
	if (NULL == charbuff)
	{
		goto __TERMINAL;
	}
	memcpy(charbuff, msg, msg_len);
	charbuff[msg_len] = '\0';
	
	/* Process the message line by line. */
	char* line = strtok(charbuff, "\r\n");
	char* value = NULL;
	if (line && (0 == strcmp(line, okmsg)))
	{
		_hx_printf("Find a new yeelight device:\r\n");
		while (line)
		{
			_hx_printf("  %s\r\n", line);
#if 0
			value = getvalue(line, "Location");
			if (value)
			{
				printf("%s\r\n", value);
			}
#endif
			line = strtok(NULL, "\r\n");
		}
	}
__TERMINAL:
	if (charbuff)
	{
		_hx_free(charbuff);
	}
	return;
}

/* Search all yeelight in local network. */
static DWORD ylight_search(LPVOID pData)
{
	int s = -1, ret = -1, len = 0;
	int timeout = SSDP_WAIT_TIMEOUT;
	BOOL bResult = FALSE;
	char* buf = NULL;
	struct sockaddr_in gaddr;
	struct in_addr ifaddr;

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
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	mreq.imr_multiaddr.s_addr = inet_addr(SSDP_DEST_ADDR);
	ret = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
	if (ret < 0)
	{
		__LOG("%s:failed to join multicast group,err = %d.\r\n", __FUNCTION__, ret);
		goto __TERMINAL;
	}

	/* Bind to multicast group address. */
	gaddr.sin_family = AF_INET;
	gaddr.sin_port = htons(SSDP_DEST_PORT);
	gaddr.sin_addr.s_addr = htonl(INADDR_ANY);
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
	/* 
	 * Search all WiFi bulbs in local network,send out a request to 
	 * multicast address to trigger the bulbs to response our request.
	 * at most 30s of this procedure.
	 */
	while (TRUE)
	{
		len = sizeof(gaddr);
		gaddr.sin_family = AF_INET;
		gaddr.sin_port = htons(SSDP_DEST_PORT);
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
			_hx_printf("%s:send %d bytes out with multicast.\r\n", __FUNCTION__, ret);
		}
		ret = recvfrom(s, buf, 1500, 0, (struct sockaddr*)&gaddr, &len);
		if (ret < 0)
		{
			__LOG("%s:failed to receive data from socket[ret = %d].\r\n", __FUNCTION__,
				ret);
		}
		else
		{
			//_hx_printf("Recived [%d] byte(s) data from[%s:%d].\r\n", ret,
			//	inet_ntoa(gaddr.sin_addr.s_addr),
			//	htons(gaddr.sin_port));
			ParseResponse(buf, ret);
		}
		/* Just pause a while. */
		Sleep(3000);
	}
	bResult = TRUE;

__TERMINAL:
	{
		if (s > 0)
		{
			closesocket(s);
		}
		if (buf)
		{
			_hx_free(buf);
		}
	}
	return bResult;
}

BOOL ylight_entry()
{
	HANDLE hThread = CreateKernelThread(
		0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		ylight_search,
		NULL,
		NULL,
		"ylight");
	return (NULL == hThread ? FALSE : TRUE);
}
