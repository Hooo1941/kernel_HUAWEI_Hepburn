#include "data_dubai_collect.h"
#include <linux/net.h>
#include <linux/list.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "hw_booster_common.h"
#include "ip_para_collec_ex.h"
#include "netlink_handle.h"
#include <securec.h>
#include "ip_para_collec.h"

struct dubai_global_variables g_data_dubai_stats;

void init_data_dubai_info(void)
{
	(void)memset_s(&g_data_dubai_stats, sizeof(struct dubai_global_variables),
		0, sizeof(struct dubai_global_variables));
	g_data_dubai_stats.cur_top_app_index = INVALID_TOP_APP_INDEX;
}

void top_app_abn_stat_report(void)
{
	struct res_msg_top_app_abn_stat res;
	struct app_ctx *g_app_ctx;
	u16 len;
	len = sizeof(struct res_msg_top_app_abn_stat);
	if (len > MAX_REQ_DATA_LEN) {
		pr_err("[DUBAI]%s data = %d is too long!", __func__, len);
		(void)memset_s(&g_data_dubai_stats.top_app_abn_stat_list, sizeof(struct app_abn_statics), 0,
			sizeof(struct app_abn_statics));
		return;
	}
	res.type = TOP_APP_ABN_STAT_REPORT;
	res.len = len;
	for (int i = 0; i < TOP_APP_CNT; i++)
		for (int j = 0; j < APP_ABN_INDICATOR_CNT; j++) {
			res.top_apps_abn_stat[i].abn_info[j].rtt_high_rate =
			g_data_dubai_stats.top_app_abn_stat_list[i].abn_info[j].rtt_high_rate;
			res.top_apps_abn_stat[i].abn_info[j].avg_rtt =
			g_data_dubai_stats.top_app_abn_stat_list[i].abn_info[j].avg_rtt;
			res.top_apps_abn_stat[i].abn_info[j].max_rtt =
			g_data_dubai_stats.top_app_abn_stat_list[i].abn_info[j].max_rtt;
			res.top_apps_abn_stat[i].abn_info[j].tcp_link_establishment_fail_rate =
			g_data_dubai_stats.top_app_abn_stat_list[i].abn_info[j].tcp_link_establishment_fail_rate;
			res.top_apps_abn_stat[i].abn_info[j].time_index =
			g_data_dubai_stats.top_app_abn_stat_list[i].abn_info[j].time_index;
	}
	g_app_ctx = get_ip_para_collec_app_ctx();
	if (g_app_ctx == NULL) {
		pr_err("[DUBAI] %s:g_app_ctx null", __func__);
		return;
	}
	if (g_app_ctx->fn)
		g_app_ctx->fn((struct res_msg_head *) &res);
	(void)memset_s(&g_data_dubai_stats.top_app_abn_stat_list, sizeof(struct app_abn_statics), 0,
		sizeof(struct app_abn_statics));
}

static bool cur_fore_is_top_app(void)
{
	if (g_data_dubai_stats.cur_top_app_index >= 0 &&
		g_data_dubai_stats.cur_top_app_index < TOP_APP_CNT)
		return true;
	return false;
}

void data_dubai_update_rtt(u32 rtt_ms)
{
	g_data_dubai_stats.app_stat.total_rtt += rtt_ms;
	if (rtt_ms > TCP_RTT_HIGH_THRESHOLD)
		g_data_dubai_stats.app_stat.high_rtt_tcp_in_pkt++;
	if (rtt_ms > g_data_dubai_stats.app_stat.max_rtt)
		g_data_dubai_stats.app_stat.max_rtt = rtt_ms;
	if (g_data_dubai_stats.app_stat.min_rtt == 0 ||
		rtt_ms < g_data_dubai_stats.app_stat.min_rtt)
		g_data_dubai_stats.app_stat.min_rtt = rtt_ms;
	if (cur_fore_is_top_app()) {
		g_data_dubai_stats.app_statics_top.total_rtt += rtt_ms;
		if (rtt_ms > TCP_RTT_HIGH_THRESHOLD)
			g_data_dubai_stats.app_statics_top.high_rtt_tcp_in_pkt++;
		if (rtt_ms > g_data_dubai_stats.app_statics_top.max_rtt)
			g_data_dubai_stats.app_statics_top.max_rtt = rtt_ms;
		if (g_data_dubai_stats.app_statics_top.min_rtt == 0 ||
			rtt_ms < g_data_dubai_stats.app_statics_top.min_rtt)
			g_data_dubai_stats.app_statics_top.min_rtt = rtt_ms;
	}
}

void data_dubai_update_syn(struct sock *sk)
{
	if (sk == NULL)
		return;
	if (sk->sk_state == TCP_ESTABLISHED) {
		g_data_dubai_stats.app_stat.tcp_established_num++;
		if (cur_fore_is_top_app())
			g_data_dubai_stats.app_statics_top.tcp_established_num++;
	} else {
		g_data_dubai_stats.app_stat.tcp_syc_num++;
		if (cur_fore_is_top_app())
			g_data_dubai_stats.app_statics_top.tcp_syc_num++;
	}
}

void data_dubai_update_tcp_in_pkt(void)
{
	g_data_dubai_stats.app_stat.total_tcp_in_pkt++;
	if (cur_fore_is_top_app())
		g_data_dubai_stats.app_statics_top.total_tcp_in_pkt++;
}

void data_dubai_stat_process(void)
{
	u32 g_foreground_uid;
	g_foreground_uid = get_ip_para_collec_foreground_uid();
	if (g_data_dubai_stats.app_stat.total_tcp_in_pkt == 0 ||
		g_data_dubai_stats.app_stat.total_rtt == 0) {
		pr_info("[DUBAI]cur_fore_uid:%d tcp_pkt_info: no inpkt", g_foreground_uid);
		return;
	}
	if (g_data_dubai_stats.app_stat.total_tcp_in_pkt <
		g_data_dubai_stats.app_stat.high_rtt_tcp_in_pkt) {
		pr_err("[DUBAI] tcp_pkt_info: pkt invalid");
		return;
	}
	u32 avg_rtt = g_data_dubai_stats.app_stat.total_rtt / g_data_dubai_stats.app_stat.total_tcp_in_pkt;
	u16 rtt_high_rate;
	u16 tcp_est_fail_rate;
	if (g_data_dubai_stats.app_stat.high_rtt_tcp_in_pkt == 0) {
		rtt_high_rate = 0;
	} else {
		rtt_high_rate = g_data_dubai_stats.app_stat.high_rtt_tcp_in_pkt * 1000 /
			g_data_dubai_stats.app_stat.total_tcp_in_pkt;
	}
	if (g_data_dubai_stats.app_stat.tcp_established_num > 0 &&
		g_data_dubai_stats.app_stat.tcp_established_num < g_data_dubai_stats.app_stat.tcp_syc_num)
		tcp_est_fail_rate = (g_data_dubai_stats.app_stat.tcp_syc_num -
		g_data_dubai_stats.app_stat.tcp_established_num) * 1000 / g_data_dubai_stats.app_stat.tcp_syc_num;
	pr_info("[DUBAI] tcp_pkt:uid:%d rtt_high:%d avg_rtt:%d max_rtt:%d min_rtt:%d"
		"tcp_fail_rate:%d \n", g_foreground_uid, rtt_high_rate, avg_rtt,
		g_data_dubai_stats.app_stat.max_rtt, g_data_dubai_stats.app_stat.min_rtt,
		tcp_est_fail_rate);
}

static void cpy_abn_statistics(struct abn_statistics src, u8 top_index, u8 kpi_index)
{
	if (top_index >= TOP_APP_CNT || top_index < 0 || kpi_index < 0 || kpi_index >= APP_ABN_INDICATOR_CNT) {
		pr_err("[DUBAI]%s:index invalid", __func__);
		return;
	}
	g_data_dubai_stats.top_app_abn_stat_list[top_index].abn_info[kpi_index].rtt_high_rate = src.rtt_high_rate;
	g_data_dubai_stats.top_app_abn_stat_list[top_index].abn_info[kpi_index].avg_rtt = src.avg_rtt;
	g_data_dubai_stats.top_app_abn_stat_list[top_index].abn_info[kpi_index].max_rtt = src.max_rtt;
	g_data_dubai_stats.top_app_abn_stat_list[top_index].abn_info[kpi_index].tcp_link_establishment_fail_rate =
	src.tcp_link_establishment_fail_rate;
	g_data_dubai_stats.top_app_abn_stat_list[top_index].abn_info[kpi_index].time_index = src.time_index;
}

static void insert_to_top_app_abn_statics_list(struct abn_statistics abn_statistics)
{
	struct app_abn_statics cur_top_app_abn_statics;
	u8 top_index;
	u16 array_len;
	if (!cur_fore_is_top_app()) {
		pr_info("[DUBAI] %s:fore app is not top app", __func__);
		return;
	}
	top_index = g_data_dubai_stats.cur_top_app_index;
	if (top_index >= TOP_APP_CNT || top_index < 0) {
		pr_err("[DUBAI]%s:index invalid", __func__);
		return;
	}
	if (APP_ABN_INDICATOR_CNT != 4) { // 4 is valid len
		pr_err("[DUBAI]%s:APP_ABN_INDICATOR_CNT invalid", __func__);
		return;
	}
	cur_top_app_abn_statics = g_data_dubai_stats.top_app_abn_stat_list[top_index];
	// the length of abn_info is 4, each index represent one abn kpi
	if (cur_top_app_abn_statics.abn_info[0].rtt_high_rate <
		abn_statistics.rtt_high_rate) {
		cpy_abn_statistics(abn_statistics, top_index, 0);
		return;
	}
	if (cur_top_app_abn_statics.abn_info[1].avg_rtt <
		abn_statistics.avg_rtt) {
		cpy_abn_statistics(abn_statistics, top_index, 1);
		return;
	}
	if (cur_top_app_abn_statics.abn_info[2].max_rtt <
		abn_statistics.max_rtt) {
		cpy_abn_statistics(abn_statistics, top_index, 2);
		return;
	}
	if (cur_top_app_abn_statics.abn_info[3].tcp_link_establishment_fail_rate <
		abn_statistics.tcp_link_establishment_fail_rate) {
		cpy_abn_statistics(abn_statistics, top_index, 3);
		return;
	}
}

void data_dubai_handle_top_app_statics(void)
{
	struct abn_statistics temp_top_abn_statistics;
	ktime_t real_time_ns;
	struct rtc_time tm;
	u16 time_index;
	if (!cur_fore_is_top_app()) {
		pr_info("[DUBAI] %s:fore app is not top app", __func__);
		return;
	}
	if (g_data_dubai_stats.app_statics_top.total_tcp_in_pkt == 0 ||
		g_data_dubai_stats.app_statics_top.total_rtt == 0) {
		pr_info("[DUBAI] %s:no inpkt", __func__);
		return;
	}
	if (g_data_dubai_stats.app_statics_top.total_tcp_in_pkt <
		g_data_dubai_stats.app_statics_top.high_rtt_tcp_in_pkt) {
		pr_info("[DUBAI] %s:invalid high_rtt_tcp_in_pkt", __func__);
		return;
	}
	temp_top_abn_statistics.avg_rtt = g_data_dubai_stats.app_statics_top.total_rtt / g_data_dubai_stats.app_statics_top.total_tcp_in_pkt;
	if (g_data_dubai_stats.app_statics_top.high_rtt_tcp_in_pkt == 0) {
		temp_top_abn_statistics.rtt_high_rate = 0;
	} else {
		temp_top_abn_statistics.rtt_high_rate = g_data_dubai_stats.app_statics_top.high_rtt_tcp_in_pkt * 1000 /
			g_data_dubai_stats.app_statics_top.total_tcp_in_pkt;
	}
	temp_top_abn_statistics.max_rtt = g_data_dubai_stats.app_statics_top.max_rtt;
	if (g_data_dubai_stats.app_statics_top.tcp_established_num > 0 &&
		g_data_dubai_stats.app_statics_top.tcp_established_num < g_data_dubai_stats.app_statics_top.tcp_syc_num) {
		temp_top_abn_statistics.tcp_link_establishment_fail_rate = (g_data_dubai_stats.app_statics_top.tcp_syc_num -
		g_data_dubai_stats.app_statics_top.tcp_established_num) * 1000 / g_data_dubai_stats.app_statics_top.tcp_syc_num;
	} else {
		temp_top_abn_statistics.tcp_link_establishment_fail_rate = 0;
	}
	real_time_ns = ktime_get_real();
	tm = rtc_ktime_to_tm(real_time_ns);
	time_index = ((((tm.tm_hour + 8) % 24) * 60) + tm.tm_min) / 2;
	temp_top_abn_statistics.time_index = time_index;
	insert_to_top_app_abn_statics_list(temp_top_abn_statistics);
}

void reset_app_stat(void)
{
	g_data_dubai_stats.qoe_cnt = 0;
	(void)memset_s(&g_data_dubai_stats.app_stat, sizeof(struct tcp_pkt_info), 0, sizeof(struct tcp_pkt_info));
	(void)memset_s(&g_data_dubai_stats.app_statics_top, sizeof(struct tcp_pkt_info), 0, sizeof(struct tcp_pkt_info));
}

void update_top_app_uids(struct dubai_update_top_idx *msg)
{
	if (msg == NULL || msg->len != TOP_APP_UID_LIST_MSG_LEN) {
		pr_err("[DUBAI]: update_top_app_uids fail, msg invalid, len = %d", msg->len);
		return;
	}
	for (int i = 0; i < TOP_APP_CNT; i++)
		g_data_dubai_stats.top_app_uid_list[i] = msg->top_app_uids[i];
}

void update_dubai_top_idx(u16 fore_uid)
{
	u8 cur_top_idx = INVALID_TOP_APP_INDEX;
	for (int i = 0; i < TOP_APP_CNT; i++) {
		if (g_data_dubai_stats.top_app_uid_list[i] == fore_uid && fore_uid != 0) {
			cur_top_idx = i;
			break;
		}
	}
	g_data_dubai_stats.cur_top_app_index = cur_top_idx;
}

struct dubai_global_variables *get_g_data_dubai_stats(void)
{
	return &g_data_dubai_stats;
}

void data_dubai_report(void)
{
	g_data_dubai_stats.qoe_cnt++;
	if (g_data_dubai_stats.qoe_cnt > 0 && g_data_dubai_stats.qoe_cnt % TOP_APP_CNT == 0) {
		data_dubai_stat_process();
		(void)memset_s(&g_data_dubai_stats.app_stat, sizeof(struct tcp_pkt_info), 0, sizeof(struct tcp_pkt_info));
	}
	if (g_data_dubai_stats.qoe_cnt > 0 && g_data_dubai_stats.qoe_cnt % TOP_APP_ABN_STAT_CAL_TIME_ROUND == 0) {
		data_dubai_handle_top_app_statics();
		g_data_dubai_stats.qoe_cnt = 0;
		(void)memset_s(&g_data_dubai_stats.app_statics_top, sizeof(struct tcp_pkt_info), 0, sizeof(struct tcp_pkt_info));
	}
}