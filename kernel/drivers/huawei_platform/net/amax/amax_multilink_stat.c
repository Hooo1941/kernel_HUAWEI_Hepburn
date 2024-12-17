/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#include <securec.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <dsm/dsm_pub.h>
#include "amax_multilink_tool.h"
#include "amax_multilink_stat.h"

/* proc系统维测打印 */
#define AMAX_PROC_FILE_MODE 0664
#define AMAX_PROC_DIR_NAME "amax"
#define AMAX_PROC_DIR "/proc/" AMAX_PROC_DIR_NAME
#define AMAX_PROC_RX_STAT_FILE "rx_stat"
#define AMAX_PROC_WIN_STAT_FILE "win_stat"
#define AMAX_PROC_TX_STAT_FILE "tx_stat"
#define AMAX_PROC_DMP_STAT_FILE "dmd_stat"
#define AMAX_DSM_CLIENT "dsm_wifi"
#define AMAX_PROC_FILE_VALUE_LENGTH 2
#define AMAX_PROC_BUFFER_LEN 2048
#define AMXA_PROC_BASE 10
#define BYTE_TO_MBIT(m) ((m) / 1024 / 1024 * 8)
#define MILLISEC_TO_SEC 1000
#define UPLOAD_TIME_6H 21600 /* 上报时间6小时，60*60*6 */
#define LOG_BUF_SIZE 512
#define DMD_EVENT_BUFF_SIZE 1024
#define DSM_MULTILINK_STATISTIC 909030017

/* Global variables */
static amax_stat *g_amax_stat = NULL;
static amax_abnormal_stat *g_amax_abnormal_stat = NULL;
static amax_tx_dmd_stat *g_amax_tx_dmd_stat = NULL;
static uint8_t g_amax_tx_dmd_trigged_now = 0;
static uint32_t g_amax_tx_dmd_trigged_time_thr = UPLOAD_TIME_6H; // 单位为秒
static struct proc_dir_entry *g_amax_proc_dir = NULL;
static struct proc_dir_entry *g_proc_amax_rx_file_entry = NULL;
static struct proc_dir_entry *g_proc_amax_win_file_entry = NULL;
static struct proc_dir_entry *g_proc_amax_tx_file_entry = NULL;
static struct proc_dir_entry *g_proc_amax_dmp_file_entry = NULL;

static uint32_t g_max_win_submit_interval = 0;
static int32_t g_last_dsm_id = 0;
static uint32_t g_max_stay_time;
static uint64_t g_beyond_stay_cnt;
static uint64_t g_amax_napi_cnt;
static uint8_t g_pps_index; // pps数组下标

static int g_amax_proc_buffer_len = 2048;
module_param(g_amax_proc_buffer_len, int, 0664);

static void amax_get_empty_hole_of_window(cache_array_recv_handle *recv_array_hdl);
static void amax_proc_exit(void);

void debug_inc_napi_cnt(void)
{
	g_amax_napi_cnt++;
}

static void window_debug_inc_rx_num_total(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].rx_num_total++;
}

static void window_debug_inc_rx_num_5g(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].rx_num_5g++;
}

static void window_debug_inc_rx_num_2g(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].rx_num_2g++;
}

void window_debug_inc_submit_num_total(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].submit_num_total++;
}

void window_debug_inc_submit_num_5g(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].submit_num_5g++;
}

void window_debug_inc_submit_num_2g(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].submit_num_2g++;
}

void window_debug_inc_timeout_move(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].timeout_move++;
}

void window_debug_inc_update_move(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].update_move++;
}

void window_debug_inc_normal_move(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].normal_move++;
}

void window_debug_inc_reset_num(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].reset_num++;
}

void window_debug_inc_self_cure_cnt(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].self_cure_cnt++;
}

void debug_inc_submit_timeout_cnt(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].submit_timeout_cnt++;
}

void debug_inc_win_wait_timeout_cnt(uint8_t tid)
{
	g_amax_stat->debug_stat.window_debug[tid].win_timeout_cnt++;
}

void debug_inc_normal_move(void)
{
	g_amax_stat->debug_stat.rx_debug.normal_move++;
}

void debug_inc_reset_num(void)
{
	g_amax_stat->debug_stat.rx_debug.reset_num++;
}

static void debug_inc_rx_num_total(void)
{
	g_amax_stat->debug_stat.rx_debug.rx_num_total++;
}

static void debug_inc_rx_num_2g(void)
{
	g_amax_stat->debug_stat.rx_debug.rx_num_2g++;
}

static void debug_inc_rx_num_5g(void)
{
	g_amax_stat->debug_stat.rx_debug.rx_num_5g++;
}

void debug_inc_empty_num(void)
{
	g_amax_stat->debug_stat.rx_debug.update_move_empty_num++;
}

void debug_inc_update_move(void)
{
	g_amax_stat->debug_stat.rx_debug.update_move++;
}

void debug_inc_timeout_move(void)
{
	g_amax_stat->debug_stat.rx_debug.timeout_move++;
}

void debug_inc_submit_num_total(void)
{
	g_amax_stat->debug_stat.rx_debug.submit_num_total++;
}

void debug_inc_submit_num_2g(void)
{
	g_amax_stat->debug_stat.rx_debug.submit_num_2g++;
}

void debug_inc_submit_num_5g(void)
{
	g_amax_stat->debug_stat.rx_debug.submit_num_5g++;
}

void debug_inc_timeout_force_update_num(void)
{
	g_amax_stat->debug_stat.rx_debug.timeout_force_update_num++;
}

void debug_inc_beyond_stay_cnt(void)
{
	g_beyond_stay_cnt++;
}

void debug_update_max_stay_time(uint32_t curr_stay_time)
{
	g_max_stay_time = curr_stay_time > g_max_stay_time ? curr_stay_time : g_max_stay_time;
}

void debug_update_max_win_interval(uint32_t curr_win_interval)
{
	g_max_win_submit_interval =
		curr_win_interval >= g_max_win_submit_interval ? curr_win_interval : g_max_win_submit_interval;
}

void debug_inc_rx_num(uint8_t fn, uint8_t tid)
{
	debug_inc_rx_num_total();
	window_debug_inc_rx_num_total(tid);
	if (fn == FN_2) {
		debug_inc_rx_num_2g();
		window_debug_inc_rx_num_2g(tid);
	} else {
		debug_inc_rx_num_5g();
		window_debug_inc_rx_num_5g(tid);
	}
}

static void amax_abnormal_inc_pkt_fail(uint8_t fn)
{
	if (g_amax_abnormal_stat == NULL)
		return;

	if (fn == FN_DOUBLE) {
		g_amax_abnormal_stat->main_fail++;
		g_amax_abnormal_stat->assist_fail++;
	} else if (fn == FN_5) {
		g_amax_abnormal_stat->main_fail++;
	} else if (fn == FN_2) {
		g_amax_abnormal_stat->assist_fail++;
	}
}

void amax_abnormal_inc_fdrop_num(cache_array_recv_handle *recv_array_hdl, uint16_t new_lowedge)
{
	if (recv_array_hdl == NULL)
		return;

	g_amax_abnormal_stat->fdrop_num += ((new_lowedge + MAX_ITEM_NUM - recv_array_hdl->rx_lowedge) & MAX_ITEM_MASK);
}

void amax_abnormal_inc_timeout_move(void)
{
	g_amax_abnormal_stat->timeout_move++;
}

void amax_abnormal_inc_update_move(void)
{
	g_amax_abnormal_stat->update_move++;
}

void amax_abnormal_inc_self_cure_cnt(void)
{
	g_amax_abnormal_stat->self_cure_cnt++;
}

void debug_inc_pkt_5g_dc(void)
{
	g_amax_stat->chr_stat.pkt_5g_dc++;
}

void debug_inc_pkt_2g_dc(void)
{
	g_amax_stat->chr_stat.pkt_2g_dc++;
}

void debug_inc_pkt_5g_fail(void)
{
	g_amax_stat->chr_stat.pkt_5g_fail++;
}

void debug_inc_pkt_2g_fail(void)
{
	g_amax_stat->chr_stat.pkt_2g_fail++;
}

void debug_inc_pkt_dc(uint8_t fn)
{
	if (fn == FN_DOUBLE) {
		debug_inc_pkt_5g_dc();
		debug_inc_pkt_2g_dc();
	} else if (fn == FN_5) {
		debug_inc_pkt_5g_dc();
	} else if (fn == FN_2) {
		debug_inc_pkt_2g_dc();
	}
}

void debug_inc_pkt_fail(uint8_t fn)
{
	amax_abnormal_inc_pkt_fail(fn);
	if (fn == FN_DOUBLE) {
		debug_inc_pkt_5g_fail();
		debug_inc_pkt_2g_fail();
	} else if (fn == FN_5) {
		debug_inc_pkt_5g_fail();
	} else if (fn == FN_2) {
		debug_inc_pkt_2g_fail();
	}
}

void amax_set_tx_5g_throughput(uint8_t index, uint32_t throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput[index] = throughput;
}

void amax_set_tx_2g_throughput(uint8_t index, uint32_t throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput[index] = throughput;
}

void amax_set_tx_5g_max_throughput(uint32_t max_throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput_max = max_throughput;
}

void amax_set_tx_2g_max_throughput(uint32_t max_throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput_max = max_throughput;
}

void amax_set_tx_5g_avg_throughput(uint32_t avg_throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput_avg = avg_throughput;
}

void amax_set_tx_2g_avg_throughput(uint32_t avg_throughput)
{
	g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput_avg = avg_throughput;
}

void amax_set_tx_contime_sum(uint8_t fn, uint32_t sum)
{
	g_amax_stat->debug_stat.tx_debug[fn].contime_sum = sum;
}

uint32_t amax_get_tx_5g_throughput(uint8_t index)
{
	return g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput[index];
}

uint32_t amax_get_tx_2g_throughput(uint8_t index)
{
	return g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput[index];
}

uint32_t amax_get_tx_5g_max_throughput(void)
{
	return g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput_max;
}

uint32_t amax_get_tx_2g_max_throughput(void)
{
	return g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput_max;
}

uint32_t amax_get_tx_avg_throughput(uint8_t fn)
{
	if (fn != FN_2 && fn != FN_5)
		return 0;
	return g_amax_stat->debug_stat.tx_debug[fn].amax_throughput_avg;
}

uint32_t *amax_get_tx_contime_array(uint8_t fn)
{
	return g_amax_stat->debug_stat.tx_debug[fn].contime;
}

uint32_t amax_get_tx_contime_sum(uint8_t fn)
{
	return g_amax_stat->debug_stat.tx_debug[fn].contime_sum;
}

void amax_inc_tx_pps(uint8_t fn)
{
	g_amax_stat->debug_stat.tx_debug[fn].amax_pps[g_pps_index]++;
}

void amax_update_tx_pps(void)
{
	uint8_t i;
	uint32_t sum_5g = 0;
	uint32_t sum_2g = 0;

	for (i = 0; i < PPS_SAMPLE_NUM; i++) {
		sum_5g += g_amax_stat->debug_stat.tx_debug[FN_5].amax_pps[i];
		sum_2g += g_amax_stat->debug_stat.tx_debug[FN_2].amax_pps[i];
	}
	g_amax_stat->debug_stat.tx_debug[FN_5].amax_pps_avg = sum_5g / PPS_SAMPLE_NUM;
	g_amax_stat->debug_stat.tx_debug[FN_2].amax_pps_avg = sum_2g / PPS_SAMPLE_NUM;

	g_pps_index = (g_pps_index + 1) % PPS_SAMPLE_NUM;
	g_amax_stat->debug_stat.tx_debug[FN_5].amax_pps[g_pps_index] = 0;
	g_amax_stat->debug_stat.tx_debug[FN_2].amax_pps[g_pps_index] = 0;

	return;
}

uint32_t amax_get_tx_pps_avg(uint8_t fn)
{
	if (fn != FN_2 && fn != FN_5)
		return 0;
	return g_amax_stat->debug_stat.tx_debug[fn].amax_pps_avg;
}

void debug_inc_tx_reset_cnt(void)
{
	g_amax_stat->chr_stat.reset_num++;
}

static void amax_get_empty_hole_of_window(cache_array_recv_handle *recv_array_hdl)
{
	uint8_t cursor = 0;
	uint16_t cur_id;
	recv_frame *recv_frame_array = NULL;

	// 缓存数组为空，将空洞置为空，并退出
	if (recv_array_hdl->rx_lowedge == ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
		memset_s(recv_array_hdl->empty_hole, sizeof(recv_array_hdl->empty_hole), 0, sizeof(recv_array_hdl->empty_hole));
		memset_s(recv_array_hdl->empty_hole_16, sizeof(recv_array_hdl->empty_hole_16), 0,
			sizeof(recv_array_hdl->empty_hole_16));
		return;
	}

	// 寻找当前缓存数组中空洞，从下沿开始找10个空洞，下沿本身应该就是空洞
	recv_frame_array = recv_array_hdl->frame_array;
	cur_id = recv_array_hdl->rx_lowedge;
	while (cur_id != ((recv_array_hdl->rx_upedge + 1) & MAX_ITEM_MASK)) {
		if (recv_frame_array[cur_id].recv_status == NOT_RECEIVED) {
			recv_array_hdl->empty_hole[cursor] = cur_id;
			recv_array_hdl->empty_hole_16[cursor++] =
				recv_array_hdl->real_lowedge + cur_id - recv_array_hdl->rx_lowedge;
			if (cursor == MAX_EMPTY_HOLE_CNT) {
				break;
			}
		}
		cur_id = ((cur_id + 1) & MAX_ITEM_MASK);
	}
}

/* proc系统维测打印 */
static void amax_clear_rx_stat(void)
{
	memset_s(&(g_amax_stat->debug_stat.rx_debug), sizeof(amax_rx_debug), 0, sizeof(amax_rx_debug));
	amax_get_empty_hole_of_window(get_recv_array_hdl_all(0));

	g_max_stay_time = 0;
	g_beyond_stay_cnt = 0;
	g_max_win_submit_interval = 0;
	g_amax_napi_cnt = 0;
}

static ssize_t amax_proc_rx_stat_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
	int32_t cnt;
	char *data = NULL;

	if (*offset > 0)
		return 0;

	data = (char *)kmalloc(AMAX_PROC_BUFFER_LEN, GFP_KERNEL);
	if (data == NULL)
		return -1;

	if (sprintf_s(data, AMAX_PROC_BUFFER_LEN,
		"rx_num_total %u\nrx_num_5g %u\nrx_num_2g %u\n"
		"submit_num_total %u\nsubmit_num_5g %u\nsubmit_num_2g %u\n"
		"normal_move %llu\ntimeout_move %llu\nupdate_move %llu\n"
		"timeout_force_update_num %u\n"
		"update_move_empty_num %u\n"
		"reset_num %u\n"
		"g_max_stay_time %u\n"
		"g_beyond_stay_cnt %llu\n"
		"g_max_win_submit_interval %u\n"
		"g_amax_napi_cnt %llu\n"
		"echo 1 to clear\n",
		g_amax_stat->debug_stat.rx_debug.rx_num_total, g_amax_stat->debug_stat.rx_debug.rx_num_5g,
		g_amax_stat->debug_stat.rx_debug.rx_num_2g, g_amax_stat->debug_stat.rx_debug.submit_num_total,
		g_amax_stat->debug_stat.rx_debug.submit_num_5g, g_amax_stat->debug_stat.rx_debug.submit_num_2g,
		g_amax_stat->debug_stat.rx_debug.normal_move,
		g_amax_stat->debug_stat.rx_debug.timeout_move, g_amax_stat->debug_stat.rx_debug.update_move,
		g_amax_stat->debug_stat.rx_debug.timeout_force_update_num,
		g_amax_stat->debug_stat.rx_debug.update_move_empty_num, g_amax_stat->debug_stat.rx_debug.reset_num,
		g_max_stay_time, g_beyond_stay_cnt, g_max_win_submit_interval, g_amax_napi_cnt) < 0) {
		kfree(data);
		return -1;
	}

	cnt = simple_read_from_buffer(buffer, len, offset, data, strlen(data));
	kfree(data);
	return cnt;
}

static ssize_t amax_proc_rx_stat_write(struct file *fp, const char *buffer, size_t len, loff_t *off)
{
	unsigned char value = 0;

	if (len >= AMAX_PROC_BUFFER_LEN)
		return -1;

	if (kstrtou8_from_user(buffer, len, AMXA_PROC_BASE, &value) != 0)
		return -1;

	if (value == 1)
		amax_clear_rx_stat();
	return len;
}

static void amax_reset_chr_stat(void)
{
	memset_s(&(g_amax_stat->chr_stat), sizeof(amax_chr_stat), 0, sizeof(amax_chr_stat));
	return;
}

static ssize_t amax_proc_tx_stat_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
	int32_t cnt;
	char *data = NULL;
	amax_sta_pair *sta_pair = NULL;

	if (*offset > 0)
		return 0;

	data = (char *)kmalloc(AMAX_PROC_BUFFER_LEN, GFP_KERNEL);
	if (data == NULL)
		return -1;

	sta_pair = get_sta_pair();
	if (sprintf_s(data, AMAX_PROC_BUFFER_LEN,
		"pkt_5g_dc %u\n"
		"pkt_2g_dc %u\n"
		"pkt_5g_fail %u\n"
		"pkt_2g_fail %u\n"
		"5g_throughput_avg %u\n"
		"2g_throughput_avg %u\n"
		"5g_contime_sum %u\n"
		"2g_contime_sum %u\n"
		"5g_pps_avg %u\n"
		"2g_pps_avg %u\n"
		"llc_id %hu %hu %hu %hu\n"
		"reset_cnt %hhu\n"
		"exp_tail_num %u\n"
		"tup_encap_cnt %u\n"
		"tail_less_cnt %u\n"
		"mark_num %u\n"
		"same_seg_num %u\n"
		"echo 1 to clear\n",
		g_amax_stat->chr_stat.pkt_5g_dc, g_amax_stat->chr_stat.pkt_2g_dc,
		g_amax_stat->chr_stat.pkt_5g_fail, g_amax_stat->chr_stat.pkt_2g_fail,
		g_amax_stat->debug_stat.tx_debug[FN_5].amax_throughput_avg,
		g_amax_stat->debug_stat.tx_debug[FN_2].amax_throughput_avg,
		g_amax_stat->debug_stat.tx_debug[FN_5].contime_sum,
		g_amax_stat->debug_stat.tx_debug[FN_2].contime_sum,
		g_amax_stat->debug_stat.tx_debug[FN_5].amax_pps_avg,
		g_amax_stat->debug_stat.tx_debug[FN_2].amax_pps_avg,
		LLC2STR(sta_pair->llc_id),
		g_amax_stat->chr_stat.reset_num, g_amax_stat->chr_stat.exp_tail_num,
		g_amax_stat->chr_stat.tup_encap_cnt, g_amax_stat->chr_stat.tail_less_cnt,
		g_amax_stat->chr_stat.mark_num, g_amax_stat->chr_stat.same_seg_num) < 0) {
		kfree(data);
		return -1;
	}

	cnt = simple_read_from_buffer(buffer, len, offset, data, strlen(data));
	kfree(data);
	return cnt;
}

static ssize_t amax_proc_tx_stat_write(struct file *fp, const char *buffer, size_t len, loff_t *off)
{
	unsigned char value = 0;

	if (len >= AMAX_PROC_BUFFER_LEN)
		return -1;

	if (kstrtou8_from_user(buffer, len, AMXA_PROC_BASE, &value) != 0)
		return -1;

	if (value == 1)
		amax_reset_chr_stat();

	return len;
}

static void amax_reset_win_debug_stat(void)
{
	int32_t i;
	cache_array_recv_handle *hdl = NULL;

	(void)memset_s(g_amax_stat->debug_stat.window_debug, sizeof(amax_rx_debug) * TID_ARR_NUM, 0,
		sizeof(amax_rx_debug) * TID_ARR_NUM);
	for (i = 0; i < TID_ARR_NUM; i++) {
		hdl = get_recv_array_hdl_all(i);
		if (!hdl)
			return;
		hdl->drop_pkts_num = 0;
		hdl->out_of_window_frame_num = 0;
		hdl->udp_oow_submit_num = 0;
		hdl->true_duplicate_num = 0;
		hdl->false_duplicate_num = 0;
		hdl->max_cache_num = 0;
	}
}

static errno_t print_debug_window_info(char *data, cache_array_recv_handle *hdl, uint8_t tid, uint32_t out_offset)
{
	return sprintf_s(data + out_offset, g_amax_proc_buffer_len - out_offset,
			"window NO.%hhu\n"
			"rx_num %u  rx_num_5g %u  rx_num_2g %u\n"
			"submit_num %u  submit_num_5g %u  submit_num_2g %u\n"
			"fdrop_num %llu  oow_num %llu  total_oow_num %llu udp_oow_submit_num %llu\n"
			"true_dup_num %u  false_dup_num %llu  max_cache_num %u\n"
			"normal_move %llu  timeout_move %llu  update_move %llu\n"
			"submit_timeout_cnt %u  win_timeout_cnt %u self_cure_cnt %u\n"
			"llc_5g %u  llc_2g %u  reset_num %u\n"
			"lowedge %u  curr_llc %u  upedge %u\n\n",
			tid, g_amax_stat->debug_stat.window_debug[tid].rx_num_total,
			g_amax_stat->debug_stat.window_debug[tid].rx_num_5g,
			g_amax_stat->debug_stat.window_debug[tid].rx_num_2g,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_total,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_5g,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_2g,
			hdl->drop_pkts_num, hdl->out_of_window_frame_num, hdl->total_oow_frame_num,
			hdl->udp_oow_submit_num, hdl->true_duplicate_num,
			hdl->false_duplicate_num, hdl->max_cache_num, g_amax_stat->debug_stat.window_debug[tid].normal_move,
			g_amax_stat->debug_stat.window_debug[tid].timeout_move,
			g_amax_stat->debug_stat.window_debug[tid].update_move,
			g_amax_stat->debug_stat.window_debug[tid].submit_timeout_cnt,
			g_amax_stat->debug_stat.window_debug[tid].win_timeout_cnt,
			g_amax_stat->debug_stat.window_debug[tid].self_cure_cnt,
			hdl->curr_llc_id_5g, hdl->curr_llc_id_2g, g_amax_stat->debug_stat.window_debug[tid].reset_num,
			hdl->real_lowedge, hdl->frame_array[hdl->rx_upedge].llc_id_16,
			((hdl->real_lowedge + MAX_ITEM_NUM - 1) % MAX_LLC_SIZE));
}

static errno_t print_window_info(char *data, cache_array_recv_handle *hdl, uint8_t tid, uint32_t out_offset)
{
	return sprintf_s(data + out_offset, g_amax_proc_buffer_len - out_offset,
			"window NO.%hhu\n"
			"rx_num %u  rx_num_5g %u  rx_num_2g %u  reset_num %u\n"
			"submit_num %u  submit_num_5g %u  submit_num_2g %u\n"
			"lowedge %u  curr_llc %u  upedge %u\n\n",
			tid, g_amax_stat->debug_stat.window_debug[tid].rx_num_total,
			g_amax_stat->debug_stat.window_debug[tid].rx_num_5g,
			g_amax_stat->debug_stat.window_debug[tid].rx_num_2g,
			g_amax_stat->debug_stat.window_debug[tid].reset_num,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_total,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_5g,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_2g,
			hdl->real_lowedge, hdl->frame_array[hdl->rx_upedge].llc_id_16,
			((hdl->real_lowedge + MAX_ITEM_NUM - 1) % MAX_LLC_SIZE));
}

static ssize_t amax_proc_win_stat_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
	int32_t cnt;
	uint8_t i;
	cache_array_recv_handle *hdl = NULL;
	char *data = NULL;
	errno_t ret;
	uint32_t out_offset = 0;

	if (*offset > 0)
		return 0;

	data = (char *)kmalloc(g_amax_proc_buffer_len, GFP_KERNEL);
	if (data == NULL)
		return -1;

	for (i = 0; i < TID_ARR_NUM; i++) {
		hdl = get_recv_array_hdl_all(i);
		if (get_stat_switch()) {
			ret = print_debug_window_info(data, hdl, i, out_offset);
		} else {
			ret = print_window_info(data, hdl, i, out_offset);
		}
		if (ret < 0) {
			kfree(data);
			return -1;
		}
		out_offset += ret;
	}

	cnt = simple_read_from_buffer(buffer, len, offset, data, strlen(data));
	kfree(data);
	return cnt;
}

static ssize_t amax_proc_win_stat_write(struct file *fp, const char *buffer, size_t len, loff_t *off)
{
	unsigned char value = 0;

	if (len >= g_amax_proc_buffer_len)
		return -1;

	if (kstrtou8_from_user(buffer, len, AMXA_PROC_BASE, &value) != 0)
		return -1;

	if (value == 1)
		amax_reset_win_debug_stat();

	return len;
}

void amax_reset_single_win_stat(uint8_t tid)
{
	(void)memset_s(&(g_amax_stat->debug_stat.window_debug[tid]), sizeof(amax_rx_debug), 0, sizeof(amax_rx_debug));

	return;
}

static const struct proc_ops g_file_proc_rx_ops_amax = {
	.proc_read = amax_proc_rx_stat_read,
	.proc_write = amax_proc_rx_stat_write,
};

static const struct proc_ops g_file_proc_win_ops_amax = {
	.proc_read = amax_proc_win_stat_read,
	.proc_write = amax_proc_win_stat_write,
};

static const struct proc_ops g_file_proc_tx_ops_amax = {
	.proc_read = amax_proc_tx_stat_read,
	.proc_write = amax_proc_tx_stat_write,
};

ssize_t amax_proc_dmp_stat_read(struct file *fp, char *buffer, size_t len, loff_t *offset);
ssize_t amax_proc_dmp_stat_write(struct file *fp, const char *buffer, size_t len, loff_t *off);

static const struct proc_ops g_file_proc_dmp_ops_amax = {
		.proc_read = amax_proc_dmp_stat_read,
		.proc_write = amax_proc_dmp_stat_write,
};

static void amax_proc_create_file(struct proc_dir_entry *dir_entry, const char *file,
	struct proc_dir_entry **file_entry, const struct proc_ops *proc_fops)

{
	if (file == NULL)
		return;

	if (proc_fops == NULL)
		return;

	if (!dir_entry || !file_entry) {
		amax_print(PRINT_ERROR, "dir_entry null!\n");
		return;
	}

	*file_entry = proc_create(file, AMAX_PROC_FILE_MODE, dir_entry, proc_fops);
	if (!*file_entry)
		amax_print(PRINT_ERROR, "create_proc_entry failed!\n");
}

static int32_t amax_proc_init(void)
{
	int32_t error;
	struct path proc_path;

	(void)memset_s(&proc_path, sizeof(proc_path), 0, sizeof(proc_path));
	error = kern_path(AMAX_PROC_DIR, LOOKUP_FOLLOW, &proc_path);
	if (error != 0) {
		g_amax_proc_dir = proc_mkdir(AMAX_PROC_DIR_NAME, NULL);
		if (!g_amax_proc_dir) {
			amax_print(PRINT_ERROR, "proc_mkdir failed ");
			return -1;
		}
	} else {
		amax_print(PRINT_ERROR, "%s already exist !\n", AMAX_PROC_DIR);
	}

	/* rx统计proc */
	amax_proc_create_file(g_amax_proc_dir, AMAX_PROC_RX_STAT_FILE, &g_proc_amax_rx_file_entry,
		&g_file_proc_rx_ops_amax);

	/* tx统计proc */
	amax_proc_create_file(g_amax_proc_dir, AMAX_PROC_TX_STAT_FILE, &g_proc_amax_tx_file_entry,
		&g_file_proc_tx_ops_amax);

	/* window统计proc */
	amax_proc_create_file(g_amax_proc_dir, AMAX_PROC_WIN_STAT_FILE, &g_proc_amax_win_file_entry,
		&g_file_proc_win_ops_amax);

	/* DMP统计proc */
	amax_proc_create_file(g_amax_proc_dir, AMAX_PROC_DMP_STAT_FILE, &g_proc_amax_dmp_file_entry,
		&g_file_proc_dmp_ops_amax);
	return 0;
}

static void amax_proc_exit(void)
{
	remove_proc_entry("amax/rx_stat", NULL);
	remove_proc_entry("amax/tx_stat", NULL);
	remove_proc_entry("amax/win_stat", NULL);
	remove_proc_entry("amax/dmd_stat", NULL);
	remove_proc_entry("amax", NULL);
	amax_print(PRINT_INFO, "amax_proc_exit end\n");
}

uint32_t amax_stat_init(void)
{
	g_amax_stat = (amax_stat *)kmalloc(sizeof(amax_stat), GFP_KERNEL);
	if (g_amax_stat == NULL) {
		amax_print(PRINT_ERROR, "g_amax_stat init failed!\n");
		return FAIL;
	}
	(void)memset_s(g_amax_stat, sizeof(amax_stat), 0, sizeof(amax_stat));

	amax_proc_init();
	return SUCC;
}

void amax_stat_exit(void)
{
	amax_proc_exit();

	if (g_amax_stat != NULL) {
		kfree(g_amax_stat);
		g_amax_stat = NULL;
	}
}

/* 清空全局大数据统计结构体所有数据 */
static void amax_tx_dmd_stat_reset(void)
{
	amax_print(PRINT_DEBUG, "[dmd]reset all\n");
	(void) memset_s(g_amax_tx_dmd_stat, sizeof(amax_tx_dmd_stat), 0, sizeof(amax_tx_dmd_stat));
}

/* 更新ML运行时长 */
static uint32_t amax_tx_dmd_update_duration(amax_tx_dmd_stat_fn *stat_fn, uint64_t end_tstamp)
{
	uint64_t ktime_interval;
	uint64_t ktime_sec;
	uint64_t start_tstamp = stat_fn->start_tstamp;
	if (start_tstamp == 0) {
		amax_print(PRINT_ERROR, "[dmd]start_tstamp is 0, error state\n");
		return FAIL;
	}
	if (end_tstamp > start_tstamp) {
		ktime_interval = ktime_sub(end_tstamp, start_tstamp);
		ktime_sec = ktime_to_ms(ktime_interval) / MILLISEC_TO_SEC;
		amax_print(PRINT_DEBUG, "[dmd]start: %llu end: %llu duration: %llu\n", start_tstamp, end_tstamp, ktime_sec);
		stat_fn->duration += ktime_sec;
		return SUCC;
	} else {
		amax_print(PRINT_ERROR, "[dmd]end_tstamp %llu is smaller than start_tstamp %llu\n", end_tstamp, start_tstamp);
		return FAIL;
	}
}

/* 记录辅链时延优于主链的包数 */
static void amax_dmd_add_assist_fast_pkt_num(amax_tx_dmd_stat_fn *stat_fn, uint32_t pkt_num)
{
	stat_fn->packet_assist_fast_num += pkt_num;
}

/* 更新辅路优于主路情况下，最大时延差 */
static void amax_dmd_update_max_diff(amax_tx_dmd_stat_fn *stat_fn, uint32_t sum_diff)
{
	stat_fn->contime_max_diff = (stat_fn->contime_max_diff > sum_diff) ? stat_fn->contime_max_diff : sum_diff;
}

/* 记录单个STA辅链比主链快的累计时延差值 */
static void amax_dmd_update_sum_diff(amax_tx_dmd_stat_fn *stat_fn, uint32_t sum_diff)
{
	if (stat_fn->contime_sum_diff < (LLONG_MAX - sum_diff)) {
		stat_fn->contime_sum_diff += sum_diff;
	} else {
		stat_fn->contime_sum_diff = sum_diff;
		stat_fn->overflw_cnt++;
	}
}

/* 更新平均时延，只在ML关闭时调用 */
static void amax_dmd_update_avg_diff(amax_tx_dmd_stat_fn *stat_fn)
{
	uint64_t long_max_diff;
	if (stat_fn->packet_assist_fast_num == 0) {
		amax_print(PRINT_ERROR, "[dmd]packet_assist_fast_num is 0!\n");
		return;
	}
	stat_fn->contime_avg_diff = div_u64(stat_fn->contime_sum_diff,
		stat_fn->packet_assist_fast_num);
	if (stat_fn->overflw_cnt > 0) {
		long_max_diff = div_u64(LLONG_MAX, stat_fn->packet_assist_fast_num);
		stat_fn->contime_avg_diff += long_max_diff * stat_fn->overflw_cnt;
	}
}

/* 清除缓存队列数据 */
static void amax_tx_dmd_contime_queue_reset(amax_contime_queue *contime_rec)
{
	(void) memset_s(contime_rec, sizeof(amax_contime_queue), 0, sizeof(amax_contime_queue));
}

/* 重置单次ML连接的统计信息 */
static void amax_tx_dmd_reset_tmp_stat(amax_tx_dmd_stat_fn *stat_fn)
{
	amax_tx_dmd_contime_queue_reset(&stat_fn->contime_queue);
	stat_fn->start_tstamp = 0;
}

/* 刷新缓存队列数据 */
static void amax_tx_dmd_contime_queue_refresh(amax_contime_queue *contime_rec, uint8_t slow_fn)
{
	int64_t offset = CONTIME_STAT_QUEUE_LEN - contime_rec->idx[slow_fn];
	offset += contime_rec->offset[slow_fn];
	(void) memset_s(contime_rec, sizeof(amax_contime_queue), 0, sizeof(amax_contime_queue));
	contime_rec->offset[slow_fn] = offset;
}

/* 辅链相对主链的时延差值计算 */
static void amax_tx_dmd_diff_calc(amax_tx_dmd_stat_fn *stat_fn, amax_contime_queue *contime_rec)
{
	int32_t diff;
	uint16_t idx;
	uint16_t fast_cnt = 0;
	int32_t fast_sum_diff = 0;
	for (idx = 0; idx < CONTIME_STAT_QUEUE_LEN; idx++) {
		diff = contime_rec->contime_data[idx].contime_ml[ML_MASTER] -
		       contime_rec->contime_data[idx].contime_ml[ML_SLAVE];
		if (diff > 0) {
			amax_dmd_update_max_diff(stat_fn, diff);
			fast_cnt++;
			fast_sum_diff += diff;
		}
	}
	amax_dmd_add_assist_fast_pkt_num(stat_fn, fast_cnt);
	amax_dmd_update_sum_diff(stat_fn, fast_sum_diff);
}

/*
 * 链路拥塞时间计算
 */
static void amax_tx_dmd_contime_calc(amax_tx_dmd_stat_fn *stat_fn, uint8_t role)
{
	uint16_t slow_idx;
	uint8_t slow_role;
	amax_contime_queue *contime_rec = &(stat_fn->contime_queue);
	if (role == ML_MASTER) {
		slow_idx = contime_rec->idx[ML_SLAVE];
		slow_role = ML_SLAVE;
	} else {
		slow_idx = contime_rec->idx[ML_MASTER];
		slow_role = ML_MASTER;
	}
	/* 较慢链路默认拥塞时间计算,未收到部分，按快路时延两倍计算 */
	for (; slow_idx < CONTIME_STAT_QUEUE_LEN; slow_idx++)
		contime_rec->contime_data[slow_idx].contime_ml[slow_role] =
				contime_rec->contime_data[slow_idx].contime_ml[role] << 1;

	amax_tx_dmd_diff_calc(stat_fn, contime_rec);
	amax_tx_dmd_contime_queue_refresh(contime_rec, slow_role);
}

static void amax_dsm_client_notify(int dsm_id, const char *fmt, ...)
{
	char buf[LOG_BUF_SIZE] = {0};
	struct dsm_client *dsm_client_buf = NULL;
	va_list ap;
	int32_t ret = 0;

	dsm_client_buf = dsm_find_client(AMAX_DSM_CLIENT);
	if (dsm_client_buf == NULL) {
		amax_print(PRINT_ERROR, "dsm dsm_wifi have not registed\n");
		return;
	}
	if (dsm_client_buf != NULL) {
		if (fmt != NULL) {
			va_start(ap, fmt);
			ret = vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, fmt, ap);
			va_end(ap);
			if (ret < 0) {
				amax_print(PRINT_ERROR, "vsnprintf_s fail, line[%d]\n", __LINE__);
				return;
			}
		} else {
			amax_print(PRINT_ERROR, "dsm_client_buf is null, line[%d]\n", __LINE__);
			return;
		}
	}

	if (!dsm_client_ocuppy(dsm_client_buf)) {
		dsm_client_record(dsm_client_buf, buf);
		dsm_client_notify(dsm_client_buf, dsm_id);
		g_last_dsm_id = dsm_id;
		amax_print(PRINT_DEBUG, "wifi dsm_client_notify success,dsm_id=%d[%s]\n", dsm_id, buf);
	} else {
		amax_print(PRINT_ERROR, "wifi dsm_client_notify failed,last_dsm_id=%d dsm_id=%d\n", g_last_dsm_id, dsm_id);

		// retry dmd record
		dsm_client_unocuppy(dsm_client_buf);
		if (!dsm_client_ocuppy(dsm_client_buf)) {
			dsm_client_record(dsm_client_buf, buf);
			dsm_client_notify(dsm_client_buf, dsm_id);
			amax_print(PRINT_DEBUG, "wifi dsm notify success, dsm_id=%d[%s]\n", dsm_id, buf);
		} else {
			amax_print(PRINT_ERROR, "wifi dsm client ocuppy, dsm notify failed, dsm_id=%d\n", dsm_id);
		}
	}
}

/* 异常数据上报 */
static void amax_abnormal_stat_upload(void)
{
	char *upload_str = NULL;

	if (g_amax_abnormal_stat == NULL) {
		amax_print(PRINT_DEBUG, "[abn]stat_fn duration is 0,do not upload\n");
		return;
	}
	upload_str = (char *)kmalloc(MAX_SINGLE_STAT_SIZE, GFP_KERNEL);
	if (upload_str == NULL) {
		amax_print(PRINT_ERROR, "[abn]malloc failed\n");
		return;
	}
	(void)memset_s(upload_str, MAX_SINGLE_STAT_SIZE, 0, MAX_SINGLE_STAT_SIZE);

	if (sprintf_s(upload_str, MAX_SINGLE_STAT_SIZE,
		"ABNORMAL_STAT: [%llu, %llu, %llu, %llu, %llu, %llu] ",
		g_amax_abnormal_stat->main_fail, g_amax_abnormal_stat->assist_fail,
		g_amax_abnormal_stat->fdrop_num, g_amax_abnormal_stat->timeout_move,
		g_amax_abnormal_stat->update_move, g_amax_abnormal_stat->self_cure_cnt) < 0) {
		amax_print(PRINT_ERROR, "sprintf_s failed\n");
		kfree(upload_str);
		return;
	}
	amax_print(PRINT_DEBUG, "[abn]upload_str:%s\n", upload_str);
#ifdef CONFIG_HUAWEI_DSM
	amax_dsm_client_notify(DSM_MULTILINK_STATISTIC, upload_str);
#endif
	kfree(upload_str);
}

/* 上传链路受益统计 */
static void amax_tx_dmd_upload_fn(uint8_t main_fn, amax_tx_dmd_stat_fn *stat_fn)
{
	char *upload_str = NULL;
	if (stat_fn->duration == 0) {
		amax_print(PRINT_DEBUG, "[dmd]stat_fn duration is 0,do not upload\n");
		return;
	}
	upload_str = (char *) kmalloc(MAX_SINGLE_STAT_SIZE, GFP_KERNEL);
	if (upload_str == NULL) {
		amax_print(PRINT_ERROR, "[dmd]malloc failed\n");
		return;
	}
	(void)memset_s(upload_str, MAX_SINGLE_STAT_SIZE, 0, MAX_SINGLE_STAT_SIZE);

	if (sprintf_s(upload_str, MAX_SINGLE_STAT_SIZE,
		"AMAX_STAT: [%u, %u, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %u] ",
		main_fn, stat_fn->count, stat_fn->duration, stat_fn->throughput_main_sum,
		stat_fn->throughput_assist_sum, stat_fn->packet_main_sum, stat_fn->packet_assist_sum,
		stat_fn->packet_assist_fast_num, stat_fn->contime_max_diff, stat_fn->contime_avg_diff,
		stat_fn->contime_sum_diff, stat_fn->overflw_cnt) < 0) {
		amax_print(PRINT_ERROR, "sprintf_s failed\n");
		kfree(upload_str);
		return;
	}
	amax_print(PRINT_DEBUG, "[dmd]upload_str:%s\n", upload_str);
#ifdef CONFIG_HUAWEI_DSM
	amax_dsm_client_notify(DSM_MULTILINK_STATISTIC, upload_str);
#endif
	kfree(upload_str);
}

/* 大数据上传 */
static uint32_t amax_tx_dmd_upload(uint64_t end_tstamp)
{
	uint64_t ktime_interval;
	uint64_t ktime_sec;
	uint64_t start_tstamp = g_amax_tx_dmd_stat->last_upload_tstamp;
	if (g_amax_tx_dmd_stat->last_upload_tstamp == 0) {
		g_amax_tx_dmd_stat->last_upload_tstamp = end_tstamp;
		amax_print(PRINT_DEBUG, "[dmd]first active set last_upload_tstamp to %llu", end_tstamp);
		return FAIL;
	}
	if (end_tstamp > start_tstamp) {
		ktime_interval = ktime_sub(end_tstamp, start_tstamp);
		ktime_sec = ktime_to_ms(ktime_interval) / MILLISEC_TO_SEC;
		amax_print(PRINT_DEBUG, "[dmd]last upload time: %llu cur time: %llu duration: %llu\n", start_tstamp, end_tstamp,
			ktime_sec);
		if (ktime_sec >= g_amax_tx_dmd_trigged_time_thr || g_amax_tx_dmd_trigged_now) {
			amax_print(PRINT_DEBUG, "[dmd]trigger upload duration[2g:%llu, 5g:%llu]\n",
				g_amax_tx_dmd_stat->stat_fn[FN_2].duration,
				g_amax_tx_dmd_stat->stat_fn[FN_5].duration);
			amax_tx_dmd_upload_fn(FN_2, &g_amax_tx_dmd_stat->stat_fn[FN_2]);
			amax_tx_dmd_upload_fn(FN_5, &g_amax_tx_dmd_stat->stat_fn[FN_5]);
			amax_abnormal_stat_upload();
			g_amax_tx_dmd_stat->last_upload_tstamp = end_tstamp;
			return SUCC;
		} else {
			amax_print(PRINT_DEBUG, "[dmd]not allow to upload ,time thr is %llu, trigger now is %u",
				g_amax_tx_dmd_trigged_time_thr, g_amax_tx_dmd_trigged_now);
			return FAIL;
		}
	} else {
		g_amax_tx_dmd_stat->last_upload_tstamp = end_tstamp;
		amax_print(PRINT_ERROR, "[dmd]end_tstamp %llu is smaller than start_tstamp %llu\n", end_tstamp, start_tstamp);
		return FAIL;
	}
}

static void amax_tx_dmd_print_cur_stat(uint8_t main_fn, amax_tx_dmd_stat_fn *stat_fn)
{
	amax_print(PRINT_DEBUG, "[dmd]AMAX_CUR_STAT: [main_fn:%u, count:%u, duration:%llu, throughput_main_sum:%llu, "
		"throughput_assist_sum:%llu, packet_main_sum:%llu, packet_assist_sum:%llu, "
		"packet_assist_fast_num:%llu, contime_max_diff:%llu, contime_avg_diff:%llu, "
		"contime_sum_diff:%llu, overflw_cnt:%u]\n",
		main_fn, stat_fn->count, stat_fn->duration, stat_fn->throughput_main_sum,
		stat_fn->throughput_assist_sum, stat_fn->packet_main_sum, stat_fn->packet_assist_sum,
		stat_fn->packet_assist_fast_num, stat_fn->contime_max_diff, stat_fn->contime_avg_diff,
		stat_fn->contime_sum_diff, stat_fn->overflw_cnt);
}

/* 清空全局异常统计结构体所有数据 */
static void amax_abnormal_stat_reset(void)
{
	amax_print(PRINT_DEBUG, "[abn]reset all\n");
	(void) memset_s(g_amax_abnormal_stat, sizeof(amax_abnormal_stat), 0, sizeof(amax_abnormal_stat));
}

/* 创建全局异常统计结构体 */
uint32_t amax_abnormal_stat_init(void)
{
	amax_print(PRINT_DEBUG, "[abn]init\n");
	if (g_amax_abnormal_stat != NULL) {
		amax_print(PRINT_ERROR, "[abn]g_amax_abnormal_stat is not NULL\n");
		kfree(g_amax_abnormal_stat);
		g_amax_abnormal_stat = NULL;
	}
	g_amax_abnormal_stat = (amax_abnormal_stat *)kmalloc(sizeof(amax_abnormal_stat), GFP_KERNEL);
	if (g_amax_abnormal_stat == NULL) {
		amax_print(PRINT_ERROR, "[abn]init failed!\n");
		return FAIL;
	}

	amax_abnormal_stat_reset();
	return SUCC;
}

/* 销毁全局异常统计结构体 */
void amax_abnormal_stat_exit(void)
{
	amax_print(PRINT_DEBUG, "[abn]exit\n");
	if (g_amax_abnormal_stat != NULL) {
		kfree(g_amax_abnormal_stat);
		g_amax_abnormal_stat = NULL;
	}
}

/* 创建全局大数据统计结构体 */
uint32_t amax_tx_dmd_stat_init(void)
{
	if (g_amax_tx_dmd_stat != NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is not NULL\n");
		kfree(g_amax_tx_dmd_stat);
		g_amax_tx_dmd_stat = NULL;
	}
	g_amax_tx_dmd_stat = (amax_tx_dmd_stat *) kmalloc(sizeof(amax_tx_dmd_stat), GFP_KERNEL);
	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]init failed!\n");
		return FAIL;
	}
	amax_tx_dmd_stat_reset();
	return SUCC;
}

/* 销毁全局大数据统计结构体 */
void amax_tx_dmd_stat_exit(void)
{
	if (g_amax_tx_dmd_stat != NULL) {
		kfree(g_amax_tx_dmd_stat);
		g_amax_tx_dmd_stat = NULL;
	}
}

/* 设置主路通路：FN2或FN5 */
void amax_tx_dmd_set_main_freq(int main_fn)
{
	if (main_fn != FN_2 && main_fn != FN_5) {
		amax_print(PRINT_ERROR, "[dmd]fn value %d is error\n", main_fn);
		return;
	}
	if (g_amax_tx_dmd_stat != NULL) {
		g_amax_tx_dmd_stat->main_fn = main_fn;
		amax_print(PRINT_DEBUG, "[dmd]set main fn to %d\n", main_fn);
	} else {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null\n");
	}
}

/*
 * 处理ML启动事件,如果已经开始了，则重置数据，重新开始统计
 *
 * 参数：start_tstamp 启动时间戳
 */
void amax_tx_dmd_handle_enable(uint64_t start_tstamp)
{
	amax_tx_dmd_stat_fn *stat_fn = NULL;
	amax_print(PRINT_DEBUG, "[dmd]dmd handle enable\n");
	if (g_amax_tx_dmd_stat != NULL) {
		stat_fn = g_amax_tx_dmd_stat->main_fn == FN_5 ?
			&g_amax_tx_dmd_stat->stat_fn[FN_5] : &g_amax_tx_dmd_stat->stat_fn[FN_2];
		if (stat_fn->start_tstamp != 0) {
			// 此初不清空数据，防止实际ML一直生效，反复调用开启场景。
			amax_print(PRINT_WARNING, "[dmd]already started on %llu\n", stat_fn->start_tstamp);
			return;
		}
		stat_fn->start_tstamp = start_tstamp;
		amax_tx_dmd_contime_queue_reset(&stat_fn->contime_queue);
	} else {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null\n");
	}
}

/*
 * 处理ML关闭事件
 *
 * 参数：end_tstamp关闭时间戳
 */
void amax_tx_dmd_handle_disable(uint64_t end_tstamp)
{
	amax_tx_dmd_stat_fn *stat_fn = NULL;
	amax_print(PRINT_DEBUG, "[dmd]handle disable\n");
	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null\n");
		return;
	}
	stat_fn = g_amax_tx_dmd_stat->main_fn == FN_5 ?
		&g_amax_tx_dmd_stat->stat_fn[FN_5] : &g_amax_tx_dmd_stat->stat_fn[FN_2];
	if (amax_tx_dmd_update_duration(stat_fn, end_tstamp) == FAIL) {
		amax_print(PRINT_ERROR, "[dmd]update duration failed\n");
		// 设置ML生效时长失败，则本次数据作废
		amax_tx_dmd_reset_tmp_stat(stat_fn);
		return;
	}
	amax_tx_dmd_reset_tmp_stat(stat_fn);
	stat_fn->count++;
	amax_dmd_update_avg_diff(stat_fn);
	amax_tx_dmd_print_cur_stat(FN_5, &g_amax_tx_dmd_stat->stat_fn[FN_5]);
	amax_tx_dmd_print_cur_stat(FN_2, &g_amax_tx_dmd_stat->stat_fn[FN_2]);
	if (amax_tx_dmd_upload(end_tstamp) == SUCC) {
		amax_tx_dmd_stat_reset();
		amax_abnormal_stat_reset();
		g_amax_tx_dmd_stat->last_upload_tstamp = end_tstamp;
	}
}

/* ML链路累加吞吐 */
void amax_tx_dmd_inc_throughput(uint8_t fn, uint64_t throughput)
{
	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null\n");
		return;
	}
	if (g_amax_tx_dmd_stat->main_fn == fn) {
		g_amax_tx_dmd_stat->stat_fn[fn].throughput_main_sum += throughput;
		return;
	}
	g_amax_tx_dmd_stat->stat_fn[fn].throughput_assist_sum += throughput;
}

/* ML链路累加包数 */
void amax_tx_dmd_inc_packet(uint8_t fn)
{
	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null\n");
		return;
	}
	if (g_amax_tx_dmd_stat->main_fn == fn) {
		g_amax_tx_dmd_stat->stat_fn[fn].packet_main_sum++;
		return;
	}
	g_amax_tx_dmd_stat->stat_fn[fn].packet_assist_sum++;
}

/*
 * 处理单个包的发送时延，用于统计时延收益。
 *
 * 参数：contime_ms 发包时延
 *      fn     发包网卡 0-主路 1-辅路
 */
void amax_tx_dmd_handle_new_contime(uint32_t contime_ms, uint8_t role)
{
	amax_contime_queue *contime_rec = NULL;
	amax_tx_dmd_stat_fn *stat_fn = NULL;
	// 此异常分支不能加维测，每个包都要走此流程，防止频繁打印crash
	if ((role != ML_MASTER && role != ML_SLAVE) || g_amax_tx_dmd_stat == NULL)
		return;
	stat_fn = g_amax_tx_dmd_stat->main_fn == FN_5 ?
		&g_amax_tx_dmd_stat->stat_fn[FN_5] : &g_amax_tx_dmd_stat->stat_fn[FN_2];
	contime_rec = &stat_fn->contime_queue;
	// 如果上次统计时未填满，则先将上次的差值消除。
	if (contime_rec->offset[role] > 0) {
		contime_rec->offset[role]--;
		return;
	}
	contime_rec->contime_data[contime_rec->idx[role]].contime_ml[role] = contime_ms;
	contime_rec->idx[role]++;
	if (contime_rec->idx[role] >= CONTIME_STAT_QUEUE_LEN) {
		amax_tx_dmd_contime_calc(stat_fn, role);
		return;
	}
}

/* debug节点读取操作响应函数 */
ssize_t amax_proc_dmp_stat_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
	int32_t cnt;
	char *data = NULL;
	amax_tx_dmd_stat_fn *stat_2g = NULL;
	amax_tx_dmd_stat_fn *stat_5g = NULL;
	if (*offset > 0)
		return 0;

	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null \n");
		return FAILED;
	}

	data = (char *) kmalloc(MAX_SINGLE_STAT_SIZE, GFP_KERNEL);
	if (data == NULL) {
		amax_print(PRINT_ERROR, "[dmd]malloc failed\n");
		return FAILED;
	}
	stat_2g = &g_amax_tx_dmd_stat->stat_fn[FN_2];
	stat_5g = &g_amax_tx_dmd_stat->stat_fn[FN_5];
	amax_tx_dmd_print_cur_stat(FN_2, stat_2g);
	amax_tx_dmd_print_cur_stat(FN_5, stat_5g);
	if (sprintf_s(data, MAX_SINGLE_STAT_SIZE,
		"AMAX_STAT: [%u, %u, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %u, %llu, %llu]\n"
		"AMAX_STAT: [%u, %u, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %u, %llu, %llu]\n",
		FN_2, stat_2g->count, stat_2g->duration, stat_2g->throughput_main_sum, stat_2g->throughput_assist_sum,
		stat_2g->packet_main_sum, stat_2g->packet_assist_sum, stat_2g->packet_assist_fast_num,
		stat_2g->contime_max_diff, stat_2g->contime_avg_diff, stat_2g->contime_sum_diff, stat_2g->overflw_cnt,
		stat_2g->contime_queue.offset[ML_MASTER], stat_2g->contime_queue.offset[ML_SLAVE],
		FN_5, stat_5g->count, stat_5g->duration, stat_5g->throughput_main_sum, stat_5g->throughput_assist_sum,
		stat_5g->packet_main_sum, stat_5g->packet_assist_sum, stat_5g->packet_assist_fast_num,
		stat_5g->contime_max_diff, stat_5g->contime_avg_diff, stat_5g->contime_sum_diff, stat_5g->overflw_cnt,
		stat_5g->contime_queue.offset[ML_MASTER], stat_5g->contime_queue.offset[ML_SLAVE]) < 0) {
		kfree(data);
		amax_print(PRINT_ERROR, "[dmd]sprintf_s failed\n");
		return FAILED;
	}

	cnt = simple_read_from_buffer(buffer, len, offset, data, strlen(data));
	kfree(data);
	return cnt;
}

/* debug节点写操作响应函数 */
ssize_t amax_proc_dmp_stat_write(struct file *fp, const char *buffer, size_t len, loff_t *off)
{
	unsigned char value = 0;

	if (len >= AMAX_PROC_BUFFER_LEN)
		return FAILED;

	if (kstrtou8_from_user(buffer, len, AMXA_PROC_BASE, &value) != 0)
		return FAILED;

	if (g_amax_tx_dmd_stat == NULL) {
		amax_print(PRINT_ERROR, "[dmd]g_amax_tx_dmd_stat is null \n");
		return FAILED;
	}

	if (value == 0) { // 0:重置操作
		g_amax_tx_dmd_trigged_now = 0;
		g_amax_tx_dmd_trigged_time_thr = UPLOAD_TIME_6H;
		amax_print(PRINT_DEBUG, "disable dmd_trigged_now\n");
	} else if (value == 1) { // 1:打开立即上报开关
		g_amax_tx_dmd_trigged_now = 1;
		amax_print(PRINT_DEBUG, "enable dmd_trigged_now\n");
	} else {
		g_amax_tx_dmd_trigged_time_thr = value * 60; // 60:转为分钟
		g_amax_tx_dmd_trigged_now = 0;
		amax_print(PRINT_DEBUG, "[dmd]set upload thr to %u\n", g_amax_tx_dmd_trigged_time_thr);
	}
	return len;
}

void debug_inc_expand_tail_cnt(void)
{
	g_amax_stat->chr_stat.exp_tail_num++;
}

void debug_inc_tuples_encap_cnt(void)
{
	g_amax_stat->chr_stat.tup_encap_cnt++;
}

void debug_inc_tuples_tail_less_cnt(void)
{
	g_amax_stat->chr_stat.tail_less_cnt++;
}

void debug_inc_mark_num(void)
{
	g_amax_stat->chr_stat.mark_num++;
}

void debug_inc_same_seg_num(void)
{
	g_amax_stat->chr_stat.same_seg_num++;
}

void amax_print_tx_stat_info(void)
{
	if (g_amax_stat == NULL)
		return;

	amax_print(PRINT_INFO, "[tx] succ:%u %u, fail:%u %u, tup_encap:%u, tail_less:%u, marked:%u, same_seg_num=%u\n",
		g_amax_stat->chr_stat.pkt_5g_dc, g_amax_stat->chr_stat.pkt_2g_dc, g_amax_stat->chr_stat.pkt_5g_fail,
		g_amax_stat->chr_stat.pkt_2g_fail, g_amax_stat->chr_stat.tup_encap_cnt, g_amax_stat->chr_stat.tail_less_cnt,
		g_amax_stat->chr_stat.mark_num, g_amax_stat->chr_stat.same_seg_num);
}

void amax_print_rx_stat_info(void)
{
	uint8_t tid;
	cache_array_recv_handle *hdl = NULL;

	if (g_amax_stat == NULL)
		return;

	for (tid = 0; tid < TID_ARR_NUM; tid++) {
		hdl = get_recv_array_hdl_all(tid);
		if (!hdl)
			return;

		amax_print(PRINT_INFO, "[NO.%hhu] rx:%u %u, submit:%u %u fdrop:%llu, oow:%llu, toow:%llu, sc_cnt:%u, "
			"tmove:%llu, umove:%llu, curr_llc:%u %u, lowedge:%u, upedge:%u\n", tid,
			g_amax_stat->debug_stat.window_debug[tid].rx_num_5g, g_amax_stat->debug_stat.window_debug[tid].rx_num_2g,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_5g,
			g_amax_stat->debug_stat.window_debug[tid].submit_num_2g,
			hdl->drop_pkts_num, hdl->out_of_window_frame_num, hdl->total_oow_frame_num,
			g_amax_stat->debug_stat.window_debug[tid].self_cure_cnt,
			g_amax_stat->debug_stat.window_debug[tid].timeout_move,
			g_amax_stat->debug_stat.window_debug[tid].update_move, hdl->curr_llc_id_5g, hdl->curr_llc_id_2g,
			hdl->real_lowedge, hdl->frame_array[hdl->rx_upedge].llc_id_16);
	}
}

void amax_reset_tx_rx_stat_info(void)
{
	if (g_amax_stat == NULL)
		return;

	amax_reset_chr_stat();
	amax_reset_win_debug_stat();
}