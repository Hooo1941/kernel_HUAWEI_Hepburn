/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_CHANNEL_H__
#define __ALS_CHANNEL_H__

#include "als_detect.h"
#define DDIC_IDX 4
typedef enum {
	LCD_OTHER,
	LCD_BOE,
	LCD_SAMSUNG,
	LCD_LG,
	LCD_VISIONOX,
	LCD_TM,
	LCD_TIANMA,
	LCD_HUAXING,
	LCD_YUNPU,
	LCD_BOE2
} lcd_type_t;

typedef enum {
	UNKNOWN_DDIC,
	DDIC_NOVATEK,
	DDIC_JINGHONG,
	DDIC_RUIDING,
	DDIC_NOVATEK37701A,
	DDIC_YUNYINGGU,
	DDIC_MAGNACHIP9155,
	DDIC_XINSI,
	DDIC_YILI,
	DDIC_RUIDING692H0,
	DDIC_ICNA3512,
	DDIC_RUIDING692H5,
	DDIC_HISI,
	DDIC_RUIDING692H5_1,
	DDIC_JICHUANG
} ddic_type_t;

typedef struct {
	const char *lcd_model;
	lcd_type_t lcd;
} lcd_t;
typedef struct {
	const char *lcd_model;
	ddic_type_t ddic;
} ddic_t;
int8_t get_lcd(int32_t tag);
int8_t get_ddic(int32_t tag);

int send_laya_als_calibrate_data_to_mcu(int32_t tag);
int send_als_calibrate_data_to_mcu(int32_t tag);
void als_ud_minus_dss_noise_send(int32_t tag,
	struct als_platform_data *pf_data, struct als_device_info *dev_info);
int als_under_tp_nv_read(int32_t tag);
int send_als_under_tp_calibrate_data_to_mcu(int32_t tag);
int als_get_calidata_nv_addr(int32_t tag, uint32_t *addr);
int als_get_sensor_id_by_tag(int32_t tag, uint32_t *s_id);
int als_get_tag_by_sensor_id(uint32_t s_id, int32_t *tag);
int als_flag_result_cali(struct als_device_info *dev_info);
void reset_als_calibrate_data(void);
void reset_als1_calibrate_data(void);
void reset_als2_calibrate_data(void);

void select_als_para_pinhole(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_rpr531(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_tmd2745(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_normal(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);

void select_als_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn);
void als_on_read_tp_module(unsigned long action, void *data);

#endif
