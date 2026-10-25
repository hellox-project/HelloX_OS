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
#define NVT_CMD_GA       249
#define NVT_CMD_SB       250
#define NVT_CMD_WILL     251
#define NVT_CMD_WONT     252
#define NVT_CMD_DO       253
#define NVT_CMD_DONT     254

/* NVT options. */
#define NVT_OPTION_ECHO            1
#define NVT_OPTION_SUPPRESS_GA     3
#define NVT_OPTION_STATUS          5
#define NVT_OPTION_CLOCKID         6
#define NVT_OPTION_TERMINAL_TYPE   24
#define NVT_OPTION_WINDOW_SIZE     31
#define NVT_OPTION_TERMINAL_SPEED  32
#define NVT_OPTION_REMOTE_FCONTROL 33
#define NVT_OPTION_ROW_MODE        34
#define NVT_OPTION_ENV             36

/* Escape sequence and control key constants */
#define ESC_KEY     0x1B  /* Escape character */
#define DEL_KEY     0x7F  /* Delete key */
#define BS_KEY      0x08  /* Backspace key */
#define TAB_KEY     0x09  /* Tab key */
#define CR_KEY      0x0D  /* Carriage return */
#define LF_KEY      0x0A  /* Line feed */

/* Escape sequences for common control keys */
#define ESC_SEQ_UP      "\x1B[A"
#define ESC_SEQ_DOWN    "\x1B[B"
#define ESC_SEQ_RIGHT   "\x1B[C"
#define ESC_SEQ_LEFT    "\x1B[D"
#define ESC_SEQ_HOME    "\x1B[H"
#define ESC_SEQ_END     "\x1B[F"
#define ESC_SEQ_INSERT  "\x1B[2~"
#define ESC_SEQ_DELETE  "\x1B[3~"
#define ESC_SEQ_PGUP    "\x1B[5~"
#define ESC_SEQ_PGDN    "\x1B[6~"
#define ESC_SEQ_F1      "\x1BOP"
#define ESC_SEQ_F2      "\x1BOQ"
#define ESC_SEQ_F3      "\x1BOR"
#define ESC_SEQ_F4      "\x1BOS"
#define ESC_SEQ_F5      "\x1B[15~"
#define ESC_SEQ_F6      "\x1B[17~"
#define ESC_SEQ_F7      "\x1B[18~"
#define ESC_SEQ_F8      "\x1B[19~"
#define ESC_SEQ_F9      "\x1B[20~"
#define ESC_SEQ_F10     "\x1B[21~"
#define ESC_SEQ_F11     "\x1B[23~"
#define ESC_SEQ_F12     "\x1B[24~"

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
