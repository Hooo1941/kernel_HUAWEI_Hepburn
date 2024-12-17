/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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

#include <securec.h>
#include <linux/types.h>
#include <linux/fb.h>

#include <linux/kthread.h>
#include <uapi/linux/sched/types.h>

#include "dpu_comp_mgr.h"
#include "dpu_effect_hiace.h"
#include "dpu_hiace_init.h"
#include "dkmd_mipi_panel_info.h"

#define GAMMA_AB_MASK BIT(0)
#define LUMA_GAMMA_AB_MASK BIT(1)
#define LOG_LUMA_AB_MASK BIT(2)
#define DETAIL_WEIGHT_AB_MASK BIT(3)

static int dpu_hiace_get_lhist_band(char __iomem *hiace_base)
{
	uint32_t lhist_en;
	int lhist_quant;
	int lhist_band;

	lhist_en = inp32(DPU_HIACE_LHIST_EN_ADDR(hiace_base));
	lhist_quant = (lhist_en >> 10) & 0x1;
	if (lhist_quant == 0)
		lhist_band = 16;
	else
		lhist_band = 8;

	return lhist_band;
}

static void dpu_read_local_hist(char __iomem *hiace_base, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	int i;
	int lhist_band;
	uint32_t local_valid = 0;
	uint32_t *local_hist_ptr = NULL;

	lhist_band = dpu_hiace_get_lhist_band(hiace_base);
	local_valid = inp32(DPU_HIACE_LOCAL_VALID_ADDR(hiace_base));
	dpu_pr_debug("local_valid =%d, lhist_band = %d", local_valid, lhist_band);
	if (local_valid == 1) {
		hiace_ctrl->status.valid_info |= DKMD_HIACE_HIST_VALID_LOCAL;
		local_hist_ptr = hiace_ctrl->hist.local_hist;
		set_reg(DPU_HIACE_LHIST_EN_ADDR(hiace_base), 1, 1, 31);

		for (i = 0; i < (DKMD_XBLOCKNUM * DKMD_YBLOCKNUM * lhist_band); i++)
			local_hist_ptr[i] = inp32(DPU_HIACE_LOCAL_HIST_ADDR(hiace_base));

		set_reg(DPU_HIACE_LHIST_EN_ADDR(hiace_base), 0, 1, 31);
		outp32(DPU_HIACE_UPDATE_LOCAL_ADDR(hiace_base), 1);
	}
}

static void dpu_read_fna_data(char __iomem *hiace_base, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	int i;
	uint32_t fna_valid = 0;
	uint32_t *fna_data_ptr = NULL;

	fna_valid = inp32(DPU_HIACE_FNA_VALID_ADDR(hiace_base));
	dpu_pr_debug("fna_valid =%d", fna_valid);
	if (fna_valid == 1) {
		hiace_ctrl->status.valid_info |= DKMD_HIACE_HIST_VALID_FNA;
		fna_data_ptr = hiace_ctrl->hist.fna_data;
		set_reg(DPU_HIACE_FNA_ADDR_ADDR(hiace_base), 1, 1, 31);

		for (i = 0; i < (DKMD_XBLOCKNUM * DKMD_YBLOCKNUM); i++)
			fna_data_ptr[i] = inp32(DPU_HIACE_FNA_DATA_ADDR(hiace_base));

		set_reg(DPU_HIACE_FNA_ADDR_ADDR(hiace_base), 0, 1, 31);
		outp32(DPU_HIACE_UPDATE_FNA_ADDR(hiace_base), 1);
	}
}

static void dpu_read_global_hist(char __iomem *dpu_base, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	uint32_t skin_count_val;
	uint32_t global_hist_ab_shadow;
	uint32_t global_hist_ab_work;
	uint32_t *global_hist_ptr = NULL;
	uint32_t *sat_global_hist_ptr = NULL;
	uint32_t *skin_count_ptr = NULL;
	char __iomem *hiace_base = NULL;
	char __iomem *dpp_base = NULL;
	int i;

	dpp_base = dpu_base + hiace_ctrl->dpp_offset;
	hiace_base = dpp_base + HIACE_OFFSET;
	global_hist_ab_shadow = inp32(DPU_HIACE_GLOBAL_HIST_AB_SHADOW_ADDR(hiace_base));
	global_hist_ab_work = inp32(DPU_HIACE_GLOBAL_HIST_AB_WORK_ADDR(hiace_base));
	dpu_pr_debug("[hiace] global_hist_ab_shadow %d, global_hist_ab_work %d ", global_hist_ab_shadow, global_hist_ab_work);

	if (global_hist_ab_shadow == global_hist_ab_work) {
		hiace_ctrl->status.valid_info |= DKMD_HIACE_HIST_VALID_GLOBAL;
		global_hist_ptr = hiace_ctrl->hist.global_hist;
		for (i = 0; i < DKMD_GLOBAL_HIST_SIZE; i++)
			global_hist_ptr[i] = inp32(DPU_HIACE_GLOBAL_HIST_0_ADDR(hiace_base) + i * 4);

		sat_global_hist_ptr = hiace_ctrl->hist.sat_global_hist;
		for (i = 0; i < DKMD_SAT_GLOBAL_HIST_SIZE; i++)
			sat_global_hist_ptr[i] = inp32(DPU_HIACE_SAT_GLOBAL_HIST_0_ADDR(hiace_base) + i * 4);

		skin_count_ptr = hiace_ctrl->hist.skin_cnt;
		for (i = 0; i < DKMD_SKIN_COUNT_SIZE; i++) {
			skin_count_val = inp32(DPU_DPP_SKINCOUNT_ADDR(dpp_base, i));
			skin_count_ptr[2 * i] = skin_count_val & 0xFFFF;
			skin_count_ptr[2 * i + 1] = (skin_count_val >> 16) & 0xFFFF;
		}

		outp32(DPU_HIACE_GLOBAL_HIST_AB_SHADOW_ADDR(hiace_base), global_hist_ab_shadow ^ 1);
	}
}

static void dpu_hist_status_init(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	mutex_lock(&hiace_ctrl->hist_status_lock);
	if (!hiace_ctrl->hist_status_inited) {
		hiace_ctrl->status.hist_stop = false;
		hiace_ctrl->hist_status_inited = true;
	}
	mutex_unlock(&hiace_ctrl->hist_status_lock);
}

static void dpu_hist_status_deinit(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	mutex_lock(&hiace_ctrl->hist_status_lock);
	if (hiace_ctrl->hist_status_inited) {
		hiace_ctrl->hist_status_inited = false;
		hiace_ctrl->status.hist_stop = true; // fb blank wake up get hist thread
		wake_up_interruptible(&hiace_ctrl->status.wq_hist);
	}
	mutex_unlock(&hiace_ctrl->hist_status_lock);
}

static void dpu_hist_status_update(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	struct hiace_hist_status *service = &hiace_ctrl->status;

	if (hiace_ctrl->hist_status_inited) {
		service->new_hist = true;
		wake_up_interruptible(&service->wq_hist);
	}
}

static bool is_hist_read_intime(char __iomem *dpu_base)
{
	uint32_t vstate;

	vstate = inp32(DPU_DSI_VSTATE_ADDR(dpu_base));
	if (vstate & (LDI_VSTATE_VSW | LDI_VSTATE_VBP | LDI_VSTATE_VACTIVE0 | LDI_VSTATE_V_WAIT_TE0))
		return false;

	return true;
}

static uint32_t convert_cfg_2_mode(uint32_t ppc_config_id)
{
	switch (ppc_config_id) {
	case PPC_CONFIG_ID_F_MODE:
		return F_FOLDING_MODE;
	case PPC_CONFIG_ID_M_MODE:
		return M_FOLDING_MODE;
	case PPC_CONFIG_ID_G_MODE:
		return G_FOLDING_MODE;
	default:
		return DEFAULT_FOLDING_MODE;
	}
}

static int32_t update_hist_rect_info(struct dpu_composer *dpu_comp, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	struct dkmd_connector_info *pinfo = NULL;
	struct dkmd_rect active_rect = {0};

	pinfo = dpu_comp->conn_info;
	dpu_check_and_return(!pinfo, -1, err, "pinfo is null");
	// get_active_display_rect is null, when use dp panel
	dpu_check_and_return(!pinfo->get_active_display_rect, -1, debug, "get_active_display_rect is null");

	if (pinfo->get_active_display_rect(pinfo, &active_rect) != 0) {
		dpu_pr_err("get panel active rect failed");
		return -1;
	}

	hiace_ctrl->hist.hist_rect.left = (uint32_t)active_rect.x;
	hiace_ctrl->hist.hist_rect.top = (uint32_t)active_rect.y;
	hiace_ctrl->hist.hist_rect.right = (uint32_t)active_rect.x + active_rect.w;
	hiace_ctrl->hist.hist_rect.bottom = (uint32_t)active_rect.y + active_rect.h;

	if (is_ppc_support(&pinfo->base))
		hiace_ctrl->hist.folding_mode = convert_cfg_2_mode(pinfo->ppc_config_id_active);
	else
		hiace_ctrl->hist.folding_mode = DEFAULT_FOLDING_MODE;

	return 0;
}

static void dpu_effect_hiace_handle_work(struct kthread_work *work)
{
	uint64_t tv0;
	char __iomem *dpu_base = NULL;
	char __iomem *hiace_base = NULL;
	struct dpu_composer *dpu_comp = NULL;
	struct dkmd_hiace_ctrl *hiace_ctrl = NULL;
	DPU_HIACE_HALF_BLOCK_INFO_UNION half_block_info;

	dpu_comp = container_of(work, struct dpu_composer, hiace_work);
	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");

	hiace_ctrl = dpu_comp->hiace_ctrl;
	dpu_check_and_no_retval(!hiace_ctrl, err, "hiace_ctrl is NULL\n");

	dpu_trace_ts_begin(&tv0);
	dpu_base = dpu_comp->comp_mgr->dpu_base;
	hiace_base = dpu_base + hiace_ctrl->dpp_offset + HIACE_OFFSET;

	down(&dpu_comp->comp.blank_sem);
	if (!dpu_comp->comp.power_on) {
		dpu_pr_debug("panel power off");
		up(&dpu_comp->comp.blank_sem);
		return;
	}

	dpu_hist_status_init(hiace_ctrl);

	/* if last frame local list is not read intime, cause current hist incomplete, skip */
	if (!hiace_ctrl->is_last_hist_read_intime) {
		dpu_pr_debug("[hiace] last lhist is not read intime, current frame skip");
		hiace_ctrl->is_last_hist_read_intime = true;
		up(&dpu_comp->comp.blank_sem);
		return;
	}

	dpu_comp_active_vsync(dpu_comp);

	// Before lhist read: forbid hardware refresh local_hist and fna
	set_reg(DPU_DPP_DBG_HIACE_ADDR(dpu_base + hiace_ctrl->dpp_offset), 0x0, 2, 3);

	mutex_lock(&hiace_ctrl->status.hist_lock);

	hiace_ctrl->hist.sum_sat = inp32(DPU_HIACE_SUM_SATURATION_ADDR(hiace_base));

	dpu_read_local_hist(hiace_base, hiace_ctrl);
	dpu_read_fna_data(hiace_base, hiace_ctrl);

	hiace_ctrl->is_last_hist_read_intime = is_hist_read_intime(dpu_base);
	dpu_pr_debug("[hiace] is_last_hist_read_intime %d", hiace_ctrl->is_last_hist_read_intime);

	// After lhist read: permit hardware refresh local_hist and fna
	set_reg(DPU_DPP_DBG_HIACE_ADDR(dpu_base + hiace_ctrl->dpp_offset), 0x3, 2, 3);

	dpu_read_global_hist(dpu_base, hiace_ctrl);

	hiace_ctrl->hist.lhist_sft = inp32(DPU_HIACE_LHIST_SFT_ADDR(hiace_base));
	hiace_ctrl->hist.lhist_bit_width = (uint32_t)dpu_hiace_get_lhist_band(hiace_base);

	half_block_info.value = inp32(DPU_HIACE_HALF_BLOCK_INFO_ADDR(hiace_base));
	hiace_ctrl->hist.lhist_half_blk_width = half_block_info.reg.reg_half_block_w;
	hiace_ctrl->hist.lhist_half_blk_height = half_block_info.reg.reg_half_block_h;

	if (update_hist_rect_info(dpu_comp, hiace_ctrl) != 0) {
		dpu_pr_debug("[hiace] update hist roi rect from dpu_comp");
		hiace_ctrl->hist.folding_mode = DEFAULT_FOLDING_MODE;
		hiace_ctrl->hist.hist_rect.left = 0;
		hiace_ctrl->hist.hist_rect.top = 0;
		hiace_ctrl->hist.hist_rect.right = dpu_comp->comp.base.xres;
		hiace_ctrl->hist.hist_rect.bottom = dpu_comp->comp.base.yres;
	}

	mutex_unlock(&hiace_ctrl->status.hist_lock);
	dpu_comp_deactive_vsync(dpu_comp);
	up(&dpu_comp->comp.blank_sem);

	hiace_ctrl->hist.valid_info = hiace_ctrl->status.valid_info;
	dpu_pr_debug("[hiace] valid info 0x%x", hiace_ctrl->hist.valid_info);
	/* global or local hist or fna is updated */
	if (hiace_ctrl->status.valid_info & 0x7) {
		dpu_hist_status_update(hiace_ctrl);
		hiace_ctrl->status.valid_info = 0;
	}

	dpu_trace_ts_end(&tv0, "hiace handle finished!");
}

static int handle_err_hist(int wait_ret)
{
	int ret;

	if (wait_ret > 0)
		ret = 3; /* panel on hist not return hist stop is true */
	else if (wait_ret == -ERESTARTSYS)
		ret = 4; /* system err and return -ERESTARTSYS */
	else
		ret = 2; /* hist not return time out */

	return ret;
}

static int32_t dpu_effect_hiace_get_hist(struct dkmd_hiace_ctrl *hiace_ctrl, void __user* argp)
{
	int ret = 0;
	long wait_ret = 0;
	int times = 0;
	unsigned long timeout = msecs_to_jiffies(100000);
	struct hiace_hist_status *status = NULL;

	dpu_check_and_return(unlikely(!argp), -EINVAL, err, "[hiace] argp is NULL");

	status = &hiace_ctrl->status;

	if (!hiace_ctrl->hiace_lock_init) {
		dpu_pr_err("[hiace] hist lock noinit");
		return -EINVAL;
	}

	while (1) {
		wait_ret = wait_event_interruptible_timeout(status->wq_hist, status->new_hist || status->hist_stop,
			(long)timeout);
		if ((wait_ret == -ERESTARTSYS) && (times++ < 100)) {
				dpu_pr_debug("[hiace] erestartsys error");
				mdelay(10);
		} else {
			if (times != 0)
				dpu_pr_warn("[hiace] wait_ret is -ERESTARTSYS, max times=%d", times);
			break;
		}
	}
	status->hist_stop = false;

	down(&hiace_ctrl->hist_lock_sem);
	if (status->new_hist) {
		status->new_hist = false;
		mutex_lock(&status->hist_lock);

		ret = (int)copy_to_user(argp, &hiace_ctrl->hist, sizeof(hiace_ctrl->hist));
		if (ret) {
			dpu_pr_err("[hiace] copy_to_user failed(param)! ret=%d.", ret);
			ret = -1;
		}

		mutex_unlock(&status->hist_lock);
	} else {
		ret = handle_err_hist((int32_t)wait_ret);
	}
	up(&hiace_ctrl->hist_lock_sem);

	return ret;
}

static void set_hiace_gama_lut(char __iomem *hiace_base, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	uint32_t ab_shadow;
	uint32_t ab_work = 0;

	ab_shadow = inp32(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base)) & GAMMA_AB_MASK;
	if (hiace_ctrl->lut.update_info & DKMD_HIACE_LUT_UPDATE_LOCAL_GAMMA) {
		ab_work = inp32(DPU_HIACE_GAMMA_AB_WORK_ADDR(hiace_base)) & GAMMA_AB_MASK;
		dpu_pr_debug("[hiace] ab_shadow=%d, ab_work=%d, sel_hdr_ab_shadow=%d",
			ab_shadow, ab_work, hiace_ctrl->sel_hdr_ab_shadow);
		if (ab_shadow == ab_work) {
			int i = 0;
			set_reg(DPU_HIACE_GAMMA_W_ADDR(hiace_base), 1, 1, 31);
			for (i = 0; i < DKMD_HIACE_GAMA_LUT_SIZE; i++) {
				outp32(DPU_HIACE_GAMMA_VXHY_W_ADDR(hiace_base), hiace_ctrl->lut.gamma_lut[i]);

				// print with interval 10
				if (i % 10 == 0)
					dpu_pr_debug("gamma_lut[%d] 0x%x", i, hiace_ctrl->lut.gamma_lut[i]);
			}
			set_reg(DPU_HIACE_GAMMA_W_ADDR(hiace_base), 0, 1, 31);
			ab_shadow ^= GAMMA_AB_MASK;
			hiace_ctrl->lut.update_info &= ~DKMD_HIACE_LUT_UPDATE_LOCAL_GAMMA;
		} else {
			dpu_pr_warn("LOCAL_GAMMA: ab_shadow %d != ab_work %d", ab_shadow, ab_work);
		}
	}
	set_reg(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base),
		(ab_shadow & 0x1) | (hiace_ctrl->sel_hdr_ab_shadow & 0xe), 4, 0);
}

static void set_hiace_hdr_lut(char __iomem *dpu_base, struct dkmd_hiace_ctrl *hiace_ctrl)
{
	uint32_t ab_shadow;
	uint32_t ab_work = 0;
	uint32_t i = 0;
	uint32_t temp = 0;
	uint32_t sel_hdr_ab_shadow = 0;
	char __iomem *dpp_base = dpu_base + hiace_ctrl->dpp_offset;
	char __iomem *hiace_base = dpp_base + HIACE_OFFSET;

	sel_hdr_ab_shadow = inp32(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base)) & 0xe;

	if (hiace_ctrl->lut.update_info & DKMD_HIACE_LUT_UPDATE_DETAIL_WEIGHT) {
		ab_shadow = inp32(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base)) & DETAIL_WEIGHT_AB_MASK;
		ab_work = inp32(DPU_HIACE_GAMMA_AB_WORK_ADDR(hiace_base)) & DETAIL_WEIGHT_AB_MASK;
		if (ab_shadow == ab_work) {
			for (i = 0; i < (DKMD_DETAIL_WEIGHT_SIZE - 1); i++) {
				temp = ((hiace_ctrl->lut.detail_weight_lut[i * 4]) |
					(hiace_ctrl->lut.detail_weight_lut[i * 4 + 1] << 8) |
					(hiace_ctrl->lut.detail_weight_lut[i * 4 + 2] << 16) |
					(hiace_ctrl->lut.detail_weight_lut[i * 4 + 3] << 24));
				outp32(DPU_DPP_DETAIL_WEIGHT_ADDR(dpp_base, i), temp);
				dpu_pr_debug("DETAIL_WEIGHT 0x%x", temp);
			}

			outp32(DPU_DPP_DETAIL_WEIGHT_ADDR(dpp_base, i),
				(hiace_ctrl->lut.detail_weight_lut[DKMD_HIACE_DETAIL_WEIGHT_TABLE_SIZE - 1]));

			sel_hdr_ab_shadow ^= DETAIL_WEIGHT_AB_MASK;
			hiace_ctrl->lut.update_info &= ~DKMD_HIACE_LUT_UPDATE_DETAIL_WEIGHT;
		} else {
			dpu_pr_warn("DETAIL_WEIGHT: ab_shadow %d != ab_work %d", ab_shadow, ab_work);
		}
	}

	if (hiace_ctrl->lut.update_info & DKMD_HIACE_LUT_UPDATE_LOG_LUMA) {
		ab_shadow = inp32(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base)) & LOG_LUMA_AB_MASK;
		ab_work = inp32(DPU_HIACE_GAMMA_AB_WORK_ADDR(hiace_base)) & LOG_LUMA_AB_MASK;
		if (ab_shadow == ab_work) {
			for (i = 0; i < (DKMD_LOG_LUM_EOTF_LUT_SIZE - 1); i++) {
				temp = ((hiace_ctrl->lut.log_lum_lut[i * 2] & 0x3FFF) |
					(hiace_ctrl->lut.log_lum_lut[i * 2 + 1] << 16));
				outp32(DPU_DPP_LOG_LUM_ADDR(dpp_base, i), temp);
				dpu_pr_debug("LOG_LUMA 0x%x", temp);
			}
			outp32(DPU_DPP_LOG_LUM_ADDR(dpp_base, i),
				(hiace_ctrl->lut.log_lum_lut[DKMD_LOG_LUM_EOTF_LUT_SIZE * 2 - 2]));

			sel_hdr_ab_shadow ^= LOG_LUMA_AB_MASK;
			hiace_ctrl->lut.update_info &= ~DKMD_HIACE_LUT_UPDATE_LOG_LUMA;
		} else {
			dpu_pr_warn("LOG_LUMA: ab_shadow %d != ab_work %d", ab_shadow, ab_work);
		}
	}

	if (hiace_ctrl->lut.update_info & DKMD_HIACE_LUT_UPDATE_LUMA_GAMMA) {
		ab_shadow = inp32(DPU_HIACE_GAMMA_AB_SHADOW_ADDR(hiace_base)) & LUMA_GAMMA_AB_MASK;
		ab_work = inp32(DPU_HIACE_GAMMA_AB_WORK_ADDR(hiace_base)) & LUMA_GAMMA_AB_MASK;
		if (ab_shadow == ab_work) {
			for (i = 0; i < DKMD_LUMA_GAMA_LUT_SIZE; i++) {
				temp = (hiace_ctrl->lut.luma_gamma_lut[i * 3]) |
					(hiace_ctrl->lut.luma_gamma_lut[i * 3 + 1] << 10) |
					(hiace_ctrl->lut.luma_gamma_lut[i * 3 + 2] << 20);
				dpu_pr_debug("LUMA_GAMA 0x%x", temp);
				outp32(DPU_DPP_LUMA_GAMMA_ADDR(dpp_base, i), temp);
			}

			sel_hdr_ab_shadow ^= LUMA_GAMMA_AB_MASK;
			hiace_ctrl->lut.update_info &= ~DKMD_HIACE_LUT_UPDATE_LUMA_GAMMA;
		} else {
			dpu_pr_warn("LUMA_GAMMA: ab_shadow %d != ab_work %d", ab_shadow, ab_work);
		}
	}

	hiace_ctrl->sel_hdr_ab_shadow = sel_hdr_ab_shadow;
}

static void dpu_effect_hiace_lut_apply(struct dpu_composer *dpu_comp)
{
	struct dkmd_hiace_ctrl *hiace_ctrl = dpu_comp->hiace_ctrl;
	char __iomem *dpu_base = dpu_comp->comp_mgr->dpu_base;
	char __iomem *hiace_base = dpu_base + hiace_ctrl->dpp_offset + HIACE_OFFSET;

	dpu_pr_debug("+ updata_info: 0x%x", hiace_ctrl->lut.update_info);

	mutex_lock(&hiace_ctrl->hiace_lut_lock);

	set_hiace_hdr_lut(dpu_base, hiace_ctrl);
	set_hiace_gama_lut(hiace_base, hiace_ctrl);

	mutex_unlock(&hiace_ctrl->hiace_lut_lock);

	dpu_pr_debug("-");
}

static int32_t dpu_effect_hiace_set_lut(struct dkmd_hiace_ctrl *hiace_ctrl, const void __user* argp)
{
	int ret;

	if (argp == NULL) {
		dpu_pr_err("argp is NULL");
		return -EINVAL;
	}

	mutex_lock(&hiace_ctrl->hiace_lut_lock);
	ret = (int)copy_from_user(&hiace_ctrl->lut, argp, sizeof(hiace_ctrl->lut));
	mutex_unlock(&hiace_ctrl->hiace_lut_lock);
	if (ret) {
		dpu_pr_err("copy_from_user failed! ret=%d", ret);
		return -1;
	}

	return 0;
}

static int dpu_effect_hiace_wake_up_hist(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	mutex_lock(&hiace_ctrl->hist_status_lock);

	hiace_ctrl->status.hist_stop = true; // fb blank wake up get hist thread
	wake_up_interruptible(&hiace_ctrl->status.wq_hist);

	mutex_unlock(&hiace_ctrl->hist_status_lock);

	return 0;
}

static void dpu_effect_hiace_queue_work(struct dpu_composer *dpu_comp)
{
	kthread_queue_work(&dpu_comp->hiace_worker, &dpu_comp->hiace_work);
}

static void dpu_effect_hiace_blank(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	dpu_hist_status_deinit(hiace_ctrl);
}

static void dpu_effect_hiace_unblank(struct dkmd_hiace_ctrl *hiace_ctrl)
{
	dpu_hist_status_init(hiace_ctrl);
}

void dpu_effect_hiace_init(struct dpu_composer *dpu_comp, uint32_t dpp_offset)
{
	struct dkmd_hiace_ctrl *hiace_ctrl = NULL;
	struct hiace_hist_status *status = NULL;
	struct sched_param param = {
		.sched_priority = MAX_RT_PRIO - 1,
	};

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");

	if (dpu_comp->hiace_ctrl) {
		dpu_pr_warn("hiace already inited");
		return;
	}

	dpu_comp->hiace_ctrl = vzalloc(sizeof(struct dkmd_hiace_ctrl));
	if (!dpu_comp->hiace_ctrl) {
		dpu_pr_err("alloc hiace ctrl failed");
		return;
	}

	hiace_ctrl = dpu_comp->hiace_ctrl;
	status = &hiace_ctrl->status;

	hiace_ctrl->dpp_offset = dpp_offset;
	hiace_ctrl->init_params = dpu_hiace_init_params;
	hiace_ctrl->blank = dpu_effect_hiace_blank;
	hiace_ctrl->unblank = dpu_effect_hiace_unblank;
	hiace_ctrl->queue_work = dpu_effect_hiace_queue_work;
	hiace_ctrl->lut_apply = dpu_effect_hiace_lut_apply;

	hiace_ctrl->set_lut = dpu_effect_hiace_set_lut;
	hiace_ctrl->get_hist = dpu_effect_hiace_get_hist;
	hiace_ctrl->wake_up_hist = dpu_effect_hiace_wake_up_hist;

	hiace_ctrl->is_last_hist_read_intime = true;

	if (!hiace_ctrl->hiace_lock_init) {
		sema_init(&hiace_ctrl->hist_lock_sem, 1);
		(void)memset_s(status, sizeof(*status), 0, sizeof(*status));
		mutex_init(&status->hist_lock);
		mutex_init(&hiace_ctrl->hist_status_lock);
		init_waitqueue_head(&status->wq_hist);
		hiace_ctrl->hiace_lock_init = true;
	}

	mutex_init(&hiace_ctrl->hiace_lut_lock);

	/* read hist thread when hiace end */
	kthread_init_worker(&dpu_comp->hiace_worker);
	dpu_comp->hiace_thread = kthread_create(kthread_worker_fn, &dpu_comp->hiace_worker, "hiace thread");
	if (IS_ERR_OR_NULL(dpu_comp->hiace_thread)) {
		dpu_pr_err("failed to create hiace_thread!");
		vfree(dpu_comp->hiace_ctrl);
		dpu_comp->hiace_ctrl = NULL;
		return;
	}
	(void)sched_setscheduler_nocheck(dpu_comp->hiace_thread, SCHED_FIFO, &param);
	(void)wake_up_process(dpu_comp->hiace_thread);

	kthread_init_work(&dpu_comp->hiace_work, dpu_effect_hiace_handle_work);
}

void dpu_effect_hiace_deinit(struct dpu_composer *dpu_comp)
{
	struct dkmd_hiace_ctrl *hiace_ctrl = NULL;
	struct hiace_hist_status *status = NULL;

	dpu_check_and_no_retval(!dpu_comp, err, "dpu_comp is NULL\n");

	if (!dpu_comp->hiace_ctrl) {
		dpu_pr_warn("hiace not inited");
		return;
	}

	hiace_ctrl = dpu_comp->hiace_ctrl;
	status = &hiace_ctrl->status;

	if (dpu_comp->hiace_thread) {
		kthread_stop(dpu_comp->hiace_thread);
		dpu_comp->hiace_thread = NULL;
	}

	mutex_destroy(&hiace_ctrl->hiace_lut_lock);

	down(&hiace_ctrl->hist_lock_sem);
	if (hiace_ctrl->hiace_lock_init) {
		mutex_destroy(&status->hist_lock);
		mutex_destroy(&hiace_ctrl->hist_status_lock);
		hiace_ctrl->hiace_lock_init = false;
	}
	up(&hiace_ctrl->hist_lock_sem);

	vfree(dpu_comp->hiace_ctrl);
	dpu_comp->hiace_ctrl = NULL;
}
