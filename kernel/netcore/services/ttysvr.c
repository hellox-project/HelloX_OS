//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 19, 2021
//    Module Name               : ttysrv.c
//    Module Funciton           : 
//                                TTY(TeleTypes) server's implementation
//                                source code.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbm.h>
#include <lwip/sockets.h>

#include "ttysvr.h"

/*
 * Negotiates with client the telnet session options.
 * This routine will be invoked after new session is
 * established and before the inter-activate begin.
 */
static BOOL __negotiate(const int client_sock)
{
	char cmd_and_opt[3];
	int result = -1;
	BOOL bResult = FALSE;

	/* Tell client to close local echo. */
	cmd_and_opt[0] = NVT_IAC;
	cmd_and_opt[1] = NVT_CMD_DONT;
	cmd_and_opt[2] = NVT_OPTION_ECHO;
	result = send(client_sock, cmd_and_opt, 3, 0);
	if (result < 0)
	{
		_hx_printf("[%s]send echo off fail[%d]\r\n", __func__, result);
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Handler to handle the incoming commands and 
 * options from client. 
 * Any command and it's associated options could arrive
 * in the stream of data, with IAC in front of them. All
 * these bytes are posted to this routine to process.
 * The bytes in buffer(iac) processed by this routine is
 * returned, so the caller could continue to process
 * the remaining bytes in buffer.
 * @length denotes the buffer(iac)'s length.
 */
static int __cmd_opt_handler(int client_sock, unsigned char* iac, int length)
{
	int index = 0;

	BUG_ON(NVT_IAC != iac[index++]);

	/* For debugging. */
	_hx_printf("[%s]telnet cmd[%d], option[%d]\r\n", __func__,
		iac[1], iac[2]);

	while (index < length)
	{
		/* Second byte is command value. */
		switch (iac[index++])
		{
		case NVT_CMD_WONT:
			if (index > length)
			{
				/* Data may cruption. */
				break;
			}
			switch (iac[index++])
			{
			case NVT_OPTION_ECHO:
				_hx_printf("[client echo off]\r\n");
				break;
			default:
				break;
			}
			break;
		case NVT_CMD_WILL:
		case NVT_CMD_DONT:
		case NVT_CMD_DO:
		default:
			break;
		}
	}
	return index;
}

/* 
 * Client request handler thread. 
 * pData contains the socket for the session between
 * the new incoming client and server side, it should
 * be released after save to local(client_sock).
 */
static unsigned long __client_req_handler(LPVOID pData)
{
	int client_sock = *(int*)pData;
	char* welcome_msg = "Welcome to HelloX. Type help for command list.\r\n";
	int ret = 0;
	unsigned char request_buffer[1024];
	char display_buffer[2048];
	int start = -1, disp_buff_len = 0;
	int buffer_len = 1024;
	__DEVICE_MESSAGE dmsg;

	BUG_ON(NULL == pData);
	/* Release the memory. */
	_hx_free(pData);

	int time_out = TELNET_RECV_TIMEOUT;

	if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(int)) < 0) {
		_hx_printf("[%s]set socket option fail\r\n", __func__);
		goto __TERMINAL;
	}

	/* Notify the server manager that a new connection is OK. */
	ttyServer.OnNewConnection(client_sock);

	/* Send a message to the client. */
	ret = send(client_sock, welcome_msg, strlen(welcome_msg), 0);
	if (ret < 0)
	{
		__LOG("[%s]send message fail[%d]\r\n", __func__, ret);
		goto __TERMINAL;
	}

	/* Negotiate session options with client. */
	if (!__negotiate(client_sock))
	{
		_hx_printf("[%s]negotiate options fail.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Main loop of handler. */
	while (TRUE)
	{
		ret = recv(client_sock, request_buffer, buffer_len, 0);
		if (ret > 0)
		{
			/* Process the client's input. */
			for (int i = 0; i < ret; i++)
			{
				/* Post to OS kernel if it's ASCII key. */
				if (request_buffer[i] < 128)
				{
					/* Skip the '\n' code. */
					if ('\n' == request_buffer[i])
					{
						continue;
					}
					dmsg.wDevMsgType = KERNEL_MESSAGE_AKEYDOWN;
					dmsg.dwDevMsgParam = (unsigned long)request_buffer[i];
					DeviceInputManager.SendDeviceMessage(
						(__COMMON_OBJECT*)&DeviceInputManager,
						&dmsg,
						NULL);
				}
				else {
					/* No ASCII key, process accordingly. */
					if (NVT_IAC == request_buffer[i])
					{
						/* NVT command. */
						i += __cmd_opt_handler(client_sock, 
							&request_buffer[i], 
							ret - i);
					}
				}
			}
		}

		/* Check the display buffer, and send to client. */
		if (start < 0)
		{
			/* Should use current rear as start. */
			start = DispBufferManager.GetCurrentRear();
		}
		while ((disp_buff_len = DispBufferManager.GetBuffer(display_buffer, 2048, &start)) > 0)
		{
			/* Send back to client. */
			ret = send(client_sock, display_buffer, disp_buff_len, 0);
			if (ret < 0)
			{
				__LOG("[%s]send to client fail[%d]\r\n", __func__, ret);
				goto __TERMINAL;
			}
		}
	}

__TERMINAL:
	/* Close the socket. */
	closesocket(client_sock);
	return 0;
}

/*
 * Server thread of TTY, which listens on the server port, then
 * create new thread to handle client's request when received.
 */
static unsigned long __tty_server(LPVOID pData)
{
	int server_sock = -1;
	int *csock_ptr = NULL;
	int ret = 0;
	int client_index = 0;
	char cthread_name[24];
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t sock_len = 0;
	HANDLE hClientThread = NULL;

	__LOG("[%s]tty server start...\r\n", __func__);

	/* Create the server socket. */
	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock < 0)
	{
		__LOG("[%s]create server sock fail[%d].\r\n", __func__, server_sock);
		goto __TERMINAL;
	}

	/* Bind to server port. */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(TELNET_SERVER_PORT);
	ret = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0)
	{
		__LOG("[%s]bind to port fail[%d]\r\n", __func__, ret);
		goto __TERMINAL;
	}

	/* Listen on. */
	ret = listen(server_sock, 5);
	if (ret < 0)
	{
		__LOG("[%s]listen fail[%d]\r\n", __func__, ret);
		goto __TERMINAL;
	}

	__LOG("[%s]wait for connecting...\r\n", __func__);
	while (TRUE)
	{
		/* 
		 * Client socket must be saved in dedicated memory, 
		 * in multiple thread context. It must be released in
		 * client handler thread.
		 */
		csock_ptr = _hx_malloc(sizeof(int));
		if (NULL == csock_ptr)
		{
			_hx_printf("[%s]malloc fail!\r\n", __func__);
			goto __TERMINAL;
		}
		sock_len = sizeof(struct sockaddr_in);
		*csock_ptr = accept(server_sock, (struct sockaddr*)&client_addr, &sock_len);
		if (*csock_ptr < 0)
		{
			__LOG("[%s]accept err[%d]\r\n", __func__, *csock_ptr);
			goto __TERMINAL;
		}

		/* 
		 * New client connection established, show 
		 * general information and create a new
		 * thread to handle this request, then wait again.
		 */
		__LOG("[%s]new request from[%s:%d] established.\r\n", __func__,
			inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
		/* Contruct a new thread name. */
		_hx_sprintf(cthread_name, "tty_client%d", client_index++);
		/* New a thread to handle this request. */
		hClientThread = CreateKernelThread(
			0,
			KERNEL_THREAD_STATUS_READY,
			PRIORITY_LEVEL_NORMAL,
			__client_req_handler,
			csock_ptr,
			NULL, cthread_name);
		if (NULL == hClientThread)
		{
			_hx_printf("[%s]create client thread fail.\r\n", __func__);
			goto __TERMINAL;
		}
	}

__TERMINAL:
	/* Close the socket if already open. */
	if (server_sock > 0)
	{
		closesocket(server_sock);
	}
	__LOG("[%s]tty server exit with[%d]\r\n", __func__, ret);
	return ret;
}

/* Initialize routine of tty server object. */
static BOOL __Initialize()
{
	HANDLE server_thread = NULL;
	BOOL bResult = FALSE;

	/* Create the server thread. */
	server_thread = CreateKernelThread(0,
		KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		__tty_server,
		NULL, NULL,
		"tty_server");
	if (NULL == server_thread)
	{
		_hx_printf("[%s]create server failed.\r\n", __func__);
		goto __TERMINAL;
	}
	ttyServer.server_thread = server_thread;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Notify the server that a new connection is established. */
static void __OnNewConnection(int sock)
{
	ttyServer.default_sock = sock;
}

/* Send a byte to the default connection. */
static void __SendChar(char byte)
{
	if (ttyServer.default_sock < 0)
	{
		/* No default connection. */
		return;
	}
	int ret = send(ttyServer.default_sock, &byte, 1, 0);
	if (ret < 0)
	{
		__LOG("[%s]send byte fail[%d]\r\n", __func__, ret);
	}
}

/* Change to a new line. */
static void __ChangeLine()
{
	/* Just send out LF and CR. */
	__SendChar('\r');
	__SendChar('\n');
}

/* Send a string to the client. */
static void __PrintString(const char* string)
{
	if (ttyServer.default_sock < 0)
	{
		/* No default connection. */
		return;
	}
	int ret = send(ttyServer.default_sock, string, strlen(string), 0);
	if (ret < 0)
	{
		__LOG("[%s]send string fail[%d]\r\n", __func__, ret);
	}
}

/* The global tty server object. */
__TTY_SERVER ttyServer = {
	NULL,                //server_thread's handle.
	-1,                  //default_sock.
	__Initialize,        //Initialize routine.
	__OnNewConnection,   //OnNewConnection.
	__SendChar,          //SendChar.
	__ChangeLine,        //ChangeLine.
	__PrintString,       //PrintString.
};
