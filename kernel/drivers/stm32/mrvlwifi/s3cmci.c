#include "stdio.h"
#include "stdlib.h"

#include "lwip/opt.h"
#include "reg_sdio.h"
#include "type.h"
#include "common.h"
#include "string.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sdio.h"
#include "sdio_func.h"
#include "marvell_ops.h"


#define CONFIG_MMC_S3C_PIO
#define CONFIG_MMC_S3C_HW_SDIO_IRQ
#undef  CONFIG_MMC_S3C_DMA

static struct stm32_host *gpstm32_host;//主要是给中断服务程序使用
#define SDIO_IRQChannel              ((u8)0x31)  /* SDIO global Interrupt */



//static struct s3c24xx_mci_pdata mini2440_mmc_cfg; 
/*= {
   .gpio_detect   = S3C2410_GPG(8),
   .gpio_wprotect = S3C2410_GPH(8),
   .set_power     = NULL,
   .ocr_avail     = MMC_VDD_32_33|MMC_VDD_33_34,
};*/

static struct mmc_host_ops stm32_ops; 

static void udelay(unsigned int us)
{
	unsigned long i,ii=500;
	for(i=0;i<us;i++){
		ii=500;
		while(ii--);
	}
}
static unsigned int readl(unsigned long addr)
{  
   return (*(volatile unsigned *)addr);
}

static unsigned int writel(unsigned int data,unsigned long addr)
{  
   return (*(volatile unsigned *)addr)=data;
}
void EnableIrq(u32 irq)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = irq;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
void DisableIrq(u32 irq)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = irq;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*typedef void __irq sido_irq_proc(void);//中断处理函数

 void setup_sdio_irq(sido_irq_proc *phandle)
 {
	ClearPending(BIT_SDI);
	pISR_SDI=(unsigned)phandle; 
	EnableIrq(BIT_SDI);
 }*/

 
/**
 * stm32_host_usedma - return whether the host is using dma or pio
 * @host: The host state mmc_host
 *
 * Return true if the host is using DMA to transfer data, else false
 * to use PIO mode. Will return static data depending on the driver
 * configuration.
 */
static int stm32_host_usedma(struct stm32_host *host)
{
#ifdef CONFIG_MMC_S3C_PIO
	return 0;
#elif defined(CONFIG_MMC_S3C_DMA)
	return 1;
#else
	return host->dodma;
#endif
}

/**
 * stm32_host_canpio - return true if host has pio code available
 *
 * Return true if the driver has been compiled with the PIO support code
 * available.
 */
/*
static bool stm32_host_canpio(void)
{
#ifdef CONFIG_MMC_S3C_PIO
	return true;
#else
	return false;
#endif
}	   */

static u32 enable_imask(struct stm32_host *host, u32 imask)
{
	u32 mask = SDIO->MASK;
	SDIO_ITConfig(imask|mask,ENABLE);	
	return 0;
}

static u32 disable_imask(struct stm32_host *host, u32 imask)
{
	u32 mask = SDIO->MASK;
	SDIO_ITConfig(~imask&mask,DISABLE);	
	return 0;
}

static void clear_imask(struct stm32_host *host)
{
	u32 mask = SDIO->MASK;
	/* preserve the SDIO IRQ mask state */
	mask &= SDIO_IT_SDIOIT;
	SDIO->MASK = mask;
}

/**
 * stm32_check_sdio_irq - test whether the SDIO IRQ is being signalled
 * @host: The host to check.
 *
 * Test to see if the SDIO interrupt is being signalled in case the
 * controller has failed to re-detect a card interrupt. Read GPE8 and
 * see if it is low and if so, signal a SDIO interrupt.
 *
 * This is currently called if a request is finished (we assume that the
 * bus is now idle) and when the SDIO IRQ is enabled in case the IRQ is
 * already being indicated.
*/
static void stm32_check_sdio_irq(struct stm32_host *host)
{
	#ifdef MASK_DEBUG
	if (host->sdio_irqen) {
	//	if (gpio_get_value(S3C2410_GPE(8)) == 0) {//这个是sdio中断数据与中断共用的引脚
			if((rGPEDAT&&(1<<8))==0){//GPE8
			printk(" signalling irq\n");
			mmc_signal_sdio_irq(host->mmc);
		}
	}
	#endif
}
#if 0
static int get_data_buffer(struct stm32_host *host,
				  u32 *bytes, u32 **pointer)
{
	//struct scatterlist *sg;
	unsigned char *sg;
	if (host->pio_active == XFER_NONE)
		return -EINVAL;

	if ((!host->mrq) || (!host->mrq->data))
		return -EINVAL;

	//if (host->pio_sgptr >= host->mrq->data->sg_len) {
	if(host->pio_sgptr >0){
		host->pio_sgptr =0;//传输完成
		pr_debug( "no more buffers (%i/%i)\n",
		      host->pio_sgptr, host->mrq->data->sg_len);
		return -EBUSY;
	}
	//sg = &host->mrq->data->sg[host->pio_sgptr];
	sg=host->mrq->data->sg;//buffer地址
	*bytes = host->mrq->data->sg_len;//buffer长度
	*pointer =(u32 *)sg;

	host->pio_sgptr=1;//scatterlist index

	pr_debug("new buffer (%i/%i)\n",
	    host->pio_sgptr, host->mrq->data->sg_len);

	return 0;
}

static u32 fifo_count(struct stm32_host *host)
{
	u32 fifostat = readl(host->base + S3C2410_SDIFSTA);

	fifostat &= S3C2410_SDIFSTA_COUNTMASK;
	return fifostat;
}

static u32 fifo_free(struct stm32_host *host)
{
	u32 fifostat = readl(host->base + S3C2410_SDIFSTA);

	fifostat &= S3C2410_SDIFSTA_COUNTMASK;
	return 63 - fifostat;
}
#endif
/**
 * stm32_enable_irq - enable IRQ, after having disabled it.
 * @host: The device state.
 * @more: True if more IRQs are expected from transfer.
 *
 * Enable the main IRQ if needed after it has been disabled.
 *
 * The IRQ can be one of the following states:
 *	- disabled during IDLE
 *	- disabled whilst processing data
 *	- enabled during transfer
 *	- enabled whilst awaiting SDIO interrupt detection
 */
static void stm32_enable_irq(struct stm32_host *host, bool more)
{
	bool enable = _FALSE;
	host->irq_enabled = more;
	host->irq_disabled = _FALSE;

	enable =(bool) (more ||((bool) host->sdio_irqen));

	if (host->irq_state != enable) {
		host->irq_state = enable;

		if (enable)
			EnableIrq(host->irq);
		else
			DisableIrq(host->irq);
	}
}

/**
 *
 */
static void stm32_disable_irq(struct stm32_host *host, bool transfer)
{
	host->irq_disabled = transfer;

	if (transfer && host->irq_state) {
		host->irq_state = _FALSE;
		DisableIrq(host->irq);
	}
}
#if 0
static void do_pio_read(struct stm32_host *host)
{
	int res;
	u32 fifo;
	u32 *ptr;
	u32 fifo_words;
	unsigned long from_ptr;

	/* write real prescaler to host, it might be set slow to fix */
	writel(host->prescaler, host->base + S3C2410_SDIPRE);

	from_ptr = host->base + host->sdidata;
	fifo = fifo_count(host);
	while (fifo!=0) {
		if (!host->pio_bytes) {
			res = get_data_buffer(host, &host->pio_bytes,
					      &host->pio_ptr);
			if (res) {
				host->pio_active = XFER_NONE;
				host->complete_what = COMPLETION_FINALIZE;

				pr_debug( "pio_read(): "
				    "complete (no more data).\n");
				return;
			}

			pr_debug(
			    "pio_read(): new target: [%i]@[%p]\n",
			    host->pio_bytes, host->pio_ptr);
		}

		pr_debug(
		    "pio_read(): fifo:[%02i] buffer:[%03i] dcnt:[%08X]\n",
		    fifo, host->pio_bytes,
		    readl(host->base + S3C2410_SDIDCNT));

		/* If we have reached the end of the block, we can
		 * read a word and get 1 to 3 bytes.  If we in the
		 * middle of the block, we have to read full words,
		 * otherwise we will write garbage, so round down to
		 * an even multiple of 4. */
		if (fifo >= host->pio_bytes)
			fifo = host->pio_bytes;
		else
			fifo -= fifo & 3;

		host->pio_bytes -= fifo;
		host->pio_count += fifo;

		fifo_words = fifo >> 2;
		ptr = host->pio_ptr;
		while (fifo_words--)
			*ptr++ = readl(from_ptr);
		host->pio_ptr = ptr;

		if (fifo & 3) {
			u32 n = fifo & 3;
			u32 data = readl(from_ptr);
			u8 *p = (u8 *)host->pio_ptr;

			while (n--) {
				*p++ = data;
				data >>= 8;
			}
		}
	}

	if (!host->pio_bytes) {
		res = get_data_buffer(host, &host->pio_bytes, &host->pio_ptr);
		if (res) {
			pr_debug(
			    "pio_read(): complete (no more buffers).\n");
			host->pio_active = XFER_NONE;
			host->complete_what = COMPLETION_FINALIZE;

			return;
		}
	}

	enable_imask(host,S3C2410_SDIIMSK_RXFIFOHALF | S3C2410_SDIIMSK_RXFIFOLAST);
}

static void do_pio_write(struct stm32_host *host)
{
	unsigned long to_ptr;
	int res;
	u32 fifo;
	u32 *ptr;

	to_ptr = host->base + host->sdidata;

	while ((fifo = fifo_free(host)) > 3) {// tx fifo 空余空间
		if (!host->pio_bytes) { //缓冲区为空，重新获取数据
			res = get_data_buffer(host, &host->pio_bytes,
							&host->pio_ptr);
			if (res) {
				pr_debug("pio_write(): complete (no more data).\n");
				host->pio_active = XFER_NONE;

				return;
			}

			pr_debug( "pio_write(): new source: [%i]@[%p]\n",
			    host->pio_bytes, host->pio_ptr);

		}

		/* If we have reached the end of the block, we have to
		 * write exactly the remaining number of bytes.  If we
		 * in the middle of the block, we have to write full
		 * words, so round down to an even multiple of 4. */
		if (fifo >= host->pio_bytes)
			fifo = host->pio_bytes;
		else
			fifo -= fifo & 3;//对齐,最终得到的是4字节对齐的数据

		host->pio_bytes -= fifo;
		host->pio_count += fifo;

		fifo = (fifo + 3) >> 2;
		ptr = host->pio_ptr;
		while (fifo--)
			writel(*ptr++, to_ptr);
		host->pio_ptr = ptr;
	}

	enable_imask(host, S3C2410_SDIIMSK_TXFIFOHALF);
}
#endif

static void stm32_send_command(struct stm32_host *host,
					struct mmc_command *cmd)
{
	u32 imsk;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	sdio_deb_enter();
	imsk=SDIO_IT_CCRCFAIL|SDIO_IT_CTIMEOUT|//卡响应的数据
		SDIO_IT_CMDREND|SDIO_IT_CMDSENT;
	enable_imask(host, imsk);//需要读取的中断
	if (cmd->data)
		host->complete_what = COMPLETION_XFERFINISH_RSPFIN;
	else if (cmd->flags & MMC_RSP_PRESENT)
		host->complete_what = COMPLETION_RSPFIN;
	else
		host->complete_what = COMPLETION_CMDSENT;
	  SDIO->DTIMER=0xFFFFFFFF;
	  SDIO_CmdInitStructure.SDIO_Argument =cmd->arg;
	  SDIO_CmdInitStructure.SDIO_CmdIndex =cmd->opcode;
	  if (cmd->flags & MMC_RSP_PRESENT)
		  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	  else if (cmd->flags & MMC_RSP_136)
		  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
	  else
	  	SDIO_CmdInitStructure.SDIO_Response=SDIO_Response_No;
	  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	  SDIO_SendCommand(&SDIO_CmdInitStructure);
	  sdio_deb_leave();
}



static void DMA_RxConfiguration(u32 *BufferDST, u32 BufferSize)
{
  DMA_InitTypeDef DMA_InitStructure;

  DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

  /* DMA2 Channel4 disable */
  DMA_Cmd(DMA2_Channel4, DISABLE);

  /* DMA2 Channel4 Config */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)SDIO_FIFO_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BufferDST;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel4, &DMA_InitStructure);

  /* DMA2 Channel4 enable */
  DMA_Cmd(DMA2_Channel4, ENABLE);
}

static void DMA_TxConfiguration(u32 *BufferSRC, u32 BufferSize)
{
  DMA_InitTypeDef DMA_InitStructure;

  DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

  /* DMA2 Channel4 disable */
  DMA_Cmd(DMA2_Channel4, DISABLE);

  /* DMA2 Channel4 Config */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)SDIO_FIFO_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BufferSRC;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = BufferSize / 4;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel4, &DMA_InitStructure);

  /* DMA2 Channel4 enable */
  DMA_Cmd(DMA2_Channel4, ENABLE);
}


static int stm32_setup_data(struct stm32_host *host, struct mmc_data *data)
{
	SDIO_DataInitTypeDef SDIO_DataInitStructure;
	u32 blocks=data->blocks;
	u32 blksz=data->blksz;
	u32 total_len;
	u32 imask;
	u8 power = 0;
	/* write DCON register */
	sdio_deb_enter();
	if (!data) {
		return 0;
	}
	if ((data->blksz & 3) != 0) {
		/* We cannot deal with unaligned blocks with more than
		 * one block being transfered. */

		if (data->blocks > 1) {
			printk("can't do non-word sized block transfers (blksz %d)\n",data->blksz);
			return -EINVAL;
		}
	}
	//reset data config	
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	SDIO_DMACmd(DISABLE);
  //reset state 
  SDIO_ClearFlag(SDIO_ICR_MASK);
	if (host->bus_width == MMC_BUS_WIDTH_4){//Set SDIO bus bandwidth.
		SDIO->CLKCR&=~(3<<11);
		SDIO->CLKCR|=SDIO_BusWide_4b;
	}
		/* FIX: set slow clock to prevent timeouts on read */
	/*if (data->flags & MMC_DATA_READ)
		SDIO->CLKCR|=0xff;*/

  if ((blksz > 0) && (blksz <= 2048) && (0 == (blksz & (blksz - 1)))){
	  power = convert_from_bytes_to_power_of_two(blksz);
	  total_len=blksz*blocks;
  }     
	else
	{
   	   return -EINVAL;
	}
  
	/* Common to all modes */
  if (total_len > SD_MAX_DATA_LENGTH)
	{
 		 return -EINVAL;
	}
  
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = total_len;
  SDIO_DataInitStructure.SDIO_DataBlockSize = (u32) power << 4;
  if (data->flags & MMC_DATA_WRITE)
	{
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	}
  else if (data->flags & MMC_DATA_READ)
	{
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	}
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;//暂时不使能DPSM
  SDIO_DataConfig(&SDIO_DataInitStructure);//DMA disable
  if (data->flags & MMC_DATA_READ){
		DMA_RxConfiguration((u32 *)data->sg,total_len);//安装DMA参数
	 	SDIO->DCTRL|=((1<<3)|SDIO_DPSM_Enable);//接收时先使能DMA和数据状态机
   }	 	
   else  if (data->flags & MMC_DATA_WRITE)
	 {
		DMA_TxConfiguration((u32 *)data->sg,total_len);
	 }
	imask = SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND; //data interrupt
	enable_imask(host, imask); //Enable all these interrupts.
  sdio_deb_leave();
	return 0;
}

static void  stm32_enable_dma(void)
{      
	u32 imask=SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND; 
	sdio_deb_enter();
	SDIO_ClearFlag(SDIO_ICR_MASK);
	SDIO->DCTRL|=(SDIO_DPSM_Enable|(1<<3));//使能DMA和数据发送
    SDIO_ITConfig(imask, ENABLE);
	 while (DMA_GetFlagStatus(DMA2_FLAG_TC4) == RESET);
	 sdio_deb_leave();

}

#define BOTH_DIR (MMC_DATA_WRITE | MMC_DATA_READ)

#ifdef MASK_DEBUG
static int stm32_prepare_pio(struct stm32_host *host, struct mmc_data *data)
{
	int rw = (data->flags & MMC_DATA_WRITE) ? 1 : 0;//上层传递下来的读写标志

	//BUG_ON((data->flags & BOTH_DIR) == BOTH_DIR);

	host->pio_sgptr = 0;
	host->pio_bytes = 0;
	host->pio_count = 0;
	host->pio_active = rw ? XFER_WRITE : XFER_READ;

	if (rw) {
		do_pio_write(host);
		enable_imask(host, S3C2410_SDIIMSK_TXFIFOHALF);
	} else {
		enable_imask(host, S3C2410_SDIIMSK_RXFIFOHALF
			     | S3C2410_SDIIMSK_RXFIFOLAST);//准备接收FIFO为数据的接收
		
	}

	return 0;
}
#endif

static void stm32_send_request(struct mmc_host *mmc)
{
	struct stm32_host *host = mmc_priv(mmc);
	struct mmc_request *mrq = host->mrq;
	struct mmc_command *cmd;
	//u32 imask;
	sdio_deb_enter();
	cmd=host->cmd_is_stop ? mrq->stop : mrq->cmd;
	host->ccnt++;
	SDIO_ClearFlag(SDIO_ICR_MASK);
	if (cmd->data) {
		int res = stm32_setup_data(host, cmd->data);
		host->dcnt++;
		if (res) {
			printk( "setup data error %d\n", res);
			cmd->error = res;
			cmd->data->error = res;
			mmc_request_done(mmc, mrq);
			return;
		}
	} 
	/* Send command */
	stm32_send_command(host, cmd);
	/* Enable Interrupt */
	stm32_enable_irq(host, _TRUE);
	sdio_deb_leave();
}

static void finalize_request(struct stm32_host *host)
{
	struct mmc_request *mrq = host->mrq;
	struct mmc_command *cmd = host->cmd_is_stop ? mrq->stop : mrq->cmd;
	u32 clkcr;
	//pr_debug("finish request!\n");
	if (host->complete_what != COMPLETION_FINALIZE)
		return;

	if (!mrq)
		return;

	if (cmd->data && (cmd->error == 0) &&
	    (cmd->data->error == 0)) {
		if (stm32_host_usedma(host) && (!host->dma_complete)) {
			printk( "DMA Missing (%d)!\n",
			    host->dma_complete);
			return;
		}
	}

	/* Read response from controller. */
	cmd->resp[0] = SDIO->RESP1;//readl(host->base + S3C2410_SDIRSP0);
	cmd->resp[1] = SDIO->RESP2;// readl(host->base + S3C2410_SDIRSP1);
	cmd->resp[2] = SDIO->RESP3;// readl(host->base + S3C2410_SDIRSP2);
	cmd->resp[3] = SDIO->RESP4;// readl(host->base + S3C2410_SDIRSP3);

	//writel(host->prescaler, host->base + S3C2410_SDIPRE);
	clkcr  = SDIO->CLKCR&(~(0xFF));
	clkcr |= host->prescaler;
	SDIO->CLKCR = clkcr;//重新写回分频值

/*	if (cmd->error)
		debug_as_failure = 1;	  */

/*	if (cmd->data && cmd->data->error)
		debug_as_failure = 1;  */

//	dbg_dumpcmd(host, cmd, debug_as_failure);

	/* Cleanup controller */
	/*writel(0, host->base + S3C2410_SDICMDARG);
	writel(S3C2410_SDIDCON_STOP, host->base + S3C2410_SDIDCON);
	writel(0, host->base + S3C2410_SDICMDCON);*/
	 SDIO_ClearFlag(SDIO_ICR_MASK);

	clear_imask(host);

	if (cmd->data && cmd->error)
		cmd->data->error = cmd->error;

	if (cmd->data && cmd->data->stop && (!host->cmd_is_stop)) {//命令要求停止
		host->cmd_is_stop = 1;
		stm32_send_request(host->mmc);
		return;
	}

	/* If we have no data transfer we are finished here */
	if (!mrq->data)
		goto request_done;
	
#ifdef MASK_DEBUG
	/* Calulate the amout of bytes transfer if there was no error */
	if (mrq->data->error == 0) {
		mrq->data->bytes_xfered =
			(mrq->data->blocks * mrq->data->blksz);
	} else {
		mrq->data->bytes_xfered = 0;
	}

	/* If we had an error while transfering data we flush the
	 * DMA channel and the fifo to clear out any garbage. */
	if (mrq->data->error != 0) {
	/*	if (stm32_host_usedma(host))
			s3c2410_dma_ctrl(host->dma, S3C2410_DMAOP_FLUSH);*/

		if (host->is2440) {
			/* Clear failure register and reset fifo. */
			writel(S3C2440_SDIFSTA_FIFORESET |
			       S3C2440_SDIFSTA_FIFOFAIL,
			       host->base + S3C2410_SDIFSTA);
		} else {
			u32 mci_con;

			/* reset fifo */
			mci_con = readl(host->base + S3C2410_SDICON);
			mci_con |= S3C2410_SDICON_FIFORESET;

			writel(mci_con, host->base + S3C2410_SDICON);
		}
	}
#endif
	
request_done:
	host->complete_what = COMPLETION_NONE;
	host->mrq = NULL;

	stm32_check_sdio_irq(host);
	mmc_request_done(host->mmc, mrq);//正常出口，请求完成交给唤醒正在等待的命令处理函数
}

static void pio_tasklet(unsigned long data)
{
	struct stm32_host *host = (struct stm32_host *) data;
	//pr_debug("pio_tasklet deal!\n");
	stm32_disable_irq(host, _TRUE);
    if(sdio_sys_wait==1)
		udelay(20);
/*	//do pio
	if (host->pio_active == XFER_WRITE)
		do_pio_write(host);

	if (host->pio_active == XFER_READ)
		do_pio_read(host);*/

	if (host->complete_what == COMPLETION_FINALIZE) {
		clear_imask(host);
		if (host->pio_active != XFER_NONE) { //XFER_NONE指明没有数据传输
			printk( "unfinished %s "
			    "- pio_count:[%u] pio_bytes:[%u]\n",
			    (host->pio_active == XFER_READ) ? "read" : "write",
			    host->pio_count, host->pio_bytes);

			if (host->mrq->data)
				host->mrq->data->error = -EINVAL;
		}

		stm32_enable_irq(host, _FALSE);
		finalize_request(host);
	} else
		stm32_enable_irq(host, _TRUE);
}

static int stm32_card_present(struct mmc_host *mmc) 
{
	return 1;
}

static void stm32_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct stm32_host *host = mmc_priv(mmc);
	pr_debug("stm32_request!\n");
	host->status = "mmc request";
	host->cmd_is_stop = 0;
	host->mrq = mrq;

	if (stm32_card_present(mmc) == 0) {
		dbg("%s: no medium present\n", __func__);
		host->mrq->cmd->error = -ENOMEDIUM;
		mmc_request_done(mmc, mrq);
	} else
		stm32_send_request(mmc);
	sdio_deb_leave();
}

static void stm32_set_clk(struct stm32_host *host, struct mmc_ios *ios)
{
	u32 mci_psc;
	SDIO_InitTypeDef SDIO_InitStructure;
	/* Set clock */
	for (mci_psc = 0; mci_psc < 255; mci_psc++) {
		host->real_rate = host->clk_rate / (host->clk_div*(mci_psc+1));

		if (host->real_rate <= ios->clock)
			break;
	}

	if (mci_psc > 255)
		mci_psc = 255;
  /* Power ON Sequence -------------------------------------------------------*/
  /* Configure the SDIO peripheral */
  SDIO_InitStructure.SDIO_ClockDiv = mci_psc; /* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
  switch(ios->bus_width){//设置总线位宽
  case MMC_BUS_WIDTH_1:
		SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
		break;
  case MMC_BUS_WIDTH_4:
		SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
		break;
  case MMC_BUS_WIDTH_8:
		SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_8b;
		break;	
  }
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
  SDIO_Init(&SDIO_InitStructure);

  host->prescaler = mci_psc;
	/* If requested clock is 0, real_rate will be 0, too */
  if (ios->clock == 0)
		host->real_rate = 0;
}




#define SDIO_PIN_MASK ((3<<10)|(3<<12)|(3<<14)|(3<<16)|(3<<18)|(3<<20))
#define SDIO_PIN_FUNC ((2<<10)|(2<<12)|(2<<14)|(2<<16)|(2<<18)|(2<<20))
/*
static void s3c2410_cfg_sdiopin()
{
	unsigned long gpecon;
	gpecon=rGPECON;
	gpecon&=~SDIO_PIN_MASK;
	gpecon|=SDIO_PIN_FUNC;
	rGPECON=gpecon;
}	*/

static void stm32_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct stm32_host *host = mmc_priv(mmc);
//	u32 mci_con;

	/* Set the power state */
	stm32_set_clk(host, ios);

	
	switch (ios->power_mode) {
	case MMC_POWER_ON:
	case MMC_POWER_UP:
	  	SDIO_SetPowerState(SDIO_PowerState_ON);
		SDIO_ClockCmd(ENABLE);
		break;

	case MMC_POWER_OFF:
	default:
		SDIO_SetPowerState(SDIO_PowerState_OFF);
		SDIO_ClockCmd(DISABLE);
		_hx_printf("stm32 power off sdio!\n");
	}
	if ((ios->power_mode == MMC_POWER_ON) ||
	    (ios->power_mode == MMC_POWER_UP)) {
		printk("running at %lukHz (requested: %ukHz).\n",
			host->real_rate/1000, ios->clock/1000);
	} else {
		printk("powered down.\n");
	}
	host->bus_width = ios->bus_width;
}

static int stm32_get_ro(struct mmc_host *mmc)
{
	return 0;//无写保护
}

static void stm32_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct stm32_host *host = mmc_priv(mmc);
//	unsigned long flags;
	u32 con;

	//local_irq_save(flags);

	con = readl(host->base + S3C2410_SDICON);
	host->sdio_irqen = enable;

	if (enable == host->sdio_irqen)
		goto same_state;

	if (enable) {
		con |= S3C2410_SDICON_SDIOIRQ;
		enable_imask(host, S3C2410_SDIIMSK_SDIOIRQ);

		if (!host->irq_state && !host->irq_disabled) {
			host->irq_state = _TRUE;
			EnableIrq(host->irq);
		}
	} else {
		disable_imask(host, S3C2410_SDIIMSK_SDIOIRQ);
		con &= ~S3C2410_SDICON_SDIOIRQ;

		if (!host->irq_enabled && host->irq_state) {
			DisableIrq(host->irq);
			host->irq_state = _FALSE;
		}
	}

	writel(con, host->base + S3C2410_SDICON);

 same_state:
	stm32_check_sdio_irq(host);
}

/*
 * ISR for SDI Interface IRQ
 * Communication between driver and ISR works as follows:
 *   host->mrq 			points to current request
 *   host->complete_what	Indicates when the request is considered done
 *     COMPLETION_CMDSENT	  when the command was sent
 *     COMPLETION_RSPFIN          when a response was received
 *     COMPLETION_XFERFINISH	  when the data transfer is finished
 *     COMPLETION_XFERFINISH_RSPFIN both of the above.
 *   host->complete_request	is the completion-object the driver waits for
 *
 * 1) Driver sets up host->mrq and host->complete_what
 * 2) Driver prepares the transfer
 * 3) Driver enables interrupts
 * 4) Driver starts transfer
 * 5) Driver waits for host->complete_rquest
 * 6) ISR checks for request status (errors and success)
 * 6) ISR sets host->mrq->cmd->error and host->mrq->data->error
 * 7) ISR completes host->complete_request
 * 8) ISR disables interrupts
 * 9) Driver wakes up and takes care of the request
 *
 * Note: "->error"-fields are expected to be set to 0 before the request
 *       was issued by mmc.c - therefore they are only set, when an error
 *       contition comes up
 */

void  stm32_irq(void)
{
	struct stm32_host *host = gpstm32_host;
	struct mmc_request *mrq = host->mrq;
	struct mmc_command *cmd = host->cmd_is_stop ? mrq->stop : mrq->cmd;
	struct mmc_data *data=cmd->data;
	u32 stm_sta, stm_imsk;
	u32 stm_clear = 0;
	sdio_deb_enter();
	stm_sta  = SDIO->STA;    //SDI Data Status Register (SDIDatSta)
	stm_imsk = SDIO->MASK;
	
	if (stm_sta & SDIO_IT_SDIOIT) { //Interrupted by functional devices.
		if (stm_imsk & SDIO_IT_SDIOIT) {
			//_hx_printf("card sdio interupt!\r\n");
			host->status = "card sdio interrupt.";
			stm_clear = SDIO_IT_SDIOIT;
			SDIO->ICR = stm_clear;
			mmc_signal_sdio_irq(host->mmc);
			//ClearPending(BIT_SDI);
			goto irq_out;
		}
	}

	if ((host->complete_what == COMPLETION_NONE) ||
	    (host->complete_what == COMPLETION_FINALIZE)) {
		host->status = "nothing to complete";
		clear_imask(host);
		goto irq_out;
	}

	if (!host->mrq) {
		host->status = "no active mrq";
		clear_imask(host);
		goto irq_out;
	}

	cmd = host->cmd_is_stop ? host->mrq->stop : host->mrq->cmd;

	if (!cmd) {
		host->status = "no active cmd";
		clear_imask(host);
		goto irq_out;
	}

	if (stm_sta & SDIO_IT_CTIMEOUT) {
		printk( "CMDSTAT: error CMDTIMEOUT\n");
		cmd->error = -ETIMEDOUT;
		host->status = "error: command timeout";
		goto fail_transfer;
	}

	if (stm_sta & SDIO_IT_CMDSENT) {
		if (host->complete_what == COMPLETION_CMDSENT) {
			host->status = "ok: command sent";
			goto close_transfer;
		}

		stm_clear |= SDIO_IT_CMDSENT;
	}

	if (stm_sta & SDIO_IT_CCRCFAIL) {
		if (cmd->flags & MMC_RSP_CRC) {
			if (host->mrq->cmd->flags & MMC_RSP_136) {
				printk("fixup: ignore CRC fail with long rsp\n");
			} else {
				/* note, we used to fail the transfer
				 * here, but it seems that this is just
				 * the hardware getting it wrong.
				 *
				 * cmd->error = -EILSEQ;
				 * host->status = "error: bad command crc";
				 * goto fail_transfer;
				*/
			}
		}
		stm_clear |= SDIO_IT_CCRCFAIL;
	}

	if (stm_sta & SDIO_IT_CMDREND) {
		stm_clear |= SDIO_IT_CMDREND;
		if (host->complete_what == COMPLETION_RSPFIN) {
			host->status = "ok: command response received";
			goto close_transfer;
		}
		if (host->complete_what == COMPLETION_XFERFINISH_RSPFIN){
				host->complete_what = COMPLETION_XFERFINISH;
				if(data->flags&MMC_DATA_WRITE){
					stm32_enable_dma();
					return;
					}				
			}	
	}

	/* errors handled after this point are only relevant
	   when a data transfer is in progress */

	if (!cmd->data)
		goto clear_status_bits;

	if (stm_sta& SDIO_IT_DCRCFAIL) {
		printk( "bad data crc (outgoing)\n");
		cmd->data->error = -EILSEQ;
		host->status = "error: bad data crc (outgoing)";
		goto fail_transfer;
	}
	
	if (stm_sta & SDIO_IT_DTIMEOUT) {
		printk( "data timeout\n");
		cmd->data->error = -ETIMEDOUT;
		host->status = "error: data timeout";
		goto fail_transfer;
	}

	if (stm_sta & SDIO_IT_DATAEND) {
		if (host->complete_what == COMPLETION_XFERFINISH) {
			host->status = "ok: data transfer completed";
			data->bytes_xfered = data->sg_len;
			host->pio_active = XFER_NONE;
			goto close_transfer;
		}

		if (host->complete_what == COMPLETION_XFERFINISH_RSPFIN){
			host->complete_what = COMPLETION_RSPFIN;
			while (DMA_GetFlagStatus(DMA2_FLAG_TC4) == RESET);
			data->bytes_xfered = data->sg_len;
			host->pio_active = XFER_NONE;
			host->status = "ok: data transfer completed";
			SDIO_ClearFlag(SDIO_IT_DATAEND);		
			goto close_transfer;
		}
		stm_clear|=SDIO_IT_DATAEND;
	}

clear_status_bits:
	SDIO->ICR = stm_clear;
	goto irq_out;

fail_transfer:
	host->pio_active = XFER_NONE;
close_transfer:
	host->complete_what = COMPLETION_FINALIZE;
	clear_imask(host);
	pio_tasklet((unsigned long)host);
	goto irq_out;
irq_out:
	//_hx_printf("  stm state:0x%08x status:%s.\n", stm_sta,host->status);	
	sdio_deb_leave();
	return ;
}

 static void init_stm32_struct(struct mmc_host_ops *phost_ops)
{
	phost_ops->request	= stm32_request;
	phost_ops->set_ios	= stm32_set_ios;
	phost_ops->get_ro	 	= stm32_get_ro;
	phost_ops->get_cd		= stm32_card_present;
	phost_ops->enable_sdio_irq = stm32_enable_sdio_irq;
}

struct mmc_host * stm32_probe(void)
{
	struct mmc_host	*mmc;
	struct stm32_host *host;
	
  init_stm32_struct(&stm32_ops);  //Initialize STM32 SDIO operations.
  mmc = mmc_alloc_host();
	host = mmc_priv(mmc);
  gpstm32_host=host;              //A global variables may used by interrupt ISR.Fuck implementation.
        host->mmc = mmc;
	host->complete_what = COMPLETION_NONE;
	host->pio_active 	= XFER_NONE; 
	host->irq=SDIO_IRQChannel;
  EnableIrq(host->irq);           //Enable sdio irq
	host->irq_state = _FALSE;
  host->irq_cd = -1;              //Without any detection.
	host->clk_div=1;
	host->clk_rate =128000000;      //50Mhz
	mmc->ops 	= &stm32_ops;
	mmc->ocr_avail	= MMC_VDD_32_33 | MMC_VDD_33_34;
#ifdef CONFIG_MMC_S3C_HW_SDIO_IRQ
	mmc->caps	= MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ;
#else
	mmc->caps	= MMC_CAP_4_BIT_DATA;
#endif
	mmc->f_min 	= host->clk_rate /257;
	mmc->f_max 	= host->clk_rate;

	/*if (host->pdata->ocr_avail)
		mmc->ocr_avail = host->pdata->ocr_avail; */

	mmc->max_blk_count	= 4095;
	mmc->max_blk_size	= 4095;
	mmc->max_req_size	= 4095 * 512;
	mmc->max_seg_size	= mmc->max_req_size;
	mmc->max_phys_segs	= 128;
	mmc->max_hw_segs	= 128;

	pr_debug( "probe:stm32f103rc sdio!\n");
	return mmc;
}
