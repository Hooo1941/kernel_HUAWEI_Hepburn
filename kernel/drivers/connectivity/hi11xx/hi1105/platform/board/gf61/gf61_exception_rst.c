/**
 * Copyright (c) @CompanyNameMagicTag 2022-2023. All rights reserved.
 *
 * Description: Rectifying gf61 Exceptions.
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
#include "bfgx_data_parse.h"
#include "exception_common.h"
#include "gpio_gf61.h"
#include "pcie_host.h"
#include "wifi_exception_rst.h"
#include "bfgx_exception_rst.h"
#include "plat_exception_rst.h"
#include "gf61_exception_rst.h"

OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag);
OAL_STATIC int32_t gle_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record);
OAL_STATIC void glf_excp_power_off(excp_comm_t *node_ptr, uint8_t flag);
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

OAL_STATIC reset_handler_t g_b_sys_func = {
    .get_power_state = get_bfgx_subsys_state,
    .pre_do = bfgx_reset_pre_do,
    .power_reset = bfgx_subsys_power_reset,
    .power_off = bfgx_excp_power_off,
    .firmware_download_pre = NULL,
    .firmware_download_do = bfgx_firmware_download_do,
    .firmware_download_fail_do = bfgx_download_fail_do,
    .firmware_download_post = bfgx_firmware_download_post,
    .post_do = bfgx_reset_post_do
};

/* gle 系统不支持硬件en */
OAL_STATIC reset_handler_t g_gle_sys_func = {
    .get_power_state = get_bfgx_subsys_state,
    .pre_do = gle_reset_pre_do,
    .power_reset = NULL,
    .power_off = glf_excp_power_off,
    .firmware_download_pre = NULL,
    .firmware_download_do = bfgx_firmware_download_do,
    .firmware_download_fail_do = bfgx_download_fail_do,
    .firmware_download_post = bfgx_firmware_download_post,
    .post_do = bfgx_reset_post_do
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
        .slave_pm_flag = OAL_FALSE,
        .core_cfg = NULL
    },
    {
        .subsys_idx = EXCP_SYS2,
        .subsys_type = EXCP_GLF_SYS,
        .subsys_name = "EXCP_GLE_SYS",
        .which_cfg = SLE_CFG,
        .chr_id = CHR_PLAT_DRV_ERROR_GLF_SYSTEM_RST,
        .bus_id = GUART,
        .subsys_reset_func = &g_gle_sys_func,
        .slave_pm_flag = OAL_FALSE,
        .core_cfg = NULL
    }
};

OAL_STATIC reset_handler_t g_reset_handler = {
    .is_ir_mode_need_dfr = NULL,
    .is_chip_force_reset = NULL,
    .is_single_wifi = NULL,
    .pre_do = chip_mpxx_pre_do,
    .power_reset = chip_mpxx_power_reset,
    .firmware_download_pre =  chip_mpxx_firmware_download_pre,
    .firmware_download_do = chip_mpxx_firmware_download_do,
    .firmware_download_mode_combo = NULL,
    .firmware_download_fail_do = NULL,
    .firmware_download_post = chip_mpxx_firmware_download_post,
    .post_do = chip_mpxx_post_do
};

OAL_STATIC excp_search_table_t g_excp_table[EXCEPTION_TYPE_BOTTOM] = {
    [BUART_PANIC]             = {BUART_PANIC,             EXCP_B_SYS,      B_CPU,       NULL},
    [GUART_PANIC]             = {GUART_PANIC,             EXCP_GLF_SYS,    SLE_CPU,     NULL},
    [HCC_BUS_PANIC]           = {HCC_BUS_PANIC,           EXCP_W_SYS,      W_CPU,       NULL},
    [HCC_BUS_TRANS_FAIL]      = {HCC_BUS_TRANS_FAIL,      EXCP_W_SYS,      W_CPU,       NULL},
    [BUART_WAKEUP_FAIL]       = {BUART_WAKEUP_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [GUART_WAKEUP_FAIL]       = {GUART_WAKEUP_FAIL,       EXCP_ALL_SYS,    ALL_CORE,    NULL},
    [WIFI_WAKEUP_FAIL]        = {WIFI_WAKEUP_FAIL,        EXCP_W_SYS,      W_CPU,       NULL},
    [BT_POWERON_FAIL]         = {BT_POWERON_FAIL,         EXCP_B_SYS,      B_CPU,       NULL},
    [WIFI_POWERON_FAIL]       = {WIFI_POWERON_FAIL,       EXCP_W_SYS,      W_CPU,       NULL},
    [SLE_POWERON_FAIL]        = {SLE_POWERON_FAIL,        EXCP_ALL_SYS,    ALL_CORE,    NULL},
    [BUART_BEATHEART_TIMEOUT] = {BUART_BEATHEART_TIMEOUT, EXCP_B_SYS,      B_CPU,       NULL},
    [GUART_BEATHEART_TIMEOUT] = {GUART_BEATHEART_TIMEOUT, EXCP_ALL_SYS,    ALL_CORE,    NULL},
    [BFGX_TIMER_TIMEOUT]      = {BFGX_TIMER_TIMEOUT,      EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统恢复 */
    [BFGX_ARP_TIMEOUT]        = {BFGX_ARP_TIMEOUT,        EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统恢复 */
    [WIFI_WATCHDOG_TIMEOUT]   = {WIFI_WATCHDOG_TIMEOUT,   EXCP_W_SYS,      W_CPU,       NULL},
    [CHIP_FATAL_ERROR]        = {CHIP_FATAL_ERROR,        EXCP_ALL_SYS,    ALL_CORE,    NULL}, /* 全系统恢复 */
    [BT_POWER_OFF_FAIL]       = {BT_POWER_OFF_FAIL,       EXCP_B_SYS,      B_CPU,       NULL},
    [SLE_POWER_OFF_FAIL]      = {SLE_POWER_OFF_FAIL,      EXCP_ALL_SYS,    ALL_CORE,    NULL},
};

/*
 * BFGX_BT = 0x00,
 * BFGX_FM = 0x01,
 * BFGX_GNSS = 0x02,
 * BFGX_IR = 0x03,
 * BFGX_NFC = 0x04,
 * BFGX_SLE = 0x05,
 * BFGX_BUTT = 0x06
 */
OAL_STATIC uint32_t g_bgfx_poweron_fail_table[BFGX_BUTT] = {
    BT_POWERON_FAIL,
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    SLE_POWERON_FAIL
};

OAL_STATIC uint32_t g_bgfx_poweroff_fail_table[BFGX_BUTT] = {
    BT_POWER_OFF_FAIL,
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    EXCEPTION_TYPE_BOTTOM, // 不支持
    SLE_POWER_OFF_FAIL
};

OAL_STATIC uint32_t g_excp_sys_map[EXCP_SYS_BUTT] = {
    W_SYS,
    B_SYS,
    G_SYS,
    SYS_BUTT,
    SYS_BUTT
};

OAL_STATIC excp_panic_table_t g_panic_table[] = {
    {BUART_PANIC, BUART},
    {GUART_PANIC, GUART}
};

OAL_STATIC excp_chip_cfg_t g_chip_cfg = {
    .chip_type = BOARD_VERSION_GF61,
    .chip_name = BOARD_VERSION_NAME_GF61,
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
    .panic_table_size = oal_array_size(g_panic_table)
};

OAL_STATIC void bfgx_excp_power_off(excp_comm_t *node_ptr, uint8_t flag)
{
    bfgx_subsys_power_off_proc(node_ptr, flag, B_SYS);
}

OAL_STATIC void glf_excp_power_off(excp_comm_t *node_ptr, uint8_t flag)
{
    bfgx_subsys_power_off_proc(node_ptr, flag, G_SYS);
}

STATIC void ssi_clear_gle_soften(void)
{
    const uint32_t sle_soft_en = 0x400007b8;
    if (ssi_write_value32_test(sle_soft_en, 0x0) != BOARD_SUCC) {
        ps_print_err("sle soft en cfg fail\n");
    }
}

STATIC int32_t hcc_clear_gle_soften(void)
{
    int32_t ret;
    hcc_bus *hbus = NULL;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    uint8_t *dev_write_msg = "WRITEM 2 0x400007b8 0x0 ";

    hbus = hcc_get_bus(HCC_EP_WIFI_DEV);

    if (wlan_is_shutdown() == OAL_FALSE) {
        if (pst_wlan_pm->wlan_pm_enable == OAL_FALSE) {
            hcc_bus_wakeup_request(hbus);
        }
    } else {
        board_sys_enable(W_SYS);
        ret = hcc_bus_reinit(hbus);
        if (ret < 0) {
            ps_print_err("reset bus fail\n");
            return -EFAIL;
        }
    }

    ret = hcc_bus_patch_write(hbus, dev_write_msg, strlen(dev_write_msg));
    if (ret != 0) {
        ps_print_err("hcc write fail\n");
        return -EFAIL;
    }

    return SUCC;
}

STATIC void clear_gle_soften(void)
{
    int32_t ret;
    ret = hcc_clear_gle_soften();
    if (ret == SUCC) {
        return;
    }

    ssi_clear_gle_soften();
}

OAL_STATIC int32_t gle_reset_pre_do(excp_comm_t *node_ptr, excp_record_t *excp_record)
{
    excp_subsys_res_t *subsys_res = oal_container_of(node_ptr, excp_subsys_res_t, node);
    excp_subsys_cfg_t *subsys_cfg = NULL;

    if (oal_unlikely(subsys_res == NULL)) {
        ps_print_err("%s: subsys_res is NULL\n", __func__);
        return -EFAIL;
    }

    subsys_cfg = subsys_res->subsys_cfg;

    // 判断在单子系统复位的时候，强制清soft en然后触发全系统复位
    if (excp_record->subsys_type != EXCP_ALL_SYS) {
        ps_print_info("gle clear soften\n");
        clear_gle_soften();
        return -EFAIL;
    }

    bfgx_reset_pre_do(node_ptr, excp_record);
    return SUCC;
}

excp_chip_cfg_t *get_gf61_chip_cfg(void)
{
    return &g_chip_cfg;
}
