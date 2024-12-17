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

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <securec.h>
#include "dpu/dpu_base_addr.h"
#include <soc_media1_crg_interface.h>
#include <dpu/soc_dpu_define.h>

#if defined(CONFIG_DRMDRIVER)
#include <platform_include/see/bl31_smc.h>
#include <platform_include/display/linux/dpu_drmdriver.h>
#endif
#include "dkmd_log.h"
#include "dkmd_object.h"
#include "dpu_config_utils.h"
#include "dpu_dvfs.h"
#include "dvfs.h"

#define SW_DVFS_OPT_NUM  8
#define HW_DVFS_OPT_NUM  32
#define MAX_INSTR_COUNT  40

#ifndef CONFIG_DVFS_RESERVE_TIME
#define DVFS_OV_RESERVE_TIME 64
#else
#define DVFS_OV_RESERVE_TIME CONFIG_DVFS_RESERVE_TIME
#endif

enum {
	OPT_VOLT_UP_INDEX = 0,
	OPT_VOLT_DOWN_INDEX = 1,
	OPT_VOLT_MAX_INDEX,
};

enum {
	INTRA_VOTE_TYPE,
	INTER_VOTE_TYPE,
};

struct intra_dvfs_record {
	uint32_t intra_prev_vote_level; // online previous frame vote perf level
	uint32_t intra_cur_vote_level; // online current frame vote perf level
	uint32_t inter_cur_vote_level; // offline current frame vote perf level
	uint32_t inter_effect_level; // actual effective level, when inter vote
	uint32_t last_vote_type; // 0 - INTRA_VOTE_TYPE; 1 - INTER_VOTE_TYPE
	uint32_t cur_frm_rate; // online cur frm rate
	uint32_t prev_frm_rate; // online prev frm rate
};

struct intra_dvfs_record g_dvfs_record;

typedef union {
	uint16_t value;
	/* data include END, WAIT_CNT, WAIT_CHG, VOL_CFG, CLK_SW, CLK_DIV, PLL_EN, PLL_GT, NOP */
	struct dvfs_instruction {
		uint32_t op     : 4;     /* bit[3:0]  : OP code for dvfs */
		uint32_t halt   : 1;     /* bit[4]    : halt flag for dvfs */
		uint32_t instr  : 11;    /* bit[15:5] : instruction code for dvfs */
	} cmd;
} dpu_dvfs_instr;

struct dvfs_vote_info {
	uint16_t vote_vol;
	uint16_t vote_clk_sel;
	uint16_t vote_clk_div;
};

static uint64_t s_dpu_core_rate_tbl_v110[] = {
	DPU_CORE_FREQ_OFF, /* L0_POWER_OFF */
	DPU_CORE_FREQ0, /* L1_060V */
	DPU_CORE_FREQ1, /* L2_065V */
	DPU_CORE_FREQ2, /* L3_070V */
	DPU_CORE_FREQ3  /* L4_080V */
};

uint32_t dpu_config_get_perf_level(uint64_t freq)
{
	uint32_t i;
	/* DPU_CORE_LEVEL_ON means L1_060V */
	for (i = DPU_CORE_LEVEL_ON; i <= DPU_PERF_LEVEL_MAX; ++i) {
		if (freq <= s_dpu_core_rate_tbl_v110[i])
			return i;
	}
	return DPU_PERF_LEVEL_MAX;
}

static uint64_t dpu_config_get_core_rate(uint32_t level)
{
	if (level >= ARRAY_SIZE(s_dpu_core_rate_tbl_v110))
		return s_dpu_core_rate_tbl_v110[DPU_PERF_LEVEL_MAX];

	return s_dpu_core_rate_tbl_v110[level];
}

/*
 * clk sw pll 0b0001: ppll2_b 0b0010: ppll0 0b0100: ppll1 0b1000: ppll2
 * vol_cfg 0b0011: 0.8v 0b0010: 0.7v 0b0001: 0.65v 0b0000: 0.6v
 */
static struct dvfs_vote_info g_vote_info[] = {
	/* level0 0.5~  - 104M, ppll0, 16div  */
	{
		.vote_vol = 0b0101,
		.vote_clk_sel = 0b0010,
		.vote_clk_div = 0b1111,
	},
	/* level1 0.6  - 278M, ppll0, 6div  */
	{
		.vote_vol = 0b0000,
		.vote_clk_sel = 0b0010,
		.vote_clk_div = 0b0101,
	},
	/* level2 0.65  - 418M, ppll0, 4div  */
	{
		.vote_vol = 0b0001,
		.vote_clk_sel = 0b0010,
		.vote_clk_div = 0b0011,
	},
	/* level3 0.7  - 480M, ppll1, 3div  */
	{
		.vote_vol = 0b0010,
		.vote_clk_sel = 0b0100,
		.vote_clk_div = 0b0010,
	},
	/* level4 0.8  - 640M, ppll2_b, div detail see Macro  */
	{
		.vote_vol = 0b0011,
		.vote_clk_sel = 0b0001,
		.vote_clk_div = FREQ3_PLL2B_CLK_DIV,
	},
};

static void dpu_dvfs_peri_init(char __iomem *pmctrl_base)
{
	SOC_PMCTRL_DSS_DVFS_CNT_CFG1_UNION sctrl_cfg1;
	SOC_PMCTRL_DSS_DVFS_CNT_CFG2_UNION sctrl_cfg2;
	SOC_PMCTRL_DSS_DVFS_CNT_CFG3_UNION sctrl_cfg3;
	SOC_PMCTRL_DSS_DVFS_INTR_CFG_UNION sctrl_cfg;
	SOC_PMCTRL_APB_DSS_DVFS_HW_EN_UNION dvfs_en_cfg;

	sctrl_cfg1.value = 0;
	sctrl_cfg1.reg.apb_pclk_cnt_us = 0x54;
	sctrl_cfg1.reg.apb_vol_stable_cnt = 0x1;
	sctrl_cfg1.reg.apb_sw_stable_cnt = 0x1;
	sctrl_cfg1.reg.apb_sw_ack_cnt = 0x6;
	sctrl_cfg1.reg.apb_div_stable_cnt = 0x1;

	outp32(SOC_PMCTRL_DSS_DVFS_CNT_CFG1_ADDR(pmctrl_base), sctrl_cfg1.value);

	sctrl_cfg2.value = 0;
	sctrl_cfg2.reg.apb_div_ack_cnt = 0x6;
	sctrl_cfg2.reg.apb_pll_lock_mode = 0x1;
	sctrl_cfg2.reg.apb_lock_time = 0x32;
	sctrl_cfg2.reg.apb_pll_gt_stable_cnt = 0x1;
	outp32(SOC_PMCTRL_DSS_DVFS_CNT_CFG2_ADDR(pmctrl_base), sctrl_cfg2.value);

	sctrl_cfg3.value = 0;
	// hw channel dvfs need 5us to wait flush_en signal from dpu
	sctrl_cfg3.reg.apb_reload_instr_stable_cnt = 0x1A1;
	outp32(SOC_PMCTRL_DSS_DVFS_CNT_CFG3_ADDR(pmctrl_base), sctrl_cfg3.value);

	sctrl_cfg.value = 0;
	sctrl_cfg.reg.apb_intr_sf_dvfs_bypass = 0x1;
	sctrl_cfg.reg.apb_intr_sf_dvfs_bypass = 0x1;
	outp32(SOC_PMCTRL_DSS_DVFS_INTR_CFG_ADDR(pmctrl_base), sctrl_cfg.value);

	dvfs_en_cfg.value = 0;
	dvfs_en_cfg.reg.apb_hw_ch_fps_cnt = 0x0;
	dvfs_en_cfg.reg.apb_hw_ch_fps_cnt_dss_bypass = 0x0;
	dvfs_en_cfg.reg.apb_hw_ch_fps_cnt_dss2qice_bypass = 0x0;
	dvfs_en_cfg.reg.apb_hw_ch_fps_cnt_dss2ddr_bypass = 0x0;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(pmctrl_base), dvfs_en_cfg.value | 0x7E000000);
}

static void dpu_dvfs_apb_init(char __iomem *pmctrl_base)
{
	SOC_PMCTRL_HW_DSS_DVFS_APB_SW_CFG_UNION sw_cfg;
	SOC_PMCTRL_HW_DSS_DVFS_APB_DIV_CFG_UNION div_cfg;
	SOC_PMCTRL_APB_DSS_DVFS_PLL_CFG_UNION pll_cfg;
	SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_UNION vf_cfg;
	SOC_PMCTRL_APB_DEBUG_CFG_UNION debug_cfg;

	sw_cfg.reg.apb_sw_init = 0x1;
	sw_cfg.reg.apb_sw = 0b0010; /* select pll0 */
	outp32(SOC_PMCTRL_HW_DSS_DVFS_APB_SW_CFG_ADDR(pmctrl_base), sw_cfg.value);
	vf_cfg.reg.apb_sw_chg_init_pulse = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_ADDR(pmctrl_base), vf_cfg.value);

	div_cfg.reg.apb_div_init = 0x1;
	div_cfg.reg.apb_div = 0x5; /* default 278M div 0x5 */
	outp32(SOC_PMCTRL_HW_DSS_DVFS_APB_DIV_CFG_ADDR(pmctrl_base), div_cfg.value);
	vf_cfg.reg.apb_div_chg_init_pulse = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_ADDR(pmctrl_base), vf_cfg.value);

	pll_cfg.reg.apb_pll_gt3 = 0x0; /* ppll2 disable   */
	pll_cfg.reg.apb_pll_en3 = 0x0;
	pll_cfg.reg.apb_pll_gt2 = 0x0; /* ppll1 disable   */
	pll_cfg.reg.apb_pll_en2 = 0x0;
	pll_cfg.reg.apb_pll_gt1 = 0x1; /* ppll0 enable    */
	pll_cfg.reg.apb_pll_en1 = 0x1;
	pll_cfg.reg.apb_pll_gt0 = 0x0; /* ppll2_b disable */
	pll_cfg.reg.apb_pll_en0 = 0x0;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_PLL_CFG_ADDR(pmctrl_base), pll_cfg.value | (0xff << 16));

	vf_cfg.reg.apb_pll_gt3_init = 0x1;
	vf_cfg.reg.apb_pll_en3_init = 0x1;
	vf_cfg.reg.apb_pll_gt2_init = 0x1;
	vf_cfg.reg.apb_pll_en2_init = 0x1;
	vf_cfg.reg.apb_pll_gt1_init = 0x1;
	vf_cfg.reg.apb_pll_en1_init = 0x1;
	vf_cfg.reg.apb_pll_gt0_init = 0x1;
	vf_cfg.reg.apb_pll_en0_init = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_ADDR(pmctrl_base), vf_cfg.value);

	debug_cfg.value = 0;
	debug_cfg.reg.apb_dss2dq_debug_en = 0x1;
	debug_cfg.reg.apb_dss_dvfs_debug_en = 0x1;
	outp32(SOC_PMCTRL_APB_DEBUG_CFG_ADDR(pmctrl_base), debug_cfg.value);
}

static int dpu_dvfs_sw_channel_cfg(char __iomem *pmctrl_base, dpu_dvfs_instr *cmd, int cmd_size, int opt_type)
{
	int i = 0;
	uint32_t value = 0;
	SOC_PMCTRL_HW_DSS_DVFS_SF_CH_UNION sf_ch;
	SOC_PMCTRL_APB_DSS_DVFS_HW_EN_UNION hw_en;

	/*
	 * OPT_VOLT_UP_INDEX: vol --> pll_en&pll_gt --> clk_sw --> clk_div --> pll_en&pll_gt --> end
	 * OPT_VOLT_DOWN_INDEX: pll_en&pll_gt --> clk_div --> clk_sw --> pll_en&pll_gt --> vol--> end
	 */
	int opt_table[OPT_VOLT_MAX_INDEX][SW_DVFS_OPT_NUM] = {
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 1, 2, 4, 3, 5, 6, 0, 7 }
	};

	if (unlikely(opt_type < 0 || opt_type >= OPT_VOLT_MAX_INDEX)) {
		dpu_pr_err("opt_type=%u is out of range", opt_type);
		return -1;
	}

	dpu_pr_debug("current peri dvfs %#x software channel status %#x hw status %#x",
		inp32(SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(pmctrl_base)));

	/* disable hardware channel */
	hw_en.value = 0;
	hw_en.reg.apb_hw_dvfs_enable = 0x0;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(pmctrl_base), hw_en.value | (0b01 << 16));

	/* clear software channel instr and state */
	sf_ch.value = 0;
	sf_ch.reg.apb_sf_instr_clr = 0x1;
	sf_ch.reg.apb_intr_sf_dvfs_clr = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(pmctrl_base), sf_ch.value);

	/* software channel command config */
	for (i = 0; i < SW_DVFS_OPT_NUM; i += 2) {
		dpu_pr_debug("dvfs high - low: %#x, %#x",
				cmd[opt_table[opt_type][i]].value, cmd[opt_table[opt_type][i + 1]].value);

		outp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, i / 2),
				(cmd[opt_table[opt_type][i]].value << 16) | cmd[opt_table[opt_type][i + 1]].value);

		dpu_pr_debug("dvfs cmd-%d: %#x --> %#x \n", i/2, SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(0xFFB81000, i / 2),
				inp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, i / 2)));
	}

	/* enable software channel */
	sf_ch.value = 0;
	sf_ch.reg.apb_sf_dvfs_en = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(pmctrl_base), sf_ch.value);

	i = 0;
	while ((i++ < 45) && ((value & BIT(16)) != BIT(16))) {
		value = inp32(SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(pmctrl_base));
		dpu_pr_debug("dvfs apb_rd_intr_sf_dvfs_raw_stat = %#x", value);
		udelay(10);
	}

	if ((value & BIT(16)) != BIT(16)) {
		dpu_pr_warn("sw dvfs failed, sw instr: %#x %#x %#x %#x",
			inp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, 0)),
			inp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, 1)),
			inp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, 2)),
			inp32(SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(pmctrl_base, 3)));
		dpu_pr_warn("curr dvfs level %#x, soft channel stat %#x, hard channel stat %#x, dvfs ref level %#x",
			inp32(SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(pmctrl_base)),
			inp32(SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(pmctrl_base)),
			inp32(SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(pmctrl_base)),
			inp32(SOC_PMCTRL_PERI_DSS_AVS_VOL_IDX_0_ADDR(pmctrl_base)));
		return -1;
	}

	sf_ch.value = 0;
	sf_ch.reg.apb_sf_instr_clr = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(pmctrl_base), sf_ch.value);

	return 0;
}

static void dpu_dvfs_hw_channel_cfg(dpu_dvfs_instr *cmd, int cmd_size)
{
	int i = 0;
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	SOC_PMCTRL_HW_DSS_DVFS_HW_CH_UNION hw_ch;
	SOC_PMCTRL_APB_DSS_DVFS_HW_EN_UNION hw_en;

	dpu_check_and_no_retval(!pmctrl_base, err, "pmctrl_base is NULL!\n");
	dpu_pr_debug("current peri dvfs %#x software channel status %#x hw status %#x",
		inp32(SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(pmctrl_base)));

	hw_ch.value = 0;
	hw_ch.reg.apb_intr_hw_dvfs_clr = 0x1;
	hw_ch.reg.apb_hw_instr_clr = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_HW_CH_ADDR(pmctrl_base), hw_ch.value);

	hw_en.value = 0;
	hw_en.reg.apb_hw_ch_fps_cnt = 0x0;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(pmctrl_base), hw_en.value | (0xE00 << 16));

	for (i = 0; i < cmd_size; i += 2) {
		dpu_pr_debug("dvfs high - low: %#x, %#x", cmd[i].value, cmd[i + 1].value);
		outp32(SOC_PMCTRL_APB_HW_INSTR_LIST_ADDR(pmctrl_base, i / 2),
				(cmd[i].value << 16) | cmd[i + 1].value);
		dpu_pr_debug("dvfs cmd-%d: %#x --> %#x \n", i/2, SOC_PMCTRL_APB_HW_INSTR_LIST_ADDR(0xFFB81000, i / 2),
				inp32(SOC_PMCTRL_APB_HW_INSTR_LIST_ADDR(pmctrl_base, i / 2)));
	}

	hw_en.value = 0;
	hw_en.reg.apb_hw_dvfs_enable = 0x1;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(pmctrl_base), hw_en.value | (0b01 << 16));
}

static void prepare_sf_instr(uint32_t vote_level, dpu_dvfs_instr *sf_dvfs_instr, uint32_t sf_instr_count)
{
	if (sf_instr_count > SW_DVFS_OPT_NUM)
		return;

	/* When the frequency is increased, the voltage is increased first,
	 * and then the frequency is decreased.
	 * When the PLL is switched, the DIV is switched first,
	 * and the SW is switched first when the frequency is increased.
	 */
	/* vol crg */
	sf_dvfs_instr[0].cmd.op = 0b0011;
	sf_dvfs_instr[0].cmd.halt = 0b0;
	sf_dvfs_instr[0].cmd.instr = g_vote_info[vote_level].vote_vol;

	/* pll en && pll gt */
	sf_dvfs_instr[1].cmd.op = 0b0110;
	sf_dvfs_instr[1].cmd.halt = 0b0;
	sf_dvfs_instr[1].cmd.instr = 0b0111;
	sf_dvfs_instr[2].cmd.op = 0b0111;
	sf_dvfs_instr[2].cmd.halt = 0b0;
	sf_dvfs_instr[2].cmd.instr = 0b0111;

	/* clk sw && clk div */
	sf_dvfs_instr[3].cmd.op = 0b0100;
	sf_dvfs_instr[3].cmd.halt = 0b0;
	sf_dvfs_instr[3].cmd.instr = g_vote_info[vote_level].vote_clk_sel;
	sf_dvfs_instr[4].cmd.op = 0b0101;
	sf_dvfs_instr[4].cmd.halt = 0b0;
	sf_dvfs_instr[4].cmd.instr = g_vote_info[vote_level].vote_clk_div;

	/* pll en && pll gt */
	sf_dvfs_instr[5].cmd.op = 0b0110;
	sf_dvfs_instr[5].cmd.halt = 0b0;
	sf_dvfs_instr[5].cmd.instr = g_vote_info[vote_level].vote_clk_sel;
	sf_dvfs_instr[6].cmd.op = 0b0111;
	sf_dvfs_instr[6].cmd.halt = 0b0;
	sf_dvfs_instr[6].cmd.instr = g_vote_info[vote_level].vote_clk_sel;

	/* end */
	sf_dvfs_instr[7].cmd.op = 0b1111;
	sf_dvfs_instr[7].cmd.halt = 0b0;
	sf_dvfs_instr[7].cmd.instr = 0b0000;
}

static void dpu_sw_dvfs_recovery(char __iomem *pmctrl_base, uint32_t vote_level)
{
	SOC_PMCTRL_HW_DSS_DVFS_HW_RD_UNION hw_rd;
	SOC_PMCTRL_HW_DSS_DVFS_SF_CH_UNION sf_ch;
	SOC_PMCTRL_HW_DSS_DVFS_HW_CH_UNION hw_ch;

	dpu_dvfs_instr max_level_instr[SW_DVFS_OPT_NUM];
	dpu_dvfs_instr sf_dvfs_instr[SW_DVFS_OPT_NUM];

	/* sf dvfs has already wait 450us, there is a high probability that the hw dvfs is busy */
	hw_rd.value = inp32(SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(pmctrl_base));
	if (hw_rd.reg.apb_rd_hw_ch_dvfs_busy == 0)
		return;

	dpu_pr_warn("hw dvfs is busy, hw dvfs state is %#x", hw_rd.value);

	/* sf dvfs exits the process to avoid mis-execution */
	sf_ch.value = 0;
	sf_ch.reg.apb_sf_escape = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(pmctrl_base), sf_ch.value);

	/* hw dvfs exits busy status */
	hw_ch.value = 0;
	hw_ch.reg.apb_hw_escape = 0x1;
	outp32(SOC_PMCTRL_HW_DSS_DVFS_HW_CH_ADDR(pmctrl_base), hw_ch.value);

	/* dvfs status is uncontrollable after exit busy status. to make sure it's right, vote max level */
	prepare_sf_instr(DPU_PERF_LEVEL_MAX, max_level_instr, SW_DVFS_OPT_NUM);
	(void)dpu_dvfs_sw_channel_cfg(pmctrl_base, max_level_instr, SW_DVFS_OPT_NUM, OPT_VOLT_UP_INDEX);

	if (vote_level == DPU_PERF_LEVEL_MAX)
		return;

	/* down to vote level from max level */
	prepare_sf_instr(vote_level, sf_dvfs_instr, SW_DVFS_OPT_NUM);
	(void)dpu_dvfs_sw_channel_cfg(pmctrl_base, sf_dvfs_instr, SW_DVFS_OPT_NUM, OPT_VOLT_DOWN_INDEX);

	return;
}

static void dpu_sw_dvfs_vote_level(uint32_t vote_level, uint32_t opt_type)
{
	int32_t ret = 0;
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	dpu_dvfs_instr sf_dvfs_instr[SW_DVFS_OPT_NUM];
	dpu_check_and_no_retval(!pmctrl_base, err, "pmctrl_base is NULL!\n");

	if (vote_level > DPU_PERF_LEVEL_MAX)
		return;

	prepare_sf_instr(vote_level, sf_dvfs_instr, SW_DVFS_OPT_NUM);

	ret = dpu_dvfs_sw_channel_cfg(pmctrl_base, sf_dvfs_instr, SW_DVFS_OPT_NUM, (int32_t)opt_type);
	if (ret != 0) {
		dpu_pr_info("sw dvfs vote failed, need recovery");
		dpu_sw_dvfs_recovery(pmctrl_base, vote_level);
	}
}

static void dpu_dvfs_enable(void)
{
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	char __iomem *media1_crg = dpu_config_get_ip_base(DISP_IP_BASE_MEDIA1_CRG);
	dpu_check_and_no_retval(!pmctrl_base, err, "pmctrl_base is NULL!\n");
	dpu_check_and_no_retval(!media1_crg, err, "media1_crg is NULL!\n");

	/* hw init */
	dpu_dvfs_peri_init(pmctrl_base);

	/* enable media1 crg dvfs en */
	outp32(SOC_MEDIA1_CRG_DSS_DVFS_ADDR(media1_crg), 0x1);

	/* init select pll0 and div 0xf for power off by 104M */
	dpu_dvfs_apb_init(pmctrl_base);

	g_dvfs_record.last_vote_type = INTER_VOTE_TYPE;
	g_dvfs_record.inter_cur_vote_level = DPU_CORE_LEVEL_ON;
	g_dvfs_record.intra_cur_vote_level = DPU_CORE_LEVEL_ON;
	g_dvfs_record.intra_prev_vote_level = DPU_CORE_LEVEL_ON;
	g_dvfs_record.inter_effect_level = DPU_CORE_LEVEL_ON;
}

static uint32_t prepare_active_hw_instr(uint32_t perf_level, uint32_t last_level,
	dpu_dvfs_instr *hw_instr, uint32_t hw_instr_count)
{
	errno_t err_ret;
	if (perf_level > DPU_PERF_LEVEL_MAX || last_level > DPU_PERF_LEVEL_MAX)
		return 0;

	if (perf_level < last_level) {
		dpu_dvfs_instr reduce_vol_instr[] = {
			/* pll_en && pll_gt */
			{ .cmd.op = 0b0110, .cmd.halt = 0b0,
				.cmd.instr = g_vote_info[perf_level].vote_clk_sel | g_vote_info[last_level].vote_clk_sel },
			{ .cmd.op = 0b0111, .cmd.halt = 0b0,
				.cmd.instr = g_vote_info[perf_level].vote_clk_sel | g_vote_info[last_level].vote_clk_sel },

			/* clk_div & clk sw */
			{ .cmd.op = 0b0101, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_div },
			{ .cmd.op = 0b0100, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },

			/* pll_en && pll_gt */
			{ .cmd.op = 0b0110, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },
			{ .cmd.op = 0b0111, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },

			/* vol_cfg 0b0011: 0.8v 0b0010: 0.7v 0b0001: 0.65v 0b0000: 0.6v */
			{ .cmd.op = 0b0011, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_vol },
		};
		err_ret = memcpy_s(hw_instr, hw_instr_count * sizeof(dpu_dvfs_instr),
			reduce_vol_instr, sizeof(reduce_vol_instr));
		if (err_ret != EOK)
			return 0;
		return ARRAY_SIZE(reduce_vol_instr);
	}
	dpu_dvfs_instr rise_vol_instr[] = {
		/* vol_cfg 0b0011: 0.8v 0b0010: 0.7v 0b0001: 0.65v 0b0000: 0.6v */
		{ .cmd.op = 0b0011, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_vol },

		/* pll_en && pll_gt */
		{ .cmd.op = 0b0110, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel |
			g_vote_info[last_level].vote_clk_sel },
		{ .cmd.op = 0b0111, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel |
			g_vote_info[last_level].vote_clk_sel },

		/* clk sw & clk_div */
		{ .cmd.op = 0b0100, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },
		{ .cmd.op = 0b0101, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_div },

		/* pll_en && pll_gt */
		{ .cmd.op = 0b0110, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },
		{ .cmd.op = 0b0111, .cmd.halt = 0b0, .cmd.instr = g_vote_info[perf_level].vote_clk_sel },
	};

	err_ret = memcpy_s(hw_instr, hw_instr_count * sizeof(dpu_dvfs_instr),
		rise_vol_instr, sizeof(rise_vol_instr));
	if (err_ret != EOK)
		return 0;

	return ARRAY_SIZE(rise_vol_instr);
}

static uint32_t prepare_idle_hw_instr(struct intra_frame_dvfs_info *dvfs_info,
	dpu_dvfs_instr *hw_instr, uint32_t hw_instr_count)
{
	errno_t err_ret;
	dpu_dvfs_instr idle_vol_instr[] = {
		/* wait ov_chg, when ov ycnt configured matching actual line */
		{ .cmd.op = 0b0010, .cmd.halt = 0b0, .cmd.instr = 0b0 },
		/* wait_cht for post processing after wait_ov_chg */
		{ .cmd.op = 0b0001, .cmd.halt = 0b0, .cmd.instr = (DVFS_OV_RESERVE_TIME >> 11)},
		{ .cmd.op = 0b0000, .cmd.halt = 0b0, .cmd.instr = (DVFS_OV_RESERVE_TIME & 0x7ff)},

		/* pll_en && pll_gt */
		{ .cmd.op = 0b0110, .cmd.halt = 0b0, .cmd.instr = g_vote_info[dvfs_info->perf_level].vote_clk_sel |
			g_vote_info[DVFS_IDLE_LEVEL].vote_clk_sel },
		{ .cmd.op = 0b0111, .cmd.halt = 0b0, .cmd.instr = g_vote_info[dvfs_info->perf_level].vote_clk_sel |
			g_vote_info[DVFS_IDLE_LEVEL].vote_clk_sel },

		/* clk_div & clk sw */
		{ .cmd.op = 0b0101, .cmd.halt = 0b0, .cmd.instr = g_vote_info[DVFS_IDLE_LEVEL].vote_clk_div },
		{ .cmd.op = 0b0100, .cmd.halt = 0b0, .cmd.instr = g_vote_info[DVFS_IDLE_LEVEL].vote_clk_sel },

		/* pll en & pll gt */
		{ .cmd.op = 0b0110, .cmd.halt = 0b0, .cmd.instr = g_vote_info[DVFS_IDLE_LEVEL].vote_clk_sel },
		{ .cmd.op = 0b0111, .cmd.halt = 0b0, .cmd.instr = g_vote_info[DVFS_IDLE_LEVEL].vote_clk_sel },

		/* vol_cfg */
		{ .cmd.op = 0b0011, .cmd.halt = 0b0, .cmd.instr = g_vote_info[DVFS_IDLE_LEVEL].vote_vol },

	};

	if (dvfs_info->perf_level > DPU_PERF_LEVEL_MAX)
		return 0;

	dpu_pr_debug("ov reserve time is %u", DVFS_OV_RESERVE_TIME);

	err_ret = memcpy_s(hw_instr, hw_instr_count * sizeof(dpu_dvfs_instr),
		idle_vol_instr, sizeof(idle_vol_instr));
	if (err_ret != EOK)
		return 0;

	return ARRAY_SIZE(idle_vol_instr);
}

static void dpu_hw_dvfs_vote_level(struct intra_frame_dvfs_info *dvfs_info, uint32_t last_level)
{
	uint32_t instr_count = 0;
	uint32_t remain_count = 0;
	uint32_t idle_instr_cnt = 0;
	dpu_dvfs_instr hw_instr[MAX_INSTR_COUNT] = { {0} };

	if (dvfs_info->perf_level != last_level) {
		dpu_pr_debug("perf_levl=%u last perf_level=%u", dvfs_info->perf_level, last_level);
		instr_count = prepare_active_hw_instr(dvfs_info->perf_level,
			last_level, hw_instr, MAX_INSTR_COUNT);
		if (instr_count == 0) {
			dpu_pr_err("prepare active instr failed");
			return;
		}
	}

	remain_count = MAX_INSTR_COUNT - instr_count;
	idle_instr_cnt = prepare_idle_hw_instr(dvfs_info, &(hw_instr[instr_count]), remain_count);
	if (idle_instr_cnt == 0) {
		dpu_pr_err("prepare idle instr failed");
		return;
	}
	instr_count += idle_instr_cnt;

	// If this frame do not need decrease voltage and no idle,
	// hardware channel just wait 5us and do nothing.
	if (instr_count == 0) {
		hw_instr[instr_count].cmd.op = 0b0001;
		hw_instr[instr_count].cmd.halt = 0b0;
		hw_instr[instr_count].cmd.instr = 0x0;
		instr_count++;

		hw_instr[instr_count].cmd.op = 0b0000;
		hw_instr[instr_count].cmd.halt = 0b0;
		hw_instr[instr_count].cmd.instr = 0x5;
		instr_count++;
	}

	/* end */
	hw_instr[instr_count].cmd.op = 0b1111;
	hw_instr[instr_count].cmd.halt = 0b0;
	hw_instr[instr_count].cmd.instr = 0b0000;
	instr_count++;

	dpu_dvfs_hw_channel_cfg(hw_instr, (int32_t)instr_count);
}

static void dpu_dvfs_intra_dvfs(struct intra_frame_dvfs_info *dvfs_info, uint32_t dpu_cur_vote_level)
{
	uint32_t sw_dvfs_frm_rate;

	sw_dvfs_frm_rate = dvfs_info->sw_dvfs_frm_rate;

	if ((g_dvfs_record.cur_frm_rate >= sw_dvfs_frm_rate || g_dvfs_record.prev_frm_rate >= sw_dvfs_frm_rate) &&
		g_dvfs_record.last_vote_type == INTRA_VOTE_TYPE) {
		if (dvfs_info->perf_level >= dpu_cur_vote_level) {
			// If next frame voltage greater than current frame voltage.
			// First use sw channel to increase voltage directly,
			dpu_sw_dvfs_vote_level(dvfs_info->perf_level, OPT_VOLT_UP_INDEX);

			// prepare hw channel instruct to decrease voltage in idle
			dpu_hw_dvfs_vote_level(dvfs_info, dvfs_info->perf_level);
			dpu_qos_qic_media1_config(dvfs_info->perf_level);
		} else {
			// If next frame voltage less current frame voltage.
			// First use sw channel to increase current voltage directly.
			// Then use sw channel to decrease to next frame voltage.
			dpu_sw_dvfs_vote_level(dpu_cur_vote_level, OPT_VOLT_UP_INDEX);

			// prepare hw channel instruct to decrease voltage: frame header and idle
			dpu_hw_dvfs_vote_level(dvfs_info, dpu_cur_vote_level);
			dpu_qos_qic_media1_config(dpu_cur_vote_level);
		}
	} else {
		dpu_hw_dvfs_vote_level(dvfs_info, dpu_cur_vote_level);
		dpu_qos_qic_media1_config(dpu_cur_vote_level);
	}

	// record current intra vote level and status
	if (g_dvfs_record.last_vote_type == INTRA_VOTE_TYPE)
		g_dvfs_record.inter_effect_level = DPU_CORE_LEVEL_ON;
	g_dvfs_record.last_vote_type = INTRA_VOTE_TYPE;
	g_dvfs_record.intra_prev_vote_level = g_dvfs_record.intra_cur_vote_level;
	g_dvfs_record.intra_cur_vote_level = dvfs_info->perf_level;
	g_dvfs_record.prev_frm_rate = g_dvfs_record.cur_frm_rate;
	g_dvfs_record.cur_frm_rate = dvfs_info->frame_rate;
	return;
}

void dpu_dvfs_intra_process(struct intra_frame_dvfs_info *dvfs_info, bool inter_locked)
{
	uint32_t sw_dvfs_frm_rate;
	uint32_t need_vote_level;
	uint32_t dpu_cur_vote_level;
	if (unlikely(!dvfs_info)) {
		dpu_pr_err("dvfs_info is null\n");
		return;
	}

	dpu_pr_debug("intra dvfs info, last: %u curr: %u",
		g_dvfs_record.intra_cur_vote_level,
		dvfs_info->perf_level);

	if ((dpu_dvfs_check_low_temperature() != 0) && (dvfs_info->perf_level == DPU_PERF_LEVEL_MAX)) {
		dvfs_info->perf_level = DPU_PERF_LEVEL_MAX - 1;
		dpu_pr_debug("low temp so perf_level=%u", dvfs_info->perf_level);
	}

	if (inter_locked) {
		dpu_pr_info("cur frame sw dvfs inter_locked is %u", inter_locked);
		dpu_cur_vote_level = g_dvfs_record.inter_effect_level;
		need_vote_level = dpu_cur_vote_level > dvfs_info->perf_level ? dpu_cur_vote_level : dvfs_info->perf_level;
		dpu_pr_debug("inter is locked, dpu_cur_vote_level=%u need_vote_level=%u",
			dpu_cur_vote_level, need_vote_level);
		dpu_sw_dvfs_vote_level(need_vote_level, OPT_VOLT_UP_INDEX);
		dpu_qos_qic_media1_config(need_vote_level);
		g_dvfs_record.last_vote_type = INTER_VOTE_TYPE;
		g_dvfs_record.inter_effect_level = need_vote_level;
		g_dvfs_record.intra_prev_vote_level = g_dvfs_record.intra_cur_vote_level;
		g_dvfs_record.intra_cur_vote_level = dvfs_info->perf_level;
		return;
	}

	sw_dvfs_frm_rate = dvfs_info->sw_dvfs_frm_rate;
	dpu_pr_debug("sw_dvfs_frm_rate = %d", sw_dvfs_frm_rate);

	if (g_dvfs_record.last_vote_type == INTRA_VOTE_TYPE) {
		dpu_cur_vote_level = DPU_CORE_LEVEL_ON;
		if (g_dvfs_record.cur_frm_rate >= sw_dvfs_frm_rate || g_dvfs_record.prev_frm_rate >= sw_dvfs_frm_rate)
			dpu_cur_vote_level = g_dvfs_record.intra_cur_vote_level;
	} else {
		dpu_cur_vote_level = g_dvfs_record.inter_effect_level;
	}

	dpu_pr_debug("g_dvfs_record->prev frm rate=%u", g_dvfs_record.prev_frm_rate);
	dpu_dvfs_intra_dvfs(dvfs_info, dpu_cur_vote_level);
}

void dpu_dvfs_inter_process(uint32_t vote_level, bool is_inter_invoke)
{
	uint32_t need_vote_level;
	uint32_t dpu_cur_vote_level;
	uint32_t opt_type;

	if ((dpu_dvfs_check_low_temperature() != 0) && (vote_level == DPU_PERF_LEVEL_MAX)) {
		vote_level = DPU_PERF_LEVEL_MAX - 1;
		dpu_pr_debug("low temp so perf_level=%u", vote_level);
	}

	if (g_dvfs_record.last_vote_type == INTRA_VOTE_TYPE) {
		// although dpu_cur_vote_level may not the actual effective level here,
		// need_vote_level is sure higher than dpu_cur_vote_level, it must be the boost process
		dpu_cur_vote_level = max_of_three(g_dvfs_record.intra_prev_vote_level, g_dvfs_record.intra_cur_vote_level,
			g_dvfs_record.inter_effect_level);
		need_vote_level = max(vote_level, dpu_cur_vote_level);
	} else {
		dpu_cur_vote_level = g_dvfs_record.inter_effect_level;
		need_vote_level =
			max_of_three(vote_level, g_dvfs_record.intra_cur_vote_level, g_dvfs_record.intra_prev_vote_level);
	}

	dpu_pr_debug("vote_level =%u need_vote_level = %u dpu_cur_vote_level=%u",
		vote_level, need_vote_level, dpu_cur_vote_level);

	// next voltage equal current voltage, also need up instructs.
	opt_type = need_vote_level >= dpu_cur_vote_level ? OPT_VOLT_UP_INDEX : OPT_VOLT_DOWN_INDEX;

	dpu_sw_dvfs_vote_level(need_vote_level, opt_type);

	dpu_qos_qic_media1_config(need_vote_level);

	g_dvfs_record.last_vote_type = INTER_VOTE_TYPE;
	g_dvfs_record.inter_effect_level = need_vote_level;

	if (is_inter_invoke) {
	    g_dvfs_record.inter_cur_vote_level = vote_level;
	} else {
	    g_dvfs_record.intra_prev_vote_level = g_dvfs_record.intra_cur_vote_level;
	    g_dvfs_record.intra_cur_vote_level = vote_level;
	}
}

void dpu_dvfs_direct_process(uint32_t vote_level, bool need_config_qos)
{
	uint32_t dpu_cur_vote_level;
	uint32_t opt_type;

	dpu_cur_vote_level = max_of_three(g_dvfs_record.intra_prev_vote_level, g_dvfs_record.intra_cur_vote_level,
		g_dvfs_record.inter_cur_vote_level);

	// next voltage equal current voltage, also need up instructs.
	opt_type = vote_level >= dpu_cur_vote_level ? OPT_VOLT_UP_INDEX : OPT_VOLT_DOWN_INDEX;

	dpu_sw_dvfs_vote_level(vote_level, opt_type);

	if (need_config_qos)
		dpu_qos_qic_media1_config(vote_level);

	g_dvfs_record.last_vote_type = INTER_VOTE_TYPE;
	g_dvfs_record.inter_effect_level = vote_level;
	g_dvfs_record.inter_cur_vote_level = vote_level;
	g_dvfs_record.intra_cur_vote_level = vote_level;
	g_dvfs_record.intra_prev_vote_level = vote_level;
}

bool is_dpu_dvfs_enable(void)
{
	return true;
}

void dpu_qos_qic_media1_config(uint32_t level)
{
	uint32_t perf_level = (g_debug_dpu_qos_enable == 0) ? DPU_PERF_LEVEL_MAX : level;
#if defined(CONFIG_DRMDRIVER)
	/* if debug_dpu_qos_enable == 0, then lock qos config to no limit */
	configure_dss_service_security(DSS_QOS_CONFIG, perf_level, 0, BIG_DPU);
#endif
}

/* qos qic-media1 config should follow dvfs level, after regulater is enabled */
void dpu_legacy_inter_frame_dvfs_vote(uint32_t vote_level, bool need_config_qos)
{
	dpu_check_and_no_retval((vote_level > DPU_PERF_LEVEL_MAX), err, "don't support perf level%u!", vote_level);

	dpu_pr_info("config clk rate=%llu vote_level=%u",
		dpu_config_get_core_rate(vote_level), vote_level);

	dpu_config_dvfs_vote_exec(dpu_config_get_core_rate(vote_level));

	if (need_config_qos)
		dpu_qos_qic_media1_config(vote_level);
}

/* when power on or exit idle, dvfs set to low level first to climb quickly,
 * if higher level is needed, dvfs will adjust again
 * qos qic-media1 config should follow dvfs level, after regulater is enabled
 * (1) power on: enable core clk -> dvfs -> enable regulater -> config qic-media1, no need config qos
 * (2) exit idle: enable core clk -> dvfs -> config qic-media1, need config qos
 */
void dpu_enable_core_clock(bool is_power_on)
{
	dpu_pr_debug("+");

	if (g_dpu_config_data.clk_gate_edc == NULL) {
		dpu_pr_err("clk_gate_edc is null!");
		return;
	}

	if (clk_prepare_enable(g_dpu_config_data.clk_gate_edc) != 0) {
		dpu_pr_err("enable clk_gate_edc failed!");
		return;
	}

	if (is_power_on)
		dpu_dvfs_enable();

	dpu_pr_debug("-");
}

void dpu_disable_core_clock(void)
{
	dpu_pr_debug("+");

	if (g_dpu_config_data.clk_gate_edc != NULL)
		clk_disable_unprepare(g_dpu_config_data.clk_gate_edc);
}

void dpu_print_dvfs_vote_status(void)
{
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	dpu_check_and_no_retval(!pmctrl_base, err, "pmctrl_base is NULL!\n");

	dpu_pr_warn("DVFS:curr dvfs level %#x, soft channel stat %#x, hard channel stat %#x, dvfs ref level %#x",
		inp32(SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(pmctrl_base)),
		inp32(SOC_PMCTRL_PERI_DSS_AVS_VOL_IDX_0_ADDR(pmctrl_base)));
}

void dpu_skip_intra_frame_idle_dvfs(void)
{
	SOC_PMCTRL_APB_DSS_DVFS_HW_EN_UNION hw_en;
	char __iomem *pmctrl_base = dpu_config_get_ip_base(DISP_IP_BASE_PMCTRL);
	dpu_check_and_no_retval(!pmctrl_base, err, "pmctrl_base is NULL!\n");

	/* disable hardware channel */
	hw_en.value = 0;
	hw_en.reg.apb_hw_dvfs_enable = 0x0;
	outp32(SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(pmctrl_base), hw_en.value | (0b01 << 16));
}
