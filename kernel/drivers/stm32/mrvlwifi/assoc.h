#ifndef __ASSOC__H__
#define __ASSOC__H__

#include "type.h"
#include "common.h"
#include "types.h"
#include "dev.h"
#include "assoc.h"

int test_bit(int nr, const unsigned long *vaddr);
void set_bit(int nr,unsigned long *vaddr);
void clear_bit(int nr,unsigned long *vaddr);


static int get_common_rates(struct lbs_private *priv,
	u8 *rates,
	u16 *rates_size);
static void lbs_set_basic_rate_flags(u8 *rates, size_t len);
static u8 iw_auth_to_ieee_auth(u8 auth);
static int lbs_set_authentication(struct lbs_private *priv, u8 bssid[6], u8 auth);
static int lbs_assoc_post(struct lbs_private *priv,
			  struct cmd_ds_802_11_associate_response *resp);
static int lbs_associate(struct lbs_private *priv,
			 struct assoc_request *assoc_req,
			 u16 command);
static int lbs_try_associate(struct lbs_private *priv,
	struct assoc_request *assoc_req);
static int is_network_compatible(struct lbs_private *priv,
				 struct bss_descriptor *bss, uint8_t mode);
static struct bss_descriptor *lbs_find_bssid_in_list(struct lbs_private *priv,
					      uint8_t *bssid, uint8_t mode);
static int assoc_helper_bssid(struct lbs_private *priv,
                              struct assoc_request * assoc_req);
 int lbs_ssid_cmp(uint8_t *ssid1, uint8_t ssid1_len, uint8_t *ssid2,
		 uint8_t ssid2_len);
static struct bss_descriptor *lbs_find_ssid_in_list(struct lbs_private *priv,
					     uint8_t *ssid, uint8_t ssid_len,
					     uint8_t *bssid, uint8_t mode,
					     int channel);
int lbs_send_specific_ssid_scan(struct lbs_private *priv, uint8_t *ssid,
				uint8_t ssid_len);

static int assoc_helper_essid(struct lbs_private *priv,
                              struct assoc_request * assoc_req);
static int assoc_helper_associate(struct lbs_private *priv,
                                  struct assoc_request * assoc_req);
static int assoc_helper_mode(struct lbs_private *priv,
                             struct assoc_request * assoc_req);
static int assoc_helper_channel(struct lbs_private *priv,
                                struct assoc_request * assoc_req);

static int assoc_helper_wep_keys(struct lbs_private *priv,
				 struct assoc_request *assoc_req);
static int assoc_helper_secinfo(struct lbs_private *priv,
                                struct assoc_request * assoc_req);
static int assoc_helper_wpa_ie(struct lbs_private *priv,
                               struct assoc_request * assoc_req);
static int lbs_adhoc_join(struct lbs_private *priv,
	struct assoc_request *assoc_req);
static int lbs_adhoc_start(struct lbs_private *priv,
	struct assoc_request *assoc_req);
int lbs_adhoc_stop(struct lbs_private *priv);
static int lbs_adhoc_post(struct lbs_private *priv,
			  struct cmd_ds_802_11_ad_hoc_result *resp);

static struct bss_descriptor *lbs_find_best_ssid_in_list(
	struct lbs_private *priv, uint8_t mode);
static int lbs_find_best_network_ssid(struct lbs_private *priv,
	uint8_t *out_ssid, uint8_t *out_ssid_len, uint8_t preferred_mode,
	uint8_t *out_mode);
static int should_deauth_infrastructure(struct lbs_private *priv,
                                        struct assoc_request * assoc_req);
int lbs_ssid_cmp(uint8_t *ssid1, uint8_t ssid1_len, uint8_t *ssid2,
		 uint8_t ssid2_len);

static int should_stop_adhoc(struct lbs_private *priv,
                             struct assoc_request * assoc_req);
void lbs_association_worker(struct lbs_private *priv);
int lbs_cmd_80211_deauthenticate(struct lbs_private *priv, u8 bssid[ETH_ALEN],
				 u16 reason);
 int lbs_scan_networks(struct lbs_private *priv, int full_scan);
 void lbs_scan_worker(struct lbs_private *priv);
 
 int lbs_set_encodeext(struct lbs_private *priv,
				   //struct iw_request_info *info,
				   struct iw_point *dwrq,char *extra,
				   struct assoc_request * assoc_req);
 void marvel_assoc_wpa_network(struct lbs_private *priv,
 char *ssid,char *key);
 void marvel_assoc_open_network(struct lbs_private *priv,
	 char *ssid,char *key,char mode,int channel);

#endif
