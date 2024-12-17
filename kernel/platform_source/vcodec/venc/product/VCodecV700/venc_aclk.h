/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: set venc aclk
 * Create: 2022-10-28
 */

#ifndef __VENC_ACLK_H__
#define __VENC_ACLK_H__

#include "venc_regulator.h"

#define VENC_ACLK_RATE        "aclk_rate"
#define VENC_ACLOCK_NAME      "aclk_venc"

int32_t get_axi_clk_info(struct platform_device *pdev);
int32_t power_on_enable_axi_clk(int32_t core_id);
void power_off_disable_axi_clk(int32_t core_id);
int32_t set_axi_clk_rate_single_core(venc_clk_t clk_type, int32_t core_id);
#endif
