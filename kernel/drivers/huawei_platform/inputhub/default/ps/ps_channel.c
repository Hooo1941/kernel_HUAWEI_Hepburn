/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */
#include "ps_channel.h"
#include "als_channel.h"
#include "als_tp_color.h"
#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"
#include "ps_sysfs.h"

#ifdef CONFIG_PROX_LCDTYPE
static int8_t g_ps_lcd = LCD_OTHER;
static int8_t g_ps_ddic = UNKNOWN_DDIC;
typedef struct {
	const char *lcd_model;
	lcd_type_t lcd;
	ddic_type_t ddic;
} ps_lcd_type_t;

static ps_lcd_type_t ps_lcd_ddic_info[] = {
	{"190_h01", LCD_BOE, DDIC_HISI},
	{"310_h01", LCD_VISIONOX, DDIC_HISI},
	{"190_c08", LCD_BOE, DDIC_RUIDING692H5},
	{"310_c08", LCD_VISIONOX, DDIC_RUIDING692H5},
	{"190_c07", LCD_BOE, DDIC_RUIDING692H0},
	{"310_c07", LCD_VISIONOX, DDIC_RUIDING692H0},
	{"190_207", LCD_BOE, DDIC_NOVATEK},
	{"310_207", LCD_VISIONOX, DDIC_NOVATEK},
	{"190_c06", LCD_BOE, DDIC_RUIDING},
	{"310_c06", LCD_VISIONOX, DDIC_RUIDING}
};

static void get_ps_lcd_ddic()
{
	uint8_t index;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;
	struct als_platform_data *pf_data = NULL;

	pf_data = als_get_platform_data(TAG_ALS);
	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("ps not find lcd_panel_type node\n");
		return;
	}
	if (pf_data->als_phone_type == ALTB) {
		if (of_property_read_string(np, "sec_lcd_panel_type", &lcd_model)) {
			hwlog_err("ps not find sec_lcd_panel_type in dts\n");
			return;
		}
	} else {
		if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
			hwlog_err("ps not find lcd_panel_type in dts\n");
			return;
		}
	}
	hwlog_info("lcd_panel_type: %s suc in dts!!\n", lcd_model);

	for (index = 0; index < ARRAY_SIZE(ps_lcd_ddic_info); index++) {
		if (!strncmp(lcd_model, ps_lcd_ddic_info[index].lcd_model, strlen(ps_lcd_ddic_info[index].lcd_model))) {
			g_ps_lcd = ps_lcd_ddic_info[index].lcd;
			g_ps_ddic = ps_lcd_ddic_info[index].ddic;
			break;
		}
	}
	hwlog_info("%s: g_ps_lcd = %d, g_ps_ddic = %d\n", __func__, g_ps_lcd, g_ps_ddic);
}
#endif

int send_ps_calibrate_data_to_mcu(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		PS_CALIDATA_NV_SIZE, "PSENSOR"))
		return -1;

	dev_info->ps_first_start_flag = 1;

	if (memcpy_s(ps_sensor_offset, sizeof(ps_sensor_offset),
		user_info.nv_data, sizeof(ps_sensor_offset)) != EOK)
		return -1;

#ifdef CONFIG_PROX_LCDTYPE
	get_ps_lcd_ddic();
	ps_sensor_offset[4] = g_ps_lcd;
	ps_sensor_offset[5] = g_ps_ddic;
#endif

	hwlog_info("nve_direct_access read ps_offset %d,%d,%d, %d,%d,%d\n",
		ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2],
		ps_sensor_offset[3], ps_sensor_offset[4], ps_sensor_offset[5]);
	if (memcpy_s(&ps_sensor_calibrate_data,
		sizeof(ps_sensor_calibrate_data),
		ps_sensor_offset,
		(sizeof(ps_sensor_calibrate_data) < PS_CALIDATA_NV_SIZE) ?
		sizeof(ps_sensor_calibrate_data) : PS_CALIDATA_NV_SIZE) != EOK)
		return -1;

	if (send_calibrate_data_to_mcu(TAG_PS, SUB_CMD_SET_OFFSET_REQ,
		ps_sensor_offset, PS_CALIDATA_NV_SIZE, false))
		return -1;

	return 0;
}

void reset_ps_calibrate_data(void)
{
	struct ps_device_info *dev_info = NULL;

	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return;
	if (dev_info->chip_type != PS_CHIP_NONE ||
		dev_info->ps_cali_supported == 1)
		send_calibrate_data_to_mcu(TAG_PS, SUB_CMD_SET_OFFSET_REQ,
			ps_sensor_calibrate_data, PS_CALIDATA_NV_SIZE, true);
}

int tp_event_to_ultrasonic(unsigned char event)
{
	struct write_info pkg_ap = { 0 };
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	int ret;

	hwlog_info("%s event =%d\n", __func__, event);

	ret = memset_s(&cpkt, sizeof(cpkt), 0, sizeof(cpkt));
	if (ret != EOK) {
		hwlog_err("send_tp_event_to_ultrasonic memset_s cpkt err\n");
		return -1;
	}
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.tag = TAG_PS;
	cpkt.subcmd = SUB_CMD_PS_TP_EVENT_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(unsigned char) + SUBCMD_LEN;
	ret = memcpy_s(cpkt.para, sizeof(cpkt.para), &event, sizeof(unsigned char));
	if (ret != EOK) {
		hwlog_err("send_tp_event_to_ultrasonic memset_s cpkt err\n");
		return -1;
	}

	ret = write_customize_cmd(&pkg_ap, NULL, false);
	if (ret != 0) {
		hwlog_err("send_tp_event_to_ultrasonic failed ret = %d\n", ret);
		return -1;
	}
	return 0;
}
