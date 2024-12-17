/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Rectifying MP16 Exceptions
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "plat_pm.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "chr_errno.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "exception_common.h"
#include "wifi_exception_rst.h"
#include "bfgx_exception_rst.h"
#include "plat_exception_rst.h"
#include "mp13_exception_rst.h"
#include "mp16_exception_rst.h"

OAL_STATIC uint32_t is_single_wifi_mp16(void);
OAL_STATIC int32_t mp16_g_cpu_is_active(excp_comm_t *node_ptr);
OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag);

/* g_cpu处理 */
reset_handler_t g_mp16_g_func = {
    .get_power_state = mp16_g_cpu_is_active,
    .pre_do = bfgx_slave_core_reset_pre_do,
    .power_reset = NULL,
    .power_off = NULL,
    .firmware_download_pre = NULL,
    .firmware_download_do = NULL,
    .firmware_download_fail_do = bfgx_slave_core_download_fail_do,
    .firmware_download_post = NULL,
    .post_do = bfgx_slave_core_reset_post_do
};

/* 主核b_cpu看到的需要处理的从核 */
excp_sla_core_cfg_t g_mp16_sla_cpu_cfg[] = {
    {
        .sla_core_idx = EXCP_CORE0,
        .sla_core_type = G_CPU,
        .bus_id = GUART,
        .sla_core_name = "G_CPU",
        .sla_reset_func = &g_mp16_g_func
    }
};

OAL_STATIC reset_handler_t g_b_cpu_func = {
    .pre_do = bfgx_master_core_reset_pre_do,
    .power_reset = NULL,
    .power_off = NULL,
    .firmware_download_pre = NULL,
    .firmware_download_do = NULL,
    .firmware_download_fail_do = bfgx_master_core_download_fail_do,
    .firmware_download_post = bfgx_master_core_firmware_download_post,
    .post_do = bfgx_master_core_reset_post_do
};

excp_core_cfg_t g_mp16_b_cpu_cfg[] = {
    {
        .master_core_type = B_CPU,
        .master_core_name = "B_CPU",
        .master_reset_func = &g_b_cpu_func,
        .sla_core_cnt = oal_array_size(g_mp16_sla_cpu_cfg),
        .sla_core_cfg = g_mp16_sla_cpu_cfg,
    }
};

OAL_STATIC reset_handler_t g_w_sys_func = {
    .get_power_state = get_wifi_subsys_state,
    .pre_do = wifi_reset_pre_do,
    .power_reset = wifi_power_reset,
    .power_off = wifi_excp_power_off,
    .firmware_download_pre = wifi_firmware_download_pre,
    .firmware_download_do = wifi_firmware_download_do,
    .firmware_download_fail_do = NULL,
    .firmware_download_post = wifi_firmware_download_post,
    .post_do = wifi_reset_post_do
};

/* b_cpu & g_cpu */
OAL_STATIC reset_handler_t g_b_sys_func = {
    .get_power_state = get_bfgx_subsys_state,
    .pre_do = subsys_all_core_pre_proc,
    .power_reset = bfgx_subsys_power_reset,
    .power_off = bfgx_excp_power_off,
    .firmware_download_pre = NULL,
    .firmware_download_do = bfgx_firmware_download_do,
    .firmware_download_fail_do = subsys_all_core_download_fail_do,
    .firmware_download_post = subsys_all_core_firmware_download_post,
    .post_do = subsys_all_core_reset_post_do
};

OAL_STATIC excp_subsys_cfg_t g_subsys_cfg[] = {
    {
        .subsys_idx = EXCP_SYS0,
        .subsys_type = EXCP_W_SYS,
        .subsys_name = "EXCP_W_SYS",
        .which_cfg = WIFI_CFG,
        .chr_id = CHR_PLAT_DRV_ERROR_W_SYSTEM_RST,
        .bus_id = UART_BUTT,
        .subsys_reset_func = &g_w_sys_func,
        .slave_pm_flag = OAL_FALSE,
        .core_cfg = NULL
    },
    {
        .subsys_idx = EXCP_SYS1,
        .subsys_type = EXCP_B_SYS,
        .subsys_name = "EXCP_B_SYS",
        .which_cfg = BFGX_CFG,
        .chr_id = CHR_PLAT_DRV_ERROR_B_SYSTEM_RST,
        .bus_id = BUART,
        .subsys_reset_func = &g_b_sys_func,
        .slave_pm_flag = OAL_TRUE,
        .core_cfg = g_mp16_b_cpu_cfg
    }
};

OAL_STATIC reset_handler_t g_reset_handler = {
    .is_single_wifi = is_single_wifi_mp16,
    .pre_do = chip_mpxx_pre_do,
    .power_reset = chip_mpxx_power_reset,
    .firmware_download_pre =  chip_mpxx_firmware_download_pre,
    .firmware_download_do = chip_mpxx_firmware_download_do,
    .firmware_download_mode_combo = firmware_download_mode_combo_mp13,
    .firmware_download_fail_do = NULL,
    .firmware_download_post = chip_mpxx_firmware_download_post,
    .post_do = chip_mpxx_post_do
};

OAL_STATIC excp_search_table_t g_excp_table[EXCEPTION_TYPE_BOTTOM] = {
    [BUART_PANIC]             = {BUART_PANIC,             EXCP_B_SYS,     B_CPU,       NULL},
    [GUART_PANIC]             = {GUART_PANIC,             EXCP_B_SYS,     B_CPU,       NULL},
    [HCC_BUS_PANIC]           = {HCC_BUS_PANIC,           EXCP_W_SYS,     W_CPU,       NULL},
    [HCC_BUS_TRANS_FAIL]      = {HCC_BUS_TRANS_FAIL,      EXCP_W_SYS,     W_CPU,       NULL},
    [BUART_WAKEUP_FAIL]       = {BUART_WAKEUP_FAIL,       EXCP_B_SYS,     B_CPU,       NULL},
    [GUART_WAKEUP_FAIL]       = {GUART_WAKEUP_FAIL,       EXCP_B_SYS,     B_CPU,       NULL},
    [WIFI_WAKEUP_FAIL]        = {WIFI_WAKEUP_FAIL,        EXCP_W_SYS,     W_CPU,       NULL},
    [BT_POWERON_FAIL]         = {BT_POWERON_FAIL,         EXCP_B_SYS,     B_CPU,       NULL},
    [FM_POWERON_FAIL]         = {FM_POWERON_FAIL,         EXCP_B_SYS,     B_CPU,       NULL},
    [GNSS_POWERON_FAIL]       = {GNSS_POWERON_FAIL,       EXCP_B_SYS,     UNKOWN_CORE, NULL}, /* GNSS异常，子系统恢复 */
    [IR_POWERON_FAIL]         = {IR_POWERON_FAIL,         EXCP_B_SYS,     B_CPU,       NULL},
    [NFC_POWERON_FAIL]        = {NFC_POWERON_FAIL,        EXCP_B_SYS,     UNKOWN_CORE, NULL},
    [WIFI_POWERON_FAIL]       = {WIFI_POWERON_FAIL,       EXCP_W_SYS,     W_CPU,       NULL},
    [SLE_POWERON_FAIL]        = {SLE_POWERON_FAIL,        EXCP_SYS_BUTT,  CORE_BUTT,   NULL},
    [BUART_BEATHEART_TIMEOUT] = {BUART_BEATHEART_TIMEOUT, EXCP_B_SYS,     UNKOWN_CORE, NULL},
    [GUART_BEATHEART_TIMEOUT] = {GUART_BEATHEART_TIMEOUT, EXCP_B_SYS,     B_CPU,       NULL},
    [BFGX_TIMER_TIMEOUT]      = {BFGX_TIMER_TIMEOUT,      EXCP_ALL_SYS,   ALL_CORE,    NULL}, /* 全系统恢复 */
    [BFGX_ARP_TIMEOUT]        = {BFGX_ARP_TIMEOUT,        EXCP_ALL_SYS,   ALL_CORE,    NULL}, /* 全系统恢复 */
    [WIFI_WATCHDOG_TIMEOUT]   = {WIFI_WATCHDOG_TIMEOUT,   EXCP_W_SYS,     W_CPU,       NULL},
    [SDIO_DUMPBCPU_FAIL]      = {SDIO_DUMPBCPU_FAIL,      EXCP_W_SYS,     W_CPU,       NULL},
    [CHIP_FATAL_ERROR]        = {CHIP_FATAL_ERROR,        EXCP_ALL_SYS,   ALL_CORE,    NULL}, /* 全系统恢复 */
    [BT_POWER_OFF_FAIL]       = {BT_POWER_OFF_FAIL,       EXCP_B_SYS,     B_CPU,       NULL},
    [FM_POWER_OFF_FAIL]       = {FM_POWER_OFF_FAIL,       EXCP_B_SYS,     B_CPU,       NULL},
    [GNSS_POWER_OFF_FAIL]     = {GNSS_POWER_OFF_FAIL,     EXCP_B_SYS,     UNKOWN_CORE, NULL},
    [IR_POWER_OFF_FAIL]       = {IR_POWER_OFF_FAIL,       EXCP_B_SYS,     B_CPU,       NULL},
    [NFC_POWER_OFF_FAIL]      = {NFC_POWER_OFF_FAIL,      EXCP_B_SYS,     B_CPU,       NULL},
};

/*
 * BFGX_BT = 0x00,
 * BFGX_FM = 0x01,
 * BFGX_GNSS = 0x02,
 * BFGX_IR = 0x03,
 * BFGX_NFC = 0x04,
 * BFGX_SLE = 0x05,
 * BFGX_ME  = 0x05, mp16 ME, mp16 SLE
 * BFGX_BUTT = 0x06
 */
OAL_STATIC uint32_t g_bgfx_poweron_fail_table[BFGX_BUTT] = {
    BT_POWERON_FAIL,
    FM_POWERON_FAIL,
    GNSS_POWERON_FAIL,
    IR_POWERON_FAIL,
    NFC_POWERON_FAIL,
    GNSS_POWERON_FAIL
};

OAL_STATIC uint32_t g_bgfx_poweroff_fail_table[BFGX_BUTT] = {
    BT_POWER_OFF_FAIL,
    FM_POWER_OFF_FAIL,
    GNSS_POWER_OFF_FAIL,
    IR_POWER_OFF_FAIL,
    NFC_POWER_OFF_FAIL,
    GNSS_POWER_OFF_FAIL
};

OAL_STATIC uint32_t g_excp_sys_map[EXCP_SYS_BUTT] = {
    W_SYS,
    B_SYS,
    SYS_BUTT,
    SYS_BUTT,
    SYS_BUTT
};

OAL_STATIC excp_panic_table_t g_panic_table[] = {
    {BUART_PANIC, BUART},
    {GUART_PANIC, GUART}
};

OAL_STATIC excp_chip_cfg_t g_chip_cfg = {
    .chip_type = BOARD_VERSION_MP16,
    .chip_name = BOARD_VERSION_NAME_MP16,
    .subsys_cnt = oal_array_size(g_subsys_cfg),
    .subsys_cfg = g_subsys_cfg,
    .chip_reset_func = &g_reset_handler,
    .excp_search = g_excp_table,
    .excp_search_size = EXCEPTION_TYPE_BOTTOM,
    .bfgx_poweron_fail = g_bgfx_poweron_fail_table,
    .poweron_fail_size = BFGX_BUTT,
    .bfgx_poweroff_fail = g_bgfx_poweroff_fail_table,
    .poweroff_fail_size = BFGX_BUTT,
    .excp_sys_map = g_excp_sys_map,
    .sys_map_size = EXCP_SYS_BUTT,
    .panic_table = g_panic_table,
    .panic_table_size = oal_array_size(g_panic_table),
};

OAL_STATIC uint32_t is_single_wifi_mp16(void)
{
    if (bfgx_is_shutdown() && (!wlan_is_shutdown())) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag)
{
    bfgx_subsys_power_off_proc(node_ptr, flag, B_SYS);
}

OAL_STATIC int32_t mp16_g_cpu_is_active(excp_comm_t *node_ptr)
{
    int32_t state;
    struct ps_core_s *ps_core_d = ps_get_core_reference(GUART);
    if (ps_core_d == NULL) {
        ps_print_info("bfgx not init\n");
        return true;
    }

    state = ps_core_chk_bfgx_active(ps_core_d);
    return state;
}

excp_chip_cfg_t *get_mp16_chip_cfg(void)
{
    return &g_chip_cfg;
}
