//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,07 2014
//    Module Name               : ethmgr.h
//    Module Funciton           : 
//                                This file countains definitions of Ethernet
//                                Manager object,which is used to manipulate all
//                                ethernet related functions.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ETHMGR_H__
#define __ETHMGR_H__

#include <StdAfx.h> /* For HelloX common data types and APIs. */
#include <TYPES.H>  /* For __atomic_t or other types. */
#include "netcfg.h"
#include "genif.h"

/* Flags to enable or disable ethernet debugging. */
//#define __ETH_DEBUG

#ifndef __func__
#define __func__ __FUNCTION__
#endif

/*
 * Messages that can be sent to Ethernet core thread.
 * The thread is driven by message,and run in background.
 * EthernetManager can trigger specified functions by 
 * sending these messages to it.
 */
#define ETH_MSG_SEND    0x0001    //Send link level frame.
#define ETH_MSG_RECEIVE 0x0002    //Receive link level frame.
#define ETH_MSG_SCAN    0x0004    //Re-scan WiFi hot spot.
#define ETH_MSG_ASSOC   0x0008    //Associate a specified hot spot.
#define ETH_MSG_ACT     0x0010    //Activate the WiFi interface.
#define ETH_MSG_DEACT   0x0020    //De-activate the WiFi interface.
#define ETH_MSG_SETCONF 0x0040    //Configure a specified interface.
#define ETH_MSG_DHCPSRV 0x0080    //Set interface's DHCP server configuration.
#define ETH_MSG_DHCPCLT 0x0100    //Set interface's DHCP client configuration.
#define ETH_MSG_SHOWINT 0x0200    //Display interface's statistics informtion.
#define ETH_MSG_DELIVER 0x0400    //Delivery a packet to upper layer.
#define ETH_MSG_POSTFRAME 0x0800  //Post a frame to ethernet core.
#define ETH_MSG_BROADCAST 0x1000  //Broadcast a ethernet frame to all eth port(s).

/* Maximal length of ethernet interface's name. */
#define MAX_ETH_NAME_LEN     MAX_NETIF_NAME_LENGTH
/* MAC address's length. */
#define ETH_MAC_LEN          6   
/* Default maximal transmition unit. */
#define ETH_DEFAULT_MTU      1500
/* Ethernet frame header's length,assume 24 bytes to accomadate more data. */
#define ETH_HEADER_LEN       24  
/* Maximal Ethernet frame's length. */
#define ETH_MAX_FRAME_LEN    1600

/* Predefine ethernet interface object. */
struct tag__ETHERNET_INTERFACE;

/*
 * Ethernet buffer object,contains one ethernet frame 
 * and it's associated information element in each one
 * of this buffer.
 */
typedef struct tag__ETHERNET_BUFFER{
	/* Pointing to next one in list. */
	struct tag__ETHERNET_BUFFER* pNext;
	/* Signature. */
	DWORD dwSignature;
	/* Source MAC address. */
	__u8 srcMAC[ETH_MAC_LEN];
	/* Destination MAC address. */
	__u8 dstMAC[ETH_MAC_LEN];
	/* Ethernet frame's type. */
	__u16 frame_type;
	/* Built in buffer. */
	__u8 Buffer[ETH_DEFAULT_MTU + ETH_HEADER_LEN];
	/* Buffer's actual length. */
	__u16 buff_length;
	/* How many data bytes in buffer. */
	__u16 act_length;
	/* Data offset in buffer. */
	__u16 data_start;
	/* Ethernet buffer status. */
	__u16 buff_status;
#define ETHERNET_BUFFER_STATUS_FREE        0x00  //Free to use.
#define ETHERNET_BUFFER_STATUS_INITIALIZED 0x01  //Buffer is initialized.
#define ETHERNET_BUFFER_STATUS_PENDING     0x02  //Pending in queue.

	/*
	 * The incoming interface of the ethernet frame 
	 * was received.NULL means that the frame is
	 * originated from local host.
	 */
	struct tag__ETHERNET_INTERFACE* pInInterface;
	/*
	 * The out going interface of the ethernt frame,
	 * It's will be set to NULL when the frame is
	 * just received,or the frame is intend to be
	 * sent to all interface(s) except the incoming
	 * interface when commit to sending.
	 */
	struct tag__ETHERNET_INTERFACE* pOutInterface;
}__ETHERNET_BUFFER;

//A struct used to obtain DHCP configurations.
typedef struct tag__DHCP_CONFIG{
	__COMMON_NETWORK_ADDRESS network_addr;
	__COMMON_NETWORK_ADDRESS network_mask;
	__COMMON_NETWORK_ADDRESS default_gw;
	__COMMON_NETWORK_ADDRESS dns_server1;
	__COMMON_NETWORK_ADDRESS dns_server2;
}__DHCP_CONFIG;

//A helper structure used to change an ethernet interface's IP configuration,
//it contains the parameters will be changed.
typedef struct tag__ETH_IP_CONFIG{
	char   ifName[2];                          //Interface's name.
	char   ethName[MAX_ETH_NAME_LEN + 1];
	UCHAR  protoType;                          //i.e,IPv4,v6,or other protocol.
	__COMMON_NETWORK_ADDRESS ipaddr;           //IP address when DHCP disabled.
	__COMMON_NETWORK_ADDRESS mask;             //Subnet mask of the interface.
	__COMMON_NETWORK_ADDRESS defgw;            //Default gateway.
	__COMMON_NETWORK_ADDRESS dnssrv_1;         //Primary DNS server address.
	__COMMON_NETWORK_ADDRESS dnssrv_2;         //Second DNS server address.
	DWORD  dwDHCPFlags;                        //Flags of DHCP operation.
}__ETH_IP_CONFIG;

//Flags of DHCP operation.
#define ETH_DHCPFLAGS_DISABLE      0x00000001    //Disable DHCP functions.
#define ETH_DHCPFLAGS_ENABLE       0x00000002    //Enable DHCP function.
#define ETH_DHCPFLAGS_RESTART      0x00000004    //Restart DHCP functions on the given interface.
#define ETH_DHCPFLAGS_RELEASE      0x00000008    //Release DHCP configurations.
#define ETH_DHCPFLAGS_RENEW        0x00000010    //Renew DHCP configurations.

//A object used to transfer associating information.
typedef struct tag__WIFI_ASSOC_INFO{
	char   ssid[24];
	char   key[24];
	char   mode;     //0 for infrastructure,1 for adHoc.
	char   channel;  //Wireless channel.
	LPVOID pPrivate; //Used to transfer private information.
}__WIFI_ASSOC_INFO;

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

	//Ethernet interface status.
	DWORD              dwInterfaceStatus;
#define ETHERNET_INTERFACE_STATUS_UNKNOWN  0x0000
#define ETHERNET_INTERFACE_STATUS_UP       0x0001
#define ETHERNET_INTERFACE_STATUS_DOWN     0x0002

	//Statistics information from ethernet level.
	DWORD              dwFrameSend;         //How many frames has been sent since boot up.
	DWORD              dwFrameSendSuccess;  //The number of frames sent out successfully.
	DWORD              dwFrameRecv;         //How many frames has been received since boot up.
	DWORD              dwFrameRecvSuccess;  //Delivery pkt to upper layer successful.
	DWORD              dwTotalSendSize;     //How many bytes has been sent since boot.
	DWORD              dwTotalRecvSize;     //Receive size.
	DWORD              dwFrameBridged;      //Frame numbers that bridged out from this int.
	DWORD              dwTxErrorNum;        //Transmition error number.
	DWORD              dwRxErrorNum;        //Receive error number.
	DWORD              dwRxMcastNum;        //Multicast frames received at this interface.
	DWORD              dwTxMcastNum;        //Multicast frames send out through this if.
}__ETH_INTERFACE_STATE;

//Maximal protocols one Ethernet Interface can bind to.
#define MAX_BIND_PROTOCOL_NUM 4

struct __PROTO_INTERFACE_BIND{
	DWORD  frame_type_mask;  //If the type value in frame AND this value is not zero,then the
	                         //frame will be deliveried to the pProtocol object.
	LPVOID pProtocol;
	LPVOID pL3Interface;
};

/* 
 * Ethernet interface object,represents 
 * an dedicated ethernet interface. 
 */
typedef struct tag__ETHERNET_INTERFACE{
	/* Name and MAC address. */
	char ethName[MAX_ETH_NAME_LEN + 1];
	char ethMac[ETH_MAC_LEN];
	/* First element of sending buffer. */
	__ETHERNET_BUFFER SendBuffer;
	/* Interface state info. */
	__ETH_INTERFACE_STATE ifState;
	/* L3 protocol(s) bound to this interface. */
	struct __PROTO_INTERFACE_BIND Proto_Interface[MAX_BIND_PROTOCOL_NUM];
	/* Interface extension. */
	LPVOID pIntExtension;
	/* Link status,duplex and speed. */
	enum __LINK_STATUS link_status;
	enum __DUPLEX duplex;
	enum __ETHERNET_SPEED speed;

	/*
	 * Sending queue(list) size of the current interface.
	 */
	volatile size_t nSendingQueueSz;

	/* 
	 * Send a frame out from this interface. 
	 * Rules to handle the user specified pOutFrame:
	 *   1. If the sending process success and returns
	 *      TRUE, the pOutFrame should be destroyed in
	 *      this routine;
	 *   2. If FALSE is returned by this routine,then
	 *      pOutFrame should be kept, since the caller may
	 *      want to keep it and retry to send later.
	 */
	BOOL (*SendFrame)(struct tag__ETHERNET_INTERFACE*, 
		__ETHERNET_BUFFER* pOutFrame);
	/* Receive frame from this interface in polling mode. */
	__ETHERNET_BUFFER* (*RecvFrame)(struct tag__ETHERNET_INTERFACE*);
	/* Controling operation of this interface. */
	BOOL (*IntControl)(struct tag__ETHERNET_INTERFACE*, 
		DWORD dwOperations, 
		LPVOID pParam);
	/* 
	 * Interface specific show out routine.
	 * will be called when 'showint' command is invoked,
	 * if not NULL. 
	 */
	VOID (*IntSpecificShow)(struct tag__ETHERNET_INTERFACE*);
}__ETHERNET_INTERFACE;

//Operation protypes for convinence.
typedef BOOL (*__ETHOPS_SEND_FRAME)(__ETHERNET_INTERFACE*, __ETHERNET_BUFFER*);
typedef __ETHERNET_BUFFER* (*__ETHOPS_RECV_FRAME)(__ETHERNET_INTERFACE*);
typedef BOOL (*__ETHOPS_INT_CONTROL)(__ETHERNET_INTERFACE*, DWORD, LPVOID);
typedef BOOL (*__ETHOPS_INITIALIZE)(__ETHERNET_INTERFACE*);

/* Name of Ethernet core thread. */
#define ETH_THREAD_NAME  "netCore"

/* 
 * Timer ID of receiving timer.
 * We use polling mode to receive link level 
 * frame in some case,such as the NIC could
 * not support interrupt.
 * So a timer is set to wakeup the NIC driver thread
 * periodicly.
 */
#define WIFI_TIMER_ID    0x1024

/* 
 * The polling time period of receiving
 * in polling mode.Unit is million second.
 */
#define WIFI_POLL_TIME   5000

/* 
 * Maximal ethernet interfaces can exist 
 * in system. It's the size of ethernet if array.
 */
#define MAX_ETH_INTERFACE_NUM 8

/* 
 * Maximal ethernet buffer element in receiving 
 * queue(list) of ethernet manager.
 * If the frames in queue exceed this value,
 * incoming frames will be droped and loged.
 */
#define MAX_ETH_RXBUFFLISTSZ  1024

/* 
 * Maximal bridging queue list,
 * ethernet frame cause the bridging queue
 * exceed this value will be droped and recorded.
 */
#define MAX_ETH_BCASTQUEUESZ  128

/*
* Maximal sending queue length of ethernet driver.
* Ethernet frame will be droped when the sending queue 
* size exceed this value.
*/
#define MAX_ETH_SENDINGQUEUESZ 8192

/* 
 * Ethernet Manager object.
 * It's the core object of HelloX's ethernet 
 * framework,ethernet functions are implemented
 * mainly in this object.
 */
struct __ETHERNET_MANAGER{
	/* All ethernet interfaces in system. */
	__ETHERNET_INTERFACE EthInterfaces[MAX_ETH_INTERFACE_NUM];
	/* Index of free slot in above array. */
	int nIntIndex;
	/* Ethernet core thread object. */
	__KERNEL_THREAD_OBJECT* EthernetCoreThread;
	/* Set to TRUE if successfully initialized. */
	BOOL bInitialized;

	/* 
	 * Receiving buffer list,include it's first, 
	 * last, number. 
	 * All incoming ethernet frames are linked 
	 * in this list and to be processed by ethernet
	 * core thread.
	 */
	__ETHERNET_BUFFER* pBufferFirst;
	__ETHERNET_BUFFER* pBufferLast;
	volatile size_t nBuffListSize;

	/* Lock of ethernet manager. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/*
	 * Ethernet frame list to broadcast out.
	 */
	__ETHERNET_BUFFER* pBroadcastFirst;
	__ETHERNET_BUFFER* pBroadcastLast;

	/*
	 * Broadcast list size,use int instead size_t since
	 * it's value maybe negative in case of bug,size_t
	 * can not reflect this.
	 */
	volatile int nBroadcastSize;
	volatile int nDropedBcastSize;

	/*
	 * Tracking the ethernet buffer object's counter,
	 * and record all ethernet buffer(s) in system.
	 */
	__atomic_t nTotalEthernetBuffs;
	/*
	 * Record the total sending queue size of all kind of
	 * ethernet drivers.
	 * The drivers can increment this value when a frame is 
	 * enqueue,and decrease it when a frame was sent out.
	 * This value is just for internal debugging.
	 */
	__atomic_t nDrvSendingQueueSz;

	/*
	 * Initializer of Ethernet Manager,should be 
	 * called in process of system initialize.
	 */
	BOOL (*Initialize)(struct __ETHERNET_MANAGER*);

	/*
	 * Operations can be called by layer 3 protocol,such 
	 * as IP, IPv6, or ethernet NIC drivers.
	 */
	__ETHERNET_INTERFACE* (*AddEthernetInterface)(char* ethName,
		char* mac,
		LPVOID pIntExtension,
		__ETHOPS_INITIALIZE Init,
		__ETHOPS_SEND_FRAME SendFrame,
		__ETHOPS_RECV_FRAME RecvFrame,
		__ETHOPS_INT_CONTROL IntCtrl);
	__ETHERNET_INTERFACE* (*GetEthernetInterface)(char* name);
	VOID (*ReleaseEthernetInterface)(__ETHERNET_INTERFACE* pEthInt);
	BOOL (*DeleteEthernetInterface)(__ETHERNET_INTERFACE*);
	BOOL (*ConfigInterface)(char* ethName,__ETH_IP_CONFIG* pConfig);
	BOOL (*Rescan)(char* ethName);
	BOOL (*Assoc)(char* ethName, __WIFI_ASSOC_INFO* pInfo);
	BOOL (*Delivery)(__ETHERNET_INTERFACE* pIf, __ETHERNET_BUFFER* p);
	BOOL (*SendFrame)(__ETHERNET_INTERFACE*, __ETHERNET_BUFFER*);
	BOOL (*BroadcastEthernetFrame)(__ETHERNET_BUFFER* pEthBuffer);
	/* Trigger a receiving poll. */
	BOOL (*TriggerReceive)(__ETHERNET_INTERFACE*);
	/* Post ethernet frame to thread. */
	BOOL (*PostFrame)(__ETHERNET_INTERFACE*, __ETHERNET_BUFFER*);
	VOID (*ShowInt)(char* ethName);
	BOOL (*ShutdownInterface)(char* ethName);
	BOOL (*UnshutInterface)(char* ethName);
	BOOL (*GetEthernetInterfaceState)(__ETH_INTERFACE_STATE* pState, int nIndex, int* pnNextInt);
	
	/* Operations to manipulate ethernet buffer object. */
	__ETHERNET_BUFFER* (*CreateEthernetBuffer)(int buffer_length,
		int alignment);
	__ETHERNET_BUFFER* (*CloneEthernetBuffer)(__ETHERNET_BUFFER* pEthBuff);
	VOID (*DestroyEthernetBuffer)(__ETHERNET_BUFFER* pEthBuff);
	BOOL (*pbuf_to_ebuf)(struct pbuf* pbuf, __ETHERNET_BUFFER* pEthBuff);

	/* Callback of link status change event. */
	VOID (*LinkStatusChange)(__ETHERNET_INTERFACE* pEthInt, 
		enum __LINK_STATUS link_status,
		enum __DUPLEX duplex, 
		enum __ETHERNET_SPEED speed);
};

/* Global ethernet manager objects. */
extern struct __ETHERNET_MANAGER EthernetManager;

/* Ethernet II frame's header. */
typedef struct tag__ETHERNET_II_HEADER {
	char mac_dst[ETH_MAC_LEN];
	char mac_src[ETH_MAC_LEN];
	uint16_t frame_type;
	char payload[0];
}__ETHERNET_II_HEADER;

/* 
 * General input process of ethernet,this routine 
 * could be set as the default genif_input if device
 * driver does not specify a new one.
 */
int general_ethernet_input(__GENERIC_NETIF* pGenif, struct pbuf* p);

/*
 * General output process of ethernet,this routine
 * could be set as the default genif_output if device
 * driver does not specify a new one.
 */
int general_ethernet_output(__GENERIC_NETIF* pGenif, struct pbuf* p);

/* 
 * Entry point array of ethernet driver,
 * each builtin ethernet driver should provide an
 * entry in this array,to initialize itself.
 */
typedef struct{
	BOOL(*EthEntryPoint)(LPVOID);
	LPVOID pData;
}__ETHERNET_DRIVER_ENTRY;
extern __ETHERNET_DRIVER_ENTRY EthernetDriverEntry[];

#endif  //__ETHMGR_H__
