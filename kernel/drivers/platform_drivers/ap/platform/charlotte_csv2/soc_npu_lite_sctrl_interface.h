#ifndef __SOC_NPU_LITE_SCTRL_INTERFACE_H__
#define __SOC_NPU_LITE_SCTRL_INTERFACE_H__ 
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_ADDR(base) ((base) + (0x000UL))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_ADDR(base) ((base) + (0x004UL))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_ADDR(base) ((base) + (0x008UL))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_ADDR(base) ((base) + (0x00CUL))
#else
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_ADDR(base) ((base) + (0x000))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_ADDR(base) ((base) + (0x004))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_ADDR(base) ((base) + (0x008))
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_ADDR(base) ((base) + (0x00C))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int sysdma_pass_mid_1 : 8;
        unsigned int sysdma_pass_mid_0 : 8;
        unsigned int reserved : 16;
    } reg;
} SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_UNION;
#endif
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_sysdma_pass_mid_1_START (0)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_sysdma_pass_mid_1_END (7)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_sysdma_pass_mid_0_START (8)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL0_sysdma_pass_mid_0_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_ts_awcache : 4;
        unsigned int npu_ts_awdomain : 2;
        unsigned int reserved_0 : 10;
        unsigned int npu_ts_arcache : 4;
        unsigned int npu_ts_ardomain : 2;
        unsigned int reserved_1 : 10;
    } reg;
} SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_UNION;
#endif
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_awcache_START (0)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_awcache_END (3)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_awdomain_START (4)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_awdomain_END (5)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_arcache_START (16)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_arcache_END (19)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_ardomain_START (20)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL1_npu_ts_ardomain_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_sdma_awcache : 4;
        unsigned int npu_sdma_awdomain : 2;
        unsigned int reserved_0 : 10;
        unsigned int npu_sdma_arcache : 4;
        unsigned int npu_sdma_ardomain : 2;
        unsigned int reserved_1 : 10;
    } reg;
} SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_UNION;
#endif
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_awcache_START (0)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_awcache_END (3)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_awdomain_START (4)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_awdomain_END (5)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_arcache_START (16)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_arcache_END (19)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_ardomain_START (20)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL2_npu_sdma_ardomain_END (21)
#ifndef __SOC_H_FOR_ASM__
typedef union {
    unsigned int value;
    struct {
        unsigned int npu_aic_awcache : 4;
        unsigned int npu_aic_awdomain : 2;
        unsigned int reserved_0 : 10;
        unsigned int npu_aic_arcache : 4;
        unsigned int npu_aic_ardomain : 2;
        unsigned int reserved_1 : 10;
    } reg;
} SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_UNION;
#endif
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_awcache_START (0)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_awcache_END (3)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_awdomain_START (4)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_awdomain_END (5)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_arcache_START (16)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_arcache_END (19)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_ardomain_START (20)
#define SOC_NPU_LITE_SCTRL_NPU_LITE_CTRL3_npu_aic_ardomain_END (21)
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif
#endif
