/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: ssi_common.c header file
 * Author: @CompanyNameTag
 */

#ifndef SSI_COMMON_H
#define SSI_COMMON_H

/* 头文件包含 */
#include "plat_type.h"
#include "oal_types.h"
#include "oal_schedule.h"

/* GPIO_SSI Base Reg */
#define SSI_SM_CLEAR   0xC  /* 8006 */
#define SSI_AON_CLKSEL 0xE  /* 8007 */
#define SSI_SEL_CTRL   0x10 /* 8008 */
#define SSI_SSI_CTRL   0x12 /* 8009 */

/* After MP15 */
#define SSI_SYS_CTL_ID 0x1C
#define SSI_GP_REG0    0x20
#define SSI_GP_REG1    0x24
#define SSI_GP_REG2    0x28
#define SSI_GP_REG3    0x2C
#define SSI_RDATA_L    0x30
#define SSI_RDATA_H    0x34
#define SSI_RPT_STS_L  0x38
#define SSI_RPT_STS_H  0x3C
#define SSI_SSI_RPT_STS_0 0x40
#define SSI_SSI_RPT_STS_1 0x44
#define SSI_SSI_RPT_STS_2 0x48
#define SSI_SSI_RPT_STS_3 0x4C
#define SSI_SSI_RPT_STS_4 0x50
#define SSI_SSI_RPT_STS_5 0x54
#define SSI_SSI_RPT_STS_6 0x58
#define SSI_SSI_RPT_STS_7 0x5C
/* After MP16 */
#define SSI_SSI_RPT_STS_8 0x60
#define SSI_SSI_RPT_STS_9 0x64
#define SSI_SSI_RPT_STS_10 0x68
#define SSI_SSI_RPT_STS_11 0x6C

#define gpio_ssi_reg(offset) (0x8000 + ((offset) >> 1))

#define SSI_ALIVE 0x0305  /* SSI_SYS_CTL_ID寄存器默认值0x0305 */

#define SSI_AON_CLKSEL_TCXO 0x0
#define SSI_AON_CLKSEL_SSI  0x1

#define SSI_RW_WORD_MOD  0x0 /* 2 bytes */
#define SSI_RW_BYTE_MOD  0x1
#define SSI_RW_DWORD_MOD 0x2 /* 4 bytes */
#define SSI_RW_SSI_MOD   0x3 /* SSI master reg */

#define SSI_AHB_MODE_SET_START 0x0
#define SSI_AHB_MODE_SET_END   0x1

#define SSI_MODULE_MASK_AON              (1 << 0)
#define SSI_MODULE_MASK_ARM_REG          (1 << 1)
#define SSI_MODULE_MASK_WCTRL            (1 << 2)
#define SSI_MODULE_MASK_BCTRL            (1 << 3)
#define SSI_MODULE_MASK_PCIE_CFG         (1 << 4)
#define SSI_MODULE_MASK_PCIE_DBI         (1 << 5)
#define SSI_MODULE_MASK_SDIO             (1 << 6)
#define SSI_MODULE_MASK_UART             (1 << 7)
#define SSI_MODULE_MASK_WCPU_PATCH       (1 << 8)
#define SSI_MODULE_MASK_BCPU_PATCH       (1 << 9)
#define SSI_MODULE_MASK_WCPU_KEY_DTCM    (1 << 10)
#define SSI_MODULE_MASK_AON_CUT          (1 << 11)
#define SSI_MODULE_MASK_PCIE_CUT         (1 << 12)
#define SSI_MODULE_MASK_COEX_CTL         (1 << 13)
#define SSI_MODULE_MASK_BCPU_EXCEPT_MEM  (1 << 14)
#define SSI_MODULE_MASK_GFCTRL           (1 << 15)
#define SSI_MODULE_MASK_SLECTRL          (1 << 16)
#define SSI_MODULE_MASK_WTRACE           (1 << 17)
#define SSI_MODULE_MASK_BTRACE           (1 << 18)
#define SSI_MODULE_MASK_GTRACE           (1 << 19)
#define SSI_MODULE_MASK_PMU              (1 << 20)
#define SSI_MODULE_MASK_TIMER            (1 << 21)

#define SSI_MODULE_MASK_BFGXCTRL (SSI_MODULE_MASK_BCTRL | SSI_MODULE_MASK_GFCTRL | SSI_MODULE_MASK_SLECTRL)

#define SSI_MODULE_MASK_COMM (SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |  \
                              SSI_MODULE_MASK_GFCTRL | SSI_MODULE_MASK_SLECTRL | \
                              SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL |  \
                              SSI_MODULE_MASK_COEX_CTL | SSI_MODULE_MASK_WCPU_KEY_DTCM |  \
                              SSI_MODULE_MASK_PMU) /* 0xf */

#define SSI_MODULE_MASK_PCIE_FULL    (SSI_MODULE_MASK_COMM | \
                                     SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI)

#define SSI_MODULE_MASK_PCIE_SET   (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG)

#define SSI_MODULE_MASK_PCIE_CUT_SET   (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_PCIE_CUT)

#define SSI_MODULE_NON_FLAG   (0)

#define SSI_WRITE_DATA 0x5a5a

#define DSM_CPU_INFO_SIZE 256        // 与上层协商上报CPU信息的最大值
#define SSI_CPU_ARM_REG_DUMP_CNT  2

#define SSI_INVALID_VALUE  0xffffffff

#define declare_ssi_err_info(etype, module_func, priv_func)     \
    {etype, #etype, module_func, priv_func}

typedef enum _ssi_err_type_ {
    SSI_ERR_COMM,
    SSI_ERR_HCC_EXCP_SDIO,
    SSI_ERR_HCC_EXCP_PCIE,
    SSI_ERR_PCIE_KIRIN_NOC,
    SSI_ERR_PCIE_CHECK_LINK_FAIL,
    SSI_ERR_PCIE_WAIT_BOOT_TIMEOUT,
    SSI_ERR_PCIE_SR_WAKEUP_FAIL,
    SSI_ERR_PCIE_SR_WAKEUP_RETRY_FAIL,
    SSI_ERR_PCIE_POWER_UP_FAIL,
    SSI_ERR_PCIE_FST_POWER_OFF_FAIL,
    SSI_ERR_PCIE_ENUM_FAIL,
    SSI_ERR_SDIO_REENUM_FAIL,
    SSI_ERR_SDIO_REINIT_FAIL,
    SSI_ERR_SDIO_PROBE_INIT_FAIL,
    SSI_ERR_SDIO_PROBE_FAIL,
    SSI_ERR_BFG_WAKE_UP_FAIL,
    SSI_ERR_FIRMWARE_DOWN_FAIL,
    SSI_ERR_FIRMWARE_DOWN_SDIO_FAIL,
    SSI_ERR_PCIE_GPIO_WAKE_FAIL,
    SSI_ERR_SDIO_CMD_WAKE_FAIL,
    SSI_ERR_SDIO_GPIO_WAKE_FAIL,
    SSI_ERR_WLAN_POWEROFF_FAIL,
    SSI_ERR_WLAN_CUSTOM_CALI_FAIL,
    SSI_ERR_BFGX_OPEN_FAIL,
    SSI_ERR_BFGX_HEART_TIMEOUT,
    SSI_ERR_BUT
} ssi_err_type;

#define SSI_ERR_LOG_LEVEL_FULL    (0) /* full log print */
#define SSI_ERR_LOG_LEVEL_CUT     (1) /* little log print */
#define SSI_ERR_LOG_LEVEL_CLOSE   (2) /* Suggest nog log */

/* 针对特定异常，
 * SSI dump寄存器定制处理 */
typedef struct _ssi_err_stru_ {
    ssi_err_type etype;
    char* type_name;
    unsigned long long (*ssi_get_module_flag)(ssi_err_type etype);
    int32_t (*ssi_private_func)(ssi_err_type etype);
} ssi_err_stru;

typedef struct _ssi_cpu_info_ {
    uint32_t cpu_state;
    uint32_t pc[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32_t lr[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32_t sp[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32_t reg_flag[SSI_CPU_ARM_REG_DUMP_CNT];
} ssi_cpu_info;

typedef struct _ssi_cpu_infos_ {
    ssi_cpu_info wcpu_info;
    ssi_cpu_info bcpu_info;
} ssi_cpu_infos;

typedef struct {
    ssi_cpu_info wcpu0_info;
    ssi_cpu_info wcpu1_info;
    ssi_cpu_info bcpu_info;
    ssi_cpu_info gcpu_info;
} mp16_ssi_cpu_infos;

typedef struct {
    ssi_cpu_info wcpu_info;
    ssi_cpu_info bcpu_info;
    ssi_cpu_info gtcpu0_info;
    ssi_cpu_info gtcpu1_info;
    ssi_cpu_info sle_info;
} gf61_ssi_cpu_infos;

typedef struct _ssi_reg_info_ {
    uint32_t base_addr;
    uint32_t offset;
    uint32_t len;
    uint32_t rw_mod;
} ssi_reg_info;

#define dump_start_addr(pst_reg_info) ((pst_reg_info)->base_addr + (pst_reg_info)->offset)

typedef union {
    struct {
        uint32_t ssi       : 1;  /* 0 */
        uint32_t memdump   : 1;  /* 1 */
        uint32_t reserved1 : 30; /* 2:31 */
    } bits;
    uint32_t flag;
} dbg_flag;


typedef enum {
    SSI_FLAG,
    MEMDUMP_FLAG,
    DUMP_CFG_BUT
} dump_cfg_enum;

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

#define SSI_CPU_ST_STR_LEN 8
extern int g_ssi_is_logfile;
extern int g_mpxx_kernel_crash;
extern uint32_t g_halt_det_cnt;
extern char *g_ssi_cpu_st_str[SSI_CPU_ST_STR_LEN];

uint16_t ssi_read16(uint16_t addr);
int32_t ssi_write16(uint16_t addr, uint16_t value);
int32_t ssi_read_value32(uint32_t addr, uint32_t *value, int16_t last_high_addr);
uint32_t ssi_read_value32_test(uint32_t addr);
int32_t ssi_write_value32(uint32_t addr, uint32_t value);
int32_t ssi_write_value32_test(uint32_t addr, uint32_t value);
int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, uint32_t reg_nums, int32_t is_logfile);
int ssi_read_reg_info(ssi_reg_info *pst_reg_info, void *buf, int32_t size, int32_t is_file);
int32_t ssi_write32(uint32_t addr, uint16_t value);
int32_t ssi_read32(uint32_t addr);
int ssi_switch_clk(uint32_t clk_type);
int ssi_clk_auto_switch_is_support(void);
int32_t wait_for_ssi_idle_timeout(int32_t mstimeout);
void ssi_force_reset_reg(void);
void ssi_check_tcxo_is_available(void);
int ssi_check_device_isalive(void);
int ssi_dump_device_regs_handler(unsigned long long module_set, uint32_t flag);
int ssi_dump_device_regs(unsigned long long module_set);
void ssi_dump_for_dfr(unsigned long long module_set);
int ssi_dump_err_regs(ssi_err_type etype);
int ssi_read_reg_info_test(uint32_t base_addr, uint32_t offset, uint32_t len, uint32_t is_logfile, uint32_t rw_mode);
int ssi_dump_err_reg(ssi_err_type etype);
oal_spin_lock_stru* get_ssi_lock_glb_addr(void);
void ssi_excetpion_dump_disable(void);
void ssi_dump_gpio(void);
int ssi_read_master_regs(ssi_reg_info *pst_reg_info, void *buf, int32_t size, int32_t is_file);
int32_t ssi_try_lock(void);
int32_t ssi_unlock(void);
int32_t ssi_gpio_valid(void);
int32_t get_dump_cfg(void);
uint32_t ssi_read_dword(uint32_t addr);
int32_t ssi_write_dword(uint32_t addr, uint32_t value);
void show_subsys_mdpll_lock_status(void);
#else
static inline int32_t ssi_gpio_valid(void)
{
    return OAL_FALSE;
}

static inline int32_t get_dump_cfg(void)
{
    return 0;
}

static inline int32_t ssi_write_value32_test(uint32_t addr, uint32_t value)
{
    return 0;
}

static inline uint32_t ssi_read_value32_test(uint32_t addr)
{
    return 0;
}

static inline int ssi_read_reg_info_test(uint32_t base_addr, uint32_t offset, uint32_t len,
                                         uint32_t is_logfile, uint32_t rw_mode)
{
    return -OAL_EFAIL;
}

static inline int32_t ssi_try_lock(void)
{
    return 0;
}

static inline int32_t ssi_unlock(void)
{
    return 0;
}

static inline int ssi_check_device_isalive(void)
{
    return -1;
}

static inline int ssi_dump_device_regs(unsigned long long module_set)
{
    return 0;
}

static inline int ssi_dump_err_regs(ssi_err_type etype)
{
    return 0;
}

static inline void ssi_excetpion_dump_disable(void)
{
}

static inline void ssi_dump_wlan_gpio(void)
{
}

static inline void ssi_dump_gpio(void)
{
}

static inline void show_subsys_mdpll_lock_status(void)
{
}

#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */
#endif /* #ifndef __SSI_COMMON_H__ */
