/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "hvgr_platform_api.h"
#include <linux/delay.h>

void hvgr_platform_reset(struct hvgr_device *gdev)
{
	writel(gdev->dm_dev.crg_rst.rst_val,
		gdev->dm_dev.crg_rst.reg_base + gdev->dm_dev.crg_rst.rst_addr);
	udelay(100); /* delay 100 us */
	writel(gdev->dm_dev.crg_rst.rstdis_val,
		gdev->dm_dev.crg_rst.reg_base + gdev->dm_dev.crg_rst.rstdis_addr);
	udelay(20); /* delay 20 us */
}

void hvgr_platform_timestamp_cbit_config(struct hvgr_device *gdev)
{
	return;
}

void hvgr_platform_enable_icg_clk(struct hvgr_device *gdev)
{
	return;
}
