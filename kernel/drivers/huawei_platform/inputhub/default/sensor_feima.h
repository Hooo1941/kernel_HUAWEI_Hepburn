/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: sensor feima header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __SENSOR_FEIMA_H__
#define __SENSOR_FEIMA_H__

#include "sensor_config.h"
#define MOTION_CTRL_PAR	3
#define MOTION_CTRL_NUM	10
struct sensor_cookie {
	int tag;
	const char *name;
	const struct attribute_group *attrs_group;
	struct device *dev;
};

typedef struct {
	uint16_t sub_cmd;
	uint16_t sar_info;
} rpc_ioctl_t;

typedef struct {
	uint32_t sub_cmd;
	uint32_t sdc_config;
} sdc_ioctl_t;

typedef struct {
	uint32_t sub_cmd;
	uint32_t config_val;
} orientation_set_mode_ioctl_t;

typedef struct {
	uint32_t sub_cmd;
	uint32_t carcrash_signal;
} carcrash_ioctl_t;

enum {
	CALL_START = 0xa2,
	CALL_STOP,
};

typedef struct {
	uint32_t sub_cmd;
	uint32_t selftest_code;
} selftest_ioctl_t;

void send_lcd_freq_to_sensorhub(uint32_t lcd_freq);
void save_light_to_sensorhub_panel_id(uint32_t mipi_level, uint32_t bl_level, uint32_t panel_id);
void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level);
int posture_sensor_enable(void);
void report_fold_status_when_poweroff_charging(int status);

#endif /* __SENSOR_FEIMA_H__ */
