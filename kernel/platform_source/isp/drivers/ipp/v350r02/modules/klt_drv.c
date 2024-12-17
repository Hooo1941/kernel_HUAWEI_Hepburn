/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    klt_drv.c
 * Description:
 *
 * Date        2021-11-27
 ******************************************************************/
#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "klt_drv.h"
#include "klt_drv_priv.h"
#include "klt_reg_offset.h"
#include "klt_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_KLT_DRV
#define KLT_REG_OFFSET (0x4)
#define KLT_PREV_BUFF_REG_OFFSET (0x8)

static void klt_start_config(klt_dev_t *dev, klt_start_cfg_t *klt_start)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_KLT_START_REG, klt_start->u32);
}

static void klt_fwd_search_config(klt_dev_t *dev, klt_fwd_search_cfg_t *fwd_search)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_FWD_CONFIG_REG, fwd_search->u32);
}

static void klt_bwd_search_config(klt_dev_t *dev, klt_bwd_search_cfg_t *bwd_search)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_BWD_CONFIG_REG, bwd_search->u32);
}

static void klt_iter_cfg_config(klt_dev_t *dev, klt_iter_cfg_cfg_t *iter_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_ITER_CONFIG_1_REG, iter_cfg->iter_cfg1.u32);
	macro_cmdlst_set_reg(dev->base_addr + KLT_ITER_CONFIG_2_REG, iter_cfg->iter_cfg2.u32);
}

static void klt_eval_param_config(klt_dev_t *dev, klt_eval_param_cfg_t *eval_param)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_EVAL_PARAM_REG, eval_param->u32);
}

static void klt_prefetch_ctrl_config(klt_dev_t *dev, klt_prefetch_ctrl_t *prefetch_ctrl)
{
	klt_prefetch_ctrl_t tmp;
	tmp.u32 = 0; // NOT USED IN V350R02;
	macro_cmdlst_set_reg(dev->base_addr + KLT_PREFETCH_CTRL_REG, tmp.u32);
}

static void klt_prev_and_next_rd_config(klt_dev_t *dev,
	klt_prev_and_next_rd_cfg_t *rd_cfg, unsigned int ds_num_layer)
{
	unsigned int i;

	for (i = 0; i < ds_num_layer; i++) {
		// 1. prev
		macro_cmdlst_set_reg(dev->base_addr + KLT_RD_PREV_BASE_ADDR_0_REG + (KLT_REG_OFFSET * i),
							rd_cfg->rd_prev_base_addr[i].u32);

		// When the rd_prev_line_stride's value is '0', means that the line stride is computed by the KLT.
		macro_cmdlst_set_reg(dev->base_addr + KLT_RD_PREV_STRIDE_0_REG + (KLT_REG_OFFSET * i),
							rd_cfg->rd_prev_line_stride[i].u32);

		// 2. next
		macro_cmdlst_set_reg(dev->base_addr + KLT_RD_NEXT_BASE_ADDR_0_REG + (KLT_REG_OFFSET * i),
							rd_cfg->rd_next_base_addr[i].u32);

		// When the rd_next_line_stride's value is '0', means that the line stride is computed by the KLT.
		macro_cmdlst_set_reg(dev->base_addr + KLT_RD_NEXT_STRIDE_0_REG + (KLT_REG_OFFSET * i),
							rd_cfg->rd_next_line_stride[i].u32);
	}
}

static int klt_prev_kp_coord_config(klt_dev_t *dev,
	klt_prev_kp_coord_cfg_t *coord_cfg, unsigned int klt_cascade_en)
{
	int ret = 0;
	// cfg x and y;
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	macro_cmdlst_set_reg_burst_data_align(CFG_TAB_KLT_MAX_KP_NUM * 2, CVDR_ALIGN_BYTES);

	ret = macro_cmdlst_set_reg_burst(dev->base_addr + KLT_KP_PREV_BUFF_X_0_REG, CFG_TAB_KLT_MAX_KP_NUM * 2, 0,
						(unsigned int*)&(coord_cfg->kp_prev_xy[0]));
	if (ret) {
		loge("Failed: macro_cmdlst_set_reg_burst");
		return ISP_IPP_ERR;
	}

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	return ISP_IPP_OK;
}

static int klt_drv_para_check(cfg_tab_klt_t *cfg_tab_klt)
{
	klt_fwd_search_cfg_t *fwd_search = &cfg_tab_klt->klt_klt_cfg.fwd_search;
	klt_bwd_search_cfg_t *bwd_search = &cfg_tab_klt->klt_klt_cfg.bwd_search;

	if (fwd_search->bits.fwd_pyr_num < KLT_MIN_FWD_PYR_NUM ||
		fwd_search->bits.fwd_pyr_num > KLT_MAX_FWD_PYR_NUM) {
		loge("Failed: fwd_pyr_num is %d", fwd_search->bits.fwd_pyr_num);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int klt_do_ds_config(klt_dev_t *dev, klt_ds_cfg_t *ds_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + KLT_DS_CONFIG_REG, ds_cfg->u32);
	return ISP_IPP_OK;
}

static int klt_do_klt_config(klt_dev_t *dev, cfg_tab_klt_t *cfg_tab_klt)
{
	klt_klt_cfg_t *p_klt_klt_cfg = &(cfg_tab_klt->klt_klt_cfg);

	if (p_klt_klt_cfg->klt_start.bits.start_klt == 0) {
		logi("Cfg nothing, because the start_klt is 0");
		return ISP_IPP_OK;
	}
	if (klt_drv_para_check(cfg_tab_klt)) {
		loge("Failed: klt_drv_para_check");
		return ISP_IPP_ERR;
	}

	klt_fwd_search_config(dev, &(p_klt_klt_cfg->fwd_search));
	klt_bwd_search_config(dev, &(p_klt_klt_cfg->bwd_search));
	klt_iter_cfg_config(dev, &(p_klt_klt_cfg->iter_param));
	klt_eval_param_config(dev, &(p_klt_klt_cfg->eval_param));
	klt_prev_and_next_rd_config(dev, &(p_klt_klt_cfg->rd_cfg), p_klt_klt_cfg->fwd_search.bits.fwd_pyr_num);
	if (klt_prev_kp_coord_config(dev, &(p_klt_klt_cfg->prev_kp_coord), cfg_tab_klt->klt_cascade_en)) {
		loge("failed: do klt_prev_kp_coord_config");
		return ISP_IPP_ERR;
	}
	klt_prefetch_ctrl_config(dev, &(p_klt_klt_cfg->prefetch_ctrl));

	return ISP_IPP_OK;
}

int klt_klt_start_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_start_cfg_t *start_cfg)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(start_cfg, ret);
	if (ret) return ret;

	klt_dev_t klt_dev = *dev;
	if (start_cfg->bits.start_klt == 0) {
		loge("Failed: start_klt is 0, the klt is not configured");
		return ISP_IPP_ERR;
	}

	klt_dev.cmd_buf = cmd_buf;
	klt_start_config(&klt_dev, start_cfg);
	return ISP_IPP_OK;
}

int klt_prepare_ds_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, klt_ds_cfg_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	klt_dev_t klt_dev = *dev;
	klt_dev.cmd_buf = cmd_buf;
	if (klt_do_ds_config(&klt_dev, table)) {
		loge("Failed: klt_do_ds_config");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

int klt_prepare_klt_cmd(klt_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_klt_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	klt_dev_t klt_dev = *dev;
	klt_dev.cmd_buf = cmd_buf;
	if (klt_do_klt_config(&klt_dev, table)) {
		loge("Failed: klt_do_klt_config");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static klt_ops_t klt_ops = {
	.prepare_klt_ds_cmd  = klt_prepare_ds_cmd,
	.prepare_klt_klt_cmd = klt_prepare_klt_cmd,
	.klt_start           = klt_klt_start_cmd,
};

klt_dev_t g_klt_devs[KLT_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_KLT,
		.ops = &klt_ops,
	},
};

/* ********************************* END ********************************* */
