#define	 __MARVELL_OPS_FILE__
#include "marvell_ops.h"



/********************************************************************************
本结构保存系统运行相关的所有参数，如MAC、连接状态、网络类型等信息。灵活应用本结构
可以更好的优化程序设计及其运行效率
********************************************************************************/
struct lbs_private *pgmarvel_priv;
u8 sdio_sys_wait=1;//sdio驱动bug，本变量用于调整系统稳定时间，直接影响系统性能
/***********************************************************************************************
****函数名:init_marvell_driver
****描述:初始化marvell驱动程序，关联配置信息存放于marvel_mode等中
****参数:无
****返回:0:失败 其他:struct lbs_private *
***********************************************************************************************/
struct lbs_private * init_marvell_driver(void)
{
  struct mmc_host *host;
  struct mmc_card *card;
  struct sdio_func *func;
  struct lbs_private *priv;
  MAR_POW(1);//断开模块电源为sdio枚举做准备
  host=stm32_probe();
  MAR_POW(0);//上电模块电源，sdio正常复位
  mmc_rescan(host);
  printk("init marvel driver ok!\n");
  card=host->card;//取出card
  func=card->sdio_func[0];//取出第一个func
  if(func){
		  priv=(struct lbs_private *)sdio_bus_probe(func);
		  if(priv){
			  printk("device probe success!\n");
			  return priv;
		  }
		  else
			  printk("device probe failed!\n");
  }
  else
	  printk("cann't find a function device!\n");
  return 0;
}

/***********************************************************************************************
****函数名:marvel_assoc_network
****描述:关联网络
****参数:priv:全局驱动变量
	    ssid:ascll码网络名称
	    key:ascll码密钥，长度为0:开放性网络，小于8bit的密钥默认为wep加密，否则启用wpa加密
	    mode:'0':基础网络(infra) ‘1’:ad-hoc网络
****返回:0:正常 其他:错误
***********************************************************************************************/
void marvel_assoc_network(struct lbs_private *priv,char *ssid,char *key,char mode,int channel)
{
		marvel_assoc_open_network(priv,ssid,key,mode,channel);
}

/***********************************************************************************************
****函数名:lbs_scan_worker
****描述:扫描网络，扫描结果存放于struct lbs_private结构中的networks域
****参数:priv:全局驱动变量
****返回:无
***********************************************************************************************/
void lbs_scan_worker(struct lbs_private *priv)
{
	lbs_deb_enter(LBS_DEB_SCAN);
	lbs_scan_networks(priv, 0);
	lbs_deb_leave(LBS_DEB_SCAN);
}

/***********************************************************************************************
****函数名:lbs_rev_pkt
****描述:函数完成WIFI链路层的数据接收，接收得到的数据存放于struct lbs_private *priv->rx_pkt中。
****参数:无
****返回:返回接收数据长度，
***********************************************************************************************/
u16 lbs_rev_pkt(void)
{
	struct lbs_private *priv  = pgmarvel_priv;
	struct if_sdio_card *card = priv->card;
	struct eth_packet *rx_pkt = &priv->rx_pkt;
	int ret;
	sdio_sys_wait = 0;
	memset(rx_pkt,0,sizeof(struct eth_packet ));
	ret = poll_sdio_interrupt(card->func);
	if(ret < 0){
			lbs_pr_err("read interrupt error!\n");
			try_bug(0);
	}
	else
		if(ret&(IF_SDIO_H_INT_UPLD|IF_SDIO_H_INT_DNLD)){
		if_sdio_interrupt(card->func);
		if(rx_pkt->len){  //Data available.
			sdio_sys_wait = 1;
			return rx_pkt->len;
		}
	}
	else{
		sdio_sys_wait = 1;
		return 0;
	}
	sdio_sys_wait = 1;
	return 0;
}

/***********************************************************************************************
****函数名:lbs_hard_start_xmit
****描述:发送以太网数据包，数据内容存放于tx_ethpkt中的data域
****参数:priv:全局驱动变量
	    tx_ethpkt:以太网数据包封包
****返回:0:正常 其他:错误
***********************************************************************************************/
char  lbs_hard_start_xmit(struct lbs_private *priv,struct eth_packet * tx_ethpkt)
{
	struct txpd *txpd;//这是用于控制硬件发送的头信息，必须放在数据包前面写入网卡
	char *p802x_hdr;
	unsigned int pkt_len;
	int ret = 0;
	lbs_deb_enter(LBS_DEB_TX);
	if (priv->surpriseremoved)
		goto free;

	if (!tx_ethpkt->len || (tx_ethpkt->len > MRVDRV_ETH_TX_PACKET_BUFFER_SIZE)) {
		lbs_deb_tx("tx err: skb length %d 0 or > %zd\n",
		       tx_ethpkt->len, MRVDRV_ETH_TX_PACKET_BUFFER_SIZE);
		goto free;
	}

	txpd=(void *)&priv->resp_buf[0][4];
	memset(txpd, 0, sizeof(struct txpd));

	p802x_hdr = tx_ethpkt->data;//802.3 mac头
	pkt_len = tx_ethpkt->len;
	/* copy destination address from 802.3 header */
	//接收地址
	memcpy(txpd->tx_dest_addr_high, p802x_hdr, ETH_ALEN);
	txpd->tx_packet_length = cpu_to_le16(pkt_len);//802.3的有效数据长度，固件会自动封装802.11数据帧
	txpd->tx_packet_location = cpu_to_le32(sizeof(struct txpd));//数据的偏移
	memcpy(&txpd[1], p802x_hdr, le16_to_cpu(txpd->tx_packet_length));//真正数据存放的地方
	priv->resp_len[0] = pkt_len + sizeof(struct txpd);//是否有数据需要发送靠的就是判断邋pkt_len是否值
	lbs_deb_tx("%s lined up packet\n", __func__);
 free:	
	if (priv->resp_len[0] > 0) {//发送数据处理//这里就是调用if_sdio_host_to_card这个函数来处理向设备发送数据	
		 ret=if_sdio_send_data(priv,priv->resp_buf[0],
								priv->resp_len[0]);
		if (ret) {
			lbs_deb_tx("host_to_card failed %d\n", ret);
			priv->dnld_sent = DNLD_RES_RECEIVED;
		}
		priv->resp_len[0] = 0;
	}
	wait_for_data_end();
	lbs_deb_leave_args(LBS_DEB_TX, ret);
	return ret;
}

/***********************************************************************************************
****函数名:wpa_L2_send_pkt
****描述:wpa eapol报文链路层发送函数
****返回:0:正常 其他:错误
***********************************************************************************************/
int wpa_L2_send_pkt(u8 *buf,u16 len)
{
	struct eth_packet tx_ethpkt;
	tx_ethpkt.data=(char *)buf;
	tx_ethpkt.len=len;
	//debug_data_stream("L2 send",buf,len);
	return lbs_hard_start_xmit(pgmarvel_priv,&tx_ethpkt);
}
/***********************************************************************************************
****函数名:lbs_ps_wakeup
****描述:用于唤醒网卡sleep模式
****参数:priv:全局驱动变量
****返回:无
***********************************************************************************************/
void lbs_ps_wakeup(struct lbs_private *priv) 
{
	__le32 Localpsmode;
	int wait_option=CMD_OPTION_WAITFORRSP;
	lbs_deb_enter(LBS_DEB_HOST);
	Localpsmode = cpu_to_le32(LBS802_11POWERMODECAM);
	lbs_prepare_and_send_command(priv, CMD_802_11_PS_MODE,//发送命令退出低功耗模式
			      CMD_SUBCMD_EXIT_PS,
			      wait_option, 0, &Localpsmode);

	lbs_deb_leave(LBS_DEB_HOST);
}
/***********************************************************************************************
****函数名:lbs_ps_sleep
****描述:用于基础网络模式下,进入sleep状态，AP将为其缓存报文
****参数:priv:全局驱动变量
****返回:无
***********************************************************************************************/
void lbs_ps_sleep(struct lbs_private *priv)
{
	int wait_option=CMD_OPTION_WAITFORRSP;
	lbs_deb_enter(LBS_DEB_HOST);
	/*
	 * PS is currently supported only in Infrastructure mode
	 * Remove this check if it is to be supported in IBSS mode also
	 */
	lbs_prepare_and_send_command(priv, CMD_802_11_PS_MODE,
			      CMD_SUBCMD_ENTER_PS, wait_option, 0, NULL);

	lbs_deb_leave(LBS_DEB_HOST);
}

/***********************************************************************************************
****函数名:init_sleep_mode
****描述:初始化全局的confirm_sleep变量，以便于lbs_send_confirmsleep迅速进入低功耗模式
****参数:无
****返回:无
***********************************************************************************************/
struct cmd_confirm_sleep confirm_sleep;
void init_sleep_mode(void)
{
	memset(&confirm_sleep, 0, sizeof(confirm_sleep));
	confirm_sleep.hdr.command = cpu_to_le16(CMD_802_11_PS_MODE);
	confirm_sleep.hdr.size = cpu_to_le16(sizeof(confirm_sleep));
	confirm_sleep.action = cpu_to_le16(CMD_SUBCMD_SLEEP_CONFIRMED);
}
 /***********************************************************************************************
 ****函数名:lbs_send_confirmsleep
 ****描述:网卡进入低功耗模式
 ****参数:priv:全局驱动变量
 ****返回:无
 ***********************************************************************************************/
 void lbs_send_confirmsleep(struct lbs_private *priv)
{
	 int ret;

	lbs_deb_enter(LBS_DEB_HOST);
	lbs_deb_hex(LBS_DEB_HOST, "sleep confirm", (u8 *) &confirm_sleep,
		sizeof(confirm_sleep));

	ret = priv->hw_host_to_card(priv, MVMS_CMD, (u8 *) &confirm_sleep,
		sizeof(confirm_sleep));
	if (ret) {
		lbs_pr_alert("confirm_sleep failed\n");
		goto out;
	}
	/* We don't get a response on the sleep-confirmation */
	priv->dnld_sent = DNLD_RES_RECEIVED;
out:
	lbs_deb_leave(LBS_DEB_HOST);
}
