/*
 *  include/linux/mmc/sdio_func.h
 *
 *  Copyright 2007-2008 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#ifndef MMC_SDIO_FUNC_H
#define MMC_SDIO_FUNC_H

#include "type.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sdio.h"
#include "if_sdio.h"
#include "common.h"

struct sdio_func;

//typedef void (sdio_irq_handler_t)(struct sdio_func *);

/*
 * SDIO function CIS tuple (unknown to the core)
 */
struct sdio_func_tuple {
	struct sdio_func_tuple *next;
	unsigned char code;
	unsigned char size;
	unsigned char data[255];
};

/*
 * SDIO function devices
 */
 #define SDIO_STATE_PRESENT	(1<<0)		/* present in sysfs */
 /********************************************************
 一般SDIO设备可能具有多个功能，本结构用于描述SDIO设备的所有的功能信息
 **********************************************************/
struct sdio_func {
	struct mmc_card		*card;		/* 所属的sdio卡*/
	struct if_sdio_card		*if_card;/*if_sdio_card是marvell驱动相关的，记录设备版本、固件等信息，这里的指针，只是为了使用更方便*/
//	struct device		dev;		/* the device */
	sdio_irq_handler_t	*irq_handler;	/* 功能中断处理函数 */
	unsigned int		num;		/* 所有功能数目 */
	/*NB*/
	unsigned char		class;		/* 接口类*/
	unsigned short		vendor;		/* 功能版本*/
	unsigned short		device;		/*设备ID*/

	unsigned		max_blksize;	/*功能支持的最大块尺寸,从设备内部读出*/
	unsigned		cur_blksize;	/* 当前使用的块尺寸 */

	unsigned		enable_timeout;	/* sdio执行的最长超时时间 */

	unsigned int		state;		/* 功能状态*/


	u8			tmpbuf[4];	/*使用DMA时对齐所需的填充 */

	unsigned		num_info;	/* 功能描述的字符串信息数目 */
	const char		**info;		/* 记录功能描述信息的数据*/

	struct sdio_func_tuple *tuples;/*sdio功能描述信息组*/
};

#define sdio_func_present(f)	((f)->state & SDIO_STATE_PRESENT)

#define sdio_func_set_present(f) ((f)->state |= SDIO_STATE_PRESENT)

#define sdio_func_id(f)		("80211card")

/*#define sdio_get_drvdata(f)	dev_get_drvdata(&(f)->dev)
#define sdio_set_drvdata(f,d)	dev_set_drvdata(&(f)->dev, d)
#define dev_to_sdio_func(d)	container_of(d, struct sdio_func, dev)*/

#define SDIO_ANY_ID (~0)

struct sdio_device_id {
	u8	class;			/* Standard interface or SDIO_ANY_ID */
	u16	vendor;			/* Vendor or SDIO_ANY_ID */
	u16	device;			/* Device ID or SDIO_ANY_ID */
};


/*
 * SDIO function device driver
 */
struct sdio_driver {
	char *name;
	struct sdio_device_id *id_table;
	char *(*probe)(struct sdio_func *, struct sdio_device_id *);//让它返回一个私有变量
	void (*remove)(struct sdio_func *);

	//struct device_driver drv;
};

#define sdio_get_drvdata(f)	 		(f->if_card)
#define sdio_set_drvdata(func, card) 	 func->if_card=card

//#define to_sdio_driver(d)	container_of(d, struct sdio_driver, drv)

#endif
