#ifndef __SOC_NPU_TZPC_INTERFACE_H__
#define __SOC_NPU_TZPC_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_TZPC_DECPROT0STAT_ADDR(base) ((base) + (0x800UL))
#define SOC_NPU_TZPC_DECPROT0SET_ADDR(base) ((base) + (0x804UL))
#define SOC_NPU_TZPC_DECPROT0CLR_ADDR(base) ((base) + (0x808UL))
#else
#define SOC_NPU_TZPC_DECPROT0STAT_ADDR(base) ((base) + (0x800))
#define SOC_NPU_TZPC_DECPROT0SET_ADDR(base) ((base) + (0x804))
#define SOC_NPU_TZPC_DECPROT0CLR_ADDR(base) ((base) + (0x808))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipc_s : 1;
        unsigned int ipc_ns : 1;
        unsigned int crg : 1;
        unsigned int sctrl : 1;
        unsigned int uart : 1;
        unsigned int hw_exp_irq : 1;
        unsigned int hw_exp_irq_ns : 1;
        unsigned int dpm : 1;
        unsigned int pcr : 1;
        unsigned int lite_sctrl : 1;
        unsigned int tiny_sctrl : 1;
        unsigned int hw_exp_irq_ns1 : 1;
        unsigned int hw_exp_irq_ns2 : 1;
        unsigned int hw_exp_irq_ns3 : 1;
        unsigned int aic1_smmu_cfg : 1;
        unsigned int aic0_smmu_cfg : 1;
        unsigned int sysdma_smmu_cfg : 1;
        unsigned int smmu_tcu_cfg : 1;
        unsigned int dmmu0 : 1;
        unsigned int dmmu1 : 1;
        unsigned int dmmu2 : 1;
        unsigned int dmmu3 : 1;
        unsigned int sysdma_mid_auth_bypass : 1;
        unsigned int reserved : 9;
    } reg;
} SOC_NPU_TZPC_DECPROT0STAT_UNION;
#endif
#define SOC_NPU_TZPC_DECPROT0STAT_ipc_s_START (0)
#define SOC_NPU_TZPC_DECPROT0STAT_ipc_s_END (0)
#define SOC_NPU_TZPC_DECPROT0STAT_ipc_ns_START (1)
#define SOC_NPU_TZPC_DECPROT0STAT_ipc_ns_END (1)
#define SOC_NPU_TZPC_DECPROT0STAT_crg_START (2)
#define SOC_NPU_TZPC_DECPROT0STAT_crg_END (2)
#define SOC_NPU_TZPC_DECPROT0STAT_sctrl_START (3)
#define SOC_NPU_TZPC_DECPROT0STAT_sctrl_END (3)
#define SOC_NPU_TZPC_DECPROT0STAT_uart_START (4)
#define SOC_NPU_TZPC_DECPROT0STAT_uart_END (4)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_START (5)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_END (5)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns_START (6)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns_END (6)
#define SOC_NPU_TZPC_DECPROT0STAT_dpm_START (7)
#define SOC_NPU_TZPC_DECPROT0STAT_dpm_END (7)
#define SOC_NPU_TZPC_DECPROT0STAT_pcr_START (8)
#define SOC_NPU_TZPC_DECPROT0STAT_pcr_END (8)
#define SOC_NPU_TZPC_DECPROT0STAT_lite_sctrl_START (9)
#define SOC_NPU_TZPC_DECPROT0STAT_lite_sctrl_END (9)
#define SOC_NPU_TZPC_DECPROT0STAT_tiny_sctrl_START (10)
#define SOC_NPU_TZPC_DECPROT0STAT_tiny_sctrl_END (10)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns1_START (11)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns1_END (11)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns2_START (12)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns2_END (12)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns3_START (13)
#define SOC_NPU_TZPC_DECPROT0STAT_hw_exp_irq_ns3_END (13)
#define SOC_NPU_TZPC_DECPROT0STAT_aic1_smmu_cfg_START (14)
#define SOC_NPU_TZPC_DECPROT0STAT_aic1_smmu_cfg_END (14)
#define SOC_NPU_TZPC_DECPROT0STAT_aic0_smmu_cfg_START (15)
#define SOC_NPU_TZPC_DECPROT0STAT_aic0_smmu_cfg_END (15)
#define SOC_NPU_TZPC_DECPROT0STAT_sysdma_smmu_cfg_START (16)
#define SOC_NPU_TZPC_DECPROT0STAT_sysdma_smmu_cfg_END (16)
#define SOC_NPU_TZPC_DECPROT0STAT_smmu_tcu_cfg_START (17)
#define SOC_NPU_TZPC_DECPROT0STAT_smmu_tcu_cfg_END (17)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu0_START (18)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu0_END (18)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu1_START (19)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu1_END (19)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu2_START (20)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu2_END (20)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu3_START (21)
#define SOC_NPU_TZPC_DECPROT0STAT_dmmu3_END (21)
#define SOC_NPU_TZPC_DECPROT0STAT_sysdma_mid_auth_bypass_START (22)
#define SOC_NPU_TZPC_DECPROT0STAT_sysdma_mid_auth_bypass_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipc_s : 1;
        unsigned int ipc_ns : 1;
        unsigned int crg : 1;
        unsigned int sctrl : 1;
        unsigned int uart : 1;
        unsigned int hw_exp_irq : 1;
        unsigned int hw_exp_irq_ns : 1;
        unsigned int dpm : 1;
        unsigned int pcr : 1;
        unsigned int lite_sctrl : 1;
        unsigned int tiny_sctrl : 1;
        unsigned int hw_exp_irq_ns1 : 1;
        unsigned int hw_exp_irq_ns2 : 1;
        unsigned int hw_exp_irq_ns3 : 1;
        unsigned int aic1_smmu_cfg : 1;
        unsigned int aic0_smmu_cfg : 1;
        unsigned int sysdma_smmu_cfg : 1;
        unsigned int smmu_tcu_cfg : 1;
        unsigned int dmmu0 : 1;
        unsigned int dmmu1 : 1;
        unsigned int dmmu2 : 1;
        unsigned int dmmu3 : 1;
        unsigned int sysdma_mid_auth_bypass : 1;
        unsigned int reserved : 9;
    } reg;
} SOC_NPU_TZPC_DECPROT0SET_UNION;
#endif
#define SOC_NPU_TZPC_DECPROT0SET_ipc_s_START (0)
#define SOC_NPU_TZPC_DECPROT0SET_ipc_s_END (0)
#define SOC_NPU_TZPC_DECPROT0SET_ipc_ns_START (1)
#define SOC_NPU_TZPC_DECPROT0SET_ipc_ns_END (1)
#define SOC_NPU_TZPC_DECPROT0SET_crg_START (2)
#define SOC_NPU_TZPC_DECPROT0SET_crg_END (2)
#define SOC_NPU_TZPC_DECPROT0SET_sctrl_START (3)
#define SOC_NPU_TZPC_DECPROT0SET_sctrl_END (3)
#define SOC_NPU_TZPC_DECPROT0SET_uart_START (4)
#define SOC_NPU_TZPC_DECPROT0SET_uart_END (4)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_START (5)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_END (5)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns_START (6)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns_END (6)
#define SOC_NPU_TZPC_DECPROT0SET_dpm_START (7)
#define SOC_NPU_TZPC_DECPROT0SET_dpm_END (7)
#define SOC_NPU_TZPC_DECPROT0SET_pcr_START (8)
#define SOC_NPU_TZPC_DECPROT0SET_pcr_END (8)
#define SOC_NPU_TZPC_DECPROT0SET_lite_sctrl_START (9)
#define SOC_NPU_TZPC_DECPROT0SET_lite_sctrl_END (9)
#define SOC_NPU_TZPC_DECPROT0SET_tiny_sctrl_START (10)
#define SOC_NPU_TZPC_DECPROT0SET_tiny_sctrl_END (10)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns1_START (11)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns1_END (11)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns2_START (12)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns2_END (12)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns3_START (13)
#define SOC_NPU_TZPC_DECPROT0SET_hw_exp_irq_ns3_END (13)
#define SOC_NPU_TZPC_DECPROT0SET_aic1_smmu_cfg_START (14)
#define SOC_NPU_TZPC_DECPROT0SET_aic1_smmu_cfg_END (14)
#define SOC_NPU_TZPC_DECPROT0SET_aic0_smmu_cfg_START (15)
#define SOC_NPU_TZPC_DECPROT0SET_aic0_smmu_cfg_END (15)
#define SOC_NPU_TZPC_DECPROT0SET_sysdma_smmu_cfg_START (16)
#define SOC_NPU_TZPC_DECPROT0SET_sysdma_smmu_cfg_END (16)
#define SOC_NPU_TZPC_DECPROT0SET_smmu_tcu_cfg_START (17)
#define SOC_NPU_TZPC_DECPROT0SET_smmu_tcu_cfg_END (17)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu0_START (18)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu0_END (18)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu1_START (19)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu1_END (19)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu2_START (20)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu2_END (20)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu3_START (21)
#define SOC_NPU_TZPC_DECPROT0SET_dmmu3_END (21)
#define SOC_NPU_TZPC_DECPROT0SET_sysdma_mid_auth_bypass_START (22)
#define SOC_NPU_TZPC_DECPROT0SET_sysdma_mid_auth_bypass_END (22)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int ipc_s : 1;
        unsigned int ipc_ns : 1;
        unsigned int crg : 1;
        unsigned int sctrl : 1;
        unsigned int uart : 1;
        unsigned int hw_exp_irq : 1;
        unsigned int hw_exp_irq_ns : 1;
        unsigned int dpm : 1;
        unsigned int pcr : 1;
        unsigned int lite_sctrl : 1;
        unsigned int tiny_sctrl : 1;
        unsigned int hw_exp_irq_ns1 : 1;
        unsigned int hw_exp_irq_ns2 : 1;
        unsigned int hw_exp_irq_ns3 : 1;
        unsigned int aic1_smmu_cfg : 1;
        unsigned int aic0_smmu_cfg : 1;
        unsigned int sysdma_smmu_cfg : 1;
        unsigned int smmu_tcu_cfg : 1;
        unsigned int dmmu0 : 1;
        unsigned int dmmu1 : 1;
        unsigned int dmmu2 : 1;
        unsigned int dmmu3 : 1;
        unsigned int sysdma_mid_auth_bypass : 1;
        unsigned int reserved : 9;
    } reg;
} SOC_NPU_TZPC_DECPROT0CLR_UNION;
#endif
#define SOC_NPU_TZPC_DECPROT0CLR_ipc_s_START (0)
#define SOC_NPU_TZPC_DECPROT0CLR_ipc_s_END (0)
#define SOC_NPU_TZPC_DECPROT0CLR_ipc_ns_START (1)
#define SOC_NPU_TZPC_DECPROT0CLR_ipc_ns_END (1)
#define SOC_NPU_TZPC_DECPROT0CLR_crg_START (2)
#define SOC_NPU_TZPC_DECPROT0CLR_crg_END (2)
#define SOC_NPU_TZPC_DECPROT0CLR_sctrl_START (3)
#define SOC_NPU_TZPC_DECPROT0CLR_sctrl_END (3)
#define SOC_NPU_TZPC_DECPROT0CLR_uart_START (4)
#define SOC_NPU_TZPC_DECPROT0CLR_uart_END (4)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_START (5)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_END (5)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns_START (6)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns_END (6)
#define SOC_NPU_TZPC_DECPROT0CLR_dpm_START (7)
#define SOC_NPU_TZPC_DECPROT0CLR_dpm_END (7)
#define SOC_NPU_TZPC_DECPROT0CLR_pcr_START (8)
#define SOC_NPU_TZPC_DECPROT0CLR_pcr_END (8)
#define SOC_NPU_TZPC_DECPROT0CLR_lite_sctrl_START (9)
#define SOC_NPU_TZPC_DECPROT0CLR_lite_sctrl_END (9)
#define SOC_NPU_TZPC_DECPROT0CLR_tiny_sctrl_START (10)
#define SOC_NPU_TZPC_DECPROT0CLR_tiny_sctrl_END (10)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns1_START (11)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns1_END (11)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns2_START (12)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns2_END (12)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns3_START (13)
#define SOC_NPU_TZPC_DECPROT0CLR_hw_exp_irq_ns3_END (13)
#define SOC_NPU_TZPC_DECPROT0CLR_aic1_smmu_cfg_START (14)
#define SOC_NPU_TZPC_DECPROT0CLR_aic1_smmu_cfg_END (14)
#define SOC_NPU_TZPC_DECPROT0CLR_aic0_smmu_cfg_START (15)
#define SOC_NPU_TZPC_DECPROT0CLR_aic0_smmu_cfg_END (15)
#define SOC_NPU_TZPC_DECPROT0CLR_sysdma_smmu_cfg_START (16)
#define SOC_NPU_TZPC_DECPROT0CLR_sysdma_smmu_cfg_END (16)
#define SOC_NPU_TZPC_DECPROT0CLR_smmu_tcu_cfg_START (17)
#define SOC_NPU_TZPC_DECPROT0CLR_smmu_tcu_cfg_END (17)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu0_START (18)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu0_END (18)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu1_START (19)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu1_END (19)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu2_START (20)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu2_END (20)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu3_START (21)
#define SOC_NPU_TZPC_DECPROT0CLR_dmmu3_END (21)
#define SOC_NPU_TZPC_DECPROT0CLR_sysdma_mid_auth_bypass_START (22)
#define SOC_NPU_TZPC_DECPROT0CLR_sysdma_mid_auth_bypass_END (22)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
