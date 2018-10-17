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

//Flags to enable or disable ethernet debugging.
//#define __ETH_DEBUG

#ifndef __func__
#define __func__ __FUNCTION__
#endif

//Messages that can be sent to Ethernet core thread.The thread is
//driven by message,and run in background.
//EthernetManager can launch specified functions by sending these messages
//to it.
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

#define MAX_ETH_NAME_LEN     31   //Maximal length of ethernet interface's name.
#define ETH_MAC_LEN          6    //MAC address's length.
#define ETH_DEFAULT_MTU      1500 //Default maximal transmition unit.
#define ETH_HEADER_LEN       24   //Ethernet frame header's length,assume 24 bytes to accomadate more data.
#define ETH_MAX_FRAME_LEN    1600 //Maximal Ethernet frame's length.

/* Predefine ethernet interface object. */
struct tag__ETHERNET_INTERFACE;

//Ethernet buffer object,each ethernet frame corresponding one of this object.
typedef struct tag__ETHERNET_BUFFER{
	struct tag__ETHERNET_BUFFER* pNext;  //Pointing to next one if has.
	DWORD      dwSignature;              //Object Indentifier.
	__u8       srcMAC[ETH_MAC_LEN];      //Source MAC address.
	__u8       dstMAC[ETH_MAC_LEN];      //Destination MAC address.
	__u16      frame_type;
	__u8       Buffer[ETH_DEFAULT_MTU + ETH_HEADER_LEN];  //Actual ethernet frame data.
	__u16      buff_length;              //Length of frame data,current is not used.
	__u16      act_length;               //Actual buffer length.
	__u16      buff_status;              //Status of the Ethernet Buffer.
#define ETHERNET_BUFFER_STATUS_FREE        0x00  //Free to use.
#define ETHERNET_BUFFER_STATUS_INITIALIZED 0x01  //Buffer is initialized.
#define ETHERNET_BUFFER_STATUS_PENDING     0x02  //Pending in queue.
	//LPVOID     pEthernetInterface;       //Ethernet Interface this buffer associated to.

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

//Common network address,used to contain any type of network address.
typedef struct tag__COMMON_NETWORK_ADDRESS{
	UCHAR AddressType;
#define NETWORK_ADDRESS_TYPE_ETHMAC    0x01
#define NETWORK_ADDRESS_TYPE_IPV4      0x02
#define NETWORK_ADDRESS_TYPE_IPV6      0x04
#define NETWORK_ADDRESS_TYPE_MASKLEN   0x08
	union{
		__u32 ipv4_addr;
		__u8  eth_mac[6];
		__u8  ipv6_addr[16];
		int   mask_length;
	}Address;
}__COMMON_NETWORK_ADDRESS;

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

//Ethernet interface object,represents an dedicated ethernet interface.
typedef struct tag__ETHERNET_INTERFACE{
	char                    ethName[MAX_ETH_NAME_LEN + 1];
	char                    ethMac[ETH_MAC_LEN];       //MAC address of the interface.
	__ETHERNET_BUFFER       SendBuffer;                //First element of sending buffer.
	__ETH_INTERFACE_STATE   ifState;                   //Interface state info.
	struct __PROTO_INTERFACE_BIND Proto_Interface[MAX_BIND_PROTOCOL_NUM];
	LPVOID                  pIntExtension;             //Private information.
	/*
	 * Sending queue(list) size of the current interface.
	 */
	volatile size_t         nSendingQueueSz;

	//Operations open to HelloX's ethernet framework.
	BOOL                    (*SendFrame)(struct tag__ETHERNET_INTERFACE*); //Sending operation.
	__ETHERNET_BUFFER*      (*RecvFrame)(struct tag__ETHERNET_INTERFACE*); //Receive operation.
	BOOL                    (*IntControl)(struct tag__ETHERNET_INTERFACE*, DWORD dwOperations, LPVOID);  //Other operations.
}__ETHERNET_INTERFACE;

//Operation protypes for convinence.
typedef BOOL                (*__ETHOPS_SEND_FRAME)(__ETHERNET_INTERFACE*);
typedef __ETHERNET_BUFFER*  (*__ETHOPS_RECV_FRAME)(__ETHERNET_INTERFACE*);
typedef BOOL                (*__ETHOPS_INT_CONTROL)(__ETHERNET_INTERFACE*, DWORD, LPVOID);
typedef BOOL                (*__ETHOPS_INITIALIZE)(__ETHERNET_INTERFACE*);

//Default name of Ethernet core thread.
#define ETH_THREAD_NAME  "netCore"

//Timer ID of receiving timer.We use polling mode to receive link level frame
//in current version,so a timer is used to wakeup the WiFi driver thread
//periodicly.
#define WIFI_TIMER_ID    0x1024

//The polling time period of receiving,in million-second.
#define WIFI_POLL_TIME   5000

//Maximal ethernet interfaces can exist in system.
#define MAX_ETH_INTERFACE_NUM 4

//Maximal ethernet buffer element in receiving list,i.e,the queue size.
#define MAX_ETH_RXBUFFLISTSZ  256

//Maximal bridging queue list,ethernet frame cause the bridging queue
//exceed this value will be droped and recorded.
#define MAX_ETH_BCASTQUEUESZ  128

/*
* Maximal sending queue length of ethernet driver.
* Ethernet frame will be droped when the sending queue size exceed
* this value.
*/
#define MAX_ETH_SENDINGQUEUESZ 256

//Ethernet Manager object,the core object of HelloX's ethernet framework.
struct __ETHERNET_MANAGER{
	__ETHERNET_INTERFACE    EthInterfaces[MAX_ETH_INTERFACE_NUM];
	int                     nIntIndex;          //Index of free slot in above array.
	__KERNEL_THREAD_OBJECT* EthernetCoreThread;
	BOOL                    bInitialized;       //Set to TRUE if successfully initialized.
	__ETHERNET_BUFFER*      pBufferFirst;       //Received buffer list header.
	__ETHERNET_BUFFER*      pBufferLast;        //Received buffer list tail.
	volatile size_t         nBuffListSize;      //How many buffers in queue.
	/* Lock of ethernet manager. */
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif

	/*
	 * Ethernet frame list to broadcast out.
	 */
	__ETHERNET_BUFFER*      pBroadcastFirst;
	__ETHERNET_BUFFER*      pBroadcastLast;
	/*
	 * Broadcast list size,use int instead size_t since
	 * it's value maybe negative in case of bug,size_t
	 * can not reflect this.
	 */
	volatile int            nBroadcastSize;
	volatile int            nDropedBcastSize;

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

	//Initializer of Ethernet Manager,should be called in process of system initialize.
	BOOL(*Initialize)       (struct __ETHERNET_MANAGER*);

	//Operations can be called by layer 3 or ethernet drivers.
	__ETHERNET_INTERFACE*   (*AddEthernetInterface)(char* ethName,
		char* mac,
		LPVOID pIntExtension,
		__ETHOPS_INITIALIZE Init,
		__ETHOPS_SEND_FRAME SendFrame,
		__ETHOPS_RECV_FRAME RecvFrame,
		__ETHOPS_INT_CONTROL IntCtrl);
	__ETHERNET_INTERFACE*   (*GetEthernetInterface)(char* name);
	VOID                    (*ReleaseEthernetInterface)(__ETHERNET_INTERFACE* pEthInt);
	BOOL                    (*DeleteEthernetInterface)(__ETHERNET_INTERFACE*);
	BOOL                    (*ConfigInterface)(char* ethName,__ETH_IP_CONFIG* pConfig);
	BOOL                    (*Rescan)(char* ethName);
	BOOL                    (*Assoc)(char* ethName, __WIFI_ASSOC_INFO* pInfo);
	BOOL                    (*Delivery)(__ETHERNET_INTERFACE* pIf, __ETHERNET_BUFFER* p);  //Called by ethernet device driver.
	BOOL                    (*SendFrame)(__ETHERNET_INTERFACE*, __ETHERNET_BUFFER*);  //Called by layer 3 entities.
	BOOL                    (*BroadcastEthernetFrame)(__ETHERNET_BUFFER* pEthBuffer);
	BOOL                    (*TriggerReceive)(__ETHERNET_INTERFACE*); //Trigger a receiving poll.
	BOOL                    (*PostFrame)(__ETHERNET_INTERFACE*, __ETHERNET_BUFFER*);  //Post ethernet frame.
	VOID                    (*ShowInt)(char* ethName);
	BOOL                    (*ShutdownInterface)(char* ethName);
	BOOL                    (*UnshutInterface)(char* ethName);
	BOOL                    (*GetEthernetInterfaceState)(__ETH_INTERFACE_STATE* pState, int nIndex, int* pnNextInt);
	__ETHERNET_BUFFER*      (*CreateEthernetBuffer)(int buffer_length);
	__ETHERNET_BUFFER*      (*CloneEthernetBuffer)(__ETHERNET_BUFFER* pEthBuff);
	VOID                    (*DestroyEthernetBuffer)(__ETHERNET_BUFFER* pEthBuff);
};

//Global ethernet manager objects.
extern struct __ETHERNET_MANAGER EthernetManager;

//Entry point array of ethernet driver,each ethernet driver should provide an
//entry in this array,to initialize itself.
typedef struct{
	BOOL(*EthEntryPoint)(LPVOID);
	LPVOID pData;
}__ETHERNET_DRIVER_ENTRY;
extern __ETHERNET_DRIVER_ENTRY EthernetDriverEntry[];

#endif  //__ETHMGR_H__
