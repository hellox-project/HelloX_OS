/*
 *  linux/drivers/mmc/s3cmci.h - Samsung S3C MCI driver
 *
 *  Copyright (C) 2004-2006 Thomas Kleffel, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __STM32_HOST__H__
#define __STM32_HOST__H__

#include "type.h"

#define SD_MAX_DATA_LENGTH              ((u32)0x01FFFFFF)

/*STM32 register bit define*/
#define SDIO_ICR_MASK 0x5FF
#define SD_DATATIMEOUT                  ((u32)0x000FFFFF)
#define SDIO_FIFO_Address               ((u32)0x40018080)

#define XFER_NONE 0
#define XFER_READ 1
#define XFER_WRITE 2

enum s3cmci_waitfor {
	COMPLETION_NONE,
	COMPLETION_FINALIZE,
	COMPLETION_CMDSENT,
	COMPLETION_RSPFIN,
	COMPLETION_XFERFINISH,
	COMPLETION_XFERFINISH_RSPFIN,
};
	

struct stm32_host {
	//struct platform_device	*pdev;
	//struct s3c24xx_mci_pdata *pdata;
	struct mmc_host		*mmc;
	//struct resource		*mem;
	//struct clk		*clk;
	unsigned long 		base;
	int			irq;
	int			irq_cd;
	int			dma;

	unsigned long		clk_rate;
	unsigned long		clk_div;
	unsigned long		real_rate;
	u8			prescaler;

	int			is2440;
	unsigned		sdiimsk;
	unsigned		sdidata;
	int			dodma;
	int			dmatogo;

	bool			irq_disabled;
	bool			irq_enabled;
	bool			irq_state;
	int			sdio_irqen;

	struct mmc_request	*mrq;
	int			cmd_is_stop;

	//spinlock_t		complete_lock;
	enum s3cmci_waitfor	complete_what;

	int			dma_complete;

	u32			pio_sgptr;
	u32			pio_bytes;
	u32			pio_count;
	u32			*pio_ptr;

	u32			pio_active;

	int			bus_width;

	//char 			dbgmsg_cmd[301];
	//char 			dbgmsg_dat[301];
	char			*status;

	unsigned int		ccnt, dcnt;
/*	struct tasklet_struct	pio_tasklet;

#ifdef CONFIG_DEBUG_FS
	struct dentry		*debug_root;
	struct dentry		*debug_state;
	struct dentry		*debug_regs;
#endif

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif*/
};
struct mmc_host * stm32_probe(void);
void  stm32_irq(void);
#endif
