//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Dec 7,2014
//    Module Name               : rx.c
//    Module Funciton           : 
//                                This module countains receive functions of
//                                WiFi module.
//                                It's part of Marvell 8686 Linux driver source
//                                code and modified by Garry,which complies GPL
//                                license.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __STDAFX_H__
#include "StdAfx.h"
#endif

#ifndef __KAPI_H__
#include "kapi.h"
#endif

#include "rxtx.h"
#include "marvell_ops.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"

#include "ethernet/ethif.h"

#pragma pack(1)
struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	__be16		h_proto;		/* packet type ID field	*/
} __attribute__((packed));

struct eth803hdr {
	u8 dest_addr[6];
	u8 src_addr[6];
	u16 h803_len;
} __attribute__((packed));

struct rfc1042hdr {
	u8 llc_dsap;
	u8 llc_ssap;
	u8 llc_ctrl;
	u8 snap_oui[3];
	u16 snap_type;
} __attribute__((packed));

struct rxpackethdr {
	struct eth803hdr eth803_hdr;
	struct rfc1042hdr rfc1042_hdr;
} __attribute__((packed));

struct rx80211packethdr {
	struct rxpd rx_pd;
	void *eth80211_hdr;
} __attribute__((packed));

#pragma pack()

//A local refered function to delivery a received packet to
//HelloX's kernel.
//  @pBuff is the packet buffer's start address,and 
//  @len is the total length of packet buffer.
//  @pIf is the original interface where the packet is received.
/*static void DeliveryPacket(const char* pBuff,int len,struct netif* pIf)
{
	struct pbuf* p           = NULL;
	struct pbuf* q           = NULL;
	int    l                 = 0;
	__IF_PBUF_ASSOC* pAssoc  = NULL;
	__KERNEL_THREAD_MESSAGE  msg;
	BOOL   bResult           = FALSE;
	
	if((NULL == pBuff) || (0 == len))
	{
		return;
	}
	
	//Create the association object to associate netif and pbuf together.
	//This object will be released by HelloX's ethernet kernel thread.
	pAssoc = (__IF_PBUF_ASSOC*)KMemAlloc(sizeof(__IF_PBUF_ASSOC),KMEM_SIZE_TYPE_ANY);
	if(NULL == pAssoc)
	{
		goto __TERMINAL;
	}
	
	//Create pbuf object.
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if(NULL == p)
	{
		goto __TERMINAL;
	}
	
	//Copy the packet buffer to pbuf.
	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t*)q->payload, (u8_t*)&pBuff[l], q->len);
		l = l + q->len;
	}
	
	//Associate the interface and pbuf together.
	pAssoc->p       = p;
	pAssoc->pnetif  = pIf;
	
	//Delivery a message to HelloX's ethernet kernel thread,this
	//will lead the packet's processing in kernel.
	msg.dwParam    = (DWORD)pAssoc;
	msg.wParam     = 0;
	msg.wCommand   = ETH_MSG_DELIVER;
	SendMessage((HANDLE)g_pWiFiDriverThread,&msg);
	
	bResult = TRUE;
	
__TERMINAL:
	if(!bResult)
	{
		if(pAssoc)
		{
			KMemFree(pAssoc,KMEM_SIZE_TYPE_ANY,0);
		}
		if(p)
		{
			pbuf_free(p);
		}
	}
	return;
}*/

/**
 *  @brief This function processes received packet and forwards it
 *  to kernel/upper layer
 *
 *  @param priv    A pointer to struct lbs_private
 *  @param skb     A pointer to skb which includes the received packet
 *  @return 	   0 or -1
 */
int lbs_process_rxed_packet(struct lbs_private *priv, char *buffer,u16 size)
{
	int ret = 0;
	struct rxpackethdr *p_rx_pkt;
	struct rxpd *p_rx_pd;
	int hdrchop;
	struct ethhdr *p_ethhdr;
	const u8 rfc1042_eth_hdr[]   = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	struct eth_packet *rx_ethpkt = &priv->rx_pkt;
	lbs_deb_enter(LBS_DEB_RX);
	p_rx_pd  = (struct rxpd *)buffer;
	p_rx_pkt = (struct rxpackethdr *) ((u8 *)p_rx_pd +
		le32_to_cpu(p_rx_pd->pkt_ptr));
		
	if (size < (ETH_HLEN + 8 + sizeof(struct rxpd))) {
		lbs_deb_rx("rx err: frame received with bad length\n");
		ret = 0;
		goto done;
	}
	
#ifdef MASK_DEBUG
	lbs_deb_rx("rx data: skb->len - pkt_ptr = %d-%zd = %zd\n",
		size, (size_t)le32_to_cpu(p_rx_pd->pkt_ptr),
		size - (size_t)le32_to_cpu(p_rx_pd->pkt_ptr));

	lbs_deb_hex(LBS_DEB_RX, "RX Data: Dest", p_rx_pkt->eth803_hdr.dest_addr,
		sizeof(p_rx_pkt->eth803_hdr.dest_addr));
	lbs_deb_hex(LBS_DEB_RX, "RX Data: Src", p_rx_pkt->eth803_hdr.src_addr,
		sizeof(p_rx_pkt->eth803_hdr.src_addr));
#endif
	if (memcmp(&p_rx_pkt->rfc1042_hdr,
		   rfc1042_eth_hdr, sizeof(rfc1042_eth_hdr)) == 0)
	{
		/*
		 *  Replace the 803 header and rfc1042 header (llc/snap) with an
		 *    EthernetII header, keep the src/dst and snap_type (ethertype)
		 *
		 *  The firmware only passes up SNAP frames converting
		 *    all RX Data from 802.11 to 802.2/LLC/SNAP frames.
		 *
		 *  To create the Ethernet II, just move the src, dst address right
		 *    before the snap_type.
		 */
		p_ethhdr = (struct ethhdr *)
		    ((u8 *) & p_rx_pkt->eth803_hdr
		     + sizeof(p_rx_pkt->eth803_hdr) + sizeof(p_rx_pkt->rfc1042_hdr)
		     - sizeof(p_rx_pkt->eth803_hdr.dest_addr)
		     - sizeof(p_rx_pkt->eth803_hdr.src_addr)
		     - sizeof(p_rx_pkt->rfc1042_hdr.snap_type));

		memcpy(p_ethhdr->h_source, p_rx_pkt->eth803_hdr.src_addr,
		       sizeof(p_ethhdr->h_source));
		memcpy(p_ethhdr->h_dest, p_rx_pkt->eth803_hdr.dest_addr,
		       sizeof(p_ethhdr->h_dest));

		/* Chop off the rxpd + the excess memory from the 802.2/llc/snap header
		 *   that was removed
		 */
		hdrchop = (u8 *)p_ethhdr - (u8 *)p_rx_pd;
	}
	else{
		lbs_deb_hex(LBS_DEB_RX, "RX Data: LLC/SNAP",
			(u8 *) & p_rx_pkt->rfc1042_hdr,
			sizeof(p_rx_pkt->rfc1042_hdr));

		/* Chop off the rxpd */
		hdrchop = (u8 *)&p_rx_pkt->eth803_hdr - (u8 *)p_rx_pd;
	}

	/* Chop off the leading header bytes so the skb points to the start of
	 *   either the reconstructed EthII frame or the 802.2/llc/snap frame
	 */
	//skb_pull(skb, hdrchop);

	rx_ethpkt->len  = size - hdrchop;
	rx_ethpkt->data = (char *)((char *)buffer+hdrchop);
	//Delivery the data into HelloX's kernel.
	//DeliveryPacket(rx_ethpkt->data,rx_ethpkt->len,NULL);

#if 0
	if (priv->enablehwauto)
	{
		priv->cur_rate = lbs_fw_index_to_data_rate(p_rx_pd->rx_rate);
	}
	lbs_compute_rssi(priv, p_rx_pd);
#endif
	ret = 0;
done:
	lbs_deb_leave_args(LBS_DEB_RX, ret);
	return ret;
}

int  wait_for_data_end(void)
{	
	struct lbs_private *priv=pgmarvel_priv;
	struct if_sdio_card *card=priv->card;
	u8 cause;
	int ret;
	while(1){
		cause = sdio_readb(card->func, IF_SDIO_H_INT_STATUS, &ret);
		//读取中断状态，这个是网卡内部的中断状态寄存器,和sdio控制器的中断状态寄存器没有关系
		if (ret){
			printk("marvel interrupt error!\n");
			return ret;
		}
		if (cause & IF_SDIO_H_INT_DNLD){//卡响应命令产生的中断，表明卡正常接收到命令
			sdio_writeb(card->func, ~IF_SDIO_H_INT_DNLD,IF_SDIO_H_INT_STATUS, &ret);//请中断挂起标志位
			if (ret)
				return ret;
			break;
		}
	}
        return 0;
}

