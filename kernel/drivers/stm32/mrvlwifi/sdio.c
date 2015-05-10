
#include "lwip/opt.h"
#include "type.h"
#include "common.h"
#include "string.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sdio.h"

/**********************************sd card ops***********************************/

int sdio_read_func_cis(struct sdio_func *func);
int sdio_read_common_cis(struct mmc_card *card);

int mmc_send_io_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
	struct mmc_command cmd;
	int i, err = 0;

	//BUG_ON(!host);

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = SD_IO_SEND_OP_COND;
	cmd.arg = ocr;
	cmd.flags = MMC_RSP_SPI_R4 | MMC_RSP_R4 | MMC_CMD_BCR;

	for (i = 100; i; i--) {
		err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		/* otherwise wait until reset completes */
		if (mmc_host_is_spi(host)) {
			/*
			 * Both R1_SPI_IDLE and MMC_CARD_BUSY indicate
			 * an initialized card under SPI, but some cards
			 * (Marvell's) only behave when looking at this
			 * one.
			 */
			if (cmd.resp[1] & MMC_CARD_BUSY)
				break;
		} else {
			if (cmd.resp[0] & MMC_CARD_BUSY)
				break;
		}

		err = -ETIMEDOUT;

		mmc_delay(10);
	}

	if (rocr)
		*rocr = cmd.resp[mmc_host_is_spi(host) ? 1 : 0];

	return err;
}



int mmc_send_if_cond(struct mmc_host *host, u32 ocr)
{
	struct mmc_command cmd;
	int err;
	static const u8 test_pattern = 0xAA;
	u8 result_pattern;

	/*
	 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
	 * before SD_APP_OP_COND. This command will harmlessly fail for
	 * SD 1.0 cards.
	 */
	cmd.opcode = SD_SEND_IF_COND;
	cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
	cmd.flags = MMC_RSP_SPI_R7 | MMC_RSP_R7 | MMC_CMD_BCR;

	err = mmc_wait_for_cmd(host, &cmd, 0);
	if (err)
		return err;

	if (mmc_host_is_spi(host))
		result_pattern = cmd.resp[1] & 0xFF;
	else
		result_pattern = cmd.resp[0] & 0xFF;

	if (result_pattern != test_pattern)
		return -EIO;

	return 0;
}
int mmc_send_relative_addr(struct mmc_host *host, unsigned int *rca)
{
	int err;
	struct mmc_command cmd;

	/*BUG_ON(!host);
	BUG_ON(!rca);*/

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = SD_SEND_RELATIVE_ADDR;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R6 | MMC_CMD_BCR;

	err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	*rca = cmd.resp[0] >> 16;

	return 0;
}

int mmc_io_rw_direct(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, u8 in, u8* out)
{
	struct mmc_command cmd;
	int err;
/*	BUG_ON(!card);
	BUG_ON(fn > 7);*/

	/* sanity check */
	if (addr & ~0x1FFFF)
		return -EINVAL;

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = SD_IO_RW_DIRECT;
	cmd.arg = write ? 0x80000000 : 0x00000000;
	cmd.arg |= fn << 28;
	cmd.arg |= (write && out) ? 0x08000000 : 0x00000000;
	cmd.arg |= addr << 9;
	cmd.arg |= in;
	cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;

	if (mmc_host_is_spi(card->host)) {
		/* host driver already reported errors */
	} else {
		if (cmd.resp[0] & R5_ERROR)
			return -EIO;
		if (cmd.resp[0] & R5_FUNCTION_NUMBER)
			return -EINVAL;
		if (cmd.resp[0] & R5_OUT_OF_RANGE)
			return -ERANGE;
	}

	if (out) {
		if (mmc_host_is_spi(card->host))
			*out = (cmd.resp[0] >> 8) & 0xFF;
		else
			*out = cmd.resp[0] & 0xFF;
	}
	//sdio_deb_leave;
	return 0;
}


/****************************************************************************/



static int sdio_read_fbr(struct sdio_func *func)
{
	int ret;
	unsigned char data;

	ret = mmc_io_rw_direct(func->card, 0, 0,
		SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF, 0, &data);
	if (ret)
		goto out;

	data &= 0x0f;

	if (data == 0x0f) {
		ret = mmc_io_rw_direct(func->card, 0, 0,
			SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF_EXT, 0, &data);
		if (ret)
			goto out;
	}

	func->class = data;

out:
	return ret;
}

static int sdio_init_func(struct mmc_card *card, unsigned int fn)
{
	int ret;
	struct sdio_func *func;

	//BUG_ON(fn > SDIO_MAX_FUNCS);

	func = sdio_alloc_func(card);
/*	if (IS_ERR(func))
		return PTR_ERR(func);*/

	func->num = fn;

	ret = sdio_read_fbr(func);
	if (ret)
		goto fail;

	ret = sdio_read_func_cis(func);//读取function的cis
	if (ret)
		goto fail;
	 pr_debug("function vendor=0x%x	device=0x%x\n",func->vendor,func->device);
	card->sdio_func[fn - 1] = func;

	return 0;

fail:
	/*
	 * It is okay to remove the function here even though we hold
	 * the host lock as we haven't registered the device yet.
	 */
	//sdio_remove_func(func);
	return ret;
}
static int sdio_read_cccr(struct mmc_card *card)
{
	int ret;
	int cccr_vsn;
	unsigned char data;

	memset(&card->cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret)
		goto out;

	cccr_vsn = data & 0x0f;

	if (cccr_vsn > SDIO_CCCR_REV_1_20) {
		_hx_printf(KERN_ERR "%s: unrecognised CCCR structure version %d\n",
			mmc_hostname(card->host), cccr_vsn);
		return -EINVAL;
	}

	card->cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret)
		goto out;

	if (data & SDIO_CCCR_CAP_SMB)
		card->cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		card->cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		card->cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_POWER, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_POWER_SMPC)
			card->cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_SPEED_SHS)
			card->cccr.high_speed = 1;
	}

out:
	return ret;
}

/*
 * If desired, disconnect the pull-up resistor on CD/DAT[3] (pin 1)
 * of the card. This may be required on certain setups of boards,
 * controllers and embedded sdio device which do not need the card's
 * pull-up. As a result, card detection is disabled and power is saved.
 */
static int sdio_disable_cd(struct mmc_card *card)
{
	int ret;
	u8 ctrl;

	if (!card->cccr.disable_cd)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret)
		return ret;

	ctrl |= SDIO_BUS_CD_DISABLE;

	return mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_IF, ctrl, NULL);
}



/*
 * Test if the card supports high-speed mode and, if so, switch to it.
 */
static int sdio_enable_hs(struct mmc_card *card)
{
	int ret;
	u8 speed;

	if (!(card->host->caps & MMC_CAP_SD_HIGHSPEED))
		return 0;

	if (!card->cccr.high_speed)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &speed);
	if (ret)
		return ret;

	speed |= SDIO_SPEED_EHS;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_SPEED, speed, NULL);
	if (ret)
		return ret;

	mmc_card_set_highspeed(card);
	mmc_set_timing(card->host, MMC_TIMING_SD_HS);

	return 0;
}



static int sdio_enable_wide(struct mmc_card *card)
{
	int ret;
	u8 ctrl;

	if (!(card->host->caps & MMC_CAP_4_BIT_DATA))
		return 0;

	if (card->cccr.low_speed && !card->cccr.wide_bus)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret)
		return ret;

	ctrl |= SDIO_BUS_WIDTH_4BIT;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_IF, ctrl, NULL);
	if (ret)
		return ret;

	mmc_set_bus_width(card->host, MMC_BUS_WIDTH_4);

	return 0;
}


static int _mmc_select_card(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	struct mmc_command cmd;

	//BUG_ON(!host);

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = MMC_SELECT_CARD;

	if (card) {
		cmd.arg = card->rca << 16;
		cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
	} else {
		cmd.arg = 0;
		cmd.flags = MMC_RSP_NONE | MMC_CMD_AC;
	}

	err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	return 0;
}
int mmc_select_card(struct mmc_card *card)
{
	//BUG_ON(!card);

	return _mmc_select_card(card->host, card);
}


/*
 * Handle the detection and initialisation of a card.
 *
 * In the case of a resume, "oldcard" will contain the card
 * we're trying to reinitialise.
 */
static int mmc_sdio_init_card(struct mmc_host *host, u32 ocr,
			      struct mmc_card *oldcard)
{
	struct mmc_card *card;
	int err;
	/*
	BUG_ON(!host);
	WARN_ON(!host->claimed);*/

	/*
	 * Inform the card of the voltage
	 */
	err = mmc_send_io_op_cond(host, host->ocr, &ocr);
	if (err)
		goto err;

	/*
	 * For SPI, enable CRC as appropriate.
	 */
	/*if (mmc_host_is_spi(host)) {
		err = mmc_spi_set_crc(host, use_spi_crc);
		if (err)
			goto err;
	}*/

	/*
	 * Allocate card structure.
	 */
	card = mmc_alloc_card(host);//静态分配不用判断分配失败
/*	if (IS_ERR(card)) {
		err = PTR_ERR(card);
		goto err;
	}*/

	card->type = MMC_TYPE_SDIO;

	/*
	 * For native busses:  set card RCA and quit open drain mode.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_send_relative_addr(host, &card->rca);
		if (err)
			goto remove;

		mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);
	}

	/*
	 * Select card, as all following commands rely on that.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_select_card(card);
		if (err)
			goto remove;
	}

	/*
	 * Read the common registers.
	 */
	err = sdio_read_cccr(card);
	if (err)
		goto remove;

	/*
	 * Read the common CIS tuples.
	 */
	err = sdio_read_common_cis(card);//很重要的一个函数，读取通用CIS信息，后面wifi会使用相关的内容
	if (err)
		goto remove;
/*
	if (oldcard) {//
		int same = (card->cis.vendor == oldcard->cis.vendor &&
			    card->cis.device == oldcard->cis.device);
		mmc_remove_card(card);
		if (!same) {
			err = -ENOENT;
			goto err;
		}
		card = oldcard;
		return 0;
	}*/

	/*
	 * Switch to high-speed (if supported).
	 */
	err = sdio_enable_hs(card);
	if (err)
		goto remove;

	/*
	 * Change to the card's maximum speed.
	 */
	if (mmc_card_highspeed(card)) {
		/*
		 * The SDIO specification doesn't mention how
		 * the CIS transfer speed register relates to
		 * high-speed, but it seems that 50 MHz is
		 * mandatory.
		 */
		mmc_set_clock(host,50000000);
	} else {
		mmc_set_clock(host, card->cis.max_dtr);
	}

	/*
	 * Switch to wider bus (if supported).
	 */
	err = sdio_enable_wide(card);
	if (err)
		goto remove;

	if (!oldcard)
		host->card = card;
	return 0;

remove:
/*	if (!oldcard)
		mmc_remove_card(card);*/
		_hx_printf("sdio init card failed!\n");

err:
	return err;
}

/*
 * Starting point for SDIO card init.
 */
int mmc_attach_sdio(struct mmc_host *host, u32 ocr)
{
	int err;
	int i, funcs;
	struct mmc_card *card;

	/*BUG_ON(!host);
	WARN_ON(!host->claimed);

	mmc_attach_bus(host, &mmc_sdio_ops);*/

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F) {
		_hx_printf(KERN_WARNING "%s: card claims to support voltages "
		       "below the defined range. These will be ignored.\n",
		       mmc_hostname(host));
		ocr &= ~0x7F;
	}

	host->ocr = mmc_select_voltage(host, ocr);

	/*
	 * Can we support the voltage(s) of the card(s)?
	 */
	if (!host->ocr) {
		err = -EINVAL;
		goto err;
	}
	/*
	 * Detect and init the card.
	 */
	err = mmc_sdio_init_card(host, host->ocr, NULL);
	if (err)
		goto err;
	card = host->card;

	/*
	 * The number of functions on the card is encoded inside
	 * the ocr.
	 */
	card->sdio_funcs = funcs = (ocr & 0x70000000) >> 28;//最多7个功能
	pr_debug("SD support %d function \n",funcs);
	
	/*
	 * If needed, disconnect card detection pull-up resistor.
	 */
	err = sdio_disable_cd(card);
	if (err)
		goto remove;
	
	/*
	 * Initialize (but don't add) all present functions.
	 */
        _hx_printf("Device function count = %d \r\n", funcs);
	for (i = 0;i < funcs;i++) {
		err = sdio_init_func(host->card, i + 1);
		if (err)
			goto remove;
	}
        printk("Device function inital [ok]!\r\n");

	//mmc_release_host(host);

	/*
	 * First add the card to the driver model...
	 */
	err = mmc_add_card(host->card);
	if (err)
		goto remove_added;
	/*
	 * ...then the SDIO functions.
	 */
	for (i = 0;i < funcs;i++) {
		err = sdio_add_func(host->card->sdio_func[i]);
		if (err)
			goto remove_added;
	}

	return 0;


remove_added:
	printk("Remove without lock if the device has been added.\n");
remove:
	printk("And with lock if it hasn't been added.\n");	
err:
	printk(KERN_ERR "%s: error %d whilst initialising SDIO card\n",
		mmc_hostname(host), err);
	return err;
}

static int process_sdio_pending_irqs(struct mmc_card *card)
{
	int i, ret, count;
	unsigned char pending;
	struct sdio_func *func;
	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_INTx, 0, &pending);
	if (ret) {
		_hx_printf(KERN_DEBUG "error %d reading SDIO_CCCR_INTx\n",ret);
		return ret;
	}

	count = 0;
	for (i = 1; i <= 7; i++) {
		if (pending & (1 << i)) {
			func = card->sdio_func[i - 1];
			if (!func) {
				_hx_printf(KERN_WARNING "pending IRQ for "
					"non-existant function\n");
				ret = -EINVAL;
			} else if (func->irq_handler) {
				func->irq_handler(func);
				count++;
			} else {
				_hx_printf(KERN_WARNING "pending IRQ with no handler\n");
				ret = -EINVAL;
			}
		}
	}

	if (count)
		return count;

	return ret;
}


int sdio_irq_thread(void *_host)
{
	struct mmc_host *host = _host;
	//struct sched_param param = { .sched_priority = 1 };
  //unsigned long period, idle_period;
	int ret;

		/*
		 * We claim the host here on drivers behalf for a couple
		 * reasons:
		 *
		 * 1) it is already needed to retrieve the CCCR_INTx;
		 * 2) we want the driver(s) to clear the IRQ condition ASAP;
		 * 3) we need to control the abort condition locally.
		 *
		 * Just like traditional hard IRQ handlers, we expect SDIO
		 * IRQ handlers to be quick and to the point, so that the
		 * holding of the host lock does not cover too much work
		 * that doesn't require that lock to be held.
		 */
	ret = process_sdio_pending_irqs(host->card);
	
		/*
		 * Adaptive polling frequency based on the assumption
		 * that an interrupt will be closely followed by more.
		 * This has a substantial benefit for network devices.
		 */
	/*	if (!(host->caps & MMC_CAP_SDIO_IRQ)) {
			if (ret > 0)
				period /= 2;
			else {
				period++;
				if (period > idle_period)
					period = idle_period;
			}
		}*/
	if (host->caps & MMC_CAP_SDIO_IRQ)
		host->ops->enable_sdio_irq(host, 1);
	/*	if (host->caps & MMC_CAP_SDIO_IRQ)
		host->ops->enable_sdio_irq(host, 0);*/

	pr_debug("%s: IRQ thread exiting with code %d\n",
		 mmc_hostname(host), ret);
	return ret;
}






/**
 *	sdio_readb - read a single byte from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xff
 *	is returned and @err_ret will contain the error code.
 */
u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	//BUG_ON(!func);

	if (err_ret)
		*err_ret = 0;

	ret = mmc_io_rw_direct(func->card, 0, func->num, addr, 0, &val);
	if (ret) {
		if (err_ret)
			*err_ret = ret;
		return 0xFF;
	}

	return val;
}

/**
 *	sdio_writeb - write a single byte to a SDIO function
 *	@func: SDIO function to access
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
void sdio_writeb(struct sdio_func *func, u8 b, unsigned int addr, int *err_ret)
{
	int ret;

	//BUG_ON(!func);
	ret = mmc_io_rw_direct(func->card, 1, func->num, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

/*
 * Calculate the maximum byte mode transfer size
 */
static __inline unsigned int sdio_max_byte_size(struct sdio_func *func)
{
	unsigned mval =min(func->card->host->max_seg_size,
			    func->card->host->max_blk_size);
	mval = min(mval, func->max_blksize);
	return min(mval, 512u); /* maximum size for byte mode */
}

/**
 *	sdio_align_size - pads a transfer size to a more optimal value
 *	@func: SDIO function
 *	@sz: original transfer size
 *
 *	Pads the original data size with a number of extra bytes in
 *	order to avoid controller bugs and/or performance hits
 *	(e.g. some controllers revert to PIO for certain sizes).
 *
 *	If possible, it will also adjust the size so that it can be
 *	handled in just a single request.
 *
 *	Returns the improved size, which might be unmodified.
 */
static u16 align_power2(u16 size)
{
	u16 i=1;
	while((i<size)&&(i<512)){
		i<<=1;
	}
	return i;			
}

unsigned int sdio_align_size(struct sdio_func *func, unsigned int sz)
{
	unsigned int orig_sz;
	unsigned int blk_sz, byte_sz;
	unsigned chunk_sz;
	orig_sz = sz;

	/*
	 * Do a first check with the controller, in case it
	 * wants to increase the size up to a point where it
	 * might need more than one block.
	 */
	sz = ((sz + 3) / 4) * 4;// mmc_align_data_size(func->card, sz);//必须4字节对齐

	/*
	 * If we can still do this with just a byte transfer, then
	 * we're done.
	 */
	if (sz <= sdio_max_byte_size(func)){//如果长度小于一个块长的话，直接返回
		
		//power=align_power2(sz);//找到一个对齐方式
		//return power;
		return sz;
	}

	if (func->card->cccr.multi_block) {
		/*
		 * Check if the transfer is already block aligned
		 */
		if ((sz % func->cur_blksize) == 0)
			return sz;

		/*
		 * Realign it so that it can be done with one request,
		 * and recheck if the controller still likes it.
		 */
		blk_sz = ((sz + func->cur_blksize - 1) /
			func->cur_blksize) * func->cur_blksize;//按当前尺寸对齐
		blk_sz = mmc_align_data_size(func->card, blk_sz);

		/*
		 * This value is only good if it is still just
		 * one request.
		 */
		if ((blk_sz % func->cur_blksize) == 0)
			return blk_sz;

		/*
		 * We failed to do one request, but at least try to
		 * pad the remainder properly.
		 */
	/*	byte_sz = mmc_align_data_size(func->card,
				sz % func->cur_blksize);
		if (byte_sz <= sdio_max_byte_size(func)) {
			blk_sz = sz / func->cur_blksize;
			return blk_sz * func->cur_blksize + byte_sz;
		}*/

	}
	else {
		/*
		 * We need multiple requests, so first check that the
		 * controller can handle the chunk size;
		 */
		chunk_sz = mmc_align_data_size(func->card,
				sdio_max_byte_size(func));
		if (chunk_sz == sdio_max_byte_size(func)) {
			/*
			 * Fix up the size of the remainder (if any)
			 */
			byte_sz = orig_sz % chunk_sz;
			if (byte_sz) {
				byte_sz = mmc_align_data_size(func->card,
						byte_sz);
			}

			return (orig_sz / chunk_sz) * chunk_sz + byte_sz;
		}
	}
	/*
	 * The controller is simply incapable of transferring the size
	 * we want in decent manner, so just return the original size.
	 */
	return orig_sz;
}

int mmc_io_rw_extended(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
	struct mmc_request mrq;
	struct mmc_command cmd;
	struct mmc_data data;
	//struct scatterlist sg;
//	unsigned char *sg;

	/*BUG_ON(!card);
	BUG_ON(fn > 7);
	BUG_ON(blocks == 1 && blksz > 512);
	WARN_ON(blocks == 0);
	WARN_ON(blksz == 0);*/

	/* sanity check */
	if (addr & ~0x1FFFF)
		return -EINVAL;

	memset(&mrq, 0, sizeof(struct mmc_request));
	memset(&cmd, 0, sizeof(struct mmc_command));
	memset(&data, 0, sizeof(struct mmc_data));

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = SD_IO_RW_EXTENDED;
	cmd.arg = write ? 0x80000000 : 0x00000000;
	cmd.arg |= fn << 28;
	cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
	cmd.arg |= addr << 9;
	if (blocks == 1 && blksz <= 512)
		cmd.arg |= (blksz == 512) ? 0 : blksz;	/* byte mode */
	else
		cmd.arg |= 0x08000000 | blocks;		/* block mode */
	cmd.flags = MMC_RSP_SPI_R5 | MMC_RSP_R5 | MMC_CMD_ADTC;

	data.blksz = blksz;
	data.blocks = blocks;
	data.flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
	/*realy save data */
	//data.sg = &sg;
	//data.sg_len = 1;
	//sg_init_one(&sg, buf, blksz * blocks);
	//修改以后的缓存
	data.sg=buf;
	data.sg_len = blksz * blocks;

	mmc_set_data_timeout(&data, card);

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;

	if (mmc_host_is_spi(card->host)) {
		/* host driver already reported errors */
	} else {
		if (cmd.resp[0] & R5_ERROR)
			return -EIO;
		if (cmd.resp[0] & R5_FUNCTION_NUMBER)
			return -EINVAL;
		if (cmd.resp[0] & R5_OUT_OF_RANGE)
			return -ERANGE;
	}

	return 0;
}

#if 0 //这个是发送任意长块数据的函数，上面是针对stm32修改后的数据结果
/* Split an arbitrarily sized data transfer into several
 * IO_RW_EXTENDED commands. */
static int sdio_io_rw_ext_helper(struct sdio_func *func, int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remainder = size;
	unsigned max_blocks;
	unsigned blocks;
	int ret;

	/* Do the bulk of the transfer using block mode (if supported). */
	//这里做点相应的改动，让设备无条件执行以设备对齐的方式写入数据
	//if (func->card->cccr.multi_block && (size > sdio_max_byte_size(func))) {
	if (func->card->cccr.multi_block ) {
		/* Blocks per command is limited by host count, host transfer
		 * size (we only use a single sg entry) and the maximum for
		 * IO_RW_EXTENDED of 511 blocks. */
		max_blocks = min(func->card->host->max_blk_count,
			func->card->host->max_seg_size / func->cur_blksize);
		max_blocks = min(max_blocks, 511);

		//while (remainder > func->cur_blksize) {
		while(remainder){
			blocks =(remainder< func->cur_blksize)?1:(remainder / func->cur_blksize);
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func->cur_blksize;

			ret = mmc_io_rw_extended(func->card, write,
				func->num, addr, incr_addr, buf,
				blocks, func->cur_blksize);
			if (ret)
				return ret;

			remainder -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}

	/* Write the remainder using byte mode. */
	while (remainder > 0) {
	#ifdef MASK_DEBUG
		size = min(remainder, sdio_max_byte_size(func));

		ret = mmc_io_rw_extended(func->card, write, func->num, addr,
			 incr_addr, buf, 1, size);
		if (ret)
			return ret;

		remainder -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	#endif
		printk("some remainder data can't send!\n");
		try_bug(0);
	}
	return 0;
}
#endif

/* Split an arbitrarily sized data transfer into several
 * IO_RW_EXTENDED commands. */
int sdio_io_rw_ext_helper(struct sdio_func *func, int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	u8* alig_bug_buf = NULL;
	unsigned remainder = size;
	u8 *pbuf=buf;
	unsigned max_blocks;
	unsigned blocks;
	int ret;
	
	//Allocate memory for temporary buffer.
	alig_bug_buf = (u8*)KMemAlloc(size,KMEM_SIZE_TYPE_ANY);
	if(NULL == alig_bug_buf)
	{
		_hx_printf("  SDIO->sdio_io_rw_ext_helper: KMemAlloc failed,size = %d.\r\n",size);
		return 0;
	}
	if(write){
		memcpy(alig_bug_buf,buf,size);
		pbuf=alig_bug_buf;
	}
	/* Do the bulk of the transfer using block mode (if supported). */
	if (func->card->cccr.multi_block && (size > sdio_max_byte_size(func))) {
		/* Blocks per command is limited by host count, host transfer
		 * size (we only use a single sg entry) and the maximum for
		 * IO_RW_EXTENDED of 511 blocks. */
		max_blocks = min(func->card->host->max_blk_count,
			func->card->host->max_seg_size / func->cur_blksize);
		max_blocks = min(max_blocks, 511);

		while (remainder > func->cur_blksize) {
			blocks = remainder / func->cur_blksize;
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func->cur_blksize;

			ret = mmc_io_rw_extended(func->card, write,
				func->num, addr, incr_addr, pbuf,
				blocks, func->cur_blksize);
			if (ret)
			{
				KMemFree(alig_bug_buf,KMEM_SIZE_TYPE_ANY,0);
				return ret;
			}

			remainder -= size;
			pbuf += size;
			if (incr_addr)
				addr += size;
		}
	}

	/* Write the remainder using byte mode. */
	while (remainder > 0) {
		u16 tmp;
		size = min(remainder, sdio_max_byte_size(func));
		tmp=align_power2(size);	//Align.
		pr_debug("really size=%d(%d)\n",size,tmp);
		
		ret = mmc_io_rw_extended(func->card, write, func->num, addr,
			 incr_addr,pbuf, 1, tmp);
		if (ret)
		{
			KMemFree(alig_bug_buf,KMEM_SIZE_TYPE_ANY,0);
			return ret;
		}
		remainder -= size;
		pbuf += size;
		if (incr_addr)
			addr += size;
	}
	KMemFree(alig_bug_buf,KMEM_SIZE_TYPE_ANY,0);
	return 0;
}

/**
 *	sdio_readsb - read from a FIFO on a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address of (single byte) FIFO
 *	@count: number of bytes to read
 *
 *	Reads from the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
 #ifdef MASK_DEBUG
int sdio_readsb(struct sdio_func *func, void *dst, unsigned int addr,
	int count)
{
	int ret;
	ret=sdio_io_rw_ext_helper(func, 0, addr, 0, dst, count);
	//debug_data_stream("read sb",dst,count);
	return ret;
}

/**
 *	sdio_writesb - write to a FIFO of a SDIO function
 *	@func: SDIO function to access
 *	@addr: address of (single byte) FIFO
 *	@src: buffer that contains the data to write
 *	@count: number of bytes to write
 *
 *	Writes to the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
int sdio_writesb(struct sdio_func *func, unsigned int addr, void *src,
	int count)
{	
	
	return sdio_io_rw_ext_helper(func, 1, addr, 0, src, count);
}

#endif
