

#ifndef __WIFI_TRAFFIC_MODEL_H__
#define __WIFI_TRAFFIC_MODEL_H__

#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/timekeeping.h>

#define RET_SUCC 0
#define RET_FAIL 1
#define WIFI_TRUE 1
#define WIFI_FALSE 0

#define TRAFFIC_MODEL_FIRST_TRAINING_CNT 200 /* 第一个周期全部数据进行训练 */
#define TRAFFIC_MODEL_TRAINING_CNT 30 /* 每个周期取前30个数据进行训练 */
#define TRAFFIC_MODEL_TRAINING_PEROID 300 /* 每个周期300 * 100ms */
#define TRAFFIC_DATA_COUNT 5 /* 统计最近5个数据 */
#define TRAFFIC_INFO_TIMEOUT_INTERVAL (10 * 1000) /* 连续两次流量更新时间超过该门限，则重新开始训练 */
#define TRAFFIC_MODEL_SEARCH_STEP 5
#define TRAFFIC_MODEL_MAX_LOSS 65535
#define TRAFFIC_MODEL_UPDATE_WEIGHT 4 /* 新参数权重0.4 */

#define TRAFFIC_MODEL_PENALTY_DELAY 30
#define TRAFFIC_MODEL_PENALTY_SWITCH 20
#define TRAFFIC_MODEL_PENALTY_WASTE 3

/* 需要将X Mbps转化为每100ms传输X字节数 */
#define TRAFFIC_MODEL_MAX_LEVEL_CNT 8
#define TRAFFIC_BYTES_TRANSFER_FACTOR (1000 * 1000 / 10 / 8)
#define TRAFFIC_MODEL_FIRST_LEVEL (2 * 1000 * 1000 / 10 / 8)
#define TRAFFIC_MODEL_SECOND_LEVEL (100 * 1000 * 1000 / 10 / 8)
#define TRAFFIC_MODEL_THIRD_LEVEL (200 * 1000 * 1000 / 10 / 8)
#define TRAFFIC_MODEL_DELTA_LEVEL 20

/* 定时器参数 1s: 1000000000  100ms: 100000000 10ms: 10000000 1ms: 1000000 */
#define TRAFFIC_STAT_TIMEOUT_PERIOD  100000000

/* 定时器超时函数 */
typedef enum hrtimer_restart (*timeout_func)(struct hrtimer *);

typedef enum {
		LOW_TPUT_LEVEL = 0,
		MEDIUM_TPUT_LEVEL = 1,
		HIGH_TPUT_LEVEL = 2,

		MODE_LEVEL_BUTT
} flexible_wifi_mode_enum;

/* 对于现在的算法，总共有三个区间: 0 - 1Mbps、1Mbps - 100Mbps、大于100Mbps，对于
   第三个区间的上限用200Mbps来表示 */
typedef struct {
	uint8_t level_cnt; /* 表示有几个区间 */
	uint32_t upper_bound[TRAFFIC_MODEL_MAX_LEVEL_CNT]; /* 每个区间的上界 */
} traffic_model_upper_bound_stru;

typedef struct {
	uint32_t first_thred;
	uint32_t second_thred;
	uint32_t delta; /* 缓冲时间 */
} traffic_model_params_stru;

typedef struct {
	uint8_t penalty_delay;
	uint8_t penalty_switch;
	uint8_t penalty_waste;
} traffic_training_stru;

typedef struct {
		uint64_t tx_bytes;
		uint64_t rx_bytes;
} traffic_stats_stru;

typedef struct {
	uint8_t below_first_thred_cnt; /* 记录连续低于门限1的次数 */
	uint8_t below_second_thred_cnt; /* 记录连续低于门限2的次数 */
	uint8_t output_level; /* flexible_wifi_mode_enum */
	uint32_t cur_control_level; /* 当前决策的流量区间 */
} traffic_model_decision_stru;

typedef struct {
	uint8_t first_flag;
	uint8_t timer_init_flag;
	uint8_t alg_mode; /* 标记算法启动或关闭 */
	uint16_t period_cnt; /* 对每100ms进行计数 */
	uint16_t training_cnt;
	uint32_t last_traffic_update_timestamp; /* 上次更新流量信息的时间，做超时管理 */
	uint64_t training_data[TRAFFIC_MODEL_TRAINING_PEROID];
	uint64_t collect_data[TRAFFIC_DATA_COUNT]; /* 统计最近5个流量数据 */
	traffic_training_stru training_info;
	traffic_stats_stru traffic_stats;
	traffic_model_upper_bound_stru upper_bound;
	traffic_model_params_stru params;
	traffic_model_decision_stru decision_info;

	struct hrtimer traffic_timer;
	struct net_device *dev;
} traffic_model_info_stru;

uint32_t traffic_model_alg_handler(uint8_t *recv_msg);
void traffic_model_set_params_handler(uint8_t *recv_msg);


#endif /* __WIFI_TRAFFIC_MODEL_H__ */

