//***********************************************************************/
//    Author                    : tywind
//    Original Date             : 15 MAY,2016
//    Module Name               : telnet2.h
//    Module Funciton           : 
//    Description               : Implementation code of telnet application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef	__TELNET_2_H_
#define __TELNET_2_H_


#define	IAC		255		       /* interpret as command: */
#define	DONT	254		       /* you are not to use option */
#define	DO		253		       /* please, you use option */
#define	WONT	252		       /* I won't use option */
#define	WILL	251		       /* I will use option */
#define	SB		250		       /* interpret as subnegotiation */
#define	SE		240		       /* end sub negotiation */

#define GA      249		       /* you may reverse the line */
#define EL      248		       /* erase the current line */
#define EC      247		       /* erase the current character */
#define	AYT		246		       /* are you there */
#define	AO		245		       /* abort output--but let prog finish */
#define	IP		244		       /* interrupt process--permanently */
#define	BREAK	243		       /* break */
#define DM      242		       /* data mark--for connect. cleaning */
#define NOP     241		       /* nop */
#define EOR     239		       /* end of record (transparent mode) */
#define ABORT   238		       /* Abort process */
#define SUSP    237		       /* Suspend process */
#define xEOF    236		       /* End of file: EOF is already used... */

#define  CRLF                     0xD
#define  TELNET_RECV			  4096
#define  TELNET_SEND			  4096
#define  TELNET_MSG_LEN           256
#define  TELNET_BUF_LEN           256
#define  TBL                      256



enum {
	OPTINDEX_NAWS,
	OPTINDEX_TSPEED,
	OPTINDEX_TTYPE,
	OPTINDEX_OENV,
	OPTINDEX_NENV,
	OPTINDEX_ECHO,
	OPTINDEX_WE_SGA,
	OPTINDEX_THEY_SGA,
	OPTINDEX_WE_BIN,
	OPTINDEX_THEY_BIN,
	NUM_OPTS
};


#define	TELQUAL_IS	0	       /* option is... */
#define	TELQUAL_SEND	1	       /* send option */
#define	TELQUAL_INFO	2	       /* ENVIRON: informational version of IS */
#define BSD_VAR 1
#define BSD_VALUE 0
#define RFC_VAR 0
#define RFC_VALUE 1

struct Opt {
	int send;			       /* what we initially send */
	int nsend;			       /* -ve send if requested to stop it */
	int ack, nak;		       /* +ve and -ve acknowledgements */
	int option;			       /* the option code */
	int index;			       /* index into telnet->opt_states[] */
	enum {
		REQUESTED, ACTIVE, INACTIVE, REALLY_INACTIVE
	} initial_state;
};


#define TELOPTS(X) \
	X(BINARY, 0)                       /* 8-bit data path */ \
	X(ECHO, 1)                         /* echo */ \
	X(RCP, 2)                          /* prepare to reconnect */ \
	X(SGA, 3)                          /* suppress go ahead */ \
	X(NAMS, 4)                         /* approximate message size */ \
	X(STATUS, 5)                       /* give status */ \
	X(TM, 6)                           /* timing mark */ \
	X(RCTE, 7)                         /* remote controlled transmission and echo */ \
	X(NAOL, 8)                         /* negotiate about output line width */ \
	X(NAOP, 9)                         /* negotiate about output page size */ \
	X(NAOCRD, 10)                      /* negotiate about CR disposition */ \
	X(NAOHTS, 11)                      /* negotiate about horizontal tabstops */ \
	X(NAOHTD, 12)                      /* negotiate about horizontal tab disposition */ \
	X(NAOFFD, 13)                      /* negotiate about formfeed disposition */ \
	X(NAOVTS, 14)                      /* negotiate about vertical tab stops */ \
	X(NAOVTD, 15)                      /* negotiate about vertical tab disposition */ \
	X(NAOLFD, 16)                      /* negotiate about output LF disposition */ \
	X(XASCII, 17)                      /* extended ascic character set */ \
	X(LOGOUT, 18)                      /* force logout */ \
	X(BM, 19)                          /* byte macro */ \
	X(DET, 20)                         /* data entry terminal */ \
	X(SUPDUP, 21)                      /* supdup protocol */ \
	X(SUPDUPOUTPUT, 22)                /* supdup output */ \
	X(SNDLOC, 23)                      /* send location */ \
	X(TTYPE, 24)                       /* terminal type */ \
	X(EOR, 25)                         /* end or record */ \
	X(TUID, 26)                        /* TACACS user identification */ \
	X(OUTMRK, 27)                      /* output marking */ \
	X(TTYLOC, 28)                      /* terminal location number */ \
	X(3270REGIME, 29)                  /* 3270 regime */ \
	X(X3PAD, 30)                       /* X.3 PAD */ \
	X(NAWS, 31)                        /* window size */ \
	X(TSPEED, 32)                      /* terminal speed */ \
	X(LFLOW, 33)                       /* remote flow control */ \
	X(LINEMODE, 34)                    /* Linemode option */ \
	X(XDISPLOC, 35)                    /* X Display Location */ \
	X(OLD_ENVIRON, 36)                 /* Old - Environment variables */ \
	X(AUTHENTICATION, 37)              /* Authenticate */ \
	X(ENCRYPT, 38)                     /* Encryption option */ \
	X(NEW_ENVIRON, 39)                 /* New - Environment variables */ \
	X(TN3270E, 40)                     /* TN3270 enhancements */ \
	X(XAUTH, 41)                       \
	X(CHARSET, 42)                     /* Character set */ \
	X(RSP, 43)                         /* Remote serial port */ \
	X(COM_PORT_OPTION, 44)             /* Com port control */ \
	X(SLE, 45)                         /* Suppress local echo */ \
	X(STARTTLS, 46)                    /* Start TLS */ \
	X(KERMIT, 47)                      /* Automatic Kermit file transfer */ \
	X(SEND_URL, 48)                    \
	X(FORWARD_X, 49)                   \
	X(PRAGMA_LOGON, 138)               \
	X(SSPI_LOGON, 139)                 \
	X(PRAGMA_HEARTBEAT, 140)           \
	X(EXOPL, 255)                      /* extended-options-list */


#define telnet_enum(x,y) TELOPT_##x = y,
enum { TELOPTS(telnet_enum) dummy=0 };
#undef telnet_enum


#define  TELMSG_CONNECT_FAILD     1
#define  TELMSG_CONNECT_OK        2
#define  TELMSG_SETUP_OK          3
#define  TELMSG_SETUP_FAILD       4
#define  TELMSG_RESPONSE_OK       5

typedef int (* telnet_cb)(unsigned int msg,const char* pMgsBuf);

typedef struct 
{
	int       sd;
	int       port;
	char      address[64];

	int       opt_states[NUM_OPTS];
	int       activated;

	int       sb_opt;
	int       login;
    void*     pcb;
	int       wparam;

	enum {
		TOP_LEVEL, SEENIAC, SEENWILL, SEENWONT, SEENDO, SEENDONT,
		SEENSB, SUBNEGOT, SUBNEG_IAC, SEENCR
	} state;

	telnet_cb cb;

}telnet_user;


void* telnet_new_seesion(const char* server,int port,telnet_cb pcb);

int   telnet_connect_seesion(void* p);

void  telnet_free_seesion(void* p);

int   telnet_send_input(void* p,const char* buf,int len);

int   telnet_recv_ret(void* p,char* buf,int len);

int   telnet_login(void* p,const char* user,const char* pass);


#endif //__TELNET_2_H_