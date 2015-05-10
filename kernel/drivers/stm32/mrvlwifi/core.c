#include "type.h"
#include "common.h"
#include "string.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sd.h"
#include "sdio.h"
#include "sdio_func.h"
#include "sdio_ids.h"
#include "if_sdio.h"

static int ffs(int x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

static  int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}





/**
 *	mmc_align_data_size - pads a transfer size to a more optimal value
 *	@card: the MMC card associated with the data transfer
 *	@sz: original transfer size
 *
 *	Pads the original data size with a number of extra bytes in
 *	order to avoid controller bugs and/or performance hits
 *	(e.g. some controllers revert to PIO for certain sizes).
 *
 *	Returns the improved size, which might be unmodified.
 *
 *	Note that this function is only relevant when issuing a
 *	single scatter gather entry.
 */
unsigned int mmc_align_data_size(struct mmc_card *card, unsigned int sz)
{
    /*
	 * FIXME: We don't have a system for the controller to tell
	 * the core about its problems yet, so for now we just 32-bit
	 * align the size.
	 */
    sz = ((sz + 3) / 4) * 4;

    return sz;
}



























/*
 * Internal function that does the actual ios call to the host driver,
 * optionally printing some debug output.
 */
static void mmc_set_ios(struct mmc_host *host)
{
    struct mmc_ios *ios = &host->ios;

    pr_debug("%s: clock %uHz busmode %u powermode %u cs %u Vdd %u "
             "width %u timing %u\n",
             mmc_hostname(host), ios->clock, ios->bus_mode,
             ios->power_mode, ios->chip_select, ios->vdd,
             ios->bus_width, ios->timing);

    host->ops->set_ios(host, ios);
}

/**
 *	mmc_request_done - finish processing an MMC request
 *	@host: MMC host which completed request
 *	@mrq: MMC request which request
 *
 *	MMC drivers should call this function when they have completed
 *	their processing of a request.
 */
void mmc_request_done(struct mmc_host *host, struct mmc_request *mrq)
{
    struct mmc_command *cmd = mrq->cmd;
    int err = cmd->error;

    if (err && cmd->retries && mmc_host_is_spi(host)) {
        if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
            cmd->retries = 0;
    }

    if (err && cmd->retries) {//重试
        _hx_printf("%s: req failed (CMD%u): %d, retrying...\n",
               mmc_hostname(host), cmd->opcode, err);

        cmd->retries--;
        cmd->error = 0;
        host->ops->request(host, mrq);
    } else {
        //led_trigger_event(host->led, LED_OFF);
        pr_debug("%s: req done (CMD%u): %d: %08x %08x %08x %08x\n",
                 mmc_hostname(host), cmd->opcode, err,
                 cmd->resp[0], cmd->resp[1],
                 cmd->resp[2], cmd->resp[3]);

        if (mrq->data) {
            pr_debug("%s:     %d bytes transferred: %d\n",
                     mmc_hostname(host),
                     mrq->data->bytes_xfered, mrq->data->error);
        }

        if (mrq->stop) {
            pr_debug("%s:     (CMD%u): %d: %08x %08x %08x %08x\n",
                     mmc_hostname(host), mrq->stop->opcode,
                     mrq->stop->error,
                     mrq->stop->resp[0], mrq->stop->resp[1],
                     mrq->stop->resp[2], mrq->stop->resp[3]);
        }

        if (mrq->done)
            mrq->done(mrq->done_data);
    }
}

/*
 * Control chip select pin on a host.
 */
void mmc_set_chip_select(struct mmc_host *host, int mode)
{
    host->ios.chip_select = mode;
    mmc_set_ios(host);
}

/*
 * Sets the host clock to the highest possible frequency that
 * is below "hz".
 */
void mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
    //	WARN_ON(hz < host->f_min);

    if (hz > host->f_max)
        hz = host->f_max;

    host->ios.clock = hz;
    mmc_set_ios(host);
}

/*
 * Change the bus mode (open drain/push-pull) of a host.
 */
void mmc_set_bus_mode(struct mmc_host *host, unsigned int mode)
{
    host->ios.bus_mode = mode;
    mmc_set_ios(host);
}

/*
 * Change data bus width of a host.
 */
void mmc_set_bus_width(struct mmc_host *host, unsigned int width)
{
    host->ios.bus_width = width;
    mmc_set_ios(host);
}



/**
 *	mmc_set_data_timeout - set the timeout for a data command
 *	@data: data phase for command
 *	@card: the MMC card associated with the data transfer
 *
 *	Computes the data timeout parameters according to the
 *	correct algorithm given the card type.
 */
void mmc_set_data_timeout(struct mmc_data *data, const struct mmc_card *card)
{
    unsigned int mult;

    /*
	 * SDIO cards only define an upper 1 s limit on access.
	 */
    if (mmc_card_sdio(card)) {
        data->timeout_ns = 1000000000;
        data->timeout_clks = 0;
        return;
    }

    /*
	 * SD cards use a 100 multiplier rather than 10
	 */
    mult = mmc_card_sd(card) ? 100 : 10;

    /*
	 * Scale up the multiplier (and therefore the timeout) by
	 * the r2w factor for writes.
	 */
    if (data->flags & MMC_DATA_WRITE)
        mult <<= card->csd.r2w_factor;

    data->timeout_ns = card->csd.tacc_ns * mult;
    data->timeout_clks = card->csd.tacc_clks * mult;

    /*
	 * SD cards also have an upper limit on the timeout.
	 */
    if (mmc_card_sd(card)) {
        unsigned int timeout_us, limit_us;

        timeout_us = data->timeout_ns / 1000;
        timeout_us += data->timeout_clks * 1000 /
                      (card->host->ios.clock / 1000);

        if (data->flags & MMC_DATA_WRITE)
            /*
			 * The limit is really 250 ms, but that is
			 * insufficient for some crappy cards.
			 */
            limit_us = 300000;
        else
            limit_us = 100000;

        /*
		 * SDHC cards always use these fixed values.
		 */
        if (timeout_us > limit_us || mmc_card_blockaddr(card)) {
            data->timeout_ns = limit_us * 1000;
            data->timeout_clks = 0;
        }
    }
    /*
	 * Some cards need very high timeouts if driven in SPI mode.
	 * The worst observed timeout was 900ms after writing a
	 * continuous stream of data until the internal logic
	 * overflowed.
	 */
    if (mmc_host_is_spi(card->host)) {
        if (data->flags & MMC_DATA_WRITE) {
            if (data->timeout_ns < 1000000000)
                data->timeout_ns = 1000000000;	/* 1s */
        } else {
            if (data->timeout_ns < 100000000)
                data->timeout_ns =  100000000;	/* 100ms */
        }
    }
}

static void mmc_start_request(struct mmc_host *host, struct mmc_request *mrq)
{
#ifdef CONFIG_MMC_DEBUG
    unsigned int i, sz;
    struct scatterlist *sg;
#endif
    //sdio_deb_enter;
    pr_debug("%s: starting CMD%u arg %08x flags %08x\n",
             mmc_hostname(host), mrq->cmd->opcode,
             mrq->cmd->arg, mrq->cmd->flags);

    if (mrq->data) {
        pr_debug("%s:     blksz %d blocks %d flags %08x "
                 "tsac %d ms nsac %d\n",
                 mmc_hostname(host), mrq->data->blksz,
                 mrq->data->blocks, mrq->data->flags,
                 mrq->data->timeout_ns / 1000000,
                 mrq->data->timeout_clks);
    }

    if (mrq->stop) {
        pr_debug("%s:     CMD%u arg %08x flags %08x\n",
                 mmc_hostname(host), mrq->stop->opcode,
                 mrq->stop->arg, mrq->stop->flags);
    }

    //	WARN_ON(!host->claimed);
    //led_trigger_event(host->led, LED_FULL);

    mrq->cmd->error = 0;
    mrq->cmd->mrq = mrq;
    if (mrq->data) {
        /*BUG_ON(mrq->data->blksz > host->max_blk_size);
		BUG_ON(mrq->data->blocks > host->max_blk_count);
		BUG_ON(mrq->data->blocks * mrq->data->blksz >
			host->max_req_size);

#ifdef CONFIG_MMC_DEBUG
		sz = 0;
		for_each_sg(mrq->data->sg, sg, mrq->data->sg_len, i)
			sz += sg->length;
		BUG_ON(sz != mrq->data->blocks * mrq->data->blksz);
#endif*/

        mrq->cmd->data = mrq->data;
        mrq->data->error = 0;
        mrq->data->mrq = mrq;
        if (mrq->stop) {
            mrq->data->stop = mrq->stop;
            mrq->stop->error = 0;
            mrq->stop->mrq = mrq;
        }
    }
    host->ops->request(host, mrq);
    //sdio_deb_leave;
}

//Should change to comply the sychronization mechenism of HelloX,a event may
//used here to notify the completion.
//Change point when migrate to HelloX.
static void mmc_wait_done(void *data)
{
    unsigned char *p = data;
    *p = 1;
}

/**
 *	mmc_wait_for_req - start a request and wait for completion
 *	@host: MMC host to start command
 *	@mrq: MMC request to start
 *
 *	Start a new MMC custom command request for a host, and wait
 *	for the command to complete. Does not attempt to parse the
 *	response.
 */
void mmc_wait_for_req(struct mmc_host *host, struct mmc_request *mrq)
{
	//DECLARE_COMPLETION_ONSTACK(complete);
	unsigned char complete=0;

	mrq->done_data = (void *)&complete;
	mrq->done = mmc_wait_done;
	mmc_start_request(host, mrq);
	
#ifdef __HX_SDIO_DEBUG
	_hx_printf("SDIO dbg: mmc_wait_for_req,begin to wait complete...\r\n");
#endif
	
	while(!complete);  //Wait the MMC request to end.
}

/**
 *	mmc_wait_for_cmd - start a command and wait for completion
 *	@host: MMC host to start command
 *	@cmd: MMC command to start
 *	@retries: maximum number of retries
 *
 *	Start a new MMC command for a host, and wait for the command
 *	to complete.  Return any error that occurred while the command
 *	was executing.  Do not attempt to parse the response.
 */
int mmc_wait_for_cmd(struct mmc_host *host, struct mmc_command *cmd, int retries)
{
    struct mmc_request mrq;

    //WARN_ON(!host->claimed);
    //sdio_deb_enter;
    memset(&mrq, 0, sizeof(struct mmc_request));

    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    mrq.cmd = cmd;
    cmd->data = NULL;

    mmc_wait_for_req(host, &mrq);
    //pr_debug("wait_for_req out!\n");
    // sdio_deb_leave;
    return cmd->error;
}
/*
 * Select timing parameters for host.
 */
void mmc_set_timing(struct mmc_host *host, unsigned int timing)
{
    host->ios.timing = timing;
    mmc_set_ios(host);
}

/*
 * Apply power to the MMC stack.  This is a two-stage process.
 * First, we enable power to the card without the clock running.
 * We then wait a bit for the power to stabilise.  Finally,
 * enable the bus drivers and clock to the card.
 *
 * We must _NOT_ enable the clock prior to power stablising.
 *
 * If a host does all the power sequencing itself, ignore the
 * initial MMC_POWER_UP stage.
 */
static void mmc_power_up(struct mmc_host *host)
{
    int bit;

    /* If ocr is set, we use it */
    if (host->ocr)
        bit = ffs(host->ocr) - 1;
    else
        bit = fls(host->ocr_avail) - 1;

    host->ios.vdd = bit;
    //if (mmc_host_is_spi(host)) {
    if(0){
        host->ios.chip_select = MMC_CS_HIGH;
        host->ios.bus_mode = MMC_BUSMODE_PUSHPULL;
    } else {
        host->ios.chip_select = MMC_CS_DONTCARE;
        host->ios.bus_mode = MMC_BUSMODE_OPENDRAIN;
    }
    host->ios.power_mode = MMC_POWER_UP;
    host->ios.bus_width = MMC_BUS_WIDTH_1;
    host->ios.timing = MMC_TIMING_LEGACY;
    //mmc_set_ios(host);

    /*
	 * This delay should be sufficient to allow the power supply
	 * to reach the minimum voltage.
	 */
    mmc_delay(1);
    if (host->f_min > 400000) {
        pr_warning("%s: Minimum clock frequency too high for "
                   "identification mode\n", mmc_hostname(host));
        host->ios.clock = host->f_min;
    } else
        host->ios.clock = 400000;

    host->ios.power_mode = MMC_POWER_ON;
    mmc_set_ios(host);

    /*
	 * This delay must be at least 74 clock sizes, or 1 ms, or the
	 * time required to reach a stable voltage.
	 */
    mmc_delay(1);//1ms

    //	pr_debug("mmc power up ok!current realy clock=%dk(SDICON=%d)\n",50000/(rSDIPRE+1),rSDICON);
}

int mmc_go_idle(struct mmc_host *host)
{
    int err;
    struct mmc_command cmd;

    /*
	 * Non-SPI hosts need to prevent chipselect going active during
	 * GO_IDLE; that would put chips into SPI mode.  Remind them of
	 * that in case of hardware that won't pull up DAT3/nCS otherwise.
	 *
	 * SPI hosts ignore ios.chip_select; it's managed according to
	 * rules that must accomodate non-MMC slaves which this layer
	 * won't even know about.
	 */
    if (!mmc_host_is_spi(host)) {
        mmc_set_chip_select(host, MMC_CS_HIGH);
        mmc_delay(1);
    }

    memset(&cmd, 0, sizeof(struct mmc_command));

    cmd.opcode = MMC_GO_IDLE_STATE;
    cmd.arg = 0;
    cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_NONE | MMC_CMD_BC;

    err = mmc_wait_for_cmd(host, &cmd, 0);

    mmc_delay(10);

    if (!mmc_host_is_spi(host)) {
        mmc_set_chip_select(host, MMC_CS_DONTCARE);
        mmc_delay(1);
    }

    host->use_spi_crc = 0;

    return err;
}


/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
    int bit;

    ocr &= host->ocr_avail;

    bit = ffs(ocr);
    if (bit) {
        bit -= 1;

        ocr &= 3 << bit;

        host->ios.vdd = bit;
        pr_debug("support vdd=%d",host->ios.vdd);
        mmc_set_ios(host);
    } else {
        pr_warning("%s: host doesn't support card's voltages\n",
                   mmc_hostname(host));
        ocr = 0;
    }

    return ocr;
}


void  mmc_rescan(struct mmc_host *host)
{
    u32 ocr;
    int err;
    mmc_power_up(host);
    mmc_go_idle(host);
    err=mmc_send_if_cond(host, host->ocr_avail);
    if(err)
        pr_debug("mmc card support SD 1.0 cards !\n");
    else
        pr_debug("mmc card support SD 2.0 cards !\n");
    /*
	 * First we search for SDIO...
	 */
    err = mmc_send_io_op_cond(host, 0, &ocr);
    if (!err) {

        pr_debug("IO op cond=0x%x\n",ocr);
        if (mmc_attach_sdio(host, ocr))
            //mmc_power_off(host);
            _hx_printf("power off sdio.\n");
        else
            printk("SD card probe [ok]!\n");
    }

}

/*sdio_add_func
 * Allocate and initialise a new MMC card structure.
 */
struct mmc_card *mmc_alloc_card(struct mmc_host *host)
{
    static struct mmc_card marvel_gcard;//静态变量，代替malloc
    struct mmc_card *card=&marvel_gcard;
    /*card = kzalloc(sizeof(struct mmc_card), GFP_KERNEL);
	if (!card)
		return ERR_PTR(-ENOMEM);*/
    memset(card,0,sizeof(struct mmc_card));
    card->host = host;
    host->card=card;//this is a bug

    /*	device_initialize(&card->dev);

	card->dev.parent = mmc_classdev(host);
	card->dev.bus = &mmc_bus_type;
	card->dev.release = mmc_release_card;
	card->dev.type = type;*/

    return card;
}

/*
 * Register a new MMC card with the driver model.
 */
int mmc_add_card(struct mmc_card *card)
{
    const char *type;

    //dev_set_name(&card->dev, "%s:%04x", mmc_hostname(card->host), card->rca);

    switch (card->type) {
    case MMC_TYPE_MMC:
        type = "MMC";
        break;
    case MMC_TYPE_SD:
        type = "SD";
        if (mmc_card_blockaddr(card))
            type = "SDHC";
        break;
    case MMC_TYPE_SDIO:
        type = "SDIO";
        break;
    default:
        type = "?";
        break;
    }

    if (mmc_host_is_spi(card->host)) {
        _hx_printf(KERN_INFO "%s: new %s%s card on SPI\n",
               mmc_hostname(card->host),
               mmc_card_highspeed(card) ? "high speed " : "",
               type);
    } else {
        _hx_printf(KERN_INFO "%s: new %s%s card at address %04x\n",
               mmc_hostname(card->host),
               mmc_card_highspeed(card) ? "high speed " : "",
               type, card->rca);
    }
    /*下面这个device的注册会引发mmc_bus_type的相关动作，
	但是这里MMC卡的驱动永远不会注册进来,所以他就是个孤魂
	野鬼飘荡在bus上面，后面使用的是sdio_driver*/
    /*ret = device_add(&card->dev);
	if (ret)
		return ret;

#ifdef CONFIG_DEBUG_FS
	mmc_add_card_debugfs(card);
#endif*/

    mmc_card_set_present(card);

    return 0;
}


/**
 *	mmc_alloc_host - initialise the per-host structure.
 *	@extra: sizeof private data structure
 *	@dev: pointer to host device model structure
 *
 *	Initialise the per-host structure.
 */

struct mmc_host *mmc_alloc_host(void)
{
    static struct mmc_host s3c2440_gsdiohost;
    struct mmc_host *host=&s3c2440_gsdiohost;
    memset(host,0,sizeof(struct mmc_host));
#define PAGE_CACHE_SIZE   (1<<12)// 4kB
    host->max_hw_segs = 1;
    host->max_phys_segs = 1;
    host->max_seg_size = PAGE_CACHE_SIZE;

    host->max_req_size = PAGE_CACHE_SIZE;
    host->max_blk_size = 512;
    host->max_blk_count = PAGE_CACHE_SIZE / 512;

    return host;
}

/*
 * Allocate and initialise a new SDIO function structure.
 */
struct sdio_func *sdio_alloc_func(struct mmc_card *card)
{
    static struct sdio_func marvel_gsdiofunc;

    struct sdio_func *func=&marvel_gsdiofunc;

    /*func = kzalloc(sizeof(struct sdio_func), GFP_KERNEL);
	if (!func)
		return ERR_PTR(-ENOMEM);*/
    memset(func,0,sizeof(struct sdio_func));
    func->card = card;


    //	device_initialize(&func->dev);

    //	func->dev.parent = &card->dev;
    //	func->dev.bus = &sdio_bus_type;
    //	func->dev.release = sdio_release_func;

    return func;
}

/*
 * Register a new SDIO function with the driver model.
 */
int sdio_add_func(struct sdio_func *func)
{
    int ret=0;

    //dev_set_name(&func->dev, "%s:%d", mmc_card_id(func->card), func->num);

    //ret = device_add(&func->dev);//调用sdio_bus进行match和probe
    //ret = device_add(&func->dev)
    //	if (ret == 0)
    sdio_func_set_present(func);

    return ret;
}

/**
 *	sdio_enable_func - enables a SDIO function for usage
 *	@func: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register
 *	access is possible.
 */
int sdio_enable_func(struct sdio_func *func)
{
    int ret;
    unsigned char reg;
    unsigned long timeout;

    //BUG_ON(!func);
    //BUG_ON(!func->card);

    pr_debug("SDIO: Enabling device %s...\n", sdio_func_id(func));

    ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
    if (ret)
        goto err;

    reg |= 1 << func->num;

    ret = mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
    if (ret)
        goto err;

    timeout =func->enable_timeout;

    while (1) {
        ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IORx, 0, &reg);
        if (ret)
            goto err;
        if (reg & (1 << func->num))
            break;

        ms_delay();
        timeout--;
        ret = -ETIME;
        if (timeout==0)
            goto err;
    }

    pr_debug("SDIO: Enabled device %s\n", sdio_func_id(func));

    return 0;

    err:
    pr_debug("SDIO: Failed to enable device %s\n", sdio_func_id(func));
    return ret;
}

static int sdio_card_irq_get(struct mmc_card *card)
{
    struct mmc_host *host = card->host;
    if (!host->sdio_irqs++)
        pr_debug("setup sdio irq success!\n");

		//EnableIrq(SDIO_IRQChannel);
    //	WARN_ON(!host->claimed);

    //if (!host->sdio_irqs++) {
    //第一次使用时要创建sdio host的irq线程，为了适应部分sd卡控制器不能中断的情况，
    //内核使用sdio_irq_thread线程定期轮询SDIO控制器的状态来调度中断请求
    //atomic_set(&host->sdio_irq_thread_abort, 0);
    /*host->sdio_irq_thread =
			kthread_run(sdio_irq_thread, host, "ksdioirqd/%s",
				mmc_hostname(host));*/
    /*	if (IS_ERR(host->sdio_irq_thread)) {
			int err = PTR_ERR(host->sdio_irq_thread);
			host->sdio_irqs--;
			return err;
		}
	}*/

    return 0;
}

/**
 *	sdio_claim_irq - claim the IRQ for a SDIO function
 *	@func: SDIO function
 *	@handler: IRQ handler callback
 *
 *	Claim and activate the IRQ for the given SDIO function. The provided
 *	handler will be called when that IRQ is asserted.  The host is always
 *	claimed already when the handler is called so the handler must not
 *	call sdio_claim_host() nor sdio_release_host().
 */
int sdio_claim_irq(struct sdio_func *func, sdio_irq_handler_t *handler)
{
    int ret;
    unsigned char reg;

    //BUG_ON(!func);
    //BUG_ON(!func->card);

    pr_debug("SDIO: Enabling IRQ for %s...\n", sdio_func_id(func));

    if (func->irq_handler) {
        pr_debug("SDIO: IRQ for %s already in use.\n", sdio_func_id(func));
        return -EBUSY;
    }

    ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IENx, 0, &reg);
    if (ret)
        return ret;

    reg |= 1 << func->num;

    reg |= 1; /* Master interrupt enable */

    ret = mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IENx, reg, NULL);
    if (ret)
        return ret;

    func->irq_handler = handler; //Install irq handler.
    ret = sdio_card_irq_get(func->card);
    if (ret)
        func->irq_handler = NULL;

    return ret;
}

/******************************************sdio bus *******************************/
static const struct sdio_device_id *sdio_match_one(struct sdio_func *func,
                                                   const struct sdio_device_id *id)
{
    if (id->class != ( u8)SDIO_ANY_ID && id->class != func->class)
        return NULL;
    if (id->vendor != (u16)SDIO_ANY_ID && id->vendor != func->vendor)
        return NULL;
    if (id->device != (u16)SDIO_ANY_ID && id->device != func->device)
        return NULL;
    return id;
}

const struct sdio_device_id if_sdio_ids[3]={//wo support marvell8385 and marvell 8686
    SDIO_ANY_ID,SDIO_VENDOR_ID_MARVELL,SDIO_DEVICE_ID_MARVELL_LIBERTAS,//marvell 8385
    SDIO_ANY_ID,SDIO_VENDOR_ID_MARVELL,SDIO_DEVICE_ID_MARVELL_8688WLAN,//marvell 8686
    0,0,0
};
static struct sdio_device_id *sdio_match_device(struct sdio_func *func)
{
    struct sdio_device_id *ids=(struct sdio_device_id *)if_sdio_ids;
    pr_debug("function vendor=0x%x	device=0x%x\n",func->vendor,func->device);
    if (ids) {
        while (ids->class || ids->vendor || ids->device) {
            if (sdio_match_one(func, ids))
                return ids;
            ids++;
        }
    }
    return NULL;
}

/**
 *	sdio_set_block_size - set the block size of an SDIO function
 *	@func: SDIO function to change
 *	@blksz: new block size or 0 to use the default.
 *
 *	The default block size is the largest supported by both the function
 *	and the host, with a maximum of 512 to ensure that arbitrarily sized
 *	data transfer use the optimal (least) number of commands.
 *
 *	A driver may call this to override the default block size set by the
 *	core. This can be used to set a block size greater than the maximum
 *	that reported by the card; it is the driver's responsibility to ensure
 *	it uses a value that the card supports.
 *
 *	Returns 0 on success, -EINVAL if the host does not support the
 *	requested block size, or -EIO (etc.) if one of the resultant FBR block
 *	size register writes failed.
 *
 */
int sdio_set_block_size(struct sdio_func *func, unsigned blksz)
{
    int ret;

    if (blksz > func->card->host->max_blk_size)
        return -EINVAL;

    if (blksz == 0) {
        blksz = min(func->max_blksize, func->card->host->max_blk_size);
        blksz = min(blksz, 512u);
    }

    ret = mmc_io_rw_direct(func->card, 1, 0,
                           SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE,
                           blksz & 0xff, NULL);
    if (ret)
        return ret;
    ret = mmc_io_rw_direct(func->card, 1, 0,
                           SDIO_FBR_BASE(func->num) + SDIO_FBR_BLKSIZE + 1,
                           (blksz >> 8) & 0xff, NULL);
    if (ret)
        return ret;
    func->cur_blksize = blksz;
    return 0;
}

struct lbs_private *sdio_bus_probe(struct sdio_func *func)
{
    struct sdio_device_id *id;
    int ret;

    id = sdio_match_device(func);
    if (!id){
        _hx_printf("Cann't find support modules!\n");
        return (struct lbs_private *)NULL;
    }
    /* Set the default block size so the driver is sure it's something
	 * sensible. */
    ret = sdio_set_block_size(func, 0);
    if (ret)
        return (struct lbs_private *)NULL;
    return if_sdio_probe(func,id);
}
