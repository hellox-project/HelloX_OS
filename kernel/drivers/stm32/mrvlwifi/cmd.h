#ifndef __CMD__H__
#define __CMD__H__

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
#include "hostcmd.h"
#include "wireless.h"
#include "dev.h"

struct lbs_private;
struct cmd_header;

#define get_unaligned_le16(val) ((__le16)(*val)) 

int lbs_cmd_copyback(struct lbs_private *priv, unsigned long extra,struct cmd_header *resp);
int __lbs_cmd(struct lbs_private *priv, uint16_t command,
	      struct cmd_header *in_cmd, int in_cmd_size,
	      int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      unsigned long callback_arg);

/* lbs_cmd() infers the size of the buffer to copy data back into, from
   the size of the target of the pointer. Since the command to be sent 
   may often be smaller, that size is set in cmd->size by the caller.*/
  //异步提交要执行的命令
/*
 #define lbs_cmd(priv, cmdnr, cmd, cb, cb_arg)		{\
	uint16_t __sz = le16_to_cpu((cmd)->hdr.size);		\
	(cmd)->hdr.size = cpu_to_le16(sizeof(*(cmd)));		\
	__lbs_cmd(priv, cmdnr, &(cmd)->hdr, __sz, cb, cb_arg);}*/
	
	
/*这个地方有个漏洞，本处定义的函数无法完成宏定义的所有功能，所以去掉
(cmd)->hdr.size = cpu_to_le16(sizeof(*(cmd)));		
*/

struct void_cmd_head{
	struct cmd_header hdr;
};
int  lbs_cmd(struct lbs_private *priv, uint16_t cmdnr, struct void_cmd_head* cmd,
			 int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *),
	      		unsigned long callback_arg);
int  lbs_cmd_with_response(struct lbs_private *priv, uint16_t cmdnr,void *cmd);
/*#define lbs_cmd(priv, cmdnr, cmd, cb, cb_arg)	({\
	uint16_t __sz = le16_to_cpu((cmd)->hdr.size);\
	(cmd)->hdr.size = cpu_to_le16(sizeof(*(cmd)));\
	__lbs_cmd(priv, cmdnr, &(cmd)->hdr, __sz, cb, cb_arg);\
})

#define lbs_cmd_with_response(priv, cmdnr, cmd)\
	lbs_cmd(priv, cmdnr, cmd, lbs_cmd_copyback, (unsigned long) (cmd))*/


int lbs_allocate_cmd_buffer(struct lbs_private *priv);

int lbs_update_hw_spec(struct lbs_private *priv);

struct lbs_private;
struct cmd_ctrl_node;
void lbs_complete_command(struct lbs_private *priv, struct cmd_ctrl_node *cmd,int result);
int lbs_process_command_response(struct lbs_private *priv, u8 *data, u32 len);
int lbs_execute_next_command(struct lbs_private *priv);

int lbs_get_tx_power(struct lbs_private *priv, s16 *curlevel, s16 *minlevel,
		     s16 *maxlevel);
void lbs_cmd_async(struct lbs_private *priv, uint16_t command,
	struct cmd_header *in_cmd, int in_cmd_size);
void lbs_set_mac_control(struct lbs_private *priv);
int lbs_update_channel(struct lbs_private *priv);
int lbs_set_snmp_mib(struct lbs_private *priv, u32 oid, u16 val);
int lbs_set_channel(struct lbs_private *priv, u8 channel);
int lbs_cmd_802_11_set_wep(struct lbs_private *priv, uint16_t cmd_action,
			   struct assoc_request *assoc);
int lbs_cmd_802_11_enable_rsn(struct lbs_private *priv, uint16_t cmd_action,
			      uint16_t *enable);

int lbs_set_radio(struct lbs_private *priv, u8 preamble, u8 radio_on);



int lbs_prepare_and_send_command(struct lbs_private *priv,
			  u16 cmd_no,
			  u16 cmd_action,
			  u16 wait_option, u32 cmd_oid, void *pdata_buf);

int lbs_cmd_802_11_key_material(struct lbs_private *priv, uint16_t cmd_action,
				struct assoc_request *assoc);


#endif



