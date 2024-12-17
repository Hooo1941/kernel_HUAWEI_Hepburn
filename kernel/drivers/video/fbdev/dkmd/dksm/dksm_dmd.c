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
#include "dksm_dmd.h"
#include "dkmd_comp.h"

static struct dsm_dev dsm_lcd_dev = {
	.name = "dsm_lcd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
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

#ifdef CONFIG_HUAWEI_DSM

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
