/******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name  :
 * Description:
 *
 * Date         2020-04-17 16:28:10
 ********************************************************************/

#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "compare_drv.h"
#include "compare_drv_priv.h"
#include "compare_reg_offset.h"
#include "compare_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_COMPARE_DRV
#define MC_MAX3_RATIO_MIN   (1)
#define MC_MAX3_RATIO_MAX   (31)
#define MC_BIN_FACTOR_MIN   (2)
#define MC_BIN_FACTOR_MAX   (170)
#define MC_BIN_NUM_MIN      (0)
#define MC_BIN_NUM_MAX      (59)

static void compare_ctrl_config(compare_dev_t *dev, cmp_ctrl_cfg_t *ctrl)
{
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_COMPARE_CFG_REG, ctrl->u32);
}

static void compare_block_config(compare_dev_t *dev, cmp_block_cfg_t *block_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_BLOCK_CFG_REG, block_cfg->u32);
}

static void compare_search_config(compare_dev_t *dev, cmp_search_cfg_t *search_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_SEARCH_CFG_REG, search_cfg->u32);
}

static void compare_offset_config(compare_dev_t *dev, cmp_offset_cfg_t *offset_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_OFFSET_CFG_REG, offset_cfg->u32);
}

static void compare_stat_config(compare_dev_t *dev, cmp_stat_cfg_t *stat_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_STAT_CFG_REG, stat_cfg->u32);
}

static void compare_prefetch_config(compare_dev_t *dev, cmp_prefetch_cfg_t *prefetch_cfg)
{
	prefetch_cfg->bits.prefetch_enable = 0; // no prefetch
	macro_cmdlst_set_reg(dev->base_addr + COMPARE_PREFETCH_CFG_REG, prefetch_cfg->u32);
}

static void compare_kpt_num_config(compare_dev_t *dev, cfg_tab_compare_t *tab_compare)
{
	unsigned int i = 0;

	// 1. cfg ref
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	macro_cmdlst_set_reg_burst_data_align(CMP_KPT_NUM_RANGE, CVDR_ALIGN_BYTES);
	if (tab_compare->cmp_cascade_en) {
		tab_compare->cmp_ref_kpt_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
		tab_compare->cmp_ref_kpt_addr += 0x4; // one burst cmd length
		logd("cmp_ref_kpt_addr = 0x%x", tab_compare->cmp_ref_kpt_addr);
	}

	macro_cmdlst_set_reg_incr(dev->base_addr + COMPARE_REF_KPT_NUMBER_0_REG,
								CMP_KPT_NUM_RANGE, INCR_MODE, WR_OPS_MODE);
	for (i = 0; i < CMP_KPT_NUM_RANGE; i++)
		macro_cmdlst_set_reg_data(tab_compare->kptnum_cfg.ref_kpt_num[i].u32);

	// 2. cfg cur
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	macro_cmdlst_set_reg_burst_data_align(CMP_KPT_NUM_RANGE, CVDR_ALIGN_BYTES);
	if (tab_compare->cmp_cascade_en) {
		tab_compare->cmp_cur_kpt_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
		tab_compare->cmp_cur_kpt_addr += 0x4; // one burst cmd length;
		logd("cmp_cur_kpt_addr = 0x%x", tab_compare->cmp_cur_kpt_addr);
	}

	macro_cmdlst_set_reg_incr(dev->base_addr + COMPARE_CUR_KPT_NUMBER_0_REG,
								CMP_KPT_NUM_RANGE, INCR_MODE, WR_OPS_MODE);
	for (i = 0; i < CMP_KPT_NUM_RANGE; i++)
		macro_cmdlst_set_reg_data(tab_compare->kptnum_cfg.cur_kpt_num[i].u32);

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
}

static void compare_stat_total_kptnum(compare_dev_t *dev, cfg_tab_compare_t *tab_compare)
{
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	if (tab_compare->cmp_cascade_en)
		tab_compare->cmp_total_kpt_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
	logd("cmp_total_kpt_addr = 0x%x", tab_compare->cmp_total_kpt_addr);
	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);

	macro_cmdlst_set_reg(dev->base_addr + COMPARE_TOTAL_KPT_NUM_REG, tab_compare->total_kptnum_cfg.u32);
}

static int compare_drv_para_check(cfg_tab_compare_t *tab_compare)
{
	cmp_stat_cfg_t *stat_cfg = &tab_compare->stat_cfg;

	if (stat_cfg->bits.max3_ratio < MC_MAX3_RATIO_MIN) {
		loge("para is error: max3_ratio = %d", stat_cfg->bits.max3_ratio);
		return ISP_IPP_ERR;
	}

	if (stat_cfg->bits.bin_factor < MC_BIN_FACTOR_MIN || stat_cfg->bits.bin_factor > MC_BIN_FACTOR_MAX) {
		loge("para is error: bin_factor = %d", stat_cfg->bits.bin_factor);
		return ISP_IPP_ERR;
	}

	if (stat_cfg->bits.bin_num > MC_BIN_NUM_MAX) {
		loge("para is error: bin_num = %d", stat_cfg->bits.bin_num);
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static int compare_do_config(compare_dev_t *dev, cfg_tab_compare_t *tab_compare)
{
	if (dev == NULL || tab_compare == NULL) {
		loge("params is NULL!!");
		return ISP_IPP_ERR;
	}

	if (compare_drv_para_check(tab_compare)) {
		loge("Failed : compare_drv_para_check");
		return ISP_IPP_ERR;
	}

	compare_block_config(dev, &(tab_compare->blk_cfg));
	compare_search_config(dev, &(tab_compare->search_cfg));
	compare_offset_config(dev, &(tab_compare->offset_cfg));
	compare_stat_config(dev, &(tab_compare->stat_cfg));
	compare_stat_total_kptnum(dev, tab_compare);
	compare_prefetch_config(dev, &(tab_compare->prefetch_cfg));
	compare_kpt_num_config(dev, tab_compare);

	// Note: cmp_en should be configure at the last sequence
	compare_ctrl_config(dev, &(tab_compare->ctrl_cfg));

	return ISP_IPP_OK;
}

int compare_prepare_cmd(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table)
{
	if (dev == NULL || cmd_buf == NULL || table == NULL) {
		loge("compare_prepare_cmd params is NULL!!");
		return ISP_IPP_ERR;
	}

	compare_dev_t cmp_dev = *dev;
	cmp_dev.cmd_buf = cmd_buf;
	if (compare_do_config(&cmp_dev, table)) {
		loge("Failed: compare_do_config");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static compare_ops_t compare_ops = {
	.prepare_cmd = compare_prepare_cmd,
};

compare_dev_t g_compare_devs[COMPARE_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_COMPARE,
		.ops = &compare_ops,
	},
};

/* ********************************* END ********************************* */
