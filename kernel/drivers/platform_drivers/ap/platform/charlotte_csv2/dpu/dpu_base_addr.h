#ifndef DPU_BASE_ADDR_H
#define DPU_BASE_ADDR_H 
#define ATF_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE 0x10941000
#define ATF_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_SIZE (0x10000)
#ifndef __SOC_H_FOR_ASM__
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + 0x438UL)
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + 0x40CUL)
#define SOC_CRGPERIPH_PEREN12_ADDR(base) ((base) + 0x470UL)
#define SOC_CRGPERIPH_PERRSTEN0_ADDR(base) ((base) + 0x060UL)
#define SOC_CRGPERIPH_PERRSTDIS0_ADDR(base) ((base) + 0x064UL)
#define SOC_CRGPERIPH_PERRSTSTAT0_ADDR(base) ((base) + 0x068UL)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(base) ((base) + (0xA94UL))
#define SOC_PMCTRL_PERI_DSS_AVS_VOL_IDX_0_ADDR(base) ((base) + (0xA40UL))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_SW_CFG_ADDR(base) ((base) + (0x030UL))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_DIV_CFG_ADDR(base) ((base) + (0x034UL))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_ADDR(base) ((base) + (0x040UL))
#define SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(base) ((base) + (0x054UL))
#define SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(base) ((base) + (0x058UL))
#define SOC_PMCTRL_HW_DSS_DVFS_HW_CH_ADDR(base) ((base) + (0x05CUL))
#define SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(base) ((base) + (0x060UL))
#define SOC_PMCTRL_APB_DSS_DVFS_PLL_CFG_ADDR(base) ((base) + (0x038UL))
#define SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(base,sf) ((base) + (0x070+ (sf)*4UL))
#define SOC_PMCTRL_APB_DEBUG_CFG_ADDR(base) ((base) + (0x1D0UL))
#define SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(base) ((base) + (0x064UL))
#define SOC_PMCTRL_APB_HW_INSTR_LIST_ADDR(base,hw) ((base) + (0x100+ (hw)*4UL))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG1_ADDR(base) ((base) + (0x044UL))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG2_ADDR(base) ((base) + (0x048UL))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG3_ADDR(base) ((base) + (0x04CUL))
#define SOC_PMCTRL_DSS_DVFS_INTR_CFG_ADDR(base) ((base) + (0x050UL))
#else
#define SOC_SCTRL_SCBAKDATA11_ADDR(base) ((base) + 0x438)
#define SOC_SCTRL_SCBAKDATA0_ADDR(base) ((base) + 0x40C)
#define SOC_CRGPERIPH_PEREN12_ADDR(base) ((base) + 0x470)
#define SOC_CRGPERIPH_PERRSTEN0_ADDR(base) ((base) + 0x060)
#define SOC_CRGPERIPH_PERRSTDIS0_ADDR(base) ((base) + 0x064)
#define SOC_CRGPERIPH_PERRSTSTAT0_ADDR(base) ((base) + 0x068)
#define SOC_PMCTRL_PERI_VALID_HRD_VOL_IDX_ADDR(base) ((base) + (0xA94))
#define SOC_PMCTRL_PERI_DSS_AVS_VOL_IDX_0_ADDR(base) ((base) + (0xA40))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_SW_CFG_ADDR(base) ((base) + (0x030))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_DIV_CFG_ADDR(base) ((base) + (0x034))
#define SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_ADDR(base) ((base) + (0x040))
#define SOC_PMCTRL_HW_DSS_DVFS_SF_CH_ADDR(base) ((base) + (0x054))
#define SOC_PMCTRL_HW_DSS_DVFS_SF_RD_ADDR(base) ((base) + (0x058))
#define SOC_PMCTRL_HW_DSS_DVFS_HW_CH_ADDR(base) ((base) + (0x05C))
#define SOC_PMCTRL_HW_DSS_DVFS_HW_RD_ADDR(base) ((base) + (0x060))
#define SOC_PMCTRL_APB_DSS_DVFS_PLL_CFG_ADDR(base) ((base) + (0x038))
#define SOC_PMCTRL_APB_SF_INSTR_LIST_ADDR(base,sf) ((base) + (0x070+ (sf)*4))
#define SOC_PMCTRL_APB_DEBUG_CFG_ADDR(base) ((base) + (0x1D0))
#define SOC_PMCTRL_APB_DSS_DVFS_HW_EN_ADDR(base) ((base) + (0x064))
#define SOC_PMCTRL_APB_HW_INSTR_LIST_ADDR(base,hw) ((base) + (0x100+ (hw)*4))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG1_ADDR(base) ((base) + (0x044))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG2_ADDR(base) ((base) + (0x048))
#define SOC_PMCTRL_DSS_DVFS_CNT_CFG3_ADDR(base) ((base) + (0x04C))
#define SOC_PMCTRL_DSS_DVFS_INTR_CFG_ADDR(base) ((base) + (0x050))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_pclk_cnt_us : 10;
        unsigned int apb_vol_stable_cnt : 4;
        unsigned int apb_sw_stable_cnt : 4;
        unsigned int apb_sw_ack_cnt : 8;
        unsigned int apb_div_stable_cnt : 4;
        unsigned int reserved : 2;
    } reg;
} SOC_PMCTRL_DSS_DVFS_CNT_CFG1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_div_ack_cnt : 8;
        unsigned int apb_pll_lock_mode : 1;
        unsigned int apb_lock_time : 13;
        unsigned int apb_pll_gt_stable_cnt : 4;
        unsigned int reserved_0 : 4;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_PMCTRL_DSS_DVFS_CNT_CFG2_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_reload_instr_stable_cnt : 16;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 12;
    } reg;
} SOC_PMCTRL_DSS_DVFS_CNT_CFG3_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_intr_sf_dvfs_bypass : 1;
        unsigned int apb_intr_sf_dvfs_m3orcpu_msk : 1;
        unsigned int apb_intr_hw_dvfs_bypass : 1;
        unsigned int apb_intr_hw_dvfs_m3orcpu_msk : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 27;
    } reg;
} SOC_PMCTRL_DSS_DVFS_INTR_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_pll_en0 : 1;
        unsigned int apb_pll_en1 : 1;
        unsigned int apb_pll_en2 : 1;
        unsigned int apb_pll_en3 : 1;
        unsigned int apb_pll_gt0 : 1;
        unsigned int apb_pll_gt1 : 1;
        unsigned int apb_pll_gt2 : 1;
        unsigned int apb_pll_gt3 : 1;
        unsigned int reserved_0 : 8;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_PMCTRL_APB_DSS_DVFS_PLL_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_hw_dvfs_enable : 1;
        unsigned int apb_hw_instr_lock : 1;
        unsigned int reserved_0 : 1;
        unsigned int apb_hw_dss2ddr_dfs_enable : 1;
        unsigned int apb_dss2ddr_instr_lock : 1;
        unsigned int reserved_1 : 1;
        unsigned int apb_hw_dss2qice_dfs_enable : 1;
        unsigned int apb_dss2qice_instr_lock : 1;
        unsigned int reserved_2 : 1;
        unsigned int apb_hw_ch_fps_cnt : 3;
        unsigned int apb_hw_ch_fps_cnt_dss_bypass : 1;
        unsigned int apb_hw_ch_fps_cnt_dss2qice_bypass : 1;
        unsigned int apb_hw_ch_fps_cnt_dss2ddr_bypass : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 16;
    } reg;
} SOC_PMCTRL_APB_DSS_DVFS_HW_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_dss2dq_debug_en : 1;
        unsigned int apb_dss2dq_valid_cnt_clr : 1;
        unsigned int apb_dss2dq_chg_cnt_clr : 1;
        unsigned int apb_flush_cnt_clr : 1;
        unsigned int apb_vsync_cnt_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int apb_dss_dvfs_debug_en : 1;
        unsigned int apb_dss_dvfs_valid_cnt_clr : 1;
        unsigned int apb_dss_dvfs_chg_cnt_clr : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_PMCTRL_APB_DEBUG_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_sw : 4;
        unsigned int apb_sw_init : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_APB_SW_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_div : 6;
        unsigned int apb_div_init : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_APB_DIV_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_vol_chg_init_pulse : 1;
        unsigned int apb_div_chg_init_pulse : 1;
        unsigned int apb_sw_chg_init_pulse : 1;
        unsigned int apb_pll_en3_init : 1;
        unsigned int apb_pll_en2_init : 1;
        unsigned int apb_pll_en1_init : 1;
        unsigned int apb_pll_en0_init : 1;
        unsigned int apb_pll_gt3_init : 1;
        unsigned int apb_pll_gt2_init : 1;
        unsigned int apb_pll_gt1_init : 1;
        unsigned int apb_pll_gt0_init : 1;
        unsigned int reserved : 21;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_APB_VF_CFG_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_intr_sf_dvfs_clr : 1;
        unsigned int apb_sf_escape : 1;
        unsigned int apb_sf_next_step : 1;
        unsigned int apb_sf_dvfs_en : 1;
        unsigned int apb_sf_instr_clr : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_SF_CH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_intr_hw_dvfs_clr : 1;
        unsigned int apb_hw_escape : 1;
        unsigned int apb_hw_next_step : 1;
        unsigned int apb_hw_flush_pls_init : 1;
        unsigned int apb_hw_instr_clr : 1;
        unsigned int reserved_0 : 1;
        unsigned int apb_hw_dss2ddr_dfs_intr_clr : 1;
        unsigned int apb_hw_dss2ddr_escape : 1;
        unsigned int apb_hw_dss2ddr_next_step : 1;
        unsigned int apb_hw_dss2ddr_flush_pls_init : 1;
        unsigned int apb_dss2ddr_instr_clr : 1;
        unsigned int reserved_1 : 1;
        unsigned int apb_hw_dss2qice_dfs_intr_clr : 1;
        unsigned int apb_hw_dss2qice_escape : 1;
        unsigned int apb_hw_dss2qice_next_step : 1;
        unsigned int apb_hw_dss2qice_flush_pls_init : 1;
        unsigned int apb_dss2qice_instr_clr : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 12;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_HW_CH_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int apb_rd_hw_cur_instr_num : 6;
        unsigned int apb_rd_hw_cur_stat : 4;
        unsigned int apb_rd_hw_ch_vol_idx : 8;
        unsigned int apb_rd_intr_hw_dvfs_raw_stat : 1;
        unsigned int apb_rd_intr_hw_dvfs_stat : 1;
        unsigned int apb_rd_hw_ch_dvfs_busy : 1;
        unsigned int reserved : 11;
    } reg;
} SOC_PMCTRL_HW_DSS_DVFS_HW_RD_UNION;
#endif
#endif
