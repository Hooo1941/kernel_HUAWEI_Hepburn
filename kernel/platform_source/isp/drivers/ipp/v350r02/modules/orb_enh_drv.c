/*********************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    orb_enh_drv.c
 * Description:
 *
 * Date         2022-07-02 20:37:22
 ***********************************************************************/
#include <linux/printk.h>
#include "ipp.h"
#include "orb_enh_drv.h"
#include "orb_enh_drv_priv.h"
#include "orb_enh_reg_offset.h"
#include "orb_enh_offset_field.h"
#include "cmdlst_manager.h"

#define LOG_TAG LOG_MODULE_ORB_ENH_DRV

static void orb_enh_set_ctrl(orb_enh_dev_t  *dev, enh_enable_cfg_t *ctrl)
{
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_ENABLE_CFG_REG, ctrl->u32);
}

static void orb_enh_set_compute_hist(orb_enh_dev_t *dev, enh_cal_hist_cfg_t *cal_hist_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_BLK_NUM_REG, cal_hist_cfg->blk_num.u32);
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_BLK_SIZE_REG, cal_hist_cfg->blk_size.u32);
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_EXTEND_CFG_REG, cal_hist_cfg->extend_cfg.u32);
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_HIST_CLIMIT_REG, cal_hist_cfg->climit_cfg.u32);
}

static void orb_enh_set_adjust_hist(orb_enh_dev_t *dev, enh_adjust_hist_cfg_t *adjust_hist_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_LUT_SCALE_REG, adjust_hist_cfg->lutscale_cfg.u32);
}

static void orb_enh_set_adjust_image(orb_enh_dev_t *dev, enh_adjust_image_cfg_t *adjust_image_cfg)
{
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_INV_BLK_SIZE_X_REG, adjust_image_cfg->inv_blk_size_x.u32);
	macro_cmdlst_set_reg(dev->base_addr + ORB_ENH_INV_BLK_SIZE_Y_REG, adjust_image_cfg->inv_blk_size_y.u32);
}

static int orb_enh_do_config(orb_enh_dev_t *dev, cfg_tab_orb_enh_t *tab_orb_enh)
{
	orb_enh_set_adjust_hist(dev, &(tab_orb_enh->adjust_hist_cfg));
	orb_enh_set_compute_hist(dev, &(tab_orb_enh->cal_hist_cfg));
	orb_enh_set_adjust_image(dev, &(tab_orb_enh->adjust_image_cfg));
	orb_enh_set_ctrl(dev, &(tab_orb_enh->enable_cfg));

	return ISP_IPP_OK;
}

int orb_enh_prepare_cmd(orb_enh_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_orb_enh_t *table)
{
	int ret = 0;
	loge_if_null_set_ret(dev, ret);
	if (ret) return ret;
	loge_if_null_set_ret(cmd_buf, ret);
	if (ret) return ret;
	loge_if_null_set_ret(table, ret);
	if (ret) return ret;

	orb_enh_dev_t enh_dev = *dev;
	enh_dev.cmd_buf = cmd_buf;
	if (orb_enh_do_config(&enh_dev, table)) {
		loge("Failed: orb_enh_do_config");
		return ISP_IPP_ERR;
	}

	return ISP_IPP_OK;
}

static orb_enh_ops_t orb_enh_ops = {
	.prepare_cmd = orb_enh_prepare_cmd,
};

orb_enh_dev_t g_orb_enh_devs[ORB_ENH_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_ORB_ENH,
		.ops = &orb_enh_ops,
	},
};

/* ********************************end***************************** */
