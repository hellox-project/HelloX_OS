//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 29,2014
//    Module Name               : network.h
//    Module Funciton           : 
//    Description               : 
//                                Network diagnostic application,common used
//                                network tools such as ping/tracert,are implemented
//                                in network.c file.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __NETWORK_H__
#define __NETWORK_H__

//Entry point of network diagnostic application.
DWORD networkEntry(LPVOID);

//Transfer ping related parameters to ping's main routine.
typedef struct{
	ip_addr_t   targetAddr;
	short       count;
	short       size;
}__PING_PARAM;

//Entry point of ping command.
void ping_Entry(void *arg);

//Flags for command processing result.
#define NET_CMD_TERMINAL     0x00000001
#define NET_CMD_INVALID      0x00000002
#define NET_CMD_FAILED       0x00000004
#define NET_CMD_SUCCESS      0x00000008

#endif
