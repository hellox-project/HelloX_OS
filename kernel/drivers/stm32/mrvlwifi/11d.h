#ifndef _LBS_11D_
#define _LBS_11D_

#include "type.h"
#include "mdef.h"
#include "dev.h"
#include "common.h"
#include "types.h"



#define UNIVERSAL_REGION_CODE			0xff

/** (Beaconsize(256)-5(IEId,len,contrystr(3))/3(FirstChan,NoOfChan,MaxPwr)
 */
#define MRVDRV_MAX_SUBBAND_802_11D		83

#define COUNTRY_CODE_LEN			3
#define MAX_NO_OF_CHAN 				40

struct cmd_ds_command;


#pragma pack(1)
/** Data structure for Country IE*/
struct ieee_subbandset {
	u8 firstchan;
	u8 nrchan;
	u8 maxtxpwr;
} __attribute__((packed));
#pragma pack()


struct ieee_ie_country_info_set {
 	struct ieee_ie_header header;
	u8 countrycode[COUNTRY_CODE_LEN];
	struct ieee_subbandset subband[1];
};
#pragma pack(1)
struct mrvl_ie_domain_param_set {
	struct mrvl_ie_header header;
	u8 countrycode[COUNTRY_CODE_LEN];
	struct ieee_subbandset subband[1];
} __attribute__((packed));

struct cmd_ds_802_11d_domain_info {
	__le16 action;
	struct mrvl_ie_domain_param_set domain;
} __attribute__((packed));

#pragma pack()


/** domain regulatory information */
struct lbs_802_11d_domain_reg {
	/** country Code*/
	u8 countrycode[COUNTRY_CODE_LEN];
	/** No. of subband*/
	u8 nr_subband;
	struct ieee_subbandset subband[MRVDRV_MAX_SUBBAND_802_11D];
};


#pragma pack(1)
struct chan_power_11d {
	u8 chan;
	u8 pwr;
} __attribute__((packed));

struct parsed_region_chan_11d {
	u8 band;
	u8 region;
	s8 countrycode[COUNTRY_CODE_LEN];
	struct chan_power_11d chanpwr[MAX_NO_OF_CHAN];
	u8 nr_chan;
} __attribute__((packed));

#pragma pack()

struct region_code_mapping {
	u8 region[COUNTRY_CODE_LEN];
	u8 code;
};


#pragma pack(1)
struct ieee_ie_country_info_full_set {
	 struct ieee_ie_header header;
	 u8 countrycode[COUNTRY_CODE_LEN];
	 struct ieee_subbandset subband[MRVDRV_MAX_SUBBAND_802_11D];
} __attribute__((packed));
#pragma pack()

struct lbs_private;
struct bss_descriptor;
int lbs_set_universaltable(struct lbs_private *priv, u8 band);
void lbs_init_11d(struct lbs_private *priv);
int lbs_cmd_802_11d_domain_info(struct lbs_private *priv,
				 struct cmd_ds_command *cmd, u16 cmdno,
				 u16 cmdOption);
int lbs_ret_802_11d_domain_info(struct cmd_ds_command *resp);
int lbs_parse_dnld_countryinfo_11d(struct lbs_private *priv,
                                        struct bss_descriptor * bss);
int lbs_create_dnld_countryinfo_11d(struct lbs_private *priv);
int lbs_parse_dnld_countryinfo_11d(struct lbs_private *priv,struct bss_descriptor * bss);


#endif

