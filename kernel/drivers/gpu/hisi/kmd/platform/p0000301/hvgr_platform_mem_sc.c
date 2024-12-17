/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "hvgr_platform_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_log_api.h"

void hvgr_sc_config_remap_register_early(struct hvgr_device *gdev)
{
	dev_info(gdev->dev, "start sc remap ctrl_reg...");
	hvgr_sc_config_remap(gdev);
}

void hvgr_sc_config_remap_register(struct hvgr_device *gdev)
{
	return;
}

void hvgr_sc_enable_scs(struct hvgr_device *gdev)
{
	return;
}

void hvgr_sc_store_remap_regs(struct hvgr_device *gdev)
{
	return;
}

void hvgr_sc_recover_remap_regs(struct hvgr_device *gdev)
{
	return;
}

void hvgr_sc_init_remap_regs(struct hvgr_device *gdev)
{
	return;
}

