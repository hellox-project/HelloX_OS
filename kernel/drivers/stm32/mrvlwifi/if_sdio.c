
#include "lwip/opt.h"
#include "common.h"
#include "sdio_func.h"
#include "sdio_ids.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sd.h"
#include "sdio.h"
#include "if_sdio.h"
#include "mdef.h"
#include "cmd.h"
#include "marvel_main.h"
#include "dev.h"
#include "assoc.h"
#include "marvell_ops.h"

extern struct lbs_private *pgmarvel_priv;
/*driver support device id tables*/

#ifdef 	MARVELL_8385_DRIVER
extern const unsigned char help_firmware_array[2140];
extern const unsigned char marvel_firmware_array[96716];
#endif

#ifdef 	MARVELL_8686_DRIVER
extern const unsigned char help_firmware_array[2516];
extern const unsigned char marvel_firmware_array[122916];
#endif

const static struct firmware help_firmware={
#ifdef MARVELL_8686_DRIVER
	2516,help_firmware_array
#endif
#ifdef MARVELL_8385_DRIVER
		2140,help_firmware_array
#endif

};

const static struct firmware marvel_firmware={
#ifdef MARVELL_8686_DRIVER
	122916,marvel_firmware_array
#endif
#ifdef MARVELL_8385_DRIVER
	96716,marvel_firmware_array
#endif

	
};


/********************************************************************/
/* I/O                                                              */
/********************************************************************/

/*
 *  For SD8385/SD8686, this function reads firmware status after
 *  the image is downloaded, or reads RX packet length when
 *  interrupt (with IF_SDIO_H_INT_UPLD bit set) is received.
 *  For SD8688, this function reads firmware status only.
 */
static u16 if_sdio_read_scratch(struct if_sdio_card *card, int *err)
{
	int ret;
	u16 scratch;

	scratch = sdio_readb(card->func, card->scratch_reg, &ret);
	if (!ret)
		scratch |= sdio_readb(card->func, card->scratch_reg + 1,
					&ret) << 8;

	if (err)
		*err = ret;

	if (ret)
		return 0xffff;

	return scratch;
}


static u16 if_sdio_read_rx_len(struct if_sdio_card *card, int *err)
{
	int ret;
	u16 rx_len;

	switch (card->model) {
	case IF_SDIO_MODEL_8385:
	case IF_SDIO_MODEL_8686:
		rx_len = if_sdio_read_scratch(card, &ret);//读取scratch reg获取接收数据的长度
		break;
	case IF_SDIO_MODEL_8688:
	default: /* for newer chipsets */
		rx_len = sdio_readb(card->func, IF_SDIO_RX_LEN, &ret);
		if (!ret)
			rx_len <<= card->rx_unit;
		else
			rx_len = 0xffff;	/* invalid length */

		break;
	}

	if (err)
		*err = ret;

	return rx_len;
}

static int if_sdio_handle_cmd(struct if_sdio_card *card,
		u8 *buffer, unsigned size)
{
	struct lbs_private *priv = card->priv;
	int ret;
	u8 i;

	lbs_deb_enter("enter if_sdio_handle_cmd\n");

	if (size > LBS_CMD_BUFFER_SIZE) {
		lbs_deb_sdio("response packet too large (%d bytes)\n",
			(int)size);
		ret = -E2BIG;
		goto out;
	}

	i = priv->resp_idx = 0;
	priv->resp_len[i] = size; //主线程会读取本字段的值来确定是否有响应要处理
	memcpy(priv->resp_buf[i], buffer, size); //copy到响应数组中，这里定义了两个priv->resp_buf
	lbs_thread(priv);
	ret = 0;
out:
	lbs_deb_leave_args("enter if_sdio_handle_cmd(ret=%d)\n", ret);
	return ret;
}

static int if_sdio_handle_data(struct if_sdio_card *card,
		u8 *buffer, unsigned int size)
{
	int ret;
	lbs_deb_enter("enter if_sdio_handle_data\n");
	if (size > MRVDRV_ETH_RX_PACKET_BUFFER_SIZE ) {
		lbs_deb_sdio("response packet too large (%d bytes)\n",
			(int)size);
		ret = -E2BIG;
		goto out;
	}
	lbs_process_rxed_packet(card->priv,(char *)buffer,size);
	ret = 0;
out:
	lbs_deb_leave_args("leave if_sdio_handle_data(ret=%d)\n", ret);
	return ret;
}

#if 0

static int if_sdio_handle_event(struct if_sdio_card *card,
		u8 *buffer, unsigned size)
{
	int ret;
	u32 event;

	lbs_deb_enter(LBS_DEB_SDIO);

	if (card->model == IF_SDIO_MODEL_8385) {
		event = sdio_readb(card->func, IF_SDIO_EVENT, &ret);
		if (ret)
			goto out;

		/* right shift 3 bits to get the event id */
		event >>= 3;
	} else {
		if (size < 4) {
			lbs_deb_sdio("event packet too small (%d bytes)\n",
				(int)size);
			ret = -EINVAL;
			goto out;
		}
		event = buffer[3] << 24;
		event |= buffer[2] << 16;
		event |= buffer[1] << 8;
		event |= buffer[0] << 0;
	}

	lbs_queue_event(card->priv, event & 0xFF);
	ret = 0;

out:
	lbs_deb_leave_args(LBS_DEB_SDIO, ret);

	return ret;
}
#endif


static int if_sdio_card_to_host(struct if_sdio_card *card)
{
	int ret;
	u8 status;
	u16 size, type, chunk;
	unsigned long timeout;
	struct lbs_private	*priv = card->priv;
	//lbs_deb_enter(LBS_DEB_SDIO);
	size = if_sdio_read_rx_len(card, &ret);
	if (ret)
		goto out;

	if (size < 4) {
		pr_debug("invalid packet size (%d bytes) from firmware\n",
			(int)size);
		ret = -EINVAL;
		goto out;
	}
	timeout = HZ;
	while (1) {
		status = sdio_readb(card->func, IF_SDIO_STATUS, &ret);
		if (ret)
			goto out;
		if (status & IF_SDIO_IO_RDY) //Data in card is available,can be fetched by host.
			break;
		if (time_after(jiffies, &timeout)) {
			ret = -ETIMEDOUT;
			goto out;
		}
		mdelay(1);
	}

	/*
	 * The transfer must be in one transaction or the firmware
	 * goes suicidal. There's no way to guarantee that for all
	 * controllers, but we can at least try.
	 */
	chunk = sdio_align_size(card->func, size);
	ret = sdio_readsb(card->func, card->buffer, card->ioport, chunk);
	//ret=sdio_io_rw_ext_helper(card->func, 0, card->ioport, 0, card->buffer, chunk);
	if (ret)
		goto out;

	chunk = card->buffer[0] | (card->buffer[1] << 8); //Data packet's length.
	type  = card->buffer[2] | (card->buffer[3] << 8); //Data pakcet's type.
	//debug_data_stream("card->buffer",card->buffer,chunk);
	lbs_deb_sdio("packet of type %d and size %d bytes\n",
		(int)type, (int)chunk);
	//debug_data_stream("packet",card->buffer,chunk);
	if (chunk > size) {
		lbs_deb_sdio("packet fragment (%d > %d)\n",
			(int)chunk, (int)size);
		ret = -EINVAL;
		goto out;
	}

	if (chunk < size) {
		lbs_deb_sdio("packet fragment (%d < %d)\n",
			(int)chunk, (int)size);
	}

	switch (type) {
	case MVMS_CMD:
		 ret = if_sdio_handle_cmd(card, card->buffer + 4, chunk - 4);  //Command process.
		if (ret)
			goto out; 
		break;
	case MVMS_DAT:
		 ret = if_sdio_handle_data(card, card->buffer + 4, chunk - 4); //Data process.
		if (ret)
			goto out; 
		break;
	/*case MVMS_EVENT:
		//处理网卡的envent，到底有哪些事件，会唤醒主线程处理
		//envent会有个kfifo，向里面填充内容后，主线程会读取
		ret = if_sdio_handle_event(card, card->buffer + 4, chunk - 4);
		if (ret)
			goto out;
		break;*/
	default:
		/*pr_debug("invalid type (%d) from firmware\n",
				(int)type);
		ret = -EINVAL;*/
		priv->connect_status = LBS_DISCONNECTED;
		ret = 0;
		goto out;
	}

out:
	if (ret)
	{
		pr_err("problem fetching packet from firmware\n");
	}
	pr_err("ret %d", ret);
	return ret;
}

static void if_sdio_host_to_card_worker(struct if_sdio_card *card)
{
	struct if_sdio_packet *packet;
	unsigned long timeout;
	u8 status;
	int ret;
	 lbs_deb_cmd_enter(LBS_DEB_SDIO);
	while (1) {
		packet = card->packets;//取出数据包
		if (packet)
			card->packets = packet->next;
		if (!packet)//正常出口，如果数据包发送完成退出
			break;
		timeout = jiffies + HZ;
		while (1) {
			status = sdio_readb(card->func, IF_SDIO_STATUS, &ret);
			if (ret)
				goto release;
			if (status & IF_SDIO_IO_RDY)//读取状态直到IO准备完成，但是有个超时值
				break;
			if (time_after(jiffies, &timeout)) {
				ret = -ETIMEDOUT;
				goto release;
			}
			mdelay(1);
		}
		ret = sdio_writesb(card->func, card->ioport,
				packet->buffer, packet->nb);//将数据写入sd卡
		if (ret)
			goto release;
release:
		pr_debug("sdio_release_host\n");
	}

	 lbs_deb_cmd_leave(LBS_DEB_SDIO);
}


/*******************************************************************/
/* Libertas callbacks       if_sdio_handle_data                                       */
/*******************************************************************/

static int if_sdio_host_to_card(struct lbs_private *priv,
		u8 type, u8 *buf, u16 nb)
{
	int ret;
	struct if_sdio_packet* host_tx_pktbuf;
	struct if_sdio_card *card;
	struct if_sdio_packet *packet, *cur;
	u16 size;

	//Allocate memory for host_tx_pktbuf.
	host_tx_pktbuf = (struct if_sdio_packet*)KMemAlloc(sizeof(struct if_sdio_packet),KMEM_SIZE_TYPE_ANY);
	if(NULL == host_tx_pktbuf)
	{
		_hx_printf("SDIO->if_sdio_host_to_card: Failed to allocate memory for pkt buffer.\r\n");
		ret = -EINVAL;
		goto out;
	}
	lbs_deb_cmd_enter_args("enter if_sdio_host_to_card type %d, bytes %d", type);

	card = priv->card;

	if (nb > (65536 - sizeof(struct if_sdio_packet) - 4)) {
		ret = -EINVAL;
		//KMemFree(host_tx_pktbuf,KMEM_SIZE_TYPE_ANY,0);
		goto out;
	}

	/*
	 * The transfer must be in one transaction or the firmware
	 * goes suicidal. There's no way to guarantee that for all
	 * controllers, but we can at least try.
	 */
	size = sdio_align_size(card->func, nb + 4);//主机优化对齐

	/*packet = kzalloc(sizeof(struct if_sdio_packet) + size,
			GFP_ATOMIC);//分配包空间
	if (!packet) {
		ret = -ENOMEM;
		goto out;
	}*/
	packet=host_tx_pktbuf;
//	memset(packet,0,sizeof(struct if_sdio_packet));

	packet->next = NULL;
	packet->nb = size;

	/*
	 * SDIO specific header.
	 */
	packet->buffer[0] = (nb + 4) & 0xff;
	packet->buffer[1] = ((nb + 4) >> 8) & 0xff;
	packet->buffer[2] = type;
	packet->buffer[3] = 0;//sdio硬件要求的头信息，buffer在这里是个零长数组

	memcpy(packet->buffer + 4, buf, nb);

//	spin_lock_irqsave(&card->lock, flags);

	if (!card->packets)
		card->packets = packet;
	else {
		cur = card->packets;
		while (cur->next)
			cur = cur->next;
		cur->next = packet;//链接到尾部
	}

	switch (type) {
	case MVMS_CMD:
		priv->dnld_sent = DNLD_CMD_SENT;//更新状态，以免出现主线程重复提交的问题
		break;
	case MVMS_DAT:
		priv->dnld_sent = DNLD_DATA_SENT;
		break;
	default:
		lbs_deb_sdio("unknown packet type %d\n", (int)type);
	}
	if_sdio_host_to_card_worker(card);
	ret = 0;

out:
	lbs_deb_cmd_leave_args("leave if_sdio_host_to_card(ret=%d)\n", ret);
	KMemFree(host_tx_pktbuf,KMEM_SIZE_TYPE_ANY,0);
	return ret;
}


int if_sdio_send_data(struct lbs_private *priv, u8 *buf, u16 nb)
{
	int ret;
	unsigned long timeout;
	u8 status;
	struct if_sdio_card *card;
	u16 size;
	card = priv->card;
	size = sdio_align_size(card->func, nb + 4);//主机优化对齐
	buf[0] = (nb + 4) & 0xff;
	buf[1] = ((nb + 4) >> 8) & 0xff;
	buf[2] = MVMS_DAT;
	buf[3] = 0;//sdio硬件要求的头信息，buffer在这里是个零长数组
	timeout = jiffies + HZ;
	while (1) {
		status = sdio_readb(card->func, IF_SDIO_STATUS, &ret);
		if (ret)
			goto out;
		if (status & IF_SDIO_IO_RDY)//读取状态直到IO准备完成，但是有个超时值
			break;
		if (time_after(jiffies, &timeout)) {
			ret = -ETIMEDOUT;
			goto out;
		}
		mdelay(1);
	}
	ret = sdio_writesb(card->func, card->ioport,
			buf, size);//将数据写入sd卡
	if (ret)
		goto out;
	 lbs_deb_cmd_leave(LBS_DEB_SDIO);
	ret = 0;
out:
	lbs_deb_cmd_leave_args("leave if_sdio_host_to_card(ret=%d)\n", ret);
	return ret;
}

/*******************************************************************/
/* SDIO callbacks                                                  */
/*******************************************************************/

void if_sdio_interrupt(struct sdio_func *func)
{
	int ret;
	struct if_sdio_card *card;
	u8 cause;

	//pr_sdio_interrupt("enter marvel sdio interrupt process!\n");
	card = sdio_get_drvdata(func);

	cause = sdio_readb(card->func, IF_SDIO_H_INT_STATUS, &ret);
	if (ret) //If the interrupt is not raised by this function,just ignore it.
	{
		goto out;
	}

	pr_sdio_interrupt("interrupt: 0x%X\n", (unsigned)cause);

	sdio_writeb(card->func, ~cause, IF_SDIO_H_INT_STATUS, &ret); //Clear interrupt bit.
	if (ret)
	{
		goto out;
	}

	/*
	 * Ignore the define name, this really means the card has
	 * successfully received the command.
	 */
	if (cause & IF_SDIO_H_INT_DNLD)  //Data download interrupt.
	{
		lbs_host_to_card_done(pgmarvel_priv);
	}

	if (cause & IF_SDIO_H_INT_UPLD) { //Data available in card,transfer to host.
		ret = if_sdio_card_to_host(card);
		if (ret)
		{
			goto out;
		}
	}

	ret = 0;

out:
	//pr_sdio_interrupt("leave sdio interrupt ret %d\n", ret);
	lbs_deb_cmd_leave("123");
}

/********************************************************************/
/* Firmware                                                         */
/********************************************************************/

static int if_sdio_prog_helper(struct if_sdio_card *card)
{
	int ret;
	u8 status;
	u8 tmp_buffer[64];
	const struct firmware *fw=card->helper;
	unsigned long timeout;
	u8 *chunk_buffer=tmp_buffer;
	u32 chunk_size;
	const u8 *firmware;
	u32 size;
	printk("enter if_sdio_prog_helper\n");

	memset(chunk_buffer,0,64);
	ret = sdio_set_block_size(card->func, 32);
	if (ret)
		goto release;
	firmware = fw->data;//获取的固件数据
	size = fw->size;//获取的固件数据长度
	while (size) {
		timeout = HZ;
		while (1) {//读取SDIO状态,直到准备好
			status = sdio_readb(card->func,IF_SDIO_STATUS, &ret);
			if (ret)
				goto release;
			if ((status & IF_SDIO_IO_RDY) &&
					(status & IF_SDIO_DL_RDY))
				break;
			if (time_after(jiffies,&timeout)) {
				ret = -ETIMEDOUT;
				goto release;
			}
			mdelay(1);
		}

		chunk_size = min(size, (size_t)60);
		*((__le32*)chunk_buffer) = cpu_to_le32(chunk_size);//固件长度占4byte
		memcpy(chunk_buffer + 4, firmware, chunk_size);//固件信息

		pr_debug("sending %d bytes chunk\n", chunk_size);
		ret = sdio_writesb(card->func, card->ioport,
				chunk_buffer, 64);
		if (ret)
			goto release;
		firmware += chunk_size;
		size -= chunk_size;
	}
	/* an empty block marks the end of the transfer */
	memset(chunk_buffer, 0, 4);
	ret = sdio_writesb(card->func, card->ioport, chunk_buffer, 64);
	if (ret)
		goto release;

	lbs_deb_sdio("waiting for helper to boot...\n");

	/* wait for the helper to boot by looking at the size register */
	timeout = jiffies + HZ;
	while (1) {
		u16 req_size;
		req_size = sdio_readb(card->func, IF_SDIO_RD_BASE, &ret);
		if (ret)
			goto release;

		req_size |= sdio_readb(card->func, IF_SDIO_RD_BASE + 1, &ret) << 8;
		if (ret)
		{
			goto release;
		}

		if (req_size != 0){
			pr_debug("down help size=%d\n",req_size);
			break;
			}

		if (time_after(jiffies, &timeout)) {
			ret = -ETIMEDOUT;
			goto release;
		}

		msleep(10);
	}

	ret = 0;
release:
	pr_debug("sdio_release_host(card->func)\n");
	pr_debug("release_firmware(fw)\n");
	if (ret)
		lbs_pr_err("failed to load helper firmware\n");

	lbs_deb_leave_args("leave if_sdio_prog_helper(ret=%d).\n", ret);

	return ret;
}


static int if_sdio_prog_real(struct if_sdio_card *card)
{
	int ret;
	u8 status;
	u8 tmp_buffer[512];
	const struct firmware *fw=card->firmware;
	unsigned long timeout;
	u8 *chunk_buffer;
	u32 chunk_size;
	const u8 *firmware;
	size_t size, req_size;
	u16 scratch;
	chunk_buffer=tmp_buffer;
	memset(chunk_buffer,0,512);
	ret = sdio_set_block_size(card->func, 32);
	if (ret)
		goto release;

	firmware = fw->data;
	size = fw->size;

	printk("firmware size=%d\n",size);
	
	while (size) {
		timeout = jiffies + HZ;
		while (1) {
			status = sdio_readb(card->func, IF_SDIO_STATUS, &ret);
			if (ret)
				goto release;
			if ((status & IF_SDIO_IO_RDY) &&
					(status & IF_SDIO_DL_RDY))
				break;
			if (time_after(jiffies, &timeout)) {
				ret = -ETIMEDOUT;
				goto release;
			}
			mdelay(1);
		}

		req_size = sdio_readb(card->func, IF_SDIO_RD_BASE, &ret);
		if (ret)
			goto release;

		req_size |= sdio_readb(card->func, IF_SDIO_RD_BASE + 1, &ret) << 8;
		if (ret)
			goto release;

		pr_debug("firmware wants %d bytes\n", (int)req_size);
		if (req_size == 0) {
			lbs_deb_sdio("firmware helper gave up early\n");
			ret = -EIO;
			goto release;
		}

		if (req_size & 0x01) {
			lbs_deb_sdio("firmware helper signalled error\n");
			ret = -EIO;
			goto release;
		}

		if (req_size > size)
			req_size = size;

		while (req_size) {
			chunk_size = min(req_size, (size_t)512);

			memcpy(chunk_buffer, firmware, chunk_size);
			pr_debug("sending %d bytes (%d bytes) chunk\n",
				chunk_size, (chunk_size + 31) / 32 * 32);//必须32字节对齐
			ret = sdio_writesb(card->func, card->ioport,
				chunk_buffer, roundup(chunk_size, 32));
			if (ret)
				goto release;

			firmware += chunk_size;
			size -= chunk_size;
			req_size -= chunk_size;
		}
	}
	ret = 0;
	lbs_deb_sdio("waiting for firmware to boot...\n");
	/* wait for the firmware to boot */
 	timeout = jiffies + HZ;
	while (1) {
		
		scratch = if_sdio_read_scratch(card, &ret);
		if (ret)
			goto release;

		if (scratch == IF_SDIO_FIRMWARE_OK){
			printk("program firmware success\n");
			break;
		}

		if (time_after(jiffies, &timeout)) {
			ret = -ETIMEDOUT;
			goto release;
		}

		msleep(10);
	} 
	ret = 0;

release:
	pr_debug("sdio_release_host(card->func)\n");
	pr_debug("release_firmware(fw)\n");
	if (ret)
		lbs_pr_err("failed to load firmware\n");
	lbs_deb_leave_args("leave if_sdio_prog_real(ret=%d)\n", ret);
	return ret;
}

static int if_sdio_prog_firmware(struct if_sdio_card *card)
{
	int ret;
	u16 scratch;
	sdio_sys_wait=0;
	scratch = if_sdio_read_scratch(card, &ret);//读固件版本状态
	if (ret)
		goto out;

	lbs_deb_sdio("firmware status = %#x\n", scratch);

	if (scratch == IF_SDIO_FIRMWARE_OK) {
		lbs_deb_sdio("firmware already loaded\n");
		goto success;
	}

	ret = if_sdio_prog_helper(card);//下载bootload
	if (ret){
		pr_err("download help firmware failed!\n");
		goto out;
	}
	ret = if_sdio_prog_real(card);//下载固件
	if (ret){
		pr_err("download marvel firmware failed!\n");
		goto out;
	}
success:
	 sdio_set_block_size(card->func, IF_SDIO_BLOCK_SIZE);
	ret = 0;

out:
	sdio_sys_wait=1;
	return ret;
}




static struct if_sdio_card gmarvel_ifsdio_card;
struct lbs_private *if_sdio_probe(struct sdio_func *func,struct sdio_device_id *id)//枚举网卡
{
	struct kfifo gmarvel_event_fifo={0,0};
	struct if_sdio_card *card;
 	struct lbs_private *priv;
	int ret=0, i;
	unsigned int model;
	u16 scratch;
	for (i = 0;i < func->card->num_info;i++) {
		pr_debug("card infomation string(%d):%s \n",i,func->card->info[i]);
		if (strcmp(func->card->info[i],"ID: 04") ==0){//这里指针对本网卡进行这种判读
			model = IF_SDIO_MODEL_8385;
			break; 
		}
		else if(strcmp(func->card->info[i],"802.11 SDIO ID: 0B") ==0){
			model = IF_SDIO_MODEL_8686;
			break; 
		}
	}

	if (i == func->card->num_info) {
		pr_err("unable to identify card model\n");
		return (struct lbs_private *)0;
	}
	card = &gmarvel_ifsdio_card;
	memset(card,0,sizeof(struct if_sdio_card));
	card->func = func;
	card->model = model;
	
	switch (card->model) {
	case IF_SDIO_MODEL_8385:
		card->scratch_reg = IF_SDIO_SCRATCH_OLD;//sdio wifi给出的一个状态寄存器接口
		break;
	case IF_SDIO_MODEL_8686:
		card->scratch_reg = IF_SDIO_SCRATCH;
		break;
	case IF_SDIO_MODEL_8688:
	default: /* for newer chipsets */
		card->scratch_reg = IF_SDIO_FW_STATUS;
		break;
	}
	card->helper=&help_firmware;
	card->firmware=&marvel_firmware;
	
	ret = sdio_enable_func(func);//使能sdio设备工作
	if (ret)
		goto release;

	ret = sdio_claim_irq(func, if_sdio_interrupt); //Claim function's interrupt.
	if (ret)
		goto disable;

	card->ioport = sdio_readb(func, IF_SDIO_IOPORT, &ret);//读写数据包的地址，有点类似以太网卡的dma地址reg10
	if (ret)
		goto release_int;

	card->ioport |= sdio_readb(func, IF_SDIO_IOPORT + 1, &ret) << 8;
	if (ret)
		goto release_int;

	card->ioport |= sdio_readb(func, IF_SDIO_IOPORT + 2, &ret) << 16;
	if (ret)
		goto release_int;
	sdio_set_drvdata(func, card);
	pr_debug("class = 0x%X, vendor = 0x%X, "
			"device = 0x%X, model = 0x%X, ioport = 0x%X\n",
			func->class, func->vendor, func->device,
			model, (unsigned)card->ioport);

	ret = if_sdio_prog_firmware(card);//通过sdio下载固件
	if (ret)
		goto reclaim;
	 priv = lbs_add_card(card);//分配net_device结构，并初始化priv的相关软件内容，按装主服务线程
	if (!priv) {
		ret = -ENOMEM;
		goto reclaim;
	}
 	pgmarvel_priv=priv;
	card->priv = priv;
	priv->event_fifo=&gmarvel_event_fifo;
	priv->card = card;
	priv->hw_host_to_card = if_sdio_host_to_card;//封装数据包，启动card->packet_worker工作队列向硬件发送数据
	priv->fw_ready = 1;//标识固件准备好
	card->rx_unit = 0;
	/** Enable interrupts now that everything is set up*/
	/*下面这段是遇到问题的调试代码*/
	lbs_deb_sdio("read firmware verison!\n");
	scratch = if_sdio_read_scratch(card, &ret);//读固件版本状态
	if (ret)
		goto out;
	lbs_deb_sdio("firmware status = %#x\n", scratch);
	if (scratch == IF_SDIO_FIRMWARE_OK) 
		lbs_deb_sdio("firmware already loaded\n");
	 lbs_pr_err("enable interrupt!\n");
	
 	sdio_writeb(func, 0x0f, IF_SDIO_H_INT_MASK, &ret);//使能网卡芯片sdio中断
	 if (ret)
	 	goto reclaim;
	lbs_deb_sdio("read interrupt mask reg=0x%x\n",sdio_readb(func,IF_SDIO_H_INT_MASK,&ret));
	ret = lbs_start_card(priv);//注册网络设备，初始化相关参数
	if (ret)
		goto err_activate_card;

out:
	pr_debug("probe card return(ret=%d)\n",ret);
	printk("wait for scan...\n");
	mdelay(10);
	if(ret)
		return (struct lbs_private *)0;
	else
		return (struct lbs_private *)(priv);//修改后的程序会返回一个私有变量给main

err_activate_card:
reclaim:
release_int:
disable:
release:
	goto out;
}

u16 wireless_card_rx(u8 *uiprxbuf)
{
	struct lbs_private *priv=pgmarvel_priv;
	struct if_sdio_card *card=priv->card;
	struct eth_packet *rx_pkt=&priv->rx_pkt;
	int ret;
	memset(rx_pkt,0,sizeof(struct eth_packet ));//清零以判断数据接收正常
	ret = poll_sdio_interrupt(card->func);
	if(ret<0){
			lbs_pr_err("read interrupt error!\n");
			try_bug(0);
	}
	else if(ret&(IF_SDIO_H_INT_UPLD|IF_SDIO_H_INT_DNLD)){//先判断数据
		if_sdio_interrupt(card->func);
		if(rx_pkt->len){//接收到数据
			debug_data_stream("Wireless data",rx_pkt->data,rx_pkt->len);
		//	memcpy((void *)uiprxbuf,(void *)rx_pkt->data,rx_pkt->len);
		//	memcpy((void *)uip_buf,(void *)rx_pkt->data,rx_pkt->len);
			return rx_pkt->len;
		}
			
	}
	else
		return 0;
	return 0;
}

int poll_sdio_interrupt(struct sdio_func *func)
{
	int ret;
	u8 cause;
	cause = sdio_readb(func, IF_SDIO_H_INT_STATUS, &ret);
	if (ret)
		return -EIO;
	return cause;
}

void wireless_card_tx(u8 *uiptxbuf,u16 len)
{	
	struct lbs_private *priv=pgmarvel_priv;
	struct eth_packet txpkt;
	txpkt.len=len;
	lbs_hard_start_xmit(priv,&txpkt);
}
