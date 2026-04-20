#ifndef _WIFI_COMMON_H_
#define _WIFI_COMMON_H_

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN					6
#endif
#ifndef MAX_AP_CHANNEL_NUM
#define MAX_AP_CHANNEL_NUM				8
#endif

#ifndef MAX_IP_MC_ENTRY
#define MAX_IP_MC_ENTRY				64
#endif

/* Bit mask value for flags, compatiable with old driver */
#ifndef STA_INFO_FLAG_AUTH_OPEN
#define STA_INFO_FLAG_AUTH_OPEN			0x01
#endif
#ifndef STA_INFO_FLAG_AUTH_WEP
#define STA_INFO_FLAG_AUTH_WEP			0x02
#endif
#ifndef STA_INFO_FLAG_ASOC
#define STA_INFO_FLAG_ASOC				0x04
#endif
#ifndef STA_INFO_FLAG_ASLEEP
#define STA_INFO_FLAG_ASLEEP			0x08
#endif

#ifndef NUM_STA_TOTAL_LIMIT
/* When CONFIG_RTL_WIFI_OFFLOAD is enabled, the current maximum is 64 */
#define NUM_STA_TOTAL_LIMIT 64	/* Maximum: 128 - CONFIG_IFACE_NUMBER */
#endif

/*
	The following structures are also used by the upper layer.
	Please do not change the content...!!!
	Please do not use any #ifdef flag in this file...!!!
	Please sync the following files:
	- rtl8192cd/wifi_common.h
	- g6_wifi_driver/include/wifi_common.h
*/

#pragma pack(1)

/* ********************************* */
/* wifi5 and wifi6 shared structures */
/* ********************************* */

typedef struct _bss_info_2_web {
	unsigned char state;
	unsigned char channel;
	unsigned char txRate;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char rssi;
	unsigned char sq;
	unsigned char ssid[33];
} bss_info_2_web;

typedef struct _sta_info_2_web {
	unsigned short aid;
	unsigned char  addr[MAC_ADDR_LEN];
	unsigned long  tx_packets;
	unsigned long  rx_packets;
	unsigned long  expired_time;  // 10 msec unit
	unsigned short flags;
	unsigned char  TxOperaRate;
	unsigned char  rssi;
	unsigned long  link_time;     // 1 sec unit
	unsigned long  idle_time;
	unsigned long  tx_fail;
	unsigned long long  tx_bytes;
	unsigned long long  rx_bytes;
	unsigned char  network;
	unsigned char  ht_info;       // bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char  RxOperaRate;
	unsigned char  auth_type;
	unsigned char  enc_type;
	unsigned char  snr;
	unsigned char  status_support;
	unsigned char  resv_1;
	unsigned short acTxOperaRate;
	unsigned char  multiap_profile;
	unsigned char  max_mode_support;
	unsigned char  max_bw_support;
} sta_info_2_web;

typedef struct _sta_extra_info_2_web {
	unsigned short aid;
	unsigned char  addr[MAC_ADDR_LEN];
	unsigned long  tx_packets;
	unsigned long  rx_packets;
	unsigned long  expired_time;  // 10 msec unit
	unsigned short flags;
	unsigned char  TxOperaRate;
	unsigned char  rssi;
	unsigned long  link_time;     // 1 sec unit
	unsigned long  idle_time;
	unsigned long  tx_fail;
	unsigned long long  tx_bytes;
	unsigned long long  rx_bytes;
	unsigned char  network;
	unsigned char  ht_info;       // bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char  RxOperaRate;
	unsigned char  auth_type;
	unsigned char  enc_type;
	unsigned char  resv_1;
	unsigned short acTxOperaRate;
	unsigned char  client_host_name[256];
	unsigned char  client_host_ip[4];
	unsigned int   tx_bytes_1s;
	unsigned int   rx_bytes_1s;
	char           rxrate[10];
	char           txrate[10];
} sta_extra_info_2_web;

struct dot11k_ap_channel_report
{
	unsigned char len;
	unsigned char op_class;
	unsigned char channel[MAX_AP_CHANNEL_NUM];
};

struct target_transition_list
{
	unsigned char  addr[MAC_ADDR_LEN];
	unsigned char  mode;         // bit7: request mode; bit6: disassoc imminent; bit5: abridged
	unsigned short disassoc_timer;
	unsigned char  target_bssid[MAC_ADDR_LEN];
	unsigned char  opclass;
	unsigned char  channel;
	unsigned char  reason_code;  // boot fromSTA
	// unsigned char  disassoc;
};


/* *********************** */
/* wifi6 shared structures */
/* *********************** */

/* ---------------------- */
/* IOCTL AP: SHEET HEADER */
/* ---------------------- */
#define IOCTL_AP_SHEET_HDR_V1
typedef struct _sheet_hdr_2_web {
	unsigned char sheet_sequence; /* start from 0 */
	unsigned char sheet_total;
} sheet_hdr_2_web;

/* ------------------- */
/* IOCTL AP: STA ENTRY */
/* ------------------- */
#define IOCTL_STA_ENTRY_V1
typedef struct _sta_entry_2_web {
	unsigned short     aid;
	unsigned char      addr[MAC_ADDR_LEN];
	unsigned int       link_time;
	unsigned int       expired_time;
	unsigned short     flags;
	unsigned short     tx_op_rate;
	unsigned short     rx_op_rate;
	unsigned char      tx_gi_ltf;
	unsigned char      rx_gi_ltf;
	unsigned long long tx_packets;
	unsigned long long rx_packets;
	unsigned long long tx_fails; /* Tx fail by HW */
	unsigned long long tx_drops; /* Tx drop by SW */
	unsigned long long tx_retry; /* Tx retry pkts */
	unsigned long long rx_errors;/* Rx errors reported by HW */
	unsigned long long rx_drops; /* Rx drop bt SW */
	unsigned long long rx_retry; /* Rx retry pkts */
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
	unsigned char      channel_bandwidth;
	unsigned char      rssi;
	unsigned char      signal_quality;
	unsigned char      wireless_mode; /* a/b/g/n/ac/ax */
	unsigned int       encrypt;       /* open/WEP/WPA/RSN */
	unsigned int       pairwise_cipher;
	unsigned int       group_cipher;
	unsigned int       akm;
	unsigned char      tln_stats_resv[2];
	unsigned char      status_support;
	unsigned char      multi_ap_profile;
	unsigned char      rm_cap;
	unsigned char      btm_support;
	unsigned char      resv_for_future[16]; /* for future use */
} sta_entry_2_web;

/* ----------------------------- */
/* IOCTL AP: WIFI DIAG STA ENTRY */
/* ----------------------------- */
/* CONFIG_WIFI_DIAGNOSIS */
#define IOCTL_AP_WIFI_DIAG_STA_ENTRY_V1
typedef struct _wifi_diag_sta_entry_2_web {
	unsigned char channel;
	unsigned char addr[MAC_ADDR_LEN];
	unsigned char rssi;
	/* following is connected bss info */
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char ssid[33];
	unsigned char ssid_length;
	unsigned int  bss_encrypt;  /* open/WEP/WPA/RSN */
	unsigned int  bss_pairwise_cipher;
	unsigned int  bss_group_cipher;
	unsigned int  bss_akm;
	/* end of connected bss info */
	unsigned char used;
	unsigned char entry;
	unsigned char status;
	unsigned int  time_stamp;
} wifi_diag_sta_entry_2_web;

/* ------------------ */
/* IOCTL AP: BSS DESC */
/* ------------------ */
#define IOCT_AP_BSS_DESC_V1
typedef struct _bss_desc_2_web {
	unsigned char  bssid[MAC_ADDR_LEN];
	unsigned char  ssid[33];
	unsigned char  ssidlen;
	unsigned char  mesh_id[33];
	unsigned char  mesh_id_length;
	unsigned char  channel;
	unsigned char  channel_bandwidth;
	unsigned char  rssi;
	unsigned char  signal_quality;
	unsigned short capability;
	unsigned short beacon_period;
	unsigned short atim_window;
	unsigned char  dtim_period;	
	unsigned short support_rate;
	unsigned short basic_rate;
	unsigned char  infra_mode;    /* Infrastructure/IBSS */
	unsigned char  wireless_mode; /* a/b/g/n/ac/ax */
	unsigned int   encrypt;       /* open/WEP/WPA/RSN */
	unsigned int   pairwise_cipher;
	unsigned int   group_cipher;
	unsigned int   akm;
	unsigned int   time_stamp;
	unsigned char  network_type;
	unsigned char mfp_opt;
} bss_desc_2_web;

typedef struct _ip_mcast_info_2_web {
	unsigned char mcmac[MAC_ADDR_LEN];
} ip_mcast_info_2_web;

typedef struct _sta_mcast_2_web {
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char ipmc_num;
	ip_mcast_info_2_web ipmc[MAX_IP_MC_ENTRY];
} sta_mcast_2_web;

/* -------------------------------- */
/* IOCTL AP: EVENT_ROAMING_INFO_RPT */
/* -------------------------------- */
/* RTL_LINK_ROAMING_REPORT */
typedef struct _ONE_STA {
	unsigned char  addr[MAC_ADDR_LEN];
	unsigned char  rssi;
	unsigned short rxRate;
	unsigned short txRate;
	unsigned char  low_qos;
	unsigned char  channel;
	unsigned char  bw;
	unsigned char  retry_ratio;
	unsigned char  fail_ratio;
	unsigned int   tx_pkts;
	unsigned char  wlanid;
	unsigned char  used;
} ONE_STA;

typedef struct _ROAMING_INFO_RPT {
	unsigned char eventID;
	unsigned int  sta_num;
	ONE_STA  sta_info[NUM_STA_TOTAL_LIMIT];
} ROAMING_INFO_RPT;

#pragma pack()

typedef bss_info_2_web bss_info;

struct ap_neighbor_info_entry
{
	unsigned char band;
	unsigned char ssid[33];
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char networktype;
	unsigned char channel;
	unsigned char rssi;
	unsigned char standard;
	unsigned char bandwidth;
	unsigned int encryp_protocol;
	unsigned int pairwise_cipher;
	unsigned int akm;

	/* the following members must be at bottom */
	unsigned char valid;
	unsigned long sec;
	unsigned long last_seen_time;
};

#endif

