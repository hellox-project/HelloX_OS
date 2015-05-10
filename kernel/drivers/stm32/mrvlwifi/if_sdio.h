/*
 *  linux/drivers/net/wireless/libertas/if_sdio.h
 *
 *  Copyright 2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#ifndef _LBS_IF_SDIO_H
#define _LBS_IF_SDIO_H
#include "type.h"
#include "common.h"
#include "sdio_func.h"
#include "sdio_ids.h"
#include "dev.h"


#define IF_SDIO_MODEL_8385	0x04
#define IF_SDIO_MODEL_8686	0x0b
#define IF_SDIO_MODEL_8688	0x10

#define IF_SDIO_IOPORT		0x00

#define IF_SDIO_H_INT_MASK	0x04
#define   IF_SDIO_H_INT_OFLOW	0x08
#define   IF_SDIO_H_INT_UFLOW	0x04
#define   IF_SDIO_H_INT_DNLD	0x02
#define   IF_SDIO_H_INT_UPLD	0x01

#define IF_SDIO_H_INT_STATUS	0x05
#define IF_SDIO_H_INT_RSR	0x06
#define IF_SDIO_H_INT_STATUS2	0x07

#define IF_SDIO_RD_BASE		0x10

#define IF_SDIO_STATUS		0x20
#define   IF_SDIO_IO_RDY	0x08
#define   IF_SDIO_CIS_RDY	0x04
#define   IF_SDIO_UL_RDY	0x02
#define   IF_SDIO_DL_RDY	0x01

#define IF_SDIO_C_INT_MASK	0x24
#define IF_SDIO_C_INT_STATUS	0x28
#define IF_SDIO_C_INT_RSR	0x2C

#define IF_SDIO_SCRATCH		0x34
#define IF_SDIO_SCRATCH_OLD	0x80fe
#define IF_SDIO_FW_STATUS	0x40
#define   IF_SDIO_FIRMWARE_OK	0xfedc

#define IF_SDIO_RX_LEN		0x42
#define IF_SDIO_RX_UNIT		0x43

#define IF_SDIO_EVENT           0x80fc

#define IF_SDIO_BLOCK_SIZE	256


struct if_sdio_packet {
	struct if_sdio_packet	*next;
	u16			nb;
	u8			buffer[1024] ;//__attribute__((aligned(4)));
};
struct firmware {
	u32 size;
	const u8 *data;
};

struct if_sdio_card {
	struct sdio_func	*func;
	struct lbs_private	*priv;
	int					model;
	unsigned long		ioport;
	unsigned int		scratch_reg;
	const struct firmware *helper;
	const struct firmware *firmware;
	u8			rx_unit;
	struct if_sdio_packet	*packets;
	u8			buffer[1600];
};


void if_sdio_interrupt(struct sdio_func *func);
u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret);
int if_sdio_send_data(struct lbs_private *priv, u8 *buf, u16 nb);
static int if_sdio_handle_event(struct if_sdio_card *card,u8 *buffer, unsigned size);
int poll_sdio_interrupt(struct sdio_func *func);
int  wait_for_data_end(void);
struct sdio_func;
struct sdio_device_id;
struct lbs_private *if_sdio_probe(struct sdio_func *func,struct sdio_device_id *id);

#endif

