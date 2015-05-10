#ifndef LINUX_MMC_CARD_H
#define LINUX_MMC_CARD_H

#include "type.h"
#include "common.h"
#include "mmc.h"
#include "core.h"
#include "host.h"
#include "sd.h"
#include "sdio.h"
#include "sdio_func.h"


struct mmc_cid {
	unsigned int		manfid;
	char			prod_name[8];
	unsigned int		serial;
	unsigned short		oemid;
	unsigned short		year;
	unsigned char		hwrev;
	unsigned char		fwrev;
	unsigned char		month;
};

struct mmc_csd {
	unsigned char		mmca_vsn;
	unsigned short		cmdclass;
	unsigned short		tacc_clks;
	unsigned int		tacc_ns;
	unsigned int		r2w_factor;
	unsigned int		max_dtr;
	unsigned int		read_blkbits;
	unsigned int		write_blkbits;
	unsigned int		capacity;
	unsigned int		read_partial:1,
				read_misalign:1,
				write_partial:1,
				write_misalign:1;
};

struct mmc_ext_csd {
	u8			rev;
	unsigned int		sa_timeout;		/* Units: 100ns */
	unsigned int		hs_max_dtr;
	unsigned int		sectors;
};

struct sd_scr {
	unsigned char		sda_vsn;
	unsigned char		bus_widths;
#define SD_SCR_BUS_WIDTH_1	(1<<0)
#define SD_SCR_BUS_WIDTH_4	(1<<2)
};

struct sd_switch_caps {
	unsigned int		hs_max_dtr;
};

struct sdio_cccr {
	unsigned int		sdio_vsn;
	unsigned int		sd_vsn;
	unsigned int		multi_block:1,
				low_speed:1,
				wide_bus:1,
				high_power:1,
				high_speed:1,
				disable_cd:1;
};

struct sdio_cis {
	unsigned short		vendor;
	unsigned short		device;
	unsigned short		blksize;
	unsigned int		max_dtr;
};


#define SDIO_MAX_FUNCS		7

/*
 * MMC device
 */
/* card type */
#define MMC_TYPE_MMC		0		/* MMC card sdio_func*/
#define MMC_TYPE_SD		1		/* SD card */
#define MMC_TYPE_SDIO		2		/* SDIO card */

/* (our) card state */
#define MMC_STATE_PRESENT	(1<<0)		/* present in sysfs */
#define MMC_STATE_READONLY	(1<<1)		/* card is read-only */
#define MMC_STATE_HIGHSPEED	(1<<2)		/* card is in high speed mode */
#define MMC_STATE_BLOCKADDR	(1<<3)		/* card uses block-addressing */

/* card quirks */
#define MMC_QUIRK_LENIENT_FN0	(1<<0)		/* allow SDIO FN0 writes outside of the VS CCCR range */

struct mmc_card {
	struct mmc_host		*host;		/* 所属的主机控制器 */
	//struct device		dev;		/* the device */
	unsigned int		rca;		/* SDIO地址 */
	unsigned int		type;		/* 卡类型mmc、sdio、sd等*/
	unsigned int		state;		/* 当前卡的状态*/
	unsigned int		quirks; 	/* 当前卡的bug信息 */

	u32			raw_cid[4];	/* 卡的CID */
	u32			raw_csd[4];	/* 卡的CSD */
	u32			raw_scr[2];	/* 卡的SCR */
	struct mmc_cid		cid;		/* 卡的认证信息，均在枚举是获取*/
	struct mmc_csd		csd;		/* 卡的特定描述信息 */
	struct mmc_ext_csd	ext_csd;	/*卡的扩展信息*/
	struct sd_scr		scr;		/*SD卡使用 */
	struct sd_switch_caps	sw_caps;	/* SD卡使用 */

	unsigned int		sdio_funcs;	/* sdio功能数*/
	struct sdio_cccr	cccr;		/* sdio通用信息*/
	struct sdio_cis		cis;		/* common tuple info */
	struct sdio_func	*sdio_func[SDIO_MAX_FUNCS]; /* sdio设备功能数目 */
	unsigned		num_info;	/* 字符信息数目*/
	const char		**info;		/*字符信息*/
	struct sdio_func_tuple	*tuples;	/* 未知功能信息 */

	//struct dentry		*debugfs_root;
};


#define mmc_card_mmc(c)		((c)->type == MMC_TYPE_MMC)
#define mmc_card_sd(c)		((c)->type == MMC_TYPE_SD)
#define mmc_card_sdio(c)	((c)->type == MMC_TYPE_SDIO)

#define mmc_card_present(c)	((c)->state & MMC_STATE_PRESENT)
#define mmc_card_readonly(c)	((c)->state & MMC_STATE_READONLY)
#define mmc_card_highspeed(c)	((c)->state & MMC_STATE_HIGHSPEED)
#define mmc_card_blockaddr(c)	((c)->state & MMC_STATE_BLOCKADDR)

#define mmc_card_set_present(c)	((c)->state |= MMC_STATE_PRESENT)
#define mmc_card_set_readonly(c) ((c)->state |= MMC_STATE_READONLY)
#define mmc_card_set_highspeed(c) ((c)->state |= MMC_STATE_HIGHSPEED)
#define mmc_card_set_blockaddr(c) ((c)->state |= MMC_STATE_BLOCKADDR)


#endif
