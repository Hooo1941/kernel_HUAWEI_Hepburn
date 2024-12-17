/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2025. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <securec.h>
#include "dksm_dmd.h"
#include "dkmd_comp.h"

// Converting error_no to character strings in the OHOS.
typedef struct error_no_map
{
	int32_t error_no;
	char *name;
} error_no_map;


int32_t lcd_errorno_to_str(int32_t error_no, char *str, int32_t buff_len) {
	int32_t i;
	int32_t ret;
	// Define local maps to save memory and reduce code modification conflicts.
	static struct error_no_map lcd_error_no_map[] = {
		{922001000, "DSM_LCD_LDI_UNDERFLOW_NO"},
		{922001001, "DSM_LCD_TE_TIME_OUT_ERROR_NO"},
		{922001002, "DSM_LCD_STATUS_ERROR_NO"},
		{922001003, "DSM_LCD_POWER_STATUS_ERROR_NO"},
		{922001008, "DSM_LCD_OVP_ERROR_NO"},
		{922001018, "DSM_LCD_ESD_STATUS_ERROR_NO"},
		{922001033, "DSM_LCD_BTB_CHECK_ERROR_NO"},
		{922001034, "DSM_LCD_BACKLIGHT_OCP_ERROR_NO"},
		{922001035, "DSM_LCD_BACKLIGHT_TSD_ERROR_NO"},
		{922001036, "DSM_LCD_MIPI_TRANSMIT_ERROR_NO"},
		{922001038, "DSM_LCD_VACTIVE_TIMEOUT_ERROR_NO"},
		{922001040, "DSM_LCD_BACKLIGHT_I2C_ERROR_NO"},
		{922001041, "DSM_LCD_BIAS_I2C_ERROR_NO"},
		{922001042, "DSM_LCD_BL_FLICKER_ERROR_NO"},
		{922001501, "DSM_LCD_PANEL_CRACK_ERROR_NO"},
		{922001502, "DSM_LCD_CHECKSUM_ERROR_NO"},
		{922001503, "DSM_LCD_POWER_ABNOMAL_ERROR_NO"},
		{922001504, "DSM_LCD_DDIC_LV_DETECT_ERROR_NO"},
		{922001505, "DSM_AMOLED_POWER_IC_CHECK_ERR_NO"},
		{922001506, "DSM_LCD_UNDERRUN_ERROR_NO"},
		{922001508, "DSM_LCD_LVD_DETECT_ERROR_NO"},
		{922001509, "DSM_LCD_MBIST_ERROR_NO"},
		{922001510, "DSM_LCD_MIPI_CHECK_ERROR_NO"},
		{922001511, "DSM_LCD_SUB_PANEL_CRACK_ERROR_NO"},
		{922001512, "DSM_LCD_SUB_CHECKSUM_ERROR_NO"},
		{922001513, "DSM_LCD_SUB_LVD_DETECT_ERROR_NO"},
		{922001514, "DSM_LCD_SUB_MBIST_ERROR_NO"},
		{922001515, "DSM_LCD_SUB_MIPI_CHECK_ERROR_NO"},
		{925004311, "DSM_DSI_DETECT_ERROR_NO"},
	};
	uint32_t lcd_error_no_map_len = sizeof(lcd_error_no_map) / sizeof(error_no_map);

	for (i = 0; i < lcd_error_no_map_len; i++) {
		if (lcd_error_no_map[i].error_no == error_no) {
			if (strlen(lcd_error_no_map[i].name) >= buff_len)
				return -1;

			ret = strncpy_s(str, buff_len, lcd_error_no_map[i].name, buff_len - 1);
			if (ret != EOK) {
				dpu_pr_err("strncpy_s error");
				return -1;
			}
		}
		return 0;
	}
	return -1;
}

struct dsm_client_ops hlcd_dsm_ops = {
	.poll_state = NULL,
	.dump_func = NULL,
	.errorno_to_str = lcd_errorno_to_str,
};

static struct dsm_dev dsm_lcd_dev = {
	.name = "LCD",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &hlcd_dsm_ops,
	.buff_size = 5120,
};

struct dsm_client *dsm_lcd_client;

struct dsm_client* dkmd_get_dmd_client(void)
{
	return dsm_lcd_client;
}

void dksm_dmd_report_vactive_end_miss(uint32_t conn_id)
{
	uint32_t cnt = 0;

	dpu_check_and_no_retval(!dsm_lcd_client, info, "dsm_lcd_client is null!");

	while ((dsm_client_ocuppy(dsm_lcd_client) != 0) && (cnt < DSM_OCCUPY_RETRY_TIMES)) {
		dpu_pr_warn("dsm_client_ocuppy failed, retry %d times", ++cnt);
		udelay(2);
	}

	if (cnt == DSM_OCCUPY_RETRY_TIMES) {
		dpu_pr_warn("dsm_client_ocuppy failed");
		return;
	}

	dsm_client_record(dsm_lcd_client, "NOTICE: do not receive vactive end itr of last frame, conn_id = %u", conn_id);

	dsm_client_notify(dsm_lcd_client, DSM_LCD_VACTIVE_TIMEOUT_ERROR_NO);
}

#if defined (CONFIG_HUAWEI_OHOS_DSM)

void dksm_dmd_register(uint32_t index, const char *lcd_name)
{
	dpu_pr_info("+");

	dpu_check_and_no_retval(dsm_lcd_client, info, "dsm_lcd_client is not null!");

	if (!lcd_name) {
		dpu_pr_warn("lcd_name is null\n");
		lcd_name = "unknown_panel";
	}

	if (index != DEVICE_COMP_PRIMARY_ID)
		return;

	dsm_lcd_dev.module_name = lcd_name;
	dsm_lcd_client = dsm_register_client(&dsm_lcd_dev);
	if (dsm_lcd_client)
		dpu_pr_info("regist dmd client succ, module_name=%s\n", lcd_name);
	else
		dpu_pr_warn("regist dmd client fail, module_name=%s\n", lcd_name);
}

#endif

