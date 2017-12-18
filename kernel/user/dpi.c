/*
 * Deep Packet Inspection code.
 */

#include <StdAfx.h>
#include <hx_eth.h>
#include <hx_inet.h>
#include <ethmgr.h>
#include <lwip/ip.h>

#include <stdlib.h>
#include <stdio.h>

/*
 * Entry point of DPI.
 */
__ETHERNET_BUFFER* Do_DPI(__ETHERNET_BUFFER* pBuffer)
{
	__ETHERNET_BUFFER* pNewBuffer = NULL;
	struct ip_hdr* ip_header = NULL;
	
	/*
	 * Clone a new buffer from the old one,do modifications on the new one
	 * and return it,the underlay code will through the new ethernet frame
	 * out.
	 */
	pNewBuffer = EthernetManager.CloneEthernetBuffer(pBuffer);
	if (NULL == pNewBuffer)
	{
		return NULL;
	}

	/*
	 * Just do some packet inspection and modification here.
	 */
	ip_header = (struct ip_hdr*)(pNewBuffer->Buffer + ETH_HEADER_LEN);

	/*
	 * Then return it back to OS kernel,network module will transmit
	 * the revised frame out.
	 */
	return pNewBuffer;
}
