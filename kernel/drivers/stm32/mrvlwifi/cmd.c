#include "type.h"
#include "common.h"
#include "cmd.h"
#include "assoc.h"
#include "hostcmd.h"
#include "dev.h"
#include "hostcmdcode.h"
#include "marvel_main.h"
#include "if_sdio.h"

/**
 *  @brief Checks whether a command is allowed in Power Save mode
 *
 *  @param command the command ID
 *  @return 	   1 if allowed, 0 if not allowed
 */
static u8 is_command_allowed_in_ps(u16 cmd)
{
	switch (cmd) {
	case CMD_802_11_RSSI:
		return 1;
	default:
		break;
	}
	return 0;
}


/**
 *  This function inserts command node to cmdfreeq
 *  after cleans it. Requires priv->driver_lock held.
 */
static void __lbs_cleanup_and_insert_cmd(struct lbs_private *priv,
					 struct cmd_ctrl_node *cmdnode)
{
	lbs_deb_cmd_enter("enter __lbs_cleanup_and_insert_cmd\n");

	if (!cmdnode)
		goto out;

	cmdnode->callback = NULL;
	cmdnode->callback_arg = 0;//回调和回调参数

	memset(cmdnode->cmdbuf, 0, LBS_CMD_BUFFER_SIZE);

	list_add_tail(&cmdnode->list, &priv->cmdfreeq);//将未使用的cmdnode链入到空闲链表中
 out:
	lbs_deb_cmd_leave("leave __lbs_cleanup_and_insert_cmd\n");
}



static void lbs_cleanup_and_insert_cmd(struct lbs_private *priv,
	struct cmd_ctrl_node *ptempcmd)
{
//	unsigned long flags;

	//spin_lock_irqsave(&priv->driver_lock, flags);
	__lbs_cleanup_and_insert_cmd(priv, ptempcmd);
	//spin_unlock_irqrestore(&priv->driver_lock, flags);
}



/**
 *  @brief This function gets a free command node if available in
 *  command free queue.
 *
 *  @param priv		A pointer to struct lbs_private structure
 *  @return cmd_ctrl_node A pointer to cmd_ctrl_node structure or NULL
 */
static struct cmd_ctrl_node *lbs_get_cmd_ctrl_node(struct lbs_private *priv)
{
	struct cmd_ctrl_node *tempnode;
//	unsigned long flags;

	//lbs_deb_cmd_enter(LBS_DEB_HOST);

	if (!priv)
		return NULL;

	//spin_lock_irqsave(&priv->driver_lock, flags);
	if (!list_empty(&priv->cmdfreeq)) {
		tempnode = list_first_entry(&priv->cmdfreeq,
					    struct cmd_ctrl_node, list);
		list_del(&tempnode->list);
	} else {
		lbs_deb_host("GET_CMD_NODE: cmd_ctrl_node is not available\n");
		tempnode = NULL;
	}

	//spin_unlock_irqrestore(&priv->driver_lock, flags);

	//lbs_deb_cmd_leave(LBS_DEB_HOST);
	return tempnode;
}

static void lbs_queue_cmd(struct lbs_private *priv,
			  struct cmd_ctrl_node *cmdnode)
{
//	unsigned long flags;
	int addtail = 1;

	//lbs_deb_cmd_enter(LBS_DEB_HOST);

	if (!cmdnode) {
		lbs_deb_host("QUEUE_CMD: cmdnode is NULL\n");
		goto done;
	}
	if (!cmdnode->cmdbuf->size) {
		lbs_deb_host("DNLD_CMD: cmd size is zero\n");
		goto done;
	}
	cmdnode->result = 0;
	//以上是cmdnode的部分常规性检查
	/* Exit_PS command needs to be queued in the header always. */
	//PS_mode发送命令时必须检查的内容
	if (le16_to_cpu(cmdnode->cmdbuf->command) == CMD_802_11_PS_MODE) {
		struct cmd_ds_802_11_ps_mode *psm = (void *) &cmdnode->cmdbuf[1];

		if (psm->action == cpu_to_le16(CMD_SUBCMD_EXIT_PS)) {
			if (priv->psstate != PS_STATE_FULL_POWER)
				addtail = 0;
		}
	}

	//spin_lock_irqsave(&priv->driver_lock, flags);

	if (addtail)
		list_add_tail(&cmdnode->list, &priv->cmdpendingq);//加入到命令挂起链表尾部，主服务线程在执行next command是会遍历cmdpending链表
	else
		list_add(&cmdnode->list, &priv->cmdpendingq);

	//spin_unlock_irqrestore(&priv->driver_lock, flags);

	lbs_deb_host("QUEUE_CMD: inserted command 0x%04x into cmdpendingq\n",
		     le16_to_cpu(cmdnode->cmdbuf->command));

done:
	//lbs_deb_cmd_leave(LBS_DEB_HOST);
	return;
}

static void lbs_submit_command(struct lbs_private *priv,
			       struct cmd_ctrl_node *cmdnode)
{
//	unsigned long flags;
	struct cmd_header *cmd;
	uint16_t cmdsize;
//	int timeo = 3 * HZ;
	int ret;

	 lbs_deb_cmd_enter(LBS_DEB_HOST);

	cmd = cmdnode->cmdbuf;//获取命令码

	//spin_lock_irqsave(&priv->driver_lock, flags);
	priv->cur_cmd = cmdnode;
	priv->cur_cmd_retcode = 0;
	//spin_unlock_irqrestore(&priv->driver_lock, flags);

	cmdsize = le16_to_cpu(cmd->size);
	/* These commands take longer */
/*	if (command == CMD_802_11_SCAN || command == CMD_802_11_ASSOCIATE)
		timeo = 5 * HZ;	  */

	//lbs_deb_hex(LBS_DEB_CMD, "DNLD_CMD", (void *) cmdnode->cmdbuf, cmdsize);
	ret = priv->hw_host_to_card(priv, MVMS_CMD, (u8 *) cmd, cmdsize);

	if (ret) {
		lbs_pr_info("DNLD_CMD: hw_host_to_card failed: %d\n", ret);
		/* Let the timer kick in and retry, and potentially reset
		   the whole thing if the condition persists */
	//	timeo = HZ/4;
	}

	/* Setup the timer after transmit command */
	//mod_timer(&priv->command_timer, jiffies + timeo);

	 lbs_deb_cmd_leave(LBS_DEB_HOST);
}


/**
 *  @brief This function allocates the command buffer and link
 *  it to command free queue.
 *
 *  @param priv		A pointer to struct lbs_private structure
 *  @return 		0 or -1
 */
int lbs_allocate_cmd_buffer(struct lbs_private *priv)
{
	int ret = 0;
	u32 bufsize;
	u32 i;
	/*这里把cmdarray的数量做少点1个，因为它占用内存过大，一个cmdbuffer占用2K的内存*/
	static u8 gmarvel_cmdarray[sizeof(struct cmd_ctrl_node) * LBS_NUM_CMD_BUFFERS];
	static u8 gmarvel_cmdbuffer[LBS_NUM_CMD_BUFFERS][LBS_CMD_BUFFER_SIZE];
	struct cmd_ctrl_node *cmdarray;

	lbs_deb_cmd_enter("enter lbs_allocate_cmd_buffer\n");

	/* Allocate and initialize the command array */
	bufsize = sizeof(struct cmd_ctrl_node) * LBS_NUM_CMD_BUFFERS;//命令控制块结构
	/*if (!(cmdarray = kzalloc(bufsize, GFP_KERNEL))) {
		lbs_deb_host("ALLOC_CMD_BUF: tempcmd_array is NULL\n");
		ret = -1;
		goto done;
	}*/
	cmdarray=(struct cmd_ctrl_node *)gmarvel_cmdarray;
	memset(cmdarray,0,bufsize);//这是cmd_ctrl_node结构的存放地，后面会在这个里面来取
	
	priv->cmd_array = cmdarray;

	/* Allocate and initialize each command buffer in the command array */
	for (i = 0; i < LBS_NUM_CMD_BUFFERS; i++) {
		 /*cmdarray[i].cmdbuf = kzalloc(LBS_CMD_BUFFER_SIZE, GFP_KERNEL);//命令参数存放地
		if (!cmdarray[i].cmdbuf) {
			lbs_deb_host("ALLOC_CMD_BUF: ptempvirtualaddr is NULL\n");
			ret = -1;
			goto done;
		}*/
		cmdarray[i].cmdbuf=(struct cmd_header *)&gmarvel_cmdbuffer[i][0];
		memset(cmdarray[i].cmdbuf,0,LBS_CMD_BUFFER_SIZE);
	}

	for (i = 0; i < LBS_NUM_CMD_BUFFERS; i++) {
		//init_waitqueue_head(&cmdarray[i].cmdwait_q);//初始化等待队列头，主服务线程可能在上面睡眠
		lbs_cleanup_and_insert_cmd(priv, &cmdarray[i]);//初始化空闲链表
	}
	ret = 0;
//done:
	lbs_deb_cmd_leave_args("leave lbs_allocate_cmd_buffer(ret=%d)\n", ret);
	return ret;
}


/**
 *  @brief Get the min, max, and current TX power
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *  @param curlevel  	Current power level in dBm
 *  @param minlevel  	Minimum supported power level in dBm (optional)
 *  @param maxlevel  	Maximum supported power level in dBm (optional)
 *
 *  @return 	   	0 on success, error on failure
 */
int lbs_get_tx_power(struct lbs_private *priv, s16 *curlevel, s16 *minlevel,
		     s16 *maxlevel)
{
	struct cmd_ds_802_11_rf_tx_power cmd;//每个cmd前面会嵌套一个cmd_header结构
	int ret;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_GET);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_TX_POWER, &cmd);
	if (ret == 0) {
		*curlevel = le16_to_cpu(cmd.curlevel);
		if (minlevel)
			*minlevel = cmd.minlevel;
		if (maxlevel)
			*maxlevel = cmd.maxlevel;
	}

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return ret;
}


void lbs_set_mac_control(struct lbs_private *priv)
{
	struct cmd_ds_mac_control cmd;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(priv->mac_control);
	cmd.reserved = 0;

	lbs_cmd_async(priv, CMD_MAC_CONTROL, &cmd.hdr, sizeof(cmd));//与lbs_cmd_with_response区别主要是不用等待响应，因此也不用睡眠

	lbs_deb_cmd_leave(LBS_DEB_CMD);
}

/**
 *  @brief Simple callback that ignores the result. Use this if
 *  you just want to send a command to the hardware, but don't
 *  care for the result.
 *
 *  @param priv         ignored
 *  @param extra        ignored
 *  @param resp         ignored
 *
 *  @return 	   	0 for success
 */
static int lbs_cmd_async_callback(struct lbs_private *priv, unsigned long extra,
		     struct cmd_header *resp)
{
	return 0;
}

/**
 *  @brief Updates the hardware details like MAC address and regulatory region
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *
 *  @return 	   	0 on success, error on failure
 */
int lbs_update_hw_spec(struct lbs_private *priv)
{
	struct cmd_ds_get_hw_spec cmd;
	//cmd_ds_get_hw_spec定义get_hw_spec命令的响应结构，具体的内容结构中有详细说明
	int ret = -1;
	u32 i;

	 lbs_deb_cmd_enter("enter lbs_update_hw_spec!\n");

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	memcpy(cmd.permanentaddr, priv->current_addr, ETH_ALEN);
	//发送命令获取硬件的spec，完成命令的响应数据存放子在cmd中
	 ret=lbs_cmd_with_response(priv,CMD_GET_HW_SPEC,(void *)&cmd);
	if (ret)
		goto out;
	

	priv->fwcapinfo = le32_to_cpu(cmd.fwcapinfo);

	/* The firmware release is in an interesting format: the patch
	 * level is in the most significant nibble ... so fix that: */
	priv->fwrelease = le32_to_cpu(cmd.fwrelease);
	priv->fwrelease = (priv->fwrelease << 8) |
		(priv->fwrelease >> 24 & 0xff);

	/* Some firmware capabilities:
	 * CF card    firmware 5.0.16p0:   cap 0x00000303
	 * USB dongle firmware 5.110.17p2: cap 0x00000303
	 */
	lbs_pr_info("%2x:%2x:%2x:%2x:%2x:%2x, fw %u.%u.%up%u, cap 0x%08x\n",
		cmd.permanentaddr[0],
		cmd.permanentaddr[1],
		cmd.permanentaddr[2],
		cmd.permanentaddr[3],
		cmd.permanentaddr[4],
		cmd.permanentaddr[5],
		priv->fwrelease >> 24 & 0xff,
		priv->fwrelease >> 16 & 0xff,
		priv->fwrelease >>  8 & 0xff,
		priv->fwrelease       & 0xff,
		priv->fwcapinfo);
	lbs_deb_cmd("GET_HW_SPEC: hardware interface 0x%x, hardware spec 0x%04x\n",
		    cmd.hwifversion, cmd.version);

	/* Determine mesh_fw_ver from fwrelease and fwcapinfo */
	/* 5.0.16p0 9.0.0.p0 is known to NOT support any mesh */
	/* 5.110.22 have mesh command with 0xa3 command id */
	/* 10.0.0.p0 FW brings in mesh config command with different id */
	/* Check FW version MSB and initialize mesh_fw_ver */
	if (MRVL_FW_MAJOR_REV(priv->fwrelease) == MRVL_FW_V5)
		priv->mesh_fw_ver = MESH_FW_OLD;
	else if ((MRVL_FW_MAJOR_REV(priv->fwrelease) >= MRVL_FW_V10) &&
		(priv->fwcapinfo & MESH_CAPINFO_ENABLE_MASK))
		priv->mesh_fw_ver = MESH_FW_NEW;
	else
		priv->mesh_fw_ver = MESH_NONE;

	/* Clamp region code to 8-bit since FW spec indicates that it should
	 * only ever be 8-bit, even though the field size is 16-bit.  Some firmware
	 * returns non-zero high 8 bits here.
	 *
	 * Firmware version 4.0.102 used in CF8381 has region code shifted.  We
	 * need to check for this problem and handle it properly.
	 */
	if (MRVL_FW_MAJOR_REV(priv->fwrelease) == MRVL_FW_V4)
		priv->regioncode = (le16_to_cpu(cmd.regioncode) >> 8) & 0xFF;
	else
		priv->regioncode = le16_to_cpu(cmd.regioncode) & 0xFF;
	//以上内容是通过读取固件版本信息确定网卡的一些mesh和区域信息
	for (i = 0; i < MRVDRV_MAX_REGION_CODE; i++) {
		/* use the region code to search for the index */
		if (priv->regioncode == lbs_region_code_to_index[i])
			break;
	}

	/* if it's unidentified region code, use the default (USA) */
	if (i >= MRVDRV_MAX_REGION_CODE) {
		priv->regioncode = 0x10;
		lbs_pr_info("unidentified region code; using the default (USA)\n");
	}

	if (priv->current_addr[0] == 0xff)
		//获取到网卡固件内部的mac地址，以此作为网卡当前的mac地址
		memmove(priv->current_addr, cmd.permanentaddr, ETH_ALEN);
	
	//memcpy(priv->dev->dev_addr, priv->current_addr, ETH_ALEN);
	//if (priv->mesh_dev)
	//	memcpy(priv->mesh_dev->dev_addr, priv->current_addr, ETH_ALEN);

	if (lbs_set_regiontable(priv, priv->regioncode, 0)) {//根据区域码获取网卡的信道表
		ret = -1;
		goto out;
	}

	if (lbs_set_universaltable(priv, 0)) {
		ret = -1;
		goto out;
	}

out:
	 lbs_deb_cmd_leave("leave lbs_update_hw_spec\n");
	return ret;
}


static struct cmd_ctrl_node *__lbs_cmd_async(struct lbs_private *priv,
	uint16_t command, struct cmd_header *in_cmd, int in_cmd_size,
	int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;

	 lbs_deb_cmd_enter(LBS_DEB_HOST);

	if (priv->surpriseremoved) {
		lbs_deb_host("PREP_CMD: card removed\n");
		cmdnode = ERR_PTR(-ENOENT);
		goto done;
	}
	//从空闲的命令链表中获取一个未使用的command node
	cmdnode = lbs_get_cmd_ctrl_node(priv);
	if (cmdnode == NULL) {
		lbs_deb_host("PREP_CMD: cmdnode is NULL\n");

		/* Wake up main thread to execute next command */
		//wake_up_interruptible(&priv->waitq);
		lbs_thread(priv);
		//如果申请不到空闲的cmd块，说明还有很多命令尚未执行，需要唤醒内核线程工作
		cmdnode = ERR_PTR(-ENOBUFS);
		goto done;
	}

	cmdnode->callback = callback;//填充命令完成的回调函数
	cmdnode->callback_arg = callback_arg;//回调参数

	/* Copy the incoming command to the buffer */
	memcpy(cmdnode->cmdbuf, in_cmd, in_cmd_size);//把命令放入到buffer

	/* Set sequence number, clean result, move to buffer */
	priv->seqnum++;
	cmdnode->cmdbuf->command = cpu_to_le16(command);//具体的命令
	cmdnode->cmdbuf->size    = cpu_to_le16(in_cmd_size);//命令长度
	cmdnode->cmdbuf->seqnum  = cpu_to_le16(priv->seqnum);//命令序列号
	cmdnode->cmdbuf->result  = 0;//清除命令结果

	lbs_deb_host("PREP_CMD: command 0x%04x\n", command);

	cmdnode->cmdwaitqwoken = 0;//当前命令尚未唤醒，主线程完成命令执行以后会将此位置1，用来唤醒本命令继续执行
	lbs_queue_cmd(priv, cmdnode);//提交命令
	//wake_up_interruptible(&priv->waitq);//唤醒主服务线程执行命令
	lbs_thread(priv);//命令处理阶段
	
 done:
	lbs_deb_cmd_leave_args( "ret %p", cmdnode->cmdwaitqwoken);
	return cmdnode;
}

int __lbs_cmd(struct lbs_private *priv, uint16_t command,
	      struct cmd_header *in_cmd, int in_cmd_size,
	      int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      unsigned long callback_arg)
{
	struct cmd_ctrl_node *cmdnode;
	//unsigned long flags = 0;  //Just for debugging.
	struct if_sdio_card *card=priv->card;
	int ret = 0;
	unsigned long time_out;
	
	lbs_deb_cmd_enter("enter __lbs_cmd\n");

	cmdnode = __lbs_cmd_async(priv, command, in_cmd, in_cmd_size,
				  callback, callback_arg);
	if (IS_ERR(cmdnode)) {
		ret = PTR_ERR(cmdnode);
		goto done;
	}
	
	// might_sleep();
	// wait_event_interruptible(cmdnode->cmdwait_q, cmdnode->cmdwaitqwoken);
	// use poll mode to check if the desired interrupt is occur,in timeout style.
	time_out = 500;
	while(1){		
		ret = poll_sdio_interrupt(card->func);
		if(ret<0){
			lbs_pr_err("read interrupt error!\n");
			try_bug(0);
		}
		//else if(ret > 0)
		//{
		//	if_sdio_interrupt(card->func);
		//}
		else if(ret&IF_SDIO_H_INT_UPLD)
		{
			if_sdio_interrupt(card->func);
			//flags = 1;
			break;
		}
		else if(ret&IF_SDIO_H_INT_DNLD)
		{
			if_sdio_interrupt(card->func);
		}
		else if(time_after(jiffies,&time_out))
		{
			priv->cmd_timed_out = 1;
			lbs_thread(priv);     //Handle timeout event,may reset the hardware.
			ret = -ETIME;
			//flags = 2;
			break;
		}
	}
	
	time_out = 500;
	while((!cmdnode->cmdwaitqwoken) && (!time_after(jiffies,&time_out)));//Wait command execute over.
	if(0 == time_out)  //Timeout,should clean up the commands.
	{
		_hx_printf("__lbs_cmd: Wait interrupt timeout.\r\n");
		priv->cur_cmd = NULL;
		if(!list_empty(&priv->cmdpendingq))
		{
			cmdnode = list_first_entry(&priv->cmdpendingq,
					   struct cmd_ctrl_node, list);
		}
	}
	//spin_lock_irqsave(&priv->driver_lock, flags);
	ret = cmdnode->result;
	if (ret)
		lbs_pr_err("PREP_CMD: command 0x%04x failed: %d\n",
			    command, ret);
	
	__lbs_cleanup_and_insert_cmd(priv, cmdnode);
	//spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	lbs_deb_cmd_leave_args( "leave __lbs_cmd(ret=%d)\n", ret);
	return ret;
}


void lbs_cmd_async(struct lbs_private *priv, uint16_t command,
	struct cmd_header *in_cmd, int in_cmd_size)
{
	struct if_sdio_card *card=priv->card;
	int ret = 0;
	
	lbs_deb_cmd_enter(LBS_DEB_CMD);
	__lbs_cmd_async(priv, command, in_cmd, in_cmd_size,
		lbs_cmd_async_callback, 0);

	/*这是一个bug的修复，本来lbs_cmd_async提交的命令是不用等待他返回的，但是卡还是
	会返回一个状态码，所以这里必须同步读取*/
	while(1){
		ret = poll_sdio_interrupt(card->func);
		if(ret<0){
			lbs_pr_err("read interrupt error!\n");
			try_bug(0);
		}
		else if(ret)//读到中断
			break;
	}
	if_sdio_interrupt(card->func);
	lbs_deb_cmd_leave(LBS_DEB_CMD);
}


/**
 *  @brief Simple callback that copies response back into command
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *  @param extra  	A pointer to the original command structure for which
 *                      'resp' is a response
 *  @param resp         A pointer to the command response
 *
 *  @return 	   	0 on success, error on failure
 */
int lbs_cmd_copyback(struct lbs_private *priv, unsigned long extra,
		     struct cmd_header *resp)
{
	struct cmd_header *buf = (void *)extra;
	uint16_t copy_len;

	copy_len = min(le16_to_cpu(buf->size), le16_to_cpu(resp->size));
	memcpy(buf, resp, copy_len);
	return 0;
}

void lbs_complete_command(struct lbs_private *priv, struct cmd_ctrl_node *cmd,
			  int result)
{
	if (cmd == priv->cur_cmd)
		priv->cur_cmd_retcode = result;

	cmd->result = result;
	cmd->cmdwaitqwoken = 1;
	//wake_up_interruptible(&cmd->cmdwait_q);//唤醒当前命令的处理，进行善后工作

	if (!cmd->callback || cmd->callback == lbs_cmd_async_callback)//没有回调或者是异步回调就直接清除命令，表示命令执行完成
		__lbs_cleanup_and_insert_cmd(priv, cmd);//清除命令
	priv->cur_cmd = NULL;
}


static int lbs_ret_reg_access(struct lbs_private *priv,//读取mac、baseband、rf寄存器的值
			       u16 type, struct cmd_ds_command *resp)
{
	int ret = 0;

	lbs_deb_cmd_enter("enter lbs_ret_reg_access\n");

	switch (type) {
	case CMD_RET(CMD_MAC_REG_ACCESS):
		{
			struct cmd_ds_mac_reg_access *reg = &resp->params.macreg;//响应的数据

			priv->offsetvalue.offset = (u32)le16_to_cpu(reg->offset);
			priv->offsetvalue.value = le32_to_cpu(reg->value);
			break;
		}

	case CMD_RET(CMD_BBP_REG_ACCESS):
		{
			struct cmd_ds_bbp_reg_access *reg = &resp->params.bbpreg;

			priv->offsetvalue.offset = (u32)le16_to_cpu(reg->offset);
			priv->offsetvalue.value = reg->value;
			break;
		}

	case CMD_RET(CMD_RF_REG_ACCESS):
		{
			struct cmd_ds_rf_reg_access *reg = &resp->params.rfreg;

			priv->offsetvalue.offset = (u32)le16_to_cpu(reg->offset);
			priv->offsetvalue.value = reg->value;
			break;
		}

	default:
		ret = -1;
	}

	lbs_deb_cmd_leave_args("leave lbs_ret_reg_access(ret=%d)", ret);
	return ret;
}

static int lbs_ret_802_11_rssi(struct lbs_private *priv,
				struct cmd_ds_command *resp)
{
	struct cmd_ds_802_11_rssi_rsp *rssirsp = &resp->params.rssirsp;

	//lbs_deb_cmd_enter(LBS_DEB_CMD);

	/* store the non average value */
	priv->SNR[TYPE_BEACON][TYPE_NOAVG] = get_unaligned_le16((__le16 *)&rssirsp->SNR);
	priv->NF[TYPE_BEACON][TYPE_NOAVG] = get_unaligned_le16((__le16 *)&rssirsp->noisefloor);

	priv->SNR[TYPE_BEACON][TYPE_AVG] = get_unaligned_le16((__le16 *)&rssirsp->avgSNR);
	priv->NF[TYPE_BEACON][TYPE_AVG] = get_unaligned_le16((__le16 *)&rssirsp->avgnoisefloor);

	priv->RSSI[TYPE_BEACON][TYPE_NOAVG] =
	    CAL_RSSI(priv->SNR[TYPE_BEACON][TYPE_NOAVG],
		     priv->NF[TYPE_BEACON][TYPE_NOAVG]);

	priv->RSSI[TYPE_BEACON][TYPE_AVG] =
	    CAL_RSSI(priv->SNR[TYPE_BEACON][TYPE_AVG] / AVG_SCALE,
		     priv->NF[TYPE_BEACON][TYPE_AVG] / AVG_SCALE);

	lbs_deb_cmd("RSSI: beacon %d, avg %d\n",
	       priv->RSSI[TYPE_BEACON][TYPE_NOAVG],
	       priv->RSSI[TYPE_BEACON][TYPE_AVG]);

	//lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_ret_802_11_bcn_ctrl(struct lbs_private * priv,
					struct cmd_ds_command *resp)
{
	struct cmd_ds_802_11_beacon_control *bcn_ctrl =
	    &resp->params.bcn_ctrl;

	//lbs_deb_cmd_enter(LBS_DEB_CMD);

	if (bcn_ctrl->action == CMD_ACT_GET) {
		priv->beacon_enable = (u8) le16_to_cpu(bcn_ctrl->beacon_enable);
		priv->beacon_period = le16_to_cpu(bcn_ctrl->beacon_period);
	}

	//lbs_deb_cmd_enter(LBS_DEB_CMD);
	return 0;
}

static __inline int handle_cmd_response(struct lbs_private *priv,
				      struct cmd_header *cmd_response)
{
	struct cmd_ds_command *resp = (struct cmd_ds_command *) cmd_response;
	int ret = 0;
//	unsigned long flags;
	uint16_t respcmd = le16_to_cpu(resp->command);

	lbs_deb_cmd_enter("enter handle_cmd_response\n");

	switch (respcmd) {
	case CMD_RET(CMD_MAC_REG_ACCESS):
	case CMD_RET(CMD_BBP_REG_ACCESS):
	case CMD_RET(CMD_RF_REG_ACCESS):
		ret = lbs_ret_reg_access(priv, respcmd, resp);//读取mac、baseband、rf寄存器的值
		break;

	case CMD_RET(CMD_802_11_SET_AFC):
	case CMD_RET(CMD_802_11_GET_AFC):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		memmove((void *)priv->cur_cmd->callback_arg, &resp->params.afc,
			sizeof(struct cmd_ds_802_11_afc));//将返回的内容传递回回调函数
		//spin_unlock_irqrestore(&priv->driver_lock, flags);

		break;

	case CMD_RET(CMD_802_11_BEACON_STOP):
		break;

	case CMD_RET(CMD_802_11_RSSI):
		ret = lbs_ret_802_11_rssi(priv, resp);
		break;

	case CMD_RET(CMD_802_11D_DOMAIN_INFO):
		//ret = lbs_ret_802_11d_domain_info(resp);
		pr_err("failed CMD_802_11D_DOMAIN_INFO\n");
		while(1){
			if(CMD_802_11D_DOMAIN_INFO!=respcmd)
				break;//this is a bug
		}
		break;

	case CMD_RET(CMD_802_11_TPC_CFG):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		memmove((void *)priv->cur_cmd->callback_arg, &resp->params.tpccfg,
			sizeof(struct cmd_ds_802_11_tpc_cfg));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		break;
	case CMD_RET(CMD_802_11_LED_GPIO_CTRL):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		memmove((void *)priv->cur_cmd->callback_arg, &resp->params.ledgpio,
			sizeof(struct cmd_ds_802_11_led_ctrl));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		break;

	case CMD_RET(CMD_GET_TSF):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		memcpy((void *)priv->cur_cmd->callback_arg,
		       (const void *)(__le16 *)&resp->params.gettsf.tsfvalue, sizeof(u64));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		break;
	case CMD_RET(CMD_BT_ACCESS):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		if (priv->cur_cmd->callback_arg)
			memcpy((void *)priv->cur_cmd->callback_arg,
			       &resp->params.bt.addr1, 2 * ETH_ALEN);
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		break;
	case CMD_RET(CMD_FWT_ACCESS):
		//spin_lock_irqsave(&priv->driver_lock, flags);
		if (priv->cur_cmd->callback_arg)
			memcpy((void *)priv->cur_cmd->callback_arg, &resp->params.fwt,
			       sizeof(resp->params.fwt));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		break;
	case CMD_RET(CMD_802_11_BEACON_CTRL):
		ret = lbs_ret_802_11_bcn_ctrl(priv, resp);
		break;

	default:
		lbs_pr_err("CMD_RESP: unknown cmd response 0x%04x\n",
			   le16_to_cpu(resp->command));
		break;
	}
	 lbs_deb_cmd_leave(LBS_DEB_HOST);
	return ret;
}

int lbs_process_command_response(struct lbs_private *priv, u8 *data, u32 len)
{
	uint16_t respcmd, curcmd;
	struct cmd_header *resp;
	int ret = 0;
//	unsigned long flags;
	uint16_t result;

	 lbs_deb_cmd_enter("enter lbs_process_command_response!\n");

	//mutex_lock(&priv->lock);
	//spin_lock_irqsave(&priv->driver_lock, flags);

	if (!priv->cur_cmd) {
		lbs_deb_host("CMD_RESP: cur_cmd is NULL\n");
		ret = -1;
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		goto done;
	}

	resp = (void *)data;
	curcmd = le16_to_cpu(priv->cur_cmd->cmdbuf->command);
	respcmd = le16_to_cpu(resp->command);
	result = le16_to_cpu(resp->result);

	lbs_deb_cmd("CMD_RESP: response 0x%04x, seq %d, size %d\n",
		     respcmd, le16_to_cpu(resp->seqnum), len);
	//lbs_deb_hex(LBS_DEB_CMD, "CMD_RESP", (void *) resp, len);

	if (resp->seqnum != priv->cur_cmd->cmdbuf->seqnum) {//当前序列和响应序列不一致说明出错
		lbs_pr_info("Received CMD_RESP with invalid sequence %d (expected %d)\n",
			    le16_to_cpu(resp->seqnum), le16_to_cpu(priv->cur_cmd->cmdbuf->seqnum));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}
	if (respcmd != CMD_RET(curcmd) &&
	    respcmd != CMD_RET_802_11_ASSOCIATE && curcmd != CMD_802_11_ASSOCIATE) {
		lbs_pr_info("Invalid CMD_RESP %x to command %x!\n", respcmd, curcmd);
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	if (resp->result == cpu_to_le16(0x0004)) {
		/* 0x0004 means -EAGAIN. Drop the response, let it time out
		   and be resubmitted */
		lbs_pr_info("Firmware returns DEFER to command %x. Will let it time out...\n",
			    le16_to_cpu(resp->command));
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	/* Now we got response from FW, cancel the command timer */
	//del_timer(&priv->command_timer);//删除超时定时器
	priv->cmd_timed_out = 0;
	if (priv->nr_retries) {
		lbs_pr_info("Received result %x to command %x after %d retries\n",
			    result, curcmd, priv->nr_retries);
		priv->nr_retries = 0;
	}

	/* Store the response code to cur_cmd_retcode. */
	priv->cur_cmd_retcode = result;
	//以下是关于低功耗的处理单独列出来
	if (respcmd == CMD_RET(CMD_802_11_PS_MODE)) {//如果发送的命令是ps，就是power save模式
		struct cmd_ds_802_11_ps_mode *psmode = (void *) &resp[1];
		u16 action = le16_to_cpu(psmode->action);

		lbs_deb_host(
		       "CMD_RESP: PS_MODE cmd reply result 0x%x, action 0x%x\n",
		       result, action);

		if (result) {//CMD_802_11_PS_MODE命令失败
			lbs_deb_host("CMD_RESP: PS command failed with 0x%x\n",
				    result);
			/*
			 * We should not re-try enter-ps command in
			 * ad-hoc mode. It takes place in
			 * lbs_execute_next_command().
			 */
			if (priv->mode == IW_MODE_ADHOC &&
			    action == CMD_SUBCMD_ENTER_PS)
				priv->psmode = LBS802_11POWERMODECAM;
		} else if (action == CMD_SUBCMD_ENTER_PS) {
			priv->needtowakeup = 0;
			priv->psstate = PS_STATE_AWAKE;

			lbs_deb_host("CMD_RESP: ENTER_PS command response\n");
			if (priv->connect_status != LBS_CONNECTED) {//进入低功耗模式之前必须保持网络连接
				/*
				 * When Deauth Event received before Enter_PS command
				 * response, We need to wake up the firmware.
				 */
				lbs_deb_host(
				       "disconnected, invoking lbs_ps_wakeup\n");

				/*spin_unlock_irqrestore(&priv->driver_lock, flags);
				mutex_unlock(&priv->lock);
				lbs_ps_wakeup(priv, 0);//退出低功耗模式
				mutex_lock(&priv->lock);
				spin_lock_irqsave(&priv->driver_lock, flags);*/
			}
		} else if (action == CMD_SUBCMD_EXIT_PS) {
			priv->needtowakeup = 0;
			priv->psstate = PS_STATE_FULL_POWER;
			lbs_deb_host("CMD_RESP: EXIT_PS command response\n");
		} else {
			lbs_deb_host("CMD_RESP: PS action 0x%X\n", action);
		}

		lbs_complete_command(priv, priv->cur_cmd, result);//唤醒命令等待队列，并清除命令
		//spin_unlock_irqrestore(&priv->driver_lock, flags);

		ret = 0;
		goto done;
	}

	/* If the command is not successful, cleanup and return failure */
	if ((result != 0 || !(respcmd & 0x8000))) {//出错处理
		lbs_deb_host("CMD_RESP: error 0x%04x in command reply 0x%04x\n",
		       result, respcmd);
		/*
		 * Handling errors here
		 */
		switch (respcmd) {
		case CMD_RET(CMD_GET_HW_SPEC):
		case CMD_RET(CMD_802_11_RESET):
			lbs_deb_host("CMD_RESP: reset failed\n");
			break;

		}
		lbs_complete_command(priv, priv->cur_cmd, result);
		//spin_unlock_irqrestore(&priv->driver_lock, flags);

		ret = -1;
		goto done;
	}

	//spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (priv->cur_cmd && priv->cur_cmd->callback) {
		ret = priv->cur_cmd->callback(priv, priv->cur_cmd->callback_arg,
				resp);
		//如果命令带有回调函数，调用回调函数。
		//一般对应是对固件的操作，如读mac等和固件相关的操作
		//典型的如lbs_cmd_with_response，会将响应的数据通过回调拷贝到相应的数据区
	} else
		ret = handle_cmd_response(priv, resp);//没有回调函数的会调用默认的处理方法

	//spin_lock_irqsave(&priv->driver_lock, flags);

	if (priv->cur_cmd) {
		/* Clean up and Put current command back to cmdfreeq */
		lbs_complete_command(priv, priv->cur_cmd, result);
	}
	//spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	//mutex_unlock(&priv->lock);
	lbs_deb_cmd_leave_args("leave lbs_process_command_response(ret=%d)\n", ret);
	return ret;
}

/**
 *  @brief This function executes next command in command
 *  pending queue. It will put firmware back to PS mode
 *  if applicable.
 *
 *  @param priv     A pointer to struct lbs_private structure
 *  @return 	   0 or -1
 */
int lbs_execute_next_command(struct lbs_private *priv)
{
	struct cmd_ctrl_node *cmdnode = NULL;
	struct cmd_header *cmd;
//	unsigned long flags;
	struct cmd_ds_802_11_ps_mode *psm;
	int ret = 0;

	/* Debug group is LBS_DEB_THREAD and not LBS_DEB_HOST, because the
	 * only caller to us is lbs_thread() and we get even when a
	 * data packet is received */
	lbs_deb_cmd_enter("enter lbs_execute_next_command\n");

	//spin_lock_irqsave(&priv->driver_lock, flags);

	if (priv->cur_cmd) {
		lbs_pr_alert( "EXEC_NEXT_CMD: already processing command!\n");
		//spin_unlock_irqrestore(&priv->driver_lock, flags);
		ret = -1;
		goto done;
	}

	if (!list_empty(&priv->cmdpendingq)) {
		cmdnode = list_first_entry(&priv->cmdpendingq,
					   struct cmd_ctrl_node, list);
	}

	//spin_unlock_irqrestore(&priv->driver_lock, flags);

	if (cmdnode) {
		cmd = cmdnode->cmdbuf;

		if (is_command_allowed_in_ps(le16_to_cpu(cmd->command))) {//power save模式下能执行的命令也就rssi
			if ((priv->psstate == PS_STATE_SLEEP) ||
			    (priv->psstate == PS_STATE_PRE_SLEEP)) {
				lbs_deb_host(
				       "EXEC_NEXT_CMD: cannot send cmd 0x%04x in psstate %d\n",
				       le16_to_cpu(cmd->command),
				       priv->psstate);
				ret = -1;
				goto done;
			}
			lbs_deb_host("EXEC_NEXT_CMD: OK to send command "
				     "0x%04x in psstate %d\n",
				     le16_to_cpu(cmd->command), priv->psstate);
		} else if (priv->psstate != PS_STATE_FULL_POWER) {
			/*
			 * 1. Non-PS command:
			 * Queue it. set needtowakeup to TRUE if current state
			 * is SLEEP, otherwise call lbs_ps_wakeup to send Exit_PS.
			 * 2. PS command but not Exit_PS:
			 * Ignore it.
			 * 3. PS command Exit_PS:
			 * Set needtowakeup to TRUE if current state is SLEEP,
			 * otherwise send this command down to firmware
			 * immediately.
			 */
			if (cmd->command != cpu_to_le16(CMD_802_11_PS_MODE)) {
				/*  Prepare to send Exit PS,
				 *  this non PS command will be sent later */
				if ((priv->psstate == PS_STATE_SLEEP)
				    || (priv->psstate == PS_STATE_PRE_SLEEP)
				    ) {//睡眠就算了，只是置为标志
					/* w/ new scheme, it will not reach here.
					   since it is blocked in main_thread. */
					priv->needtowakeup = 1;
				} else{
					//lbs_ps_wakeup(priv, 0);//低功耗模式下的唤醒
					printk("power down mode,cann't exec cmd!\n");
					try_bug(0);
					}	

				ret = 0;
				goto done;
			} else {
				/*
				 * PS command. Ignore it if it is not Exit_PS.
				 * otherwise send it down immediately.
				 */
				 psm = (void *)&cmd[1];

				lbs_deb_host(
				       "EXEC_NEXT_CMD: PS cmd, action 0x%02x\n",
				       psm->action);
				if (psm->action !=
				    cpu_to_le16(CMD_SUBCMD_EXIT_PS)) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: ignore ENTER_PS cmd\n");
					list_del(&cmdnode->list);
					//spin_lock_irqsave(&priv->driver_lock, flags);
					lbs_complete_command(priv, cmdnode, 0);
					//spin_unlock_irqrestore(&priv->driver_lock, flags);

					ret = 0;
					goto done;
				}

				if ((priv->psstate == PS_STATE_SLEEP) ||
				    (priv->psstate == PS_STATE_PRE_SLEEP)) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: ignore EXIT_PS cmd in sleep\n");
					list_del(&cmdnode->list);
					//spin_lock_irqsave(&priv->driver_lock, flags);
					lbs_complete_command(priv, cmdnode, 0);
					//spin_unlock_irqrestore(&priv->driver_lock, flags);
					priv->needtowakeup = 1;

					ret = 0;
					goto done;
				}

				lbs_deb_host(
				       "EXEC_NEXT_CMD: sending EXIT_PS\n");
			}
		}
		list_del(&cmdnode->list);
		lbs_deb_host("EXEC_NEXT_CMD: sending command 0x%04x\n",
			    le16_to_cpu(cmd->command));
		lbs_submit_command(priv, cmdnode);//真正将命令写入网卡的函数
	} 
#ifdef SUPPORT_POWER_SLEEP
	else {
		/*
		 * check if in power save mode, if yes, put the device back
		 * to PS mode
		 */
		if ((priv->psmode != LBS802_11POWERMODECAM) &&
		    (priv->psstate == PS_STATE_FULL_POWER) &&
		    ((priv->connect_status == LBS_CONNECTED) ||
		    (priv->mesh_connect_status == LBS_CONNECTED))) {
			if (priv->secinfo.WPAenabled ||
			    priv->secinfo.WPA2enabled) {
				/* check for valid WPA group keys */
				if (priv->wpa_mcast_key.len ||
				    priv->wpa_unicast_key.len) {
					lbs_deb_host(
					       "EXEC_NEXT_CMD: WPA enabled and GTK_SET"
					       " go back to PS_SLEEP");
					//lbs_ps_sleep(priv, 0);
					try_bug(0);
				}
			} else {
				lbs_deb_host(
				       "EXEC_NEXT_CMD: cmdpendingq empty, "
				       "go back to PS_SLEEP");
				//lbs_ps_sleep(priv, 0);
				try_bug(0);
			}
		}
	}
#endif

	ret = 0;
done:
	 lbs_deb_cmd_leave(LBS_DEB_THREAD);
	return ret;
}

static int lbs_cmd_802_11_ps_mode(struct cmd_ds_command *cmd,
				   u16 cmd_action)
{
	struct cmd_ds_802_11_ps_mode *psm = &cmd->params.psmode;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	cmd->command = cpu_to_le16(CMD_802_11_PS_MODE);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_ps_mode) +
				S_DS_GEN);
	psm->action = cpu_to_le16(cmd_action);
	psm->multipledtim = 0;
	switch (cmd_action) {
	case CMD_SUBCMD_ENTER_PS:
		lbs_deb_cmd("PS command:" "SubCode- Enter PS\n");

		psm->locallisteninterval = 0;
		psm->nullpktinterval = 0;
		psm->multipledtim =
		    cpu_to_le16(MRVDRV_DEFAULT_MULTIPLE_DTIM);
		break;

	case CMD_SUBCMD_EXIT_PS:
		lbs_deb_cmd("PS command:" "SubCode- Exit PS\n");
		break;

	case CMD_SUBCMD_SLEEP_CONFIRMED:
		lbs_deb_cmd("PS command: SubCode- sleep confirm\n");
		break;

	default:
		break;
	}

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_reg_access(struct cmd_ds_command *cmdptr,
			       u8 cmd_action, void *pdata_buf)
{
	struct lbs_offset_value *offval;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	offval = (struct lbs_offset_value *)pdata_buf;

	switch (le16_to_cpu(cmdptr->command)) {
	case CMD_MAC_REG_ACCESS:
		{
			struct cmd_ds_mac_reg_access *macreg;

			cmdptr->size =
			    cpu_to_le16(sizeof (struct cmd_ds_mac_reg_access)
					+ S_DS_GEN);
			macreg =
			    (struct cmd_ds_mac_reg_access *)&cmdptr->params.
			    macreg;

			macreg->action = cpu_to_le16(cmd_action);
			macreg->offset = cpu_to_le16((u16) offval->offset);
			macreg->value = cpu_to_le32(offval->value);

			break;
		}

	case CMD_BBP_REG_ACCESS:
		{
			struct cmd_ds_bbp_reg_access *bbpreg;

			cmdptr->size =
			    cpu_to_le16(sizeof
					     (struct cmd_ds_bbp_reg_access)
					     + S_DS_GEN);
			bbpreg =
			    (struct cmd_ds_bbp_reg_access *)&cmdptr->params.
			    bbpreg;

			bbpreg->action = cpu_to_le16(cmd_action);
			bbpreg->offset = cpu_to_le16((u16) offval->offset);
			bbpreg->value = (u8) offval->value;

			break;
		}

	case CMD_RF_REG_ACCESS:
		{
			struct cmd_ds_rf_reg_access *rfreg;

			cmdptr->size =
			    cpu_to_le16(sizeof
					     (struct cmd_ds_rf_reg_access) +
					     S_DS_GEN);
			rfreg =
			    (struct cmd_ds_rf_reg_access *)&cmdptr->params.
			    rfreg;

			rfreg->action = cpu_to_le16(cmd_action);
			rfreg->offset = cpu_to_le16((u16) offval->offset);
			rfreg->value = (u8) offval->value;

			break;
		}

	default:
		break;
	}

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_802_11_monitor_mode(struct cmd_ds_command *cmd,
				      u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_802_11_monitor_mode *monitor = &cmd->params.monitor;

	cmd->command = cpu_to_le16(CMD_802_11_MONITOR_MODE);
	cmd->size =
	    cpu_to_le16(sizeof(struct cmd_ds_802_11_monitor_mode) +
			     S_DS_GEN);

	monitor->action = cpu_to_le16(cmd_action);
	if (cmd_action == CMD_ACT_SET) {
		monitor->mode =
		    cpu_to_le16((u16) (*(u32 *) pdata_buf));
	}

	return 0;
}

static int lbs_cmd_802_11_rssi(struct lbs_private *priv,
				struct cmd_ds_command *cmd)
{

	lbs_deb_cmd_enter(LBS_DEB_CMD);
	cmd->command = cpu_to_le16(CMD_802_11_RSSI);
	cmd->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_rssi) + S_DS_GEN);
	cmd->params.rssi.N = cpu_to_le16(DEFAULT_BCN_AVG_FACTOR);

	/* reset Beacon SNR/NF/RSSI values */
	priv->SNR[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->SNR[TYPE_BEACON][TYPE_AVG] = 0;
	priv->NF[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->NF[TYPE_BEACON][TYPE_AVG] = 0;
	priv->RSSI[TYPE_BEACON][TYPE_NOAVG] = 0;
	priv->RSSI[TYPE_BEACON][TYPE_AVG] = 0;

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}


static int lbs_cmd_bt_access(struct cmd_ds_command *cmd,
			       u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_bt_access *bt_access = &cmd->params.bt;
	lbs_deb_cmd_enter_args(LBS_DEB_CMD, cmd_action);

	cmd->command = cpu_to_le16(CMD_BT_ACCESS);
	cmd->size = cpu_to_le16((sizeof(struct cmd_ds_bt_access)+S_DS_GEN));
	cmd->result = 0;
	bt_access->action = cpu_to_le16(cmd_action);

	switch (cmd_action) {
	case CMD_ACT_BT_ACCESS_ADD:
		memcpy(bt_access->addr1, pdata_buf, 2 * ETH_ALEN);
		lbs_deb_hex(LBS_DEB_MESH, "BT_ADD: blinded MAC addr", bt_access->addr1, 6);
		break;
	case CMD_ACT_BT_ACCESS_DEL:
		memcpy(bt_access->addr1, pdata_buf, 1 * ETH_ALEN);
		lbs_deb_hex(LBS_DEB_MESH, "BT_DEL: blinded MAC addr", bt_access->addr1, 6);
		break;
	case CMD_ACT_BT_ACCESS_LIST:
		bt_access->id = cpu_to_le32(*(u32 *) pdata_buf);
		break;
	case CMD_ACT_BT_ACCESS_RESET:
		break;
	case CMD_ACT_BT_ACCESS_SET_INVERT:
		bt_access->id = cpu_to_le32(*(u32 *) pdata_buf);
		break;
	case CMD_ACT_BT_ACCESS_GET_INVERT:
		break;
	default:
		break;
	}
	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_fwt_access(struct cmd_ds_command *cmd,
			       u16 cmd_action, void *pdata_buf)
{
	struct cmd_ds_fwt_access *fwt_access = &cmd->params.fwt;
	lbs_deb_cmd_enter_args(LBS_DEB_CMD,cmd_action);

	cmd->command = cpu_to_le16(CMD_FWT_ACCESS);
	cmd->size = cpu_to_le16((sizeof(struct cmd_ds_fwt_access)+S_DS_GEN));
	cmd->result = 0;

	if (pdata_buf)
		memcpy(fwt_access, pdata_buf, sizeof(*fwt_access));
	else
		memset(fwt_access, 0, sizeof(*fwt_access));

	fwt_access->action = cpu_to_le16(cmd_action);

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

static int lbs_cmd_bcn_ctrl(struct lbs_private * priv,
				struct cmd_ds_command *cmd,
				u16 cmd_action)
{
	struct cmd_ds_802_11_beacon_control
		*bcn_ctrl = &cmd->params.bcn_ctrl;

	lbs_deb_cmd_enter(LBS_DEB_CMD);
	cmd->size =
	    cpu_to_le16(sizeof(struct cmd_ds_802_11_beacon_control)
			     + S_DS_GEN);
	cmd->command = cpu_to_le16(CMD_802_11_BEACON_CTRL);

	bcn_ctrl->action = cpu_to_le16(cmd_action);
	bcn_ctrl->beacon_enable = cpu_to_le16(priv->beacon_enable);
	bcn_ctrl->beacon_period = cpu_to_le16(priv->beacon_period);

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return 0;
}

/**
 *  @brief This function prepare the command before send to firmware.
 *
 *  @param priv		A pointer to struct lbs_private structure
 *  @param cmd_no	command number
 *  @param cmd_action	command action: GET or SET
 *  @param wait_option	wait option: wait response or not
 *  @param cmd_oid	cmd oid: treated as sub command
 *  @param pdata_buf	A pointer to informaion buffer
 *  @return 		0 or -1
 */
int lbs_prepare_and_send_command(struct lbs_private *priv,
			  u16 cmd_no,
			  u16 cmd_action,
			  u16 wait_option, u32 cmd_oid, void *pdata_buf)
{
	int ret = 0;
	struct cmd_ctrl_node *cmdnode;
	struct cmd_ds_command *cmdptr;
 //	unsigned long flags;
	unsigned long time_out=1000;
	struct if_sdio_card *card=priv->card;
	
	 lbs_deb_cmd_enter(LBS_DEB_HOST);

	if (!priv) {
		lbs_deb_host("PREP_CMD: priv is NULL\n");
		ret = -1;
		goto done;
	}

	if (priv->surpriseremoved) {
		lbs_deb_host("PREP_CMD: card removed\n");
		ret = -1;
		goto done;
	}

	cmdnode = lbs_get_cmd_ctrl_node(priv);

	if (cmdnode == NULL) {
		lbs_deb_host("PREP_CMD: cmdnode is NULL\n");

		/* Wake up main thread to execute next command */
		//wake_up_interruptible(&priv->waitq);
		lbs_thread(priv);
		ret = -1;
		goto done;
	}

	cmdnode->callback = NULL;
	cmdnode->callback_arg = (unsigned long)pdata_buf;

	cmdptr = (struct cmd_ds_command *)cmdnode->cmdbuf;

	lbs_deb_host("PREP_CMD: command 0x%04x\n", cmd_no);

	/* Set sequence number, command and INT option */
	priv->seqnum++;
	cmdptr->seqnum = cpu_to_le16(priv->seqnum);

	cmdptr->command = cpu_to_le16(cmd_no);
	cmdptr->result = 0;

	switch (cmd_no) {
	case CMD_802_11_PS_MODE:
		ret = lbs_cmd_802_11_ps_mode(cmdptr, cmd_action);
		break;

	case CMD_MAC_REG_ACCESS:
	case CMD_BBP_REG_ACCESS:
	case CMD_RF_REG_ACCESS:
		ret = lbs_cmd_reg_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_802_11_MONITOR_MODE:
		ret = lbs_cmd_802_11_monitor_mode(cmdptr,
				          cmd_action, pdata_buf);
		break;

	case CMD_802_11_RSSI:
		ret = lbs_cmd_802_11_rssi(priv, cmdptr);
		break;

	case CMD_802_11_SET_AFC:
	case CMD_802_11_GET_AFC:

		cmdptr->command = cpu_to_le16(cmd_no);
		cmdptr->size = cpu_to_le16(sizeof(struct cmd_ds_802_11_afc) +
					   S_DS_GEN);

		memmove(&cmdptr->params.afc,
			pdata_buf, sizeof(struct cmd_ds_802_11_afc));

		ret = 0;
		goto done;
#ifdef MASK_DEBUG
	case CMD_802_11D_DOMAIN_INFO:
		ret = lbs_cmd_802_11d_domain_info(priv, cmdptr,
						   cmd_no, cmd_action);
		break;
#endif
	case CMD_802_11_TPC_CFG:
		cmdptr->command = cpu_to_le16(CMD_802_11_TPC_CFG);
		cmdptr->size =
		    cpu_to_le16(sizeof(struct cmd_ds_802_11_tpc_cfg) +
				     S_DS_GEN);

		memmove(&cmdptr->params.tpccfg,
			pdata_buf, sizeof(struct cmd_ds_802_11_tpc_cfg));

		ret = 0;
		break;
	case CMD_802_11_LED_GPIO_CTRL:
		{
			struct mrvl_ie_ledgpio *gpio =
			    (struct mrvl_ie_ledgpio*)
			    cmdptr->params.ledgpio.data;

			memmove(&cmdptr->params.ledgpio,
				pdata_buf,
				sizeof(struct cmd_ds_802_11_led_ctrl));

			cmdptr->command =
			    cpu_to_le16(CMD_802_11_LED_GPIO_CTRL);

#define ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN 8
			cmdptr->size =
			    cpu_to_le16(le16_to_cpu(gpio->header.len)
				+ S_DS_GEN
				+ ACTION_NUMLED_TLVTYPE_LEN_FIELDS_LEN);
			gpio->header.len = gpio->header.len;

			ret = 0;
			break;
		}

	case CMD_BT_ACCESS:
		ret = lbs_cmd_bt_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_FWT_ACCESS:
		ret = lbs_cmd_fwt_access(cmdptr, cmd_action, pdata_buf);
		break;

	case CMD_GET_TSF:
		cmdptr->command = cpu_to_le16(CMD_GET_TSF);
		cmdptr->size = cpu_to_le16(sizeof(struct cmd_ds_get_tsf) +
					   S_DS_GEN);
		ret = 0;
		break;
	case CMD_802_11_BEACON_CTRL:
		ret = lbs_cmd_bcn_ctrl(priv, cmdptr, cmd_action);
		break;
	default:
		lbs_pr_err("PREP_CMD: unknown command 0x%04x\n", cmd_no);
		ret = -1;
		break;
	}

	/* return error, since the command preparation failed */
	if (ret != 0) {
		lbs_deb_host("PREP_CMD: command preparation failed\n");
		lbs_cleanup_and_insert_cmd(priv, cmdnode);
		ret = -1;
		goto done;
	}

	cmdnode->cmdwaitqwoken = 0;

	lbs_queue_cmd(priv, cmdnode);
	//wake_up_interruptible(&priv->waitq);//唤醒主线程
	lbs_thread(priv);
	
	if (wait_option & CMD_OPTION_WAITFORRSP) {
		lbs_deb_host("PREP_CMD: wait for response\n");
		//might_sleep();
		/*wait_event_interruptible(cmdnode->cmdwait_q,
					 cmdnode->cmdwaitqwoken);*/
	while(1){
		
		ret = poll_sdio_interrupt(card->func);
		if(ret<0){
			lbs_pr_err("read interrupt error!\n");
			try_bug(0);
		}
		else if(ret&IF_SDIO_H_INT_DNLD)
				if_sdio_interrupt(card->func);
		else if(ret&IF_SDIO_H_INT_UPLD){//读到数据中断
			if_sdio_interrupt(card->func);
			break;
		}
		else if(time_after(jiffies,&time_out)){
			priv->cmd_timed_out=1;
			 lbs_thread(priv);//处理超时
			 ret=-ETIME;
			 _hx_printf("  lbs_prepare_and_send_command: Wait interrupt timeout.\r\n");
			 break;
			}
		}
	 
	while(!cmdnode->cmdwaitqwoken);//等待命令处理完成		
		
	}

	//spin_lock_irqsave(&priv->driver_lock, flags);
	if (priv->cur_cmd_retcode) {
		lbs_deb_host("PREP_CMD: command failed with return code %d\n",
		       priv->cur_cmd_retcode);
		priv->cur_cmd_retcode = 0;
		ret = -1;
	}
	//spin_unlock_irqrestore(&priv->driver_lock, flags);

done:
	lbs_deb_cmd_leave_args(LBS_DEB_HOST, ret);
	return ret;
}

/**
 *  @brief Get the radio channel
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *
 *  @return 	   	The channel on success, error on failure
 */
int lbs_get_channel(struct lbs_private *priv)
{
	struct cmd_ds_802_11_rf_channel cmd;
	int ret = 0;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_OPT_802_11_RF_CHANNEL_GET);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_CHANNEL, &cmd);
	if (ret)
		goto out;

	ret = le16_to_cpu(cmd.channel);
	lbs_deb_cmd("current radio channel is %d\n", ret);

out:
	lbs_deb_cmd_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

int lbs_update_channel(struct lbs_private *priv)
{
	int ret;

	/* the channel in f/w could be out of sync; get the current channel */
	lbs_deb_cmd_enter(LBS_DEB_ASSOC);

	ret = lbs_get_channel(priv);//获取当前信道的通道号
	if (ret > 0) {
		priv->curbssparams.channel = ret;
		ret = 0;
	}
	lbs_deb_cmd_leave_args(LBS_DEB_ASSOC, ret);
	return ret;
}

/**
 *  @brief Set the radio channel
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *  @param channel  	The desired channel, or 0 to clear a locked channel
 *
 *  @return 	   	0 on success, error on failure
 */
int lbs_set_channel(struct lbs_private *priv, u8 channel)
{
	struct cmd_ds_802_11_rf_channel cmd;
	int ret = 0;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_OPT_802_11_RF_CHANNEL_SET);
	cmd.channel = cpu_to_le16(channel);

	ret = lbs_cmd_with_response(priv, CMD_802_11_RF_CHANNEL, &cmd);
	if (ret)
		goto out;

	priv->curbssparams.channel = (uint8_t) le16_to_cpu(cmd.channel);
	lbs_deb_cmd("channel switch from %d to %d\n", old_channel,
		priv->curbssparams.channel);

out:
	lbs_deb_cmd_leave_args(LBS_DEB_CMD,  ret);
	return ret;
}


int lbs_cmd_802_11_set_wep(struct lbs_private *priv, uint16_t cmd_action,
			   struct assoc_request *assoc)
{
	struct cmd_ds_802_11_set_wep cmd;
	int ret = 0;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof(cmd));
	cmd.hdr.command = cpu_to_le16(CMD_802_11_SET_WEP);
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	cmd.action = cpu_to_le16(cmd_action);

	if (cmd_action == CMD_ACT_ADD) {
		int i;

		/* default tx key index */
		cmd.keyindex = cpu_to_le16(assoc->wep_tx_keyidx &
					   CMD_WEP_KEY_INDEX_MASK);

		/* Copy key types and material to host command structure */
		for (i = 0; i < 4; i++) {
			struct enc_key *pkey = &assoc->wep_keys[i];

			switch (pkey->len) {
			case KEY_LEN_WEP_40:
				cmd.keytype[i] = CMD_TYPE_WEP_40_BIT;
				memmove(cmd.keymaterial[i], pkey->key, pkey->len);
				lbs_deb_cmd("SET_WEP: add key %d (40 bit)\n", i);
				break;
			case KEY_LEN_WEP_104:
				cmd.keytype[i] = CMD_TYPE_WEP_104_BIT;
				memmove(cmd.keymaterial[i], pkey->key, pkey->len);
				lbs_deb_cmd("SET_WEP: add key %d (104 bit)\n", i);
				break;
			case 0:
				break;
			default:
				lbs_deb_cmd("SET_WEP: invalid key %d, length %d\n",
					    i, pkey->len);
				ret = -1;
				goto done;
				//break;
			}
		}
	} else if (cmd_action == CMD_ACT_REMOVE) {
		/* ACT_REMOVE clears _all_ WEP keys */

		/* default tx key index */
		cmd.keyindex = cpu_to_le16(priv->wep_tx_keyidx &
					   CMD_WEP_KEY_INDEX_MASK);
		lbs_deb_cmd("SET_WEP: remove key %d\n", priv->wep_tx_keyidx);
	}

	ret = lbs_cmd_with_response(priv, CMD_802_11_SET_WEP, &cmd);
done:
	lbs_deb_cmd_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

int lbs_cmd_802_11_enable_rsn(struct lbs_private *priv, uint16_t cmd_action,
			      uint16_t *enable)
{
	struct cmd_ds_802_11_enable_rsn cmd;
	int ret;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(cmd_action);

	if (cmd_action == CMD_ACT_GET)
		cmd.enable = 0;
	else {
		if (*enable)
			cmd.enable = cpu_to_le16(CMD_ENABLE_RSN);
		else
			cmd.enable = cpu_to_le16(CMD_DISABLE_RSN);
		lbs_deb_cmd("ENABLE_RSN: %d\n", *enable);
	}

	ret = lbs_cmd_with_response(priv, CMD_802_11_ENABLE_RSN, &cmd);
	if (!ret && cmd_action == CMD_ACT_GET)
		*enable = le16_to_cpu(cmd.enable);

	lbs_deb_cmd_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

static void set_one_wpa_key(struct MrvlIEtype_keyParamSet *keyparam,
                            struct enc_key *key)
{
	lbs_deb_enter(LBS_DEB_CMD);

	if (key->flags & KEY_INFO_WPA_ENABLED)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_ENABLED);
	if (key->flags & KEY_INFO_WPA_UNICAST)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_UNICAST);
	if (key->flags & KEY_INFO_WPA_MCAST)
		keyparam->keyinfo |= cpu_to_le16(KEY_INFO_WPA_MCAST);

	keyparam->type = cpu_to_le16(TLV_TYPE_KEY_MATERIAL);
	keyparam->keytypeid = cpu_to_le16(key->type);
	keyparam->keylen = cpu_to_le16(key->len);
	memcpy(keyparam->key, key->key, key->len);

	/* Length field doesn't include the {type,length} header */
	keyparam->length = cpu_to_le16(sizeof(*keyparam) - 4);
	lbs_deb_leave(LBS_DEB_CMD);
}

int lbs_cmd_802_11_key_material(struct lbs_private *priv, uint16_t cmd_action,
				struct assoc_request *assoc)
{
	struct cmd_ds_802_11_key_material cmd;
	int ret = 0;
	int index = 0;

	lbs_deb_enter(LBS_DEB_CMD);

	cmd.action = cpu_to_le16(cmd_action);
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));

	if (cmd_action == CMD_ACT_GET) {
		cmd.hdr.size = cpu_to_le16(S_DS_GEN + 2);
	} else {
		memset(cmd.keyParamSet, 0, sizeof(cmd.keyParamSet));

		if (test_bit(ASSOC_FLAG_WPA_UCAST_KEY, &assoc->flags)) {
			set_one_wpa_key(&cmd.keyParamSet[index],
					&assoc->wpa_unicast_key);
			index++;
		}

		if (test_bit(ASSOC_FLAG_WPA_MCAST_KEY, &assoc->flags)) {
			set_one_wpa_key(&cmd.keyParamSet[index],
					&assoc->wpa_mcast_key);
			index++;
		}

		/* The common header and as many keys as we included */
		cmd.hdr.size = cpu_to_le16(offsetof(struct cmd_ds_802_11_key_material,//(cmd),
						    keyParamSet[index]));
	}
	ret = lbs_cmd_with_response(priv, CMD_802_11_KEY_MATERIAL, &cmd);
	/* Copy the returned key to driver private data */
	if (!ret && cmd_action == CMD_ACT_GET) {
		void *buf_ptr = cmd.keyParamSet;
		void *resp_end = &(&cmd)[1];

		while (buf_ptr < resp_end) {
			struct MrvlIEtype_keyParamSet *keyparam = buf_ptr;
			struct enc_key *key;
			uint16_t param_set_len = le16_to_cpu(keyparam->length);
			uint16_t key_len = le16_to_cpu(keyparam->keylen);
			uint16_t key_flags = le16_to_cpu(keyparam->keyinfo);
			uint16_t key_type = le16_to_cpu(keyparam->keytypeid);
			char *end;

			end =(char *)keyparam + sizeof(keyparam->type)
				+ sizeof(keyparam->length) + param_set_len;

			/* Make sure we don't access past the end of the IEs */
			if (end > resp_end)
				break;

			if (key_flags & KEY_INFO_WPA_UNICAST)
				key = &priv->wpa_unicast_key;
			else if (key_flags & KEY_INFO_WPA_MCAST)
				key = &priv->wpa_mcast_key;
			else
				break;

			/* Copy returned key into driver */
			memset(key, 0, sizeof(struct enc_key));
			if (key_len > sizeof(key->key))
				break;
			key->type = key_type;
			key->flags = key_flags;
			key->len = key_len;
			memcpy(key->key, keyparam->key, key->len);

			buf_ptr = (void *)(end + 1);
		}
	}

	//lbs_deb_leave_args(LBS_DEB_CMD, "ret %d", ret);
	return ret;
}

int lbs_set_radio(struct lbs_private *priv, u8 preamble, u8 radio_on)
{
	struct cmd_ds_802_11_radio_control cmd;
	int ret = -EINVAL;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);

	/* Only v8 and below support setting the preamble */
	if (priv->fwrelease < 0x09000000) {
		switch (preamble) {
		case RADIO_PREAMBLE_SHORT:
			if (!(priv->capability & WLAN_CAPABILITY_SHORT_PREAMBLE))
				goto out;
			/* Fall through */
		case RADIO_PREAMBLE_AUTO:
		case RADIO_PREAMBLE_LONG:
			cmd.control = cpu_to_le16(preamble);
			break;
		default:
			goto out;
		}
	}

	if (radio_on)
		cmd.control |= cpu_to_le16(0x1);
	else {
		cmd.control &= cpu_to_le16(~0x1);
		priv->txpower_cur = 0;
	}

	lbs_deb_cmd("RADIO_CONTROL: radio %s, preamble %d\n",
		    radio_on ? "ON" : "OFF", preamble);

	priv->radio_on = radio_on;

	ret = lbs_cmd_with_response(priv, CMD_802_11_RADIO_CONTROL, &cmd);

out:
	lbs_deb_cmd_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

#ifdef MASK_DEBUG//这个部分网卡不会响应

static int __lbs_mesh_config_send(struct lbs_private *priv,
				  struct cmd_ds_mesh_config *cmd,
				  uint16_t action, uint16_t type)
{
	int ret;
	u16 command = CMD_MESH_CONFIG_OLD;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	/*
	 * Command id is 0xac for v10 FW along with mesh interface
	 * id in bits 14-13-12.
	 */
	if (priv->mesh_fw_ver == MESH_FW_NEW)
		command = CMD_MESH_CONFIG |
			  (MESH_IFACE_ID << MESH_IFACE_BIT_OFFSET);

	cmd->hdr.command = cpu_to_le16(command);
	cmd->hdr.size = cpu_to_le16(sizeof(struct cmd_ds_mesh_config));
	cmd->hdr.result = 0;

	cmd->type = cpu_to_le16(type);
	cmd->action = cpu_to_le16(action);

	ret = lbs_cmd_with_response(priv, command, cmd);

	lbs_deb_cmd_leave(LBS_DEB_CMD);
	return ret;
}

#define IEEE80211_MAX_SSID_LEN		32

/* This function is the CMD_MESH_CONFIG legacy function.  It only handles the
 * START and STOP actions.  The extended actions supported by CMD_MESH_CONFIG
 * are all handled by preparing a struct cmd_ds_mesh_config and passing it to
 * lbs_mesh_config_send.
 */
int lbs_mesh_config(struct lbs_private *priv, uint16_t action, uint16_t chan)
{
	struct cmd_ds_mesh_config cmd;
	struct mrvl_meshie *ie;
	DECLARE_SSID_BUF(ssid);

	memset(&cmd, 0, sizeof(cmd));
	cmd.channel = cpu_to_le16(chan);
	ie = (struct mrvl_meshie *)cmd.data;

	switch (action) {
	case CMD_ACT_MESH_CONFIG_START:
		ie->id = WLAN_EID_GENERIC;
		ie->val.oui[0] = 0x00;
		ie->val.oui[1] = 0x50;
		ie->val.oui[2] = 0x43;
		ie->val.type = MARVELL_MESH_IE_TYPE;
		ie->val.subtype = MARVELL_MESH_IE_SUBTYPE;
		ie->val.version = MARVELL_MESH_IE_VERSION;
		ie->val.active_protocol_id = MARVELL_MESH_PROTO_ID_HWMP;
		ie->val.active_metric_id = MARVELL_MESH_METRIC_ID;
		ie->val.mesh_capability = MARVELL_MESH_CAPABILITY;
		ie->val.mesh_id_len = priv->mesh_ssid_len;
		memcpy(ie->val.mesh_id, priv->mesh_ssid, priv->mesh_ssid_len);
		ie->len = sizeof(struct mrvl_meshie_val) -
			IW_ESSID_MAX_SIZE + priv->mesh_ssid_len;
		cmd.length = cpu_to_le16(sizeof(struct mrvl_meshie_val));
		break;
	case CMD_ACT_MESH_CONFIG_STOP:
		break;
	default:
		return -1;
	}
	lbs_deb_cmd("mesh config action %d type %x channel %d SSID \n",
		    action, priv->mesh_tlv, chan);
		   // print_ssid(ssid, priv->mesh_ssid, priv->mesh_ssid_len)

	return __lbs_mesh_config_send(priv, &cmd, action, priv->mesh_tlv);
}

#endif

/**
 *  @brief Set an SNMP MIB value
 *
 *  @param priv    	A pointer to struct lbs_private structure
 *  @param oid  	The OID to set in the firmware
 *  @param val  	Value to set the OID to
 *
 *  @return 	   	0 on success, error on failure
 */
int lbs_set_snmp_mib(struct lbs_private *priv, u32 oid, u16 val)
{
	struct cmd_ds_802_11_snmp_mib cmd;
	int ret;

	lbs_deb_cmd_enter(LBS_DEB_CMD);

	memset(&cmd, 0, sizeof (cmd));
	cmd.hdr.size = cpu_to_le16(sizeof(cmd));
	cmd.action = cpu_to_le16(CMD_ACT_SET);
	cmd.oid = cpu_to_le16((u16) oid);

	switch (oid) {
	case SNMP_MIB_OID_BSS_TYPE:
		cmd.bufsize = cpu_to_le16(sizeof(u8));
		cmd.value[0] = (val == IW_MODE_ADHOC) ? 2 : 1;
		break;
	case SNMP_MIB_OID_11D_ENABLE:
	case SNMP_MIB_OID_FRAG_THRESHOLD:
	case SNMP_MIB_OID_RTS_THRESHOLD:
	case SNMP_MIB_OID_SHORT_RETRY_LIMIT:
	case SNMP_MIB_OID_LONG_RETRY_LIMIT:
		cmd.bufsize = cpu_to_le16(sizeof(u16));
		*((__le16 *)(&cmd.value)) = cpu_to_le16(val);
		break;
	default:
		lbs_deb_cmd("SNMP_CMD: (set) unhandled OID 0x%x\n", oid);
		ret = -EINVAL;
		goto out;
	}

	lbs_deb_cmd("SNMP_CMD: (set) oid 0x%x, oid size 0x%x, value 0x%x\n",
		    le16_to_cpu(cmd.oid), le16_to_cpu(cmd.bufsize), val);

	ret = lbs_cmd_with_response(priv, CMD_802_11_SNMP_MIB, &cmd);

out:
	lbs_deb_cmd_leave_args(LBS_DEB_CMD, ret);
	return ret;
}

int __inline lbs_cmd(struct lbs_private *priv, uint16_t cmdnr, struct void_cmd_head* cmd,
			 int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      		unsigned long callback_arg)
{
	uint16_t __sz = le16_to_cpu((cmd)->hdr.size);		
	//(cmd)->hdr.size = cpu_to_le16(sizeof(*(cmd)));		
	return __lbs_cmd(priv, cmdnr, &(cmd)->hdr, __sz, callback, callback_arg);
}

int __inline lbs_cmd_with_response(struct lbs_private *priv, uint16_t cmdnr,void *cmd)
{
	return lbs_cmd(priv, cmdnr, (struct void_cmd_head*)cmd, lbs_cmd_copyback, (unsigned long) (cmd));
}
