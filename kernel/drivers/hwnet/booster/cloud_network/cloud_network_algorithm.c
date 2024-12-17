/*
 * cloud_cellular_algorithm.c
 *
 * cloud network path select methods module implementation
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include "cloud_network_algorithm.h"
#include "cloud_network_utils.h"
#include "cloud_network_detect.h"


#define PROBE_TIME_INTERVAL 300
#define RESPOND_TIMEOUT_DUR 5000
#define ARR_LENGTH 20
#define DELAY_VAL_BAD_TH 80
#define DELAY_VAL_GOOD_TH 60
#define DELAY_RELATIVE_BAD_TH 20
#define DELAY_RELATIVE_GOOD_TH 10
#define DELAY_GRAD_BAD_TH 1
#define DELAY_GRAD_GOOD_TH 0
#define DELAY_GRAD_INC_TH 5
#define LOSSRATE_BAD_TH 6
#define LOSSRATE_GOOD_TH 4
#define CNT_BAD 3
#define CNT_GOOD (-3)
#define MAX_LOSS_RATE 100
#define DOUBLE_NUMBER 2
#define DOUBLE_DOUBLE_NUMBER 4
#define MIN_QUEUE_LENGTH_FOR_GRAD 2

typedef struct {
	int value;
	int grad_val;
	int last_grad_val; // for compare with grad_val
	int min_val; // the min value of queue
	int relative_val; // equal to value-min_val, if the value is lagre, this mean the path qos change too much, ths is bad.
	int queue[ARR_LENGTH]; // record the continued detect result
	int head; // head index for the queue
	int tail; // tail index for the queue
} path_info_stat;

typedef struct {
	int val_cnt_bad;
	int relative_cnt_bad;
	int grad_cnt_bad;
} path_info_cnt;

typedef enum {
	PERFORM_NO_VALUE = 0,
	PERFORM_BAD,
	PERFORM_FLUCTUATE,
	PERFORM_GOOD,
} perform_level;

typedef struct {
	bool is_usage;
	struct hrtimer timer;
	int lossrate;
	path_info_cnt lossrate_cnt;
	path_info_stat delay_stat;
	path_info_cnt delay_cnt;
} path_info;

typedef struct {
	path_info cellular1;
	path_info cellular2;
	algorithm_path exp_status; // expect path, cellular1 or 2 or both
	algorithm_path real_status; // the current path which in use.
	status_change_callback status_change; // notice the result to the caller
} algorithm_ctx;

static algorithm_ctx *g_algCtx __read_mostly;

static path_info* path_type_to_info(algorithm_path path_type)
{
	switch (path_type) {
	case DETECT_PATH_BIT_MODEM_1:
		return &(g_algCtx->cellular1);
	case DETECT_PATH_BIT_MODEM_2:
		return &(g_algCtx->cellular2);
	// other path，such as WiFi, can add case condition at here
	default:
		log_err("cloud cellular algorithm, algorithm utils path type error");
		return NULL;
	}
}

static bool is_queue_full(int head, int tail, int size)
{
	if (size == 0) {
		log_err("input error, size is zero!");
		return true;
	}
	return (tail + 1) % size == head;
}

static int get_arr_min(path_info_stat* stat)
{
	if (unlikely(stat == NULL)) {
		log_err("cloud cellular algorithm get array minimum value, stat null pointer error");
		return ERROR;
	}
	if (stat->head == stat->tail) {
		log_err("cloud cellular algorithm get array minimum value, queue is empty");
		return 0;
	}
	int i;
	int out = stat->queue[0];
	for (i = stat->head; i != stat->tail; i = (i + 1) % ARR_LENGTH) {
		if (out > stat->queue[i])
			out = stat->queue[i];
	}
	return out;
}

static int get_queue_size(int head, int tail, int size)
{
	if (unlikely(size == 0)) {
		log_err("size == 0");
		return ERROR;
	}
	return (tail - head + size) % size;
}

static void update_path_stat(path_info_stat* stat, int value)
{
	if (unlikely(stat == NULL)) {
		log_err("cloud cellular algorithm update path stat, stat null potiner error");
		return;
	}
	
	stat->value = value;
	stat->queue[stat->tail] = value;

	// caculate the min only if the queue is full, else just add the value into the queue
	if (is_queue_full(stat->head, stat->tail, ARR_LENGTH)) {
		stat->min_val = get_arr_min(stat);
		stat->head = (stat->head + 1) % ARR_LENGTH;
		stat->relative_val = value - stat->min_val;
	} else {
		stat->min_val = value;
		stat->relative_val = 0;
	}

	if (get_queue_size(stat->head, stat->tail, ARR_LENGTH) > MIN_QUEUE_LENGTH_FOR_GRAD) {
		int last_val = stat->queue[(stat->tail + ARR_LENGTH - 1) % ARR_LENGTH];
		int llast_val = stat->queue[(stat->tail + ARR_LENGTH - 2) % ARR_LENGTH];
		stat->grad_val = (value - last_val);
		stat->last_grad_val = (last_val - llast_val);
	} else {
		stat->grad_val = 0;
		stat->last_grad_val = 0;
	}

	stat->tail = (stat->tail + 1) % ARR_LENGTH;
}

static int init_path_info(path_info *path_ptr)
{
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm init path info, pointer error");
		return ERROR;
	}
		
	path_ptr->lossrate = 0;
	path_ptr->delay_stat.value = 0;
	path_ptr->delay_stat.head = 0;
	path_ptr->delay_stat.tail = 0;
	path_ptr->delay_cnt.grad_cnt_bad = 0;
	path_ptr->delay_cnt.relative_cnt_bad = 0;
	path_ptr->delay_cnt.val_cnt_bad = 0;
	path_ptr->lossrate_cnt.grad_cnt_bad = 0;
	path_ptr->lossrate_cnt.relative_cnt_bad = 0;
	path_ptr->lossrate_cnt.val_cnt_bad = 0;
	return SUCCESS;
}

/*
	val: detected value of this time；
*/
static void update_stat_count(int *counter, int val, int upper_bound, int lower_bound)
{
	if (unlikely(counter == NULL)) {
		log_err("cloud cellular algorithm update statistic count, counter null pointer error");
		return;
	}
	/*
		1. current is good, and get bad once, the tendency was become worse
		2. current is bad, and get good one, the tendency was become better
	*/
	if (val >= upper_bound) {
		if (*counter < 0) {
			*counter = 1;
		} else {
			*counter += 1;
		}
	} else if (val < lower_bound) {
		if (*counter > 0) {
			*counter = -1;
		} else {
			*counter -= 1;
		}
	} else {
		*counter = 0;
	}
}

static int sgn_func(int val)
{
	if (val > 0) {
		return 1;
	} else if (val < 0) {
		return -1;
	} else {
		return 0;
	}
}

static int update_path_count(algorithm_path path_type)
{
	path_info *path_ptr = path_type_to_info(path_type);
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm update path count, null pointer error");
		return ERROR;
	}

	update_stat_count(&(path_ptr->delay_cnt.val_cnt_bad),
		path_ptr->delay_stat.value, DELAY_VAL_BAD_TH, DELAY_VAL_GOOD_TH);
	update_stat_count(&(path_ptr->delay_cnt.relative_cnt_bad),
		path_ptr->delay_stat.relative_val, DELAY_RELATIVE_BAD_TH, DELAY_RELATIVE_GOOD_TH);
	update_stat_count(&(path_ptr->delay_cnt.grad_cnt_bad),
		sgn_func(path_ptr->delay_stat.grad_val), DELAY_GRAD_BAD_TH, DELAY_GRAD_GOOD_TH);
	update_stat_count(&(path_ptr->lossrate_cnt.val_cnt_bad),
		path_ptr->lossrate, LOSSRATE_BAD_TH, LOSSRATE_GOOD_TH);
	return SUCCESS;
}

/*
	caculate the link quality, the logic is:
	become Bad:
		1）rtt grad > last rtt grad > threshold，or;
		2）rtt grad become bad for three times continuily, or;
		3）one of rtt, loast rat, relative rtt become bad for three times
	become Good:
		1）rtt, relative rtt, lost rate were become better for three times
*/
static perform_level evaluate_link_quality(algorithm_path path_type)
{
	path_info *path_ptr = path_type_to_info(path_type);
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm evaluate link quality, null pointer error");
		return PERFORM_NO_VALUE;
	}
	perform_level ret;
	ret = PERFORM_FLUCTUATE;

	if (path_ptr->delay_stat.grad_val > DELAY_GRAD_INC_TH && path_ptr->delay_stat.last_grad_val > DELAY_GRAD_INC_TH
		&& path_ptr->delay_stat.grad_val > path_ptr->delay_stat.last_grad_val)
		ret = PERFORM_BAD;

	if (path_ptr->delay_cnt.val_cnt_bad >= CNT_BAD || path_ptr->delay_cnt.relative_cnt_bad >= CNT_BAD
		|| path_ptr->delay_cnt.grad_cnt_bad >= CNT_BAD || path_ptr->lossrate_cnt.val_cnt_bad >= CNT_BAD)
		ret = PERFORM_BAD;

	if (path_ptr->delay_cnt.val_cnt_bad <= CNT_GOOD && path_ptr->delay_cnt.relative_cnt_bad <= CNT_GOOD
		&& path_ptr->lossrate_cnt.val_cnt_bad <= CNT_GOOD)
		ret = PERFORM_GOOD;

	log_info("cloud cellular algorithm, evaluate_link_quality result:%d", ret);
	return ret;
}

/*
	if the detect module get the detect result, it will call this function to tell algorthm module, the info include
	rtt, loss rate. and the algorthm module will update the path status, and caculate link quality.
	and according to the quality, tell the caller expect status.
	restriction: the caller have to call this function serially.
*/
static void cellular_usage_path_respond(int path_type, u32 rtt, int average_rtt, int loss_rate)
{
	algorithm_path temp_path_type = (algorithm_path)path_type;
	if (rtt < 0 || loss_rate < 0 || loss_rate > MAX_LOSS_RATE) {
		log_err("cloud cellular algorithm cellular usage path respond, Invalid parameter. rtt= %d, loss_rate = %d", rtt, loss_rate);
		return;
	}
	log_info("cellular usage path respond, rtt is %d, loss_rate is %d", rtt, loss_rate);

	perform_level ret = PERFORM_NO_VALUE;
	path_info *path_ptr = path_type_to_info(temp_path_type);
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm cellular usage path respond, path null pointer error");
		return;
	}
	if (!path_ptr->is_usage)
		return;
	if (hrtimer_active(&path_ptr->timer))
		hrtimer_cancel(&(path_ptr->timer));

	hrtimer_start(&(path_ptr->timer), ms_to_ktime(RESPOND_TIMEOUT_DUR), HRTIMER_MODE_REL);
	path_ptr->lossrate = loss_rate;
	update_path_stat(&(path_ptr->delay_stat), rtt);
	update_path_count(path_type);
	ret =  evaluate_link_quality(path_type);
	switch (ret) {
	case PERFORM_BAD:
		if (!g_algCtx->cellular1.is_usage || !g_algCtx->cellular2.is_usage) {
			g_algCtx->exp_status = DETECT_PATH_BIT_MODEM_1 | DETECT_PATH_BIT_MODEM_2;
			g_algCtx->status_change(g_algCtx->exp_status);
		}
		break;
	case PERFORM_GOOD:
		if (g_algCtx->cellular1.is_usage && g_algCtx->cellular2.is_usage) {
			if (path_type == PATH_MODEM_1) {
				g_algCtx->exp_status = DETECT_PATH_BIT_MODEM_1;
			} else if (path_type == PATH_MODEM_2) {
				g_algCtx->exp_status = DETECT_PATH_BIT_MODEM_2;
			}
			g_algCtx->status_change(g_algCtx->exp_status);
		}
		break;
	case PERFORM_FLUCTUATE:
		break;
	default:
		log_err("cloud cellular algorithm, the evaluation result is incorrect");
		break;
	}
	return;
}

static enum hrtimer_restart usage_path_respond_timeout(struct hrtimer *timer)
{
	log_info("cloud cellular algorithm, usage path respond timeout");
	if (timer == NULL) {
		log_err("timer is null");
		return HRTIMER_NORESTART;
	}
	
	/*
		if dual-cellular was not start, and timeout occurs, it is means the current path
		was become bad, so it has to start the other path. currently we can't get the other path quality
		so we have to start other path. and make the exp_status to dual path. and notice to the caller start dual path
	*/
	if (!g_algCtx->cellular1.is_usage || !g_algCtx->cellular2.is_usage) {
		g_algCtx->exp_status = (DETECT_PATH_BIT_MODEM_1 | DETECT_PATH_BIT_MODEM_2);
		g_algCtx->status_change(g_algCtx->exp_status);
	}
	return HRTIMER_NORESTART;
}
static int start_timeout_timer(algorithm_path path_type)
{
	log_info("start timeout timer");
	path_info *path_ptr = path_type_to_info(path_type);
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm start_probe_path, path null pointer error");
		return ERROR;
	}
	if (hrtimer_active(&path_ptr->timer))
		hrtimer_cancel(&(path_ptr->timer));
	path_ptr->timer.function = usage_path_respond_timeout;
	hrtimer_start(&(path_ptr->timer), ms_to_ktime(RESPOND_TIMEOUT_DUR), HRTIMER_MODE_REL);
	return SUCCESS;
}

static int stop_timeout_timer(algorithm_path path_type)
{
	log_info("stop timeout timer");
	path_info *path_ptr = path_type_to_info(path_type);
	if (unlikely(path_ptr == NULL)) {
		log_err("cloud cellular algorithm start_probe_path, path null pointer error");
		return ERROR;
	}
	if (hrtimer_active(&path_ptr->timer))
		hrtimer_cancel(&(path_ptr->timer));
	init_path_info(path_ptr);
	return SUCCESS;
}

/*
	called by other function, and it is used to notice algorithm module which current path was changed
	when this function was called, the algorthm module has to update the timer of detect path and then
	run the logic:
	0==》1  : start detect;
	1==》0: stop detect;
	1==》other: updated the detect path;
*/
int cellular_usage_notification(algorithm_path paths_in_use)
{
	log_info("cloud cellular algorithm, cellular usage notification");
	if (unlikely(g_algCtx == NULL)) {
		log_info("g_algCtx does not init!");
		return ERROR;
	}
	bool cellular1_usage = (paths_in_use & PATH_MODEM_1) > 0 ? true : false;
	bool cellular2_usage = (paths_in_use & PATH_MODEM_2) > 0 ? true : false;

	/*
		if the cellular1 path was not change, it means the detect was already detect.
		current only focus on cellular 1 and cellular 2
	*/
	if (g_algCtx->cellular1.is_usage != cellular1_usage) {
		if (cellular1_usage) {
			start_timeout_timer(DETECT_PATH_VAL_MODEM_1);
		} else {
			stop_timeout_timer(DETECT_PATH_VAL_MODEM_1);
		}
	}
	
	if (g_algCtx->cellular2.is_usage != cellular2_usage) {
		if (cellular2_usage) {
			start_timeout_timer(DETECT_PATH_VAL_MODEM_2);
		} else {
			stop_timeout_timer(DETECT_PATH_VAL_MODEM_2);
		}
	}
	
	int old_status = g_algCtx->real_status;
	g_algCtx->real_status = paths_in_use;
	g_algCtx->cellular1.is_usage = cellular1_usage;
	g_algCtx->cellular2.is_usage = cellular2_usage;
	
	if (old_status == 0 && g_algCtx->real_status != 0) {
		detect_module_start(ALGORITHM_MODULE_ID, PROBE_TIME_INTERVAL,
			(int)g_algCtx->real_status, cellular_usage_path_respond);
	} else if (old_status != 0 && g_algCtx->real_status == 0) {
		detect_module_stop(ALGORITHM_MODULE_ID);
	} else if (old_status != 0 && g_algCtx->real_status != 0) {
		change_detect_path(ALGORITHM_MODULE_ID, (int)g_algCtx->real_status);
	}
	g_algCtx->exp_status = DETECT_PATH_BIT_ERROR;
	return SUCCESS;
}

/*
	call by other    module for start the algorithm, the step is:
	1. cellular_algorithm_start is called by other function(caller)
	2. paths_in_use is used to init the path, the caller tell us which path was used currentlly.
	3. status_change is used to init the call back function of caller. we use this call to notice the caller expect status
		mostly it is path1 + path2.
	4. when this function was called, init the timer of each path. and then start the timer which path was use. the
		algorithm module wait detect result sendback in given time(RESPOND_TIMEOUT_DUR).
		if the timer was out, it means algorithm module was not get the detected result on the path. and the
		usage_path_respond_timeout function will be called by timer. and the status_change function will be
		called in timeout function to notice the caller to start the dual path.
*/
void cellular_algorithm_start(algorithm_path paths_in_use, status_change_callback status_change)
{
	log_info("cloud cellular algorithm, cellular algorithm start");
	if (g_algCtx != NULL) {
		log_info("cloud cellular algorithm, context has kmalloced");
		return;
	}
	g_algCtx = (algorithm_ctx *)kmalloc(sizeof(algorithm_ctx), GFP_ATOMIC);
	if (unlikely(g_algCtx == NULL)) {
		log_err("cloud cellular algorithm, context kmalloc failed");
		return;
	}

	g_algCtx->real_status = paths_in_use;
	if (paths_in_use > 0) {
		g_algCtx->exp_status = DETECT_PATH_BIT_ERROR;
		g_algCtx->status_change = status_change;
	}

	// init the path info, the each bit value of paths_in_use reprsent one path
	g_algCtx->cellular1.is_usage = (paths_in_use & PATH_MODEM_1) > 0 ? true : false;
	init_path_info(&(g_algCtx->cellular1));
	hrtimer_init(&(g_algCtx->cellular1.timer), CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	g_algCtx->cellular2.is_usage = (paths_in_use & PATH_MODEM_2) > 0 ? true : false;
	init_path_info(&(g_algCtx->cellular2));
	hrtimer_init(&(g_algCtx->cellular2.timer), CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	if (g_algCtx->real_status != DETECT_PATH_BIT_ERROR) {
		if (g_algCtx->cellular1.is_usage)
			start_timeout_timer(DETECT_PATH_VAL_MODEM_1);

		if (g_algCtx->cellular2.is_usage)
			start_timeout_timer(DETECT_PATH_VAL_MODEM_2);
		/* stard detect, the detect module will detect the path by interval. if get the result,
		, use cellular_usage_path_respond to notice the result */
		detect_module_start(ALGORITHM_MODULE_ID, PROBE_TIME_INTERVAL,
			(int)paths_in_use, cellular_usage_path_respond);
	}
}

/*
	used for other function call, if it is called, the algorithm will be stop.
*/
void cellular_algorithm_stop(void)
{
	log_info("cloud cellular algorithm, stop module exit");
	if (unlikely(g_algCtx == NULL)) {
		log_err("cloud cellular algorithm, cellular algorithm stop has exited");
		return;
	}
	
	detect_module_stop(ALGORITHM_MODULE_ID);
	if (hrtimer_active(&(g_algCtx->cellular1.timer)))
		hrtimer_cancel(&(g_algCtx->cellular1.timer));
	
	if (hrtimer_active(&(g_algCtx->cellular2.timer)))
		hrtimer_cancel(&(g_algCtx->cellular2.timer));
	kfree(g_algCtx);
	g_algCtx = NULL;
}

algorithm_path cellular_expect_status(void)
{
	return g_algCtx->exp_status;
}
