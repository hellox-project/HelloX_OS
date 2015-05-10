/*
 * linux/drivers/mmc/core/sdio_cis.c
 *
 * Author:	Nicolas Pitre
 * Created:	June 11, 2007
 * Copyright:	MontaVista Software Inc.
 *
 * Copyright 2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include "type.h"
#include "common.h"
#include "string.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "card.h"
#include "sdio.h"
#include "sdio_func.h"

static int cistpl_vers_1(struct mmc_card *card, struct sdio_func *func,
			 const unsigned char *buf, unsigned size)
{
    static char gvers_buffer[100];
    unsigned i, nr_strings;
    char **buffer, *string;

    /* Find all null-terminated (including zero length) strings in
	   the TPLLV1_INFO field. Trailing garbage is ignored. */
    buf += 2;
    size -= 2;

    nr_strings = 0;
    for (i = 0; i < size; i++) {
        if (buf[i] == 0xff)
            break;
        if (buf[i] == 0)
            nr_strings++;// 00 xx xx 00  00  xx xx xx 0xff
    }
    if (nr_strings == 0)
        return 0;

    size = i;

    /*buffer = kzalloc(sizeof(char*) * nr_strings + size, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;*/
    buffer=(char **)&gvers_buffer;
    memset(buffer,0,sizeof(buffer));
    string = (char*)(buffer + nr_strings);//存放字符串的地方,这里是移动nr_strings指针个单位

    for (i = 0; i < nr_strings; i++) {
        buffer[i] = string;
        strcpy(string, (const char *)buf);
        if(func)
            pr_debug("function infomation string(%d):%s \n",i,string);
        else
            pr_debug("card infomation string(%d):%s \n",i,string);

        string += strlen(string) + 1;
        buf += strlen((const char *)buf) + 1;
    }

    if (func) {
        func->num_info = nr_strings;
        func->info = (const char**)buffer;
    } else {
        card->num_info = nr_strings;
        card->info = (const char**)buffer;


    }
    return 0;
}


/*这个函数就是一个bug级别的,后面判断设备是否符合驱动就靠func->vendor = vendor;
func->device = device;这些ID了*/
static int cistpl_manfid(struct mmc_card *card, struct sdio_func *func,
			 const unsigned char *buf, unsigned size)
{
    unsigned int vendor, device;

    /* TPLMID_MANF */
    vendor = buf[0] | (buf[1] << 8);

    /* TPLMID_CARD */
    device = buf[2] | (buf[3] << 8);

    if (func) {
        func->vendor = vendor;
        func->device = device;
    } else {
        card->cis.vendor = vendor;
        card->cis.device = device;
    }

    return 0;
}

static const unsigned char speed_val[16] =
{ 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };
static const unsigned int speed_unit[8] =
{ 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };

/* FUNCE tuples with these types get passed to SDIO drivers */
static const unsigned char funce_type_whitelist[] = {
    4 /* CISTPL_FUNCE_LAN_NODE_ID used in Broadcom cards */
        };

static int cistpl_funce_whitelisted(unsigned char type)
{
    int i;

    for (i = 0; i < 1; i++) {
        if (funce_type_whitelist[i] == type)
            return 1;
    }
    return 0;
}

static int cistpl_funce_common(struct mmc_card *card,
			       const unsigned char *buf, unsigned size)
{
    if (size < 0x04 || buf[0] != 0)
        return -EINVAL;

    /* TPLFE_FN0_BLK_SIZE */
    card->cis.blksize = buf[1] | (buf[2] << 8);

    /* TPLFE_MAX_TRAN_SPEED */
    card->cis.max_dtr = speed_val[(buf[3] >> 3) & 15] *
                        speed_unit[buf[3] & 7];

    return 0;
}

static int cistpl_funce_func(struct sdio_func *func,
			     const unsigned char *buf, unsigned size)
{
    unsigned vsn;
    unsigned min_size;

    /* let SDIO drivers take care of whitelisted FUNCE tuples */
    if (cistpl_funce_whitelisted(buf[0]))
        return -EILSEQ;

    vsn = func->card->cccr.sdio_vsn;
    min_size = (vsn == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size < min_size || buf[0] != 1)
        return -EINVAL;

    /* TPLFE_MAX_BLK_SIZE */
    func->max_blksize = buf[12] | (buf[13] << 8);//最大的块尺寸

    /* TPLFE_ENABLE_TIMEOUT_VAL, present in ver 1.1 and above */
    if (vsn > SDIO_SDIO_REV_1_00)
        func->enable_timeout = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->enable_timeout = 1000;//默认是1s

    return 0;
}

static int cistpl_funce(struct mmc_card *card, struct sdio_func *func,
			const unsigned char *buf, unsigned size)
{
    int ret;

    /*
	 * There should be two versions of the CISTPL_FUNCE tuple,
	 * one for the common CIS (function 0) and a version used by
	 * the individual function's CIS (1-7). Yet, the later has a
	 * different length depending on the SDIO spec version.
	 */
    if (func)
        ret = cistpl_funce_func(func, buf, size);
    else
        ret = cistpl_funce_common(card, buf, size);

    if (ret && ret != -EILSEQ) {
        _hx_printf(KERN_ERR "%s: bad CISTPL_FUNCE size %u "
               "type %u\n", mmc_hostname(card->host), size, buf[0]);
    }

    return ret;
}

typedef int (tpl_parse_t)(struct mmc_card *, struct sdio_func *,
                          const unsigned char *, unsigned);

struct cis_tpl {
    unsigned char code;
    unsigned char min_size;
    tpl_parse_t *parse;
};

static const struct cis_tpl cis_tpl_list[] = {
    {	0x15,	3,	cistpl_vers_1		},
    {	0x20,	4,	cistpl_manfid		},
    {	0x21,	2,	/* cistpl_funcid */	},
    {	0x22,	0,	cistpl_funce		},
};

//#define ARRAY_SIZE(cis_tpl_list)  4

static int sdio_read_cis(struct mmc_card *card, struct sdio_func *func)
{
    static  struct sdio_func_tuple marvel_gtpl[5];
    int ret;
    struct sdio_func_tuple *this, **prev;
    unsigned long i, ptr = 0;
    unsigned char x, fn;
    unsigned char tpl_code, tpl_link;
    unsigned char tpl_cnt=0;
    const struct cis_tpl *tpl;
    /*
	 * Note that this works for the common CIS (function number 0) as
	 * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
	 * have the same offset.
	 */
    for (i = 0; i < 3; i++) {
        if (func)
            fn = func->num;//这里如果是读取功能的CIS，直接取功能号，然后求出寄存器偏移地址
        else
            fn = 0;

        ret = mmc_io_rw_direct(card, 0, 0,
                               SDIO_FBR_BASE(fn) + SDIO_FBR_CIS + i, 0, &x);
        if (ret)
            return ret;
        ptr |= x << (i * 8);//24bit
    }

    if (func)
        prev = &func->tuples;
    else
        prev = &card->tuples;

    //BUG_ON(*prev);

    do {

        ret = mmc_io_rw_direct(card, 0, 0, ptr++, 0, &tpl_code);
        if (ret)
            break;

        /* 0xff means we're done */
        if (tpl_code == 0xff)
            break;

        /* null entries have no link field or data */
        if (tpl_code == 0x00)
            continue;

        ret = mmc_io_rw_direct(card, 0, 0, ptr++, 0, &tpl_link);
        if (ret)
            break;

        /* a size of 0xff also means we're done */
        if (tpl_link == 0xff)
            break;

        /*this = kmalloc(sizeof(*this) + tpl_link, GFP_KERNEL);
		if (!this)
			return -ENOMEM;*/
        if(tpl_cnt==5)
            return -ENOMEM;
        else
            this=&marvel_gtpl[tpl_cnt++];//malloc

        for (i = 0; i < tpl_link; i++) {//read tpl data
            ret = mmc_io_rw_direct(card, 0, 0,
                                   ptr + i, 0, &this->data[i]);
            if (ret)
                break;
        }
        if (ret) {
            //kfree(this);
            tpl_cnt--;
            break;
        }

        for (i = 0; i < ARRAY_SIZE(cis_tpl_list); i++)
            if (cis_tpl_list[i].code == tpl_code)
                break;
        if (i < ARRAY_SIZE(cis_tpl_list)) {
            tpl = cis_tpl_list + i;
            if (tpl_link < tpl->min_size) {
                _hx_printf(KERN_ERR
                       "%s: bad CIS tuple 0x%02x"
                       " (length = %u, expected >= %u)\n",
                       mmc_hostname(card->host),
                       tpl_code, tpl_link, tpl->min_size);
                ret = -EINVAL;
            } else if (tpl->parse) {//根据cis的具体内容解析
                ret = tpl->parse(card, func,
                                 this->data, tpl_link);
            }
            /*
			 * We don't need the tuple anymore if it was
			 * successfully parsed by the SDIO core or if it is
			 * not going to be parsed by SDIO drivers.
			 */
            if (!ret || ret != -EILSEQ){
                //kfree(this);
                tpl_cnt--;
                //printk("error......\n");
            }
        } else {
            /* unknown tuple */
            ret = -EILSEQ;
        }

        if (ret == -EILSEQ) {
            /* this tuple is unknown to the core or whitelisted */
            this->next = NULL;
            this->code = tpl_code;
            this->size = tpl_link;
            *prev = this;
            prev = &this->next;//对于不清楚的tpl建立链表，一辈子只有一次，所以直接链入
            _hx_printf(KERN_DEBUG
                   "%s: queuing CIS tuple 0x%02x length %u\n",
                   mmc_hostname(card->host), tpl_code, tpl_link);
            /* keep on analyzing tuples */
            ret = 0;
        }
        ptr += tpl_link;
    } while (!ret);

    /*
	 * Link in all unknown tuples found in the common CIS so that
	 * drivers don't have to go digging in two places.
	 */
    if (func)
        *prev = card->tuples;//将卡的tpl链入到尾部

    return ret;
}



int sdio_read_common_cis(struct mmc_card *card)
{
    return sdio_read_cis(card, NULL);
}


int sdio_read_func_cis(struct sdio_func *func)
{
    int ret;

    ret = sdio_read_cis(func->card, func);
    if (ret)
        return ret;

    /*
	 * Since we've linked to tuples in the card structure,
	 * we must make sure we have a reference to it.
	 */
    //	get_device(&func->card->dev);

    /*
	 * Vendor/device id is optional for function CIS, so
	 * copy it from the card structure as needed.
	 */
    //识别的主要结构
    if (func->vendor == 0) {
        func->vendor = func->card->cis.vendor;
        func->device = func->card->cis.device;
    }

    return 0;
}

