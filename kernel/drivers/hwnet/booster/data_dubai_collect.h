#ifndef _DATA_DUBAI_COLLECT_H
#define _DATA_DUBAI_COLLECT_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/rtc.h>
#include <linux/timer.h>
#include <linux/timekeeping.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/net.h>
#include <uapi/linux/netfilter.h>
#include <uapi/linux/netfilter_ipv4.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "netlink_handle.h"

#define APP_ABN_INDICATOR_CNT 4
#define TOP_APP_CNT 10
#define TCP_RTT_HIGH_THRESHOLD 200
#define TOP_APP_ABN_STAT_CAL_TIME_ROUND 120
#define TOP_APP_UID_LIST_MSG_LEN 24
#define INVALID_TOP_APP_INDEX 0xFE

struct abn_statistics { // single indicator struct
	u16 rtt_high_rate;
	u16 avg_rtt;
	u16 max_rtt;
	u16 tcp_link_establishment_fail_rate;
	u16 time_index;
};

struct tcp_pkt_info { // statics
	u16 total_tcp_in_pkt;
	u16 high_rtt_tcp_in_pkt;
	u16 total_rtt;
	u16 max_rtt;
	u16 min_rtt;
	u16 tcp_syc_num;
	u16 tcp_established_num;
};

struct app_abn_statics { // top app abn kpis
	struct abn_statistics abn_info[APP_ABN_INDICATOR_CNT];
};

struct res_msg_top_app_abn_stat {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct app_abn_statics top_apps_abn_stat[TOP_APP_CNT];
};

struct dubai_update_top_idx {
	u16 type;
	u16 len;
	u16 top_app_uids[TOP_APP_CNT]; // final static index, 0-9 correspond to each top app
};

/* The structure of data dubai is defined as: */
struct dubai_global_variables {
	u8 cur_top_app_index;
	struct app_abn_statics top_app_abn_stat_list[TOP_APP_CNT];
	struct tcp_pkt_info app_stat;
	struct tcp_pkt_info app_statics_top;
	u16 qoe_cnt;
	u16 top_app_uid_list[TOP_APP_CNT]; // if app not installed, the invalid uid is 0
};

void data_dubai_report(void);
void top_app_abn_stat_report(void);
void data_dubai_update_rtt(u32 rtt_ms);
void data_dubai_update_syn(struct sock *sk);
void data_dubai_update_tcp_in_pkt(void);
void data_dubai_stat_process(void);
void data_dubai_handle_top_app_statics(void);
void reset_app_stat(void);
void init_data_dubai_info(void);
void update_dubai_top_idx(u16 fore_uid);
void update_top_app_uids(struct dubai_update_top_idx *msg);
struct dubai_global_variables *get_data_dubai_stats(void);

#endif // _DATA_DUBAI_COLLECT_H