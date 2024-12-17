/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Rectifying MP13 Exceptions
 * Author: @CompanyNameTag
 */

/* 头文件包含 */
#include "oal_util.h"
#include "plat_pm.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "plat_firmware_download.h"
#include "board.h"
#include "bfgx_dev.h"
#include "bfgx_core.h"
#include "chr_errno.h"
#include "exception_common.h"
#include "wifi_exception_rst.h"
#include "bfgx_exception_rst.h"
#include "plat_exception_rst.h"
#include "mp13_exception_rst.h"

OAL_STATIC uint32_t is_single_wifi_mp13(void);
OAL_STATIC uint32_t is_ir_mode_need_dfr_mp13(uint32_t subsys_type);
OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag);

OAL_STATIC reset_handler_t g_w_sys_func = {
    .get_power_state = get_wifi_subsys_state,
    .pre_do = wifi_reset_pre_do,
    .power_reset = mp13_wifi_subsys_reset,  // ada有额外的外置buck控制，不能用公共接口
    .power_off = wifi_excp_power_off,
    .firmware_download_pre = wifi_firmware_download_pre,
    .firmware_download_do = wifi_firmware_download_do,
    .firmware_download_fail_do = NULL,
    .firmware_download_post = wifi_firmware_download_post,
    .post_do = wifi_reset_post_do
};

OAL_STATIC reset_handler_t g_b_sys_func = {
    .get_power_state = get_bfgx_subsys_state,
    .pre_do = bfgx_reset_pre_do,
    .power_reset = mp13_bfgx_subsys_reset, // ada有额外的外置buck控制，不能用公共接口
    .power_off = bfgx_excp_power_off,
    .firmware_download_pre = NULL,
    .firmware_download_do = bfgx_firmware_download_do,
    .firmware_download_fail_do = bfgx_download_fail_do,
    .firmware_download_post = bfgx_firmware_download_post,
    .post_do = bfgx_reset_post_do
};

OAL_STATIC excp_subsys_cfg_t g_subsys_cfg[] = {
    /* EXCP_W_SYS bus_id为EXCP_BUS_BUTT, pm_data获取为struct wlan_pm_s */
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
        .slave_pm_flag = OAL_FALSE,
        .core_cfg = NULL
    }
};

OAL_STATIC reset_handler_t g_reset_func = {
    .is_ir_mode_need_dfr = is_ir_mode_need_dfr_mp13,
    .is_single_wifi = is_single_wifi_mp13,
    .pre_do = chip_mpxx_pre_do,
    .power_reset = chip_mpxx_power_reset,
    .firmware_download_pre = chip_mpxx_firmware_download_pre,
    .firmware_download_do = chip_mpxx_firmware_download_do,
    .firmware_download_mode_combo = firmware_download_mode_combo_mp13,
    .firmware_download_post = chip_mpxx_firmware_download_post,
    .post_do = chip_mpxx_post_do
};

OAL_STATIC excp_search_table_t g_excp_table[EXCEPTION_TYPE_BOTTOM] = {
    [BUART_PANIC]             = {BUART_PANIC,             EXCP_B_SYS,      B_CPU,       NULL},
    [HCC_BUS_PANIC]           = {HCC_BUS_PANIC,           EXCP_W_SYS,      W_CPU,       NULL},
    [HCC_BUS_TRANS_FAIL]      = {HCC_BUS_TRANS_FAIL,      EXCP_W_SYS,      W_CPU,       NULL},
    [BUART_WAKEUP_FAIL]       = {BUART_WAKEUP_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [WIFI_WAKEUP_FAIL]        = {WIFI_WAKEUP_FAIL,        EXCP_W_SYS,      W_CPU,       NULL},
    [BT_POWERON_FAIL]         = {BT_POWERON_FAIL,         EXCP_B_SYS,      B_CPU,       NULL},
    [FM_POWERON_FAIL]         = {FM_POWERON_FAIL,         EXCP_B_SYS,      B_CPU,       NULL},
    [GNSS_POWERON_FAIL]       = {GNSS_POWERON_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [IR_POWERON_FAIL]         = {IR_POWERON_FAIL,         EXCP_B_SYS,      B_CPU,       NULL},
    [NFC_POWERON_FAIL]        = {NFC_POWERON_FAIL,        EXCP_B_SYS,      B_CPU,       NULL},
    [WIFI_POWERON_FAIL]       = {WIFI_POWERON_FAIL,       EXCP_W_SYS,      W_CPU,       NULL},
    [BUART_BEATHEART_TIMEOUT] = {BUART_BEATHEART_TIMEOUT, EXCP_B_SYS,      B_CPU,       NULL},
    [BFGX_TIMER_TIMEOUT]      = {BFGX_TIMER_TIMEOUT,      EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统复位 */
    [BFGX_ARP_TIMEOUT]        = {BFGX_ARP_TIMEOUT,        EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统复位 */
    [WIFI_WATCHDOG_TIMEOUT]   = {WIFI_WATCHDOG_TIMEOUT,   EXCP_W_SYS,      W_CPU,       NULL},
    [SDIO_DUMPBCPU_FAIL]      = {SDIO_DUMPBCPU_FAIL,      EXCP_W_SYS,      W_CPU,       NULL},
    [CHIP_FATAL_ERROR]        = {CHIP_FATAL_ERROR,        EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统恢复 */
    [BT_POWER_OFF_FAIL]       = {BT_POWER_OFF_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [FM_POWER_OFF_FAIL]       = {FM_POWER_OFF_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [GNSS_POWER_OFF_FAIL]     = {GNSS_POWER_OFF_FAIL,     EXCP_B_SYS,      B_CPU,       NULL},
    [IR_POWER_OFF_FAIL]       = {IR_POWER_OFF_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [NFC_POWER_OFF_FAIL]      = {NFC_POWER_OFF_FAIL,      EXCP_B_SYS,      B_CPU,       NULL},
};

/*
 * BFGX_BT = 0x00,
 * BFGX_FM = 0x01,
 * BFGX_GNSS = 0x02,
 * BFGX_IR = 0x03,
 * BFGX_NFC = 0x04,
 * BFGX_SLE = 0x05,
 * BFGX_ME  = 0x05, MP16 ME, MP16C SLE
 * BFGX_BUTT = 0x06
 */
OAL_STATIC uint32_t g_bgfx_poweron_fail_table[BFGX_BUTT] = {
    BT_POWERON_FAIL,
    FM_POWERON_FAIL,
    GNSS_POWERON_FAIL,
    IR_POWERON_FAIL,
    NFC_POWERON_FAIL,
    EXCEPTION_TYPE_BOTTOM /* 无SLE或ME场景 */
};

OAL_STATIC uint32_t g_bgfx_poweroff_fail_table[BFGX_BUTT] = {
    BT_POWER_OFF_FAIL,
    FM_POWER_OFF_FAIL,
    GNSS_POWER_OFF_FAIL,
    IR_POWER_OFF_FAIL,
    NFC_POWER_OFF_FAIL,
    EXCEPTION_TYPE_BOTTOM /* 无SLE或ME场景 */
};

OAL_STATIC uint32_t g_excp_sys_map[EXCP_SYS_BUTT] = {
    W_SYS,
    B_SYS,
    SYS_BUTT,
    SYS_BUTT,
    SYS_BUTT
};

OAL_STATIC excp_panic_table_t g_panic_table[] = {
    {BUART_PANIC, BUART}
};

OAL_STATIC excp_chip_cfg_t g_chip_cfg = {
    .chip_type = BOARD_VERSION_MP13,
    .chip_name = BOARD_VERSION_NAME_MP13,
    .subsys_cnt = oal_array_size(g_subsys_cfg),
    .subsys_cfg = g_subsys_cfg,
    .chip_reset_func = &g_reset_func,
    .excp_search = g_excp_table,
    .excp_search_size = EXCEPTION_TYPE_BOTTOM,
    .bfgx_poweron_fail = g_bgfx_poweron_fail_table,
    .poweron_fail_size = BFGX_BUTT,
    .bfgx_poweroff_fail = g_bgfx_poweroff_fail_table,
    .poweroff_fail_size = BFGX_BUTT,
    .excp_sys_map = g_excp_sys_map,
    .sys_map_size = EXCP_SYS_BUTT,
    .panic_table = g_panic_table,
    .panic_table_size = oal_array_size(g_panic_table)
};

OAL_STATIC uint32_t is_single_wifi_mp13(void)
{
    if (bfgx_is_shutdown() && (!wlan_is_shutdown())) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/* 返回false不需要做dfr */
OAL_STATIC uint32_t is_ir_mode_need_dfr_mp13(uint32_t subsys_type)
{
    if ((ps_core_ir_only_mode() == OAL_TRUE) && subsys_type == EXCP_B_SYS) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag)
{
    bfgx_subsys_power_off_proc(node_ptr, flag, B_SYS);
}

int32_t firmware_download_mode_combo_mp13(excp_comm_t *node_ptr)
{
    int32_t ret = EXCEPTION_SUCCESS;
    excp_chip_cfg_t *chip_cfg = NULL;
    excp_chip_res_t *chip_res = oal_container_of(node_ptr, excp_chip_res_t, node);

    chip_cfg = chip_res->chip_cfg;
    if (oal_unlikely(chip_cfg->chip_reset_func->is_single_wifi == NULL)) {
        ps_print_err("%s: is_single_wifi is NULL\n", __func__);
        return -EXCEPTION_FAIL;
    }

    /* 混合加载: 单wifi场景, 全部加载 */
    if (chip_cfg->chip_reset_func->is_single_wifi()) {
        ps_print_info("%s: firmware_download BFGX_AND_WIFI_CFG"NEWLINE, __func__);
        ret = firmware_download_function(BFGX_AND_WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
        return ret;
    }

    if (bfgx_is_shutdown() == false) {
        ps_print_info("%s: firmware_download BFGX_CFG"NEWLINE, __func__);
        ret = firmware_download_function(BFGX_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
        wifi_power_reset(NULL);
    }

    if (wlan_is_shutdown() == false) {
        ps_print_info("%s: firmware_download WIFI_CFG"NEWLINE, __func__);
        ret = firmware_download_function(WIFI_CFG, hcc_get_bus(HCC_EP_WIFI_DEV));
    }

    board_sys_disable(B_SYS);

    if (bfgx_is_shutdown() == false) {
        board_sys_enable(B_SYS);
    }
    return ret;
}

excp_chip_cfg_t *get_mp13_chip_cfg(void)
{
    return &g_chip_cfg;
}
