#ifndef __SOC_PCR_INTERFACE_H__
#define __SOC_PCR_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_PCR_EN_ADDR(base) ((base) + (0x000UL))
#define SOC_PCR_CTRL0_ADDR(base) ((base) + (0x004UL))
#define SOC_PCR_CTRL1_ADDR(base) ((base) + (0x008UL))
#define SOC_PCR_CLK_GATE_ADDR(base) ((base) + (0x00CUL))
#define SOC_PCR_CFG_BYPASS_ADDR(base) ((base) + (0x010UL))
#define SOC_PCR_CLK_SW_SET_ADDR(base) ((base) + (0x014UL))
#define SOC_PCR_SAMPLE_INTERVAL_ADDR(base) ((base) + (0x018UL))
#define SOC_PCR_DIDT_WINDOW_ADDR(base) ((base) + (0x01CUL))
#define SOC_PCR_MAXPOWER_WINDOW_ADDR(base) ((base) + (0x020UL))
#define SOC_PCR_FREQ_DN_PERIOD_ADDR(base) ((base) + (0x024UL))
#define SOC_PCR_FREQ_UP_PERIOD0_ADDR(base) ((base) + (0x028UL))
#define SOC_PCR_FREQ_UP_PERIOD1_ADDR(base) ((base) + (0x02CUL))
#define SOC_PCR_FREQ_UP_PERIOD2_ADDR(base) ((base) + (0x030UL))
#define SOC_PCR_FREQ_UP_PERIOD3_ADDR(base) ((base) + (0x034UL))
#define SOC_PCR_THRESHOLD_DN_BUDGET0_ADDR(base) ((base) + (0x040UL))
#define SOC_PCR_THRESHOLD_DN_BUDGET1_ADDR(base) ((base) + (0x044UL))
#define SOC_PCR_THRESHOLD_DN_BUDGET2_ADDR(base) ((base) + (0x048UL))
#define SOC_PCR_THRESHOLD_DN_BUDGET3_ADDR(base) ((base) + (0x04CUL))
#define SOC_PCR_THRESHOLD_UP_BUDGET0_ADDR(base) ((base) + (0x050UL))
#define SOC_PCR_THRESHOLD_UP_BUDGET1_ADDR(base) ((base) + (0x054UL))
#define SOC_PCR_THRESHOLD_UP_BUDGET2_ADDR(base) ((base) + (0x058UL))
#define SOC_PCR_THRESHOLD_UP_BUDGET3_ADDR(base) ((base) + (0x05CUL))
#define SOC_PCR_THRESHOLD_DN_DIDT0_ADDR(base) ((base) + (0x060UL))
#define SOC_PCR_THRESHOLD_DN_DIDT1_ADDR(base) ((base) + (0x064UL))
#define SOC_PCR_THRESHOLD_DN_DIDT2_ADDR(base) ((base) + (0x068UL))
#define SOC_PCR_THRESHOLD_DN_DIDT3_ADDR(base) ((base) + (0x06CUL))
#define SOC_PCR_THRESHOLD_UP_DIDT0_ADDR(base) ((base) + (0x070UL))
#define SOC_PCR_THRESHOLD_UP_DIDT1_ADDR(base) ((base) + (0x074UL))
#define SOC_PCR_THRESHOLD_UP_DIDT2_ADDR(base) ((base) + (0x078UL))
#define SOC_PCR_THRESHOLD_UP_DIDT3_ADDR(base) ((base) + (0x07CUL))
#define SOC_PCR_SNAPSHOT_ADDR(base) ((base) + (0x080UL))
#define SOC_PCR_SNAPSHOT_AVG_POWER_ADDR(base) ((base) + (0x084UL))
#define SOC_PCR_SNAPSHOT_HIS_MAXPOWER_ADDR(base) ((base) + (0x088UL))
#define SOC_PCR_SNAPSHOT_DIDT_ADDR(base) ((base) + (0x08CUL))
#define SOC_PCR_SNAPSHOT_HIS_MAXDIDT_ADDR(base) ((base) + (0x090UL))
#define SOC_PCR_SNAPSHOT_ENERGY_ADDR(base) ((base) + (0x094UL))
#define SOC_PCR_SNAPSHOT_ACC_ENERGY_ADDR(base) ((base) + (0x098UL))
#define SOC_PCR_SNAPSHOT_STATUS0_ADDR(base) ((base) + (0x09CUL))
#define SOC_PCR_SNAPSHOT_STATUS1_ADDR(base) ((base) + (0x0A0UL))
#define SOC_PCR_DEBUG_CLR_ADDR(base) ((base) + (0x0A4UL))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT0_ADDR(base) ((base) + (0x0B0UL))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT1_ADDR(base) ((base) + (0x0B4UL))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT2_ADDR(base) ((base) + (0x0B8UL))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT3_ADDR(base) ((base) + (0x0BCUL))
#define SOC_PCR_INTR_STAT_ADDR(base) ((base) + (0x0C0UL))
#define SOC_PCR_INTR_RAW_STAT_ADDR(base) ((base) + (0x0C4UL))
#define SOC_PCR_INTR_EN_ADDR(base) ((base) + (0x0C8UL))
#define SOC_PCR_INTR_CLR_ADDR(base) ((base) + (0x0CCUL))
#else
#define SOC_PCR_EN_ADDR(base) ((base) + (0x000))
#define SOC_PCR_CTRL0_ADDR(base) ((base) + (0x004))
#define SOC_PCR_CTRL1_ADDR(base) ((base) + (0x008))
#define SOC_PCR_CLK_GATE_ADDR(base) ((base) + (0x00C))
#define SOC_PCR_CFG_BYPASS_ADDR(base) ((base) + (0x010))
#define SOC_PCR_CLK_SW_SET_ADDR(base) ((base) + (0x014))
#define SOC_PCR_SAMPLE_INTERVAL_ADDR(base) ((base) + (0x018))
#define SOC_PCR_DIDT_WINDOW_ADDR(base) ((base) + (0x01C))
#define SOC_PCR_MAXPOWER_WINDOW_ADDR(base) ((base) + (0x020))
#define SOC_PCR_FREQ_DN_PERIOD_ADDR(base) ((base) + (0x024))
#define SOC_PCR_FREQ_UP_PERIOD0_ADDR(base) ((base) + (0x028))
#define SOC_PCR_FREQ_UP_PERIOD1_ADDR(base) ((base) + (0x02C))
#define SOC_PCR_FREQ_UP_PERIOD2_ADDR(base) ((base) + (0x030))
#define SOC_PCR_FREQ_UP_PERIOD3_ADDR(base) ((base) + (0x034))
#define SOC_PCR_THRESHOLD_DN_BUDGET0_ADDR(base) ((base) + (0x040))
#define SOC_PCR_THRESHOLD_DN_BUDGET1_ADDR(base) ((base) + (0x044))
#define SOC_PCR_THRESHOLD_DN_BUDGET2_ADDR(base) ((base) + (0x048))
#define SOC_PCR_THRESHOLD_DN_BUDGET3_ADDR(base) ((base) + (0x04C))
#define SOC_PCR_THRESHOLD_UP_BUDGET0_ADDR(base) ((base) + (0x050))
#define SOC_PCR_THRESHOLD_UP_BUDGET1_ADDR(base) ((base) + (0x054))
#define SOC_PCR_THRESHOLD_UP_BUDGET2_ADDR(base) ((base) + (0x058))
#define SOC_PCR_THRESHOLD_UP_BUDGET3_ADDR(base) ((base) + (0x05C))
#define SOC_PCR_THRESHOLD_DN_DIDT0_ADDR(base) ((base) + (0x060))
#define SOC_PCR_THRESHOLD_DN_DIDT1_ADDR(base) ((base) + (0x064))
#define SOC_PCR_THRESHOLD_DN_DIDT2_ADDR(base) ((base) + (0x068))
#define SOC_PCR_THRESHOLD_DN_DIDT3_ADDR(base) ((base) + (0x06C))
#define SOC_PCR_THRESHOLD_UP_DIDT0_ADDR(base) ((base) + (0x070))
#define SOC_PCR_THRESHOLD_UP_DIDT1_ADDR(base) ((base) + (0x074))
#define SOC_PCR_THRESHOLD_UP_DIDT2_ADDR(base) ((base) + (0x078))
#define SOC_PCR_THRESHOLD_UP_DIDT3_ADDR(base) ((base) + (0x07C))
#define SOC_PCR_SNAPSHOT_ADDR(base) ((base) + (0x080))
#define SOC_PCR_SNAPSHOT_AVG_POWER_ADDR(base) ((base) + (0x084))
#define SOC_PCR_SNAPSHOT_HIS_MAXPOWER_ADDR(base) ((base) + (0x088))
#define SOC_PCR_SNAPSHOT_DIDT_ADDR(base) ((base) + (0x08C))
#define SOC_PCR_SNAPSHOT_HIS_MAXDIDT_ADDR(base) ((base) + (0x090))
#define SOC_PCR_SNAPSHOT_ENERGY_ADDR(base) ((base) + (0x094))
#define SOC_PCR_SNAPSHOT_ACC_ENERGY_ADDR(base) ((base) + (0x098))
#define SOC_PCR_SNAPSHOT_STATUS0_ADDR(base) ((base) + (0x09C))
#define SOC_PCR_SNAPSHOT_STATUS1_ADDR(base) ((base) + (0x0A0))
#define SOC_PCR_DEBUG_CLR_ADDR(base) ((base) + (0x0A4))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT0_ADDR(base) ((base) + (0x0B0))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT1_ADDR(base) ((base) + (0x0B4))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT2_ADDR(base) ((base) + (0x0B8))
#define SOC_PCR_FREQ_DN_TRIGGER_CNT3_ADDR(base) ((base) + (0x0BC))
#define SOC_PCR_INTR_STAT_ADDR(base) ((base) + (0x0C0))
#define SOC_PCR_INTR_RAW_STAT_ADDR(base) ((base) + (0x0C4))
#define SOC_PCR_INTR_EN_ADDR(base) ((base) + (0x0C8))
#define SOC_PCR_INTR_CLR_ADDR(base) ((base) + (0x0CC))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_EN_UNION;
#endif
#define SOC_PCR_EN_pcr_en_START (0)
#define SOC_PCR_EN_pcr_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int didt_detect_en : 1;
        unsigned int maxpower_detect_en : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PCR_CTRL0_UNION;
#endif
#define SOC_PCR_CTRL0_didt_detect_en_START (0)
#define SOC_PCR_CTRL0_didt_detect_en_END (0)
#define SOC_PCR_CTRL0_maxpower_detect_en_START (1)
#define SOC_PCR_CTRL0_maxpower_detect_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int force_idle : 1;
        unsigned int freq_dn_trig_cnt_en : 1;
        unsigned int dvfs_busy_bypass : 1;
        unsigned int dvfs_busy_keep : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_PCR_CTRL1_UNION;
#endif
#define SOC_PCR_CTRL1_force_idle_START (0)
#define SOC_PCR_CTRL1_force_idle_END (0)
#define SOC_PCR_CTRL1_freq_dn_trig_cnt_en_START (1)
#define SOC_PCR_CTRL1_freq_dn_trig_cnt_en_END (1)
#define SOC_PCR_CTRL1_dvfs_busy_bypass_START (2)
#define SOC_PCR_CTRL1_dvfs_busy_bypass_END (2)
#define SOC_PCR_CTRL1_dvfs_busy_keep_START (3)
#define SOC_PCR_CTRL1_dvfs_busy_keep_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pclk_cg_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_CLK_GATE_UNION;
#endif
#define SOC_PCR_CLK_GATE_pclk_cg_bypass_START (0)
#define SOC_PCR_CLK_GATE_pclk_cg_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int clk_sw_bypass : 1;
        unsigned int to_gpio_bypass : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_PCR_CFG_BYPASS_UNION;
#endif
#define SOC_PCR_CFG_BYPASS_clk_sw_bypass_START (0)
#define SOC_PCR_CFG_BYPASS_clk_sw_bypass_END (0)
#define SOC_PCR_CFG_BYPASS_to_gpio_bypass_START (1)
#define SOC_PCR_CFG_BYPASS_to_gpio_bypass_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int clk_sw_freq_dn0 : 2;
        unsigned int clk_sw_freq_dn1 : 2;
        unsigned int clk_sw_freq_dn2 : 2;
        unsigned int clk_sw_freq_dn3 : 2;
        unsigned int reserved : 24;
    } reg;
} SOC_PCR_CLK_SW_SET_UNION;
#endif
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn0_START (0)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn0_END (1)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn1_START (2)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn1_END (3)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn2_START (4)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn2_END (5)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn3_START (6)
#define SOC_PCR_CLK_SW_SET_clk_sw_freq_dn3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sample_interval : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_PCR_SAMPLE_INTERVAL_UNION;
#endif
#define SOC_PCR_SAMPLE_INTERVAL_sample_interval_START (0)
#define SOC_PCR_SAMPLE_INTERVAL_sample_interval_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int didt_window : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_PCR_DIDT_WINDOW_UNION;
#endif
#define SOC_PCR_DIDT_WINDOW_didt_window_START (0)
#define SOC_PCR_DIDT_WINDOW_didt_window_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int maxpower_window : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_PCR_MAXPOWER_WINDOW_UNION;
#endif
#define SOC_PCR_MAXPOWER_WINDOW_maxpower_window_START (0)
#define SOC_PCR_MAXPOWER_WINDOW_maxpower_window_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int first_dn_period : 10;
        unsigned int reserved_0 : 6;
        unsigned int dn_period : 10;
        unsigned int reserved_1 : 6;
    } reg;
} SOC_PCR_FREQ_DN_PERIOD_UNION;
#endif
#define SOC_PCR_FREQ_DN_PERIOD_first_dn_period_START (0)
#define SOC_PCR_FREQ_DN_PERIOD_first_dn_period_END (9)
#define SOC_PCR_FREQ_DN_PERIOD_dn_period_START (16)
#define SOC_PCR_FREQ_DN_PERIOD_dn_period_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int up_period0 : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_PCR_FREQ_UP_PERIOD0_UNION;
#endif
#define SOC_PCR_FREQ_UP_PERIOD0_up_period0_START (0)
#define SOC_PCR_FREQ_UP_PERIOD0_up_period0_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int up_period1 : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_PCR_FREQ_UP_PERIOD1_UNION;
#endif
#define SOC_PCR_FREQ_UP_PERIOD1_up_period1_START (0)
#define SOC_PCR_FREQ_UP_PERIOD1_up_period1_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int up_period2 : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_PCR_FREQ_UP_PERIOD2_UNION;
#endif
#define SOC_PCR_FREQ_UP_PERIOD2_up_period2_START (0)
#define SOC_PCR_FREQ_UP_PERIOD2_up_period2_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int up_period3 : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_PCR_FREQ_UP_PERIOD3_UNION;
#endif
#define SOC_PCR_FREQ_UP_PERIOD3_up_period3_START (0)
#define SOC_PCR_FREQ_UP_PERIOD3_up_period3_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_budget0 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_BUDGET0_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_BUDGET0_threshold_dn_budget0_START (0)
#define SOC_PCR_THRESHOLD_DN_BUDGET0_threshold_dn_budget0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_budget1 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_BUDGET1_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_BUDGET1_threshold_dn_budget1_START (0)
#define SOC_PCR_THRESHOLD_DN_BUDGET1_threshold_dn_budget1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_budget2 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_BUDGET2_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_BUDGET2_threshold_dn_budget2_START (0)
#define SOC_PCR_THRESHOLD_DN_BUDGET2_threshold_dn_budget2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_budget3 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_BUDGET3_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_BUDGET3_threshold_dn_budget3_START (0)
#define SOC_PCR_THRESHOLD_DN_BUDGET3_threshold_dn_budget3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_budget0 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_BUDGET0_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_BUDGET0_threshold_up_budget0_START (0)
#define SOC_PCR_THRESHOLD_UP_BUDGET0_threshold_up_budget0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_budget1 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_BUDGET1_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_BUDGET1_threshold_up_budget1_START (0)
#define SOC_PCR_THRESHOLD_UP_BUDGET1_threshold_up_budget1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_budget2 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_BUDGET2_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_BUDGET2_threshold_up_budget2_START (0)
#define SOC_PCR_THRESHOLD_UP_BUDGET2_threshold_up_budget2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_budget3 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_BUDGET3_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_BUDGET3_threshold_up_budget3_START (0)
#define SOC_PCR_THRESHOLD_UP_BUDGET3_threshold_up_budget3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_didt0 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_DIDT0_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_DIDT0_threshold_dn_didt0_START (0)
#define SOC_PCR_THRESHOLD_DN_DIDT0_threshold_dn_didt0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_didt1 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_DIDT1_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_DIDT1_threshold_dn_didt1_START (0)
#define SOC_PCR_THRESHOLD_DN_DIDT1_threshold_dn_didt1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_didt2 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_DIDT2_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_DIDT2_threshold_dn_didt2_START (0)
#define SOC_PCR_THRESHOLD_DN_DIDT2_threshold_dn_didt2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_dn_didt3 : 32;
    } reg;
} SOC_PCR_THRESHOLD_DN_DIDT3_UNION;
#endif
#define SOC_PCR_THRESHOLD_DN_DIDT3_threshold_dn_didt3_START (0)
#define SOC_PCR_THRESHOLD_DN_DIDT3_threshold_dn_didt3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_didt0 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_DIDT0_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_DIDT0_threshold_up_didt0_START (0)
#define SOC_PCR_THRESHOLD_UP_DIDT0_threshold_up_didt0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_didt1 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_DIDT1_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_DIDT1_threshold_up_didt1_START (0)
#define SOC_PCR_THRESHOLD_UP_DIDT1_threshold_up_didt1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_didt2 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_DIDT2_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_DIDT2_threshold_up_didt2_START (0)
#define SOC_PCR_THRESHOLD_UP_DIDT2_threshold_up_didt2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int threshold_up_didt3 : 32;
    } reg;
} SOC_PCR_THRESHOLD_UP_DIDT3_UNION;
#endif
#define SOC_PCR_THRESHOLD_UP_DIDT3_threshold_up_didt3_START (0)
#define SOC_PCR_THRESHOLD_UP_DIDT3_threshold_up_didt3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snapshot : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_SNAPSHOT_UNION;
#endif
#define SOC_PCR_SNAPSHOT_snapshot_START (0)
#define SOC_PCR_SNAPSHOT_snapshot_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_cur_avg_power : 32;
    } reg;
} SOC_PCR_SNAPSHOT_AVG_POWER_UNION;
#endif
#define SOC_PCR_SNAPSHOT_AVG_POWER_snap_cur_avg_power_START (0)
#define SOC_PCR_SNAPSHOT_AVG_POWER_snap_cur_avg_power_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_his_maxpower : 32;
    } reg;
} SOC_PCR_SNAPSHOT_HIS_MAXPOWER_UNION;
#endif
#define SOC_PCR_SNAPSHOT_HIS_MAXPOWER_snap_his_maxpower_START (0)
#define SOC_PCR_SNAPSHOT_HIS_MAXPOWER_snap_his_maxpower_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_cur_didt : 32;
    } reg;
} SOC_PCR_SNAPSHOT_DIDT_UNION;
#endif
#define SOC_PCR_SNAPSHOT_DIDT_snap_cur_didt_START (0)
#define SOC_PCR_SNAPSHOT_DIDT_snap_cur_didt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_his_maxdidt : 32;
    } reg;
} SOC_PCR_SNAPSHOT_HIS_MAXDIDT_UNION;
#endif
#define SOC_PCR_SNAPSHOT_HIS_MAXDIDT_snap_his_maxdidt_START (0)
#define SOC_PCR_SNAPSHOT_HIS_MAXDIDT_snap_his_maxdidt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_energy : 32;
    } reg;
} SOC_PCR_SNAPSHOT_ENERGY_UNION;
#endif
#define SOC_PCR_SNAPSHOT_ENERGY_snap_energy_START (0)
#define SOC_PCR_SNAPSHOT_ENERGY_snap_energy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int snap_acc_energy : 32;
    } reg;
} SOC_PCR_SNAPSHOT_ACC_ENERGY_UNION;
#endif
#define SOC_PCR_SNAPSHOT_ACC_ENERGY_snap_acc_energy_START (0)
#define SOC_PCR_SNAPSHOT_ACC_ENERGY_snap_acc_energy_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_sw_req : 1;
        unsigned int pcr_sw : 2;
        unsigned int reserved_0 : 1;
        unsigned int cur_stat : 3;
        unsigned int reserved_1 : 1;
        unsigned int power_stat_didt_dn : 4;
        unsigned int power_stat_didt_up : 4;
        unsigned int power_stat_avg_dn : 4;
        unsigned int power_stat_avg_up : 4;
        unsigned int reserved_2 : 8;
    } reg;
} SOC_PCR_SNAPSHOT_STATUS0_UNION;
#endif
#define SOC_PCR_SNAPSHOT_STATUS0_pcr_sw_req_START (0)
#define SOC_PCR_SNAPSHOT_STATUS0_pcr_sw_req_END (0)
#define SOC_PCR_SNAPSHOT_STATUS0_pcr_sw_START (1)
#define SOC_PCR_SNAPSHOT_STATUS0_pcr_sw_END (2)
#define SOC_PCR_SNAPSHOT_STATUS0_cur_stat_START (4)
#define SOC_PCR_SNAPSHOT_STATUS0_cur_stat_END (6)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_didt_dn_START (8)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_didt_dn_END (11)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_didt_up_START (12)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_didt_up_END (15)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_avg_dn_START (16)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_avg_dn_END (19)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_avg_up_START (20)
#define SOC_PCR_SNAPSHOT_STATUS0_power_stat_avg_up_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int reserved : 32;
    } reg;
} SOC_PCR_SNAPSHOT_STATUS1_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int clr_his_maxpower : 1;
        unsigned int clr_his_maxdidt : 1;
        unsigned int clr_freq_dn_trig_cnt : 1;
        unsigned int reserved : 29;
    } reg;
} SOC_PCR_DEBUG_CLR_UNION;
#endif
#define SOC_PCR_DEBUG_CLR_clr_his_maxpower_START (0)
#define SOC_PCR_DEBUG_CLR_clr_his_maxpower_END (0)
#define SOC_PCR_DEBUG_CLR_clr_his_maxdidt_START (1)
#define SOC_PCR_DEBUG_CLR_clr_his_maxdidt_END (1)
#define SOC_PCR_DEBUG_CLR_clr_freq_dn_trig_cnt_START (2)
#define SOC_PCR_DEBUG_CLR_clr_freq_dn_trig_cnt_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int freq_dn0_trig_cnt : 32;
    } reg;
} SOC_PCR_FREQ_DN_TRIGGER_CNT0_UNION;
#endif
#define SOC_PCR_FREQ_DN_TRIGGER_CNT0_freq_dn0_trig_cnt_START (0)
#define SOC_PCR_FREQ_DN_TRIGGER_CNT0_freq_dn0_trig_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int freq_dn1_trig_cnt : 32;
    } reg;
} SOC_PCR_FREQ_DN_TRIGGER_CNT1_UNION;
#endif
#define SOC_PCR_FREQ_DN_TRIGGER_CNT1_freq_dn1_trig_cnt_START (0)
#define SOC_PCR_FREQ_DN_TRIGGER_CNT1_freq_dn1_trig_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int freq_dn2_trig_cnt : 32;
    } reg;
} SOC_PCR_FREQ_DN_TRIGGER_CNT2_UNION;
#endif
#define SOC_PCR_FREQ_DN_TRIGGER_CNT2_freq_dn2_trig_cnt_START (0)
#define SOC_PCR_FREQ_DN_TRIGGER_CNT2_freq_dn2_trig_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int freq_dn3_trig_cnt : 32;
    } reg;
} SOC_PCR_FREQ_DN_TRIGGER_CNT3_UNION;
#endif
#define SOC_PCR_FREQ_DN_TRIGGER_CNT3_freq_dn3_trig_cnt_START (0)
#define SOC_PCR_FREQ_DN_TRIGGER_CNT3_freq_dn3_trig_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_intr_stat : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_INTR_STAT_UNION;
#endif
#define SOC_PCR_INTR_STAT_pcr_intr_stat_START (0)
#define SOC_PCR_INTR_STAT_pcr_intr_stat_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_intr_raw_stat : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_INTR_RAW_STAT_UNION;
#endif
#define SOC_PCR_INTR_RAW_STAT_pcr_intr_raw_stat_START (0)
#define SOC_PCR_INTR_RAW_STAT_pcr_intr_raw_stat_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_intr_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_INTR_EN_UNION;
#endif
#define SOC_PCR_INTR_EN_pcr_intr_en_START (0)
#define SOC_PCR_INTR_EN_pcr_intr_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int pcr_intr_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_PCR_INTR_CLR_UNION;
#endif
#define SOC_PCR_INTR_CLR_pcr_intr_clr_START (0)
#define SOC_PCR_INTR_CLR_pcr_intr_clr_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
