//***********************************************************************/
//    Author                    : twind
//    Original Date             : Nov,02 2015
//    Module Name               : KAPI.H
//    Module Funciton           : 
//                                Declares all socket routines can be
//                                used by other modules in kernel.
//                                This file is used by user application,so all
//                                routines declared in it is system calls.
//
//    Last modified Author      : twind
//    Last modified Date        : Jan 09,2012
//    Last modified Content     :
//                                1. System calls offered by GUI module are added.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "KAPI.H"

#ifndef __K_SKCOET_H__
#define __K_SKCOET_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char  u8_t;
typedef unsigned short u16_t;
typedef unsigned long  u32_t;


/** For compatibility with BSD code */
struct in_addr {
	u32_t s_addr;
};

/* This is the aligned version of ip_addr_t,
used as local variable, on the stack, etc. */
struct ip_addr {
	u32_t addr;
};

/* members are in network byte order */
struct sockaddr_in {
  u8_t sin_len;
  u8_t sin_family;
  u16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr 
{
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

#ifndef socklen_t
#  define socklen_t u32_t
#endif

#define MAX_ETH_NAME_LEN     31

//A helper structure used to change an ethernet interface's IP configuration,
//it contains the parameters will be changed.
typedef struct tag__ETH_IP_CONFIG{
	char   ifName[2];                //Interface's name.
	char   ethName[MAX_ETH_NAME_LEN + 1];
	struct ip_addr ipaddr;           //IP address when DHCP disabled.
	struct ip_addr mask;             //Subnet mask of the interface.
	struct ip_addr defgw;            //Default gateway.
	struct ip_addr dnssrv_1;         //Primary DNS server address.
	struct ip_addr dnssrv_2;         //Second DNS server address.
	DWORD  dwDHCPFlags;              //Flags of DHCP operation.
}__ETH_IP_CONFIG;

//Interface state data associate to ethernet interface,HelloX specified. All this state
//information is saved in ethernet object.
typedef struct tag__ETH_INTERFACE_STATE{
	__ETH_IP_CONFIG    IpConfig;
	__ETH_IP_CONFIG    DhcpConfig;   //DHCP server configuration,for example,the offering IP address range.
	BOOL               bDhcpCltEnabled;
	BOOL               bDhcpSrvEnabled;
	BOOL               bDhcpCltOK;   //If get IP address successfully in DHCP client mode.
	DWORD              dwDhcpLeasedTime;  //Leased time of the DHCP configuration,in million second.

	//Variables to control the sending process.
	BOOL               bSendPending; //Indicate if there is pending packets to send.

	//Statistics information from ethernet level.
	DWORD              dwFrameSend;         //How many frames has been sent since boot up.
	DWORD              dwFrameSendSuccess;  //The number of frames sent out successfully.
	DWORD              dwFrameRecv;         //How many frames has been received since boot up.
	DWORD              dwFrameRecvSuccess;  //Delivery pkt to upper layer successful.
	DWORD              dwTotalSendSize;     //How many bytes has been sent since boot.
	DWORD              dwTotalRecvSize;     //Receive size.
}__ETH_INTERFACE_STATE;

#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define  SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define  SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define  SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define  SO_KEEPALIVE   0x0008 /* keep connections alive */
#define  SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define  SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */
#define  SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define  SO_LINGER      0x0080 /* linger on close if data present */
#define  SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define  SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */

#define SO_DONTLINGER   ((int)(~SO_LINGER))

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF    0x1001    /* Unimplemented: send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT  0x1004    /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* Unimplemented: send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
#define SO_CONTIMEO  0x1009    /* Unimplemented: connect timeout */
#define SO_NO_CHECK  0x100a    /* don't create UDP checksum */

#define FD_SETSIZE 64

/*
 * Structure used for manipulating linger option.
 */
struct linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time */
};

typedef struct fd_set {
	unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;

int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int bind(int s, const struct sockaddr *name, socklen_t namelen);

int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
int close(int s);
int connect(int s, const struct sockaddr *name, socklen_t namelen);
int listen(int s, int backlog);
int recv(int s, void *mem, size_t len, int flags);
int read(int s, void *mem, size_t len);
int recvfrom(int s, void *mem, size_t len, int flags,struct sockaddr *from, socklen_t *fromlen);

int send(int s, const void *dataptr, size_t size, int flags);
int sendto(int s, const void *dataptr, size_t size, int flags,	const struct sockaddr *to, socklen_t tolen);
int socket(int domain, int type, int protocol);
int write(int s, const void *dataptr, size_t size);
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,struct timeval *timeout);

int ioctl(int s, long cmd, void *argp);
int fcntl(int s, int cmd, int val);

#ifdef __cplusplus
}
#endif


#endif //__K_SKCOET_H__
