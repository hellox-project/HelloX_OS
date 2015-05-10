#ifndef __RX_TX__H__
#define __RX_TX__H__
#include "type.h"
#include "dev.h"
/*我们TCP/IP关心的内容*/
/*
struct eth_packet{
	u16 len;
	char *data;
};*/
int lbs_process_rxed_packet(struct lbs_private *priv, char *buffer,u16 size);
char  lbs_hard_start_xmit(struct lbs_private *priv,struct eth_packet * tx_ethpkt);
u16 wireless_card_rx(u8 *buf);
void wireless_card_tx(u8 *buf,u16 len);
u16 lbs_rev_pkt(void);
int  wait_for_data_end(void);

#endif



