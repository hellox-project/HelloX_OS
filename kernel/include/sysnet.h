//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 26,2014
//    Module Name               : sysnet.h
//    Module Funciton           : 
//                                HelloX operating system's network abstraction
//                                layer.
//                                It defines several interfaces which should be
//                                implemented by any network protocol,and defines
//                                standard BSD like socket APIs for application.
//    Last modified Author      :
//    Last modified Date        : Jun 26,2014
//    Last modified Content     :
//                                1. 
//    Lines number              :
//***********************************************************************/

#ifndef __SYSNET_H__
#define __SYSNET_H__

//A routine should be implemented by any network protocl to register itself
//into OS kernel,and initialize the stack.
typedef BOOL (*__NETWORK_PROTOCOL_ENTRY)(VOID* pArg);

//An entry routine is present for each enabled network protocol.

#ifdef __CFG_NET_IPv4  //TCP/IP v4 stack is enabled.
BOOL IPv4_Entry(VOID* pArg);
#endif

#endif
