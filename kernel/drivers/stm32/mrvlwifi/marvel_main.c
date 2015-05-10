
#include "lwip/opt.h"
#include "type.h"
#include "common.h"
#include "dev.h"
#include "mdef.h"
#include "mac80211.h"
#include "if.h"
#include "marvel_main.h"
#include "hostcmdcode.h"
#include "cmd.h"
#include "11d.h"
#include "types.h"

#define LBS_TX_PWR_DEFAULT      20	/*100mW */
#define LBS_TX_PWR_US_DEFAULT   20	/*100mW */
#define LBS_TX_PWR_JP_DEFAULT   16	/*50mW */
#define LBS_TX_PWR_FR_DEFAULT   20	/*100mW */
#define LBS_TX_PWR_EMEA_DEFAULT	20	/*100mW */

/* Format { channel, frequency (MHz), maxtxpower } */
/* band: 'B/G', region: USA FCC/Canada IC */
static struct chan_freq_power channel_freq_power_US_BG[] = {
	{1, 2412, LBS_TX_PWR_US_DEFAULT},
	{2, 2417, LBS_TX_PWR_US_DEFAULT},
	{3, 2422, LBS_TX_PWR_US_DEFAULT},
	{4, 2427, LBS_TX_PWR_US_DEFAULT},
	{5, 2432, LBS_TX_PWR_US_DEFAULT},
	{6, 2437, LBS_TX_PWR_US_DEFAULT},
	{7, 2442, LBS_TX_PWR_US_DEFAULT},
	{8, 2447, LBS_TX_PWR_US_DEFAULT},
	{9, 2452, LBS_TX_PWR_US_DEFAULT},
	{10, 2457, LBS_TX_PWR_US_DEFAULT},
	{11, 2462, LBS_TX_PWR_US_DEFAULT}
};

/* band: 'B/G', region: Europe ETSI */
static struct chan_freq_power channel_freq_power_EU_BG[] = {
	{1, 2412, LBS_TX_PWR_EMEA_DEFAULT},
	{2, 2417, LBS_TX_PWR_EMEA_DEFAULT},
	{3, 2422, LBS_TX_PWR_EMEA_DEFAULT},
	{4, 2427, LBS_TX_PWR_EMEA_DEFAULT},
	{5, 2432, LBS_TX_PWR_EMEA_DEFAULT},
	{6, 2437, LBS_TX_PWR_EMEA_DEFAULT},
	{7, 2442, LBS_TX_PWR_EMEA_DEFAULT},
	{8, 2447, LBS_TX_PWR_EMEA_DEFAULT},
	{9, 2452, LBS_TX_PWR_EMEA_DEFAULT},
	{10, 2457, LBS_TX_PWR_EMEA_DEFAULT},
	{11, 2462, LBS_TX_PWR_EMEA_DEFAULT},
	{12, 2467, LBS_TX_PWR_EMEA_DEFAULT},
	{13, 2472, LBS_TX_PWR_EMEA_DEFAULT}
};

/* band: 'B/G', region: Spain */
static struct chan_freq_power channel_freq_power_SPN_BG[] = {
	{10, 2457, LBS_TX_PWR_DEFAULT},
	{11, 2462, LBS_TX_PWR_DEFAULT}
};

/* band: 'B/G', region: France */
static struct chan_freq_power channel_freq_power_FR_BG[] = {
	{10, 2457, LBS_TX_PWR_FR_DEFAULT},
	{11, 2462, LBS_TX_PWR_FR_DEFAULT},
	{12, 2467, LBS_TX_PWR_FR_DEFAULT},
	{13, 2472, LBS_TX_PWR_FR_DEFAULT}
};

/* band: 'B/G', region: Japan */
static struct chan_freq_power channel_freq_power_JPN_BG[] = {
	{1, 2412, LBS_TX_PWR_JP_DEFAULT},
	{2, 2417, LBS_TX_PWR_JP_DEFAULT},
	{3, 2422, LBS_TX_PWR_JP_DEFAULT},
	{4, 2427, LBS_TX_PWR_JP_DEFAULT},
	{5, 2432, LBS_TX_PWR_JP_DEFAULT},
	{6, 2437, LBS_TX_PWR_JP_DEFAULT},
	{7, 2442, LBS_TX_PWR_JP_DEFAULT},
	{8, 2447, LBS_TX_PWR_JP_DEFAULT},
	{9, 2452, LBS_TX_PWR_JP_DEFAULT},
	{10, 2457, LBS_TX_PWR_JP_DEFAULT},
	{11, 2462, LBS_TX_PWR_JP_DEFAULT},
	{12, 2467, LBS_TX_PWR_JP_DEFAULT},
	{13, 2472, LBS_TX_PWR_JP_DEFAULT},
	{14, 2484, LBS_TX_PWR_JP_DEFAULT}
};

/**
 * the structure for channel, frequency and power
 */
struct region_cfp_table {
	u8 region;
	struct chan_freq_power *cfp_BG;
	int cfp_no_BG;
};

/**
 * the structure for the mapping between region and CFP
 */
static struct region_cfp_table region_cfp_table[] = {
	{0x10,			/*US FCC */
	 channel_freq_power_US_BG,
	 ARRAY_SIZE(channel_freq_power_US_BG),
	 }
	,
	{0x20,			/*CANADA IC */
	 channel_freq_power_US_BG,
	 ARRAY_SIZE(channel_freq_power_US_BG),
	 }
	,
	{0x30, /*EU*/ channel_freq_power_EU_BG,
	 ARRAY_SIZE(channel_freq_power_EU_BG),
	 }
	,
	{0x31, /*SPAIN*/ channel_freq_power_SPN_BG,
	 ARRAY_SIZE(channel_freq_power_SPN_BG),
	 }
	,
	{0x32, /*FRANCE*/ channel_freq_power_FR_BG,
	 ARRAY_SIZE(channel_freq_power_FR_BG),
	 }
	,
	{0x40, /*JAPAN*/ channel_freq_power_JPN_BG,
	 ARRAY_SIZE(channel_freq_power_JPN_BG),
	 }
	,
/*Add new region here */
};

/**
 * the table to keep region code
 */
u16 lbs_region_code_to_index[MRVDRV_MAX_REGION_CODE] =
    { 0x10, 0x20, 0x30, 0x31, 0x32, 0x40 };

/**
 * 802.11b/g supported bitrates (in 500Kb/s units)
 */
u8 lbs_bg_rates[MAX_RATES] =
    { 0x02, 0x04, 0x0b, 0x16, 0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c,
0x00, 0x00 };

/**
 * FW rate table.  FW refers to rates by their index in this table, not by the
 * rate value itself.  Values of 0x00 are
 * reserved positions.
 */
static u8 fw_data_rates[MAX_RATES] =
    { 0x02, 0x04, 0x0B, 0x16, 0x00, 0x0C, 0x12,
      0x18, 0x24, 0x30, 0x48, 0x60, 0x6C, 0x00
};	

u32 lbs_fw_index_to_data_rate(u8 idx)
{
	if (idx >= sizeof(fw_data_rates))
		idx = 0;
	return fw_data_rates[idx];
}

void lbs_notify_command_response(struct lbs_private *priv, u8 resp_idx)
{
	//lbs_deb_enter(LBS_DEB_THREAD);

	if (priv->psstate == PS_STATE_SLEEP)
		priv->psstate = PS_STATE_AWAKE;

	/* Swap buffers by flipping the response index */
	//BUG_ON(resp_idx > 1);
	priv->resp_idx = resp_idx;

	lbs_thread(priv);

	//lbs_deb_leave(LBS_DEB_THREAD);
}

void lbs_host_to_card_done(struct lbs_private *priv)
{
   //	unsigned long flags;

	 lbs_deb_cmd_enter("enter lbs_host_to_card_done\n");

	 //spin_lock_irqsave(&priv->driver_lock, flags);
	 priv->dnld_sent = DNLD_RES_RECEIVED;

	/* Wake main thread if commands are pending */
	 if (!priv->cur_cmd || priv->tx_pending_len > 0)
	 {
		 //wake_up_interruptible(&priv->waitq);
		 lbs_thread(priv);
	 }
	 
	 //spin_unlock_irqrestore(&priv->driver_lock, flags);
	 lbs_deb_cmd_leave("leave lbs_host_to_card_done\n");
}

static int lbs_init_adapter(struct lbs_private *priv)
{
	size_t bufsize;
	int i, ret = 0;
	static u8 gnet_bss_desc[MAX_NETWORK_COUNT * sizeof(struct bss_descriptor)];

	lbs_deb_enter("enter lbs_init_adapter\n");
	/* Allocate buffer to store the BSSID list */
	bufsize = MAX_NETWORK_COUNT * sizeof(struct bss_descriptor);
	/*priv->networks = kzalloc(bufsize, GFP_KERNEL);
	if (!priv->networks) {
		lbs_pr_err("Out of memory allocating beacons\n");
		ret = -1;
		goto out;
	}*/
	priv->networks=(struct bss_descriptor *)gnet_bss_desc;
	memset(priv->networks,0,bufsize);

	/* Initialize scan result lists */
	INIT_LIST_HEAD(&priv->network_free_list);//Initialize scan related list and objects.
	INIT_LIST_HEAD(&priv->network_list);
	//Link all bss descriptors into free list.
	for (i = 0; i < MAX_NETWORK_COUNT; i++) {
		list_add_tail(&priv->networks[i].list,
			      &priv->network_free_list);
	}

	memset(priv->current_addr, 0xff, ETH_ALEN);
	priv->connect_status = LBS_DISCONNECTED;
	priv->mesh_connect_status = LBS_DISCONNECTED;
	priv->secinfo.auth_mode = IW_AUTH_ALG_OPEN_SYSTEM;
	priv->mode = IW_MODE_INFRA;
	priv->curbssparams.channel = DEFAULT_AD_HOC_CHANNEL;
	priv->mac_control = CMD_ACT_MAC_RX_ON | CMD_ACT_MAC_TX_ON;
	priv->radio_on = 1;
	priv->enablehwauto = 1;
	priv->capability = WLAN_CAPABILITY_SHORT_PREAMBLE;
	priv->psmode = LBS802_11POWERMODECAM;
	priv->psstate = PS_STATE_FULL_POWER;
	//mutex_init(&priv->lock);

	/*setup_timer(&priv->command_timer, command_timer_fn,
		(unsigned long)priv);*/

	INIT_LIST_HEAD(&priv->cmdfreeq);
	INIT_LIST_HEAD(&priv->cmdpendingq);

	/*spin_lock_init(&priv->driver_lock);
	init_waitqueue_head(&priv->cmd_pending);*/

	/* Allocate the command buffers */
	if (lbs_allocate_cmd_buffer(priv)) {
		lbs_pr_err("Out of memory allocating command buffers\n");
		ret = -ENOMEM;
		goto out;
	}
	priv->resp_idx = 0;
	priv->resp_len[0] = 0; //Contains the response from hardware.

	/* Create the event FIFO */
	/*priv->event_fifo = kfifo_alloc(sizeof(u32) * 16, GFP_KERNEL, NULL);
	if (IS_ERR(priv->event_fifo)) {
		lbs_pr_err("Out of memory allocating event FIFO buffer\n");
		ret = -ENOMEM;
		goto out;
	}*/

out:
	lbs_deb_leave_args("leave lbs_init_adapter(ret=%d)\n", ret);

	return ret;
}

int lbs_process_event(struct lbs_private *priv, u32 event)
{
	int ret = 0;
	lbs_deb_enter(LBS_DEB_CMD);

	switch (event) {
	case MACREG_INT_CODE_LINK_SENSED:
		printk("EVENT: link sensed\n");//掉线
		break;

	case MACREG_INT_CODE_DEAUTHENTICATED:
		printk("EVENT: deauthenticated\n");
		//lbs_mac_event_disconnected(priv);
		break;

	case MACREG_INT_CODE_DISASSOCIATED:
		printk("EVENT: disassociated\n");
		//lbs_mac_event_disconnected(priv);
		break;

	case MACREG_INT_CODE_LINK_LOST_NO_SCAN:
		printk("EVENT: link lost\n");
		//lbs_mac_event_disconnected(priv);
		break;

	case MACREG_INT_CODE_PS_SLEEP:
		printk("EVENT: ps sleep\n");

		/* handle unexpected PS SLEEP event */
		if (priv->psstate == PS_STATE_FULL_POWER) {
			printk("EVENT: in FULL POWER mode, ignoreing PS_SLEEP\n");
			break;
		}
		priv->psstate = PS_STATE_PRE_SLEEP;

		//lbs_ps_confirm_sleep(priv);

		break;

	case MACREG_INT_CODE_HOST_AWAKE:
		printk("EVENT: host awake\n");
		//lbs_send_confirmwake(priv);
		break;

	case MACREG_INT_CODE_PS_AWAKE:
		printk("EVENT: ps awake\n");
		/* handle unexpected PS AWAKE event */
		if (priv->psstate == PS_STATE_FULL_POWER) {
			printk(
			       "EVENT: In FULL POWER mode - ignore PS AWAKE\n");
			break;
		}

		priv->psstate = PS_STATE_AWAKE;

		if (priv->needtowakeup) {
			/*
			 * wait for the command processing to finish
			 * before resuming sending
			 * priv->needtowakeup will be set to FALSE
			 * in lbs_ps_wakeup()
			 */
			printk("waking up ...\n");
			//lbs_ps_wakeup(priv, 0);
		}
		break;

	case MACREG_INT_CODE_MIC_ERR_UNICAST:
		printk("EVENT: UNICAST MIC ERROR\n");
		//handle_mic_failureevent(priv, MACREG_INT_CODE_MIC_ERR_UNICAST);
		break;

	case MACREG_INT_CODE_MIC_ERR_MULTICAST:
		printk("EVENT: MULTICAST MIC ERROR\n");
		//handle_mic_failureevent(priv, MACREG_INT_CODE_MIC_ERR_MULTICAST);
		break;

	case MACREG_INT_CODE_MIB_CHANGED:
		printk("EVENT: MIB CHANGED\n");
		break;
	case MACREG_INT_CODE_INIT_DONE:
		printk("EVENT: INIT DONE\n");
		break;
	case MACREG_INT_CODE_ADHOC_BCN_LOST:
		printk("EVENT: ADHOC beacon lost\n");
		break;
	case MACREG_INT_CODE_RSSI_LOW:
		printk("EVENT: rssi low\n");
		break;
	case MACREG_INT_CODE_SNR_LOW:
		printk("EVENT: snr low\n");
		break;
	case MACREG_INT_CODE_MAX_FAIL:
		printk("EVENT: max fail\n");
		break;
	case MACREG_INT_CODE_RSSI_HIGH:
		printk("EVENT: rssi high\n");
		break;
	case MACREG_INT_CODE_SNR_HIGH:
		printk("EVENT: snr high\n");
		break;
#ifdef MASK_DEBUG
	case MACREG_INT_CODE_MESH_AUTO_STARTED:
		/* Ignore spurious autostart events if autostart is disabled */
		if (!priv->mesh_autostart_enabled) {
			lbs_pr_info("EVENT: MESH_AUTO_STARTED (ignoring)\n");
			break;
		}
		lbs_pr_info("EVENT: MESH_AUTO_STARTED\n");
		priv->mesh_connect_status = LBS_CONNECTED;
		if (priv->mesh_open) {
			netif_carrier_on(priv->mesh_dev);
			if (!priv->tx_pending_len)
				netif_wake_queue(priv->mesh_dev);
		}
		priv->mode = IW_MODE_ADHOC;
		schedule_work(&priv->sync_channel);
		break;
#endif
	default:
		printk("EVENT: unknown event id %d\n", event);
		break;
	}

	lbs_deb_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

/**
 *  @brief This function handles the major jobs in the LBS driver.
 *  It handles all events generated by firmware, RX data received
 *  from firmware and TX data sent from kernel.
 *
 *  @param data    A pointer to lbs_thread structure
 *  @return 	   0
 */
//HelloX: This thread will be waken up by other routines through the calling of
//wake_up_interruptible routine,in HelloX's implementation,this mechanism should be
//replaced by event,a dedicated event object is defined under lbs_private structure,
//the lbs_thread should wait on this event object when shouldsleep = 1.
//Other routines who want to process lbs command,should set the event object status
//to signal,thus lead the wake up of lbs_thread.
int lbs_thread(struct lbs_private *priv)
{
  //struct net_device *dev = data;
  //struct lbs_private *priv = dev->ml_priv;
	//wait_queue_t wait;
	int shouldsleep;
	u8 resp_idx;

	//lbs_deb_enter(LBS_DEB_THREAD);

	//init_waitqueue_entry(&wait, current);
	do{
		/*lbs_deb_thread("1: currenttxskb %p, dnld_sent %d\n",
				priv->currenttxskb, priv->dnld_sent);	 */

		/*add_wait_queue(&priv->waitq, &wait);
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irq(&priv->driver_lock);

		if (kthread_should_stop())
			shouldsleep = 0;	*/
		if (priv->surpriseremoved)
			shouldsleep = 1;	/* We need to wait until we're _told_ to die */
		else if (priv->psstate == PS_STATE_SLEEP)
			shouldsleep = 1;	/* Sleep mode. Nothing we can do till it wakes */
		else if (priv->cmd_timed_out)
			shouldsleep = 0;	/* Command timed out. Recover */
		else if (!priv->fw_ready)
			shouldsleep = 1;	/* Firmware not ready. We're waiting for it */
		else if (priv->dnld_sent)
			shouldsleep = 1;	/* Something is en route to the device already */
		//else if (priv->tx_pending_len > 0)
		//	shouldsleep = 0;	/* We've a packet to send */
		else if (priv->resp_len[priv->resp_idx])
			shouldsleep = 0;	/* We have a command response */
		else if (priv->cur_cmd)
			shouldsleep = 1;	/* Can't send a command; one already running */
		else if (!list_empty(&priv->cmdpendingq))
			shouldsleep = 0;	/* We have a command to send */
		/*  else if (__kfifo_len(priv->event_fifo))
			shouldsleep = 0;	*/ /* We have an event to process */
		else
			shouldsleep = 1;	/* No command */

		if (shouldsleep) {
			/*lbs_deb_thread("sleeping, connect_status %d, "
				"psmode %d, psstate %d\n",
				priv->connect_status,
				priv->psmode, priv->psstate);*/
			//spin_unlock_irq(&priv->driver_lock);
			//schedule();
			break;  //HelloX: This break clause might be added by the modifier,to giveup execute.
			        //Should remove this clause when implement this thread under HelloX.
		} 
		/*else
			spin_unlock_irq(&priv->driver_lock);*/

	  /*lbs_deb_thread("2: currenttxskb %p, dnld_send %d\n",
			       priv->currenttxskb, priv->dnld_sent);*/

		/*set_current_state(TASK_RUNNING);
		remove_wait_queue(&priv->waitq, &wait);*/
		//HelloX: Should reset the event status to non-signaling here,to achieve sychronization
		//purpose.

		/*lbs_deb_thread("3: currenttxskb %p, dnld_sent %d\n",
			       priv->currenttxskb, priv->dnld_sent);

		if (kthread_should_stop()) {
			lbs_deb_thread("break from main thread\n");
			break;
		}*/

		if (priv->surpriseremoved) {
			lbs_deb_thread("adapter removed; waiting to die...\n");
			continue;
		}

		/*lbs_deb_thread("4: currenttxskb %p, dnld_sent %d\n",
		       priv->currenttxskb, priv->dnld_sent);*/

		/* Process any pending command response */
		//spin_lock_irq(&priv->driver_lock);
		resp_idx = priv->resp_idx;
		if (priv->resp_len[resp_idx]) {
		//	spin_unlock_irq(&priv->driver_lock);
			lbs_process_command_response(priv,
				priv->resp_buf[resp_idx],
				priv->resp_len[resp_idx]);
			//spin_lock_irq(&priv->driver_lock);
			priv->resp_len[resp_idx] = 0;
		}
		//spin_unlock_irq(&priv->driver_lock);

		/* command timeout stuff */
		if (priv->cmd_timed_out && priv->cur_cmd)
		{
			struct cmd_ctrl_node *cmdnode = priv->cur_cmd;

			if (++priv->nr_retries > 0) {
				lbs_pr_info("Excessive timeouts submitting "
					"command 0x%04x\n",
					le16_to_cpu(cmdnode->cmdbuf->command));
				lbs_complete_command(priv, cmdnode, -ETIMEDOUT);
				priv->nr_retries = 0;
				/*if (priv->reset_card)
					priv->reset_card(priv);*/
			} else {
				priv->cur_cmd = NULL;
				priv->dnld_sent = DNLD_RES_RECEIVED;
				lbs_pr_info("requeueing command 0x%04x due "
					"to timeout (#%d)\n",
					le16_to_cpu(cmdnode->cmdbuf->command),
					priv->nr_retries);

				/* Stick it back at the _top_ of the pending queue
				   for immediate resubmission */
				list_add(&cmdnode->list, &priv->cmdpendingq);
			}
		}
		priv->cmd_timed_out = 0;

		/* Process hardware events, e.g. card removed, link lost */
		//spin_lock_irq(&priv->driver_lock);
	  /*	while (__kfifo_len(priv->event_fifo)) {
				u32 event;
			__kfifo_get(priv->event_fifo,(unsigned char *)&event,sizeof(event));
			//spin_unlock_irq(&priv->driver_lock);
			lbs_process_event(priv, event);
			//spin_lock_irq(&priv->driver_lock);
		} */
		//spin_unlock_irq(&priv->driver_lock);

		if (!priv->fw_ready)
			continue;

		/* Check if we need to confirm Sleep Request received previously */
		if (priv->psstate == PS_STATE_PRE_SLEEP &&
		    !priv->dnld_sent && !priv->cur_cmd) {
			if (priv->connect_status == LBS_CONNECTED) {
				/*lbs_deb_thread("pre-sleep, currenttxskb %p, "
					"dnld_sent %d, cur_cmd %p\n",
					priv->currenttxskb, priv->dnld_sent,
					priv->cur_cmd);*/

				//lbs_ps_confirm_sleep(priv);
				lbs_pr_alert("ignore PS_SleepConfirm in "
					"non-connected state\n");
				while(1){
					if(priv->psstate != PS_STATE_PRE_SLEEP)//bug
						break;
					}
			} else {
				/* workaround for firmware sending
				 * deauth/linkloss event immediately
				 * after sleep request; remove this
				 * after firmware fixes it
				 */
				priv->psstate = PS_STATE_AWAKE;
				lbs_pr_alert("ignore PS_SleepConfirm in "
					"non-connected state\n");
			}
		}

		/* The PS state is changed during processing of Sleep Request
		 * event above
		 */
		if ((priv->psstate == PS_STATE_SLEEP) ||
		    (priv->psstate == PS_STATE_PRE_SLEEP))
			continue;

		/* Execute the next command */
		if (!priv->dnld_sent && !priv->cur_cmd)
			lbs_execute_next_command(priv);

		/* Wake-up command waiters which can't sleep in
		 * lbs_prepare_and_send_command
		 */
		/*if (!list_empty(&priv->cmdpendingq))
			wake_up_all(&priv->cmd_pending);*/

#ifdef MASK_DEBUG

		//spin_lock_irq(&priv->driver_lock);
		if (!priv->dnld_sent && priv->tx_pending_len > 0) {
			int ret = priv->hw_host_to_card(priv, MVMS_DAT,
							priv->tx_pending_buf,
							priv->tx_pending_len);
			if (ret) {
				lbs_deb_tx("host_to_card failed %d\n", ret);
				priv->dnld_sent = DNLD_RES_RECEIVED;
			}
			priv->tx_pending_len = 0;
			/*if (!priv->currenttxskb) {
				//We can wake the queues immediately if we aren't
				 // waiting for TX feedback 
				if (priv->connect_status == LBS_CONNECTED)
					netif_wake_queue(priv->dev);
				if (priv->mesh_dev &&
				    priv->mesh_connect_status == LBS_CONNECTED)
					netif_wake_queue(priv->mesh_dev);
			}*/
		}
#endif
		//spin_unlock_irq(&priv->driver_lock);
	}while(0);  //HelloX: Maybe modified by the previous author,should change to while(1) clause when
	            //migrate to HelloX.

	//del_timer(&priv->command_timer);
	//wake_up_all(&priv->cmd_pending);

	//lbs_deb_leave("leave main thread!\n");

	return 0;
}

/**
 * @brief This function adds the card. it will probe the
 * card, allocate the lbs_priv and initialize the device.
 *
 *  @param card    A pointer to card
 *  @return 	   A pointer to struct lbs_private structure
 */

static struct lbs_private gmarvell_priv;
struct lbs_private *lbs_add_card(void *card)
{
	struct lbs_private *priv =&gmarvell_priv;
	lbs_deb_enter("enter lbs_add_card!\n");

	memset(priv,0,sizeof(struct lbs_private));
	if (lbs_init_adapter(priv)) {//Initialize priv structure.
		lbs_pr_err("failed to initialize adapter structure.\n");
		goto err_init_adapter;
	} 
	priv->card = card;//if_sdio_card
	priv->mesh_open = 0;
	priv->infra_open = 0;
	_hx_sprintf((char *)priv->mesh_ssid,"mesh");
	priv->mesh_ssid_len = 4;

	priv->wol_criteria = 0xffffffff;
	priv->wol_gpio = 0xff;

	goto done;

err_init_adapter:
done:
	lbs_deb_leave_args("leave lbs_add_card",0);
	return priv;
}



/**
 * @brief This function gets the HW spec from the firmware and sets
 *        some basic parameters.
 *
 *  @param priv    A pointer to struct lbs_private structure
 *  @return 	   0 or -1
 */
static int lbs_setup_firmware(struct lbs_private *priv)
{
	int ret = -1;
	 s16 curlevel = 0, minlevel = 0, maxlevel = 0;

	 lbs_deb_enter("enter lbs_setup_firmware\n");

	/* Read MAC address from firmware */
	memset(priv->current_addr, 0xff, ETH_ALEN);
	ret = lbs_update_hw_spec(priv);//根据固件的情况更新MAC地址、区域信道等细节信息填充到priv中
	if (ret)
		goto done;
	/* Read power levels if available */
	ret = lbs_get_tx_power(priv, &curlevel, &minlevel, &maxlevel);//获取当前功率、最小功率和最大功率
	if (ret == 0) {
		priv->txpower_cur = curlevel;
		priv->txpower_min = minlevel;
		priv->txpower_max = maxlevel;
	}

	lbs_set_mac_control(priv);	  
done:
	lbs_deb_leave_args("leave lbs_setup_firmware(ret=%d)\n", ret);

	return ret;
}

int lbs_start_card(struct lbs_private *priv)
{
	int ret = -1;
	 lbs_deb_enter("enter lbs_start_card\n");
	ret = lbs_setup_firmware(priv);//读取和设置相关的固件参数
	if (ret)
		goto done;
	lbs_init_11d(priv);//不是能802.11d
	lbs_update_channel(priv);//获取当前信道编号记录在priv->curbssparams.channel中
done:
	lbs_pr_info("Marvell WLAN 802.11 adapter\n");
	lbs_deb_leave_args("leave lbs_start_card(ret=%d)\n", ret);

	return ret;
}

/**
 *  @brief This function finds the CFP in
 *  region_cfp_table based on region and band parameter.
 *
 *  @param region  The region code
 *  @param band	   The band
 *  @param cfp_no  A pointer to CFP number
 *  @return 	   A pointer to CFP
 */
struct chan_freq_power *lbs_get_region_cfp_table(u8 region, int *cfp_no)
{
	int i, end;

	lbs_deb_enter(LBS_DEB_MAIN);

	end = ARRAY_SIZE(region_cfp_table);

	for (i = 0; i < end ; i++) {
		lbs_pr_debug("region_cfp_table[i].region=%d\n",
			region_cfp_table[i].region);
		if (region_cfp_table[i].region == region) {
			*cfp_no = region_cfp_table[i].cfp_no_BG;//返回区域对应的channel、frequency、power的数目
			lbs_deb_leave(LBS_DEB_MAIN);
			return region_cfp_table[i].cfp_BG;//返回获取到的cfp表地址
		}
	}

	lbs_deb_leave_args(LBS_DEB_MAIN, "ret NULL");
	return NULL;
}

int lbs_set_regiontable(struct lbs_private *priv, u8 region, u8 band)
{
	int ret = 0;
	int i = 0;

	struct chan_freq_power *cfp;
	int cfp_no;

	lbs_deb_enter(LBS_DEB_MAIN);

	memset(priv->region_channel, 0, sizeof(priv->region_channel));

	cfp = lbs_get_region_cfp_table(region, &cfp_no);//获取区域的802.11b/g的信道、频率和功率对应表
	if (cfp != NULL) {
		priv->region_channel[i].nrcfp = cfp_no;
		priv->region_channel[i].CFP = cfp;
	} else {
		lbs_pr_debug("wrong region code %#x in band B/G\n",
		       region);
		ret = -1;
		goto out;
	}
	priv->region_channel[i].valid = 1;
	priv->region_channel[i].region = region;
	priv->region_channel[i].band = band;
	i++;
out:
	lbs_deb_leave_args(LBS_DEB_MAIN, ret);
	return ret;
}

void lbs_queue_event(struct lbs_private *priv, u32 event)
{
  //unsigned long flags;
	lbs_deb_enter(LBS_DEB_THREAD);
	//spin_lock_irqsave(&priv->driver_lock, flags);

	if (priv->psstate == PS_STATE_SLEEP)
		priv->psstate = PS_STATE_AWAKE;

	__kfifo_put(priv->event_fifo, (unsigned char *)&event, sizeof(u32));
	//wake_up_interruptible(&priv->waitq);
	lbs_thread(priv);

	//spin_unlock_irqrestore(&priv->driver_lock, flags);
	lbs_deb_leave(LBS_DEB_THREAD);
}
