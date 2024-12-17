#ifndef __SOC_DP_MONITOR_INTERFACE_H__
#define __SOC_DP_MONITOR_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_DP_MONITOR_DPM_CTRL_EN_ADDR(base) ((base) + (0x000UL))
#define SOC_DP_MONITOR_SDPM_ENABLE_ADDR(base) ((base) + (0x004UL))
#define SOC_DP_MONITOR_SG_DDPM_EN_ADDR(base) ((base) + (0x008UL))
#define SOC_DP_MONITOR_DPM_ENABLE_ADDR(base) ((base) + (0x00CUL))
#define SOC_DP_MONITOR_SHIFT_BIT_ADDR(base) ((base) + (0x010UL))
#define SOC_DP_MONITOR_VOLT_ADDR(base) ((base) + (0x014UL))
#define SOC_DP_MONITOR_SOFT_PULSE_ADDR(base) ((base) + (0x01CUL))
#define SOC_DP_MONITOR_VOLT_GEAR_ADDR(base,VOLT_GEAR) ((base) + (0x020+0x4*(VOLT_GEAR)))
#define SOC_DP_MONITOR_STA_VOLT_PARAM_ADDR(base,FX1_NUM) ((base) + (0x040+0x4*(FX1_NUM)))
#define SOC_DP_MONITOR_FX2_CONST_ADDR(base,FX2_VAR_NUM) ((base) + (0x080+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_FX2_VAR1_ADDR(base,FX2_VAR_NUM) ((base) + (0x0A0+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_FX2_VAR2_ADDR(base,FX2_VAR_NUM) ((base) + (0x0C0+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_SIG_POWER_PARAM_ADDR(base,P_NUM) ((base) + (0x0E0+0x4*(P_NUM)))
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_ADDR(base,DPM_NUM) ((base) + (0x260+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_FX2_RESULT_SNAP_ADDR(base,DPM_NUM) ((base) + (0x2B0+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_FINAL_STA_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x300+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_ACC_DPM_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x340+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_ACC_DYN_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x380+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_DVFS_CFG_APB_ADDR(base) ((base) + (0x3C0UL))
#define SOC_DP_MONITOR_VOLT_CODE_APB_ADDR(base) ((base) + (0x400UL))
#define SOC_DP_MONITOR_CPU_COUNTER_ADDR(base,THREAD_NUM) ((base) + (0x440+0x4*(THREAD_NUM)))
#define SOC_DP_MONITOR_RSM_ID_SEL_READ_ADDR(base) ((base) + (0x4D0UL))
#define SOC_DP_MONITOR_DPM_COUNTER_ADDR(base,RSM_SIG_NUM) ((base) + (0x500+0x4*(RSM_SIG_NUM)))
#define SOC_DP_MONITOR_SORT_THD_ADDR(base) ((base) + (0x800UL))
#define SOC_DP_MONITOR_SORT_MAX_ADDR(base) ((base) + (0x804UL))
#define SOC_DP_MONITOR_SORT_MULTI_WINDOW_ADDR(base) ((base) + (0x808UL))
#define SOC_DP_MONITOR_SORT_REP_WINDOW_ADDR(base) ((base) + (0x81CUL))
#define SOC_DP_MONITOR_ALL_RESULT_SHIFT_ADDR(base) ((base) + (0x820UL))
#define SOC_DP_MONITOR_SORT_PARA_ADDR(base) ((base) + (0x824UL))
#define SOC_DP_MONITOR_MEM_CTRL_ADDR(base) ((base) + (0x828UL))
#define SOC_DP_MONITOR_IDLE_THD_ADDR(base) ((base) + (0x82CUL))
#define SOC_DP_MONITOR_SORT_NUM_ADDR(base) ((base) + (0x830UL))
#define SOC_DP_MONITOR_INTR_CLR_REG_ADDR(base) ((base) + (0x840UL))
#define SOC_DP_MONITOR_SORT_REP_DATA_ADDR(base,DPM_NUM) ((base) + (0x880+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_ADDR(base,DPM_NUM) ((base) + (0x8c0+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_ADDR(base,DPM_NUM) ((base) + (0x900+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_HECA_INTR_ADDR(base,DPM_NUM) ((base) + (0x940+04*(DPM_NUM)))
#else
#define SOC_DP_MONITOR_DPM_CTRL_EN_ADDR(base) ((base) + (0x000))
#define SOC_DP_MONITOR_SDPM_ENABLE_ADDR(base) ((base) + (0x004))
#define SOC_DP_MONITOR_SG_DDPM_EN_ADDR(base) ((base) + (0x008))
#define SOC_DP_MONITOR_DPM_ENABLE_ADDR(base) ((base) + (0x00C))
#define SOC_DP_MONITOR_SHIFT_BIT_ADDR(base) ((base) + (0x010))
#define SOC_DP_MONITOR_VOLT_ADDR(base) ((base) + (0x014))
#define SOC_DP_MONITOR_SOFT_PULSE_ADDR(base) ((base) + (0x01C))
#define SOC_DP_MONITOR_VOLT_GEAR_ADDR(base,VOLT_GEAR) ((base) + (0x020+0x4*(VOLT_GEAR)))
#define SOC_DP_MONITOR_STA_VOLT_PARAM_ADDR(base,FX1_NUM) ((base) + (0x040+0x4*(FX1_NUM)))
#define SOC_DP_MONITOR_FX2_CONST_ADDR(base,FX2_VAR_NUM) ((base) + (0x080+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_FX2_VAR1_ADDR(base,FX2_VAR_NUM) ((base) + (0x0A0+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_FX2_VAR2_ADDR(base,FX2_VAR_NUM) ((base) + (0x0C0+0x4*(FX2_VAR_NUM)))
#define SOC_DP_MONITOR_SIG_POWER_PARAM_ADDR(base,P_NUM) ((base) + (0x0E0+0x4*(P_NUM)))
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_ADDR(base,DPM_NUM) ((base) + (0x260+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_FX2_RESULT_SNAP_ADDR(base,DPM_NUM) ((base) + (0x2B0+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_FINAL_STA_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x300+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_ACC_DPM_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x340+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_ACC_DYN_ENERGY_SNAP_ADDR(base,DPM_NUM) ((base) + (0x380+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_DVFS_CFG_APB_ADDR(base) ((base) + (0x3C0))
#define SOC_DP_MONITOR_VOLT_CODE_APB_ADDR(base) ((base) + (0x400))
#define SOC_DP_MONITOR_CPU_COUNTER_ADDR(base,THREAD_NUM) ((base) + (0x440+0x4*(THREAD_NUM)))
#define SOC_DP_MONITOR_RSM_ID_SEL_READ_ADDR(base) ((base) + (0x4D0))
#define SOC_DP_MONITOR_DPM_COUNTER_ADDR(base,RSM_SIG_NUM) ((base) + (0x500+0x4*(RSM_SIG_NUM)))
#define SOC_DP_MONITOR_SORT_THD_ADDR(base) ((base) + (0x800))
#define SOC_DP_MONITOR_SORT_MAX_ADDR(base) ((base) + (0x804))
#define SOC_DP_MONITOR_SORT_MULTI_WINDOW_ADDR(base) ((base) + (0x808))
#define SOC_DP_MONITOR_SORT_REP_WINDOW_ADDR(base) ((base) + (0x81C))
#define SOC_DP_MONITOR_ALL_RESULT_SHIFT_ADDR(base) ((base) + (0x820))
#define SOC_DP_MONITOR_SORT_PARA_ADDR(base) ((base) + (0x824))
#define SOC_DP_MONITOR_MEM_CTRL_ADDR(base) ((base) + (0x828))
#define SOC_DP_MONITOR_IDLE_THD_ADDR(base) ((base) + (0x82C))
#define SOC_DP_MONITOR_SORT_NUM_ADDR(base) ((base) + (0x830))
#define SOC_DP_MONITOR_INTR_CLR_REG_ADDR(base) ((base) + (0x840))
#define SOC_DP_MONITOR_SORT_REP_DATA_ADDR(base,DPM_NUM) ((base) + (0x880+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_ADDR(base,DPM_NUM) ((base) + (0x8c0+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_ADDR(base,DPM_NUM) ((base) + (0x900+0x4*(DPM_NUM)))
#define SOC_DP_MONITOR_HECA_INTR_ADDR(base,DPM_NUM) ((base) + (0x940+04*(DPM_NUM)))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rs_cg_bypass : 1;
        unsigned int sync_cg_off : 1;
        unsigned int apb_read_cnt_en : 1;
        unsigned int heca_wr_mem_ce : 1;
        unsigned int dpm_counter_en : 1;
        unsigned int sdpm_only_en : 1;
        unsigned int dvfs_cfg_hard_en : 1;
        unsigned int heca_clk_en : 1;
        unsigned int heca_dyn_cfg_update : 1;
        unsigned int reserved_0 : 7;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_DP_MONITOR_DPM_CTRL_EN_UNION;
#endif
#define SOC_DP_MONITOR_DPM_CTRL_EN_rs_cg_bypass_START (0)
#define SOC_DP_MONITOR_DPM_CTRL_EN_rs_cg_bypass_END (0)
#define SOC_DP_MONITOR_DPM_CTRL_EN_sync_cg_off_START (1)
#define SOC_DP_MONITOR_DPM_CTRL_EN_sync_cg_off_END (1)
#define SOC_DP_MONITOR_DPM_CTRL_EN_apb_read_cnt_en_START (2)
#define SOC_DP_MONITOR_DPM_CTRL_EN_apb_read_cnt_en_END (2)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_wr_mem_ce_START (3)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_wr_mem_ce_END (3)
#define SOC_DP_MONITOR_DPM_CTRL_EN_dpm_counter_en_START (4)
#define SOC_DP_MONITOR_DPM_CTRL_EN_dpm_counter_en_END (4)
#define SOC_DP_MONITOR_DPM_CTRL_EN_sdpm_only_en_START (5)
#define SOC_DP_MONITOR_DPM_CTRL_EN_sdpm_only_en_END (5)
#define SOC_DP_MONITOR_DPM_CTRL_EN_dvfs_cfg_hard_en_START (6)
#define SOC_DP_MONITOR_DPM_CTRL_EN_dvfs_cfg_hard_en_END (6)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_clk_en_START (7)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_clk_en_END (7)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_dyn_cfg_update_START (8)
#define SOC_DP_MONITOR_DPM_CTRL_EN_heca_dyn_cfg_update_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sdpm_enable : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_DP_MONITOR_SDPM_ENABLE_UNION;
#endif
#define SOC_DP_MONITOR_SDPM_ENABLE_sdpm_enable_START (0)
#define SOC_DP_MONITOR_SDPM_ENABLE_sdpm_enable_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sg_ddpm_en : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DP_MONITOR_SG_DDPM_EN_UNION;
#endif
#define SOC_DP_MONITOR_SG_DDPM_EN_sg_ddpm_en_START (0)
#define SOC_DP_MONITOR_SG_DDPM_EN_sg_ddpm_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int monitor_ctrl_en : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_DP_MONITOR_DPM_ENABLE_UNION;
#endif
#define SOC_DP_MONITOR_DPM_ENABLE_monitor_ctrl_en_START (0)
#define SOC_DP_MONITOR_DPM_ENABLE_monitor_ctrl_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cfg_cnt_init_dyn : 9;
        unsigned int dyn_shift_bit : 4;
        unsigned int sdpm_shift_bit : 5;
        unsigned int volt_temp_var_shift : 4;
        unsigned int cfg_cnt_init_sta : 9;
        unsigned int reserved : 1;
    } reg;
} SOC_DP_MONITOR_SHIFT_BIT_UNION;
#endif
#define SOC_DP_MONITOR_SHIFT_BIT_cfg_cnt_init_dyn_START (0)
#define SOC_DP_MONITOR_SHIFT_BIT_cfg_cnt_init_dyn_END (8)
#define SOC_DP_MONITOR_SHIFT_BIT_dyn_shift_bit_START (9)
#define SOC_DP_MONITOR_SHIFT_BIT_dyn_shift_bit_END (12)
#define SOC_DP_MONITOR_SHIFT_BIT_sdpm_shift_bit_START (13)
#define SOC_DP_MONITOR_SHIFT_BIT_sdpm_shift_bit_END (17)
#define SOC_DP_MONITOR_SHIFT_BIT_volt_temp_var_shift_START (18)
#define SOC_DP_MONITOR_SHIFT_BIT_volt_temp_var_shift_END (21)
#define SOC_DP_MONITOR_SHIFT_BIT_cfg_cnt_init_sta_START (22)
#define SOC_DP_MONITOR_SHIFT_BIT_cfg_cnt_init_sta_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int volt_param_a : 4;
        unsigned int volt_param_b : 10;
        unsigned int reserved : 18;
    } reg;
} SOC_DP_MONITOR_VOLT_UNION;
#endif
#define SOC_DP_MONITOR_VOLT_volt_param_a_START (0)
#define SOC_DP_MONITOR_VOLT_volt_param_a_END (3)
#define SOC_DP_MONITOR_VOLT_volt_param_b_START (4)
#define SOC_DP_MONITOR_VOLT_volt_param_b_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int soft_cnt_pulse : 1;
        unsigned int soft_energy_pulse : 1;
        unsigned int snapshot_soft_pulse : 1;
        unsigned int freq_update_soft_pulse : 1;
        unsigned int volt_update_soft_pulse : 1;
        unsigned int fsm_update_soft_pulse : 1;
        unsigned int soft_cpu_cnt_pulse : 1;
        unsigned int reserved : 25;
    } reg;
} SOC_DP_MONITOR_SOFT_PULSE_UNION;
#endif
#define SOC_DP_MONITOR_SOFT_PULSE_soft_cnt_pulse_START (0)
#define SOC_DP_MONITOR_SOFT_PULSE_soft_cnt_pulse_END (0)
#define SOC_DP_MONITOR_SOFT_PULSE_soft_energy_pulse_START (1)
#define SOC_DP_MONITOR_SOFT_PULSE_soft_energy_pulse_END (1)
#define SOC_DP_MONITOR_SOFT_PULSE_snapshot_soft_pulse_START (2)
#define SOC_DP_MONITOR_SOFT_PULSE_snapshot_soft_pulse_END (2)
#define SOC_DP_MONITOR_SOFT_PULSE_freq_update_soft_pulse_START (3)
#define SOC_DP_MONITOR_SOFT_PULSE_freq_update_soft_pulse_END (3)
#define SOC_DP_MONITOR_SOFT_PULSE_volt_update_soft_pulse_START (4)
#define SOC_DP_MONITOR_SOFT_PULSE_volt_update_soft_pulse_END (4)
#define SOC_DP_MONITOR_SOFT_PULSE_fsm_update_soft_pulse_START (5)
#define SOC_DP_MONITOR_SOFT_PULSE_fsm_update_soft_pulse_END (5)
#define SOC_DP_MONITOR_SOFT_PULSE_soft_cpu_cnt_pulse_START (6)
#define SOC_DP_MONITOR_SOFT_PULSE_soft_cpu_cnt_pulse_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int volt_gear : 32;
    } reg;
} SOC_DP_MONITOR_VOLT_GEAR_UNION;
#endif
#define SOC_DP_MONITOR_VOLT_GEAR_volt_gear_START (0)
#define SOC_DP_MONITOR_VOLT_GEAR_volt_gear_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sta_volt_param_apb : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_DP_MONITOR_STA_VOLT_PARAM_UNION;
#endif
#define SOC_DP_MONITOR_STA_VOLT_PARAM_sta_volt_param_apb_START (0)
#define SOC_DP_MONITOR_STA_VOLT_PARAM_sta_volt_param_apb_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fx2_const_apb : 32;
    } reg;
} SOC_DP_MONITOR_FX2_CONST_UNION;
#endif
#define SOC_DP_MONITOR_FX2_CONST_fx2_const_apb_START (0)
#define SOC_DP_MONITOR_FX2_CONST_fx2_const_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fx2_var1_apb : 32;
    } reg;
} SOC_DP_MONITOR_FX2_VAR1_UNION;
#endif
#define SOC_DP_MONITOR_FX2_VAR1_fx2_var1_apb_START (0)
#define SOC_DP_MONITOR_FX2_VAR1_fx2_var1_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fx2_var2_apb : 32;
    } reg;
} SOC_DP_MONITOR_FX2_VAR2_UNION;
#endif
#define SOC_DP_MONITOR_FX2_VAR2_fx2_var2_apb_START (0)
#define SOC_DP_MONITOR_FX2_VAR2_fx2_var2_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sig_power_param : 32;
    } reg;
} SOC_DP_MONITOR_SIG_POWER_PARAM_UNION;
#endif
#define SOC_DP_MONITOR_SIG_POWER_PARAM_sig_power_param_START (0)
#define SOC_DP_MONITOR_SIG_POWER_PARAM_sig_power_param_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cur_volt_snap : 11;
        unsigned int cur_temp_snap : 9;
        unsigned int reserved : 12;
    } reg;
} SOC_DP_MONITOR_VOLT_TEMP_SNAP_UNION;
#endif
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_cur_volt_snap_START (0)
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_cur_volt_snap_END (10)
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_cur_temp_snap_START (11)
#define SOC_DP_MONITOR_VOLT_TEMP_SNAP_cur_temp_snap_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int fx2_result_snap : 22;
        unsigned int reserved : 10;
    } reg;
} SOC_DP_MONITOR_FX2_RESULT_SNAP_UNION;
#endif
#define SOC_DP_MONITOR_FX2_RESULT_SNAP_fx2_result_snap_START (0)
#define SOC_DP_MONITOR_FX2_RESULT_SNAP_fx2_result_snap_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int final_sta_energy_snap : 32;
    } reg;
} SOC_DP_MONITOR_FINAL_STA_ENERGY_SNAP_UNION;
#endif
#define SOC_DP_MONITOR_FINAL_STA_ENERGY_SNAP_final_sta_energy_snap_START (0)
#define SOC_DP_MONITOR_FINAL_STA_ENERGY_SNAP_final_sta_energy_snap_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int acc_dpm_energy_apb : 32;
    } reg;
} SOC_DP_MONITOR_ACC_DPM_ENERGY_SNAP_UNION;
#endif
#define SOC_DP_MONITOR_ACC_DPM_ENERGY_SNAP_acc_dpm_energy_apb_START (0)
#define SOC_DP_MONITOR_ACC_DPM_ENERGY_SNAP_acc_dpm_energy_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int acc_dyn_energy_apb : 32;
    } reg;
} SOC_DP_MONITOR_ACC_DYN_ENERGY_SNAP_UNION;
#endif
#define SOC_DP_MONITOR_ACC_DYN_ENERGY_SNAP_acc_dyn_energy_apb_START (0)
#define SOC_DP_MONITOR_ACC_DYN_ENERGY_SNAP_acc_dyn_energy_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dvfs_cfg_apb : 30;
        unsigned int reserved : 2;
    } reg;
} SOC_DP_MONITOR_DVFS_CFG_APB_UNION;
#endif
#define SOC_DP_MONITOR_DVFS_CFG_APB_dvfs_cfg_apb_START (0)
#define SOC_DP_MONITOR_DVFS_CFG_APB_dvfs_cfg_apb_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int volt_code_apb : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DP_MONITOR_VOLT_CODE_APB_UNION;
#endif
#define SOC_DP_MONITOR_VOLT_CODE_APB_volt_code_apb_START (0)
#define SOC_DP_MONITOR_VOLT_CODE_APB_volt_code_apb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int cpu_cnt_apb : 32;
    } reg;
} SOC_DP_MONITOR_CPU_COUNTER_UNION;
#endif
#define SOC_DP_MONITOR_CPU_COUNTER_cpu_cnt_apb_START (0)
#define SOC_DP_MONITOR_CPU_COUNTER_cpu_cnt_apb_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int rsm_id_sel_read : 3;
        unsigned int reserved : 29;
    } reg;
} SOC_DP_MONITOR_RSM_ID_SEL_READ_UNION;
#endif
#define SOC_DP_MONITOR_RSM_ID_SEL_READ_rsm_id_sel_read_START (0)
#define SOC_DP_MONITOR_RSM_ID_SEL_READ_rsm_id_sel_read_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int dpm_counter : 32;
    } reg;
} SOC_DP_MONITOR_DPM_COUNTER_UNION;
#endif
#define SOC_DP_MONITOR_DPM_COUNTER_dpm_counter_START (0)
#define SOC_DP_MONITOR_DPM_COUNTER_dpm_counter_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_thd : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_DP_MONITOR_SORT_THD_UNION;
#endif
#define SOC_DP_MONITOR_SORT_THD_sort_thd_START (0)
#define SOC_DP_MONITOR_SORT_THD_sort_thd_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_max : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_DP_MONITOR_SORT_MAX_UNION;
#endif
#define SOC_DP_MONITOR_SORT_MAX_sort_max_START (0)
#define SOC_DP_MONITOR_SORT_MAX_sort_max_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_multi_window : 20;
        unsigned int reserved : 12;
    } reg;
} SOC_DP_MONITOR_SORT_MULTI_WINDOW_UNION;
#endif
#define SOC_DP_MONITOR_SORT_MULTI_WINDOW_sort_multi_window_START (0)
#define SOC_DP_MONITOR_SORT_MULTI_WINDOW_sort_multi_window_END (19)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_rep_window : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_DP_MONITOR_SORT_REP_WINDOW_UNION;
#endif
#define SOC_DP_MONITOR_SORT_REP_WINDOW_sort_rep_window_START (0)
#define SOC_DP_MONITOR_SORT_REP_WINDOW_sort_rep_window_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int all_result_shift : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_DP_MONITOR_ALL_RESULT_SHIFT_UNION;
#endif
#define SOC_DP_MONITOR_ALL_RESULT_SHIFT_all_result_shift_START (0)
#define SOC_DP_MONITOR_ALL_RESULT_SHIFT_all_result_shift_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_para_0 : 8;
        unsigned int sort_para_1 : 8;
        unsigned int sort_para_2 : 8;
        unsigned int sort_para_3 : 8;
    } reg;
} SOC_DP_MONITOR_SORT_PARA_UNION;
#endif
#define SOC_DP_MONITOR_SORT_PARA_sort_para_0_START (0)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_0_END (7)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_1_START (8)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_1_END (15)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_2_START (16)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_2_END (23)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_3_START (24)
#define SOC_DP_MONITOR_SORT_PARA_sort_para_3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int mem_ctrl : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_DP_MONITOR_MEM_CTRL_UNION;
#endif
#define SOC_DP_MONITOR_MEM_CTRL_mem_ctrl_START (0)
#define SOC_DP_MONITOR_MEM_CTRL_mem_ctrl_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int idle_thd : 32;
    } reg;
} SOC_DP_MONITOR_IDLE_THD_UNION;
#endif
#define SOC_DP_MONITOR_IDLE_THD_idle_thd_START (0)
#define SOC_DP_MONITOR_IDLE_THD_idle_thd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sort_num : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_DP_MONITOR_SORT_NUM_UNION;
#endif
#define SOC_DP_MONITOR_SORT_NUM_sort_num_START (0)
#define SOC_DP_MONITOR_SORT_NUM_sort_num_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int intr_clr_reg : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DP_MONITOR_INTR_CLR_REG_UNION;
#endif
#define SOC_DP_MONITOR_INTR_CLR_REG_intr_clr_reg_START (0)
#define SOC_DP_MONITOR_INTR_CLR_REG_intr_clr_reg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int idle_cnter : 8;
        unsigned int top1_cnter : 8;
        unsigned int top1_index : 4;
        unsigned int top2_cnter : 8;
        unsigned int top2_index : 4;
    } reg;
} SOC_DP_MONITOR_SORT_REP_DATA_UNION;
#endif
#define SOC_DP_MONITOR_SORT_REP_DATA_idle_cnter_START (0)
#define SOC_DP_MONITOR_SORT_REP_DATA_idle_cnter_END (7)
#define SOC_DP_MONITOR_SORT_REP_DATA_top1_cnter_START (8)
#define SOC_DP_MONITOR_SORT_REP_DATA_top1_cnter_END (15)
#define SOC_DP_MONITOR_SORT_REP_DATA_top1_index_START (16)
#define SOC_DP_MONITOR_SORT_REP_DATA_top1_index_END (19)
#define SOC_DP_MONITOR_SORT_REP_DATA_top2_cnter_START (20)
#define SOC_DP_MONITOR_SORT_REP_DATA_top2_cnter_END (27)
#define SOC_DP_MONITOR_SORT_REP_DATA_top2_index_START (28)
#define SOC_DP_MONITOR_SORT_REP_DATA_top2_index_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int top1_cnter : 12;
        unsigned int top1_index : 4;
        unsigned int idle_cnter : 12;
        unsigned int reserved : 4;
    } reg;
} SOC_DP_MONITOR_SORT_REP_DATA_TOP1_UNION;
#endif
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_top1_cnter_START (0)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_top1_cnter_END (11)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_top1_index_START (12)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_top1_index_END (15)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_idle_cnter_START (16)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP1_idle_cnter_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int top2_cnter : 12;
        unsigned int top2_index : 4;
        unsigned int idle_cnter : 12;
        unsigned int reserved : 4;
    } reg;
} SOC_DP_MONITOR_SORT_REP_DATA_TOP2_UNION;
#endif
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_top2_cnter_START (0)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_top2_cnter_END (11)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_top2_index_START (12)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_top2_index_END (15)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_idle_cnter_START (16)
#define SOC_DP_MONITOR_SORT_REP_DATA_TOP2_idle_cnter_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int heca_intr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DP_MONITOR_HECA_INTR_UNION;
#endif
#define SOC_DP_MONITOR_HECA_INTR_heca_intr_START (0)
#define SOC_DP_MONITOR_HECA_INTR_heca_intr_END (0)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
