#ifndef __MARVEL_MAIN__H__
#define __MARVEL_MAIN__H__

#include "type.h"
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
#include "wireless.h"

void lbs_notify_command_response(struct lbs_private *priv, u8 resp_idx);
void lbs_host_to_card_done(struct lbs_private *priv);
 int lbs_thread(struct lbs_private *priv);//主线程，处理所有固件生成的envent、接收和发送数据以及执行网卡命令
struct lbs_private *lbs_add_card(void *card);
int lbs_start_card(struct lbs_private *priv);
int lbs_set_regiontable(struct lbs_private *priv, u8 region, u8 band);
void lbs_scan_worker(struct lbs_private *priv);
void lbs_queue_event(struct lbs_private *priv, u32 event);
u32 lbs_fw_index_to_data_rate(u8 idx);



#endif

