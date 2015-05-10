#include "type.h"
#include "common.h"
#include "types.h"
#include "hostcmdcode.h"
#include "dev.h"
#include "mac80211.h"
#include "cmd.h"

#define list_for_each_entry_bssdes(pos, head, member)                 \
	for (pos = list_entry((head)->next,struct bss_descriptor, member);	\
	&pos->member != (head);                                             \
	pos = list_entry(pos->member.next,struct bss_descriptor, member))

#define LBS_IOCTL_USER_SCAN_CHAN_MAX  50
//! Approximate amount of data needed to pass a scan result back to iwlist
#define MAX_SCAN_CELL_SIZE  (IW_EV_ADDR_LEN             \
                             + IW_ESSID_MAX_SIZE        \
                             + IW_EV_UINT_LEN           \
                             + IW_EV_FREQ_LEN           \
                             + IW_EV_QUAL_LEN           \
                             + IW_ESSID_MAX_SIZE        \
                             + IW_EV_PARAM_LEN          \
                             + 40)	/* 40 for WPAIE */

//! Memory needed to store a max sized channel List TLV for a firmware scan
#define CHAN_TLV_MAX_SIZE  (sizeof(struct mrvl_ie_header)              \
  + (MRVDRV_MAX_CHANNELS_PER_SCAN * sizeof(struct chanscanparamset)))

//! Memory needed to store a max number/size SSID TLV for a firmware scan
#define SSID_TLV_MAX_SIZE  (1 * sizeof(struct mrvl_ie_ssid_param_set))

//! Maximum memory needed for a cmd_ds_802_11_scan with all TLVs at max
#define MAX_SCAN_CFG_ALLOC (sizeof(struct cmd_ds_802_11_scan)	\
  + CHAN_TLV_MAX_SIZE + SSID_TLV_MAX_SIZE)

//! The maximum number of channels the firmware can scan per command
#define MRVDRV_MAX_CHANNELS_PER_SCAN   14

/**
 * @brief Number of channels to scan per firmware scan command issuance.
 *
 *  Number restricted to prevent hitting the limit on the amount of scan data
 *  returned in a single firmware scan command.
 */
#define MRVDRV_CHANNELS_PER_SCAN_CMD   4    //限制驱动一次scan命令返回扫描数据的个数

//! Scan time specified in the channel TLV for each channel for passive scans
#define MRVDRV_PASSIVE_SCAN_CHAN_TIME  100

//! Scan time specified in the channel TLV for each channel for active scans
#define MRVDRV_ACTIVE_SCAN_CHAN_TIME   100

#define DEFAULT_MAX_SCAN_AGE (15 * HZ)

/*********************************************************************/
/*                                                                   */
/*  Main scanning support                                            */
/*                                                                   */
/*********************************************************************/

/**
 *  @brief Create a channel list for the driver to scan based on region info
 *
 *  Only used from lbs_scan_setup_scan_config()
 *
 *  Use the driver region/band information to construct a comprehensive list
 *    of channels to scan.  This routine is used for any scan that is not
 *    provided a specific channel list to scan.
 *
 *  @param priv          A pointer to struct lbs_private structure
 *  @param scanchanlist  Output parameter: resulting channel list to scan
 *
 *  @return              void
 */
//struct lbs_private;
//struct chanscanparamset;
static int lbs_scan_create_channel_list(struct lbs_private *priv,
					struct chanscanparamset *scanchanlist)
{
	struct region_channel *scanregion;
	struct chan_freq_power *cfp;
	u32 rgnidx;
	int chanidx;
	int nextchan;
	uint8_t scantype;

	chanidx = 0;

	/* Set the default scan type to the user specified type, will later
	 *   be changed to passive on a per channel basis if restricted by
	 *   regulatory requirements (11d or 11h)
	 */
	scantype = CMD_SCAN_TYPE_ACTIVE;

	 for (rgnidx = 0; rgnidx < ARRAY_SIZE(priv->region_channel); rgnidx++) {//区域的所有信道
	//	for (rgnidx = 0; rgnidx < MAX_REGION_CHANNEL_NUM; rgnidx++) {

		#ifdef ENABLE_80211D_DEBUG
		if (priv->enable11d && (priv->connect_status != LBS_CONNECTED)
		    && (priv->mesh_connect_status != LBS_CONNECTED)) {
			/* Scan all the supported chan for the first scan */
			if (!priv->universal_channel[rgnidx].valid)
				continue;
			scanregion = &priv->universal_channel[rgnidx];

			/* clear the parsed_region_chan for the first scan */
			memset(&priv->parsed_region_chan, 0x00,
			       sizeof(priv->parsed_region_chan));
		} 
		#else 
			if (!priv->region_channel[rgnidx].valid)
				continue;
			scanregion = &priv->region_channel[rgnidx];
	
		#endif

		for (nextchan = 0; nextchan < scanregion->nrcfp; nextchan++, chanidx++) {
			struct chanscanparamset *chan = &scanchanlist[chanidx];//返回的扫描通道链表

			cfp = scanregion->CFP + nextchan;

			/*if (priv->enable11d)//disable 802.11d
				scantype = lbs_get_scan_type_11d(cfp->channel,
								 &priv->parsed_region_chan);*/

			if (scanregion->band == BAND_B || scanregion->band == BAND_G)
				chan->radiotype = CMD_SCAN_RADIO_TYPE_BG;

			if (scantype == CMD_SCAN_TYPE_PASSIVE) {
				chan->maxscantime = cpu_to_le16(MRVDRV_PASSIVE_SCAN_CHAN_TIME);
				chan->chanscanmode.passivescan = 1;
			} else {
				chan->maxscantime = cpu_to_le16(MRVDRV_ACTIVE_SCAN_CHAN_TIME);
				chan->chanscanmode.passivescan = 0;
			}

			chan->channumber = cfp->channel;
		}
	}
	return chanidx;
}

/*
 * Add SSID TLV of the form:
 *
 * TLV-ID SSID     00 00
 * length          06 00
 * ssid            4d 4e 54 45 53 54
 */
static int lbs_scan_add_ssid_tlv(struct lbs_private *priv, u8 *tlv)
{
	struct mrvl_ie_ssid_param_set *ssid_tlv = (void *)tlv;

	ssid_tlv->header.type = cpu_to_le16(TLV_TYPE_SSID);
	ssid_tlv->header.len = cpu_to_le16(priv->scan_ssid_len);
	memcpy(ssid_tlv->ssid, priv->scan_ssid, priv->scan_ssid_len);
	return sizeof(ssid_tlv->header) + priv->scan_ssid_len;
}

/*
 * Add CHANLIST TLV of the form
 *
 * TLV-ID CHANLIST 01 01
 * length          5b 00
 * channel 1       00 01 00 00 00 64 00
 *   radio type    00
 *   channel          01
 *   scan type           00
 *   min scan time          00 00
 *   max scan time                64 00
 * channel 2       00 02 00 00 00 64 00
 * channel 3       00 03 00 00 00 64 00
 * channel 4       00 04 00 00 00 64 00
 * channel 5       00 05 00 00 00 64 00
 * channel 6       00 06 00 00 00 64 00
 * channel 7       00 07 00 00 00 64 00
 * channel 8       00 08 00 00 00 64 00
 * channel 9       00 09 00 00 00 64 00
 * channel 10      00 0a 00 00 00 64 00
 * channel 11      00 0b 00 00 00 64 00
 * channel 12      00 0c 00 00 00 64 00
 * channel 13      00 0d 00 00 00 64 00
 *
 */
static int lbs_scan_add_chanlist_tlv(uint8_t *tlv,
				     struct chanscanparamset *chan_list,
				     int chan_count)
{
	size_t size = sizeof(struct chanscanparamset) *chan_count;
	struct mrvl_ie_chanlist_param_set *chan_tlv = (void *)tlv;

	chan_tlv->header.type = cpu_to_le16(TLV_TYPE_CHANLIST);
	memcpy(chan_tlv->chanscanparam, chan_list, size);
	chan_tlv->header.len = cpu_to_le16(size);
	return sizeof(chan_tlv->header) + size;
}

extern u8 lbs_bg_rates[MAX_RATES];

/*
 * Add RATES TLV of the form
 *
 * TLV-ID RATES    01 00
 * length          0e 00
 * rates           82 84 8b 96 0c 12 18 24 30 48 60 6c
 *
 * The rates are in lbs_bg_rates[], but for the 802.11b
 * rates the high bit isn't set. lbs_scan_add_rates_tlv
 */
static int lbs_scan_add_rates_tlv(uint8_t *tlv)
{
	int i;
	struct mrvl_ie_rates_param_set *rate_tlv = (void *)tlv;

	rate_tlv->header.type = cpu_to_le16(TLV_TYPE_RATES);
	tlv += sizeof(rate_tlv->header);
	for (i = 0; i < MAX_RATES; i++) {
		*tlv = lbs_bg_rates[i];
		if (*tlv == 0)
			break;
		/* This code makes sure that the 802.11b rates (1 MBit/s, 2
		   MBit/s, 5.5 MBit/s and 11 MBit/s get's the high bit set.
		   Note that the values are MBit/s * 2, to mark them as
		   basic rates so that the firmware likes it better */
		if (*tlv == 0x02 || *tlv == 0x04 ||
		    *tlv == 0x0b || *tlv == 0x16)
			*tlv |= 0x80;
		tlv++;
	}
	rate_tlv->header.len = cpu_to_le16(i);
	return sizeof(rate_tlv->header) + i;
}


static __inline void clear_bss_descriptor(struct bss_descriptor *bss)
{
	/* Don't blow away ->list, just BSS data */
	memset(bss, 0, offsetof(struct bss_descriptor, list));
}

static __inline unsigned compare_ether_addr(const u8 *addr1, const u8 *addr2)
{
	const u16 *a = (const u16 *) addr1;
	const u16 *b = (const u16 *) addr2;

	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) != 0;
}

static __inline int is_same_network(struct bss_descriptor *src,
				  struct bss_descriptor *dst)
{
	/* A network is only a duplicate if the channel, BSSID, and ESSID
	 * all match.  We treat all <hidden> with the same BSSID and channel
	 * as one network */
	return ((src->ssid_len == dst->ssid_len) &&
		(src->channel == dst->channel) &&
		!compare_ether_addr(src->bssid, dst->bssid) &&
		!memcmp(src->ssid, dst->ssid, src->ssid_len));
}

/*********************************************************************/
/*                                                                   */
/*  Misc helper functions                                            */
/*                                                                   */
/*********************************************************************/

/**
 *  @brief Unsets the MSB on basic rates
 *
 * Scan through an array and unset the MSB for basic data rates.
 *
 *  @param rates     buffer of data rates
 *  @param len       size of buffer
 */
static void lbs_unset_basic_rate_flags(u8 *rates, size_t len)
{
	int i;

	for (i = 0; i < len; i++)
		rates[i] &= 0x7f;
}

/*********************************************************************/
/*                                                                   */
/*  Result interpretation                                            */
/*                                                                   */
/*********************************************************************/

/**
 *  @brief Interpret a BSS scan response returned from the firmware
 *
 *  Parse the various fixed fields and IEs passed back for a a BSS probe
 *  response or beacon from the scan command.  Record information as needed
 *  in the scan table struct bss_descriptor for that entry.
 *
 *  @param bss  Output parameter: Pointer to the BSS Entry
 *
 *  @return             0 or -1
 */
 #define DECLARE_SSID_BUF(var) char var[IEEE80211_MAX_SSID_LEN * 4 + 1]

static int lbs_process_bss(struct bss_descriptor *bss,
			   uint8_t **pbeaconinfo, int *bytesleft)
{
	struct ieee_ie_fh_param_set *fh;
	struct ieee_ie_ds_param_set *ds;
	struct ieee_ie_cf_param_set *cf;
	struct ieee_ie_ibss_param_set *ibss;
	
//	DECLARE_SSID_BUF(ssid);
	struct ieee_ie_country_info_set *pcountryinfo;
	
	uint8_t *pos, *end, *p;
	uint8_t n_ex_rates = 0, got_basic_rates = 0, n_basic_rates = 0;
	uint16_t beaconsize = 0;
	int ret;

	lbs_deb_enter(LBS_DEB_SCAN);

	if (*bytesleft >= sizeof(beaconsize)) {
		/* Extract & convert beacon size from the command buffer */
		uint16_t *ptmp_data=(uint16_t *)*pbeaconinfo;
		beaconsize = get_unaligned_le16(ptmp_data);
		lbs_deb_scan("beaconsize=%d\n",beaconsize);
		*bytesleft -= sizeof(beaconsize);
		*pbeaconinfo += sizeof(beaconsize);
	}

	if (beaconsize == 0 || beaconsize > *bytesleft) {
		*pbeaconinfo += *bytesleft;
		*bytesleft = 0;
		ret = -1;
		goto done;
	}

	/* Initialize the current working beacon pointer for this BSS iteration */
	pos = *pbeaconinfo;
	end = pos + beaconsize;

	/* Advance the return beacon pointer past the current beacon */
	*pbeaconinfo += beaconsize;
	*bytesleft -= beaconsize;

	memcpy(bss->bssid, pos, ETH_ALEN);
	lbs_deb_scan("process_bss: BSSID %2x:%2x:%2x:%2x:%2x:%2x\n", 
		bss->bssid[0],bss->bssid[1],
		bss->bssid[2],bss->bssid[3],
		bss->bssid[4],bss->bssid[5]);
	pos += ETH_ALEN;

	if ((end - pos) < 12) {
		lbs_deb_scan("process_bss: Not enough bytes left\n");
		ret = -1;
		goto done;
	}

	/*
	 * next 4 fields are RSSI, time stamp, beacon interval,
	 *   and capability information
	 */

	/* RSSI is 1 byte long */
	bss->rssi = *pos;
	lbs_deb_scan("process_bss: RSSI %d\n", *pos);
	pos++;

	/* time stamp is 8 bytes long */
	pos += 8;

	/* beacon interval is 2 bytes long */
	bss->beaconperiod = get_unaligned_le16(pos);
	pos += 2;

	/* capability information is 2 bytes long */
	bss->capability = get_unaligned_le16(pos);
	lbs_deb_scan("process_bss: capabilities 0x%04x\n", bss->capability);
	pos += 2;

	if (bss->capability & WLAN_CAPABILITY_PRIVACY)
		lbs_deb_scan("process_bss: WEP enabled\n");
	else
		lbs_deb_scan("process_bss: WEP off\n");
	if (bss->capability & WLAN_CAPABILITY_IBSS){
		bss->mode = IW_MODE_ADHOC;
		lbs_deb_scan("mode:adhoc");
	}
	
	else{
		bss->mode = IW_MODE_INFRA;
		lbs_deb_scan("mode:manage");
	}

	/* rest of the current buffer are IE's */
	lbs_deb_scan("process_bss: IE len %zd\n", end - pos);
	//lbs_deb_hex(LBS_DEB_SCAN, "process_bss: IE info", pos, end - pos);

	/* process variable IE */
	while (pos <= end - 2) {
		if (pos + pos[1] > end) {
			lbs_deb_scan("process_bss: error in processing IE, "
				     "bytes left < IE length\n");
			break;
		}

	switch (pos[0]) {
		case WLAN_EID_SSID:
			bss->ssid_len = min(IEEE80211_MAX_SSID_LEN, pos[1]);
			memcpy(bss->ssid, pos + 2, bss->ssid_len);
			lbs_deb_scan("got SSID IE: '%s', len %u\n",
			             //print_ssid(ssid,(const char *)bss->ssid, bss->ssid_len),
			             bss->ssid,
			             bss->ssid_len);
			break;

		case WLAN_EID_SUPP_RATES:
			n_basic_rates = min(MAX_RATES, pos[1]);
			memcpy(bss->rates, pos + 2, n_basic_rates);
			got_basic_rates = 1;
			lbs_deb_scan("got RATES IE\n");
			break;

		case WLAN_EID_FH_PARAMS:
			fh = (struct ieee_ie_fh_param_set *) pos;
			memcpy(&bss->phy.fh, fh, sizeof(*fh));
			lbs_deb_scan("got FH IE\n");
			break;

		case WLAN_EID_DS_PARAMS:
			ds = (struct ieee_ie_ds_param_set *) pos;
			bss->channel = ds->channel;
			memcpy(&bss->phy.ds, ds, sizeof(*ds));
			lbs_deb_scan("got DS IE, channel %d\n", bss->channel);
			break;

		case WLAN_EID_CF_PARAMS:
			cf = (struct ieee_ie_cf_param_set *) pos;
			memcpy(&bss->ss.cf, cf, sizeof(*cf));
			lbs_deb_scan("got CF IE\n");
			break;

		case WLAN_EID_IBSS_PARAMS:
			ibss = (struct ieee_ie_ibss_param_set *) pos;
			bss->atimwindow = ibss->atimwindow;
			memcpy(&bss->ss.ibss, ibss, sizeof(*ibss));
			lbs_deb_scan("got IBSS IE\n");
			break;

		case WLAN_EID_COUNTRY:
			pcountryinfo = (struct ieee_ie_country_info_set *) pos;
			lbs_deb_scan("got COUNTRY IE\n");
			if (pcountryinfo->header.len < sizeof(pcountryinfo->countrycode)
			    || pcountryinfo->header.len > 254) {
				lbs_deb_scan("%s: 11D- Err CountryInfo len %d, min %zd, max 254\n",
					     __func__,
					     pcountryinfo->header.len,
					     sizeof(pcountryinfo->countrycode));
				ret = -1;
				goto done;
		  }

		  memcpy(&bss->countryinfo, pcountryinfo,
				pcountryinfo->header.len + 2);
			//lbs_deb_hex(LBS_DEB_SCAN, "process_bss: 11d countryinfo",
			//	    (uint8_t *) pcountryinfo,
			//	    (int) (pcountryinfo->header.len + 2));
		  break;

		case WLAN_EID_EXT_SUPP_RATES:
			/* only process extended supported rate if data rate is
			 * already found. Data rate IE should come before
			 * extended supported rate IE
			 */
			lbs_deb_scan("got RATESEX IE\n");
			if (!got_basic_rates) {
				lbs_deb_scan("... but ignoring it\n");
				break;
			}

			n_ex_rates = pos[1];
			if (n_basic_rates + n_ex_rates > MAX_RATES)
				n_ex_rates = MAX_RATES - n_basic_rates;

			p = bss->rates + n_basic_rates;
			memcpy(p, pos + 2, n_ex_rates);
			break;

		case WLAN_EID_GENERIC:
			if (pos[1] >= 4 &&
			    pos[2] == 0x00 && pos[3] == 0x50 &&
			    pos[4] == 0xf2 && pos[5] == 0x01) {
				bss->wpa_ie_len = min(pos[1] + 2, MAX_WPA_IE_LEN);
				memcpy(bss->wpa_ie, pos, bss->wpa_ie_len);
				lbs_deb_scan("got WPA IE\n");
			/*	lbs_deb_hex(LBS_DEB_SCAN, "WPA IE", bss->wpa_ie,
					    bss->wpa_ie_len);*/
			} else if (pos[1] >= MARVELL_MESH_IE_LENGTH &&
				   pos[2] == 0x00 && pos[3] == 0x50 &&
				   pos[4] == 0x43 && pos[5] == 0x04) {
				lbs_deb_scan("got mesh IE\n");
				bss->mesh = 1;
			} else {
				lbs_deb_scan("got generic IE: %02x:%02x:%02x:%02x, len %d\n",
					pos[2], pos[3],
					pos[4], pos[5],
					pos[1]);
			}
			break;

		case WLAN_EID_RSN:
			lbs_deb_scan("got RSN IE\n");
			bss->rsn_ie_len = min(pos[1] + 2, MAX_WPA_IE_LEN);
			memcpy(bss->rsn_ie, pos, bss->rsn_ie_len);
			/*lbs_deb_hex(LBS_DEB_SCAN, "process_bss: RSN_IE",
				    bss->rsn_ie, bss->rsn_ie_len);*/
			break;

		default:
			lbs_deb_scan("got IE 0x%04x, len %d\n",
				     pos[0], pos[1]);
			break;
		}

		pos += pos[1] + 2;
	}

	/* Timestamp */
	bss->last_scanned = jiffies;
	lbs_unset_basic_rate_flags(bss->rates, sizeof(bss->rates));

	ret = 0;

done:
	lbs_deb_leave_args(LBS_DEB_SCAN, ret);
	return ret;
}


/*********************************************************************/
/*                                                                   */
/*  Command execution                                                */
/*                                                                   */
/*********************************************************************/

/**
 *  @brief This function handles the command response of scan
 *
 *  Called from handle_cmd_response() in cmdrespc.
 *
 *   The response buffer for the scan command has the following
 *      memory layout:
 *
 *     .-----------------------------------------------------------.
 *     |  header (4 * sizeof(u16)):  Standard command response hdr |
 *     .-----------------------------------------------------------.
 *     |  bufsize (u16) : sizeof the BSS Description data          |
 *     .-----------------------------------------------------------.
 *     |  NumOfSet (u8) : Number of BSS Descs returned             |
 *     .-----------------------------------------------------------.
 *     |  BSSDescription data (variable, size given in bufsize)    |
 *     .-----------------------------------------------------------.
 *     |  TLV data (variable, size calculated using header->size,  |
 *     |            bufsize and sizeof the fixed fields above)     |
 *     .-----------------------------------------------------------.
 *
 *  @param priv    A pointer to struct lbs_private structure
 *  @param resp    A pointer to cmd_ds_command
 *
 *  @return        0 or -1
 */

static int lbs_ret_80211_scan(struct lbs_private *priv, unsigned long dummy,
			      struct cmd_header *resp)
{
	struct cmd_ds_802_11_scan_rsp *scanresp = (void *)resp;
	struct bss_descriptor *iter_bss;
	//struct bss_descriptor *safe;
	uint8_t *bssinfo;
 	uint16_t scanrespsize;
	int bytesleft;
	int idx;
 	int tlvbufsize;
	int ret;
	struct bss_descriptor *new    = NULL;
	struct bss_descriptor *found  = NULL;
	struct bss_descriptor *oldest = NULL;	

	lbs_deb_enter(LBS_DEB_SCAN);

	/* Prune old entries from scan table */
	//这个SSID有一个生存时间
	 /*list_for_each_entry_safe (iter_bss, safe, &priv->network_list, list) {
		unsigned long stale_time = iter_bss->last_scanned + DEFAULT_MAX_SCAN_AGE;
		if (time_before(jiffies, stale_time))
			continue;
		list_move_tail (&iter_bss->list, &priv->network_free_list);
		clear_bss_descriptor(iter_bss);
	} */

	if (scanresp->nr_sets > MAX_NETWORK_COUNT) {
		lbs_deb_scan("SCAN_RESP: too many scan results (%d, max %d)\n",
			     scanresp->nr_sets, MAX_NETWORK_COUNT);
		ret = -1;
		goto done;
	}

	bytesleft = get_unaligned_le16(&scanresp->bssdescriptsize);
	lbs_deb_scan("SCAN_RESP: bssdescriptsize %d\n", bytesleft);

	scanrespsize = le16_to_cpu(resp->size);
	lbs_deb_scan("SCAN_RESP: scan results %d\n", scanresp->nr_sets);

	bssinfo = scanresp->bssdesc_and_tlvbuffer;

	/* The size of the TLV buffer is equal to the entire command response
	 *   size (scanrespsize) minus the fixed fields (sizeof()'s), the
	 *   BSS Descriptions (bssdescriptsize as bytesLef) and the command
	 *   response header (S_DS_GEN)
	 */
 	tlvbufsize = scanrespsize - (bytesleft + sizeof(scanresp->bssdescriptsize)
				     + sizeof(scanresp->nr_sets)
				     + S_DS_GEN);	   

	/*
	 *  Process each scan response returned (scanresp->nr_sets). Save
	 *    the information in the newbssentry and then insert into the
	 *    driver scan table either as an update to an existing entry
	 *    or as an addition at the end of the table
	 */
	//Allocate heap memory to new scaned bss_descriptor.
	new = (struct bss_descriptor*)KMemAlloc(sizeof(struct bss_descriptor),KMEM_SIZE_TYPE_ANY);
	if(NULL == new)
	{
		goto done;
	}
	for (idx = 0; idx < scanresp->nr_sets && bytesleft; idx++) {
		/* Process the data fields and IEs returned for this BSS */
		memset(new, 0, sizeof (struct bss_descriptor));
		if (lbs_process_bss(new, &bssinfo, &bytesleft) != 0) {
			/* error parsing the scan response, skipped */
			lbs_deb_scan("SCAN_RESP: process_bss returned ERROR(%d)\n",tlvbufsize);
			continue;
		}

		/* Try to find this bss in the scan table */
		list_for_each_entry_bssdes(iter_bss, &priv->network_list, list) {
			if (is_same_network(iter_bss, new)) {
				found = iter_bss;
				break;
			}

			if ((oldest == NULL) ||
			    (iter_bss->last_scanned < oldest->last_scanned))
				oldest = iter_bss;
		}

		if (found) {
			/* found, clear it */
			clear_bss_descriptor(found);
		} else if (!list_empty(&priv->network_free_list)) {
			/* Pull one from the free list */
			found = list_entry(priv->network_free_list.next,
					   struct bss_descriptor, list);
			list_move_tail(&found->list, &priv->network_list);
		} else if (oldest) {
			/* If there are no more slots, expire the oldest */
			found = oldest;
			clear_bss_descriptor(found);
			list_move_tail(&found->list, &priv->network_list);
		} else {
			continue;
		}

		lbs_deb_scan("SCAN_RESP: BSSID %2x:%2x:%2x:%2x:%2x:%2x\n",
				new->bssid[0],new->bssid[1],new->bssid[2],new->bssid[3],
				new->bssid[4],new->bssid[5]);

		/* Copy the locally created newbssentry to the scan table */
		memcpy(found, new, offsetof(struct bss_descriptor, list));//链入
	}

	ret = 0;

done:
	if(new)  //Should free the memory.
	{
		KMemFree(new,KMEM_SIZE_TYPE_ANY,0);
	}
	lbs_deb_leave_args(LBS_DEB_SCAN, ret);
	return ret;
}

/*
 * Generate the CMD_802_11_SCAN command with the proper tlv
 * for a bunch of channels.
 */
static int lbs_do_scan(struct lbs_private *priv, uint8_t bsstype,
		       struct chanscanparamset *chan_list, int chan_count)
{
	int ret = -ENOMEM;
	u8* gscan_cmdbuf = NULL;  //[MAX_SCAN_CFG_ALLOC];
	struct cmd_ds_802_11_scan *scan_cmd;
	uint8_t *tlv;	/* pointer into our current, growing TLV storage area */

	lbs_deb_enter("\n");
	//debug_data_stream("chan param",(char *)chan_list,
	//	sizeof(struct chanscanparamset)*chan_count);
	lbs_deb_scan( "bsstype %d, chanlist[].chan %d, chan_count %d\n",
		bsstype, chan_list ? chan_list[0].channumber : -1,
		chan_count);
	
	//Allocate memory for gscan_cmdbuf.
	gscan_cmdbuf = (u8*)KMemAlloc(sizeof(u8) * MAX_SCAN_CFG_ALLOC,KMEM_SIZE_TYPE_ANY);
	if(NULL == gscan_cmdbuf)
	{
		_hx_printf("  lbs_do_scan: Allocate memory for gscan_cmdbuf failed.\r\n");
		goto out;
	}
	/* create the fixed part for scan command */
	/*scan_cmd = kzalloc(MAX_SCAN_CFG_ALLOC, GFP_KERNEL);
	if (scan_cmd == NULL)
		goto out;*/
	scan_cmd = (struct cmd_ds_802_11_scan *)gscan_cmdbuf;
	memset(scan_cmd,0,MAX_SCAN_CFG_ALLOC);

	tlv = scan_cmd->tlvbuffer;
	/* TODO: do we need to scan for a specific BSSID?
	memcpy(scan_cmd->bssid, priv->scan_bssid, ETH_ALEN); */
	scan_cmd->bsstype = bsstype;

	/* add TLVs */
	if (priv->scan_ssid_len)//指定扫描
		tlv += lbs_scan_add_ssid_tlv(priv, tlv);
	if (chan_list && chan_count)
		tlv += lbs_scan_add_chanlist_tlv(tlv, chan_list, chan_count);
	tlv += lbs_scan_add_rates_tlv(tlv);

	/* This is the final data we are about to send */
	scan_cmd->hdr.size = cpu_to_le16(tlv - (uint8_t *)scan_cmd);
	//lbs_deb_hex(LBS_DEB_SCAN, "SCAN_CMD", (void *)scan_cmd,
	//	    sizeof(*scan_cmd));
	//lbs_deb_hex(LBS_DEB_SCAN, "SCAN_TLV", scan_cmd->tlvbuffer,
	//	    tlv - scan_cmd->tlvbuffer);
	//debug_data_stream("scan command",(char *)&scan_cmd->hdr,
  //	le16_to_cpu(scan_cmd->hdr.size));
	ret= __lbs_cmd(priv, CMD_802_11_SCAN, &scan_cmd->hdr,
			le16_to_cpu(scan_cmd->hdr.size),
			lbs_ret_80211_scan, 0);
	
out:
	if(gscan_cmdbuf)
	{
		KMemFree(gscan_cmdbuf,KMEM_SIZE_TYPE_ANY,0);
	}
	lbs_deb_leave_args(LBS_DEB_SCAN, ret);
	return ret;
}


/**
 *  @brief Internal function used to start a scan based on an input config
 *
 *  Use the input user scan configuration information when provided in
 *    order to send the appropriate scan commands to firmware to populate or
 *    update the internal driver scan table
 *
 *  @param priv          A pointer to struct lbs_private structure
 *  @param full_scan     Do a full-scan (blocking)
 *
 *  @return              0 or < 0 if error
 */

const u8 const_chan_parm[14*7]={0x0,  0x1,  0x0,  0x0,  0x0,  0x64,  0x0,  0x0,  0x2,  0x0,  
0x0,  0x0,  0x64,  0x0,  0x0,  0x3,  0x0,  0x0,  0x0,  0x64,  
0x0,  0x0,  0x4,  0x0,  0x0,  0x0,  0x64,  0x0,
0x0,  0x5,  0x0,  0x0,  0x0,  0x64,  0x0,  0x0,  0x6,  0x0,  
0x0,  0x0,  0x64,  0x0,  0x0,  0x7,  0x0,  0x0,  0x0,  0x64,  
0x0,  0x0,  0x8,  0x0,  0x0,  0x0,  0x64,  0x0,  
0x0,  0x9,  0x0,  0x0,  0x0,  0x64,  0x0,  0x0,  0xa,  0x0,  
0x0,  0x0,  0x64,  0x0,  0x0,  0xb,  0x0,  0x0,  0x0,  0x64,  
0x0,  0x0,  0xc,  0x0,  0x0,  0x0,  0x64,  0x0,
0x0,  0xd,  0x0,  0x0,  0x0,  0x64,  0x0,  0x0,  0xe,  0x0,  
0x0,  0x0,  0x64,  0x0};


int lbs_scan_networks(struct lbs_private *priv, int full_scan)
{
	int ret = -ENOMEM;
	struct chanscanparamset *gmarvel_scan_param = NULL; //[LBS_IOCTL_USER_SCAN_CHAN_MAX];
	struct chanscanparamset *chan_list;
	struct chanscanparamset *curr_chans;
	int chan_count;
	uint8_t bsstype = CMD_BSS_TYPE_ANY;
	int numchannels = MRVDRV_CHANNELS_PER_SCAN_CMD;
	union iwreq_data wrqu;
	struct bss_descriptor *iter;
	int i = 0;
	static int first = 1;  //Indicate if the first time to call this routine,network list will be cleared if so.

	lbs_deb_enter_args(LBS_DEB_SCAN, full_scan);
	
	//Allocate memory for gmarvel_scan_param.
	gmarvel_scan_param = (struct chanscanparamset*)KMemAlloc( \
	    sizeof(struct chanscanparamset) * LBS_IOCTL_USER_SCAN_CHAN_MAX, \
	    KMEM_SIZE_TYPE_ANY);
	if(NULL == gmarvel_scan_param)
	{
		_hx_printf("  lbs_scan_networks: Allocate memory for gmarvel_scan_param failed.\r\n");
		goto out2;
	}

	/* Cancel any partial outstanding partial scans if this scan
	 * is a full scan.
	 */
/*	if (full_scan && delayed_work_pending(&priv->scan_work))
		cancel_delayed_work(&priv->scan_work);*/

	/* User-specified bsstype or channel list
	TODO: this can be implemented if some user-space application
	need the feature. Formerly, it was accessible from debugfs,
	but then nowhere used.
	if (user_cfg) {
		if (user_cfg->bsstype)
		bsstype = user_cfg->bsstype;
	} */
	
	/*list_for_each_entry_bssdes(iter, &priv->network_list, list){
		list_move_tail(&iter->list, &priv->network_free_list);
		clear_bss_descriptor(iter);
		
	}*/
	
	//Initialize scan related data structures,mainly bss descriptors,in the first time.
	if(first)
	{
		INIT_LIST_HEAD(&priv->network_free_list);
		INIT_LIST_HEAD(&priv->network_list);
		for (i = 0; i < MAX_NETWORK_COUNT; i++) {
			list_add_tail(&priv->networks[i].list,
			&priv->network_free_list);
			clear_bss_descriptor(&priv->networks[i]);
		}
		first = 0;
	}

	lbs_deb_scan("numchannels %d, bsstype %d\n", numchannels, bsstype);

	/* Create list of channels to scan */
	/*chan_list = kzalloc(sizeof(struct chanscanparamset) *
			    LBS_IOCTL_USER_SCAN_CHAN_MAX, GFP_KERNEL);*/

	chan_list = gmarvel_scan_param;
	memset(chan_list,0,sizeof(struct chanscanparamset) * \
			LBS_IOCTL_USER_SCAN_CHAN_MAX);
	
	/*if (!chan_list) {
		lbs_pr_alert("SCAN: chan_list empty\n");
		goto out;
	}*/

	/* We want to scan all channels */
	// debug_data_stream("chan list",(char *)priv->region_channel,sizeof(struct region_channel ));
	// debug_data_stream("cpf information",(char *)priv->region_channel[0].CFP,sizeof(struct chan_freq_power)*14);
	chan_count = lbs_scan_create_channel_list(priv, chan_list);
	memcpy(gmarvel_scan_param,const_chan_parm,14*7);

	//netif_stop_queue(priv->dev);
	//netif_carrier_off(priv->dev);
	/*if (priv->mesh_dev) {
		netif_stop_queue(priv->mesh_dev);
		netif_carrier_off(priv->mesh_dev);
	}*/

	/* Prepare to continue an interrupted scan */
	lbs_deb_scan("chan_count %d, scan_channel %d\n",
		     chan_count, priv->scan_channel);
	curr_chans = chan_list;
	/* advance channel list by already-scanned-channels */
	if (priv->scan_channel > 0) {
		curr_chans += priv->scan_channel;
		chan_count -= priv->scan_channel;
	}

	/* Send scan command(s)
	 * numchannels contains the number of channels we should maximally scan
	 * chan_count is the total number of channels to scan
	 */

	while (chan_count) {
		int to_scan = min(numchannels, chan_count);
		lbs_deb_scan("scanning %d of %d channels\n",
			     to_scan, chan_count);
		ret = lbs_do_scan(priv, bsstype, curr_chans,
				  to_scan);
		if (ret) {
			lbs_pr_err("SCAN_CMD failed\n");
			goto out2;
		}
		curr_chans += to_scan;
		chan_count -= to_scan;
		memcpy(gmarvel_scan_param,const_chan_parm,14*7);
		/* somehow schedule the next part of the scan */
#ifdef MASK_DEBUG
		if (chan_count && !full_scan &&
		    !priv->surpriseremoved) {
			/* -1 marks just that we're currently scanning */
			if (priv->scan_channel < 0)
				priv->scan_channel = to_scan;
			else
				priv->scan_channel += to_scan;
			cancel_delayed_work(&priv->scan_work);
			queue_delayed_work(priv->work_thread, &priv->scan_work,
					   msecs_to_jiffies(300));
			/* skip over GIWSCAN event */
			goto out;
		}
#endif

	}
	memset(&wrqu, 0, sizeof(union iwreq_data));
	//wireless_send_event(priv->dev, SIOCGIWSCAN, &wrqu, NULL);

	/* Dump the scan table */
	//mutex_lock(&priv->lock);
	printf_scan("scan table:\n");
	list_for_each_entry_bssdes(iter, &priv->network_list, list)
		printf_scan("%02d: BSSID %pM, RSSI %d, SSID '%s'\n",
			     i++, iter->bssid, iter->rssi,
			     iter->ssid);
	//mutex_unlock(&priv->lock);

out2:
	//Release memory.
	if(gmarvel_scan_param)
	{
		KMemFree(gmarvel_scan_param,KMEM_SIZE_TYPE_ANY,0);
	}
	priv->scan_channel = 0;

//out:
	/*if (priv->connect_status == LBS_CONNECTED) {
		//netif_carrier_on(priv->dev);
		if (!priv->tx_pending_len)
			netif_wake_queue(priv->dev);
	}
	if (priv->mesh_dev && (priv->mesh_connect_status == LBS_CONNECTED)) {
		netif_carrier_on(priv->mesh_dev);
		if (!priv->tx_pending_len)
			netif_wake_queue(priv->mesh_dev);
	}
	kfree(chan_list);*/

	lbs_deb_leave_args(LBS_DEB_SCAN, ret);
	return ret;
}

/**
 *  @brief Compare two SSIDs
 *
 *  @param ssid1    A pointer to ssid to compare
 *  @param ssid2    A pointer to ssid to compare
 *
 *  @return         0: ssid is same, otherwise is different
 */
int lbs_ssid_cmp(uint8_t *ssid1, uint8_t ssid1_len, uint8_t *ssid2,
		 uint8_t ssid2_len)
{
	if (ssid1_len != ssid2_len)
		return -1;

	return memcmp(ssid1, ssid2, ssid1_len);
}

struct bss_descriptor *find_beacon_bss(struct lbs_private *priv,
					     uint8_t *ssid, uint8_t ssid_len,int mode)
{
	struct bss_descriptor *iter_bss = NULL;
	list_for_each_entry_bssdes(iter_bss, &priv->network_list, list) {
		if (lbs_ssid_cmp(iter_bss->ssid, iter_bss->ssid_len,
				 ssid, ssid_len) != 0)
			continue; /* ssid doesn't match */
		if (iter_bss->mode==mode)
			break;
	}
	return iter_bss;
}

/**
 *  @brief Send a scan command for all available channels filtered on a spec
 *
 *  Used in association code and from debugfs
 *
 *  @param priv             A pointer to struct lbs_private structure
 *  @param ssid             A pointer to the SSID to scan for
 *  @param ssid_len         Length of the SSID
 *
 *  @return                0-success, otherwise fail
 */
int lbs_send_specific_ssid_scan(struct lbs_private *priv, uint8_t *ssid,
				uint8_t ssid_len)
{
//	DECLARE_SSID_BUF(ssid_buf);
	int ret = 0;

	lbs_deb_enter_args(LBS_DEB_SCAN,ssid);

	if (!ssid_len)
		goto out;

	memcpy(priv->scan_ssid, ssid, ssid_len);
	priv->scan_ssid_len = ssid_len;

	lbs_scan_networks(priv, 1);
	if (priv->surpriseremoved) {
		ret = -1;
		goto out;
	}

out:
	lbs_deb_leave_args(LBS_DEB_SCAN, ret);
	return ret;
}
