/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: config for reporting fault to fault manager..
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
#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include <linux/types.h>
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"

#define ERR_TYPE_TEEOS_CRASH     0x0
#define ERR_TYPE_TA_CRASH        0x1

#define TEE_ASSERT_MASK          0x03
#define TEE_DASSERT_MASK         0x02

#define TEE_SENSOR_SCAN_INTERVAL 1000
#define EXIST_ERR                1

#define SHIFT_32                 32
#define SHIFT_16                 16
#define MASK_32                  0xffffffff
#define MASK_16                  0xffff

#define TEE_NODE_NAME            "TEE OS"
#define TEE_SENSOR_NAME          "TEE OS"

#define combine_64(h, m, l) (((uint64_t)(h) << SHIFT_32) | ((m) << SHIFT_16) | l)

#define init_tee_dev_node(_type, _id, _name, _cap, _perm, _devid, _ops) \
	{ \
		.node_type = (_type), \
		.node_id = (_id), \
		.node_name = (_name), \
		.capacity = (_cap), \
		.permission = (_perm), \
		.owner_devid = (_devid), \
		.ops = (_ops) \
	}

#define init_tee_sensor_table(_type, _name, _class, _attr, _debounce, _interval, _proc, _enable, \
	_func, _data, _assert, _deassert) \
	{ \
		.sensor_type = (_type), \
		.sensor_name = (_name), \
		.sensor_class = (_class), \
		.sensor_class_cfg = { \
			.discrete_sensor = { \
				.attribute = (_attr), \
				.debounce_time = (_debounce) \
			} \
		}, \
		.scan_interval = (_interval), \
		.proc_flag = (_proc), \
		.enable_flag =  (_enable), \
		.pf_scan_func = (_func), \
		.private_data = (_data), \
		.assert_event_mask = (_assert), \
		.deassert_event_mask = (_deassert) \
	}

typedef enum {
	TEE_DEV_NODE = 0,
	TEE_DEV_NODE_MAX
} tee_dev_node_idx;

typedef enum {
	TEE_DEV_NODE_TYPE = 0,
	TEE_DEV_NODE_TYPE_MAX
} tee_dev_node_type_idx;

typedef enum {
	TEE_SENSOR = 0,
	TEE_SENSOR_MAX
} tee_sensor_idx;

struct tee_fault_info {
	unsigned int sensor_type;
	unsigned int dev_id;
	unsigned int dev_type_id;
	unsigned int sensor_id;
	unsigned int fault_type;
	unsigned int data_len;
	unsigned char fault_data[DMS_MAX_EVENT_DATA_LENGTH];
};

struct tee_fault_list {
	struct tee_fault_info info;
	struct list_head list;
};

struct tee_fault_event {
	unsigned int fault_num;
	struct tee_fault_list fault_list;
	struct mutex mutex;
};

struct tee_fault_dev {
	unsigned int dev_id;
	unsigned int sensor_num;
	struct dms_sensor_object_cfg *sensor_object;
	struct tee_fault_event event_queue[TEE_SENSOR_MAX];
	struct dms_node *dev_node;
};

struct tee_fault_ctrl {
	unsigned int dev_num;
	struct tee_fault_dev dev[TEE_DEV_NODE_MAX][TEE_DEV_NODE_TYPE_MAX];
};

typedef int (*dms_register_dev_node_func)(struct dms_node *node);
typedef int (*dms_unregister_dev_node_func)(struct dms_node *node);
typedef unsigned int (*dms_sensor_register_func)(struct dms_node *owner_node,
					  struct dms_sensor_object_cfg *psensor_obj_cfg);
typedef unsigned int (*dms_sensor_node_unregister_func)(struct dms_node *owner_node);

int tee_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);

#ifdef CONFIG_TEE_FAULT_MANAGER
void fault_monitor_start(int32_t type);
void fault_monitor_end(void);
#endif

#endif
