//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 30,2018
//    Module Name               : dpimgr.h
//    Module Funciton           : 
//                                Deep Packet Inspection function of HelloX,encapsulated
//                                as DPI Manager and defined in this file.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __DPIMGR_H__
#define __DPIMGR_H__

#include <config.h>
#include <netcfg.h>

/* Packet directions from the interface's perspective. */
typedef enum tag__DPI_DIRECTION {
	dpi_in = 0,
	dpi_out = 1,
}__DPI_DIRECTION;

/* 
 * DPI engine element. 
 * The dpiFilter routine will be invoked by DPI manager,given
 * the packet and interface.
 * dpiAction routine will be called if dpiFilter returns TRUE.
 */
typedef struct tag__DPI_ENGINE {
	BOOL(*dpiFilter)(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);
	/* 
	 * If the packet is droped or eaten by 
	 * the action routine,then NULL will be 
	 * returned. 
	 * A pbuf object(maybe the same pbuf of p)
	 * will be returned if the packet is modified
	 * or unchanged.
	 */
	struct pbuf* (*dpiAction)(struct pbuf* p, struct netif* _interface, __DPI_DIRECTION dir);
}__DPI_ENGINE;

/* An array contains all implemented DPI engines. */
extern __DPI_ENGINE DPIEngineArray[];

/* DPI Manager. */
typedef struct tag__DPI_MANAGER {
	/* Initializer of this object. */
	BOOL (*Initialize)(struct tag__DPI_MANAGER* pMgr);
	/* Enable or disable DPI on a specified interface. */
	BOOL (*dpiEnable)(char* if_name, BOOL bEnable);
	/* Will be invoked when pakcet into an interface. */
	struct pbuf* (*dpiPacketIn)(struct pbuf* p, struct netif* in_if);
	/* Output direction of DPI processing. */
	struct pbuf* (*dpiPacketOut)(struct pbuf* p, struct netif* out_if);
	/* Send out raw IP packet. */
	BOOL (*dpiSendRaw)(ip_addr_t* src_addr, ip_addr_t* dst_addr,
		uint8_t protocol, uint8_t ttl, uint8_t tos,
		uint16_t hdr_id, char* pBuffer, int buff_length);
	/* Get data from pbuf. */
	char* (*copy_from_pbuf)(struct pbuf* p);
}__DPI_MANAGER;

/* Global DPI Manager object. */
extern __DPI_MANAGER DPIManager;

#endif //__DPIMGR_H__
