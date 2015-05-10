#ifndef __HOST__H__
#define __HOST__H__

#include "type.h"
#include "common.h"
#include "mmc.h"
#include "core.h"
#include "card.h"
#include "sdio.h"
#include "s3cmci.h"


/*MMC card io state*/
/* command output mode */
#define MMC_BUSMODE_OPENDRAIN	1
#define MMC_BUSMODE_PUSHPULL	2
/* SPI chip select */
#define MMC_CS_DONTCARE		0
#define MMC_CS_HIGH		1
#define MMC_CS_LOW		2
/* power supply mode */
#define MMC_POWER_OFF		0
#define MMC_POWER_UP		1
#define MMC_POWER_ON		2

/* data bus width */
#define MMC_BUS_WIDTH_1		0
#define MMC_BUS_WIDTH_4		2
#define MMC_BUS_WIDTH_8		3
/* timing specification used */
#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2


struct mmc_ios {
	unsigned int	clock;			/* clock rate */
	unsigned short	vdd;
/* vdd stores the bit number of the selected voltage range from below. */
	unsigned char	bus_mode;		/* command output mode */

	unsigned char	chip_select;		/* SPI chip select */
	
	unsigned char	power_mode;		/* power supply mode */
	
	unsigned char	bus_width;		/* data bus width */
	
	unsigned char	timing;			/* timing specification used */
};



struct mmc_host_ops {
	/*
	 * Hosts that support power saving can use the 'enable' and 'disable'
	 * methods to exit and enter power saving states. 'enable' is called
	 * when the host is claimed and 'disable' is called (or scheduled with
	 * a delay) when the host is released. The 'disable' is scheduled if
	 * the disable delay set by 'mmc_set_disable_delay()' is non-zero,
	 * otherwise 'disable' is called immediately. 'disable' may be
	 * scheduled repeatedly, to permit ever greater power saving at the
	 * expense of ever greater latency to re-enable. Rescheduling is
	 * determined by the return value of the 'disable' method. A positive
	 * value gives the delay in milliseconds.
	 *
	 * In the case where a host function (like set_ios) may be called
	 * with or without the host claimed, enabling and disabling can be
	 * done directly and will nest correctly. Call 'mmc_host_enable()' and
	 * 'mmc_host_lazy_disable()' for this purpose, but note that these
	 * functions must be paired.
	 *
	 * Alternatively, 'mmc_host_enable()' may be paired with
	 * 'mmc_host_disable()' which calls 'disable' immediately.  In this
	 * case the 'disable' method will be called with 'lazy' set to 0.
	 * This is mainly useful for error paths.
	 *
	 * Because lazy disable may be called from a work queue, the 'disable'
	 * method must claim the host when 'lazy' != 0, which will work
	 * correctly because recursion is detected and handled.
	 */
	int (*enable)(struct mmc_host *host);
	int (*disable)(struct mmc_host *host, int lazy);
	void	(*request)(struct mmc_host *host, struct mmc_request *req);
	/*
	 * Avoid calling these three functions too often or in a "fast path",
	 * since underlaying controller might implement them in an expensive
	 * and/or slow way.
	 *
	 * Also note that these functions might sleep, so don't call them
	 * in the atomic contexts!
	 *
	 * Return values for the get_ro callback should be:
	 *   0 for a read/write card
	 *   1 for a read-only card
	 *   -ENOSYS when not supported (equal to NULL callback)
	 *   or a negative errno value when something bad happened
	 *
	 * Return values for the get_cd callback should be:
	 *   0 for a absent card
	 *   1 for a present card
	 *   -ENOSYS when not supported (equal to NULL callback)
	 *   or a negative errno value when something bad happened
	 */
	void	(*set_ios)(struct mmc_host *host, struct mmc_ios *ios);
	int	(*get_ro)(struct mmc_host *host);
	int	(*get_cd)(struct mmc_host *host);

	void	(*enable_sdio_irq)(struct mmc_host *host, int enable);
};


#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

/* Host capabilities */
#define MMC_CAP_4_BIT_DATA	(1 << 0)	/* Can the host do 4 bit transfers */
#define MMC_CAP_MMC_HIGHSPEED	(1 << 1)	/* Can do MMC high-speed timing */
#define MMC_CAP_SD_HIGHSPEED	(1 << 2)	/* Can do SD high-speed timing */
#define MMC_CAP_SDIO_IRQ	(1 << 3)	/* Can signal pending SDIO IRQs */
#define MMC_CAP_SPI		(1 << 4)	/* Talks only SPI protocols */
#define MMC_CAP_NEEDS_POLL	(1 << 5)	/* Needs polling for card-detection */
#define MMC_CAP_8_BIT_DATA	(1 << 6)	/* Can the host do 8 bit transfers */
#define MMC_CAP_DISABLE		(1 << 7)	/* Can the host be disabled */
#define MMC_CAP_NONREMOVABLE	(1 << 8)	/* Nonremovable e.g. eMMC */
#define MMC_CAP_WAIT_WHILE_BUSY	(1 << 9)	/* Waits while card is busy */

struct mmc_host {
	int			index;
	const struct mmc_host_ops *ops;/*mmc主机操作的接口函数*/
	unsigned int		f_min;
	unsigned int		f_max;/*sdio速率*/
	u32			ocr_avail;/*sdio主机电压支持范围*/

	unsigned long		caps;		/* SDIO主控制器的功能属性 */


	/* host specific block data */
	unsigned int		max_seg_size;	/* 块传输最大分片尺寸*/
	unsigned short		max_hw_segs;	/* 块传输最大分片数目 */
	unsigned short		max_phys_segs;	/* 块传输最大支持的物理分片数目 */
	unsigned short		unused;
	unsigned int		max_req_size;	/* 请求最大的数据长度 */
	unsigned int		max_blk_size;	/*一个mmc请求块最大尺寸*/
	unsigned int		max_blk_count;	/*最大的请求块数目*/

	/* private data */
	//spinlock_t		lock;		/* lock for claim and bus ops */

	struct mmc_ios		ios;		/*当前mmc的io设置状态*/
	u32			ocr;		/* 当前电压设置*/

	/* group bitfields together to minimize padding */
	unsigned int		use_spi_crc:1;
	unsigned int		claimed:1;	/* host exclusively claimed */
	unsigned int		bus_dead:1;	/* bus has been released */
#ifdef CONFIG_MMC_DEBUG
	unsigned int		removed:1;	/* host is being removed */
#endif

	/* Only used with MMC_CAP_DISABLE (本驱动不具备一下属性)*/
	int			enabled;	/* host is enabled */
	int			nesting_cnt;	/* "enable" nesting count */
	int			en_dis_recurs;	/* detect recursion */
	unsigned int		disable_delay;	/* disable delay in msecs */
	//struct delayed_work	disable;	/* disabling work */

	struct mmc_card		*card;		/* 枚举到的mmc卡的数据链表 */

//	wait_queue_head_t	wq;
//	struct task_struct	*claimer;	/* task that has host claimed */
	int			claim_cnt;	/* "claim" nesting count */

//	struct delayed_work	detect;

	const struct mmc_bus_ops *bus_ops;	/* mmc主机总线驱动 */
	unsigned int		bus_refs;	/*未使用*/

	unsigned int		sdio_irqs;
//	struct task_struct	*sdio_irq_thread;
//	atomic_t		sdio_irq_thread_abort;

#ifdef CONFIG_LEDS_TRIGGERS
	struct led_trigger	*led;		/* activity led */
#endif

//	struct dentry		*debugfs_root;

//	unsigned long		private[0] ____cacheline_aligned;

	struct stm32_host private;/*指向对应的主机控制器*/
};

#define mmc_host_is_spi(host)	((host)->caps & MMC_CAP_SPI)

#define mmc_dev(x)	((x)->parent)
#define mmc_classdev(x)	(&(x)->class_dev)
//#define mmc_hostname(x)	(dev_name(&(x)->class_dev))
#define mmc_hostname(x)	("wifi")


/*
static inline void *mmc_priv(struct mmc_host *host)
{
	return (void *)(&(host->private));
} */
static void mmc_signal_sdio_irq(struct mmc_host *host)
{
	host->ops->enable_sdio_irq(host, 0);
	sdio_irq_thread(host);
}

#define mmc_priv(host)  (struct stm32_host *)(&(host->private))
/*#define mmc_signal_sdio_irq(host) 	do{
									  host->ops->enable_sdio_irq(host, 0);
										sdio_irq_thread(host);
								    }while(0)	  */
#endif
