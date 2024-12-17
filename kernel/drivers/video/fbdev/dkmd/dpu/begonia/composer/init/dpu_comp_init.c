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

#include <dpu/soc_dpu_define.h>
#include <dpu/dpu_itfsw.h>
#include <dkmd_cmdlist.h>
#include "dpu_comp_mgr.h"
#include "dpu_comp_init.h"
#include "dpu_lut_init.h"
#ifdef SURPORT_LOCAL_DIMMING
#include "localdimming/dpu_ld_init.h"
#endif
#include "dpu_cmdlist.h"
#include "dkmd_object.h"
#include "dpu_comp_config_utils.h"
#include "cmdlist_interface.h"
#include "dpu_connector.h"

static void dpu_comp_itfch_config(struct dpu_composer *dpu_comp,
	struct dkmd_connector_info *pinfo, uint32_t cmdlist_id)
{
	uint32_t pre_itfch_offset = (uint32_t)(DPU_ITF_CH0_OFFSET + pinfo->base.pipe_sw_itfch_idx * 0x100);
	dpu_pr_debug("dpu_comp_itfch_config %d", pinfo->base.pipe_sw_itfch_idx);

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_IMG_SIZE_ADDR(pre_itfch_offset),
		((pinfo->base.dsc_out_height - 1) << 16) | (pinfo->base.dsc_out_width - 1));

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_ITFSW_DATA_SEL_ADDR(pre_itfch_offset), 0x0);

	if (pinfo->colorbar_enable) {
		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_DPP_CLRBAR_CTRL_ADDR(pre_itfch_offset),
			((pinfo->base.xres / 12 - 1) << 24) | 0x1);

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_CLRBAR_START_ADDR(pre_itfch_offset), 0x1);
	}
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_REG_CTRL_FLUSH_EN_ADDR(pre_itfch_offset), 0x1);

	if (pinfo->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX] == CONNECTOR_ID_DSI2)
		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_VSYNC_DELAY_CNT_CTRL_ADDR(pre_itfch_offset), 0x1b3);
	else
		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_ITF_CH_VSYNC_DELAY_CNT_CTRL_ADDR(pre_itfch_offset), 0x1a1);
}

static void dpu_comp_pipe_sw_init(struct dpu_composer *dpu_comp, uint32_t cmdlist_id)
{
	uint32_t primary_post_ch_offset = 0x0;
	uint32_t external_post_ch_offset = 0x0;
	uint32_t ctrl_sig_en = 0x0;
	struct dkmd_connector_info *pinfo = dpu_comp->conn_info;

	/* dual-mipi or cphy 1+1 */
	ctrl_sig_en = (pinfo->sw_post_chn_num > EXTERNAL_CONNECT_CHN_IDX) ? 0x0 : 0x1;
	primary_post_ch_offset = pinfo->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX] * 0x1C;
	dpu_pr_debug("dpu_comp_pipe_sw_init %d, %d", pinfo->base.pipe_sw_itfch_idx, primary_post_ch_offset);

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SIG_CTRL_0_ADDR(DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, BIT((unsigned int)pinfo->base.pipe_sw_itfch_idx));

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, ctrl_sig_en);

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_DAT_CTRL_0_ADDR(DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, BIT((unsigned int)pinfo->base.pipe_sw_itfch_idx));

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(DPU_PIPE_SW_OFFSET) \
		+ primary_post_ch_offset, 0x1);

	/* dual-mipi or cphy 1+1 */
	if (pinfo->sw_post_chn_num > EXTERNAL_CONNECT_CHN_IDX) {
		external_post_ch_offset = pinfo->sw_post_chn_idx[EXTERNAL_CONNECT_CHN_IDX] * 0x1C;

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SIG_CTRL_0_ADDR(DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, BIT((unsigned int)pinfo->base.pipe_sw_itfch_idx));

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SW_POS_CTRL_SIG_EN_0_ADDR(DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0x1);

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_DAT_CTRL_0_ADDR(DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, BIT((unsigned int)pinfo->base.pipe_sw_itfch_idx));

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SW_POS_CTRL_DAT_EN_0_ADDR(DPU_PIPE_SW_OFFSET) \
			+ external_post_ch_offset, 0x1);

		/* TODO: check split swap and size config */
		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SPLIT_HSIZE_ADDR(DPU_PIPE_SW_OFFSET),
			((pinfo->base.dsc_out_width / 2 - 1) << 13) | (pinfo->base.dsc_out_width - 1));

		dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SPLIT_CTL_ADDR(DPU_PIPE_SW_OFFSET), 0x0);
		if (pinfo->split_swap_enable)
			dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, DPU_PIPE_SW_SPLIT_CTL_ADDR(DPU_PIPE_SW_OFFSET), 0x1);
	}

	dpu_comp_itfch_config(dpu_comp, pinfo, cmdlist_id);
}

void dpu_comp_release(struct dpu_composer *dpu_comp)
{
	if (unlikely(!dpu_comp)) {
		dpu_pr_err("dpu_comp is null\n");
		return;
	}

	dkmd_cmdlist_release_locked(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist);
	dpu_comp->init_scene_cmdlist = INVALID_CMDLIST_ID;
}

void dpu_comp_init(struct dpu_composer *dpu_comp)
{
	uint32_t cmdlist_id = INVALID_CMDLIST_ID;
	struct dkmd_connector_info *pinfo = NULL;

	if (unlikely(!dpu_comp || !dpu_comp->conn_info || !dpu_comp->comp_mgr)) {
		dpu_pr_err("dpu_comp or present_data or comp_mgr is null\n");
		return;
	}

	pinfo = dpu_comp->conn_info;

	if (dpu_comp->init_scene_cmdlist == INVALID_CMDLIST_ID) {
		/* 1. request initialize cmdlist scene id */
		dpu_comp->init_scene_cmdlist = cmdlist_create_user_client(DPU_SCENE_INITAIL, SCENE_NOP_TYPE, 0, 0);

		/* 2. prepare cmdlist buffer content */
		cmdlist_id = cmdlist_create_user_client(DPU_SCENE_INITAIL, REGISTER_CONFIG_TYPE, 0, PAGE_SIZE);
		if (dpu_enable_lp_flag()) {
			dpu_level1_clock_lp(cmdlist_id);
			dpu_level2_clock_lp(cmdlist_id);
			dpu_ch1_level2_clock_lp(cmdlist_id);
			dpu_memory_lp(cmdlist_id);
		} else {
			dpu_memory_no_lp(cmdlist_id);
			dpu_ch1_memory_no_lp(cmdlist_id);
		}

		if (dpu_lut_init(dpu_comp)) {
			dpu_pr_err("dpu_lut_init fail");
			dkmd_cmdlist_release_locked(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist);
			dpu_comp->init_scene_cmdlist = INVALID_CMDLIST_ID;
			return;
		}

		dpu_lbuf_init(cmdlist_id, pinfo->base.xres, pinfo->base.yres, pinfo->base.dsc_en);
		dpu_sdma_debug_init(cmdlist_id);

#ifdef SURPORT_LOCAL_DIMMING
		if (dpu_ld_init(dpu_comp)) {
			dpu_pr_err("cmdlist get payload fail");
			dkmd_cmdlist_release_locked(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist);
			dpu_comp->init_scene_cmdlist = INVALID_CMDLIST_ID;
			return;
		}
#endif

		if (!is_offline_panel(&dpu_comp->conn_info->base))
			dpu_comp_pipe_sw_init(dpu_comp, cmdlist_id);

		cmdlist_append_client(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist, cmdlist_id);
		cmdlist_flush_client(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist);
	} else {
		cmdlist_flush_client(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist);
	}

	dpu_cmdlist_init_commit(dpu_comp->comp_mgr->dpu_base,
		dkmd_cmdlist_get_dma_addr(DPU_SCENE_INITAIL, dpu_comp->init_scene_cmdlist));

	dpu_enable_m1_qic_intr(dpu_comp->comp_mgr->actrl_base);
}

void send_reset_hardware_event(struct composer *comp)
{
	char *envp[2] = { "ResetHW=1", NULL };

	if (!comp) {
		dpu_pr_err("comp is NULL Pointer");
		return;
	}

	kobject_uevent_env(&(comp->base.peri_device->dev.kobj), KOBJ_CHANGE, envp);
	dpu_pr_info("reset hardware event!");
}
