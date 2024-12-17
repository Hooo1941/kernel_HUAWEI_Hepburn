/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __DP_DRV_H__
#define __DP_DRV_H__

#include "dkmd_peri.h"
#include "dp_ctrl.h"
#include <platform_include/display/linux/dpu_dss_dp.h>

enum {
	GFX_DP = 0,
	GFX_DP1,
	GFX_DP2,
};

#define DTS_GFX_DP_NAME    "gfx_dp"
#define DTS_GFX_DP1_NAME   "gfx_dp1"
#define DTS_GFX_DP2_NAME   "gfx_dp2"

#define DTS_COMP_DP        "dkmd,"DTS_GFX_DP_NAME
#define DTS_COMP_DP1       "dkmd,"DTS_GFX_DP1_NAME
#define DTS_COMP_DP2       "dkmd,"DTS_GFX_DP2_NAME

#define MAX_DPTX_COUNT 2
#define MASTER_DPTX_IDX 0
#define SLAVE_DPTX_IDX 1

#define MIN_DPTX_RATE 0
#define MIN_DPTX_LANE 1

#define MAX_DPTX_DEV_INDEX    6 // support six dptx max

struct dp_private {
	struct dkmd_connector_info connector_info;

	char __iomem *hsdt1_crg_base;
	char __iomem *sub_harden_crg_base;
	char __iomem *hsdt1_sysctrl_base;
	char __iomem *dp_sctrl_base;

	/* other dp information */
	struct dp_ctrl dp[MAX_DPTX_COUNT];

	struct platform_device *device;
	uint32_t hpd_gpio;
	uint32_t hpd_irq_no;
	uint32_t dp_hpd_plug;
};

extern int dpu_multi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode,
								TYPEC_PLUG_ORIEN_E typec_orien, uint32_t dp_id, int port_id);

extern struct platform_device *g_dkmd_dp_devive[MAX_DPTX_DEV_INDEX];
int get_dp_devive_index(int port_id, uint32_t *dev_index);
int dptx_init_dpc_num(struct dp_ctrl *dptx);

static inline struct dp_private *to_dp_private(struct dkmd_connector_info *info)
{
	return container_of(info, struct dp_private, connector_info);
}

int32_t dp_set_backlight(struct dpu_connector *connector, const void *value);

#endif
