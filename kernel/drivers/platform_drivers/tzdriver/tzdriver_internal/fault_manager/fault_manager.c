/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: config for reporting fault to fault manager.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/errno.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include "fault_manager.h"
#include "cmdmonitor.h"
#include "tc_ns_log.h"

struct tee_fault_ctrl g_tee_fault_ctrl;
static dms_register_dev_node_func g_dms_register_dev_node_func = NULL;
static dms_unregister_dev_node_func g_dms_unregister_dev_node_func = NULL;
static dms_sensor_register_func g_dms_sensor_register_func = NULL;
static dms_sensor_node_unregister_func g_dms_sensor_node_unregister_func = NULL;
static bool g_fault_registerd = false;

static unsigned int g_recoverable_fault_table[] = {
	ERR_TYPE_TA_CRASH
};

/*
 * if fault is recoverable, we just report once and delete form queue,
 * or we should keep reporting.
 */
static bool in_recoverable_table(unsigned int fault_type)
{
	int i;
	int size = sizeof(g_recoverable_fault_table) / sizeof(unsigned int);

	for (i = 0; i < size; i++) {
		if (fault_type == g_recoverable_fault_table[i])
			return true;
	}
	return false;
}

static int tee_register_dms_func(void)
{
	g_dms_register_dev_node_func =
		(dms_register_dev_node_func)(uintptr_t)kallsyms_lookup_name("dms_register_dev_node");
	if (g_dms_register_dev_node_func == NULL) {
		tloge("can not find register dev node\n");
		return -1;
	}

	g_dms_unregister_dev_node_func =
		(dms_unregister_dev_node_func)(uintptr_t)kallsyms_lookup_name("dms_unregister_dev_node");
	if (g_dms_unregister_dev_node_func == NULL) {
		tloge("can not find unregister dev node\n");
		return -1;
	}

	g_dms_sensor_register_func = (dms_sensor_register_func)(uintptr_t)kallsyms_lookup_name("dms_sensor_register");
	if (g_dms_sensor_register_func == NULL) {
		tloge("can not find register sensor\n");
		return -1;
	}

	g_dms_sensor_node_unregister_func =
			(dms_sensor_node_unregister_func)(uintptr_t)kallsyms_lookup_name("dms_sensor_node_unregister");
	if (g_dms_sensor_node_unregister_func == NULL) {
		tloge("can not find unregister sensor\n");
		return -1;
	}

	return 0;
}

static struct tee_fault_ctrl *get_tee_fault_ctrl(void)
{
	return &g_tee_fault_ctrl;
}

int32_t tee_dev_node_ops_init(struct dms_node *device)
{
	(void)device;
	tlogi("tee dev node ops init\n");
	return 0;
}

void tee_dev_node_ops_uninit(struct dms_node *device)
{
	(void)device;
	tlogi("tee dev node ops uninit\n");
	return 0;
}

static struct dms_node_operations g_tee_ops = {
	.init = tee_dev_node_ops_init,
	.uninit = tee_dev_node_ops_uninit,
	.get_info_list = NULL,
	.get_state = NULL,
	.get_capacity = NULL,
	.set_power_state = NULL,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

static struct dms_node g_tee_dev_node_table[TEE_DEV_NODE_MAX][TEE_DEV_NODE_TYPE_MAX] = {
	{
		init_tee_dev_node(DMS_DEV_TYPE_TEE_OS, TEE_DEV_NODE, TEE_NODE_NAME, 0x1, 0x1, 0x0, &g_tee_ops),
	},
};

static struct dms_sensor_object_cfg g_tee_sensor_table[TEE_DEV_NODE_MAX][TEE_SENSOR_MAX] = {
	{
		init_tee_sensor_table(DMS_SEN_TYPE_CRYPTO_SENSOR, TEE_SENSOR_NAME, DMS_DISCRETE_SENSOR_CLASS,
								DMS_SENSOR_ATTRIB_THRES_NONE, 0, TEE_SENSOR_SCAN_INTERVAL,
								DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG,
								tee_fault_event_scan, /* scan function */
								combine_64(TEE_DEV_NODE, TEE_DEV_NODE_TYPE, TEE_SENSOR), /* private data */
								TEE_ASSERT_MASK, TEE_DASSERT_MASK),
	},
};

static void tee_fault_list_init(struct tee_fault_dev *dev)
{
	int i;
	for (i = 0; i < dev->sensor_num; i++) {
		mutex_init(&dev->event_queue[i].mutex);
		INIT_LIST_HEAD(&dev->event_queue[i].fault_list.list);
	}
}


static void tee_fault_ctrl_init(struct tee_fault_ctrl *ctrl)
{
	int i, j;

	ctrl->dev_num = TEE_DEV_NODE_MAX;
	for (i = 0; i < TEE_DEV_NODE_MAX; i++) {
		for (j = 0; j < TEE_DEV_NODE_TYPE_MAX; j++) {
			ctrl->dev[i][j].dev_id = i * TEE_DEV_NODE_TYPE_MAX + j;
			ctrl->dev[i][j].dev_node = &g_tee_dev_node_table[i][j];
			ctrl->dev[i][j].sensor_num = TEE_SENSOR_MAX;
			ctrl->dev[i][j].sensor_object = g_tee_sensor_table[i];
			tee_fault_list_init(&ctrl->dev[i][j]);
		}
	}
}

static void tee_fault_ctrl_exit(struct tee_fault_ctrl *ctrl)
{
	int i, j, k;

	for (i = 0; i < TEE_DEV_NODE_MAX; i++) {
		for (j = 0; j < TEE_DEV_NODE_TYPE_MAX; j++) {
			for (k = 0; k < ctrl->dev[i][j].sensor_num; k++)
				mutex_destroy(&ctrl->dev[i][j].event_queue[k].mutex);
		}
	}
}

static int tee_register_one_node_sensor(struct tee_fault_dev *dev)
{
	int i;
	int ret;

	ret = g_dms_register_dev_node_func(dev->dev_node);
	if (ret != 0) {
		tloge("register dev node failed, ret %d\n", ret);
		return ret;
	}

	for (i = 0; i < dev->sensor_num; i++) {
		ret = g_dms_sensor_register_func(dev->dev_node, &dev->sensor_object[i]);
		if (ret != 0) {
			tloge("register sensor failed, ret %d\n", ret);
			if (i > 0)
				g_dms_sensor_node_unregister_func(dev->dev_node);
			g_dms_unregister_dev_node_func(dev->dev_node);
			return ret;
		}
	}
	return 0;
}

static void tee_unregister_node_sensor(struct tee_fault_dev *dev)
{
	g_dms_sensor_node_unregister_func(dev->dev_node);
	g_dms_unregister_dev_node_func(dev->dev_node);
}

static int add_fault_to_event_queue(struct tee_fault_info *info)
{
	int ret;
	struct tee_fault_dev *dev = NULL;
	struct tee_fault_event *current_queue = NULL;
	struct tee_fault_list *new_fault = NULL;
	struct tee_fault_list *pos = NULL;
	struct tee_fault_list *n = NULL;
	struct tee_fault_ctrl *ctrl = get_tee_fault_ctrl();

	bool check = (info == NULL) || (ctrl == NULL);
	if (check) {
		tloge("invalid ptr\n");
		return -1;
	}

	check = (info->dev_id >= TEE_DEV_NODE_MAX) || (info->sensor_id >= TEE_SENSOR_MAX) ||
			(info->dev_type_id >= TEE_DEV_NODE_TYPE_MAX);
	if (check) {
		tloge("invalid param\n");
		return -1;
	}

	dev = &ctrl->dev[info->dev_id][info->dev_type_id];
	current_queue = &dev->event_queue[info->sensor_id];

	new_fault = kzalloc(sizeof(struct tee_fault_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (new_fault == NULL) {
		tloge("new fault alloc failed\n");
		return -1;
	}

	ret = memcpy_s(&new_fault->info, sizeof(struct tee_fault_info), info, sizeof(struct tee_fault_info));
	if (ret) {
		tloge("memcpy fault error\n");
		kfree(new_fault);
		return -1;
	}

	mutex_lock(&current_queue->mutex);
	list_for_each_entry_safe (pos, n, &current_queue->fault_list.list, list) {
		if (pos->info.sensor_type == new_fault->info.sensor_type &&
			pos->info.fault_type == new_fault->info.fault_type) {
				mutex_unlock(&current_queue->mutex);
				kfree(new_fault);
				return EXIST_ERR;
			}
	}
	list_add(&new_fault->list, &current_queue->fault_list.list);
	current_queue->fault_num++;

	mutex_unlock(&current_queue->mutex);
	return 0;
}

static int generate_fault_info(int32_t type)
{
	uint32_t data_len;
	int fault_type;
	char *fault_message = NULL;
	struct tee_fault_info info;
	int ret;

	switch (type) {
	case TYPE_CRASH_TEE:
		fault_type = ERR_TYPE_TEEOS_CRASH;
		fault_message = "TEEOS Crash";
		data_len = strlen(fault_message);
		break;

	case TYPE_CRASH_TA:
		fault_type = ERR_TYPE_TA_CRASH;
		fault_message = "TA Crash";
		data_len = strlen(fault_message);
		break;
	default:
		tloge("invalid crash type\n");
		return -1;
	}

	info.dev_id = TEE_DEV_NODE_TYPE;
	info.dev_type_id = TEE_DEV_NODE_TYPE;
	info.sensor_id = TEE_SENSOR;
	info.sensor_type = DMS_SEN_TYPE_OS_CRITICAL_STOP;
	info.fault_type = fault_type;
	info.data_len = data_len;

	ret = memcpy_s(info.fault_data, DMS_MAX_EVENT_DATA_LENGTH, fault_message, data_len + 1);
	if (ret) {
		tloge("memcpy fault error\n");
		return -1;
	}

	ret = add_fault_to_event_queue(&info);
	if (ret < 0)
		tloge("add fault to current queue failed\n");

	return ret;
}

int tee_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	int ret;
	struct tee_fault_list *pos = NULL;
	struct tee_fault_list *n = NULL;
	struct tee_fault_event *current_queue = NULL;
	struct tee_fault_info *info = NULL;

	unsigned int dev_id = (private_data >> SHIFT_32) & MASK_32;
	unsigned int dev_type_id = (private_data & MASK_32) >> SHIFT_16;
	unsigned int sensor_id = private_data & MASK_16;

	struct tee_fault_ctrl *ctrl = get_tee_fault_ctrl();

	bool check = (data == NULL) || (dev_id >= ctrl->dev_num) || (dev_type_id >= TEE_DEV_NODE_TYPE_MAX) ||
					(sensor_id >= TEE_SENSOR_MAX);
	if (check) {
		tloge("invalid params\n");
		return -1;
	}

	data->event_count = 0;
	current_queue = &ctrl->dev[dev_id][dev_type_id].event_queue[sensor_id];

	mutex_lock(&current_queue->mutex);
	list_for_each_entry_safe(pos, n, &current_queue->fault_list.list, list) {
		info = &pos->info;
		data->sensor_data[data->event_count].current_value = info->fault_type;
		data->sensor_data[data->event_count].data_size = info->data_len;
		ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
						info->fault_data, info->data_len);

		if (ret) {
			tlogw("memcpy to fault manager failed\n");
			continue;
		}

		if (in_recoverable_table(info->fault_type)) {
			list_del(&pos->list);
			kfree(pos);
			pos = NULL;
			current_queue->fault_num--;
		}

		data->event_count++;
		if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT)
			break;
	}
	mutex_unlock(&current_queue->mutex);
	return 0;
}

static void tee_release_registered_node_sensor(int i, int j, struct tee_fault_ctrl *ctrl)
{
	if (j < 0) {
		j = TEE_DEV_NODE_TYPE_MAX - 1;
		i--;
	}

	if (i < 0 || j < 0)
		return;

	while (i >= 0) {
		while (j >= 0) {
			tee_unregister_node_sensor(&ctrl->dev[i][j]);
			j--;
		}
		j = TEE_DEV_NODE_TYPE_MAX - 1;
		i--;
	}
}

static void tee_delete_one_queue(struct tee_fault_event *event)
{
	struct tee_fault_list *pos = NULL;
	struct tee_fault_list *n = NULL;

	if (event == NULL) {
		tlogw("invalid queue");
		return;
	}

	mutex_lock(&event->mutex);
	list_for_each_entry_safe(pos, n, &event->fault_list.list, list) {
		list_del(&pos->list);
		kfree(pos);
		pos = NULL;
		event->fault_num--;
	}
	mutex_unlock(&event->mutex);
}

static void tee_release_fault_queue(struct tee_fault_ctrl *ctrl)
{
	int i, j, k;

	for (i = 0; i < TEE_DEV_NODE_MAX; i++) {
		for (j = 0; j < TEE_DEV_NODE_TYPE_MAX; j++) {
			struct tee_fault_dev *dev = &ctrl->dev[i][j];
			for (k = 0; k < dev->sensor_num; k++)
				tee_delete_one_queue(&dev->event_queue[k]);
		}
	}
}

static int tee_register_node_sensor(struct tee_fault_ctrl *ctrl)
{
	int i, j;
	int ret;

	if (ctrl == NULL) {
		tloge("invalid ctrl\n");
		return -1;
	}

	for (i = 0; i < TEE_DEV_NODE_MAX; i++) {
		for (j = 0; j < TEE_DEV_NODE_TYPE_MAX; j++) {
			struct tee_fault_dev *dev = &ctrl->dev[i][j];
			if (dev == NULL) {
				tloge("invalid dev\n");
				continue;
			}
			ret = tee_register_one_node_sensor(dev);
			if (ret) {
				tloge("register node sensor failed\n");
				tee_release_registered_node_sensor(i, j - 1, ctrl);
				return ret;
			}
		}
	}
	return 0;
}

static int tee_fault_report_init(void)
{
	int ret;
	struct tee_fault_ctrl *ctrl = get_tee_fault_ctrl();

	tee_fault_ctrl_init(ctrl);
	ret = tee_register_dms_func();
	if (ret != 0)
		goto exit;
	ret = tee_register_node_sensor(ctrl);
	if (ret) {
		tloge("tee register node sensor failed\n");
		goto exit;
	}
	g_fault_registerd = true;
	return 0;
exit:
	tee_fault_ctrl_exit(ctrl);
	return -1;
}

static void tee_fault_report_exit(void)
{
	struct tee_fault_ctrl *ctrl = get_tee_fault_ctrl();

	tee_release_fault_queue(ctrl);
	tee_release_registered_node_sensor(TEE_DEV_NODE, TEE_DEV_NODE_TYPE, ctrl);
	tee_fault_ctrl_exit(ctrl);
}

void fault_monitor_start(int32_t type)
{
	int ret;
	if (!g_fault_registerd) {
		ret = tee_fault_report_init();
		if (ret) {
			tloge("fault monitor start failed\n");
			return;
		}
	}
	ret = generate_fault_info(type);
	if (ret)
		tloge("generate fault info failed\n");
	return;
}

void fault_monitor_end(void)
{
	if (!g_fault_registerd)
		return;
	tee_fault_report_exit();
	g_fault_registerd = false;
}
