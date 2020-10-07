//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Sep 09,2017
//    Module Name               : pppoe.h
//    Module Funciton           : 
//                                PPP over Ethernet functions header,the entry
//                                point of PPPoE,global variables,global functions,
//                                are put into this file.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __PPPOE_H__
#define __PPPOE_H__

/* Compile only the PPPOE switch is enabled. */
#ifdef __CFG_NET_PPPOE

/* For spin lock in SMP. */
#include <config.h>
#include <TYPES.H>

/* For ip_addr_t. */
#include "lwip/ip_addr.h"
/* For err_t type. */
#include "lwip/err.h"

/* PPPoE instance(session) name length. */
#define PPPOE_SESSION_NAME_LEN 31

/* Maximal PPPoE user name length. */
#define PPPOE_USER_NAME_LEN 31

/* Maximal PPPoE password. */
#define PPPOE_PASSWORD_LEN 31

/* maximal length of authentication type in string. */
#define PPPOE_AUTH_TYPE_LEN 31

/* Periodic scanning timer's ID and time spanning. */
#define PPPOE_PERIODICTIMER_ID 1024
#define PPPOE_PERIODICTIMER_SPAN (1100 * 5) //(1000 * 5)

/* 
 * Maximal restart counter of one pppoe session. 
 * The session will never be restarted by system automatically
 * when restart_count reach this value.
 * NOTE: Because of the existing bugs in pppoe source code,
 *       it will lead system crash after several(>3) times
 *       of restart on one pppoe instance,so we set this
 *       restriction,it will be removed when bugs shooted.
 */
#define PPPOE_SESSION_RESTART_COUNT 64

/*
 * Authentication types of PPPoE,default is ANY,that is,try the
 * method following ANY one by one,until success or all failed.
 */
typedef enum tag__PPPOE_AUTH_TYPE{
	PPPOE_AUTH_NONE = 0,
	PPPOE_AUTH_ANY,
	PPPOE_AUTH_PAP,
	PPPOE_AUTH_CHAP
}__PPPOE_AUTH_TYPE;

/* PPPoE session status. */
typedef enum tag__PPPOE_SESSION_STATUS{
	SESSION_IDLE = 0,      /* Just initialized. */
	SESSION_CONNECTING,    /* Connecting in progress. */
	SESSION_CONNECTED,     /* Session established. */
	SESSION_DISCONNECTED,  /* Session disconnected. */
	SESSION_DISABLED       /* Disabled manually. */
}__PPPOE_SESSION_STATUS;

/* Global variables for one PPPoE session,simple to management. */
typedef struct tag__PPPOE_INSTANCE{
	struct tag__PPPOE_INSTANCE* pNext;
	int instance_id;
	int ppp_session_id; /* ID of PPP session. */
	int restart_count;
	__PPPOE_SESSION_STATUS status;
	__PPPOE_AUTH_TYPE authType;
	char session_name[PPPOE_SESSION_NAME_LEN + 1];
	char user_name[PPPOE_USER_NAME_LEN + 1];
	char password[PPPOE_PASSWORD_LEN + 1];
	/* Generic netif the session based on. */
	__GENERIC_NETIF* pGenif;
	/* netif that the session based on. */
	struct netif* netif;
	/* netif created after session connected. */
	struct netif* ppif;
}__PPPOE_INSTANCE;

/* Messages PPPoE manager main thread can handle. */
#define PPPOE_MSG_POSTFRAME        (MSG_USER_START + 0x01)
#define PPPOE_MSG_SENDPACKET       (MSG_USER_START + 0x02)
#define PPPOE_MSG_LINKSTATUS       (MSG_USER_START + 0x03)
#define PPPOE_MSG_STARTSESSION     (MSG_USER_START + 0x04)
#define PPPOE_MSG_STOPSESSION      (MSG_USER_START + 0x05)

/*
* Helper object used as parameter when post message.
* Pack the parameters all to one package and delivery to
* PPPoE main thread by message,since dwParam can only
* carry one pointer.
*/
typedef struct tag__PPPOE_POSTFRAME_BLOCK{
	__PPPOE_INSTANCE* pInstance;
	//__ETHERNET_INTERFACE* pEthInt;
	__u16 frame_type;
	struct pbuf* p;
	struct tag__PPPOE_POSTFRAME_BLOCK* pNext;
}__PPPOE_POSTFRAME_BLOCK;

/* Same as above object,to pack all parameters when send packet. */
typedef struct tag__PPP_SENDPACKET_BLOCK{
	struct netif* out_if;
	struct pbuf* pkt_buff;
	ip_addr_t addr;
	struct tag__PPP_SENDPACKET_BLOCK* pNext;
}__PPP_SENDPACKET_BLOCK;

/* 
 * Maximal out going list and incoming list's size,the out going packet or
 * incoming frame will be droped if the list size exceed this value.
 */
#define PPPOE_MAX_PENDINGLIST_SIZE 256

/* PPPoE manager object. */
typedef struct tag__PPPOE_MANAGER{
	/* Global PPPoE instance list. */
	__PPPOE_INSTANCE* pInstanceList;

	/* List of out going PPP packet(s). */
	__PPP_SENDPACKET_BLOCK* pOutgFirst;
	__PPP_SENDPACKET_BLOCK* pOutgLast;
	volatile int nOutgSize;

	/* List of incoming PPPoE frame. */
	__PPPOE_POSTFRAME_BLOCK* pIncomFirst;
	__PPPOE_POSTFRAME_BLOCK* pIncomLast;
	volatile int nIncomSize;

	/* Spin lock under SMP. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/* Handle of PPPoE main thread. */
	HANDLE hMainThread;

	/* Initializer of PPPoE manager. */
	BOOL (*Initialize)(struct tag__PPPOE_MANAGER* pMgr);
	/* Create a PPPoE instance. */
	__PPPOE_INSTANCE* (*CreatePPPoEInstance)(char* ethName, char* instName, char* user_name,
		char* password, __PPPOE_AUTH_TYPE authType);

	/* Start a PPPoE session. */
	BOOL (*StartPPPoE)(__PPPOE_INSTANCE* pInstance);
	BOOL (*StartPPPoEByName)(char* instance);

	/* Stop a PPPoE session. */
	BOOL (*StopPPPoE)(__PPPOE_INSTANCE* pInstance);
	BOOL (*StopPPPoEByName)(char* instance);

	/* Destroy a PPPoE instance. */
	void (*DestroyPPPoEInstance)(__PPPOE_INSTANCE* pInstance);
	void (*DestroyPPPoEInstanceByName)(char* instance);

	/* Post a PPPoE frame to main thread. */
	BOOL (*PostFrame)(__PPPOE_POSTFRAME_BLOCK* pBlock);

	/* Send layer3 packet out through PPP/PPPoE main thread. */
	BOOL (*SendPacket)(struct netif* netif, struct pbuf* pb, ip_addr_t* ipaddr);
}__PPPOE_MANAGER;

/* 
 * Send out a IP packet through PPP session. 
 * This routine's implementation is in ppp.c file,
 * and this routine is called by SendPacket.
 */
err_t pppifOutput(struct netif *netif, struct pbuf *pb, ip_addr_t *ipaddr);

/* PPPoE main thread's name. */
#define PPPOE_MAIN_THREAD_NAME "pppoeMain"

/* Global PPPoE manager object. */
extern __PPPOE_MANAGER pppoeManager;

/* List all PPPoE instance in system. */
void ListPPPoE();

/* Maximal PPPoE instance in ssystem. */
#define PPPOE_MAX_INSTANCE_NUM 4

/*
 * Record the binding relationship of PPPoE and ethernet interface
 * object.It's used by PPPoE protocol object to fit HelloX's network
 * frame.
 */
typedef struct tag__PPPOE_ETHIF_BINDING{
	/* 
	 * pEthInt Should no usage when genif available. 
	 * But preserve it for backward compitible,since some
	 * NIC's driver(such as usb ethernet) is not revised
	 * to fit genif framework.
	 */
	__ETHERNET_INTERFACE* pEthInt; 

	/* The genif that session based on. */
	__GENERIC_NETIF* pGenif;
	/* The lwIP netif that session based on. */
	struct netif* netif;
	__PPPOE_INSTANCE* pInstance;
}__PPPOE_ETHIF_BINDING;

/* Binding object array and it's size. */
extern __PPPOE_ETHIF_BINDING pppoeBinding[PPPOE_MAX_INSTANCE_NUM];
extern int current_bind_num;

#endif //__NET_CFG_PPPOE

#endif //__PPPOE_H__
