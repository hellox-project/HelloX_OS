//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 19, 2021
//    Module Name               : ttysrv.h
//    Module Funciton           : 
//                                TTY(TeleTypes) server's definitions,
//                                constants, and proto-types are defined
//                                in this file.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

/*
 * TTY, is very old computer inter-action mechanism that in
 * early days of computer emerges, it uses telegram lines
 * as input/output between user and computer. I never seen this
 * thing and only has a little knowledge about, but I feel the
 * name 'TTY' is very cool, so we use it here as the telnet
 * server side's implementation in HelloX.
 */

#ifndef __TTYSVR_H__
#define __TTYSVR_H__

/* Telnet server's default port. */
#define TELNET_SERVER_PORT 23

/* Default timeout value for waiting input. */
#define TELNET_RECV_TIMEOUT 200

/* NVT commands. */
#define NVT_IAC          255    /* Interpret As Command. */
#define NVT_CMD_EOF      236
#define NVT_CMD_SUSP     237
#define NVT_CMD_ABORT    238
#define NVT_CMD_EOR      239
#define NVT_CMD_SE       240
#define NVT_CMD_NOP      241
#define NVT_CMD_DM       242
#define NVT_CMD_BRK      243
#define NVT_CMD_IP       244
#define NVT_CMD_AO       245
#define NVT_CMD_AYT      246
#define NVT_CMD_EC       247
#define NVT_CMD_EL       248
#define NTV_CMD_GA       249
#define NVT_CMD_SB       250
#define NVT_CMD_WILL     251
#define NVT_CMD_WONT     252
#define NVT_CMD_DO       253
#define NVT_CMD_DONT     254

/* NVT options. */
#define NVT_OPTION_ECHO            1
#define NVT_OPTION_STATUS          5
#define NVT_OPTION_CLOCKID         6
#define NVT_OPTION_TERMINAL_TYPE   24
#define NVT_OPTION_WINDOW_SIZE     31
#define NVT_OPTION_TERMINAL_SPEED  32
#define NVT_OPTION_REMOTE_FCONTROL 33
#define NVT_OPTION_ROW_MODE        34
#define NVT_OPTION_ENV             36

/* 
 * TTY server is the only global object that 
 * interfacing with OS kernel.
 */
typedef struct tag__TTY_SERVER {
	/* Server thread's handle. */
	HANDLE server_thread;

	/* 
	 * Default socket of client. 
	 * If set, means the connection is established
	 * and data can be xfered on this connection.
	 */
	int default_sock;

	/* Initiliazer of tty server. */
	BOOL (*Initialize)();
	/* Notify by client that new connection established. */
	void (*OnNewConnection)(int sock);

	/* Output routines. */
	void (*SendChar)(char byte);
	void (*ChangeLine)();
	void (*PrintString)(const char* string);
}__TTY_SERVER;

/* Global tty server object. */
extern __TTY_SERVER ttyServer;

#endif //__TTYSVR_H__
