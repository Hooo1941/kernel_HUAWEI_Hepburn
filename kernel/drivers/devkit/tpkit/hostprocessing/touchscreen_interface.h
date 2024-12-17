/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2049. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef TOUCHSCREEN_INTERFACE_H
#define TOUCHSCREEN_INTERFACE_H

/*
 * 1, fingerprint driver will include this header file
 * 2, fingerprint hal redefine tp_fp_event/tp_fp_event_source/tp_to_udfp_data
 * in fp_device.c, we need to keep them consistent when modifying
 */

enum tp_fp_event {
	TP_EVENT_FINGER_DOWN = 0,
	TP_EVENT_FINGER_UP = 1,
	TP_EVENT_HOVER_DOWN = 2,
	TP_EVENT_HOVER_UP = 3,
	TP_FP_EVENT_MAX,
};

enum tp_fp_event_source {
	UDFP_DATA_SOURCE_DEFAULT = 0,
	UDFP_DATA_SOURCE_THP = 1,
	UDFP_DATA_SOURCE_SYNAPTICS = 2,
	UDFP_DATA_SOURCE_GOODIX = 3,
	UDFP_DATA_SOURCE_SIMULATE = 255,
};

struct tp_to_udfp_data {
	uint32_t version;
	enum tp_fp_event tp_event;
	uint32_t mis_touch_count_area;
	uint32_t touch_to_fingerdown_time;
	uint32_t pressure;
	uint32_t mis_touch_count_pressure;
	uint32_t tp_coverage;
	uint16_t tp_fingersize;
	uint32_t tp_x;
	uint32_t tp_y;
	uint32_t tp_major;
	uint32_t tp_minor;
	uint8_t tp_ori;
	uint32_t tp_peak;
	enum tp_fp_event_source tp_source;
};

struct ud_fp_ops {
	int (*fp_irq_notify)(struct tp_to_udfp_data *tp_data, size_t len);
};

void fp_ops_register(struct ud_fp_ops *ops);
void fp_ops_unregister(struct ud_fp_ops *ops);
struct ud_fp_ops *fp_get_ops(void);

#endif
