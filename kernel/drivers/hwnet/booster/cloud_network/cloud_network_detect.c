/*
 * cloud_network_detect.c
 *
 * cloud network detect compass function kernel module implementation
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

#include "cloud_network_detect.h"
#include <linux/spinlock.h>

#include "cloud_network_honhdr.h"
#include "cloud_network_option.h"
#include "cloud_network_utils.h"
#include "cloud_network_param.h"
#include "cloud_network_rx.h"
#include "cloud_network_device.h"
#include "cloud_network_tx.h"

struct detect_ctrl {
	struct list_queue work_queue;
	struct work_struct detect_handle_work;
};

static struct detect_ops detect_modules[DETECT_MODULE_ID_MAX];
static struct detect_path_info detect_paths[DETECT_PATH_NUM_MAX + 1];
struct hrtimer send_packet_hrtimer;
static struct detect_ctrl detect_mgr;

static int path_bit_to_val(int path_bit)
{
	switch (path_bit) {
	case DETECT_PATH_BIT_MODEM_1:
		return DETECT_PATH_VAL_MODEM_1;
	case DETECT_PATH_BIT_MODEM_2:
		return DETECT_PATH_VAL_MODEM_2;
	case DETECT_PATH_BIT_WIFI_1:
		return DETECT_PATH_VAL_WIFI_1;
	case DETECT_PATH_BIT_WIFI_2:
		return DETECT_PATH_VAL_WIFI_2;
	default:
		log_err("path_bit_to_val error");
		return DETECT_PATH_VAL_ERROR;
	}
}

static int path_val_to_bit(int path_val)
{
	switch (path_val) {
	case DETECT_PATH_VAL_MODEM_1:
		return DETECT_PATH_BIT_MODEM_1;
	case DETECT_PATH_VAL_MODEM_2:
		return DETECT_PATH_BIT_MODEM_2;
	case DETECT_PATH_VAL_WIFI_1:
		return DETECT_PATH_BIT_WIFI_1;
	case DETECT_PATH_VAL_WIFI_2:
		return DETECT_PATH_BIT_WIFI_2;
	default:
		log_err("path_val_to_bit error");
		return DETECT_PATH_BIT_ERROR;
	}
}

void change_detect_path(int module_id, int paths)
{
	if (module_id <= DETECT_MODULE_ID_ERROR || module_id >= DETECT_MODULE_ID_MAX) {
		log_err("module id is invalid");
		return;
	}
	detect_modules[module_id].paths = paths;
}

static int get_all_detect_paths()
{
	int paths = 0;
	for (int i = 0; i < DETECT_MODULE_ID_MAX; i++) {
		if (detect_modules[i].module_id != DETECT_MODULE_ID_ERROR)
			paths |= detect_modules[i].paths;
	}
	return paths;
}

static int get_min_detect_interval()
{
	int min_interval = INT_MAX;
	for (int i = 0; i < DETECT_MODULE_ID_MAX; i++) {
		if (detect_modules[i].module_id > -1 && detect_modules[i].time_interval < min_interval)
			min_interval = detect_modules[i].time_interval;
	}

	return min_interval;
}

void add_path_timestamp_send(int paths, u32 timestamp_send)
{
	for (int i = 0b1000; i != 0; i >>= 1) { /* search valid path */
		if (i & paths) {
			int path = path_bit_to_val(i);
			if (path < 0 || path >= DETECT_PATH_NUM_MAX)
				return;

			if (detect_paths[path].send_iterator < 0 ||
				detect_paths[path].send_iterator >= PATH_LIST_NUM) {
				log_info("iter is invalid, error");
				break;
			}

			spin_lock_bh(&(detect_paths[path].lock));
			if (detect_paths[path].rtt_list != NULL &&
				detect_paths[path].timestamp_list != NULL) {
				detect_paths[path].rtt_list[detect_paths[path].send_iterator] =
					-1;
				detect_paths[path].timestamp_list[detect_paths[path].send_iterator] =
					timestamp_send;
				detect_paths[path].send_iterator++;
				detect_paths[path].send_iterator %= PATH_LIST_NUM;
			}
			spin_unlock_bh(&(detect_paths[path].lock));
		}
	}
}

static void update_path_data(int path, int timestamp_send, int timestamp_recv)
{
	int rtt = timestamp_recv - timestamp_send;
	int iter;

	if (path < 0 || path >= DETECT_PATH_NUM_MAX)
		return;

	spin_lock_bh(&(detect_paths[path].lock));
	iter = detect_paths[path].send_iterator;
	do {
		if (detect_paths[path].timestamp_list == NULL) {
			log_info("timestamp list is NULL Pointer, error");
			continue;
		}
		if (detect_paths[path].rtt_list == NULL) {
			log_info("rtt list is NULL Pointer, error");
			continue;
		}
		if (iter < 0 || iter >= PATH_LIST_NUM) {
			log_info("iter is invalid, error");
			break;
		}
		if (detect_paths[path].timestamp_list[iter] == timestamp_send) {
			detect_paths[path].rtt_list[iter] = rtt;
			break;
		}
		if (detect_paths[path].timestamp_list[iter] < timestamp_send)
			break;
		iter = (iter - 1 + PATH_LIST_NUM) % PATH_LIST_NUM;
	} while (iter != detect_paths[path].send_iterator);
	spin_unlock_bh(&(detect_paths[path].lock));
	return;
}

static int calculate_path_average_rtt(int path)
{
	int count = 0;
	int sum_rtt = 0;

	if (path < 0 || path >= DETECT_PATH_NUM_MAX)
		return sum_rtt;

	for (int i = 0; i < PATH_LIST_NUM; i++) {
		spin_lock_bh(&(detect_paths[path].lock));
		if (detect_paths[path].rtt_list[i] > 0) {
			count++;
			sum_rtt += detect_paths[path].rtt_list[i];
		}
		spin_unlock_bh(&(detect_paths[path].lock));
	}
	return sum_rtt / count;
}

static int calculate_path_loss_rate(int path)
{
	int count = 0;

	if (path < 0 || path >= DETECT_PATH_NUM_MAX)
		return count;

	spin_lock_bh(&(detect_paths[path].lock));
	int iter = detect_paths[path].send_iterator;
	do {
		if (detect_paths[path].rtt_list[iter] != -1)
			break;
		iter = (iter - 1 + PATH_LIST_NUM) % PATH_LIST_NUM;
	} while (iter != detect_paths[path].send_iterator);

	do {
		if (detect_paths[path].rtt_list[iter] == -1)
			count++;
		if (detect_paths[path].rtt_list[iter] == 0)
			break;
		iter = (iter - 1 + PATH_LIST_NUM) % PATH_LIST_NUM;
	} while (iter != detect_paths[path].send_iterator);
	spin_unlock_bh(&(detect_paths[path].lock));
	return count;
}

static void detect_paths_init()
{
	for (int i = 0; i < DETECT_PATH_NUM_MAX; i++) {
		spin_lock_init(&detect_paths[i].lock);
		spin_lock_bh(&(detect_paths[i].lock));
		detect_paths[i].rtt_list = (int *)kmalloc(sizeof(int) * PATH_LIST_NUM, GFP_ATOMIC);
		if (detect_paths[i].rtt_list != NULL)
			memset_s(detect_paths[i].rtt_list, sizeof(int) * PATH_LIST_NUM,
				0, sizeof(int) * PATH_LIST_NUM);

		detect_paths[i].timestamp_list = (int *)kmalloc(sizeof(int) * PATH_LIST_NUM, GFP_ATOMIC);
		if (detect_paths[i].timestamp_list != NULL)
			memset_s(detect_paths[i].timestamp_list, sizeof(int) * PATH_LIST_NUM,
				0, sizeof(int) * PATH_LIST_NUM);
		detect_paths[i].send_iterator = 0;
		spin_unlock_bh(&(detect_paths[i].lock));
	}
	return;
}

static void detect_paths_deinit()
{
	for (int i = 0; i < DETECT_PATH_NUM_MAX; i++) {
		spin_lock_bh(&(detect_paths[i].lock));
		if (!detect_paths[i].rtt_list) {
			kfree(detect_paths[i].rtt_list);
			detect_paths[i].rtt_list = NULL;
		}

		if (!detect_paths[i].timestamp_list) {
			kfree(detect_paths[i].timestamp_list);
			detect_paths[i].timestamp_list = NULL;
		}
		spin_unlock_bh(&(detect_paths[i].lock));
	}
}

static void detect_paths_reset()
{
	for (int i = 0; i < DETECT_PATH_NUM_MAX; i++) {
		spin_lock_bh(&(detect_paths[i].lock));
		if (detect_paths[i].rtt_list != NULL)
			memset_s(detect_paths[i].rtt_list, sizeof(int) * PATH_LIST_NUM,
				0, sizeof(int) * PATH_LIST_NUM);

		if (detect_paths[i].timestamp_list != NULL)
			memset_s(detect_paths[i].timestamp_list, sizeof(int) * PATH_LIST_NUM,
				0, sizeof(int) * PATH_LIST_NUM);
		spin_unlock_bh(&(detect_paths[i].lock));
	}
	return;
}

static struct sk_buff *alloc_detect_skb(int timestamp)
{
	struct sk_buff *skb = NULL;

	skb = alloc_option_skb(sizeof(u8) + sizeof(u8) + sizeof(u32));
	if (unlikely(skb == NULL)) {
		log_err("NULL Pointer, error");
		return NULL;
	}

	option_timestamp_append(skb, timestamp);
	struct iphdr *iph = ip_hdr(skb);
	struct udphdr *udph = udp_hdr(skb);

	iph->tot_len = htons(IP_SIZE + UDP_SIZE + HON_SIZE + get_options_len(skb) + HMAC_SIZE);
	udph->len = htons(UDP_SIZE + HON_SIZE + get_options_len(skb) + HMAC_SIZE);
	skb_put(skb, HMAC_SIZE);
	return skb;
}

int detect_packet_recv(int path, struct hon_option_detect *detect)
{
	struct detect_work_rcv_packet detect_recv;

	if (!is_cloud_network_enable())
		return HON_OPT_HANDLE_FAILED;

	detect_recv.type = DETECT_WORK_RCV_PACKET;
	detect_recv.msg_len = sizeof(struct detect_work_rcv_packet);
	detect_recv.path = path;
	detect_recv.timestamp_send = ntohl(detect->timestamp);
	detect_recv.timestamp_recv = get_rel_time_us();
	struct list_node *node = list_node_alloc((void *)(&detect_recv), sizeof(struct detect_work_rcv_packet));
	if (node == NULL) {
		log_err("NULL Pointer, error");
		return HON_OPT_HANDLE_FAILED;
	}
	enqueue_list_queue(&(detect_mgr.work_queue), node);
	schedule_work(&(detect_mgr.detect_handle_work));
	return HON_OPT_HANDLE_SUCCESS;
}

static void send_detect_packet(void)
{
	struct detect_work_send_packet detect_send;

	if (!is_cloud_network_enable())
		return;

	detect_send.type = DETECT_WORK_SEND_PACKET;
	detect_send.msg_len = sizeof(struct detect_work_send_packet);
	detect_send.paths = get_all_detect_paths();
	struct list_node *node = list_node_alloc(&detect_send, sizeof(struct detect_work_send_packet));
	log_info("send detect path %d", detect_send.paths);

	enqueue_list_queue(&(detect_mgr.work_queue), node);
	schedule_work(&(detect_mgr.detect_handle_work));
	return;
}

static enum hrtimer_restart add_send_packet_task(struct hrtimer *timer)
{
	if (!is_cloud_network_enable())
		return HRTIMER_NORESTART;

	if (unlikely(timer == NULL)) {
		log_err("NULL Pointer, error");
		return HRTIMER_NORESTART;
	}
	send_detect_packet();
	hrtimer_forward_now(timer, ms_to_ktime(get_min_detect_interval()));
	return HRTIMER_RESTART;
}

static void send_packet_hrtimer_start()
{
	if (!hrtimer_active(&send_packet_hrtimer))
		hrtimer_start(&send_packet_hrtimer, ms_to_ktime(get_min_detect_interval()), HRTIMER_MODE_REL);
	return;
}

void detect_work_send_packet_handle(struct detect_work_send_packet *detect_send)
{
	struct sk_buff *skb = NULL;
	u32 timestamp_send;
	if (unlikely(detect_send == NULL)) {
		log_err("NULL Pointer, error");
		return;
	}
	timestamp_send = get_rel_time_us();
	skb = alloc_detect_skb(timestamp_send);
	if (unlikely(skb == NULL))
		return;

	add_path_timestamp_send(detect_send->paths, timestamp_send);
	skb_send(skb, detect_send->paths);
	return;
}

void detect_work_rcv_packet_handle(struct detect_work_rcv_packet *detect_recv)
{
	int average_rtt;
	int loss_rate;
	int path_bit;

	if (unlikely(detect_recv == NULL)) {
		log_err("NULL Pointer, error");
		return;
	}

	update_path_data(detect_recv->path, detect_recv->timestamp_send,
		detect_recv->timestamp_recv);
	average_rtt = calculate_path_average_rtt(detect_recv->path);
	loss_rate = calculate_path_loss_rate(detect_recv->path);
	path_bit = path_val_to_bit(detect_recv->path);

	for (int i = 0; i < DETECT_MODULE_ID_MAX; i++) {
		if (detect_modules[i].module_id == -1)
			continue;

		if (path_bit & detect_modules[i].paths && detect_modules[i].detect_fn)
			detect_modules[i].detect_fn(detect_recv->path,
				detect_recv->timestamp_recv - detect_recv->timestamp_send,
				average_rtt, loss_rate);
	}
	return;
}

static void detect_module_start_handle(struct detect_work_start_info *detect_info)
{
	if (unlikely(detect_info == NULL))
		return;

	int module_id = detect_info->module_id;
	if (module_id <= DETECT_MODULE_ID_ERROR || module_id >= DETECT_MODULE_ID_MAX) {
		log_err("module id is invalid");
		return;
	}

	spin_lock_bh(&(detect_modules[module_id].lock));
	if (detect_modules[module_id].module_id == module_id) {
		log_info("this module id has started");
		spin_unlock_bh(&(detect_modules[module_id].lock));
		return;
	}
	detect_modules[module_id].module_id = detect_info->module_id;
	detect_modules[module_id].time_interval = detect_info->time_interval;
	detect_modules[module_id].paths = detect_info->paths;
	detect_modules[module_id].detect_fn = detect_info->detect_fn;
	send_packet_hrtimer_start();
	spin_unlock_bh(&(detect_modules[module_id].lock));
	send_detect_packet();
	return;
}

static void detect_module_stop_handle(struct detect_work_stop_info *detect_info)
{
	if (unlikely(detect_info == NULL))
		return;

	int module_id = detect_info->module_id;
	if (module_id <= DETECT_MODULE_ID_ERROR || module_id >= DETECT_MODULE_ID_MAX) {
		log_err("module id is invalid");
		return;
	}

	spin_lock_bh(&(detect_modules[module_id].lock));
	detect_modules[module_id].module_id = DETECT_MODULE_ID_ERROR;
	detect_modules[module_id].time_interval = 0;
	detect_modules[module_id].paths = 0;
	detect_modules[module_id].detect_fn = NULL;
	if (get_all_detect_paths() == 0)
		hrtimer_cancel(&send_packet_hrtimer);
	spin_unlock_bh(&(detect_modules[module_id].lock));
	return;
}

static void detect_work_handle(struct work_struct *work)
{
	struct list_node *node = NULL;

	if (unlikely(work == NULL)) {
		log_err("work is NULL, error");
		return;
	}

	if (!is_cloud_network_enable())
		return;

	while (list_queue_empty(&(detect_mgr.work_queue)) == 0) {
		node = dequeue_list_queue(&(detect_mgr.work_queue));
		if (node == NULL) {
			log_err("dequeue a null node, error");
			break;
		}
		struct detect_tlv *tlv = (struct detect_tlv *)(node->data);
		switch (tlv->type) {
		case DETECT_WORK_SEND_PACKET:
			detect_work_send_packet_handle((struct detect_work_send_packet *)(node->data));
			break;
		case DETECT_WORK_RCV_PACKET:
			detect_work_rcv_packet_handle((struct detect_work_rcv_packet *)(node->data));
			break;
		case DETECT_WORK_START:
			detect_module_start_handle((struct detect_work_start_info *)(node->data));
			break;
		case DETECT_WORK_STOP:
			detect_module_stop_handle((struct detect_work_stop_info *)(node->data));
			break;
		default:
			log_err("type %u unknown, error", tlv->type);
		}
		list_node_free(node);
		node = NULL;
	}
	return;
}

/* module_id should be defined in cloud_network_detect.h */
void detect_module_start(int module_id, int time_interval, int paths, detect_callback *detect_fn)
{
	struct detect_work_start_info detect_info;

	log_info("module id %d start detect %d", module_id, time_interval);

	if (module_id <= DETECT_MODULE_ID_ERROR || module_id >= DETECT_MODULE_ID_MAX) {
		log_err("module id is invalid");
		return;
	}
	if (paths == 0 || paths > 0b1111) {
		log_err("paths is illegal");
		return;
	}

	detect_info.type = DETECT_WORK_START;
	detect_info.msg_len = sizeof(struct detect_work_start_info);
	detect_info.module_id = module_id;
	detect_info.time_interval = time_interval;
	detect_info.paths = paths;
	detect_info.detect_fn = detect_fn;
	struct list_node *node = list_node_alloc(&detect_info, sizeof(struct detect_work_start_info));

	enqueue_list_queue(&detect_mgr.work_queue, node);
	schedule_work(&(detect_mgr.detect_handle_work));
	return;
}

void detect_module_stop(int module_id)
{
	struct detect_work_stop_info detect_info;
	log_info("module id %d stop detect", module_id);

	if (module_id <= DETECT_MODULE_ID_ERROR || module_id >= DETECT_MODULE_ID_MAX) {
		log_err("module id is invalid");
		return;
	}

	detect_info.type = DETECT_WORK_STOP;
	detect_info.msg_len = sizeof(struct detect_work_stop_info);
	detect_info.module_id = module_id;
	struct list_node *node = list_node_alloc((void *)(&detect_info), sizeof(struct detect_work_stop_info));
	enqueue_list_queue(&detect_mgr.work_queue, node);
	schedule_work(&(detect_mgr.detect_handle_work));
	return;
}

void cloud_network_detect_reset(void)
{
	log_info("cloud network detect reset");

	cancel_work_sync(&(detect_mgr.detect_handle_work));
	clean_list_queue(&(detect_mgr.work_queue));
	hrtimer_cancel(&send_packet_hrtimer);
	detect_paths_reset();
	return;
}

void cloud_network_detect_exit(void)
{
	log_info("cloud network detect exit");

	cancel_work_sync(&(detect_mgr.detect_handle_work));
	clean_list_queue(&(detect_mgr.work_queue));
	hrtimer_cancel(&send_packet_hrtimer);
	detect_paths_deinit();
	return;
}

int cloud_network_detect_init(void)
{
	log_info("cloud network detect init");

	init_list_queue(&(detect_mgr.work_queue));
	INIT_WORK(&(detect_mgr.detect_handle_work), detect_work_handle);
	hrtimer_init(&send_packet_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	send_packet_hrtimer.function = add_send_packet_task;

	for (int i = 0; i < DETECT_MODULE_ID_MAX; i++) {
		detect_modules[i].module_id = DETECT_MODULE_ID_ERROR;
		detect_modules[i].paths = 0;
		detect_modules[i].time_interval = INT_MAX;
		detect_modules[i].detect_fn = NULL;
		spin_lock_init(&detect_modules[i].lock);
	}

	detect_paths_init();
	return SUCCESS;
}
