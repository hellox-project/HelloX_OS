//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep,07 2014
//    Module Name               : ethif.h
//    Module Funciton           : 
//                                This file countains definitions of Ethernet
//                                interface.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __ETHIF_H__
#define __ETHIF_H__

//Flags to enable or disable ethernet debugging.
//#define __ETH_DEBUG

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

//Maximal length of ethernet interface.
#define MAX_ETH_NAME_LEN     31
#define ETH_MAC_LEN          6
#define ETH_DEFAULT_MTU      1500 //Default maximal transmition unit.

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
	
	//Statistics information from ethernet level.
	DWORD              dwFrameSend;         //How many frames has been sent since boot up.
	DWORD              dwFrameSendSuccess;  //The number of frames sent out successfully.
	DWORD              dwFrameRecv;         //How many frames has been received since boot up.
	DWORD              dwFrameRecvSuccess;  //Delivery pkt to upper layer successful.
	DWORD              dwTotalSendSize;     //How many bytes has been sent since boot.
	DWORD              dwTotalRecvSize;     //Receive size.
}__ETH_INTERFACE_STATE;

//Ethernet interface object,represents an dedicated ethernet interface.
typedef struct tag__ETHERNET_INTERFACE{
	char                    ethName[MAX_ETH_NAME_LEN + 1];
	char                    ethMac[ETH_MAC_LEN];       //MAC address of the interface.
	char                    SendBuff[ETH_DEFAULT_MTU]; //Sending buffer.
	int                     buffSize;                  //How many valid bytes in sending buffer.
	__ETH_INTERFACE_STATE   ifState;       //Interface state info.
	LPVOID                  pL3Interface;  //Layer3 interface associated to this ethernet interface.
	LPVOID                  pIntExtension; //Private information.
	
	//Operations open to HelloX's ethernet framework.
	BOOL                    (*SendFrame)(struct tag__ETHERNET_INTERFACE*);  //Sending operation.
	struct pbuf*            (*RecvFrame)(struct tag__ETHERNET_INTERFACE*);                 //Receive operation.
	BOOL                    (*IntControl)(struct tag__ETHERNET_INTERFACE*,DWORD dwOperations,LPVOID);  //Other operations.
}__ETHERNET_INTERFACE;

//Association of pbuf and netif,which is used by ethernet kernel thread
//to determine where the packet(pbuf) is received(netif).
typedef struct tag__IF_PBUF_ASSOC{
	struct pbuf*  p;
	struct netif* pnetif;
}__IF_PBUF_ASSOC;

//Operation protypes for convinence.
typedef BOOL              (*__ETHOPS_SEND_FRAME)(__ETHERNET_INTERFACE*);
typedef struct pbuf*      (*__ETHOPS_RECV_FRAME)(__ETHERNET_INTERFACE*);
typedef BOOL              (*__ETHOPS_INT_CONTROL)(__ETHERNET_INTERFACE*,DWORD,LPVOID);
typedef BOOL              (*__ETHOPS_INITIALIZE)(__ETHERNET_INTERFACE*);

//Default name of Ethernet core thread.
#define ETH_THREAD_NAME  "eth_thread"

//Timer ID of receiving timer.We use polling mode to receive link level frame
//in current version,so a timer is used to wakeup the WiFi driver thread
//periodicly.
#define WIFI_TIMER_ID    0x1024

//The polling time period of receiving,in million-second.
#define WIFI_POLL_TIME   200

//Maximal ethernet interfaces can exist in system.
#define MAX_ETH_INTERFACE_NUM 1

//Ethernet Manager object,the core object of HelloX's ethernet framework.
struct __ETHERNET_MANAGER{
	__ETHERNET_INTERFACE    EthInterfaces[MAX_ETH_INTERFACE_NUM];
	int                     nIntIndex;          //Index of free slot in above array.
	__KERNEL_THREAD_OBJECT* EthernetCoreThread;
	BOOL                    bInitialized;       //Set to TRUE if successfully initialized.
	
	//Initializer of Ethernet Manager,should be called in process of system initialize.
	BOOL                    (*Initialize)(struct __ETHERNET_MANAGER*);
	
	//Operations can be called by layer 3 or ethernet drivers.
	__ETHERNET_INTERFACE*   (*AddEthernetInterface)(char* ethName,
	                                                char* mac,
	                                                LPVOID pIntExtension,
	                                                __ETHOPS_INITIALIZE Init,
	                                                __ETHOPS_SEND_FRAME SendFrame,
	                                                __ETHOPS_RECV_FRAME RecvFrame,
	                                                __ETHOPS_INT_CONTROL IntCtrl);
	BOOL                    (*ConfigInterface)(char* ethName,
	                                           __ETH_IP_CONFIG* pConfig);
	BOOL                    (*Rescan)(char* ethName);
	BOOL                    (*Assoc)(char* ethName,__WIFI_ASSOC_INFO* pInfo);
	BOOL                    (*Delivery)(__ETHERNET_INTERFACE* pIf,struct pbuf* p);  //Called by ethernet device driver.
	BOOL                    (*SendFrame)(__ETHERNET_INTERFACE*,struct pbuf*);  //Called by layer 3 entities.
	VOID                    (*ShowInt)(char* ethName);
	BOOL                    (*ShutdownInterface)(char* ethName);
	BOOL                    (*UnshutInterface)(char* ethName);
};

//Global ethernet manager objects.
extern struct __ETHERNET_MANAGER EthernetManager;

//Entry point array of ethernet driver,each ethernet driver should provide an
//entry in this array,to initialize itself.
typedef struct{
	BOOL  (*EthEntryPoint)(LPVOID);
	LPVOID pData;
}__ETHERNET_DRIVER_ENTRY;
extern __ETHERNET_DRIVER_ENTRY EthernetDriverEntry[];

#endif  //__ETHIF_H__
