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
	SESSION_IDLE,
	SESSION_RUNNING
}__PPPOE_SESSION_STATUS;

/* Global variables for one PPPoE session,simple to management. */
typedef struct tag__PPPOE_INSTANCE{
	struct tag__PPPOE_INSTANCE* pNext;
	int instance_id;
	int ppp_session_id; /* ID of PPP session. */
	__PPPOE_SESSION_STATUS status;
	__PPPOE_AUTH_TYPE authType;
	char session_name[PPPOE_SESSION_NAME_LEN + 1];
	char user_name[PPPOE_USER_NAME_LEN + 1];
	char password[PPPOE_PASSWORD_LEN + 1];
	/* Ethernet interface the session based on. */
	__ETHERNET_INTERFACE* pEthInt;
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
	__ETHERNET_INTERFACE* pEthInt;
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
#define PPPOE_MAX_PENDINGLIST_SIZE 128

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
	__ETHERNET_INTERFACE* pEthInt;
	__PPPOE_INSTANCE* pInstance;
}__PPPOE_ETHIF_BINDING;

/* Binding object array and it's size. */
extern __PPPOE_ETHIF_BINDING pppoeBinding[PPPOE_MAX_INSTANCE_NUM];
extern int current_bind_num;

#endif //__NET_CFG_PPPOE

#endif //__PPPOE_H__
