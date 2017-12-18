//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 29,2017
//    Module Name               : netglob.c
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

#include <StdAfx.h>
#include <string.h>

#include "netcfg.h"
#include "netglob.h"

/* Initializer of network global object. */
static BOOL NetGlobInitialize(__NETWORK_GLOBAL* pNetGlob)
{
	pNetGlob->ResetDNSServer();
	strncpy(pNetGlob->domain_name, DEFAULT_DOMAIN_NAME, DOMAIN_NAME_LENGTH);
	return TRUE;
}

/* Reset DNS server to default value. */
static void ResetDNSServer()
{
	NetworkGlobal.dns_primary.addr = 0;
	NetworkGlobal.dns_secondary.addr = 0;
}

/* Network global object. */
__NETWORK_GLOBAL NetworkGlobal = {
	0,                   //dns_primary.
	0,                   //dns_secondary.
	{ 0 },               //domain_name.
	ResetDNSServer,      //ResetDNSServer.
	NetGlobInitialize,   //Intialize.
};
