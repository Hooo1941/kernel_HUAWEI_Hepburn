/*
 * npu_platform_pm.c
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include "npu_platform_pm.h"

#include "npu_log.h"
#include "npu_pm_top_common.h"
#include "npu_pm_top_specify.h"
#include "npu_pm_non_top_common.h"
#include "npu_pm_non_top_specify.h"
#include "npu_pm_framework.h"

enum npu_subip g_powerup_order[NPU_SUBIP_MAX] = {
	NPU_TOP_COMMON,
	NPU_TOP_SPECIFY,
	NPU_NON_TOP_COMMON,
	NPU_NON_TOP_SPECIFY,
	NPU_NON_TOP_SPECIFY_NOSEC
};

enum npu_subip g_powerdown_order[NPU_SUBIP_MAX] = {
	NPU_NON_TOP_SPECIFY_NOSEC,
	NPU_NON_TOP_SPECIFY,
	NPU_NON_TOP_COMMON,
	NPU_TOP_SPECIFY,
	NPU_TOP_COMMON
};

struct npu_pm_subip_action g_pm_subip_action_talbe[NPU_SUBIP_MAX] = {
	{NPU_TOP_COMMON, npu_top_common_powerup, npu_top_common_powerdown},
	{NPU_TOP_SPECIFY, npu_top_specify_powerup, npu_top_sepcify_powerdown},
	{NPU_NON_TOP_COMMON, npu_non_top_common_powerup, npu_non_top_common_powerdown},
	{NPU_NON_TOP_SPECIFY, npu_non_top_specify_powerup, npu_non_top_specify_powerdown},
	{NPU_NON_TOP_SPECIFY_NOSEC, npu_non_top_specify_nosec_powerup, npu_non_top_specify_nosec_powerdown},
};

struct npu_pm_work_strategy g_pm_work_strategy_table[NPU_WORKMODE_MAX] = {
	{
		NPU_NONSEC,
		{npu_bitmap_set(0, NPU_SEC)},
		1,
		{NPU_SUBIP_ALL}
	},
	{
		NPU_SEC,
		{npu_bitmap_set(0, NPU_NONSEC) |
		npu_bitmap_set(0, NPU_ISPNN_SEPARATED) |
		npu_bitmap_set(0, NPU_ISPNN_SHARED) | npu_bitmap_set(0, NPU_INIT)},
		0,
		{npu_bitmap_clear(NPU_SUBIP_ALL, NPU_NON_TOP_SPECIFY_NOSEC)}
	},
	{
		NPU_ISPNN_SEPARATED, /* do the same things with ISPNN_SHARED */
		{npu_bitmap_set(0, NPU_SEC)},
		0,
		{NPU_SUBIP_ALL}
	},
	{
		NPU_ISPNN_SHARED,
		{npu_bitmap_set(0, NPU_SEC)},
		0,
		{NPU_SUBIP_ALL}
	},
	{
		NPU_INIT,
		{npu_bitmap_set(0, NPU_SEC)},
		0,
		{NPU_SUBIP_ALL}
	},
	{
		NPU_NONSEC_GE,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_NONSEC_EFF,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_NONSEC_TS,
		{npu_bitmap_set(0, NPU_SEC)},
		0,
		{NPU_SUBIP_ALL}
	},
	{
		NPU_FFRT,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_FFRT_GE,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_FFRT_EFF,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_HTS,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_HTS_GE,
		{UINT32_MAX},
		false,
		{0}
	},
	{
		NPU_HTS_EFF,
		{UINT32_MAX},
		false,
		{0}
	}
};

int npu_pm_init(struct npu_dev_ctx *dev_ctx)
{
	struct npu_power_mgr *power_mgr = NULL;

	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	power_mgr = &dev_ctx->pm;
	power_mgr->strategy_table_addr =
		(vir_addr_t)(uintptr_t)&g_pm_work_strategy_table[0];
	power_mgr->action_table_addr =
		(vir_addr_t)(uintptr_t)&g_pm_subip_action_talbe[0];
	power_mgr->powerup_order = g_powerup_order;
	power_mgr->powerdown_order = g_powerdown_order;
	power_mgr->power_array_size = ARRAY_SIZE(g_powerup_order);

	return 0;
}

int npu_plat_switch_hwts_aicore_pool(struct npu_dev_ctx *dev_ctx,
	npu_work_mode_info_t *work_mode_info, uint32_t power_status)
{
	unused(dev_ctx);
	unused(work_mode_info);
	unused(power_status);

	/* no need to switch hwts aicore pool in this version */
	return 0;
}

int npu_plat_dev_pm_suspend(void)
{
	return 0;
}
