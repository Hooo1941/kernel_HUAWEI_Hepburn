#ifndef _LPM_RUNTIME_OFFSET_H_
#define _LPM_RUNTIME_OFFSET_H_ 
#ifndef RUNTIME_VAR_BASE
#define RUNTIME_VAR_BASE (0)
#define RUNTIME_VAR_SIZE 0x2E0
#endif
#define RUNTIME_CPU_VAR_ADDR RUNTIME_VAR_BASE
#define RUNTIME_CPU_VAR_SIZE 0x40U
#define RUNTIME_GPU_VAR_ADDR (RUNTIME_CPU_VAR_ADDR + RUNTIME_CPU_VAR_SIZE)
#define RUNTIME_GPU_VAR_SIZE 0x20U
#define RUNTIME_TMP_VAR_ADDR (RUNTIME_GPU_VAR_ADDR + RUNTIME_GPU_VAR_SIZE)
#define RUNTIME_TMP_VAR_SIZE (0x20U)
#define RUNTIME_GEN_VAR_ADDR (RUNTIME_TMP_VAR_ADDR + RUNTIME_TMP_VAR_SIZE)
#define RUNTIME_GEN_VAR_SIZE 0x20U
#define RUNTIME_PERIL_VAR_ADDR (RUNTIME_GEN_VAR_ADDR + RUNTIME_GEN_VAR_SIZE)
#define RUNTIME_PERIL_VAR_SIZE (0x20U)
#define RUNTIME_DDR_VAR_ADDR (RUNTIME_PERIL_VAR_ADDR + RUNTIME_PERIL_VAR_SIZE)
#define RUNTIME_DDR_VAR_SIZE 0x40
#define RUNTIME_SHARE_MEM_END_ADDR (RUNTIME_DDR_VAR_ADDR + RUNTIME_DDR_VAR_SIZE)
#define RUNTIME_LPMCU_RUN_VAR_ADDR RUNTIME_SHARE_MEM_END_ADDR
#define RUNTIME_LPMCU_RUN_VAR_SIZE 0x50
#define RUNTIME_SR_VAR_BASE_ADDR (RUNTIME_LPMCU_RUN_VAR_ADDR + RUNTIME_LPMCU_RUN_VAR_SIZE)
#define RUNTIME_SR_VAR_BASE_SIZE (RUNTIME_VAR_SIZE - (RUNTIME_SR_VAR_BASE_ADDR - RUNTIME_VAR_BASE))
#define RUNTIME_WAKE_STATUS_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x000)
#define WAKE_STATUS (WORD_REF(RUNTIME_WAKE_STATUS_ADDR))
#define RUNTIME_DEEP_SLEEP_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x004)
#define SYS_DSLEEP_CNT (WORD_REF(RUNTIME_DEEP_SLEEP_CNT_ADDR))
#define RUNTIME_DEEP_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x008)
#define SYS_DWAKE_CNT (WORD_REF(RUNTIME_DEEP_WAKE_CNT_ADDR))
#define RUNTIME_SLEEP_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x00C)
#define SYS_SLEEP_CNT (WORD_REF(RUNTIME_SLEEP_CNT_ADDR))
#define RUNTIME_SLEEP_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x010)
#define SYS_SLEEP_WAKE_CNT (WORD_REF(RUNTIME_SLEEP_WAKE_CNT_ADDR))
#define RUNTIME_SLOW_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x014)
#define SYS_SLOW_CNT (WORD_REF(RUNTIME_SLOW_CNT_ADDR))
#define RUNTIME_SLOW_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x018)
#define SYS_SLOW_WAKE_CNT (WORD_REF(RUNTIME_SLOW_WAKE_CNT_ADDR))
#define RUNTIME_LIGHT_SLEEP_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x01C)
#define LIGHT_SLEEP_CNT (WORD_REF(RUNTIME_LIGHT_SLEEP_CNT_ADDR))
#define RUNTIME_LIGHT_SLEEP_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x020)
#define LIGHT_SLEEP_RESUME_CNT (WORD_REF(RUNTIME_LIGHT_SLEEP_RESUME_CNT_ADDR))
#define RUNTIME_AP_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x024)
#define AP_WAKE_CNT (WORD_REF(RUNTIME_AP_WAKE_CNT_ADDR))
#define RUNTIME_HIFI_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x028)
#define HIFI_WAKE_CNT (WORD_REF(RUNTIME_HIFI_WAKE_CNT_ADDR))
#define RUNTIME_MODEM_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x02C)
#define MODEM_WAKE_CNT (WORD_REF(RUNTIME_MODEM_WAKE_CNT_ADDR))
#define RUNTIME_IOMCU_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x030)
#define IOM3_WAKE_CNT (WORD_REF(RUNTIME_IOMCU_WAKE_CNT_ADDR))
#define RUNTIME_LPMCU_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x034)
#define LPM3_WAKE_CNT (WORD_REF(RUNTIME_LPMCU_WAKE_CNT_ADDR))
#define RUNTIME_GENERAL_SEE_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x038)
#define GENERAL_SEE_WAKE_CNT (WORD_REF(RUNTIME_GENERAL_SEE_WAKE_CNT_ADDR))
#define RUNTIME_AP_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x03C)
#define AP_SUSPEND_CNT (WORD_REF(RUNTIME_AP_SUSPEND_CNT_ADDR))
#define RUNTIME_AP_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x040)
#define AP_RESUME_CNT (WORD_REF(RUNTIME_AP_RESUME_CNT_ADDR))
#define RUNTIME_MODEM_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x044)
#define MODEM_SUSPEND_CNT (WORD_REF(RUNTIME_MODEM_SUSPEND_CNT_ADDR))
#define RUNTIME_MODEM_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x048)
#define MODEM_RESUME_CNT (WORD_REF(RUNTIME_MODEM_RESUME_CNT_ADDR))
#define RUNTIME_HIFI_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x04C)
#define HIFI_SUSPEND_CNT (WORD_REF(RUNTIME_HIFI_SUSPEND_CNT_ADDR))
#define RUNTIME_HIFI_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x050)
#define HIFI_RESUME_CNT (WORD_REF(RUNTIME_HIFI_RESUME_CNT_ADDR))
#define RUNTIME_IOMCU_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x054)
#define IOM3_SUSPEND_CNT (WORD_REF(RUNTIME_IOMCU_SUSPEND_CNT_ADDR))
#define RUNTIME_IOMCU_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x058)
#define IOM3_RESUME_CNT (WORD_REF(RUNTIME_IOMCU_RESUME_CNT_ADDR))
#define RUNTIME_GENERAL_SEE_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x05C)
#define GENERAL_SEE_SUSPEND_CNT (WORD_REF(RUNTIME_GENERAL_SEE_SUSPEND_CNT_ADDR))
#define RUNTIME_GENERAL_SEE_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x060)
#define GENERAL_SEE_RESUME_CNT (WORD_REF(RUNTIME_GENERAL_SEE_RESUME_CNT_ADDR))
#define RUNTIME_LIGHTSLEEP_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x064)
#define LIGHTSLEEP_WAKE_IRQ (WORD_REF(RUNTIME_LIGHTSLEEP_WAKE_IRQ_ADDR))
#define RUNTIME_LIGHTSLEEP_WAKE_IRQ1_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x068)
#define LIGHTSLEEP_WAKE_IRQ1 (WORD_REF(RUNTIME_LIGHTSLEEP_WAKE_IRQ1_ADDR))
#define RUNTIME_SLOW_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x06C)
#define SLOW_WAKE_IRQ (WORD_REF(RUNTIME_SLOW_WAKE_IRQ_ADDR))
#define RUNTIME_SLOW_WAKE_IRQ1_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x070)
#define SLOW_WAKE_IRQ1 (WORD_REF(RUNTIME_SLOW_WAKE_IRQ1_ADDR))
#define RUNTIME_SLEEP_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x074)
#define SLEEP_WAKE_IRQ (WORD_REF(RUNTIME_SLEEP_WAKE_IRQ_ADDR))
#define RUNTIME_SLEEP_WAKE_IRQ1_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x078)
#define SLEEP_WAKE_IRQ1 (WORD_REF(RUNTIME_SLEEP_WAKE_IRQ1_ADDR))
#define RUNTIME_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x07C)
#define WAKE_IRQ (WORD_REF(RUNTIME_WAKE_IRQ_ADDR))
#define RUNTIME_WAKE_IRQ1_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x080)
#define WAKE_IRQ1 (WORD_REF(RUNTIME_WAKE_IRQ1_ADDR))
#define RUNTIME_WAKE_IRQ2_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x084)
#define WAKE_IRQ2 (WORD_REF(RUNTIME_WAKE_IRQ2_ADDR))
#define RUNTIME_WAKE_IRQ3_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x088)
#define WAKE_IRQ3 (WORD_REF(RUNTIME_WAKE_IRQ3_ADDR))
#define RUNTIME_AP_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x08c)
#define AP_WAKE_IRQ (WORD_REF(RUNTIME_AP_WAKE_IRQ_ADDR))
#define RUNTIME_MODEM_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x090)
#define MODEM_WAKE_IRQ (WORD_REF(RUNTIME_MODEM_WAKE_IRQ_ADDR))
#define RUNTIME_HIFI_WAKE_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x094)
#define HIFI_WAKE_IRQ (WORD_REF(RUNTIME_HIFI_WAKE_IRQ_ADDR))
#define RUNTIME_AP_NSIPC_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x098)
#define AP_NSIPC_IRQ (WORD_REF(RUNTIME_AP_NSIPC_IRQ_ADDR))
#define RUNTIME_AP_AO_NSIPC_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x09c)
#define AP_AO_NSIPC_IRQ (WORD_REF(RUNTIME_AP_AO_NSIPC_IRQ_ADDR))
#define RUNTIME_GIC_OUT_IRQ_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x0a0)
#define GIC_OUT_IRQ (WORD_REF(RUNTIME_GIC_OUT_IRQ_ADDR))
#define RUNTIME_IOMCU_WAKEIRQ_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x100)
#define IOMCU_WAKEIRQ_CNT (WORD_REF(RUNTIME_IOMCU_WAKEIRQ_CNT_ADDR))
#define RUNTIME_CPU_MEMORY_STEP0_VOLT_BIAS_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x104)
#define RUNTIME_CPU_MEMORY_STEP1_VOLT_BIAS_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x108)
#define CPU_MEMVOLT_STEP1_BIAS (WORD_REF(RUNTIME_CPU_MEMORY_STEP1_VOLT_BIAS_ADDR))
#define RUNTIME_CPU_MEMORY_STEP2_VOLT_BIAS_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x10C)
#define CPU_MEMVOLT_STEP2_BIAS (WORD_REF(RUNTIME_CPU_MEMORY_STEP2_VOLT_BIAS_ADDR))
#define RUNTIME_SLEEP_TIME_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x110)
#define SYS_SLEEP_TIME (WORD_REF(RUNTIME_SLEEP_TIME_ADDR))
#define RUNTIME_SLEEP_CAN_ENTER_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x114)
#define SYS_SLEEP_CAN_ENTER_CNT (WORD_REF(RUNTIME_SLEEP_CAN_ENTER_CNT_ADDR))
#define RUNTIME_CCPUNR_SUSPEND_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x118)
#define CCPUNR_SUSPEND_CNT (WORD_REF(RUNTIME_CCPUNR_SUSPEND_CNT_ADDR))
#define RUNTIME_CCPUNR_RESUME_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x11C)
#define CCPUNR_RESUME_CNT (WORD_REF(RUNTIME_CCPUNR_RESUME_CNT_ADDR))
#define RUNTIME_AP_WAKE_IRQ_PIE_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x120)
#define AP_WAKE_IRQ_PIE (WORD_REF(RUNTIME_AP_WAKE_IRQ_PIE_ADDR))
#define RUNTIME_MODEM_NR_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x124)
#define MODEM_NR_WAKE_CNT (WORD_REF(RUNTIME_MODEM_NR_WAKE_CNT_ADDR))
#define RUNTIME_OTHER_WAKE_CNT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x128)
#define OTHER_WAKE_CNT (WORD_REF(RUNTIME_OTHER_WAKE_CNT_ADDR))
#define RUNTIME_WAKE_IRQ4_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x12C)
#define WAKE_IRQ4 (WORD_REF(RUNTIME_WAKE_IRQ4_ADDR))
#define RUNTIME_WAKE_IRQ5_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x130)
#define WAKE_IRQ5 (WORD_REF(RUNTIME_WAKE_IRQ5_ADDR))
#define RUNTIME_CLENT_VOTE_TIME_ADDR(n) (RUNTIME_SR_VAR_BASE_ADDR + 0x134 + (n) * 4)
#define CLENT_VOTE_TIME(n) (WORD_REF(RUNTIME_CLENT_VOTE_TIME_ADDR(n)))
#define RUNTIME_SR_DEBUG_REG0_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x15C)
#define SR_DEBUG_REG0 (WORD_REF(RUNTIME_SR_DEBUG_REG0_ADDR))
#define RUNTIME_PCTRL_PERI_STAT69_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x160)
#define PCTRL_PERI_STAT69 (WORD_REF(RUNTIME_PCTRL_PERI_STAT69_ADDR))
#define RUNTIME_ACTRL_NONSEC_STATUS35_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x164)
#define ACTRL_NONSEC_STATUS35 (WORD_REF(RUNTIME_ACTRL_NONSEC_STATUS35_ADDR))
#define RUNTIME_ACTRL_NONSEC_STATUS52_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x168)
#define ACTRL_NONSEC_STATUS52 (WORD_REF(RUNTIME_ACTRL_NONSEC_STATUS52_ADDR))
#define RUNTIME_SR_SCTRL_SCCTRL_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x16C)
#define SR_SCTRL_SCCTRL (WORD_REF(RUNTIME_SR_SCTRL_SCCTRL_ADDR))
#define RUNTIME_SCTRL_RESUME_WATCHDOG_CTRL_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x170)
#define SCTRL_RESUME_WATCHDOG_CTRL (WORD_REF(RUNTIME_SCTRL_RESUME_WATCHDOG_CTRL_ADDR))
#define RUNTIME_SCTRL_SCIOMCUSTAT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x174)
#define SCTRL_SCIOMCUSTAT (WORD_REF(RUNTIME_SCTRL_SCIOMCUSTAT_ADDR))
#define RUNTIME_SCTRL_SCLK_SW2FLL_STAT_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x178)
#define SCTRL_SCLK_SW2FLL_STAT (WORD_REF(RUNTIME_SCTRL_SCLK_SW2FLL_STAT_ADDR))
#define RUNTIME_SCTRL_PERI_POWER_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x17C)
#define SCTRL_PERI_POWER (WORD_REF(RUNTIME_SCTRL_PERI_POWER_ADDR))
#define RUNTIME_SCTRL_TCXO_CTRL_3_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x180)
#define SCTRL_TCXO_CTRL_3 (WORD_REF(RUNTIME_SCTRL_TCXO_CTRL_3_ADDR))
#define RUNTIME_ACTRL_NONSEC_STATUS6_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x184)
#define ACTRL_NONSEC_STATUS6 (WORD_REF(RUNTIME_ACTRL_NONSEC_STATUS6_ADDR))
#define RUNTIME_CRGPERIPH_PERI_STAT3_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x188)
#define CRGPERIPH_PERI_STAT3 (WORD_REF(RUNTIME_CRGPERIPH_PERI_STAT3_ADDR))
#define RUNTIME_ARMPC_SR_PARAMETER_ADDR (RUNTIME_SR_VAR_BASE_ADDR + 0x18C)
#define ARMPC_SR_PARAMETER (WORD_REF(RUNTIME_ARMPC_SR_PARAMETER_ADDR))
#define RUNTIME_SPACE_ADDR_END (RUNTIME_ARMPC_SR_PARAMETER_ADDR)
#if (RUNTIME_SPACE_ADDR_END >= (RUNTIME_VAR_BASE + RUNTIME_VAR_SIZE))
 #error "runtime space overflow!!!"
#endif
#define WAKE_STATUS_OFFSET (RUNTIME_WAKE_STATUS_ADDR - RUNTIME_VAR_BASE)
#define SYS_DSLEEP_CNT_OFFSET (RUNTIME_DEEP_SLEEP_CNT_ADDR - RUNTIME_VAR_BASE)
#define SYS_DWAKE_CNT_OFFSET (RUNTIME_DEEP_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define SYS_SLEEP_CNT_OFFSET (RUNTIME_SLEEP_CNT_ADDR - RUNTIME_VAR_BASE)
#define LIGHT_SLEEP_CNT_OFFSET (RUNTIME_LIGHT_SLEEP_CNT_ADDR - RUNTIME_VAR_BASE)
#define LIGHT_SLEEP_RESUME_CNT_OFFSET (RUNTIME_LIGHT_SLEEP_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define AP_WAKE_CNT_OFFSET (RUNTIME_AP_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define MODEM_WAKE_CNT_OFFSET (RUNTIME_MODEM_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define HIFI_WAKE_CNT_OFFSET (RUNTIME_HIFI_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define IOMCU_WAKE_CNT_OFFSET (RUNTIME_IOMCU_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define LPM3_WAKE_CNT_OFFSET (RUNTIME_LPMCU_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define AP_SUSPEND_CNT_OFFSET (RUNTIME_AP_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define AP_RESUME_CNT_OFFSET (RUNTIME_AP_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define MODEM_SUSPEND_CNT_OFFSET (RUNTIME_MODEM_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define MODEM_RESUME_CNT_OFFSET (RUNTIME_MODEM_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define HIFI_SUSPEND_CNT_OFFSET (RUNTIME_HIFI_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define HIFI_RESUME_CNT_OFFSET (RUNTIME_HIFI_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define IOMCU_SUSPEND_CNT_OFFSET (RUNTIME_IOMCU_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define IOMCU_RESUME_CNT_OFFSET (RUNTIME_IOMCU_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define GENERAL_SEE_SUSPEND_CNT_OFFSET (RUNTIME_GENERAL_SEE_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define GENERAL_SEE_RESUME_CNT_OFFSET (RUNTIME_GENERAL_SEE_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define SLEEP_WAKE_IRQ_OFFSET (RUNTIME_SLEEP_WAKE_IRQ_ADDR - RUNTIME_VAR_BASE)
#define SLEEP_WAKE_IRQ1_OFFSET (RUNTIME_SLEEP_WAKE_IRQ1_ADDR - RUNTIME_VAR_BASE)
#define WAKE_IRQ_OFFSET (RUNTIME_WAKE_IRQ_ADDR - RUNTIME_VAR_BASE)
#define WAKE_IRQ1_OFFSET (RUNTIME_WAKE_IRQ1_ADDR - RUNTIME_VAR_BASE)
#define WAKE_IRQ2_OFFSET (RUNTIME_WAKE_IRQ2_ADDR - RUNTIME_VAR_BASE)
#define WAKE_IRQ3_OFFSET (RUNTIME_WAKE_IRQ3_ADDR - RUNTIME_VAR_BASE)
#define AP_WAKE_IRQ_OFFSET (RUNTIME_AP_WAKE_IRQ_ADDR - RUNTIME_VAR_BASE)
#define AP_NSIPC_IRQ_OFFSET (RUNTIME_AP_NSIPC_IRQ_ADDR - RUNTIME_VAR_BASE)
#define AP_AO_NSIPC_IRQ_OFFSET (RUNTIME_AP_AO_NSIPC_IRQ_ADDR - RUNTIME_VAR_BASE)
#define GIC_OUT_IRQ_OFFSET (RUNTIME_GIC_OUT_IRQ_ADDR - RUNTIME_VAR_BASE)
#define SYS_SLEEP_TIME_OFFSET (RUNTIME_SLEEP_TIME_ADDR - RUNTIME_VAR_BASE)
#define SYS_SLEEP_CAN_ENTER_CNT_OFFSET (RUNTIME_SLEEP_CAN_ENTER_CNT_ADDR - RUNTIME_VAR_BASE)
#define CCPUNR_SUSPEND_CNT_OFFSET (RUNTIME_CCPUNR_SUSPEND_CNT_ADDR - RUNTIME_VAR_BASE)
#define CCPUNR_RESUME_CNT_OFFSET (RUNTIME_CCPUNR_RESUME_CNT_ADDR - RUNTIME_VAR_BASE)
#define AP_WAKE_IRQ_PIE_OFFSET (RUNTIME_AP_WAKE_IRQ_PIE_ADDR - RUNTIME_VAR_BASE)
#define MODEM_NR_WAKE_CNT_OFFSET (RUNTIME_MODEM_NR_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define OTHER_WAKE_CNT_OFFSET (RUNTIME_OTHER_WAKE_CNT_ADDR - RUNTIME_VAR_BASE)
#define CLENT_VOTE_TIME_OFFSET(n) (RUNTIME_CLENT_VOTE_TIME_ADDR(n) - RUNTIME_VAR_BASE)
#define LPMCU_TELE_MNTN_DATA_TICKMARK_SIZE 0x150
#ifndef LPMCU_GEN_MNTN_SIZE
#define IPC_TRACK_SIZE (0x1000)
#define TASK_TRACK_SIZE 0x2C0
#define INT_TRACK_SIZE 0x2C0
#define EXC_SPECIAL_SAVE_SIZE 0x40
#define INT_LONG_TIME_TRACK_SIZE 0xF0
#define IRQ_MASK_LONG_TIME_TRACK_SIZE 0xF0
#define STACK_BACKUP_SRAM_SIZE 0x300
#endif
#ifdef INT_LONG_TIME_TRACK_BASE
#define INT_LONG_TIME_TRACK_OFFSET (INT_LONG_TIME_TRACK_BASE - LPMCU_GEN_MNTN_BASE)
#define IRQ_MASK_LONG_TIME_TRACK_OFFSET (IRQ_MASK_LONG_TIME_TRACK_BASE - LPMCU_GEN_MNTN_BASE)
#define STACK_BACKUP_SRAM_OFFSET (STACK_BACKUP_SRAM_BASE - LPMCU_GEN_MNTN_BASE)
#else
#define INT_LONG_TIME_TRACK_OFFSET 0xC94
#define IRQ_MASK_LONG_TIME_TRACK_OFFSET 0xD84
#define STACK_BACKUP_SRAM_OFFSET 0x630
#endif
#endif