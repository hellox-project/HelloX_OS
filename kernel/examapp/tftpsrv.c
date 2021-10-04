/*
 * TFTP Server
 * Copyright 2014 - Acri Emanuele <crossbower@gmail.com>
 *
 * This program is under a GPLv3+ license.
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "stdint.h"
#include "types.h"
#include "io.h"
#include "socket/sockets.h"

/* Version number. */
#define __MAJOR_VER 1
#define __MINOR_VER 2

/* Recv time out value in ms. */
#define RECV_TIMEOUT 10000
/* Retry times when fail. */
#define RECV_RETRIES 5

/* Default UDP port of the server. */
#define DEFAULT_TFTP_PORT 69

/* Default start port value of TFTP server session. */
#define TFTP_PORT_START 1025

 /* tftp opcode mnemonic */
enum opcode {
	RRQ = 1,
	WRQ,
	DATA,
	ACK,
	ERROR
};

/* tftp transfer mode */
enum mode {
	NETASCII = 1,
	OCTET
};

/* tftp message structure */
#pragma pack(push,1)
typedef union {

	uint16_t opcode;

	struct {
		uint16_t opcode; /* RRQ or WRQ */
		uint8_t filename_and_mode[514];
	} request;

	struct {
		uint16_t opcode; /* DATA */
		uint16_t block_number;
		uint8_t data[512];
	} data;

	struct {
		uint16_t opcode; /* ACK */
		uint16_t block_number;
	} ack;

	struct {
		uint16_t opcode; /* ERROR */
		uint16_t error_code;
		uint8_t error_string[512];
	} error;

} tftp_message;
#pragma pack(pop)

/* base directory */
char *base_directory;

/*
void cld_handler(int sig) {
	int status;
	wait(&status);
}
*/

ssize_t tftp_send_data(int s, uint16_t block_number, uint8_t *data,
	ssize_t dlen, struct sockaddr_in *sock, socklen_t slen)
{
	tftp_message m;
	ssize_t c;

	m.opcode = htons(DATA);
	m.data.block_number = htons(block_number);
	memcpy(m.data.data, data, dlen);

	if ((c = sendto(s, &m, 4 + dlen, 0,
		(struct sockaddr *) sock, slen)) < 0) {
		_hx_printf("[%s]send to error[%d]\r\n", __func__, c);
	}

	return c;
}

ssize_t tftp_send_ack(int s, uint16_t block_number,
	struct sockaddr_in *sock, socklen_t slen)
{
	tftp_message m;
	ssize_t c;

	m.opcode = htons(ACK);
	m.ack.block_number = htons(block_number);

	if ((c = sendto(s, &m, sizeof(m.ack), 0,
		(struct sockaddr *) sock, slen)) < 0) {
		perror("server: sendto()");
	}

	return c;
}

ssize_t tftp_send_error(int s, int error_code, char *error_string,
	struct sockaddr_in *sock, socklen_t slen)
{
	tftp_message m;
	ssize_t c;

	if (strlen(error_string) >= 512) {
		printf("server: tftp_send_error(): error string too long\n");
		return -1;
	}

	m.opcode = htons(ERROR);
	m.error.error_code = error_code;
	strcpy(m.error.error_string, error_string);

	if ((c = sendto(s, &m, 4 + strlen(error_string) + 1, 0,
		(struct sockaddr *) sock, slen)) < 0) {
		perror("server: sendto()");
	}

	return c;
}

ssize_t tftp_recv_message(int s, tftp_message *m, struct sockaddr_in *sock, socklen_t *slen)
{
	ssize_t c;

	if ((c = recvfrom(s, m, sizeof(*m), 0, (struct sockaddr *) sock, slen)) < 0) {
		_hx_printf("[%s]recvfrom sock[%d] error with code[%d], dest[%s:%d]\r\n", 
			__func__, s, c, inet_ntoa(sock->sin_addr), ntohs(sock->sin_port));
		/* Indicates the caller to try again. */
		if (errno != EAGAIN)
		{
			errno = EAGAIN;
		}
	}

	return c;
}

void tftp_handle_request(tftp_message *m, ssize_t len,
	struct sockaddr_in *client_sock, socklen_t slen)
{
	int s;
	//struct protoent *pp;
	char *filename, *mode_s, *end;
	FILE *fd;
	int mode, io_accu = 0, batch = 0;
	uint16_t opcode;
	struct sockaddr_in this_side;

	/* open new socket, on new port, to handle client request */

#if 0
	if ((pp = getprotobyname("udp")) == 0) {
		fprintf(stderr, "server: getprotobyname() error\n");
		exit(1);
	}
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("server: socket()");
		exit(1);
	}

	int time_out = RECV_TIMEOUT;

	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(int)) < 0) {
		perror("server: setsockopt()");
		exit(1);
	}

	/* Bind to local port. */
	this_side.sin_family = AF_INET;
	this_side.sin_port = htons(TFTP_PORT_START);
	this_side.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(this_side.sin_zero), 0, sizeof(this_side.sin_zero));

	/* bind socket to the server address */
	if (bind(s, (struct sockaddr *)&this_side, sizeof(struct sockaddr)) == -1)
	{
		_hx_printf("[%s]bind sock to port[%d] fail.\r\n", 
			__func__, ntohs(this_side.sin_port));
		exit(1);
	}

	/* parse client request */

	filename = m->request.filename_and_mode;
	end = &filename[len - 2 - 1];

	if (*end != '\0') {
		printf("%s.%u: invalid filename or mode\n",
			inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
		tftp_send_error(s, 0, "invalid filename or mode", client_sock, slen);
		exit(1);
	}

	mode_s = strchr(filename, '\0') + 1;

	if (mode_s > end) {
		printf("%s.%u: transfer mode not specified\n",
			inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
		tftp_send_error(s, 0, "transfer mode not specified", client_sock, slen);
		exit(1);
	}

	if (strncmp(filename, "../", 3) == 0 || strstr(filename, "/../") != NULL ||
		(filename[0] == '/' && strncmp(filename, base_directory, strlen(base_directory)) != 0)) {
		printf("%s.%u: filename outside base directory\n",
			inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
		tftp_send_error(s, 0, "filename outside base directory", client_sock, slen);
		exit(1);
	}

	opcode = ntohs(m->opcode);
	fd = fopen(filename, opcode == RRQ ? "r" : "w");

	if (fd == NULL) {
		perror("server: fopen()");
		tftp_send_error(s, errno, "open file error", client_sock, slen);
		exit(1);
	}

	mode = (0 == strcmp(mode_s, "netascii")) ? NETASCII :
		(0 == strcmp(mode_s, "octet")) ? OCTET : 0;

	if (mode == 0) {
		printf("%s.%u: invalid transfer mode\n",
			inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
		tftp_send_error(s, 0, "invalid transfer mode", client_sock, slen);
		exit(1);
	}

	printf("%s.%u: request received: %s '%s' %s\n",
		inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
		ntohs(m->opcode) == RRQ ? "get" : "put", filename, mode_s);

	// TODO: add netascii handling

	if (opcode == RRQ) {
		tftp_message m;

		uint8_t data[512];
		ssize_t dlen, c, send_bytes = 0;

		uint16_t block_number = 0;

		int countdown;
		int to_close = 0;

		while (!to_close) {

			dlen = fread(data, 1, sizeof(data), fd);
			block_number++;

			if (dlen < 512) {
				/* last data block to send. */
				to_close = 1;
			}

			for (countdown = RECV_RETRIES; countdown; countdown--) {

				send_bytes = tftp_send_data(s, block_number, data, dlen, client_sock, slen);

				if (send_bytes < 0) {
					printf("%s.%u: send failed, exit.\r\n",
						inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
					goto __TERMINAL;
				}

				/* Try to get the acknowledge from client. */
				c = tftp_recv_message(s, &m, client_sock, &slen);

				if (c >= 0 && c < 4) {
					printf("%s.%u: message with invalid size received\n",
						inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
					tftp_send_error(s, 0, "invalid request size", client_sock, slen);
					goto __TERMINAL;
				}

				if (c >= 4) {
					/* Valid acknowledge received, process it accordingly. */
					if (ntohs(m.opcode) == ERROR) {
						printf("%s.%u: error message received: %u %s\n",
							inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
							ntohs(m.error.error_code), m.error.error_string);
						goto __TERMINAL;
					}

					if (ntohs(m.opcode) != ACK) {
						printf("%s.%u: invalid message during transfer received\n",
							inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
						tftp_send_error(s, 0, "invalid message during transfer", client_sock, slen);
						goto __TERMINAL;
					}

					if (ntohs(m.ack.block_number) != block_number) 
					{
						_hx_printf("[%s]invalid block number received[%d], should be[%d]\r\n",
							__func__, ntohs(m.ack.block_number), block_number);
						/* Invalid block number. */
						if (ntohs(m.ack.block_number) == block_number - 1)
						{
							/*
							* One reason may lead by the drop of
							* packet, i.e, the packet sent to client is miss
							* and the client send the ack again. Just try
							* to send again.
							*/
							continue;
						}
						else
						{
							/* Unknown reason, giveup. */
							printf("%s.%u: invalid ack number[%d] received.\r\n",
								inet_ntoa(client_sock->sin_addr), 
								ntohs(client_sock->sin_port), ntohs(m.ack.block_number));
							tftp_send_error(s, 0, "invalid ack number from client. ", 
								client_sock, slen);
							goto __TERMINAL;
						}
					}

					/* Block number is ok, client receive OK. */
					io_accu += send_bytes;
					batch += send_bytes;
					if (batch >= (128 * 1024))
					{
						batch = 0;
						_hx_printf("[%s][%d] bytes sent to[%s:%d]\r\n", __func__, io_accu,
							inet_ntoa(client_sock->sin_addr),
							ntohs(client_sock->sin_port));
					}
					break;
				}

				if (errno != EAGAIN) {
					printf("%s.%u: transfer killed\n",
						inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
					goto __TERMINAL;
				}
			}

			if (!countdown) {
				printf("%s.%u: transfer timed out\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
				goto __TERMINAL;
			}

			if (ntohs(m.opcode) == ERROR) {
				printf("%s.%u: error message received: %u %s\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
					ntohs(m.error.error_code), m.error.error_string);
				goto __TERMINAL;
			}

			if (ntohs(m.opcode) != ACK) {
				printf("%s.%u: invalid message during transfer received\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
				tftp_send_error(s, 0, "invalid message during transfer", client_sock, slen);
				goto __TERMINAL;
			}
#if 0
			if (ntohs(m.ack.block_number) != block_number) { // the ack number is too high
				printf("%s.%u: invalid ack number[%d] received.\r\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
				tftp_send_error(s, 0, "invalid ack number", client_sock, slen);
				goto __TERMINAL;
			}
#endif
		}
	}
	/* Put file to server. */
	else if (opcode == WRQ) {

		tftp_message m;

		ssize_t c;

		uint16_t block_number = 0;

		int countdown;
		int to_close = 0;

		c = tftp_send_ack(s, block_number, client_sock, slen);

		if (c < 0) {
			printf("%s.%u: send ack fail with err[%d], quit.\r\n",
				inet_ntoa(client_sock->sin_addr), 
				ntohs(client_sock->sin_port), c);
			goto __TERMINAL;
		}

		while (!to_close) {

			for (countdown = RECV_RETRIES; countdown; countdown--) {

				c = tftp_recv_message(s, &m, client_sock, &slen);

				if (c >= 0 && c < 4) {
					printf("%s.%u: message with invalid size[%d] received\n",
						inet_ntoa(client_sock->sin_addr), 
						ntohs(client_sock->sin_port), c);
					tftp_send_error(s, 0, "invalid request size", client_sock, slen);
					goto __TERMINAL;
				}

				if (c >= 4) {
					break;
				}

				if (errno != EAGAIN) {
					printf("%s.%u: transfer killed\n",
						inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
					goto __TERMINAL;
				}

				c = tftp_send_ack(s, block_number, client_sock, slen);

				if (c < 0) {
					printf("%s.%u: send ack failed with err[%d], exit.\r\n",
						inet_ntoa(client_sock->sin_addr), 
						ntohs(client_sock->sin_port), c);
					goto __TERMINAL;
				}
			}

			if (!countdown) {
				printf("%s.%u: transfer timed out\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
				goto __TERMINAL;
			}

			block_number++;

			if (c < sizeof(m.data)) {
				to_close = 1;
			}

			if (ntohs(m.opcode) == ERROR) {
				printf("%s.%u: error message received: %u %s\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port),
					ntohs(m.error.error_code), m.error.error_string);
				goto __TERMINAL;
			}

			if (ntohs(m.opcode) != DATA) {
				printf("%s.%u: invalid message during transfer received\n",
					inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));
				tftp_send_error(s, 0, "invalid message during transfer", client_sock, slen);
				goto __TERMINAL;
			}

			if (ntohs(m.ack.block_number) != block_number) {
				printf("%s.%u: invalid block number[%d] received\n",
					inet_ntoa(client_sock->sin_addr), 
					ntohs(client_sock->sin_port), ntohs(m.ack.block_number));
				tftp_send_error(s, 0, "invalid block number", client_sock, slen);
				goto __TERMINAL;
			}

			c = fwrite(m.data.data, 1, c - 4, fd);
			if (c < 0) {
				_hx_printf("[%s]error to write file[%d]\r\n", __func__, c);
				goto __TERMINAL;
			}
			else {
				io_accu += c;
				if (0 == io_accu % (128 * 1024))
				{
					_hx_printf("[%s]receive and write [%d] bytes to file.\r\n", __func__, io_accu);
				}
			}

			c = tftp_send_ack(s, block_number, client_sock, slen);

			if (c < 0) {
				printf("%s.%u: send ack failed with err[%d], exit. \r\n",
					inet_ntoa(client_sock->sin_addr), 
					ntohs(client_sock->sin_port), c);
				goto __TERMINAL;
			}
		}
	}

	printf("%s.%u: transfer completed\n",
		inet_ntoa(client_sock->sin_addr), ntohs(client_sock->sin_port));

__TERMINAL:
	fclose(fd);
	closesocket(s);

	/* 
	 * Must NOT invoke exit() since it will lead the 
	 * termination of tftp server thread, and there is only
	 * one thread in current's implementation under hellox.
	 */
	//exit(0);
}

int _hxmain(int argc, char *argv[])
{
	int s;
	uint16_t port = 0;
	//struct servent *ss;
	struct sockaddr_in server_sock;

	if (argc < 2) {
		printf("usage:\n\t%s [base directory] [port]\n", argv[0]);
		exit(1);
	}

#if 0
	base_directory = argv[1];
	if (chdir(base_directory) < 0) {
		perror("server: chdir()");
		exit(1);
	}
#endif

	if (argc > 2) {
		if (sscanf(argv[2], "%hu", &port)) {
			port = htons(port);
		}
		else {
			printf("error: invalid port number\n");
			exit(1);
		}
	}
	else {
#if 0
		if ((ss = getservbyname("tftp", "udp")) == 0) {
			fprintf(stderr, "server: getservbyname() error\n");
			exit(1);
		}
#endif
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("server: socket() error");
		exit(1);
	}

	server_sock.sin_family = AF_INET;
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sock.sin_port = port ? port : htons(DEFAULT_TFTP_PORT); //ss->s_port;

	if (bind(s, (struct sockaddr *) &server_sock, sizeof(server_sock)) == -1) {
		_hx_printf("[%s]server: bind() error.\r\n", __func__);
		closesocket(s);
		exit(1);
	}

	//signal(SIGCLD, (void *)cld_handler);

	_hx_printf("TFTP server for HelloX started, version:%d.%d\r\n", __MAJOR_VER, __MINOR_VER);
	_hx_printf("listening on [%d]:\r\n", ntohs(server_sock.sin_port));

	while (1) {
		struct sockaddr_in client_sock;
		socklen_t slen = sizeof(client_sock);
		ssize_t len;

		tftp_message message;
		uint16_t opcode;

		if ((len = tftp_recv_message(s, &message, &client_sock, &slen)) < 0) {
			continue;
		}

		if (len < 4) {
			printf("%s.%u: request with invalid size received\n",
				inet_ntoa(client_sock.sin_addr), ntohs(client_sock.sin_port));
			tftp_send_error(s, 0, "invalid request size", &client_sock, slen);
			continue;
		}

		opcode = ntohs(message.opcode);

		if (opcode == RRQ || opcode == WRQ) {
			/* Invoke the handler to process request. */
			tftp_handle_request(&message, len, &client_sock, slen);
			break;
		}
		else 
		{
			printf("%s.%u: invalid request received: opcode[%d] \n",
				inet_ntoa(client_sock.sin_addr), ntohs(client_sock.sin_port),
				opcode);
			tftp_send_error(s, 0, "invalid opcode", &client_sock, slen);
		}

	}

	closesocket(s);
	_hx_printf("TFTP Server for HelloX quit.\r\n");
	return 0;
}
