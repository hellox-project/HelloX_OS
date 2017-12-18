//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 29,2017
//    Module Name               : netglob.h
//    Module Funciton           : 
//                                Global variables of network module,of
//                                HelloX.Arrange all these global variables
//                                into an object,to make management easy.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __NETGLOB_H__
#define __NETGLOB_H__

/* For ip_addr_t. */
#include "lwip/ip_addr.h"

/* For DOMAIN_NAME_LENGTH. */
#include "netcfg.h"

/* Global object that contains all network related global varialbes. */
typedef struct tag__NETWORK_GLOBAL{
	ip_addr_t dns_primary;
	ip_addr_t dns_secondary;
	
	/* Domain name. */
	char domain_name[DOMAIN_NAME_LENGTH];

	/* Operations. */
	void (*ResetDNSServer)(); /* Reset DNS server to default value. */
	BOOL (*Initialize)(struct tag__NETWORK_GLOBAL* pNetGlob);
}__NETWORK_GLOBAL;

extern __NETWORK_GLOBAL NetworkGlobal;

#endif
