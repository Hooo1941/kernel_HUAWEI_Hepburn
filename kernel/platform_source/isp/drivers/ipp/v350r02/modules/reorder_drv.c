/*******************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    reorder_drv.c
 * Description:
 *
 * Date        2022-07-04
 ******************************************************************/
#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "reorder_drv.h"
#include "reorder_drv_priv.h"
#include "reorder_reg_offset.h"
#include "reorder_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_REORDER_DRV

static void reorder_ctrl_config(reorder_dev_t *dev, rdr_ctrl_cfg_t  *ctrl)
{
	macro_cmdlst_set_reg(dev->base_addr + REORDER_REORDER_CFG_REG, ctrl->u32);
}

static void reorder_block_config(reorder_dev_t *dev, rdr_block_cfg_t *block_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + REORDER_BLOCK_CFG_REG, block_cfg->u32);
}

static void reorder_set_total_kpts(reorder_dev_t *dev, cfg_tab_reorder_t *tab_reorder)
{
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	if (tab_reorder->rdr_cascade_en)
		tab_reorder->rdr_total_num_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
	logd("rdr_total_num_addr = 0x%x", tab_reorder->rdr_total_num_addr);

	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
	macro_cmdlst_set_reg_data(CMDLST_PADDING_DATA);
	macro_cmdlst_set_reg(dev->base_addr + REORDER_TOTAL_KPT_NUM_REG, tab_reorder->total_kptnum_cfg.u32);
}

static void reorder_prefetch_config(reorder_dev_t *dev, rdr_prefetch_cfg_t *prefetch_cfg)
{
	prefetch_cfg->bits.prefetch_enable = 0;
	macro_cmdlst_set_reg(dev->base_addr + REORDER_PREFETCH_CFG_REG, prefetch_cfg->u32);
}

static void reorder_kpt_num_config(reorder_dev_t *dev, cfg_tab_reorder_t *tab_reorder)
{
	unsigned int i = 0;

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
	macro_cmdlst_set_reg_burst_data_align(RDR_KPT_NUM_RANGE, CVDR_ALIGN_BYTES);
	if (tab_reorder->rdr_cascade_en) {
		tab_reorder->rdr_kpt_num_addr = dev->cmd_buf->start_addr_isp_map + dev->cmd_buf->data_size;
		tab_reorder->rdr_kpt_num_addr += 0x4; // one burst cmd length
		logd("rdr_kpt_num_addr = 0x%x", tab_reorder->rdr_kpt_num_addr);
	}

	macro_cmdlst_set_reg_incr(dev->base_addr + REORDER_KPT_NUMBER_0_REG, RDR_KPT_NUM_RANGE, INCR_MODE, WR_OPS_MODE);
	for (i = 0; i < RDR_KPT_NUM_RANGE; i++)
		macro_cmdlst_set_reg_data(tab_reorder->rdr_kptnum_cfg[i].u32);
	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
}

static int reorder_drv_para_check(cfg_tab_reorder_t *tab_reorder)
{
	if (tab_reorder->ctrl_cfg.bits.descriptor_type != 0) { // 0: SIFT(AR_FEATURE)
		loge("Failed: descriptor_type = %d, Please check", tab_reorder->ctrl_cfg.bits.descriptor_type);
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static int reorder_do_config(reorder_dev_t *dev, cfg_tab_reorder_t *tab_reorder)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(tab_reorder, ret);
	if (ret) return ret;

	if (reorder_drv_para_check(tab_reorder)) {
		loge("Failed: reorder_drv_para_check");
		return ISP_IPP_ERR;
	}

	reorder_block_config(dev, &(tab_reorder->block_cfg));
	reorder_set_total_kpts(dev, tab_reorder);
	reorder_prefetch_config(dev, &(tab_reorder->prefetch_cfg));
	reorder_kpt_num_config(dev, tab_reorder);
	reorder_ctrl_config(dev, &(tab_reorder->ctrl_cfg));

	return ISP_IPP_OK;
}

int reorder_prepare_cmd(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	reorder_dev_t rdr_dev = *dev;
	rdr_dev.cmd_buf = cmd_buf;
	if (reorder_do_config(&rdr_dev, table)) {
		loge("Failed: reorder_do_config");
		return ISP_IPP_ERR;
	}
	return ISP_IPP_OK;
}

static reorder_ops_t reorder_ops = {
	.prepare_cmd = reorder_prepare_cmd,
};

reorder_dev_t g_reorder_devs[REORDER_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_REORDER,
		.ops = &reorder_ops,
	},
};

/* ********************************* END ********************************* */
