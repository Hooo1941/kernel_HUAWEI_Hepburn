/*********************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    arfeature_drv.c
 * Description:
 *
 * Date         2022-07-02 20:37:22
 ***********************************************************************/
#include <linux/printk.h>
#include "ipp.h"
#include "arfeature_drv.h"
#include "arfeature_drv_priv.h"
#include "arfeature_reg_offset.h"
#include "arfeature_reg_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_ARFEATURE_DRV
#define REG_ADDR_LEN 4

// arfeature
static void arfeature_set_ctrl(arfeature_dev_t *dev, arf_top_cfg_t *top_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_TOP_CFG_REG, top_cfg->u32);
}

static void arfeature_set_img_size(arfeature_dev_t *dev, arf_img_size_t *size_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_IMAGE_SIZE_REG, size_cfg->u32);
}

static void arfeature_set_block(arfeature_dev_t *dev, arf_blk_cfg_t *blk_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_BLOCK_NUM_CFG_REG, blk_cfg->blk_num.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_BLOCK_SIZE_CFG_INV_REG, blk_cfg->blk_size.u32);
}

static void arfeature_set_detect_number_lmt(arfeature_dev_t *dev, arf_detect_num_lmt_t *detect_num_lmt)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_DETECT_NUMBER_LIMIT_REG, detect_num_lmt->u32);
}

static void arfeature_set_sigma_coeff(arfeature_dev_t *dev, arf_sigma_coef_t *sigma_coeff_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_SIGMA_COEF_REG, sigma_coeff_cfg->u32);
}

static void arfeature_set_gauss_coeff(arfeature_dev_t *dev, arf_gauss_coeff_t *gauss_coeff_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_GAUSS_ORG_REG, gauss_coeff_cfg->gauss_org.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_GSBLUR_1ST_REG, gauss_coeff_cfg->gsblur_1st.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_GAUSS_2ND_REG, gauss_coeff_cfg->gauss_2nd.u32);
}

static void arfeature_set_dog_edge_thd(arfeature_dev_t *dev, arf_dog_edge_thd_t *edge_thd)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_DOG_EDGE_THRESHOLD_REG, edge_thd->u32);
}

static void arfeature_set_descriptor_thd(arfeature_dev_t *dev, arf_descriptor_thd_t *mag_thd_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_DESCRIPTOR_THRESHOLD_REG, mag_thd_cfg->u32);
}

static void arfeature_set_kltpre_cfg(arfeature_dev_t *dev, arf_klt_pre_cfg_t *kltpre_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_KLT_PRE_BLOCK_SIZE_CFG_INV_REG, kltpre_cfg->blk_inv.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_KLT_PRE_BLOCK_CFG_REG,kltpre_cfg->blk_num.u32);
}

static void arfeature_set_mfpda_cfg(arfeature_dev_t *dev, arf_mfpda_para_cfg_t *mfpda_para)
{
	unsigned int i;
	for (i = 0; i < ARFEATURE_MFPDA_CFG_RANGE; i++) {
		macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_MFPDA_CFG_0_REG + (i * REG_ADDR_LEN),
								mfpda_para->mfpda_cfg[i].u32);
		macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_BLOCK_MFPDA_CFG_0_REG + (i * REG_ADDR_LEN),
								mfpda_para->blk_mfpda_cfg[i].u32);
	}
}

static void arfeature_set_roi_window_cfg(arfeature_dev_t *dev, arf_roi_window_cfg_t *roi_window_cfg)
{
	unsigned int i;
	for (i = 0; i < ARFEATURE_MAX_ROI_WINDOW_NUM; i++) {
		macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_ROI_WINDOW_START_CFG_0_REG + (i * REG_ADDR_LEN),
								roi_window_cfg->window_start[i].u32);
		macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_ROI_WINDOW_END_CFG_0_REG + (i * REG_ADDR_LEN),
								roi_window_cfg->window_end[i].u32);
	}

	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_ROI_WINDOW_MODE_CFG_REG, roi_window_cfg->window_mode.u32);
}

static void arfeature_set_kpt_lmt(arfeature_dev_t *dev, arf_kpt_limit_t *kpt_lmt_cfg)
{
	unsigned int i = 0, j = 0, cnt = 0;
	unsigned int data_size = ARFEATURE_MAX_BLK_NUM;
	unsigned int reg_size = 0;
	unsigned int index_cnt = 0;

	cnt = (data_size / CMDLST_BURST_MAX_SIZE);
	if ((data_size % CMDLST_BURST_MAX_SIZE) != 0)
		cnt = cnt + 1;

	for (i = 0; i < cnt; i++) {
		reg_size = ((data_size - i * CMDLST_BURST_MAX_SIZE) > CMDLST_BURST_MAX_SIZE) ?
					CMDLST_BURST_MAX_SIZE :
					(data_size - i * CMDLST_BURST_MAX_SIZE);

		macro_cmdlst_set_reg_incr(dev->base_addr + AR_FEATURE_KPT_LIMIT_0_REG +
									i * CMDLST_BURST_MAX_SIZE * REG_ADDR_LEN,
									reg_size, INCR_MODE, WR_OPS_MODE);

		for (j = 0; j < reg_size; j++)
			macro_cmdlst_set_reg_data(kpt_lmt_cfg[index_cnt++].u32);
	}

	macro_cmdlst_set_addr_align(CVDR_ALIGN_BYTES);
}

static void arfeature_set_cvdr_rd(arfeature_dev_t *dev, arf_cvdr_rd_cfg_para_t *rd_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr  + AR_FEATURE_CVDR_RD_CFG_REG, rd_cfg->rd_base_cfg.u32);
	macro_cmdlst_set_reg(dev->base_addr  + AR_FEATURE_CVDR_RD_LWG_REG, rd_cfg->rd_lwg.u32);
	macro_cmdlst_set_reg(dev->base_addr  + AR_FEATURE_PRE_CVDR_RD_FHG_REG, rd_cfg->pre_rd_fhg.u32);
	macro_cmdlst_set_reg(dev->base_addr  + AR_FEATURE_CVDR_RD_AXI_FS_REG, rd_cfg->rd_axi_fs.u32);
	macro_cmdlst_set_reg(dev->base_addr  + AR_FEATURE_CVDR_RD_AXI_LINE_REG, rd_cfg->rd_axi_line.u32);
}

static void arfeature_set_cvdr_wr(arfeature_dev_t *dev, arf_cvdr_wr_cfg_para_t *wr_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_CVDR_WR_CFG_REG, wr_cfg->wr_base_cfg.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_PRE_CVDR_WR_AXI_FS_REG, wr_cfg->pre_wr_axi_fs.u32);
	macro_cmdlst_set_reg(dev->base_addr + AR_FEATURE_CVDR_WR_AXI_LINE_REG, wr_cfg->wr_axi_line.u32);
}

static int arfeature_do_config(arfeature_dev_t *dev, cfg_tab_arf_t *table)
{
	arfeature_set_img_size(dev, &(table->img_size));
	arfeature_set_block(dev, &(table->blk_cfg));
	arfeature_set_detect_number_lmt(dev, &(table->detect_num_lmt));
	arfeature_set_sigma_coeff(dev, &(table->sigma_coeff_cfg));
	arfeature_set_gauss_coeff(dev, &(table->gauss_coeff_cfg));
	arfeature_set_dog_edge_thd(dev, &(table->dog_edge_thd_cfg));
	arfeature_set_descriptor_thd(dev, &(table->mag_thd_cfg));
	arfeature_set_mfpda_cfg(dev, &(table->mfpda_para));
	arfeature_set_roi_window_cfg(dev, &(table->roi_window_cfg));

	if (table->top_cfg.bits.mode >= ARF_DETECTION_MODE)
		arfeature_set_kpt_lmt(dev, &(table->kpt_lmt_cfg[0]));

	arfeature_set_cvdr_rd(dev, &(table->cvdr_cfg.rd_cfg));
	arfeature_set_cvdr_wr(dev, &(table->cvdr_cfg.wr_cfg));
	arfeature_set_kltpre_cfg(dev,&(table->kptpre_cfg));
	arfeature_set_ctrl(dev, &(table->top_cfg));

	return ISP_IPP_OK;
}

int arfeature_prepare_cmd(arfeature_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_arf_t *table)
{
	int ret = 0;
	arfeature_dev_t arf_dev = *dev;

	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	arf_dev.cmd_buf = cmd_buf;
	if (arfeature_do_config(&arf_dev, table)) {
		loge("Failed : arfeature_do_config");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static arfeature_ops_t arfeature_ops = {
	.prepare_cmd = arfeature_prepare_cmd,
};

arfeature_dev_t g_arfeature_devs[ARFEATURE_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_ARF,
		.ops = &arfeature_ops,
	},
};

/* ********************************* END ********************************* */

