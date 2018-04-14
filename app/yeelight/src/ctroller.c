/* 
 * Source code of yeelight controller.
 * The controller is a dedicated background kernel thread,running in background
 * and control the yeelight bulb by sending command.
 * The other functions,such as user inter-activate thread,yeelight server thread,
 * and others may control the bulb by sending message to controller.
 */

#include <hellox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yeelight.h"

 /* Construct a valid toggle command message to send to the bulb. */
static BOOL ConstructToggleCmd(struct yeelight_object* pLight, char* pCmdBuff, int length)
{
	int cmd_len = 0;
	BUG_ON(NULL == pCmdBuff);
	BUG_ON(0 == length);
	BUG_ON(NULL == pLight);
	cmd_len = strlen(YLIGHT_TOGGLE_CMD + strlen(pLight->id));
	if (cmd_len > length) /* Command buffer too short. */
	{
		return FALSE;
	}
	_hx_sprintf(pCmdBuff, YLIGHT_TOGGLE_CMD, pLight->id);
	_hx_printf(pCmdBuff); //Debugging.
	return TRUE;
}

/* Toggle yeelight object. */
BOOL ToggleYlight(struct yeelight_object* light)
{
	int sock = -1;
	BOOL bResult = FALSE;
	char* cmd = NULL;
	int ret = -1;
	struct sockaddr_in sa;

	BUG_ON(NULL == light);
	if (light->sock < 0)
	{
		/* No socket created yet,create a new one. */
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0)
		{
			__LOG("Can not create socket object[sock = %d].\r\n",
				sock);
			goto __TERMINAL;
		}

		/* Set the sending and receiving timeout value. */
		int timeout = YLIGHT_WAIT_TIMEOUT;
		ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		if (ret < 0)
		{
			__LOG("%s:failed to set sock's recv timeout[ret = %d].\r\n",
				__FUNCTION__,
				ret);
			goto __TERMINAL;
		}
		/*
		ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
		if (ret < 0)
		{
		__LOG("%s:failed to set sock's snd timeout[ret = %d].\r\n",
		__FUNCTION__,
		ret);
		goto __TERMINAL;
		}*/

		/* Connect to yeelight object. */
		sa.sin_family = AF_INET;
		sa.sin_addr = light->socket.sin_addr;
		sa.sin_port = light->socket.sin_port;
		memset(sa.sin_zero, 0, sizeof(sa.sin_zero));
		ret = connect(sock, (struct sockaddr*)&sa, sizeof(sa));
		if (ret < 0)
		{
			__LOG("Can not connect to light object[ret = %d].\r\n", ret);
			goto __TERMINAL;
		}

		__LOG("Connect to light object OK.\r\n");
		/* Save to use next time,the connection is keeping open. */
		light->sock = sock;
	}

	cmd = (char*)_hx_malloc(1500);
	if (NULL == cmd)
	{
		goto __TERMINAL;
	}
	/* Construct toggle command. */
	if (!ConstructToggleCmd(light, cmd, 1500))
	{
		goto __TERMINAL;
	}
	ret = send(sock, cmd, strlen(cmd), 0);
	if (ret < 0)
	{
		_hx_printf("Failed to send toggle command.\r\n");
		goto __TERMINAL;
	}
	ret = recv(sock, cmd, 1500, 0);
	if (ret < 0)
	{
		__LOG("Failed to recv resp[ret = %d].\r\n", ret);
		goto __TERMINAL;
	}
	/* Show out the status info received. */
	if (ret > 128)
	{
		_hx_printf("Received status update message[size = %d].\r\n", ret);
	}
	else
	{
		cmd[ret] = 0;
		_hx_printf("Status update:%s\r\n", cmd);
	}

	bResult = TRUE;
__TERMINAL:
	if (!bResult)
	{
		if (light->sock >= 0)
		{
			close(light->sock);
			light->sock = -1;
		}
	}
	if (cmd)
	{
		_hx_free(cmd);
	}
	return bResult;
}

/* Entry point of yeelight controller thread. */
DWORD ylight_controller(LPVOID pData)
{
	MSG msg;
	struct yeelight_object* pLight = NULL;

	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.wCommand)
			{
			case YLIGHT_MSG_TOGGLE:
				__LOG("%s:toggle command received.\r\n", __func__);
				pLight = (struct yeelight_object*)msg.dwParam;
				if (pLight)
				{
					ToggleYlight(pLight); /* Toggle the light. */
				}
				break;
			case YLIGHT_MSG_SETRGB:
				break;
			case KERNEL_MESSAGE_TERMINAL:
				goto __TERMINAL;
			default:
				break;
			}
		}
	}

__TERMINAL:
	__LOG("Bulb controller exit.\r\n");
	return 0;
}
