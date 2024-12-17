

#include "wifi_traffic_model.h"
#include "wifi_ext_event.h"
#include "wifi_ext_mark.h"

#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include "securec.h"

static traffic_model_info_stru g_traffic_info = {
	.first_flag = WIFI_TRUE,
	.timer_init_flag = WIFI_FALSE,
	.alg_mode = WIFI_FALSE, /* 算法默认关闭 */
	.period_cnt = 0,
	.training_cnt = TRAFFIC_MODEL_FIRST_TRAINING_CNT,
	.params = { TRAFFIC_MODEL_FIRST_LEVEL, TRAFFIC_MODEL_SECOND_LEVEL, TRAFFIC_MODEL_DELTA_LEVEL },
};

int8_t g_index_of_collect_data;
bool is_cyclic;

static uint32_t traffic_model_reset(void);
enum hrtimer_restart traffic_stat_timeout_handler(struct hrtimer *timer);

static traffic_model_info_stru* get_traffic_info(void)
{
	return &g_traffic_info;
}

static uint64_t* get_training_data(void)
{
	return g_traffic_info.training_data;
}

static traffic_training_stru* get_training_info(void)
{
	return &(g_traffic_info.training_info);
}

static traffic_stats_stru* get_traffic_stats(void)
{
	return &(g_traffic_info.traffic_stats);
}

static traffic_model_upper_bound_stru* get_upper_bound(void)
{
	return &(g_traffic_info.upper_bound);
}

traffic_model_params_stru* get_traffic_params(void)
{
	return &(g_traffic_info.params);
}

traffic_model_decision_stru* get_decision_info(void)
{
	return &(g_traffic_info.decision_info);
}

static struct hrtimer* get_traffic_timer(void)
{
	return &(g_traffic_info.traffic_timer);
}

/* 功能：获取系统时间 */
static uint64_t get_ktimestamp(void)
{
	ktime_t ktime;
	uint64_t time_ms;

	ktime = ktime_get_real();
	time_ms = (uint64_t)ktime_to_ms(ktime);

	return time_ms;
}

/* 功能: 计算start与end之间的时间间隔(ms) */
static uint32_t get_run_time(uint64_t start_time, uint64_t end_time)
{
	uint64_t run_time;
	run_time = end_time - start_time;
	return run_time;
}

/* 功能: 注销定时器 */
static uint32_t delete_timer(struct hrtimer *timer, uint8_t timer_init_flag)
{
	if (timer == NULL || timer_init_flag == WIFI_FALSE)
		return RET_FAIL;
	hrtimer_cancel(timer);
	return RET_SUCC;
}

/*  功能：创建定时器 */
static uint32_t create_timer(struct hrtimer *timer, uint8_t timer_init_flag, uint32_t second, uint32_t nano_second,
	timeout_func handler)
{
	ktime_t kt;

	if (timer == NULL) return RET_FAIL;

	/* 如果定时器已存在，则先删除再创建 */
	if (timer_init_flag == WIFI_TRUE)
		delete_timer(timer, timer_init_flag);

	/* 设置超时时间 */
	kt = ktime_set(second, nano_second);
	hrtimer_init(timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer->function = handler; /* 超时处理函数 */
	hrtimer_start(timer, kt, HRTIMER_MODE_REL);

	return RET_SUCC;
}

/* 功能: 更新参数 */
static uint32_t params_update_with_weight(uint32_t history_param, uint32_t new_param)
{
	uint32_t result = (new_param * TRAFFIC_MODEL_UPDATE_WEIGHT +
		history_param * (10 - TRAFFIC_MODEL_UPDATE_WEIGHT)) / 10; /* 小数系数定点化：先放大10倍再缩小10倍 */
	return result;
}

/* 功能: 流量上限转化为档位 */
static uint8_t transform_control_level_to_level(uint32_t control_level)
{
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();

	if (control_level == upper_bound->upper_bound[LOW_TPUT_LEVEL]) {
		return LOW_TPUT_LEVEL;
	} else if (control_level == upper_bound->upper_bound[MEDIUM_TPUT_LEVEL]) {
		return MEDIUM_TPUT_LEVEL;
	} else if (control_level == upper_bound->upper_bound[HIGH_TPUT_LEVEL]) {
		return HIGH_TPUT_LEVEL;
	}

	return HIGH_TPUT_LEVEL;
}

/* 功能: 处理HAL层启动/关闭命令 */
uint32_t traffic_model_alg_handler(uint8_t *recv_msg)
{
	uint32_t ret;
	struct hrtimer *traffic_timer = get_traffic_timer();
	traffic_model_info_stru *traffic_info = get_traffic_info();

	if (recv_msg == NULL) {
		pr_info("TrafficModel: traffic_model_alg_handler, recv_msg is null");
		return RET_FAIL;
	}

	traffic_info->alg_mode = recv_msg[1]; /* 字节1为命令内容 */
	pr_info("TrafficModel: recv_msg is %d", traffic_info->alg_mode);

	/* 如果关闭算法，则注销定时器 */
	if (traffic_info->alg_mode == WIFI_FALSE) {
		ret = delete_timer(traffic_timer, traffic_info->timer_init_flag);
		if (ret == RET_SUCC) {
			traffic_info->timer_init_flag = WIFI_FALSE;
			pr_info("TrafficModel: traffic timer has been deleted");
		}
		return ret;
	}

	/* 如果开启算法，则初始化参数，并启动定时器 */
	ret = traffic_model_reset();
	if (ret != RET_SUCC) {
		pr_info("TrafficModel: fail to reset alg");
		return RET_FAIL;
	}
	ret = create_timer(traffic_timer, traffic_info->timer_init_flag, 0, TRAFFIC_STAT_TIMEOUT_PERIOD,
		traffic_stat_timeout_handler);
	if (ret == RET_SUCC) {
		traffic_info->timer_init_flag = WIFI_TRUE;
		g_index_of_collect_data = -1;
		pr_info("TrafficModel: traffic timer has been created");
	}

	return ret;
}

/* 功能: 处理HAL层更新参数命令 */
void traffic_model_set_params_handler(uint8_t *recv_msg)
{
	traffic_model_info_stru *traffic_info = get_traffic_info();
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();
	traffic_model_upper_bound_stru history_bound;
	uint32_t alpha, beta;
	uint32_t ret;

	if (recv_msg == NULL) {
		pr_info("TrafficModel: traffic_model_set_params_handler, recv_msg is null");
		return;
	}

	if (memcpy_s(&history_bound, sizeof(traffic_model_upper_bound_stru), upper_bound,
		sizeof(traffic_model_upper_bound_stru)) != EOK) {
		pr_info("TrafficModel: traffic_model_set_params_handler, memcpy failed");
		return;
	}
	upper_bound->upper_bound[LOW_TPUT_LEVEL] = recv_msg[1] * TRAFFIC_BYTES_TRANSFER_FACTOR;
	upper_bound->upper_bound[MEDIUM_TPUT_LEVEL] = recv_msg[2] * TRAFFIC_BYTES_TRANSFER_FACTOR;

	pr_info("TrafficModel: upper_bound[0] from %d to %d, upper_bound[1] from %d to %d",
		history_bound.upper_bound[LOW_TPUT_LEVEL], upper_bound->upper_bound[LOW_TPUT_LEVEL],
		history_bound.upper_bound[MEDIUM_TPUT_LEVEL], upper_bound->upper_bound[MEDIUM_TPUT_LEVEL]);

	/* 参数变更无需重复训练，将当前参数直接等比例缩小 */
	alpha = upper_bound->upper_bound[LOW_TPUT_LEVEL] * 10 / history_bound.upper_bound[LOW_TPUT_LEVEL];
	beta = upper_bound->upper_bound[MEDIUM_TPUT_LEVEL] * 10 / history_bound.upper_bound[MEDIUM_TPUT_LEVEL];

	pr_info("TrafficModel: alpha %d, beta %d, now params %d, %d",
		alpha, beta, traffic_info->params.first_thred, traffic_info->params.second_thred);

	traffic_info->params.first_thred = traffic_info->params.first_thred * alpha / 10;
	traffic_info->params.second_thred = traffic_info->params.second_thred * beta / 10;

	pr_info("TrafficModel: update params to %d, %d",
		traffic_info->params.first_thred, traffic_info->params.second_thred);
}

static uint32_t flexible_get_net_dev(void)
{
	traffic_model_info_stru *traffic_info = get_traffic_info();
	struct net_device *dev = NULL;
	int flags;

	dev = dev_get_by_name(&init_net, "wlan0");
	if (dev == NULL) {
		pr_info("TrafficModel: get dev wlan0 failed");
		return RET_FAIL;
	}
	flags = dev->flags & IFF_UP;
	if (flags == 0) {
		pr_info("TrafficModel: wlan0 is down");
		dev_put(dev);
		return RET_FAIL;
	}

	traffic_info->dev = dev;
	return RET_SUCC;
}

/* 功能：获取当前net_dev里保存的流量数据 */
/* 验证场景：在漫游、切换等场景下的流量数据是否具有连续性 */
static uint32_t flexible_get_traffic_stats(uint64_t *tx_bytes, uint64_t *rx_bytes)
{
	traffic_model_info_stru *traffic_info = get_traffic_info();
	int flags;

	if (tx_bytes == NULL || rx_bytes == NULL)
		return RET_FAIL;

	if (traffic_info->dev == NULL)
		return RET_FAIL;

	traffic_info->dev->netdev_ops->ndo_get_stats(traffic_info->dev);
	*tx_bytes = traffic_info->dev->stats.tx_bytes;
	*rx_bytes = traffic_info->dev->stats.rx_bytes;

	return RET_SUCC;
}

/* 功能：MSS方案启动后进行信息初始化 */
/* 什么场景需要重新训练，梳理一下 */
static uint32_t traffic_model_reset(void)
{
	traffic_model_info_stru *traffic_info = get_traffic_info();
	traffic_training_stru *training_info = get_training_info();
	traffic_stats_stru *stats = get_traffic_stats();
	traffic_model_params_stru *params = get_traffic_params();
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();
	uint32_t ret;

	traffic_info->alg_mode = WIFI_TRUE;
	training_info->penalty_delay = TRAFFIC_MODEL_PENALTY_DELAY;
	training_info->penalty_switch = TRAFFIC_MODEL_PENALTY_SWITCH;
	training_info->penalty_waste = TRAFFIC_MODEL_PENALTY_WASTE;

	traffic_info->period_cnt = 0;
	traffic_info->last_traffic_update_timestamp = (uint32_t)get_ktimestamp();

	/* 更新traffic信息 */
	ret = flexible_get_net_dev();
	if (ret != RET_SUCC) {
		pr_info("TrafficModel: fail to get net dev, return");
		return ret;
	}
	flexible_get_traffic_stats(&stats->tx_bytes, &stats->rx_bytes);

	if (traffic_info->first_flag == WIFI_TRUE) {
		traffic_info->training_cnt = TRAFFIC_MODEL_FIRST_TRAINING_CNT;
		upper_bound->level_cnt = MODE_LEVEL_BUTT;
		upper_bound->upper_bound[LOW_TPUT_LEVEL] = TRAFFIC_MODEL_FIRST_LEVEL;
		upper_bound->upper_bound[MEDIUM_TPUT_LEVEL] = TRAFFIC_MODEL_SECOND_LEVEL;
		upper_bound->upper_bound[HIGH_TPUT_LEVEL] = TRAFFIC_MODEL_THIRD_LEVEL;
		params->first_thred = upper_bound->upper_bound[LOW_TPUT_LEVEL];
		params->second_thred = upper_bound->upper_bound[MEDIUM_TPUT_LEVEL];
		params->delta = TRAFFIC_MODEL_DELTA_LEVEL;
	} else {
		traffic_info->training_cnt = TRAFFIC_MODEL_TRAINING_CNT;
	}

	traffic_info->decision_info.below_first_thred_cnt = params->delta;
	traffic_info->decision_info.below_second_thred_cnt = params->delta;

	pr_info("TrafficModel: traffic_model_reset succ");
	return RET_SUCC;
}

/* 功能: 计算流量区间 */
static uint32_t traffic_model_control_level_calculation(uint64_t training_data,
	traffic_model_params_stru *temp_params, traffic_model_decision_stru *decision_info)
{
	uint32_t control_level;
	uint8_t below_first_thred_cnt, below_second_thred_cnt;
	uint32_t temp_thred1, temp_thred2, temp_delta;
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();

	if (temp_params == NULL || decision_info == NULL)
		return upper_bound->upper_bound[LOW_TPUT_LEVEL];

	below_first_thred_cnt = decision_info->below_first_thred_cnt; /* 记录连续低于门限1的次数 */
	below_second_thred_cnt = decision_info->below_second_thred_cnt; /* 记录连续低于门限2的次数 */
	temp_thred1 = temp_params->first_thred;
	temp_thred2 = temp_params->second_thred;
	temp_delta = temp_params->delta;

	if (training_data >= temp_thred2) {
		below_first_thred_cnt = 0;
		below_second_thred_cnt = 0;
	} else if (training_data < temp_thred2 && training_data >= temp_thred1) {
		below_first_thred_cnt = 0;
		below_second_thred_cnt = (below_second_thred_cnt >= temp_delta) ? temp_delta : below_second_thred_cnt + 1;
	} else {
		below_first_thred_cnt = (below_first_thred_cnt >= temp_delta) ? temp_delta : below_first_thred_cnt + 1;
		below_second_thred_cnt = (below_second_thred_cnt >= temp_delta) ? temp_delta : below_second_thred_cnt + 1;
	}

	/* 如果连续低于门限2的次数小于temp_delta，则判定输出为第三区间 */
	if (below_second_thred_cnt < temp_delta) {
		control_level = upper_bound->upper_bound[HIGH_TPUT_LEVEL];
	} else if (below_first_thred_cnt >= temp_delta) {
		/* 如果连续低于门限1的次数大于等于temp_delta，则判定输出为第一区间  */
		control_level = upper_bound->upper_bound[LOW_TPUT_LEVEL];
	} else {
		/* 其他情况都是第二区间 */
		control_level = upper_bound->upper_bound[MEDIUM_TPUT_LEVEL];
	}

	decision_info->below_first_thred_cnt = below_first_thred_cnt;
	decision_info->below_second_thred_cnt = below_second_thred_cnt;
	return control_level;
}


static uint16_t traffic_model_loss_calculation(uint64_t *training_data, traffic_model_params_stru *temp_params)
{
	uint8_t index_i;
	uint32_t control_level[TRAFFIC_MODEL_TRAINING_PEROID];
	int16_t loss, loss_delay, loss_waste, loss_switch;
	traffic_model_info_stru *traffic_info = get_traffic_info();
	traffic_training_stru *training_info = get_training_info();
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();
	traffic_model_decision_stru decision_info;

	decision_info.below_first_thred_cnt = temp_params->delta; /* 记录连续低于门限1的次数 */
	decision_info.below_second_thred_cnt = temp_params->delta; /* 记录连续低于门限2的次数 */

	/* 判断当前参数的流量区间 */
	for (index_i = 0; index_i < traffic_info->training_cnt; index_i++)
		control_level[index_i] = traffic_model_control_level_calculation(training_data[index_i], temp_params,
			&decision_info);

	/* 计算当前参数的loss值 */
	loss_delay = 0;
	loss_waste = 0;
	loss_switch = 0;
	/* 取0 - (trainning_cnt - 1)的control_level来和1 - training_cnt的流量来比较 */
	for (index_i = 0; index_i < (traffic_info->training_cnt - 1); index_i++) {
		/* 当前流量超过判定的区间 */
		if (control_level[index_i] < training_data[index_i + 1])
			loss_delay++;

		/* 当前流量在区间1，而判断为区间2 */
		if (control_level[index_i] == upper_bound->upper_bound[MEDIUM_TPUT_LEVEL] &&
			training_data[index_i + 1] < upper_bound->upper_bound[LOW_TPUT_LEVEL])
			loss_waste++;

		/* 当前流量在区间1或2，而判断为区间3 */
		if (control_level[index_i] == upper_bound->upper_bound[HIGH_TPUT_LEVEL] &&
			training_data[index_i + 1] < upper_bound->upper_bound[MEDIUM_TPUT_LEVEL])
			loss_waste++;

		/* 判决产生区间变化 */
		if (index_i >= 1 && control_level[index_i]  > control_level[index_i - 1])
			loss_switch++;
	}

	loss = loss_delay * training_info->penalty_delay + loss_waste * training_info->penalty_waste +
		loss_switch * training_info->penalty_switch;

	return loss;
}

static void flexible_traffic_model_training(traffic_model_info_stru *traffic_info, traffic_model_params_stru *params)
{
	uint8_t index_i, index_j, index_k;
	uint16_t min_loss = TRAFFIC_MODEL_MAX_LOSS;
	uint16_t temp_loss;
	traffic_model_params_stru target_params = {};
	traffic_model_params_stru temp_params = {};
	uint64_t *training_data = get_training_data();
	traffic_model_upper_bound_stru *upper_bound = get_upper_bound();

	if (traffic_info == NULL || params == NULL)
		return;

	/* 搜索合适的参数(网格搜索) */
	for (index_i = 1; index_i <= TRAFFIC_MODEL_SEARCH_STEP; index_i++) {
		for (index_j = 1; index_j <= TRAFFIC_MODEL_SEARCH_STEP; index_j++) {
			for (index_k = 1; index_k <= TRAFFIC_MODEL_SEARCH_STEP; index_k++) {
				temp_params.first_thred = index_i * upper_bound->upper_bound[0] / TRAFFIC_MODEL_SEARCH_STEP;
				temp_params.second_thred = index_j * upper_bound->upper_bound[1] / TRAFFIC_MODEL_SEARCH_STEP;
				temp_params.delta = index_k * TRAFFIC_MODEL_DELTA_LEVEL / TRAFFIC_MODEL_SEARCH_STEP;
				temp_loss = traffic_model_loss_calculation(training_data, &temp_params);
				if (temp_loss > min_loss) {
					continue;
				}
				/* 更新参数 */
				min_loss = temp_loss;
				target_params.first_thred = temp_params.first_thred;
				target_params.second_thred = temp_params.second_thred;
				target_params.delta = temp_params.delta;
			}
		}
	}

	/* 保存训练过的参数 */
	if (traffic_info->first_flag == WIFI_TRUE) {
		traffic_info->params.first_thred = target_params.first_thred;
		traffic_info->params.second_thred = target_params.second_thred;
		traffic_info->params.delta = target_params.delta;
		traffic_info->first_flag = 0;
	} else {
		traffic_info->params.first_thred =
			params_update_with_weight(traffic_info->params.first_thred, target_params.first_thred);
		traffic_info->params.second_thred =
			params_update_with_weight(traffic_info->params.second_thred, target_params.second_thred);
		traffic_info->params.delta =
			params_update_with_weight(traffic_info->params.delta, target_params.delta);
	}
	/* 初始化decision_info中的计数 */
	traffic_info->decision_info.below_first_thred_cnt = target_params.delta;
	traffic_info->decision_info.below_second_thred_cnt = target_params.delta;
	traffic_info->training_cnt = TRAFFIC_MODEL_TRAINING_CNT;

	pr_info("TrafficModel: params update, new params: first_thred %d, second_thred %d, delta %d end",
		traffic_info->params.first_thred, traffic_info->params.second_thred, traffic_info->params.delta);
}

/* 功能: 根据本周期的流量，以及流量判断参数，判断所处流量区间 */
static void flexible_traffic_model_decision(traffic_model_params_stru *params, uint64_t cur_traffic_bytes)
{
	uint32_t control_level;
	uint8_t msg[2];
	traffic_model_info_stru *traffic_info = get_traffic_info();
	traffic_model_decision_stru *decision_info = get_decision_info();

	control_level = traffic_model_control_level_calculation(cur_traffic_bytes, params, decision_info);
	if (control_level != decision_info->cur_control_level) {
		decision_info->cur_control_level = control_level;
		decision_info->output_level = transform_control_level_to_level(control_level);
		/* 上报到HAL层 */
		msg[0] = SET_MSS_TPUT_LEVEL;
		msg[1]= decision_info->output_level;
		cfg80211_drv_mss_report(msg, sizeof(msg));
	}
}

static uint64_t get_cur_traffic_bytes(traffic_model_info_stru *traffic_info, uint64_t cur_traffic_bytes)
{
	if (g_index_of_collect_data == -1) {
		memset_s(traffic_info->collect_data, sizeof(traffic_info->collect_data), 0, sizeof(traffic_info->collect_data));
		is_cyclic = false;
	}

	g_index_of_collect_data = (g_index_of_collect_data + 1) % TRAFFIC_DATA_COUNT;
	traffic_info->collect_data[g_index_of_collect_data] = cur_traffic_bytes;
	if (g_index_of_collect_data >= (TRAFFIC_DATA_COUNT - 1))
		is_cyclic = true;

	uint8_t i = 0;
	uint64_t average_traffic_bytes = 0;
	for (i = 0; i < TRAFFIC_DATA_COUNT; i++)
		average_traffic_bytes += traffic_info->collect_data[i];

	if (!is_cyclic) {
		average_traffic_bytes = average_traffic_bytes / (g_index_of_collect_data + 1);
	} else {
		average_traffic_bytes = average_traffic_bytes / TRAFFIC_DATA_COUNT;
	}

	return average_traffic_bytes;
}

/* 功能: 流量模型入口函数 */
static uint32_t flexible_traffic_process(traffic_model_info_stru *traffic_info)
{
	uint32_t timestamp;
	uint64_t tx_bytes, rx_bytes, cur_traffic_bytes;
	traffic_model_params_stru *params = get_traffic_params();
	traffic_stats_stru *stats = get_traffic_stats();
	uint16_t period_cnt = traffic_info->period_cnt;
	uint32_t ret;

	/* 获取本周期的tx_bytes & rx_bytes */
	ret = flexible_get_traffic_stats(&tx_bytes, &rx_bytes);
	if (ret != RET_SUCC)
		return ret;

	/* 获取当前时间 */
	timestamp = (uint32_t)get_ktimestamp();

	/* 判断tx/rx bytes是否出现反转，如果反转则丢弃 */
	if (tx_bytes < stats->tx_bytes || rx_bytes < stats->rx_bytes) {
		stats->tx_bytes = tx_bytes;
		stats->rx_bytes = rx_bytes;
		return RET_FAIL;
	}
	cur_traffic_bytes = (tx_bytes - stats->tx_bytes) + (rx_bytes - stats->rx_bytes);
	cur_traffic_bytes = get_cur_traffic_bytes(traffic_info, cur_traffic_bytes);

	stats->tx_bytes = tx_bytes;
	stats->rx_bytes = rx_bytes;

	/* 如果当前时间与上次记录的时间大于门限，则重置算法 */
	uint32_t interval = (uint32_t)get_run_time(traffic_info->last_traffic_update_timestamp, timestamp);
	if (interval > TRAFFIC_INFO_TIMEOUT_INTERVAL) {
		pr_info("sTrafficModel: last timestamp %lu, now timestamp %lu, interval %lu",
			traffic_info->last_traffic_update_timestamp, timestamp, interval);
		pr_info("TrafficModel: traffic timeout, reset alg params");
		traffic_model_reset();
	}

	/* 训练阶段保存流量 */
	if (traffic_info->period_cnt < traffic_info->training_cnt)
		traffic_info->training_data[traffic_info->period_cnt] = cur_traffic_bytes;

	traffic_info->period_cnt = (traffic_info->period_cnt + 1) % TRAFFIC_MODEL_TRAINING_PEROID;
	traffic_info->last_traffic_update_timestamp = timestamp;

	/* 收集到足够的训练数据，进行训练 */
	if (traffic_info->period_cnt == traffic_info->training_cnt)
		flexible_traffic_model_training(traffic_info, params);

	/* 流量区间判断: 超过门限就上升，持续低于门限一段缓冲时间才下降 */
	if (traffic_info->first_flag == 0)
		flexible_traffic_model_decision(params, cur_traffic_bytes);

	return RET_SUCC;
}

/* 功能: 流量统计定时器到期处理函数 */
enum hrtimer_restart traffic_stat_timeout_handler(struct hrtimer *timer)
{
	ktime_t kt;
	traffic_model_info_stru *traffic_info = get_traffic_info();

	/* 如果算法启动，则继续启动定时器；否则不再启动定时器 */
	if (traffic_info->alg_mode == WIFI_TRUE) {
		flexible_traffic_process(traffic_info);
		kt = ktime_set(0, TRAFFIC_STAT_TIMEOUT_PERIOD);
		hrtimer_forward_now(timer, kt);
		return HRTIMER_RESTART;
	} else {
		return HRTIMER_NORESTART;
	}
}

