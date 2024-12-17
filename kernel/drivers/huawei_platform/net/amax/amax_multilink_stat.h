/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: This module is to support wifi multilink feature.
 * Create: 2022-09-02
 */
#ifndef AMAX_MULTILINK_STAT_H_
#define AMAX_MULTILINK_STAT_H_

#include <linux/types.h>
#include "amax_multilink_main.h"

#define THROUGHPUT_SAMPLE_NUM 10
#define CONTIME_SAMPLE_NUM 32
#define PPS_SAMPLE_NUM 10
#define RATIO_NUM 2
#define MAX_SINGLE_STAT_SIZE 512

typedef struct {
	uint32_t amax_throughput[THROUGHPUT_SAMPLE_NUM];
	uint32_t amax_throughput_max;
	uint32_t amax_throughput_avg;
	uint32_t amax_pps[PPS_SAMPLE_NUM];
	uint32_t amax_pps_avg;
	uint32_t contime[CONTIME_SAMPLE_NUM];
	uint32_t contime_sum;
} amax_tx_debug;

typedef struct {
	uint64_t main_fail;
	uint64_t assist_fail;
	uint64_t fdrop_num;
	uint64_t timeout_move;
	uint64_t update_move;
	uint64_t self_cure_cnt;
} amax_abnormal_stat;

/* 主/辅路时延结构体 */
typedef struct {
	int32_t contime_ml[ML_ROLE_NUM]; // 主辅路阻塞时间
} amax_contime_info;

/* 存储sta时延信息的队列长度 */
#define CONTIME_STAT_QUEUE_LEN 32

/* 用于时延统计结构体 */
typedef struct {
	uint8_t idx[ML_ROLE_NUM];           // 主辅路在contime_data数组当前索引值，0-主路 1-辅路
	uint64_t offset[ML_ROLE_NUM];       // 记录上一次数组满时，主辅路的差包数
	amax_contime_info contime_data[CONTIME_STAT_QUEUE_LEN];
} amax_contime_queue;

/* 收益统计结构体 */
typedef struct {
	uint8_t count;                      /* 生效次数 */
	uint64_t duration;                  /* 连接时长 */
	uint64_t throughput_main_sum;       /* 主路吞吐总和（包含冗余模式和非冗余模式） */
	uint64_t throughput_assist_sum;     /* 辅路吞吐总和 */
	uint64_t packet_main_sum;           /* 主路包数（包含冗余模式和非冗余模式） */
	uint64_t packet_assist_sum;         /* 辅路包数 */
	uint64_t packet_assist_fast_num;    /* 辅路比主路发的快的包数 */
	uint64_t contime_max_diff;          /* 辅路优于主路情况下，最大时延差 */
	uint64_t contime_avg_diff;          /* 辅路优于主路情况下，平均时延差 */
	uint64_t contime_sum_diff;          /* 辅路优于主路情况下，时延总和 */
	uint32_t overflw_cnt;               /* contime_sum_diff发生整数翻转的次数 */
	uint64_t start_tstamp;              /* ML开启时间戳(us) */
	amax_contime_queue contime_queue;   /* 用于时延统计的结构体 */
} amax_tx_dmd_stat_fn;

typedef struct {
	uint8_t main_fn;                       /* 当前主路通路，0-2g,1-5g */
	amax_tx_dmd_stat_fn stat_fn[FN_NUM];   /* 分别存放主路为2.4g和5g情况下的收益统计 */
	uint64_t last_upload_tstamp;           /* 上次上传的时间戳(us) */
} amax_tx_dmd_stat;

// 该结构体用于上行rx debug使用
typedef struct {
	uint32_t rx_num_total;
	uint32_t rx_num_2g;
	uint32_t rx_num_5g;
	uint32_t submit_num_total;
	uint32_t submit_num_2g;
	uint32_t submit_num_5g;
	uint64_t timeout_move;
	uint64_t update_move;
	uint64_t normal_move;
	uint32_t reset_num;
	uint32_t timeout_force_update_num;
	uint32_t update_move_empty_num;
	uint32_t submit_timeout_cnt;
	uint32_t win_timeout_cnt;
	uint32_t self_cure_cnt;
} amax_rx_debug;

typedef struct {
	amax_tx_debug tx_debug[RATIO_NUM];
	amax_rx_debug rx_debug;
	amax_rx_debug window_debug[TID_ARR_NUM];
} amax_debug;

typedef struct {
	uint8_t air_usage; /* 空口占空比，包含自身txrx使用。更新频率 */
} amax_air_stat;

typedef struct {
	uint32_t pkt_5g_dc;   /* 冗余模式下，5G包发送成功个数 */
	uint32_t pkt_2g_dc;   /* 冗余模式下，2G包发送成功个数 */
	uint32_t pkt_5g_fail; /* 冗余模式下，5G包发送失败个数 */
	uint32_t pkt_2g_fail; /* 冗余模式下，2G包发送失败个数 */
	uint8_t reset_num;
	uint32_t exp_tail_num;
	uint32_t tup_encap_cnt;
	uint32_t tail_less_cnt;
	uint32_t mark_num;
	uint32_t same_seg_num;
} amax_chr_stat;

/* proc系统维测打印 */
typedef struct amax_stat {
	amax_air_stat air_stat[RATIO_NUM];
	amax_chr_stat chr_stat;
	amax_debug debug_stat; /* 此部分在release版本中不应该编译进去 无需考虑内存开销 */
} amax_stat;

uint32_t amax_stat_init(void);
void amax_stat_exit(void);
uint32_t amax_abnormal_stat_init(void);
void amax_abnormal_stat_exit(void);
void amax_abnormal_inc_fdrop_num(cache_array_recv_handle *recv_array_hdl, uint16_t new_lowedge);
void amax_abnormal_inc_timeout_move(void);
void amax_abnormal_inc_update_move(void);
void amax_abnormal_inc_self_cure_cnt(void);
uint32_t amax_tx_dmd_stat_init(void);
void amax_tx_dmd_stat_exit(void);
void amax_tx_dmd_set_main_freq(int main_fn);
void amax_tx_dmd_handle_enable(uint64_t start_tstamp);
void amax_tx_dmd_handle_disable(uint64_t end_tstamp);
void amax_tx_dmd_inc_throughput(uint8_t fn, uint64_t throughput);
void amax_tx_dmd_inc_packet(uint8_t fn);
void amax_tx_dmd_handle_new_contime(uint32_t contime_ms, uint8_t role);
void debug_update_max_win_interval(uint32_t curr_win_interval);
void debug_update_max_stay_time(uint32_t curr_stay_time);
void debug_inc_beyond_stay_cnt(void);
void debug_inc_submit_timeout_cnt(uint8_t tid);
void debug_inc_win_wait_timeout_cnt(uint8_t tid);
void amax_reset_single_win_stat(uint8_t tid);
void debug_inc_normal_move(void);
void debug_inc_reset_num(void);
void debug_inc_empty_num(void);
void debug_inc_update_move(void);
void debug_inc_timeout_move(void);
void debug_inc_submit_num_total(void);
void debug_inc_submit_num_2g(void);
void debug_inc_submit_num_5g(void);
void debug_inc_timeout_force_update_num(void);
void debug_inc_pkt_5g_dc(void);
void debug_inc_pkt_2g_dc(void);
void debug_inc_pkt_5g_fail(void);
void debug_inc_pkt_2g_fail(void);
void debug_inc_pkt_dc(uint8_t fn);
void debug_inc_pkt_fail(uint8_t fn);
void window_debug_inc_submit_num_total(uint8_t tid);
void window_debug_inc_submit_num_5g(uint8_t tid);
void window_debug_inc_submit_num_2g(uint8_t tid);
void window_debug_inc_timeout_move(uint8_t tid);
void window_debug_inc_update_move(uint8_t tid);
void window_debug_inc_normal_move(uint8_t tid);
void window_debug_inc_reset_num(uint8_t tid);
void window_debug_inc_self_cure_cnt(uint8_t tid);
void amax_set_tx_5g_throughput(uint8_t index, uint32_t throughput);
void amax_set_tx_2g_throughput(uint8_t index, uint32_t throughput);
void amax_set_tx_5g_max_throughput(uint32_t max_throughput);
void amax_set_tx_2g_max_throughput(uint32_t max_throughput);
void amax_set_tx_5g_avg_throughput(uint32_t avg_throughput);
void amax_set_tx_2g_avg_throughput(uint32_t avg_throughput);
void amax_set_tx_contime_sum(uint8_t fn, uint32_t sum);
uint32_t amax_get_tx_5g_throughput(uint8_t index);
uint32_t amax_get_tx_2g_throughput(uint8_t index);
uint32_t amax_get_tx_5g_max_throughput(void);
uint32_t amax_get_tx_2g_max_throughput(void);
uint32_t amax_get_tx_avg_throughput(uint8_t fn);
uint32_t *amax_get_tx_contime_array(uint8_t fn);
uint32_t amax_get_tx_contime_sum(uint8_t fn);
void amax_inc_tx_pps(uint8_t fn);
void amax_update_tx_pps(void);
uint32_t amax_get_tx_pps_avg(uint8_t fn);
void debug_inc_tx_reset_cnt(void);
void debug_inc_expand_tail_cnt(void);
void debug_inc_tuples_encap_cnt(void);
void debug_inc_tuples_tail_less_cnt(void);
void debug_inc_mark_num(void);
void debug_inc_same_seg_num(void);
void debug_inc_rx_num(uint8_t fn, uint8_t tid);
void debug_inc_napi_cnt(void);
void amax_print_tx_stat_info(void);
void amax_print_rx_stat_info(void);
void amax_reset_tx_rx_stat_info(void);
#endif
