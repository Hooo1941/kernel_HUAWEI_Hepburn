/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag detect header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __MAG_DETECT_H__
#define __MAG_DETECT_H__

#include "sensor_sysfs.h"

#define MAG_DEV_COUNT_MAX 2

#define PDC_SIZE 27
#define HALL_STATE_GPIO 195   // HALL_GPIO_195: general fold phone and GRL's B_C frame
#define HALL_1_STATE_GPIO 246  // HALL_GPIO_246: GRL A_B frame before V4-C

struct compass_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t axis_map_x;
	uint8_t axis_map_y;
	uint8_t axis_map_z;
	uint8_t negate_x;
	uint8_t negate_y;
	uint8_t negate_z;
	int16_t angle;
	uint8_t outbit;
	uint8_t calibrate_method;
	GPIO_NUM_TYPE gpio_drdy;
	GPIO_NUM_TYPE gpio_rst;
	uint8_t soft_filter;
	uint8_t charger_trigger;
	uint8_t pdc_data[PDC_SIZE];
	uint16_t poll_interval;
	uint8_t compass_extend_data[SENSOR_PLATFORM_EXTEND_DATA_SIZE];
#if defined(CHA_MAG_DETACH) || defined(BBK_MAG_DETACH)
	uint8_t pdc_one_data[PDC_SIZE];
	uint8_t pdc_two_data[PDC_SIZE];
	uint8_t fold_type;
	uint8_t hall_gpio;
	uint8_t hall_1_gpio;
#endif
};

struct mag_platform_iron_data {
	uint8_t pdc_data_far[PDC_SIZE];
};

struct mag_device_info {
	int32_t obj_tag;
	uint32_t detect_list_id;
	uint8_t mag_dev_index;
	uint8_t mag_first_start_flag;
	uint8_t mag_folder_function;
	int32_t mag_threshold_for_als_calibrate;
	int32_t akm_cal_algo;
	int32_t akm_need_charger_current;
	int32_t akm_current_x_fac;
	int32_t akm_current_y_fac;
	int32_t akm_current_z_fac;
};
#if defined(CHA_MAG_DETACH) || defined(BBK_MAG_DETACH)
struct mag_compensation_val {
	uint8_t charger_x_val;
	uint8_t charger_x_map;
	uint8_t charger_y_val;
	uint8_t charger_y_map;
	uint8_t charger_z_val;
	uint8_t charger_z_map;
};
struct mag_compensation_val* mag_get_compensation(void);
#endif
struct compass_platform_data *mag_get_platform_data(int32_t tag);
struct mag_device_info *mag_get_device_info(int32_t tag);
char *mag_get_sensors_id_string(void);
void mag_get_sensors_id_from_dts(struct device_node *dn);
void read_mag_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm);
int mag1_data_from_mcu(const struct pkt_header *head);
int mag_data_from_mcu(const struct pkt_header *head);
int mag_set_far_softiron(int32_t tag);
void mag_detect_init(struct sensor_detect_manager *sm, uint32_t len);

#endif
