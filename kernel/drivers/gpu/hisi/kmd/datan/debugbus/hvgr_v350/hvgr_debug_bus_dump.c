/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_debug_bus_dump.h"

#include <securec.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/moduleparam.h>
#include <linux/statfs.h>
#include <linux/namei.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/spinlock.h>
#include <linux/security.h>

#include "hvgr_regmap.h"
#include "hvgr_dm_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_file.h"

const static char *debug_bus_tag = "debug_bus";

static bool debug_bus_enable = true;
module_param(debug_bus_enable, bool, S_IRUGO | S_IWUSR);

static bool page_fault_debug_bus_enable;
module_param(page_fault_debug_bus_enable, bool, S_IRUGO | S_IWUSR);

static unsigned int gpu_hang_count;
module_param(gpu_hang_count, uint, S_IRUGO | S_IWUSR);

bool kdatan_pagefault_delay_reset(void)
{
	return page_fault_debug_bus_enable;
}

u32 kdatan_debug_bus_get_buffer_size(struct hvgr_device * const gdev)
{
	u32 buffer_size;
	u32 core_number;

	if (gdev == NULL)
		return 0u;

	core_number = hvgr_get_core_nums(gdev);
	if (core_number == 0u) {
		hvgr_err(gdev, HVGR_DATAN, "debugbus use default gpc nums");
		core_number = (u32)HVGR_DEBUG_BUS_MAX_GPC_CORE_ID + 1u;
	}
	buffer_size = HVGR_DEBUG_BUS_BUFFER_SIZE_PRE_GPC * core_number +
		HVGR_DEBUG_BUS_BUFFER_SIZE_MISC;
	return buffer_size;
}

static bool kdatan_debug_bus_write_to_buffer(struct bus_dump_resource_info *info, const char *data)
{
	struct hvgr_device *gdev = info->gdev;
	loff_t *pos = &info->pos;
	u32 buffer_size = info->buffer_size;
	int ret;

	if (*pos + DUMP_DATA_MAX_SIZE > buffer_size) {
		hvgr_err(gdev, HVGR_DATAN, "debugbus data is too big");
		return false;
	}

	ret = snprintf_s(info->buffer + *pos, buffer_size - *pos, buffer_size - *pos - 1, "%s",
		data);
	if (ret < 0) {
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);
		return false;
	}
	*pos += (loff_t)ret;
	return true;
}

static bool kdatan_debug_bus_record_reg_data(struct bus_dump_resource_info *info, u32 addr)
{
	struct hvgr_device *gdev = info->gdev;
	char data[DUMP_DATA_MAX_SIZE] = {0};
	int ret;
	u32 val = hvgr_read_reg(gdev, addr);

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, RECORD_FORMAT, addr,
		val);
	if (ret < 0) {
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);
		return false;
	}

	return kdatan_debug_bus_write_to_buffer(info, data);
}

static u32 kdatan_debug_bus_read(struct hvgr_device * const gdev, u32 cfg)
{
	u32 ret = 0;
	int count = 0;
	unsigned long flags;
	struct hvgr_debugbus_dev * const datan_bus_dev = &gdev->datan_dev.datan_bus_dev;

	spin_lock_irqsave(&datan_bus_dev->hwaccess_lock, flags);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_DBG_CFG), cfg);

	/* retry read GPU_DBG_STATUS data for MAX_READ_TIMES */
	while ((!hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_DBG_STATUS)))
			&& (count < MAX_READ_TIMES))
		count++;

	if (count >= MAX_READ_TIMES) {
		spin_unlock_irqrestore(&datan_bus_dev->hwaccess_lock, flags);
		return ret;
	}

	ret = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_DBG_RDATA));
	spin_unlock_irqrestore(&datan_bus_dev->hwaccess_lock, flags);
	return ret;
}

static void kdatan_bus_dump_state_type(struct bus_dump_resource_info *info, int core_id,
	int dest_id, int state_type, int state_size)
{
	u32 ret;
	int res;
	int dbg_addr = 0;
	bool dump_success = true;
	char data[DUMP_DATA_MAX_SIZE] = {0};
	struct hvgr_device *gdev = info->gdev;

	for (; dbg_addr <= state_size; dbg_addr++) {
		u32 cfg = (u32)dbg_addr |
			(((u32)state_type) << HVGR_DEBUG_BUS_STATE_TYPE_OFFSET) |
			(((u32)dest_id) << HVGR_DEBUG_BUS_DEST_ID_OFFSET) |
			(((u32)core_id) << HVGR_DEBUG_BUS_CORE_ID_OFFSET);
		ret = kdatan_debug_bus_read(gdev, cfg);
		if (ret == INVALID_REGISTER_VALUE)
			continue;

		(void)memset_s(data, DUMP_DATA_MAX_SIZE, 0, DUMP_DATA_MAX_SIZE);
		res = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, RECORD_FORMAT,
				 cfg, ret);
		if (res < 0)
			hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

		if (!kdatan_debug_bus_write_to_buffer(info, data))
			dump_success = false;
	}

	if (!dump_success)
		hvgr_err(gdev, HVGR_DATAN, "%s err write data to buffer", __func__);
}

static void kdatan_bus_dump_gpc_vtc_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_VTC, STATE_TYEP_0, DEST_ID_GPC_VTC_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_sdc_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SDC, STATE_TYEP_0, DEST_ID_GPC_SDC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SDC, STATE_TYEP_1, DEST_ID_GPC_SDC_STATE_1_SIZE);
}

static void kdatan_bus_dump_gpc_lsc_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_LSC, STATE_TYEP_0, DEST_ID_GPC_LSC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_LSC, STATE_TYEP_2, DEST_ID_GPC_LSC_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_LSC, STATE_TYEP_3, DEST_ID_GPC_LSC_STATE_3_SIZE);
}

static void kdatan_bus_dump_gpc_gtc_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_GTC, STATE_TYEP_0, DEST_ID_GPC_GTC_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_wm_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_WM, STATE_TYEP_0, DEST_ID_GPC_WM_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_rttc_data(struct bus_dump_resource_info *info, int core_id)
{
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_RTTC, STATE_TYEP_0, DEST_ID_GPC_RTTC_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_dm_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_DM, STATE_TYEP_0, DEST_ID_GPC_DM_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_sq_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SQ, STATE_TYEP_0, DEST_ID_GPC_SQ_STATS_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SQ, STATE_TYEP_1, DEST_ID_GPC_SQ_STATS_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SQ, STATE_TYEP_2, DEST_ID_GPC_SQ_STATS_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SQ, STATE_TYEP_3, DEST_ID_GPC_SQ_STATS_3_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_SQ, STATE_TYEP_4, DEST_ID_GPC_SQ_STATS_4_SIZE);
}

static void kdatan_bus_dump_gpc_dg_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_DG, STATE_TYEP_0, DEST_ID_GPC_DG_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_eu_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_EU, STATE_TYEP_0, DEST_ID_GPC_EU_STATS_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_EU1, STATE_TYEP_0, DEST_ID_GPC_EU1_STATS_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_EU2, STATE_TYEP_0, DEST_ID_GPC_EU2_STATS_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_EU3, STATE_TYEP_0, DEST_ID_GPC_EU3_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_rcp_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_RCP, STATE_TYEP_0, DEST_ID_GPC_RCP_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_ts_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TS, STATE_TYEP_0, DEST_ID_GPC_TS_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_rast_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_RAST, STATE_TYEP_0, DEST_ID_GPC_RAST_STATS_0_SIZE);
}

static void kdatan_bus_dump_gpc_zs_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_ZS, STATE_TYEP_0, DEST_ID_GPC_ZS_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_ZS, STATE_TYEP_2, DEST_ID_GPC_ZS_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_ZS, STATE_TYEP_4, DEST_ID_GPC_ZS_STATE_4_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_ZS, STATE_TYEP_6, DEST_ID_GPC_ZS_STATE_6_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_ZS, STATE_TYEP_8, DEST_ID_GPC_ZS_STATE_8_SIZE);
}

static void kdatan_bus_dump_gpc_blnd_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_0, DEST_ID_GPC_BLND_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_2, DEST_ID_GPC_BLND_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_4, DEST_ID_GPC_BLND_STATE_4_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_6, DEST_ID_GPC_BLND_STATE_6_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_8, DEST_ID_GPC_BLND_STATE_8_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_10, DEST_ID_GPC_BLND_STATE_10_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_BLND, STATE_TYEP_12, DEST_ID_GPC_BLND_STATE_12_SIZE);
}

static void kdatan_bus_dump_gpc_vary_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_VARY, STATE_TYEP_0, DEST_ID_GPC_VARY_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_cbi_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_CBI, STATE_TYEP_0, DEST_ID_GPC_CBI_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_pec_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_PEC, STATE_TYEP_0, DEST_ID_GPC_PEC_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_cpb_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_CPB, STATE_TYEP_0, DEST_ID_GPC_CPB_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_tu_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TSM, STATE_TYEP_0, DEST_ID_GPC_TSM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TSM, STATE_TYEP_1, DEST_ID_GPC_TSM_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TRD, STATE_TYEP_0, DEST_ID_GPC_TRD_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TAG, STATE_TYEP_0, DEST_ID_GPC_TAG_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TSG, STATE_TYEP_0, DEST_ID_GPC_TSG_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TRS, STATE_TYEP_0, DEST_ID_GPC_TRS_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TCC, STATE_TYEP_0, DEST_ID_GPC_TCC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TCC, STATE_TYEP_2, DEST_ID_GPC_TCC_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TCC, STATE_TYEP_3, DEST_ID_GPC_TCC_STATE_3_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_GPC_TFU, STATE_TYEP_0, DEST_ID_GPC_TFU_STATE_0_SIZE);
}

static void kdatan_bus_dump_gpc_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_gpc_vtc_data(info, core_id);
	kdatan_bus_dump_gpc_sdc_data(info, core_id);
	kdatan_bus_dump_gpc_lsc_data(info, core_id);
	kdatan_bus_dump_gpc_gtc_data(info, core_id);
	kdatan_bus_dump_gpc_wm_data(info, core_id);
	kdatan_bus_dump_gpc_rttc_data(info, core_id);
	kdatan_bus_dump_gpc_dm_data(info, core_id);
	kdatan_bus_dump_gpc_sq_data(info, core_id);
	kdatan_bus_dump_gpc_dg_data(info, core_id);
	kdatan_bus_dump_gpc_eu_data(info, core_id);
	kdatan_bus_dump_gpc_rcp_data(info, core_id);
	kdatan_bus_dump_gpc_ts_data(info, core_id);
	kdatan_bus_dump_gpc_rast_data(info, core_id);
	kdatan_bus_dump_gpc_zs_data(info, core_id);
	kdatan_bus_dump_gpc_blnd_data(info, core_id);
	kdatan_bus_dump_gpc_vary_data(info, core_id);
	kdatan_bus_dump_gpc_cbi_data(info, core_id);
	kdatan_bus_dump_gpc_pec_data(info, core_id);
	kdatan_bus_dump_gpc_cpb_data(info, core_id);
	kdatan_bus_dump_gpc_tu_data(info, core_id);
}

static void kdatan_bus_dump_com_ctrl_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_COM_CTRL, STATE_TYEP_0, DEST_ID_COM_CTRL_STATE_0_SIZE);
}

static void kdatan_bus_dump_bvh_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BVHM, STATE_TYEP_0, DEST_ID_BVH_BVHM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BSETUP, STATE_TYEP_0, DEST_ID_BVH_BSETUP_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BRANK, STATE_TYEP_0, DEST_ID_BVH_BRANK_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BEMIT, STATE_TYEP_0, DEST_ID_BVH_BEMIT_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BEMIT, STATE_TYEP_1, DEST_ID_BVH_BEMIT_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_BSAH, STATE_TYEP_0, DEST_ID_BVH_BSAH_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BVH_PEC, STATE_TYEP_0, DEST_ID_BVH_PEC_STATE_0_SIZE);
}

static void datan_bus_dump_fcm_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_JOB, STATE_TYEP_0, DEST_ID_FCM_JOB_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_JOB, STATE_TYEP_2, DEST_ID_FCM_JOB_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_JOB, STATE_TYEP_3, DEST_ID_FCM_JOB_STATE_3_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_CQ_QM, STATE_TYEP_0, DEST_ID_FCM_CQ_QM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_CQ_SM, STATE_TYEP_0, DEST_ID_FCM_CQ_SM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_FCP, STATE_TYEP_0, DEST_ID_FCM_FCP_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_FCP_CTRL, STATE_TYEP_0, DEST_ID_FCM_FCP_CTRL_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_MISC_CTRL, STATE_TYEP_0, DEST_ID_FCM_MISC_CTRL_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_PEC, STATE_TYEP_0, DEST_ID_FCM_PEC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_TOP_FCM, STATE_TYEP_0, DEST_ID_FCM_TOP_FCM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_FCM_TOP_GPU, STATE_TYEP_0, DEST_ID_FCM_TOP_GPU_STATE_0_SIZE);
}

static void datan_bus_dump_l2mmu_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_L2RO, STATE_TYEP_0, DEST_ID_L2MMU_L2RO_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_L2RW, STATE_TYEP_0, DEST_ID_L2MMU_L2RW_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_L2MIF, STATE_TYEP_0, DEST_ID_L2MMU_L2MIF_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_0, DEST_ID_L2MMU_MMU_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_1, DEST_ID_L2MMU_MMU_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_2, DEST_ID_L2MMU_MMU_STATE_2_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_3, DEST_ID_L2MMU_MMU_STATE_3_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_4, DEST_ID_L2MMU_MMU_STATE_4_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_5, DEST_ID_L2MMU_MMU_STATE_5_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_6, DEST_ID_L2MMU_MMU_STATE_6_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_7, DEST_ID_L2MMU_MMU_STATE_7_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_8, DEST_ID_L2MMU_MMU_STATE_8_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MMU, STATE_TYEP_9, DEST_ID_L2MMU_MMU_STATE_9_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_GBI, STATE_TYEP_0, DEST_ID_L2MMU_GBI_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_MCB, STATE_TYEP_0, DEST_ID_L2MMU_MCB_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_CPB, STATE_TYEP_0, DEST_ID_L2MMU_CPB_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_PEU, STATE_TYEP_0, DEST_ID_L2MMU_PEU_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_RTU, STATE_TYEP_0, DEST_ID_L2MMU_RTU_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_RTU1, STATE_TYEP_0, DEST_ID_L2MMU_RTU1_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_L2VPM, STATE_TYEP_0, DEST_ID_L2MMU_L2VPM_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB, STATE_TYEP_0, DEST_ID_L2MMU_UTLB_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB, STATE_TYEP_1,	DEST_ID_L2MMU_UTLB_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB1, STATE_TYEP_0, DEST_ID_L2MMU_UTLB1_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB1, STATE_TYEP_1, DEST_ID_L2MMU_UTLB1_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB2, STATE_TYEP_0, DEST_ID_L2MMU_UTLB2_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_L2MMU_UTLB2, STATE_TYEP_1, DEST_ID_L2MMU_UTLB2_STATE_1_SIZE);
}

static void datan_bus_dump_bin_debug_data(struct bus_dump_resource_info *info, int core_id)
{
	hvgr_info(info->gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_JCU, STATE_TYEP_0, DEST_ID_BIN_JCU_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_VF, STATE_TYEP_0, DEST_ID_BIN_VF_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_VF, STATE_TYEP_1, DEST_ID_BIN_VF_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_PA, STATE_TYEP_0, DEST_ID_BIN_PA_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_TESS, STATE_TYEP_0, DEST_ID_BIN_TESS_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_TESS, STATE_TYEP_1, DEST_ID_BIN_TESS_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_UTC, STATE_TYEP_0, DEST_ID_BIN_UTC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_OA, STATE_TYEP_0, DEST_ID_BIN_OA_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_OA, STATE_TYEP_1, DEST_ID_BIN_OA_STATE_1_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_GPC_ARB, STATE_TYEP_0, DEST_ID_BIN_GPC_ARB_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_BTC_TOP, STATE_TYEP_0, DEST_ID_BIN_BTC_TOP_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_BTS, STATE_TYEP_0, DEST_ID_BIN_BTS_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_BRAST, STATE_TYEP_0, DEST_ID_BIN_BRAST_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_BCZ, STATE_TYEP_0, DEST_ID_BIN_BCZ_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_REJ_SYNC, STATE_TYEP_0, DEST_ID_BIN_REJ_SYNC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_SPU, STATE_TYEP_0, DEST_ID_BIN_SPU_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_SC, STATE_TYEP_0, DEST_ID_BIN_SC_STATE_0_SIZE);
	kdatan_bus_dump_state_type(info, core_id, DEST_ID_BIN_PEC, STATE_TYEP_0, DEST_ID_BIN_PEC_STATE_0_SIZE);
}

static void datan_bus_dump_gpu_control_register(struct bus_dump_resource_info *info)
{
	struct hvgr_device *gdev = info->gdev;
	u32 addr = gdev->dm_dev.reg_base.gpu_reg_base;
	char data[DUMP_DATA_MAX_SIZE] = {0};
	u32 reg_offset;
	int ret;

	hvgr_info(gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);

	reg_offset = gpu_control_reg(gdev, GPU_DBG_RDATA);

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, GPU_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

	kdatan_debug_bus_write_to_buffer(info, data);

	for (; addr <= reg_offset; addr += (u32)sizeof(u32))
		kdatan_debug_bus_record_reg_data(info, addr);
}

static void datan_bus_dump_cq_control_register(struct bus_dump_resource_info *info)
{
	struct hvgr_device *gdev = info->gdev;
	int chn;
	int ret;
	u32 addr = cq_control_reg(gdev, CQ_ENABLE);
	u32 reg_offset;
	char data[DUMP_DATA_MAX_SIZE] = {0};

	hvgr_info(gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, CQ_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);
	kdatan_debug_bus_write_to_buffer(info, data);

	kdatan_debug_bus_record_reg_data(info, cq_control_reg(gdev, CQ_IRQ_TOP));
	kdatan_debug_bus_record_reg_data(info, cq_control_reg(gdev, CQ_SUBMIT));

	for (chn = 0; chn < CQ_CHANNEL_NUM; chn++) {
		addr = cq_control_reg(gdev, cq_irq_rawstat(chn));
		reg_offset = cq_control_reg(gdev, cq_bind_subchn_os(chn));
		for (; addr <= reg_offset; addr += (u32)sizeof(u32))
			kdatan_debug_bus_record_reg_data(info, addr);
	}
}

static void datan_bus_dump_misc_control_register(struct bus_dump_resource_info *info)
{
	struct hvgr_device *gdev = info->gdev;
	int ret;
	u32 i;
	u32 addr = GPU_IRQ_RAWSTAT;
	u32 reg_offset = GPU_IRQ_STATUS;
	char data[DUMP_DATA_MAX_SIZE] = {0};

	hvgr_info(gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, MISC_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

	kdatan_debug_bus_write_to_buffer(info, data);

	for (; addr <= reg_offset; addr += (u32)sizeof(u32))
		kdatan_debug_bus_record_reg_data(info, addr);

	for (i = 0; i < FAULT_NUMBER; i++) {
		addr = GPU_FAULTSTATUS;
		reg_offset = GPU_FAULTDATA3;
		addr += i * 0x20;
		reg_offset += i * 0x20;
		for (; addr <= reg_offset; addr += (u32)sizeof(u32))
			kdatan_debug_bus_record_reg_data(info, addr);
	}

	for (addr = JS_PRESENT; addr <= FF_CONFIG_6; addr += (u32)sizeof(u32))
		kdatan_debug_bus_record_reg_data(info, addr);
}

static const int job_control_reg_dump[] = {
	JOB_IRQ_STATUS
};

static const int job_slot_reg_dump[] = {
	JSX_IRQ_RAWSTAT,
	JSX_IRQ_MASK,
	JSX_IRQ_STATUS
};

static const int job_jc_reg_dump[] = {
	JSX_JCY_HEAD_LO,
	JSX_JCY_HEAD_HI,
	JSX_JCY_AFFINITY,
	JSX_JCY_CONFIG,
	JSX_JCY_JCD_LO,
	JSX_JCY_JCD_HI,
	JSX_JCY_JD_MSB,
	JSX_JCY_WAIT_CYCLE
};

static void datan_bus_dump_job_control_register(struct bus_dump_resource_info *info)
{
	struct hvgr_device *gdev = info->gdev;
	u32 addr;
	size_t array_size;
	u32 i;
	u32 j;
	char data[DUMP_DATA_MAX_SIZE] = {0};
	int ret;

	hvgr_info(gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, JOB_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

	kdatan_debug_bus_write_to_buffer(info, data);

	/* get the Job control registers */
	array_size = ARRAY_SIZE(job_control_reg_dump);
	for (i = 0; i < array_size; i++) {
		addr = job_control_reg(gdev, job_control_reg_dump[i]);
		kdatan_debug_bus_record_reg_data(info, addr);
	}

	/* get the Job Slot registers */
	array_size = ARRAY_SIZE(job_slot_reg_dump);
	for (j = 0; j < BASE_FCM_MAX_NR_SLOTS; j++) {
		for (i = 0; i < array_size; i++) {
			addr = job_slot_x_reg(gdev, j, job_slot_reg_dump[i]);
			kdatan_debug_bus_record_reg_data(info, addr);
		}
	}

	/* get the Job Chain level registers */
	array_size = ARRAY_SIZE(job_jc_reg_dump);
	for (j = 0; j < BASE_FCM_MAX_NR_SLOTS; j++) {
		int jc;

		for (jc = 0; jc < JOB_CHAIN_NUM; jc++) {
			u32 jc_idx = ((u32)j << BASE_NUM_TWO) + (u32)jc;

			for (i = 0; i < array_size; i++) {
				addr = job_jc_y_reg(gdev, jc_idx, job_jc_reg_dump[i]);
				kdatan_debug_bus_record_reg_data(info, addr);
			}
		}
	}
}

static void datan_bus_dump_mmu_register(struct bus_dump_resource_info *info)
{
	char data[DUMP_DATA_MAX_SIZE] = {0};
	int ret;

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, MMU_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(info->gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

	kdatan_debug_bus_write_to_buffer(info, data);
}

static void datan_bus_dump_fcp_dma_regs(struct bus_dump_resource_info *info)
{
	u32 dma_ch = 0;
	u32 addr;
	u32 dump_addr;
	struct hvgr_device *gdev = info->gdev;

	kdatan_debug_bus_record_reg_data(info, fcp_dma_reg(gdev, FCP_DMA_CH_INT_TCRAW));

	for (; dma_ch < MAX_DMA_CHANNEL_NUMBER; dma_ch++) {
		addr = FCP_DMA_CX_CNT;
		for (; addr <= FCP_DMA_BUFDATA; addr += FCP_REG_SIZE) {
			dump_addr = fcp_dma_chanel_reg(gdev, dma_ch, addr);
			kdatan_debug_bus_record_reg_data(info, dump_addr);
		}
	}
}

static void datan_bus_dump_fcp_register(struct bus_dump_resource_info *info)
{
	char data[DUMP_DATA_MAX_SIZE] = {0};
	int ret;

	ret = snprintf_s(data, DUMP_DATA_MAX_SIZE, DUMP_DATA_MAX_SIZE - 1, FCP_CONTROL_TITLE);
	if (ret < 0)
		hvgr_err(info->gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);

	kdatan_debug_bus_write_to_buffer(info, data);
	datan_bus_dump_fcp_dma_regs(info);
}

static bool hvgr_debug_bus_get_duplicated_file_name(char *file_name, char *time)
{
	struct kstat fstat;
	int ret;

	ret = snprintf_s(file_name, MAX_FILE_NAME_SIZE, MAX_FILE_NAME_SIZE - 1,
		HVGR_DEBUG_BUS_DATA_FILE_DUPLICATED,
		HVGR_RELEASE_NAME, time);
	if (ret < 0)
		return false;

	if (vfs_stat(file_name, &fstat) < 0)
		return true;

	/* If file exits, delete it first */
	if (hvgr_file_delete_file(file_name) < 0)
		return false;

	return true;
}

static void datan_debug_bus_get_common_string(char *file_name, char *time)
{
	struct rtc_time tm;
	int ret;
	struct hvgr_device *gdev;

	gdev = hvgr_get_device();
	rtc_time64_to_tm(get_seconds(), &tm);
	ret = snprintf_s(time, MAX_TIME_SIZE, MAX_TIME_SIZE + 1, HVGR_DEBUG_BUS_TIME,
		tm.tm_year + BASE_YEAR, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
		tm.tm_min, tm.tm_sec);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "%s %s", debug_bus_tag, time);

	ret = snprintf_s(file_name, MAX_FILE_NAME_SIZE, MAX_FILE_NAME_SIZE - 1,
		HVGR_DEBUG_BUS_DATA_FILE, HVGR_RELEASE_NAME, time);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "%s %s", debug_bus_tag, file_name);
}

static bool datan_debug_bus_get_dump_file_name(char *file_name, int dump_mode, char *time)
{
	struct kstat fstat;

	if (dump_mode == HVGR_DEBUG_BUS_DUMP_SEC)
		return hvgr_debug_bus_get_duplicated_file_name(file_name, time);

	if (dump_mode != HVGR_DEBUG_BUS_DUMP_FIR)
		return false;

	datan_debug_bus_get_common_string(file_name, time);

	if (vfs_stat(file_name, &fstat) < 0)
		return true;

	/* If file exits, delete it first */
	if (hvgr_file_delete_file(file_name) < 0)
		return false;

	return true;
}

static void datan_get_debug_bus_version(struct hvgr_device * const gdev, char *data, size_t size)
{
	int ret;
	/* For V350, Not For V100 or V120 or V200 */
	ret = snprintf_s(data, size, size - 1, VERSION_350);
	if (ret < 0)
		hvgr_err(gdev, HVGR_DATAN, "debug bus [%s] snprintf_s failed.", __func__);
}

static void datan_bus_dump_core_register(struct bus_dump_resource_info *info)
{
	struct hvgr_device *gdev = info->gdev;
	u32 core_id = 0;

	hvgr_info(gdev, HVGR_DATAN, "%s %s in", debug_bus_tag, __func__);
	for (; core_id <= HVGR_DEBUG_BUS_MAX_CORE_ID; core_id++) {
		u64 core_mask = (1 << core_id);
		core_mask &= hvgr_pm_get_availiable_cores(gdev);
		hvgr_info(gdev, HVGR_DATAN, "core_mask:%lu.", core_mask);
		if (core_mask != 0)
			kdatan_bus_dump_gpc_debug_data(info, core_id);

		/* core_id equal to HVGR_DEBUG_BUS_MAX_COM_CTRL_CORE_ID(12), dump com data */
		if (core_id == HVGR_DEBUG_BUS_MAX_COM_CTRL_CORE_ID)
			kdatan_bus_dump_com_ctrl_debug_data(info, core_id);

		/* core_id equal to HVGR_DEBUG_BUS_FCM_CORE_ID(13), dump fcm data */
		if (core_id == HVGR_DEBUG_BUS_FCM_CORE_ID)
			datan_bus_dump_fcm_debug_data(info, core_id);

		/* core_id equal to HVGR_DEBUG_BUS_L2MMU_CORE_ID(14), dump L2MMU data */
		if (core_id == HVGR_DEBUG_BUS_L2MMU_CORE_ID)
			datan_bus_dump_l2mmu_debug_data(info, core_id);

		/* core_id equal to HVGR_DEBUG_BUS_BIN_COER_ID(15), dump bin data */
		if (core_id == HVGR_DEBUG_BUS_BIN_COER_ID)
			datan_bus_dump_bin_debug_data(info, core_id);

		/* core_id equal to MAX_COM_CTRL_CORE_ID */
		if (core_id == HVGR_DEBUG_BUS_MAX_COM_CTRL_CORE_ID)
			kdatan_bus_dump_com_ctrl_debug_data(info, core_id);

		/* core_id equal to MAX_BVH_CORE_ID */
		if (core_id == HVGR_DEBUG_BUS_MAX_BVH_CORE_ID)
			kdatan_bus_dump_bvh_debug_data(info, core_id);
	}
}

inline static bool policy_cores_powered(struct hvgr_device * const gdev)
{
	int gpu_status = hvgr_pm_get_gpu_status(gdev);
	return gpu_status == HVGR_PM_GPU_READY;
}

static void hvgr_debug_bus_write_buffer_to_file(char *buffer, int dump_mode, char *time)
{
	char file_name[MAX_FILE_NAME_SIZE] = {0};
	struct file *data_file = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	if (strlen(buffer) == 0)
		return;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (!datan_debug_bus_get_dump_file_name(file_name, dump_mode, time)) {
		set_fs(old_fs);
		return;
	}

	data_file = filp_open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUGO | S_IWUSR);
	if (IS_ERR_OR_NULL(data_file)) {
		set_fs(old_fs);
		return;
	}
	hvgr_file_write_to_file(data_file, buffer, strlen(buffer), &pos);
	(void)vfs_fsync(data_file, 0);

	(void)filp_close(data_file, NULL);
	set_fs(old_fs);
}

void hvgr_debug_bus_write_to_file(char *buffer, char *file_name)
{
	struct file *data_file = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	if (strlen(buffer) == 0)
		return;
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	data_file = filp_open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUGO | S_IWUSR);
	if (IS_ERR_OR_NULL(data_file)) {
		set_fs(old_fs);
		return;
	}
	hvgr_file_write_to_file(data_file, buffer, strlen(buffer), &pos);
	(void)vfs_fsync(data_file, 0);

	(void)filp_close(data_file, NULL);
	set_fs(old_fs);
}

bool kdatan_need_to_dump_duplicately(void)
{
	return true;
}

#ifdef CONFIG_HVGR_DFX_DEBUG_BUS_DUMP
static bool datan_debug_bus_check_disk(void)
{
	int num;

	if ((hvgr_file_get_free_space(DATA_DIR) >> HVGR_FILE_BYTE2MB) < MIN_FREE_SPACE)
		return false;

	num = hvgr_file_get_file_num(HVGR_DEBUG_BUS_DATA_DIR, HVGR_DEBUG_BUS_FILE_PREFIX);
	if (num == -ENOENT)
		return true;
	else if (num < 0)
		return false;

	return num < HVGR_DEBUG_BUS_MAX_NUMBER;
}

static int kdatan_bus_dump_data_to_buffer(struct bus_dump_resource_info *info)
{
	char data[DUMP_DATA_MAX_SIZE] = {0};
	struct hvgr_device *gdev = info->gdev;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);
	if (!policy_cores_powered(gdev)) {
		hvgr_err(gdev, HVGR_DATAN, "debugbus gpu is not ready.");
		return -1;
	}

	datan_get_debug_bus_version(gdev, data, DUMP_DATA_MAX_SIZE);
	kdatan_debug_bus_write_to_buffer(info, data);

	datan_bus_dump_core_register(info);
	datan_bus_dump_job_control_register(info);
	datan_bus_dump_gpu_control_register(info);
	datan_bus_dump_misc_control_register(info);
	datan_bus_dump_mmu_register(info);
	datan_bus_dump_fcp_register(info);
	datan_bus_dump_cq_control_register(info);
	hvgr_info(gdev, HVGR_DATAN, "%s OUT", __func__);
	return 0;
}

void kdatan_debug_bus_start_dump(struct hvgr_device * const gdev)
{
	int ret = -1;
	int dup_ret = -1;
	struct bus_dump_resource_info info = { 0 };
	struct hvgr_debugbus_dev * const datan_bus_dev = &gdev->datan_dev.datan_bus_dev;
	int buffer_cnt = 0;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);
	if (!debug_bus_enable)
		return;

	if (datan_debug_bus_check_disk()) {
		mutex_lock(&datan_bus_dev->debug_bus_mutex);
		if (datan_bus_dev->debugbus_cnt != 0) {
			hvgr_err(gdev, HVGR_DATAN, "%s debugbus already Dump count %d",
				__func__, datan_bus_dev->debugbus_cnt);
			mutex_unlock(&datan_bus_dev->debug_bus_mutex);
			return;
		}

		buffer_cnt = kdatan_need_to_dump_duplicately() ?
			HVGR_DEBUG_BUS_CNT_SEC : HVGR_DEBUG_BUS_CNT_FIR ;
		info.gdev = gdev;
		info.buffer_size = kdatan_debug_bus_get_buffer_size(gdev);

		datan_bus_dev->debugbus_buf = kvzalloc(info.buffer_size * buffer_cnt, GFP_KERNEL);
		if (!datan_bus_dev->debugbus_buf) {
			hvgr_err(gdev, HVGR_DATAN, "%s debugbus alloc buffer fail", __func__);
			mutex_unlock(&datan_bus_dev->debug_bus_mutex);
			return;
		}

		datan_bus_dev->debugbus_buf[0] = '\0';
		info.buffer = datan_bus_dev->debugbus_buf;
		ret = kdatan_bus_dump_data_to_buffer(&info);

		/*
		 * Dump twice for file comparison
		 * Same files indicate that gpu job hangs
		 * Different files indicate that gpu job timeouts
		 */
		if (kdatan_need_to_dump_duplicately()) {
			hvgr_info(gdev, HVGR_DATAN, "%s duplicately IN", __func__);
			datan_bus_dev->debugbus_buf[info.buffer_size] = '\0';
			info.pos = 0;
			info.buffer = &datan_bus_dev->debugbus_buf[info.buffer_size];
			dup_ret = kdatan_bus_dump_data_to_buffer(&info);
		}

		if (ret != 0 || dup_ret != 0) {
			hvgr_err(gdev, HVGR_DATAN, "%s debugbus dump fail", __func__);
			vfree(datan_bus_dev->debugbus_buf);
			datan_bus_dev->debugbus_buf = NULL;
			mutex_unlock(&datan_bus_dev->debug_bus_mutex);
			return;
		}
		datan_bus_dev->debugbus_cnt++;
		mutex_unlock(&datan_bus_dev->debug_bus_mutex);
		/* Queue a work to write debug bus data into files */
		(void)queue_work(datan_bus_dev->gpu_debugbus_wait_wq,
			&datan_bus_dev->gpu_debugbus_wait_work);
		hvgr_info(gdev, HVGR_DATAN, "%s Dump OUT", __func__);
	} else {
		hvgr_err(gdev, HVGR_DATAN, "free space is not enough, skip");
	}
	gpu_hang_count++;
}

void kdatan_debug_bus_write_file_worker(struct work_struct *data)
{
	int ret = 0;
	char time[MAX_TIME_SIZE] = {0};
	struct hvgr_device *gdev = NULL;
	struct hvgr_datan_dev *datan_dev = NULL;
	struct hvgr_debugbus_dev *bus_dev = NULL;
	u32 buffer_size = 0;

	bus_dev = container_of(data, struct hvgr_debugbus_dev, gpu_debugbus_wait_work);
	datan_dev = bus_dev->global_datan_dev;
	gdev = container_of(datan_dev, struct hvgr_device, datan_dev);

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	/* Judge the /data/hisi_logs/debugbus/ Dir exist ? */
	mutex_lock(&datan_dev->dump_dir_lock);
	if (!hvgr_file_make_dir(HVGR_DEBUG_BUS_DATA_DIR, HVGR_DEBUG_BUS_DATA_DIR_MODE)) {
		hvgr_err(gdev, HVGR_DATAN, "%s mkdir fail", __func__);
		ret = -1;
	}
	mutex_unlock(&datan_dev->dump_dir_lock);

	if (!datan_debug_bus_check_disk())
		ret = -1;

	mutex_lock(&bus_dev->debug_bus_mutex);
	if (bus_dev->debugbus_buf != NULL) {
		if (ret == 0) {
			hvgr_debug_bus_write_buffer_to_file(bus_dev->debugbus_buf,
				HVGR_DEBUG_BUS_DUMP_FIR, time);
			/*
			 * Dump twice for file comparison
			 * Same files indicate that gpu job hangs
			 * Different files indicate that gpu job timeouts
			 */
			buffer_size = kdatan_debug_bus_get_buffer_size(gdev);
			if (kdatan_need_to_dump_duplicately())
				hvgr_debug_bus_write_buffer_to_file(
					&bus_dev->debugbus_buf[buffer_size],
					HVGR_DEBUG_BUS_DUMP_SEC, time);
		}
		vfree(bus_dev->debugbus_buf);
		bus_dev->debugbus_buf = NULL;
	}
	bus_dev->debugbus_cnt--;
	mutex_unlock(&bus_dev->debug_bus_mutex);
	return;
}

void kdatan_debug_bus_cbit_config(struct hvgr_device * const gdev)
{
	u32 val;

	if (!debug_bus_enable)
		return;

	/* XBI_CFG Cbit config */
	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, CCE_CBI_CBIT));
	val |= HVGR_XBI_CONFIG_OFFSET;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, CCE_CBI_CBIT), val);

	/* BTC Cbit config */
	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, BTC_CONFIG_4));
	val |= HVGR_BTC_CONFIG_OFFSET;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, BTC_CONFIG_4), val);

	/* GPC Cbit config */
	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_2));
	val |= HVGR_GPC_2_CONFIG_OFFSET;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_2), val);

	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_3));
	val |= HVGR_GPC_3_CONFIG_OFFSET;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_3), val);

	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_4));
	val |= HVGR_GPC_4_CONFIG_OFFSET;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_4), val);
}

void kdatan_bus_dump_thread_init(struct hvgr_device * const gdev)
{
	struct hvgr_datan_dev * const datan_dev_tmp = &gdev->datan_dev;
	struct hvgr_debugbus_dev * const datan_bus_dev = &gdev->datan_dev.datan_bus_dev;

	hvgr_info(gdev, HVGR_DATAN, "%s IN", __func__);

	mutex_init(&datan_bus_dev->debug_bus_mutex);
	spin_lock_init(&datan_bus_dev->hwaccess_lock);

	datan_bus_dev->global_datan_dev = datan_dev_tmp;
	datan_bus_dev->debugbus_buf = NULL;
	datan_bus_dev->debugbus_cnt = 0;

	gpu_hang_count = 0;
	kdatan_debug_bus_cbit_config(gdev);

	datan_bus_dev->gpu_debugbus_wait_wq = alloc_workqueue("hvgr_debugbus_wait",
						WQ_HIGHPRI | WQ_UNBOUND, 1);
	if (datan_bus_dev->gpu_debugbus_wait_wq == NULL) {
		hvgr_err(gdev, HVGR_DATAN, "debugbus alloc_workqueue fail");
		return;
	}

	INIT_WORK(&datan_bus_dev->gpu_debugbus_wait_work, kdatan_debug_bus_write_file_worker);
}

void kdatan_bus_dump_thread_term(struct hvgr_device * const gdev)
{
	struct hvgr_debugbus_dev * const datan_bus_dev = &gdev->datan_dev.datan_bus_dev;

	if (datan_bus_dev->gpu_debugbus_wait_wq != NULL) {
		flush_workqueue(datan_bus_dev->gpu_debugbus_wait_wq);
		destroy_workqueue(datan_bus_dev->gpu_debugbus_wait_wq);
	}
	if (datan_bus_dev->debugbus_buf != NULL)
		vfree(datan_bus_dev->debugbus_buf);
	datan_bus_dev->debugbus_buf = NULL;
}

void hvgr_kdatan_dump_debug_bus_data(struct hvgr_device * const gdev, u32 type)
{
	kdatan_debug_bus_start_dump(gdev);
}

void hvgr_kdatan_dump_debug_bus_data_for_ecall(void)
{
	struct hvgr_device *gdev = hvgr_get_device();

	if (gdev == NULL)
		return;

	(void)hvgr_pm_request_gpu(gdev, false);
	kdatan_debug_bus_start_dump(gdev);
	hvgr_pm_release_gpu(gdev);
	hvgr_info(gdev, HVGR_DATAN, "%s success", __func__);
}

void kdatan_dump_chain_state(struct hvgr_device * const gdev)
{
	const u32 state_base_l = 260; // base addr of LOW SCH CHAIN STATE
	const u32 state_base_h = 388; // base addr of HIGH SCH CHAIN STATE
	int i;
	u32 val_l, val_h;
	u32 addr_base = (((u32)STATE_TYEP_2) << HVGR_DEBUG_BUS_STATE_TYPE_OFFSET) |
		(((u32)DEST_ID_FCM_JOB) << HVGR_DEBUG_BUS_DEST_ID_OFFSET) |
		(((u32)HVGR_DEBUG_BUS_FCM_CORE_ID) << HVGR_DEBUG_BUS_CORE_ID_OFFSET);

	for (i = 0; i < MAX_JOB_SLOT_NR; i++) {
		u32 cfg_l = ((u32)i + state_base_l) | addr_base;
		u32 cfg_h = ((u32)i + state_base_h) | addr_base;

		val_l = kdatan_debug_bus_read(gdev, cfg_l);
		val_h = kdatan_debug_bus_read(gdev, cfg_h);
		hvgr_debug(gdev, HVGR_DATAN, " JS%d STATE_LOW=0x%08x, STATE_HIGH=0x%08x ",
			i, val_l, val_h);
	}
}

#endif
// end CONFIG_HVGR_DFX_DEBUG_BUS_DUMP
