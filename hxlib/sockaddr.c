/* 
 * Testing app of socket address's 
 * retrieving functions, i.e, getsockname and
 * getpeername routines.
 */

#include <hellox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sockets/sockets.h>
#include <sockets/ip_addr.h>

/* Main routine. */
int _sockaddr_main(int argc, char* argv[])
{
	/*
	 * Procedures:
	 * 1. Create socket using command parameters;
	 * 2. Try to connect the peer server;
	 * 3. Retrieve local and peer addresses;
	 * 4. Show the addresses;
	 * 5. Close the socket and return.
	 */
	int socks = -1, name_len = 0;
	struct sockaddr_in sock_addr, server_sock;
	struct ip_addr server_addr;
	unsigned short server_port;

	if (argc < 3)
	{
		printf("Too few arguments[hxlib server_addr server_port]\n");
		return -1;
	}
	inet_aton(argv[1], &server_addr);
	server_port = atoi(argv[2]);

	/* Create socket. */
	if ((socks = lwip_socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Can not create socket.\r\n");
		return -1;
	}
	/* Connect to server. */
	printf("Connect server[%s/%d]...\r\n",
		argv[1], server_port);
	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = server_addr.addr;
	server_sock.sin_port = htons(server_port);
	memset(server_sock.sin_zero, 0, sizeof(server_sock.sin_zero));

	if (lwip_connect(socks, (const struct sockaddr*)&server_sock, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Connect to server[0x%X] failed.\n", 
			server_sock.sin_addr.s_addr);
		return -1;
	}
	printf("Connect OK.\n");

	/* Retrieve local and peer sock address. */
	name_len = sizeof(struct sockaddr);
	lwip_getsockname(socks, (struct sockaddr*)&sock_addr, &name_len);
	printf("Local sock info:\n");
	printf("  addr: %s\n", inet_ntoa(sock_addr.sin_addr));
	printf("  port: %d\n", htons(sock_addr.sin_port));

	lwip_getpeername(socks, (struct sockaddr*)&sock_addr, &name_len);
	printf("Peer sock info:\n");
	printf("  addr: %s\n", inet_ntoa(sock_addr.sin_addr));
	printf("  port: %d\n", htons(sock_addr.sin_port));

	closesocket(socks);
	return 0;
}
