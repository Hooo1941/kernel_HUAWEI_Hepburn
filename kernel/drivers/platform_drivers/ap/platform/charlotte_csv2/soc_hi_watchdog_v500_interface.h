#ifndef __SOC_HI_WATCHDOG_V500_INTERFACE_H__
#define __SOC_HI_WATCHDOG_V500_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_HI_WatchDog_V500_LOCKEN_ADDR(base) ((base) + (0x000))
#define SOC_HI_WatchDog_V500_LOCKDIS_ADDR(base) ((base) + (0x004))
#define SOC_HI_WatchDog_V500_LOCK_ADDR(base) ((base) + (0x008))
#define SOC_HI_WatchDog_V500_CNT_INTR_TIME_ADDR(base) ((base) + (0x00C))
#define SOC_HI_WatchDog_V500_CNT_RST_TIME_ADDR(base) ((base) + (0x010))
#define SOC_HI_WatchDog_V500_WD_EN_ADDR(base) ((base) + (0x014))
#define SOC_HI_WatchDog_V500_WD_KICK_ADDR(base) ((base) + (0x018))
#define SOC_HI_WatchDog_V500_INTR_WD_CLR_ADDR(base) ((base) + (0x01C))
#define SOC_HI_WatchDog_V500_INTR_WD_MASK_ADDR(base) ((base) + (0x020))
#define SOC_HI_WatchDog_V500_WD_RST_MASK_ADDR(base) ((base) + (0x024))
#define SOC_HI_WatchDog_V500_INTR_WD_RAW_ADDR(base) ((base) + (0x028))
#define SOC_HI_WatchDog_V500_INTR_WD_ADDR(base) ((base) + (0x02C))
#define SOC_HI_WatchDog_V500_WD_CNT_ADDR(base) ((base) + (0x030))
#define SOC_HI_WatchDog_V500_CLK_GATE_ADDR(base) ((base) + (0x034))
#define SOC_HI_WatchDog_V500_WD_ID_ADDR(base) ((base) + (0xFFC))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int locken : 32;
    } reg;
} SOC_HI_WatchDog_V500_LOCKEN_UNION;
#endif
#define SOC_HI_WatchDog_V500_LOCKEN_locken_START (0)
#define SOC_HI_WatchDog_V500_LOCKEN_locken_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lockdis : 32;
    } reg;
} SOC_HI_WatchDog_V500_LOCKDIS_UNION;
#endif
#define SOC_HI_WatchDog_V500_LOCKDIS_lockdis_START (0)
#define SOC_HI_WatchDog_V500_LOCKDIS_lockdis_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_LOCK_UNION;
#endif
#define SOC_HI_WatchDog_V500_LOCK_lock_START (0)
#define SOC_HI_WatchDog_V500_LOCK_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cnt_intr_time : 32;
    } reg;
} SOC_HI_WatchDog_V500_CNT_INTR_TIME_UNION;
#endif
#define SOC_HI_WatchDog_V500_CNT_INTR_TIME_cnt_intr_time_START (0)
#define SOC_HI_WatchDog_V500_CNT_INTR_TIME_cnt_intr_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cnt_rst_time : 32;
    } reg;
} SOC_HI_WatchDog_V500_CNT_RST_TIME_UNION;
#endif
#define SOC_HI_WatchDog_V500_CNT_RST_TIME_cnt_rst_time_START (0)
#define SOC_HI_WatchDog_V500_CNT_RST_TIME_cnt_rst_time_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_WD_EN_UNION;
#endif
#define SOC_HI_WatchDog_V500_WD_EN_wd_en_START (0)
#define SOC_HI_WatchDog_V500_WD_EN_wd_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_kick_pulse : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_WD_KICK_UNION;
#endif
#define SOC_HI_WatchDog_V500_WD_KICK_wd_kick_pulse_START (0)
#define SOC_HI_WatchDog_V500_WD_KICK_wd_kick_pulse_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_wd_clr_pulse : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_INTR_WD_CLR_UNION;
#endif
#define SOC_HI_WatchDog_V500_INTR_WD_CLR_intr_wd_clr_pulse_START (0)
#define SOC_HI_WatchDog_V500_INTR_WD_CLR_intr_wd_clr_pulse_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_wd_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_INTR_WD_MASK_UNION;
#endif
#define SOC_HI_WatchDog_V500_INTR_WD_MASK_intr_wd_mask_START (0)
#define SOC_HI_WatchDog_V500_INTR_WD_MASK_intr_wd_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_rst_mask : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_WD_RST_MASK_UNION;
#endif
#define SOC_HI_WatchDog_V500_WD_RST_MASK_wd_rst_mask_START (0)
#define SOC_HI_WatchDog_V500_WD_RST_MASK_wd_rst_mask_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_wd_raw : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_INTR_WD_RAW_UNION;
#endif
#define SOC_HI_WatchDog_V500_INTR_WD_RAW_intr_wd_raw_START (0)
#define SOC_HI_WatchDog_V500_INTR_WD_RAW_intr_wd_raw_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_wd : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_HI_WatchDog_V500_INTR_WD_UNION;
#endif
#define SOC_HI_WatchDog_V500_INTR_WD_intr_wd_START (0)
#define SOC_HI_WatchDog_V500_INTR_WD_intr_wd_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_cnt : 32;
    } reg;
} SOC_HI_WatchDog_V500_WD_CNT_UNION;
#endif
#define SOC_HI_WatchDog_V500_WD_CNT_wd_cnt_START (0)
#define SOC_HI_WatchDog_V500_WD_CNT_wd_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_clk_wd_gate : 1;
        unsigned int wd_dbg_bypass : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_HI_WatchDog_V500_CLK_GATE_UNION;
#endif
#define SOC_HI_WatchDog_V500_CLK_GATE_st_clk_wd_gate_START (0)
#define SOC_HI_WatchDog_V500_CLK_GATE_st_clk_wd_gate_END (0)
#define SOC_HI_WatchDog_V500_CLK_GATE_wd_dbg_bypass_START (1)
#define SOC_HI_WatchDog_V500_CLK_GATE_wd_dbg_bypass_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wd_id : 32;
    } reg;
} SOC_HI_WatchDog_V500_WD_ID_UNION;
#endif
#define SOC_HI_WatchDog_V500_WD_ID_wd_id_START (0)
#define SOC_HI_WatchDog_V500_WD_ID_wd_id_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
