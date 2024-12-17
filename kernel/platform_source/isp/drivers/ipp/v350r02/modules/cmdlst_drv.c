/* *****************************************************************
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2018. All rights reserved.
 * File name    cmdlst_drv.c
 * Description:
 *
 * Date         2022-07-06 16:28:10
 *******************************************************************/

#define LOG_TAG LOG_MODULE_CMDLST_DRV
#include "linux/printk.h"
#include "cmdlst_manager.h"
#include "cmdlst_drv.h"
#include "cmdlst_drv_priv.h"
#include "cmdlst_reg_offset.h"
#include "cvdr_ipp_reg_offset.h"


// -----------------------------start 1
// p1 Cfg: sw_resource/sw_priority
static int cmdlst_set_sw_resource(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *cfg)
{
	unsigned int ch_offset = CH_CFG_OFFSET * cfg->channel_id;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_RESOURCES_0_REG + ch_offset, cfg->sw_resource_cfg.u32);
	return ISP_IPP_OK;
}

// p2 Cfg: sw_task
static int cmdlst_set_sw_task(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *cfg)
{
	unsigned int ch_offset = CH_CFG_OFFSET * cfg->channel_id;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_TASK_0_REG + ch_offset, cfg->sw_task_cfg.u32);
	return ISP_IPP_OK;
}

// p3 Cfg: max_rd_burst_length/ start_addr/ end_addr/ vprd_prefetch_bypass_0
static int cmdlst_set_common_cfg(cmdlst_dev_t *dev)
{
	cmdlst_burst_cfg_t burst_cfg = {0};
	burst_cfg.bits.max_rd_burst_length = 0; // r02-ipp only can cfg 0;

	hispcpe_reg_set(CVDR_REG,    CVDR_IPP_CVDR_IPP_VP_RD_IF_CFG_0_REG,
					(((unsigned int)1U) << 31U)); // vprd_prefetch_bypass_0 = 1
	hispcpe_reg_set(CMDLIST_REG, CMDLST_CFG_REG,        burst_cfg.u32);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_START_ADDR_REG, JPG_CMDLST_OFFSET);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_END_ADDR_REG,   JPG_CMDLST_OFFSET + CMDLST_OFFSET_ADDR);

	return ISP_IPP_OK;
}

// p3 Cfg: vp_rd
static int cmdlst_set_vp_rd(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *cfg)
{
	unsigned int ch_offset = CH_CFG_OFFSET * cfg->channel_id;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_ADDR_0_REG + ch_offset,   cfg->sw_cvdr_rd_addr.u32);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_0_0_REG + ch_offset, cfg->sw_cvdr_rd_data0.u32);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_1_0_REG + ch_offset, cfg->sw_cvdr_rd_data1.u32);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_2_0_REG + ch_offset, cfg->sw_cvdr_rd_data2.u32);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_3_0_REG + ch_offset, cfg->sw_cvdr_rd_data3.u32);

	return ISP_IPP_OK;
}

int cmdlst_do_config(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *config_table)
{
	loge_if(cmdlst_set_sw_resource(dev, config_table));
	loge_if(cmdlst_set_sw_task(dev, config_table));
	loge_if(cmdlst_set_common_cfg(dev));
	loge_if(cmdlst_set_vp_rd(dev, config_table));
	return ISP_IPP_OK;
}

// -----------------------------end   1
static int cmdlst_set_ch_cfg(cmdlst_dev_t *dev, cmdlst_ch_cfg_t *cfg, unsigned int channel_id)
{
	unsigned int ch_offset = CH_CFG_OFFSET * channel_id;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + ch_offset, cfg->u32);
	return ISP_IPP_OK;
}

int cmdlst_get_debug_info(cmdlst_dev_t *dev, cmdlst_debug_info_t *st)
{
	unsigned int ch_offset = CH_CFG_OFFSET * st->channel_id;
	st->fsm_info  = hispcpe_reg_get(CMDLIST_REG, CMDLST_DEBUG_CHANNEL_0_REG + ch_offset);
	st->last_exec = hispcpe_reg_get(CMDLIST_REG, CMDLST_LAST_EXEC_PTR_0_REG + ch_offset);
	return ISP_IPP_OK;
}

int cmdlst_set_vp_wr_flush(cmdlst_dev_t *dev, cmd_buf_t *cmd_buf, unsigned int channel_id)
{
	dev->cmd_buf = cmd_buf;
	macro_cmdlst_set_reg(dev->base_addr + CMDLST_VP_WR_FLUSH_0_REG + CH_CFG_OFFSET * channel_id, 0x00000001);
	return ISP_IPP_OK;
}

static cmdlst_ops_t cmdlst_ops = {
	.do_config       = cmdlst_do_config,
	.set_sw_ch       = cmdlst_set_ch_cfg,
	.set_sw_resource = cmdlst_set_sw_resource,
	.set_vp_rd       = cmdlst_set_vp_rd,
	.get_debug_info  = cmdlst_get_debug_info,
};

cmdlst_dev_t g_cmdlst_devs[CMDLST_DEV_COUNT] = {
	[0] = {
		.base_addr = IPP_BASE_ADDR_CMDLST,
		.ops = &cmdlst_ops,
	},
};

/* ********************************* END ********************************* */
