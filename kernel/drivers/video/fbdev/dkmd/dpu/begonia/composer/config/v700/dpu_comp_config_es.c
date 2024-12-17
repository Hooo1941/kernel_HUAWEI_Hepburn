/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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

#include "dkmd_log.h"
#include "dkmd_object.h"
#include "cmdlist_interface.h"
#include "dpu_comp_config_utils.h"
#include "dpu_panel_partial_ctl.h"
#include <dpu/dpu_lbuf.h>

struct swid_info g_sdma_swid_tlb_info[SDMA_SWID_NUM] = {
	{DPU_SCENE_ONLINE_0,   0,  19},
	{DPU_SCENE_ONLINE_1,  20,  39},
	{DPU_SCENE_ONLINE_2,  40,  47},
	{DPU_SCENE_ONLINE_3,  48,  55},
	{DPU_SCENE_OFFLINE_0, 60,  67},
	{DPU_SCENE_OFFLINE_1, 68,  75},
	{DPU_SCENE_OFFLINE_2, 76,  76},
};

struct swid_info g_wch_swid_tlb_info[WCH_SWID_NUM] = {
	{DPU_SCENE_ONLINE_0,  57,  59},
	{DPU_SCENE_OFFLINE_0, 77,  79},
};

void dpu_comp_wch_axi_sel_set_reg(char __iomem *dbcu_base)
{
	void_unused(dbcu_base);
}

char __iomem *dpu_comp_get_tbu1_base(char __iomem *dpu_base)
{
	if (dpu_base == NULL) {
		dpu_pr_err("dpu_base is nullptr!\n");
		return NULL;
	}

	return dpu_base + DPU_SMMU1_OFFSET;
}

void dpu_level1_clock_lp(uint32_t cmdlist_id)
{
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00013100, 0x00000000); // DPU_GLB_MODULE_CLK_SEL_ADDR
}

void dpu_level2_clock_lp(uint32_t cmdlist_id)
{
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00012740, 0x00000000); // DPU_CMD_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000150E4, 0x00000000); // WCH0 DPU_WCH_CH_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0006D0E4, 0x00000000); // WCH1 DPU_WCH_CH_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000710E4, 0x00000000); // WCH2 DPU_WCH_CH_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00019788, 0x00000000); // DBCU DPU_DBCU_CLK_SEL_0_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001978C, 0x00000000); // DBCU DPU_DBCU_CLK_SEL_1_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001B000, 0x00000000); // LBUF DPU_LBUF_CTL_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001B008, 0x00000000); // LBUF DPU_LBUF_PART_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CD008, 0x00000002); // ARSR0 DPU_ARSR_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00078008, 0x00000002); // ARSR1 DPU_ARSR_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0007F008, 0x00000002); // VSCF0 DPU_VSCF_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00083008, 0x00000002); // VSCF1 DPU_VSCF_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00087008, 0x00000002); // HDR DPU_HDR_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0008D008, 0x00000000); // CLD0 DPU_CLD_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0008E008, 0x00000000); // CLD1 DPU_CLD_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00075010, 0x00000000); // HEMCD0 DPU_HEMCD_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00076010, 0x00000000); // HEMCD1 DPU_HEMCD_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C000, 0x00000000); // RCH_OV0 DPU_RCH_OV_RCH_OV0_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C008, 0x00000000); // RCH_OV1 DPU_RCH_OV_RCH_OV1_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C010, 0x00000000); // RCH_OV2 DPU_RCH_OV_RCH_OV2_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C018, 0x00000000); // RCH_OV3 DPU_RCH_OV_RCH_OV3_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C020, 0x00000000); // RCH_OV DPU_RCH_OV_WRAP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B4008, 0x00000000); // DPP0 DPU_DPP_CH_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B0008, 0x00000000); // DDIC0 DPU_DDIC_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000D407C, 0x00000000); // DSC0 DPU_DSC_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000D4248, 0x00000000); // DSC0 DPU_DSC_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00013124, 0x00000000); // GLB DPU_GLB_DISP_CH0_MODULE_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001312C, 0x00000000); // GLB DPU_GLB_DISP_CH1_MODULE_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00013134, 0x00000000); // GLB DPU_GLB_DISP_AVHR_MODULE_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001313C, 0x00000000); // GLB DPU_GLB_DISP_MODULE_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00040400, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00040800, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00040C00, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00041000, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00041400, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00041800, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00041C00, 0x00000000); // DACC_CFG SOC_DACC_CLK_SEL_REG_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CC070, 0x00000001); // ITF_CH0 DPU_ITF_CH_ITFCH_CLK_CTL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CC170, 0x00000001); // ITF_CH1 DPU_ITF_CH_ITFCH_CLK_CTL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CC270, 0x00000001); // ITF_CH2 DPU_ITF_CH_ITFCH_CLK_CTL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CC370, 0x00000001); // ITF_CH3 DPU_ITF_CH_ITFCH_CLK_CTL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CC4F4, 0x00000000); // PIPE_SW DPU_PIPE_SW_CLK_SEL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000136A0, 0x00000000); // GLB DPU_GLB_REG_DVFS_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00013710, 0x00000000); // GLB DPU_GLB_DBG_CTRL1_ADDR
}

void dpu_ch1_level2_clock_lp(uint32_t cmdlist_id)
{
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD008, 0x00000000); // DPP1 DPU_DPP_CH_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000D9008, 0x00000000); // DDIC1 DPU_DDIC_TOP_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000FE07C, 0x00000000); // DSC1 DPU_DSC_CLK_SEL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000FE248, 0x00000000); // DSC1 DPU_DSC_TOP_CLK_SEL_ADDR
}

void dpu_memory_lp(uint32_t cmdlist_id)
{
	void_unused(cmdlist_id);
}

void dpu_memory_no_lp(uint32_t cmdlist_id)
{
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001273C, 0x00000008); // CMD DPU_CMD_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001503C, 0x88888888); // WCH0 DPU_WCH_DMA_CMP_MEM_CTRL0_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00015040, 0x88888888); // WCH0 DPU_WCH_DMA_CMP_MEM_CTRL1_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00015218, 0x00000088); // WCH0 DPU_WCH_SCF_COEF_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0006D03C, 0x88888888); // WCH1 DPU_WCH_DMA_CMP_MEM_CTRL0_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0006D040, 0x88888888); // WCH1 DPU_WCH_DMA_CMP_MEM_CTRL1_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0006D218, 0x00000088); // WCH1 DPU_WCH_SCF_COEF_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0007103C, 0x88888888); // WCH2 DPU_WCH_DMA_CMP_MEM_CTRL0_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00071040, 0x88888888); // WCH2 DPU_WCH_DMA_CMP_MEM_CTRL1_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00071218, 0x00000088); // WCH2 DPU_WCH_SCF_COEF_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001B010, 0x00000000); // LBUF DPU_LBUF_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CD218, 0x00000088); // ARSR0 DPU_ARSR_SCF_COEF_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CD290, 0x00000008); // ARSR0 DPU_ARSR_SCF_LB_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CD3E8, 0x00000008); // ARSR0 DPU_ARSR_ARSR2P_LB_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000CD3EC, 0x00000088); // ARSR0 DPU_ARSR_COEF_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00078218, 0x00000088); // ARSR1 DPU_ARSR_SCF_COEF_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00078290, 0x00000008); // ARSR1 DPU_ARSR_SCF_LB_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000783E8, 0x00000008); // ARSR1 DPU_ARSR_ARSR2P_LB_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000783EC, 0x00000088); // ARSR1 DPU_ARSR_COEF_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0007F290, 0x00000008); // VSCF0 DPU_VSCF_SCF_LB_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00083290, 0x00000008); // VSCF1 DPU_VSCF_SCF_LB_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00087404, 0x00000008); // HDR DPU_HDR_DEGAMMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00087418, 0x00000008); // HDR DPU_HDR_GMP_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00087434, 0x00000008); // HDR DPU_HDR_GAMMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x00075008, 0x00888888); // HEMCD0 DPU_HEMCD_MEM_CTRL_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C060, 0x00000008); // RCH_OV0 DPU_RCH_OV_OV_MEM_CTRL_0_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C064, 0x00000008); // RCH_OV1 DPU_RCH_OV_OV_MEM_CTRL_1_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C068, 0x00000008); // RCH_OV2 DPU_RCH_OV_OV_MEM_CTRL_2_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x0001C06C, 0x00000008); // RCH_OV3 DPU_RCH_OV_OV_MEM_CTRL_3_ADDR

	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B4500, 0x00000008); // DPP0 DPU_DPP_SPR_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B4704, 0x00000008); // DPP0 DPU_DPP_DEGAMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B4804, 0x00000008); // DPP0 DPU_DPP_ROIGAMA0_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B4904, 0x00000008); // DPP0 DPU_DPP_ROIGAMA1_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B49A4, 0x00000008); // DPP0 DPU_DPP_GMP_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000B58A4, 0x00000008); // DPP0 DPU_DPP_SPR_GAMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000D4084, 0x88888888); // DSC0 DPU_DSC_MEM_CTRL_ADDR
}

void dpu_ch1_memory_no_lp(uint32_t cmdlist_id)
{
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD500, 0x00000008); // DPP1 DPU_DPP_SPR_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD704, 0x00000008); // DPP1 DPU_DPP_DEGAMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD804, 0x00000008); // DPP1 DPU_DPP_DEGAMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD904, 0x00000008); // DPP1 DPU_DPP_ROIGAMA1_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD9A4, 0x00000008); // DPP1 DPU_DPP_GMP_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000DD8A4, 0x00000008); // DPP1 DPU_DPP_SPR_GAMA_MEM_CTRL_ADDR
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id,  0x000FE084, 0x88888888); // DSC1 DPU_DSC_MEM_CTRL_ADDR
}

void dpu_lbuf_init(uint32_t cmdlist_id, uint32_t xres, uint32_t yres, uint32_t dsc_en)
{
	void_unused(xres);
	void_unused(yres);
	void_unused(dsc_en);
	struct dpu_lbuf_ctl_reg lbuf_ctl_reg = {0};

	lbuf_ctl_reg.sour_auto_ctrl.reg.ov_auto_ctrl_en = 1;
	dkmd_set_reg(DPU_SCENE_INITAIL, cmdlist_id, 0x0001B104, lbuf_ctl_reg.sour_auto_ctrl.value);
}

/* set this bit to indicate LPMCU dpu power on, then LPMCU need check dpu lbuf before ddr dfs
 * clear this bit to indicate LPMCU dpu power off, then LPMCU needn't check dpu lbuf before ddr dfs
 */
void dpu_power_on_state_for_ddr_dfs(char __iomem *pctrl_base, bool is_power_on)
{
	void_unused(pctrl_base);
	void_unused(is_power_on);
}

void dpu_enable_m1_qic_intr(char __iomem *actrl_base)
{
	void_unused(actrl_base);
}

/* dynamic on and off are not supported */
bool dpu_enable_lp_flag(void)
{
	return true;
}

uint32_t dpu_get_ch_sid(void)
{
	uint32_t ch_sid = 1;
	return ch_sid;
}

void dpu_dacc_resume(char __iomem *dpu_base)
{
	void_unused(dpu_base);
}

void dpu_check_dbuf_state(char __iomem *dpu_base, uint32_t scene_id)
{
	void_unused(dpu_base);
	void_unused(scene_id);
}

bool dpu_is_smmu_bypass(void)
{
	return false;
}

ktime_t get_te_hw_timestamp_us(char __iomem *dpu_base, int32_t sw_itfch_idx)
{
	void_unused(dpu_base);
	void_unused(sw_itfch_idx);
	ktime_t timestamp;

	timestamp = ktime_get();

	return ktime_to_us(timestamp);
}

void dpu_comp_abnormal_dump_axi(char __iomem *dpu_base)
{
	void_unused(dpu_base);
}
void dpu_comp_abnormal_dump_qic(char __iomem *media1_ctrl_base)
{
	void_unused(media1_ctrl_base);
}

void dpu_comp_set_glb_rs_reg(struct dpu_composer *dpu_comp, bool enable)
{
	uint32_t clear_bit = 0;
	struct dkmd_connector_info *pinfo = NULL;

	if (unlikely(!dpu_comp || !dpu_comp->conn_info || !dpu_comp->comp_mgr || !dpu_comp->comp_mgr->dpu_base)) {
		dpu_pr_err("dpu_comp or conn_info or dpu_base is null");
		return;
	}

	if (!enable) {
		outp32(DPU_GLB_RS_CLEAR_ADDR(dpu_comp->comp_mgr->dpu_base + DPU_GLB0_OFFSET), 0);
		return;
	}

	pinfo = dpu_comp->conn_info;
	clear_bit = BIT(pinfo->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX]);
	if (pinfo->sw_post_chn_num > 1)
		clear_bit = clear_bit | BIT(pinfo->sw_post_chn_idx[EXTERNAL_CONNECT_CHN_IDX]);
	outp32(DPU_GLB_RS_CLEAR_ADDR(dpu_comp->comp_mgr->dpu_base + DPU_GLB0_OFFSET), clear_bit);

	dpu_pr_info("sw_post_chn_idx:%d, clear_bit:%#x", pinfo->sw_post_chn_idx[PRIMARY_CONNECT_CHN_IDX], clear_bit);
}

void dpu_ppc_set_1st_part_cmd_addr(char __iomem *dpu_base, struct panel_partial_ctrl *priv, uint32_t ppc_config_id)
{
	void_unused(dpu_base);
	void_unused(priv);
	void_unused(ppc_config_id);
}

void dpu_ppc_set_2nd_part_cmd_addr(char __iomem *dpu_base, struct panel_partial_ctrl *priv, uint32_t ppc_config_id)
{
	void_unused(dpu_base);
	void_unused(priv);
	void_unused(ppc_config_id);
}

void dpu_ppc_init_interactive_reg(char __iomem *dpu_base, struct panel_partial_ctrl *priv)
{
	void_unused(dpu_base);
	void_unused(priv);
}
