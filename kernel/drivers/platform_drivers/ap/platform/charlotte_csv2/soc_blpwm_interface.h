#ifndef __SOC_BLPWM_INTERFACE_H__
#define __SOC_BLPWM_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_BLPWM_IN_CTRL_ADDR(base) ((base) + (0x000UL))
#define SOC_BLPWM_IN_DIV_ADDR(base) ((base) + (0x004UL))
#define SOC_BLPWM_OUT_CTRL_ADDR(base) ((base) + (0x100UL))
#define SOC_BLPWM_OUT_DIV_ADDR(base) ((base) + (0x104UL))
#define SOC_BLPWM_OUT_CFG_ADDR(base) ((base) + (0x108UL))
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
