/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    mc_drv.c
 * Description:
 * Date         2022-07-04
 ******************************************************************/
#include <linux/printk.h>
#include "ipp.h"
#include "mc_drv.h"
#include "mc_drv_priv.h"
#include "mc_reg_offset.h"
#include "mc_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_MC_DRV

static void mc_en_config(mc_dev_t *dev, mc_en_cfg_t *mc_en)
{
	// start mc, It should be configured last;
	macro_cmdlst_set_reg(dev->base_addr + MC_EN_CFG_REG, mc_en->u32);
}

static void mc_imu_config(mc_dev_t *dev, mc_imu_cfg_t *imu_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_IMU_CFG_REG, imu_cfg->u32);
}

static void mc_ctrl_config(mc_dev_t *dev, mc_ctrl_cfg_t *ctrl_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_MC_CFG_REG, ctrl_cfg->u32);
}

static void mc_inlier_thld_config(mc_dev_t *dev, mc_inlier_thld_t *inlier_thld)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_THRESHOLD_CFG_REG, inlier_thld->u32);
}

static void mc_inlier_num_config(mc_dev_t *dev, mc_inlier_number_t *inlier_number)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_INLIER_NUMBER_REG, inlier_number->u32);
}

static void mc_kpt_size_config(mc_dev_t *dev, mc_kpt_size_cfg_t *kpt_size)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_KPT_SIZE_REG, kpt_size->u32);
}

static void mc_static_config(mc_dev_t *dev, mc_static_cfg_t *static_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + MC_STATIC_CFG1_REG, static_cfg->static_cfg1.u32);
	macro_cmdlst_set_reg(dev->base_addr + MC_STATIC_CFG2_REG, static_cfg->static_cfg2.u32);
}

static void mc_matched_kpts_config(mc_dev_t *dev, cfg_tab_mc_t *tab_mc)
{
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	if (tab_mc->mc_cascade_en == CASCADE_ENABLE) {
		tab_mc->mc_match_points_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
		logd("mc_match_points_addr = 0x%x", tab_mc->mc_match_points_addr);
	}

	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
	macro_cmdlst_set_reg(dev->base_addr + MC_MATCH_POINTS_REG, tab_mc->matched_kpts_cfg.u32);
}

static void mc_prefetch_config(mc_dev_t *dev, mc_prefetch_t *prefetch_cfg)
{
	if (prefetch_cfg->cur_prefetch_cfg.bits.cur_prefetch_enable != 0) {
		loge("warning: cur_prefetch_enable must configured to 0, because no prefetch");
		prefetch_cfg->cur_prefetch_cfg.bits.cur_prefetch_enable = 0;
	}

	if (prefetch_cfg->ref_prefetch_cfg.bits.ref_prefetch_enable != 0) {
		loge("warning: ref_prefetch_enable must configured to 0, because no prefetch");
		prefetch_cfg->ref_prefetch_cfg.bits.ref_prefetch_enable = 0;
	}

	macro_cmdlst_set_reg(dev->base_addr  + MC_REF_PREFETCH_CFG_REG, prefetch_cfg->ref_prefetch_cfg.u32);
	macro_cmdlst_set_reg(dev->base_addr  + MC_CUR_PREFETCH_CFG_REG, prefetch_cfg->cur_prefetch_cfg.u32);
}

static void mc_index_pairs_config(mc_dev_t *dev, cfg_tab_mc_t *tab_mc)
{
	unsigned int i = 0;
	unsigned int mc_reg_size = 0;
	unsigned int mc_data_size = MC_KPT_NUM_MAX; // must cfg 1600 times;
	mc_index_pairs_cfg_t *index_pairs_cfg = &(tab_mc->index_pairs_cfg);

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	macro_cmdlst_set_reg_burst_data_align(MC_KPT_NUM_MAX, CVDR_ALIGN_BYTES);
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	tab_mc->mc_index_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
	logd("mc_index_addr = 0x%x, mc_data_size = %d, mc_cascade_en = %d",
		tab_mc->mc_index_addr, mc_data_size, tab_mc->mc_cascade_en);

	if (tab_mc->mc_cascade_en == CASCADE_ENABLE) {
		for (i = 0; i < (mc_data_size / CMDLST_BURST_READ_MAX_SIZE); i++) {
			mc_reg_size = ((mc_data_size - i * CMDLST_BURST_READ_MAX_SIZE) > CMDLST_BURST_READ_MAX_SIZE) ?
							CMDLST_BURST_READ_MAX_SIZE :
							(mc_data_size - i * CMDLST_BURST_READ_MAX_SIZE);
			macro_cmdlst_set_reg_incr(dev->base_addr + MC_INDEX_CFG_0_REG, mc_reg_size, INCR_MODE, WR_OPS_MODE);
			macro_cmdlst_set_addr_offset(mc_reg_size);
		}
	} else if (tab_mc->mc_cascade_en == CASCADE_DISABLE) {
		for (i = 0; i < mc_data_size; i++)
			macro_cmdlst_set_reg(dev->base_addr + MC_INDEX_CFG_0_REG + i * BYTES_PER_WORD, index_pairs_cfg->index_pairs[i].u32);
	}

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
}

static void mc_coord_pairs_config(mc_dev_t *dev, mc_coord_pairs_cfg_t *coord_pairs_cfg)
{
	unsigned int i = 0;

	// Cur and ref should be configured alternately and the cur is configured first; must cfg 1600*2 times;
	for (i = 0; i < MC_KPT_NUM_MAX; i++) {
		macro_cmdlst_set_reg(dev->base_addr + MC_COORDINATE_CFG_0_REG, coord_pairs_cfg->cur_coordinate[i].u32);
		macro_cmdlst_set_reg(dev->base_addr + MC_COORDINATE_CFG_0_REG, coord_pairs_cfg->ref_coordinate[i].u32);
	}
}

static int mc_drv_para_check(cfg_tab_mc_t *tab_mc)
{
	mc_ctrl_cfg_t *ctrl_cfg = &tab_mc->ctrl_cfg;

	if (ctrl_cfg->bits.svd_iterations > MC_SVD_ITERATIONS_MAX ||
		ctrl_cfg->bits.svd_iterations < MC_SVD_ITERATIONS_MIN) {
		loge("config_table->svd_iterations = %d error", ctrl_cfg->bits.svd_iterations);
		return ISP_IPP_ERR;
	}

	if (ctrl_cfg->bits.max_iterations < MC_H_MATRIX_ITERATIONS_MIN) {
		loge("config_table->max_iterations = %d error", ctrl_cfg->bits.max_iterations);
		return ISP_IPP_ERR;
	}

	if (tab_mc->kpt_size_cfg.bits.kpt_size != ARF_DESCRIPTOR_SIZE) { // must be 144;
		loge("Error descriptor size: kpt_size = %d", tab_mc->kpt_size_cfg.bits.kpt_size);
		return ISP_IPP_ERR;
	}

	if (tab_mc->matched_kpts_cfg.bits.matched_kpts > MC_KPT_NUM_MAX) {
		loge("Error matched_kpts = %d", tab_mc->matched_kpts_cfg.bits.matched_kpts);
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}


static int mc_do_config(mc_dev_t *dev, cfg_tab_mc_t *tab_mc)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(tab_mc, ret);
	if (ret) return ret;

	if (mc_drv_para_check(tab_mc)) {
		loge("Failed: mc_drv_para_check");
		return ISP_IPP_ERR;
	}

	mc_imu_config(dev, &(tab_mc->imu_cfg));
	mc_ctrl_config(dev, &(tab_mc->ctrl_cfg));
	mc_inlier_thld_config(dev, &(tab_mc->inlier_thld_cfg));
	mc_inlier_num_config(dev, &(tab_mc->inlier_number_cfg));
	mc_kpt_size_config(dev, &(tab_mc->kpt_size_cfg));
	mc_static_config(dev, &(tab_mc->static_cfg));
	mc_matched_kpts_config(dev, tab_mc);
	mc_prefetch_config(dev, &(tab_mc->prefech_cfg));

	if (tab_mc->ctrl_cfg.bits.cfg_mode == MC_CFG_MODE_INDEX_PAIRS)
		mc_index_pairs_config(dev, tab_mc);

	if (tab_mc->ctrl_cfg.bits.cfg_mode == MC_CFG_MODE_COORD_PAIRS)
		mc_coord_pairs_config(dev, &(tab_mc->coord_pairs_cfg));

	return ISP_IPP_OK;
}

int mc_prepare_cmd(mc_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_mc_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	mc_dev_t mc_dev = *dev;
	mc_dev.cmd_buf = cmd_buf;
	if (mc_do_config(&mc_dev, table)) {
		loge("Failed: mc_do_config");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

int mc_enable_cmd(mc_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_mc_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	mc_dev_t mc_dev = *dev;
	mc_dev.cmd_buf = cmd_buf;
	if (table->mc_en_cfg.bits.mc_en == 0) {
		loge("Failed: mc_en is 0");
		return ISP_IPP_ERR;
	}
	mc_en_config(&mc_dev, &(table->mc_en_cfg));
	return ISP_IPP_OK;
}

static mc_ops_t mc_ops = {
	.prepare_cmd = mc_prepare_cmd,
};

mc_dev_t g_mc_devs[MC_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_MC,
		.ops = &mc_ops,
	},
};

/* ********************************* END ********************************* */

