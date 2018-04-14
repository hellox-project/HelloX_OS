/* 
 * Server side code of yeelight controller.
 * The server listens incoming request of client on a dedicated port,
 * accepts the request and establishes a TCP connection,then
 * processes the request from client and does controlling according
 * the command.
 * All commands from client to server are encapsulated in JSON object.
 */

#include <hellox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inet.h>
#include <socket/sockets.h>

#include "yeelight.h"

/* Return values of ProcessCommand routine. */
#define PROCESS_CMD_EXIT     0 /* Exit the server thread. */
#define PROCESS_CMD_CONTINUE 1 /* Command string is not complete. */
#define PROCESS_CMD_SUCCESS  2 /* Process command success. */
#define PROCESS_CMD_FAILED   3 /* Failed to process command. */

/* Command that trigger the server thread to exit. */
#define QUIT_COMMAND "quit"

/* Process the incoming command from client. */
static int ProcessCommand(char* cmd_string, int len, HANDLE hCtrlThread)
{
	MSG msg;
	static char* cmd_buffer = NULL;
	static int end_pos = 0;
	int ret = PROCESS_CMD_CONTINUE;

	BUG_ON(NULL == cmd_string);
	BUG_ON(NULL == hCtrlThread);
	BUG_ON(len < 0);

	/* Allocate local command buffer. */
	if (NULL == cmd_buffer)
	{
		cmd_buffer = (char*)_hx_malloc(1500);
		if (NULL == cmd_buffer)
		{
			goto __TERMINAL;
		}
	}

	/* Copy the incoming command string to local command buffer. */
	if (end_pos + len >= 1500)
	{
		ret = PROCESS_CMD_FAILED;
		goto __TERMINAL;
	}
	memcpy(cmd_buffer + end_pos, cmd_string, len);
	end_pos += len;
	cmd_buffer[end_pos] = 0;
	if (0 == strcmp(cmd_buffer, QUIT_COMMAND))
	{
		ret = PROCESS_CMD_EXIT;
		goto __TERMINAL;
	}
	if (end_pos < 2) /* Too few character,continue to get command. */
	{
		ret = PROCESS_CMD_CONTINUE;
		goto __TERMINAL;
	}
	/* A complete command must be terminated by return key(\r\n). */
	if ((cmd_buffer[end_pos - 1] == '\n') && (cmd_buffer[end_pos - 2] == '\r'))
	{
		/* 
		 * Just parse the command string and send command 
		 * to ylight controller thread,who will manipulate the bulb.
		 */
		cmd_buffer[end_pos - 2] = 0;
		_hx_printf("Command[%s] is OK,parse it...\r\n", cmd_buffer);
		if (0 == strcmp(cmd_buffer, "turnon"))
		{
			msg.wCommand = YLIGHT_MSG_TOGGLE;
			msg.dwParam = (DWORD)pLightObject;
			SendMessage(hCtrlThread, &msg);
			ret = PROCESS_CMD_SUCCESS;
		}
		else if (0 == strcmp(cmd_buffer, "turnoff"))
		{
			msg.wCommand = YLIGHT_MSG_TOGGLE;
			msg.dwParam = (DWORD)pLightObject;
			SendMessage(hCtrlThread, &msg);
			ret = PROCESS_CMD_SUCCESS;
		}
		else
		{
			ret = PROCESS_CMD_FAILED;
		}
	}
	else /* More input is desired. */
	{
		ret = PROCESS_CMD_CONTINUE;
	}

__TERMINAL:
	if (ret != PROCESS_CMD_CONTINUE)
	{
		/* Release the command buffer and reset it. */
		BUG_ON(NULL == cmd_buffer);
		_hx_free(cmd_buffer);
		cmd_buffer = NULL;
		end_pos = 0;
	}
	return ret;
}

/* 
 * Entry point of server side.
 * pData is the handle of main thread of yeelight controller,
 * which can be invoked to manipulate the bulb by sending
 * message to it.
 */
DWORD ylight_server(LPVOID pData)
{
	int srv_sock = -1;
	int clt_sock = -1; /* Socket after client connection is accepted. */
	int ret = -1;
	struct sockaddr_in sa;
	u32_t addr_len = 0;
	char* incoming_data = NULL;
	int timeout = 0;
	WORD wr = 0x0700;
	int i = 0, result = 0;
	MSG msg;

	BUG_ON(NULL == pData);

	/* Create the server socket. */
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock < 0)
	{
		__LOG("Failed to create server socket[sock = %d].\r\n",
			srv_sock);
		goto __TERMINAL;
	}

	/* Set receiving timeout value. */
	timeout = YLIGHT_WAIT_TIMEOUT;
	ret = setsockopt(srv_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	if (ret < 0)
	{
		__LOG("%s:failed to set sock's timeout.\r\n", __FUNCTION__);
		goto __TERMINAL;
	}

	/* Bind to server port,use any local IP interface. */
	sa.sin_family = AF_INET;
	sa.sin_port = _hx_htons(YLIGHT_SERVER_PORT);
	sa.sin_addr.s_addr = _hx_htonl(INADDR_ANY);
	memset(sa.sin_zero, 0, sizeof(sa.sin_zero));
	ret = bind(srv_sock, (const struct sockaddr*)&sa, sizeof(sa));
	if (ret < 0)
	{
		__LOG("Failed to bind the server port to local[ret = %d].\r\n",
			ret);
		goto __TERMINAL;
	}

	/* Listen on local network. */
	ret = listen(srv_sock, 1);
	if (ret < 0)
	{
		__LOG("Failed to listen[ret = %d].\r\n", ret);
		goto __TERMINAL;
	}

	/* Allocate the incoming data buffer. */
	incoming_data = (char*)_hx_malloc(1500);
	if (NULL == incoming_data)
	{
		goto __TERMINAL;
	}

	while (TRUE)
	{
		/* Check if we should exit,bypass all message except the TERMINAL. */
		while (PeekMessage(&msg))
		{
			if (KERNEL_MESSAGE_TERMINAL == msg.wCommand)
			{
				goto __TERMINAL;
			}
		}

		/* Accept the incoming client. */
		addr_len = sizeof(sa);
		clt_sock = accept(srv_sock, (struct sockaddr*)&sa, &addr_len);
		if (clt_sock < 0)
		{
			continue; /* May caused by timeout. */
		}

		/* Show client information. */
		_hx_printf("Connection from[%s/%d] is established.\r\n",
			inet_ntoa(sa.sin_addr),
			_hx_ntohs(sa.sin_port));

		/* Process the incoming request from connected client. */
		while (TRUE) {
			ret = recv(clt_sock, incoming_data, 1500, 0);
			if (ret > 0)
			{
				/* Just show the incoming command. */
				for (i = 0; i < ret; i++)
				{
					if (incoming_data[i] == '\r')
					{
						GotoHome();
						continue;
					}
					if (incoming_data[i] == '\n')
					{
						ChangeLine();
						continue;
					}
					/* Show out all ASCII characters. */
					wr += incoming_data[i];
					PrintChar(wr);
					wr -= incoming_data[i];
				}
				result = ProcessCommand(incoming_data, ret, (HANDLE)pData);
				if (PROCESS_CMD_SUCCESS == result)
				{
					/* Send 'OK' to client. */
				}
				else if (PROCESS_CMD_FAILED == result)
				{
					/* Send 'FAIL' to client. */
				}
				else if (PROCESS_CMD_EXIT == result)
				{
					/* Send 'BYE' to client and quit. */
					break;
				}
			}
			else /* The connection may lost. */
			{
				__LOG("Connection from client lost.\r\n");
				break;
			}
		}
		/* Close the connection. */
		close(clt_sock);
		clt_sock = -1;
	}

__TERMINAL:
	/* Close all opened socket(s). */
	if (srv_sock > 0)
	{
		close(srv_sock);
	}
	if (clt_sock > 0)
	{
		close(clt_sock);
	}
	/* Release receiving buffer. */
	if (incoming_data)
	{
		_hx_free(incoming_data);
	}
	/* Show exit message. */
	__LOG("Bulb server exit.\r\n");
	return 0;
}
